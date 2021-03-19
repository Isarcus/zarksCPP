#include "pch.h"
#include "noise.h"

#include <chrono>
#include <cmath>
#include <random>
#include <unordered_map>
#include <iostream>
#include <utility>

namespace std
{
	// Necessary in order to use Vec as a key in an std::unordered_map
	template <>
	struct hash<zmath::Vec>
	{
		size_t operator()(const zmath::Vec& k) const
		{
			const size_t multBy = 73; // 31 
			size_t res = 17;
			res = res * 73 + hash<double>()(k.X);
			res = res * 73 + hash<double>()(k.Y);
			return res;
		}
	};
}

namespace zmath
{
	// Default NoiseConfig values
	NoiseConfig::NoiseConfig()
	{
		bounds = Vec(1000, 1000);
		boxSizeInitial = 1000;
		octaves = 8;
		normalize = true;
		seed = std::chrono::system_clock::now().time_since_epoch().count(); // default seed is current system time
		lNorm = 2; // lNorm is normally 2 for the distance formula. Try 1 for manhattan distance, or other numbers!

		octDecrease = 0.5;

		// Simplex
		r = 0.625;
		rMinus = 4.0;

		// Worley
		nearest = {0, 2};
	}

	// Note: runs more than 2x faster than the Golang equivalent I made first. Woohoo!
	// This might actually be my fastest implementation yet, and it's not even fully optimized
	Map Simplex(NoiseConfig cfg)
	{
		// Refer to https://github.com/Isarcus/zarks/blob/main/zmath/noise/simplex.go for original code.

		// Constants
		const double F2D = 0.3660254037844386467637231707529361834714026;
		const double G2D = 0.2113248654051871177454256097490212721761991;
		auto skew =   [F2D](Vec pt) { return Vec(pt.X + pt.Sum() * F2D, pt.Y + pt.Sum() * F2D); };
		auto unskew = [G2D](Vec pt) { return Vec(pt.X - pt.Sum() * G2D, pt.Y - pt.Sum() * G2D); };
		double r2 = cfg.r * cfg.r;

		// RNG
		std::default_random_engine eng(cfg.seed);
		std::uniform_real_distribution<double> angleRNG(0, 2.0*std::_Pi);

		// Map setup
		cfg.bounds = cfg.bounds.Floor();
		Map map = *new Map(cfg.bounds);

		std::cout << "Creating new Simplex Map:\n";
		std::cout << " -> Width:  " << cfg.bounds.X << "\n";
		std::cout << " -> Height: " << cfg.bounds.Y << "\n";
		std::cout << " -> Seed:   " << cfg.seed << "\n";
		
		// Dive in
		for (int oct = 0; oct < cfg.octaves; oct++)
		{
			double octInfluence = std::pow(cfg.octDecrease, oct);
			double scale = 1.0 / (std::pow(0.5, oct) * cfg.boxSizeInitial);

			std::unordered_map<Vec, Vec> hash;

			for (int x = 0; x < cfg.bounds.X; x++)
			{
				for (int y = 0; y < cfg.bounds.Y; y++)
				{
					Vec ipt(scale * x, scale * y);
					Vec skewed = skew(ipt);
					Vec itl = skewed - skewed.Floor(); // internal simplex coordinate

					Vec corners[3];
					Vec vectors[3];

					corners[0] = skewed.Floor();
					corners[1] = (itl.X > itl.Y) ? corners[0] + Vec(1, 0) : corners[0] + Vec(0, 1);
					corners[2] = corners[0] + Vec(1, 1);

					for (int i = 0; i < 3; i++)
					{
						if (hash.find(corners[i]) == hash.end())
						{
							// if the current key does not exist, generate a new key-value pair
							double angle = angleRNG(eng);
							vectors[i] = Vec(std::cos(angle), std::sin(angle));

							hash.emplace(corners[i], vectors[i]);
						}
						else
						{
							// otherwise just use the existing value
							vectors[i] = hash.at(corners[i]);
						}
					}

					// Final summation
					double Z = 0;
					for (int i = 0; i < 3; i++)
					{
						Vec displacement = ipt - unskew(corners[i]);
						double distance = displacement.LNorm(cfg.lNorm); // Distance formula

						// change 4.0 value for cool effects!
						double influence = std::pow(std::max(0.0, r2 - distance * distance), cfg.rMinus);
						Z += influence * displacement.Dot(vectors[i]);
					}

					map.At(x, y) += Z * octInfluence;
				}
			} 

			std::cout << " -> Octave " << oct+1 << " Finished.\n";
		}

		if (cfg.normalize) map.Interpolate(0, 1);

		return map;
	}

	// Note to self: it seems like hash maps are a solid way to go for storing RNG noise vectors. This isn't as fast,
	// however, as the Golang implementation at https://github.com/Isarcus/zarks/blob/main/zmath/noise/perlin.go because
	// that one goes box-by-box rather than iterating across the image column-by-column. Still, this one is respectable!
	Map Perlin(NoiseConfig cfg)
	{
		// Helpful interpolation lambda
		auto interp5 = [](double i0, double i1, double t) {
			double weight = 6 * std::pow(t, 5) - 15 * std::pow(t, 4) + 10 * std::pow(t, 3);
			return weight * i1 + (1.0 - weight) * i0;
		};

		// RNG
		std::default_random_engine eng(cfg.seed);
		std::uniform_real_distribution<double> angleRNG(0, 2.0*std::_Pi);

		// Initialize map
		Map map = *new Map(cfg.bounds);

		std::cout << "Generating new Perlin map:\n";
		std::cout << " -> Width:  " << cfg.bounds.X << "\n";
		std::cout << " -> Height: " << cfg.bounds.Y << "\n";
		std::cout << " -> Seed:   " << cfg.seed << "\n";

		// Beep beep so let's ride
		for (int oct = 0; oct < cfg.octaves; oct++)
		{
			double octInfluence = std::pow(cfg.octDecrease, oct);
			double scale = 1.0 / (std::pow(0.5, oct) * cfg.boxSizeInitial);

			std::unordered_map<Vec, Vec> hash;

			for (int x = 0; x < cfg.bounds.X; x++)
			{
				for (int y = 0; y < cfg.bounds.Y; y++)
				{
					Vec coord(x * scale, y * scale);
					Vec base = coord.Floor();
					Vec itl = coord - base;
					
					double dots[2][2]; // Making El Truco proud
					for (int vx = 0; vx <= 1; vx++)
					{
						for (int vy = 0; vy <= 1; vy++)
						{
							// declaring vector here (instead of *always* using the hash) slightly improves
							// performance in high-octave passes
							Vec vector;

							// Randomly generate new vectors if not already present
							Vec test = base + Vec(vx, vy);
							if (hash.find(test) == hash.end())
							{
								double angle = angleRNG(eng);
								Vec value = Vec(std::cos(angle), std::sin(angle));
								hash.emplace(test, value);

								vector = value;
							}
							else
							{
								vector = hash.at(test);
							}

							// Get dot product
							dots[vx][vy] = (itl - Vec(vx, vy)).Dot(vector);
						}
					}

					double y0, y1, Z;
					y0 = interp5(dots[0][0], dots[1][0], itl.X);
					y1 = interp5(dots[0][1], dots[1][1], itl.X);
					Z = interp5(y0, y1, itl.Y);

					map.At(x, y) += Z * octInfluence;
				}
			}

			std::cout << " -> Octave " << oct+1 << " finished.\n";
		}

		if (cfg.normalize) map.Interpolate(0, 1);

		return map;
	}


	Map Worley(NoiseConfig cfg)
	{
		// this coordList works for simpler algorithms that use fewer than ~5 points
		const int coordLen = 25;
		Vec* coordList = new Vec[coordLen];
		int idx = 0;
		for (int x = -2; x <= 2; x++)
			for (int y = -2; y <= 2; y++)
				coordList[idx++] = Vec(x, y);

		// RNG
		std::default_random_engine eng(cfg.seed);
		std::uniform_real_distribution<double> uniformRNG(0, 1);

		std::cout << "Generating new Worley map:\n";
		std::cout << " -> Width:  " << cfg.bounds.X << "\n";
		std::cout << " -> Height: " << cfg.bounds.Y << "\n";
		std::cout << " -> Seed:   " << cfg.seed << "\n";

		Map m = *new Map(cfg.bounds);

		// Allocate this here, no point in constantly de- and re-allocating it in the loop
		double distanceLen = coordLen; // TODO: multiply by config.N once implemented
		double* distances = new double[distanceLen];

		for (int oct = 0; oct < cfg.octaves; oct++)
		{
			double octInfluence = std::pow(cfg.octDecrease, oct);
			double scale = 1.0 / (std::pow(0.5, oct) * cfg.boxSizeInitial);

			std::unordered_map<Vec, Vec> hash;

			for (int x = 0; x < cfg.bounds.X; x++)
			{
				for (int y = 0; y < cfg.bounds.Y; y++)
				{
					Vec coord(x * scale, y * scale);
					Vec base = coord.Floor();
					Vec itl = coord - base;

					// Get the distances to each point
					for (int i = 0; i < coordLen; i++)
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
						for (int i = 1; i < distanceLen; i++)
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
					for (int i = cfg.nearest.min; i < cfg.nearest.max; i++)
					{
						Z *= distances[i];
					}
					//std::cout << distances[1] << "\n";

					// Final summation
					m.At(x, y) += Z * octInfluence;
				}
			}
		}

		delete[] distances;

		if (cfg.normalize) m.Interpolate(0, 1);

		return m;
	}

	Map WorleyPlex(NoiseConfig cfg, Map& baseMap)
	{
		if (cfg.bounds != baseMap.Bounds())
		{
			std::cout << "Bounds mismatch :/ \n";
			return Map(Vec());
		}

		const int coordLen = 25;
		Vec* coordList = new Vec[coordLen];
		int idx = 0;
		for (int x = -2; x <= 2; x++)
			for (int y = -2; y <= 2; y++)
				coordList[idx++] = Vec(x, y);

		// RNG
		std::default_random_engine eng(cfg.seed);
		std::uniform_real_distribution<double> uniformRNG(0, 1);

		std::cout << "Generating new Worleyplex map:\n";
		std::cout << " -> Width:  " << cfg.bounds.X << "\n";
		std::cout << " -> Height: " << cfg.bounds.Y << "\n";
		std::cout << " -> Seed:   " << cfg.seed << "\n";

		Map m = *new Map(cfg.bounds);

		// Allocate this here, no point in constantly de- and re-allocating it in the loop
		double distanceLen = coordLen; // TODO: multiply by config.N once implemented
		double* distances = new double[distanceLen];

		for (int oct = 0; oct < cfg.octaves; oct++)
		{
			double octInfluence = std::pow(cfg.octDecrease, oct);
			double scale = 1.0 / (std::pow(0.5, oct) * cfg.boxSizeInitial);

			std::unordered_map<Vec, Vec> hash;

			for (int x = 0; x < cfg.bounds.X; x++)
			{
				for (int y = 0; y < cfg.bounds.Y; y++)
				{
					Vec coord(x * scale, y * scale);
					Vec base = coord.Floor();
					Vec itl = coord - base;

					// Get the distances to each point
					for (int i = 0; i < coordLen; i++)
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
						for (int i = 1; i < distanceLen; i++)
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
					for (int i = cfg.nearest.min; i < cfg.nearest.max; i++)
					{
						Z *= distances[i];
					}
					//std::cout << distances[1] << "\n";

					// Final summation
					m.At(x, y) += Z * octInfluence;
				}
			}
		}

		delete[] distances;

		if (cfg.normalize) m.Interpolate(0, 1);

		return m;
	}
}