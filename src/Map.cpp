#include <zarks/math/Map.h>
#include <zarks/internal/zmath_internals.h>
#include <zarks/io/binary.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <exception>

#define LOOP_MAP for (int x = 0; x < bounds.X; x++) for (int y = 0; y < bounds.Y; y++)
#define BOUNDABORT( m ) if (bounds != m.bounds) throw std::runtime_error("Map bounds don't match!")

namespace zmath
{

Map::Map()
	: Sampleable2D()
{}

Map::Map(VecInt bounds)
	: Sampleable2D(bounds)
{}

Map::Map(int x, int y)
	: Map(VecInt(x, y))
{}

Map::Map(const Map& map)
	: Sampleable2D()
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
	FreeData();
}

Map& Map::operator=(const Map& rhs)
{
	if (bounds != rhs.bounds)
	{
		FreeData();

		bounds = rhs.bounds;

		data = alloc2d<double>(bounds.X, bounds.Y);
	}

	LOOP_MAP data[x][y] = rhs.data[x][y];
	return *this;
}

Map& Map::operator=(Map&& rhs)
{
	if (this != &rhs)
	{
		FreeData();

		data = rhs.data;
		bounds = rhs.bounds;

		rhs.data = nullptr;
		rhs.bounds = VecInt(0, 0);
	}

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

Map Map::Copy(VecInt min, VecInt max) const
{
	return (*this)(min, max);
}

Map Map::operator()(VecInt min_, VecInt max_) const
{
	// Create correctly bounded and organized min and max vectors
	VecInt min = Vec::Min(min_, max_);
	VecInt max = Vec::Max(min_, max_);
	min = VecInt::Max(min, VecInt(0, 0));
	max = VecInt::Min(max, bounds);
	VecInt newBounds(max - min);

	// Create new map
	Map m(newBounds);
	for (int x = 0; x < newBounds.X; x++)
	{
		for (int y = 0; y < newBounds.Y; y++)
		{
			m[x][y] = data[min.X + x][min.Y + y];
		}
	}

	return m;
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

Map& Map::Abs()
{
	LOOP_MAP data[x][y] = std::abs(data[x][y]);
	return *this;
}

Map& Map::FillBorder(int thickness, double val)
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

Map& Map::Fill(VecInt min, VecInt max, double val)
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

Map& Map::Replace(double val, double with)
{
	LOOP_MAP if (data[x][y] == val) data[x][y] = with;
	return *this;
}

Map& Map::Apply(const GaussField& gauss)
{
	LOOP_MAP data[x][y] += gauss.Sample(x, y);
	return *this;
}

Map& Map::Apply(double(*calculation)(double))
{
	LOOP_MAP data[x][y] = calculation(data[x][y]);
	return *this;
}

Map Map::SlopeMap()
{
	Map m(bounds);

	LOOP_MAP m.At(x, y) = SlopeAt(VecInt(x, y));

	return m;
}

Map& Map::BoundMax(double newMax)
{
	LOOP_MAP data[x][y] = std::min(newMax, data[x][y]);
	return *this;
}

Map& Map::BoundMin(double newMin)
{
	LOOP_MAP data[x][y] = std::max(newMin, data[x][y]);
	return *this;
}

Map& Map::Bound(double newMin, double newMax)
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
		uint8_t arr[sizeof(double)];
		ToBytes(arr, data[x][y], Endian::Little);

		file.write((char*)arr, sizeof(arr));
	}
		
	std::cout << "File saved at " << path << "\n";
}

} // namespace zmath
