#pragma once

#include "temporal/types.h"
#include "tree.h"

namespace temporal::natural::tree
{

// A sexagenary month. absolute is the ordinary 1-based month sequence from the
// epoch; relative is the 1..12 position inside the cycle.
class Month
{
  public:
    constexpr Month() noexcept : absolute(0), relative(0) {}

    explicit constexpr Month(Timestamp absoluteValue) noexcept
        : absolute(absoluteValue), relative(cycleIndex(absoluteValue, 12))
    {
    }

    constexpr Month(Timestamp absoluteValue, Timestamp relativeValue) noexcept
        : absolute(absoluteValue), relative(relativeValue)
    {
    }

    constexpr Timestamp getAbsolute() const noexcept { return absolute; }
    constexpr Timestamp getRelative() const noexcept { return relative; }

    // Month 1 is the Yin month of the Jia-Zi year: Bing-Yin, cycle index 3.
    // Shift the continuous (zero-based) month count by two positions.
    constexpr Tree getTree() const noexcept
    {
        const Timestamp continuous = absolute > 0 ? absolute - 1 : absolute;
        return Tree::fromIndex(positiveModulo(continuous + 2, 60) + 1);
    }

    friend constexpr bool operator==(const Month &left, const Month &right) noexcept
    {
        return left.absolute == right.absolute && left.relative == right.relative;
    }

    friend constexpr bool operator!=(const Month &left, const Month &right) noexcept { return !(left == right); }

  private:
    Timestamp absolute;
    Timestamp relative;
};

} // namespace temporal::natural::tree
