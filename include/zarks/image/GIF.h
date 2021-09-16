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
        void Save(std::string path, std::vector<RGBA> palette, VecInt bounds = VecInt(0, 0)) const;

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
    };
} // namespace zmath
