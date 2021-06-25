#pragma once

#include "Color.h"
#include "Rect.h"
#include "Map.h"
#include "VecT.h"

#include <string>
#include <vector>
#include <array>

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

		~Image();

		VecInt Bounds() const;

		Image& operator= (const Image& img);
		bool operator!() const;

		// Accessors and copy/paste

		RGBA& At(int x, int y);
		RGBA& At(VecInt pos);
		const RGBA& At(int x, int y) const;
		const RGBA& At(VecInt pos) const;

		RGBA* const& operator[](int x);
		const RGBA* const& operator[](int x) const;

		Image& Copy() const;
		Image& Copy(zmath::VecInt min, zmath::VecInt max) const;
		Image& Paste(Image img, VecInt at);
		Image& Paste(Image img, Rect within);

		// Manipulators
		
		Image& Resize(VecInt to_bounds);
		Image& Resize(double scaleFactor);
		Image& Negative();
		Image& RestrictPalette(const std::vector<RGBA>& palette);
		Image& Fractalify(int octaves);
		Image& Droppify(std::array<Vec, 3> origins, std::array<double, 3> periods);
		Image& BlurGaussian(double sigma);
		Image& WarpGaussian(const Map& map, double sigma);

		// Save an image using STBI
		void Save(std::string path, unsigned int channels = 3) const;

	private:
		RGBA** data;
		zmath::VecInt bounds;

		bool isCopy;
	};
}