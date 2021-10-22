#include <zarks/image/GIFStructs.h>
#include <zarks/io/binary.h>

namespace zmath
{

namespace gif
{

//          //
// LSDFlags //
//          //

LSDFlags::LSDFlags()
    : colorTableSize(0)
    , sortFlag(0)
    , colorResolution(0)
    , globalTableFlag(0)
{}

LSDFlags::LSDFlags(uint8_t flagByte)
    : colorTableSize  ( flagByte & COLOR_TABLE_SIZE)
    , sortFlag        ((flagByte & SORT_FLAG)               >> 3)
    , colorResolution ((flagByte & COLOR_RESOLUTION)        >> 4)
    , globalTableFlag ((flagByte & GLOBAL_COLOR_TABLE_FLAG) >> 7)
{}

//         //
// IDFlags //
//         //

IDFlags::IDFlags()
    : colorTableSize(0)
    , __reservedBits(0)
    , sortFlag(0)
    , interlaceFlag(0)
    , localTableFlag(0)
{}

IDFlags::IDFlags(uint8_t flagByte)
    : colorTableSize ( flagByte & COLOR_TABLE_SIZE)
    , __reservedBits ((flagByte & RESERVED_BITS)          >> 3)
    , sortFlag       ((flagByte & SORT_FLAG)              >> 5)
    , interlaceFlag  ((flagByte & INTERLACE_FLAG)         >> 6)
    , localTableFlag ((flagByte & LOCAL_COLOR_TABLE_FLAG) >> 7)
{}

//                 //
// ImageDescriptor //
//                 //

ImageDescriptor::ImageDescriptor(std::istream& is)
{
    uint8_t buf[IMAGE_DESCRIPTOR_LENGTH];
    const uint8_t *ptr = buf;

    is.read((char*)buf, IMAGE_DESCRIPTOR_LENGTH);

    offsetWidth = ReadBuf<uint16_t>(ptr, Endian::Little);
    offsetHeight = ReadBuf<uint16_t>(ptr, Endian::Little);
    width = ReadBuf<uint16_t>(ptr, Endian::Little);
    height = ReadBuf<uint16_t>(ptr, Endian::Little);
    flags = IDFlags(*ptr);
}

std::ostream& operator<<(std::ostream& os, const ImageDescriptor& desc)
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

//            //
// Exceptions //
//            //

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

// BadStreamException
BadStreamException::BadStreamException(std::string error)
    : GifLoadingException(error)
{}

std::string BadStreamException::getErrorName() const
{
    return "[BadStreamException]";
}

// FormatException
FormatException::FormatException(std::string error)
    : GifLoadingException(error)
{}

std::string FormatException::getErrorName() const
{
    return "[FormatException]";
}

} // namespace gif

} // namespace zmath
