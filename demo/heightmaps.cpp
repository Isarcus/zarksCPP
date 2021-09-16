/**
 * Author: Isarcus
 * 
 * Created: August 12, 2021
 * 
 * Description:
 *  This file contains some simple examples of how to
 *  generate interesting heightmaps using Simplex (or
 *  Perlin) Noise, and save those heightmaps to an
 *  STL file.
 * 
 */

#include <zarks/noise/noise2D.h>
#include <zarks/math/Map.h>
#include <zarks/math/3D/Shape3D.h>

using namespace zmath;

// Generate a heightmap filled with winding rivers
Map example_rivers();
// Generate a heightmap that reminds me of the bark of a pine tree
Map example_pine_bark();
// Generate an island-filled landscape
Map example_archipelago();

int main()
{
    // Generate a heightmap using one of the examples
    Map map = example_pine_bark();

    // Create a 3D triangle tessellation from the heightmap
    Tessellation3D tess(map, Vec3(0.5, 0.5, 0.5));

    // Write the tessellation to an STL file
    tess.WriteSTL("shape.stl", false);

    // Note: If your computer does not have software capable of
    // opening STL files, I recommend uploading the generated file
    // to https://www.viewstl.com/ to visualize it.

    // Load the just-saved STL file and re-save it to show that they are identical
    std::ifstream fin("shape.stl", std::ios_base::binary);
    Tessellation3D loadTess = Tessellation3D::LoadSTL(fin);
    loadTess.WriteSTL("shape_copy.stl", false);

    return 0;
}

Map example_rivers()
{
    // Set noise generation parameters
    NoiseConfig cfg;
    cfg.bounds = VecInt(500, 500);
    cfg.boxSize = VecInt(300, 300);
    cfg.normalize = false; // do not normalize between 0 and 1
    
    // Generate basic heightmap
    Map rivers = Simplex(cfg);

    // Because cfg.normalize = false, the heightmap should be somewhat
    // evenly distributed between positive and negative values. This means
    // that taking the absolute value of each point in the heightmap will
    // create winding, river-like lines in the map where the height is
    // zero. After Abs(), we chain-call Interpolate() to adjust the range
    // of heightmap values.
    rivers.Abs().Interpolate(0, 1);

    // Adjust configuration parameters. Here, we're looking for a regular
    // old, normalized heightmap that we can use to make the texture of
    // the landscape just a little more interesting. We also want a new
    // seed so the new map isn't just a copy of the old one.
    cfg.NewSeed();
    cfg.normalize = true;
    cfg.boxSize = cfg.bounds;
    Map map = Simplex(cfg);

    // Multiply each point in the maps together. This will guarantee that
    // the first map keeps its rivers at height 0, which adding the maps
    // together would not do.
    // As a side note, adding them would still create an interesting
    // texture, but I think the rivers look a little more realistic when
    // you multiply at this step.
    map *= rivers;

    // After the multiplication, we know the minimum value is still 0,
    // but the maximum value is somewhere between 0 and 1. We normalize
    // the map here, then put everything to a positive power less than 1,
    // which will shift lower values upwards more than higher values, on
    // average. This makes the land near the rivers have steeper slopes,
    // which can be a pretty cool effect.
    map.Interpolate(0, 1).Pow(0.55);

    // We want our rivers to look like there's actually water in them.
    // Adjusting all values below some arbitrary minimum to be equal to
    // that minimum will make all low-height parts of the map flat. This
    // is also useful in creating islands; just choose a higher minimum.
    map.BoundMin(0.15);
    
    // Interpolate within a nice range for the 3D model
    map.Interpolate(15, 50);

    return map;
}

Map example_pine_bark()
{
    // Set noise generation parameters
    NoiseConfig cfg;
    cfg.bounds = VecInt(500, 500);
    cfg.boxSize = VecInt(100, 400); // stretch the noise map vertically
    cfg.normalize = false; // Don't normalize between 0 and 1
    cfg.octaves = 6;

    // Generate basic heightmap
    Map map_bark = Simplex(cfg);

    // Here, we're doing something very similar to the river example up
    // above. The difference is that after calling Abs(), we pass 1 in as
    // the new 'minimum' and 0 as the new 'maximum' in Interpolate(). This
    // has the effect of flipping the map so that all low points are now
    // high, and vice versa. Interpolate() is a handy function for many
    // reasons, but this is an especially nice use case!
    map_bark.Abs().Interpolate(1, 0);

    // Create plateau-like regions by setting all values above some
    // arbitrary maximum to that maximum.
    map_bark.BoundMax(0.81).Interpolate(0, 1);

    // Taking all values to a power greater than 1 will adjust low values
    // lower more than high values, on average. This makes the regions
    // bordering the plateaus have steeper slopes.
    map_bark.Pow(2.5);
    
    // Get a new seed, and adjust the layers of detail (octaves) to be
    // higher. Also, this map will also not be stretched vertically.
    cfg.NewSeed();
    cfg.octaves = 8;
    cfg.boxSize = VecInt(400, 400);

    // Generate the texture map and interpolate within a meaningful range
    Map map_texture = Simplex(cfg);
    map_texture.Interpolate(0, 0.3);

    // Add the texture map to the bark map
    map_bark += map_texture;

    // Interpolate within a nice range for the 3D model
    map_bark.Interpolate(20, 50);

    return map_bark;
}

Map example_archipelago()
{
    // Set noise generation parameters
    NoiseConfig cfg;
    cfg.bounds = VecInt(500, 500);
    cfg.boxSize = VecInt(250, 250);
    cfg.normalize = false;
    
    // Here's a fun example of how not to use method chaining! I like
    // to keep it to only a couple methods per line, and only if they
    // are working together to perform one simple task. A good example
    // might be adding two heightmaps together, and interpolating the
    // result to a new range, e.g. "map1.Add(map2).Interpolate(0, 1);"
    // But hey, to each their own. In order, this line performs the
    // following steps:
    // (1) Generate a basic heightmap
    // (2) Take the absolute value of the generated heightmap
    // (3) Interpolate the heightmap between 0 and 1
    // (4) Set a minimum bound, creating ocean-like flat regions
    // (5) Interpolate within a nice range for the 3D model
    return Simplex(cfg).Abs().Interpolate(0, 1).BoundMin(0.25).Interpolate(15, 50);
}