/**
 * Author: Isarcus
 * 
 * Created: September 3, 2021
 * 
 * Description:
 *  Generate basic Simplex, Perlin, and Worley heightmaps
 *  and save them as images to easily test if the noise
 *  algorithms are working as intended.
 * 
 */

#include <zarks/noise/noise2D.h>
#include <zarks/noise/Noiser.h>
#include <zarks/image/Image.h>

using namespace zmath;

void simplex();
void perlin();
void worley();

int main()
{
    ::simplex();
    ::perlin();
    ::worley();

    return 0;
}

void simplex()
{
    Map map = Simplex(NoiseConfig());
    Image image(map);
    image.Save("simplex.png");

    Noiser noiser(&SimplexPoint, 1);
    map = noiser(VecInt(1000, 1000), 8);
    image = Image(map);
    image.Save("simplex_noiser.png");

    NoiseConfig cfg;
    cfg.boxSize = cfg.bounds / 64;
    cfg.r = 0.4;
    cfg.octaves = 1;
    map = Simplex(cfg);
    image = Image(map);
    image.Save("simplex_dots.png");
}

void perlin()
{
    Map map = Perlin(NoiseConfig());
    Image image(map);
    image.Save("perlin.png");

    Noiser noiser(&PerlinPoint, 1);
    map = noiser(VecInt(1000, 1000), 8);
    image = Image(map);
    image.Save("perlin_noiser.png");
}

void worley()
{
    NoiseConfig cfg;
    cfg.octaves = 2;
    Map map = Worley(cfg);
    Image image(map);
    image.Save("worley.png");
}
