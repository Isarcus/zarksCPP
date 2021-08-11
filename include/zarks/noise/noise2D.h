#pragma once

#include <zarks/math/Map.h>

#include <cstdint>
#include <memory>

namespace zmath
{
	typedef struct GridConfig {
		GridConfig();
		GridConfig(Vec bounds, Vec boxSize);

		Vec	bounds;
		Vec boxSize;
	} GridConfig;

	typedef struct NoiseConfig : GridConfig {
		NoiseConfig();
		void NewSeed();

		// Universal noise inputs
		int		octaves;
		bool	normalize;
		int64_t seed;
		double  lNorm;

		double octDecrease; // factor by which each octave's influence is multiplied; usually 0.5

		// Simplex-specific values
		double r;		// this should be ~.625
		double rMinus;	// this should be 4.0 unless you're wockying the slush

		// Worley-specific values
		std::pair<int, int> nearest;
	} NoiseConfig;

	std::unique_ptr<Map> Simplex(NoiseConfig cfg);
	std::unique_ptr<Map> Perlin(NoiseConfig cfg);
	std::unique_ptr<Map> Worley(NoiseConfig cfg);

	std::unique_ptr<Map> WorleyPlex(NoiseConfig cfg, Map& baseMap);
}