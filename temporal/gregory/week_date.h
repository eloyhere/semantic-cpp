#pragma once

#include "temporal/types.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>

#include "gregory.h"

namespace temporal::gregory
{

// An ISO 8601 week date: week-based year, week number and weekday. It is a
// separate type because the week-based year is not the calendar year
// (2021-01-01 belongs to 2020-W53), mixing the two silently is one of the
// classic date bugs, and C++20's <chrono> has no ISO week at all. The week rule
// is fixed to ISO 8601 here (Monday first, week 1 contains January 4th);
// locale-specific week rules belong to a formatting layer, not to a date value.
class WeekDate
{
  public:
    WeekDate() = delete;

    static constexpr int getWeeksInYear(Year weekYear) noexcept
    {
        const DayNumber januaryFirst = Calendar::toDayNumber(weekYear, Month::January, 1);
        const int index = isoNumber(Calendar::getWeekday(januaryFirst)) - 1;
        const bool fiftyThree = index == 3 || (index == 2 && Calendar::isLeapYear(weekYear));
        return fiftyThree ? 53 : 52;
    }

    static constexpr std::optional<WeekDate> tryOf(Year weekYear, int week, Weekday weekday) noexcept
    {
        if (!weekYear.isOk() || week < 1 || week > getWeeksInYear(weekYear))
        {
            return std::nullopt;
        }
        return WeekDate(weekYear, week, weekday);
    }

    static constexpr WeekDate of(Year weekYear, int week, Weekday weekday)
    {
        const std::optional<WeekDate> value = tryOf(weekYear, week, weekday);
        if (!value)
        {
            throw std::invalid_argument("temporal: invalid ISO week date");
        }
        return *value;
    }

    static constexpr std::optional<WeekDate> tryFrom(const Date &date) noexcept
    {
        const Timestamp serial = date.getDayNumber().getValue();
        const Timestamp thursday = serial + (3 - (isoNumber(date.getWeekday()) - 1));
        if (thursday < std::numeric_limits<std::int32_t>::min() || thursday > std::numeric_limits<std::int32_t>::max())
        {
            return std::nullopt;
        }

        const Year weekYear = Calendar::fromDayNumber(DayNumber(static_cast<std::int32_t>(thursday))).y;
        if (!weekYear.isOk())
        {
            return std::nullopt;
        }

        const DayNumber januaryFourth = Calendar::toDayNumber(weekYear, Month::January, 4);
        const Timestamp firstMonday =
            static_cast<Timestamp>(januaryFourth.getValue()) - (isoNumber(Calendar::getWeekday(januaryFourth)) - 1);
        const int week = static_cast<int>((serial - firstMonday) / 7 + 1);
        return WeekDate(weekYear, week, date.getWeekday());
    }

    static constexpr WeekDate from(const Date &date)
    {
        const std::optional<WeekDate> value = tryFrom(date);
        if (!value)
        {
            throw std::out_of_range("temporal: ISO week date outside the supported range");
        }
        return *value;
    }

    constexpr Year getWeekYear() const noexcept { return weekYear; }
    constexpr int getWeek() const noexcept { return week; }
    constexpr Weekday getWeekday() const noexcept { return weekday; }

    constexpr bool operator==(const WeekDate &other) const noexcept
    {
        return weekYear == other.weekYear && week == other.week && weekday == other.weekday;
    }

    constexpr bool operator!=(const WeekDate &other) const noexcept { return !(*this == other); }

    constexpr std::optional<Date> tryToDate() const noexcept
    {
        const DayNumber januaryFourth = Calendar::toDayNumber(weekYear, Month::January, 4);
        const Timestamp firstMonday =
            static_cast<Timestamp>(januaryFourth.getValue()) - (isoNumber(Calendar::getWeekday(januaryFourth)) - 1);
        const Timestamp serial = firstMonday + static_cast<Timestamp>(week - 1) * 7 + (isoNumber(weekday) - 1);
        if (serial < std::numeric_limits<std::int32_t>::min() || serial > std::numeric_limits<std::int32_t>::max())
        {
            return std::nullopt;
        }
        return Date::tryFromDayNumber(DayNumber(static_cast<std::int32_t>(serial)));
    }

    constexpr Date toDate() const
    {
        const std::optional<Date> value = tryToDate();
        if (!value)
        {
            throw std::out_of_range("temporal: date outside the supported range");
        }
        return *value;
    }

    // 2024-W01-1, with the same signed four-digit week-year spelling as Date.
    std::string to_string() const
    {
        std::string text;
        const std::int32_t yearValue = weekYear.getValue();
        if (yearValue < 0)
        {
            text.push_back('-');
        }
        const Timestamp magnitude =
            yearValue < 0 ? -static_cast<Timestamp>(yearValue) : static_cast<Timestamp>(yearValue);
        const std::string yearText = std::to_string(magnitude);
        text.append(static_cast<std::size_t>(4) - yearText.size(), '0');
        text += yearText;
        text += "-W";
        detail::appendTwoDigits(text, week);
        text.push_back('-');
        text.push_back(static_cast<char>('0' + isoNumber(weekday)));
        return text;
    }

  private:
    constexpr WeekDate(Year weekYear, int week, Weekday weekday) noexcept
        : weekYear(weekYear), week(week), weekday(weekday)
    {
    }

    Year weekYear;
    int week;
    Weekday weekday;
};

} // namespace temporal::gregory
