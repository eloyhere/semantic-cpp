#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>

#include "unit.h"

namespace temporal
{

// An exact signed duration, stored as whole seconds plus a nanoseconds remainder
// in [0, 1000000000). This is the counterpart of Period: Period is a calendar
// rule (P1M), Duration is an exact length (P30D or 30*86400 seconds), the same
// split as java.time's Duration/Period, Noda Time's Duration/Period and ISO
// 8601's time/calendar designators.
//
// Both components are signed inputs. The constructor normalises a negative or
// oversized nanosecond remainder by borrowing/carrying into seconds, so equality
// and hashing only ever see the canonical form.
class Duration
{
  public:
    constexpr Duration() noexcept : seconds(0), nanoseconds(0) {}

    constexpr Duration(Timestamp secondsValue, Timestamp nanosecondsValue) : seconds(0), nanoseconds(0)
    {
        Timestamp carry = nanosecondsValue / NanosPerSecond;
        Timestamp remainder = nanosecondsValue % NanosPerSecond;
        if (remainder < 0)
        {
            remainder += NanosPerSecond;
            --carry;
        }
        if (carry > 0 && secondsValue > std::numeric_limits<Timestamp>::max() - carry)
        {
            throw std::overflow_error("temporal: duration overflow");
        }
        if (carry < 0 && secondsValue < std::numeric_limits<Timestamp>::min() - carry)
        {
            throw std::overflow_error("temporal: duration overflow");
        }
        seconds = secondsValue + carry;
        nanoseconds = static_cast<std::int32_t>(remainder);
    }

    static constexpr Duration ofSeconds(Timestamp seconds) noexcept { return Duration(seconds, 0); }

    static constexpr Duration ofNanoseconds(Timestamp nanoseconds) noexcept { return Duration(0, nanoseconds); }

    // Exact units only: months and years are calendar rules and are rejected.
    static constexpr Duration of(Timestamp amount, Unit unit)
    {
        const Timestamp factor = nanosecondsPerUnit(unit);
        if (factor == 0)
        {
            throw std::invalid_argument("temporal: a calendar unit has no exact duration");
        }
        if (amount > std::numeric_limits<Timestamp>::max() / factor ||
            amount < std::numeric_limits<Timestamp>::min() / factor)
        {
            throw std::overflow_error("temporal: duration overflow");
        }
        return Duration(0, amount * factor);
    }

    constexpr Timestamp getSeconds() const noexcept { return seconds; }
    constexpr std::int32_t getNanoseconds() const noexcept { return nanoseconds; }

    constexpr bool isZero() const noexcept { return seconds == 0 && nanoseconds == 0; }
    constexpr bool isNegative() const noexcept { return seconds < 0; }

    constexpr Duration negated() const
    {
        if (seconds == std::numeric_limits<Timestamp>::min())
        {
            throw std::overflow_error("temporal: duration overflow");
        }
        return Duration(-seconds, -static_cast<Timestamp>(nanoseconds));
    }

    constexpr Duration operator-() const { return negated(); }

    constexpr Duration plus(const Duration &other) const
    {
        constexpr Timestamp maximum = std::numeric_limits<Timestamp>::max();
        constexpr Timestamp minimum = std::numeric_limits<Timestamp>::min();
        if (other.seconds > 0 && seconds > maximum - other.seconds)
        {
            throw std::overflow_error("temporal: duration overflow");
        }
        if (other.seconds < 0 && seconds < minimum - other.seconds)
        {
            throw std::overflow_error("temporal: duration overflow");
        }
        return Duration(seconds + other.seconds, static_cast<Timestamp>(nanoseconds) + other.nanoseconds);
    }

    constexpr Duration minus(const Duration &other) const { return plus(other.negated()); }

    constexpr Duration operator+(const Duration &other) const { return plus(other); }
    constexpr Duration operator-(const Duration &other) const { return minus(other); }

    // Total length in nanoseconds; throws for durations beyond +-292 years.
    constexpr Timestamp toNanoseconds() const
    {
        constexpr Timestamp maximum = std::numeric_limits<Timestamp>::max();
        constexpr Timestamp minimum = std::numeric_limits<Timestamp>::min();
        if (seconds > maximum / NanosPerSecond || seconds < minimum / NanosPerSecond)
        {
            throw std::overflow_error("temporal: duration does not fit in nanoseconds");
        }
        const Timestamp base = seconds * NanosPerSecond;
        if (base > maximum - nanoseconds)
        {
            throw std::overflow_error("temporal: duration does not fit in nanoseconds");
        }
        return base + nanoseconds;
    }

    constexpr bool operator==(const Duration &other) const noexcept
    {
        return seconds == other.seconds && nanoseconds == other.nanoseconds;
    }

    constexpr bool operator!=(const Duration &other) const noexcept { return !(*this == other); }

    constexpr bool operator<(const Duration &other) const noexcept
    {
        if (seconds != other.seconds)
        {
            return seconds < other.seconds;
        }
        return nanoseconds < other.nanoseconds;
    }

    constexpr bool operator<=(const Duration &other) const noexcept { return *this < other || *this == other; }

    constexpr bool operator>(const Duration &other) const noexcept { return other < *this; }
    constexpr bool operator>=(const Duration &other) const noexcept { return other <= *this; }

  private:
    Timestamp seconds;
    std::int32_t nanoseconds;
};

} // namespace temporal

namespace std
{
template <> struct hash<temporal::Duration>
{
    size_t operator()(const temporal::Duration &value) const noexcept
    {
        const size_t secondHash = hash<temporal::Timestamp>{}(value.getSeconds());
        const size_t nanosecondHash = hash<std::int32_t>{}(value.getNanoseconds());
        return secondHash ^ (nanosecondHash + 0x9e3779b97f4a7c15ULL + (secondHash << 6) + (secondHash >> 2));
    }
};
} // namespace std
