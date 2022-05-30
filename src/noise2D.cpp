#include <zarks/noise/noise2D.h>
#include <zarks/noise/NoiseHash.h>
#include <zarks/internal/zmath_internals.h>
#include <zarks/internal/noise_internals.h>

#include <cmath>
#include <chrono>
#include <iostream>
#include <utility>
#include <algorithm>
#include <thread>

namespace zmath
{

static Map SimplexThreaded(const NoiseConfig& cfg);

GridConfig::GridConfig()
    : bounds(Vec(1000, 1000))
    , boxSize(Vec(1000, 1000))
{}

GridConfig::GridConfig(Vec bounds, Vec boxSize)
    : bounds(bounds)
    , boxSize(boxSize)
{}

// Default NoiseConfig values
NoiseConfig::NoiseConfig()
    : GridConfig()
    , octaves(8)
    , normalize(true)
    , seed(std::chrono::system_clock::now().time_since_epoch().count())
    , lNorm(2)
    , octDecrease(0.5)
    // Simplex
    , r(0.625)
    , rMinus(4.0)
    // Worley
    , nearest{0, 2}
{}

void NoiseConfig::NewSeed()
{
    seed = std::chrono::system_clock::now().time_since_epoch().count();
}

//                 //
// Noise Functions //
//                 //

static auto GetSimplexFunc(const NoiseHash& hash, const NoiseConfig& cfg)
{
    const double r2 = std::pow(cfg.r, 2);
    return [=](Vec ipt)
    {
        // Compute skewed coordinate
        Vec skewed = simplex::skew(ipt);
        // Compute internal simplex coordinate
        Vec itl = skewed - skewed.Floor();

        // Corners of this simplex
        VecInt corners[3];
        // Angle vectors of this simplex's corners
        Vec vectors[3];

        // Base corner is just lowest point in this simplex
        corners[0] = skewed.Floor();
        // "Middle" corner depends on internal X and Y
        corners[1] = (itl.X > itl.Y) ? corners[0] + VecInt(1, 0) : corners[0] + VecInt(0, 1);
        // High corner is just highers point in this simplex
        corners[2] = corners[0] + Vec(1, 1);

        for (int i = 0; i < 3; i++)
        {
            vectors[i] = hash[corners[i]];
        }

        // Perform final summation for this coordinate
        double ret = 0;
        for (int i = 0; i < 3; i++)
        {
            Vec displacement = ipt - simplex::unskew(corners[i]);
            double distance = displacement.LNorm(cfg.lNorm); // Distance formula
            double influence = std::pow(std::max(0.0, r2 - distance * distance), cfg.rMinus);

            ret += influence * displacement.Dot(vectors[i]);
        }

        return ret;
    };
}

Map Simplex(const NoiseConfig& cfg)
{
    if (cfg.numThreads > 0)
    {
        return SimplexThreaded(cfg);
    }

    // RNG
    NoiseHash hash(cfg.seed);

    // Map setup
    Map map(cfg.bounds);

    std::cout << "Creating new Simplex Map:\n"
                << " -> Width:  " << cfg.bounds.X << "\n"
                << " -> Height: " << cfg.bounds.Y << "\n"
                << " -> Seed:   " << cfg.seed << "\n";
    
    // Dive in
    auto func = GetSimplexFunc(hash, cfg);
    for (int oct = 0; oct < cfg.octaves; oct++)
    {
        // Reshuffle the hash permutation table
        hash.Shuffle();

        // Calculate the influence of this octave on the overall noise map.
        // Influence most commonly decreases by a factor of 2 each octave.
        double octInfluence = std::pow(cfg.octDecrease, oct);

        // Calculate vector with which to scale coordinates ths octave.
        Vec scaleVec = (Vec(1.0, 1.0) / cfg.boxSize) / std::pow(0.5, oct);

        // Iterate through the whole map
        for (int x = 0; x < cfg.bounds.X; x++)
        {
            for (int y = 0; y < cfg.bounds.Y; y++)
            {
                // Compute input coordinate
                Vec ipt = scaleVec * Vec(x, y);
                
                // Add this coordinate to the map, weighted appropriately
                map.At(x, y) += func(ipt) * octInfluence;
            }
        } 

        // Notify octave completion
        std::cout << " -> Octave \033[1;32m" << oct + 1 << "\033[0m Finished.\r" << std::flush;
    }
    std::cout << " -> All done!                       \n";

    // Perform final normalization, if applicable
    if (cfg.normalize) map.Interpolate(0, 1);

    return map;
}

Map Perlin(const NoiseConfig& cfg)
{
    // Initialize map
    Map map(cfg.bounds);

    std::cout << "Generating new Perlin map:\n"
                << " -> Width:  " << cfg.bounds.X << "\n"
                << " -> Height: " << cfg.bounds.Y << "\n"
                << " -> Seed:   " << cfg.seed << "\n";

    // RNG
    NoiseHash hash(cfg.seed);

    // Beep beep so let's ride
    for (int oct = 0; oct < cfg.octaves; oct++)
    {
        // Reshuffle the hash permutation table
        hash.Shuffle();

        // Determine influence of this octave
        double octInfluence = std::pow(cfg.octDecrease, oct);

        // Determine size of each Perlin box in this octave
        Vec octaveBoxSize = cfg.boxSize * octInfluence;

        // Loop box-by-box through the map
        for (double bx = 0, bnx = 0; bx < cfg.bounds.X; bx += octaveBoxSize.X, bnx++)
        {
            for (double by = 0, bny = 0; by < cfg.bounds.Y; by += octaveBoxSize.Y, bny++)
            {
                // We are now operating within a single Perlin box.
                // Here, determine the random vectors of this box's four corners.
                VecInt base = Vec(bx, by).Floor();
                Vec corners[2][2];
                for (int cx = 0; cx <= 1; cx++)
                {
                    for (int cy = 0; cy <= 1; cy++)
                    {
                        // Randomly generate new vectors if not already present
                        VecInt test = VecInt(bnx + cx, bny + cy);
                        corners[cx][cy] = hash[test];
                    }
                }

                // With this box's corner vector's determined, we can now loop through
                // each point within the box to determine how much to add to the heightmap.

                // The size of this particular box. This can vary by 1 if octaveBoxSize is
                // not a whole number, which is quite common.
                VecInt thisBoxSize = (Vec(bx, by) + octaveBoxSize).Floor() - Vec(bx, by).Floor();
                double dots[2][2];
                for (int ix = 0; ix < thisBoxSize.X; ix++)
                {
                    // X Bound check
                    if (base.X + ix > cfg.bounds.X) continue;

                    for (int iy = 0; iy < thisBoxSize.Y; iy++)
                    {
                        // Y bound check
                        if (base.Y + iy > cfg.bounds.Y) continue;

                        // Internal coordinate within this box; ranging from (0, 0) to (1, 1)
                        Vec itl = Vec(ix, iy) / Vec(thisBoxSize);

                        // Absolute coordinate within the larger map
                        VecInt absolute = base + VecInt(ix, iy);

                        // For each corner, determine the dot product of its random
                        // directional vector with the distance vector from that corner
                        // to the current internal coordinate
                        dots[0][0] = (itl - Vec(0, 0)).Dot(corners[0][0]);
                        dots[0][1] = (itl - Vec(0, 1)).Dot(corners[0][1]);
                        dots[1][0] = (itl - Vec(1, 0)).Dot(corners[1][0]);
                        dots[1][1] = (itl - Vec(1, 1)).Dot(corners[1][1]);

                        // Interpolate dot product results
                        double y0, y1, Z;
                        y0 = interp5(dots[0][0], dots[1][0], itl.X);
                        y1 = interp5(dots[0][1], dots[1][1], itl.X);
                        Z = interp5(y0, y1, itl.Y);

                        map.At(absolute) += Z * octInfluence;
                    }
                }
            }
        }

        // Notify octave completion
        std::cout << " -> Octave \033[1;32m" << oct + 1 << "\033[0m Finished.\r" << std::flush;
    }
    std::cout << " -> All done!                       \n";

    if (cfg.normalize) map.Interpolate(0, 1);

    return map;
}

Map Worley(const NoiseConfig& cfg)
{
    // this coordList works for simpler algorithms that use fewer than ~5 points
    std::vector<Vec> coordList;
    for (int x = -3; x <= 3; x++)
        for (int y = -3; y <= 3; y++)
            coordList.push_back(Vec(x, y));

    // RNG
    NoiseHash hash(cfg.seed);

    std::cout << "Generating new Worley map:\n"
                << " -> Width:  " << cfg.bounds.X << "\n"
                << " -> Height: " << cfg.bounds.Y << "\n"
                << " -> Seed:   " << cfg.seed << "\n";

    // Initialize map
    Map map(cfg.bounds);

    unsigned distanceLen = coordList.size();
    std::vector<double> distances(distanceLen);

    for (int oct = 0; oct < cfg.octaves; oct++)
    {
        // Reshuffle the hash permutation table
        hash.Shuffle();

        double octInfluence = std::pow(cfg.octDecrease, oct);
        Vec scaleVec = (Vec(1.0, 1.0) / cfg.boxSize) / std::pow(0.5, oct);

        for (int x = 0; x < cfg.bounds.X; x++)
        {
            for (int y = 0; y < cfg.bounds.Y; y++)
            {
                Vec coord = scaleVec * Vec(x, y);
                VecInt base = coord.Floor();
                Vec itl = coord - base;

                // Get the distances to each point
                for (unsigned i = 0; i < distanceLen; i++)
                {
                    VecInt test = base + coordList[i];

                    distances[i] = (hash[test] + coordList[i] - itl).LNorm(cfg.lNorm);

                    // uncomment for quantized distance; looks best with lnorm = 2
                    // distances[i] = ((int)(distances[i] * 20.0)) / 20.0;
                }

                // Sort the distances, low to high
                std::sort(distances.begin(), distances.end());

                // Compute brightness
                double Z = 1;
                for (int i = cfg.nearest.first; i < cfg.nearest.second; i++)
                {
                    Z *= distances[i];
                }
                //std::cout << distances[1] << "\n";

                // Final summation
                map.At(x, y) += Z * octInfluence;
            }
        }

        // Notify octave completion
        std::cout << " -> Octave \033[1;32m" << oct + 1 << "\033[0m Finished.\r" << std::flush;
    }
    std::cout << " -> All done!                       \n";

    if (cfg.normalize) map.Interpolate(0, 1);

    return map;
}

// WorleyPlex is identical to Worley in almost every way, with the main exception
// being that the vector LNorm used to compute distances between points depends on
// the values of a passed-in heightmap. This can create some cool effects!
Map WorleyPlex(const NoiseConfig& cfg, const Map& baseMap)
{
    if (cfg.bounds != baseMap.Bounds())
    {
        std::cout << "Bounds mismatch :/ \n";
        return Map(baseMap.Bounds());
    }

    std::vector<Vec> coordList;
    for (int x = -2; x <= 2; x++)
        for (int y = -2; y <= 2; y++)
            coordList.push_back(Vec(x, y));

    // RNG
    NoiseHash hash(cfg.seed);

    std::cout << "Generating new Worleyplex map:\n"
                << " -> Width:  " << cfg.bounds.X << "\n"
                << " -> Height: " << cfg.bounds.Y << "\n"
                << " -> Seed:   " << cfg.seed << "\n";

    Map map(cfg.bounds);

    // Allocate this here, no point in constantly de- and re-allocating it in the loop
    unsigned distanceLen = coordList.size(); // TODO: multiply by config.N once implemented
    std::vector<double> distances(distanceLen);

    for (int oct = 0; oct < cfg.octaves; oct++)
    {
        // Reshuffle the hash permutation table
        hash.Shuffle();

        double octInfluence = std::pow(cfg.octDecrease, oct);
        Vec scaleVec = (Vec(1.0, 1.0) / cfg.boxSize) / std::pow(0.5, oct);

        for (int x = 0; x < cfg.bounds.X; x++)
        {
            for (int y = 0; y < cfg.bounds.Y; y++)
            {
                Vec coord = scaleVec * Vec(x, y);
                Vec base = coord.Floor();
                Vec itl = coord - base;

                // Get the distances to each point
                for (unsigned i = 0; i < distanceLen; i++)
                {
                    Vec test = base + coordList[i];

                    // This is where the base map is used
                    distances[i] = (hash[test] + coordList[i] - itl).LNorm(baseMap(x, y));
                }

                // Sort the distances, low to high
                std::sort(distances.begin(), distances.end());

                // Compute brightness
                double Z = 1;
                for (int i = cfg.nearest.first; i < cfg.nearest.second; i++)
                {
                    Z *= distances[i];
                }

                // Final summation
                map.At(x, y) += Z * octInfluence;
            }
        }

        // Notify octave completion
        std::cout << " -> Octave \033[1;32m" << oct + 1 << "\033[0m Finished.\r" << std::flush;
    }
    std::cout << " -> All done!                       \n";

    if (cfg.normalize) map.Interpolate(0, 1);

    return map;
}

//                          //
// Multithreaded Algorithms //
//                          //

void SimplexChunk(const NoiseConfig* cfg, Map* map, VecInt min, VecInt max, const std::vector<NoiseHash>* hashes)
{
    for (int oct = 0; oct < cfg->octaves; oct++)
    {
        const NoiseHash& hash = (*hashes)[oct];
        auto func = GetSimplexFunc(hash, *cfg);

        double octInfluence = std::pow(cfg->octDecrease, oct);
        Vec scaleVec = (Vec(1.0, 1.0) / cfg->boxSize) / std::pow(0.5, oct);

        for (int x = min.X; x < max.X; x++)
        {
            for (int y = min.Y; y < max.Y; y++)
            {
                // Compute input coordinate
                Vec ipt = scaleVec * Vec(x, y);

                // Add this coordinate to the map, weighted appropriately
                (*map)(x, y) += func(ipt) * octInfluence;
            }
        }
    }
}

Map SimplexThreaded(const NoiseConfig& cfg)
{
    size_t seed = (cfg.seed) ? cfg.seed : std::chrono::system_clock::now().time_since_epoch().count();
    Map map(cfg.bounds);
    
    std::vector<NoiseHash> hashes;
    hashes.reserve(cfg.octaves);
    NoiseHash hash(seed);
    for (int i = 0; i < cfg.octaves; i++)
    {
        hash.Shuffle();
        hashes.push_back(hash);
    }

    std::vector<std::thread> threads(cfg.numThreads);
    for (int i = 0; i < cfg.numThreads; i++)
    {
        VecInt min(
            double(i) / cfg.numThreads * cfg.bounds.X,
            0
        );
        VecInt max(
            double(i + 1) / cfg.numThreads * cfg.bounds.X,
            cfg.bounds.Y
        );

        threads[i] = std::thread(SimplexChunk, &cfg, &map, min, max, &hashes);
    }

    for (int i = 0; i < cfg.numThreads; i++)
    {
        threads[i].join();
    }

    if (cfg.normalize)
    {
        map.Interpolate(0, 1);
    }

    return map;
}

} // namespace zmath
