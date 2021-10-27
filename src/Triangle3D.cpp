#include <zarks/math/3D/Triangle3D.h>

namespace zmath
{

Triangle3D::Triangle3D(Vec3 v1, Vec3 v2, Vec3 v3) : vertices{ v1, v2, v3 } {}

Triangle3D Triangle3D::Set(Vec3 v1, Vec3 v2, Vec3 v3)
{
	vertices[0] = v1;
	vertices[1] = v2;
	vertices[2] = v3;
	return *this;
}

Vec3 Triangle3D::Centroid() const
{
	return (vertices[0] + vertices[1] + vertices[2]) / 3;
}

Vec3 Triangle3D::Min() const
{
	return Vec3::Min(vertices[0], Vec3::Min(vertices[1], vertices[2]));
}

Vec3 Triangle3D::Max() const
{
	return Vec3::Max(vertices[0], Vec3::Max(vertices[1], vertices[2]));
}

double Triangle3D::MinX() const
{
	return std::min(vertices[0].X, std::min(vertices[1].X, vertices[2].X));
}

double Triangle3D::MinY() const
{
	return std::min(vertices[0].Y, std::min(vertices[1].Y, vertices[2].Y));
}

double Triangle3D::MinZ() const
{
	return std::min(vertices[0].Z, std::min(vertices[1].X, vertices[2].X));
}

double Triangle3D::MaxX() const
{
	return std::max(vertices[0].X, std::max(vertices[1].X, vertices[2].X));
}

double Triangle3D::MaxY() const
{
	return std::max(vertices[0].Y, std::max(vertices[1].Y, vertices[2].Y));
}

double Triangle3D::MaxZ() const
{
	return std::max(vertices[0].Z, std::max(vertices[1].Z, vertices[2].Z));
}

Triangle3D Triangle3D::Shift(Vec3 by) const
{
	return Triangle3D(
		vertices[0] + by,
		vertices[1] + by,
		vertices[2] + by
	);
}

Triangle3D Triangle3D::Rotate(double thetaX, double thetaY, double thetaZ, Vec3 around) const
{
	return Triangle3D(
		vertices[0].Rotate(thetaX, thetaY, thetaZ, around),
		vertices[1].Rotate(thetaX, thetaY, thetaZ, around),
		vertices[2].Rotate(thetaX, thetaY, thetaZ, around)
	);
}

Triangle3D Triangle3D::RotateX(double theta, Vec3 around) const
{
	return Triangle3D(
		vertices[0].RotateX(theta, around),
		vertices[1].RotateX(theta, around),
		vertices[2].RotateX(theta, around)
	);
}

Triangle3D Triangle3D::RotateY(double theta, Vec3 around) const
{
	return Triangle3D(
		vertices[0].RotateY(theta, around),
		vertices[1].RotateY(theta, around),
		vertices[2].RotateY(theta, around)
	);
}

Triangle3D Triangle3D::RotateZ(double theta, Vec3 around) const
{
	return Triangle3D(
		vertices[0].RotateZ(theta, around),
		vertices[1].RotateZ(theta, around),
		vertices[2].RotateZ(theta, around)
	);
}

std::ostream& operator<<(std::ostream& out, const Triangle3D& tri)
{
	return out
		<< "{ " << tri.vertices[0]
		<< "  " << tri.vertices[1]
		<< "  " << tri.vertices[2] << " }";
}

} // namespace zmath
