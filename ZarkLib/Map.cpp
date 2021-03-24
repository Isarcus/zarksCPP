#include "pch.h"
#include "Map.h"
#include "zmath_internals.h"
#include <cmath>
#include <fstream>
#include <iostream>

// If anyone other than me, Isaac Brooks, ever reads this, I hope you know I'm better than this. I need to believe I'm better than this.
#define LOOPSTART for (int x = 0; x < bounds.X; x++) { for (int y = 0; y < bounds.Y; y++) {
#define LOOPEND } }
#define LOOP for (int x = 0; x < bounds.X; x++) for (int y = 0; y < bounds.Y; y++)
#define BOUNDABORT if (bounds != m.bounds) return *this;

double nonsense;

namespace zmath
{
	Map::Map(Vec bounds_)
	{
		subMap = false;
		bounds = bounds_.Floor();

		double** newData = new double* [int(bounds.X)];
		for (int x = 0; x < bounds.X; x++)
		{
			newData[x] = new double[int(bounds.Y)];

			for (int y = 0; y < bounds.Y; y++)
			{
				newData[x][y] = 0;
			}
		}

		data = newData;
	}

	Map::Map(double x, double y)
	{
		subMap = false;
		bounds = Vec(x, y).Floor();

		double** newData = new double* [bounds.X];
		for (int x = 0; x < bounds.X; x++)
		{
			newData[x] = new double[bounds.Y];

			for (int y = 0; y < bounds.Y; y++)
			{
				newData[x][y] = 0;
			}
		}

		data = newData;
	}

	void Map::Delete()
	{
		// Only an original map should deallocate all of the underlying memory...
		if (!subMap)
		{
			for (int x = 0; x < bounds.X; x++)
			{
				delete[] data[x];
			}
		}

		// ...but all maps have a unique **double for 'data' that is safe to delete
		delete[] data;
	}

	double* Map::operator[](int x) const
	{
		return data[x];
	}

	double& Map::At(Vec pt) const
	{
		if (pt < bounds && pt >= ZV) return data[int(pt.X)][int(pt.Y)];
		return nonsense;
	}

	double& Map::At(int x, int y) const
	{
		if (x < bounds.X && y < bounds.Y && x >= 0 && y >= 0) return data[x][y];
		return nonsense;
	}

	void Map::Set(Vec pt, double val)
	{
		data[int(pt.X)][int(pt.Y)] = val;
	}

	void Map::Set(int x, int y, double val)
	{
		data[x][y] = val;
	}

	void Map::operator=(Map m)
	{
		if (!subMap) for (int x = 0; x < bounds.X; x++) delete[] data[x];
		delete[] data;

		data = m.data;
		bounds = m.bounds;
		subMap = m.subMap;
	}

	double Map::GetMin() const
	{
		double min = data[0][0];

		LOOP min = (min > data[x][y]) ? data[x][y] : min;

		return min;
	}

	double Map::GetMax() const
	{
		double max = data[0][0];

		LOOP max = (max < data[x][y]) ? data[x][y] : max;

		return max;
	}

	minmax Map::GetMinMax() const
	{
		double min, max;
		min = max = data[0][0];

		// I'm the only one who has to read this code I'm the only one who has to read this code I'm the only one who has to read this code I'm the only 
		LOOP
		{
			max = (max < data[x][y]) ? data[x][y] : max;
			min = (min > data[x][y]) ? data[x][y] : min;
		}

		return minmax{ min, max };
	}

	Vec Map::Bounds() const
	{
		return bounds;
	}

	double Map::Sum() const
	{
		double sum = 0;
		LOOP sum += data[x][y];
		return sum;
	}

	double Map::Mean() const
	{
		return Sum() / bounds.Area();
	}

	double Map::Variance() const
	{
		double mean = Mean();
		double variance = 0;

		LOOP variance += std::pow(mean - data[x][y], 2);

		return variance / bounds.Area() - 1;
	}

	double Map::Std() const
	{
		return std::sqrt(Variance());
	}

	bool Map::ContainsCoord(Vec pos) const
	{
		return pos.Floor() < bounds && pos >= ZV;
	}

	Vec Map::DerivativeAt(Vec pos) const
	{
		if (!ContainsCoord(pos)) return Vec();

		bool posX = ContainsCoord(pos + Vec( 1,  0));
		bool posY = ContainsCoord(pos + Vec( 0,  1));
		bool negX = ContainsCoord(pos + Vec(-1,  0));
		bool negY = ContainsCoord(pos + Vec( 0, -1));

		Vec dh;
		Vec weight;

		double val = At(pos);

		if (posX) {
			dh.X += 2.0 * (At(pos + Vec(1, 0)) - val);
			weight.X += 2;

			if (posY) {
				dh.X += At(pos + Vec(1, 1)) - val;
				dh.Y += At(pos + Vec(1, 1)) - val;

				weight.X++;
				weight.Y++;
			}
			if (negY) {
				dh.X += At(pos + Vec(1, -1)) - val;
				dh.Y -= At(pos + Vec(1, -1)) - val;

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

	double Map::GradientAt(Vec pos) const
	{
		Vec dh = DerivativeAt(pos);
		return std::atan2(pos.Y, pos.X);
	}

	double Map::SlopeAt(Vec pos) const
	{
		return DerivativeAt(pos).DistForm(Vec());
	}

	Map& Map::Copy() const
	{
		Map* m = new Map(bounds);
		LOOP m->data[x][y] = data[x][y];
		return *m;
	}

	Map& Map::Copy(Vec min, Vec max) const
	{
		Map smaller = (*this)(min, max);
		Map copy = smaller.Copy();
		delete &smaller;

		return copy;
	}

	Map& Map::operator()(Vec min_, Vec max_) const
	{
		// Create correctly bounded and organized min and max vectors
		Vec min = Vec::Min(min_, max_).Floor();
		Vec max = Vec::Max(min_, max_).Floor();
		min = Vec::Max(min, Vec());
		max = Vec::Min(max, bounds);

		// Initialize null map
		Map m = *new Map();
		m.bounds = max - min;

		// Make the new map's data a subset of the called map's data
		m.data = new double* [max.X - min.X];
		for (int idx = 0, x = min.X; x < max.X; x++, idx++)
		{
			m.data[idx] = &(data[x][(int)min.Y]);
		}

		return m;
	}

	Map& Map::Clear(double val)
	{
		LOOP data[x][y] = val;
		return *this;
	}

	Map& Map::Interpolate(double newMin, double newMax)
	{
		minmax old = GetMinMax();
		double oldRange = old.max - old.min;
		if (oldRange == 0)
		{
			LOOP data[x][y] = newMin;
			return *this;
		}

		double newRange = newMax - newMin;

		LOOP data[x][y] = (data[x][y] - old.min) / oldRange * newRange + newMin;

		return *this;
	}

	Map& zmath::Map::Abs()
	{
		LOOP data[x][y] = std::abs(data[x][y]);
		return *this;
	}

	Map zmath::Map::SlopeMap()
	{
		Map m = *new Map(bounds);

		LOOP m.At(x, y) = SlopeAt(Vec(x, y));

		return m;
	}

	Map& Map::operator+=(Map m)
	{
		BOUNDABORT;

		LOOP data[x][y] += m.data[x][y];
		
		return *this;
	}

	Map& Map::operator-=(Map m)
	{
		BOUNDABORT;

		LOOP data[x][y] -= m.data[x][y];

		return *this;
	}

	Map& Map::operator*=(Map m)
	{
		BOUNDABORT;

		LOOP data[x][y] *= m.data[x][y];

		return *this;
	}

	Map& Map::operator/=(Map m)
	{
		BOUNDABORT;

		LOOP
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
		LOOP data[x][y] += val;
		return *this;
	}

	Map& Map::operator-=(double val)
	{
		LOOP data[x][y] -= val;
		return *this;
	}

	Map& Map::operator*=(double val)
	{
		LOOP data[x][y] *= val;
		return *this;
	}

	Map& Map::operator/=(double val)
	{
		if (val != 0) LOOP data[x][y] /= val;
		return *this;
	}

	Map& Map::Add(Map m) { return *this += m; }
	Map& Map::Sub(Map m) { return *this -= m; }
	Map& Map::Mul(Map m) { return *this *= m; }
	Map& Map::Div(Map m) { return *this /= m; }

	Map& Map::Add(double val) { return *this += val; }
	Map& Map::Sub(double val) { return *this -= val; }
	Map& Map::Mul(double val) { return *this *= val; }
	Map& Map::Div(double val) { return *this /= val; }

	Map& Map::Pow(double exp)
	{
		LOOP data[x][y] = std::pow(data[x][y], exp);
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
		LOOP
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

	// Private initializer for totally nil map
	Map::Map()
	{
		subMap = true;
		data = NULL;
		bounds = Vec();
	}

}