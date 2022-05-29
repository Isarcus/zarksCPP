#include <zarks/noise/NoiseHash.h>
#include <zarks/internal/zmath_internals.h>

#include <chrono>
#include <algorithm>

namespace zmath
{

NoiseHash::NoiseHash(size_t seed, size_t numAngles, hash_t hash2D)
    : eng((seed) ? seed : std::chrono::system_clock::now().time_since_epoch().count())
    , hash2D(hash2D)
    , angleTable(numAngles)
{
    if (numAngles == 0)
    {
        throw std::runtime_error("numAngles must be at least 1");
    }

    // Create continuous angles table
    for (size_t i = 0; i < numAngles; i++)
    {
        angleTable[i] = Vec::UnitVector(i * PIX2 / numAngles);
    }

    // Create random permutation table
    std::iota(permTable, permTable + PERM_TABLE_SIZE, 0);
    std::shuffle(permTable, permTable + PERM_TABLE_SIZE, eng);
}

Vec NoiseHash::operator[](VecInt key) const
{
    uint32_t perm = permTable[hash2D(key.X, key.Y) % PERM_TABLE_SIZE];
    return angleTable[perm % angleTable.size()];
}

void NoiseHash::Shuffle()
{
    std::shuffle(permTable, permTable + PERM_TABLE_SIZE, eng);
}

unsigned hash_cantor(unsigned a, unsigned b)
{
    return (a + b) * (a + b + 1) / 2 + a;
}

unsigned hash_cantor_improved(unsigned a, unsigned b)
{
    return ((a + b) * (a + b + 1) / 2 + a) ^ (a*b);
}

unsigned hash_szudzik(unsigned a, unsigned b)
{
    return (a >= b) ? a*a + a + b : a + b*b;
}

unsigned hash_szudzik_improved(unsigned a, unsigned b)
{
    return ((a >= b) ? a*a + a + b : a + b*b) ^ (a*b);
}

} // namespace zmath
