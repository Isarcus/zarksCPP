// This code simulates a one-dimensional cellular automaton, as described here:
// https://mathworld.wolfram.com/ElementaryCellularAutomaton.html

#include <zarks/image/Image.h>

#include <iostream>

using namespace zmath;

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./automaton <rule> <iters>" << std::endl;
        exit(1);
    }

    const int rule = std::atoi(argv[1]);
    const int iters = std::atoi(argv[2]);
    const int width = iters*2 + 1;
    std::cout << "Using rule " << rule << " to generate a " << width
              << "x" << iters << " image\n";

    Mat2D<bool> mat(width, iters);
    mat(iters, 0) = true;

    for (int i = 1; i < iters; i++)
    {
        for (int x = 0; x < width; x++)
        {
            int rule_query = 0;
            for (int dx = -1; dx <= 1; dx++)
            {
                int tx = x + dx;
                if (tx >= 0 && tx < width && mat(tx, i-1))
                {
                    rule_query += 1 << (1 - dx);
                }
            }

            int bit = 1 << rule_query;
            mat(x, i) = rule & bit;
        }
    }

    Image img(mat.Bounds());
    img.ApplySample(mat, [](bool b){
        return (b) ? RGBA::Black() : RGBA::White();
    });
    img.Save("automaton.png");
}
