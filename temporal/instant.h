#pragma once

#include "temporal/types.h"

#include <chrono>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>

#include "day.h"
#include "duration.h"
#include "field.h"
#include "unit.h"

namespace temporal
{

// An absolute instant, stored as a Day plus a nanosecond offset. The day is
// always counted on the proleptic scale: -2696-03-11 is day 1, and the day
// before it is day 0.
//
//   day.days   proleptic day number, 1-based
//   nanos      offset from the start of that day, normalised to
//              [0, 86400000000000)
//
// The constructor accepts any signed nanos and normalises it by carrying or
// borrowing, so equality, hashing and ordering never see two spellings of the
// same instant. Day only stores the day count; Instant owns the time of day and
// all instant arithmetic.
//
// The Unix scale is a pure conversion: epoch day 1 is 1970-01-01, related to the
// proleptic scale by ProlepticDayOfUnixEpoch. fromUnixSeconds/toUnixSeconds are
// exact and do not depend on the natural-calendar anchor except for that one
// constant; toProleptic/toEpoch expose both 1-based day numbers.
//
// The epoch is a day in the natural calendar, not in the Gregorian one. The two
// calendars do not agree on the day number of that label (Gregorian
// extrapolation is 14 days away from the natural epoch), so this class does not
// convert to Gregorian dates. OffsetDateTime/ZoneTemporal own the Unix-to-local
// conversion; natural will own the proleptic anchor.
//
// Leap seconds are not represented, as in java.time, Temporal and
// std::chrono::sys_time.
class Instant
{
  public:
    using NanosecondCount = Timestamp;

    constexpr Instant() noexcept : day{}, nanos(0) {}

    constexpr Instant(Timestamp days, Timestamp nanosecondsOfDay) : day(days), nanos(0)
    {
        normalize(nanosecondsOfDay);
    }

    explicit constexpr Instant(const Day &value, Timestamp nanosecondsOfDay = 0) : day(value), nanos(0)
    {
        normalize(nanosecondsOfDay);
    }

    constexpr const Day &getDay() const noexcept { return day; }
    constexpr NanosecondCount getNanosecondsOfDay() const noexcept { return nanos; }

    // Proleptic day number, 1-based: -2696-03-11 is day 1.
    constexpr Timestamp toProleptic() const noexcept { return day.getDays(); }

    // Epoch day number, 1-based: 1970-01-01 is day 1.
    constexpr Timestamp toEpoch() const
    {
        constexpr Timestamp offset = ProlepticDayOfUnixEpoch - 1;
        if (day.getDays() < std::numeric_limits<Timestamp>::min() + offset)
        {
            throw std::overflow_error("temporal: epoch day overflow");
        }
        return day.getDays() - offset;
    }

    // Exact Unix conversion. 1970-01-01T00:00:00Z is day 1, second 0.
    static constexpr Instant fromUnixSeconds(Timestamp seconds, Timestamp nanoseconds = 0)
    {
        Timestamp dayShift = seconds / SecondsPerDay;
        Timestamp secondOfDay = seconds % SecondsPerDay;
        if (secondOfDay < 0)
        {
            secondOfDay += SecondsPerDay;
            --dayShift;
        }
        constexpr Timestamp offset = ProlepticDayOfUnixEpoch - 1;
        if (dayShift > std::numeric_limits<Timestamp>::max() - offset - 1)
        {
            throw std::overflow_error("temporal: instant overflow");
        }
        return Instant(dayShift + 1 + offset, secondOfDay * NanosPerSecond + nanoseconds);
    }

    constexpr Timestamp toUnixSeconds() const
    {
        constexpr Timestamp offset = ProlepticDayOfUnixEpoch - 1;
        const Timestamp epochDay = day.getDays() - offset;
        return (epochDay - 1) * SecondsPerDay + nanos / NanosPerSecond;
    }

    static constexpr Instant fromUnixNanoseconds(Timestamp nanoseconds)
    {
        Timestamp dayShift = nanoseconds / NanosPerDay;
        Timestamp remainder = nanoseconds % NanosPerDay;
        if (remainder < 0)
        {
            remainder += NanosPerDay;
            --dayShift;
        }
        return fromUnixSeconds(dayShift * SecondsPerDay, remainder);
    }

    constexpr Timestamp toUnixNanoseconds() const
    {
        constexpr Timestamp maximum = std::numeric_limits<Timestamp>::max();
        constexpr Timestamp minimum = std::numeric_limits<Timestamp>::min();
        const Timestamp seconds = toUnixSeconds();
        if (seconds > maximum / NanosPerSecond || seconds < minimum / NanosPerSecond)
        {
            throw std::overflow_error("temporal: instant does not fit in Unix nanoseconds");
        }
        const Timestamp base = seconds * NanosPerSecond;
        if (base > maximum - nanos)
        {
            throw std::overflow_error("temporal: instant does not fit in Unix nanoseconds");
        }
        return base + nanos;
    }

    static Instant now()
    {
        const auto sinceUnix = std::chrono::system_clock::now() - std::chrono::system_clock::from_time_t(0);
        return fromUnixNanoseconds(std::chrono::duration_cast<std::chrono::nanoseconds>(sinceUnix).count());
    }

    constexpr int getHour() const noexcept { return static_cast<int>(nanos / NanosPerHour); }

    constexpr int getMinute() const noexcept { return static_cast<int>((nanos % NanosPerHour) / NanosPerMinute); }

    constexpr int getSecond() const noexcept { return static_cast<int>((nanos % NanosPerMinute) / NanosPerSecond); }

    constexpr int getNanosecond() const noexcept { return static_cast<int>(nanos % NanosPerSecond); }

    // Field access for table-driven code. Both day scales are available, plus
    // the clock fields of the instant's day.
    constexpr Timestamp get(Field field) const
    {
        switch (field)
        {
        case Field::ProlepticDay:
            return toProleptic();
        case Field::EpochDay:
            return toEpoch();
        case Field::HourOfDay:
            return getHour();
        case Field::MinuteOfHour:
            return getMinute();
        case Field::SecondOfMinute:
            return getSecond();
        case Field::NanosecondOfSecond:
            return getNanosecond();
        default:
            throw std::invalid_argument("temporal: field is not an instant field");
        }
    }

    constexpr bool isSupported(Field field) const noexcept
    {
        return field == Field::ProlepticDay || field == Field::EpochDay || field == Field::HourOfDay ||
               field == Field::MinuteOfHour || field == Field::SecondOfMinute || field == Field::NanosecondOfSecond;
    }

    // Unit-based arithmetic. Exact units (nanoseconds through weeks) are
    // converted to nanoseconds and added; months and years are rejected because
    // an instant has no calendar with which to resolve their length.
    constexpr Instant plus(Timestamp amount, Unit unit) const
    {
        const Timestamp factor = nanosecondsPerUnit(unit);
        if (factor == 0)
        {
            throw std::invalid_argument("temporal: a calendar unit cannot be applied to an instant");
        }
        return plusNanoseconds(multiply(amount, factor));
    }

    // Whole units from this instant to other, truncating towards zero.
    constexpr Timestamp until(const Instant &other, Unit unit) const
    {
        const Timestamp factor = nanosecondsPerUnit(unit);
        if (factor == 0)
        {
            throw std::invalid_argument("temporal: a calendar unit cannot be measured on an instant");
        }
        return other.differenceInNanoseconds(*this) / factor;
    }

    // Add or subtract an exact duration. Duration carries whole seconds, so
    // this is where Instant and Period/Duration meet: months and years cannot
    // reach an Instant because they have no exact length.
    constexpr Instant plus(const Duration &duration) const
    {
        Timestamp dayShift = duration.getSeconds() / SecondsPerDay;
        Timestamp seconds = duration.getSeconds() % SecondsPerDay;
        if (seconds < 0)
        {
            seconds += SecondsPerDay;
            --dayShift;
        }
        Timestamp value = nanos + seconds * NanosPerSecond + duration.getNanoseconds();
        const Timestamp extraDays = value / NanosPerDay;
        value %= NanosPerDay;
        const Timestamp totalDayShift = dayShift + extraDays;

        if (totalDayShift > 0 && day.getDays() > std::numeric_limits<Timestamp>::max() - totalDayShift)
        {
            throw std::overflow_error("temporal: instant overflow");
        }
        if (totalDayShift < 0 && day.getDays() < std::numeric_limits<Timestamp>::min() - totalDayShift)
        {
            throw std::overflow_error("temporal: instant overflow");
        }
        return Instant(day.getDays() + totalDayShift, value);
    }

    constexpr Instant minus(const Duration &duration) const { return plus(duration.negated()); }

    constexpr Instant operator+(const Duration &duration) const { return plus(duration); }
    constexpr Instant operator-(const Duration &duration) const { return minus(duration); }

    // Exact signed distance as a Duration; unlike differenceInNanoseconds this
    // has no 292-year limit, because Duration carries a 64-bit seconds field.
    constexpr Duration operator-(const Instant &other) const
    {
        constexpr Timestamp maximum = std::numeric_limits<Timestamp>::max();
        constexpr Timestamp minimum = std::numeric_limits<Timestamp>::min();
        if ((other.day.getDays() < 0 && day.getDays() > maximum + other.day.getDays()) ||
            (other.day.getDays() > 0 && day.getDays() < minimum + other.day.getDays()))
        {
            throw std::overflow_error("temporal: instant difference overflow");
        }
        const Timestamp dayDifference = day.getDays() - other.day.getDays();
        if (dayDifference > maximum / SecondsPerDay || dayDifference < minimum / SecondsPerDay)
        {
            throw std::overflow_error("temporal: instant difference overflow");
        }
        return Duration(dayDifference * SecondsPerDay, nanos - other.nanos);
    }

    // Add or subtract an exact number of nanoseconds. The offset may be
    // negative and larger than a day; the result is renormalised.
    constexpr Instant plusNanoseconds(Timestamp nanoseconds) const
    {
        Timestamp dayShift = nanoseconds / NanosPerDay;
        Timestamp remainder = nanoseconds % NanosPerDay;
        if (remainder < 0)
        {
            remainder += NanosPerDay;
            --dayShift;
        }

        Timestamp value = nanos + remainder;
        if (value >= NanosPerDay)
        {
            value -= NanosPerDay;
            ++dayShift;
        }

        if (dayShift > 0 && day.getDays() > std::numeric_limits<Timestamp>::max() - dayShift)
        {
            throw std::overflow_error("temporal: instant overflow");
        }
        if (dayShift < 0 && day.getDays() < std::numeric_limits<Timestamp>::min() - dayShift)
        {
            throw std::overflow_error("temporal: instant overflow");
        }
        return Instant(day.getDays() + dayShift, value);
    }

    constexpr Instant minusNanoseconds(Timestamp nanoseconds) const
    {
        if (nanoseconds == std::numeric_limits<Timestamp>::min())
        {
            throw std::overflow_error("temporal: instant overflow");
        }
        return plusNanoseconds(-nanoseconds);
    }

    constexpr Instant plusDays(Timestamp days) const
    {
        if (days > 0 && day.getDays() > std::numeric_limits<Timestamp>::max() - days)
        {
            throw std::overflow_error("temporal: instant overflow");
        }
        if (days < 0 && day.getDays() < std::numeric_limits<Timestamp>::min() - days)
        {
            throw std::overflow_error("temporal: instant overflow");
        }
        return Instant(day.getDays() + days, nanos);
    }

    // Exact signed distance in nanoseconds. Throws std::overflow_error when the
    // distance does not fit in 64 bits (about 292 years).
    constexpr Timestamp differenceInNanoseconds(const Instant &other) const
    {
        constexpr Timestamp maximum = std::numeric_limits<Timestamp>::max();
        constexpr Timestamp minimum = std::numeric_limits<Timestamp>::min();
        if ((other.day.getDays() < 0 && day.getDays() > maximum + other.day.getDays()) ||
            (other.day.getDays() > 0 && day.getDays() < minimum + other.day.getDays()))
        {
            throw std::overflow_error("temporal: instant difference overflow");
        }

        const Timestamp dayDifference = day.getDays() - other.day.getDays();
        if (dayDifference > maximum / NanosPerDay || dayDifference < minimum / NanosPerDay)
        {
            throw std::overflow_error("temporal: instant difference overflow");
        }

        Timestamp result = dayDifference * NanosPerDay;
        const Timestamp nanosecondDifference = nanos - other.nanos;
        if (nanosecondDifference > 0 && result > maximum - nanosecondDifference)
        {
            throw std::overflow_error("temporal: instant difference overflow");
        }
        if (nanosecondDifference < 0 && result < minimum - nanosecondDifference)
        {
            throw std::overflow_error("temporal: instant difference overflow");
        }
        return result + nanosecondDifference;
    }

    constexpr bool operator==(const Instant &other) const noexcept
    {
        return day.getDays() == other.day.getDays() && nanos == other.nanos;
    }

    constexpr bool operator!=(const Instant &other) const noexcept { return !(*this == other); }

    constexpr bool operator<(const Instant &other) const noexcept
    {
        if (day.getDays() != other.day.getDays())
        {
            return day.getDays() < other.day.getDays();
        }
        return nanos < other.nanos;
    }

    constexpr bool operator<=(const Instant &other) const noexcept { return *this < other || *this == other; }

    constexpr bool operator>(const Instant &other) const noexcept { return other < *this; }
    constexpr bool operator>=(const Instant &other) const noexcept { return other <= *this; }

  private:
    constexpr void normalize(Timestamp nanosecondsOfDay)
    {
        Timestamp dayShift = nanosecondsOfDay / NanosPerDay;
        Timestamp remainder = nanosecondsOfDay % NanosPerDay;
        if (remainder < 0)
        {
            remainder += NanosPerDay;
            --dayShift;
        }
        if (dayShift > 0 && day.getDays() > std::numeric_limits<Timestamp>::max() - dayShift)
        {
            throw std::overflow_error("temporal: instant overflow");
        }
        if (dayShift < 0 && day.getDays() < std::numeric_limits<Timestamp>::min() - dayShift)
        {
            throw std::overflow_error("temporal: instant overflow");
        }
        day = Day(day.getDays() + dayShift);
        nanos = remainder;
    }

    static constexpr Timestamp multiply(Timestamp amount, Timestamp factor)
    {
        if (amount > std::numeric_limits<Timestamp>::max() / factor ||
            amount < std::numeric_limits<Timestamp>::min() / factor)
        {
            throw std::overflow_error("temporal: instant overflow");
        }
        return amount * factor;
    }

    Day day;
    Timestamp nanos;
};

} // namespace temporal

namespace std
{
template <> struct hash<temporal::Instant>
{
    size_t operator()(const temporal::Instant &value) const noexcept
    {
        const size_t dayHash = hash<temporal::Timestamp>{}(value.getDay().getDays());
        const size_t nanosecondHash = hash<temporal::Timestamp>{}(value.getNanosecondsOfDay());
        return dayHash ^ (nanosecondHash + 0x9e3779b97f4a7c15ULL + (dayHash << 6) + (dayHash >> 2));
    }
};
} // namespace std
