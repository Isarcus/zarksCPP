/**
 * Author: Isarcus
 * 
 * Created: November 14, 2021
 * 
 * Description: This file attempts to provide an answer of sorts to
 *              the math question, "given a function, f(x), describe
 *              the set of points consisting of the centers of all
 *              circles with a given radius, r, that are tangent to
 *              f(x)". But it goes a step further, allowing radius
 *              to vary as a function of x.
 */

#include <zarks/math/Map.h>
#include <zarks/image/Image.h>

#include <cmath>
#include <iostream>

using namespace zmath;

// @return the passed in angle, in radians
constexpr double rads(double angle)
{
    return (angle / 180.0) * 3.141592653589793238462643383;
}

// @return the slope of the function at x
template <typename FUNC>
double slope(double x, FUNC func)
{
    constexpr double dx = 0.0001;
    return (func(x + dx) - func(x)) / dx;
}

// Returns the offset of a function at a given point, such
// that [(x, f(x)) +- offset] represent the centers of circles
// which are tangent to f(x) at x.
template <typename FUNC>
Vec getOffset(double x, double d, FUNC func)
{
    double theta = rads(90) - std::atan(slope(x, func));
    return Vec::UnitVector(theta, d) * Vec(-1, 1);
}

// Helpful lambda factory
auto getLambda(double (*func)(double), double plus, double shrink)
{
    return [func, plus, shrink](double x) {
        return func(x * shrink) + plus;
    };
}

int main()
{
    VecInt bounds(500, 500);
    Map map(bounds);
    Vec scale(0.01, 0.01);

    // This is the function to be tested
    auto func = getLambda(std::cos, 2.5, 3);
    
    // Radius of the circles
    auto radius = getLambda(std::cos, 0, 3);

    // Loop through each x in a large enough range to generate
    // a complete graph
    for (int x = -20.0/scale.X; x < bounds.X + 20.0/scale.X; x++)
    {
        // Calculate input x, f(x), and offset(x)
        double inputX = x * scale.X;
        Vec funcPoint(inputX, func(inputX));
        Vec offset = getOffset(inputX, radius(inputX), func);

        // Three points to be graphed
        const VecInt graphMe[3]{
            funcPoint / scale,
            (funcPoint + offset) / scale,
            (funcPoint - offset) / scale
        };
        //std::cout << graphMe[0] << ", " << graphMe[1] << ", " << graphMe[2] << "\n";

        // Graph the points
        for (int i = 0; i < 3; i++)
        {
            if (map.ContainsCoord(graphMe[i]))
            {
                map.At(graphMe[i]) = 1;
            }
        }
    }

    // Save graph as image
    map.FlipVertical();
    Image(map).Save("graph.png");
}
