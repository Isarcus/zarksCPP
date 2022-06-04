#include <zarks/math/GaussBlur.h>
#include <zarks/image/Image.h>

using namespace zmath;

int main()
{
    Mat2D<double> mat(VecInt(1000, 1000), [](int x, int y){
        return ((x / 100) + (y / 100)) % 2;
    });

    GaussianBlur(mat, 10);

    Image(mat).Save("gauss.png");
}
