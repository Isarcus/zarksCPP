#pragma once

#include <zarks/math/3D/Vec3.h>

#include <iostream>

namespace zmath
{
	class Mat3
	{
	public:
		Mat3();
		Mat3(double arr[3][3]);

		Mat3& operator=(const Mat3& mat3);
		Mat3& operator=(const double* const* arr);

		double* operator[](int row);
		const double* operator[](int row) const;

		Mat3 operator+(const Mat3& mat3) const;
		Mat3 operator-(const Mat3& mat3) const;
		Mat3 operator*(const Mat3& mat3) const;

		Mat3& operator+=(const Mat3& mat3);
		Mat3& operator-=(const Mat3& mat3);
		Mat3& operator*=(const Mat3& mat3);

		Vec3 operator*(const Vec3& v3);

		static Mat3 Rotation(double thetaX, double thetaY, double thetaZ);
		static Mat3 RotationX(double theta);
		static Mat3 RotationY(double theta);
		static Mat3 RotationZ(double theta);

		friend std::ostream& operator<<(std::ostream& out, const Mat3& mat3);

	private:
		double data[3][3];
	};
}
