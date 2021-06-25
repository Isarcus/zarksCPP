#include "pch.h"

#include "Vec.h"
#include "zmath_internals.h"

#include <cmath>
#include <iostream>

/*
namespace zmath {
    Vec::Vec()
        : X(0)
        , Y(0)
    {}

    Vec::Vec(Vec&& v)
        : X(v.X)
        , Y(v.Y)
    {}

    Vec::Vec(const Vec& v)
        : X(v.X)
        , Y(v.Y)
    {}

    Vec::Vec(double x, double y)
        : X(x)
        , Y(y)
    {}

    void Vec::Set(double x_, double y_)
    {
        X = x_;
        Y = y_;
    }

    double Vec::Slope() const
    {
        if (X == 0)
        {
            if (Y > 0)  return DOUBLEMAX;
            if (Y < 0)  return DOUBLEMIN;
            return 0;
        }

        return X / Y;
    }

    double Vec::Area() const
    {
        double ret = X * Y;
        return (ret < 0) ? -ret : ret;
    }

    double Vec::Min() const
    {
        if (X < Y) return X;

        return Y;
    }

    double Vec::Max() const
    {
        if (X > Y) return X;

        return Y;
    }

    double Vec::Sum() const
    {
        return X + Y;
    }

    double Vec::Dot(Vec v) const
    {
        return X * v.X + Y * v.Y;
    }

    double Vec::DistForm(Vec v) const
    {
        return std::sqrt((X - v.X) * (X - v.X) + (Y - v.Y) * (Y - v.Y));
    }

    double Vec::DistForm() const
    {
        return std::sqrt(X * X + Y * Y);
    }

    double Vec::DistManhattan(Vec v) const
    {
        double dx = v.X - X;
        double dy = v.Y - Y;
        return ((dx < 0) ? -dx : dx) + ((dy < 0) ? -dy : dy);
    }

    double Vec::LNorm(double L)
    {
        if (L == 1) return std::abs(X) + std::abs(Y);
        else if (L == 2) return DistForm();

        return std::pow(std::pow(std::abs(X), L) + std::pow(std::abs(Y), L), 1.0 / L);
    }

    Vec Vec::Floor() const
    {
        return Vec(floor(X), floor(Y));
    }

    Vec Vec::Ceil() const
    {
        return Vec(ceil(X), ceil(Y));
    }

    Vec Vec::Abs() const
    {
        return Vec(std::abs(X), std::abs(Y));
    }

    Vec& Vec::operator=(Vec&& v)
    {
        X = v.X;
        Y = v.Y;
        return *this;
    }

    Vec& Vec::operator=(const Vec& v)
    {
        X = v.X;
        Y = v.Y;
        return *this;
    }

    bool Vec::operator!() const
    {
        return !(X || Y);
    }

    // Normal math operators
    Vec Vec::operator+(Vec v) const { return Vec(X + v.X, Y + v.Y); }
    Vec Vec::operator-(Vec v) const { return Vec(X - v.X, Y - v.Y);  }
    Vec Vec::operator*(Vec v) const { return Vec(X * v.X, Y * v.Y); }
    Vec Vec::operator/(Vec v) const
    {
        double x_, y_;
        if (v.X == 0)
        {
            if      (X > 0) x_ = DOUBLEMAX;
            else if (X < 0) x_ = DOUBLEMIN;
            else            x_ = 0;
        }
        else
        {
            x_ = X / v.X;
        }

        if (v.Y == 0)
        {
            if      (Y > 0) y_ = DOUBLEMAX;
            else if (Y < 0) y_ = DOUBLEMIN;
            else            y_ = 0;
        }
        else
        {
            y_ = Y / v.Y;
        }

        return Vec(x_, y_);
    }

    // Math= operators (modify the underlying Vec)
    Vec& Vec::operator+=(Vec v)
    {
        X += v.X;
        Y += v.Y;
        return *this;
    }

    Vec& Vec::operator-=(Vec v)
    {
        X -= v.X;
        Y -= v.Y;
        return *this;
    }

    Vec& Vec::operator*=(Vec v)
    {
        X *= v.X;
        Y *= v.Y;
        return *this;
    }

    Vec& Vec::operator/=(Vec v)
    {
        if (v.X == 0)
        {
            if      (X > 0) X = DOUBLEMAX;
            else if (X < 0) X = DOUBLEMIN;
            else            X = 0;
        }
        else
        {
            X /= v.X;
        }

        if (v.Y == 0)
        {
            if      (Y > 0) Y = DOUBLEMAX;
            else if (Y < 0) Y = DOUBLEMIN;
            else            Y = 0;
        }
        else
        {
            Y /= v.Y;
        }
        return *this;
    }

    Vec Vec::operator+(double val) const { return Vec(X + val, Y + val); }
    Vec Vec::operator-(double val) const { return Vec(X - val, Y - val); }
    Vec Vec::operator*(double val) const { return Vec(X * val, Y * val); }
    Vec Vec::operator/(double val) const
    {
        if (val == 0)
        {
            double x_, y_;

            if      (X > 0) x_ = DOUBLEMAX;
            else if (X < 0) x_ = DOUBLEMIN;
            else            x_ = 0;

            if      (Y > 0) y_ = DOUBLEMAX;
            else if (Y < 0) y_ = DOUBLEMIN;
            else            y_ = 0;

            return Vec(x_, y_);
        }

        return Vec(X / val, Y / val);
    }

    Vec& Vec::operator+=(double val)
    {
        X += val;
        Y += val;
        return *this;
    }

    Vec& Vec::operator-=(double val)
    {
        X -= val;
        Y -= val;
        return *this;
    }

    Vec& Vec::operator*=(double val)
    {
        X *= val;
        Y *= val;
        return *this;
    }

    Vec& Vec::operator/=(double val)
    {
        if (val == 0)
        {
            if      (X > 0) X = DOUBLEMAX;
            else if (X < 0) X = DOUBLEMIN;
            else            X = 0;

            if      (Y > 0) Y = DOUBLEMAX;
            else if (Y < 0) Y = DOUBLEMIN;
            else            Y = 0;
        }
        else
        {
            X /= val;
            Y /= val;
        }
        return *this;
    }

    bool Vec::operator==(Vec v) const { return (X == v.X && Y == v.Y); }
    bool Vec::operator!=(Vec v) const { return (X != v.X || Y != v.Y); }
    bool Vec::operator<(Vec v)  const { return (X < v.X && Y < v.Y); }
    bool Vec::operator>(Vec v)  const { return (X > v.X && Y > v.Y); }
    bool Vec::operator<=(Vec v) const { return (X <= v.X && Y <= v.Y); }
    bool Vec::operator>=(Vec v) const { return (X >= v.X && Y >= v.Y); }

    Vec Vec::Min(Vec v1, Vec v2)
    {
        return Vec(
            std::min(v1.X, v2.X),
            std::min(v1.Y, v2.Y)
        );
    }

    Vec Vec::Max(Vec v1, Vec v2)
    {
        return Vec(
            std::max(v1.X, v2.X),
            std::max(v1.Y, v2.Y)
        );
    }


    std::ostream& operator<<(std::ostream& out, Vec v)
    {
        return out << "(" << v.X << ", " << v.Y << ")";
    }

}
*/