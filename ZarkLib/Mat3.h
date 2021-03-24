#pragma once

#include <iostream>

namespace zmath
{
	typedef struct Vec3 Vec3;

	class Mat3
	{
	public:
		Mat3();
		Mat3(double arr[3][3]);

		Mat3 operator=(Mat3 mat3);
		Mat3 operator=(double** arr);

		double* operator[](int row);

		Mat3 operator+(Mat3 mat3) const;
		Mat3 operator-(Mat3 mat3) const;
		Mat3 operator*(Mat3 mat3) const;

		Mat3& operator+=(Mat3 mat3);
		Mat3& operator-=(Mat3 mat3);
		Mat3& operator*=(Mat3 mat3);

		Vec3 operator*(Vec3 v3);

		static Mat3 Rotation(double thetaX, double thetaY, double thetaZ);
		static Mat3 RotationX(double theta);
		static Mat3 RotationY(double theta);
		static Mat3 RotationZ(double theta);

		friend std::ostream& operator<<(std::ostream& out, Mat3 mat3);

	private:
		double data[3][3];
	};
}
