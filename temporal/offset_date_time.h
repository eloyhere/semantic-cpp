#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "instant.h"
#include "local_date.h"
#include "local_date_time.h"
#include "local_time.h"
#include "offset.h"
#include "overflow.h"
#include "period.h"

namespace temporal
{

// A local date-time plus a fixed UTC offset: the equivalent of java.time.
// OffsetDateTime, Temporal's plain date-time with an offset, and .NET's
// DateTimeOffset. It is a value with a well-defined position on the time line
// (local time minus offset), but it is not an Instant: it cannot resolve a
// time-zone rule, so it never has to deal with a DST gap or overlap.
//
// Conversion to Instant uses the exact Unix position; no natural-calendar
// extrapolation is involved.
class OffsetGregoryDateTime
{
  public:
    using date_time_type = LocalGregoryDateTime;
    using date_type = LocalGregoryDate;

    OffsetGregoryDateTime() = delete;

    constexpr OffsetGregoryDateTime(const date_time_type &dateTime, const Offset &offset) noexcept
        : dateTime(dateTime), offset(offset)
    {
    }

    static constexpr OffsetGregoryDateTime from(const date_time_type &dateTime, const Offset &offset) noexcept
    {
        return OffsetGregoryDateTime(dateTime, offset);
    }

    constexpr const date_time_type &getDateTime() const noexcept { return dateTime; }
    constexpr const date_type &getDate() const noexcept { return dateTime.getDate(); }
    constexpr const LocalGregoryTime &getTime() const noexcept { return dateTime.getTime(); }
    constexpr const Offset &getOffset() const noexcept { return offset; }

    constexpr gregory::Year getYear() const noexcept { return dateTime.getYear(); }
    constexpr gregory::Month getMonth() const noexcept { return dateTime.getMonth(); }
    constexpr int getDay() const noexcept { return dateTime.getDay(); }
    constexpr int getHour() const noexcept { return dateTime.getHour(); }
    constexpr int getMinute() const noexcept { return dateTime.getMinute(); }
    constexpr int getSecond() const noexcept { return dateTime.getSecond(); }
    constexpr int getNanosecond() const noexcept { return dateTime.getNanosecond(); }

    constexpr Timestamp get(Field field) const { return dateTime.get(field); }
    constexpr bool isSupported(Field field) const noexcept { return dateTime.isSupported(field); }

    constexpr OffsetGregoryDateTime plus(const Duration &duration) const
    {
        return OffsetGregoryDateTime(dateTime.plus(duration), offset);
    }

    constexpr OffsetGregoryDateTime minus(const Duration &duration) const
    {
        return OffsetGregoryDateTime(dateTime.minus(duration), offset);
    }

    constexpr OffsetGregoryDateTime plus(const Period &period, Overflow overflow = Overflow::Constrain) const
    {
        return OffsetGregoryDateTime(dateTime.plus(period, overflow), offset);
    }

    constexpr OffsetGregoryDateTime minus(const Period &period, Overflow overflow = Overflow::Constrain) const
    {
        return OffsetGregoryDateTime(dateTime.minus(period, overflow), offset);
    }

    constexpr OffsetGregoryDateTime plus(Timestamp amount, Unit unit) const
    {
        return OffsetGregoryDateTime(dateTime.plus(amount, unit), offset);
    }

    // Keep the represented instant and change only the offset: 08:00+08:00
    // becomes 00:00Z. This is the operation that must not be confused with
    // "reinterpret the same wall clock in another zone" (withOffsetSameLocal in
    // java.LocalGregoryTime, which has no counterpart here yet).
    constexpr OffsetGregoryDateTime withOffsetSameInstant(const Offset &offset) const
    {
        const Timestamp utcSeconds = getUtcSeconds();
        const Timestamp localSeconds = utcSeconds + offset.getSeconds();
        const Timestamp day = floorDiv(localSeconds, 86400);
        const Timestamp secondOfDay = localSeconds - day * 86400;
        if (day < std::numeric_limits<std::int32_t>::min() || day > std::numeric_limits<std::int32_t>::max())
        {
            throw std::out_of_range("temporal: offset date-time outside the supported range");
        }
        return OffsetGregoryDateTime(
            date_time_type(date_type::fromDayNumber(DayNumber(static_cast<std::int32_t>(day))),
                           LocalGregoryTime::ofNanoseconds(secondOfDay * NanosPerSecond + dateTime.getTime().getNanosecond())),
            offset);
    }

    // Seconds since 1970-01-01T00:00:00Z. This is the Unix position of the
    // represented instant, independent of Instant's natural-calendar epoch.
    constexpr Timestamp toUnixSeconds() const { return getUtcSeconds() - 719163LL * 86400; }

    constexpr Instant toInstant() const { return Instant::fromUnixSeconds(toUnixSeconds(), getNanosecond()); }

    static constexpr OffsetGregoryDateTime fromInstant(const Instant &instant, const Offset &offset)
    {
        return fromUnixSeconds(instant.toUnixSeconds(), offset)
            .plus(Duration(0, instant.getNanosecondsOfDay() % NanosPerSecond));
    }

    constexpr Duration durationUntil(const OffsetGregoryDateTime &other) const
    {
        return Duration(other.toUnixSeconds() - toUnixSeconds(), other.getNanosecond() - getNanosecond());
    }

    constexpr Duration operator-(const OffsetGregoryDateTime &other) const { return other.durationUntil(*this); }

    constexpr Timestamp until(const OffsetGregoryDateTime &other, Unit unit) const
    {
        if (isCalendarBased(unit) || unit == Unit::Days || unit == Unit::Weeks)
        {
            return dateTime.until(other.dateTime, unit);
        }
        const Timestamp factor = nanosecondsPerUnit(unit);
        if (factor == 0)
        {
            throw std::invalid_argument("temporal: unit cannot be measured on an offset date-time");
        }
        return durationUntil(other).toNanoseconds() / factor;
    }

    static constexpr OffsetGregoryDateTime fromUnixSeconds(Timestamp seconds, const Offset &offset)
    {
        constexpr Timestamp SecondsPerDay = 86400;
        constexpr Timestamp unixEpochDay = 719163;
        constexpr Timestamp maximum = std::numeric_limits<Timestamp>::max();
        constexpr Timestamp minimum = std::numeric_limits<Timestamp>::min();

        if (seconds > maximum - unixEpochDay * SecondsPerDay || seconds < minimum + unixEpochDay * SecondsPerDay)
        {
            throw std::out_of_range("temporal: unix seconds outside the supported range");
        }
        const Timestamp utcSeconds = seconds + unixEpochDay * SecondsPerDay;
        const Timestamp localSeconds = utcSeconds + offset.getSeconds();
        const Timestamp day = floorDiv(localSeconds, SecondsPerDay);
        const Timestamp secondOfDay = localSeconds - day * SecondsPerDay;
        if (day < std::numeric_limits<std::int32_t>::min() || day > std::numeric_limits<std::int32_t>::max())
        {
            throw std::out_of_range("temporal: offset date-time outside the supported range");
        }
        return OffsetGregoryDateTime(date_time_type(date_type::fromDayNumber(DayNumber(static_cast<std::int32_t>(day))),
                                                    LocalGregoryTime::ofNanoseconds(secondOfDay * NanosPerSecond)),
                                     offset);
    }

    std::string to_string() const { return dateTime.to_string() + offset.to_string(); }

    static constexpr std::optional<OffsetGregoryDateTime> tryParse(std::string_view text) noexcept
    {
        std::size_t bodyLength = 0;
        std::optional<Offset> offset;
        if (!text.empty() && (text.back() == 'Z' || text.back() == 'z'))
        {
            offset = Offset();
            bodyLength = text.size() - 1;
        }
        else if (text.size() >= 6 && (text[text.size() - 6] == '+' || text[text.size() - 6] == '-') &&
                 text[text.size() - 3] == ':')
        {
            offset = Offset::tryParse(text.substr(text.size() - 6));
            bodyLength = text.size() - 6;
        }
        else
        {
            return std::nullopt;
        }
        if (!offset)
        {
            return std::nullopt;
        }
        const std::optional<date_time_type> dateTime = date_time_type::tryParse(text.substr(0, bodyLength));
        if (!dateTime)
        {
            return std::nullopt;
        }
        return OffsetGregoryDateTime(*dateTime, *offset);
    }

    static constexpr OffsetGregoryDateTime parse(std::string_view text)
    {
        const std::optional<OffsetGregoryDateTime> value = tryParse(text);
        if (!value)
        {
            throw std::invalid_argument("temporal: invalid offset date-time text");
        }
        return *value;
    }

    // Ordering is by the represented instant, not by the local fields:
    // 2023-12-31T16:00:00Z and 2024-01-01T00:00:00+08:00 are equal.
    constexpr bool operator==(const OffsetGregoryDateTime &other) const noexcept
    {
        return getUtcSeconds() == other.getUtcSeconds() && getTime().getNanosecond() == other.getTime().getNanosecond();
    }

    constexpr bool operator!=(const OffsetGregoryDateTime &other) const noexcept { return !(*this == other); }

    constexpr bool operator<(const OffsetGregoryDateTime &other) const noexcept
    {
        const Timestamp seconds = getUtcSeconds();
        const Timestamp otherSeconds = other.getUtcSeconds();
        if (seconds != otherSeconds)
        {
            return seconds < otherSeconds;
        }
        return getTime().getNanosecond() < other.getTime().getNanosecond();
    }

    constexpr bool operator<=(const OffsetGregoryDateTime &other) const noexcept
    {
        return *this < other || *this == other;
    }

    constexpr bool operator>(const OffsetGregoryDateTime &other) const noexcept { return other < *this; }
    constexpr bool operator>=(const OffsetGregoryDateTime &other) const noexcept { return other <= *this; }

  private:
    constexpr Timestamp getUtcSeconds() const noexcept
    {
        const Timestamp localSeconds = static_cast<Timestamp>(dateTime.getDayNumber().getValue()) * 86400 +
                                       dateTime.getTime().getNanoseconds() / NanosPerSecond;
        return localSeconds - offset.getSeconds();
    }

    static constexpr Timestamp floorDiv(Timestamp value, Timestamp divisor) noexcept
    {
        Timestamp quotient = value / divisor;
        if (value % divisor != 0 && (value < 0) != (divisor < 0))
        {
            --quotient;
        }
        return quotient;
    }

    date_time_type dateTime;
    Offset offset;
};

inline std::ostream &operator<<(std::ostream &stream, const OffsetGregoryDateTime &value)
{
    return stream << value.to_string();
}

} // namespace temporal

namespace temporal::gregory
{
using OffsetDateTime = OffsetGregoryDateTime;
}

namespace std
{
template <> struct hash<temporal::OffsetGregoryDateTime>
{
    size_t operator()(const temporal::OffsetGregoryDateTime &value) const noexcept
    {
        const size_t secondHash = hash<temporal::Timestamp>{}(value.toUnixSeconds());
        const size_t nanosecondHash = hash<int>{}(value.getNanosecond());
        return secondHash ^ (nanosecondHash + 0x9e3779b97f4a7c15ULL + (secondHash << 6) + (secondHash >> 2));
    }
};
} // namespace std
