#pragma once

#include <iostream>

namespace zmath
{
	typedef struct Vec3
	{
		double X, Y, Z;

		Vec3();
		Vec3(double x, double y, double z);
		Vec3(double val);

		void Set(double x, double y, double z);
		void operator= (Vec3 v);

		double Dot(Vec3 v) const;
		double DistForm(Vec3 v) const;
		double DistForm() const;

		Vec3 Rotate(double thetaX, double thetaY, double thetaZ) const;
		Vec3 RotateX(double theta) const;
		Vec3 RotateY(double theta) const;
		Vec3 RotateZ(double theta) const;
		Vec3 Rotate(double thetaX, double thetaY, double thetaZ, Vec3 around) const;
		Vec3 RotateX(double theta, Vec3 around) const;
		Vec3 RotateY(double theta, Vec3 around) const;
		Vec3 RotateZ(double theta, Vec3 around) const;

		Vec3 operator+  (Vec3 v) const;
		Vec3 operator-  (Vec3 v) const;
		Vec3 operator/  (Vec3 v) const;
		Vec3 operator*  (Vec3 v) const;
		void operator+= (Vec3 v);
		void operator-= (Vec3 v);
		void operator*= (Vec3 v);
		void operator/= (Vec3 v);

		Vec3 operator+  (double val) const;
		Vec3 operator-  (double val) const;
		Vec3 operator/  (double val) const;
		Vec3 operator*  (double val) const;
		void operator+= (double val);
		void operator-= (double val);
		void operator*= (double val);
		void operator/= (double val);

		bool operator== (Vec3 v) const;
		bool operator!= (Vec3 v) const;
		bool operator<  (Vec3 v) const;
		bool operator>  (Vec3 v) const;
		bool operator<= (Vec3 v) const;
		bool operator>= (Vec3 v) const;

		static Vec3 Min(Vec3 v1, Vec3 v2);
		static Vec3 Max(Vec3 v1, Vec3 v2);

		friend std::ostream& operator<<(std::ostream& out, Vec3 v3);
		
	} Vec3;
}