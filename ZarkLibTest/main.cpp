#define _CRT_SECURE_NO_WARNINGS

#include "noise.h"
#include "Zimg.h"
#include "Rect.h"
#include "Shape3D.h"
#include "Tessellation3D.h"

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
	NoiseConfig cfg;
	cfg.bounds = { 500, 500 };
	cfg.boxSize = { 400, 400 };
	cfg.nearest = { 2, 4 };
	cfg.octaves = 2;
	cfg.lNorm = 1.5;
	cfg.seed = 16270097341950460;
	Map m = *Worley(cfg);

	m = *m.SlopeMap();
	m.Interpolate(0, 1);

	Image img(m);
	img.Save("test.png");
	
	m.Interpolate(6, 40);
	Tessellation3D tess(m);
	tess.Rotate(ZM_PID2, 0, 0);
	tess.WriteSTL("test.stl", false);
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

	Map noiseMap = *Worley(cfg);
	Map slope = *noiseMap.SlopeMap();
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

	img.Save(std::string("noise.png"), COLOR_CHANNELS);
}