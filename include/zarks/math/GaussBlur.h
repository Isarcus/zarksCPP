#pragma once

#include <zarks/internal/Mat2D.h>
#include <zarks/math/GaussField.h>

#include <vector>
#include <thread>
#include <utility>

namespace zmath
{

template <typename T, typename SUM_T = T>
void GaussianBlurChunk(const Mat2D<T>* mat,
                       Mat2D<T>* blurred,
                       VecInt min,
                       VecInt max,
                       const std::vector<std::pair<VecInt, double>>& points)
{
    for (int x = min.X; x < max.X; x++)
    {
        for (int y = min.Y; y < max.Y; y++)
        {
            VecInt blurCoord(x, y);

            // Summation for this blurred pixel
            SUM_T sum = SUM_T();
            double weight = 0;
            for (auto pair : points)
            {
                VecInt origCoord = blurCoord + pair.first;
                if (blurred->ContainsCoord(origCoord))
                {
                    sum += (*mat)(origCoord) * pair.second;
                    weight += pair.second;
                }
            }
            (*blurred)(blurCoord) = sum / weight;
        }
    }
}

template<typename T, typename SUM_T = T>
void GaussianBlur(Mat2D<T>& mat, int sigma, int devs = 2, int numThreads = std::thread::hardware_concurrency())
{
    VecInt bounds = mat.Bounds();
    Mat2D<T> blurred(bounds);

    // Pre-compute weights for each point
    const std::vector<std::pair<VecInt, double>> points = GaussField(sigma, 1.0).Points(sigma*devs);

    if (numThreads > 1)
    {
        // Split passed matrix into regions to deal with per thread
        std::vector<std::thread> threads(numThreads);
        for (int i = 0; i < numThreads; i++)
        {
            threads[i] = std::thread(
                GaussianBlurChunk<T, SUM_T>,
                &mat,
                &blurred,
                Vec(bounds.X*(double(i) / numThreads), 0),
                Vec(bounds.X*(double(i+1) / numThreads), bounds.Y),
                points
            );
        }
        for (int i = 0; i < numThreads; i++)
        {
            threads[i].join();
        }
    }
    else
    {
        blurred.Apply([&](int x, int y){
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
            return sum / weight;
        });
    }

    mat = blurred;
}

} // namespace zmath
