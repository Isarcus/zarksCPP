/**
 * Author: Isarcus
 * 
 * Created: A long time ago
 * 
 * Description:
 *  A simple script for creating what I like to think of
 *  as computer-generated modern art. Try experimenting
 *  with some of the parameters for really cool effects!
 * 
 */

#include <zarks/noise/noise2D.h> // All 2D noise functions are here
#include <zarks/math/Map.h>      // Map class (for 2D heightmaps)
#include <zarks/image/Image.h>   // Image class (for RGBA images)

// The entire zarks library is contained within this namespace
using namespace zmath;

int main()
{
	// Set configuration for this noise map
	NoiseConfig cfg;
	cfg.bounds = VecInt(500, 500);  // Overall map size
	cfg.boxSize = VecInt(500, 500); // Size of a noise 'box'
	cfg.octaves = 1; // How many octaves of detail to generate
	cfg.lNorm = 20;  // Vector L# norm used to calculate distances
	cfg.nearest = { 2, 4 }; // Range of nearest points to use for Worley noise

	// Generate noise map with the provided config
	Map noiseMap = Worley(cfg);

	// Get the slope of the previous map (this will look cooler, in my opinion)
	Map slope = noiseMap.SlopeMap();
	slope.Interpolate(0, 1);

	// Create a color scheme to color an image with
	std::vector<RGBA> colors{
		RGBA(0, 0, 0),      // Color where map values are 0
		RGBA(0, 0, 255),    // This would be the color where map values are 0.5
		RGBA(255, 255, 255) // Color where map values are 1
	};

	// Create an image from the slope map and color scheme
	Image img(slope, colors);

	// Save 3-channel (RGB) image with a creative title
	img.Save("noise.png");

    return 0;
}
