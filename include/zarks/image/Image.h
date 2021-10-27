#pragma once

#include <zarks/internal/Sampleable2D.h>
#include <zarks/image/color.h>
#include <zarks/math/Rect.h>
#include <zarks/math/Map.h>
#include <zarks/math/VecT.h>

#include <string>
#include <vector>
#include <array>
#include <memory>

namespace zmath
{
	class Image : public Sampleable2D<RGBA>
	{
	public:
		Image(int width, int height, RGBA col = RGBA::Black());
		Image(VecInt bounds_in, RGBA col = RGBA::Black());
		Image(const Map& m);
		Image(const Map& m, Scheme scheme);
		Image(std::string path);
		Image(const Image& img);
		Image(Image&& img);
		Image();

		~Image();

		VecInt Bounds() const;

		Image& operator=(const Image& img);
		Image& operator=(Image&& img);

		// Copy/paste

		Image& Paste(const Image& img, VecInt at);
		Image& Tile(const Image& tile, VecInt tileSize, VecInt offset = VecInt(0, 0));

		// Manipulators
		
		Image& Resize(VecInt to_bounds);
		Image& Resize(double scaleFactor);
		Image& Clear(RGBA col = RGBA::Black());
		Image& Negative();
		Image& RestrictPalette(const std::vector<RGBA>& palette);
		Image& Fractalify(int octaves);
		Image& Droppify(const std::array<Vec, 3>& origins, const std::array<double, 3>& periods);
		Image& BlurGaussian(double sigma, bool blurAlpha = true);
		Image& PixelateGaussian(const Map& map, double sigma);
		Image& EnhanceContrast(double sigma);

		// Save an image using STBI
		void Save(std::string path, unsigned int channels = 3) const;
		void SaveMNIST(std::string path_images, std::string path_labels, int columns, int emptyBorderSize = 2) const;
	};
}
