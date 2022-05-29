#include <zarks/noise/Noiser.h>

namespace zmath
{

//              //
// Noiser Class //
//              //

Noiser::Noiser(double (*noiseFunc)(Vec, NoiseHash&), size_t seed)
    : hash(seed)
    , noiseFunc(noiseFunc)
{}

Map Noiser::operator()(VecInt dimensions, int octaves, bool interpolate)
{
    std::cout << "Generating new Noiser map:\n"
              << " -> Width:  " << dimensions.X << "\n"
              << " -> Height: " << dimensions.Y << "\n";

    Map map(dimensions);

    for (int oct = 0; oct < octaves; oct++)
    {
        AddOctave(map, oct);
        std::cout << " -> Octave \033[1;32m" << oct + 1 << "\033[0m Finished.\r" << std::flush;
    }
    std::cout << " -> All done!                       \n";

    if (interpolate)
    {
        map.Interpolate(0, 1);
    }

    return map;
}

void Noiser::AddOctave(Map& map, int octave)
{
    hash.Shuffle();

    const double octPow = std::pow(2, octave);
    const double octInfluence = 1.0 / octPow;

    const VecInt dim = map.Bounds();
    const Vec scale = Vec(octPow, octPow) / Vec(dim);

    for (int x = 0; x < dim.X; x++)
    {
        for (int y = 0; y < dim.Y; y++)
        {
            const Vec point = Vec(x, y) * scale;
            map(x, y) += octInfluence * noiseFunc(point, hash);
        }
    }
}

//                   //
// Example Functions //
//                   //

double SimplexPoint(Vec coord, NoiseHash& hash)
{
    Vec skewed = simplex::skew(coord);
    Vec itl = skewed - skewed.Floor();
    // Corners of this simplex
    VecInt corners[3];
    corners[0] = skewed.Floor();
    corners[1] = corners[0] + ((itl.X > itl.Y) ? VecInt(1, 0) : VecInt(0, 1));
    corners[2] = corners[0] + VecInt(1, 1);

    double Z = 0;
    for (int i = 0; i < 3; i++)
    {
        Vec displacement = coord - simplex::unskew(corners[i]);
        double distance = displacement.DistForm();
        double influence = std::pow(std::max(0.0, 0.625 - distance * distance), 4.0);
        Z += influence * displacement.Dot(hash[corners[i]]);
    }

    return Z;
}

double PerlinPoint(Vec coord, NoiseHash& hash)
{
    VecInt base = coord.Floor();
    Vec itl = coord - coord.Floor();
    
    double dots[2][2];
    dots[0][0] = itl.Dot(hash[base]);
    dots[0][1] = (itl - VecInt(0, 1)).Dot(hash[base + VecInt(0, 1)]);
    dots[1][0] = (itl - VecInt(1, 0)).Dot(hash[base + VecInt(1, 0)]);
    dots[1][1] = (itl - VecInt(1, 1)).Dot(hash[base + VecInt(1, 1)]);

    return interp5(
        interp5(dots[0][0], dots[1][0], itl.X),
        interp5(dots[0][1], dots[1][1], itl.X),
        itl.Y
    );
}

} // namespace zmath
