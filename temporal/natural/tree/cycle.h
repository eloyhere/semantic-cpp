#pragma once

#include "temporal/types.h"

namespace temporal::natural::tree
{

constexpr Timestamp positiveModulo(Timestamp value, Timestamp divisor) noexcept
{
    const Timestamp remainder = value % divisor;
    return remainder < 0 ? remainder + divisor : remainder;
}

// absolute is a 1-based signed ordinal with no zero: 1, 2, ... and -1, -2, ...
// The value just before 1 is -1, so -1 maps to the last position of the cycle.
constexpr Timestamp cycleIndex(Timestamp absolute, Timestamp modulus) noexcept
{
    if (absolute > 0)
    {
        return positiveModulo(absolute - 1, modulus) + 1;
    }
    return modulus - positiveModulo(-absolute - 1, modulus);
}

} // namespace temporal::natural::tree
