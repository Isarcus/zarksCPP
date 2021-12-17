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
    , _reservedBits(0)
    , sortFlag(0)
    , interlaceFlag(0)
    , localTableFlag(0)
{}

IDFlags::IDFlags(uint8_t flagByte)
    : colorTableSize ( flagByte & COLOR_TABLE_SIZE)
    , _reservedBits  ((flagByte & RESERVED_BITS)          >> 3)
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
        << "   -> _reservedBits:  " << desc.flags._reservedBits << "\n"
        << "   -> sortFlag:       " << desc.flags.sortFlag << "\n"
        << "   -> interlaceFlag:  " << desc.flags.interlaceFlag << "\n"
        << "   -> localTableFlag: " << desc.flags.localTableFlag << "\n";
}

//                   //
// GraphicsExtension //
//                   //

GraphicsExtension::GraphicsExtension()
    : transparentFlag(0)
    , userInputFlag(0)
    , disposalMethod(0)
    , _reservedBits(0)
{}

GraphicsExtension::GraphicsExtension(std::istream& is)
{
    // Read first byte; this must be ExtensionType::GRAPHICS
    ExtensionType extType = (ExtensionType)is.get();
    if (extType != ExtensionType::GRAPHICS)
    {
        throw std::runtime_error("Called GraphicsExtension constructor on a non-graphics control block!");
    }

    // Read initial Graphics Control byte and make sure it is 4
    uint8_t byteSize = (uint8_t)is.get();
    if (byteSize != 4)
    {
        throw FormatException("Expected Graphics Extension of size 4, not " + std::to_string(byteSize));
    }

    // Read in the rest of the block
    uint8_t buf[5], *ptr = buf;
    is.read((char*)buf, 5);

    // Interpret packed field
    uint8_t flagByte = buf[0];
    transparentFlag =  flagByte & TRANSPARENT_FLAG;
    userInputFlag   = (flagByte & USER_INPUT_FLAG) >> 1;
    disposalMethod  = (flagByte & DISPOSAL_METHOD) >> 2;
    _reservedBits   = (flagByte & RESERVED_BITS)   >> 5;

    // Read in duration
    duration = FromBytes<uint16_t>(ptr + 1, Endian::Little);
    
    // Read in transparency index, if applicable
    transparentIdx = (transparentFlag) ? buf[3] : 0;

    // Sanity check - ensure block terminator is null
    if (buf[4])
    {
        throw FormatException("No null terminator on Graphics Extension block");
    }
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
