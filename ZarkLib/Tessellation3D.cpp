#include "pch.h"
#include "Tessellation3D.h"
#include "binary.h"
#include "zmath_internals.h"
#include "numerals.h";

namespace zmath
{
	Tessellation3D::Tessellation3D() noexcept {}
	Tessellation3D::Tessellation3D(const Tessellation3D& tess) noexcept : data(tess.data) {}
	Tessellation3D::Tessellation3D(const Tessellation3D&& tess) noexcept : data(tess.data) {}

	Tessellation3D::Tessellation3D(double* vertices, int numTri)
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

	Tessellation3D::Tessellation3D(Map map, Vec3 scale, bool fillSides, bool fillBase)
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

	void Tessellation3D::Print() const
	{
		std::cout << "Printing Shape3D with " << data.size() << " triangles.\n";

		for (const Triangle3D& tri : data)
		{
			std::cout << tri.vertices[0] << "\n";
			std::cout << tri.vertices[1] << "\n";
			std::cout << tri.vertices[2] << "\n\n";
		}
	}

	std::array<Vec3, 2> Tessellation3D::Bounds() const
	{
		if (!data.size()) return {};

		std::array<Vec3, 2> arr = { data[0].vertices[0], data[0].vertices[0] };

		for (const Triangle3D& tri : data)
		{
			for (const Vec3& v : tri.vertices)
			{
				arr[0] = Vec3::Min(arr[0], v);
				arr[1] = Vec3::Max(arr[1], v);
			}
		}

		return arr;
	}

	Tessellation3D& Tessellation3D::Add(Triangle3D tri)
	{
		data.push_back(tri);

		return *this;
	}

	Tessellation3D& Tessellation3D::Add(Tessellation3D shape)
	{
		for (Triangle3D& tri : shape.data) data.push_back(tri);

		return *this;
	}

	Tessellation3D& Tessellation3D::Add(Vec3 v1, Vec3 v2, Vec3 v3)
	{
		data.push_back(Triangle3D(v1, v2, v3));

		return *this;
	}

	Tessellation3D::Tessellation3D(Triangle3D* triangles, int numTri)
	{
		for (int i = 0; i < numTri; i++)
		{
			data.push_back(triangles[i]);
		}
	}

	Tessellation3D& Tessellation3D::Shift(Vec3 by)
	{
		for (Triangle3D& tri : data) tri = tri.Shift(by);

		return *this;
	}

	Tessellation3D& Tessellation3D::Shift(double x, double y, double z)
	{
		return Shift(Vec3(x, y, z));
	}

	Tessellation3D& Tessellation3D::Rotate(double thetaX, double thetaY, double thetaZ, Vec3 around)
	{
		for (Triangle3D& tri : data) tri = tri.Rotate(thetaX, thetaY, thetaZ, around);

		return *this;
	}

	Tessellation3D& Tessellation3D::Scale(double by, Vec3 around)
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

	Tessellation3D& Tessellation3D::Scale(double scaleX, double scaleY, double scaleZ, Vec3 around)
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

	Tessellation3D& Tessellation3D::operator=(Tessellation3D& shape) noexcept
	{
		data = shape.data;

		return *this;
	}

	Tessellation3D& Tessellation3D::operator=(Tessellation3D&& shape) noexcept
	{
		data = shape.data;

		return *this;
	}

	void Tessellation3D::WriteSTL(std::ofstream& f, int beginning, int end) const
	{
		const char header[80]{ "ZarkLib STL file, generated from a Shape3D!" };
		const char attrib[2] = { 0 };
		const char normvec[12] = { 0 };

		// bounds checking
		end = end ? std::min(end, (int)data.size()) : data.size();
		if (beginning < 0 || beginning >= end || end < 0) return;

		// Write header to file
		f.write(header, 80);

		// Write number of triangles to file
		uint8_t triCt[4];
		ToBytes(triCt, (uint32_t)(end - beginning), Endian::Little);
		f.write((char*)triCt, 4);

		// Write triangle data to file
		BaseConverter bc(10, 16, 2);
		uint8_t buf[4];
		for (int i = beginning; i < end; i++)
		{
			const Triangle3D& tri = data[i];

			// Write the (blank) normal vector
			f.write(normvec, 12);

			// Write each vertex
			for (const Vec3& vertex : tri.vertices)
			{
				//std::cout << "Writing vertex: " << vertex << "\n";

				float x, y, z;
				x = vertex.X;
				y = vertex.Y;
				z = vertex.Z;
				// std::cout << "X: " << x << " Y: " << y << " Z: " << z << "\n"; // passed the sanity check

				ToBytes(buf, x, Endian::Little);
				f.write((char*)buf, 4);
				//std::cout << " -> X: " << bc(buf[0]) << " " << bc(buf[1]) << " " << bc(buf[2]) << " " << bc(buf[3]) << "\n";

				ToBytes(buf, y, Endian::Little);
				f.write((char*)buf, 4);
				//std::cout << " -> Y: " << bc(buf[0]) << " " << bc(buf[1]) << " " << bc(buf[2]) << " " << bc(buf[3]) << "\n";

				ToBytes(buf, z, Endian::Little);
				f.write((char*)buf, 4);
				//std::cout << " -> Z: " << bc(buf[0]) << " " << bc(buf[1]) << " " << bc(buf[2]) << " " << bc(buf[3]) << "\n";
			}

			// Write the (blank) attribute byte count
			f.write(attrib, 2);
		}
	}

	Tessellation3D Tessellation3D::Square(double size, Vec3 center)
	{
		double sizeD2 = size / 2;
		Tessellation3D square;

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

	Tessellation3D Tessellation3D::Rectangle(double sizeX, double sizeY, Vec3 center)
	{
		Tessellation3D rectangle;

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

	Tessellation3D Tessellation3D::NGon(int sides, double radius, Vec3 center)
	{
		if (sides < 3) return Tessellation3D();

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

		Tessellation3D ngon;
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

	Tessellation3D Tessellation3D::Cube(double size, Vec3 center)
	{
		const double PI_D2 = 3.1415926535897932384626 / 2.0;
		
		double sizeD2 = size / 2.0;
		Tessellation3D cube;

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

	Tessellation3D Tessellation3D::Prism(int baseSides, double radius, double height, Vec3 center)
	{
		// Can't make prism if base isn't a shape
		if (baseSides < 3) return Tessellation3D();

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
		Tessellation3D prism;
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
		Tessellation3D base = NGon(baseSides, radius, center);
		base.Shift(Vec3(0, 0, -heightD2));
		prism.Add(base);

		base.Shift(Vec3(0, 0, height));
		prism.Add(base);

		delete[] baseCoords;

		return prism;
	}

	Tessellation3D Tessellation3D::Sphere(int resolution, double radius, Vec3 center)
	{
		if (resolution < 3) return Tessellation3D();

		// Allocate the circle coordinate arrays
		Vec3* circleCoordsCurrent = new Vec3[resolution];
		Vec3* circleCoordsNext = new Vec3[resolution];

		// Assign correct values to initial circleCoordsNext, which will then be copied into circleCoordsCurrent
		for (int i = 0; i < resolution; i++) circleCoordsNext[i] = center - Vec3(0, 0, radius);

		// Go through each circle layer and generate triangles
		Tessellation3D sphere;
		for (int layer = 1; layer < resolution; layer++)
		{
			// Copy Next into Current
			memcpy(circleCoordsCurrent, circleCoordsNext, sizeof(Vec3) * resolution);

			// Calculate the correct values for Next
			double sphereRad = -radius + 2.0 * layer * radius / resolution;
			double circleRad = std::sqrt(radius*radius - sphereRad*sphereRad);
			for (int i = 0; i < resolution; i++)
			{
				double theta = 2.0 * ZM_PI / resolution * i;
				circleCoordsNext[i] = Vec3(
					std::cos(theta)*circleRad,
					std::sin(theta)*circleRad,
					sphereRad
				) + center;
			}

			// Generate triangles
			for (int i = 0; i < resolution; i++)
			{
				if (layer != 1)
				{
					sphere.Add(
						circleCoordsCurrent[i],
						circleCoordsNext[(i + 1) % resolution],
						circleCoordsCurrent[(i + 1) % resolution]
					);
				}

				sphere.Add(
					circleCoordsNext[(i + 1) % resolution],
					circleCoordsCurrent[i],
					circleCoordsNext[i]
				);
				
			}
		}

		// Add top cap
		for (int i = 0; i < resolution; i++)
		{
			sphere.Add(
				circleCoordsNext[i],
				center + Vec3(0, 0, radius),
				circleCoordsNext[(i + 1) % resolution]
			);
		}

		// free up memory
		delete[] circleCoordsCurrent;
		delete[] circleCoordsNext;

		return sphere;
	}

}