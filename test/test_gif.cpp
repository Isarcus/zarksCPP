#include <zarks/io/bitmath.h>
#include <zarks/io/BitBuffer.h>
#include <zarks/io/numerals.h>
#include <zarks/io/logdefs.h>
#include <zarks/image/GIF.h>

#include <iostream>
#include <string>

using namespace zmath;

int main(int argc, char** argv)
{
    std::ios_base::sync_with_stdio(false);

    if (argc != 2)
    {
        LOG_ERROR("test_gif should be run with: ./testgif infile.gif")
        exit(1);
    }

    // Load in GIF from file
    std::string infile = argv[1];
    GIF gif(infile);
    Image frame = gif.At(0);
    frame.Save("frame.png");

    // Save just-loaded GIF in new file
    LOG_INFO("Re-saving GIF as gif.gif");
    gif.Save("gif.gif", true);
    gif.Save("gif_local.gif", false);

    // Ensure GIF was just saved correctly by loading it in
    LOG_INFO("Re-loading GIF at gif.gif")
    GIF reload("gif.gif");
    GIF reload_local("gif_local.gif");
    reload_local.Save("reload_local.gif");

    return 0;
}
