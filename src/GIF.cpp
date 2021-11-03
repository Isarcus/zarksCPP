#include <zarks/image/GIF.h>
#include <zarks/io/binary.h>
#include <zarks/io/BitBuffer.h>
#include <zarks/io/logdefs.h>
#include <zarks/math/KMeans.h>

#include <fstream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>

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

//            //
// SaveConfig //
//            //

GIF::SaveConfig::SaveConfig()
    : paletteSize(0)
    , globalPalette(true)
{}

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
    __attribute__((unused)) uint16_t canvasWidth = FromBytes<uint16_t>(AdvancePtr(ptr, 2), Endian::Little);
    __attribute__((unused)) uint16_t canvasHeight = FromBytes<uint16_t>(AdvancePtr(ptr, 2), Endian::Little);
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
        globalColorTable = loadColorTable(is, getColorTableSize(flags.colorTableSize));
    }

    // Process blocks for as long as needed
    while (true)
    {
        try
        {
            auto pair = loadNextFrame(is, globalColorTable);
            frames.push_back(pair.first);
            LOG_DEBUG("Loaded frame #" << frames.size());
        }
        catch (const gif::EndOfStreamException& e)
        {
            break;
        }
        catch (const gif::GifLoadingException& e)
        {
            std::cerr << e << std::endl;
            std::cerr << " -> Stream position: " << is.tellg() << std::endl;
            break;
        }
    }
}

void GIF::Save(std::string path, const SaveConfig& cfg) const
{
    // Open and check stream
    std::ofstream os(path, std::ios_base::binary);
    if (!os)
    {
        throw std::runtime_error("Could not open file at " + path);
    }

    // Call generic save method
    Save(os, cfg);
}

void GIF::Save(std::ostream& os, const SaveConfig& cfg) const
{
    // Check stream
    if (!os)
    {
        throw std::runtime_error("Invalid output stream passed to GIF::Save()");
    }

    // Make sure there are frames to write
    if (frames.empty())
    {
        throw std::runtime_error("Tried to save an empty GIF");
    }

    // Determine if there is a global color table and size of color table(s)
    bool global = (cfg.globalPalette) ? true : !cfg.palette.empty();
    unsigned numColors = (cfg.palette.empty());
    if (cfg.palette.empty())
    {
        numColors = cfg.paletteSize ? std::min(255U, cfg.paletteSize) : 255U;
    }
    else
    {
        numColors = std::min(cfg.palette.size(), 255UL);
    }

    // Determine bounds
    VecInt bounds = (cfg.bounds != VecInt()) ? cfg.bounds : Bounds();

    std::cout << global << numColors << bounds << "\n";
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

void GIF::writeGraphicsExtension(std::ostream& os, double duration)
{
    // Determine the duration of this frame
    duration = std::max(0.0, std::min(duration, 655.35));
    uint16_t intDur = (uint16_t)(std::round(duration * 100));

    // Create temporary buffer
    char buf[8]{};
    buf[0] = (char)BlockType::EXTENSION;
    buf[1] = (char)ExtensionType::GRAPHICS;
    buf[2] = 4; // 4 bytes follow this one, plus a null terminator 
    ToBytes(buf + 4, intDur, Endian::Little);

    // Write temporary buffer to output stream
    os.write(buf, 8);
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
    unsigned unusedColors = std::pow(2, std::ceil(std::log2(usedColors))) - usedColors;

    // Write all used colors to the stream
    for (unsigned i = 0; i < usedColors; i++)
    {
        RGBA col = palette[i];
        uint8_t rgb[3] { col.R, col.G, col.B };
        os.write((char*)rgb, 3);
    }

    // Write all unused colors
    char blanks[254 * 3]{}; // holds maximum possible length of unused color bytes
    os.write(blanks, unusedColors * 3);
}

void GIF::writeFrame(std::ostream& os, const Image& frame, VecInt bounds, const std::vector<RGBA>& palette)
{
    // Indices vector for LZW compression
    std::vector<uint8_t> indices;
    indices.reserve(bounds.Area());

    // Loop through image and fill indices vector
    Vec scale = Vec(bounds) / Vec(frame.Bounds());
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
    compressLZW(os, indices, std::ceil(std::log2(palette.size())));
}

void GIF::compressLZW(std::ostream& os, const std::vector<uint8_t>& indices, uint8_t minBits)
{

}

//                      //
// Helpful LOAD Methods //
//                      //

std::pair<Image, uint16_t> GIF::loadNextFrame(std::istream& is, const std::vector<RGBA>& globalColorTable)
{
    BlockType blockType;
    is.read((char*)(&blockType), 1);

    // Make sure stream is still valid. If not, assume EOF has been reached
    if (!is)
    {
        throw gif::BadStreamException("Failed to read first byte of block");
    }

    // Declarations
    Image image;
    uint16_t duration = 0;

    // Based on the type of this block, attempt to either load in a new
    // frame or interpret an extension block
    switch (blockType)
    {
    
    case BlockType::EXTENSION:
    {
        LOG_DEBUG("EXTENSION BLOCK " << is.tellg());
        readExtensionBlock(is);
        return loadNextFrame(is, globalColorTable);
    } // case BlockType::EXTENSION

    case BlockType::IMAGE:
    {
        LOG_DEBUG("IMAGE BLOCK " << is.tellg());
        // Get the image descriptor
        ImageDescriptor desc(is);

        // Load raw image data
        auto rawData = loadImageData(is);
        // Parse raw data for LZW codes
        auto lzwCodes = decompressLZW(rawData);

        // Read color codes and create image
        if (desc.flags.localTableFlag)
        {
            std::vector<RGBA> localColorTable = loadColorTable(is, getColorTableSize(desc.flags.colorTableSize));
            image = decodeImage(VecInt(desc.width, desc.height), lzwCodes, localColorTable);
        }
        else if (globalColorTable.size())
        {
            image = decodeImage(VecInt(desc.width, desc.height), lzwCodes, globalColorTable);
        }
        else
        {
            throw gif::FormatException("Missing global and local color table!");
        }
        break;
    } // case BlockType::IMAGE

    case BlockType::END_OF_FILE:
        throw gif::EndOfStreamException("Encountered end of file byte");

    default:
    {
        std::ostringstream errstr;
        errstr << "Unrecognized first byte of block: 0x" << std::hex
               << std::setw(2) << std::setfill('0') << (int)blockType;
        throw gif::FormatException(errstr.str());
    } // default

    } // switch

    return std::pair<Image, uint16_t>(image, duration);
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
            LOG_DEBUG(" -> LZW code #" << prevCode << " (code size == "
                      << (int)codeSize << "; table size == " << codeTableCurrent.size()
                      << ")");
            
            // Special cases that *shouldn't* happen, but technically
            // can, because you can do whatever the heck you want if
            // you're writing a GIF encoder
            if (prevCode == clearCode) continue;
            if (prevCode == EOICode) break;
        }

        // Read the next code
        uint16_t thisCode = bbuf.Read(bitIdx, codeSize);
        bitIdx += codeSize;

        // Notify of this code
        LOG_DEBUG(" -> LZW code #" << thisCode << " (code size == "
                  << (int)codeSize << "; table size == " << codeTableCurrent.size()
                  << ")");

        // If this code is a clear code, reset the code table, code size,
        // and the maximum allowable code value for this code size
        if (thisCode == clearCode)
        {
            LOG_DEBUG(" -> CC");
            // Reset the code table and code size
            codeTableCurrent = codeTableBase;
            codeSize = codeSizeBase;
            maxCodeThisSize = std::pow(2, codeSize) - 1;
            // Set prevCode to CC
            prevCode = thisCode;
            continue;
        }
        // If this code is an EOI code, we're all done!
        else if (thisCode == EOICode)
        {
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
            const auto& prevCodeVal = codeTableCurrent.at(prevCode);
            // let K be the first index of {CODE-1}
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

Image GIF::decodeImage(VecInt bounds, const std::vector<uint8_t>& indices, const std::vector<RGBA>& colorTable)
{
    Image image(bounds);

    size_t idx = 0;
    for (int y = 0; y < bounds.Y; y++)
    {
        for (int x = 0; x < bounds.X; x++)
        {
            if (idx >= indices.size()) break;

            try
            {
                image[x][y] = colorTable.at(indices[idx]);
            }
            catch(const std::exception& e)
            {
                throw gif::FormatException("Index " + std::to_string(indices[idx]) +
                                           " exceeds the color table of size:" +
                                           std::to_string(colorTable.size()));
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

int GIF::getColorTableSize(int bitField)
{
    return std::pow(2, 1 + bitField);
}

std::vector<std::vector<uint8_t>> GIF::getBaseCodeTable(int minCodeSize)
{
    const uint16_t numColorCodes = std::min(256, (int)std::pow(2, minCodeSize));

    std::vector<std::vector<uint8_t>> table;
    table.reserve(numColorCodes + 2);
    for (uint16_t c = 0; c < numColorCodes; c++)
        table.push_back({(uint8_t)c});
    table.push_back({}); // clear code
    table.push_back({}); // EOI code

    return table;
}

} // namespace zmath
