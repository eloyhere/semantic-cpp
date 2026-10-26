#pragma once

#include "temporal/types.h"
#include "tree.h"

namespace temporal::natural::tree
{

// A sexagenary year. absolute is the ordinary 1-based year sequence from the
// epoch; relative is the 1..60 position inside the cycle.
class Year
{
  public:
    constexpr Year() noexcept : absolute(0), relative(0) {}

    explicit constexpr Year(Timestamp absoluteValue) noexcept
        : absolute(absoluteValue), relative(cycleIndex(absoluteValue, 60))
    {
    }

    constexpr Year(Timestamp absoluteValue, Timestamp relativeValue) noexcept
        : absolute(absoluteValue), relative(relativeValue)
    {
    }

    constexpr Timestamp getAbsolute() const noexcept { return absolute; }
    constexpr Timestamp getRelative() const noexcept { return relative; }
    constexpr Tree getTree() const noexcept { return Tree(absolute); }

    friend constexpr bool operator==(const Year &left, const Year &right) noexcept
    {
        return left.absolute == right.absolute && left.relative == right.relative;
    }

    friend constexpr bool operator!=(const Year &left, const Year &right) noexcept { return !(left == right); }

  private:
    Timestamp absolute;
    Timestamp relative;
};

} // namespace temporal::natural::tree
