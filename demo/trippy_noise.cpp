#include <zarks/noise/noise2D.h>
#include <zarks/image/GIF.h>
#include <zarks/io/logdefs.h>

#include <vector>

using namespace zmath;

int main()
{
    // Worley parameters
    NoiseConfig cfg;
    cfg.seed = 2;
    cfg.octaves = 1;
    cfg.bounds = {400, 400};
    cfg.boxSize = VecInt(150, 150);
    cfg.nearest = {2, 4};
    std::vector<double> lNorms = {
        0.6, 0.625, 0.65, 0.675, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0,
        1.1, 1.2, 1.3, 1.4, 1.6, 1.8, 2.0, 2.3, 2.6, 3.0,
        4.0, 5.0, 7.0, 9.0, 12.0, 15.0, 18.0, 21.0, 25.0
    };

    // Construct GIF from frames
    GIF gif;
    for (double norm : lNorms)
    {
        LOG_INFO("lNorm = " << norm);
        cfg.lNorm = norm;
        gif.Add(Image(Worley(cfg).SlopeMap().Interpolate(0, 1)));
    }

    // Add frames in reverse
    size_t num_frames = lNorms.size();
    for (size_t i = 0; i < lNorms.size(); i++)
    {
        gif.Add(gif.At(num_frames - i - 1));
    }
    gif.Add(gif.At(0), 0);

    // Create greyscale palette
    std::vector<RGBA> palette(256);
    for (int i = 0; i < 256; i++)
    {
        palette[i] = RGBA(i);
    }

    // Save GIF
    gif.Save("trippy.gif", cfg.bounds, palette, {0.1});
}
