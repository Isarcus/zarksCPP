#pragma once

namespace zmath
{
	typedef unsigned char uint8;

	typedef struct RGBA {
		uint8 R, G, B, A;

		RGBA(uint8 r, uint8 g, uint8 b, uint8 a);
		RGBA(uint8 r, uint8 g, uint8 b);
		RGBA(uint8 brightness, uint8 a);
		RGBA(uint8 brightness);
		RGBA();

		RGBA Opaque() const;
		RGBA Transparent() const;
		RGBA Negative() const;

		uint8& operator[] (int i);
		const uint8& operator[] (int i) const;
		bool operator== (RGBA c) const;

		static RGBA Black();
		static RGBA White();
		static RGBA Interpolate(const RGBA& c0, const RGBA& c1, double t);

		static double Distance(const RGBA& c0, const RGBA& c1);
	} RGBA;

	typedef struct Scheme {
		Scheme(int n, RGBA* colors, double* thresholds);
		Scheme(int n, RGBA* colors);

		int n; // number of colors
		RGBA* colors;
		double* thresholds; // should always be n-2
	} Scheme;
}