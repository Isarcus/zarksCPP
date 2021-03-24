#pragma once

#include <limits>
#define DOUBLEMAX std::numeric_limits<double>::max()
#define DOUBLEMIN std::numeric_limits<double>::lowest() // whoever designed std naming was high on many drugs

#define ZM_PI 3.14159265358979323846264
#define ZM_PID2 ZM_PI / 2.0

namespace zmath
{
	typedef struct
	{
		double min;
		double max;
	} minmax;
}