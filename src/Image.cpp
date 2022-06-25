#include <zarks/image/Image.h>
#include <zarks/math/GaussField.h>
#include <zarks/internal/zmath_internals.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STBI_MSC_SECURE_CRT // apparently necessary for Visual Studio
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#pragma GCC diagnostic pop

#include <iostream>
#include <fstream>
#include <algorithm>

#define LOOP_IMAGE for (int x = 0; x < bounds.X; x++) for (int y = 0; y < bounds.Y; y++)
#define LOOP_IMAGE_HORIZONTAL for (int y = 0; y < bounds.Y; y++) for (int x = 0; x < bounds.X; x++)

using namespace zmath;

namespace zmath
{

Image::Image()
    : Mat2D()
{}

Image::Image(int width, int height, RGBA col)
    : Mat2D(VecInt::Max(VecInt(width, height), VecInt(1, 1)), col)
{}

Image::Image(VecInt bounds_in, RGBA col)
    : Image(bounds_in.X, bounds_in.Y, col)
{}

Image::Image(const Mat2D<double>& mat)
    : Image(mat.Bounds())
{
    ApplySample(mat, [](double d){ return RGBA(255.999 * d); });
}

Image::Image(const Mat2D<double>& mat, const std::vector<RGBA>& colors, const std::vector<double>& thresholds)
    : Image(mat.Bounds())
{
    const size_t numColors = colors.size();

    std::vector<double> thresh_final;
    if (numColors == 0)
    {
        throw std::runtime_error("Must provide at least one color");
    }
    else if (numColors == 1)
    {
        Clear(colors[0]);
    }
    else if (thresholds.empty())
    {
        // Use evenly spaced thresholds if none provided
        std::pair<double, double> minmax = mat.GetMinMax();
        double range = minmax.second - minmax.first;
        thresh_final.reserve(2*numColors - 1);
        for (size_t i = 0; i < numColors*2 - 1; i++)
        {
            thresh_final.push_back(minmax.first + (i*range) / (numColors*2 - 2));
        }
    }
    else if (thresholds.size() == numColors*2 - 1)
    {
        thresh_final = thresholds;
        std::sort(thresh_final.begin(), thresh_final.end());
    }
    else
    {
        throw std::runtime_error("Number of thresholds must be two times the number of colors, minus one");
    }

    // Create final color vector
    std::vector<RGBA> use_colors;
    use_colors.reserve(2*numColors - 1);
    use_colors.push_back(colors.front());
    for (size_t i = 1; i < numColors; i++)
    {
        use_colors.push_back(RGBA::Interpolate(
            colors[i - 1],
            colors[i],
            0.5
        ));
        use_colors.push_back(colors[i]);
    }

    const auto iter_begin = thresh_final.begin();
    const auto iter_end = thresh_final.end();
    ApplySample(mat, [&](double d)
    {
        auto iter = std::lower_bound(iter_begin, iter_end, d);
        if (iter == iter_begin)
            return use_colors.front();
        else if (iter == iter_end)
            return use_colors.back();

        // Interpolate between neighboring colors
        size_t diff = iter - iter_begin;
        double thresh_upper = *iter;
        double thresh_lower = *(iter - 1);
        return RGBA::Interpolate(
            use_colors[diff - 1],
            use_colors[diff],
            (d - thresh_lower) / (thresh_upper - thresh_lower)
        );
    });
}

Image::Image(std::string path)
{
    int width, height, channels = -1;
    uint8_t* stbImg = stbi_load(path.c_str(), &width, &height, &channels, 0);

    // Abort if it fails to load
    if (!stbImg || channels == -1)
    {
        throw std::runtime_error("Could not load image at " + path);
    }

    bounds = VecInt(width, height);
    data = new RGBA[bounds.Area()];
    int stbIdx = 0;
    LOOP_IMAGE_HORIZONTAL
    {
        switch (channels)
        {
        case 1: // Grayscale
            at_itl(x, y) = RGBA(stbImg[stbIdx]);
            stbIdx++;
            break;

        case 2: // Grayscale with alpha
            at_itl(x, y) = RGBA(
                stbImg[stbIdx],
                stbImg[stbIdx + 1]);
            stbIdx += 2;
            break;

        case 3: // RGB
            at_itl(x, y) = RGBA(
                stbImg[stbIdx],
                stbImg[stbIdx + 1],
                stbImg[stbIdx + 2]);
            stbIdx += 3;
            break;

        case 4: // RGBA
            at_itl(x, y) = RGBA(
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
    : Mat2D(img)
{}

Image::Image(Image&& img)
    : Mat2D(std::move(img))
{}

Image& Image::operator=(const Image& img)
{
    Mat2D::operator=(img);

    return *this;
}

Image& Image::operator=(Image&& img)
{
    Mat2D::operator=(std::move(img));

    return *this;
}

Image& Image::Tile(const Image& tile, VecInt tileSize, VecInt offset)
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

Image& Image::Resize(VecInt toBounds)
{
    Image img(toBounds);

    Vec scale = Vec(bounds) / Vec(img.bounds);

    for (int x = 0; x < img.bounds.X; x++)
    {
        for (int y = 0; y < img.bounds.Y; y++)
        {
            VecInt samplePos = Vec(x, y) * scale;

            img(x, y) = at_itl(samplePos);
        }
    }

    return *this = img;
}

Image& Image::Resize(double scaleFactor)
{
    return Resize(Vec(bounds) * scaleFactor);
}

Image& Image::Negative()
{
    Apply([](RGBA c){ return c.Negative(); });

    return *this;
}

Image& Image::RestrictPalette(const std::vector<RGBA>& palette)
{
    assert(palette.size());

    Apply([&](RGBA col){
        int idx_min = -1;
        double val_min = 500000; // higher than max distance between colors

        for (unsigned i = 0; i < palette.size(); i++)
        {
            double min = RGBA::Distance(col, palette[i]);

            if (min < val_min)
            {
                idx_min = i;
                val_min = min;
            }
        }

        return palette.at(idx_min);
    });

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

Image& Image::Droppify(const std::array<Vec, 3>& origins, const std::array<double, 3>& periods)
{
    Apply([=](RGBA pix, int x, int y){
        // Calculate weights
        std::array<double, 3> weights;
        for (int i = 0; i < 3; i++)
        {
            weights[i] = std::sin(2.0 * PI * origins[i].DistForm(Vec(x, y)) / periods[i]);
            weights[i] = (1.0 + weights[i]) / 2.0;
        }

        // Adjust intensity of weights
        double intensity = distForm<double, 3>(weights);
        for (auto& w : weights) w /= intensity;

        // Apply weights
        for (int i = 0; i < 3; i++)
        {
            pix[i] = (double)pix[i] * weights[i];
        }

        return pix;
    });

    return *this;
}

// Blurs an image Gaussianly!
Image& Image::BlurGaussian(double sigma, bool blurAlpha)
{
    int radius = sigma * 2;
    GaussField gauss(sigma, 1.0, Vec());
    const auto& points = gauss.Points(radius);
    
    Image imgNew(bounds);

    LOOP_IMAGE
    {
        VecInt imgPos(x,y);

        double influence = 0;
        std::array<double, 4> rgba{};
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

        for (auto& c : rgba) c /= influence;

        imgNew(x, y) = RGBA((uint8_t)std::min(255.0, std::round(rgba[0])),
                            (uint8_t)std::min(255.0, std::round(rgba[1])),
                            (uint8_t)std::min(255.0, std::round(rgba[2])),
                            (blurAlpha) ? At(imgPos).A : (uint8_t)std::min(255.0, std::round(rgba[3])));
    }

    return *this = imgNew;
}

Image& Image::PixelateGaussian(const Map& map, double sigma)
{
    Mat2D<std::pair<Vec, double>> transforms(bounds);

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
            if (map.ContainsCoord(pointPos) && point.second*map(x, y) > transforms.At(pointPos).second)
            {
                transforms.At(pointPos) = { imgPos, point.second*map(x, y) };
            }
        }
    }

    std::cout << " -> Applying transforms . . .";

    Image imgNew(bounds);
    LOOP_IMAGE
    {
        VecInt samplePos = transforms(x, y).first;
        if (!map.ContainsCoord(samplePos))
        {
            samplePos = Vec::Max(VecInt(0, 0), Vec::Min(bounds - 1, samplePos));
        }
        imgNew(x, y) = At(samplePos);
    }

    *this = imgNew;

    std::cout << " All done!\n";

    return *this;
}

Image& Image::WarpGaussian(const Map& map, double sigma, double amplitude)
{
    // Scaling and sampling constants
    const VecInt mapBounds = map.Bounds();
    const Vec scaleVec = Vec(mapBounds) / Vec(bounds);
    const Vec scaleSigma = scaleVec * sigma;
    const GaussField gauss(scaleSigma, amplitude);

    // Generate warp map
    Mat2D<std::pair<Vec, int>> result(bounds, {Vec(), 0});
    for (int x = 0; x < bounds.X; x++)
    {
        for (int y = 0; y < bounds.Y; y++)
        {
            const Vec mapPoint = Vec(x, y) * scaleVec;
            // Determine influence for this point
            for (int dx = -scaleSigma.X*2; dx <= scaleSigma.X*2; dx++)
            {
                for (int dy = -scaleSigma.Y*2; dy <= scaleSigma.Y*2; dy++)
                {
                    // This represents the point currently being sampled
                    VecInt offsetVec(dx, dy);
                    VecInt resultPoint = ((mapPoint + offsetVec) / scaleVec).Round();

                    // Skip any out of bounds points
                    if (resultPoint.Min() >= 0 && resultPoint.X < bounds.X && resultPoint.Y < bounds.Y)
                    {
                        Vec weightVec = Vec(offsetVec) * gauss(dx, dy) * map.Sample(mapPoint);
                        auto& elem = result.At(resultPoint);
                        elem.first += weightVec;
                        elem.second++;
                    }
                }
            }
        }
    }

    // Reduce result map
    for (int x = 0; x < bounds.X; x++)
    {
        for (int y = 0; y < bounds.Y; y++)
        {
            auto& pair = result(x, y);
            pair.first /= pair.second;
        }
    }

    // Warp current image
    Image warped(bounds);
    for (int x = 0; x < bounds.X; x++)
    {
        for (int y = 0; y < bounds.Y; y++)
        {
            Vec samplePoint = Vec(x, y) + result(x, y).first / scaleVec;
            samplePoint = samplePoint.Bound(VecInt(), bounds - 1);
            warped(x, y) = Sample(samplePoint);
        }
    }

    return *this = std::move(warped);
}

Image& Image::EnhanceContrast(double sigma)
{
    Image blurred(*this);
    blurred.BlurGaussian(sigma);

    LOOP_IMAGE
    {
        RGBA& thisPixel = at_itl(x, y);
        const RGBA blurPixel = blurred.at_itl(x, y);
        double dR = ((int)thisPixel.R - (int)blurPixel.R) / 255.0;
        double dG = ((int)thisPixel.G - (int)blurPixel.G) / 255.0;
        double dB = ((int)thisPixel.B - (int)blurPixel.B) / 255.0;

        if (dR < 0) thisPixel.R *= (1.0 + dR);
        else thisPixel.R += dR * (255.0 - thisPixel.R);
        if (dG < 0) thisPixel.G *= (1.0 + dG);
        else thisPixel.G += dG * (255.0 - thisPixel.G);
        if (dB < 0) thisPixel.B *= (1.0 + dB);
        else thisPixel.B += dB * (255.0 - thisPixel.B);
    }

    return *this;
}

static inline uint8_t* rgba_write_1(uint8_t* ptr, RGBA col)
{
    ptr[0] = col.Brightness() * 255.999;
    return ptr + 1;
}

static inline uint8_t* rgba_write_2(uint8_t* ptr, RGBA col)
{
    ptr[0] = col.Brightness() * 255.999;
    ptr[1] = col.A;
    return ptr + 2;
}

static inline uint8_t* rgba_write_3(uint8_t* ptr, RGBA col)
{
    ptr[0] = col.R;
    ptr[1] = col.G;
    ptr[2] = col.B;
    return ptr + 3;
}

static inline uint8_t* rgba_write_4(uint8_t* ptr, RGBA col)
{
    ptr[0] = col.R;
    ptr[1] = col.G;
    ptr[2] = col.B;
    ptr[3] = col.A;
    return ptr + 4;
}

std::vector<uint8_t> Image::EncodeRaw(int channels) const
{
    if (channels < 1 || channels > 4)
    {
        throw std::runtime_error("Invalid number of channels: " + std::to_string(channels));
    }
    
    static uint8_t* (*const funcs[4])(uint8_t*, RGBA){
        rgba_write_1,
        rgba_write_2,
        rgba_write_3,
        rgba_write_4
    };

    size_t length = bounds.Area();
    std::vector<uint8_t> arr(length * channels);
    auto func = funcs[channels - 1];

    uint8_t* ptr = arr.data();
    LOOP_IMAGE_HORIZONTAL
    {
        ptr = func(ptr, at_itl(x, y));
    }

    return arr;
}

void Image::Save(std::string path, Format format, int channels) const
{
    static constexpr int MAX_JPG_QUALITY = 100;

    std::vector<uint8_t> arr = EncodeRaw(channels);

    switch (format)
    {
    case Format::PNG:
        stbi_write_png(path.c_str(), bounds.X, bounds.Y, channels, arr.data(), bounds.X * channels);
        break;

    case Format::JPG:
        stbi_write_jpg(path.c_str(), bounds.X, bounds.Y, channels, arr.data(), MAX_JPG_QUALITY);
        break;

    case Format::BMP:
        stbi_write_bmp(path.c_str(), bounds.X, bounds.Y, channels, arr.data());
        break;

    default:
        throw std::runtime_error("Unrecognized image format: " + std::to_string((int)format));
    }
}

void Image::SaveMNIST(std::string path_images, std::string path_labels, int columns, int emptyBorderSize) const
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
    char header_empty[16]{};
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

            // Write map
            unsigned char bytes[MNIST_IMG_SIZE];
            int idx_byte = 0;
            for (int y = 0; y < MNIST_IMG_HEIGHT; y++)
            {
                for (int x = 0; x < MNIST_IMG_WIDTH; x++)
                {
                    bytes[idx_byte++] = std::round(map(x, y) * 255.0);
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

} // namespace zmath
