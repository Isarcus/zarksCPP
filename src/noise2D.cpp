#include <zarks/noise/noise2D.h>
#include <zarks/internal/zmath_internals.h>

#include <chrono>
#include <cmath>
#include <random>
#include <unordered_map>
#include <iostream>
#include <utility>

namespace std
{
	// Necessary in order to use Vec as a key in an std::unordered_map
	template <typename T>
	struct hash<zmath::VecT<T>>
	{
		size_t operator()(const zmath::VecT<T>& k) const
		{
			static constexpr size_t multBy = 73; // 31 
			size_t res = 17;
			res = res * multBy + hash<T>()(k.X);
			res = res * multBy + hash<T>()(k.Y);
			return res;
		}
	};
}

namespace zmath
{
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

	Map Simplex(const NoiseConfig& cfg)
	{
		// Helpful constants
		static constexpr double F2D = 0.3660254037844386467637231707529361834714026;
		static constexpr double G2D = 0.2113248654051871177454256097490212721761991;
		auto skew =   [](Vec pt) { return Vec(pt.X + pt.Sum() * F2D, pt.Y + pt.Sum() * F2D); };
		auto unskew = [](Vec pt) { return Vec(pt.X - pt.Sum() * G2D, pt.Y - pt.Sum() * G2D); };
		const double r2 = cfg.r * cfg.r;

		// RNG
		std::default_random_engine eng(cfg.seed);
		std::uniform_real_distribution<double> angleRNG(0, 2.0*PI);

		// Map setup
		Map map(cfg.bounds);

		std::cout << "Creating new Simplex Map:\n";
		std::cout << " -> Width:  " << cfg.bounds.X << "\n";
		std::cout << " -> Height: " << cfg.bounds.Y << "\n";
		std::cout << " -> Seed:   " << cfg.seed << "\n";
		
		// Dive in
		for (int oct = 0; oct < cfg.octaves; oct++)
		{
			// Calculate the influence of this octave on the overall noise map.
			// Influence most commonly decreases by a factor of 2 each octave.
			double octInfluence = std::pow(cfg.octDecrease, oct);

			// Calculate vector with which to scale coordinates ths octave.
			Vec scaleVec = (Vec(1.0, 1.0) / cfg.boxSize) / std::pow(0.5, oct);

			// Create hashmap from coordinates to unit vectors to reuse randomly generated angles
			std::unordered_map<VecInt, Vec> hash;

			// Iterate through the whole map
			for (int x = 0; x < cfg.bounds.X; x++)
			{
				for (int y = 0; y < cfg.bounds.Y; y++)
				{
					// Compute input coordinate
					Vec ipt = scaleVec * Vec(x, y);
					// Compute skewed coordinate
					Vec skewed = skew(ipt);
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
						if (hash.count(corners[i]))
						{
							// Use the already assigned angle vector if it exists
							vectors[i] = hash.at(corners[i]);
						}
						else
						{
							// Otherwise, generate and assign a new one
							double angle = angleRNG(eng);
							vectors[i] = Vec(std::cos(angle), std::sin(angle));
							hash.emplace(corners[i], vectors[i]);
						}
					}

					// Perform final summation for this coordinate
					double Z = 0;
					for (int i = 0; i < 3; i++)
					{
						Vec displacement = ipt - unskew(corners[i]);
						double distance = displacement.LNorm(cfg.lNorm); // Distance formula

						double influence = std::pow(std::max(0.0, r2 - distance * distance), cfg.rMinus);
						Z += influence * displacement.Dot(vectors[i]);
					}

					// Add this coordinate to the map, weighted appropriately
					map.At(x, y) += Z * octInfluence;
				}
			} 

			// Notify octave completion
			std::cout << " -> Octave " << oct + 1 << " Finished.\n";
		}

		// Perform final normalization, if applicable
		if (cfg.normalize) map.Interpolate(0, 1);

		return map;
	}

	Map Perlin(const NoiseConfig& cfg)
	{
		// RNG
		std::default_random_engine eng(cfg.seed);
		std::uniform_real_distribution<double> angleRNG(0, 2.0*PI);

		// Initialize map
		Map map(cfg.bounds);

		std::cout << "Generating new Perlin map:\n";
		std::cout << " -> Width:  " << cfg.bounds.X << "\n";
		std::cout << " -> Height: " << cfg.bounds.Y << "\n";
		std::cout << " -> Seed:   " << cfg.seed << "\n";

		// Beep beep so let's ride
		for (int oct = 0; oct < cfg.octaves; oct++)
		{
			std::unordered_map<VecInt, Vec> hash;
			double octInfluence = std::pow(cfg.octDecrease, oct);
			Vec octaveBoxSize = cfg.boxSize * octInfluence;

			// Box minimum x and y
			for (double bx = 0, bnx = 0; bx < cfg.bounds.X; bx += octaveBoxSize.X, bnx++)
			{
				for (double by = 0, bny = 0; by < cfg.bounds.Y; by += octaveBoxSize.Y, bny++)
				{
					// Determine vectors of this box's corners
					VecInt base = Vec(bx, by).Floor();
					Vec corners[2][2];
					for (int cx = 0; cx <= 1; cx++)
					{
						for (int cy = 0; cy <= 1; cy++)
						{
							// Randomly generate new vectors if not already present
							VecInt test = VecInt(bnx + cx, bny + cy);
							if (hash.count(test))
							{
								corners[cx][cy] = hash.at(test);
							}
							else
							{
								double angle = angleRNG(eng);
								Vec value = Vec(std::cos(angle), std::sin(angle));
								hash.emplace(test, value);

								corners[cx][cy] = value;
							}
						}
					}

					// Loop through this box and sum to map
					VecInt thisBoxSize = (Vec(bx, by) + octaveBoxSize).Floor() - Vec(bx, by).Floor();
					double dots[2][2];
					for (int ix = 0; ix < thisBoxSize.X; ix++)
					{
						if (base.X + ix > cfg.bounds.X) continue;

						for (int iy = 0; iy < thisBoxSize.Y; iy++)
						{
							if (base.Y + iy > cfg.bounds.Y) continue;

							Vec itl = Vec(ix, iy) / Vec(thisBoxSize);
							VecInt absolute = base + VecInt(ix, iy);

							dots[0][0] = (itl - Vec(0, 0)).Dot(corners[0][0]);
							dots[0][1] = (itl - Vec(0, 1)).Dot(corners[0][1]);
							dots[1][0] = (itl - Vec(1, 0)).Dot(corners[1][0]);
							dots[1][1] = (itl - Vec(1, 1)).Dot(corners[1][1]);

							double y0, y1, Z;
							y0 = interp5(dots[0][0], dots[1][0], itl.X);
							y1 = interp5(dots[0][1], dots[1][1], itl.X);
							Z = interp5(y0, y1, itl.Y);

							map.At(absolute) += Z * octInfluence;
						}
					}
				}
			}

			std::cout << " -> Octave " << oct + 1 << " finished.\n";
		}

		if (cfg.normalize) map.Interpolate(0, 1);

		return map;
	}

	Map Worley(const NoiseConfig& cfg)
	{
		// this coordList works for simpler algorithms that use fewer than ~5 points
		std::vector<Vec> coordList;
		for (int x = -2; x <= 2; x++)
			for (int y = -2; y <= 2; y++)
				coordList.push_back(Vec(x, y));

		// RNG
		std::default_random_engine eng(cfg.seed);
		std::uniform_real_distribution<double> uniformRNG(0, 1);

		std::cout << "Generating new Worley map:\n";
		std::cout << " -> Width:  " << cfg.bounds.X << "\n";
		std::cout << " -> Height: " << cfg.bounds.Y << "\n";
		std::cout << " -> Seed:   " << cfg.seed << "\n";

		Map map(cfg.bounds);

		// Allocate this here, no point in constantly de- and re-allocating it in the loop
		unsigned distanceLen = coordList.size(); // TODO: multiply by config.N once implemented
		std::vector<double> distances(distanceLen);

		for (int oct = 0; oct < cfg.octaves; oct++)
		{
			double octInfluence = std::pow(cfg.octDecrease, oct);
			Vec scaleVec = (Vec(1.0, 1.0) / cfg.boxSize) / std::pow(0.5, oct);

			std::unordered_map<Vec, Vec> hash;

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
						if (hash.find(test) == hash.end())
						{
							hash.emplace(test, Vec(uniformRNG(eng), uniformRNG(eng)));
						}

						distances[i] = (hash.at(test) + coordList[i] - itl).LNorm(cfg.lNorm);

						// uncomment for quantized distance; looks best with lnorm = 2
						// distances[i] = ((int)(distances[i] * 20.0)) / 20.0;
					}

					// Sort the distances, low to high. TODO: use a better sorting algorithm
					while (true)
					{
						bool done = true;
						for (unsigned i = 1; i < distanceLen; i++)
						{
							if (distances[i - 1] > distances[i])
							{
								double temp = distances[i];
								distances[i] = distances[i - 1];
								distances[i - 1] = temp;
								done = false;
							}
						}
						if (done) break;
					}

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
		}

		if (cfg.normalize) map.Interpolate(0, 1);

		return map;
	}

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
		std::default_random_engine eng(cfg.seed);
		std::uniform_real_distribution<double> uniformRNG(0, 1);

		std::cout << "Generating new Worleyplex map:\n";
		std::cout << " -> Width:  " << cfg.bounds.X << "\n";
		std::cout << " -> Height: " << cfg.bounds.Y << "\n";
		std::cout << " -> Seed:   " << cfg.seed << "\n";

		Map map(cfg.bounds);

		// Allocate this here, no point in constantly de- and re-allocating it in the loop
		unsigned distanceLen = coordList.size(); // TODO: multiply by config.N once implemented
		std::vector<double> distances(distanceLen);

		for (int oct = 0; oct < cfg.octaves; oct++)
		{
			double octInfluence = std::pow(cfg.octDecrease, oct);
			Vec scaleVec = (Vec(1.0, 1.0) / cfg.boxSize) / std::pow(0.5, oct);

			std::unordered_map<Vec, Vec> hash;

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
						if (hash.find(test) == hash.end())
						{
							hash.emplace(test, Vec(uniformRNG(eng), uniformRNG(eng)));
						}

						// This is where the base map is used
						distances[i] = (hash.at(test) + coordList[i] - itl).LNorm(baseMap[x][y]);

						// uncomment for quantized distance; looks best with lnorm = 2
						// distances[i] = ((int)(distances[i] * 20.0)) / 20.0;
					}

					// Sort the distances, low to high. TODO: use a better sorting algorithm
					while (true)
					{
						bool done = true;
						for (unsigned i = 1; i < distanceLen; i++)
						{
							if (distances[i - 1] > distances[i])
							{
								double temp = distances[i];
								distances[i] = distances[i - 1];
								distances[i - 1] = temp;
								done = false;
							}
						}
						if (done) break;
					}

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
		}

		if (cfg.normalize) map.Interpolate(0, 1);

		return map;
	}


}