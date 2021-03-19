#pragma once

#include <string>

namespace zmath
{
	typedef struct Vec
	{
		double X;
		double Y;

		Vec();
		Vec(double x_, double y_);

		void Set(double x_, double y_);

		double Slope() const;
		double Area() const;
		double Min() const;
		double Max() const;
		double Sum() const;

		double Dot(Vec v) const;
		double DistForm(Vec v) const;
		double DistForm() const;
		double DistManhattan(Vec v) const;
		double LNorm(double L);

		Vec Floor() const;
		Vec Ceil() const;

		std::string String() const;

		Vec operator=  (Vec v);
		bool operator!() const;

		Vec operator+  (Vec v) const;
		Vec operator-  (Vec v) const;
		Vec operator*  (Vec v) const;
		Vec operator/  (Vec v) const;
		void operator+= (Vec v);
		void operator-= (Vec v);
		void operator*= (Vec v);
		void operator/= (Vec v);

		Vec operator+  (double val) const;
		Vec operator-  (double val) const;
		Vec operator*  (double val) const;
		Vec operator/  (double val) const;
		void operator+= (double val);
		void operator-= (double val);
		void operator*= (double val);
		void operator/= (double val);

		bool operator== (Vec v) const;
		bool operator!= (Vec v) const;
		bool operator<  (Vec v) const;
		bool operator>  (Vec v) const;
		bool operator<= (Vec v) const;
		bool operator>= (Vec v) const;

		friend std::ostream& operator<<(std::ostream& out, Vec v);

		static Vec Min(Vec v1, Vec v2);
		static Vec Max(Vec v1, Vec v2);
	} Vec;

	const Vec ZV(0, 0);
}