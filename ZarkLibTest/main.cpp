#include "noise.h"
#include "Zimg.h"
#include "Rect.h"

#include "Shape3D.h"

#include <iostream>

using namespace zmath;

const unsigned long COLOR_CHANNELS = 3; // just rgb; no alpha for simplicity's sake
const double PI = 3.14159265358979323846264;

void modernArt();

int main()
{
	std::ofstream file("test.stl");

	Shape3D mainShape = Shape3D::Prism(7, 3, 2, Vec3());

	mainShape.WriteSTL(file);
	
	file.close();

	return 0;
}

void modernArt()
{
	// Generate image!
	NoiseConfig cfg;
	cfg.bounds = Vec(1000, 1000);
	cfg.boxSizeInitial = 300;
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
	zimg::RGBA colors[colorCt]{
		zimg::RGBA(0, 0, 0),
		zimg::RGBA(255, 255, 255),
	};

	zimg::Scheme scheme(colorCt, colors);
	zimg::Image img(slope, scheme);

	img.Save(std::string("noise.png"), 3);
}