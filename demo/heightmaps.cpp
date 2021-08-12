/**
 * Author: Isarcus
 * 
 * Created: August 12, 2021
 * 
 * Description:
 *  TODO
 * 
 */

#include <zarks/math/Map.h>
#include <zarks/noise/noise2D.h>
#include <zarks/math/3D/Shape3D.h>

using namespace zmath;

Map example_pine_bark();

int main()
{
    Map map = example_pine_bark();

    Tessellation3D tess(map, Vec3(0.5, 0.5, 0.5));

    tess.WriteSTL("shape.stl", false);

    return 0;
}

Map example_pine_bark()
{
    NoiseConfig cfg;
    cfg.bounds = VecInt(500, 500);
    cfg.boxSize = VecInt(100, 400);
    cfg.normalize = false;
    cfg.octaves = 6;

    Map map_bark = Simplex(cfg);
    map_bark.Abs();
    map_bark.Interpolate(1, 0).BoundMax(0.83);
    map_bark.Interpolate(0, 1).Pow(1.5);
    map_bark.Interpolate(20, 40);
    
    cfg.NewSeed();
    cfg.octaves = 8;
    cfg.boxSize = cfg.bounds;
    Map map_texture = Simplex(cfg);
    map_texture.Interpolate(0, 15);

    map_bark += map_texture;

    return map_bark;
}