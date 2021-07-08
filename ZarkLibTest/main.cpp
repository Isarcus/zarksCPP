#define _CRT_SECURE_NO_WARNINGS

#include "noise.h"
#include "Zimg.h"
#include "Rect.h"
#include "Shape3D.h"

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
	std::string imglib = getenv("IMGLIB");
	Image img(imglib + "/ai/split_me.png");
	
	img.SaveMNIST(
		imglib + "/ai/homemade_data.bt",
		imglib + "/ai/homemade_labels.bt",
		4
	);
}

void modernArt()
{
	// Generate image!
	NoiseConfig cfg;
	cfg.bounds = Vec(1000, 1000);
	//cfg.boxSize = 300;
	cfg.octaves = 1;
	cfg.octDecrease = 0.5;
	cfg.lNorm = 20; // 20

	cfg.nearest = { 2, 4 }; // { 2, 4 }

	Map noiseMap = Worley(cfg);
	Map slope = noiseMap.SlopeMap();
	slope.Interpolate(0, 1);

	//slope.Interpolate(-1, 1).Abs().Interpolate(1, 0);
	//slope.Save("noise.zmap");

	const int colorCt = 2;
	zmath::RGBA colors[colorCt]{
		zmath::RGBA(0, 0, 0),
		zmath::RGBA(255, 255, 255),
	};

	zmath::Scheme scheme(colorCt, colors);
	zmath::Image img(slope, scheme);

	img.Save(std::string("noise.png"), 3);
}