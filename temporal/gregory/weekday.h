#pragma once

#include "temporal/types.h"

#include <optional>

namespace temporal::gregory
{

// ISO 8601 weekday numbering: Monday is 1 and Sunday is 7. Platforms disagree
// on the origin (C++ std::chrono, .NET and JavaScript count Sunday as 0; Java
// counts Monday as 1), so both conversions are named after the origin they
// accept instead of being implicit. Whether Saturday and Sunday form a weekend
// is a civil convention rather than astronomy, so isWeekend() lives here too.
enum class Weekday : unsigned char
{
    Monday = 1,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};

constexpr int isoNumber(Weekday value) noexcept
{
    return static_cast<int>(value);
}

constexpr int sundayNumber(Weekday value) noexcept
{
    return isoNumber(value) % 7;
}

constexpr std::optional<Weekday> weekdayFromIso(int value) noexcept
{
    if (value < 1 || value > 7)
    {
        return std::nullopt;
    }
    return static_cast<Weekday>(value);
}

constexpr std::optional<Weekday> weekdayFromSunday(int value) noexcept
{
    if (value < 0 || value > 6)
    {
        return std::nullopt;
    }
    if (value == 0)
    {
        return Weekday::Sunday;
    }
    return static_cast<Weekday>(value);
}

constexpr bool isWeekend(Weekday value) noexcept
{
    return value == Weekday::Saturday || value == Weekday::Sunday;
}

} // namespace temporal::gregory
