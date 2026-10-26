#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace temporal
{

constexpr int MaximumOffsetSeconds = 18 * 3600;

// A fixed offset from UTC, distinct from a time zone: an offset is a constant
// number of seconds, while a zone is a set of rules with DST gaps and overlaps.
// Keeping Offset separate is the lesson of Haskell's ZonedTime (offset only,
// cannot answer DST) and of Temporal's ZonedDateTime/offset split.
//
// The range is ISO 8601's +-18:00. A zero offset formats as Z, which is what
// RFC 3339 uses for UTC.
class Offset
{
  public:
    constexpr Offset() noexcept : seconds(0) {}

    static constexpr std::optional<Offset> tryOfSeconds(int seconds) noexcept
    {
        if (seconds < -MaximumOffsetSeconds || seconds > MaximumOffsetSeconds)
        {
            return std::nullopt;
        }
        return Offset(seconds);
    }

    static constexpr Offset ofSeconds(int seconds)
    {
        const std::optional<Offset> value = tryOfSeconds(seconds);
        if (!value)
        {
            throw std::invalid_argument("temporal: offset outside +-18:00");
        }
        return *value;
    }

    static constexpr Offset ofHours(int hours)
    {
        if (hours < -18 || hours > 18)
        {
            throw std::invalid_argument("temporal: offset outside +-18:00");
        }
        return Offset(hours * 3600);
    }

    // The sign of hours applies to minutes too: ofHoursMinutes(-5, 30) is
    // -05:30. Use ofSeconds for negative sub-hour offsets such as -00:30.
    static constexpr Offset ofHoursMinutes(int hours, int minutes)
    {
        if (minutes < 0 || minutes > 59)
        {
            throw std::invalid_argument("temporal: offset minutes outside 0..59");
        }
        if (hours < -18 || hours > 18)
        {
            throw std::invalid_argument("temporal: offset outside +-18:00");
        }
        const int sign = hours < 0 ? -1 : 1;
        const int total = hours * 3600 + sign * minutes * 60;
        if (total < -MaximumOffsetSeconds || total > MaximumOffsetSeconds)
        {
            throw std::invalid_argument("temporal: offset outside +-18:00");
        }
        return Offset(total);
    }

    constexpr int getSeconds() const noexcept { return seconds; }
    constexpr bool isZero() const noexcept { return seconds == 0; }

    // Z, or +HH:MM / -HH:MM.
    std::string to_string() const
    {
        if (seconds == 0)
        {
            return "Z";
        }
        std::string text;
        const int magnitude = seconds < 0 ? -seconds : seconds;
        text.push_back(seconds < 0 ? '-' : '+');
        text.push_back(static_cast<char>('0' + magnitude / 3600 / 10));
        text.push_back(static_cast<char>('0' + magnitude / 3600 % 10));
        text.push_back(':');
        text.push_back(static_cast<char>('0' + magnitude % 3600 / 60 / 10));
        text.push_back(static_cast<char>('0' + magnitude % 3600 / 60 % 10));
        return text;
    }

    static constexpr std::optional<Offset> tryParse(std::string_view text) noexcept
    {
        if (text.size() == 1 && (text[0] == 'Z' || text[0] == 'z'))
        {
            return Offset(0);
        }
        if (text.size() != 6 || (text[0] != '+' && text[0] != '-') || text[3] != ':')
        {
            return std::nullopt;
        }

        const int hours = parseDigits(text, 1, 2);
        const int minutes = parseDigits(text, 4, 2);
        if (hours < 0 || minutes < 0 || hours > 18 || minutes > 59)
        {
            return std::nullopt;
        }
        int total = hours * 3600 + minutes * 60;
        if (total > MaximumOffsetSeconds)
        {
            return std::nullopt;
        }
        if (text[0] == '-')
        {
            total = -total;
        }
        return Offset(total);
    }

    static constexpr Offset parse(std::string_view text)
    {
        const std::optional<Offset> value = tryParse(text);
        if (!value)
        {
            throw std::invalid_argument("temporal: invalid UTC offset");
        }
        return *value;
    }

    constexpr bool operator==(const Offset &other) const noexcept { return seconds == other.seconds; }
    constexpr bool operator!=(const Offset &other) const noexcept { return seconds != other.seconds; }
    constexpr bool operator<(const Offset &other) const noexcept { return seconds < other.seconds; }
    constexpr bool operator<=(const Offset &other) const noexcept { return seconds <= other.seconds; }
    constexpr bool operator>(const Offset &other) const noexcept { return seconds > other.seconds; }
    constexpr bool operator>=(const Offset &other) const noexcept { return seconds >= other.seconds; }

  private:
    explicit constexpr Offset(int seconds) noexcept : seconds(seconds) {}

    static constexpr int parseDigits(std::string_view text, std::size_t position, std::size_t count) noexcept
    {
        int result = 0;
        for (std::size_t index = 0; index < count; ++index)
        {
            const char character = text[position + index];
            if (character < '0' || character > '9')
            {
                return -1;
            }
            result = result * 10 + (character - '0');
        }
        return result;
    }

    int seconds;
};

} // namespace temporal

namespace std
{
template <> struct hash<temporal::Offset>
{
    size_t operator()(const temporal::Offset &value) const noexcept { return hash<int>{}(value.getSeconds()); }
};
} // namespace std
