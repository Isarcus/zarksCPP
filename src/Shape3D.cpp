#include <zarks/math/3D/Shape3D.h>

namespace zmath
{
    Shape3D::Shape3D() noexcept {}

    Shape3D::Shape3D(const Shape3D& shape) noexcept
        : vertices(shape.vertices)
        , indices(shape.indices)
    { }

    Shape3D::Shape3D(std::vector<Vec3> vertices, std::vector<int> indices) noexcept
        : vertices(vertices)
        , indices(indices)
    {}

    Shape3D& Shape3D::Add(const Shape3D& shape, bool verticesOnly)
    {
        int startIdx = vertices.size();
        for (const Vec3& v : shape.vertices)
        {
            vertices.push_back(v);
        }
            
        if (verticesOnly) return *this;

        for (const int& idx : shape.indices)
        {
            indices.push_back(idx + startIdx);
        }

        return *this;
    }

    Shape3D& Shape3D::Shift(Vec3 by)
    {
        for (Vec3& v : vertices) v += by;

        return *this;
    }

    Shape3D& Shape3D::Shift(double x, double y, double z)
    {
        for (Vec3& v : vertices) v += Vec3(x, y, z);

        return *this;
    }

    Shape3D& Shape3D::Rotate(double thetaX, double thetaY, double thetaZ, Vec3 around)
    {
        for (Vec3& v : vertices) v = v.Rotate(thetaX, thetaY, thetaZ, around);

        return *this;
    }

    Shape3D& Shape3D::Rotate(Vec3 angles, Vec3 around)
    {
        for (Vec3& v : vertices) v = v.Rotate(angles.X, angles.Y, angles.Z, around);

        return *this;
    }

    Shape3D& Shape3D::Scale(double by, Vec3 around)
    {
        for (Vec3& v : vertices) v = v.Scale(by, around);

        return *this;
    }

    Shape3D& Shape3D::Scale(double scaleX, double scaleY, double scaleZ, Vec3 around)
    {
        for (Vec3& v : vertices) v = v.Scale(Vec3(scaleX, scaleY, scaleZ), around);

        return *this;
    }

    Shape3D& Shape3D::operator=(const Shape3D& shape) noexcept
    {
        vertices = shape.vertices;
        indices = shape.indices;

        return *this;
    }

    Shape3D& Shape3D::operator=(Shape3D&& shape) noexcept
    {
        vertices = std::move(shape.vertices);
        indices = std::move(shape.indices);

        return *this;
    }

    Shape3D& Shape3D::STLCleanup(bool scaleIfSmall)
    {
        if (vertices.size() == 0 || indices.size() == 0) return *this;

        // Determine the minimum distance between any two connected vertices...
        const double minSafestDistance = 1.1; // 1.01 not enough; this works fine
        double minDist = minSafestDistance;

        size_t idx = 0;
        while (idx + 2 < indices.size())
        {
            const Vec3& v1 = vertices[indices[idx++]];
            const Vec3& v2 = vertices[indices[idx++]];
            const Vec3& v3 = vertices[indices[idx++]];

            minDist = std::min(minDist, v1.DistForm(v2));
            minDist = std::min(minDist, v2.DistForm(v3));
            minDist = std::min(minDist, v3.DistForm(v1));
        }

        // ... and apply necessary scaling
        if (minDist == 0)
        {
            std::cout << " -> Minimum triangle side length found to be zero. The shape is ill-formed, and could not be fixed.\n";
            return *this;
        }
        else if (minDist < minSafestDistance)
        {
            double scale = 1.1 * minSafestDistance / minDist;
            for (Vec3& v : vertices) v = v.Scale(scale);

            std::cout << " -> Minimum triangle side length found to be: " << minDist << ". Subsequent transformation applied.\n";
        }

        // Determine shift necessary to put shape in the all-positive quadrant...
        const double minSafestVal = 10.0; // arbitrary, but should be > 0
        Vec3 minCoord = vertices[0];
        for (const Vec3& v : vertices)
        {
            minCoord = Vec3::Min(minCoord, v);
        }

        // ... and shift accordingly
        Vec3 shiftBy;
        if (minCoord.X < minSafestVal) shiftBy.X = (minSafestVal - minCoord.X);
        if (minCoord.Y < minSafestVal) shiftBy.Y = (minSafestVal - minCoord.Y);
        if (minCoord.Z < minSafestVal) shiftBy.Z = (minSafestVal - minCoord.Z);
        if (shiftBy != Vec3())
        {
            Shift(shiftBy);
            std::cout << " -> Minimum coordinate found to be: " << minCoord << ". Subsequent shift applied.\n";
        }
        
        
        return *this;
    }

    Tessellation3D Shape3D::Tessellate()
    {
        Tessellation3D tess;

        size_t idx = 0;
        while (idx+2 < indices.size())
        {
            tess.Add(
                vertices[indices[idx]],
                vertices[indices[idx + 1]],
                vertices[indices[idx + 2]]
            );

            idx += 3;
        }

        return tess;
    }

    Shape3D Shape3D::RectangularPrism(Vec3 min, Vec3 max)
    {
        /* View of a cube from the -Y, +X, +Z sides:
        *
        *            7 - - - - 6
        *  +Z      / |       / |          +Y
        *         /  |      /  |          
        *  /\    4 - - - - 5   |        /|
        *  ||    |   3 - - | - 2        /
        *  ||    |  /      |  /        /
        *  ||    | /       | /        /
        *        0 - - - - 1
        * 
        *         -------->   +X
        */

        Shape3D prism;
        prism.vertices = {
            // bottom four vertices
            Vec3(min.X, min.Y, min.Z), // 0 -- equivalent to 'min' but this looks nicer
            Vec3(max.X, min.Y, min.Z), // 1
            Vec3(max.X, max.Y, min.Z), // 2
            Vec3(min.X, max.Y, min.Z), // 3

            // top four vertices
            Vec3(min.X, min.Y, max.Z), // 4
            Vec3(max.X, min.Y, max.Z), // 5
            Vec3(max.X, max.Y, max.Z), // 6 -- equivalent to 'max' but this looks nicer
            Vec3(min.X, max.Y, max.Z), // 7
        };

        // Whatever you do, future me, PLEASE just don't touch this painstakingly constructed index array
        prism.indices = {
            // Z faces
            0, 2, 1,
            2, 0, 3,
            6, 4, 5,
            4, 6, 7,

            // Y faces
            5, 0, 1,
            0, 5, 4,
            3, 6, 2,
            6, 3, 7,

            // X faces
            4, 3, 0,
            3, 4, 7,
            2, 5, 1,
            5, 2, 6,
        };

        return prism;
    }

    Shape3D Shape3D::Cube(Vec3 min, double size)
    {
        return RectangularPrism(min, min + size);
    }

    Shape3D Shape3D::Polygon(int sides, double radius, Vec3 center)
    {
        Shape3D polygon;

        // Vertices
        for (int i = 0; i < sides; i++)
        {
            double theta = (2.0 * PI * i) / sides;

            polygon.vertices.push_back(Vec3(
                cos(theta) * radius,
                sin(theta) * radius,
                0
            ) + center);
        }

        // Indices
        for (int i = 0; i < sides - 2; i++)
        {
            if (i % 2) // odd
            {
                polygon.indices.push_back(sides - 1 - (i / 2));
                polygon.indices.push_back((i / 2) + 1);
                polygon.indices.push_back((i / 2) + 2);
            }
            else // even
            {
                polygon.indices.push_back((i / 2) + 1);
                polygon.indices.push_back(sides - 1 - (i / 2));
                polygon.indices.push_back((sides - (i / 2)) % sides);
            }
        }

        return polygon;
    }

    Shape3D Shape3D::Polygon(std::vector<Vec3> points2D, bool ccw)
    {
        Shape3D polygon;

        int sides = points2D.size();
        if (sides < 3) return polygon;

        if (!ccw)
        {
            size_t pointNum = points2D.size();
            for (size_t i = 0; i < pointNum / 2; i++)
            {
                Vec3 temp = points2D[i];
                points2D[i] = points2D[pointNum - 1 - i];
                points2D[pointNum - 1 - i] = temp;
            }
        }

        for (int i = 0; i < sides - 2; i++)
        {
            if (i % 2) // odd
            {
                polygon.indices.push_back(sides - 1 - (i / 2));
                polygon.indices.push_back((i / 2) + 1);
                polygon.indices.push_back((i / 2) + 2);
            }
            else // even
            {
                polygon.indices.push_back((i / 2) + 1);
                polygon.indices.push_back(sides - 1 - (i / 2));
                polygon.indices.push_back((sides - (i / 2)) % sides);
            }
        }

        return polygon;
    }

    Shape3D Shape3D::Prism(int sides, double radius, double height, Vec3 baseCenter)
    {
        Shape3D prism;

        Shape3D ngon = Polygon(sides, radius, baseCenter);
        prism.Add(ngon);
        ngon.Shift(0, 0, height); // TODO: also rotate (for counter-clockwise rule) without flipping vertices
        prism.Add(ngon);

        for (int i = 0; i < sides; i++)
        {
            prism.indices.push_back(i);
            prism.indices.push_back((i + 1) % sides + sides);
            prism.indices.push_back(i + sides);

            prism.indices.push_back((i + 1) % sides + sides);
            prism.indices.push_back(i);
            prism.indices.push_back((i + 1) % sides);
        }

        return prism;
    }

    Shape3D Shape3D::TriangularPyramid(std::array<Vec3, 4> vertices, bool acceptOrder)
    {
        Shape3D pyramid;

        if (acceptOrder)
        {
            pyramid.indices = {
                0, 1, 2,
                0, 2, 3,
                0, 3, 1,
                3, 2, 1,
            };

            for (const Vec3& v : vertices) pyramid.vertices.push_back(v);

            return pyramid;
        }

        // Determine the centroid so that the prism can be made counterclockwise-external, by STL specs
        Vec3 center;
        for (const Vec3& v : vertices) center += v;
        center /= 4.0;

        // Start by finding if there exists a greatest/least X-coordinate vertex
        //Vec3 greatestCoords = vertices[0];
        //Vec3 leastCoords = vertices[0];
        //int outlierIndices[3]{};
        // for (const Vec3& v : vertices)
        // {
        //     // TODO: WORK THIS MATH OUT!!!
        // }

        return pyramid;
    }

    Shape3D Shape3D::Sphere(int resolution, double radius, Vec3 center)
    {
        const int heightResFactor = 2;

        Shape3D sphere;

        // Generate vertices
        int resolutionVertical = resolution * heightResFactor;
        double heightRadStep = PI / resolutionVertical;
        for (int i = 1; i < resolutionVertical; i++)
        {
            double heightRadians = -PID2 + (heightRadStep * i);
            double circleHeight = radius * std::sin(heightRadians);
            double circleRadius = radius * std::cos(heightRadians);

            Shape3D circle = Polygon(resolution, circleRadius, center + Vec3(0, 0, circleHeight));

            sphere.Add(circle, true);
        }
        
        // Assign vertices between layers
        for (int i = 0; i < resolutionVertical - 2; i++) // vertical layer loop
        {
            int idxVertical = i * resolution;

            for (int j = 0; j < resolution; j++) // circle point loop
            {
                int idx = idxVertical + j;

                // Triangle 1 of square
                sphere.indices.push_back(idx);
                sphere.indices.push_back(idxVertical + ((j + 1) % resolution));
                sphere.indices.push_back(idxVertical + ((j + 1) % resolution) + resolution);

                // Triangle 2 of square
                sphere.indices.push_back(idxVertical + ((j + 1) % resolution) + resolution);
                sphere.indices.push_back(idx + resolution);
                sphere.indices.push_back(idx);
            }
        }

        // Create cap vertices
        sphere.vertices.push_back(center - Vec3(0, 0, radius));
        sphere.vertices.push_back(center + Vec3(0, 0, radius));

        // Assign cap indices
        int vertexCt = sphere.vertices.size();
        int capTop = vertexCt - 1;
        int capBot = vertexCt - 2;
        for (int i = 0; i < resolution; i++)
        {
            // Top cap
            sphere.indices.push_back(capTop - resolution - 1 + i);
            sphere.indices.push_back(capTop - resolution - 1 + ((i + 1) % resolution));
            sphere.indices.push_back(capTop);

            // Bottom cap
            sphere.indices.push_back(capBot);
            sphere.indices.push_back((i + 1) % resolution);
            sphere.indices.push_back(i);

        }

        return sphere;
    }

    Shape3D Shape3D::Equation2D(Vec min, Vec max, double(*function)(double, double), bool sides, Vec stepXY)
    {
        Shape3D map;

        Vec intRes;
        for (double x = min.X; x <= max.X; x += stepXY.X)
        {
            for (double y = min.Y; y <= max.Y; y += stepXY.Y)
            {
                map.vertices.push_back(Vec3(x, y, function(x, y)));

                if (!intRes.X) intRes.Y++;
            }

            intRes.X++;
        }

        //Vec intRes = ((max - min) / stepXY).Abs().Floor();

        for (int x = 0; x < intRes.X - 1; x++)
        {
            for (int y = 0; y < intRes.Y - 1; y++)
            {
                int curIdx = y + x*intRes.Y;

                map.indices.push_back(curIdx);
                map.indices.push_back(curIdx + intRes.Y);
                map.indices.push_back(curIdx + intRes.Y + 1);

                map.indices.push_back(curIdx + intRes.Y + 1);
                map.indices.push_back(curIdx + 1);
                map.indices.push_back(curIdx);
            }
        }

        // if (sides)
        // {
        //     // First, the bottom
        //     Vec maxActual = min + stepXY * intRes;


        // }

        return map;
    }
}
