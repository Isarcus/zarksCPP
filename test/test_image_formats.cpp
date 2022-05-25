#include <zarks/image/Image.h>

#include <filesystem>

namespace fs = std::filesystem;

using namespace zmath;

int main()
{
    Image img(400, 200);

    img.Apply([](int x, int y){
        int quad = x/100 + y/100 * 4;
        return RGBA(
            (quad & 0b001) ? 255 : 0,
            (quad & 0b010) ? 255 : 0,
            (quad & 0b100) ? 255 : 0
        );
    });

    // Create saving directory
    std::string stem("image_formats");
    fs::create_directory(stem);

    // Save images with all possible settings
    for (int i = 1; i <= 4; i++)
    {
        std::string str_i = std::to_string(i);
        img.Save(stem + "/test_PNG_" + str_i + ".png", Image::Format::PNG, i);
        img.Save(stem + "/test_JPG_" + str_i + ".jpg", Image::Format::JPG, i);
        img.Save(stem + "/test_BMP_" + str_i + ".bmp", Image::Format::BMP, i);
    }
}
