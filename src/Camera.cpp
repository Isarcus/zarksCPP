#include <zarks/math/3D/Camera.h>
#include <zarks/internal/zmath_internals.h>

#include <cmath>

namespace zmath
{

//         //
// Helpers //
//         //

static inline double modulo(double val, double n)
{
    return val - std::floor(val/n) * n;
}

static inline double getAngleDiff(double from, double to)
{
    return modulo(PI + from - to, PIX2) - PI;
}

static inline Vec3 getAngleBetween(Vec3 from, Vec3 to, double rollAngle = 0)
{
    Vec3 diff = to - from;
    double distHorizontal = distForm(diff.X, diff.Y);
    return Vec3(
        std::atan2(diff.Z, distHorizontal), // pitch
        std::atan2(diff.Y, diff.X),         // yaw
        rollAngle                           // roll
    );
}

//        //
// Camera //
//        //

Camera::Camera(Vec3 pos, double fovHorizontal, double fovVertical)
    : pos(pos)
    , fovHorizontal(fovHorizontal)
    , fovVertical(fovVertical)
{}

Camera::Camera(Vec3 pos, Vec3 angle, double fovHorizontal, double fovVertical)
    : pos(pos)
    , angle(angle)
    , fovHorizontal(fovHorizontal)
    , fovVertical(fovVertical)
{}

Vec Camera::To2D(Vec3 coord) const
{
    Vec3 angleToCoord = getAngleBetween(pos, coord);
    double pitch = angleToCoord.X - angle.X;
    double yaw = angleToCoord.Y - angle.Y;

    pitch = getAngleDiff(pitch, -fovVertical/2) / fovVertical;
    yaw = getAngleDiff(yaw, -fovHorizontal/2) / fovHorizontal;
    
    return Vec(yaw, pitch).Rotate(angle.Z, Vec(0.5, 0.5));
}

std::vector<Vec> Camera::To2D(const std::vector<Vec3>& coords) const
{
    std::vector<Vec> translated(coords.size());
    for (size_t i = 0; i < coords.size(); i++)
    {
        translated[i] = To2D(coords[i]);
    }

    return translated;
}

void Camera::LookAt(Vec3 at)
{
    angle = getAngleBetween(pos, at, angle.Z);
}

void Camera::LookAt(Vec3 at, double roll)
{
    angle = getAngleBetween(pos, at, roll);
}

//         //
// Setters //
//         //

void Camera::SetPos(Vec3 pos)
{
    this->pos = pos;
}

void Camera::SetAngle(Vec3 angle)
{
    this->angle = angle;
}

void Camera::SetPitch(double a)
{
    angle.X = a;
}

void Camera::SetYaw(double a)
{
    angle.Y = a;
}

void Camera::SetRoll(double a)
{
    angle.Z = a;
}

void Camera::SetFovHorizontal(double a)
{
    fovHorizontal = a;
}

void Camera::SetFovVertical(double a)
{
    fovVertical = a;
}

//         //  
// Getters //
//         //

Vec3 Camera::GetPos() const
{
    return pos;
}

Vec3 Camera::GetAngle() const
{
    return angle;
}

double Camera::GetPitch() const
{
    return angle.X;
}

double Camera::GetYaw() const
{
    return angle.Y;
}

double Camera::GetRoll() const
{
    return angle.Z;
}

double Camera::GetFovHorizontal() const
{
    return fovHorizontal;
}

double Camera::GetFovVertical() const
{
    return fovVertical;
}

} // namespace zmath



//   X -= cam.X; Y -= cam.Y; Z -= cam.Z;

//   // Get horizontal distance from cam to point
//   double dist_horizontal = dist(X, Y);

//   // Determine "absolute" pitch and yaw (relative to a camera with 0 yaw ad 0 pitch)
//   double pitch_absolute = std::atan2(Z, dist_horizontal);
//   double yaw_absolute = std::atan2(Y, X);

//   // Determine pitch and yaw relative to camera's direction
//   double pitch_relative = angle_diff(pitch_absolute, cam.pitch);
//   double yaw_relative = angle_diff(yaw_absolute, cam.yaw);

//   // Determine normalized pitch and yaw
//   double pitch_norm = angle_diff(pitch_relative, -cam.fov_vertical/2) / cam.fov_vertical;
//   double yaw_norm = angle_diff(yaw_relative, -cam.fov_horizontal/2) / cam.fov_horizontal;

//   return Vec{yaw_norm, pitch_norm};

