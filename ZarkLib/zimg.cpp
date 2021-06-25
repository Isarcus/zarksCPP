#include "pch.h"
#include "Zimg.h"
#include "zmath_internals.h"
#include "MapT.h"
#include "Gauss.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_MSC_SECURE_CRT // apparently necessary for Visual Studio
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>

#define LOOP_IMAGE for (int x = 0; x < bounds.X; x++) for (int y = 0; y < bounds.Y; y++)

using namespace zmath;

namespace zmath
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

Image::Image(zmath::VecInt bounds_in, RGBA col)
{
	isCopy = false;
	
	// Bound image to smallest possible size of (1, 1)
	bounds = VecInt::Max(bounds_in, VecInt(1, 1));

	data = alloc2d<RGBA>(bounds.X, bounds.Y, col);
}

Image::Image(const zmath::Map& m)
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

Image::Image(const zmath::Map& m, Scheme scheme)
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

Image::Image(const Image& img)
{
	(*this) = img;
}

Image::~Image()
{
	if (!isCopy) for (int x = 0; x < bounds.X; x++) delete[] data[x];
	delete[] data;
}

VecInt Image::Bounds() const
{
	return bounds;
}

Image& Image::operator=(const Image& img)
{
	free2d(data, bounds.X);
	bounds = img.bounds;
	isCopy = false;

	data = alloc2d<RGBA>(bounds.X, bounds.Y, RGBA::Black());
	LOOP_IMAGE
	{
		data[x][y] = img[x][y];
	}

	return *this;
}

bool Image::operator!() const
{
	return data == nullptr;
}

RGBA& Image::At(int x, int y)
{
	if (x < 0 || x >= bounds.X ||
		y < 0 || y >= bounds.Y)
	{
		throw std::runtime_error("Out of bounds Image access!");
	}

	return data[x][y];
}

RGBA& Image::At(zmath::VecInt pos)
{
	return At(pos.X, pos.Y);
}

const RGBA& Image::At(int x, int y) const
{
	if (x < 0 || x >= bounds.X ||
		y < 0 || y >= bounds.Y)
	{
		throw std::runtime_error("Out of bounds Image access!");
	}

	return data[x][y];
}

const RGBA& Image::At(VecInt pos) const
{
	return At(pos.X, pos.Y);
}

RGBA* const& zmath::Image::operator[](int x)
{
	return data[x];
}

const RGBA* const& zmath::Image::operator[](int x) const
{
	return data[x];
}

Image& Image::Copy() const
{
	Image img(bounds);

	LOOP_IMAGE img.data[x][y] = data[x][y];

	return img;
}

Image& Image::Copy(zmath::VecInt min_, zmath::VecInt max_) const
{
	VecInt min = VecInt::Max(VecInt::Min(min_, max_), VecInt());
	VecInt max = VecInt::Min(VecInt::Max(min_, max_), bounds);

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

Image& Image::Paste(Image img, zmath::VecInt at)
{
	VecInt max = VecInt::Min(at.Floor() + img.bounds, bounds);
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
	VecT<double> scale = (VecT<double>)img.bounds / within.Dimensions();
		
	for (int x = within.min.X; x < within.max.X; x++)
	{
		for (int y = within.min.Y; y < within.max.Y; y++)
		{
			VecInt copyFromIdx = (VecT<double>(x, y) * scale).Floor();
			VecInt copyToIdx = VecInt(x, y) + within.min;
				
			At(copyToIdx) = img.At(copyFromIdx);
		}
	}
		
	return *this;
}

Image& zmath::Image::Resize(VecInt to_bounds)
{
	Image img(to_bounds);

	Vec scale =  Vec(bounds) / Vec(img.bounds);

	for (int x = 0; x < img.bounds.X; x++)
	{
		for (int y = 0; y < img.bounds.Y; y++)
		{
			VecInt samplePos = Vec(x, y) * scale;

			img[x][y] = At(samplePos);
		}
	}


	return *this = img;
}

Image& zmath::Image::Resize(double scaleFactor)
{
	return Resize(Vec(bounds) * scaleFactor);
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

Image& Image::Fractalify(int octaves)
{
	if (octaves < 1) return *this;
	
	int gridRes = std::pow(2, octaves);
	double octInfluence = 1.0 / gridRes;

	// Create a smaller version of this image
	VecT<double> boxSize = bounds / gridRes;
	Image img(boxSize);
	for (int x = 0; x < img.bounds.X; x++)
	{
		for (int y = 0; y < img.bounds.Y; y++)
		{
			Vec coordHere = Vec(x, y) * gridRes;

			img.At(x, y) = At(coordHere);
		}
	}

	// Copy that smaller image into this one, (2^octaves)^2 times
	for (int x_box = 0; x_box < gridRes; x_box++)
	{
		for (int y_box = 0; y_box < gridRes; y_box++)
		{
			VecInt coordStart = boxSize * VecT<double>(x_box, y_box);
			VecInt coordEnd = boxSize * VecT<double>(x_box + 1, y_box + 1).Floor();
			
			for (int x = coordStart.X, x_small = 0; x < coordEnd.X; x++, x_small++)
			{
				for (int y = coordStart.Y, y_small = 0; y < coordEnd.Y; y++, y_small++)
				{
					RGBA colSub = img.At(VecInt::Min(VecInt(x_small, y_small), img.bounds - VecInt(1, 1)));
					RGBA colHere = At(x, y);
					At(x, y) = RGBA::Interpolate(colHere, colSub, 0.5*octInfluence);
				}
			}
		}
	}

	return Fractalify(octaves - 1);
}

Image& zmath::Image::Droppify(std::array<Vec, 3> origins, std::array<double, 3> periods)
{
	LOOP_IMAGE
	{
		Vec pos(x, y);

		// Calculate weights
		std::array<double, 3> weights;
		for (int i = 0; i < 3; i++)
		{
			weights[i] = std::sin(2.0 * ZM_PI * origins[i].DistForm(pos) / periods[i]);
			weights[i] = (1.0 + weights[i]) / 2.0;
		}

		// Adjust intensity of weights
		double intensity = DistForm(weights);
		weights /= intensity;

		// Apply weights
		RGBA& pix = data[x][y];
		for (int i = 0; i < 3; i++)
		{
			pix[i] = (double)pix[i] * weights[i];
		}
	}

	return *this;
}

// Blurs an image Gaussianly!
Image& zmath::Image::BlurGaussian(double sigma)
{
	int radius = sigma * 2;
	GaussField gauss(sigma, 1.0, Vec());
	const auto& points = gauss.Points(radius);
	
	Image imgNew(bounds);

	LOOP_IMAGE
	{
		VecInt imgPos(x,y);

		double influence = 0;
		std::array<double, 4> rgba;
		for (const auto& point : points)
		{
			VecInt pointPos = point.first + imgPos;

			// If contains coord
			if (pointPos >= VecInt() && pointPos < bounds)
			{
				influence += point.second;
				const RGBA& addCol = At(pointPos);
				rgba[0] += point.second * addCol.R;
				rgba[1] += point.second * addCol.G;
				rgba[2] += point.second * addCol.B;
				rgba[3] += point.second * addCol.A;
			}
		}

		rgba /= influence;

		imgNew[x][y] = RGBA((uint8)std::min(255.0, std::round(rgba[0])),
							(uint8)std::min(255.0, std::round(rgba[1])),
							(uint8)std::min(255.0, std::round(rgba[2])),
							(uint8)std::min(255.0, std::round(rgba[3])));
	}

	return *this = imgNew;
}

// Warps an image Gaussianly-ish!
Image& zmath::Image::WarpGaussian(const Map& map, double sigma)
{
	MapT<std::pair<Vec, double>> transforms(bounds);

	int radius = sigma * 2.0;
	GaussField gauss(sigma, 1.0, Vec());
	const auto points = gauss.Points(radius);

	std::cout << "Performing Gaussian Warp on " << bounds << " Image:\n"
		      << " -> sigma:  " << sigma << "\n"
		      << " -> points: " << points.size() << "\n";

	LOOP_IMAGE
	{
		VecInt imgPos(x, y);
		
		// Loop through gauss field points
		for (const auto& point : points)
		{
			// if map contains point and point's influence is larger than transform's current influence
			VecInt pointPos = point.first + imgPos;
			if (map.ContainsCoord(pointPos) && point.second*map[x][y] > transforms.At(pointPos).second)
			{
				transforms.At(pointPos) = { imgPos, point.second*map[x][y] };
				//std::cout << "set " << point.first << " to " << imgPos << " " << point.second << "\n";
			}
		}
	}

	std::cout << " -> Applying transforms . . .";

	Image imgNew(bounds);
	LOOP_IMAGE
	{
		VecInt samplePos = transforms[x][y].first;
		if (!map.ContainsCoord(samplePos))
		{
			samplePos = Vec::Max(VecInt(0, 0), Vec::Min(bounds - 1, samplePos));
		}
		imgNew[x][y] = At(samplePos);
		//std::cout << "setting " << Vec(x, y) << " to " << samplePos << "\n";
	}

	*this = imgNew;

	std::cout << " All done!\n";

	return *this;
}

void Image::Save(std::string path, unsigned int channels) const
{
	if (channels != 3 && channels != 4)
	{
		throw std::runtime_error("Image: I only know how to save 3- and 4-channel images!");
	}

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

} // namespace zimg