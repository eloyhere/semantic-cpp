#pragma once

#include "temporal/types.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "field.h"
#include "overflow.h"
#include "period.h"
#include "unit.h"

namespace temporal
{

// A signed number of whole days. It is a duration, never a calendar unit: it is
// deliberately not possible to add "one month" as a number of days.
class Days
{
  public:
    constexpr Days() noexcept : count(0) {}
    explicit constexpr Days(Timestamp count) noexcept : count(count) {}

    constexpr Timestamp getCount() const noexcept { return count; }
    constexpr bool isZero() const noexcept { return count == 0; }
    constexpr bool isNegative() const noexcept { return count < 0; }

    constexpr Days operator-() const
    {
        if (count == std::numeric_limits<Timestamp>::min())
        {
            throw std::overflow_error("temporal: day count overflow");
        }
        return Days(-count);
    }

    constexpr bool operator==(Days other) const noexcept { return count == other.count; }
    constexpr bool operator!=(Days other) const noexcept { return count != other.count; }
    constexpr bool operator<(Days other) const noexcept { return count < other.count; }
    constexpr bool operator<=(Days other) const noexcept { return count <= other.count; }
    constexpr bool operator>(Days other) const noexcept { return count > other.count; }
    constexpr bool operator>=(Days other) const noexcept { return count >= other.count; }

  private:
    Timestamp count;
};

// A serial day count on the calendar-independent time line, counted in Rata Die
// days: 0001-01-01 of the proleptic Gregorian calendar is day 1, 1970-01-01 is
// day 719163, and day 0 is 0000-12-31. This is the interchange representation
// between calendars: converting a date to another calendar only requires its
// day number, never the calendar that produced it.
class DayNumber
{
  public:
    constexpr DayNumber() noexcept : value(0) {}
    explicit constexpr DayNumber(std::int32_t value) noexcept : value(value) {}

    constexpr std::int32_t getValue() const noexcept { return value; }

    constexpr DayNumber operator+(Days offset) const
    {
        const Timestamp result = static_cast<Timestamp>(value) + offset.getCount();
        if (result < std::numeric_limits<std::int32_t>::min() || result > std::numeric_limits<std::int32_t>::max())
        {
            throw std::overflow_error("temporal: day number overflow");
        }
        return DayNumber(static_cast<std::int32_t>(result));
    }

    constexpr DayNumber operator-(Days offset) const { return *this + (-offset); }

    constexpr Days operator-(DayNumber other) const { return Days(static_cast<Timestamp>(value) - other.value); }

    constexpr bool operator==(DayNumber other) const noexcept { return value == other.value; }
    constexpr bool operator!=(DayNumber other) const noexcept { return value != other.value; }
    constexpr bool operator<(DayNumber other) const noexcept { return value < other.value; }
    constexpr bool operator<=(DayNumber other) const noexcept { return value <= other.value; }
    constexpr bool operator>(DayNumber other) const noexcept { return value > other.value; }
    constexpr bool operator>=(DayNumber other) const noexcept { return value >= other.value; }

  private:
    std::int32_t value;
};

constexpr DayNumber operator+(Days offset, DayNumber day)
{
    return day + offset;
}

// The decoded fields of a date in calendar Calendar. Year and Month are the
// calendar's own field types, so a lunisolar calendar is free to use a month
// value that carries a leap-month flag instead of an integer 1..12.
template <typename Year, typename Month> struct DateFields
{
    Year y;
    Month m;
    int d;
};

// A calendar date in a compile-time calendar. It stores only a serial day
// number, so all fields are always mutually consistent, copying is trivial and
// the object is four bytes wide; the calendar's fields are decoded on demand by
// the policy. There is no default constructor: a "zero date" (as in .NET's
// DateTime.MinValue or an all-zero std::tm) is a well-known source of silent
// bugs, and every date must be created from real fields or a real day number.
//
// Calendar is a compile-time policy with this contract (all functions are
// constexpr and noexcept unless marked otherwise):
//
//   using year_type                        // calendar-specific year value
//   using month_type                       // calendar-specific month value
//   using weekday_type                     // calendar-specific weekday value
//   using fields = DateFields<year_type, month_type>
//   static constexpr const char *getName()
//   static constexpr bool isLeapYear(year_type)
//   static constexpr int getMonthsInYear(year_type)          // 12 for Gregorian
//   static constexpr int getDaysInMonth(year_type, month_type)
//   static constexpr int getDaysInYear(year_type)
//   static constexpr bool isValidDate(year_type, month_type, int day)
//   static constexpr DayNumber toDayNumber(year_type, month_type, int day)
//   static constexpr fields fromDayNumber(DayNumber)
//   static constexpr weekday_type getWeekday(DayNumber)
//   static constexpr int getDayOfYear(DayNumber)
//   static constexpr Timestamp getField(DayNumber, Field)
//   static constexpr bool isSupportedField(Field)
//   static constexpr DayNumber addYears(DayNumber, Timestamp years,
//                                       Overflow overflow = Overflow::Constrain)
//   static constexpr DayNumber addMonths(DayNumber, Timestamp months,
//                                        Overflow overflow = Overflow::Constrain)
//   static constexpr Period until(DayNumber start, DayNumber end)
//   static constexpr Timestamp until(DayNumber start, DayNumber end, Unit)
//   static constexpr DayNumber getMinimumDayNumber()
//   static constexpr DayNumber getMaximumDayNumber()
//   static std::string format(DayNumber)
//   static constexpr std::optional<DayNumber> tryParse(std::string_view)
//
// The policy owns the field types because eras, month names, year ranges and
// week conventions are calendar data, not universal concepts. It also owns
// month/year arithmetic (a lunisolar calendar cannot treat "one month" as a
// fixed ordinal step) and the representable day-number window, so a calendar
// whose year numbering starts much earlier or later than the Gregorian one does
// not silently truncate dates.
template <typename Calendar> class BasicDate
{
  public:
    using calendar_type = Calendar;
    using year_type = typename Calendar::year_type;
    using month_type = typename Calendar::month_type;
    using weekday_type = typename Calendar::weekday_type;

    BasicDate() = delete;

    explicit constexpr BasicDate(DayNumber day) : serial(day)
    {
        if (day < Calendar::getMinimumDayNumber() || day > Calendar::getMaximumDayNumber())
        {
            throw std::out_of_range("temporal: day number outside the calendar range");
        }
    }

    static constexpr BasicDate minimum() { return BasicDate(Calendar::getMinimumDayNumber()); }
    static constexpr BasicDate maximum() { return BasicDate(Calendar::getMaximumDayNumber()); }

    static constexpr std::optional<BasicDate> tryFromDayNumber(DayNumber day) noexcept
    {
        if (day < Calendar::getMinimumDayNumber() || day > Calendar::getMaximumDayNumber())
        {
            return std::nullopt;
        }
        return BasicDate(day);
    }

    static constexpr BasicDate fromDayNumber(DayNumber day) { return BasicDate(day); }

    // Strict factory: an invalid date is never normalised into a different one
    // (JavaScript's Date rolls 2024-02-30 into March 1st; C++20's
    // year_month_day happily stores an invalid value). Either the fields are
    // valid and the result is exact, or there is no result.
    static constexpr std::optional<BasicDate> tryOf(year_type year, month_type month, int day) noexcept
    {
        if (!Calendar::isValidDate(year, month, day))
        {
            return std::nullopt;
        }
        return tryFromDayNumber(Calendar::toDayNumber(year, month, day));
    }

    static constexpr BasicDate of(year_type year, month_type month, int day)
    {
        if (!Calendar::isValidDate(year, month, day))
        {
            throw std::invalid_argument("temporal: invalid date");
        }
        return BasicDate(Calendar::toDayNumber(year, month, day));
    }

    static constexpr std::optional<BasicDate> tryParse(std::string_view text) noexcept
    {
        const std::optional<DayNumber> day = Calendar::tryParse(text);
        if (!day)
        {
            return std::nullopt;
        }
        return tryFromDayNumber(*day);
    }

    static constexpr BasicDate parse(std::string_view text)
    {
        const std::optional<BasicDate> value = tryParse(text);
        if (!value)
        {
            throw std::invalid_argument("temporal: invalid date text");
        }
        return *value;
    }

    constexpr DayNumber getDayNumber() const noexcept { return serial; }

    constexpr year_type getYear() const noexcept { return Calendar::fromDayNumber(serial).y; }
    constexpr month_type getMonth() const noexcept { return Calendar::fromDayNumber(serial).m; }
    constexpr int getDay() const noexcept { return Calendar::fromDayNumber(serial).d; }

    // Conventional EpochDay: days since 1970-01-01, 1-based (1970-01-01 is 1),
    // as in java.time's LocalDate.toEpochDay() but with the 1-based convention
    // used by Instant::toEpoch(). This is the Unix epoch, not the
    // natural-calendar epoch of Instant::toProleptic().
    constexpr Timestamp toEpochDay() const noexcept
    {
        return static_cast<Timestamp>(serial.getValue()) - 719163 + 1;
    }

    static constexpr std::optional<BasicDate> tryFromEpochDay(Timestamp epochDay) noexcept
    {
        constexpr Timestamp serialOfEpochDayOne = 719163;
        constexpr Timestamp offset = serialOfEpochDayOne - 1;
        if (epochDay < std::numeric_limits<std::int32_t>::min() - offset ||
            epochDay > std::numeric_limits<std::int32_t>::max() - offset)
        {
            return std::nullopt;
        }
        return tryFromDayNumber(DayNumber(static_cast<std::int32_t>(epochDay + offset)));
    }

    static constexpr BasicDate fromEpochDay(Timestamp epochDay)
    {
        const std::optional<BasicDate> value = tryFromEpochDay(epochDay);
        if (!value)
        {
            throw std::out_of_range("temporal: epoch day outside the supported range");
        }
        return *value;
    }

    constexpr weekday_type getWeekday() const noexcept { return Calendar::getWeekday(serial); }

    constexpr int getDayOfYear() const noexcept { return Calendar::getDayOfYear(serial); }

    constexpr int getDaysInMonth() const noexcept { return Calendar::getDaysInMonth(getYear(), getMonth()); }

    constexpr int getDaysInYear() const noexcept { return Calendar::getDaysInYear(getYear()); }

    constexpr bool isLeapYear() const noexcept { return Calendar::isLeapYear(getYear()); }

    // Generic field access for table-driven code and formatters. The typed
    // getters above remain the preferred API; this mirrors java.time's
    // TemporalField without its long-typed, runtime-failing surface, because the
    // field set is closed and the policy states which fields it defines.
    constexpr Timestamp get(Field field) const
    {
        if (field == Field::EpochDay)
        {
            return toEpochDay();
        }
        if (field == Field::ProlepticDay)
        {
            return toEpochDay() + (ProlepticDayOfUnixEpoch - 1);
        }
        return Calendar::getField(serial, field);
    }

    constexpr bool isSupported(Field field) const noexcept
    {
        if (isDayField(field))
        {
            return true;
        }
        return Calendar::isSupportedField(field);
    }

    // Unit-based arithmetic, mirroring java.time's TemporalUnit and Temporal's
    // Duration units. A calendar date accepts days, weeks, months and years; a
    // time-based unit is rejected instead of being silently converted into an
    // approximate number of days.
    constexpr BasicDate plus(Timestamp amount, Unit unit, Overflow overflow = Overflow::Constrain) const
    {
        constexpr Timestamp maximum = std::numeric_limits<Timestamp>::max();
        constexpr Timestamp minimum = std::numeric_limits<Timestamp>::min();
        switch (unit)
        {
        case Unit::Days:
            return *this + Days(amount);
        case Unit::Weeks:
            if (amount > maximum / 7 || amount < minimum / 7)
            {
                throw std::overflow_error("temporal: week offset overflow");
            }
            return *this + Days(amount * 7);
        case Unit::Months:
            return BasicDate(Calendar::addMonths(serial, amount, overflow));
        case Unit::Years:
            return BasicDate(Calendar::addYears(serial, amount, overflow));
        default:
            throw std::invalid_argument("temporal: a time-based unit cannot be applied to a date");
        }
    }

    // Whole units from this date to other, with the sign of other - this. Weeks
    // truncate towards zero, and months/years follow the calendar's own month
    // length rules rather than a fixed 30/365-day approximation.
    constexpr Timestamp until(const BasicDate &other, Unit unit) const
    {
        switch (unit)
        {
        case Unit::Days:
            return (other - *this).getCount();
        case Unit::Weeks:
            return (other - *this).getCount() / 7;
        case Unit::Months:
        case Unit::Years:
            return Calendar::until(serial, other.serial, unit);
        default:
            throw std::invalid_argument("temporal: a time-based unit cannot be measured on a date");
        }
    }

    // Adding days is exact. Adding months or years is calendar arithmetic and
    // follows the policy's clamping rule (Gregorian: 2024-01-31 + 1 month is
    // 2024-02-29). The order years, months, days is fixed and observable.
    constexpr BasicDate plus(const Period &period, Overflow overflow = Overflow::Constrain) const
    {
        DayNumber current = serial;
        if (period.getYears() != 0)
        {
            current = Calendar::addYears(current, period.getYears(), overflow);
        }
        if (period.getMonths() != 0)
        {
            current = Calendar::addMonths(current, period.getMonths(), overflow);
        }
        if (period.getDays() != 0)
        {
            current = current + Days(period.getDays());
        }
        return BasicDate(current);
    }

    constexpr BasicDate minus(const Period &period, Overflow overflow = Overflow::Constrain) const
    {
        return plus(period.negated(), overflow);
    }

    // The period that leads from this date to other, normalised as years,
    // months and days. Because period addition clamps at month ends, the round
    // trip is not guaranteed for every pair (java.time has the same caveat);
    // use the day-number difference when an exact distance is needed.
    constexpr Period until(const BasicDate &other) const { return Calendar::until(serial, other.serial); }

    constexpr BasicDate operator+(Days offset) const { return BasicDate(serial + offset); }
    constexpr BasicDate operator-(Days offset) const { return BasicDate(serial - offset); }
    constexpr BasicDate operator+(const Period &period) const { return plus(period); }
    constexpr BasicDate operator-(const Period &period) const { return minus(period); }
    constexpr Days operator-(const BasicDate &other) const { return serial - other.serial; }

    constexpr BasicDate &operator+=(Days offset)
    {
        *this = *this + offset;
        return *this;
    }

    constexpr BasicDate &operator-=(Days offset)
    {
        *this = *this - offset;
        return *this;
    }

    std::string to_string() const { return Calendar::format(serial); }

    constexpr bool operator==(const BasicDate &other) const noexcept { return serial == other.serial; }
    constexpr bool operator!=(const BasicDate &other) const noexcept { return serial != other.serial; }
    constexpr bool operator<(const BasicDate &other) const noexcept { return serial < other.serial; }
    constexpr bool operator<=(const BasicDate &other) const noexcept { return serial <= other.serial; }
    constexpr bool operator>(const BasicDate &other) const noexcept { return serial > other.serial; }
    constexpr bool operator>=(const BasicDate &other) const noexcept { return serial >= other.serial; }

  private:
    DayNumber serial;
};

// Convert between calendars through the serial day number. Every date in the
// target calendar's range is representable; a target range that does not cover
// the source date throws std::out_of_range.
template <typename Target, typename Source> constexpr Target calendarCast(const Source &source)
{
    return Target::fromDayNumber(source.getDayNumber());
}

template <typename Calendar> std::ostream &operator<<(std::ostream &stream, const BasicDate<Calendar> &value)
{
    return stream << value.to_string();
}

} // namespace temporal

namespace std
{
template <> struct hash<temporal::DayNumber>
{
    size_t operator()(const temporal::DayNumber &value) const noexcept
    {
        return hash<std::int32_t>{}(value.getValue());
    }
};

template <typename Calendar> struct hash<temporal::BasicDate<Calendar>>
{
    size_t operator()(const temporal::BasicDate<Calendar> &value) const noexcept
    {
        return hash<std::int32_t>{}(value.getDayNumber().getValue());
    }
};
} // namespace std
