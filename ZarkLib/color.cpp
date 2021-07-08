#include "pch.h"
#include "Color.h"

#include <cmath>
#include <exception>
#include <iostream>
#include <cassert>

namespace zmath
{

	zmath::RGBA::RGBA(uint8 r, uint8 g, uint8 b, uint8 a)
		: R(r)
		, G(g)
		, B(b)
		, A(a)
	{ }

	zmath::RGBA::RGBA(uint8 r, uint8 g, uint8 b)
		: R(r)
		, G(g)
		, B(b)
		, A(255)
	{ }

	RGBA::RGBA(uint8 brightness, uint8 a)
		: R(brightness)
		, G(brightness)
		, B(brightness)
		, A(a)
	{ }

	RGBA::RGBA(uint8 brightness)
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

	uint8& RGBA::operator[](int i)
	{
		switch (i)
		{
		case 0:  return R;
		case 1:  return G;
		case 2:  return B;
		case 3:  return A;
		}

		assert(false);
	}

	const uint8& RGBA::operator[](int i) const
	{
		switch (i)
		{
		case 0:  return R;
		case 1:  return G;
		case 2:  return B;
		case 3:  return A;
		}

		assert(false);
	}

	bool RGBA::operator==(RGBA c) const
	{
		return R == c.R && G == c.G && B == c.B && A == c.A;
	}

	double RGBA::Brightness(bool accountForAlpha) const
	{
		return ((int)R + (int)G + (int)B) * (accountForAlpha ? (A / 255.0) : 1) / 765.0;
	}

	RGBA RGBA::Black()
	{
		return RGBA();
	}

	RGBA RGBA::White()
	{
		return RGBA(255, 255, 255);
	}

	RGBA RGBA::Interpolate(const RGBA& c0, const RGBA& c1, double t)
	{
		double t0 = 1 - t;
		return RGBA(
			std::round(t0 * c0.R + t * c1.R),
			std::round(t0 * c0.G + t * c1.G),
			std::round(t0 * c0.B + t * c1.B),
			std::round(t0 * c0.A + t * c1.A)
		);
	}

	double RGBA::Distance(const RGBA& c0, const RGBA& c1)
	{
		return std::sqrt(
			std::pow((int)c0.R - (int)c1.R, 2) +
			std::pow((int)c0.G - (int)c1.G, 2) +
			std::pow((int)c0.B - (int)c1.B, 2)
		);
	}

	//			 //
	//	SCHEME	 //
	//			 //

	Scheme::Scheme(int n_, RGBA* colors_, double* thresholds_)
	{
		n = n_;
		colors = colors_;
		thresholds = thresholds_;
	}

	Scheme::Scheme(int n_, RGBA* colors_)
	{
		n = n_;
		colors = colors_;

		if (n < 3)
		{
			thresholds = nullptr;
			return;
		}

		thresholds = new double[n - 2];
		for (int i = 0; i < n - 2; i++) thresholds[i] = (i+1) / (double)(n - 1);
	}

	std::ostream& operator<<(std::ostream& os, const RGBA& c)
	{
		return os << "(" << (int)c.R << ", " << (int)c.G << ", " << (int)c.B << ", " << (int)c.A << ")";
	}

}