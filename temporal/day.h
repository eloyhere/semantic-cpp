#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace temporal
{

// An absolute day number. Day owns only the day count; the time of day lives on
// Instant, which is the type that normalises and uses the pair.
class Day
{
  public:
    constexpr Day() noexcept : days(0) {}

    explicit constexpr Day(Timestamp value) noexcept : days(value) {}

    constexpr Timestamp getDays() const noexcept { return days; }

    friend constexpr Day operator+(const Day &value, Timestamp amount) { return Day(add(value.days, amount)); }

    friend constexpr Day operator+(Timestamp amount, const Day &value) { return Day(add(value.days, amount)); }

    friend constexpr Day operator-(const Day &value, Timestamp amount) { return Day(subtract(value.days, amount)); }

    friend constexpr Timestamp operator-(const Day &left, const Day &right) noexcept
    {
        return left.days - right.days;
    }

    constexpr Day &operator+=(Timestamp amount)
    {
        *this = *this + amount;
        return *this;
    }

    constexpr Day &operator-=(Timestamp amount)
    {
        *this = *this - amount;
        return *this;
    }

    friend constexpr bool operator==(const Day &left, const Day &right) noexcept { return left.days == right.days; }

    friend constexpr bool operator!=(const Day &left, const Day &right) noexcept { return !(left == right); }

    friend constexpr bool operator<(const Day &left, const Day &right) noexcept { return left.days < right.days; }

    friend constexpr bool operator<=(const Day &left, const Day &right) noexcept { return left.days <= right.days; }

    friend constexpr bool operator>(const Day &left, const Day &right) noexcept { return right < left; }

    friend constexpr bool operator>=(const Day &left, const Day &right) noexcept { return right <= left; }

  private:
    static constexpr Timestamp add(Timestamp value, Timestamp amount)
    {
        if (amount > 0 && value > std::numeric_limits<Timestamp>::max() - amount)
        {
            throw std::overflow_error("temporal: day overflow");
        }
        if (amount < 0 && value < std::numeric_limits<Timestamp>::min() - amount)
        {
            throw std::overflow_error("temporal: day overflow");
        }
        return value + amount;
    }

    static constexpr Timestamp subtract(Timestamp value, Timestamp amount)
    {
        if (amount > 0 && value < std::numeric_limits<Timestamp>::min() + amount)
        {
            throw std::overflow_error("temporal: day overflow");
        }
        if (amount < 0 && value > std::numeric_limits<Timestamp>::max() + amount)
        {
            throw std::overflow_error("temporal: day overflow");
        }
        return value - amount;
    }

    Timestamp days;
};

} // namespace temporal
