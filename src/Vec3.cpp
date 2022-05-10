#include <zarks/math/3D/Vec3.h>
#include <zarks/math/3D/Mat3.h>

#include <cmath>

namespace zmath
{

Vec3::Vec3()
    : X(0)
    , Y(0)
    , Z(0)
{}

Vec3::Vec3(double x, double y, double z)
    : X(x)
    , Y(y)
    , Z(z)
{}

Vec3::Vec3(double val)
    : X(val)
    , Y(val)
    , Z(val)
{}

double Vec3::Dot(const Vec3& v) const
{
    return X*v.X + Y*v.Y + Z*v.Z;
}

double Vec3::DistForm(const Vec3& v) const
{
    return std::sqrt(std::pow(X - v.X, 2) + std::pow(Y - v.Y, 2) + std::pow(Z - v.Z, 2));
}

double Vec3::DistForm() const
{
    return std::sqrt(X*X + Y*Y + Z*Z);
}

Vec3 Vec3::Rotate(double thetaX, double thetaY, double thetaZ) const
{
    return Mat3::Rotation(thetaX, thetaY, thetaZ) * (*this);
}

Vec3 Vec3::RotateX(double theta) const
{
    return Mat3::RotationX(theta) * (*this);
}

Vec3 Vec3::RotateY(double theta) const
{
    return Mat3::RotationY(theta) * (*this);
}

Vec3 Vec3::RotateZ(double theta) const
{
    return Mat3::RotationZ(theta) * (*this);
}

Vec3 Vec3::Rotate(double thetaX, double thetaY, double thetaZ, Vec3 around) const
{
    return around + (*this - around).Rotate(thetaX, thetaY, thetaZ);
}

Vec3 Vec3::RotateX(double theta, const Vec3& around) const
{
    return around + (*this - around).RotateX(theta);
}

Vec3 Vec3::RotateY(double theta, const Vec3& around) const
{
    return around + (*this - around).RotateY(theta);
}

Vec3 Vec3::RotateZ(double theta, const Vec3& around) const
{
    return around + (*this - around).RotateZ(theta);
}

Vec3 Vec3::Scale(const Vec3& by, const Vec3& around) const
{
    return Vec3(
        X + (X - around.X) * (by.X - 1),
        Y + (Y - around.Y) * (by.Y - 1),
        Z + (Z - around.Z) * (by.Z - 1)
    );
}

Vec3 Vec3::operator+(const Vec3& v) const { return Vec3(X + v.X, Y + v.Y, Z + v.Z); }
Vec3 Vec3::operator-(const Vec3& v) const { return Vec3(X - v.X, Y - v.Y, Z - v.Z); }
Vec3 Vec3::operator*(const Vec3& v) const { return Vec3(X * v.X, Y * v.Y, Z * v.Z); }
Vec3 Vec3::operator/(const Vec3& v) const { return Vec3(X / v.X, Y / v.Y, Z / v.Z); }

Vec3& Vec3::operator+=(const Vec3& v) { return *this = *this + v; }
Vec3& Vec3::operator-=(const Vec3& v) { return *this = *this - v; }
Vec3& Vec3::operator*=(const Vec3& v) { return *this = *this * v; }
Vec3& Vec3::operator/=(const Vec3& v) { return *this = *this / v; }

Vec3 Vec3::operator+(double val) const { return *this + Vec3(val); }
Vec3 Vec3::operator-(double val) const { return *this - Vec3(val); }
Vec3 Vec3::operator*(double val) const { return *this * Vec3(val); }
Vec3 Vec3::operator/(double val) const { return *this / Vec3(val); }

Vec3& Vec3::operator+=(double val) { return *this += Vec3(val); }
Vec3& Vec3::operator-=(double val) { return *this -= Vec3(val); }
Vec3& Vec3::operator*=(double val) { return *this *= Vec3(val); }
Vec3& Vec3::operator/=(double val) { return *this /= Vec3(val); }

bool Vec3::operator==(const Vec3& v) const { return X == v.X && Y == v.Y && Z == v.Z; }
bool Vec3::operator!=(const Vec3& v) const { return X != v.X || Y != v.Y || Z != v.Z; }
bool Vec3::operator< (const Vec3& v) const { return X <  v.X && Y <  v.Y && Z <  v.Z; }
bool Vec3::operator> (const Vec3& v) const { return X >  v.X && Y >  v.Y && Z >  v.Z; }
bool Vec3::operator<=(const Vec3& v) const { return X <= v.X && Y <= v.Y && Z <= v.Z; }
bool Vec3::operator>=(const Vec3& v) const { return X >= v.X && Y >= v.Y && Z >= v.Z; }

Vec3 Vec3::Min(const Vec3& v1, const Vec3& v2)
{
    return Vec3(
        std::min(v1.X, v2.X),
        std::min(v1.Y, v2.Y),
        std::min(v1.Z, v2.Z)
    );
}

Vec3 Vec3::Max(const Vec3& v1, const Vec3& v2)
{
    return Vec3(
        std::max(v1.X, v2.X),
        std::max(v1.Y, v2.Y),
        std::max(v1.Z, v2.Z)
    );
}

std::ostream& operator<<(std::ostream& out, const Vec3& v3)
{
    return out << "(" << v3.X << ", " << v3.Y << ", " << v3.Z << ")";
}

} // namespace zmath
