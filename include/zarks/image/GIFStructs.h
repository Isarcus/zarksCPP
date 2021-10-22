#pragma once

#include <vector>
#include <iostream>
#include <cstdint>

namespace zmath
{

namespace gif
{
    static constexpr const char* HEADER_89a = "GIF89a";
    static constexpr const char* HEADER_87a = "GIF87a";

    static constexpr uint16_t MAX_CODE_TABLE_SIZE = 0xFFF;
    static constexpr size_t IMAGE_DESCRIPTOR_LENGTH = 9;

    // Enum of all possible block types in a GIF.
    // The first byte of any new block *must* be one of these
    // values, or the GIF is unreadable.
    enum class BlockType : uint8_t
    {
        EXTENSION   = 0x21,
        IMAGE       = 0x2C,
        END_OF_FILE = 0x3B
    };

    // Enum of all possible extension block sub-types in a GIF.
    // Any extension block *must* have one of these values as
    // the second byte, or the GIF is unreadable.
    enum class ExtensionType : uint8_t
    {
        APPLICATION = 0xFF,
        GRAPHICS    = 0xF9,
        PLAINTEXT   = 0x01,
        COMMENT     = 0xFE
    };

    // Struct for Logical Screen Descriptor bit flags
    typedef struct LSDFlags
    {
        // Zero-initializes all fields
        LSDFlags();
        // Requires the fifth byte of the Logical Screen Descriptor
        LSDFlags(uint8_t flagByte);

        /* * * * * * * * * */
        /* D C C C B A A A */
        /* * * * * * * * * */

        /* A */ unsigned colorTableSize : 3;
        /* B */ unsigned sortFlag : 1;
        /* C */ unsigned colorResolution : 3;
        /* D */ unsigned globalTableFlag : 1;

        /* A */ static constexpr uint8_t COLOR_TABLE_SIZE        = 0b00000111;
        /* B */ static constexpr uint8_t SORT_FLAG               = 0b00001000;
        /* C */ static constexpr uint8_t COLOR_RESOLUTION        = 0b01110000;
        /* D */ static constexpr uint8_t GLOBAL_COLOR_TABLE_FLAG = 0b10000000;
    } LSDFlags;

    // Struct for Image Descriptor bit flags
    typedef struct IDFlags
    {
        // Zero-initializes all fields
        IDFlags();
        // Requires the tenth byte of an Image Descriptor
        IDFlags(uint8_t flagByte);

        /* * * * * * * * * */
        /* E D C B B A A A */
        /* * * * * * * * * */

        /* A */ unsigned colorTableSize : 3;
        /* B */ unsigned __reservedBits : 2;
        /* C */ unsigned sortFlag : 1;
        /* D */ unsigned interlaceFlag : 1;
        /* E */ unsigned localTableFlag : 1;

        /* A */ static constexpr uint8_t COLOR_TABLE_SIZE       = 0b00000111;
        /* B */ static constexpr uint8_t RESERVED_BITS          = 0b00011000;
        /* C */ static constexpr uint8_t SORT_FLAG              = 0b00100000;
        /* D */ static constexpr uint8_t INTERLACE_FLAG         = 0b01000000;
        /* E */ static constexpr uint8_t LOCAL_COLOR_TABLE_FLAG = 0b10000000;
    } IDFlags;

    typedef struct LZWFrame
    {
        uint8_t minCodeSize;
        std::vector<uint8_t> data;
    } LZWFrame;

    typedef struct ImageDescriptor
    {
        // Requires that the stream has *just* read the first byte
        // of an image descriptor, i.e. BlockType::IMAGE.
        ImageDescriptor(std::istream& is);

        uint16_t offsetWidth, offsetHeight;
        uint16_t width, height;
        IDFlags flags;

        friend std::ostream& operator<<(std::ostream& os, const ImageDescriptor& desc);
    } ImageDescriptor;

    //            //
    // Exceptions //
    //            //

    class GifLoadingException : public std::runtime_error
    {
    public:
        GifLoadingException(std::string error);
        virtual std::string getErrorName() const = 0;

        friend std::ostream& operator<<(std::ostream& os, const GifLoadingException& e); 
    };

    class EndOfStreamException : public GifLoadingException
    {
    public:
        EndOfStreamException(std::string error);
        virtual std::string getErrorName() const override;
    };

    class BadStreamException : public GifLoadingException
    {
    public:
        BadStreamException(std::string error);
        virtual std::string getErrorName() const override;
    };

    class FormatException : public GifLoadingException
    {
    public:
        FormatException(std::string error);
        virtual std::string getErrorName() const override;
    };

} // namespace gif

} // namespace zmath
