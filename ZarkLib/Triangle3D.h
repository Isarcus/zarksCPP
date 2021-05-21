#pragma once

#include "Vec3.h"

namespace zmath
{
	typedef struct Triangle3D {
		Vec3 vertices[3];

		Triangle3D(Vec3 v1, Vec3 v2, Vec3 v3);

		Triangle3D Set(Vec3 v1, Vec3 v2, Vec3 v3);
		Triangle3D operator=(Triangle3D tri);
		
		Vec3 Centroid() const;
		Vec3 Min() const;
		Vec3 Max() const;
		double MinX() const;
		double MinY() const;
		double MinZ() const;
		double MaxX() const;
		double MaxY() const;
		double MaxZ() const;

		Triangle3D Shift(Vec3 by) const;
		Triangle3D Rotate(double thetaX, double thetaY, double thetaZ, Vec3 around = Vec3()) const;
		Triangle3D RotateX(double theta, Vec3 around = Vec3()) const;
		Triangle3D RotateY(double theta, Vec3 around = Vec3()) const;
		Triangle3D RotateZ(double theta, Vec3 around = Vec3()) const;

		friend std::ostream& operator<<(std::ostream& out, Triangle3D tri);
	} Triangle3D;
}
