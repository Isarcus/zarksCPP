#pragma once

#include <limits>
#include <array>
#include <cmath>

#include <zarks/image/color.h>

namespace zmath
{
	constexpr double PI = 3.14159265358979323846264;
	constexpr double PID2 = PI / 2.0;
	constexpr double DOUBLEMAX = std::numeric_limits<double>::max();
	constexpr double DOUBLEMIN = std::numeric_limits<double>::lowest();

	//              //
	// DECLARATIONS //
	//              //

	template <typename T>
	class VecT;

	//       //
	// LISTS //
	//       //

	template <typename T>
	T DistForm(T elems...)
	{
		T sum = 0;
		for (const T& elem : elems)
		{
			sum += std::pow(elem, 2);
		}
		return std::sqrt(sum);
	}

	template <typename T, int N>
	T DistForm(std::array<T, N> arr)
	{
		T sum = 0;
		for (const T& elem : arr)
		{
			sum += std::pow(elem, 2);
		}
		return std::sqrt(sum);
	}

	//        //
	// MEMORY //
	//        //

	template <typename T>
	T** alloc2d(int width, int height, const T& fill = T())
	{
		T** data = new T * [width];
		for (int x = 0; x < width; x++)
		{
			data[x] = new T[height];
			for (int y = 0; y < height; y++)
			{
				data[x][y] = fill;
			}
		}

		return data;
	}

	template <typename T>
	void free2d(T**& data, int width)
	{
		if (data)
		{
			for (int x = 0; x < width; x++)
			{
				delete[] data[x];
			}
			delete[] data;

			data = nullptr;
		}
	}

	//       //
	// ARRAY //
	//       //

	template <typename T, int N>
	std::array<T, N>& operator/=(std::array<T, N>& arr1, const std::array<T, N>& arr2)
	{
		for (int i = 0; i < N; i++)
		{
			arr1[i] /= arr2[i];
		}
		return arr1;
	}

	template <typename T, int N>
	std::array<T, N>& operator/=(std::array<T, N>& arr1, double div)
	{
		for (int i = 0; i < N; i++)
		{
			arr1[i] /= div;
		}
		return arr1;
	}

	template <typename T>
	void reverse(T& arr)
	{
		const size_t size = arr.size();
		for (size_t i = 0; i < size / 2; i++)
		{
			const auto temp = std::move(arr[i]);
			arr[i] = arr[size - 1 - i];
			arr[size - 1 - i] = temp;
		}
	}

	//      //
	// MATH //
	//      //

	inline double interpLinear(double val0, double val1, double t)
	{
		return t * val1 + (1.0 - t) * val0;
	}

	inline double interp5(double val0, double val1, double t)
	{
		double t_adj = 6 * std::pow(t, 5) - 15 * std::pow(t, 4) + 10 * std::pow(t, 3);
		return interpLinear(val0, val1, t_adj);
	}

	inline RGBA interp5(RGBA val0, RGBA val1, double t)
	{
		double t_adj = 6 * std::pow(t, 5) - 15 * std::pow(t, 4) + 10 * std::pow(t, 3);
		return RGBA(
			std::round(interpLinear(val0.R, val1.R, t_adj)),
			std::round(interpLinear(val0.G, val1.G, t_adj)),
			std::round(interpLinear(val0.B, val1.B, t_adj)),
			std::round(interpLinear(val0.A, val1.A, t_adj))
		);
	}

	template <typename T>
	T AbsT(const T& val)
	{
		return std::abs(val);
	}

	template <typename T>
	VecT<T> AbsT(const VecT<T>& val)
	{
		return val.Abs();
	}
}
