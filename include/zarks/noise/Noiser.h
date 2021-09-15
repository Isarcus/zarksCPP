#pragma once

#include <zarks/math/Map.h>
#include <zarks/noise/noise2D.h>
#include <zarks/internal/noise_internals.h>

#include <unordered_map>
#include <random>

namespace zmath
{
    class Noiser
    {
    public:
        Noiser(double (*noiseFunc)(double, double, std::unordered_map<VecInt, Vec>&));

        Map operator()(VecInt dimensions, int octaves, uint64_t seed = 0);
        void AddOctave(Map& map, int octave, uint64_t seed);

    private:
        // Hash map to keep track of all vectors in the current octave
        std::unordered_map<VecInt, Vec> hash;
        // The noise function to call on each pixel
        double (*noiseFunc)(double, double, std::unordered_map<VecInt, Vec>&);
        // RNG
        std::default_random_engine eng;
    };
}
