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
Map example_archipelago();
Map example_rivers();

int main()
{
    Map map = example_rivers();

    Tessellation3D tess(map, Vec3(0.5, 0.5, 0.5));

    tess.WriteSTL("shape.stl", false);

    return 0;
}

// Generate a heightmap that reminds me of the bark of a pine tree
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

// Generate an island-filled landscape
Map example_archipelago()
{
    NoiseConfig cfg;
    cfg.bounds = VecInt(500, 500);
    cfg.boxSize = VecInt(250, 250);
    cfg.normalize = false;
    
    Map map = Simplex(cfg);
    map.Abs().Interpolate(0, 1).BoundMin(0.25);
    map.Interpolate(15, 50);

    return map;
}

// Generate a heightmap of a river-filled landscape
Map example_rivers()
{
    NoiseConfig cfg;
    cfg.bounds = VecInt(500, 500);
    cfg.boxSize = VecInt(300, 300);
    cfg.normalize = false;
    
    Map rivers = Simplex(cfg);
    rivers.Abs().Interpolate(0, 1);

    cfg.normalize = true;
    cfg.boxSize = cfg.bounds;
    Map map = Simplex(cfg);

    map *= rivers;
    map.Interpolate(0, 1).Pow(0.55).BoundMin(0.15);
    map.Interpolate(15, 50);

    return map;
}