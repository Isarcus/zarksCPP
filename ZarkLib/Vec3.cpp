#include "pch.h"
#include "Vec3.h"

#include "Mat3.h"
#include "zmath_internals.h"

#include <cmath>

namespace zmath
{
	Vec3::Vec3()
		: X(0)
		, Y(0)
		, Z(0)
	{}

	Vec3::Vec3(double x, double y, double z)
		: X(x)
		, Y(y)
		, Z(z)
	{}

	Vec3::Vec3(double val)
		: X(val)
		, Y(val)
		, Z(val)
	{}

	void Vec3::Set(double x, double y, double z)
	{
		X = x;
		Y = y;
		Z = z;
	}

	void Vec3::operator=(Vec3 v)
	{
		X = v.X;
		Y = v.Y;
		Z = v.Z;
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

	void Vec3::operator+=(Vec3 v) { *this = *this + v; }
	void Vec3::operator-=(Vec3 v) { *this = *this - v; }
	void Vec3::operator*=(Vec3 v) { *this = *this * v; }
	void Vec3::operator/=(Vec3 v) { *this = *this / v; }

	Vec3 Vec3::operator+(double val) const { return *this + Vec3(val); }
	Vec3 Vec3::operator-(double val) const { return *this - Vec3(val); }
	Vec3 Vec3::operator*(double val) const { return *this * Vec3(val); }
	Vec3 Vec3::operator/(double val) const { return *this / Vec3(val); }

	void Vec3::operator+=(double val) { *this += Vec3(val); }
	void Vec3::operator-=(double val) { *this -= Vec3(val); }
	void Vec3::operator*=(double val) { *this *= Vec3(val); }
	void Vec3::operator/=(double val) { *this /= Vec3(val); }

	bool Vec3::operator==(Vec3 v) const { return X == v.X && Y == v.Y && Z == v.Z; }
	bool Vec3::operator!=(Vec3 v) const { return X != v.X || Y != v.Y || Z != v.Z; }
	bool Vec3::operator< (Vec3 v) const { return X <  v.X && Y <  v.Y && Z <  v.Z; }
	bool Vec3::operator> (Vec3 v) const { return X >  v.X && Y >  v.Y && Z >  v.Z; }
	bool Vec3::operator<=(Vec3 v) const { return X <= v.X && Y <= v.Y && Z <= v.Z; }
	bool Vec3::operator>=(Vec3 v) const { return X >= v.X && Y >= v.Y && Z >= v.Z; }

	Vec3 Vec3::Min(Vec3 v1, Vec3 v2)
	{
		return Vec3(
			(v1.X < v2.X) ? v1.X : v2.X,
			(v1.Y < v2.Y) ? v1.Y : v2.Y,
			(v1.Z < v2.Z) ? v1.Z : v2.Z
		);
	}

	Vec3 Vec3::Max(Vec3 v1, Vec3 v2)
	{
		return Vec3(
			(v1.X > v2.X) ? v1.X : v2.X,
			(v1.Y > v2.Y) ? v1.Y : v2.Y,
			(v1.Z > v2.Z) ? v1.Z : v2.Z
		);
	}

	std::ostream& operator<<(std::ostream& out, Vec3 v3)
	{
		out << "(" << v3.X << ", " << v3.Y << ", " << v3.Z << ")";
		return out;
	}

}

