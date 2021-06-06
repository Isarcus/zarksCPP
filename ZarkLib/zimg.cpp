#include "pch.h"
#include "Zimg.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_MSC_SECURE_CRT // apparently necessary for Visual Studio
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>

#define LOOP_IMAGE for (int x = 0; x < bounds.X; x++) for (int y = 0; y < bounds.Y; y++)

using namespace zmath;

namespace zimg
{
	Image::Image(int width, int height, RGBA col)
	{
		isCopy = false;
		
		if (width < 1 || height < 1)
		{
			data = nullptr;
			bounds = Vec();
			return;
		}

		bounds = Vec(width, height);

		data = new RGBA* [width];
		for (int x = 0; x < width; x++)
		{
			data[x] = new RGBA[height];
			for (int y = 0; y < height; y++)
			{
				data[x][y] = col;
			}
		}
	}

	Image::Image(zmath::Vec bounds, RGBA col)
	{
		isCopy = false;
		
		if (bounds.X < 1 || bounds.Y < 1)
		{
			data = nullptr;
			bounds = Vec();
			return;
		}

		int width = bounds.X, height = bounds.Y;
		bounds = Vec(width, height);

		data = new RGBA* [width];
		for (int x = 0; x < width; x++)
		{
			data[x] = new RGBA[height];
			for (int y = 0; y < height; y++)
			{
				data[x][y] = col;
			}
		}
	}

	Image::Image(zmath::Map& m)
	{
		isCopy = false;

		if (m.Bounds().X < 1 || m.Bounds().Y < 1)
		{
			data = nullptr;
			bounds = Vec();
			return;
		}

		int width = m.Bounds().X, height = m.Bounds().Y;
		bounds = Vec(width, height);

		data = new RGBA* [width];
		for (int x = 0; x < width; x++)
		{
			data[x] = new RGBA[height];
			for (int y = 0; y < height; y++)
			{
				uint8 shade = 255.999 * m[x][y];
				data[x][y] = RGBA(shade, shade, shade);
			}
		}
	}

	Image::Image(zmath::Map& m, Scheme scheme)
	{
		isCopy = false;

		if (m.Bounds().X < 1 || m.Bounds().Y < 1)
		{
			data = nullptr;
			bounds = Vec();
			return;
		}

		int width = m.Bounds().X, height = m.Bounds().Y;
		bounds = Vec(width, height);

		// Create an accurate thresholds array
		double* thresholds = new double[scheme.n];
		thresholds[0] = 0;
		thresholds[scheme.n - 1] = 1;
		for (int i = 1; i < scheme.n - 1; i++) thresholds[i] = scheme.thresholds[i - 1];

		// Loop and assign colors
		data = new RGBA* [width];
		for (int x = 0; x < width; x++)
		{
			data[x] = new RGBA[height];
			for (int y = 0; y < height; y++)
			{
				double val = m[x][y];

				int idxUpper = 0;
				for (int i = 0; i < scheme.n; i++)
				{
					if (val < thresholds[i])
					{
						idxUpper = i;
						break;
					}
				}

				double min = thresholds[idxUpper - 1];
				double range = thresholds[idxUpper] - min;

				data[x][y] = RGBA::Interpolate(
					scheme.colors[idxUpper - 1],
					scheme.colors[idxUpper],
					(val - min) / range
				);
			}
		}

		delete[] thresholds;
	}

	Image::Image(std::string path)
	{
		int width, height, channels = -1;
		uint8* stbImg = stbi_load(path.c_str(), &width, &height, &channels, 0);

		// Abort if it fails to load, or you'll crash the damn computer again
		if (!stbImg || channels == -1) // I included the 'channels == -1' check bc I'm paranoid
		{
			std::cout << "[ERROR] Could not load image at " << path << "\n";

			data = nullptr;
			bounds = Vec();
			isCopy = false;
			return;
		}

		bounds = Vec(width, height);
		isCopy = false;

		// Allocate data
		data = new RGBA* [width];
		for (int x = 0; x < width; x++)
		{
			data[x] = new RGBA[height]{};
		}

		int stbIdx = 0;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				switch (channels)
				{
				case 1: // Grayscale
					data[x][y] = RGBA(stbImg[stbIdx]);
					stbIdx++;
					break;

				case 2: // Grayscale with alpha
					data[x][y] = RGBA(
						stbImg[stbIdx],
						stbImg[stbIdx + 1]);
					stbIdx += 2;
					break;

				case 3: // RGB
					data[x][y] = RGBA(
						stbImg[stbIdx],
						stbImg[stbIdx + 1],
						stbImg[stbIdx + 2]);
					stbIdx += 3;
					break;

				case 4: // RGBA
					data[x][y] = RGBA(
						stbImg[stbIdx],
						stbImg[stbIdx + 1],
						stbImg[stbIdx + 2],
						stbImg[stbIdx + 3]);
					stbIdx += 4;
					break;
				} // switch (channels)
			} // for x
		} // for y

		free(stbImg);

		std::cout << "Successfully loaded image at " << path << "\n"
			      << " -> Channels:   " << channels << "\n"
			      << " -> Dimensions: " << width << "x" << height << "\n";
	}

	Image::~Image()
	{
		if (!isCopy) for (int x = 0; x < bounds.X; x++) delete[] data[x];
		delete[] data;
	}

	void Image::operator=(Image img)
	{
		data = img.data;
		bounds = img.bounds;
		isCopy = img.isCopy;
	}

	bool Image::operator!() const
	{
		return data == nullptr;
	}

	RGBA& Image::At(int x, int y)
	{
		return data[x][y];
	}

	RGBA& Image::At(zmath::Vec pos)
	{
		return data[(int)pos.X][(int)pos.Y];
	}

	const RGBA& Image::At(int x, int y) const
	{
		return data[x][y];
	}

	const RGBA& Image::At(zmath::Vec pos) const
	{
		return data[(int)pos.X][(int)pos.Y];
	}

	Image& Image::Copy() const
	{
		Image img(bounds);

		LOOP_IMAGE img.data[x][y] = data[x][y];

		return img;
	}

	Image& Image::Copy(zmath::Vec min_, zmath::Vec max_) const
	{
		Vec min = Vec::Max(Vec::Min(min_, max_), Vec());
		Vec max = Vec::Min(Vec::Max(min_, max_), bounds);

		Image* img = new Image(max - min);
		for (int x = min.X; x < max.X; x++)
		{
			for (int y = min.Y; y < max.Y; y++)
			{
				img->data[x][y] = data[x - (int)min.X][y - (int)min.Y];
			}
		}

		return *img;
	}

	Image& Image::Copy(zmath::Rect rect) const
	{
		// Adjust rect so it's within the bounds of Image
		rect = rect.Floor().Intersection(Rect(bounds));

		// Return null image if there is no overlap
		if (!rect) return *new Image(0, 0);

		Image img(rect.Dimensions());
		for (int x = rect.min.X; x < rect.max.X; x++)
		{
			for (int y = rect.min.Y; y < rect.max.Y; y++)
			{
				img.At(x - rect.min.X, y - rect.min.Y) = data[x][y];
			}
		}
	}

	Image& Image::Paste(Image img, zmath::Vec at)
	{
		Vec max = Vec::Min(at.Floor() + img.bounds, bounds);
		for (int x = at.X; x < max.X; x++)
		{
			for (int y = at.Y; y < max.Y; y++)
			{
				data[x][y] = img.data[x - (int)at.X][y - (int)at.Y];
			}
		}

		return *this;
	}

	Image& Image::Paste(Image img, zmath::Rect within)
	{
		// Get the scale of the pasted image compared to the area it's being pasted into
		within.Floor();
		Vec scale = img.bounds / within.Dimensions();
		
		for (int x = within.min.X; x < within.max.X; x++)
		{
			for (int y = within.min.Y; y < within.max.Y; y++)
			{
				Vec copyFromIdx = (Vec(x, y) * scale).Floor();
				Vec copyToIdx = Vec(x, y) + within.min;
				
				At(copyToIdx) = img.At(copyFromIdx);
			}
		}
		
		return *this;
	}

	Image& Image::Negative()
	{
		LOOP_IMAGE data[x][y] = data[x][y].Negative();

		return *this;
	}

	Image& Image::RestrictPalette(const std::vector<RGBA>& palette)
	{
		assert(palette.size());

		LOOP_IMAGE
		{
			int idx_min = -1;
			double val_min = 500000; // higher than max distance between colors

			for (unsigned i = 0; i < palette.size(); i++)
			{
				double min = RGBA::Distance(data[x][y], palette[i]);

				if (min < val_min)
				{
					idx_min = i;
					val_min = min;
				}
			}

			data[x][y] = palette.at(idx_min);
		}

		return *this;
	}

	void Image::Save(std::string path, unsigned int channels = 4) const
	{
		if (channels != 3 && channels != 4) return;

		uint8* pixels = new uint8[(uint64_t)bounds.X * (uint64_t)bounds.Y * channels];

		int index = 0;
		for (int y = 0; y < bounds.Y; y++)
		{
			for (int x = 0; x < bounds.X; x++)
			{
				RGBA col = data[x][y];

				pixels[index++] = col.R;
				pixels[index++] = col.G;
				pixels[index++] = col.B;
				if (channels == 4) pixels[index++] = col.A;
			}
		}

		stbi_write_png(path.c_str(), bounds.X, bounds.Y, channels, pixels, bounds.X * channels);

		delete[] pixels;
	}



}