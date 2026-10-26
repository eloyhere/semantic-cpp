#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "duration.h"
#include "local_date.h"
#include "local_time.h"
#include "overflow.h"
#include "period.h"

namespace temporal
{

// A Gregorian local date plus a wall-clock time, with no time zone and no
// offset: the equivalent of java.time.LocalDateTime, Temporal.PlainDateTime and
// Noda Time's LocalDateTime. It is deliberately a different type from Instant:
// the same wall-clock reading can denote two different instants across a DST
// boundary, so the two must never be interchangeable.
class LocalDateTime
{
  public:
    using date_type = LocalDate;
    using time_type = LocalTime;

    LocalDateTime() = delete;

    constexpr LocalDateTime(const date_type &dateValue, const time_type &timeValue) noexcept
        : date(dateValue), time(timeValue)
    {
    }

    static constexpr std::optional<LocalDateTime> tryOf(gregory::Year year, gregory::Month month, int day, int hour,
                                                        int minute, int second = 0, int nanosecond = 0) noexcept
    {
        const std::optional<date_type> dateValue = date_type::tryOf(year, month, day);
        const std::optional<time_type> timeValue = time_type::tryOf(hour, minute, second, nanosecond);
        if (!dateValue || !timeValue)
        {
            return std::nullopt;
        }
        return LocalDateTime(*dateValue, *timeValue);
    }

    static constexpr LocalDateTime of(gregory::Year year, gregory::Month month, int day, int hour, int minute,
                                      int second = 0, int nanosecond = 0)
    {
        const std::optional<LocalDateTime> value = tryOf(year, month, day, hour, minute, second, nanosecond);
        if (!value)
        {
            throw std::invalid_argument("temporal: invalid date-time");
        }
        return *value;
    }

    static constexpr std::optional<LocalDateTime> tryParse(std::string_view text) noexcept
    {
        const std::size_t dateLength = (!text.empty() && (text.front() == '-' || text.front() == '+')) ? 11 : 10;
        if (text.size() <= dateLength || text[dateLength] != 'T')
        {
            return std::nullopt;
        }
        const std::optional<date_type> dateValue = date_type::tryParse(text.substr(0, dateLength));
        const std::optional<time_type> timeValue = time_type::tryParse(text.substr(dateLength + 1));
        if (!dateValue || !timeValue)
        {
            return std::nullopt;
        }
        return LocalDateTime(*dateValue, *timeValue);
    }

    static constexpr LocalDateTime parse(std::string_view text)
    {
        const std::optional<LocalDateTime> value = tryParse(text);
        if (!value)
        {
            throw std::invalid_argument("temporal: invalid date-time text");
        }
        return *value;
    }

    constexpr const date_type &getDate() const noexcept { return date; }
    constexpr const time_type &getTime() const noexcept { return time; }

    constexpr DayNumber getDayNumber() const noexcept { return date.getDayNumber(); }
    constexpr gregory::Year getYear() const noexcept { return date.getYear(); }
    constexpr gregory::Month getMonth() const noexcept { return date.getMonth(); }
    constexpr int getDay() const noexcept { return date.getDay(); }
    constexpr gregory::Weekday getWeekday() const noexcept { return date.getWeekday(); }
    constexpr int getDayOfYear() const noexcept { return date.getDayOfYear(); }
    constexpr int getHour() const noexcept { return time.getHour(); }
    constexpr int getMinute() const noexcept { return time.getMinute(); }
    constexpr int getSecond() const noexcept { return time.getSecond(); }
    constexpr int getNanosecond() const noexcept { return time.getNanosecond(); }

    constexpr Timestamp get(Field field) const
    {
        if (isDayField(field) || isDateField(field))
        {
            return date.get(field);
        }
        return time.get(field);
    }

    constexpr bool isSupported(Field field) const noexcept
    {
        return (isDayField(field) || isDateField(field)) ? date.isSupported(field) : time.isSupported(field);
    }

    // A Duration moves along the exact time line: 23:30 + 2h is 01:30 the next
    // day. A Period moves the calendar date and keeps the time unchanged.
    constexpr LocalDateTime plus(const Duration &duration) const
    {
        Timestamp dayShift = duration.getSeconds() / SecondsPerDay;
        Timestamp seconds = duration.getSeconds() % SecondsPerDay;
        if (seconds < 0)
        {
            seconds += SecondsPerDay;
            --dayShift;
        }
        Timestamp nanoseconds = time.getNanoseconds() + seconds * NanosPerSecond + duration.getNanoseconds();
        const Timestamp extraDays = nanoseconds / NanosPerDay;
        nanoseconds %= NanosPerDay;
        return LocalDateTime(date + Days(dayShift + extraDays), time_type::ofNanoseconds(nanoseconds));
    }

    constexpr LocalDateTime minus(const Duration &duration) const { return plus(duration.negated()); }

    constexpr LocalDateTime plus(const Period &period, Overflow overflow = Overflow::Constrain) const
    {
        return LocalDateTime(date.plus(period, overflow), time);
    }

    constexpr LocalDateTime minus(const Period &period, Overflow overflow = Overflow::Constrain) const
    {
        return plus(period.negated(), overflow);
    }

    // Unit-based convenience: months and years move the calendar date and keep
    // the wall-clock time; every exact unit moves along the time line.
    constexpr LocalDateTime plus(Timestamp amount, Unit unit) const
    {
        if (unit == Unit::Months || unit == Unit::Years)
        {
            return LocalDateTime(date.plus(amount, unit), time);
        }
        return plus(Duration::of(amount, unit));
    }

    constexpr LocalDateTime minus(Timestamp amount, Unit unit) const
    {
        if (amount == std::numeric_limits<Timestamp>::min())
        {
            throw std::overflow_error("temporal: date-time offset overflow");
        }
        return plus(-amount, unit);
    }

    constexpr LocalDateTime operator+(const Duration &duration) const { return plus(duration); }
    constexpr LocalDateTime operator-(const Duration &duration) const { return minus(duration); }
    constexpr LocalDateTime operator+(const Period &period) const { return plus(period); }
    constexpr LocalDateTime operator-(const Period &period) const { return minus(period); }

    // Exact signed distance along the time line. Calendar distances (months,
    // years) are a different question and belong to getDate().until().
    constexpr Duration durationUntil(const LocalDateTime &other) const
    {
        const Timestamp days = (other.date - date).getCount();
        return Duration(days * SecondsPerDay, other.time.getNanoseconds() - time.getNanoseconds());
    }

    // Whole units to other. Date-based units follow the calendar date and ignore
    // the wall-clock time (java.time's LocalDateTime.until does the same); time
    // units use the exact Duration and therefore never depend on DST.
    constexpr Timestamp until(const LocalDateTime &other, Unit unit) const
    {
        if (isCalendarBased(unit) || unit == Unit::Days || unit == Unit::Weeks)
        {
            return date.until(other.date, unit);
        }
        const Timestamp factor = nanosecondsPerUnit(unit);
        if (factor == 0)
        {
            throw std::invalid_argument("temporal: unit cannot be measured on a date-time");
        }
        return durationUntil(other).toNanoseconds() / factor;
    }

    std::string to_string() const { return date.to_string() + "T" + time.to_string(); }

    constexpr bool operator==(const LocalDateTime &other) const noexcept
    {
        return date == other.date && time == other.time;
    }

    constexpr bool operator!=(const LocalDateTime &other) const noexcept { return !(*this == other); }

    constexpr bool operator<(const LocalDateTime &other) const noexcept
    {
        if (date != other.date)
        {
            return date < other.date;
        }
        return time < other.time;
    }

    constexpr bool operator<=(const LocalDateTime &other) const noexcept { return *this < other || *this == other; }

    constexpr bool operator>(const LocalDateTime &other) const noexcept { return other < *this; }
    constexpr bool operator>=(const LocalDateTime &other) const noexcept { return other <= *this; }

  private:
    date_type date;
    time_type time;
};

inline std::ostream &operator<<(std::ostream &stream, const LocalDateTime &value)
{
    return stream << value.to_string();
}

} // namespace temporal

namespace temporal::gregory
{
using DateTime = LocalDateTime;
}

namespace std
{
template <> struct hash<temporal::LocalDateTime>
{
    size_t operator()(const temporal::LocalDateTime &value) const noexcept
    {
        const size_t dateHash = hash<temporal::LocalDate>{}(value.getDate());
        const size_t timeHash = hash<temporal::LocalTime>{}(value.getTime());
        return dateHash ^ (timeHash + 0x9e3779b97f4a7c15ULL + (dateHash << 6) + (dateHash >> 2));
    }
};
} // namespace std
