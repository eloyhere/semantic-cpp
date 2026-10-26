#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace temporal
{

// A calendar period, not a duration: years and months are applied to a date
// through the calendar rules, so P1M (one month) is not 30 days. A period is
// applied in the order years, months, days, and that order is observable:
// 2024-01-31 + P1M1D is 2024-03-01, while adding the day first gives
// 2024-02-29 + 1 month = 2024-03-29. The order matches java.time.Period and
// Temporal.Duration, and is documented on BasicDate::plus().
class Period
{
  public:
    constexpr Period() noexcept : years(0), months(0), days(0) {}
    constexpr Period(Timestamp years, Timestamp months, Timestamp days) noexcept
        : years(years), months(months), days(days)
    {
    }

    static constexpr Period ofYears(Timestamp years) noexcept { return Period(years, 0, 0); }
    static constexpr Period ofMonths(Timestamp months) noexcept { return Period(0, months, 0); }
    static constexpr Period ofDays(Timestamp days) noexcept { return Period(0, 0, days); }

    constexpr Timestamp getYears() const noexcept { return years; }
    constexpr Timestamp getMonths() const noexcept { return months; }
    constexpr Timestamp getDays() const noexcept { return days; }

    constexpr bool isZero() const noexcept { return years == 0 && months == 0 && days == 0; }

    constexpr Period negated() const { return Period(negate(years), negate(months), negate(days)); }

    constexpr Period operator-() const { return negated(); }

    constexpr bool operator==(const Period &other) const noexcept
    {
        return years == other.years && months == other.months && days == other.days;
    }

    constexpr bool operator!=(const Period &other) const noexcept { return !(*this == other); }

  private:
    static constexpr Timestamp negate(Timestamp value)
    {
        if (value == std::numeric_limits<Timestamp>::min())
        {
            throw std::overflow_error("temporal: period overflow");
        }
        return -value;
    }

    Timestamp years;
    Timestamp months;
    Timestamp days;
};

} // namespace temporal
