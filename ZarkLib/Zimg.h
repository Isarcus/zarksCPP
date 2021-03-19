#pragma once

#include "Color.h"
#include "Rect.h"
#include "Map.h"

#include <string>

namespace zimg
{
	class Image {
	public:
		Image(int width, int height, RGBA col = RGBA::Black());
		Image(zmath::Vec bounds, RGBA col = RGBA::Black());
		Image(zmath::Map& m);
		Image(zmath::Map& m, Scheme scheme);
		Image(std::string path);

		void Delete();

		void operator= (Image img);
		bool operator!() const;

		// Accessors and copy/paste

		RGBA At(int x, int y) const;
		RGBA At(zmath::Vec pos) const;
		bool Set(int x, int y, RGBA col);
		bool Set(zmath::Vec pos, RGBA col);

		Image& Copy() const;
		Image& Copy(zmath::Vec min, zmath::Vec max) const;
		Image& Copy(zmath::Rect rect) const;
		Image& Paste(Image img, zmath::Vec at);
		Image& Paste(Image img, zmath::Rect within);

		// Manipulators
		
		Image Negative();

		// Save an image using STBI
		void Save(std::string path, unsigned int channels) const;

	private:
		RGBA** data;
		zmath::Vec bounds;

		bool isCopy;
	};
}