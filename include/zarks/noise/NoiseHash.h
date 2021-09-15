#pragma once

#include <zarks/internal/noise_internals.h>

#include <random>
#include <unordered_map>

namespace zmath
{
    class NoiseHash
    {
    public:
        NoiseHash(uint64_t seed = 0);

        bool Exists(VecInt key) const;
        Vec& Create(VecInt key, Vec val);
        Vec& Create(VecInt key);

        void Clear();

        Vec& operator[](VecInt key);

        Vec& At(VecInt key);
        const Vec& At(VecInt key) const;

        static constexpr uint64_t RANDOM_SEED = 0;

    private:
        std::unordered_map<VecInt, Vec> hash;
        std::default_random_engine eng;
        std::uniform_real_distribution<double> angleRNG;
    };
} // namespace zmath
