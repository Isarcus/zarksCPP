#pragma once

#include <zarks/image/color.h>
#include <zarks/image/Image.h>

#include <deque>
#include <vector>

namespace zmath
{
    class GIF
    {
    public:
        // Create an empty GIF object, ready to accept new frames.
        GIF();
        // Load a GIF from a filepath. If the path does not exist,
        // throws std::runtime_error.
        GIF(std::string path);
        // Load a GIF from a binary input stream.
        GIF(std::istream&& is);
        // Load a GIF from a binary input stream.
        GIF(std::istream& is);

        // Write a GIF to file!
        // @param path the path at which to save the GIF. This should end in ".gif"
        // @param bounds the bounds of each frame of the GIF.
        //        You may leave this value as zeroes to default
        //        to the size of the first frame in the GIF.
        // @param paletteSize The number of colors to use in the GIF.
        //        the actual number of colors used will be the passed
        //        value plus one. Min 1 and max 255 (really 2-256).
        void Save(std::string path, uint8_t paletteSize = 255, VecInt bounds = VecInt(0, 0)) const;

        // Write a GIF to file!
        // @param path the path at which to save the GIF. This should end in ".gif"
        // @param bounds the bounds of each frame of the GIF.
        //        You may leave this value as zeroes to default
        //        to the size of the first frame in the GIF.
        // @param palette the set of colors to use in the GIF.
        //        If palette.size() < 2, this function throws
        //        std::runtime_error. If palette.size() > 256,
        //        only the first 256 colors will be used.
        void Save(std::string path, const std::vector<RGBA>& palette, VecInt bounds = VecInt(0, 0)) const;

        // Adds a frame to the GIF.
        // @param img the Image to be added
        // @param adjustBounds whether to fit the bounds of this frame
        //        to those of the frame at index 0.
        // @param idx Where to insert this frame in the frame array.
        //        Allowable values include -1 for the very end of the
        //        array, 0 for the very beginning, and any value in the
        //        range [1, GIF.Size()) to insert somewhere in the middle. 
        void Add(const Image& img, bool adjustBounds = false, int idx = -1);

        // Frame accessor
        Image& At(int frame);
        // Const frame accessor
        const Image& At(int frame) const;

        // Returns the number of frames in the GIF
        int Size() const;

        // Returns the bounds of the first frame in the GIF.
        // For a newly loaded GIF (i.e. not default-constructed and appended
        // to by the user), the bounds will be the same for all frames.
        VecInt Bounds() const;

    private:
        std::deque<Image> frames;

        static constexpr const char* HEADER_89a = "GIF89a";
        static constexpr const char* HEADER_87a = "GIF87a";

        static constexpr uint16_t MAX_CODE_TABLE_SIZE = 0xFFF;

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
            // of an image descriptor, i.e. IMAGE_SEPARATOR.
            ImageDescriptor(std::istream& is);

            uint16_t offsetWidth, offsetHeight;
            uint16_t width, height;
            IDFlags flags;
        } ImageDescriptor;

        friend std::ostream& operator<<(std::ostream& os, const ImageDescriptor& desc);

        //                 //
        // Helpful Methods //
        //                 //

        // Load a frame from the GIF input stream, or throw EndOfStreamException 
        //  if there are no more frames to be loaded. If `globalColorTable` is 
        //  empty and the loaded frame is missing a local color table, then an 
        //  instance of NoColorTableException will be thrown.
        // @param is the input stream to read from.
        // @param globalColorTable the global color table. This should be left
        //        empty if there is no global color table.
        // @return An std::pair containing an image and the duration of that
        //         image. the duration will be 0 if not specified by a graphics
        //         extension block preceding the loaded image.
        std::pair<Image, uint16_t> loadNextFrame(std::istream& is, const std::vector<RGBA>& globalColorTable);

        // Process an extension block, starting at the byte immediately following
        //  the EXTENSION_INTRODUCER byte. If the first byte that this function
        //  reads does not match any of the types enumerated in ExtensionTypes,
        //  then BadBlockException will be thrown.
        // @param is the input stream to read from.
        void readExtensionBlock(std::istream& is);

        // Load an image from the GIF input stream, starting immediately after 
        //  the Image Descriptor bytes
        // @param is the input stream to read from.
        // @return the raw LZW-compressed image data of one image frame,
        //         without the separator byte in between blocks.
        static LZWFrame loadImageData(std::istream& is);

        // Load raw sub-block data with separator bytes removed, starting
        //  at the first byte (i.e. the size indicator byte) of the first
        //  sub-block.
        static std::vector<uint8_t> loadSubBlocks(std::istream& is);

        // Decode raw LZW data into color indices.
        // @param data the raw LZW-compressed image data of one image frame.
        // @return a vector of 8-bit color indices.
        static std::vector<uint8_t> decompressLZW(const LZWFrame& data);

        // Decode a series of LZW image codes given a color table.
        //  If any LZW codes reference a color beyond the length of the
        //  color table, then ColorTableOutOfBoundsException will be
        //  thrown.
        // @param bounds the bounds of the image to be created.
        // @param indices color indices.
        // @param colorTable a colorTable. This function will throw a
        //        gif::ColorTableException if anyindex exceeds the size
        //        of the colorTable.
        // @return A fully decoded image.
        static Image decodeImage(VecInt bounds, const std::vector<uint8_t>& indices, const std::vector<RGBA>& colorTable);

        // @param is the input stream to read from.
        // @param numColors this should refer to the total number of colors in
        //                  the table, i.e. 2^(1 + LSDFlags::colorTableSize).
        // @return An RGB color table of size `numColors`
        static std::vector<RGBA> loadColorTable(std::istream& is, unsigned numColors);

        // @param bitField a value from 0-7; the last three bits of
        //        the fifth byte of the Logical Screen Descriptor or
        //        the last three bits of the tenth byte of an Image
        //        Descriptor (e.g. LSDFlags::colorTableSize and
        //        IDFlags::colorTableSize).
        // @return 2^(1 + bitField)
        static int getColorTableSize(int bitField);

    };

    namespace gif
    {
        //                    //
        // Helpful Exceptions //
        //                    //

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

        class BadBlockException : public GifLoadingException
        {
        public:
            BadBlockException(std::string error);
            virtual std::string getErrorName() const override;
        };

        class ColorTableException : public GifLoadingException
        {
        public:
            ColorTableException(std::string error);
            virtual std::string getErrorName() const override;
        };

    } // namespace gif
    
} // namespace zmath
