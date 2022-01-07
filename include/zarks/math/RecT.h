#pragma once

#include <zarks/math/VecT.h>

#include <iostream>

namespace zmath
{
    // forward declare RecT
	template <typename>
	struct RecT;

	// forward declare operator<<
	template <typename T>
	std::ostream& operator<<(std::ostream&, const RecT<T>&);

    template <typename T>
    class RecT
    {
    public:
        typedef VecT<T> V;

        RecT();
        RecT(V dimensions);
        RecT(V point1, V point2);
        RecT(T x1, T y1, T x2, T y2);

        RecT(const RecT& rect) = default;
        RecT(RecT&& rect) = default;
        RecT& operator=(const RecT& rect) = default;
        RecT& operator=(RecT&& rect) = default;

        //                    //
        // Features of a RecT //
        //                    //

        T Dx() const;
        T Dy() const;
        T Diagonal() const;
        T Area() const;
        T OverlapArea(RecT rect) const;
        V Dimensions() const;
        V Center() const;
        V Min() const;
        V Max() const;
        V TopLeft() const;
        V BottomRight() const;

        //                  //
        // Spatial Relation //
        //                  //

        bool Overlaps(RecT rect) const;
        bool Contains(V v) const;
        bool Contains(T x, T y) const;
        bool Contains(RecT r) const;
        bool Borders(RecT rect) const;
        bool Borders(V v) const;
        bool Borders(T x, T y) const;

        bool operator==(RecT rect) const;
        bool operator!=(RecT rect) const;
        bool operator!() const;

        //              //
        // Manipulation //
        //              //

        RecT Expand(V v) const;
        RecT Expand(T x, T y) const;
        RecT Shrink(V v) const;
        RecT Shrink(T x, T y) const;
        RecT Intersection(V min, V max) const;
        RecT Intersection(RecT rect) const;
        RecT Shift(V v) const;
        RecT Shift(T x, T y) const;
        RecT Scale(double factor, V around = V()) const;
        RecT Flip(bool xAxis = true, bool yAxis = true, V around = V()) const;
        RecT Floor() const;
        RecT Ceil() const;

        friend std::ostream& operator<< <>(std::ostream& os, const RecT& rect);

    private:
        V min;
        V max;
    };

    // Helpful typedefs
    typedef RecT<double> Rect;
    typedef RecT<int> RectInt;

} // namespace zmath

// Implementation
namespace zmath
{

template <typename T>
inline RecT<T>::RecT() : min(), max() {}

template <typename T>
inline RecT<T>::RecT(V dimensions) : RecT(dimensions, V()) {}

template <typename T>
inline RecT<T>::RecT(V point1, V point2)
    : min(V::Min(point1, point2)), max(V::Max(point1, point2))
{
}

template <typename T>
inline RecT<T>::RecT(T x1, T y1, T x2, T y2) : RecT(V(x1, y1), V(x2, y2)) {}

template <typename T>
inline T RecT<T>::Dx() const
{
    return max.X - min.X;
}

template <typename T>
inline T RecT<T>::Dy() const
{
    return max.Y - min.Y;
}

template <typename T>
inline T RecT<T>::Diagonal() const
{
    return std::sqrt(std::pow(Dx(), 2) + std::pow(Dy(), 2));
}

template <typename T>
inline T RecT<T>::Area() const
{
    return Dx() * Dy();
}

template <typename T>
inline T RecT<T>::OverlapArea(RecT<T> rect) const
{
    if (Overlaps(rect))
    {
        V overlapSides = V::Min(max, rect.max) - V::Max(min, rect.min);
        return overlapSides.Area();
    }
    else
    {
        return 0;
    }
}

template <typename T>
inline typename RecT<T>::V RecT<T>::Dimensions() const
{
    return max - min;
}

template <typename T>
inline typename RecT<T>::V RecT<T>::Center() const
{
    return (min + max) / 2;
}

template <typename T>
inline typename RecT<T>::V RecT<T>::Min() const
{
    return min;
}

template <typename T>
inline typename RecT<T>::V RecT<T>::Max() const
{
    return max;
}

template <typename T>
inline typename RecT<T>::V RecT<T>::TopLeft() const
{
    return V(min.X, max.Y);
}

template <typename T>
inline typename RecT<T>::V RecT<T>::BottomRight() const
{
    return V(max.X, min.Y);
}

template <typename T>
inline bool RecT<T>::Overlaps(RecT<T> rect) const
{
    return !(min.X > rect.max.X ||
             max.X < rect.min.X ||
             min.Y > rect.max.Y ||
             max.Y < rect.min.Y);
}

template <typename T>
inline bool RecT<T>::Contains(V v) const
{
    return v.X >= min.X &&
           v.X <= max.X &&
           v.Y >= min.Y &&
           v.Y <= max.Y;
}

template <typename T>
inline bool RecT<T>::Contains(T x, T y) const
{
    return Contains(V(x, y));
}

template <typename T>
inline bool RecT<T>::Contains(RecT<T> r) const
{
    return Contains(r.min) && Contains(r.max);
}

template <typename T>
inline bool RecT<T>::Borders(RecT<T> rect) const
{
    return Borders(rect.min) || Borders(rect.max);
}

template <typename T>
inline bool RecT<T>::Borders(V v) const
{
    return v.X == min.X ||
           v.X == max.X ||
           v.Y == min.Y ||
           v.Y == max.Y;
}

template <typename T>
inline bool RecT<T>::Borders(T x, T y) const
{
    return Borders(V(x, y));
}

template <typename T>
inline bool RecT<T>::operator==(RecT<T> rect) const
{
    return min == rect.min && max == rect.max;
}

template <typename T>
inline bool RecT<T>::operator!=(RecT<T> rect) const
{
    return !(*this == rect);
}

template <typename T>
inline bool RecT<T>::operator!() const
{
    return !min && !max;
}

//              //
// Manipulation //
//              //

template <typename T>
inline RecT<T> RecT<T>::Expand(V v) const
{
    RecT<T> r = *this;
    ((v.X > 0) ? r.max.X : r.min.X) += v.X;
    ((v.Y > 0) ? r.max.Y : r.min.Y) += v.Y;
    return r;
}

template <typename T>
inline RecT<T> RecT<T>::Expand(T x, T y) const
{
    return Expand(V(x, y));
}

template <typename T>
inline RecT<T> RecT<T>::Shrink(V v) const
{
    RecT<T> r = *this;

    // X
    if (v.X > 0)
        r.max.X = std::max(min.X, max.X - v.X);
    else
        r.min.X = std::min(max.X, min.X - v.X);

    // Y
    if (v.Y > 0)
        r.max.Y = std::max(min.Y, max.Y - v.Y);
    else
        r.min.Y = std::min(max.Y, min.Y - v.Y);

    return r;
}

template <typename T>
inline RecT<T> RecT<T>::Shrink(T x, T y) const
{
    return Shrink(V(x, y));
}

template <typename T>
inline RecT<T> RecT<T>::Intersection(V min_, V max_) const
{
    return Intersection(RecT<T>(min_, max_));
}

template <typename T>
inline RecT<T> RecT<T>::Intersection(RecT<T> rect) const
{
    if (Overlaps(rect))
    {
        return RecT(
            Vec::Max(rect.min, min),
            Vec::Min(rect.max, max)
        );
    }
    else
    {
        return RecT();
    }
}

template <typename T>
inline RecT<T> RecT<T>::Shift(V v) const
{
    return RecT<T>(
        min + v,
        max + v
    );
}

template <typename T>
inline RecT<T> RecT<T>::Shift(T x, T y) const
{
    return Shift(V(x, y));
}

template <typename T>
inline RecT<T> RecT<T>::Scale(double factor, V around) const
{
    RecT<T> r = *this;

    r.min += (min - around) * (factor - 1);
    r.max += (max - around) * (factor - 1);

    return r;
}

template <typename T>
inline RecT<T> RecT<T>::Flip(bool xAxis, bool yAxis, V around) const
{
    RecT<T> r = *this;

    if (xAxis)
    {
        // This is counterintuitive at first, but the min and max Y's
        // swap in a sense when you flip across the X axis
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

template <typename T>
inline RecT<T> RecT<T>::Floor() const
{
    return RecT(min.Floor(), max.Floor());
}

template <typename T>
inline RecT<T> RecT<T>::Ceil() const
{
    return RecT(min.Ceil(), max.Ceil());
}

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const RecT<T>& rect)
{
    return os << '{' << rect.min << ", " << rect.max << '}';
}

} // namespace zmath
