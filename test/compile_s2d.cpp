#include <zarks/internal/Sampleable2D.h>

#include <iostream>
#include <cmath>

using namespace zmath;

int main()
{
    const VecInt bounds(20, 20);

    // Check all overloads of Apply
    Sampleable2D<double> s1(bounds, 1.0);
    s1.Apply([](double v){ return v * 2; });
    s1.Apply([](int v){ return v * 2; });
    s1.Apply([](int x, int y, double v){ return ((x + y) % 2) ? x + y : v; });
    std::cout << s1(0, 0) << '\n'; // 4
    std::cout << s1(2, 9) << '\n'; // 11
    s1.Apply([](int x, int y) { return x * y; });

    // Check all overloads of ApplySample
    Sampleable2D<Vec> s2(bounds, Vec(1.0, 1.0));
    s2.ApplySample<double>(s1, [](double d){
        return Vec::UnitVector(d);
    });
    s1.ApplySample(s2, [](Vec v, double d) {
        return v.DistManhattan(Vec(d, d));
    });

    // Copy-ish constructor from Sampleable2D of a different type
    Sampleable2D<int> s3(Sampleable2D<bool>(bounds), [](bool b) { return b ? 1 : -1; });
    std::cout << s3(19, 19) << '\n';

    // Type deduction for overloaded functions
    s1.Apply(std::cos);
    s1.Apply(std::abs);
    s1.ApplySample(s3, std::abs);
    Sampleable2D<unsigned> s4(s1, std::abs);
}
