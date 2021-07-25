#pragma once

#include "color.h"
#include "Rect.h"
#include "Map.h"
#include "VecT.h"

#include <string>
#include <vector>
#include <array>
#include <memory>

namespace zmath
{
	class Image {
	public:
		Image(int width, int height, RGBA col = RGBA::Black());
		Image(zmath::VecInt bounds_in, RGBA col = RGBA::Black());
		Image(const Map& m);
		Image(const Map& m, Scheme scheme);
		Image(std::string path);
		Image(const Image& img);
		Image(Image&& img);
		Image();

		~Image();

		VecInt Bounds() const;

		Image& operator= (const Image& img);
		Image& operator= (Image&& img);

		// Accessors and copy/paste

		RGBA& At(int x, int y);
		RGBA& At(VecInt pos);
		const RGBA& At(int x, int y) const;
		const RGBA& At(VecInt pos) const;

		RGBA* const& operator[](int x);
		const RGBA* operator[](int x) const;

		std::unique_ptr<Image> Copy(zmath::VecInt min, zmath::VecInt max) const;
		Image& Paste(Image img, VecInt at);
		Image& Paste(Image img, Rect within);

		// Manipulators
		
		Image& Resize(VecInt to_bounds);
		Image& Resize(double scaleFactor);
		Image& Clear(RGBA col = RGBA::Black());
		Image& Negative();
		Image& RestrictPalette(const std::vector<RGBA>& palette);
		Image& Fractalify(int octaves);
		Image& Droppify(std::array<Vec, 3> origins, std::array<double, 3> periods);
		Image& BlurGaussian(double sigma, bool blurAlpha = true);
		Image& PixelateGaussian(const Map& map, double sigma);
		Image& EnhanceContrast(double sigma);

		// Save an image using STBI
		void Save(std::string path, unsigned int channels = 3) const;
		void SaveMNIST(std::string path_images, std::string path_labels, int columns, int emptyBorderSize = 2) const;

	private:
		RGBA** data;
		zmath::VecInt bounds;

		bool isCopy;
	};
}