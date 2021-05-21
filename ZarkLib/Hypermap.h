#pragma once

#include <array>

namespace zmath
{
	template <class T, int nD>
	class Hypermap {
	public:
		Hypermap(std::array<int, nD> dimensions, T initVal = T());
		~Hypermap();

		// Accessors
		T& At(std::array<int, nD> coord);
		T& operator[] (std::array<int, nD> coord);

		// Data characteristics & manipulation
		T GetMin() const;
		T GetMax() const;
		std::array<T, 2> GetMinMax();
		std::array<int, nD> Dimensions();

		T Sum();
		T Mean();

		// Operators

		Hypermap<T, nD>& operator= (Hypermap<T, nD>& hmap);

		Hypermap<T, nD>& operator+= (Hypermap<T, nD>& hmap);
		Hypermap<T, nD>& operator-= (Hypermap<T, nD>& hmap);
		Hypermap<T, nD>& operator*= (Hypermap<T, nD>& hmap);
		Hypermap<T, nD>& operator/= (Hypermap<T, nD>& hmap);

	private:
		std::array<int, nD> dimensions;
		size_t data_size;
		T* data;
		
		T& At(int idx);
		T& operator[] (int idx);
		size_t GetIdx(std::array<int, nD> ofCoord) const;
	};
}

#define HYPERMAP_INL
#include "Hypermap.inl"
#undef HYPERMAP_INL
