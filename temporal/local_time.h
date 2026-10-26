#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "duration.h"
#include "field.h"

namespace temporal
{

template <typename Calendar> class BasicDateTime;

// A wall-clock time of day, without a date and without a time zone: the
// equivalent of java.time.LocalTime, Temporal.PlainTime and Noda Time's
// LocalTime. Storing nanoseconds within the day keeps it exact and lets LocalTime
// share Duration arithmetic with Instant.
//
// A LocalTime is always in [00:00:00, 24:00:00); arithmetic wraps modulo one day,
// which is what LocalTime.plus and PlainTime.add do. It is not an instant: a
// LocalTime alone cannot be compared across calendars or LocalTime zones, so conversions
// to Instant are deliberately absent.
class LocalTime
{
  public:
    constexpr LocalTime() noexcept : nanoseconds(0) {}

    static constexpr LocalTime midnight() noexcept { return LocalTime(0); }

    static constexpr std::optional<LocalTime> tryOf(int hour, int minute, int second = 0, int nanosecond = 0) noexcept
    {
        if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59 || nanosecond < 0 ||
            nanosecond > 999999999)
        {
            return std::nullopt;
        }
        return LocalTime(static_cast<Timestamp>(hour) * 3600LL * NanosPerSecond +
                    static_cast<Timestamp>(minute) * 60LL * NanosPerSecond +
                    static_cast<Timestamp>(second) * NanosPerSecond + nanosecond);
    }

    static constexpr LocalTime of(int hour, int minute, int second = 0, int nanosecond = 0)
    {
        const std::optional<LocalTime> value = tryOf(hour, minute, second, nanosecond);
        if (!value)
        {
            throw std::invalid_argument("temporal: invalid LocalTime of day");
        }
        return *value;
    }

    // Strict, like LocalTime.ofNanoOfDay: the value must already be inside one
    // day. Arithmetic uses the private constructor instead.
    static constexpr LocalTime ofNanoseconds(Timestamp nanosecondsOfDay)
    {
        if (nanosecondsOfDay < 0 || nanosecondsOfDay >= NanosPerDay)
        {
            throw std::invalid_argument("temporal: LocalTime of day outside [0, 24h)");
        }
        return LocalTime(nanosecondsOfDay);
    }

    constexpr Timestamp getNanoseconds() const noexcept { return nanoseconds; }

    constexpr int getHour() const noexcept { return static_cast<int>(nanoseconds / (3600LL * NanosPerSecond)); }

    constexpr int getMinute() const noexcept
    {
        return static_cast<int>((nanoseconds % (3600LL * NanosPerSecond)) / (60LL * NanosPerSecond));
    }

    constexpr int getSecond() const noexcept
    {
        return static_cast<int>((nanoseconds % (60LL * NanosPerSecond)) / NanosPerSecond);
    }

    constexpr int getNanosecond() const noexcept { return static_cast<int>(nanoseconds % NanosPerSecond); }

    // LocalTime-of-day arithmetic wraps: 23:00 + 2h is 01:00, not an error.
    constexpr LocalTime plus(const Duration &duration) const
    {
        Timestamp seconds = duration.getSeconds() % SecondsPerDay;
        Timestamp total = nanoseconds + seconds * NanosPerSecond + duration.getNanoseconds();
        total %= NanosPerDay;
        if (total < 0)
        {
            total += NanosPerDay;
        }
        return LocalTime(total);
    }

    constexpr LocalTime minus(const Duration &duration) const { return plus(duration.negated()); }

    constexpr LocalTime operator+(const Duration &duration) const { return plus(duration); }
    constexpr LocalTime operator-(const Duration &duration) const { return minus(duration); }

    // Signed LocalTime line distance: 01:00.until(23:00) is +22h,
    // 23:00.until(01:00) is -22h, matching LocalTime.until.
    constexpr Duration until(const LocalTime &other) const
    {
        return Duration::ofNanoseconds(other.nanoseconds - nanoseconds);
    }

    constexpr Timestamp get(Field field) const
    {
        switch (field)
        {
        case Field::HourOfDay:
            return getHour();
        case Field::MinuteOfHour:
            return getMinute();
        case Field::SecondOfMinute:
            return getSecond();
        case Field::NanosecondOfSecond:
            return getNanosecond();
        default:
            throw std::invalid_argument("temporal: field is not a LocalTime-of-day field");
        }
    }

    constexpr bool isSupported(Field field) const noexcept
    {
        return field == Field::HourOfDay || field == Field::MinuteOfHour || field == Field::SecondOfMinute ||
               field == Field::NanosecondOfSecond;
    }

    // HH:MM:SS with an optional fractional second (1..9 digits on input, trailing
    // zeros trimmed on output). Reduced precision HH:MM is accepted on input.
    static constexpr std::optional<LocalTime> tryParse(std::string_view text) noexcept
    {
        if (text.size() != 5 && text.size() != 8 && (text.size() < 10 || text.size() > 18))
        {
            return std::nullopt;
        }
        if (text[2] != ':')
        {
            return std::nullopt;
        }

        const int hour = parseDigits(text, 0, 2);
        const int minute = parseDigits(text, 3, 2);
        if (hour < 0 || minute < 0)
        {
            return std::nullopt;
        }

        int second = 0;
        Timestamp nanosecond = 0;
        if (text.size() >= 8)
        {
            if (text[5] != ':')
            {
                return std::nullopt;
            }
            second = parseDigits(text, 6, 2);
            if (second < 0)
            {
                return std::nullopt;
            }
        }
        if (text.size() > 8)
        {
            if (text[8] != '.')
            {
                return std::nullopt;
            }
            const std::size_t digits = text.size() - 9;
            for (std::size_t index = 0; index < digits; ++index)
            {
                const int digit = digitValue(text[9 + index]);
                if (digit < 0)
                {
                    return std::nullopt;
                }
                nanosecond = nanosecond * 10 + digit;
            }
            for (std::size_t index = digits; index < 9; ++index)
            {
                nanosecond *= 10;
            }
        }
        return tryOf(hour, minute, second, static_cast<int>(nanosecond));
    }

    static constexpr LocalTime parse(std::string_view text)
    {
        const std::optional<LocalTime> value = tryParse(text);
        if (!value)
        {
            throw std::invalid_argument("temporal: invalid LocalTime text");
        }
        return *value;
    }

    std::string to_string() const
    {
        std::string text;
        appendTwoDigits(text, getHour());
        text.push_back(':');
        appendTwoDigits(text, getMinute());
        text.push_back(':');
        appendTwoDigits(text, getSecond());
        if (getNanosecond() != 0)
        {
            int value = getNanosecond();
            char buffer[9];
            for (int index = 8; index >= 0; --index)
            {
                buffer[index] = static_cast<char>('0' + value % 10);
                value /= 10;
            }
            std::size_t end = 9;
            while (end > 0 && buffer[end - 1] == '0')
            {
                --end;
            }
            text.push_back('.');
            text.append(buffer, end);
        }
        return text;
    }

    constexpr bool operator==(const LocalTime &other) const noexcept { return nanoseconds == other.nanoseconds; }
    constexpr bool operator!=(const LocalTime &other) const noexcept { return nanoseconds != other.nanoseconds; }
    constexpr bool operator<(const LocalTime &other) const noexcept { return nanoseconds < other.nanoseconds; }
    constexpr bool operator<=(const LocalTime &other) const noexcept { return nanoseconds <= other.nanoseconds; }
    constexpr bool operator>(const LocalTime &other) const noexcept { return nanoseconds > other.nanoseconds; }
    constexpr bool operator>=(const LocalTime &other) const noexcept { return nanoseconds >= other.nanoseconds; }

  private:
    explicit constexpr LocalTime(Timestamp nanosecondsOfDay) noexcept : nanoseconds(nanosecondsOfDay) {}

    static constexpr int digitValue(char value) noexcept { return value >= '0' && value <= '9' ? value - '0' : -1; }

    static constexpr int parseDigits(std::string_view text, std::size_t position, std::size_t count) noexcept
    {
        int result = 0;
        for (std::size_t index = 0; index < count; ++index)
        {
            const int digit = digitValue(text[position + index]);
            if (digit < 0)
            {
                return -1;
            }
            result = result * 10 + digit;
        }
        return result;
    }

    static void appendTwoDigits(std::string &text, int value)
    {
        text.push_back(static_cast<char>('0' + value / 10));
        text.push_back(static_cast<char>('0' + value % 10));
    }

    template <typename Calendar> friend class BasicDateTime;

    Timestamp nanoseconds;
};

} // namespace temporal

namespace std
{
template <> struct hash<temporal::LocalTime>
{
    size_t operator()(const temporal::LocalTime &value) const noexcept
    {
        return hash<temporal::Timestamp>{}(value.getNanoseconds());
    }
};
} // namespace std
