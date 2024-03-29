#include <zarks/image/Image.h>
#include <zarks/internal/zmath_internals.h>
#include <zarks/math/MapT.h>
#include <zarks/math/GaussField.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STBI_MSC_SECURE_CRT // apparently necessary for Visual Studio
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <iostream>
#include <fstream>

#define LOOP_IMAGE for (int x = 0; x < bounds.X; x++) for (int y = 0; y < bounds.Y; y++)
#define LOOP_IMAGE_HORIZONTAL for (int y = 0; y < bounds.Y; y++) for (int x = 0; x < bounds.X; x++)

using namespace zmath;

namespace zmath
{

Image::Image(int width, int height, RGBA col)
	: Sampleable2D(VecInt::Max(VecInt(width, height), VecInt(1, 1)), col)
{}

Image::Image(zmath::VecInt bounds_in, RGBA col)
	: Image(bounds_in.X, bounds_in.Y, col)
{}

Image::Image(const zmath::Map& m)
	: Image(m.Bounds())
{
	LOOP_IMAGE
	{
		uint8 shade = 255.999 * m[x][y];
		data[x][y] = RGBA(shade, shade, shade);
	}
}

Image::Image(const zmath::Map& m, Scheme scheme)
	: Image(m.Bounds())
{
	// Create an accurate thresholds array
	std::vector<double> thresholds(scheme.colors.size());
	thresholds.back() = 1;
	for (unsigned i = 1; i < scheme.colors.size() - 1; i++)
	{
		thresholds[i] = scheme.thresholds[i - 1];
	}

	// Loop and assign colors
	LOOP_IMAGE
	{
		double val = m[x][y];

		int idxUpper = 0;
		for (unsigned i = 0; i < scheme.colors.size(); i++)
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
		return;
	}

	bounds = Vec(width, height);

	// Allocate data
	data = alloc2d<RGBA>(width, height);

	int stbIdx = 0;
	LOOP_IMAGE_HORIZONTAL
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
	}

	free(stbImg);

	std::cout << "Successfully loaded image at " << path << "\n"
			    << " -> Channels:   " << channels << "\n"
			    << " -> Dimensions: " << width << "x" << height << "\n";
}

Image::Image(const Image& img)
{
	(*this) = img;
}

Image::Image(Image&& img)
{
	*this = std::move(img);
}

Image::Image()
	: Image(1, 1)
{}

Image::~Image()
{
	free2d(data, bounds.X);
}

VecInt Image::Bounds() const
{
	return bounds;
}

Image& Image::operator=(const Image& img)
{
	if (bounds != img.bounds)
	{
		free2d(data, bounds.X);
		bounds = img.bounds;

		data = alloc2d<RGBA>(bounds.X, bounds.Y, RGBA::Black());
	}

	LOOP_IMAGE data[x][y] = img[x][y];
	return *this;
}

Image& Image::operator=(Image&& img)
{
	if (this != &img)
	{
		free2d(data, bounds.X);
		bounds = img.bounds;
		data = img.data;

		img.data = nullptr;
		img.bounds = VecInt(0, 0);
	}

	return *this;
}

std::unique_ptr<Image> Image::Copy(zmath::VecInt min_, zmath::VecInt max_) const
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

	return std::unique_ptr<Image>(img);
}

Image& Image::Paste(const Image& img, zmath::VecInt at)
{
	for (int x = 0; x < img.bounds.X; x++)
	{
		for (int y = 0; y < img.bounds.Y; y++)
		{
			VecInt thisCoord = at + VecInt(x, y);
			if (ContainsCoord(thisCoord))
			{
				data[thisCoord.X][thisCoord.Y] = img.data[x][y];
			}
		}
	}

	return *this;
}

Image& zmath::Image::Tile(const Image& tile, VecInt tileSize, VecInt offset)
{
	Image tileAdj = tile;
	tileAdj.Resize(tileSize);
	VecInt tileBounds = tileAdj.Bounds();

	// Bound offset within range of ( -tileBounds, {0, 0} ]
	VecInt offsetAdj = offset.Mod(tileBounds);
	if (offsetAdj != VecInt(0, 0)) offsetAdj -= tileBounds;

	for (int tileX = 0; offsetAdj.X + tileX * tileBounds.X < bounds.X; tileX++)
	{
		for (int tileY = 0; offsetAdj.Y + tileY * tileBounds.Y < bounds.Y; tileY++)
		{
			VecInt thisPaste = offsetAdj + VecInt(tileX, tileY) * tileBounds;

			Paste(tileAdj, thisPaste);
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

Image& zmath::Image::Clear(RGBA col)
{
	LOOP_IMAGE
	{
		data[x][y] = col;
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
			weights[i] = std::sin(2.0 * PI * origins[i].DistForm(pos) / periods[i]);
			weights[i] = (1.0 + weights[i]) / 2.0;
		}

		// Adjust intensity of weights
		double intensity = DistForm<double, 3>(weights);
		operator/=<double, 3>(weights, intensity);

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
Image& zmath::Image::BlurGaussian(double sigma, bool blurAlpha)
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
			const VecInt pointPos = point.first + imgPos;

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

		operator/=<double, 4>(rgba, influence);

		imgNew[x][y] = RGBA((uint8)std::min(255.0, std::round(rgba[0])),
							(uint8)std::min(255.0, std::round(rgba[1])),
							(uint8)std::min(255.0, std::round(rgba[2])),
							(blurAlpha) ? At(imgPos).A : (uint8)std::min(255.0, std::round(rgba[3])));
	}

	return *this = imgNew;
}

// Warps an image Gaussianly-ish!
Image& zmath::Image::PixelateGaussian(const Map& map, double sigma)
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

Image& zmath::Image::EnhanceContrast(double sigma)
{
	Image blurred(*this);
	blurred.BlurGaussian(sigma);

	LOOP_IMAGE
	{
		double dR = ((int)data[x][y].R - (int)blurred[x][y].R) / 255.0;
		double dG = ((int)data[x][y].G - (int)blurred[x][y].G) / 255.0;
		double dB = ((int)data[x][y].B - (int)blurred[x][y].B) / 255.0;

		if (dR < 0) data[x][y].R *= (1.0 + dR);
		else data[x][y].R += dR * (255.0 - data[x][y].R);
		if (dG < 0) data[x][y].G *= (1.0 + dG);
		else data[x][y].G += dG * (255.0 - data[x][y].G);
		if (dB < 0) data[x][y].B *= (1.0 + dB);
		else data[x][y].B += dB * (255.0 - data[x][y].B);
	}

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

void zmath::Image::SaveMNIST(std::string path_images, std::string path_labels, int columns, int emptyBorderSize) const
{
	constexpr int MNIST_IMG_WIDTH = 28;
	constexpr int MNIST_IMG_HEIGHT = 28;
	constexpr int MNIST_IMG_SIZE = MNIST_IMG_WIDTH * MNIST_IMG_HEIGHT;
	static const VecInt MNIST_BOUNDS(MNIST_IMG_WIDTH, MNIST_IMG_HEIGHT);

	VecInt minBounds = MNIST_BOUNDS * VecInt(columns, 10);
	if (bounds.X < minBounds.X || bounds.Y < minBounds.Y)
	{
		std::cout << "Sorry, this image is too small to convert to MNIST data! " << bounds << "\n";
		return;
	}
	Image copy(*this);
	//copy.BlurGaussian(1.0, false);
	copy.Resize(minBounds);

	// Open files, check if ok, and write empty headers
	std::ofstream fout_images(path_images, std::ios_base::binary);
	std::ofstream fout_labels(path_labels, std::ios_base::binary);
	if (fout_images.fail())
	{
		std::cout << "[ERROR] Couldn't create file at " << path_images << "\n";
		return;
	}
	if (fout_labels.fail())
	{
		std::cout << "[ERROR] Couldn't create file at " << path_labels << "\n";
		return;
	}
	char header_empty[16];
	fout_images.write(header_empty, 16);
	fout_labels.write(header_empty, 8);

	// Write data to files
	for (int col = 0; col < columns; col++)
	{
		for (int row = 0; row < 10; row++)
		{
			// Get map brightness
			Map map(MNIST_IMG_WIDTH, MNIST_IMG_HEIGHT);
			VecInt startPos = MNIST_BOUNDS * VecInt(col, row);
			for (int x = 0; x < MNIST_IMG_WIDTH; x++)
			{
				for (int y = 0; y < MNIST_IMG_HEIGHT; y++)
				{
					VecInt mapPos(x, y);
					VecInt samplePos = startPos + mapPos;

					double brightness = copy.At(samplePos).Brightness();
					map.Set(mapPos, brightness);
				}
			}

			// Normalize 
			map.Interpolate(1, 0);
			map.FillBorder(emptyBorderSize, 0);
			map.Interpolate(0, 1);
			//Image im_ex(map);
			//im_ex.Save("test/" + std::to_string(row) + "_" + std::to_string(col) + ".png");

			// Write map
			unsigned char bytes[MNIST_IMG_SIZE];
			int idx_byte = 0;
			for (int y = 0; y < MNIST_IMG_HEIGHT; y++)
			{
				for (int x = 0; x < MNIST_IMG_WIDTH; x++)
				{
					bytes[idx_byte++] = std::round(map[x][y] * 255.0);
				}
			}

			fout_images.write((char*)bytes, MNIST_IMG_SIZE);
			char row_char = row;
			fout_labels.write(&row_char, 1);
		}
	}
	fout_images.close();
	fout_labels.close();

	std::cout << "Finished writing data for " << columns * 10 << " MNIST images\n";
}

} // namespace zimg
