#include <zarks/internal/Mat2D.h>

#include <iostream>
#include <cmath>

using namespace zmath;

int main()
{
    const VecInt bounds(20, 20);

    // Check constructors
    Mat2D<int>(20, 20, [](){ static int x = 3; return x++; });
    Mat2D<int>(bounds, [](){ static int x = 3; return x++; });
    Mat2D<int>(20, 20, [](int x, int y){ return x*y; });
    Mat2D<int>(bounds, [](int x, int y){ return ((x/2 + y/2) % 2) ? 0 : 1; });
    Mat2D<int>(Mat2D<bool>(bounds), [](bool b) { return b ? 1 : -1; });

    // Check all overloads of Apply
    Mat2D<double> m1(bounds, 1.0);
    m1.Apply([](){ static int i = -4; return i *= -1.25; });
    m1.Apply([](double v){ return v * 2; });
    m1.Apply([](int v){ return v * 2; });
    m1.Apply([](double v, int x, int y){ return ((x + y) % 2) ? x + y : v; });
    std::cout << m1(0, 0) << '\n';
    std::cout << m1(2, 9) << '\n';
    m1.Apply([](int x, int y) { return x * y; });

    // Check all overloads of ApplySample
    Mat2D<Vec> m2(bounds, Vec(1.0, 1.0));
    m2.ApplySample(m1, [](double d){
        return Vec::UnitVector(d);
    });
    m1.ApplySample(m2, [](double d, Vec v) {
        return v.DistManhattan(Vec(d, d));
    });

    // Type deduction for overloaded functions
    m1.Apply(std::cos);
    m1.Apply(std::abs);
    m1.ApplySample(Mat2D<int>(bounds), std::abs);
    Mat2D<unsigned> m3(m1, std::abs);
}
