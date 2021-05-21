#pragma once

#include "Triangle3D.h"
#include "Map.h"

#include <vector>
#include <array>
#include <fstream>

namespace zmath
{
	/* Class Tesselation3D:
	* This class was an experiment with creating a 3D representation of a shape with triangles,
	* such that it could easily be converted into an STL file. However, errors arise in scaling
	* and shifting shapes with this class and its presets upon conversion to STL. So, while this
	* class remains helpful for its STL encoding, all of its static presets should be considered
	* deprecated, and the class '' is preferred for its gap-less assignment of triangle
	* vertices and therefore (mostly) error-less STL encoding.
	*/
	class Tessellation3D {
	public:
		Tessellation3D() noexcept;
		Tessellation3D(const Tessellation3D& tess) noexcept;
		Tessellation3D(Tessellation3D&& tess) noexcept;
		Tessellation3D(Triangle3D* triangles, int numTri);
		Tessellation3D(double* vertices, int numTri);
		Tessellation3D(Map map, Vec3 scale = Vec3(1, 1, 1), bool fillSides = true, bool fillBase = true);

		void Print() const;
		std::array<Vec3, 2> Bounds() const;

		Tessellation3D& Add(Triangle3D tri);
		Tessellation3D& Add(Tessellation3D shape);
		Tessellation3D& Add(Vec3 v1, Vec3 v2, Vec3 v3);

		Tessellation3D& Shift(Vec3 by);
		Tessellation3D& Shift(double x, double y, double z);
		Tessellation3D& Rotate(double thetaX, double thetaY, double thetaZ, Vec3 around = Vec3());
		Tessellation3D& Scale(double by, Vec3 around = Vec3());
		Tessellation3D& Scale(double scaleX, double scaleY, double scaleZ, Vec3 around = Vec3());

		// This copies by value the entire dataset from one shape to another, so also a potentially expensive operation
		Tessellation3D& operator=(Tessellation3D& shape) noexcept;
		Tessellation3D& operator=(Tessellation3D&& shape) noexcept;

		// Write data to STL file
		void WriteSTL(std::ofstream& f, bool normals, int beginning = 0, int end = 0) const;

		//         //
		// PRESETS //
		//         //

		// Square returns a square in the XY plane centered around the passed point, or around (0, 0) by default
		static Tessellation3D Square(double size, Vec3 center = Vec3());
		static Tessellation3D Rectangle(double sizeX, double sizeY, Vec3 center = Vec3());
		static Tessellation3D NGon(int sides, double radius, Vec3 center = Vec3());

		static Tessellation3D Cube(double size, Vec3 center = Vec3());
		static Tessellation3D Prism(int baseSides, double radius, double height, Vec3 center = Vec3());
		static Tessellation3D Pyramid(int baseSides, double radius, double height, Vec3 center = Vec3());

		static Tessellation3D Sphere(int resolution, double radius, Vec3 center = Vec3());

		//             //
		// STL WRITING //
		//             //

		static void WriteVertex(std::ofstream& f, const Vec3& v);

		static Tessellation3D LoadSTL(std::ifstream& f);

	private:
		std::vector<Triangle3D> data;
	};
}