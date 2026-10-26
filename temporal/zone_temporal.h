#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>

#include "calendar.h"
#include "duration.h"
#include "instant.h"
#include "local_date_time.h"
#include "local_time.h"
#include "offset_date_time.h"
#include "overflow.h"
#include "period.h"
#include "unit.h"
#include "zone.h"

namespace temporal
{

// A local date-time together with the rules of a time zone. This is the
// equivalent of java.LocalTime.ZonedDateTime and Temporal.ZonedDateTime, named after
// the two pieces it joins: the calendar date-time and the zone.
//
// A ZoneTemporal stores the *resolved* local date-time, its offset at that
// instant, and the zone. The offset is not a separate source of truth: it is the
// zone's answer for the represented instant, kept so that printing and field
// access do not need to consult the rules again.
//
// Local times that do not exist (spring-forward gap) or exist twice (fall-back
// overlap) are resolved at construction with Disambiguation; Compatible is the
// java.time/Temporal default. The same rule applies after calendar arithmetic,
// because adding a month can land on a missing or repeated local time.
class ZonedGregoryDateTime
{
  public:
    using date_time_type = LocalDateTime;
    using offset_date_time_type = OffsetGregoryDateTime;
    using date_type = LocalDate;

    ZonedGregoryDateTime() = delete;

    static std::optional<ZonedGregoryDateTime> tryOf(const date_time_type &dateTime, const Zone &zone,
                                                     Disambiguation disambiguation = Disambiguation::Compatible)
    {
        const Timestamp local = toLocalSeconds(dateTime);
        const ZoneLocalResult result = zone.getLocalResult(local);
        Offset chosen;
        switch (result.kind)
        {
        case LocalResultKind::Single:
            chosen = result.first;
            break;
        case LocalResultKind::Gap:
            if (disambiguation == Disambiguation::Reject)
            {
                return std::nullopt;
            }
            // Compatible and Later shift forward through the gap; Earlier maps
            // the wall clock with the post-transition offset, moving backwards.
            chosen = disambiguation == Disambiguation::Earlier ? result.second : result.first;
            break;
        case LocalResultKind::Overlap:
            if (disambiguation == Disambiguation::Reject)
            {
                return std::nullopt;
            }
            chosen = disambiguation == Disambiguation::Later ? result.second : result.first;
            break;
        }
        return tryFromInstant(local - chosen.getSeconds(), dateTime.getTime().getNanosecond(), zone);
    }

    static ZonedGregoryDateTime of(const date_time_type &dateTime, const Zone &zone,
                                   Disambiguation disambiguation = Disambiguation::Compatible)
    {
        const std::optional<ZonedGregoryDateTime> value = tryOf(dateTime, zone, disambiguation);
        if (!value)
        {
            throw std::invalid_argument("temporal: local date-time cannot be resolved in the zone");
        }
        return *value;
    }

    static std::optional<ZonedGregoryDateTime> tryAtUnixSeconds(Timestamp seconds, int nanoseconds, const Zone &zone)
    {
        return tryFromInstant(seconds, nanoseconds, zone);
    }

    static ZonedGregoryDateTime atUnixSeconds(Timestamp seconds, const Zone &zone)
    {
        const std::optional<ZonedGregoryDateTime> value = tryFromInstant(seconds, 0, zone);
        if (!value)
        {
            throw std::out_of_range("temporal: instant outside the supported range");
        }
        return *value;
    }

    const Zone &getZone() const noexcept { return zone; }
    const Offset &getOffset() const noexcept { return offsetDateTime.getOffset(); }
    const date_time_type &getDateTime() const noexcept { return offsetDateTime.getDateTime(); }
    const date_type &getDate() const noexcept { return offsetDateTime.getDate(); }
    const LocalTime &getTime() const noexcept { return offsetDateTime.getTime(); }
    const offset_date_time_type &getOffsetDateTime() const noexcept { return offsetDateTime; }

    gregory::Year getYear() const noexcept { return getDateTime().getYear(); }
    gregory::Month getMonth() const noexcept { return getDateTime().getMonth(); }
    int getDay() const noexcept { return getDateTime().getDay(); }
    gregory::Weekday getWeekday() const noexcept { return getDateTime().getWeekday(); }
    int getHour() const noexcept { return getDateTime().getHour(); }
    int getMinute() const noexcept { return getDateTime().getMinute(); }
    int getSecond() const noexcept { return getDateTime().getSecond(); }
    int getNanosecond() const noexcept { return getDateTime().getNanosecond(); }

    Timestamp get(Field field) const { return getDateTime().get(field); }
    bool isSupported(Field field) const noexcept { return getDateTime().isSupported(field); }

    Timestamp toUnixSeconds() const { return offsetDateTime.toUnixSeconds(); }

    Instant toInstant() const { return Instant::fromUnixSeconds(toUnixSeconds(), getNanosecond()); }

    static ZonedGregoryDateTime fromInstant(const Instant &instant, const Zone &zone)
    {
        const std::optional<ZonedGregoryDateTime> value =
            tryFromInstant(instant.toUnixSeconds(), instant.getNanosecondsOfDay() % NanosPerSecond, zone);
        if (!value)
        {
            throw std::out_of_range("temporal: instant outside the supported range");
        }
        return *value;
    }

    Duration durationUntil(const ZonedGregoryDateTime &other) const
    {
        return Duration(other.toUnixSeconds() - toUnixSeconds(), other.getNanosecond() - getNanosecond());
    }

    Duration operator-(const ZonedGregoryDateTime &other) const { return other.durationUntil(*this); }

    ZonedGregoryDateTime plus(const Duration &duration) const
    {
        const Duration total = Duration(toUnixSeconds(), getNanosecond()).plus(duration);
        const std::optional<ZonedGregoryDateTime> value =
            tryFromInstant(total.getSeconds(), total.getNanoseconds(), zone);
        if (!value)
        {
            throw std::out_of_range("temporal: zone date-time outside the supported range");
        }
        return *value;
    }

    ZonedGregoryDateTime minus(const Duration &duration) const { return plus(duration.negated()); }

    ZonedGregoryDateTime plus(const Period &period, Overflow overflow = Overflow::Constrain,
                              Disambiguation disambiguation = Disambiguation::Compatible) const
    {
        return of(getDateTime().plus(period, overflow), zone, disambiguation);
    }

    ZonedGregoryDateTime minus(const Period &period, Overflow overflow = Overflow::Constrain,
                               Disambiguation disambiguation = Disambiguation::Compatible) const
    {
        return of(getDateTime().minus(period, overflow), zone, disambiguation);
    }

    ZonedGregoryDateTime plus(Timestamp amount, Unit unit,
                              Disambiguation disambiguation = Disambiguation::Compatible) const
    {
        // LocalTime-based units are exact: add them to the instant, then re-read the
        // zone rules. Date-based units move the local calendar date and resolve
        // the result, which is why a DST day is not always 24 hours.
        if (isTimeBased(unit))
        {
            return plus(Duration::of(amount, unit));
        }
        return of(getDateTime().plus(amount, unit), zone, disambiguation);
    }

    ZonedGregoryDateTime minus(Timestamp amount, Unit unit,
                               Disambiguation disambiguation = Disambiguation::Compatible) const
    {
        if (isTimeBased(unit))
        {
            if (amount == std::numeric_limits<Timestamp>::min())
            {
                throw std::overflow_error("temporal: zone date-time offset overflow");
            }
            return plus(Duration::of(-amount, unit));
        }
        return of(getDateTime().minus(amount, unit), zone, disambiguation);
    }

    // Keep the represented instant and change the zone: 08:00+08:00[Asia/Shanghai]
    // becomes 00:00Z[UTC]. The local fields change, the instant does not.
    ZonedGregoryDateTime withZoneSameInstant(const Zone &zone) const
    {
        const std::optional<ZonedGregoryDateTime> value = tryFromInstant(toUnixSeconds(), getNanosecond(), zone);
        if (!value)
        {
            throw std::out_of_range("temporal: zone date-time outside the supported range");
        }
        return *value;
    }

    // Keep the wall-clock fields and reinterpret them in another zone. This can
    // hit a gap or an overlap, so the disambiguation policy is explicit.
    ZonedGregoryDateTime withZoneSameLocal(const Zone &zone,
                                           Disambiguation disambiguation = Disambiguation::Compatible) const
    {
        return of(getDateTime(), zone, disambiguation);
    }

    // RFC 9557 style: 2024-03-31T03:30:00+02:00[Test/Spring].
    std::string to_string() const { return offsetDateTime.to_string() + "[" + zone.getIdentifier() + "]"; }

    // Ordering is by represented instant, like OffsetDateTime.
    bool operator==(const ZonedGregoryDateTime &other) const noexcept
    {
        return toUnixSeconds() == other.toUnixSeconds() && getNanosecond() == other.getNanosecond();
    }

    bool operator!=(const ZonedGregoryDateTime &other) const noexcept { return !(*this == other); }

    bool operator<(const ZonedGregoryDateTime &other) const noexcept
    {
        const Timestamp seconds = toUnixSeconds();
        const Timestamp otherSeconds = other.toUnixSeconds();
        if (seconds != otherSeconds)
        {
            return seconds < otherSeconds;
        }
        return getNanosecond() < other.getNanosecond();
    }

    bool operator<=(const ZonedGregoryDateTime &other) const noexcept { return *this < other || *this == other; }

    bool operator>(const ZonedGregoryDateTime &other) const noexcept { return other < *this; }
    bool operator>=(const ZonedGregoryDateTime &other) const noexcept { return other <= *this; }

  private:
    ZonedGregoryDateTime(const offset_date_time_type &offsetDateTime, const Zone &zone) noexcept
        : offsetDateTime(offsetDateTime), zone(zone)
    {
    }

    static std::optional<ZonedGregoryDateTime> tryFromInstant(Timestamp seconds, int nanoseconds, const Zone &zone)
    {
        if (nanoseconds < 0 || nanoseconds >= 1000000000)
        {
            return std::nullopt;
        }
        const Offset offset = zone.getOffsetAt(seconds);
        if (offset.getSeconds() > 0 && seconds > std::numeric_limits<Timestamp>::max() - offset.getSeconds())
        {
            return std::nullopt;
        }
        if (offset.getSeconds() < 0 && seconds < std::numeric_limits<Timestamp>::min() - offset.getSeconds())
        {
            return std::nullopt;
        }

        const Timestamp local = seconds + offset.getSeconds();
        const Timestamp unixDay = floorDiv(local, 86400);
        const Timestamp secondOfDay = local - unixDay * 86400;
        const Timestamp day = unixDay + 719163;
        if (day < std::numeric_limits<std::int32_t>::min() || day > std::numeric_limits<std::int32_t>::max())
        {
            return std::nullopt;
        }

        const std::optional<date_type> date = date_type::tryFromDayNumber(DayNumber(static_cast<std::int32_t>(day)));
        if (!date)
        {
            return std::nullopt;
        }
        const LocalTime LocalTime = LocalTime::ofNanoseconds(secondOfDay * NanosPerSecond + nanoseconds);
        return ZonedGregoryDateTime(offset_date_time_type(date_time_type(*date, LocalTime), offset), zone);
    }

    static Timestamp toLocalSeconds(const date_time_type &dateTime) noexcept
    {
        return offset_date_time_type(dateTime, Offset()).toUnixSeconds();
    }

    static Timestamp floorDiv(Timestamp value, Timestamp divisor) noexcept
    {
        Timestamp quotient = value / divisor;
        if (value % divisor != 0 && (value < 0) != (divisor < 0))
        {
            --quotient;
        }
        return quotient;
    }

    offset_date_time_type offsetDateTime;
    Zone zone;
};

inline std::ostream &operator<<(std::ostream &stream, const ZonedGregoryDateTime &value)
{
    return stream << value.to_string();
}

} // namespace temporal

namespace temporal::gregory
{
using ZoneTemporal = ZonedGregoryDateTime;
}

namespace std
{
template <> struct hash<temporal::ZonedGregoryDateTime>
{
    size_t operator()(const temporal::ZonedGregoryDateTime &value) const noexcept
    {
        const size_t secondHash = hash<temporal::Timestamp>{}(value.toUnixSeconds());
        const size_t nanosecondHash = hash<int>{}(value.getNanosecond());
        return secondHash ^ (nanosecondHash + 0x9e3779b97f4a7c15ULL + (secondHash << 6) + (secondHash >> 2));
    }
};
} // namespace std
