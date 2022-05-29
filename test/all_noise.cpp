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

#include <chrono>
#include <filesystem>

using namespace zmath;

void simplex();
void perlin();
void worley();
void time_threads();

int main()
{
    ::simplex();
    perlin();
    worley();
    time_threads();

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

void time_threads()
{
    Map map, mapThreaded;
    NoiseConfig cfg;
    cfg.bounds = VecInt(2000, 2000);
    cfg.boxSize = cfg.bounds / 2;
    cfg.octaves = 8;
    cfg.seed = std::chrono::system_clock::now().time_since_epoch().count();

    std::vector<int> threads{ 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 64 };

    std::string dir("timed_noise");
    std::filesystem::create_directory(dir);
    
    for (int t : threads)
    {
        cfg.numThreads = t;

        decltype(std::chrono::system_clock::now()) start, end;
        start = std::chrono::system_clock::now();
        map = Simplex(cfg);
        end = std::chrono::system_clock::now();
        std::cout.precision(3);
        std::cout << t << " threads: " << (end - start).count() / 1.0e9 << " sec \n";

        Image(map).Save(dir + "/simplex_" + std::to_string(t) + "t.png");
    }
}
