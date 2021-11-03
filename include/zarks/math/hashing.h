#pragma once

#include <zarks/math/VecT.h>

#include <functional>

namespace std
{

template <>
struct hash<zmath::VecInt>
{
    size_t operator()(zmath::VecInt k) const
    {
        static constexpr size_t multBy = 7919;
        size_t res = 17;
        res = res * multBy + hash<unsigned>()(k.X);
        res = res * multBy + hash<unsigned>()(k.Y);
        return res;
    }
};

template <>
struct hash<const char*>
{
    size_t operator()(const char* c)
    {
        size_t result = 0;
        constexpr size_t prime = 31;
        while (*c)
        {
            result = *c + (result * prime);
            c++;
        }
        return result;
    }
};

} // namespace std
