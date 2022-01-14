#include <zarks/noise/noise2D.h>
#include <zarks/image/Image.h>

using namespace zmath;

int main()
{
    // Generate noise map
    VecInt bounds(300, 300);
    NoiseConfig cfg;
    cfg.seed = 2;
    cfg.octaves = 6;
    cfg.boxSize = bounds / 2;
    cfg.bounds = bounds;
    Map map = Simplex(cfg);

    // Create checkerboard image
    Image image(bounds);
    for (int x = 0; x < bounds.X; x++)
    {
        for (int y = 0; y < bounds.Y; y++)
        {
            image(x, y) = ((x/20 + y/20) % 2) ? RGBA::Black() : RGBA::White();
        }
    }

    // Warp and save image
    double sigma = 10.0;
    double amplitude = 40.0;
    image.WarpGaussian(map, sigma, amplitude);
    image.Save("warped.png");
    
    // Save noise map for comparison
    Image(map).Save("noise.png");
}
