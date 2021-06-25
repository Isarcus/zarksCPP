#pragma once
#include "VecT.h"
#include "Gauss.h"
#include "zmath_internals.h"

#include <string>

namespace zmath
{
	class Map
	{
		typedef VecT<double> Vec;
	public:
		Map(VecInt bounds);
		Map(int x, int y);
		Map(const Map& map);
		Map(Map&& map);
		~Map();

		// Accessors

		double* operator[](const int& x) const;

		double& At(const VecInt& pt);
		const double& At(const VecInt& pt) const;
		double& At(const int& x, const int& y);
		const double& At(const int& x, const int& y) const;

		void Set(VecInt pt, double val);
		void Set(int x, int y, double val);

		Map& operator= (const Map& m);
		Map& operator= (Map&& m);

		// Map characteristics

		double GetMin() const;
		double GetMax() const;
		std::pair<double, double> GetMinMax() const;
		VecInt Bounds() const;

		double Sum() const;
		double Mean() const;
		double Variance() const;
		double Std() const;

		bool ContainsCoord(VecInt pos) const;
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
		Map& Apply(const GaussField& gauss);
		Map& Apply(double(*calculation)(double));

		Map& SlopeMap();
		Map& BoundMax(double newMax);
		Map& BoundMin(double newMin);
		Map& Bound(double newMin, double newMax);

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

		VecInt bounds;
		double** data;
		bool subMap; // only true for maps created with operator() calls
	};
}