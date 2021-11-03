#pragma once

#include <iostream>
#include <vector>

namespace zmath
{
	typedef struct RGBA {
		uint8_t R, G, B, A;

		RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		RGBA(uint8_t r, uint8_t g, uint8_t b);
		RGBA(uint8_t brightness, uint8_t a);
		RGBA(uint8_t brightness);
		RGBA();

		RGBA Opaque() const;
		RGBA Transparent() const;
		RGBA Negative() const;

		uint8_t& operator[] (int i);
		uint8_t operator[] (int i) const;
		bool operator== (RGBA c) const;

		double Brightness(bool accountForAlpha = false) const;

		static RGBA Black();
		static RGBA White();
		static RGBA Interpolate(RGBA c0, RGBA c1, double t);

		static double Distance(RGBA c0, RGBA c1);

		friend std::ostream& operator<<(std::ostream& os, RGBA c);
	} RGBA;

	typedef struct Scheme {
		Scheme(const std::vector<RGBA>& colors, const std::vector<double>& thresholds);
		Scheme(const std::vector<RGBA>& colors);

		std::vector<RGBA> colors;
		std::vector<double> thresholds; // should always be n-2
	} Scheme;
}
