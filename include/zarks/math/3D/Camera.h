#include <zarks/math/VecT.h>
#include <zarks/math/3D/Vec3.h>

#include <vector>

namespace zmath
{
    class Camera
    {
    public:
        Camera(Vec3 pos, double fovHorizontal, double fovVertical);
        Camera(Vec3 pos, Vec3 angle, double fovHorizontal, double fovVertical);

        Vec To2D(Vec3 coord) const;
        std::vector<Vec> To2D(const std::vector<Vec3>& coords) const;
        
        template <typename ITER_IN, typename ITER_OUT>
        void To2D(ITER_IN begin, ITER_IN end, ITER_OUT out);

        void LookAt(Vec3 pos);
        void LookAt(Vec3 pos, double roll);

        //         //
        // Setters //
        //         //

        void SetPos(Vec3 pos);
        void SetAngle(Vec3 angle);
        void SetPitch(double a);
        void SetYaw(double a);
        void SetRoll(double a);
        void SetFovHorizontal(double a);
        void SetFovVertical(double a);

        //         //  
        // Getters //
        //         //

        Vec3 GetPos() const;
        Vec3 GetAngle() const;
        double GetPitch() const;
        double GetYaw() const;
        double GetRoll() const;
        double GetFovHorizontal() const;
        double GetFovVertical() const;

    private:
        Vec3 pos;
        Vec3 angle; // pitch, yaw, roll
        double fovHorizontal, fovVertical;
    };
} // namespace zmath

//                //
// Implementation //
//                //

namespace zmath
{

template <typename ITER_IN, typename ITER_OUT>
void Camera::To2D(ITER_IN begin, ITER_IN end, ITER_OUT out)
{
    while (begin != end)
    {
        *out++ = To2D(*begin++);
    }
}

} // namespace zmath
