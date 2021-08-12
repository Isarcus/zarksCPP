#define _CRT_SECURE_NO_WARNINGS

#include <zarks/noise/noise2D.h>
#include <zarks/image/Image.h>
#include <zarks/math/3D/Shape3D.h>

using namespace zmath;

int main()
{
    // Generate image!
	NoiseConfig cfg;
	cfg.bounds = VecInt(1000, 1000);
	//cfg.boxSize = 300;
	cfg.octaves = 1;
	cfg.octDecrease = 0.5;
	cfg.lNorm = 20; // 20

	cfg.nearest = { 2, 4 }; // { 2, 4 }

	Map noiseMap = Worley(cfg);
	Map slope = noiseMap.SlopeMap();
	slope.Interpolate(0, 1);

	std::vector<RGBA> colors{
		RGBA(0, 0, 0),
		RGBA(255, 255, 255)
	};

	zmath::Scheme scheme(colors);
	zmath::Image img(slope, scheme);

	img.Save("noise.png", 3);

    return 0;
}