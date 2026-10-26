#pragma once

#include "temporal/types.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "../calendar.h"
#include "../overflow.h"
#include "../period.h"
#include "era.h"
#include "month.h"
#include "weekday.h"
#include "year.h"

namespace temporal::gregory
{

namespace detail
{

inline constexpr int monthLengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

constexpr bool isDigit(char value) noexcept
{
    return value >= '0' && value <= '9';
}

// Parses a fixed number of ASCII digits; -1 means that the range is not numeric.
constexpr int parseDigits(std::string_view text, std::size_t position, std::size_t count) noexcept
{
    int result = 0;
    for (std::size_t index = 0; index < count; ++index)
    {
        const char character = text[position + index];
        if (!isDigit(character))
        {
            return -1;
        }
        result = result * 10 + (character - '0');
    }
    return result;
}

// Howard Hinnant's era-based civil calendar algorithm: days since 1970-01-01 of
// the proleptic Gregorian calendar, valid for negative years because the era is
// computed with a floor-like adjustment. The implementation is int64 throughout
// and therefore cannot overflow for the Gregorian year range.
constexpr Timestamp daysFromCivil(Timestamp year, unsigned int month, unsigned int day) noexcept
{
    year -= month <= 2 ? 1 : 0;
    const Timestamp era = (year >= 0 ? year : year - 399) / 400;
    const unsigned int yearOfEra = static_cast<unsigned int>(year - era * 400);
    const unsigned int monthInMarchYear = (month + 9U) % 12U;
    const unsigned int dayOfYear = (153U * monthInMarchYear + 2U) / 5U + day - 1U;
    const unsigned int dayOfEra = yearOfEra * 365U + yearOfEra / 4U - yearOfEra / 100U + dayOfYear;
    return era * 146097 + static_cast<Timestamp>(dayOfEra) - 719468;
}

constexpr DateFields<Year, Month> civilFromDays(Timestamp z) noexcept
{
    z += 719468;
    const Timestamp era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned int dayOfEra = static_cast<unsigned int>(z - era * 146097);
    const unsigned int yearOfEra = (dayOfEra - dayOfEra / 1460U + dayOfEra / 36524U - dayOfEra / 146096U) / 365U;
    const Timestamp year = static_cast<Timestamp>(yearOfEra) + era * 400;
    const unsigned int dayOfYear = dayOfEra - (365U * yearOfEra + yearOfEra / 4U - yearOfEra / 100U);
    const unsigned int monthInMarchYear = (5U * dayOfYear + 2U) / 153U;
    const unsigned int day = dayOfYear - (153U * monthInMarchYear + 2U) / 5U + 1U;
    const unsigned int month = monthInMarchYear < 10U ? monthInMarchYear + 3U : monthInMarchYear - 9U;
    return DateFields<Year, Month>{Year(static_cast<std::int32_t>(year + (month <= 2U ? 1 : 0))),
                                   static_cast<Month>(month), static_cast<int>(day)};
}

inline void appendTwoDigits(std::string &text, int value)
{
    text.push_back(static_cast<char>('0' + value / 10));
    text.push_back(static_cast<char>('0' + value % 10));
}

} // namespace detail

// The Gregorian calendar policy. It is a stateless compile-time description of
// the proleptic Gregorian calendar (the Julian/Gregorian cutover is not
// modelled; every date before 1582-10-15 is interpreted proleptically), with
// ISO 8601 field vocabulary: the Gregorian era pair, the twelve civil months
// and Monday-based weekdays. Month and year arithmetic clamps at the end of the
// month, which is the behaviour of java.time, Temporal's default
// overflow: 'constrain' and Noda Time; C++20's unbounded std::chrono arithmetic
// is the counter-example that produces invalid 2019-02-31 values.
struct Calendar
{
    using Year = temporal::gregory::Year;
    using Month = temporal::gregory::Month;
    using Weekday = temporal::gregory::Weekday;
    using fields = temporal::DateFields<Year, Month>;

    static constexpr const char *getName() noexcept { return "gregory"; }

    static constexpr bool isLeapYear(Year year) noexcept
    {
        const Timestamp value = year.getValue();
        return (value % 4 == 0 && value % 100 != 0) || value % 400 == 0;
    }

    static constexpr int getMonthsInYear(Year) noexcept { return 12; }

    static constexpr int getDaysInMonth(Year year, Month month) noexcept
    {
        return detail::monthLengths[number(month) - 1] + (month == Month::February && isLeapYear(year) ? 1 : 0);
    }

    static constexpr int getDaysInYear(Year year) noexcept { return isLeapYear(year) ? 366 : 365; }

    static constexpr bool isValidDate(Year year, Month month, int day) noexcept
    {
        return year.isOk() && day >= 1 && day <= getDaysInMonth(year, month);
    }

    static constexpr DayNumber toDayNumber(Year year, Month month, int day) noexcept
    {
        return DayNumber(
            static_cast<std::int32_t>(detail::daysFromCivil(year.getValue(), static_cast<unsigned int>(number(month)),
                                                            static_cast<unsigned int>(day)) +
                                      719163));
    }

    static constexpr fields fromDayNumber(DayNumber day) noexcept
    {
        return detail::civilFromDays(static_cast<Timestamp>(day.getValue()) - 719163);
    }

    static constexpr Weekday getWeekday(DayNumber day) noexcept
    {
        // Rata Die day 1 (0001-01-01) is a Monday.
        Timestamp index = (static_cast<Timestamp>(day.getValue()) - 1) % 7;
        if (index < 0)
        {
            index += 7;
        }
        return static_cast<Weekday>(index + 1);
    }

    static constexpr int getDayOfYear(DayNumber day) noexcept
    {
        const fields value = fromDayNumber(day);
        int result = value.d;
        for (int month = 1; month < number(value.m); ++month)
        {
            result += getDaysInMonth(value.y, static_cast<Month>(month));
        }
        return result;
    }

    // Field accessors for the Gregorian policy. The values are plain integers (an
    // ordinal month, an ISO weekday, a week-year) so that generic code can
    // consume them without knowing the calendar's types; strongly typed
    // accessors live on BasicDate.
    static constexpr Timestamp getField(DayNumber day, Field field)
    {
        const fields value = fromDayNumber(day);
        switch (field)
        {
        case Field::Year:
            return value.y.getValue();
        case Field::YearOfEra:
            return value.y.getYearInEra();
        case Field::Month:
            return number(value.m);
        case Field::DayOfMonth:
            return value.d;
        case Field::DayOfYear:
            return getDayOfYear(day);
        case Field::DayOfWeek:
            return isoNumber(getWeekday(day));
        case Field::WeekOfWeekYear:
        case Field::WeekYear:
        {
            const Timestamp serial = day.getValue();
            const Timestamp thursday = serial + (3 - (isoNumber(getWeekday(day)) - 1));
            const Year weekYear = fromDayNumber(DayNumber(static_cast<std::int32_t>(thursday))).y;
            if (field == Field::WeekYear)
            {
                return weekYear.getValue();
            }
            const DayNumber januaryFourth = toDayNumber(weekYear, Month::January, 4);
            const Timestamp firstMonday =
                static_cast<Timestamp>(januaryFourth.getValue()) - (isoNumber(getWeekday(januaryFourth)) - 1);
            return (serial - firstMonday) / 7 + 1;
        }
        case Field::DaysInMonth:
            return getDaysInMonth(value.y, value.m);
        case Field::DaysInYear:
            return getDaysInYear(value.y);
        case Field::IsLeapMonth:
            return 0;
        }
        throw std::invalid_argument("temporal: unsupported calendar field");
    }

    static constexpr bool isSupportedField(Field field) noexcept { return isDateField(field); }

    static constexpr DayNumber addYears(DayNumber day, Timestamp years, Overflow overflow = Overflow::Constrain)
    {
        if (years < -20000 || years > 20000)
        {
            throw std::overflow_error("temporal: year offset outside the supported range");
        }
        const fields value = fromDayNumber(day);
        const Timestamp year = static_cast<Timestamp>(value.y.getValue()) + years;
        if (year < Year::minimum() || year > Year::maximum())
        {
            throw std::out_of_range("temporal: date outside the supported range");
        }
        const Year targetYear(static_cast<std::int32_t>(year));
        const int daysInTargetMonth = getDaysInMonth(targetYear, value.m);
        if (overflow == Overflow::Reject && value.d > daysInTargetMonth)
        {
            throw std::out_of_range("temporal: day does not exist in the target month");
        }
        const int targetDay = std::min(value.d, daysInTargetMonth);
        return toDayNumber(targetYear, value.m, targetDay);
    }

    static constexpr DayNumber addMonths(DayNumber day, Timestamp months, Overflow overflow = Overflow::Constrain)
    {
        if (months < -240012 || months > 240012)
        {
            throw std::overflow_error("temporal: month offset outside the supported range");
        }
        const fields value = fromDayNumber(day);
        const Timestamp index = static_cast<Timestamp>(value.y.getValue()) * 12 + (number(value.m) - 1) + months;
        Timestamp year = index / 12;
        Timestamp month = index % 12;
        if (month < 0)
        {
            month += 12;
            --year;
        }
        if (year < Year::minimum() || year > Year::maximum())
        {
            throw std::out_of_range("temporal: date outside the supported range");
        }
        const Year targetYear(static_cast<std::int32_t>(year));
        const Month targetMonth(static_cast<Month>(month + 1));
        const int daysInTargetMonth = getDaysInMonth(targetYear, targetMonth);
        if (overflow == Overflow::Reject && value.d > daysInTargetMonth)
        {
            throw std::out_of_range("temporal: day does not exist in the target month");
        }
        const int targetDay = std::min(value.d, daysInTargetMonth);
        return toDayNumber(targetYear, targetMonth, targetDay);
    }

    // The calendar-aware distance, normalised as years, months and days in the
    // same spirit as java.time.Period.between. The result is conventional: for
    // pairs whose month arithmetic clamps, start plus the result need not be
    // end (2024-02-29 to 2028-02-28), so use day numbers for exact distances.
    static constexpr Period until(DayNumber start, DayNumber end)
    {
        if (end < start)
        {
            return until(end, start).negated();
        }

        const fields first = fromDayNumber(start);
        const fields last = fromDayNumber(end);
        const Timestamp firstMonth = static_cast<Timestamp>(first.y.getValue()) * 12 + (number(first.m) - 1);
        const Timestamp lastMonth = static_cast<Timestamp>(last.y.getValue()) * 12 + (number(last.m) - 1);
        Timestamp totalMonths = lastMonth - firstMonth;
        Timestamp days = static_cast<Timestamp>(last.d) - first.d;
        if (totalMonths > 0 && days < 0)
        {
            --totalMonths;
            const DayNumber anchor = addMonths(start, totalMonths);
            days = static_cast<Timestamp>(end.getValue()) - anchor.getValue();
        }
        return Period(totalMonths / 12, totalMonths % 12, days);
    }

    // Whole months or years between two dates, using the same month-end rule as
    // java.time's ChronoUnit.MONTHS: a month only counts once the day-of-month
    // has been reached, so 2023-01-31 to 2023-02-28 is zero months even though
    // plus one month clamps to the same date. The packed month*32 + day form is
    // the comparison java.time uses.
    static constexpr Timestamp until(DayNumber start, DayNumber end, Unit unit)
    {
        if (unit != Unit::Months && unit != Unit::Years)
        {
            throw std::invalid_argument("temporal: unit is not calendar-based");
        }
        const fields first = fromDayNumber(start);
        const fields last = fromDayNumber(end);
        const Timestamp firstMonth = static_cast<Timestamp>(first.y.getValue()) * 12 + (number(first.m) - 1);
        const Timestamp lastMonth = static_cast<Timestamp>(last.y.getValue()) * 12 + (number(last.m) - 1);
        const Timestamp months = ((lastMonth * 32 + last.d) - (firstMonth * 32 + first.d)) / 32;
        return unit == Unit::Months ? months : months / 12;
    }

    static constexpr DayNumber getMinimumDayNumber() noexcept
    {
        return DayNumber(static_cast<std::int32_t>(detail::daysFromCivil(Year::minimum(), 1U, 1U) + 719163));
    }

    static constexpr DayNumber getMaximumDayNumber() noexcept
    {
        return DayNumber(static_cast<std::int32_t>(detail::daysFromCivil(Year::maximum(), 12U, 31U) + 719163));
    }

    static std::string format(DayNumber day)
    {
        const fields value = fromDayNumber(day);
        const std::int32_t yearValue = value.y.getValue();
        std::string text;
        if (yearValue < 0)
        {
            text.push_back('-');
        }
        const Timestamp magnitude =
            yearValue < 0 ? -static_cast<Timestamp>(yearValue) : static_cast<Timestamp>(yearValue);
        const std::string yearText = std::to_string(magnitude);
        text.append(static_cast<std::size_t>(4) - yearText.size(), '0');
        text += yearText;
        text.push_back('-');
        detail::appendTwoDigits(text, number(value.m));
        text.push_back('-');
        detail::appendTwoDigits(text, value.d);
        return text;
    }

    // Strict ISO 8601 calendar date: [+-]YYYY-MM-DD with a four-digit
    // astronomical year. A negative zero year is rejected, and 2024-02-30 is
    // rejected instead of being rolled forward.
    static constexpr std::optional<DayNumber> tryParse(std::string_view text) noexcept
    {
        std::size_t position = 0;
        bool negative = false;
        if (!text.empty() && (text.front() == '-' || text.front() == '+'))
        {
            negative = text.front() == '-';
            position = 1;
        }
        if (text.size() - position != 10)
        {
            return std::nullopt;
        }
        if (text[position + 4] != '-' || text[position + 7] != '-')
        {
            return std::nullopt;
        }

        const int yearValue = detail::parseDigits(text, position, 4);
        const int monthValue = detail::parseDigits(text, position + 5, 2);
        const int dayValue = detail::parseDigits(text, position + 8, 2);
        if (yearValue < 0 || monthValue < 0 || dayValue < 0)
        {
            return std::nullopt;
        }
        if (negative && yearValue == 0)
        {
            return std::nullopt;
        }

        const Year year(static_cast<std::int32_t>(negative ? -yearValue : yearValue));
        const std::optional<Month> month = monthFromNumber(monthValue);
        if (!month || !isValidDate(year, *month, dayValue))
        {
            return std::nullopt;
        }
        return toDayNumber(year, *month, dayValue);
    }
};

using Date = temporal::BasicDate<Calendar>;

} // namespace temporal::gregory
