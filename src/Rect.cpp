#include <zarks/math/Rect.h>

#include <iostream>

namespace zmath
{

Rect::Rect()
{
	min = Vec();
	max = Vec();
}

Rect::Rect(Vec dimensions)
{
	min = Vec(0, 0);
	max = dimensions;
}

Rect::Rect(Vec point1, Vec point2)
{
	min = Vec::Min(point1, point2);
	max = Vec::Max(point1, point2);
}

Rect::Rect(double x1, double y1, double x2, double y2)
{
	min = Vec(std::min(x1, x2), std::min(y1, y2));
	max = Vec(std::max(x1, x2), std::max(y1, y2));
}

double Rect::Dx() const
{
	return max.X - min.X;
}

double Rect::Dy() const
{
	return max.Y - min.Y;
}

double Rect::Diagonal() const
{
	return max.DistForm(min);
}

double Rect::Area() const
{
	return Dx() * Dy();
}

double Rect::OverlapArea(Rect rect) const
{
	if (Overlaps(rect))
	{
		Vec overlapSides = Vec::Min(max, rect.max) - Vec::Max(min, rect.min);
		return overlapSides.Area();
	}
	return 0;
}

Vec Rect::Dimensions() const
{
	return max - min;
}

Vec Rect::Center() const
{
	return (max - min) / 2 + min;
}

bool Rect::Overlaps(Rect rect) const
{
	return !(min.X >= rect.max.X ||
				max.X <= rect.min.X ||
				min.Y >= rect.max.Y ||
				max.Y <= rect.min.Y);
}

bool Rect::Contains(Vec v) const
{
	return	v.X > min.X &&
			v.X < max.X &&
			v.Y > min.Y &&
			v.Y < max.Y;
}

bool Rect::Contains(double x, double y)
{
	return Contains(Vec(x, y));
}

bool Rect::Contains(Rect r) const
{
	return Contains(r.min) && Contains(r.max);
}

void Rect::operator=(Rect rect)
{
	min = rect.min;
	max = rect.max;
}

bool Rect::operator==(Rect rect) const
{
	return min == rect.min && max == rect.max;
}

bool Rect::operator!() const
{
	return !min && !max;
}

Rect Rect::Expand(Vec v) const
{
	Rect r = *this;
	if (v.X > 0) r.max.X += v.X;
	else		 r.min.X += v.X;

	if (v.Y > 0) r.max.Y += v.Y;
	else		 r.min.Y += v.Y;

	return r;
}

Rect Rect::Expand(double x, double y) const
{
	return Expand(Vec(x, y));
}

Rect Rect::Shrink(Vec v) const
{
	Rect r = *this;

	if (v.X > 0) r.max.X = std::max(min.X, max.X - v.X);
	else		 r.min.X = std::min(max.X, min.X - v.X);

	if (v.Y > 0) r.max.Y = std::max(min.Y, max.Y - v.Y);
	else		 r.min.Y = std::min(max.Y, min.Y - v.Y);

	return r;
}

Rect Rect::Shrink(double x, double y) const
{
	return Shrink(Vec(x, y));
}

Rect Rect::Intersection(Vec min_, Vec max_)
{
	return Intersection(Rect(min_, max_));
}

Rect Rect::Intersection(Rect rect)
{
	if (Overlaps(rect))
	{
		return Rect(
			Vec::Max(rect.min, min),
			Vec::Min(rect.max, max)
		);
	}

	return Rect();
}

Rect Rect::Shift(Vec v) const
{
	return Rect(min + v, max + v);
}

Rect Rect::Shift(double x, double y) const
{
	return Shift(Vec(x, y));
}

Rect Rect::Scale(double factor, Vec around) const
{
	Rect r = *this;

	r.min += (min - around) * (factor - 1);
	r.max += (max - around) * (factor - 1);

	return r;
}

Rect Rect::Flip(bool xAxis, bool yAxis, Vec around) const
{
	Rect r = *this;

	if (xAxis)
	{
		// This is counterintuitive at first, but the min and max Y's swap in a sense when you flip across the X axis
		r.min.Y = max.Y + (around.Y - max.Y) * 2;
		r.max.Y = min.Y + (around.Y - min.Y) * 2;
	}
	if (yAxis)
	{
		// Same swapping happens here for X across Y axis 
		r.min.X = max.X + (around.X - max.X) * 2;
		r.max.X = min.X + (around.X - min.X) * 2;
	}

	return r;
}

Rect Rect::Floor() const
{
	return Rect(min.Floor(), max.Floor());
}

Rect Rect::Ceil() const
{
	return Rect(min.Ceil(), max.Ceil());
}

std::ostream& operator<<(std::ostream& os, const Rect& rect)
{
	return os << "{" << rect.min << " " << rect.max << "}";
}

} // namespace zmath
