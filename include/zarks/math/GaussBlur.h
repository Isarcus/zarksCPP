#pragma once

#include <zarks/internal/Sampleable2D.h>
#include <zarks/math/GaussField.h>

#include <utility>

namespace zmath
{

template<typename T, typename SUM_T = T>
void GaussianBlur(Sampleable2D<T>& samp, int sigma, int devs = 2)
{
    GaussField gauss(sigma, 1.0);
    VecInt bounds = samp.Bounds();
    Sampleable2D<std::pair<SUM_T, double>> blurred(bounds);

    // Pre-compute weights for each point
    const int halfSide = devs*sigma;
    const int weightSide = halfSide*2 + 1;
    const VecInt offset(halfSide, halfSide);
    Sampleable2D<double> weights(weightSide, weightSide);
    for (int dx = -halfSide; dx <= halfSide; dx++)
    {
        for (int dy = -halfSide; dy <= halfSide; dy++)
        {
            VecInt rel(dx, dy);
            weights(rel + offset) = gauss(dx, dy);
        }
    }

    // Perform Gaussian summation
    for (int x = 0; x < bounds.X; x++)
    {
        for (int y = 0; y < bounds.Y; y++)
        {
            // Summation for this pixel
            T val = samp(x, y);
            for (int dx = -halfSide; dx <= halfSide; dx++)
            {
                for (int dy = -halfSide; dy <= halfSide; dy++)
                {
                    VecInt coord(x + dx, y + dy);
                    if (blurred.ContainsCoord(coord))
                    {
                        double weight = weights(VecInt(dx, dy) + offset);
                        auto& pair = blurred(coord);
                        pair.first += val * weight;
                        pair.second += weight;
                    }
                }
            }
        }
    }

    // Scale result according to weights
    auto samp_it = samp.begin();
    for (const auto& pair : blurred)
    {
        *samp_it++ = pair.first / pair.second;
    }
}

}
