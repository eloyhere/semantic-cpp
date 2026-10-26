#pragma once

#include "temporal/types.h"

#include <cstdint>

namespace temporal
{

// One field vocabulary for calendar dates and instants, following java.time's
// single ChronoField enum rather than one enum per value type: generic code can
// switch once, and each type reports via isSupported() which fields it can
// answer. Calendar fields are calendar-neutral identifiers interpreted by the
// calendar policy; time fields are properties of a time-of-day. Month is the
// ordinal month within the year (a lunisolar calendar may answer 13 and set
// IsLeapMonth), and WeekOfWeekYear/WeekYear follow ISO 8601.
enum class Field
{
    // Calendar date fields.
    Year,
    YearOfEra,
    Month,
    DayOfMonth,
    DayOfYear,
    DayOfWeek,
    WeekOfWeekYear,
    WeekYear,
    DaysInMonth,
    DaysInYear,
    IsLeapMonth,

    // Day-number fields.
    // Whole days since the natural-calendar epoch -2696-03-11, 1-based (the
    // epoch day is 1).
    ProlepticDay,
    // Whole days since 1970-01-01, 1-based (1970-01-01 is 1).
    EpochDay,

    // Time-of-day fields.
    HourOfDay,
    MinuteOfHour,
    SecondOfMinute,
    NanosecondOfSecond
};

constexpr bool isDateField(Field field) noexcept
{
    switch (field)
    {
    case Field::Year:
    case Field::YearOfEra:
    case Field::Month:
    case Field::DayOfMonth:
    case Field::DayOfYear:
    case Field::DayOfWeek:
    case Field::WeekOfWeekYear:
    case Field::WeekYear:
    case Field::DaysInMonth:
    case Field::DaysInYear:
    case Field::IsLeapMonth:
        return true;
    default:
        return false;
    }
}

constexpr bool isDayField(Field field) noexcept
{
    return field == Field::ProlepticDay || field == Field::EpochDay;
}

constexpr bool isTimeField(Field field) noexcept
{
    return field == Field::HourOfDay || field == Field::MinuteOfHour || field == Field::SecondOfMinute ||
           field == Field::NanosecondOfSecond;
}

} // namespace temporal
