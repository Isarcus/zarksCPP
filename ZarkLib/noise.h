#pragma once

#include <cstdint>

#include "Map.h"

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

	Map& Simplex(NoiseConfig cfg);
	Map& Perlin(NoiseConfig cfg);
	Map& Worley(NoiseConfig cfg);

	Map& WorleyPlex(NoiseConfig cfg, Map& baseMap);
}