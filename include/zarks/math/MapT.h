#pragma once

#include <zarks/math/VecT.h>
#include <zarks/internal/zmath_internals.h>

#include <cmath>
#include <stdexcept>

#define LOOP_MAP for (int x = 0; x < bounds.X; x++) for (int y = 0; y < bounds.Y; y++) 

namespace zmath
{
	template <typename T>
	class MapT
	{
	public:
		MapT(VecInt bounds);
		MapT(int x, int y);
		MapT(const MapT& map);
		MapT(MapT&& map);
		~MapT();

		// Accessors

		T* const& operator[](int x);
		const T* const& operator[](int x) const;

		T& At(VecInt pt);
		const T& At(VecInt pt) const;
		T& At(int x, int y);
		const T& At(int x, int y) const;

		void Set(VecInt pt, T val);
		void Set(int x, int y, T val);

		MapT& operator= (const MapT& m);
		MapT& operator= (MapT&& m);

		// Map characteristics

		T GetMin() const;
		T GetMax() const;
		std::pair<T, T> GetMinMax() const;
		VecInt Bounds() const;

		T Sum() const;
		T Mean() const;
		T Variance() const;
		T Std() const;

		bool ContainsCoord(VecInt pos) const;

		// Chainable manipulation functions

		MapT& Copy() const;
		MapT& Clear(T val);
		MapT& Interpolate(T newMin, T newMax);
		MapT& Abs();

		// Math operator overloads

		MapT& operator+= (const MapT& m);
		MapT& operator-= (const MapT& m);
		MapT& operator*= (const MapT& m);
		MapT& operator/= (const MapT& m);
		MapT& operator+= (T val);
		MapT& operator-= (T val);
		MapT& operator*= (T val);
		MapT& operator/= (T val);

		// Chainable functions

		MapT& Add(const MapT& m);
		MapT& Sub(const MapT& m);
		MapT& Mul(const MapT& m);
		MapT& Div(const MapT& m);
		MapT& Add(T val);
		MapT& Sub(T val);
		MapT& Mul(T val);
		MapT& Div(T val);

	private:
		VecInt bounds;
		T** data;
	};
}

//                //
// IMPLEMENTATION //
//                //

namespace zmath
{

template<typename T>
inline MapT<T>::MapT(VecInt bounds)
	: bounds(bounds)
	, data(alloc2d<T>(bounds.X, bounds.Y))
{}

template<typename T>
inline MapT<T>::MapT(int x, int y)
	: MapT(VecInt(x, y))
{}

template<typename T>
inline MapT<T>::MapT(const MapT& map)
	: MapT(map.bounds)
{
	*this = map;
}

template<typename T>
inline MapT<T>::MapT(MapT&& map)
	: MapT(map)
{}

template<typename T>
inline MapT<T>::~MapT()
{
	free2d<T>(data, bounds.X);
}

template<typename T>
inline T* const& MapT<T>::operator[](int x)
{
	return data[x];
}

template<typename T>
inline const T* const& MapT<T>::operator[](int x) const
{
	return data[x];
}

template<typename T>
inline T& MapT<T>::At(VecInt pt)
{
	return At(pt.X, pt.Y);
}

template<typename T>
inline const T& MapT<T>::At(VecInt pt) const
{
	return At(pt.X, pt.Y);
}

template<typename T>
inline T& MapT<T>::At(int x, int y)
{
	if (x < 0 || x >= bounds.X ||
		y < 0 || y >= bounds.Y)
	{
		throw std::runtime_error("Tried to access MapT out of bounds!");
	}

	return data[x][y];
}

template<typename T>
inline const T& MapT<T>::At(int x, int y) const
{
	if (x < 0 || x >= bounds.X ||
		y < 0 || y >= bounds.Y)
	{
		throw std::runtime_error("Tried to access MapT out of bounds!");
	}

	return data[x][y];
}

template<typename T>
inline void MapT<T>::Set(VecInt pt, T val)
{
	Set(pt.X, pt.Y, val);
}

template<typename T>
inline void MapT<T>::Set(int x, int y, T val)
{
	if (x < 0 || x >= bounds.X ||
		y < 0 || y >= bounds.Y)
	{
		throw std::runtime_error("Tried to access MapT out of bounds!");
	}

	return data[x][y] = val;
}

template<typename T>
inline MapT<T>& MapT<T>::operator= (const MapT& m)
{
	bounds = m.bounds;
	LOOP_MAP
	{
		data[x][y] = m[x][y];
	}

	return *this;
}

template<typename T>
inline MapT<T>& MapT<T>::operator= (MapT&& m)
{
	if (this != &m)
	{
		free2d(data, bounds.X);

		data = m.data;
		bounds = m.bounds;

		m.data = nullptr;
		m.bounds = VecInt(0, 0);
	}

	return *this;
}

template<typename T>
inline T MapT<T>::GetMin() const
{
	T min = data[0][0];

	LOOP_MAP
	{
		min = (data[x][y] < min) ? data[x][y] : min;
	}

	return min;
}

template<typename T>
inline T MapT<T>::GetMax() const
{
	T max = data[0][0];

	LOOP_MAP
	{
		max = (data[x][y] > max) ? data[x][y] : max;
	}

	return max;
}

template<typename T>
inline std::pair<T, T> MapT<T>::GetMinMax() const
{
	std::pair<T, T> minmax{ data[0][0], data[0][0] };

	LOOP_MAP
	{
		minmax.first = (data[x][y] < minmax.first) ? data[x][y] : minmax.first;
		minmax.second = (data[x][y] > minmax.second) ? data[x][y] : minmax.second;
	}

	return minmax;
}

template<typename T>
inline VecInt MapT<T>::Bounds() const
{
	return bounds;
}

template<typename T>
inline T MapT<T>::Sum() const
{
	T sum = T();

	LOOP_MAP
	{
		sum += data[x][y];
	}

	return sum;
}

template<typename T>
inline T MapT<T>::Mean() const
{
	return Sum() / (bounds.X * bounds.Y - 1);
}

template<typename T>
inline T MapT<T>::Variance() const
{
	T mean = Mean();
	T sumDiffs = T();

	LOOP_MAP
	{
		T diff = AbsT(data[x][y] - mean);
		sumDiffs += diff * diff;
	}

	return sumDiffs / (bounds.X * bounds.Y - 1);
}

template<typename T>
inline T MapT<T>::Std() const
{
	return std::sqrt(Variance());
}

template<typename T>
inline bool MapT<T>::ContainsCoord(VecInt pos) const
{
	return pos > VecInt(0, 0) && pos < bounds;
}

// Chainable manipulation functions

template<typename T>
inline MapT<T>& MapT<T>::Copy() const
{
	MapT<T>* m = new MapT<T>(*this);
	return *m;
}

template<typename T>
inline MapT<T>& MapT<T>::Clear(T val)
{
	LOOP_MAP
	{
		data[x][y] = val;
	}

	return *this;
}

template<typename T>
inline MapT<T>& MapT<T>::Interpolate(T newMin, T newMax)
{
	std::pair<T, T> oldMinMax = GetMinMax();
	T oldRange = oldMinMax.second - oldMinMax.first;
	T newRange = newMax - newMin;
	T scale = newRange / oldRange;

	LOOP_MAP
	{
		data[x][y] = (data[x][y] - oldMinMax.first) * scale + newMax;
	}

	return *this;
}

template<typename T>
inline MapT<T>& MapT<T>::Abs()
{
	LOOP_MAP
	{
		data[x][y] = AbsT(data[x][y]);
	}

	return *this;
}

// Math operator overloads

template<typename T>
inline MapT<T>& MapT<T>::operator+= (const MapT& m)
{
	LOOP_MAP
	{
		data[x][y] += m[x][y];
	}
	return *this;
}

template<typename T>
inline MapT<T>& MapT<T>::operator-= (const MapT& m)
{
	LOOP_MAP
	{
		data[x][y] -= m[x][y];
	}
	return *this;
}

template<typename T>
inline MapT<T>& MapT<T>::operator*= (const MapT& m)
{
	LOOP_MAP
	{
		data[x][y] *= m[x][y];
	}
	return *this;
}

template<typename T>
inline MapT<T>& MapT<T>::operator/= (const MapT& m)
{
	LOOP_MAP
	{
		data[x][y] /= m[x][y];
	}
	return *this;
}

template<typename T>
inline MapT<T>& MapT<T>::operator+= (T val)
{
	LOOP_MAP
	{
		data[x][y] += val;
	}
	return *this;
}

template<typename T>
inline MapT<T>& MapT<T>::operator-= (T val)
{
	LOOP_MAP
	{
		data[x][y] -= val;
	}
	return *this;
}

template<typename T>
inline MapT<T>& MapT<T>::operator*= (T val)
{
	LOOP_MAP
	{
		data[x][y] *= val;
	}
	return *this;
}

template<typename T>
inline MapT<T>& MapT<T>::operator/= (T val)
{
	LOOP_MAP
	{
		data[x][y] /= val;
	}
	return *this;
}

// Chainable functions

template<typename T>
inline MapT<T>& MapT<T>::Add(const MapT& m) { return (*this) += m; }

template<typename T>
inline MapT<T>& MapT<T>::Sub(const MapT& m) { return (*this) -= m; }

template<typename T>
inline MapT<T>& MapT<T>::Mul(const MapT& m) { return (*this) *= m; }

template<typename T>
inline MapT<T>& MapT<T>::Div(const MapT& m) { return (*this) /= m; }

template<typename T>
inline MapT<T>& MapT<T>::Add(T val) { return (*this) += val; }

template<typename T>
inline MapT<T>& MapT<T>::Sub(T val) { return (*this) -= val; }

template<typename T>
inline MapT<T>& MapT<T>::Mul(T val) { return (*this) *= val; }

template<typename T>
inline MapT<T>& MapT<T>::Div(T val) { return (*this) /= val; }

} // namespace zmath

#undef LOOP_MAP
