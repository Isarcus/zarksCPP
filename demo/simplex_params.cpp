#include <zarks/noise/noise2D.h>
#include <zarks/image/Image.h>

using namespace zmath;

int main()
{
    // Create noise configuration struct
    NoiseConfig cfg;

    // Make the noise deterministic
    cfg.seed = 1;

    // Adjust parameters
    cfg.octaves = 1;
    cfg.r = 0.65;
    cfg.rMinus = 0.65;
    cfg.boxSize = VecInt(300, 300);

    // Generate a Simplex slope map and normalize it
    Map map = Simplex(cfg).SlopeMap();
    map.Interpolate(0, 1);

    // Save creation as an image
    Image(map).Save("test.png");
}
