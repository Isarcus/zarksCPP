#include <zarks/image/GIF.h>
#include <zarks/math/binary.h>

#include <fstream>
#include <cstring>
#include <sstream>
#include <iomanip>

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
    char descriptor[7], *ptr = descriptor;
    is.read(descriptor, 7);
    __attribute__((unused)) uint16_t canvasWidth = FromBytes<uint16_t>(AdvancePtr(ptr, 2), Endian::Little);
    __attribute__((unused)) uint16_t canvasHeight = FromBytes<uint16_t>(AdvancePtr(ptr, 2), Endian::Little);
    LSDFlags flags(*AdvancePtr(ptr, 1));
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
        } catch (const gif::ColorTableException& e) {
            std::cerr << e << "\n";
            break;
        } catch (const gif::BadBlockException& e) {
            std::cerr << e << "\n";
            break;
        } catch (const gif::EndOfStreamException& e) {
            std::cout << e << "\n";
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
    , sortFlag        ((flagByte & SORT_FLAG)              >> 3)
    , colorResolution ((flagByte & COLOR_RESOLUTION)       >> 4)
    , globalTableFlag ((flagByte & GLOBAL_COLOR_TABLE_FLAG >> 7))
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

//                 //
// Private Methods //
//                 //

std::pair<Image, uint16_t> GIF::loadNextFrame(std::istream& is, const std::vector<RGBA>& globalColorTable)
{
    uint8_t identityByte;
    is.read((char*)(&identityByte), 1);

    // Make sure stream is still valid. If not, assume EOF has been reached
    if (!is)
    {
        throw gif::EndOfStreamException("Failed to read first byte of block");
    }

    // Declarations
    Image image;
    uint16_t duration = 0;

    // Based on the type of this block, attempt to either load in a new
    // frame or interpret an extension block
    switch (identityByte)
    {
    case EXTENSION_INTRODUCER: {
        // TODO
        break;
    } // case EXTENSION_INTRODUCER

    case IMAGE_SEPARATOR: {
        // Get the image descriptor
        ImageDescriptor desc(is);

        // Load raw image data
        auto rawData = loadImageData(is);
        // Parse raw data for LZW codes
        auto lzwCodes = decompressLZW(rawData);

        // Read color codes and create image
        if (desc.flags.localTableFlag) {
            std::vector<RGBA> localColorTable = loadColorTable(is, getColorTableSize(desc.flags.colorTableSize));
            image = decodeImage(lzwCodes, localColorTable);
        } else if (globalColorTable.size()) {
            image = decodeImage(lzwCodes, globalColorTable);
        } else {
            throw gif::ColorTableException("Missing global and local color table!");
        }
        break;
    } // case IMAGE_SEPARATOR

    default: {
        std::ostringstream errstr;
        errstr << "Unrecognized first byte of block: 0x" << std::hex
               << std::setw(2) << std::setfill('0') << (int)identityByte;
        throw gif::BadBlockException(errstr.str());
    } // default

    } // switch

    return std::pair<Image, uint16_t>(image, duration);
}

GIF::LZWFrame GIF::loadImageData(std::istream& is)
{
    LZWFrame frame;

    // Read LZW Minimum Code Size
    is.read((char*)&frame.minCodeSize, 1);
    if (!is)
    {
        throw gif::BadBlockException("No image data to load following image separator!");
    }

    // Before actually reading in all of the sub-blocks, seek through the stream
    // to determine how many blocks need to be read (and how big they are)

    // totalSeeked counts bytes seeked relative to the very first byte of the
    // first image sub-block (i.e. header[2] up above)
    uint8_t nextBlockSize = 1;
    long totalSeeked = 1; 
    std::vector<short> subBlockSizes;
    while (nextBlockSize)
    {
        // Get size of the next block
        is.read((char*)&nextBlockSize, 1);
        is.seekg(nextBlockSize, std::ios_base::cur);

        if (is) {
            totalSeeked += short(nextBlockSize) + 1;
            subBlockSizes.push_back(nextBlockSize);
        } else {
            throw gif::BadBlockException("Bad image data!");
        }
    }

    // With the length of all sub-blocks determined, reset the stream back
    // to the beginning of the image data
    is.seekg(-totalSeeked, std::ios_base::cur);

    // The final number of bytes of all sub-blocks put together, after
    // removing separators, should be totalSeeked - subBlockSizes.size()
    frame.data.resize(totalSeeked - subBlockSizes.size());

    // Read all LZW data into frame.data, ignoring separators
    char* ptr = (char*)frame.data.data();
    for (auto thisBlockSize : subBlockSizes)
    {
        uint8_t checkThisBlockSize;
        is.read((char*)checkThisBlockSize, 1);

        if (checkThisBlockSize == thisBlockSize) {
            is.read(ptr, thisBlockSize);
            ptr += thisBlockSize;
        } else {
            throw gif::BadBlockException("GIF block size mismatch!");
        }
    }

    // Make sure final byte is null as a sanity check
    uint8_t shouldBeNull = 1;
    is.read((char*)&shouldBeNull, 1);
    if (shouldBeNull)
    {
        throw gif::BadBlockException("Putative final byte of LZW data was not null!");
    }

    return frame;
}

std::vector<uint16_t> GIF::decompressLZW(const LZWFrame& data)
{
    std::vector<uint16_t> codes;

    // TODO

    return codes;
}

Image GIF::decodeImage(const std::vector<uint16_t>& codes, const std::vector<RGBA>& colorTable)
{
    Image image;

    // TODO

    return image;
}

std::vector<RGBA> GIF::loadColorTable(std::istream& is, uint8_t numColors)
{
    const unsigned bytesToRead = (unsigned)numColors * 3;
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
