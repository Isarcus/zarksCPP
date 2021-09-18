#pragma once

#include <cstdint>
#include <cstddef>
#include <zarks/io/bitdefs.h>
#include <zarks/internal/zmath_internals.h>

namespace zmath
{

    // @return The first `n` bits, starting from zero.
    //         For example, NBits(3) == 0b111
    constexpr inline size_t NBits(uint8_t n)
    {  
        if (n < BITS_IN_SIZE) {
            return BITS[n] - 1;
        } else if (n == BITS_IN_SIZE) {
            return SIZE_MAX;
        } else {
            throw std::runtime_error("Too many bits requested in NBits");
        }
    }

    // @return The first `n` bits, starting from the maximum
    //         allowable value of size_t. For example, NBitsRev(24)
    //         == 0xFFF0 0000 0000 0000.
    constexpr inline size_t NBitsRev(uint8_t n)
    {
        if (n < BITS_IN_SIZE) {
            return (BITS[n] - 1) << (BITS_IN_SIZE - n);
        } else if (n == BITS_IN_SIZE) {
            return SIZE_MAX;
        } else {
            throw std::runtime_error("Too many bits requested in NBits");
        }
    }

    // @return Bits in the range [a, b), starting from zero.
    //         For example, BitRange(3, 7) == 0b01111000. The
    //         order of `a` and `b` does not matter.
    constexpr inline size_t BitRange(uint8_t a, uint8_t b)
    {
        return NBits(a) ^ NBits(b);
    }

    // @return Bits in the range [a, b), starting from the maximum
    //         allowable value of size_t. For example, BitRange(4, 20)
    //         == 0x0FFF F000 0000 0000.
    //         The order of `a` and `b` does not matter.
    constexpr inline size_t BitRangeRev(uint8_t a, uint8_t b)
    {
        return NBitsRev(a) ^ NBitsRev(b);
    }

} // namespace zmath
