#pragma once

#include <ostream>
#include <iostream>
#include <cmath>

namespace zmath
{
	// forward declare VecT
	template <typename>
	struct VecT;

	// forward declare operator<<
	template <typename T>
	std::ostream& operator<<(std::ostream&, const VecT<T>&);

	template <typename T>
	struct VecT
	{
		T X, Y;

		VecT();
		VecT(T x, T y);
		VecT(const VecT& vec) = default;
		VecT(VecT&& vec) = default;

		T Min() const;
		T Max() const;
		T Sum() const;
		T Area() const;

		T Dot(VecT v) const;
		T DistForm(VecT v) const;
		T DistForm() const;
		T DistManhattan(VecT v) const;
		T LNorm(double L) const;
		double Angle() const;

		VecT Mod(VecT denom) const;
		VecT Floor() const;
		VecT Ceil() const;
		VecT Abs() const;

		VecT& operator=  (const VecT& v);
		VecT& operator=  (VecT&& v);
		bool operator!() const;

		VecT operator+  (VecT v) const;
		VecT operator-  (VecT v) const;
		VecT operator*  (VecT v) const;
		VecT operator/  (VecT v) const;
		VecT& operator+= (VecT v);
		VecT& operator-= (VecT v);
		VecT& operator*= (VecT v);
		VecT& operator/= (VecT v);

		VecT operator+  (T val) const;
		VecT operator-  (T val) const;
		VecT operator*  (T val) const;
		VecT operator/  (T val) const;
		VecT& operator+= (T val);
		VecT& operator-= (T val);
		VecT& operator*= (T val);
		VecT& operator/= (T val);

		bool operator== (VecT v) const;
		bool operator!= (VecT v) const;
		bool operator<  (VecT v) const;
		bool operator>  (VecT v) const;
		bool operator<= (VecT v) const;
		bool operator>= (VecT v) const;

		friend std::ostream& operator<< <>(std::ostream& out, const VecT<T>& v);

		static VecT Min(VecT v1, VecT v2);
		static VecT Max(VecT v1, VecT v2);
		static VecT UnitVector(double angle, double magnitude = 1.0);

		template <typename W>
		operator VecT<W>() const;
	};

	typedef VecT<int> VecInt;
	typedef VecT<double> Vec;
} // namespace zmath

//                //
// Implementation //
//                //

namespace zmath
{

template <typename T>
inline VecT<T>::VecT()
	: X(0)
	, Y(0)
{}

template <typename T>
inline VecT<T>::VecT(T x, T y)
	: X(x)
	, Y(y)
{}

template <typename T>
inline T VecT<T>::Min() const
{
	return std::min(X, Y);
}

template <typename T>
inline T VecT<T>::Max() const
{
	return std::max(X, Y);
}

template <typename T>
inline T VecT<T>::Sum() const
{
	return X + Y;
}

template <typename T>
inline T VecT<T>::Area() const
{
	return X * Y;
}

template <typename T>
inline T VecT<T>::Dot(VecT v) const
{
	return X * v.X + Y * v.Y;
}

template <typename T>
inline T VecT<T>::DistForm(VecT v) const
{
	return std::sqrt(std::pow(X - v.X, 2) +
		             std::pow(Y - v.Y, 2));
}

template <typename T>
inline T VecT<T>::DistForm() const
{
	return DistForm(VecT<T>());
}

template <typename T>
inline T VecT<T>::DistManhattan(VecT v) const
{
	return std::abs(X - v.X) + std::abs(Y - v.Y);
}

template <typename T>
inline T VecT<T>::LNorm(double L) const
{
	if (L == 1) return std::abs(X) + std::abs(Y);
	else if (L == 2) return DistForm();

	return std::pow(std::pow(std::abs(X), L) + std::pow(std::abs(Y), L), 1.0 / L);
}

template <typename T>
inline double VecT<T>::Angle() const
{
	return std::atan2(Y, X);
}

template <typename T>
inline VecT<T> VecT<T>::Mod(VecT denom) const
{
	return VecT(
		std::fmod(X, denom.X),
		std::fmod(Y, denom.Y)
	);
}

template <typename T>
inline VecT<T> VecT<T>::Floor() const
{
	return VecT<T>(std::floor(X),
		std::floor(Y));
}

template <typename T>
inline VecT<T> VecT<T>::Ceil() const
{
	return VecT<T>(std::ceil(X),
		std::ceil(Y));
}

template <typename T>
inline VecT<T> VecT<T>::Abs() const
{
	return VecT<T>(std::abs(X),
		std::abs(Y));
}

template <typename T>
inline VecT<T>& VecT<T>::operator=(const VecT& v)
{
	X = v.X;
	Y = v.Y;

	return *this;
}

template <typename T>
inline VecT<T>& VecT<T>::operator=(VecT&& v)
{
	return *this = v;
}

template <typename T>
inline bool VecT<T>::operator!() const
{
	return !(X || Y);
}

template <typename T>
inline VecT<T> VecT<T>::operator+(VecT v) const
{
	return VecT(X + v.X,
		Y + v.Y);
}

template <typename T>
inline VecT<T> VecT<T>::operator-(VecT v) const
{
	return VecT(X - v.X,
		Y - v.Y);
}

template <typename T>
inline VecT<T> VecT<T>::operator*(VecT v) const
{
	return VecT(X * v.X,
		Y * v.Y);
}

template <typename T>
inline VecT<T> VecT<T>::operator/(VecT v) const
{
	return VecT(X / v.X,
		Y / v.Y);
}

template <typename T>
inline VecT<T>& VecT<T>::operator+=(VecT v)
{
	X += v.X;
	Y += v.Y;
	return *this;
}

template <typename T>
inline VecT<T>& VecT<T>::operator-=(VecT v)
{
	X -= v.X;
	Y -= v.Y;
	return *this;
}

template <typename T>
inline VecT<T>& VecT<T>::operator*=(VecT v)
{
	X *= v.X;
	Y *= v.Y;
	return *this;
}

template <typename T>
inline VecT<T>& VecT<T>::operator/=(VecT v)
{
	X /= v.X;
	Y /= v.Y;
	return *this;
}

template <typename T>
inline VecT<T> VecT<T>::operator+(T val) const { return *this + VecT(val, val); }

template <typename T>
inline VecT<T> VecT<T>::operator-(T val) const { return *this - VecT(val, val); }

template <typename T>
inline VecT<T> VecT<T>::operator*(T val) const { return *this * VecT(val, val); }

template <typename T>
inline VecT<T> VecT<T>::operator/(T val) const { return *this / VecT(val, val); }

template <typename T>
inline VecT<T>& VecT<T>::operator+=(T val) { return *this += VecT(val, val); }

template <typename T>
inline VecT<T>& VecT<T>::operator-=(T val) { return *this -= VecT(val, val); }

template <typename T>
inline VecT<T>& VecT<T>::operator*=(T val) { return *this *= VecT(val, val); }

template <typename T>
inline VecT<T>& VecT<T>::operator/=(T val) { return *this /= VecT(val, val); }

template <typename T>
inline bool VecT<T>::operator==(VecT v) const { return X == v.X && Y == v.Y; }

template <typename T>
inline bool VecT<T>::operator!=(VecT v) const { return X != v.X || Y != v.Y; }

template <typename T>
inline bool VecT<T>::operator<(VecT v) const { return X < v.X && Y < v.Y; }

template <typename T>
inline bool VecT<T>::operator>(VecT v) const { return X > v.X && Y > v.Y; }

template <typename T>
inline bool VecT<T>::operator<=(VecT v) const { return X <= v.X && Y <= v.Y; }

template <typename T>
inline bool VecT<T>::operator>=(VecT v) const { return X >= v.X && Y >= v.Y; }

template <typename T>
inline VecT<T> VecT<T>::Min(VecT v1, VecT v2)
{
	return VecT(
		std::min(v1.X, v2.X),
		std::min(v1.Y, v2.Y)
	);
}

template <typename T>
inline VecT<T> VecT<T>::Max(VecT v1, VecT v2)
{
	return VecT(
		std::max(v1.X, v2.X),
		std::max(v1.Y, v2.Y)
	);
}

template <typename T>
inline VecT<T> VecT<T>::UnitVector(double angle, double magnitude)
{
	return VecT (
		std::cos(angle),
		std::sin(angle)
	) * magnitude;
}

template <typename T>
template<typename W>
inline VecT<T>::operator VecT<W>() const
{
	return VecT<W>((W)X, (W)Y);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const VecT<T>& vec)
{
	return os << "(" << vec.X << ", " << vec.Y << ")";
}

} // namespace zmath
