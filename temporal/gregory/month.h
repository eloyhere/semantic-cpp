#pragma once

#include "temporal/types.h"

#include <optional>

namespace temporal::gregory
{

// Gregorian/Civil month names. The enumeration makes the ISO 8601 number
// 1..12 the only interpretation: every API that numbers months from zero
// (C tm_mon, JavaScript Date.getMonth, ICU Calendar.MONTH) has produced
// off-by-one bugs, so the number is exposed through number() and never through
// an implicit integer conversion.
enum class Month : unsigned char
{
    January = 1,
    February,
    March,
    April,
    May,
    June,
    July,
    August,
    September,
    October,
    November,
    December
};

constexpr int number(Month value) noexcept
{
    return static_cast<int>(value);
}

constexpr std::optional<Month> monthFromNumber(int value) noexcept
{
    if (value < 1 || value > 12)
    {
        return std::nullopt;
    }
    return static_cast<Month>(value);
}

} // namespace temporal::gregory
