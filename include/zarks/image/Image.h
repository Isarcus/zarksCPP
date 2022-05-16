#pragma once

#include <zarks/internal/Mat2D.h>
#include <zarks/image/color.h>
#include <zarks/math/Map.h>
#include <zarks/math/VecT.h>

#include <string>
#include <vector>
#include <array>

namespace zmath
{
    class Image : public Mat2D<RGBA>
    {
    public:
        Image(int width, int height, RGBA col = RGBA::Black());
        Image(VecInt bounds_in, RGBA col = RGBA::Black());
        Image(const Mat2D<double>& mat);
        Image(const Mat2D<double>& mat, const Scheme& scheme);
        Image(std::string path);
        Image(const Image& img);
        Image(Image&& img);
        Image();

        Image& operator=(const Image& img);
        Image& operator=(Image&& img);

        // Image-specific functions
        
        Image& Tile(const Image& tile, VecInt tileSize, VecInt offset = VecInt(0, 0));
        Image& Resize(VecInt toBounds);
        Image& Resize(double scaleFactor);
        Image& Negative();
        Image& RestrictPalette(const std::vector<RGBA>& palette);
        Image& Fractalify(int octaves);
        Image& Droppify(const std::array<Vec, 3>& origins, const std::array<double, 3>& periods);
        Image& BlurGaussian(double sigma, bool blurAlpha = true);
        Image& PixelateGaussian(const Map& map, double sigma);
        Image& WarpGaussian(const Map& map, double sigma, double amplitude);
        Image& EnhanceContrast(double sigma);

        // Save an image using STBI
        void Save(std::string path, unsigned int channels = 3) const;
        void SaveMNIST(std::string path_images, std::string path_labels, int columns, int emptyBorderSize = 2) const;
    };

} // namespace zmath
