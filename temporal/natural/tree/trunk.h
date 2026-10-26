#pragma once

#include "temporal/types.h"
#include "cycle.h"

namespace temporal::natural::tree
{

// The ten heavenly stems. Values are 1-based so that absolute == 1 maps to Jia.
enum class Trunk : unsigned char
{
    Jia = 1,
    Yi,
    Bing,
    Ding,
    Wu,
    Ji,
    Geng,
    Xin,
    Ren,
    Gui
};

constexpr Trunk trunkOf(Timestamp absolute) noexcept
{
    return static_cast<Trunk>(cycleIndex(absolute, 10));
}

} // namespace temporal::natural::tree
