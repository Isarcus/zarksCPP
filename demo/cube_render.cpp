#include <zarks/math/3D/Camera.h>
#include <zarks/image/Image.h>
#include <zarks/image/GIF.h>

#include <iostream>

using namespace zmath;

// Inefficient implementation of Bresenham's Line Algorithm,
// but good enough for simple cubes.
template <typename FUNC = void(*)(VecInt)>
void bresenhams(VecInt p1, VecInt p2, FUNC func)
{
    VecInt diff = p2 - p1;
    bool flipX = diff.X < 0, flipY = diff.Y < 0;
    diff = diff.Abs();
    bool flipCoords = diff.Y > diff.X;
    if (flipX)
    {
        p1.X *= -1;
        p2.X *= -1;
    }
    if (flipY)
    {
        p1.Y *= -1;
        p2.Y *= -1;
    }
    if (flipCoords)
    {
        p1 = p1.Flip();
        p2 = p2.Flip();
        diff = diff.Flip();
        std::swap(flipX, flipY);
    }
    
    // Perform algorithm within newly limited coordinate space
    int D = 2*diff.Y - diff.X;
    int y = p1.Y;
    for (int x = p1.X; x <= p2.X; x++)
    {
        VecInt point(
            (flipX) ? x*-1 : x,
            (flipY) ? y*-1 : y
        );
        func((flipCoords) ? point.Flip() : point);

        if (D > 0)
        {
            y++;
            D -= 2*diff.X;
        }
        D += 2*diff.Y;
    }
}

int main()
{
    std::vector<Vec3> input{
        Vec3(0, 0, 0),
        Vec3(0, 0, 1),
        Vec3(0, 1, 0),
        Vec3(0, 1, 1),
        Vec3(1, 0, 0),
        Vec3(1, 0, 1),
        Vec3(1, 1, 0),
        Vec3(1, 1, 1)
    };

    std::vector<std::pair<int, int>> indices{
        // square 1
        {0, 1},
        {1, 3},
        {3, 2},
        {2, 0},

        // square 2
        {4, 5},
        {5, 7},
        {7, 6},
        {6, 4},

        // connect squares
        {0, 4},
        {1, 5},
        {3, 7},
        {2, 6}
    };

    Vec3 basePos(-2, 0.75, 0.25);
    Camera cam(basePos, radians(100), radians(100));
    VecInt bounds(64, 64);

    GIF gif;
    const int numFrames = 40;
    const double amplitude = 1.0;
    const double squareCoeff = -4.0*amplitude / std::pow(numFrames, 2);
    for (int i = 0; i < numFrames; i++)
    {
        double Zdiff = squareCoeff * std::pow(i - numFrames/2.0, 2) + amplitude;
        cam.SetPos(basePos + Vec3(0, 0, Zdiff));
        std::vector<Vec> output;
        output.resize(input.size());
        cam.To2D(input.begin(), input.end(), output.begin());
        Image frame(bounds);
        for (auto pair : indices)
        {
            Vec p1 = output[pair.first], p2 = output[pair.second];
            Vec diff = (p1 - p2).Abs();
            if ((diff.X > cam.GetFovHorizontal() / PIX2 && (p1.X < 0 || p1.X > 1) && (p2.X > 0 || p2.X < 1)) ||
                (diff.Y > cam.GetFovVertical()   / PIX2 && (p1.Y < 0 || p1.Y > 1) && (p2.Y > 0 || p2.Y < 1)))
                    continue;

            VecInt c1 = p1 * bounds;
            VecInt c2 = p2 * bounds;
            //std::cout << output[pair.first] << ' ' << output[pair.second] << '\n';
            bresenhams(c1, c2, [&](VecInt point){
                if (frame.ContainsCoord(point))
                {
                    frame(point) = RGBA::White();
                }
            });
        }
        gif.Add(frame);
    }
    
    gif.Save("cube.gif", bounds, {RGBA::Black(), RGBA::White()}, {0.02});
}
