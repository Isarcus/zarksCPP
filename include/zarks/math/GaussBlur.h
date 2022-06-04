#pragma once

#include <zarks/internal/Mat2D.h>
#include <zarks/math/GaussField.h>

#include <utility>

namespace zmath
{

template<typename T, typename SUM_T = T>
void GaussianBlur(Mat2D<T>& mat, int sigma, int devs = 2)
{
    GaussField gauss(sigma, 1.0);
    VecInt bounds = mat.Bounds();
    Mat2D<T> blurred(bounds);

    // Pre-compute weights for each point
    const std::vector<std::pair<VecInt, double>> points = gauss.Points(sigma*devs);

    // Perform Gaussian summation
    for (int x = 0; x < bounds.X; x++)
    {
        for (int y = 0; y < bounds.Y; y++)
        {
            VecInt blurCoord(x, y);

            // Summation for this blurred pixel
            SUM_T sum = SUM_T();
            double weight = 0;
            for (auto pair : points)
            {
                VecInt origCoord = blurCoord + pair.first;
                if (blurred.ContainsCoord(origCoord))
                {
                    sum += mat(origCoord) * pair.second;
                    weight += pair.second;
                }
            }
            blurred(blurCoord) = sum / weight;
        }
    }

    mat = blurred;
}

}
