#include "localisation/datetime.h"

#include "test_framework.h"

#include "temporal/gregory/gregory.h"
#include "temporal/local_date.h"
#include "temporal/local_date_time.h"
#include "temporal/local_time.h"
#include "temporal/offset.h"
#include "temporal/offset_date_time.h"
#include "temporal/zone.h"
#include "temporal/zone_temporal.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

// Golden strings in this file are derived from the CLDR 49 ca-gregorian.json
// patterns and names, not from the formatter's own output. Non-ASCII text is
// written as UTF-8 byte escapes so that the expectations are exact regardless of
// the compiler's source character set: "\xE2\x80\xAF" is the narrow no-break
// space (U+202F) that CLDR puts between seconds and AM/PM in English, and
// "\xE2\x80\x8F" is the right-to-left mark (U+200F) in the Arabic numeric
// patterns. An escape is always followed by a string-literal break before an
// ASCII hex digit, because C++ would otherwise absorb the digit into \xNN.
//
// Time zone tokens (z, zzzz, V) are dropped because the library has no zone
// name data, which is why the long and full time styles equal the medium one.
// The DateTimeFormatter cases follow java.time.format.DateTimeFormatter: 'u' is
// the proleptic year, 'y' the year-of-era, 'D' the day-of-year and so on.

namespace
{
using localisation::DateStyle;
using localisation::DateTimeFormatter;
using localisation::FormatStyle;
using localisation::Locale;
using localisation::TimeStyle;

auto dateOf(int year, int month, int day) -> temporal::LocalGregoryDate
{
    return temporal::gregory::Date::of(temporal::gregory::Year(static_cast<std::int32_t>(year)),
                                       static_cast<temporal::gregory::Month>(month), day);
}

auto timeOf(int hour, int minute, int second = 0, int nanosecond = 0) -> temporal::LocalGregoryTime
{
    return temporal::LocalGregoryTime::of(hour, minute, second, nanosecond);
}

auto dateTimeOf(int year, int month, int day, int hour, int minute, int second = 0)
    -> temporal::LocalGregoryDateTime
{
    return temporal::LocalGregoryDateTime(dateOf(year, month, day), timeOf(hour, minute, second));
}

auto localizedDate(const Locale &locale, FormatStyle style) -> DateTimeFormatter
{
    return DateTimeFormatter::ofLocalizedDate(style).withLocale(locale);
}

auto localizedTime(const Locale &locale, FormatStyle style) -> DateTimeFormatter
{
    return DateTimeFormatter::ofLocalizedTime(style).withLocale(locale);
}

auto localizedDateTime(const Locale &locale, FormatStyle dateStyle, FormatStyle timeStyle) -> DateTimeFormatter
{
    return DateTimeFormatter::ofLocalizedDateTime(dateStyle, timeStyle).withLocale(locale);
}
} // namespace

TEST_CASE("datetime: english golden strings")
{
    const Locale english("en");
    const auto date = dateOf(2024, 8, 15); // Thursday
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    // en patterns: M/d/yy, MMM d, y, MMMM d, y, EEEE, MMMM d, y.
    CHECK_EQ(localisation::formatDate(date, english, DateStyle::short_), std::string("8/15/24"));
    CHECK_EQ(localisation::formatDate(date, english, DateStyle::medium), std::string("Aug 15, 2024"));
    CHECK_EQ(localisation::formatDate(date, english, DateStyle::long_), std::string("August 15, 2024"));
    CHECK_EQ(localisation::formatDate(date, english, DateStyle::full), std::string("Thursday, August 15, 2024"));

    // en patterns: h:mm a, h:mm:ss a, h:mm:ss a z, h:mm:ss a zzzz. The "a" is
    // preceded by U+202F, and the zone suffix is dropped.
    CHECK_EQ(localisation::formatTime(time, english, TimeStyle::short_), std::string("3:04\xE2\x80\xAFPM"));
    CHECK_EQ(localisation::formatTime(time, english, TimeStyle::medium), std::string("3:04:05\xE2\x80\xAFPM"));
    CHECK_EQ(localisation::formatTime(time, english, TimeStyle::long_), std::string("3:04:05\xE2\x80\xAFPM"));
    CHECK_EQ(localisation::formatTime(time, english, TimeStyle::full), std::string("3:04:05\xE2\x80\xAFPM"));

    // en date-time glue is "{1}, {0}" for every style.
    CHECK_EQ(localisation::formatDateTime(value, english), std::string("Aug 15, 2024, 3:04:05\xE2\x80\xAFPM"));
    CHECK_EQ(localisation::formatDateTime(value, english, DateStyle::full, TimeStyle::full),
             std::string("Thursday, August 15, 2024, 3:04:05\xE2\x80\xAFPM"));
}

TEST_CASE("datetime: german golden strings")
{
    const Locale german("de");
    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    // de patterns: dd.MM.yy, dd.MM.y, d. MMMM y, EEEE, d. MMMM y.
    CHECK_EQ(localisation::formatDate(date, german, DateStyle::short_), std::string("15.08.24"));
    CHECK_EQ(localisation::formatDate(date, german, DateStyle::medium), std::string("15.08.2024"));
    CHECK_EQ(localisation::formatDate(date, german, DateStyle::long_), std::string("15. August 2024"));
    CHECK_EQ(localisation::formatDate(date, german, DateStyle::full), std::string("Donnerstag, 15. August 2024"));

    // German is a 24-hour locale, so no day period appears at all.
    CHECK_EQ(localisation::formatTime(time, german, TimeStyle::short_), std::string("15:04"));
    CHECK_EQ(localisation::formatTime(time, german, TimeStyle::medium), std::string("15:04:05"));
    CHECK_EQ(localisation::formatTime(time, german, TimeStyle::long_), std::string("15:04:05"));

    CHECK_EQ(localisation::formatDateTime(value, german), std::string("15.08.2024, 15:04:05"));
    CHECK_EQ(localisation::formatDateTime(value, german, DateStyle::full, TimeStyle::short_),
             std::string("Donnerstag, 15. August 2024, 15:04"));
}

TEST_CASE("datetime: french golden strings")
{
    const Locale french("fr");
    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    // fr patterns: dd/MM/y, d MMM y, d MMMM y, EEEE d MMMM y.
    CHECK_EQ(localisation::formatDate(date, french, DateStyle::short_), std::string("15/08/2024"));
    CHECK_EQ(localisation::formatDate(date, french, DateStyle::medium), std::string("15 ao\xC3\xBBt 2024"));
    CHECK_EQ(localisation::formatDate(date, french, DateStyle::long_), std::string("15 ao\xC3\xBBt 2024"));
    CHECK_EQ(localisation::formatDate(date, french, DateStyle::full), std::string("jeudi 15 ao\xC3\xBBt 2024"));

    CHECK_EQ(localisation::formatTime(time, french, TimeStyle::medium), std::string("15:04:05"));

    // fr short glue is "{1} {0}" (space) while medium is "{1}, {0}" (comma).
    CHECK_EQ(localisation::formatDateTime(value, french, DateStyle::short_, TimeStyle::short_),
             std::string("15/08/2024 15:04"));
    CHECK_EQ(localisation::formatDateTime(value, french, DateStyle::medium, TimeStyle::medium),
             std::string("15 ao\xC3\xBBt 2024, 15:04:05"));
}

TEST_CASE("datetime: japanese golden strings")
{
    const Locale japanese("ja");
    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    // ja patterns: y/MM/dd, y/MM/dd, y年M月d日, y年M月d日EEEE.
    CHECK_EQ(localisation::formatDate(date, japanese, DateStyle::short_), std::string("2024/08/15"));
    CHECK_EQ(localisation::formatDate(date, japanese, DateStyle::medium), std::string("2024/08/15"));
    CHECK_EQ(localisation::formatDate(date, japanese, DateStyle::long_),
             std::string("2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5"));
    CHECK_EQ(localisation::formatDate(date, japanese, DateStyle::full),
             std::string("2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5\xE6\x9C\xA8\xE6\x9B\x9C\xE6\x97\xA5"));

    CHECK_EQ(localisation::formatTime(time, japanese, TimeStyle::medium), std::string("15:04:05"));
    // ja full time is "H時mm分ss秒 zzzz".
    CHECK_EQ(localisation::formatTime(time, japanese, TimeStyle::full),
             std::string("15\xE6\x99\x82" "04\xE5\x88\x86" "05\xE7\xA7\x92"));

    CHECK_EQ(localisation::formatDateTime(value, japanese), std::string("2024/08/15 15:04:05"));
    CHECK_EQ(localisation::formatDateTime(value, japanese, DateStyle::full, TimeStyle::full),
             std::string("2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5\xE6\x9C\xA8\xE6\x9B\x9C\xE6\x97\xA5 15\xE6\x99\x82"
                         "04\xE5\x88\x86" "05\xE7\xA7\x92"));
}

TEST_CASE("datetime: arabic golden strings")
{
    const Locale arabic("ar");
    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    // ar numeric patterns carry a right-to-left mark around each separator:
    // d\u200F/M\u200F/y and dd\u200F/MM\u200F/y.
    CHECK_EQ(localisation::formatDate(date, arabic, DateStyle::short_), std::string("15\xE2\x80\x8F/8\xE2\x80\x8F/2024"));
    CHECK_EQ(localisation::formatDate(date, arabic, DateStyle::medium),
             std::string("15\xE2\x80\x8F/08\xE2\x80\x8F/2024"));
    CHECK_EQ(localisation::formatDate(date, arabic, DateStyle::long_),
             std::string("15 \xD8\xA3\xD8\xBA\xD8\xB3\xD8\xB7\xD8\xB3 2024"));
    // ar full date is "EEEE\u060C d MMMM y" (Arabic comma, U+060C).
    CHECK_EQ(localisation::formatDate(date, arabic, DateStyle::full),
             std::string("\xD8\xA7\xD9\x84\xD8\xAE\xD9\x85\xD9\x8A\xD8\xB3\xD8\x8C 15 \xD8\xA3\xD8\xBA\xD8\xB3\xD8\xB7\xD8\xB3 2024"));

    // ar is a 12-hour locale with the day period after the time.
    CHECK_EQ(localisation::formatTime(time, arabic, TimeStyle::short_), std::string("3:04 \xD9\x85"));
    CHECK_EQ(localisation::formatTime(time, arabic, TimeStyle::medium), std::string("3:04:05 \xD9\x85"));

    CHECK_EQ(localisation::formatDateTime(value, arabic),
             std::string("15\xE2\x80\x8F/08\xE2\x80\x8F/2024\xD8\x8C 3:04:05 \xD9\x85"));
}

TEST_CASE("datetime: russian golden strings")
{
    const Locale russian("ru");
    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    // ru patterns: dd.MM.y, then d MMM y\u202F'г'. and its long/full variants.
    CHECK_EQ(localisation::formatDate(date, russian, DateStyle::short_), std::string("15.08.2024"));
    CHECK_EQ(localisation::formatDate(date, russian, DateStyle::medium),
             std::string("15 \xD0\xB0\xD0\xB2\xD0\xB3. 2024\xE2\x80\xAF\xD0\xB3."));
    CHECK_EQ(localisation::formatDate(date, russian, DateStyle::long_),
             std::string("15 \xD0\xB0\xD0\xB2\xD0\xB3\xD1\x83\xD1\x81\xD1\x82\xD0\xB0 2024\xE2\x80\xAF\xD0\xB3."));
    CHECK_EQ(localisation::formatDate(date, russian, DateStyle::full),
             std::string("\xD1\x87\xD0\xB5\xD1\x82\xD0\xB2\xD0\xB5\xD1\x80\xD0\xB3, 15 \xD0\xB0\xD0\xB2\xD0\xB3\xD1\x83\xD1\x81\xD1\x82\xD0\xB0 "
                         "2024\xE2\x80\xAF\xD0\xB3."));

    CHECK_EQ(localisation::formatTime(time, russian, TimeStyle::medium), std::string("15:04:05"));
    CHECK_EQ(localisation::formatDateTime(value, russian),
             std::string("15 \xD0\xB0\xD0\xB2\xD0\xB3. 2024\xE2\x80\xAF\xD0\xB3., 15:04:05"));
}

TEST_CASE("datetime: chinese golden strings")
{
    const Locale simplified("zh");
    const Locale traditional("zh-Hant");
    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    // zh patterns: y/M/d, y年M月d日 (medium and long), y年M月d日EEEE.
    CHECK_EQ(localisation::formatDate(date, simplified, DateStyle::short_), std::string("2024/8/15"));
    CHECK_EQ(localisation::formatDate(date, simplified, DateStyle::medium),
             std::string("2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5"));
    CHECK_EQ(localisation::formatDate(date, simplified, DateStyle::long_),
             std::string("2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5"));
    CHECK_EQ(localisation::formatDate(date, simplified, DateStyle::full),
             std::string("2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5\xE6\x98\x9F\xE6\x9C\x9F\xE5\x9B\x9B"));

    // zh long/full times start with a zone token ("z HH:mm:ss"), which has to
    // disappear with its separator instead of leaving a leading space.
    CHECK_EQ(localisation::formatTime(time, simplified, TimeStyle::medium), std::string("15:04:05"));
    CHECK_EQ(localisation::formatTime(time, simplified, TimeStyle::long_), std::string("15:04:05"));

    // The full date-time must not keep two spaces where the zone was.
    CHECK_EQ(localisation::formatDateTime(value, simplified),
             std::string("2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5 15:04:05"));
    CHECK_EQ(localisation::formatDateTime(value, simplified, DateStyle::full, TimeStyle::full),
             std::string("2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5\xE6\x98\x9F\xE6\x9C\x9F\xE5\x9B\x9B 15:04:05"));

    // zh-Hant differs from zh in the weekday separator and in using the
    // flexible day period (B) before the 12-hour clock.
    CHECK_EQ(localisation::formatDate(date, traditional, DateStyle::full),
             std::string("2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5 \xE6\x98\x9F\xE6\x9C\x9F\xE5\x9B\x9B"));
    CHECK_EQ(localisation::formatTime(time, traditional, TimeStyle::short_),
             std::string("\xE4\xB8\x8B\xE5\x8D\x88" "3:04"));
    CHECK_EQ(localisation::formatDateTime(value, traditional, DateStyle::full, TimeStyle::full),
             std::string("2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5 \xE6\x98\x9F\xE6\x9C\x9F\xE5\x9B\x9B "
                         "\xE4\xB8\x8B\xE5\x8D\x88" "3:04:05"));
}

TEST_CASE("datetime: hindi golden strings")
{
    const Locale hindi("hi");
    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    // hi patterns: d/M/yy, d MMM y, d MMMM y, EEEE, d MMMM y.
    CHECK_EQ(localisation::formatDate(date, hindi, DateStyle::short_), std::string("15/8/24"));
    CHECK_EQ(localisation::formatDate(date, hindi, DateStyle::medium),
             std::string("15 \xE0\xA4\x85\xE0\xA4\x97\xE0\xA5\xB0 2024"));
    CHECK_EQ(localisation::formatDate(date, hindi, DateStyle::long_),
             std::string("15 \xE0\xA4\x85\xE0\xA4\x97\xE0\xA4\xB8\xE0\xA5\x8D\xE0\xA4\xA4 2024"));
    CHECK_EQ(localisation::formatDate(date, hindi, DateStyle::full),
             std::string("\xE0\xA4\x97\xE0\xA5\x81\xE0\xA4\xB0\xE0\xA5\x81\xE0\xA4\xB5\xE0\xA4\xBE\xE0\xA4\xB0, 15 "
                         "\xE0\xA4\x85\xE0\xA4\x97\xE0\xA4\xB8\xE0\xA5\x8D\xE0\xA4\xA4 2024"));

    // hi is a 12-hour locale whose lower-case "pm" comes after the time.
    CHECK_EQ(localisation::formatTime(time, hindi, TimeStyle::short_), std::string("3:04 pm"));
    CHECK_EQ(localisation::formatTime(time, hindi, TimeStyle::medium), std::string("3:04:05 pm"));
    CHECK_EQ(localisation::formatDateTime(value, hindi),
             std::string("15 \xE0\xA4\x85\xE0\xA4\x97\xE0\xA5\xB0 2024, 3:04:05 pm"));
}

TEST_CASE("datetime: other bundled locales")
{
    // es exercises the quoted literal in "d 'de' MMMM 'de' y" and the optional
    // "(zzzz)" group of its full time style.
    const Locale spanish("es");
    CHECK_EQ(localisation::formatDate(dateOf(2024, 8, 15), spanish, DateStyle::long_),
             std::string("15 de agosto de 2024"));
    CHECK_EQ(localisation::formatDate(dateOf(2024, 8, 15), spanish, DateStyle::full),
             std::string("jueves, 15 de agosto de 2024"));
    CHECK_EQ(localisation::formatTime(timeOf(15, 4, 5), spanish, TimeStyle::full), std::string("15:04:05"));

    // it full date-time uses "{1} {0}" and both wide names.
    CHECK_EQ(localisation::formatDateTime(dateTimeOf(2024, 8, 15, 15, 4, 5), Locale("it"), DateStyle::full,
                                          TimeStyle::medium),
             std::string("gioved\xC3\xAC 15 agosto 2024 15:04:05"));

    // pt abbreviates the month in the medium pattern only.
    CHECK_EQ(localisation::formatDate(dateOf(2024, 8, 15), Locale("pt")),
             std::string("15 de ago. de 2024"));

    // tr puts the weekday after the year in the full pattern.
    CHECK_EQ(localisation::formatDate(dateOf(2024, 8, 15), Locale("tr"), DateStyle::full),
             std::string("15 A\xC4\x9Fustos 2024 Per\xC5\x9F" "embe"));

    // he prefixes the month with the preposition bet: "d בMMM y".
    CHECK_EQ(localisation::formatDate(dateOf(2024, 8, 15), Locale("he")),
             std::string("15 \xD7\x91\xD7\x90\xD7\x95\xD7\x92\xD7\xB3 2024"));

    // ko puts the day period before the 12-hour clock.
    CHECK_EQ(localisation::formatTime(timeOf(15, 4, 5), Locale("ko"), TimeStyle::short_),
             std::string("\xEC\x98\xA4\xED\x9B\x84 3:04"));
    CHECK_EQ(localisation::formatDateTime(dateTimeOf(2024, 8, 15, 15, 4, 5), Locale("ko"), DateStyle::full,
                                          TimeStyle::full),
             std::string("2024\xEB\x85\x84 8\xEC\x9B\x94 15\xEC\x9D\xBC \xEB\xAA\xA9\xEC\x9A\x94\xEC\x9D\xBC "
                         "\xEC\x98\xA4\xED\x9B\x84 3\xEC\x8B\x9C 4\xEB\xB6\x84 5\xEC\xB4\x88"));
}

TEST_CASE("datetime: calendar boundaries")
{
    // 2024-02-29 is a leap day and a Thursday.
    const auto leapDay = dateOf(2024, 2, 29);
    CHECK_EQ(localisation::formatDate(leapDay, Locale("en"), DateStyle::full),
             std::string("Thursday, February 29, 2024"));
    CHECK_EQ(localisation::formatDate(leapDay, Locale("de"), DateStyle::medium), std::string("29.02.2024"));
    CHECK_EQ(localisation::formatDate(leapDay, Locale("de"), DateStyle::long_), std::string("29. Februar 2024"));
    CHECK_EQ(localisation::formatDate(leapDay, Locale("ja"), DateStyle::long_),
             std::string("2024\xE5\xB9\xB4" "2\xE6\x9C\x88" "29\xE6\x97\xA5"));
    CHECK_EQ(localisation::formatDate(leapDay, Locale("fr")), std::string("29 f\xC3\xA9vr. 2024"));

    // 1999-12-31 is the last day of the 20th century's last year.
    const auto endOf1999 = dateOf(1999, 12, 31);
    CHECK_EQ(localisation::formatDate(endOf1999, Locale("en"), DateStyle::short_), std::string("12/31/99"));
    CHECK_EQ(localisation::formatDate(endOf1999, Locale("en"), DateStyle::medium), std::string("Dec 31, 1999"));
    CHECK_EQ(localisation::formatDate(endOf1999, Locale("de"), DateStyle::short_), std::string("31.12.99"));
    CHECK_EQ(localisation::formatDate(endOf1999, Locale("zh"), DateStyle::short_), std::string("1999/12/31"));

    // 2000-01-01 crosses the century: "yy" is "00", "y" is "2000".
    const auto startOf2000 = dateOf(2000, 1, 1);
    CHECK_EQ(localisation::formatDate(startOf2000, Locale("en"), DateStyle::short_), std::string("1/1/00"));
    CHECK_EQ(localisation::formatDate(startOf2000, Locale("de"), DateStyle::short_), std::string("01.01.00"));
    CHECK_EQ(localisation::formatDate(startOf2000, Locale("ja"), DateStyle::short_), std::string("2000/01/01"));
}

TEST_CASE("datetime: midnight, noon and the half-day clock")
{
    const Locale english("en");

    // Midnight is 12 AM on the half-day clock but 00:00 on a 24-hour clock.
    const auto midnight = timeOf(0, 0, 0);
    CHECK_EQ(localisation::formatTime(midnight, english, TimeStyle::short_), std::string("12:00\xE2\x80\xAF" "AM"));
    CHECK_EQ(localisation::formatTime(midnight, english, TimeStyle::medium),
             std::string("12:00:00\xE2\x80\xAF" "AM"));
    CHECK_EQ(localisation::formatTime(midnight, Locale("de")), std::string("00:00:00"));
    // ja medium time is "H:mm:ss", so the hour is not zero-padded.
    CHECK_EQ(localisation::formatTime(midnight, Locale("ja")), std::string("0:00:00"));
    CHECK_EQ(localisation::formatTime(midnight, Locale("zh")), std::string("00:00:00"));
    CHECK_EQ(localisation::formatTime(midnight, Locale("ko"), TimeStyle::short_),
             std::string("\xEC\x98\xA4\xEC\xA0\x84 12:00"));
    CHECK_EQ(localisation::formatTime(midnight, Locale("ar")),
             std::string("12:00:00 \xD8\xB5"));

    // Noon is 12 PM, not 0 PM.
    const auto noon = timeOf(12, 0, 0);
    CHECK_EQ(localisation::formatTime(noon, english, TimeStyle::medium),
             std::string("12:00:00\xE2\x80\xAFPM"));
    CHECK_EQ(localisation::formatTime(noon, Locale("de")), std::string("12:00:00"));
    CHECK_EQ(localisation::formatTime(noon, Locale("ko"), TimeStyle::short_),
             std::string("\xEC\x98\xA4\xED\x9B\x84 12:00"));

    // Morning and evening half-day boundaries.
    CHECK_EQ(localisation::formatTime(timeOf(9, 5, 0), english, TimeStyle::short_),
             std::string("9:05\xE2\x80\xAF" "AM"));
    CHECK_EQ(localisation::formatTime(timeOf(13, 0, 0), english, TimeStyle::short_),
             std::string("1:00\xE2\x80\xAFPM"));
    CHECK_EQ(localisation::formatTime(timeOf(23, 59, 0), english, TimeStyle::short_),
             std::string("11:59\xE2\x80\xAFPM"));
}

TEST_CASE("datetime: date pattern letters")
{
    const Locale english("en");
    const Locale german("de");
    const Locale japanese("ja");
    const auto date = dateOf(2024, 8, 15);

    CHECK_EQ(localisation::formatDatePattern("y yy yyyy", date, english), std::string("2024 24 2024"));
    CHECK_EQ(localisation::formatDatePattern("y yy yyyy", dateOf(1999, 12, 31), english),
             std::string("1999 99 1999"));
    CHECK_EQ(localisation::formatDatePattern("y yy yyyy", dateOf(2005, 1, 2), english),
             std::string("2005 05 2005"));
    CHECK_EQ(localisation::formatDatePattern("y yy yyyy", dateOf(5, 1, 2), english), std::string("5 05 0005"));

    CHECK_EQ(localisation::formatDatePattern("M MM MMM MMMM MMMMM", date, english),
             std::string("8 08 Aug August A"));
    CHECK_EQ(localisation::formatDatePattern("M MM MMM MMMM MMMMM", dateOf(2024, 3, 5), german),
             std::string("3 03 M\xC3\xA4rz M\xC3\xA4rz M"));
    // Japanese narrow month names are numbers.
    CHECK_EQ(localisation::formatDatePattern("MMMMM", date, japanese), std::string("8"));

    CHECK_EQ(localisation::formatDatePattern("d dd", dateOf(2024, 8, 5), english), std::string("5 05"));
    CHECK_EQ(localisation::formatDatePattern("d dd", date, english), std::string("15 15"));

    CHECK_EQ(localisation::formatDatePattern("E EEEE EEEEE EEEEEE", date, english),
             std::string("Thu Thursday T Th"));
    CHECK_EQ(localisation::formatDatePattern("EEEE", date, japanese),
             std::string("\xE6\x9C\xA8\xE6\x9B\x9C\xE6\x97\xA5"));

    CHECK_EQ(localisation::formatDatePattern("G GG GGGG GGGGG", date, english),
             std::string("AD AD Anno Domini A"));
    // Year 0 is 1 BCE, so the year before it is 2 BCE.
    CHECK_EQ(localisation::formatDatePattern("G GGGG GGGGG", dateOf(-5, 1, 1), english),
             std::string("BC Before Christ B"));

    CHECK_EQ(localisation::formatDatePattern("D DD DDD", dateOf(2024, 2, 29), english),
             std::string("60 60 060"));
    CHECK_EQ(localisation::formatDatePattern("D", dateOf(2024, 12, 31), english), std::string("366"));
}

TEST_CASE("datetime: time pattern letters")
{
    const Locale english("en");
    const Locale german("de");

    CHECK_EQ(localisation::formatTimePattern("H HH h hh K kk m mm s ss", timeOf(15, 4, 5), english),
             std::string("15 15 3 03 3 15 4 04 5 05"));
    // 00:00:00 is hour 0, 12 AM, 0 K-hours and 24 k-hours.
    CHECK_EQ(localisation::formatTimePattern("H HH h hh K kk", timeOf(0, 0, 0), english),
             std::string("0 00 12 12 0 24"));
    // 12:00:00 is hour 12, 12 AM/PM boundary and 0 K-hours.
    CHECK_EQ(localisation::formatTimePattern("h hh K kk", timeOf(12, 0, 0), english),
             std::string("12 12 0 12"));
    CHECK_EQ(localisation::formatTimePattern("h hh", timeOf(9, 5, 0), english), std::string("9 09"));

    CHECK_EQ(localisation::formatTimePattern("a aaaa aaaaa", timeOf(15, 4, 5), english), std::string("PM PM p"));
    CHECK_EQ(localisation::formatTimePattern("a aaaa aaaaa", timeOf(9, 5, 0), english), std::string("AM AM a"));
    // German has no lower-case am/pm distinction, so every width is "PM".
    CHECK_EQ(localisation::formatTimePattern("a aaaa aaaaa", timeOf(15, 4, 5), german), std::string("PM PM PM"));

    // "b" uses the fixed midnight and noon names.
    CHECK_EQ(localisation::formatTimePattern("b bbbb", timeOf(0, 0, 0), english),
             std::string("midnight midnight"));
    CHECK_EQ(localisation::formatTimePattern("b bbbb", timeOf(12, 0, 0), english), std::string("noon noon"));
    CHECK_EQ(localisation::formatTimePattern("b", timeOf(9, 0, 0), english), std::string("AM"));

    CHECK_EQ(localisation::formatTimePattern("S SS SSS SSSSSSSSS", timeOf(0, 0, 0, 123456789), english),
             std::string("1 12 123 123456789"));
    CHECK_EQ(localisation::formatTimePattern("S SS SSS", timeOf(0, 0, 0, 5000000), english),
             std::string("0 00 005"));
    CHECK_EQ(localisation::formatTimePattern("S SSSSSSSSS", timeOf(0, 0, 0, 0), english),
             std::string("0 000000000"));
}

TEST_CASE("datetime: quoting and optional groups")
{
    const Locale english("en");
    const Locale spanish("es");
    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 0);

    // Quoted literals are copied verbatim; '' is a literal apostrophe (the CLDR
    // "o''clock" example).
    CHECK_EQ(localisation::formatDatePattern("'Year' y", date, english), std::string("Year 2024"));
    CHECK_EQ(localisation::formatDatePattern("y 'o''clock'", date, english), std::string("2024 o'clock"));
    // A quoted "z" is literal text, not a time zone token.
    CHECK_EQ(localisation::formatDatePattern("'z' y", date, english), std::string("z 2024"));
    CHECK_EQ(localisation::formatDatePattern("d 'de' MMMM 'de' y", date, spanish),
             std::string("15 de agosto de 2024"));

    // An optional group whose content renders empty disappears with its
    // delimiters; a non-empty group keeps its content.
    CHECK_EQ(localisation::formatTimePattern("HH:mm[ z]", time, english), std::string("15:04"));
    CHECK_EQ(localisation::formatTimePattern("HH:mm (z)", time, english), std::string("15:04"));
    CHECK_EQ(localisation::formatTimePattern("z HH:mm", time, english), std::string("15:04"));
    CHECK_EQ(localisation::formatTimePattern("HH:mm[ 'UTC']", time, english), std::string("15:04 UTC"));
    CHECK_EQ(localisation::formatTimePattern("HH:mm:ss zzzz", time, english), std::string("15:04:00"));

    // Literal text around a dropped element keeps its own spacing; only the
    // separator next to the dropped element is removed.
    CHECK_EQ(localisation::formatTimePattern("  HH:mm  ", time, english), std::string("  15:04  "));
    CHECK_EQ(localisation::formatTimePattern("[z] HH:mm", time, english), std::string("15:04"));
    CHECK_EQ(localisation::formatTimePattern("HH:mm[ z]' UTC'", time, english), std::string("15:04 UTC"));
}

TEST_CASE("datetime: locale fallback chain")
{
    const auto date = dateOf(2024, 8, 15);

    // No de-AT/de-CH rows: both fall back to "de".
    CHECK_EQ(localisation::formatDate(date, Locale("de-AT")), std::string("15.08.2024"));
    CHECK_EQ(localisation::formatDate(date, Locale("de-CH")), std::string("15.08.2024"));
    // No en-US/en-GB rows: both fall back to "en".
    CHECK_EQ(localisation::formatDate(date, Locale("en-US")), std::string("Aug 15, 2024"));
    CHECK_EQ(localisation::formatDate(date, Locale("en-GB")), std::string("Aug 15, 2024"));
    // fr-CA falls back to "fr".
    CHECK_EQ(localisation::formatDate(date, Locale("fr-CA")), std::string("15 ao\xC3\xBBt 2024"));

    // zh-TW canonicalises to zh-Hant-TW and resolves through language-script to
    // the zh-Hant row, while zh-CN (zh-Hans-CN) resolves to plain zh.
    CHECK_EQ(localisation::formatDate(date, Locale("zh-TW"), DateStyle::full),
             std::string("2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5 \xE6\x98\x9F\xE6\x9C\x9F\xE5\x9B\x9B"));
    CHECK_EQ(localisation::formatDate(date, Locale("zh-CN"), DateStyle::full),
             std::string("2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5\xE6\x98\x9F\xE6\x9C\x9F\xE5\x9B\x9B"));
    CHECK_EQ(localisation::formatDate(date, Locale("zh-Hant-TW")), localisation::formatDate(date, Locale("zh-Hant")));

    // "und" and unknown tags fall back to English.
    CHECK_EQ(localisation::formatDate(date, Locale("und")), std::string("Aug 15, 2024"));
    CHECK_EQ(localisation::formatDate(date, Locale("xx-YY")), std::string("Aug 15, 2024"));

    // Locale canonicalisation is case-insensitive.
    CHECK_EQ(localisation::formatDate(date, Locale("ZH-hant")),
             localisation::formatDate(date, Locale("zh-Hant")));
}

TEST_CASE("datetime: field overloads and validation")
{
    const Locale german("de");
    CHECK_EQ(localisation::formatDate(2024, 8, 15, german), std::string("15.08.2024"));
    CHECK_EQ(localisation::formatDate(2024, 8, 15, german), localisation::formatDate(dateOf(2024, 8, 15), german));
    CHECK_EQ(localisation::formatTime(15, 4, 5, Locale("ja")), std::string("15:04:05"));
    CHECK_EQ(localisation::formatDateTime(2024, 8, 15, 15, 4, 5, Locale("en")),
             std::string("Aug 15, 2024, 3:04:05\xE2\x80\xAFPM"));

    CHECK_THROWS_AS(localisation::formatDate(2024, 13, 1, german), std::invalid_argument);
    CHECK_THROWS_AS(localisation::formatDate(2023, 2, 29, german), std::invalid_argument);
    CHECK_THROWS_AS(localisation::formatTime(24, 0, 0, german), std::invalid_argument);
    CHECK_THROWS_AS(localisation::formatTime(0, 60, 0, german), std::invalid_argument);
    CHECK_THROWS_AS(localisation::formatDateTime(2024, 2, 30, 0, 0, 0, german), std::invalid_argument);
}

TEST_CASE("datetime: default styles and composition")
{
    const Locale english("en");
    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    CHECK_EQ(localisation::formatDate(date, english),
             localisation::formatDate(date, english, DateStyle::medium));
    CHECK_EQ(localisation::formatTime(time, english),
             localisation::formatTime(time, english, TimeStyle::medium));
    CHECK_EQ(localisation::formatDateTime(value, english),
             localisation::formatDateTime(value, english, DateStyle::medium, TimeStyle::medium));
    CHECK_EQ(localisation::formatDateTime(value, english, DateStyle::full),
             localisation::formatDateTime(value, english, DateStyle::full, TimeStyle::medium));

    // For the English glue "{1}, {0}" the composed result is exactly the date,
    // a comma, a space and the time.
    CHECK_EQ(localisation::formatDateTime(value, english),
             localisation::formatDate(value.getDate(), english) + ", " + localisation::formatTime(value.getTime(), english));

    // Style pairs are independent: the glue row follows the date style.
    CHECK_EQ(localisation::formatDateTime(value, english, DateStyle::short_, TimeStyle::full),
             std::string("8/15/24, 3:04:05\xE2\x80\xAFPM"));
    CHECK_EQ(localisation::formatDateTime(value, english, DateStyle::full, TimeStyle::short_),
             std::string("Thursday, August 15, 2024, 3:04\xE2\x80\xAFPM"));
}

TEST_CASE("datetime: every bundled locale has names for every pattern width")
{
    // A generated table with a missing name would silently render an empty
    // field, so probe every name width through the public pattern API.
    const char *tags[] = {"en", "de", "fr", "es", "it", "pt", "ru", "tr", "ar", "he", "hi", "ja", "ko", "zh", "zh-Hant"};
    const auto date = dateOf(2024, 8, 15);
    const auto afternoon = timeOf(15, 4, 5);

    for (const char *tag : tags)
    {
        const Locale locale(tag);
        CHECK(!localisation::formatDatePattern("MMM", date, locale).empty());
        CHECK(!localisation::formatDatePattern("MMMM", date, locale).empty());
        CHECK(!localisation::formatDatePattern("MMMMM", date, locale).empty());
        CHECK(!localisation::formatDatePattern("QQQ", date, locale).empty());
        CHECK(!localisation::formatDatePattern("QQQQ", date, locale).empty());
        CHECK(!localisation::formatDatePattern("QQQQQ", date, locale).empty());
        CHECK(!localisation::formatDatePattern("E", date, locale).empty());
        CHECK(!localisation::formatDatePattern("EEEE", date, locale).empty());
        CHECK(!localisation::formatDatePattern("EEEEE", date, locale).empty());
        CHECK(!localisation::formatDatePattern("EEEEEE", date, locale).empty());
        CHECK(!localisation::formatDatePattern("G", date, locale).empty());
        CHECK(!localisation::formatDatePattern("GGGG", date, locale).empty());
        CHECK(!localisation::formatDatePattern("GGGGG", date, locale).empty());
        CHECK(!localisation::formatTimePattern("a", afternoon, locale).empty());
        CHECK(!localisation::formatTimePattern("aaaa", afternoon, locale).empty());
        CHECK(!localisation::formatTimePattern("aaaaa", afternoon, locale).empty());
        CHECK(!localisation::formatTimePattern("b", timeOf(0, 0, 0), locale).empty());
        CHECK(!localisation::formatTimePattern("b", timeOf(12, 0, 0), locale).empty());
    }
}

TEST_CASE("datetime: every bundled locale formats every style")
{
    const char *tags[] = {"en", "de", "fr", "es", "it", "pt", "ru", "tr", "ar", "he", "hi", "ja", "ko", "zh", "zh-Hant"};
    const DateStyle dateStyles[] = {DateStyle::short_, DateStyle::medium, DateStyle::long_, DateStyle::full};
    const TimeStyle timeStyles[] = {TimeStyle::short_, TimeStyle::medium, TimeStyle::long_, TimeStyle::full};
    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    for (const char *tag : tags)
    {
        const Locale locale(tag);
        for (DateStyle style : dateStyles)
        {
            const std::string text = localisation::formatDate(date, locale, style);
            CHECK(!text.empty());
            CHECK(text.front() != ' ' && text.back() != ' ');
            CHECK(text.find("  ") == std::string::npos);
            CHECK(text.find("()") == std::string::npos);
            CHECK(text.find("[]") == std::string::npos);
        }
        for (TimeStyle style : timeStyles)
        {
            const std::string text = localisation::formatTime(time, locale, style);
            CHECK(!text.empty());
            CHECK(text.front() != ' ' && text.back() != ' ');
            CHECK(text.find("  ") == std::string::npos);
        }
        for (DateStyle dateStyle : dateStyles)
        {
            for (TimeStyle timeStyle : timeStyles)
            {
                const std::string text = localisation::formatDateTime(value, locale, dateStyle, timeStyle);
                CHECK(!text.empty());
                CHECK(text.front() != ' ' && text.back() != ' ');
                CHECK(text.find("  ") == std::string::npos);
                CHECK(text.find("()") == std::string::npos);
                CHECK(text.find('{') == std::string::npos);
                CHECK(text.find('}') == std::string::npos);
                // The composed text keeps the date and the time of the pieces.
                CHECK(text.find(localisation::formatDate(value.getDate(), locale, dateStyle)) != std::string::npos);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// DateTimeFormatter: the Java-aligned layer
// ---------------------------------------------------------------------------

TEST_CASE("formatter: java pattern strings")
{
    const Locale english("en");
    const auto date = dateOf(2024, 8, 15); // Thursday
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    // The examples from java.time.format.DateTimeFormatter's documentation.
    CHECK_EQ(DateTimeFormatter::ofPattern("uuuu-MM-dd'T'HH:mm:ss", english).format(value),
             std::string("2024-08-15T15:04:05"));
    CHECK_EQ(DateTimeFormatter::ofPattern("EEE, MMM d, uuuu", english).format(date),
             std::string("Thu, Aug 15, 2024"));
    CHECK_EQ(DateTimeFormatter::ofPattern("h:mm a", english).format(time), std::string("3:04 PM"));
    CHECK_EQ(DateTimeFormatter::ofPattern("G y", english).format(date), std::string("AD 2024"));
    CHECK_EQ(DateTimeFormatter::ofPattern("D", english).format(date), std::string("228"));
    CHECK_EQ(DateTimeFormatter::ofPattern("w", english).format(date), std::string("33"));
    CHECK_EQ(DateTimeFormatter::ofPattern("S", english).format(timeOf(0, 0, 0, 123456789)), std::string("1"));

    // ofPattern without a locale is the neutral English default.
    CHECK_EQ(DateTimeFormatter::ofPattern("uuuu-MM-dd").format(date), std::string("2024-08-15"));

    // pattern() and toString() expose the pattern text.
    const auto formatter = DateTimeFormatter::ofPattern("uuuu-MM-dd'T'HH:mm:ss", english);
    CHECK_EQ(formatter.pattern(), std::string_view("uuuu-MM-dd'T'HH:mm:ss"));
    CHECK_EQ(formatter.toString(), std::string("uuuu-MM-dd'T'HH:mm:ss"));
    CHECK_EQ(formatter.locale(), english);
}

TEST_CASE("formatter: java pattern letter widths")
{
    const Locale english("en");
    const auto date = dateOf(2024, 8, 15); // Thursday
    const auto time = timeOf(15, 4, 5, 123456789);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    const auto pattern = [&](const char *text, const temporal::LocalGregoryDate &subject) {
        return DateTimeFormatter::ofPattern(text, english).format(subject);
    };
    const auto timePattern = [&](const char *text, const temporal::LocalGregoryTime &subject) {
        return DateTimeFormatter::ofPattern(text, english).format(subject);
    };

    // Era: 1-3 abbreviated, 4 full, 5 narrow.
    CHECK_EQ(pattern("G GG GGG GGGG GGGGG", date), std::string("AD AD AD Anno Domini A"));
    // Proleptic year and year-of-era: 2 is reduced, <4 is SignStyle.NORMAL,
    // >=4 is SignStyle.EXCEEDS_PAD.
    CHECK_EQ(pattern("u uu uuu uuuu", date), std::string("2024 24 2024 2024"));
    CHECK_EQ(pattern("u uu uuu uuuu", dateOf(-5, 1, 1)), std::string("-5 05 -005 -0005"));
    CHECK_EQ(pattern("y yy yyy yyyy G", dateOf(-5, 1, 1)), std::string("6 06 006 0006 BC"));
    CHECK_EQ(pattern("G y u", dateOf(0, 1, 1)), std::string("BC 1 0"));

    // Month and the stand-alone form.
    CHECK_EQ(pattern("M MM MMM MMMM MMMMM", date), std::string("8 08 Aug August A"));
    CHECK_EQ(pattern("L LL LLL LLLL LLLLL", date), std::string("8 08 Aug August A"));
    // Day of month and day of year.
    CHECK_EQ(pattern("d dd", date), std::string("15 15"));
    CHECK_EQ(pattern("D DD DDD", date), std::string("228 228 228"));
    // Quarter and the stand-alone form.
    CHECK_EQ(pattern("Q QQ QQQ QQQQ QQQQQ", date), std::string("3 03 Q3 3rd quarter 3"));
    CHECK_EQ(pattern("q qq qqq qqqq qqqqq", date), std::string("3 03 Q3 3rd quarter 3"));
    // Week-based year, week of week-based year, week of month and the n-th
    // weekday of the month.
    CHECK_EQ(pattern("Y YY YYYY", date), std::string("2024 24 2024"));
    CHECK_EQ(pattern("w ww", date), std::string("33 33"));
    CHECK_EQ(pattern("W", date), std::string("3"));
    CHECK_EQ(pattern("F", date), std::string("3"));
    // Day of week, text widths and the localized numeric forms.
    CHECK_EQ(pattern("E EEEE EEEEE EEEEEE", date), std::string("Thu Thursday T Th"));
    CHECK_EQ(pattern("e ee eee eeee eeeee", date), std::string("4 04 Thu Thursday T"));
    CHECK_EQ(pattern("c ccc cccc ccccc", date), std::string("4 Thu Thursday T"));
    // Day periods.
    CHECK_EQ(timePattern("a aaaa aaaaa", time), std::string("PM PM p"));
    CHECK_EQ(timePattern("b bbbb", timeOf(0, 0, 0)), std::string("midnight midnight"));
    CHECK_EQ(timePattern("b", timeOf(12, 0, 0)), std::string("noon"));
    // Hour fields.
    CHECK_EQ(timePattern("h hh K KK k kk H HH", time), std::string("3 03 3 03 15 15 15 15"));
    CHECK_EQ(timePattern("H h K k", timeOf(0, 0, 0)), std::string("0 12 0 24"));
    CHECK_EQ(timePattern("H h K k", timeOf(12, 0, 0)), std::string("12 12 0 12"));
    // Minute, second and fraction of second.
    CHECK_EQ(timePattern("m mm s ss", time), std::string("4 04 5 05"));
    CHECK_EQ(timePattern("S SS SSS SSSSSSSSS", time), std::string("1 12 123 123456789"));
    CHECK_EQ(timePattern("S SS SSS", timeOf(0, 0, 0, 5000000)), std::string("0 00 005"));
    // Milli-of-day, nano-of-second and nano-of-day.
    CHECK_EQ(timePattern("A", time), std::string("54245123"));
    CHECK_EQ(timePattern("n", time), std::string("123456789"));
    CHECK_EQ(timePattern("N", time), std::string("54245123456789"));

    // Zone letters without zone data render nothing; no exception is raised
    // because a zone is allowed to be absent (see the header's limitations).
    CHECK_EQ(pattern("z", date), std::string());
    CHECK_EQ(pattern("VV", date), std::string());
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm[ Z]", english).format(value), std::string("15:04"));
}

TEST_CASE("formatter: localized date styles")
{
    struct Expectation
    {
        const char *tag;
        const char *shortText;
        const char *mediumText;
        const char *longText;
        const char *fullText;
    };

    const Expectation expectations[] = {
        {"en", "8/15/24", "Aug 15, 2024", "August 15, 2024", "Thursday, August 15, 2024"},
        {"de", "15.08.24", "15.08.2024", "15. August 2024", "Donnerstag, 15. August 2024"},
        {"fr", "15/08/2024", "15 ao\xC3\xBBt 2024", "15 ao\xC3\xBBt 2024", "jeudi 15 ao\xC3\xBBt 2024"},
        {"ja", "2024/08/15", "2024/08/15", "2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5",
         "2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5\xE6\x9C\xA8\xE6\x9B\x9C\xE6\x97\xA5"},
        {"ar", "15\xE2\x80\x8F/8\xE2\x80\x8F/2024", "15\xE2\x80\x8F/08\xE2\x80\x8F/2024",
         "15 \xD8\xA3\xD8\xBA\xD8\xB3\xD8\xB7\xD8\xB3 2024",
         "\xD8\xA7\xD9\x84\xD8\xAE\xD9\x85\xD9\x8A\xD8\xB3\xD8\x8C 15 \xD8\xA3\xD8\xBA\xD8\xB3\xD8\xB7\xD8\xB3 2024"},
        {"ru", "15.08.2024", "15 \xD0\xB0\xD0\xB2\xD0\xB3. 2024\xE2\x80\xAF\xD0\xB3.",
         "15 \xD0\xB0\xD0\xB2\xD0\xB3\xD1\x83\xD1\x81\xD1\x82\xD0\xB0 2024\xE2\x80\xAF\xD0\xB3.",
         "\xD1\x87\xD0\xB5\xD1\x82\xD0\xB2\xD0\xB5\xD1\x80\xD0\xB3, 15 \xD0\xB0\xD0\xB2\xD0\xB3\xD1\x83\xD1\x81\xD1\x82\xD0\xB0 "
         "2024\xE2\x80\xAF\xD0\xB3."},
        {"zh", "2024/8/15", "2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5",
         "2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5",
         "2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5\xE6\x98\x9F\xE6\x9C\x9F\xE5\x9B\x9B"},
        {"hi", "15/8/24", "15 \xE0\xA4\x85\xE0\xA4\x97\xE0\xA5\xB0 2024",
         "15 \xE0\xA4\x85\xE0\xA4\x97\xE0\xA4\xB8\xE0\xA5\x8D\xE0\xA4\xA4 2024",
         "\xE0\xA4\x97\xE0\xA5\x81\xE0\xA4\xB0\xE0\xA5\x81\xE0\xA4\xB5\xE0\xA4\xBE\xE0\xA4\xB0, 15 "
         "\xE0\xA4\x85\xE0\xA4\x97\xE0\xA4\xB8\xE0\xA5\x8D\xE0\xA4\xA4 2024"},
    };

    const auto date = dateOf(2024, 8, 15);
    for (const Expectation &expectation : expectations)
    {
        const Locale locale(expectation.tag);
        CHECK_EQ(localizedDate(locale, FormatStyle::short_).format(date), std::string(expectation.shortText));
        CHECK_EQ(localizedDate(locale, FormatStyle::medium).format(date), std::string(expectation.mediumText));
        CHECK_EQ(localizedDate(locale, FormatStyle::long_).format(date), std::string(expectation.longText));
        CHECK_EQ(localizedDate(locale, FormatStyle::full).format(date), std::string(expectation.fullText));
    }
}

TEST_CASE("formatter: localized time styles")
{
    struct Expectation
    {
        const char *tag;
        const char *shortText;
        const char *mediumText;
        const char *longText;
        const char *fullText;
    };

    const Expectation expectations[] = {
        {"en", "3:04\xE2\x80\xAFPM", "3:04:05\xE2\x80\xAFPM", "3:04:05\xE2\x80\xAFPM", "3:04:05\xE2\x80\xAFPM"},
        {"de", "15:04", "15:04:05", "15:04:05", "15:04:05"},
        {"fr", "15:04", "15:04:05", "15:04:05", "15:04:05"},
        {"ja", "15:04", "15:04:05", "15:04:05", "15\xE6\x99\x82" "04\xE5\x88\x86" "05\xE7\xA7\x92"},
        {"ar", "3:04 \xD9\x85", "3:04:05 \xD9\x85", "3:04:05 \xD9\x85", "3:04:05 \xD9\x85"},
        {"ru", "15:04", "15:04:05", "15:04:05", "15:04:05"},
        {"zh", "15:04", "15:04:05", "15:04:05", "15:04:05"},
        {"hi", "3:04 pm", "3:04:05 pm", "3:04:05 pm", "3:04:05 pm"},
    };

    const auto time = timeOf(15, 4, 5);
    for (const Expectation &expectation : expectations)
    {
        const Locale locale(expectation.tag);
        CHECK_EQ(localizedTime(locale, FormatStyle::short_).format(time), std::string(expectation.shortText));
        CHECK_EQ(localizedTime(locale, FormatStyle::medium).format(time), std::string(expectation.mediumText));
        CHECK_EQ(localizedTime(locale, FormatStyle::long_).format(time), std::string(expectation.longText));
        CHECK_EQ(localizedTime(locale, FormatStyle::full).format(time), std::string(expectation.fullText));
    }
}

TEST_CASE("formatter: localized date-time styles")
{
    struct Expectation
    {
        const char *tag;
        const char *shortText;
        const char *mediumText;
        const char *longText;
        const char *fullText;
    };

    // Each row is the (style, style) pair, i.e. the same style for date and
    // time; CLDR composes the two with the locale's date-time glue pattern.
    const Expectation expectations[] = {
        {"en", "8/15/24, 3:04\xE2\x80\xAFPM", "Aug 15, 2024, 3:04:05\xE2\x80\xAFPM",
         "August 15, 2024, 3:04:05\xE2\x80\xAFPM", "Thursday, August 15, 2024, 3:04:05\xE2\x80\xAFPM"},
        {"de", "15.08.24, 15:04", "15.08.2024, 15:04:05", "15. August 2024, 15:04:05",
         "Donnerstag, 15. August 2024, 15:04:05"},
        {"fr", "15/08/2024 15:04", "15 ao\xC3\xBBt 2024, 15:04:05", "15 ao\xC3\xBBt 2024, 15:04:05",
         "jeudi 15 ao\xC3\xBBt 2024, 15:04:05"},
        {"ja", "2024/08/15 15:04", "2024/08/15 15:04:05",
         "2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5 15:04:05",
         "2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5\xE6\x9C\xA8\xE6\x9B\x9C\xE6\x97\xA5 15\xE6\x99\x82"
         "04\xE5\x88\x86" "05\xE7\xA7\x92"},
        {"ar", "15\xE2\x80\x8F/8\xE2\x80\x8F/2024\xD8\x8C 3:04 \xD9\x85",
         "15\xE2\x80\x8F/08\xE2\x80\x8F/2024\xD8\x8C 3:04:05 \xD9\x85",
         "15 \xD8\xA3\xD8\xBA\xD8\xB3\xD8\xB7\xD8\xB3 2024\xD8\x8C 3:04:05 \xD9\x85",
         "\xD8\xA7\xD9\x84\xD8\xAE\xD9\x85\xD9\x8A\xD8\xB3\xD8\x8C 15 \xD8\xA3\xD8\xBA\xD8\xB3\xD8\xB7\xD8\xB3 "
         "2024\xD8\x8C 3:04:05 \xD9\x85"},
        {"ru", "15.08.2024, 15:04", "15 \xD0\xB0\xD0\xB2\xD0\xB3. 2024\xE2\x80\xAF\xD0\xB3., 15:04:05",
         "15 \xD0\xB0\xD0\xB2\xD0\xB3\xD1\x83\xD1\x81\xD1\x82\xD0\xB0 2024\xE2\x80\xAF\xD0\xB3., 15:04:05",
         "\xD1\x87\xD0\xB5\xD1\x82\xD0\xB2\xD0\xB5\xD1\x80\xD0\xB3, 15 \xD0\xB0\xD0\xB2\xD0\xB3\xD1\x83\xD1\x81\xD1\x82\xD0\xB0 "
         "2024\xE2\x80\xAF\xD0\xB3., 15:04:05"},
        {"zh", "2024/8/15 15:04", "2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5 15:04:05",
         "2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5 15:04:05",
         "2024\xE5\xB9\xB4" "8\xE6\x9C\x88" "15\xE6\x97\xA5\xE6\x98\x9F\xE6\x9C\x9F\xE5\x9B\x9B 15:04:05"},
        {"hi", "15/8/24, 3:04 pm", "15 \xE0\xA4\x85\xE0\xA4\x97\xE0\xA5\xB0 2024, 3:04:05 pm",
         "15 \xE0\xA4\x85\xE0\xA4\x97\xE0\xA4\xB8\xE0\xA5\x8D\xE0\xA4\xA4 2024, 3:04:05 pm",
         "\xE0\xA4\x97\xE0\xA5\x81\xE0\xA4\xB0\xE0\xA5\x81\xE0\xA4\xB5\xE0\xA4\xBE\xE0\xA4\xB0, 15 "
         "\xE0\xA4\x85\xE0\xA4\x97\xE0\xA4\xB8\xE0\xA5\x8D\xE0\xA4\xA4 2024, 3:04:05 pm"},
    };

    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);
    for (const Expectation &expectation : expectations)
    {
        const Locale locale(expectation.tag);
        CHECK_EQ(localizedDateTime(locale, FormatStyle::short_, FormatStyle::short_).format(value),
                 std::string(expectation.shortText));
        CHECK_EQ(localizedDateTime(locale, FormatStyle::medium, FormatStyle::medium).format(value),
                 std::string(expectation.mediumText));
        CHECK_EQ(localizedDateTime(locale, FormatStyle::long_, FormatStyle::long_).format(value),
                 std::string(expectation.longText));
        CHECK_EQ(localizedDateTime(locale, FormatStyle::full, FormatStyle::full).format(value),
                 std::string(expectation.fullText));
    }
}

TEST_CASE("formatter: localized defaults and withLocale immutability")
{
    const auto date = dateOf(2024, 8, 15);

    // ofLocalized* starts in English, like Java's default-format locale.
    const auto base = DateTimeFormatter::ofLocalizedDate(FormatStyle::medium);
    CHECK_EQ(base.locale(), Locale("en"));
    CHECK_EQ(base.pattern(), std::string_view("MMM d, y"));
    CHECK_EQ(base.format(date), std::string("Aug 15, 2024"));

    // withLocale returns a copy: the original formatter and its pattern do not
    // change, and the copy resolves the pattern of its own locale.
    const auto german = base.withLocale(Locale("de"));
    CHECK_EQ(base.locale(), Locale("en"));
    CHECK_EQ(base.pattern(), std::string_view("MMM d, y"));
    CHECK_EQ(base.format(date), std::string("Aug 15, 2024"));
    CHECK_EQ(german.locale(), Locale("de"));
    CHECK_EQ(german.pattern(), std::string_view("dd.MM.y"));
    CHECK_EQ(german.format(date), std::string("15.08.2024"));

    // The same holds for the time and date-time factories.
    const auto time = timeOf(15, 4, 5);
    const auto baseTime = DateTimeFormatter::ofLocalizedTime(FormatStyle::short_);
    CHECK_EQ(baseTime.pattern(), std::string_view("h:mm\xE2\x80\xAF" "a"));
    const auto japaneseTime = baseTime.withLocale(Locale("ja"));
    CHECK_EQ(baseTime.format(time), std::string("3:04\xE2\x80\xAFPM"));
    CHECK_EQ(japaneseTime.format(time), std::string("15:04"));
    CHECK_EQ(baseTime.pattern(), std::string_view("h:mm\xE2\x80\xAF" "a"));

    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);
    const auto baseDateTime = DateTimeFormatter::ofLocalizedDateTime(FormatStyle::short_, FormatStyle::short_);
    // The English glue "{1}, {0}" is composed into one CLDR pattern.
    CHECK_EQ(baseDateTime.pattern(), std::string_view("M/d/yy, h:mm\xE2\x80\xAF" "a"));
    CHECK_EQ(baseDateTime.withLocale(Locale("fr")).format(value), std::string("15/08/2024 15:04"));
    CHECK_EQ(baseDateTime.format(value), std::string("8/15/24, 3:04\xE2\x80\xAFPM"));

    // A pattern formatter keeps its pattern when the locale changes; only the
    // names follow the new locale.
    const auto patternFormatter = DateTimeFormatter::ofPattern("MMMM y", Locale("en"));
    const auto frenchNames = patternFormatter.withLocale(Locale("fr"));
    CHECK_EQ(patternFormatter.pattern(), std::string_view("MMMM y"));
    CHECK_EQ(frenchNames.pattern(), std::string_view("MMMM y"));
    CHECK_EQ(patternFormatter.format(date), std::string("August 2024"));
    CHECK_EQ(frenchNames.format(date), std::string("ao\xC3\xBBt 2024"));

    // ofLocalizedDateTime(style) uses the same style on both sides.
    CHECK_EQ(DateTimeFormatter::ofLocalizedDateTime(FormatStyle::medium).format(value),
             DateTimeFormatter::ofLocalizedDateTime(FormatStyle::medium, FormatStyle::medium).format(value));
}

TEST_CASE("formatter: quoting and optional sections")
{
    const Locale english("en");
    const Locale german("de");
    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 0);

    // Java's quoting rules: '' inside a literal is one apostrophe, and a
    // pattern that is only '' is a literal apostrophe as well.
    CHECK_EQ(DateTimeFormatter::ofPattern("'Year' y", english).format(date), std::string("Year 2024"));
    CHECK_EQ(DateTimeFormatter::ofPattern("y 'o''clock'", english).format(date), std::string("2024 o'clock"));
    CHECK_EQ(DateTimeFormatter::ofPattern("''", english).format(date), std::string("'"));
    CHECK_EQ(DateTimeFormatter::ofPattern("'z' y", english).format(date), std::string("z 2024"));
    CHECK_EQ(DateTimeFormatter::ofPattern("d 'de' MMMM 'de' y", Locale("es")).format(date),
             std::string("15 de agosto de 2024"));
    CHECK_EQ(DateTimeFormatter::ofPattern("'H:mm' HH:mm", english).format(time), std::string("H:mm 15:04"));

    // Optional sections disappear when their content renders empty and stay
    // otherwise, as in Java.
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm[ z]", english).format(time), std::string("15:04"));
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm[ 'UTC']", english).format(time), std::string("15:04 UTC"));
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm[ Z]", english).format(time), std::string("15:04"));
    CHECK_EQ(DateTimeFormatter::ofPattern("z HH:mm", english).format(time), std::string("15:04"));
    CHECK_EQ(DateTimeFormatter::ofPattern("[z] HH:mm", english).format(time), std::string("15:04"));
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm[.ss]", english).format(timeOf(15, 4, 5)),
             std::string("15:04.05"));
    // Nested sections follow the same rule from the inside out.
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm[[ z]]", english).format(time), std::string("15:04"));
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm[[ 'UTC']]", english).format(time), std::string("15:04 UTC"));

    // Parentheses are an extension for CLDR patterns such as "H:mm:ss (zzzz)";
    // they behave like an optional section.
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm (z)", english).format(time), std::string("15:04"));
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm (Z)", english).format(time), std::string("15:04"));
    CHECK_EQ(DateTimeFormatter::ofPattern("'(' y ')'", english).format(date), std::string("( 2024 )"));

    // Literal spacing is preserved; only a separator next to a dropped element
    // is removed.
    CHECK_EQ(DateTimeFormatter::ofPattern("  HH:mm  ", english).format(time), std::string("  15:04  "));

    // German CLDR full date uses quoted literals in a different position.
    CHECK_EQ(DateTimeFormatter::ofPattern("EEEE, d. MMMM y", german).format(date),
             std::string("Donnerstag, 15. August 2024"));
}

TEST_CASE("formatter: invalid patterns throw")
{
    const Locale english("en");

    // Unknown pattern letters are rejected, including Java's newer 'g'.
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("j", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("uuuu g", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("U", english), std::invalid_argument);
    // Reserved characters.
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("{", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("}", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("HH:mm #", english), std::invalid_argument);
    // Quoting and section balance.
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("'unterminated", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("HH:mm[ z", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("HH:mm] z", english), std::invalid_argument);
    // Repetition counts Java rejects as well.
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("cc", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("MMMMMM", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("ddd", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("DDDD", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("SSSSSSSSSS", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("VVV", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("OO", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("zzzzz", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("XXXXXX", english), std::invalid_argument);
    // 'p' must precede a field, and the padded field must fit.
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("pp", english), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("p'x'", english), std::invalid_argument);
    CHECK_EQ(DateTimeFormatter::ofPattern("ppH", english).format(timeOf(5, 4)), std::string(" 5"));
    CHECK_EQ(DateTimeFormatter::ofPattern("pppH", english).format(timeOf(15, 4)), std::string(" 15"));
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("pH", english).format(timeOf(15, 4)), std::invalid_argument);

    // An empty pattern is legal and renders nothing.
    CHECK_EQ(DateTimeFormatter::ofPattern("", english).format(dateOf(2024, 8, 15)), std::string());
}

TEST_CASE("formatter: invalid fields throw")
{
    const Locale english("en");
    const auto dateFormatter = DateTimeFormatter::ofPattern("uuuu-MM-dd", english);
    const auto timeFormatter = DateTimeFormatter::ofPattern("HH:mm:ss", english);

    CHECK_EQ(dateFormatter.formatDate(2024, 8, 15), std::string("2024-08-15"));
    CHECK_EQ(timeFormatter.formatTime(15, 4, 5), std::string("15:04:05"));
    CHECK_EQ(timeFormatter.formatTime(15, 4, 5, 123456789), std::string("15:04:05"));

    // Month 13, 30 February, 29 February of a common year, 24:00 and
    // out-of-range minutes and seconds are all std::invalid_argument.
    CHECK_THROWS_AS(dateFormatter.formatDate(2024, 13, 1), std::invalid_argument);
    CHECK_THROWS_AS(dateFormatter.formatDate(2024, 0, 1), std::invalid_argument);
    CHECK_THROWS_AS(dateFormatter.formatDate(2024, 2, 30), std::invalid_argument);
    CHECK_THROWS_AS(dateFormatter.formatDate(2023, 2, 29), std::invalid_argument);
    CHECK_THROWS_AS(dateFormatter.formatDate(2024, 4, 31), std::invalid_argument);
    CHECK_THROWS_AS(timeFormatter.formatTime(24, 0, 0), std::invalid_argument);
    CHECK_THROWS_AS(timeFormatter.formatTime(-1, 0, 0), std::invalid_argument);
    CHECK_THROWS_AS(timeFormatter.formatTime(0, 60, 0), std::invalid_argument);
    CHECK_THROWS_AS(timeFormatter.formatTime(0, 0, 60), std::invalid_argument);
    CHECK_THROWS_AS(timeFormatter.formatTime(0, 0, 0, 1000000000), std::invalid_argument);
}

TEST_CASE("formatter: field availability")
{
    const Locale english("en");
    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    // A pattern field that the value cannot supply is reported instead of
    // silently rendering a zero.
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("uuuu-MM-dd", english).format(time), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("D", english).format(time), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("EEEE", english).format(time), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("HH:mm", english).format(date), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("a", english).format(date), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("S", english).format(date), std::invalid_argument);
    CHECK_THROWS_AS(DateTimeFormatter::ofPattern("uuuu", english).format(time), std::invalid_argument);

    // A date-time has both parts, and literal-only patterns need neither.
    CHECK_EQ(DateTimeFormatter::ofPattern("uuuu-MM-dd HH:mm", english).format(value), std::string("2024-08-15 15:04"));
    CHECK_EQ(DateTimeFormatter::ofPattern("'hello'", english).format(date), std::string("hello"));
    CHECK_EQ(DateTimeFormatter::ofPattern("'hello'", english).format(time), std::string("hello"));

    // An optional section whose fields the value cannot supply is omitted whole,
    // which is Java's optional-section behaviour.
    CHECK_EQ(DateTimeFormatter::ofPattern("uuuu[ HH:mm]", english).format(date), std::string("2024"));
    CHECK_EQ(DateTimeFormatter::ofPattern("uuuu[ HH:mm]", english).format(value), std::string("2024 15:04"));
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm[ uuuu]", english).format(time), std::string("15:04"));
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm[ uuuu]", english).format(value), std::string("15:04 2024"));
    CHECK_EQ(DateTimeFormatter::ofPattern("uuuu[[ HH:mm]]", english).format(date), std::string("2024"));
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm [uuuu]", english).format(time), std::string("15:04"));
    // Parenthesised sections follow the same rule.
    CHECK_EQ(DateTimeFormatter::ofPattern("uuuu (HH:mm)", english).format(date), std::string("2024"));
}

TEST_CASE("formatter: zone offsets")
{
    const Locale english("en");
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    const auto plus = temporal::OffsetGregoryDateTime::from(value, temporal::Offset::ofHoursMinutes(1, 30));
    const auto plusSeconds = temporal::OffsetGregoryDateTime::from(value, temporal::Offset::ofSeconds(5415));
    const auto minus = temporal::OffsetGregoryDateTime::from(value, temporal::Offset::ofHoursMinutes(-8, 30));
    const auto utc = temporal::OffsetGregoryDateTime::from(value, temporal::Offset());

    const auto offset = [&](const char *pattern, const temporal::OffsetGregoryDateTime &subject) {
        return DateTimeFormatter::ofPattern(pattern, english).format(subject);
    };

    // Z: 1-3 are "+HHMM" with "+0000" for zero, 4 is the localized full form,
    // 5 is "+HH:MM:ss" with "Z" for zero (java.time's OffsetIdPrinterParser).
    CHECK_EQ(offset("Z", plus), std::string("+0130"));
    CHECK_EQ(offset("ZZ", plus), std::string("+0130"));
    CHECK_EQ(offset("ZZZ", plus), std::string("+0130"));
    CHECK_EQ(offset("ZZZZ", plus), std::string("GMT+01:30"));
    CHECK_EQ(offset("ZZZZZ", plus), std::string("+01:30"));
    CHECK_EQ(offset("Z", utc), std::string("+0000"));
    CHECK_EQ(offset("ZZZZ", utc), std::string("GMT"));
    CHECK_EQ(offset("ZZZZZ", utc), std::string("Z"));
    CHECK_EQ(offset("Z", minus), std::string("-0830"));
    CHECK_EQ(offset("ZZZZ", minus), std::string("GMT-08:30"));
    CHECK_EQ(offset("ZZZZZ", plusSeconds), std::string("+01:30:15"));

    // O: the localized offset in the short (hour without a leading zero) and
    // full (two-digit hour) forms, "GMT" for a zero offset.
    CHECK_EQ(offset("O", plus), std::string("GMT+1:30"));
    CHECK_EQ(offset("OOOO", plus), std::string("GMT+01:30"));
    CHECK_EQ(offset("O", utc), std::string("GMT"));
    CHECK_EQ(offset("OOOO", utc), std::string("GMT"));
    CHECK_EQ(offset("O", minus), std::string("GMT-8:30"));
    CHECK_EQ(offset("O", plusSeconds), std::string("GMT+1:30:15"));

    // X: 'Z' for a zero offset at every width; seconds appear from width 4.
    CHECK_EQ(offset("X", plus), std::string("+0130"));
    CHECK_EQ(offset("XX", plus), std::string("+0130"));
    CHECK_EQ(offset("XXX", plus), std::string("+01:30"));
    CHECK_EQ(offset("XXXX", plusSeconds), std::string("+013015"));
    CHECK_EQ(offset("XXXXX", plusSeconds), std::string("+01:30:15"));
    CHECK_EQ(offset("X", utc), std::string("Z"));
    CHECK_EQ(offset("XX", utc), std::string("Z"));
    CHECK_EQ(offset("XXXXX", utc), std::string("Z"));
    CHECK_EQ(offset("X", minus), std::string("-0830"));

    // x: the same widths, but never 'Z'.
    CHECK_EQ(offset("x", utc), std::string("+00"));
    CHECK_EQ(offset("xx", utc), std::string("+0000"));
    CHECK_EQ(offset("xxx", utc), std::string("+00:00"));
    CHECK_EQ(offset("xxxx", utc), std::string("+0000"));
    CHECK_EQ(offset("xxxxx", utc), std::string("+00:00"));
    CHECK_EQ(offset("x", plus), std::string("+0130"));
    CHECK_EQ(offset("xxx", plus), std::string("+01:30"));

    // A value without an offset renders no offset, and an optional section
    // around it disappears.
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm Z", english).format(value), std::string("15:04"));
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm[ Z]", english).format(value), std::string("15:04"));
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm Z", english).format(plus), std::string("15:04 +0130"));
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm[ Z]", english).format(plus), std::string("15:04 +0130"));

    // The offset overload also renders the date and time parts.
    CHECK_EQ(DateTimeFormatter::ofPattern("uuuu-MM-dd'T'HH:mmXXX", english).format(plusSeconds),
             std::string("2024-08-15T15:04+01:30"));

    // A zoned value supplies both the offset and the zone identifier: V prints
    // the identifier, while z and v need CLDR display names and stay empty.
    const auto zone = temporal::Zone::fixed(temporal::Offset::ofHours(8), "Asia/Shanghai");
    const auto zoned = temporal::ZonedGregoryDateTime::of(value, zone);
    CHECK_EQ(DateTimeFormatter::ofPattern("uuuu-MM-dd'T'HH:mmXXX VV", english).format(zoned),
             std::string("2024-08-15T15:04+08:00 Asia/Shanghai"));
    CHECK_EQ(DateTimeFormatter::ofPattern("VV", english).format(zoned), std::string("Asia/Shanghai"));
    CHECK_EQ(DateTimeFormatter::ofPattern("VV").format(zoned), std::string("Asia/Shanghai"));
    CHECK_EQ(DateTimeFormatter::ofPattern("z", english).format(zoned), std::string());
    CHECK_EQ(DateTimeFormatter::ofPattern("VV", english).format(value), std::string());
    CHECK_EQ(DateTimeFormatter::ofPattern("VV", english).format(plus), std::string());
    CHECK_EQ(DateTimeFormatter::ofPattern("HH:mm[ VV]", english).format(zoned),
             std::string("15:04 Asia/Shanghai"));
}

TEST_CASE("formatter: week and quarter fields")
{
    const Locale english("en");
    const auto pattern = [&](const char *text, int year, int month, int day) {
        return DateTimeFormatter::ofPattern(text, english).format(dateOf(year, month, day));
    };

    // 2024-08-15 is ISO week 33 of 2024, week 3 of August and the third
    // Thursday of the month.
    CHECK_EQ(pattern("Y w W F", 2024, 8, 15), std::string("2024 33 3 3"));
    // 2021-01-01 belongs to ISO week 53 of 2020.
    CHECK_EQ(pattern("Y w", 2021, 1, 1), std::string("2020 53"));
    CHECK_EQ(pattern("e c", 2021, 1, 1), std::string("5 5"));
    // 2024-12-30 belongs to ISO week 1 of 2025.
    CHECK_EQ(pattern("Y w", 2024, 12, 30), std::string("2025 1"));
    // Week of month counts Monday-based weeks from the week of the 1st.
    // 2024-12-01 is a Sunday, so December the 1st is week 1 on its own.
    CHECK_EQ(pattern("W", 2024, 12, 30), std::string("6"));
    // 2024-09-01 is also a Sunday: the 1st is week 1, the 2nd already week 2.
    CHECK_EQ(pattern("W", 2024, 9, 1), std::string("1"));
    CHECK_EQ(pattern("W", 2024, 9, 2), std::string("2"));
    CHECK_EQ(pattern("W", 2024, 9, 9), std::string("3"));

    // Quarters are real CLDR names, not numbers dressed up as text.
    CHECK_EQ(pattern("QQQ", 2024, 8, 15), std::string("Q3"));
    CHECK_EQ(pattern("QQQQ", 2024, 8, 15), std::string("3rd quarter"));
    CHECK_EQ(pattern("QQQQQ", 2024, 8, 15), std::string("3"));
    CHECK_EQ(pattern("QQQQ", 2024, 1, 1), std::string("1st quarter"));
    CHECK_EQ(DateTimeFormatter::ofPattern("QQQQ", Locale("de")).format(dateOf(2024, 8, 15)),
             std::string("3. Quartal"));
    CHECK_EQ(DateTimeFormatter::ofPattern("QQQQ", Locale("fr")).format(dateOf(2024, 8, 15)),
             std::string("3e trimestre"));

    // Day of year across a leap year and the last day of a common year.
    CHECK_EQ(pattern("D", 2024, 12, 31), std::string("366"));
    CHECK_EQ(pattern("D", 2023, 12, 31), std::string("365"));
}

TEST_CASE("formatter: legacy free functions agree with the formatter")
{
    const char *tags[] = {"en", "de", "fr", "es", "it", "pt", "ru", "tr", "ar", "he", "hi", "ja", "ko", "zh", "zh-Hant"};
    const FormatStyle formatStyles[] = {FormatStyle::short_, FormatStyle::medium, FormatStyle::long_,
                                        FormatStyle::full};
    const DateStyle dateStyles[] = {DateStyle::short_, DateStyle::medium, DateStyle::long_, DateStyle::full};
    const TimeStyle timeStyles[] = {TimeStyle::short_, TimeStyle::medium, TimeStyle::long_, TimeStyle::full};

    const auto date = dateOf(2024, 8, 15);
    const auto time = timeOf(15, 4, 5);
    const auto value = dateTimeOf(2024, 8, 15, 15, 4, 5);

    for (const char *tag : tags)
    {
        const Locale locale(tag);
        for (std::size_t index = 0; index < 4; ++index)
        {
            CHECK_EQ(localisation::formatDate(date, locale, dateStyles[index]),
                     localizedDate(locale, formatStyles[index]).format(date));
            CHECK_EQ(localisation::formatTime(time, locale, timeStyles[index]),
                     localizedTime(locale, formatStyles[index]).format(time));
            CHECK_EQ(localisation::formatDateTime(value, locale, dateStyles[index], timeStyles[index]),
                     localizedDateTime(locale, formatStyles[index], formatStyles[index]).format(value));
        }
        CHECK_EQ(localisation::formatDatePattern("EEEE', 'MMMM d', 'y", date, locale),
                 DateTimeFormatter::ofPattern("EEEE', 'MMMM d', 'y", locale).format(date));
        CHECK_EQ(localisation::formatTimePattern("HH:mm:ss.SSS", timeOf(1, 2, 3, 456000000), locale),
                 DateTimeFormatter::ofPattern("HH:mm:ss.SSS", locale).format(timeOf(1, 2, 3, 456000000)));
        CHECK_EQ(localisation::formatDateTimePattern("G y D", value, locale),
                 DateTimeFormatter::ofPattern("G y D", locale).format(value));
    }
}
