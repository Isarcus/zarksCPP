#pragma once
#include <zarks/math/VecT.h>
#include <zarks/math/GaussField.h>
#include <zarks/internal/Sampleable2D.h>

#include <string>
#include <memory>

namespace zmath
{
	class Map : public Sampleable2D<double>
	{
	private:
		Map();

	public:
		Map(VecInt bounds);
		Map(int x, int y);
		Map(const Map& map);
		Map(Map&& map);
		~Map();

		Map& operator= (const Map& m);
		Map& operator= (Map&& m);

		// Map characteristics

		double GetMin() const;
		double GetMax() const;
		std::pair<double, double> GetMinMax() const;

		double Sum() const;
		double Mean() const;
		double Variance() const;
		double Std() const;

		Vec DerivativeAt(VecInt pos) const;
		double SlopeAt(VecInt pos) const;

		// Return a deepcopy of an underlying section of this map
		Map Copy(VecInt min, VecInt max) const;
		Map operator() (VecInt min, VecInt max) const;

		// Chainable manipulation functions

		Map& Clear(double val);
		Map& Interpolate(double newMin, double newMax);
		Map& Abs();
		Map& FillBorder(int thickness, double val);
		Map& Fill(VecInt min, VecInt max, double val);
		Map& Replace(double val, double with);
		Map& Apply(const GaussField& gauss);
		Map& Apply(double(*calculation)(double));

		Map SlopeMap();
		Map& BoundMax(double newMax);
		Map& BoundMin(double newMin);
		Map& Bound(double newMin, double newMax);

		// Math operator overloads

		Map& operator+= (const Map& m);
		Map& operator-= (const Map& m);
		Map& operator*= (const Map& m);
		Map& operator/= (const Map& m);
		Map& operator+= (double val);
		Map& operator-= (double val);
		Map& operator*= (double val);
		Map& operator/= (double val);

		// Chainable functions

		Map& Add(const Map& m);
		Map& Sub(const Map& m);
		Map& Mul(const Map& m);
		Map& Div(const Map& m);
		Map& Add(double val);
		Map& Sub(double val);
		Map& Mul(double val);
		Map& Div(double val);

		Map& Pow(double exp);

		void Save(std::string path);

	private:
		bool subMap; // only true for maps created with operator() calls
	};
}
