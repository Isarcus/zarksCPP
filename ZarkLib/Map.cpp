#include "pch.h"
#include "Map.h"
#include "zmath_internals.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <exception>

#define LOOP_MAP for (int x = 0; x < bounds.X; x++) for (int y = 0; y < bounds.Y; y++)
#define BOUNDABORT( m ) if (bounds != m.bounds) throw std::runtime_error("Map bounds don't match!")

namespace zmath
{

Map::Map(VecInt bounds)
	: Sampleable2D(bounds)
	, subMap(false)
{}

Map::Map(int x, int y)
	: Map(VecInt(x, y))
{}

Map::Map(const Map& map)
	: Sampleable2D()
	, subMap(false)
{
	*this = map;
}

Map::Map(Map&& map)
	: Sampleable2D()
{
	*this = std::move(map);
}

Map::~Map()
{
	if (!subMap)
	{
		FreeData();
	}
}

Map& Map::operator=(const Map& m)
{
	if (bounds != m.bounds)
	{
		FreeData();

		bounds = m.bounds;
		subMap = false;

		data = alloc2d<double>(bounds.X, bounds.Y);
	}

	LOOP_MAP data[x][y] = m.data[x][y];
	return *this;
}

Map& Map::operator=(Map&& map)
{
	FreeData();

	data = map.data;
	bounds = map.bounds;
	subMap = map.subMap;

	map.data = nullptr;
	map.bounds = VecInt(0, 0);
	map.subMap = false;

	return *this;
}

double Map::GetMin() const
{
	double min = data[0][0];

	LOOP_MAP min = std::min(min, data[x][y]);

	return min;
}

double Map::GetMax() const
{
	double max = data[0][0];

	LOOP_MAP max = std::max(max, data[x][y]);

	return max;
}

std::pair<double, double> Map::GetMinMax() const
{
	std::pair<double, double> minmax;

	// I'm the only one who has to read this code I'm the only one who has to read this code I'm the only one who has to read this code I'm the only 
	LOOP_MAP
	{
		minmax.first = std::min(minmax.first, data[x][y]);
		minmax.second = std::max(minmax.second, data[x][y]);
	}

	return minmax;
}

VecInt Map::Bounds() const
{
	return bounds;
}

double Map::Sum() const
{
	double sum = 0;
	LOOP_MAP sum += data[x][y];
	return sum;
}

double Map::Mean() const
{
	return Sum() / (double)bounds.Area();
}

double Map::Variance() const
{
	double mean = Mean();
	double variance = 0;

	LOOP_MAP variance += std::pow(mean - data[x][y], 2);

	return variance / (double)(bounds.Area() - 1);
}

double Map::Std() const
{
	return std::sqrt(Variance());
}

Vec Map::DerivativeAt(VecInt pos) const
{
	if (!ContainsCoord(pos)) return Vec();

	bool posX = ContainsCoord(pos + VecInt( 1,  0));
	bool posY = ContainsCoord(pos + VecInt( 0,  1));
	bool negX = ContainsCoord(pos + VecInt(-1,  0));
	bool negY = ContainsCoord(pos + VecInt( 0, -1));

	Vec dh;
	Vec weight;

	double val = At(pos);

	if (posX) {
		dh.X += 2.0 * (At(pos + Vec(1, 0)) - val);
		weight.X += 2;

		if (posY) {
			dh.X += At(pos + VecInt(1, 1)) - val;
			dh.Y += At(pos + VecInt(1, 1)) - val;

			weight.X++;
			weight.Y++;
		}
		if (negY) {
			dh.X += At(pos + VecInt(1, -1)) - val;
			dh.Y -= At(pos + VecInt(1, -1)) - val;

			weight.X++;
			weight.Y++;
		}
	}

	if (negX) {
		dh.X -= 2.0 * (At(pos + Vec(-1, 0)) - val);
		weight.X += 2;

		if (posY) {
			dh.X -= At(pos + Vec(-1, 1)) - val;
			dh.Y += At(pos + Vec(-1, 1)) - val;

			weight.X++;
			weight.Y++;
		}
		if (negY) {
			dh.X -= At(pos + Vec(-1, -1)) - val;
			dh.Y -= At(pos + Vec(-1, -1)) - val;

			weight.X++;
			weight.Y++;
		}
	}
	if (posY) {
		dh.Y += 2.0 * (At(pos + Vec(0, 1)) - val);
		weight.Y += 2;
	}
	if (negY) {
		dh.Y -= 2.0 * (At(pos + Vec(0, -1)) - val);
		weight.Y += 2;
	}

	return dh / weight;
}

double Map::SlopeAt(VecInt pos) const
{
	return DerivativeAt(pos).DistForm();
}

std::unique_ptr<Map> Map::Copy(VecInt min, VecInt max) const
{
	return (*this)(min, max);
}

std::unique_ptr<Map> Map::operator()(VecInt min_, VecInt max_) const
{
	// Create correctly bounded and organized min and max vectors
	VecInt min = Vec::Min(min_, max_);
	VecInt max = Vec::Max(min_, max_);
	min = VecInt::Max(min, VecInt(0, 0));
	max = VecInt::Min(max, bounds);

	// Initialize submap
	Map& m = *new Map(max - min);
	m.subMap = true;

	// Make the new map's data a subset of the called map's data
	m.data = new double* [max.X - min.X];
	for (int idx = 0, x = min.X; x < max.X; x++, idx++)
	{
		m.data[idx] = &(data[x][(int)min.Y]);
	}

	return std::unique_ptr<Map>(&m);
}

Map& Map::Clear(double val)
{
	LOOP_MAP data[x][y] = val;
	return *this;
}

Map& Map::Interpolate(double newMin, double newMax)
{
	auto old = GetMinMax();
	double oldRange = old.second - old.first;
	if (oldRange == 0)
	{
		LOOP_MAP data[x][y] = newMin;
		return *this;
	}

	double newRange = newMax - newMin;

	LOOP_MAP data[x][y] = (data[x][y] - old.first) / oldRange * newRange + newMin;

	return *this;
}

Map& zmath::Map::Abs()
{
	LOOP_MAP data[x][y] = std::abs(data[x][y]);
	return *this;
}

Map& zmath::Map::FillBorder(int thickness, double val)
{
	thickness = std::min(thickness, bounds.Min());
	// Left
	Fill({ 0, 0 }, { thickness, bounds.Y }, val);
	// Right
	Fill({ bounds.X - thickness, 0 }, { bounds.X, bounds.Y }, val);
	// Top (no corners)
	Fill({ thickness, bounds.Y - thickness }, { bounds.X - thickness, bounds.Y }, val);
	// Bottom (no corners)
	Fill({ thickness, 0 }, { bounds.X - thickness, thickness }, val);
	return *this;
}

Map& zmath::Map::Fill(VecInt min, VecInt max, double val)
{
	for (int x = min.X; x < max.X; x++)
	{
		for (int y = min.Y; y < max.Y; y++)
		{
			data[x][y] = val;
		}
	}
	return *this;
}

Map& zmath::Map::Replace(double val, double with)
{
	LOOP_MAP if (data[x][y] == val) data[x][y] = with;
	return *this;
}

Map& zmath::Map::Apply(const GaussField& gauss)
{
	LOOP_MAP data[x][y] += gauss.Sample(x, y);
	return *this;
}

Map& zmath::Map::Apply(double(*calculation)(double))
{
	LOOP_MAP data[x][y] = calculation(data[x][y]);
	return *this;
}

std::unique_ptr<Map> zmath::Map::SlopeMap()
{
	Map& m = *new Map(bounds);

	LOOP_MAP m.At(x, y) = SlopeAt(VecInt(x, y));

	return std::unique_ptr<Map>(&m);
}

Map& zmath::Map::BoundMax(double newMax)
{
	LOOP_MAP data[x][y] = std::min(newMax, data[x][y]);
	return *this;
}

Map& zmath::Map::BoundMin(double newMin)
{
	LOOP_MAP data[x][y] = std::max(newMin, data[x][y]);
	return *this;
}

Map& zmath::Map::Bound(double newMin, double newMax)
{
	LOOP_MAP data[x][y] = std::min(newMax, std::max(newMin, data[x][y]));
	return *this;
}

Map& Map::operator+=(const Map& m)
{
	BOUNDABORT(m);

	LOOP_MAP data[x][y] += m.data[x][y];
		
	return *this;
}

Map& Map::operator-=(const Map& m)
{
	BOUNDABORT(m);

	LOOP_MAP data[x][y] -= m.data[x][y];

	return *this;
}

Map& Map::operator*=(const Map& m)
{
	BOUNDABORT(m);

	LOOP_MAP data[x][y] *= m.data[x][y];

	return *this;
}

Map& Map::operator/=(const Map& m)
{
	BOUNDABORT(m);

	LOOP_MAP
	{
		if (m.data[x][y] == 0)
		{
			if (data[x][y] > 0) data[x][y] = DOUBLEMAX;
			else if (data[x][y] < 0) data[x][y] = DOUBLEMIN;
		}
		else
		{
			data[x][y] /= m.data[x][y];
		}
	}

	return *this;
}

Map& Map::operator+=(double val)
{
	LOOP_MAP data[x][y] += val;
	return *this;
}

Map& Map::operator-=(double val)
{
	LOOP_MAP data[x][y] -= val;
	return *this;
}

Map& Map::operator*=(double val)
{
	LOOP_MAP data[x][y] *= val;
	return *this;
}

Map& Map::operator/=(double val)
{
	if (val != 0) LOOP_MAP data[x][y] /= val;
	return *this;
}

Map& Map::Add(const Map& m) { return *this += m; }
Map& Map::Sub(const Map& m) { return *this -= m; }
Map& Map::Mul(const Map& m) { return *this *= m; }
Map& Map::Div(const Map& m) { return *this /= m; }

Map& Map::Add(double val) { return *this += val; }
Map& Map::Sub(double val) { return *this -= val; }
Map& Map::Mul(double val) { return *this *= val; }
Map& Map::Div(double val) { return *this /= val; }

Map& Map::Pow(double exp)
{
	LOOP_MAP data[x][y] = std::pow(data[x][y], exp);
	return *this;
}

void Map::Save(std::string path)
{
	std::ofstream file;

	file.open(path, std::ios::binary | std::ios::out);

	uint8_t header[64];
	uint8_t boundX[4];
	uint8_t boundY[4];

	uint32_t uintX = bounds.X;
	uint32_t uintY = bounds.Y;

	// little-endian encoding
	boundX[0] = uintX;
	boundX[1] = uintX >> 8;
	boundX[2] = uintX >> 16;
	boundX[3] = uintX >> 24;

	boundY[0] = uintY;
	boundY[1] = uintY >> 8;
	boundY[2] = uintY >> 16;
	boundY[3] = uintY >> 24;

	// Write the intro data
	file.write((char*)header, sizeof(header));
	file.write((char*)boundX, sizeof(boundX));
	file.write((char*)boundY, sizeof(boundY));

	// Write the actual map data, little-endian
	LOOP_MAP
	{
		double valDouble = data[x][y];
		uint64_t val = *reinterpret_cast<uint64_t*>(&valDouble);

		uint8_t arr[8];
		arr[0] = val;
		arr[1] = val >> 8;
		arr[2] = val >> 16;
		arr[3] = val >> 24;
		arr[4] = val >> 32;
		arr[5] = val >> 40;
		arr[6] = val >> 48;
		arr[7] = val >> 56;

		file.write((char*)arr, sizeof(arr));
	}
		
	std::cout << "File saved at " << path << "\n";
}

} // namespace zmath