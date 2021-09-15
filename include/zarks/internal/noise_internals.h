#pragma once

#include <zarks/math/VecT.h>

#include <unordered_map>

namespace std
{
    // Necessary in order to use Vec as a key in an std::unordered_map
    template <typename T>
    struct hash<zmath::VecT<T>>
    {
        size_t operator()(const zmath::VecT<T>& k) const
        {
            static constexpr size_t multBy = 73; // 31
            size_t res = 17;
            res = res * multBy + hash<T>()(k.X);
            res = res * multBy + hash<T>()(k.Y);
            return res;
        }
    };
}
