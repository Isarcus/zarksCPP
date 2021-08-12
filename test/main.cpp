#define _CRT_SECURE_NO_WARNINGS

#include <zarks/noise/noise2D.h>
#include <zarks/image/Image.h>
#include <zarks/math/3D/Shape3D.h>

#include <iostream>
#include <cstdlib>
#include <string>
#include <random>

using namespace zmath;

const unsigned long COLOR_CHANNELS = 3; // just rgb; no alpha for simplicity's sake
const double PI = 3.14159265358979323846264;

void modernArt();

int main()
{
	modernArt();
}

void modernArt()
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

	img.Save("noise.png", COLOR_CHANNELS);
}