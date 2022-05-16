#pragma once

#include <zarks/internal/Mat2D.h>

#include <vector>

namespace zmath
{
    // @return The index of the mean with the least distance to `val`,
    //  as compared with the < operator.
    template <typename T, typename DISTANCE_CALC>
    size_t ComputeNearestMean(const std::vector<T>& means, const T& val);

    // @return An iterator pointing to the mean with the least distance
    //  to `val`, as compared with the < operator.
    template <typename T, typename DISTANCE_CALC, typename ITER_T>
    ITER_T ComputeNearestMean(ITER_T begin, ITER_T end, const T& val);

    // Runs the Naive K Means algorithm, as described at https://en.wikipedia.org/wiki/K-means_clustering.
    // @param T the type of the data and means to run the algorithm on.
    // @param DISTANCE_CALC a type that overloads operator()(const T&, const T&),
    //        which must return a type comparable with the < operator.
    // @param SUM_T a type which overloads operator+=(const T&), as well as
    //        operator/(size_t). The latter must return a type that is either
    //        equivalent or implicitly convertible to T. For builtin numeric
    //        types, SUM_T should be the same as T. For types where summation
    //        does not make sense, such as zmath::RGBA, a separate summation
    //        type must be used.
    // @param means a vector of means to initialize the algorithm. This must
    //        be non-empty, or an exception will be thrown. This vector may be
    //        modified as the algorithm runs.
    // @param data a dataset to run the algorithm on.
    // @return A vector of indices of the mean that has been assigned to each
    //         value in the dataset.
    template <typename T, typename DISTANCE_CALC, typename SUM_T = T>
    std::vector<size_t> ComputeKMeans(std::vector<T>& means, const std::vector<T>& data);

    template <typename T, typename DISTANCE_CALC, typename SUM_T = T>
    Mat2D<size_t> ComputeKMeans(std::vector<T>& means, const Mat2D<T>& data);

    //                //
    // Implementation //
    //                //

    template <typename T, typename DISTANCE_CALC>
    size_t ComputeNearestMean(const std::vector<T>& means, const T& val)
    {
        // Create distance calculator
        DISTANCE_CALC distCalc;

        // Compute mean with the least distance to val
        size_t idx = 0;
        auto lowestDist = distCalc(means.at(0), val);
        for (size_t i = 1; i < means.size(); i++)
        {
            auto thisDist = distCalc(means[i], val);
            if (thisDist < lowestDist)
            {
                lowestDist = thisDist;
                idx = i;
            }
        }

        // Return index of nearest mean
        return idx;
    }

    template <typename T, typename DISTANCE_CALC, typename ITER_T>
    ITER_T ComputeNearestMean(ITER_T begin, ITER_T end, const T& val)
    {
        // Create distance calculator
        DISTANCE_CALC distCalc;

        // Run compute loop
        ITER_T ret = begin;
        auto lowestDist = distCalc(*begin, val);
        for (ITER_T iter = ++begin; iter != end; iter++)
        {
            auto thisDist = distCalc(*iter, val);
            if (thisDist < lowestDist)
            {
                lowestDist = thisDist;
                ret = iter;
            }
        }

        // Return iterator to nearest mean
        return ret;
    }

    template <typename T, typename DISTANCE_CALC, typename SUM_T = T>
    std::vector<size_t> ComputeKMeans(std::vector<T>& means, const std::vector<T>& data)
    {
        size_t data_size = data.size();
        std::vector<size_t> meanLocs(data_size);

        // Run main algorithm as long as needed
        while (true)
        {
            // No changes to mean assignments made yet
            size_t changes = 0;

            // Assign each value to its nearest mean
            for (size_t i = 0; i < data_size; i++)
            {
                // Compute new mean location for this datum
                size_t newMeanLoc = ComputeNearestMean<T, DISTANCE_CALC>(means, data[i]);
                // See if the new mean location is different from the last one
                if (newMeanLoc != meanLocs[i]) changes++;
                // Assign new mean location
                meanLocs[i] = newMeanLoc;
            }

            // Break if no changes were made
            if (!changes) break;

            // Compute sums for each mean
            std::vector<std::pair<size_t, SUM_T>> sums(means.size());
            for (size_t i = 0; i < data_size; i++)
            {
                auto& sum = sums[meanLocs[i]];
                sum.first++;
                sum.second += data[i];
            }

            // Compute new means
            for (size_t i = 0; i < means.size(); i++)
            {
                if (sums[i].first)
                {
                    means[i] = sums[i].second / sums[i].first;
                }
            }
        }

        return meanLocs;
    }

    template <typename T, typename DISTANCE_CALC, typename SUM_T = T>
    Mat2D<size_t> ComputeKMeans(std::vector<T>& means, const Mat2D<T>& data)
    {
        VecInt bounds = data.Bounds();
        Mat2D<size_t> meanLocs(bounds);

        // Run main algorithm as long as needed
        while (true)
        {
            // No changes to mean assignments made yet
            size_t changes = 0;

            // Assign each value to its nearest mean
            for (int x = 0; x < bounds.X; x++)
            {
                for (int y = 0; y < bounds.Y; y++)
                {
                    // Compute new mean location for this datum
                    size_t newMeanLoc = ComputeNearestMean<T, DISTANCE_CALC>(means, data(x, y));
                    // See if the new mean location is different from the last one
                    if (newMeanLoc != meanLocs(x, y)) changes++;
                    // Assign new mean location
                    meanLocs(x, y) = newMeanLoc;
                }
            }

            // Break if no changes were made
            if (!changes) break;

            // Compute sums for each mean
            std::vector<std::pair<size_t, SUM_T>> sums(means.size());

            // Assign each value to its nearest mean
            for (int x = 0; x < bounds.X; x++)
            {
                for (int y = 0; y < bounds.Y; y++)
                {
                    auto& sum = sums[meanLocs(x, y)];
                    sum.first++;
                    sum.second += data(x, y);
                }
            }

            // Compute new means
            for (size_t i = 0; i < means.size(); i++)
            {
                if (sums[i].first)
                {
                    means[i] = sums[i].second / sums[i].first;
                }
            }
        }

        return meanLocs;
    }

} // namespace zmath
