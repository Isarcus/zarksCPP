#include <zarks/math/3D/Mat3.h>

#include <cmath>
#include <cstring>
#include <iomanip>
#include <string>

#define LOOP for (int row = 0; row < 3; row++) for (int col = 0; col < 3; col++)

namespace zmath
{

Mat3::Mat3() 
    : data{ {1, 0, 0},
            {0, 1, 0},
            {0, 0, 1} } {}

Mat3::Mat3(const double arr[3][3])
    : data{ {arr[0][0], arr[0][1], arr[0][2]},
            {arr[1][0], arr[1][1], arr[1][2]},
            {arr[2][0], arr[2][1], arr[2][2]} } {}

Mat3& Mat3::operator=(const Mat3& mat3)
{
    LOOP data[row][col] = mat3.data[row][col];

    return *this;
}

Mat3& Mat3::operator=(const double* const* arr)
{
    LOOP data[row][col] = arr[row][col];

    return *this;
}

double* Mat3::operator[](int row)
{
    return data[row];
}

const double* Mat3::operator[](int row) const
{
    return data[row];
}


Mat3 Mat3::operator+(const Mat3& mat3) const
{
    Mat3 ret;
    LOOP ret[row][col] = data[row][col] + mat3[row][col];

    return ret;
}

Mat3 Mat3::operator-(const Mat3& mat3) const
{
    Mat3 ret;
    LOOP ret[row][col] = data[row][col] - mat3[row][col];

    return ret;
}

Mat3 Mat3::operator*(const Mat3& mat3) const
{
    double ret[3][3];
    LOOP
    {
        ret[row][col] = data[row][0] * mat3[0][col] + data[row][1] * mat3[1][col] + data[row][2] * mat3[2][col];
    }
    return ret;
}

Mat3& Mat3::operator+=(const Mat3& mat3)
{
    LOOP data[row][col] += mat3[row][col];
    return *this;
}

Mat3& Mat3::operator-=(const Mat3& mat3)
{
    LOOP data[row][col] -= mat3[row][col];
    return *this;
}

Mat3& Mat3::operator*=(const Mat3& mat3)
{
    double result[3][3];
    LOOP
    {
        result[row][col] = data[row][0]*mat3[0][col] + data[row][1]*mat3[1][col] + data[row][2]*mat3[2][col];
    }

    memcpy(data, result, sizeof(result));

    return *this;
}

Vec3 Mat3::operator*(const Vec3& v3)
{
    return Vec3(
        v3.X*data[0][0] + v3.Y*data[0][1] + v3.Z*data[0][2],
        v3.X*data[1][0] + v3.Y*data[1][1] + v3.Z*data[1][2],
        v3.X*data[2][0] + v3.Y*data[2][1] + v3.Z*data[2][2]
    );
}

Mat3 Mat3::Rotation(double gamma, double beta, double alpha)
{
    using namespace std;

    double rotData[3][3]
    {
        {cos(alpha)*cos(beta), cos(alpha)*sin(beta)*sin(gamma) - sin(alpha)*cos(gamma), cos(alpha)*sin(beta)*cos(gamma) + sin(alpha)*sin(gamma)},
        {sin(alpha)*cos(beta), sin(alpha)*sin(beta)*sin(gamma) + cos(alpha)*cos(gamma), sin(alpha)*sin(beta)*cos(gamma) - cos(alpha)*sin(gamma)},
        {-sin(beta),       cos(beta)*sin(gamma),                        cos(beta)*cos(gamma)                       }
    };

    return Mat3(rotData);
}

Mat3 Mat3::RotationX(double theta)
{
    using namespace std;

    double rotData[3][3]
    {
        {1, 0, 0},
        {0, cos(theta), -sin(theta)},
        {0, sin(theta),  cos(theta)}
    };

    return Mat3(rotData);
}

Mat3 Mat3::RotationY(double theta)
{
    using namespace std;

    double rotData[3][3]
    {
        {cos(theta),  0, sin(theta)},
        {0,           1, 0},
        {-sin(theta), 0, cos(theta)}
    };

    return Mat3(rotData);
}

Mat3 Mat3::RotationZ(double theta)
{
    using namespace std;

    double rotData[3][3]
    {
        {cos(theta), -sin(theta), 0},
        {sin(theta),  cos(theta), 0},
        {0,           0,          1},
    };

    return Mat3(rotData);
}

std::ostream& operator<<(std::ostream& out, const Mat3& mat3)
{
    for (int row = 0; row < 3; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            std::string num = std::to_string(mat3[row][col]);
            out << std::setw(5) << num.substr(0, 5) << " ";
        }
        out << "\n";
    }

    return out;
}

} // namespace zmath
