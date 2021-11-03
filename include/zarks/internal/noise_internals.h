#pragma once

#include <zarks/math/VecT.h>

#include <unordered_map>

namespace std
{

// Necessary in order to use VecInt as a key in an std::unordered_map
template <>
struct hash<zmath::VecInt>
{
    size_t operator()(const zmath::VecInt& k) const
    {
        static constexpr size_t multBy = 7919;
        size_t res = 17;
        res = res * multBy + hash<unsigned>()(k.X);
        res = res * multBy + hash<unsigned>()(k.Y);
        return res;
    }
};

} // namespace std

namespace zmath
{

namespace simplex
{

static constexpr double F2D = 0.3660254037844386467637231707529361834714026;
static constexpr double G2D = 0.2113248654051871177454256097490212721761991;

inline Vec skew(Vec coord)
{
    return Vec(
        coord.X + coord.Sum() * F2D,
        coord.Y + coord.Sum() * F2D
    );
}

inline Vec unskew(Vec coord)
{
    return Vec(
        coord.X - coord.Sum() * G2D,
        coord.Y - coord.Sum() * G2D
    );
}

} // namespace simplex

} // namespace zmath
