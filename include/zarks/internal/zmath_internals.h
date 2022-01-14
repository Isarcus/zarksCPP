#pragma once

#include <limits>
#include <array>
#include <cmath>

namespace zmath
{

constexpr double PI = 3.14159265358979323846264;
constexpr double PIX2 = PI * 2.0;
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

template <typename T, int N>
constexpr T DistForm(const std::array<T, N>& arr)
{
	T sum = 0;
	for (const T& elem : arr)
	{
		sum += std::pow(elem, 2);
	}
	return std::sqrt(sum);
}

//      //
// MATH //
//      //

constexpr double interpLinear(double val0, double val1, double t)
{
	return t * val1 + (1.0 - t) * val0;
}

template <typename T>
T interpLinear(const T& val0, const T& val1, double t)
{
	return T::Interpolate(val0, val1, t);
}

constexpr double interp5(double t)
{
	return 6 * std::pow(t, 5) - 15 * std::pow(t, 4) + 10 * std::pow(t, 3);
}

template <typename T>
T interp5(const T& val0, const T& val1, double t)
{
	return interpLinear(val0, val1, interp5(t));
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

} // namespace zmath
