#pragma once

#include <zarks/math/Map.h>
#include <zarks/noise/NoiseHash.h>
#include <zarks/internal/noise_internals.h>

#include <unordered_map>
#include <random>

namespace zmath
{
    class Noiser
    {
    public:
        Noiser(double (*noiseFunc)(Vec, NoiseHash&), size_t seed = 0);

        Map operator()(VecInt dimensions, int octaves, bool interpolate = true);
        void AddOctave(Map& map, int octave);

    private:
        // Hash map to keep track of all vectors in the current octave
        NoiseHash hash;
        // The noise function to call on each pixel
        double (*noiseFunc)(Vec, NoiseHash&);
        // RNG
        std::default_random_engine eng;
    };

    double SimplexPoint(Vec coord, NoiseHash& hash);
    double PerlinPoint(Vec coord, NoiseHash& hash);
}
