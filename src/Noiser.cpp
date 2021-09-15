#include <zarks/noise/Noiser.h>

namespace zmath
{

Noiser::Noiser(double (*noiseFunc)(double, double, std::unordered_map<VecInt, Vec>&))
    : noiseFunc(noiseFunc)
{}

Map Noiser::operator()(VecInt dimensions, int octaves, uint64_t seed)
{
    Map map(dimensions);

    for (int oct = 0; oct < octaves; oct++)
    {
        AddOctave(map, oct, seed);
    }

    return map;
}

void Noiser::AddOctave(Map& map, int octave, uint64_t seed)
{
    const double octPow = std::pow(2, octave);
    const double octInfluence = 1.0 / octPow;

    const VecInt dim = map.Bounds();
    const Vec scale = Vec(octPow, octPow) / Vec(dim);

    for (int x = 0; x < dim.X; x++)
    {
        for (int y = 0; y < dim.Y; y++)
        {
            const Vec point = Vec(x, y) * scale;
            map[x][y] += noiseFunc(point.X, point.Y, hash);
        }
    }
}

} // namespace zmath
