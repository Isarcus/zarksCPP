#include <zarks/image/GIF.h>
#include <zarks/io/binary.h>
#include <zarks/io/BitBuffer.h>
#include <zarks/io/logdefs.h>

#include <fstream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>

namespace zmath
{

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
        try {
            auto pair = loadNextFrame(is, globalColorTable);
            frames.push_back(pair.first);
            LOG_DEBUG("Loaded frame #" << frames.size());
        } catch (const gif::ColorTableException& e) {
            std::cerr << e << std::endl;
            std::cerr << " -> Stream position: " << is.tellg() << std::endl;
            break;
        } catch (const gif::BadBlockException& e) {
            std::cerr << e << std::endl;
            std::cerr << " -> Stream position: " << is.tellg() << std::endl;
            break;
        } catch (const gif::EndOfStreamException& e) {
            break;
        }
    }
}

void GIF::Save(std::string path, uint8_t paletteSize, VecInt bounds) const
{

}

void GIF::Save(std::string path, const std::vector<RGBA>& palette, VecInt bounds) const
{

}

void GIF::Add(const Image& img, bool adjustBounds, int idx)
{
    if (idx == -1) {
        frames.push_back(img);
    } else {
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

//                //
// Struct Methods //
//                //

GIF::LSDFlags::LSDFlags()
    : colorTableSize(0)
    , sortFlag(0)
    , colorResolution(0)
    , globalTableFlag(0)
{}

GIF::LSDFlags::LSDFlags(uint8_t flagByte)
    : colorTableSize  ( flagByte & COLOR_TABLE_SIZE)
    , sortFlag        ((flagByte & SORT_FLAG)               >> 3)
    , colorResolution ((flagByte & COLOR_RESOLUTION)        >> 4)
    , globalTableFlag ((flagByte & GLOBAL_COLOR_TABLE_FLAG) >> 7)
{}

GIF::IDFlags::IDFlags()
    : colorTableSize(0)
    , __reservedBits(0)
    , sortFlag(0)
    , interlaceFlag(0)
    , localTableFlag(0)
{}

GIF::IDFlags::IDFlags(uint8_t flagByte)
    : colorTableSize ( flagByte & COLOR_TABLE_SIZE)
    , __reservedBits ((flagByte & RESERVED_BITS)          >> 3)
    , sortFlag       ((flagByte & SORT_FLAG)              >> 5)
    , interlaceFlag  ((flagByte & INTERLACE_FLAG)         >> 6)
    , localTableFlag ((flagByte & LOCAL_COLOR_TABLE_FLAG) >> 7)
{}

GIF::ImageDescriptor::ImageDescriptor(std::istream& is)
{
    uint8_t buf[9];
    const uint8_t *ptr = buf;

    is.read((char*)buf, 9);

    offsetWidth = ReadBuf<uint16_t>(ptr, Endian::Little);
    offsetHeight = ReadBuf<uint16_t>(ptr, Endian::Little);
    width = ReadBuf<uint16_t>(ptr, Endian::Little);
    height = ReadBuf<uint16_t>(ptr, Endian::Little);
    flags = IDFlags(*ptr);
}

std::ostream& operator<<(std::ostream& os, const GIF::ImageDescriptor& desc)
{
    return os
        << "Image descriptor data:\n"
        << " -> offsetWidth:  " << desc.offsetWidth << "\n"
        << " -> offsetHeight: " << desc.offsetHeight << "\n"
        << " -> width:  " << desc.width << "\n"
        << " -> height: " << desc.height << "\n"
        << " -> flags:\n"
        << "   -> colorTableSize: " << desc.flags.colorTableSize << "\n"
        << "   -> __reservedBits: " << desc.flags.__reservedBits << "\n"
        << "   -> sortFlag:       " << desc.flags.sortFlag << "\n"
        << "   -> interlaceFlag:  " << desc.flags.interlaceFlag << "\n"
        << "   -> localTableFlag: " << desc.flags.localTableFlag << "\n";
}

//                 //
// Private Methods //
//                 //

std::pair<Image, uint16_t> GIF::loadNextFrame(std::istream& is, const std::vector<RGBA>& globalColorTable)
{
    BlockType blockType;
    is.read((char*)(&blockType), 1);

    // Make sure stream is still valid. If not, assume EOF has been reached
    if (!is)
    {
        throw gif::BadBlockException("Failed to read first byte of block");
    }

    // Declarations
    Image image;
    uint16_t duration = 0;

    // Based on the type of this block, attempt to either load in a new
    // frame or interpret an extension block
    switch (blockType)
    {
    case BlockType::EXTENSION: {
        LOG_DEBUG("EXTENSION BLOCK " << is.tellg());
        readExtensionBlock(is);
        return loadNextFrame(is, globalColorTable);
    } // case EXTENSION_INTRODUCER

    case BlockType::IMAGE: {
        LOG_DEBUG("IMAGE BLOCK " << is.tellg());
        // Get the image descriptor
        ImageDescriptor desc(is);

        // Load raw image data
        auto rawData = loadImageData(is);
        // Parse raw data for LZW codes
        auto lzwCodes = decompressLZW(rawData);

        // Read color codes and create image
        if (desc.flags.localTableFlag) {
            std::vector<RGBA> localColorTable = loadColorTable(is, getColorTableSize(desc.flags.colorTableSize));
            image = decodeImage(VecInt(desc.width, desc.height), lzwCodes, localColorTable);
        } else if (globalColorTable.size()) {
            image = decodeImage(VecInt(desc.width, desc.height), lzwCodes, globalColorTable);
        } else {
            throw gif::ColorTableException("Missing global and local color table!");
        }
        break;
    } // case IMAGE_SEPARATOR

    case BlockType::END_OF_FILE:
        throw gif::EndOfStreamException("End of file block reached!");

    default: {
        std::ostringstream errstr;
        errstr << "Unrecognized first byte of block: 0x" << std::hex
               << std::setw(2) << std::setfill('0') << (int)blockType;
        throw gif::BadBlockException(errstr.str());
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
        throw gif::BadBlockException("Could not read extension type after extension introducer!");
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
        throw gif::BadBlockException(os.str());
    } // default

    } // switch
}

GIF::LZWFrame GIF::loadImageData(std::istream& is)
{
    LZWFrame frame;

    // Read LZW Minimum Code Size
    is.read((char*)&frame.minCodeSize, 1);
    LOG_DEBUG(" -> LZW minimum code size: " << (int)frame.minCodeSize);
    if (!is)
    {
        throw gif::BadBlockException("No image data to load following image separator!");
    }

    if (frame.minCodeSize > 8)
    {
        throw gif::BadBlockException("LZW minimum code size is too big: " +
                                     std::to_string(frame.minCodeSize));
    } else if (frame.minCodeSize < 2) {
        throw gif::BadBlockException("LZW minimum code size is too small: " +
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

        if (nextBlockSize) {
            LOG_DEBUG(" -> Encountered sub-block of size " << (int)nextBlockSize << " at " << is.tellg());
            is.seekg(nextBlockSize, is.cur);
        } else {
            LOG_DEBUG(" -> Final sub-block encountered; next block begins at " << is.tellg());
            break;
        }

        if (is) {
            totalSeeked += long(nextBlockSize) + 1;
            subBlockSizes.push_back(nextBlockSize);
        } else {
            throw gif::BadBlockException("Couldn't read sub-block");
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

        if (checkThisBlockSize == thisBlockSize) {
            is.read(ptr, thisBlockSize);
            ptr += thisBlockSize;
        } else {
            std::ostringstream os;
            os << std::hex << std::setfill('0')
               << "Sub-block size mismatch: Expected 0x" << std::setw(2)
               << (int)thisBlockSize << " but got 0x" << std::setw(2)
               << (int)checkThisBlockSize << std::dec << " (is.tellg() == "
               << is.tellg() << ")";
            throw gif::BadBlockException(os.str());
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
        throw gif::BadBlockException(os.str());
    }

    LOG_DEBUG(" -> Done reading sub-blocks; stream is now at " << is.tellg());

    return data;
}

std::vector<uint8_t> GIF::decompressLZW(const LZWFrame& data)
{
    // LZW codes with special meanings
    const uint16_t clearCode = std::pow(2, data.minCodeSize);
    const uint16_t EOICode = clearCode + 1;
    const uint16_t numSingleCodes = std::min(256, (int)std::pow(2, data.minCodeSize));
 
    // Stream of all codes retrieved so far
    std::vector<uint8_t> indexStream;

    // LZW code table
    std::vector<std::vector<uint8_t>> codeTableBase;
    codeTableBase.reserve(MAX_CODE_TABLE_SIZE);
    for (uint16_t c = 0; c < numSingleCodes; c++)
        codeTableBase.push_back({(uint8_t)c});
    codeTableBase.push_back({}); // clear code
    codeTableBase.push_back({}); // EOI code
    std::vector<std::vector<uint8_t>> codeTableCurrent = codeTableBase;

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
        throw gif::BadBlockException("Expected clearCode to be first code of LZW stream!");
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
        }

        // Read the next code
        uint16_t thisCode = bbuf.Read(bitIdx, codeSize);
        bitIdx += codeSize;

        // Notify of this code
        LOG_DEBUG(" -> LZW code #" << thisCode << " (code size == "
                  << (int)codeSize << "; table size == " << codeTableCurrent.size()
                  << ")");

        // Decide what to do with this code
        if (thisCode == clearCode) {
            LOG_DEBUG(" -> CC");
            // Reset the code table and code size
            codeTableCurrent = codeTableBase;
            codeSize = codeSizeBase;
            // Set prevCode to CC
            prevCode = thisCode;
            continue;
        } else if (thisCode == EOICode) {
            LOG_DEBUG(" -> EOI");
            break;
        } else if (thisCode < codeTableCurrent.size()) {
            // output {CODE} to index stream
            for (auto idx : codeTableCurrent.at(thisCode)) indexStream.push_back(idx);

            // let K be the first index in {CODE}
            uint8_t K = codeTableCurrent.at(thisCode).at(0);

            // add {CODE-1}+K to the code table
            std::vector<uint8_t> newCode = codeTableCurrent.at(prevCode);
            newCode.push_back(K);
            codeTableCurrent.push_back(newCode);

        } else {
            const auto& prevCodeVal = codeTableCurrent.at(prevCode);
            // let K be the first index of {CODE-1}
            uint8_t K = prevCodeVal.at(0);

            // add {CODE-1}+K to code table
            std::vector<uint8_t> newCode = prevCodeVal;
            newCode.push_back(K);
            codeTableCurrent.push_back(newCode);

            // output {CODE-1}+K to index stream
            for (auto idx : newCode) indexStream.push_back(idx);

        }

        // Print out the last code in the code table
        LOG_DEBUG("Last code (#" << codeTableCurrent.size() - 1
                  << " in the code table: ");
        for (auto c : codeTableCurrent.back()) std::cout << (int)c << " ";
        std::cout << std::endl;

        // So we know what CODE-1 is
        prevCode = thisCode;

        // Check if code size needs to be updated
        if (codeTableCurrent.size() > maxCodeThisSize)
        {
            maxCodeThisSize = std::pow(2, ++codeSize) - 1;
        }

        // codeSize should not exceed 12 bits
        if (codeSize > 12)
        {
            throw std::runtime_error("Exceeded maximum code size!");
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

            try {
                image[x][y] = colorTable.at(indices[idx]);
            } catch(const std::exception& e) {
                throw gif::ColorTableException("Index " + std::to_string(indices[idx]) +
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

namespace gif
{

//                    //
// Helpful Exceptions //
//                    //

// GifLoadingException
GifLoadingException::GifLoadingException(std::string error)
    : std::runtime_error(error)
{}

std::ostream& operator<<(std::ostream& os, const GifLoadingException& e)
{
    return os << e.getErrorName() << ": " << e.what();
}

// EndOfStreamException
EndOfStreamException::EndOfStreamException(std::string error)
    : GifLoadingException(error)
{}

std::string EndOfStreamException::getErrorName() const
{
    return "[EndOfStreamException]";
}

// BadBlockException
BadBlockException::BadBlockException(std::string error)
    : GifLoadingException(error)
{}

std::string BadBlockException::getErrorName() const
{
    return "[BadBlockException]";
}

// ColorTableException
ColorTableException::ColorTableException(std::string error)
    : GifLoadingException(error)
{}

std::string ColorTableException::getErrorName() const
{
    return "[ColorTableException]";
}

} // namespace gif

} // namespace zmath
