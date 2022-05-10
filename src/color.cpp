#include <zarks/image/color.h>

#include <cmath>
#include <exception>
#include <iostream>

namespace zmath
{

RGBA::RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    : R(r)
    , G(g)
    , B(b)
    , A(a)
{ }

RGBA::RGBA(uint8_t r, uint8_t g, uint8_t b)
    : R(r)
    , G(g)
    , B(b)
    , A(255)
{ }

RGBA::RGBA(uint8_t brightness, uint8_t a)
    : R(brightness)
    , G(brightness)
    , B(brightness)
    , A(a)
{ }

RGBA::RGBA(uint8_t brightness)
    : R(brightness)
    , G(brightness)
    , B(brightness)
    , A(255)
{ }

RGBA::RGBA()
    : R(0)
    , G(0)
    , B(0)
    , A(255)
{ }

RGBA RGBA::Opaque() const
{
    return RGBA(R, G, B);
}

RGBA RGBA::Transparent() const
{
    return RGBA(R, G, B, 0);
}

RGBA RGBA::Negative() const
{
    return RGBA(255 - R, 255 - G, 255 - B, A);
}

uint8_t& RGBA::operator[](int i)
{
    switch (i)
    {
    case 0:  return R;
    case 1:  return G;
    case 2:  return B;
    case 3:  return A;
    }

    throw std::runtime_error("Invalid RGBA index: " + std::to_string(i));
}

uint8_t RGBA::operator[](int i) const
{
    switch (i)
    {
    case 0:  return R;
    case 1:  return G;
    case 2:  return B;
    case 3:  return A;
    }

    throw std::runtime_error("Invalid RGBA index: " + std::to_string(i));
}

bool RGBA::operator==(RGBA c) const
{
    return R == c.R && G == c.G && B == c.B && A == c.A;
}

RGBA RGBA::operator* (double factor)
{
    return RGBA(
        std::max(0.0, std::min(R*factor, 255.0)),
        std::max(0.0, std::min(G*factor, 255.0)),
        std::max(0.0, std::min(B*factor, 255.0)),
        A
    );
}

double RGBA::Brightness(bool accountForAlpha) const
{
    return ((int)R + (int)G + (int)B) * (accountForAlpha ? (A / 255.0) : 1) / 765.0;
}

RGBA RGBA::Black()
{
    return RGBA{0, 0, 0, 255};
}

RGBA RGBA::White()
{
    return RGBA{255, 255, 255, 255};
}

RGBA RGBA::Interpolate(RGBA c0, RGBA c1, double t)
{
    double t0 = 1 - t;
    return RGBA(
        std::round(t0 * c0.R + t * c1.R),
        std::round(t0 * c0.G + t * c1.G),
        std::round(t0 * c0.B + t * c1.B),
        std::round(t0 * c0.A + t * c1.A)
    );
}

double RGBA::Distance(RGBA c0, RGBA c1)
{
    return std::sqrt(
        std::pow((int)c0.R - (int)c1.R, 2) +
        std::pow((int)c0.G - (int)c1.G, 2) +
        std::pow((int)c0.B - (int)c1.B, 2)
    );
}

//              //
//    SCHEME    //
//              //

Scheme::Scheme(const std::vector<RGBA>& colors, const std::vector<double>& thresholds)
    : colors(colors)
    , thresholds(thresholds)
{}

Scheme::Scheme(const std::vector<RGBA>& colors)
    : colors(colors)
{
    thresholds = std::vector<double>(std::max(0, (int)colors.size() - 2));
    for (unsigned i = 0; i < thresholds.size(); i++)
    {
        thresholds[i] = (i + 1) / (double)(colors.size() - 1);
    }
}

std::ostream& operator<<(std::ostream& os, RGBA c)
{
    return os << "(" << (int)c.R << ", " << (int)c.G << ", " << (int)c.B << ", " << (int)c.A << ")";
}

} // namespace zmath
