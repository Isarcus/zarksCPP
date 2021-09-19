#include <zarks/io/bitmath.h>
#include <zarks/io/BitBuffer.h>
#include <zarks/io/numerals.h>
#include <zarks/image/GIF.h>

#include <iostream>

using namespace zmath;

void testBitRange();
void testBitBuffer();
void testGIF();

int main()
{
    testBitRange();
    testBitBuffer();
    testGIF();

    return 0;
}

void testBitRange()
{
    size_t val = BitRangeRev(0, 20);
    size_t val_corr = 0xFFFFF00000000000;

    if (val == val_corr) {
        std::cout << "All good!\n";
    } else {
        std::cout << "Grrrr! " << val << " " << val_corr << "\n";
    }

    std::cout << BitRange(3, 6) << " " << BitRange(6, 3) << "\n";
}

void testBitBuffer()
{
    BitBuffer bbuf;

    // 0b00000111
    //        ^
    bbuf << BitField(0b111, 3);
    // 0b00001111
    //      ^ 
    bbuf << BitField(0b01, 2);
    // 0b10001111
    //   ^
    bbuf << BitField(0b100, 3);

    for (int i = 8; i >= 0; i--)
    {
        std::cout << bbuf[i];
    }
    std::cout << "\n";
}

void testGIF()
{
    GIF gif("giflib-logo.gif");

    Image frame = gif.At(0);

    frame.Save("frame.png");
}
