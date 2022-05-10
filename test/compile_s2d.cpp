#include <zarks/internal/Sampleable2D.h>

#include <iostream>
#include <cmath>

using namespace zmath;

int main()
{
    const VecInt bounds(20, 20);

    // Check constructors
    Sampleable2D<int>(20, 20, [](){ static int x = 3; return x++; });
    Sampleable2D<int>(bounds, [](){ static int x = 3; return x++; });
    Sampleable2D<int>(20, 20, [](int x, int y){ return x*y; });
    Sampleable2D<int>(bounds, [](int x, int y){ return ((x/2 + y/2) % 2) ? 0 : 1; });
    Sampleable2D<int>(Sampleable2D<bool>(bounds), [](bool b) { return b ? 1 : -1; });

    // Check all overloads of Apply
    Sampleable2D<double> s1(bounds, 1.0);
    s1.Apply([](){ static int i = -4; return i *= -1.25; });
    s1.Apply([](double v){ return v * 2; });
    s1.Apply([](int v){ return v * 2; });
    s1.Apply([](double v, int x, int y){ return ((x + y) % 2) ? x + y : v; });
    std::cout << s1(0, 0) << '\n';
    std::cout << s1(2, 9) << '\n';
    s1.Apply([](int x, int y) { return x * y; });

    // Check all overloads of ApplySample
    Sampleable2D<Vec> s2(bounds, Vec(1.0, 1.0));
    s2.ApplySample(s1, [](double d){
        return Vec::UnitVector(d);
    });
    s1.ApplySample(s2, [](double d, Vec v) {
        return v.DistManhattan(Vec(d, d));
    });

    // Type deduction for overloaded functions
    s1.Apply(std::cos);
    s1.Apply(std::abs);
    s1.ApplySample(Sampleable2D<int>(bounds), std::abs);
    Sampleable2D<unsigned> s4(s1, std::abs);
}
