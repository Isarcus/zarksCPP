#include "pch.h"
#include "Vec3.h"

#include "Mat3.h"
#include "zmath_internals.h"

#include <cmath>

namespace zmath
{
	Vec3::Vec3() noexcept
		: X(0)
		, Y(0)
		, Z(0)
	{}

	Vec3::Vec3(const Vec3& v) noexcept
		: X(v.X)
		, Y(v.Y)
		, Z(v.Z)
	{ }

	Vec3::Vec3(Vec3 && v) noexcept
		: X(v.X)
		, Y(v.Y)
		, Z(v.Z)
	{ }

	Vec3::Vec3(double x, double y, double z) noexcept
		: X(x)
		, Y(y)
		, Z(z)
	{}

	Vec3::Vec3(double val) noexcept
		: X(val)
		, Y(val)
		, Z(val)
	{}

	Vec3& Vec3::Set(double x, double y, double z)
	{
		X = x;
		Y = y;
		Z = z;
		return *this;
	}

	Vec3& Vec3::operator=(const Vec3& v) noexcept
	{
		X = v.X;
		Y = v.Y;
		Z = v.Z;
		return *this;
	}

	Vec3& Vec3::operator=(Vec3&& v) noexcept
	{
		X = v.X;
		Y = v.Y;
		Z = v.Z;
		return *this;
	}

	double Vec3::Dot(Vec3 v) const
	{
		return X*v.X + Y*v.Y + Z*v.Z;
	}

	double Vec3::DistForm(Vec3 v) const
	{
		return std::sqrt(std::pow(X - v.X, 2) + std::pow(Y - v.Y, 2) + std::pow(Z - v.Z, 2));
	}

	double Vec3::DistForm() const
	{
		return std::sqrt(X*X + Y*Y + Z*Z);
	}

	Vec3 Vec3::Rotate(double thetaX, double thetaY, double thetaZ) const
	{
		return Mat3::Rotation(thetaX, thetaY, thetaZ) * (*this);
	}

	Vec3 Vec3::RotateX(double theta) const
	{
		return Mat3::RotationX(theta) * (*this);
	}

	Vec3 Vec3::RotateY(double theta) const
	{
		return Mat3::RotationY(theta) * (*this);
	}

	Vec3 Vec3::RotateZ(double theta) const
	{
		return Mat3::RotationZ(theta) * (*this);
	}

	Vec3 Vec3::Rotate(double thetaX, double thetaY, double thetaZ, Vec3 around) const
	{
		return around + (*this - around).Rotate(thetaX, thetaY, thetaZ);
	}

	Vec3 Vec3::RotateX(double theta, Vec3 around) const
	{
		return around + (*this - around).RotateX(theta);
	}

	Vec3 Vec3::RotateY(double theta, Vec3 around) const
	{
		return around + (*this - around).RotateY(theta);
	}

	Vec3 Vec3::RotateZ(double theta, Vec3 around) const
	{
		return around + (*this - around).RotateZ(theta);
	}

	Vec3 Vec3::Scale(double by, Vec3 around) const
	{
		return (*this) + ((*this) - around) * (by - 1);
	}

	Vec3 Vec3::Scale(double scaleX, double scaleY, double scaleZ, Vec3 around) const
	{
		return Vec3(
			X + (X - around.X) * (scaleX - 1),
			Y + (Y - around.Y) * (scaleY - 1),
			Z + (Z - around.Z) * (scaleZ - 1)
		);
	}

	Vec3 Vec3::operator+(Vec3 v) const { return Vec3(X + v.X, Y + v.Y, Z + v.Z); }
	Vec3 Vec3::operator-(Vec3 v) const { return Vec3(X - v.X, Y - v.Y, Z - v.Z); }
	Vec3 Vec3::operator*(Vec3 v) const { return Vec3(X * v.X, Y * v.Y, Z * v.Z); }
	Vec3 Vec3::operator/(Vec3 v) const
	{
		double x, y, z;
		
		if (v.X == 0) x = (X == 0) ? 0 : (X < 0) ? DOUBLEMIN : DOUBLEMAX; else x = X / v.X; // sorry haha
		if (v.Y == 0) y = (Y == 0) ? 0 : (Y < 0) ? DOUBLEMIN : DOUBLEMAX; else y = Y / v.Y;
		if (v.Z == 0) z = (Z == 0) ? 0 : (Z < 0) ? DOUBLEMIN : DOUBLEMAX; else z = Z / v.Z;

		return Vec3(x, y, z);
	}

	Vec3& Vec3::operator+=(Vec3 v) { return (*this = *this + v, *this); }
	Vec3& Vec3::operator-=(Vec3 v) { return (*this = *this - v, *this); }
	Vec3& Vec3::operator*=(Vec3 v) { return (*this = *this * v, *this); }
	Vec3& Vec3::operator/=(Vec3 v) { return (*this = *this / v, *this); }

	Vec3 Vec3::operator+(double val) const { return *this + Vec3(val); }
	Vec3 Vec3::operator-(double val) const { return *this - Vec3(val); }
	Vec3 Vec3::operator*(double val) const { return *this * Vec3(val); }
	Vec3 Vec3::operator/(double val) const { return *this / Vec3(val); }

	Vec3& Vec3::operator+=(double val) { return *this += Vec3(val); }
	Vec3& Vec3::operator-=(double val) { return *this -= Vec3(val); }
	Vec3& Vec3::operator*=(double val) { return *this *= Vec3(val); }
	Vec3& Vec3::operator/=(double val) { return *this /= Vec3(val); }

	bool Vec3::operator==(Vec3 v) const { return X == v.X && Y == v.Y && Z == v.Z; }
	bool Vec3::operator!=(Vec3 v) const { return X != v.X || Y != v.Y || Z != v.Z; }
	bool Vec3::operator< (Vec3 v) const { return X <  v.X && Y <  v.Y && Z <  v.Z; }
	bool Vec3::operator> (Vec3 v) const { return X >  v.X && Y >  v.Y && Z >  v.Z; }
	bool Vec3::operator<=(Vec3 v) const { return X <= v.X && Y <= v.Y && Z <= v.Z; }
	bool Vec3::operator>=(Vec3 v) const { return X >= v.X && Y >= v.Y && Z >= v.Z; }

	Vec3 Vec3::Min(const Vec3& v1, const Vec3& v2)
	{
		return Vec3(
			std::min(v1.X, v2.X),
			std::min(v1.Y, v2.Y),
			std::min(v1.Z, v2.Z)
		);
	}

	Vec3 Vec3::Max(const Vec3& v1, const Vec3& v2)
	{
		return Vec3(
			std::max(v1.X, v2.X),
			std::max(v1.Y, v2.Y),
			std::max(v1.Z, v2.Z)
		);
	}

	Vec3 Vec3::Min(const Vec3& v1, Vec3&& v2)
	{
		// this is OK!! It calls the (Vec3&, Vec3&) Min function, instead of infinite recursion
		return Min(v1, v2);
	}

	Vec3 Vec3::Max(const Vec3& v1, Vec3&& v2)
	{
		return Max(v1, v2);
	}

	std::ostream& operator<<(std::ostream& out, Vec3 v3)
	{
		out << "(" << v3.X << ", " << v3.Y << ", " << v3.Z << ")";
		return out;
	}

}

