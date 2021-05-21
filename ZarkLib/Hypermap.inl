#ifndef HYPERMAP_INL
#include "Hypermap.h"
#endif

#include <exception>
#include <cassert>

#define DIMCHECK for (int i = 0; i < nD; i++) if (dimensions[i] != hmap.dimensions[i]) throw std::exception("Dimensions mismatch!");

namespace zmath
{
	template<class T, int nD>
	inline Hypermap<T, nD>::Hypermap(std::array<int, nD> dimensions, T initVal)
		: data_size(1)
		, dimensions(dimensions)
	{
		// Determine size of the data
		for (const int& d : dimensions)
		{
			if (d < 1)
			{
				data_size = -1;
				data = nullptr;

				throw std::exception("Invalid Hypermap dimensions; no negative numbers!");
			}

			data_size *= d;
		}

		data = new T[data_size];
	}

	template<class T, int nD>
	inline Hypermap<T, nD>::~Hypermap()
	{
		delete[] data;
	}

	//           //
	// ACCESSORS //
	//           //

	template<class T, int nD>
	inline T& Hypermap<T, nD>::At(std::array<int, nD> coord)
	{
		return data[GetIdx(coord)]
	}

	template<class T, int nD>
	inline T& Hypermap<T, nD>::operator[](std::array<int, nD> coord)
	{
		return data[GetIdx(coord)];
	}

	template<class T, int nD>
	inline T& Hypermap<T, nD>::At(int idx)
	{
		return data(idx);
	}

	template<class T, int nD>
	inline T& Hypermap<T, nD>::operator[](int idx)
	{
		return data[idx];
	}

	template<class T, int nD>
	inline T Hypermap<T, nD>::GetMin() const
	{
		T min = data[0];

		for (int i = 0; i < data_size; i++)
		{
			min = (min < data[i]) ? min : data[i];
		}

		return min;
	}

	template<class T, int nD>
	inline T Hypermap<T, nD>::GetMax() const
	{
		T max = data[0];

		for (int i = 0; i < data_size; i++)
		{
			max = (max > data[i]) ? max : data[i];
		}

		return max;
	}

	template<class T, int nD>
	inline std::array<T, 2> Hypermap<T, nD>::GetMinMax()
	{
		T min = data[0];
		T max = data[0];

		for (int i = 0; i < data_size; i++)
		{
			T& elem = data[i];

			max = (max > elem) ? max : elem;
			min = (min < elem) ? min : elem;
		}

		return std::array<T, 2>{min, max};
	}

	template<class T, int nD>
	inline std::array<int, nD> Hypermap<T, nD>::Dimensions()
	{
		return dimensions;
	}

	template<class T, int nD>
	inline T Hypermap<T, nD>::Sum()
	{
		T sum = T();

		for (int i = 0; i < datSize; i++) sum += data[i];

		return T();
	}

	template<class T, int nD>
	inline T Hypermap<T, nD>::Mean()
	{
		T sum = Sum();

		return sum / data_size;
	}

	//           //
	// OPERATORS //
	//           //

	template<class T, int nD>
	inline Hypermap<T, nD>& Hypermap<T, nD>::operator=(Hypermap<T, nD>& hmap)
	{
		DIMCHECK;

		memcpy(data, hmap.data, sizeof(T) * data_size);

		return *this;
	}

	template<class T, int nD>
	inline Hypermap<T, nD>& Hypermap<T, nD>::operator+=(Hypermap<T, nD>& hmap)
	{
		DIMCHECK;

		for (int i = 0; i < data_size; i++) data[i] += hmap.data[i];

		return *this;
	}

	template<class T, int nD>
	inline Hypermap<T, nD>& Hypermap<T, nD>::operator-=(Hypermap<T, nD>& hmap)
	{
		DIMCHECK;

		for (int i = 0; i < data_size; i++) data[i] -= hmap.data[i];

		return *this;
	}

	template<class T, int nD>
	inline Hypermap<T, nD>& Hypermap<T, nD>::operator*=(Hypermap<T, nD>& hmap)
	{
		DIMCHECK;

		for (int i = 0; i < data_size; i++) data[i] *= hmap.data[i];

		return *this;
	}

	template<class T, int nD>
	inline Hypermap<T, nD>& Hypermap<T, nD>::operator/=(Hypermap<T, nD>& hmap)
	{
		DIMCHECK;

		for (int i = 0; i < data_size; i++) data[i] /= hmap.data[i];

		return *this;
	}

	template<class T, int nD>
	inline size_t Hypermap<T, nD>::GetIdx(std::array<int, nD> ofCoord) const
	{
		int placeVal = data_size;
		int idx = 0;

		for (int i = 0; i < nD; i++)
		{
			const int& ptElem = ofCoord[i];
			const int& dimElem = dimensions[i];

			assert(ptElem > 0 & ptElem < dimensions[i]);

			placeVal /= dimElem;
			idx += placeVal * ptElem;
		}

		return idx;
	}

}