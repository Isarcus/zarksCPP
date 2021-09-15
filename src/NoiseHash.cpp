#include <zarks/noise/NoiseHash.h>

#include <zarks/internal/zmath_internals.h>

#include <chrono>

namespace zmath
{

NoiseHash::NoiseHash(uint64_t seed)
    : eng(seed == RANDOM_SEED ? std::chrono::system_clock::now().time_since_epoch().count() : seed)
    , angleRNG(0, 2.0*PI)
{}

bool NoiseHash::Exists(VecInt key) const
{
    return hash.count(key) == 1;
}

Vec& NoiseHash::Create(VecInt key, Vec val)
{
    hash.emplace(key, val);
    return hash.at(key);
}

Vec& NoiseHash::Create(VecInt key)
{
    return Create(key, Vec::UnitVector(angleRNG(eng)));
}

void NoiseHash::Clear()
{
    hash.clear();
}

Vec& NoiseHash::operator[](VecInt key)
{
    if (Exists(key))
        return hash.at(key);
    else
        return Create(key);
}

Vec& NoiseHash::At(VecInt key)
{
    return hash.at(key);
}

const Vec& NoiseHash::At(VecInt key) const
{
    return hash.at(key);
}

} // namespace zmath
