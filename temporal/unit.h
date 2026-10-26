#pragma once

#include "temporal/types.h"

#include <cstdint>

namespace temporal
{

constexpr Timestamp NanosPerSecond = 1000000000LL;
constexpr Timestamp NanosPerMinute = 60LL * NanosPerSecond;
constexpr Timestamp NanosPerHour = 60LL * NanosPerMinute;
constexpr Timestamp NanosPerDay = 24LL * NanosPerHour;
constexpr Timestamp SecondsPerMinute = 60;
constexpr Timestamp MinutesPerHour = 60;
constexpr Timestamp HoursPerDay = 24;
constexpr Timestamp SecondsPerDay = 86400;

// Proleptic day number of 1970-01-01 (1-based Unix epoch day 1).
constexpr Timestamp ProlepticDayOfUnixEpoch = 1704153;

// Units of temporal arithmetic. The split follows the fault line that every
// mature date-time API converges on (java.time ChronoUnit, Temporal Duration,
// Noda Time Duration vs Period, ISO 8601 P1M vs P30D):
//
//   time-based units   Nanoseconds .. Hours are exact durations
//   date-based units   Days and Weeks are exact multiples of 24 hours
//   calendar units     Months and Years are calendar rules, not durations:
//                      February has 28 or 29 days, so P1M is not P30D and the
//                      result depends on the date it is applied to
//
// A Unit is only an identifier. The actual arithmetic lives on the value that
// owns the relevant calendar: BasicDate rejects time-based units, Instant
// rejects months and years, and both accept days.
enum class Unit
{
    Nanoseconds,
    Microseconds,
    Milliseconds,
    Seconds,
    Minutes,
    Hours,
    Days,
    Weeks,
    Months,
    Years
};

constexpr bool isTimeBased(Unit unit) noexcept
{
    return unit == Unit::Nanoseconds || unit == Unit::Microseconds || unit == Unit::Milliseconds ||
           unit == Unit::Seconds || unit == Unit::Minutes || unit == Unit::Hours;
}

constexpr bool isDateBased(Unit unit) noexcept
{
    return !isTimeBased(unit);
}

constexpr bool isCalendarBased(Unit unit) noexcept
{
    return unit == Unit::Months || unit == Unit::Years;
}

// Exact length of a time-based unit in nanoseconds; 0 for a calendar unit (whose
// length cannot be known without a date). Days and Weeks are exact here because
// an Instant has no time zone and therefore no daylight-saving transitions.
constexpr Timestamp nanosecondsPerUnit(Unit unit) noexcept
{
    switch (unit)
    {
    case Unit::Nanoseconds:
        return 1;
    case Unit::Microseconds:
        return 1000;
    case Unit::Milliseconds:
        return 1000000;
    case Unit::Seconds:
        return 1000000000LL;
    case Unit::Minutes:
        return 60LL * 1000000000LL;
    case Unit::Hours:
        return 60LL * 60LL * 1000000000LL;
    case Unit::Days:
        return 24LL * 60LL * 60LL * 1000000000LL;
    case Unit::Weeks:
        return 7LL * 24LL * 60LL * 60LL * 1000000000LL;
    case Unit::Months:
    case Unit::Years:
        return 0;
    }
    return 0;
}

} // namespace temporal
