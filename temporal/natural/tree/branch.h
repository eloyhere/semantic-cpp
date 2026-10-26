#pragma once

#include "temporal/types.h"
#include "cycle.h"

namespace temporal::natural::tree
{

// The twelve earthly branches. Values are 1-based so that absolute == 1 maps
// to Zi.
enum class Branch : unsigned char
{
    Zi = 1,
    Chou,
    Yin,
    Mao,
    Chen,
    Si,
    Wu,
    Wei,
    Shen,
    You,
    Xu,
    Hai
};

constexpr Branch branchOf(Timestamp absolute) noexcept
{
    return static_cast<Branch>(cycleIndex(absolute, 12));
}

} // namespace temporal::natural::tree
