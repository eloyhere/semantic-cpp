#include "temporal/temporal.h"

#include "test_framework.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>

using temporal::Day;
using temporal::DayNumber;
using temporal::Days;
using temporal::Disambiguation;
using temporal::Duration;
using temporal::Field;
using temporal::Instant;
using temporal::isCalendarBased;
using temporal::isDateBased;
using temporal::isDateField;
using temporal::isDayField;
using temporal::isTimeBased;
using temporal::isTimeField;
using temporal::LocalResultKind;
using temporal::nanosecondsPerUnit;
using temporal::NanosPerDay;
using temporal::Offset;
using temporal::Overflow;
using temporal::Period;
using temporal::Time;
using temporal::Timestamp;
using temporal::Unit;
using temporal::Zone;
using temporal::ZoneLocalResult;
using temporal::gregory::Calendar;
using temporal::gregory::Date;
using temporal::gregory::DateTime;
using temporal::gregory::Era;
using temporal::gregory::isoNumber;
using temporal::gregory::isWeekend;
using temporal::gregory::Month;
using temporal::gregory::monthFromNumber;
using temporal::gregory::number;
using temporal::gregory::OffsetDateTime;
using temporal::gregory::sundayNumber;
using temporal::gregory::WeekDate;
using temporal::gregory::Weekday;
using temporal::gregory::weekdayFromIso;
using temporal::gregory::weekdayFromSunday;
using temporal::gregory::Year;
using temporal::gregory::ZoneTemporal;

namespace
{

struct DateVector
{
    int year;
    Month month;
    int day;
    int serial;
    int isoWeekday;
    int weekYear;
    int week;
    int dayOfYear;
};

// Expected values come from Python's proleptic Gregorian date.toordinal() and
// date.isocalendar(); negative years were generated with a day-by-day
// implementation that does not share code with the library.
const DateVector dateVectors[] = {
    {1, Month::January, 1, 1, 1, 1, 1, 1},
    {1, Month::December, 31, 365, 1, 2, 1, 365},
    {4, Month::February, 29, 1155, 7, 4, 9, 60},
    {100, Month::March, 1, 36219, 1, 100, 9, 60},
    {400, Month::February, 29, 145791, 2, 400, 9, 60},
    {1582, Month::October, 4, 577725, 1, 1582, 40, 277},
    {1582, Month::October, 15, 577736, 5, 1582, 41, 288},
    {1600, Month::February, 29, 584082, 2, 1600, 9, 60},
    {1700, Month::March, 1, 620607, 1, 1700, 9, 60},
    {1900, Month::February, 28, 693654, 3, 1900, 9, 59},
    {1900, Month::March, 1, 693655, 4, 1900, 9, 60},
    {1970, Month::January, 1, 719163, 4, 1970, 1, 1},
    {2000, Month::February, 29, 730179, 2, 2000, 9, 60},
    {2023, Month::December, 31, 738885, 7, 2023, 52, 365},
    {2024, Month::January, 1, 738886, 1, 2024, 1, 1},
    {2024, Month::February, 29, 738945, 4, 2024, 9, 60},
    {2024, Month::October, 26, 739185, 6, 2024, 43, 300},
    {2025, Month::December, 29, 739614, 1, 2026, 1, 363},
    {2026, Month::January, 1, 739617, 4, 2026, 1, 1},
    {2100, Month::February, 28, 766703, 7, 2100, 8, 59},
    {2100, Month::March, 1, 766704, 1, 2100, 9, 60},
    {4000, Month::February, 29, 1460664, 2, 4000, 9, 60},
    {9999, Month::December, 31, 3652059, 5, 9999, 52, 365},
    {-1, Month::December, 31, -366, 5, -1, 52, 365},
    {-4, Month::February, 29, -1767, 4, -4, 9, 60},
    {-100, Month::March, 1, -36830, 4, -100, 9, 60},
    {-400, Month::February, 29, -146403, 2, -400, 9, 60},
    {-2696, Month::March, 11, -984989, 2, -2696, 11, 71},
    {-9999, Month::January, 1, -3652424, 1, -9999, 1, 1},
    {-9999, Month::December, 31, -3652060, 1, -9998, 1, 365},
};

Date makeDate(int year, Month month, int day)
{
    return Date::of(Year(year), month, day);
}

} // namespace

static_assert(sizeof(Date) == sizeof(std::int32_t), "a date must stay a single day number");
static_assert(sizeof(Instant) >= sizeof(Day), "an instant stores a Day plus a scale tag");
static_assert(std::is_trivially_copyable<Date>::value, "dates are values");
static_assert(std::is_trivially_copyable<Instant>::value, "instants are values");
static_assert(Date::of(Year(2024), Month::February, 29).getDay() == 29, "leap day is constexpr");
static_assert(Date::of(Year(1), Month::January, 1).getWeekday() == Weekday::Monday, "weekday is constexpr");
static_assert(Instant(-1, 0).toProleptic() == -1, "a day before the natural epoch is proleptic day 0");
static_assert(Instant(0, 0).toProleptic() == 0, "proleptic day 0 is the day before the natural epoch");
static_assert(Instant(1704153, 0).toEpoch() == 1, "1970-01-01 is epoch day 1");
static_assert(Instant(0, 0).toEpoch() == -1704152, "the epoch day scale is 1-based");

TEST_CASE("temporal gregorian era and year numbering")
{
    CHECK_EQ(Year(2024).getEra(), Era::Ce);
    CHECK_EQ(Year(1).getEra(), Era::Ce);
    CHECK_EQ(Year(0).getEra(), Era::Bce);
    CHECK_EQ(Year(-1).getEra(), Era::Bce);
    CHECK_EQ(Year(0).getYearInEra(), 1);
    CHECK_EQ(Year(-1).getYearInEra(), 2);
    CHECK_EQ(Year(-9999).getYearInEra(), 10000);
    CHECK_EQ(Year::fromEra(Era::Bce, 1), Year(0));
    CHECK_EQ(Year::fromEra(Era::Bce, 2), Year(-1));
    CHECK_EQ(Year::fromEra(Era::Ce, 2024), Year(2024));
    CHECK_THROWS_AS(Year::fromEra(Era::Bce, 0), std::invalid_argument);
    CHECK_THROWS_AS(Year::fromEra(Era::Ce, 10000), std::invalid_argument);
    CHECK(Year(9999).isOk());
    CHECK(Year(-9999).isOk());
    CHECK_FALSE(Year(10000).isOk());
    CHECK_FALSE(Year(-10000).isOk());
    CHECK_EQ(Year::minimum(), -9999);
    CHECK_EQ(Year::maximum(), 9999);
}

TEST_CASE("temporal gregorian month and weekday vocabulary")
{
    CHECK_EQ(number(Month::January), 1);
    CHECK_EQ(number(Month::February), 2);
    CHECK_EQ(number(Month::December), 12);
    CHECK_EQ(monthFromNumber(0), std::nullopt);
    CHECK_EQ(monthFromNumber(13), std::nullopt);
    CHECK_EQ(*monthFromNumber(2), Month::February);
    CHECK_EQ(*monthFromNumber(12), Month::December);

    CHECK_EQ(isoNumber(Weekday::Monday), 1);
    CHECK_EQ(isoNumber(Weekday::Sunday), 7);
    CHECK_EQ(sundayNumber(Weekday::Sunday), 0);
    CHECK_EQ(sundayNumber(Weekday::Monday), 1);
    CHECK_EQ(sundayNumber(Weekday::Saturday), 6);
    CHECK_EQ(*weekdayFromIso(7), Weekday::Sunday);
    CHECK_EQ(weekdayFromIso(0), std::nullopt);
    CHECK_EQ(*weekdayFromSunday(0), Weekday::Sunday);
    CHECK_EQ(*weekdayFromSunday(6), Weekday::Saturday);
    CHECK_EQ(weekdayFromSunday(7), std::nullopt);
    CHECK(isWeekend(Weekday::Saturday));
    CHECK(isWeekend(Weekday::Sunday));
    CHECK_FALSE(isWeekend(Weekday::Friday));
}

TEST_CASE("temporal gregorian known dates")
{
    for (const DateVector &vector : dateVectors)
    {
        const Date date = makeDate(vector.year, vector.month, vector.day);
        CHECK_EQ(date.getDayNumber().getValue(), vector.serial);
        CHECK_EQ(date.getYear().getValue(), vector.year);
        CHECK_EQ(number(date.getMonth()), number(vector.month));
        CHECK_EQ(date.getDay(), vector.day);
        CHECK_EQ(isoNumber(date.getWeekday()), vector.isoWeekday);
        CHECK_EQ(date.getDayOfYear(), vector.dayOfYear);
        CHECK_EQ(Date::fromDayNumber(date.getDayNumber()), date);

        const WeekDate week = WeekDate::from(date);
        CHECK_EQ(week.getWeekYear().getValue(), vector.weekYear);
        CHECK_EQ(week.getWeek(), vector.week);
        CHECK_EQ(isoNumber(week.getWeekday()), vector.isoWeekday);
        CHECK_EQ(week.toDate(), date);

        CHECK_EQ(Date::parse(date.to_string()), date);
    }
}

TEST_CASE("temporal gregorian leap years")
{
    CHECK(Calendar::isLeapYear(Year(2000)));
    CHECK(Calendar::isLeapYear(Year(1600)));
    CHECK(Calendar::isLeapYear(Year(2024)));
    CHECK(Calendar::isLeapYear(Year(0)));
    CHECK(Calendar::isLeapYear(Year(-4)));
    CHECK(Calendar::isLeapYear(Year(-400)));
    CHECK_FALSE(Calendar::isLeapYear(Year(1900)));
    CHECK_FALSE(Calendar::isLeapYear(Year(1700)));
    CHECK_FALSE(Calendar::isLeapYear(Year(2023)));
    CHECK_FALSE(Calendar::isLeapYear(Year(-1)));
    CHECK_FALSE(Calendar::isLeapYear(Year(-100)));
    CHECK_EQ(Calendar::getDaysInYear(Year(2024)), 366);
    CHECK_EQ(Calendar::getDaysInYear(Year(2023)), 365);
    CHECK_EQ(makeDate(2024, Month::February, 1).getDaysInMonth(), 29);
    CHECK_EQ(makeDate(2023, Month::February, 1).getDaysInMonth(), 28);
    CHECK_EQ(makeDate(2024, Month::April, 1).getDaysInMonth(), 30);
    CHECK_EQ(makeDate(2024, Month::July, 1).getDaysInMonth(), 31);
}

TEST_CASE("temporal gregorian invalid dates are rejected, never normalised")
{
    CHECK_FALSE(Date::tryOf(Year(2023), Month::February, 29).has_value());
    CHECK_FALSE(Date::tryOf(Year(2024), Month::April, 31).has_value());
    CHECK_FALSE(Date::tryOf(Year(2024), Month::January, 0).has_value());
    CHECK_FALSE(Date::tryOf(Year(2024), Month::January, 32).has_value());
    CHECK_FALSE(Date::tryOf(Year(10000), Month::January, 1).has_value());
    CHECK_FALSE(Date::tryOf(Year(-10000), Month::January, 1).has_value());
    CHECK_THROWS_AS(Date::of(Year(2023), Month::February, 29), std::invalid_argument);
    CHECK_THROWS_AS(Date::of(Year(2024), Month::June, 31), std::invalid_argument);
    CHECK_THROWS_AS(Date::of(Year(10000), Month::January, 1), std::invalid_argument);
}

TEST_CASE("temporal gregorian day number range")
{
    CHECK_EQ(Date::minimum().getDayNumber().getValue(), -3652424);
    CHECK_EQ(Date::maximum().getDayNumber().getValue(), 3652059);
    CHECK_EQ(Date::minimum().getYear().getValue(), -9999);
    CHECK_EQ(Date::minimum().getMonth(), Month::January);
    CHECK_EQ(Date::minimum().getDay(), 1);
    CHECK_EQ(Date::maximum().getYear().getValue(), 9999);
    CHECK_EQ(Date::maximum().getMonth(), Month::December);
    CHECK_EQ(Date::maximum().getDay(), 31);
    CHECK_FALSE(Date::tryFromDayNumber(DayNumber(-3652425)).has_value());
    CHECK_FALSE(Date::tryFromDayNumber(DayNumber(3652060)).has_value());
    CHECK_THROWS_AS(Date::fromDayNumber(DayNumber(-3652425)), std::out_of_range);
    CHECK_THROWS_AS(Date::fromDayNumber(DayNumber(3652060)), std::out_of_range);
}

TEST_CASE("temporal gregorian serial round trip")
{
    for (int serial = -3652424; serial < -3652424 + 20000; ++serial)
    {
        const Date date = Date::fromDayNumber(DayNumber(serial));
        CHECK_EQ(date.getDayNumber().getValue(), serial);
        CHECK_EQ(Date::of(date.getYear(), date.getMonth(), date.getDay()), date);
    }
    for (int serial = 3652059 - 20000; serial <= 3652059; ++serial)
    {
        const Date date = Date::fromDayNumber(DayNumber(serial));
        CHECK_EQ(date.getDayNumber().getValue(), serial);
        CHECK_EQ(Date::of(date.getYear(), date.getMonth(), date.getDay()), date);
    }
    for (int year = 1900; year <= 2100; ++year)
    {
        for (int month = 1; month <= 12; ++month)
        {
            const Month currentMonth = static_cast<Month>(month);
            const int days = Calendar::getDaysInMonth(Year(year), currentMonth);
            for (int day = 1; day <= days; ++day)
            {
                const Date date = Date::of(Year(year), currentMonth, day);
                CHECK_EQ(Date::fromDayNumber(date.getDayNumber()), date);
                CHECK_EQ(date.getDay(), day);
                CHECK_EQ(number(date.getMonth()), month);
                CHECK_EQ(date.getYear().getValue(), year);
            }
        }
    }
}

TEST_CASE("temporal gregorian day arithmetic")
{
    const Date leapDay = makeDate(2024, Month::February, 29);
    CHECK_EQ(leapDay + Days(1), makeDate(2024, Month::March, 1));
    CHECK_EQ(leapDay - Days(1), makeDate(2024, Month::February, 28));
    CHECK_EQ(leapDay - Days(59), makeDate(2024, Month::January, 1));
    CHECK_EQ(makeDate(2023, Month::December, 31) + Days(1), makeDate(2024, Month::January, 1));
    CHECK_EQ(makeDate(2024, Month::February, 28) + Days(2), makeDate(2024, Month::March, 1));
    CHECK_EQ((makeDate(2024, Month::March, 1) - makeDate(2024, Month::February, 28)).getCount(), 2);
    CHECK_EQ((makeDate(2024, Month::February, 28) - makeDate(2024, Month::March, 1)).getCount(), -2);
    CHECK_EQ((leapDay - leapDay).getCount(), 0);
    CHECK_EQ(leapDay.getDayOfYear(), 60);
    CHECK_EQ(makeDate(2023, Month::December, 31).getDayOfYear(), 365);

    Date changed = leapDay;
    changed += Days(1);
    CHECK_EQ(changed, makeDate(2024, Month::March, 1));
    changed -= Days(2);
    CHECK_EQ(changed, makeDate(2024, Month::February, 28));

    CHECK_THROWS_AS(Date::maximum() + Days(1), std::out_of_range);
    CHECK_THROWS_AS(Date::minimum() - Days(1), std::out_of_range);
}

TEST_CASE("temporal gregorian month and year arithmetic clamps")
{
    CHECK_EQ(makeDate(2024, Month::January, 31) + Period(0, 1, 0), makeDate(2024, Month::February, 29));
    CHECK_EQ(makeDate(2023, Month::January, 31) + Period(0, 1, 0), makeDate(2023, Month::February, 28));
    CHECK_EQ(makeDate(2024, Month::March, 31) - Period(0, 1, 0), makeDate(2024, Month::February, 29));
    CHECK_EQ(makeDate(2024, Month::February, 29) + Period(1, 0, 0), makeDate(2025, Month::February, 28));
    CHECK_EQ(makeDate(2024, Month::February, 29) + Period(4, 0, 0), makeDate(2028, Month::February, 29));
    CHECK_EQ(makeDate(2024, Month::December, 31) + Period(0, 2, 0), makeDate(2025, Month::February, 28));
    CHECK_EQ(makeDate(2024, Month::January, 31) + Period(0, 1, 1), makeDate(2024, Month::March, 1));
    CHECK_EQ(makeDate(2024, Month::January, 30) + Period(0, 1, 1), makeDate(2024, Month::March, 1));
    CHECK_EQ((makeDate(2024, Month::January, 30) + Period(0, 0, 1)) + Period(0, 1, 0),
             makeDate(2024, Month::February, 29));
    CHECK_EQ(makeDate(2024, Month::March, 1) - Period(0, 0, 1), makeDate(2024, Month::February, 29));
    CHECK_EQ(makeDate(2024, Month::March, 31).plus(Period(0, -1, 0)), makeDate(2024, Month::February, 29));
    CHECK_EQ(makeDate(2024, Month::March, 31).minus(Period(0, 1, 0)), makeDate(2024, Month::February, 29));
    CHECK_THROWS_AS(Date::maximum() + Period(0, 1, 0), std::out_of_range);
    CHECK_THROWS_AS(Date::minimum() - Period(0, 1, 0), std::out_of_range);
}

TEST_CASE("temporal gregorian period between dates")
{
    CHECK_EQ(makeDate(2024, Month::January, 31).until(makeDate(2024, Month::March, 1)), Period(0, 1, 1));
    CHECK_EQ(makeDate(2024, Month::March, 1).until(makeDate(2024, Month::January, 31)), Period(0, -1, -1));
    CHECK_EQ(makeDate(2024, Month::January, 1).until(makeDate(2024, Month::January, 1)), Period());
    CHECK_EQ(makeDate(2020, Month::February, 29).until(makeDate(2024, Month::February, 29)), Period(4, 0, 0));
    CHECK_EQ(makeDate(2024, Month::January, 1).until(makeDate(2025, Month::January, 1)), Period(1, 0, 0));
    CHECK_EQ(makeDate(2024, Month::January, 15).until(makeDate(2024, Month::March, 20)), Period(0, 2, 5));
    CHECK_EQ(makeDate(2024, Month::March, 20).until(makeDate(2024, Month::January, 15)), Period(0, -2, -5));

    const Date start = makeDate(2024, Month::January, 31);
    const Date end = makeDate(2024, Month::March, 1);
    CHECK_EQ(start + start.until(end), end);

    // Documented caveat: clamping makes the round trip inexact for some pairs
    // (java.time.Period has the same behaviour).
    const Date leapStart = makeDate(2024, Month::February, 29);
    const Date later = makeDate(2028, Month::February, 28);
    CHECK_EQ(leapStart.until(later), Period(3, 11, 30));
    CHECK_EQ(leapStart + leapStart.until(later), makeDate(2028, Month::February, 27));
}

TEST_CASE("temporal gregorian iso week dates")
{
    CHECK_EQ(WeekDate::getWeeksInYear(Year(2004)), 53);
    CHECK_EQ(WeekDate::getWeeksInYear(Year(2015)), 53);
    CHECK_EQ(WeekDate::getWeeksInYear(Year(2020)), 53);
    CHECK_EQ(WeekDate::getWeeksInYear(Year(2021)), 52);
    CHECK_EQ(WeekDate::getWeeksInYear(Year(2016)), 52);
    CHECK_EQ(WeekDate::getWeeksInYear(Year(2026)), 53);

    CHECK_EQ(WeekDate::from(makeDate(2021, Month::January, 1)), WeekDate::of(Year(2020), 53, Weekday::Friday));
    CHECK_EQ(WeekDate::from(makeDate(2020, Month::December, 31)), WeekDate::of(Year(2020), 53, Weekday::Thursday));
    CHECK_EQ(WeekDate::from(makeDate(2024, Month::January, 1)), WeekDate::of(Year(2024), 1, Weekday::Monday));
    CHECK_EQ(WeekDate::from(makeDate(1995, Month::January, 1)), WeekDate::of(Year(1994), 52, Weekday::Sunday));
    CHECK_EQ(WeekDate::from(makeDate(1996, Month::December, 31)), WeekDate::of(Year(1997), 1, Weekday::Tuesday));
    CHECK_EQ(WeekDate::of(Year(2020), 53, Weekday::Friday).toDate(), makeDate(2021, Month::January, 1));
    CHECK_EQ(WeekDate::of(Year(2024), 9, Weekday::Thursday).toDate(), makeDate(2024, Month::February, 29));
    CHECK_EQ(WeekDate::of(Year(2026), 1, Weekday::Monday).toDate(), makeDate(2025, Month::December, 29));
    CHECK_EQ(WeekDate::of(Year(2026), 1, Weekday::Monday).to_string(), "2026-W01-1");

    CHECK_THROWS_AS(WeekDate::of(Year(2021), 53, Weekday::Monday), std::invalid_argument);
    CHECK_THROWS_AS(WeekDate::of(Year(2024), 0, Weekday::Monday), std::invalid_argument);
    CHECK_FALSE(WeekDate::tryOf(Year(2021), 53, Weekday::Monday).has_value());

    for (int year = 1900; year <= 2100; ++year)
    {
        for (int month = 1; month <= 12; ++month)
        {
            const Month currentMonth = static_cast<Month>(month);
            const int days = Calendar::getDaysInMonth(Year(year), currentMonth);
            for (int day = 1; day <= days; ++day)
            {
                const Date date = Date::of(Year(year), currentMonth, day);
                const WeekDate week = WeekDate::from(date);
                CHECK_EQ(week.toDate(), date);
                CHECK(week.getWeek() >= 1);
                CHECK(week.getWeek() <= WeekDate::getWeeksInYear(week.getWeekYear()));
            }
        }
    }
}

TEST_CASE("temporal gregorian parsing and formatting")
{
    CHECK_EQ(Date::parse("2024-02-29"), makeDate(2024, Month::February, 29));
    CHECK_EQ(Date::parse("0000-02-29"), makeDate(0, Month::February, 29));
    CHECK_EQ(Date::parse("-0001-12-31"), makeDate(-1, Month::December, 31));
    CHECK_EQ(Date::parse("+2024-01-01"), makeDate(2024, Month::January, 1));
    CHECK_EQ(makeDate(2024, Month::February, 29).to_string(), "2024-02-29");
    CHECK_EQ(makeDate(0, Month::January, 1).to_string(), "0000-01-01");
    CHECK_EQ(makeDate(-1, Month::December, 31).to_string(), "-0001-12-31");
    CHECK_EQ(makeDate(-9999, Month::January, 1).to_string(), "-9999-01-01");

    const char *invalid[] = {
        "",           "2024",        "2024-2-9",
        "2024-02-9",  "2024-02-30",  "2024-13-01",
        "2024-00-10", "2024-01-00",  "2024-01-32",
        "2023-02-29", "-0000-01-01", "2024-02-29x",
        "2024/02/29", " 2024-02-29", "2024-02-29T00:00:00",
    };
    for (const char *text : invalid)
    {
        CHECK_FALSE(Date::tryParse(text).has_value());
    }
    CHECK_THROWS_AS(Date::parse("2024-02-30"), std::invalid_argument);

    for (const DateVector &vector : dateVectors)
    {
        const Date date = makeDate(vector.year, vector.month, vector.day);
        CHECK_EQ(Date::parse(date.to_string()), date);
    }
}

TEST_CASE("temporal gregorian comparison and hashing")
{
    CHECK(makeDate(2024, Month::January, 1) < makeDate(2024, Month::January, 2));
    CHECK(makeDate(2023, Month::December, 31) < makeDate(2024, Month::January, 1));
    CHECK(makeDate(2024, Month::January, 1) <= makeDate(2024, Month::January, 1));
    CHECK(makeDate(2024, Month::January, 2) > makeDate(2024, Month::January, 1));
    CHECK(makeDate(2024, Month::January, 1) >= makeDate(2024, Month::January, 1));
    CHECK(makeDate(2024, Month::January, 1) != makeDate(2024, Month::January, 2));

    std::unordered_set<Date> dates;
    dates.insert(makeDate(2024, Month::January, 1));
    dates.insert(makeDate(2024, Month::January, 1));
    dates.insert(makeDate(2024, Month::January, 2));
    CHECK_EQ(dates.size(), std::size_t(2));
    CHECK_EQ(dates.count(makeDate(2024, Month::January, 1)), std::size_t(1));
}

TEST_CASE("temporal unit vocabulary")
{
    CHECK(isTimeBased(Unit::Nanoseconds));
    CHECK(isTimeBased(Unit::Hours));
    CHECK_FALSE(isTimeBased(Unit::Days));
    CHECK(isDateBased(Unit::Days));
    CHECK(isDateBased(Unit::Weeks));
    CHECK(isCalendarBased(Unit::Months));
    CHECK(isCalendarBased(Unit::Years));
    CHECK_FALSE(isCalendarBased(Unit::Weeks));
    CHECK_EQ(nanosecondsPerUnit(Unit::Seconds), 1000000000LL);
    CHECK_EQ(nanosecondsPerUnit(Unit::Days), 86400000000000LL);
    CHECK_EQ(nanosecondsPerUnit(Unit::Months), 0LL);
}

TEST_CASE("temporal gregorian field access")
{
    const Date date = makeDate(2024, Month::February, 29);
    CHECK_EQ(date.get(Field::Year), 2024);
    CHECK_EQ(date.get(Field::YearOfEra), 2024);
    CHECK_EQ(date.get(Field::Month), 2);
    CHECK_EQ(date.get(Field::DayOfMonth), 29);
    CHECK_EQ(date.get(Field::DayOfYear), 60);
    CHECK_EQ(date.get(Field::DayOfWeek), 4);
    CHECK_EQ(date.get(Field::WeekOfWeekYear), 9);
    CHECK_EQ(date.get(Field::WeekYear), 2024);
    CHECK_EQ(date.get(Field::DaysInMonth), 29);
    CHECK_EQ(date.get(Field::DaysInYear), 366);
    CHECK_EQ(date.get(Field::IsLeapMonth), 0);
    CHECK(date.isSupported(Field::WeekYear));

    CHECK_EQ(makeDate(2021, Month::January, 1).get(Field::WeekOfWeekYear), 53);
    CHECK_EQ(makeDate(2021, Month::January, 1).get(Field::WeekYear), 2020);
    CHECK_EQ(makeDate(2021, Month::January, 1).get(Field::DayOfWeek), 5);
    CHECK_EQ(makeDate(0, Month::January, 1).get(Field::Year), 0);
    CHECK_EQ(makeDate(0, Month::January, 1).get(Field::YearOfEra), 1);
}

TEST_CASE("temporal gregorian unit arithmetic")
{
    const Date date = makeDate(2024, Month::January, 31);
    CHECK_EQ(date.plus(1, Unit::Days), makeDate(2024, Month::February, 1));
    CHECK_EQ(date.plus(1, Unit::Weeks), makeDate(2024, Month::February, 7));
    CHECK_EQ(date.plus(1, Unit::Months), makeDate(2024, Month::February, 29));
    CHECK_EQ(date.plus(1, Unit::Years), makeDate(2025, Month::January, 31));
    CHECK_EQ(makeDate(2024, Month::February, 29).plus(1, Unit::Years), makeDate(2025, Month::February, 28));
    CHECK_EQ(date.plus(-1, Unit::Months), makeDate(2023, Month::December, 31));
    CHECK_THROWS_AS(date.plus(1, Unit::Hours), std::invalid_argument);
    CHECK_THROWS_AS(date.plus(std::numeric_limits<Timestamp>::max(), Unit::Weeks), std::overflow_error);

    CHECK_EQ(makeDate(2024, Month::January, 1).until(makeDate(2025, Month::March, 1), Unit::Days), 425);
    CHECK_EQ(makeDate(2024, Month::January, 1).until(makeDate(2025, Month::March, 1), Unit::Weeks), 60);
    CHECK_EQ(makeDate(2024, Month::January, 1).until(makeDate(2025, Month::March, 1), Unit::Months), 14);
    CHECK_EQ(makeDate(2024, Month::January, 1).until(makeDate(2025, Month::March, 1), Unit::Years), 1);
    CHECK_EQ(makeDate(2024, Month::January, 31).until(makeDate(2024, Month::February, 29), Unit::Months), 0);
    CHECK_EQ(makeDate(2024, Month::January, 31).until(makeDate(2024, Month::March, 1), Unit::Months), 1);
    CHECK_EQ(makeDate(2024, Month::February, 1).until(makeDate(2024, Month::March, 1), Unit::Months), 1);
    CHECK_EQ(makeDate(2024, Month::March, 1).until(makeDate(2024, Month::January, 1), Unit::Days), -60);
    CHECK_EQ(makeDate(2024, Month::January, 1).until(makeDate(2024, Month::January, 1), Unit::Days), 0);
    CHECK_THROWS_AS(makeDate(2024, Month::January, 1).until(makeDate(2024, Month::March, 1), Unit::Hours),
                    std::invalid_argument);
}

TEST_CASE("temporal instant fields and units")
{
    const Instant instant(10, 3661123456789LL);
    CHECK_EQ(instant.get(Field::ProlepticDay), 10);
    CHECK_EQ(instant.get(Field::EpochDay), 10 - 1704152);
    CHECK_EQ(instant.get(Field::HourOfDay), 1);
    CHECK_EQ(instant.get(Field::MinuteOfHour), 1);
    CHECK_EQ(instant.get(Field::SecondOfMinute), 1);
    CHECK_EQ(instant.get(Field::NanosecondOfSecond), 123456789);

    CHECK_EQ(instant.plus(1, Unit::Hours), Instant(10, 7261123456789LL));
    CHECK_EQ(instant.plus(1, Unit::Days), Instant(11, 3661123456789LL));
    CHECK_EQ(instant.plus(-2, Unit::Seconds), Instant(10, 3659123456789LL));
    CHECK_EQ(instant.plus(-1, Unit::Weeks), Instant(3, 3661123456789LL));

    CHECK_EQ(instant.until(Instant(11, 3661123456789LL), Unit::Hours), 24);
    CHECK_EQ(instant.until(Instant(11, 3661123456789LL), Unit::Days), 1);
    CHECK_EQ(instant.until(Instant(10, 3661123456789LL + 1000), Unit::Microseconds), 1);
    CHECK_EQ(instant.until(Instant(10, 3661123456789LL), Unit::Nanoseconds), 0);
    CHECK_THROWS_AS(instant.plus(1, Unit::Months), std::invalid_argument);
    CHECK_THROWS_AS(instant.until(instant, Unit::Years), std::invalid_argument);
}

TEST_CASE("temporal instant signed fields and normalisation")
{
    const Instant zero;
    CHECK_EQ(zero.toProleptic(), 0);
    CHECK_EQ(zero.getNanosecondsOfDay(), 0);

    const Instant time(-3, 3661123456789LL);
    CHECK_EQ(time.toProleptic(), -3);
    CHECK_EQ(time.getNanosecondsOfDay(), 3661123456789LL);
    CHECK_EQ(time.getHour(), 1);
    CHECK_EQ(time.getMinute(), 1);
    CHECK_EQ(time.getSecond(), 1);
    CHECK_EQ(time.getNanosecond(), 123456789);

    CHECK_EQ(Instant(0, -1), Instant(-1, NanosPerDay - 1));
    CHECK_EQ(Instant(0, NanosPerDay), Instant(1, 0));
    CHECK_EQ(Instant(5, -NanosPerDay - 1), Instant(3, NanosPerDay - 1));
    CHECK_EQ(Instant(-1, NanosPerDay - 1).getNanosecond(), 999999999);
    CHECK_THROWS_AS(Instant(std::numeric_limits<Timestamp>::max(), NanosPerDay), std::overflow_error);
}

TEST_CASE("temporal instant comparison and arithmetic")
{
    CHECK(Instant(-1, 0) < Instant(0, 0));
    CHECK(Instant(-2, 0) < Instant(-1, 0));
    CHECK(Instant(-1, 5) < Instant(-1, 6));
    CHECK(Instant(0, 1) < Instant(1, 0));
    CHECK(Instant(1, 0) > Instant(0, NanosPerDay - 1));
    CHECK(Instant(1, 5) == Instant(1, 5));
    CHECK(Instant(1, 5) != Instant(1, 6));

    CHECK_EQ(Instant(100, 500).plusNanoseconds(NanosPerDay - 500), Instant(101, 0));
    CHECK_EQ(Instant(100, 500).minusNanoseconds(501), Instant(99, NanosPerDay - 1));
    CHECK_EQ(Instant(100, 500).plusDays(2), Instant(102, 500));
    CHECK_EQ(Instant(100, 500).plusDays(-2), Instant(98, 500));
    CHECK_EQ(Instant(1, 0).plusNanoseconds(-1), Instant(0, NanosPerDay - 1));
    CHECK_EQ(Instant(-1, 0).plusNanoseconds(-1), Instant(-2, NanosPerDay - 1));
    CHECK_EQ(Instant(2, 0).differenceInNanoseconds(Instant(1, 0)), 86400000000000LL);
    CHECK_EQ(Instant(1, 0).differenceInNanoseconds(Instant(2, 0)), -86400000000000LL);
    CHECK_EQ(Instant(1, 500).differenceInNanoseconds(Instant(1, 400)), 100LL);
    CHECK_THROWS_AS(Instant(0, 0).minusNanoseconds(std::numeric_limits<Timestamp>::min()), std::overflow_error);
}
TEST_CASE("temporal field vocabulary predicates")
{
    CHECK(isDateField(Field::Year));
    CHECK(isDateField(Field::IsLeapMonth));
    CHECK_FALSE(isDateField(Field::HourOfDay));
    CHECK(isDayField(Field::EpochDay));
    CHECK(isDayField(Field::ProlepticDay));
    CHECK_FALSE(isDayField(Field::Year));
    CHECK(isTimeField(Field::HourOfDay));
    CHECK_FALSE(isTimeField(Field::EpochDay));
    CHECK_FALSE(isTimeField(Field::Year));
    CHECK(Instant(0, 0).isSupported(Field::EpochDay));
    CHECK_FALSE(Instant(0, 0).isSupported(Field::Year));
}

TEST_CASE("temporal duration normalisation and arithmetic")
{
    CHECK_EQ(Duration(0, -1), Duration(-1, 999999999));
    CHECK_EQ(Duration(1, 1000000000), Duration(2, 0));
    CHECK_EQ(Duration(0, 1500000000), Duration(1, 500000000));
    CHECK_EQ(Duration(0, 1500000000).getSeconds(), 1);
    CHECK_EQ(Duration(0, 1500000000).getNanoseconds(), 500000000);
    CHECK_EQ(Duration::ofNanoseconds(-1), Duration(-1, 999999999));
    CHECK_EQ(Duration(2, 0).plus(Duration(-1, 500000000)), Duration(1, 500000000));
    CHECK_EQ(Duration(2, 0).minus(Duration(1, 0)), Duration(1, 0));
    CHECK_EQ(Duration(0, 500000000).negated(), Duration(-1, 500000000));
    CHECK_EQ(Duration::of(2, Unit::Hours), Duration(7200, 0));
    CHECK_EQ(Duration::of(1, Unit::Days), Duration(86400, 0));
    CHECK_EQ(Duration(1, 0).toNanoseconds(), 1000000000LL);
    CHECK(Duration(0, 1) < Duration(1, 0));
    CHECK(Duration(-1, 999999999) < Duration(0, 0));
    CHECK_THROWS_AS(Duration::of(1, Unit::Months), std::invalid_argument);
    CHECK_THROWS_AS(Duration(std::numeric_limits<Timestamp>::max(), 1000000000), std::overflow_error);
    CHECK_THROWS_AS(Duration(std::numeric_limits<Timestamp>::min(), -1), std::overflow_error);
    CHECK_THROWS_AS(Duration(std::numeric_limits<Timestamp>::max(), 0).toNanoseconds(), std::overflow_error);
}

TEST_CASE("temporal instant duration arithmetic")
{
    const Instant instant(10, 500);
    CHECK_EQ(instant + Duration::of(1, Unit::Hours), Instant(10, 3600000000500LL));
    CHECK_EQ(instant + Duration::of(1, Unit::Days), Instant(11, 500));
    CHECK_EQ(instant - Duration::of(1, Unit::Days), Instant(9, 500));
    CHECK_EQ(Instant(11, 500) - instant, Duration::of(1, Unit::Days));
    CHECK_EQ(instant - instant, Duration());
    CHECK_EQ(Instant(-1, 0) + Duration::of(1, Unit::Days), Instant(0, 0));
    CHECK_EQ(Instant(0, 0) - Duration::of(1, Unit::Days), Instant(-1, 0));
    CHECK_EQ(Instant(0, 0) - Duration::of(1, Unit::Nanoseconds), Instant(-1, NanosPerDay - 1));
}

TEST_CASE("temporal time of day")
{
    const Time time = Time::of(1, 2, 3, 4);
    CHECK_EQ(time.getNanoseconds(), 3723000000004LL);
    CHECK_EQ(time.get(Field::HourOfDay), 1);
    CHECK_EQ(time.get(Field::MinuteOfHour), 2);
    CHECK_EQ(time.get(Field::SecondOfMinute), 3);
    CHECK_EQ(time.get(Field::NanosecondOfSecond), 4);
    CHECK(time.isSupported(Field::HourOfDay));
    CHECK_FALSE(time.isSupported(Field::Year));
    CHECK_EQ(Time::parse("01:02:03.000000004"), time);
    CHECK_EQ(Time::parse("23:59"), Time::of(23, 59));
    CHECK_EQ(Time::of(23, 0).plus(Duration::of(2, Unit::Hours)), Time::of(1, 0));
    CHECK_EQ(Time::of(1, 0).minus(Duration::of(2, Unit::Hours)), Time::of(23, 0));
    CHECK_EQ(Time::of(1, 0).until(Time::of(23, 0)), Duration::of(22, Unit::Hours));
    CHECK_EQ(Time::of(23, 0).until(Time::of(1, 0)), Duration::of(-22, Unit::Hours));
    CHECK_EQ(Time::of(1, 2, 3).to_string(), "01:02:03");
    CHECK_EQ(Time::of(1, 2, 3, 400000000).to_string(), "01:02:03.4");
    CHECK_EQ(Time::midnight().to_string(), "00:00:00");
    CHECK_EQ(Time::parse("00:00:00.000000001"), Time::of(0, 0, 0, 1));
    CHECK_THROWS_AS(Time::of(24, 0), std::invalid_argument);
    CHECK_THROWS_AS(Time::ofNanoseconds(NanosPerDay), std::invalid_argument);

    const char *invalid[] = {"", "24:00", "01:60", "1:02:03", "01:02:03.", "01:02:03.1234567890", "01:02:03Z"};
    for (const char *text : invalid)
    {
        CHECK_FALSE(Time::tryParse(text).has_value());
    }
}

TEST_CASE("temporal date-time composition")
{
    const DateTime value = DateTime::of(Year(2024), Month::February, 29, 23, 30, 0, 0);
    CHECK_EQ(value.to_string(), "2024-02-29T23:30:00");
    CHECK_EQ(value.getDate(), makeDate(2024, Month::February, 29));
    CHECK_EQ(value.getTime(), Time::of(23, 30));
    CHECK_EQ(value.get(Field::DayOfMonth), 29);
    CHECK_EQ(value.get(Field::HourOfDay), 23);
    CHECK(value.isSupported(Field::Year));
    CHECK(value.isSupported(Field::NanosecondOfSecond));
    CHECK_EQ(value.plus(Duration::of(1, Unit::Hours)), DateTime::of(Year(2024), Month::March, 1, 0, 30));
    CHECK_EQ(value.plus(Period(1, 0, 0)), DateTime::of(Year(2025), Month::February, 28, 23, 30));
    CHECK_EQ(value.durationUntil(value.plus(Duration::of(90, Unit::Minutes))), Duration::of(90, Unit::Minutes));
    CHECK_EQ(DateTime::parse("2024-02-29T23:30:00.5"),
             DateTime::of(Year(2024), Month::February, 29, 23, 30, 0, 500000000));
    CHECK_EQ(DateTime::parse(value.to_string()), value);
    CHECK(value < value.plus(Duration::of(1, Unit::Nanoseconds)));

    const char *invalid[] = {"2024-02-29 23:30:00", "2024-02-30T00:00:00", "2024-02-29T24:00:00", "2024-02-29"};
    for (const char *text : invalid)
    {
        CHECK_FALSE(DateTime::tryParse(text).has_value());
    }
}

TEST_CASE("temporal overflow policy")
{
    const Date endOfMonth = makeDate(2024, Month::January, 31);
    CHECK_EQ(endOfMonth.plus(1, Unit::Months, Overflow::Constrain), makeDate(2024, Month::February, 29));
    CHECK_THROWS_AS(endOfMonth.plus(1, Unit::Months, Overflow::Reject), std::out_of_range);
    CHECK_THROWS_AS(endOfMonth.plus(Period(0, 1, 0), Overflow::Reject), std::out_of_range);
    CHECK_EQ(makeDate(2024, Month::February, 29).plus(1, Unit::Years, Overflow::Constrain),
             makeDate(2025, Month::February, 28));
    CHECK_THROWS_AS(makeDate(2024, Month::February, 29).plus(1, Unit::Years, Overflow::Reject), std::out_of_range);
    CHECK_EQ(makeDate(2024, Month::January, 15).plus(1, Unit::Months, Overflow::Reject),
             makeDate(2024, Month::February, 15));
    CHECK_EQ(makeDate(2024, Month::January, 31).minus(Period(0, 1, 0), Overflow::Constrain),
             makeDate(2023, Month::December, 31));
}
TEST_CASE("temporal date-time unit arithmetic")
{
    const DateTime value = DateTime::of(Year(2024), Month::January, 31, 23, 30);
    CHECK_EQ(value.plus(1, Unit::Months), DateTime::of(Year(2024), Month::February, 29, 23, 30));
    CHECK_EQ(value.plus(2, Unit::Hours), DateTime::of(Year(2024), Month::February, 1, 1, 30));
    CHECK_EQ(value.plus(1, Unit::Days), DateTime::of(Year(2024), Month::February, 1, 23, 30));
    CHECK_EQ(value.minus(1, Unit::Months), DateTime::of(Year(2023), Month::December, 31, 23, 30));
}
TEST_CASE("temporal utc offset")
{
    CHECK_EQ(Offset::parse("Z"), Offset());
    CHECK_EQ(Offset::parse("+08:00"), Offset::ofHours(8));
    CHECK_EQ(Offset::parse("-05:30"), Offset::ofSeconds(-(5 * 3600 + 30 * 60)));
    CHECK_EQ(Offset::ofHoursMinutes(-5, 30), Offset::ofSeconds(-19800));
    CHECK_EQ(Offset::ofHours(8).to_string(), "+08:00");
    CHECK_EQ(Offset::ofSeconds(-19800).to_string(), "-05:30");
    CHECK_EQ(Offset().to_string(), "Z");
    CHECK_EQ(Offset::parse("+00:00"), Offset());
    CHECK_THROWS_AS(Offset::ofSeconds(18 * 3600 + 1), std::invalid_argument);
    CHECK_FALSE(Offset::tryParse("+18:01").has_value());
    CHECK_FALSE(Offset::tryParse("+8:00").has_value());
    CHECK_FALSE(Offset::tryParse("08:00").has_value());
    CHECK_FALSE(Offset::tryParse("+19:00").has_value());
}

TEST_CASE("temporal offset date-time")
{
    const OffsetDateTime value = OffsetDateTime::parse("2024-01-01T00:00:00+08:00");
    CHECK_EQ(value.to_string(), "2024-01-01T00:00:00+08:00");
    CHECK_EQ(value.get(Field::Year), 2024);
    CHECK_EQ(value.get(Field::HourOfDay), 0);
    CHECK_EQ(value.getOffset(), Offset::ofHours(8));
    CHECK_EQ(value.toUnixSeconds(), 1704038400LL);

    const OffsetDateTime utc = OffsetDateTime::parse("2023-12-31T16:00:00Z");
    CHECK_EQ(value, utc);
    CHECK_EQ(value.toUnixSeconds(), utc.toUnixSeconds());
    CHECK(value <= utc);
    CHECK(value >= utc);

    CHECK_EQ(OffsetDateTime::parse("1970-01-01T08:00:00+08:00").toUnixSeconds(), 0);
    CHECK_EQ(OffsetDateTime::fromUnixSeconds(0, Offset::ofHours(8)).to_string(), "1970-01-01T08:00:00+08:00");

    const OffsetDateTime converted = value.withOffsetSameInstant(Offset());
    CHECK_EQ(converted.to_string(), "2023-12-31T16:00:00Z");
    CHECK_EQ(converted, value);

    const OffsetDateTime later = value.plus(Duration::of(1, Unit::Days));
    CHECK_EQ(later.to_string(), "2024-01-02T00:00:00+08:00");
    CHECK_EQ(value.plus(1, Unit::Months).to_string(), "2024-02-01T00:00:00+08:00");
    CHECK(value < later);

    const char *invalid[] = {
        "2024-01-01T00:00:00",       // no offset
        "2024-01-01T00:00:00+08",    // incomplete offset
        "2024-01-01T00:00:00+18:01", // offset out of range
        "2024-02-30T00:00:00Z",      // invalid date
        "2024-01-01 00:00:00Z",      // missing T
    };
    for (const char *text : invalid)
    {
        CHECK_FALSE(OffsetDateTime::tryParse(text).has_value());
    }
}
TEST_CASE("temporal zone rules and local results")
{
    const Timestamp spring = OffsetDateTime::parse("2024-03-31T01:00:00Z").toUnixSeconds();
    const Zone zone = Zone::of("Test/Spring", Offset::ofHours(1), {{spring, Offset::ofHours(2)}});
    CHECK_EQ(zone.getIdentifier(), std::string("Test/Spring"));
    CHECK_FALSE(zone.isFixed());
    CHECK_EQ(zone.getOffsetAt(spring - 1), Offset::ofHours(1));
    CHECK_EQ(zone.getOffsetAt(spring), Offset::ofHours(2));
    CHECK_EQ(Zone::utc().getOffsetAt(-100000), Offset());
    CHECK(Zone::utc().isFixed());

    const Timestamp localGap = OffsetDateTime::parse("2024-03-31T02:30:00Z").toUnixSeconds();
    const ZoneLocalResult gap = zone.getLocalResult(localGap);
    CHECK_EQ(gap.kind, LocalResultKind::Gap);
    CHECK_EQ(gap.first, Offset::ofHours(1));
    CHECK_EQ(gap.second, Offset::ofHours(2));

    const Timestamp localSingle = OffsetDateTime::parse("2024-03-31T01:30:00Z").toUnixSeconds();
    const ZoneLocalResult single = zone.getLocalResult(localSingle);
    CHECK_EQ(single.kind, LocalResultKind::Single);
    CHECK_EQ(single.first, Offset::ofHours(1));

    const Timestamp fall = OffsetDateTime::parse("2024-10-27T01:00:00Z").toUnixSeconds();
    const Zone overlapZone = Zone::of("Test/Fall", Offset::ofHours(2), {{fall, Offset::ofHours(1)}});
    const Timestamp localOverlap = OffsetDateTime::parse("2024-10-27T02:30:00Z").toUnixSeconds();
    const ZoneLocalResult overlap = overlapZone.getLocalResult(localOverlap);
    CHECK_EQ(overlap.kind, LocalResultKind::Overlap);
    CHECK_EQ(overlap.first, Offset::ofHours(2));
    CHECK_EQ(overlap.second, Offset::ofHours(1));
}

TEST_CASE("temporal zone temporal gap and overlap")
{
    const Timestamp spring = OffsetDateTime::parse("2024-03-31T01:00:00Z").toUnixSeconds();
    const Zone springZone = Zone::of("Test/Spring", Offset::ofHours(1), {{spring, Offset::ofHours(2)}});
    const DateTime localGap = DateTime::of(Year(2024), Month::March, 31, 2, 30);

    CHECK_FALSE(ZoneTemporal::tryOf(localGap, springZone, Disambiguation::Reject).has_value());
    CHECK_THROWS_AS(ZoneTemporal::of(localGap, springZone, Disambiguation::Reject), std::invalid_argument);

    const ZoneTemporal compatible = ZoneTemporal::of(localGap, springZone);
    CHECK_EQ(compatible.to_string(), "2024-03-31T03:30:00+02:00[Test/Spring]");
    CHECK_EQ(compatible.getHour(), 3);
    CHECK_EQ(compatible.getOffset(), Offset::ofHours(2));

    const ZoneTemporal earlier = ZoneTemporal::of(localGap, springZone, Disambiguation::Earlier);
    CHECK_EQ(earlier.to_string(), "2024-03-31T01:30:00+01:00[Test/Spring]");
    const ZoneTemporal later = ZoneTemporal::of(localGap, springZone, Disambiguation::Later);
    CHECK_EQ(later, compatible);

    const Timestamp fall = OffsetDateTime::parse("2024-10-27T01:00:00Z").toUnixSeconds();
    const Zone fallZone = Zone::of("Test/Fall", Offset::ofHours(2), {{fall, Offset::ofHours(1)}});
    const DateTime localOverlap = DateTime::of(Year(2024), Month::October, 27, 2, 30);
    CHECK_FALSE(ZoneTemporal::tryOf(localOverlap, fallZone, Disambiguation::Reject).has_value());

    const ZoneTemporal first = ZoneTemporal::of(localOverlap, fallZone);
    const ZoneTemporal second = ZoneTemporal::of(localOverlap, fallZone, Disambiguation::Later);
    CHECK_EQ(first.to_string(), "2024-10-27T02:30:00+02:00[Test/Fall]");
    CHECK_EQ(second.to_string(), "2024-10-27T02:30:00+01:00[Test/Fall]");
    CHECK(first < second);
    CHECK_EQ(second.toUnixSeconds() - first.toUnixSeconds(), 3600);
}

TEST_CASE("temporal zone temporal instant arithmetic")
{
    const Timestamp spring = OffsetDateTime::parse("2024-03-31T01:00:00Z").toUnixSeconds();
    const Zone zone = Zone::of("Test/Spring", Offset::ofHours(1), {{spring, Offset::ofHours(2)}});
    const ZoneTemporal value = ZoneTemporal::of(DateTime::of(Year(2024), Month::March, 31, 1, 30), zone);
    CHECK_EQ(value.to_string(), "2024-03-31T01:30:00+01:00[Test/Spring]");

    const ZoneTemporal after = value.plus(Duration::of(1, Unit::Hours));
    CHECK_EQ(after.to_string(), "2024-03-31T03:30:00+02:00[Test/Spring]");
    CHECK_EQ(after.toUnixSeconds() - value.toUnixSeconds(), 3600);

    const ZoneTemporal utc = value.withZoneSameInstant(Zone::utc());
    CHECK_EQ(utc.to_string(), "2024-03-31T00:30:00Z[UTC]");
    CHECK_EQ(utc.toUnixSeconds(), value.toUnixSeconds());
    CHECK_EQ(utc.getHour(), 0);

    const ZoneTemporal sameLocal = value.withZoneSameLocal(Zone::utc());
    CHECK_EQ(sameLocal.to_string(), "2024-03-31T01:30:00Z[UTC]");
    CHECK(sameLocal != value);

    const ZoneTemporal fromInstant = ZoneTemporal::atUnixSeconds(value.toUnixSeconds(), zone);
    CHECK_EQ(fromInstant, value);
    CHECK_EQ(fromInstant.to_string(), value.to_string());

    const ZoneTemporal nextDay = value.plus(1, Unit::Days);
    CHECK_EQ(nextDay.to_string(), "2024-04-01T01:30:00+02:00[Test/Spring]");
}
TEST_CASE("temporal zone temporal unit semantics")
{
    const Timestamp fall = OffsetDateTime::parse("2024-10-27T01:00:00Z").toUnixSeconds();
    const Zone zone = Zone::of("Test/Fall", Offset::ofHours(2), {{fall, Offset::ofHours(1)}});
    const ZoneTemporal value = ZoneTemporal::of(DateTime::of(Year(2024), Month::October, 27, 2, 30), zone);
    CHECK_EQ(value.to_string(), "2024-10-27T02:30:00+02:00[Test/Fall]");

    // A time-based unit is exact: 02:30+02:00 + 1h is 02:30+01:00 (the same
    // wall clock, one instant later), not 03:30.
    const ZoneTemporal afterHour = value.plus(1, Unit::Hours);
    CHECK_EQ(afterHour.to_string(), "2024-10-27T02:30:00+01:00[Test/Fall]");
    CHECK_EQ(afterHour.toUnixSeconds() - value.toUnixSeconds(), 3600);

    // A date-based unit moves the local date, so the day can be 25 hours long.
    const ZoneTemporal nextDay = value.plus(1, Unit::Days);
    CHECK_EQ(nextDay.to_string(), "2024-10-28T02:30:00+01:00[Test/Fall]");
    CHECK_EQ(nextDay.toUnixSeconds() - value.toUnixSeconds(), 25 * 3600);
}
TEST_CASE("temporal day storage and epoch scales")
{
    const Instant instant(3, 4);
    CHECK_EQ(instant.getDay().getDays(), 3);
    CHECK_EQ(instant.getNanosecondsOfDay(), 4);
    CHECK_EQ(Instant(Day{1}, 2).getDay().getDays(), 1);
    CHECK_EQ(Instant(Day{1}, 2).getNanosecondsOfDay(), 2);
    CHECK_EQ(Instant(0, -1).getDay().getDays(), -1);
    CHECK_EQ(Instant(0, -1).getNanosecondsOfDay(), NanosPerDay - 1);

    CHECK_EQ(Instant(0, 0).toProleptic(), 0);
    CHECK_EQ(Instant(-1, 0).toProleptic(), -1);
    CHECK_EQ(Instant(1704153, 0).toEpoch(), 1);
    CHECK_EQ(Instant(0, 0).toEpoch(), -1704152);
    CHECK_EQ(Instant(1704153, 0).get(Field::EpochDay), 1);
    CHECK_EQ(Instant(0, 0).get(Field::ProlepticDay), 0);

    CHECK_EQ(makeDate(1970, Month::January, 1).toEpochDay(), 1);
    CHECK_EQ(makeDate(2024, Month::January, 1).toEpochDay(), 19724);
    CHECK_EQ(Date::fromEpochDay(1), makeDate(1970, Month::January, 1));
    CHECK_FALSE(Date::tryFromEpochDay(std::numeric_limits<Timestamp>::max()).has_value());

    CHECK_EQ(makeDate(1970, Month::January, 1).get(Field::EpochDay), 1);
    CHECK_EQ(makeDate(1970, Month::January, 1).get(Field::ProlepticDay), 1704153);
    CHECK_EQ(DateTime::of(Year(1970), Month::January, 1, 0, 0).get(Field::EpochDay), 1);
    CHECK_EQ(OffsetDateTime::parse("1970-01-01T00:00:00Z").get(Field::EpochDay), 1);
    const ZoneTemporal zoneEpoch = ZoneTemporal::of(DateTime::of(Year(1970), Month::January, 1, 0, 0), Zone::utc());
    CHECK_EQ(zoneEpoch.get(Field::EpochDay), 1);

    CHECK_EQ(Instant::fromUnixSeconds(0).toProleptic(), 1704153);
    CHECK_EQ(Instant::fromUnixSeconds(0).toEpoch(), 1);
    CHECK_EQ(Instant::fromUnixSeconds(0).toUnixSeconds(), 0);
    CHECK_EQ(Instant::fromUnixSeconds(-1).toUnixSeconds(), -1);
    CHECK_EQ(Instant::fromUnixSeconds(1704067200).toUnixSeconds(), 1704067200);
    CHECK_EQ(Instant::fromUnixNanoseconds(1).getNanosecondsOfDay(), 1);
    CHECK_EQ(Instant::fromUnixNanoseconds(1).toUnixNanoseconds(), 1);
    const Instant now = Instant::now();
    CHECK(now > Instant::fromUnixSeconds(1704067200));
    CHECK(now < Instant::fromUnixSeconds(2000000000));
}
TEST_CASE("temporal instant interop")
{
    const Instant epoch = Instant::fromUnixSeconds(0);
    CHECK_EQ(epoch.getDay().getDays(), 1704153);
    CHECK_EQ(epoch.toEpoch(), 1);
    CHECK_EQ(epoch.toProleptic(), 1704153);
    CHECK_EQ(epoch.toUnixSeconds(), 0);
    CHECK_EQ(Instant::fromUnixSeconds(1704067200).toEpoch(), 19724);
    CHECK_EQ(Instant::fromUnixSeconds(1704067200).toUnixSeconds(), 1704067200);

    const OffsetDateTime utc = OffsetDateTime::parse("2024-01-01T00:00:00Z");
    CHECK_EQ(utc.toInstant().toUnixSeconds(), utc.toUnixSeconds());
    CHECK_EQ(OffsetDateTime::fromInstant(utc.toInstant(), Offset()).to_string(), "2024-01-01T00:00:00Z");

    const ZoneTemporal zoned = ZoneTemporal::of(DateTime::of(Year(2024), Month::January, 1, 0, 0), Zone::utc());
    CHECK_EQ(zoned.toInstant().toUnixSeconds(), zoned.toUnixSeconds());
    CHECK_EQ(ZoneTemporal::fromInstant(zoned.toInstant(), Zone::utc()).to_string(), "2024-01-01T00:00:00Z[UTC]");
}

TEST_CASE("temporal date-time unit distances")
{
    const DateTime start = DateTime::of(Year(2024), Month::January, 1, 0, 0);
    const DateTime end = DateTime::of(Year(2024), Month::January, 2, 2, 0);
    CHECK_EQ(start.until(end, Unit::Hours), 26);
    CHECK_EQ(start.until(end, Unit::Days), 1);
    CHECK_EQ(start.until(end, Unit::Minutes), 26 * 60);
    CHECK_EQ(start.durationUntil(end), Duration::of(26, Unit::Hours));

    const OffsetDateTime offsetStart = OffsetDateTime::from(start, Offset());
    const OffsetDateTime offsetEnd = OffsetDateTime::from(end, Offset());
    CHECK_EQ(offsetStart.until(offsetEnd, Unit::Hours), 26);
    CHECK_EQ(offsetStart - offsetEnd, Duration::of(-26, Unit::Hours));
}
TEST_CASE("temporal day operators")
{
    const Day left(1);
    const Day right(2);
    CHECK_EQ(left + 3, Day(4));
    CHECK_EQ(3 + left, Day(4));
    CHECK_EQ(left - 3, Day(-2));
    CHECK_EQ(right - left, 1);
    CHECK_EQ(left - right, -1);
    CHECK(left < right);
    CHECK(left <= left);
    CHECK(right > left);
    CHECK(left != right);

    Day value = left;
    value += 3;
    CHECK_EQ(value, Day(4));
    value -= 3;
    CHECK_EQ(value, left);
}
TEST_CASE("temporal calendar cycles")
{
    const Instant yearStart = Instant::fromUnixSeconds(1704067200);     // 2024-01-01
    const Instant februaryStart = Instant::fromUnixSeconds(1706745600); // 2024-02-01
    const Instant marchStart = Instant::fromUnixSeconds(1709251200);    // 2024-03-01
    const Instant yearEnd = Instant::fromUnixSeconds(1735689600);       // 2025-01-01

    const temporal::Month february(24289, 2, februaryStart, marchStart);
    CHECK_EQ(february.getAbsolute(), 24289);
    CHECK_EQ(february.getRelative(), 2);
    CHECK_EQ(february.getStart(), februaryStart);
    CHECK_EQ(february.getEnd(), marchStart);
    CHECK(february.contains(februaryStart));
    CHECK_FALSE(february.contains(yearStart));
    CHECK_EQ(february.getDuration(), Duration::of(29, Unit::Days));

    const temporal::Year year(2024, 2024, yearStart, yearEnd);
    CHECK_EQ(year.getAbsolute(), 2024);
    CHECK_EQ(year.getRelative(), 2024);
    CHECK(year.contains(februaryStart));
    CHECK_EQ(year.getDuration(), Duration::of(366, Unit::Days));

    const temporal::Week week(105800, 5, februaryStart, Instant::fromUnixSeconds(1707350400));
    CHECK_EQ(week.getRelative(), 5);
    CHECK_EQ(week.getDuration(), Duration::of(7, Unit::Days));

    const temporal::Era era(1, 0, yearStart, yearEnd);
    CHECK_EQ(era.getAbsolute(), 1);
    CHECK_EQ(era.getRelative(), 0);
    CHECK_EQ(era.getDuration(), Duration::of(366, Unit::Days));

    static_assert(!std::is_same<temporal::Year, temporal::Month>::value, "cycle types are distinct");
    static_assert(!std::is_same<temporal::Month, temporal::Week>::value, "cycle types are distinct");
}
TEST_CASE("temporal local types")
{
    const temporal::LocalDate date = temporal::LocalDate::of(Year(2024), Month::January, 1);
    const temporal::LocalTime time = temporal::LocalTime::of(1, 2, 3, 4);
    const temporal::LocalDateTime dateTime(date, time);
    CHECK_EQ(dateTime.getDate(), date);
    CHECK_EQ(dateTime.getTime(), time);
    CHECK_EQ(dateTime.getYear(), Year(2024));
    CHECK_EQ(dateTime.getMonth(), Month::January);
    CHECK_EQ(dateTime.getHour(), 1);
    CHECK_EQ(dateTime.getMinute(), 2);
    CHECK_EQ(dateTime.getSecond(), 3);
    CHECK_EQ(dateTime.getNanosecond(), 4);

    const temporal::OffsetGregoryDateTime offset(dateTime, Offset());
    CHECK_EQ(offset.getDateTime(), dateTime);
    CHECK_EQ(offset.getOffset(), Offset());
    CHECK_EQ(offset.to_string(), "2024-01-01T01:02:03.000000004Z");
}
TEST_CASE("temporal cycle lengths")
{
    static_assert(std::is_same<temporal::Timestamp, std::int64_t>::value, "Timestamp is int64_t");
    static_assert(std::is_same<temporal::Module, std::uint64_t>::value, "Module is uint64_t");

    const Instant start = Instant::fromUnixSeconds(1704067200);
    const Instant end = Instant::fromUnixSeconds(1704153600);

    std::vector<temporal::Day> days;
    for (int index = 0; index < 7; ++index)
    {
        days.push_back(temporal::Day(index));
    }
    const temporal::Week week(1, 1, days, start, end);
    CHECK_EQ(week.lengthOfDay(), 7);
    CHECK_EQ(week.lengthOfWeek(), 1);

    const temporal::Month february(24289, 2, Instant::fromUnixSeconds(1706745600),
                                   Instant::fromUnixSeconds(1709251200));
    CHECK_EQ(february.lengthOfDay(), 29);
    CHECK_EQ(february.lengthOfWeek(), 4);

    std::vector<temporal::Month> months(12);
    const temporal::Year year(2024, 2024, false, months, Instant::fromUnixSeconds(1704067200),
                              Instant::fromUnixSeconds(1735689600));
    CHECK_EQ(year.lengthOfMonth(), 12);
    CHECK_EQ(year.lengthOfDay(), 366);
    CHECK_EQ(year.lengthOfWeek(), 52);

    std::vector<temporal::Year> years;
    years.push_back(year);
    years.push_back(year);
    const temporal::Era era(1, 0, years, Instant::fromUnixSeconds(1704067200), Instant::fromUnixSeconds(1735689600));
    CHECK_EQ(era.lengthOfYear(), 2);
}
TEST_CASE("temporal cycle parents")
{
    const Instant start = Instant::fromUnixSeconds(1704067200);
    const Instant end = Instant::fromUnixSeconds(1704153600);

    const temporal::Era era(1, 0, start, end);
    const temporal::Year year(2024, 2024, start, end, &era);
    const temporal::Month month(24289, 2, start, end, &year);
    const temporal::Week week(105800, 5, start, end, &month);

    CHECK_EQ(year.getEra(), &era);
    CHECK_EQ(month.getYear(), &year);
    CHECK_EQ(week.getMonth(), &month);

    temporal::Year detached(2025, 2025, start, end);
    CHECK_EQ(detached.getEra(), nullptr);
    detached.setEra(&era);
    CHECK_EQ(detached.getEra(), &era);
}
TEST_CASE("temporal concrete type names")
{
    static_assert(std::is_same<temporal::LocalDateTime, temporal::gregory::DateTime>::value,
                  "gregory::DateTime is LocalDateTime");
    static_assert(std::is_same<temporal::OffsetGregoryDateTime, temporal::gregory::OffsetDateTime>::value,
                  "gregory::OffsetDateTime is OffsetGregoryDateTime");
    static_assert(std::is_same<temporal::ZonedGregoryDateTime, temporal::gregory::ZoneTemporal>::value,
                  "gregory::ZoneTemporal is ZonedGregoryDateTime");
    static_assert(std::is_same<temporal::LocalDate, temporal::gregory::Date>::value, "gregory::Date is LocalDate");

    const temporal::LocalDateTime value = temporal::LocalDateTime::of(Year(2024), Month::January, 1, 0, 0);
    CHECK_EQ(value.getYear(), Year(2024));
}
