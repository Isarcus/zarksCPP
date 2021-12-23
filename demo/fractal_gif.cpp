#include <zarks/noise/fractals.h>
#include <zarks/image/GIF.h>

#include <vector>

using namespace zmath;

int main()
{
    // Set up fractal parameters
    VecInt bounds(400, 400);
    FractalConfig cfg;
    cfg.reps = 60;

    // Construct GIF frame by frame
    GIF gif;
    for (int i = 0; i < 100; i++)
    {
        // Gradually change the 'c' parameter for different Julia sets
        double theta = PIX2 * i / 100.0;
        cfg.c = std::complex<double>(
            std::cos(theta) * 0.7,
            std::sin(theta) * 0.7
        );

        // Generate this fractal
        Map fractal = Julia(bounds, cfg);

        // Make it pretty
        fractal.Apply([](double v){
            return (v) ? std::min(1.0, 0.1 + (std::sin(v/5.0) + 1.0) / 2.0) : 0;
        });

        // Add fractal to the GIF
        gif.Add(fractal);
    }

    // Create greyscale palette
    std::vector<RGBA> palette(256);
    for (int i = 0; i < 256; i++)
    {
        palette[i] = RGBA(i);
    }

    // Save GIF with greyscale palette
    gif.Save("fractal.gif", bounds, palette);
}
