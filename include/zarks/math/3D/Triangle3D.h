#pragma once

#include <zarks/math/3D/Vec3.h>
#include <array>

namespace zmath
{
	typedef struct Triangle3D {
		std::array<Vec3, 3> vertices;

		Triangle3D() = default;
		Triangle3D(Vec3 v1, Vec3 v2, Vec3 v3);
		Triangle3D(const Triangle3D& tri) = default;
		Triangle3D(Triangle3D&& tri) = default;

		Triangle3D& operator=(const Triangle3D&) = default;
		Triangle3D& operator=(Triangle3D&&) = default;

		Triangle3D Set(Vec3 v1, Vec3 v2, Vec3 v3);
		
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

		friend std::ostream& operator<<(std::ostream& out, const Triangle3D& tri);
	} Triangle3D;
}
