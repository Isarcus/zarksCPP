#pragma once

#include <zarks/math/VecT.h>

namespace zmath
{
	typedef struct Rect {
		Vec min;
		Vec max;

		Rect();
		Rect(Vec dimensions);
		Rect(Vec point1, Vec point2);
		Rect(double x1, double y1, double x2, double y2);

		//                    //
		// Features of a Rect //
		//                    //

		double Dx() const;
		double Dy() const;
		double Diagonal() const;
		double Area() const;
		double OverlapArea(Rect rect) const;
		Vec Dimensions() const;
		Vec Center() const;

		
		//                  //
		// Spatial Relation //
		//                  //

		bool Overlaps(Rect rect) const;
		bool Contains(Vec v) const;
		bool Contains(double x, double y);
		bool Contains(Rect r) const;

		void operator=(Rect rect);
		bool operator==(Rect rect) const;
		bool operator!() const;

		//              //
		// Manipulation //
		//              //

		Rect Expand(Vec v) const;
		Rect Expand(double x, double y) const;
		Rect Shrink(Vec v) const;
		Rect Shrink(double x, double y) const;
		Rect Intersection(Vec min, Vec max);
		Rect Intersection(Rect rect);
		Rect Shift(Vec v) const;
		Rect Shift(double x, double y) const;
		Rect Scale(double factor, Vec around = Vec(0, 0)) const;
		Rect Flip(bool xAxis = true, bool yAxis = true, Vec around = Vec(0, 0)) const;
		Rect Floor() const;
		Rect Ceil() const;

	} Rect;

	std::ostream& operator<<(std::ostream& out, const Rect& rect);
}