#include <zarks/noise/noise2D.h>
#include <zarks/image/Image.h>

using namespace zmath;

int main()
{
    // Set noise parameters
    NoiseConfig cfg;
    cfg.bounds = VecInt(800, 800);
    cfg.octaves = 1;
    cfg.normalize = false;
    cfg.lNorm = 1.1;

    Map total(cfg.bounds);

    // Manually generate and summate each octave
    for (int i = 0; i < 8; i++)
    {
        double octFactor = std::pow(2, i);
        cfg.boxSize = Vec(cfg.bounds) / octFactor;
        cfg.seed = i + 1;
        Map noise = Simplex(cfg);
        
        // Because the NoiseConfig::normalize parameter is set to false,
        // noise maps generated by the Simplex() function will fall in some
        // unspecified range with an average of about zero. Taking the
        // absolute value of these noise maps will create an interesting
        // line pattern. Doing this for each individual octave before it is
        // added to the "total" map results in fractal-like branching!
        noise.Abs();
        noise /= octFactor;
        total += noise;
    }

    total.Interpolate(-1, 1).Abs();
    Image(total).Save("ridges.png");
}
