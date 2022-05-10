#include <zarks/math/GaussField.h>

#include <cmath>

static constexpr double ACCEPTABLE_FLOAT_ERROR = 0.0000001;

namespace zmath
{
    GaussField::GaussField(const Vec& sigma, double amplitude, const Vec& center)
    : center(center)
    , sigma(sigma)
    , amplitude(amplitude)
{
    SetSigma(sigma);
}

GaussField::GaussField(double sigma, double amplitude, const Vec& center)
    : GaussField(Vec(sigma, sigma), amplitude, center)
{}

void GaussField::SetAmplitude(double val)
{
    amplitude = val;
}

void GaussField::SetSigma(double val)
{
    SetSigma(Vec(val, val));
}

void GaussField::SetSigma(const Vec& val)
{
    sigma = Vec::Max(Vec(), val);
}

void GaussField::SetCenter(const Vec& val)
{
    center = val;
}

double GaussField::operator()(const Vec& pos) const
{
    return (*this)(pos.X, pos.Y);
}

double GaussField::operator()(double x, double y) const
{
    return amplitude * std::exp(-1.0 * computeDimensionalWeights(x, y).Sum());
}

std::vector<std::pair<Vec, double>> GaussField::Points(double radius, int resolution) const
{
    std::vector<std::pair<Vec, double>> points;

    for (int x = 0; x <= resolution; x++)
    {
        for (int y = 0; y <= resolution; y++)
        {
            Vec pos = (Vec(x, y) - (Vec(resolution, resolution) / 2.0)) / (resolution / 2.0);

            if (pos.DistForm() <= 1.0 + ACCEPTABLE_FLOAT_ERROR)
            {
                pos *= radius;

                points.push_back({pos, (*this)(pos)});
            }
        }
    }

    return points;
}

std::vector<std::pair<VecInt, double>> GaussField::Points(int radius) const
{
    std::vector<std::pair<VecInt, double>> points;

    for (int x = -radius; x <= radius; x++)
    {
        for (int y = -radius; y <= radius; y++)
        {
            VecInt pos(x, y);

            if (pos.DistForm() <= radius + ACCEPTABLE_FLOAT_ERROR)
            {
                points.push_back({ pos + center, (*this)(pos + center) });
            }
        }
    }

    return points;
}

Vec GaussField::computeDimensionalWeights(double x, double y) const
{
    return Vec(
        computeWeight(x - center.X, sigma.X),
        computeWeight(y - center.Y, sigma.Y)
    );
}

double GaussField::computeWeight(double distance, double sigma)
{
    return std::pow(distance / sigma, 2) / 2.0;
}

} // namespace zmath
