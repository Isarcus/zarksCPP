#pragma once

#include <zarks/math/VecT.h>

#include <random>
#include <vector>

namespace zmath
{
    unsigned hash_cantor(unsigned a, unsigned b);
    unsigned hash_cantor_improved(unsigned a, unsigned b);
    unsigned hash_szudzik(unsigned a, unsigned b);
    unsigned hash_szudzik_improved(unsigned a, unsigned b);

    class NoiseHash
    {
    public:
        typedef unsigned (*hash_t)(unsigned, unsigned);

        NoiseHash(size_t seed = 0, size_t numAngles = 13, hash_t hash2D = hash_szudzik_improved);

        Vec operator[](VecInt key) const;

        void Shuffle();

        static constexpr size_t PERM_TABLE_SIZE = 256;

    private:
        std::default_random_engine eng;
        unsigned (*hash2D)(unsigned, unsigned);
        std::vector<Vec> angleTable;
        uint32_t permTable[PERM_TABLE_SIZE];
    };
} // namespace zmath
