#pragma once

#include <zarks/math/Map.h>

#include <complex>

namespace zmath
{
    typedef struct FractalConfig
    {
        typedef std::complex<double> complex;

        FractalConfig();

        // The bounds to be evaluated in the complex plane.
        complex min, max;

        // The power to which to raise the point in each iteration.
        complex pow;

        // Julia only - the number to be added in each iteration.
        complex c;

        // The number of iterations to run on each point. Runtime should scale
        // linearly with this number.
        int reps;
    } FractalConfig;

    // @param mapBounds the bounds of the map to be returned.
    // @param cfg the configuration to use for this fractal.
    Map Mandelbrot(VecInt mapBounds, FractalConfig cfg);

    // @param mapBounds the bounds of the map to be returned.
    // @param cfg the configuration to use for this fractal.
    Map Julia(VecInt mapBounds, FractalConfig cfg);

} // namespace zmath
