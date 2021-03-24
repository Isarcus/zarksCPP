#pragma once

#include "Triangle3D.h"

#include <vector>
#include <fstream>

namespace zmath
{
	class Shape3D {
	public:
		Shape3D();
		Shape3D(Triangle3D* triangles, int numTri);
		Shape3D(double* vertices, int numTri);

		Shape3D& Add(Triangle3D tri);
		Shape3D& Add(Shape3D shape);
		Shape3D& Add(Vec3 v1, Vec3 v2, Vec3 v3);

		// Shift is an expensive operation; only perform once all triangles are added!
		Shape3D& Shift(Vec3 by);

		// Rotate is a VERY expensive operation; only perform once all triangles are added!
		Shape3D& Rotate(double thetaX, double thetaY, double thetaZ, Vec3 around = Vec3());

		// This copies by value the entire dataset from one shape to another, so also a potentially expensive operation
		Shape3D& operator=(Shape3D& shape);

		// Write data to STL file
		void WriteSTL(std::ofstream& f) const;

		//         //
		// PRESETS //
		//         //

		// Square returns a square in the XY plane centered around the passed point, or around (0, 0) by default
		static Shape3D Square(double size, Vec3 center = Vec3());
		static Shape3D Rectangle(double sizeX, double sizeY, Vec3 center = Vec3());
		static Shape3D NGon(int sides, double radius, Vec3 center = Vec3());

		static Shape3D Cube(double size, Vec3 center = Vec3());
		static Shape3D Prism(int baseSides, double radius, double height, Vec3 center = Vec3());
		static Shape3D Pyramid(int baseSides, double radius, double height, Vec3 center = Vec3());

		static Shape3D Sphere(double radius, Vec3 center = Vec3());

	private:
		std::vector<Triangle3D> data;
	};
}