#pragma once

#include <string>

namespace zmath
{
    std::string ConvertBase(int val, int toBase, int minWidth);
    std::string ConvertBase(std::string val, int fromBase, int toBase, int minWidth);

    std::string DToS(int val);

    class BaseConverter
    {
    public:
        BaseConverter(int fromBase, int toBase, int minWidth = 0);
        
        std::string operator() (std::string val, int minWidth = -1);

        // for convenience with base 10 to something else
        std::string operator() (int val, int minWidth = -1);

    private:
        int fromBase, toBase;
        int minW;
    };
}
