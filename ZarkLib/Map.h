#pragma once
#include "Vec.h"
#include "zmath_internals.h"

#include <string>

namespace zmath
{
	class Map
	{
	public:
		Map(Vec bounds);
		Map(int x, int y);
		Map(const Map& map);
		Map(Map&& map);
		~Map();

		// Accessors

		double* operator[](int x) const;

		double& At(Vec pt);
		const double& At(Vec pt) const;
		double& At(int x, int y);
		const double& At(int x, int y) const;

		void Set(Vec pt, double val);
		void Set(int x, int y, double val);

		void operator= (const Map& m);
		void operator= (Map&& m);

		// Map characteristics

		double GetMin() const;
		double GetMax() const;
		minmax GetMinMax() const;
		Vec Bounds() const;

		double Sum() const;
		double Mean() const;
		double Variance() const;
		double Std() const;

		bool ContainsCoord(Vec pos) const;
		Vec DerivativeAt(Vec pos) const;
		double GradientAt(Vec pos) const;
		double SlopeAt(Vec pos) const;

		// Chainable manipulation functions
		
		Map& Copy() const;
		Map& Copy(Vec min, Vec max) const;
		Map& operator() (Vec min, Vec max) const; // return a reference to an underlying section of the map
		Map& Clear(double val);
		Map& Interpolate(double newMin, double newMax);
		Map& Abs();

		Map& SlopeMap();

		// Math operator overloads

		Map& operator+= (Map& m);
		Map& operator-= (Map& m);
		Map& operator*= (Map& m);
		Map& operator/= (Map& m);
		Map& operator+= (double val);
		Map& operator-= (double val);
		Map& operator*= (double val);
		Map& operator/= (double val);

		// Chainable functions

		Map& Add(Map& m);
		Map& Sub(Map& m);
		Map& Mul(Map& m);
		Map& Div(Map& m);
		Map& Add(double val);
		Map& Sub(double val);
		Map& Mul(double val);
		Map& Div(double val);

		Map& Pow(double exp);

		void Save(std::string path);

	private:
		Map(); // for use in operator() overload

		Vec bounds;
		double** data;
		bool subMap; // only true for maps created with operator() calls
	};
}