#include <zarks/image/GIF.h>
#include <zarks/io/binary.h>
#include <zarks/io/BitBuffer.h>
#include <zarks/io/logdefs.h>
#include <zarks/io/LZWTree.h>
#include <zarks/math/KMeans.h>

#include <fstream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <random>

namespace zmath
{

// For structs and exceptions in GIFStructs.h
using namespace gif;

//               //
// K-Means Types //
//               //

struct RGBACounter
{
    unsigned R, G, B;
    RGBACounter() : R(0), G(0), B(0) {}

    void operator+=(zmath::RGBA c)
    {
        R += c.R;
        G += c.G;
        B += c.B;
    }

    RGBA operator/(size_t s)
    {
        return RGBA(
            R / s,
            G / s,
            B / s);
    }
};

struct RGBADist
{
    double operator()(RGBA c1, RGBA c2)
    {
        return RGBA::Distance(c1, c2);
    }
};

//     //
// GIF //
//     //

GIF::GIF()
{}

GIF::GIF(std::string path)
    : GIF(std::ifstream(path, std::ios_base::binary))
{}

GIF::GIF(std::istream&& is)
    : GIF(is)
{}

GIF::GIF(std::istream& is)
{
    // Check validity of stream
    if (!is)
    {
        throw std::runtime_error("Invalid input stream while reading GIF");
    }

    // Read 6-byte header and make sure GIF is either 89a or 87a format
    char header[7]{};
    is.read(header, 6);
    if (std::strcmp(header, HEADER_87a) &&
        std::strcmp(header, HEADER_89a))
    {
        throw std::runtime_error(std::string("Invalid GIF header: ") + header);
    }
    
    // Read the 7-byte logical screen descriptor
    uint8_t descriptor[7], *ptr = descriptor;
    is.read((char*)descriptor, 7);
    uint16_t canvasWidth = FromBytes<uint16_t>(AdvancePtr(ptr, 2), Endian::Little);
    uint16_t canvasHeight = FromBytes<uint16_t>(AdvancePtr(ptr, 2), Endian::Little);
    LSDFlags flags(*ptr);
    // Ignore the last two bytes of the descriptor. The following link
    // explains why the 6th and 7th bytes, which represent the background
    // color index and the pixel aspect ratio, respectively, have no use
    // in a modern GIF viewer:
    // http://giflib.sourceforge.net/whatsinagif/bits_and_bytes.html

    // If applicable, load the global color table
    std::vector<RGBA> globalColorTable;
    if (flags.globalTableFlag)
    {
        globalColorTable = loadColorTable(is, loadColorTableSize(flags.colorTableSize));
    }

    // Process blocks for as long as needed
    VecInt bounds(canvasWidth, canvasHeight);
    while (true)
    {
        // Try to load next frame
        try
        {
            auto pair = loadNextFrame(is, bounds, globalColorTable);
            frames.push_back(pair.first);
            LOG_DEBUG("Loaded frame #" << frames.size());
        }
        // This exception signifies that the correctly-formatted end of
        // the GIF has been found
        catch (const gif::EndOfStreamException& e)
        {
            break;
        }
        // This exception will only be thrown if the decoder ran into
        // an issue with the GIF being loaded
        catch (const gif::GifLoadingException& e)
        {
            LOG_ERROR(e);
            LOG_ERROR(" -> Stream position: " << is.tellg());
            break;
        }
    }
}

void GIF::Save(std::string path,
               VecInt bounds,
               const std::vector<RGBA>& palette,
               const std::vector<double>& durations) const
{
    // Check stream
    std::ofstream os(path, std::ios_base::binary);
    if (!os)
    {
        throw std::runtime_error("Could not open file at " + path);
    }

    // Make sure there are frames to write
    if (frames.empty())
    {
        throw std::runtime_error("Tried to save an empty GIF");
    }

    // Determine bounds
    if (bounds == VecInt()) bounds = Bounds();

    // Determine palette
    const std::vector<RGBA>& finalPalette = (palette.size() < 2) ?
                                             getDefaultPalette(256) :
                                             palette;

    // Determine durations
    const std::vector<double>& finalDurations = (durations.empty()) ?
                                                std::vector<double>{ 0.1 } :
                                                durations;

    // Output standard log message
    LOG_INFO("Saving GIF of size " << bounds << " and palette size " <<
             computeColorTableSize(finalPalette) << " at " << path);
    
    // Write header
    os.write(HEADER_89a, HEADER_SIZE);

    // Write LSD
    writeLSD(os, bounds, true, computeColorTableSize(palette));

    // Write global color table
    writeColorTable(os, finalPalette);

    // Write Netscape 2.0 looping extension
    writeNetscape2Extension(os);

    // Write all frames
    for (size_t i = 0; i < frames.size(); i++)
    {
        writeGraphicsExtension(os, finalDurations[i % finalDurations.size()]);
        writeFrame(os, frames[i], bounds, finalPalette, false);
    }

    // Write final GIF terminator byte
    os.put((char)BlockType::END_OF_FILE);
}

void GIF::Save(std::string path,
               bool global,
               VecInt bounds,
               unsigned paletteSize,
               const std::vector<double>& durations) const
{
    // Check stream
    std::ofstream os(path, std::ios_base::binary);
    if (!os)
    {
        throw std::runtime_error("Could not open file at " + path);
    }

    // Make sure there are frames to write
    if (frames.empty())
    {
        throw std::runtime_error("Tried to save an empty GIF");
    }

    // Determine bounds
    if (bounds == VecInt()) bounds = Bounds();

    // Output standard log message
    LOG_INFO("Saving GIF of size " << bounds << " and palette size " <<
             computeColorTableSize(paletteSize) << " at " << path);

    // Write header
    os.write(HEADER_89a, HEADER_SIZE);

    // Write LSD
    writeLSD(os, bounds, global, computeColorTableSize(paletteSize));

    // Create and write global table, if applicable
    std::vector<RGBA> globalColorTable;
    if (global)
    {
        globalColorTable = getDefaultPalette(paletteSize);
        writeColorTable(os, globalColorTable);
    }

    // Write Netscape 2.0 looping extension
    writeNetscape2Extension(os);

    // Determine durations
    const std::vector<double>& finalDurations = (durations.empty()) ?
                                                std::vector<double>{ 0.1 } :
                                                durations;

    // Write all frames
    for (size_t i = 0; i < frames.size(); i++)
    {
        LOG_DEBUG("Writing frame #" << i);
        const std::vector<RGBA>& colorTableToUse = (global) ?
                                                    globalColorTable :
                                                    getKMeansPalette(frames[i], paletteSize);

        writeGraphicsExtension(os, finalDurations[i % finalDurations.size()]);
        writeFrame(os, frames[i], bounds, colorTableToUse, !global);
    }

    // Write final GIF terminator byte
    os.put((char)BlockType::END_OF_FILE);
}

void GIF::Add(const Image& img, bool adjustBounds, int idx)
{
    if (idx == -1)
    {
        frames.push_back(img);
    }
    else
    {
        auto iter = frames.begin();
        std::advance(iter, idx);
        frames.insert(iter, img);
    }
}

Image& GIF::At(int frame)
{
    if (frame == -1)
        return frames.back();
    else
        return frames.at(frame);
}

const Image& GIF::At(int frame) const
{
    if (frame == -1)
        return frames.back();
    else
        return frames.at(frame);
}

int GIF::Size() const
{
    return frames.size();
}

VecInt GIF::Bounds() const
{
    if (frames.size())
        return frames[0].Bounds();
    else
        return VecInt(0, 0);
}

//                      //
// Helpful SAVE Methods //
//                      //

unsigned GIF::computeColorTableSize(const std::vector<RGBA>& palette)
{
    return computeColorTableSize(palette.size());
}

unsigned GIF::computeColorTableSize(unsigned numColors)
{
    if (numColors < 2)
        return 256;
    else
        return std::min(256U, (unsigned)std::pow(2, std::ceil(std::log2(numColors))));
}

std::vector<RGBA> GIF::getDefaultPalette(unsigned numColors)
{
    // Determine the number of colors for this palette
    numColors = computeColorTableSize(numColors);
    LOG_DEBUG("Creating default palette of size " << numColors);

    // Initialize the palette vector
    std::vector<RGBA> palette(numColors);
    unsigned idx = 0;
    int root3 = std::floor(std::pow(numColors, 1.0 / 3.0));
    for (int r = 0; r < root3; r++)
        for (int g = 0; g < root3; g++)
            for (int b = 0; b < root3; b++)
                palette[idx++] = RGBA(
                    (uint8_t)std::floor(255.99 * (r / std::max(1.0, (double)root3 - 1))),
                    (uint8_t)std::floor(255.99 * (g / std::max(1.0, (double)root3 - 1))),
                    (uint8_t)std::floor(255.99 * (b / std::max(1.0, (double)root3 - 1))));

    // Fill the rest of the palette with pseudorandom values
    std::mt19937 rng;
    while (idx < numColors)
    {
        RGBA c;
        uint32_t rand = rng();
        c.R = rand;
        c.G = rand >> 8;
        c.B = rand >> 16;

        palette[idx++] = c;
    }

    return palette;
}

std::vector<RGBA> GIF::getKMeansPalette(const Image& frame, unsigned numColors)
{
    std::vector<RGBA> palette = getDefaultPalette(numColors);

    // Run K Means algorithm
    LOG_DEBUG("Running K Means algorithm on palette of size " << palette.size());
    ComputeKMeans<RGBA, RGBADist, RGBACounter>(palette, frame);

    return palette;
}

void GIF::writeLSD(std::ostream& os, VecInt bounds, bool globalTable, unsigned globalTableSize)
{
    uint8_t lsd[7]{};
    ToBytes<uint16_t>(lsd, bounds.X, Endian::Little);
    ToBytes<uint16_t>(lsd + 2, bounds.Y, Endian::Little);
    if (globalTable)
    {
        lsd[4] = 0b10000000;
        int colorTableBits = std::log2(globalTableSize) - 1;
        lsd[4] |= colorTableBits << 4;
        lsd[4] |= colorTableBits;
    }
    os.write((char*)lsd, 7);
}

void GIF::writeNetscape2Extension(std::ostream& os, uint16_t numReps)
{
    os.put((uint8_t)BlockType::EXTENSION);
    os.put((uint8_t)ExtensionType::APPLICATION);
    os.put(11); // length of this sub-block
    os.write("NETSCAPE2.0", 11);
    os.put(3); // length of next sub-block
    os.put(1);
    WriteBuf(os, numReps, Endian::Little);
    os.put(0); // null block terminator
}

void GIF::writeGraphicsExtension(std::ostream& os, double duration)
{
    LOG_DEBUG("Writing graphics extension with duration " << duration);

    // Determine the duration of this frame
    duration = std::max(0.0, std::min(duration, 655.35));
    uint16_t intDur = (uint16_t)(std::round(duration * 100));

    // Create temporary buffer
    uint8_t buf[8]{};
    buf[0] = (uint8_t)BlockType::EXTENSION;
    buf[1] = (uint8_t)ExtensionType::GRAPHICS;
    buf[2] = 4; // 4 bytes follow this one, plus a null terminator 
    ToBytes(buf + 4, intDur, Endian::Little);

    // Write temporary buffer to output stream
    os.write((char*)buf, 8);
}

void GIF::writeColorTable(std::ostream& os, const std::vector<RGBA>& palette)
{
    // Make sure palette size is at least 2
    if (palette.size() < 2)
    {
        throw std::runtime_error("GIF palette must have at least 2 colors");
    }

    // Determine used and unused colors
    unsigned usedColors = std::min(palette.size(), 256UL);
    unsigned totalColors = computeColorTableSize(palette);
    unsigned unusedColors = totalColors - usedColors;
    LOG_DEBUG("Writing color table with " << usedColors << " used colors and " <<
              unusedColors << " unused colors");

    // Write all used colors to the stream
    for (unsigned i = 0; i < usedColors; i++)
    {
        RGBA col = palette[i];
        uint8_t rgb[3] { col.R, col.G, col.B };
        os.write((char*)rgb, 3);
    }

    // Write all unused colors
    if (unusedColors)
    {
        char blanks[254 * 3]{}; // holds maximum possible length of unused color bytes
        os.write(blanks, unusedColors * 3);
    }
}

void GIF::writeFrame(std::ostream& os, const Image& frame, VecInt bounds, const std::vector<RGBA>& palette, bool writeTable)
{
    // Write beginning of image block
    uint8_t buf[10]{};
    buf[0] = (uint8_t)BlockType::IMAGE;
    ToBytes<uint16_t>(buf + 5, bounds.X, Endian::Little);
    ToBytes<uint16_t>(buf + 7, bounds.Y, Endian::Little);
    if (writeTable)
    {
        // If there's a local table, say so and tell its length
        buf[9] = 0b10000000 | uint8_t(std::ceil(std::log2(computeColorTableSize(palette))) - 1);
    }
    os.write((char*)buf, 10);

    if (writeTable)
    {
        writeColorTable(os, palette);
    }

    // Assume minimum code size of 8, for simplicity's sake
    os.put(8);
    
    // Indices vector for LZW compression
    std::vector<uint8_t> indices;
    indices.reserve(bounds.Area());

    // Loop through image and fill indices vector
    LOG_DEBUG("Scaling frame from " << frame.Bounds() << " to " << bounds);
    Vec scale = Vec(frame.Bounds()) / Vec(bounds);
    for (int y = 0; y < bounds.Y; y++)
    {
        for (int x = 0; x < bounds.X; x++)
        {
            // Sample a color from the image and compute the index in the
            // color table which best represents that color
            RGBA color = frame.Sample(Vec(x, y) * scale);
            indices.push_back(ComputeNearestMean<RGBA, RGBADist>(palette, color)); 
        }
    }

    // Undergo LZW compression for this frame
    compressLZW(os, indices);
}

void GIF::compressLZW(std::ostream& os, const std::vector<uint8_t>& indices)
{
    LOG_DEBUG("Running LZW compression on " << indices.size() << " indices");
    
    // Set clear code and EOI code. Assume 8 bits required per index (9 with extra codes).
    const uint16_t clearCode = 256;
    const uint16_t EOICode = 257;
    const uint8_t baseCodeSize = 9;
    uint8_t codeSize = baseCodeSize;
    size_t codesNextSize = std::pow(2, codeSize);

    // Create an LZW tree for keeping track of used codes.
    // Even though it should NEVER receive a clearCode or an EOICode,
    // initialize the tree with EOICode nodes so that the first
    // unique code it produces is EOICode + 1.
    LZWTree tree(EOICode + 1);

    // Create BitBuffer to hold variable-length LZW codes
    BitBuffer bbuf;

    // Write initial clear code
    bbuf << BitField(clearCode, codeSize);

    // Loop through all indices
    size_t lastEncodedIdx = 0;
    for (size_t i = 0; i < indices.size(); i++)
    {
        size_t code = tree.Add(indices[i]);
        if (code != tree.npos)
        {
            // the index buffer is now equal to indices[i], meaning
            // indices[i] has not been encoded yet
            lastEncodedIdx = i - 1;

            // LOG_DEBUG("Writing code " << code << " of size " << (int)codeSize);
            // LOG_DEBUG(" -> tree size: " << tree.Size());
            bbuf << BitField(code, codeSize);
            // LOG_DEBUG("Code written; bbuf size " << bbuf.Size() << "; capacity " << bbuf.Capacity());

            if (tree.Size() > codesNextSize)
            {
                codeSize++;
                codesNextSize = std::pow(2, codeSize);
            }

            if (codeSize > 12)
            {
                // TODO: should this be earlier??
                bbuf << BitField(clearCode, 12);
                tree.Reset(EOICode + 1);
                codeSize = baseCodeSize;
                codesNextSize = std::pow(2, codeSize);
            }
        }
    }

    for (size_t i = lastEncodedIdx; i < indices.size(); i++)
    {
        bbuf << BitField(indices[i], codeSize);
    }

    // Write final EOI code
    bbuf << BitField(EOICode, codeSize);

    // Write bitbuffer to output stream
    writeSubBlocks(os, bbuf.Data(), bbuf.SizeBytes());
}

void GIF::writeSubBlocks(std::ostream& os, const void* data, size_t bytes)
{
    const char* ptr = static_cast<const char*>(data);
    const char* const maxPtr = static_cast<const char*>(data) + bytes;
    while (ptr < maxPtr)
    {
        size_t toWrite = std::min(maxPtr - ptr, 255L);
        os.put(toWrite);
        os.write(ptr, toWrite);
        ptr += toWrite;
    }
    os.put(0);
}

//                      //
// Helpful LOAD Methods //
//                      //

std::pair<Image, uint16_t> GIF::loadNextFrame(std::istream& is, VecInt canvasBounds, const std::vector<RGBA>& globalColorTable)
{
    BlockType blockType = (BlockType)is.get();

    // Make sure stream is still valid. If not, assume EOF has been reached
    if (!is)
    {
        throw gif::BadStreamException("Failed to read first byte of block");
    }

    // Load blocks until an image block is reached. If a graphics control
    // extension is encountered first, then load it in and continue
    Image image;
    GraphicsExtension graphics;
    while (blockType != BlockType::IMAGE)
    {
        switch (blockType)
        {
        
        case BlockType::EXTENSION:
        {
            LOG_DEBUG("EXTENSION BLOCK " << is.tellg());
            ExtensionType extType = (ExtensionType)is.peek();

            // If graphics extension encountered, load it
            if (extType == ExtensionType::GRAPHICS)
                graphics = GraphicsExtension(is);
            // Otherwise, ignore this block
            else
                readExtensionBlock(is);
            
            break;
        } // case BlockType::EXTENSION

        case BlockType::END_OF_FILE:
            throw gif::EndOfStreamException("Encountered EOF byte");

        default:
        {
            std::ostringstream errstr;
            errstr << "Unrecognized first byte of block: 0x" << std::hex
                << std::setw(2) << std::setfill('0') << (int)blockType;
            throw gif::FormatException(errstr.str());
        } // default

        } // switch

        // Read first byte of new block
        is.read((char*)(&blockType), 1);

    } // while

    LOG_DEBUG("IMAGE BLOCK " << is.tellg());
    // Get the image descriptor
    ImageDescriptor desc(is);
    
    // Read color codes and create image
    const std::vector<RGBA>& colorTable = (desc.flags.localTableFlag) ? 
                                          loadColorTable(is, loadColorTableSize(desc.flags.colorTableSize)) :
                                          globalColorTable;

    // Load raw image data
    auto rawData = loadImageData(is);
    // Parse raw data for LZW codes
    auto lzwCodes = decompressLZW(rawData);
    
    // Ensure color table to be used is ok
    if (colorTable.empty())
    {
        throw gif::FormatException("Missing global and local color table!");
    }

    // Load in frame
    VecInt frameBounds(desc.width, desc.height);
    VecInt offset(desc.offsetWidth, desc.offsetHeight);
    if (frames.empty())
    {
        // Decode frame normally if it is the first one
        image = decodeImage(canvasBounds, frameBounds, offset, lzwCodes, colorTable);
    }
    else if (graphics.transparentFlag)
    {
        // If transparency flag is set and there exists a previous frame,
        // then use that previous frame for parts of this frame
        image = decodeImage(canvasBounds, frameBounds, offset, lzwCodes, colorTable, frames.back(), graphics.transparentIdx);
    }
    else
    {
        // If no transparency, still pass in previous frame in case the
        // frame bounds are less than the total bounds or there is an
        // offset for this frame
        image = decodeImage(canvasBounds, frameBounds, offset, lzwCodes, colorTable, &frames.back());
    }

    return std::pair<Image, uint16_t>(image, graphics.duration);
}

void GIF::readExtensionBlock(std::istream& is)
{
    ExtensionType type;
    is.read((char*)&type, 1);

    if (!is)
    {
        throw gif::FormatException("Could not read extension type after extension introducer!");
    }

    switch (type)
    {
    // Plaintext and application extension blocks each have a brief section of
    // beginning data that can be skipped over. The rest of their data consists
    // of sub-blocks, which can be read just like raw image data.
    case ExtensionType::PLAINTEXT:
    case ExtensionType::APPLICATION:
        uint8_t canSkip;
        is.read((char*)&canSkip, 1);
        is.seekg(canSkip, is.cur);
        [[fallthrough]];
    // TODO: Actually parse a graphics control block!
    // For now, though, it's safe to treat it like sub-block data.
    case ExtensionType::GRAPHICS:

    // Comment blocks consist solely of sub-blocks
    case ExtensionType::COMMENT:
        loadSubBlocks(is);
        break;
    
    default: {
        std::ostringstream os;
        os << "Unknown extension type following extension introducer: 0x"
           << std::hex << std::setfill('0') << std::setw(2) << (int)type;
        throw gif::FormatException(os.str());
    } // default

    } // switch
}

LZWFrame GIF::loadImageData(std::istream& is)
{
    LZWFrame frame;

    // Read LZW Minimum Code Size
    is.read((char*)&frame.minCodeSize, 1);
    LOG_DEBUG(" -> LZW minimum code size: " << (int)frame.minCodeSize);
    if (!is)
    {
        throw gif::BadStreamException("No image data to load following image separator!");
    }

    if (frame.minCodeSize > 8)
    {
        throw gif::FormatException("LZW minimum code size is too big: " +
                                   std::to_string(frame.minCodeSize));
    }
    else if (frame.minCodeSize < 2)
    {
        throw gif::FormatException("LZW minimum code size is too small: " +
                                   std::to_string(frame.minCodeSize));
    }

    // Read sub-block data from input stream
    frame.data = loadSubBlocks(is);

    return frame;
}

std::vector<uint8_t> GIF::loadSubBlocks(std::istream& is)
{
    // Determine the size of each sub-block in the sequence
    uint8_t nextBlockSize = 0;
    long totalSeeked = 1; 
    std::vector<short> subBlockSizes;
    while (true)
    {
        // Get size of the next block
        is.read((char*)&nextBlockSize, 1);

        if (nextBlockSize)
        {
            LOG_DEBUG(" -> Encountered sub-block of size " << (int)nextBlockSize << " at " << is.tellg());
            is.seekg(nextBlockSize, is.cur);
        }
        else
        {
            LOG_DEBUG(" -> Final sub-block encountered; next block begins at " << is.tellg());
            break;
        }

        if (is)
        {
            totalSeeked += long(nextBlockSize) + 1;
            subBlockSizes.push_back(nextBlockSize);
        }
        else
        {
            throw gif::BadStreamException("Couldn't read sub-block");
        }
    }

    // With the length of all sub-blocks determined, reset the stream back
    // to the beginning of the image data
    is.seekg(-totalSeeked, is.cur);
    LOG_DEBUG(" -> Stream returned to position " << is.tellg());

    // Allocate data vector to hold subBlock data
    std::vector<uint8_t> data(totalSeeked - subBlockSizes.size());

    // Read sub-blocks
    char* ptr = (char*)data.data();
    for (auto thisBlockSize : subBlockSizes)
    {
        // Sanity check: make sure sub-block sizes match recorded sizes
        uint8_t checkThisBlockSize;
        is.read((char*)&checkThisBlockSize, 1);

        if (checkThisBlockSize == thisBlockSize)
        {
            is.read(ptr, thisBlockSize);
            ptr += thisBlockSize;
        }
        else
        {
            std::ostringstream os;
            os << std::hex << std::setfill('0')
               << "Sub-block size mismatch: Expected 0x" << std::setw(2)
               << (int)thisBlockSize << " but got 0x" << std::setw(2)
               << (int)checkThisBlockSize << std::dec << " (is.tellg() == "
               << is.tellg() << ")";
            throw gif::FormatException(os.str());
        }
    }

    // Make sure final byte is null as a sanity check
    uint8_t shouldBeNull = 1;
    is.read((char*)&shouldBeNull, 1);
    if (shouldBeNull)
    {
        std::ostringstream os;
        os << "Putative final byte of sub-block data was not null: 0x"
           << std::hex << std::setw(2) << std::setfill('0')
           << (int)shouldBeNull;
        throw gif::FormatException(os.str());
    }

    LOG_DEBUG(" -> Done reading sub-blocks; stream is now at " << is.tellg());

    return data;
}

std::vector<uint8_t> GIF::decompressLZW(const LZWFrame& data)
{
    // LZW codes with special meanings
    const uint16_t clearCode = std::pow(2, data.minCodeSize);
    const uint16_t EOICode = clearCode + 1;
 
    // Stream of all codes retrieved so far
    std::vector<uint8_t> indexStream;

    // Create the base color table
    const std::vector<std::vector<uint8_t>> codeTableBase = getBaseCodeTable(data.minCodeSize);

    // Create the current code table
    std::vector<std::vector<uint8_t>> codeTableCurrent = codeTableBase;
    codeTableCurrent.reserve(MAX_CODE_TABLE_SIZE);

    // Current size in bits of each LZW code
    const uint8_t codeSizeBase = data.minCodeSize + 1;
    uint8_t codeSize = codeSizeBase;

    // Buffer to read codes from
    const BitBuffer bbuf(data.data.data(), data.data.size());
    // Next bit to read from in the BitBuffer
    size_t bitIdx = 0;
    // Maximum code allowed by codeSize
    size_t maxCodeThisSize = std::pow(2, codeSize) - 1;

    // Read the first code of the code stream; this should be clearCode
    uint16_t prevCode = bbuf.Read(bitIdx, codeSize);
    bitIdx += codeSize;
    if (prevCode != clearCode)
    {
        throw gif::FormatException("Expected clearCode to be first code of LZW stream!");
    }

    while (true)
    {
        // If a clear code has just been received, load the very
        // first actual color code of the new code stream
        if (prevCode == clearCode)
        {
            LOG_DEBUG("Beginning new stream after clear code!");
            prevCode = bbuf.Read(bitIdx, codeSize);
            bitIdx += codeSize;
            indexStream.push_back(codeTableCurrent.at(prevCode).at(0));
            
            // Special cases that *shouldn't* happen, but technically
            // can, because you can do whatever the heck you want if
            // you're writing a GIF encoder
            if (prevCode == clearCode)
                continue;
            else if (prevCode == EOICode)
                break;
        }

        // Read the next code
        uint16_t thisCode = bbuf.Read(bitIdx, codeSize);
        bitIdx += codeSize;

        if (thisCode == clearCode)
        {
            // If this code is a clear code, reset the code table, code size,
            // and the maximum allowable code value for this code size
            LOG_DEBUG(" -> CC");

            // Reset the code table and code size
            codeTableCurrent = codeTableBase;
            codeSize = codeSizeBase;
            maxCodeThisSize = std::pow(2, codeSize) - 1;

            // Set prevCode to CC
            prevCode = thisCode;
            continue;
        }
        else if (thisCode == EOICode)
        {
            // If this code is an EOI code, we're all done!
            LOG_DEBUG(" -> EOI");
            break;
        }

        // If the current size of the code table is maxed out, then
        // just add the indices of this code to the index stream
        if (codeTableCurrent.size() == MAX_CODE_TABLE_SIZE)
        {
            for (auto idx : codeTableCurrent.at(thisCode))
                indexStream.push_back(idx);
            continue;
        }

        // If this code is in the color table
        if (thisCode < codeTableCurrent.size())
        {
            // output {CODE} to index stream
            for (auto idx : codeTableCurrent.at(thisCode))
                indexStream.push_back(idx);

            // let K be the first index in {CODE}
            uint8_t K = codeTableCurrent.at(thisCode).at(0);

            // add {CODE-1}+K to the code table
            std::vector<uint8_t> newCode = codeTableCurrent.at(prevCode);
            newCode.push_back(K);
            codeTableCurrent.push_back(newCode);
        }
        else
        {
            // let K be the first index of {CODE-1}
            const auto& prevCodeVal = codeTableCurrent.at(prevCode);
            uint8_t K = prevCodeVal.at(0);

            // add {CODE-1}+K to code table
            std::vector<uint8_t> newCode = prevCodeVal;
            newCode.push_back(K);
            codeTableCurrent.push_back(newCode);

            // output {CODE-1}+K to index stream
            for (auto idx : newCode)
                indexStream.push_back(idx);
        }

        // So we know what CODE-1 is
        prevCode = thisCode;

        // Check if code size needs to be updated
        if (codeTableCurrent.size() > maxCodeThisSize)
        {
            maxCodeThisSize = std::pow(2, ++codeSize) - 1;
        }

        // codeSize must not exceed 12 bits
        if (codeSize > 12)
        {
            throw gif::FormatException("Exceeded maximum code size!");
        }
    }

    return indexStream;
}

Image GIF::decodeImage(VecInt canvasBounds,
                       VecInt frameBounds,
                       VecInt offset,
                       const std::vector<uint8_t>& indices,
                       const std::vector<RGBA>& colorTable,
                       const Image* prevFrame)
{
    // Create frame and copy relevant parts from previous frame
    Image image(canvasBounds);
    if (prevFrame)
    {
        image.CopyNotInRange(*prevFrame, offset, offset + frameBounds);
    }

    size_t idx = 0;
    for (int y = 0; y < frameBounds.Y; y++)
    {
        for (int x = 0; x < frameBounds.X; x++)
        {
            if (idx >= indices.size())
                break;

            // Get this color index
            int colorIdx = indices[idx];

            // Set the current image pixel to the color at colorIdx
            try
            {
                image.At(VecInt(x, y) + offset) = colorTable.at(colorIdx);
            }
            catch(const std::runtime_error& e)
            {
                throw gif::FormatException("Index " + std::to_string(colorIdx) +
                                           " exceeds the color table of size:" +
                                           std::to_string(colorTable.size()));
            }

            idx++;
        }
    }

    return image;
}

Image GIF::decodeImage(VecInt canvasBounds,
                       VecInt frameBounds,
                       VecInt offset,
                       const std::vector<uint8_t>& indices,
                       const std::vector<RGBA>& colorTable,
                       const Image& prevFrame,
                       int transparentIdx)
{
    // Create frame and copy relevant parts from previous frame
    Image image(canvasBounds);
    image.CopyNotInRange(prevFrame, offset, offset + frameBounds);

    size_t idx = 0;
    for (int y = 0; y < frameBounds.Y; y++)
    {
        for (int x = 0; x < frameBounds.X; x++)
        {
            if (idx >= indices.size())
                break;
            
            // Get this color index
            int colorIdx = indices[idx];

            if (colorIdx == transparentIdx)
            {
                // If this is the transparent index, use previous frame's color
                image.At(VecInt(x, y) + offset) = prevFrame.At(VecInt(x, y) + offset);
            }
            else
            {
                // Otherwise, set the current image pixel to the color at colorIdx
                try
                {
                    image.At(VecInt(x, y) + offset) = colorTable.at(colorIdx);
                }
                catch(const std::runtime_error& e)
                {
                    throw gif::FormatException("Index " + std::to_string(colorIdx) +
                                               " exceeds the color table of size:" +
                                               std::to_string(colorTable.size()));
                }
            }

            idx++;
        }
    }

    return image;
}

std::vector<RGBA> GIF::loadColorTable(std::istream& is, unsigned numColors)
{
    LOG_DEBUG("Loading color table of length " << numColors);
    const unsigned bytesToRead = numColors * 3;
    uint8_t* buf = new uint8_t[bytesToRead], *ptr = buf;
    is.read((char*)buf, bytesToRead);
    
    std::vector<RGBA> table(numColors);
    for (RGBA& color : table)
    {
        color = RGBA(
            *ptr,
            *(ptr + 1),
            *(ptr + 2)
        );

        ptr += 3;
    }

    delete[] buf;

    LOG_DEBUG("Finished loading color table of length " << numColors);

    return table;
}

int GIF::loadColorTableSize(int bitField)
{
    return std::pow(2, 1 + bitField);
}

std::vector<std::vector<uint8_t>> GIF::getBaseCodeTable(int minCodeSize)
{
    const uint16_t numColorCodes = std::min(256, (int)std::pow(2, minCodeSize));

    std::vector<std::vector<uint8_t>> table;
    table.reserve(4096); // Reserve 2^12 codes
    for (uint16_t c = 0; c < numColorCodes; c++)
        table.push_back({(uint8_t)c});
    table.push_back({}); // clear code
    table.push_back({}); // EOI code

    return table;
}

} // namespace zmath
