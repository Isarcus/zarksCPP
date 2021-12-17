#pragma once

#include <zarks/image/color.h>
#include <zarks/image/Image.h>
#include <zarks/image/GIFStructs.h>

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
        // @param path the path at which to save the GIF. This should end
        //        in ".gif"
        // @param bounds the dimensions with which to save the GIF. If left
        //        uninitialized, defaults to the dimensions of the first
        //        frame in the GIF.
        // @param palette the global palette to use for coloring the GIF.
        //        Only the first 256 colors will be used. If left
        //        uninitialized, defaults to a palette of size 256.
        // @param durations the durations of each frame in the GIF.
        //        If the vector has only one element, then that element will
        //        determine the duration of all frames in the GIF.
        //        If the vector has fewer elements than there are frames, then
        //        frame durations will "wrap" around the vector.
        //        If the vector has more elements than there are frames, then
        //        only the first <number of frames> elements will be used.
        void Save(std::string path,
                  VecInt bounds = VecInt(),
                  const std::vector<RGBA>& palette = {},
                  const std::vector<double>& durations = {}) const;

        // Write a GIF to file!
        // @param paletteSize the size of the palette to use for each image.
        // @param global whether all frames should share the same palette.
        void Save(std::string path,
                  bool global,
                  VecInt bounds = VecInt(),
                  unsigned paletteSize = 256,
                  const std::vector<double>& durations = {}) const;

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

        //                      //
        // Helpful SAVE Methods //
        //                      //

        // Evaluate the true size of a color table.
        static unsigned computeColorTableSize(const std::vector<RGBA>& palette);
        static unsigned computeColorTableSize(unsigned numColors);

        // @param numColors the number of colors in the final palette.
        //        This value will be rounded up to the nearest power of 2, up
        //        to 256.
        // @return a color palette.
        static std::vector<RGBA> getDefaultPalette(unsigned numColors);

        // Compute a palette to represent a frame of a GIF using the K
        // Means algorithm.
        // @param frame the frame for which to compute the palette.
        // @param numColors the number of colors in the final palette.
        //        This value will be rounded up to the nearest power of 2, up
        //        to 256.
        // @return A color palette.
        static std::vector<RGBA> getKMeansPalette(const Image& frame, unsigned numColors);

        // Write the Logical Screen Descriptor.
        // @param os the output stream to write to.
        // @param bounds the dimensions of the GIF.
        // @param globalTable whether there is a global color table.
        // @param globalTableSize the size of the global color table.
        //        This value is ignored if globalTable is false.
        static void writeLSD(std::ostream& os, VecInt bounds, bool globalTable, unsigned globalTableSize);

        // Write the Netscape 2.0 Application Extension block.
        // @param os the output stream to write to.
        // @param numReps the number of loops to execute before the GIF stops
        //        playing. Leave this as zero for infinite repetitions.
        static void writeNetscape2Extension(std::ostream& os, uint16_t numReps = 0);

        // Write a Graphics Control Extension block.
        // @param os the output stream to write to.
        // @param duration the length of time, in seconds, that the next frame
        //        should be displayed for. This will be rounded to hundredths
        //        of a second, and the maximum value is (2^16 - 1) / 100, or
        //        655.35.
        static void writeGraphicsExtension(std::ostream& os, double duration);

        // Write a color table.
        // @param os the output stream to write to.
        // @param palette the colors to write. Order will be preserved.
        //        If palette.size() < 2, std::runtime_error will be thrown.
        //        If palette.size() > 256, only the first 256 colors will be used.
        //        If palette.size() is within the specified range but is not a
        //        power of 2, the total number of colors written will be rounded
        //        up to the nearest power of 2.
        static void writeColorTable(std::ostream& os, const std::vector<RGBA>& palette);

        // Write a single frame as an Image Block.
        // @param os the output stream to write to.
        // @param frame the image to write.
        static void writeFrame(std::ostream& os, const Image& frame, VecInt bounds, const std::vector<RGBA>& palette, bool writeTable);

        // Compress a series of 8-bit indices to LZW codes and write them to the
        //  output stream.
        // @param os the output stream to write to.
        // @param indices the indices to compress.
        static void compressLZW(std::ostream& os, const std::vector<uint8_t>& indices);

        // Write blocks of bytes to an output stream.
        // @param os the output stream to write to.
        // @param data a pointer to the data to be written.
        // @param bytes the number of bytes to write.
        static void writeSubBlocks(std::ostream& os, const void* data, size_t bytes);

        //                      //
        // Helpful LOAD Methods //
        //                      //

        // Load a frame from the GIF input stream, or throw EndOfStreamException 
        //  if there are no more frames to be loaded. If `globalColorTable` is 
        //  empty and the loaded frame is missing a local color table, then
        //  FormatException will be thrown.
        // @param is the input stream to read from.
        // @param globalColorTable the global color table. This should be left
        //        empty if there is no global color table.
        // @return An std::pair containing an image and the duration of that
        //         image. the duration will be 0 if not specified by a graphics
        //         extension block preceding the loaded image.
        std::pair<Image, uint16_t> loadNextFrame(std::istream& is, VecInt canvasBounds, const std::vector<RGBA>& globalColorTable);

        // Process an extension block, starting at the byte immediately following
        //  the EXTENSION_INTRODUCER byte. If the first byte that this function
        //  reads does not match any of the types enumerated in ExtensionTypes,
        //  then FormatException will be thrown.
        // @param is the input stream to read from.
        void readExtensionBlock(std::istream& is);

        // Load an image from the GIF input stream, starting immediately after 
        //  the Image Descriptor bytes
        // @param is the input stream to read from.
        // @return the raw LZW-compressed image data of one image frame,
        //         without the separator byte in between blocks.
        static gif::LZWFrame loadImageData(std::istream& is);

        // Load raw sub-block data with separator bytes removed, starting
        //  at the first byte (i.e. the size indicator byte) of the first
        //  sub-block.
        static std::vector<uint8_t> loadSubBlocks(std::istream& is);

        // Decode raw LZW data into color indices.
        // @param data the raw LZW-compressed image data of one image frame.
        // @return a vector of 8-bit color indices.
        static std::vector<uint8_t> decompressLZW(const gif::LZWFrame& data);

        // Decode a series of LZW image codes given a color table.
        //  If any LZW codes reference a color beyond the length of the
        //  color table, then FormatException will be thrown.
        // @param bounds the bounds of the image to be created.
        // @param indices color indices.
        // @param colorTable a colorTable. This function will throw a
        //        gif::ColorTableException if anyindex exceeds the size
        //        of the colorTable.
        // @return A fully decoded image.
        static Image decodeImage(VecInt canvasBounds,
                                 VecInt frameBounds,
                                 VecInt offset,
                                 const std::vector<uint8_t>& indices,
                                 const std::vector<RGBA>& colorTable);
        static Image decodeImage(VecInt canvasBounds,
                                 VecInt frameBounds,
                                 VecInt offset,
                                 const std::vector<uint8_t>& indices,
                                 const std::vector<RGBA>& colorTable,
                                 const Image& prevFrame,
                                 int transparentIdx);

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
        static int loadColorTableSize(int bitField);

        // Return an LZW code table containing only the basic color codes,
        // clear code, and EOI code.
        // @param minCodeSize the minimum size of a *color* code, in bits.
        static std::vector<std::vector<uint8_t>> getBaseCodeTable(int minCodeSize);
    };
    
} // namespace zmath
