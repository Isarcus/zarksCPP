#pragma once

#include <zarks/math/3D/Vec3.h>
#include <zarks/math/3D/Tessellation3D.h>
#include <zarks/math/VecT.h>

namespace zmath
{
	class Shape3D
	{
	public:
		Shape3D() noexcept;
		Shape3D(const Shape3D& shape) noexcept;
		Shape3D(std::vector<Vec3> vertices, std::vector<int> indices) noexcept;

		Shape3D& operator= (const Shape3D& shape) noexcept;
		Shape3D& operator= (Shape3D&& shape) noexcept;

		Shape3D& Add(const Shape3D& shape, bool verticesOnly = false);

		Shape3D& Shift(Vec3 by);
		Shape3D& Shift(double x, double y, double z);

		Shape3D& Rotate(double thetaX, double thetaY, double thetaZ, Vec3 around = Vec3());
		Shape3D& Rotate(Vec3 angles, Vec3 around = Vec3());
		Shape3D& Scale(double by, Vec3 around = Vec3());
		Shape3D& Scale(double scaleX, double scaleY, double scaleZ, Vec3 around = Vec3());

		//             //
		// STL-RELATED //
		//             //

		Shape3D& STLCleanup(bool scaleIfSmall = true);
		Tessellation3D Tessellate();

		//         //
		// PRESETS //
		//         //

		static Shape3D Polygon(int sides, double radius, Vec3 center);
		static Shape3D Polygon(const std::vector<Vec3> points2D, bool ccw = true);

		static Shape3D RectangularPrism(Vec3 min, Vec3 max);
		static Shape3D Cube(Vec3 min, double size);
		static Shape3D Prism(int sides, double radius, double height, Vec3 baseCenter);

		/* @param acceptOrder: Should be true iff vertices are supplied such that (0, 1, 2) create a counterclockwise triangle
		*  when viewed from the outside and (3) is behind that triangle. */
		static Shape3D TriangularPyramid(std::array<Vec3, 4> vertices, bool acceptOrder = false);

		static Shape3D Sphere(int resolution, double radius, Vec3 center = Vec3());

		static Shape3D Equation2D(Vec min, Vec max, double (*function)(double, double), bool sides = true, Vec resXY = Vec(1, 1));
		
	private:
		std::vector<Vec3> vertices;
		std::vector<int> indices;
	};
}