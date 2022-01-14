#include <zarks/image/Image.h>
#include <zarks/math/KMeans.h>

#include <cmath>
#include <iomanip>

using namespace zmath;

struct RGBACounter
{
    unsigned R, G, B;
    RGBACounter() : R(0), G(0), B(0) {}

    void operator+=(zmath::RGBA c)
    {
        R += c.R;
        G += c.G;
        B += c.B;
    }

    RGBA operator/(size_t s)
    {
        return RGBA(
            R / s,
            G / s,
            B / s
        );
    }
};

struct RGBADist
{
    double operator()(RGBA c1, RGBA c2)
    {
        return RGBA::Distance(c1, c2);
    }
};

int main()
{
    // Create image to run algorithm on
    Image img(512, 512);
    for (int x = 0; x < 512; x++)
    {
        for (int y = 0; y < 512; y++)
        {
            img(x, y) = RGBA(
                x / 2,
                0,
                y / 2
            );
        }
    }
    img.Save("RB.png");

    // Run algorithm on image

    std::vector<RGBA> means
    {
        RGBA(0, 0, 0),
        RGBA(255, 0, 0),
        RGBA(0, 0, 255)
    };

    auto meanLocs = ComputeKMeans<RGBA, RGBADist, RGBACounter> (means, img);

    VecInt bounds = meanLocs.Bounds();
    for (int x = 0; x < bounds.X; x++)
    {
        for (int y = 0; y < bounds.Y; y++)
        {
            img(x, y) = means[meanLocs(x, y)];
        }
    }

    img.Save("RB_kmeans.png");
}
