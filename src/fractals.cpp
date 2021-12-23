#include <zarks/noise/fractals.h>

#include <cmath>

namespace zmath
{

typedef std::complex<double> complex;

//               //
// FractalConfig //
//               //

FractalConfig::FractalConfig()
    : min(-1.5, -1.5)
    , max(1.5, 1.5)
    , pow(2.0)
    , reps(20)
{}

//                   //
// Fractal Functions //
//                   //

static auto getMandelbrotSampler(int reps, complex pow)
{
    return [reps, pow](complex point)
    {
        complex cur = point;
        for (int i = 0; i < reps; i++)
        {
            cur = std::pow(cur, pow);
            cur += point;

            if (std::abs(cur) > 2.0)
                return i;
        }

        return 0;
    };
} // getMandelbrotSampler()

static auto getJuliaSampler(int reps, complex pow, complex c)
{
    return [reps, pow, c](complex point)
    {
        for (int i = 0; i < reps; i++)
        {
            point = std::pow(point, pow);
            point += c;

            if (std::abs(point) > 2.0)
                return i + 1;
        }

        return 0;
    };
} // getJuliaSampler()

template <typename SAMPLER>
Map getFractal(VecInt mapBounds, complex min, complex max, SAMPLER sampler)
{
    Map map(mapBounds);
    complex scale = max - min;
    scale = complex(scale.real() / mapBounds.X, scale.imag() / mapBounds.Y);

    for (int x = 0; x < mapBounds.X; x++)
    {
        for (int y = 0; y < mapBounds.Y; y++)
        {
            complex point = complex(x * scale.real(), y * scale.imag()) + min;
            map[x][y] = sampler(point);
        }
    }

    return map;
} // getFractal()

Map Mandelbrot(VecInt mapBounds, FractalConfig cfg)
{
    auto sampler = getMandelbrotSampler(cfg.reps, cfg.pow);
    return getFractal(mapBounds, cfg.min, cfg.max, sampler);
} // Mandelbrot()

Map Julia(VecInt mapBounds, FractalConfig cfg)
{
    auto sampler = getJuliaSampler(cfg.reps, cfg.pow, cfg.c);
    return getFractal(mapBounds, cfg.min, cfg.max, sampler);
} // Julia()

} // namespace zmath
