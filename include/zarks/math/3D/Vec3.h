#pragma once

#include <iostream>

namespace zmath
{
    typedef struct Vec3
    {
        double X, Y, Z;

        Vec3();
        Vec3(double x, double y, double z);
        Vec3(double val);

        double Dot(const Vec3& v) const;
        double DistForm(const Vec3& v) const;
        double DistForm() const;

        Vec3 Rotate(double thetaX, double thetaY, double thetaZ) const;
        Vec3 RotateX(double theta) const;
        Vec3 RotateY(double theta) const;
        Vec3 RotateZ(double theta) const;
        Vec3 Rotate(double thetaX, double thetaY, double thetaZ, Vec3 around) const;
        Vec3 RotateX(double theta, const Vec3& around) const;
        Vec3 RotateY(double theta, const Vec3& around) const;
        Vec3 RotateZ(double theta, const Vec3& around) const;

        Vec3 Scale(const Vec3& by, const Vec3& around = Vec3()) const;
        Vec3 UnitVector() const;

        Vec3 operator+  (const Vec3& v) const;
        Vec3 operator-  (const Vec3& v) const;
        Vec3 operator/  (const Vec3& v) const;
        Vec3 operator*  (const Vec3& v) const;
        Vec3& operator+= (const Vec3& v);
        Vec3& operator-= (const Vec3& v);
        Vec3& operator*= (const Vec3& v);
        Vec3& operator/= (const Vec3& v);

        Vec3 operator+   (double val) const;
        Vec3 operator-   (double val) const;
        Vec3 operator/   (double val) const;
        Vec3 operator*   (double val) const;
        Vec3& operator+= (double val);
        Vec3& operator-= (double val);
        Vec3& operator*= (double val);
        Vec3& operator/= (double val);

        bool operator== (const Vec3& v) const;
        bool operator!= (const Vec3& v) const;
        bool operator<  (const Vec3& v) const;
        bool operator>  (const Vec3& v) const;
        bool operator<= (const Vec3& v) const;
        bool operator>= (const Vec3& v) const;

        static Vec3 Min(const Vec3& v1, const Vec3& v2);
        static Vec3 Max(const Vec3& v1, const Vec3& v2);

        friend std::ostream& operator<<(std::ostream& out, const Vec3& v3);
        
    } Vec3;
} // namespace zmath
