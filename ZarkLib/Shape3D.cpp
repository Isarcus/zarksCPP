#include "pch.h"
#include "Shape3D.h"
#include "binary.h"
#include "zmath_internals.h"

namespace zmath
{
	Shape3D::Shape3D() {}

	Shape3D::Shape3D(double* vertices, int numTri)
	{
		int idx = 0;
		for (int i = 0; i < numTri; i++)
		{
			data.push_back(Triangle3D(
				vertices[idx++],
				vertices[idx++],
				vertices[idx++]
			));
		}
	}

	Shape3D::Shape3D(Map map, Vec3 scale, bool fillSides, bool fillBase)
	{
		Vec bounds = map.Bounds();

		// First the heightmap data
		for (int x = 0; x < bounds.X - 1; x++)
		{
			for (int y = 0; y < bounds.Y - 1; y++)
			{
				Vec3 baseCoord(x, y, map[x][y]);
				Vec3 topCoord(x + 1, y + 1, map[x + 1][y + 1]);
			}
		}
	}

	void Shape3D::Print() const
	{
		std::cout << "Printing Shape3D with " << data.size() << " triangles.\n";

		for (const Triangle3D& tri : data)
		{
			std::cout << tri.vertices[0] << "\n";
			std::cout << tri.vertices[1] << "\n";
			std::cout << tri.vertices[2] << "\n\n";
		}
	}

	Shape3D& Shape3D::Add(Triangle3D tri)
	{
		data.push_back(tri);

		return *this;
	}

	Shape3D& Shape3D::Add(Shape3D shape)
	{
		for (Triangle3D& tri : shape.data) data.push_back(tri);

		return *this;
	}

	Shape3D& Shape3D::Add(Vec3 v1, Vec3 v2, Vec3 v3)
	{
		data.push_back(Triangle3D(v1, v2, v3));

		return *this;
	}

	Shape3D::Shape3D(Triangle3D* triangles, int numTri)
	{
		for (int i = 0; i < numTri; i++)
		{
			data.push_back(triangles[i]);
		}
	}

	Shape3D& Shape3D::Shift(Vec3 by)
	{
		for (Triangle3D& tri : data) tri = tri.Shift(by);

		return *this;
	}

	Shape3D& Shape3D::Rotate(double thetaX, double thetaY, double thetaZ, Vec3 around)
	{
		for (Triangle3D& tri : data) tri = tri.Rotate(thetaX, thetaY, thetaZ, around);

		return *this;
	}

	Shape3D& Shape3D::Scale(double by, Vec3 around)
	{
		for (Triangle3D& tri : data)
		{
			for (Vec3& vertex : tri.vertices)
			{
				vertex = vertex.Scale(by, around);
			}
		}

		return *this;
	}

	Shape3D& Shape3D::Scale(double scaleX, double scaleY, double scaleZ, Vec3 around)
	{
		for (Triangle3D& tri : data)
		{
			for (Vec3& vertex : tri.vertices)
			{
				vertex = vertex.Scale(scaleX, scaleY, scaleZ, around);
			}
		}

		return *this;
	}

	Shape3D& Shape3D::operator=(Shape3D& shape)
	{
		data = shape.data;

		return *this;
	}

	void Shape3D::WriteSTL(std::ofstream& f) const
	{
		const char header[80]{ "ZarkLib STL file, generated from a Shape3D" };
		const char attrib[2] = { 0 };
		const char normvec[12] = { 0 };

		char triCt[4];
		ToBytes(triCt, (uint32_t)data.size(), Endian::Little);

		// Write header to file
		f.write(header, 80);

		// Write number of triangles to file
		f.write(triCt, 4);

		// Write triangle data to file
		char buf[4];
		for (const Triangle3D& tri : data)
		{
			// Write the (blank) normal vector
			f.write(normvec, 12);

			// Write each vertex
			for (const Vec3& vertex : tri.vertices)
			{
				ToBytes(buf, (float)vertex.X, Endian::Little);
				f.write(buf, 4);

				ToBytes(buf, (float)vertex.Y, Endian::Little);
				f.write(buf, 4);

				ToBytes(buf, (float)vertex.Z, Endian::Little);
				f.write(buf, 4);
			}

			// Write the (blank) attribute byte count
			f.write(attrib, 2);
		}
	}

	Shape3D Shape3D::Square(double size, Vec3 center)
	{
		double sizeD2 = size / 2;
		Shape3D square;

		/* 0 0 0 0 2
		*  0 0 0 / |
		*  0 0 / 0 |
		*  0 / 0 0 |
		*  1_______3
		*/
		square.Add(
			Vec3(center.X - sizeD2, center.Y - sizeD2, center.Z),
			Vec3(center.X + sizeD2, center.Y + sizeD2, center.Z),
			Vec3(center.X + sizeD2, center.Y - sizeD2, center.Z)
		);

		/* 3_______1
		*  | 0 0 / 0
		*  | 0 / 0 0
		*  | / 0 0 0
		*  2 0 0 0 3
		*/
		square.Add(
			Vec3(center.X + sizeD2, center.Y + sizeD2, center.Z),
			Vec3(center.X - sizeD2, center.Y - sizeD2, center.Z),
			Vec3(center.X - sizeD2, center.Y + sizeD2, center.Z)
		);

		return square;
	}

	Shape3D Shape3D::Rectangle(double sizeX, double sizeY, Vec3 center)
	{
		Shape3D rectangle;

		double sizeXD2 = sizeX / 2.0;
		double sizeYD2 = sizeY / 2.0;

		rectangle.Add(Triangle3D(
			center - Vec3(sizeXD2, sizeYD2, 0),
			center + Vec3(sizeXD2, sizeYD2, 0),
			center + Vec3(sizeXD2, 0,		0)
		));

		rectangle.Add(Triangle3D(
			center + Vec3(sizeXD2, sizeYD2, 0),
			center - Vec3(sizeXD2, sizeYD2, 0),
			center + Vec3(0,	   sizeYD2, 0)
		));

		return rectangle;
	}

	Shape3D Shape3D::NGon(int sides, double radius, Vec3 center)
	{
		if (sides < 3) return Shape3D();

		// Generate the base coordinates
		Vec3* baseCoords = new Vec3[sides];
		for (int i = 0; i < sides; i++)
		{
			double angle = 2.0 * ZM_PI * i / sides;

			baseCoords[i] = Vec3(
				std::cos(angle) * radius,
				std::sin(angle) * radius,
				0
			) + center;
		}

		Shape3D ngon;
		for (int i = 0; i < sides - 2; i++)
		{
			Vec3 v1, v2, v3;

			if (i % 2) // odd
			{
				v2 = baseCoords[(i / 2) + 1];
				v1 = baseCoords[sides - 1 - (i / 2)];
				v3 = baseCoords[(i / 2) + 2];
			}
			else // even
			{
				v1 = baseCoords[(i / 2) + 1];
				v2 = baseCoords[sides - 1 - (i / 2)];
				v3 = baseCoords[(sides - (i / 2)) % sides];
			}

			ngon.Add(Triangle3D(v1, v2, v3));
		}

		delete[] baseCoords;

		return ngon;
	}

	Shape3D Shape3D::Cube(double size, Vec3 center)
	{
		const double PI_D2 = 3.1415926535897932384626 / 2.0;
		
		double sizeD2 = size / 2.0;
		Shape3D cube;

		// Z axis perpendicular (XY plane)
		cube.Add(Square(size).Shift(center + Vec3(0, 0, -sizeD2)));
		cube.Add(Square(size).Shift(center + Vec3(0, 0, sizeD2)));
		
		// X axis perpendicular (YZ plane)
		cube.Add(Square(size).Rotate(0, -PI_D2, 0).Shift(center + Vec3(sizeD2,  0, 0)));
		cube.Add(Square(size).Rotate(0,  PI_D2, 0).Shift(center + Vec3(-sizeD2, 0, 0)));

		// Y axis perpendicular (XZ plane)
		cube.Add(Square(size).Rotate( PI_D2, 0, 0).Shift(center + Vec3(0, sizeD2, 0)));
		cube.Add(Square(size).Rotate(-PI_D2, 0, 0).Shift(center + Vec3(0, -sizeD2, 0)));

		return cube;
	}

	Shape3D Shape3D::Prism(int baseSides, double radius, double height, Vec3 center)
	{
		// Can't make prism if base isn't a shape
		if (baseSides < 3) return Shape3D();

		// Generate the base coordinates
		Vec3* baseCoords = new Vec3[baseSides];
		for (int i = 0; i < baseSides; i++)
		{
			double angle = 2.0 * ZM_PI * i / baseSides;

			baseCoords[i] = Vec3(
				std::cos(angle) * radius,
				std::sin(angle) * radius,
				0
			) + center;
		}

		// Generate the side triangles
		double heightD2 = height / 2.0;
		Shape3D prism;
		for (int i = 0; i < baseSides; i++)
		{
			prism.Add(Triangle3D(
				baseCoords[i] - Vec3(0, 0, heightD2),
				baseCoords[(i+1) % baseSides] - Vec3(0, 0, heightD2),
				baseCoords[(i+1) % baseSides] + Vec3(0, 0, heightD2)
			));

			prism.Add(Triangle3D(
				baseCoords[i] - Vec3(0, 0, heightD2),
				baseCoords[(i + 1) % baseSides] + Vec3(0, 0, heightD2),
				baseCoords[i] + Vec3(0, 0, heightD2)
			));
		}

		// Generate the base
		Shape3D base = NGon(baseSides, radius, center);
		base.Shift(Vec3(0, 0, -heightD2));
		prism.Add(base);

		base.Shift(Vec3(0, 0, height));
		prism.Add(base);

		delete[] baseCoords;

		return prism;
	}

}