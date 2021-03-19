#pragma once

#include <limits>
#define DOUBLEMAX std::numeric_limits<double>::max()
#define DOUBLEMIN std::numeric_limits<double>::lowest() // whoever designed std naming was high on many drugs

namespace zmath
{
	typedef struct
	{
		double min;
		double max;
	} minmax;
}