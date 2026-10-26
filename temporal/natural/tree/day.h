#pragma once

#include "temporal/types.h"
#include "tree.h"

namespace temporal::natural::tree
{

// The day pillar is not stored: day 0 is -2696-03-11, which is Ding-Hai in the
// sexagenary cycle (index 24).
constexpr Tree dayTree(Timestamp daysSinceEpoch) noexcept
{
    return Tree::fromIndex(positiveModulo(daysSinceEpoch + 23, 60) + 1);
}

// The hour pillar is also computed, not stored. Shichen 0 is the Zi hour of
// Ding-Hai day: Geng-Zi, cycle index 37.
constexpr Tree hourTree(Timestamp shichenSinceEpoch) noexcept
{
    return Tree::fromIndex(positiveModulo(shichenSinceEpoch + 36, 60) + 1);
}

// 23:00-00:59 is Zi, 01:00-02:59 is Chou, ... 21:00-22:59 is Hai.
constexpr Timestamp shichenOfHour(int hour) noexcept
{
    return hour == 23 ? 0 : (hour + 1) / 2;
}

} // namespace temporal::natural::tree
