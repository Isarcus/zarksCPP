#include <zarks/io/bitmath.h>
#include <zarks/io/BitBuffer.h>
#include <zarks/io/numerals.h>
#include <zarks/io/logdefs.h>
#include <zarks/image/GIF.h>

#include <iostream>
#include <string>

using namespace zmath;

void testBitRange();
void testBitBuffer();
void testGIF();

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        LOG_ERROR("test_gif should be run with: ./testgif infile.gif")
        exit(1);
    }

    std::string infile = argv[1];
    GIF gif(infile);
    Image frame = gif.At(0);
    frame.Save("frame.png");

    gif.Save("gif.gif", true);

    return 0;
}
