#include "localisation/localisation.h"

#include "test_framework.h"

#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// Golden strings in this file are the CLDR 49 ca-gregorian.json patterns and
// names for the covered locales, not a recording of the formatter's own output:
// en medium "Nov 9, 2026", de medium "09.11.2026", ja medium "2026/11/09",
// ar medium "09<RLM>/11<RLM>/2026" (RLM is U+200F) and so on. Non-ASCII text is
// written as UTF-8 byte escapes, following tests/test_datetime.cpp, so the
// expectations do not depend on the compiler's source character set:
//
//   "\xE2\x80\xAF"  narrow no-break space (U+202F) before AM/PM in English
//   "\xE2\x80\x8F"  right-to-left mark (U+200F) in Arabic numeric dates
//
// An escape is always followed by a string-literal break where the next
// character is an ASCII hex digit, because C++ would otherwise absorb it into
// the \xNN escape. Time zone tokens (z, zzzz) are dropped because the library
// carries no zone name data, which is why the long and full time styles equal
// the medium one.
//
// 2026-11-09 is a Monday, which is what makes the full date styles interesting.

namespace
{
using localisation::Argument;
using localisation::Arguments;
using localisation::Catalog;
using localisation::FormatStyle;
using localisation::Locale;
using localisation::Message;

const Locale arabic("ar");
const Locale english("en");
const Locale german("de");
const Locale japanese("ja");

std::string format(const Message &message, const Locale &locale, const Arguments &arguments)
{
    return message.format(locale, arguments);
}

// The message a failing format throws, so a test can assert on the wording
// instead of only on the exception type.
std::string failureOf(const std::string &pattern, const Locale &locale, const Arguments &arguments)
{
    try
    {
        (void)Message(pattern).format(locale, arguments);
    }
    catch (const std::invalid_argument &error)
    {
        return std::string(error.what());
    }
    catch (const std::exception &error)
    {
        return std::string("wrong exception type: ") + error.what();
    }
    return std::string("no exception");
}

bool contains(const std::string &text, const std::string &needle)
{
    return text.find(needle) != std::string::npos;
}
} // namespace

TEST_CASE("message datetime: English date and time styles")
{
    const Arguments date = Arguments().setDate("d", 2026, 11, 9);
    const Arguments time = Arguments().setTime("t", 14, 5, 9);

    CHECK_EQ(format(Message("{d, date, short}"), english, date), std::string("11/9/26"));
    CHECK_EQ(format(Message("{d, date, medium}"), english, date), std::string("Nov 9, 2026"));
    CHECK_EQ(format(Message("{d, date, long}"), english, date), std::string("November 9, 2026"));
    CHECK_EQ(format(Message("{d, date, full}"), english, date), std::string("Monday, November 9, 2026"));

    CHECK_EQ(format(Message("{t, time, short}"), english, time), std::string("2:05\xE2\x80\xAFPM"));
    CHECK_EQ(format(Message("{t, time, medium}"), english, time), std::string("2:05:09\xE2\x80\xAFPM"));
    CHECK_EQ(format(Message("{t, time, long}"), english, time), std::string("2:05:09\xE2\x80\xAFPM"));
    CHECK_EQ(format(Message("{t, time, full}"), english, time), std::string("2:05:09\xE2\x80\xAFPM"));

    // The style keyword is trimmed and may sit next to the braces.
    CHECK_EQ(format(Message("{d,date,long}"), english, date), std::string("November 9, 2026"));
    CHECK_EQ(format(Message("{d, date, long }"), english, date), std::string("November 9, 2026"));
}

TEST_CASE("message datetime: German date and time styles")
{
    const Arguments date = Arguments().setDate("d", 2026, 11, 9);
    const Arguments time = Arguments().setTime("t", 14, 5, 9);

    CHECK_EQ(format(Message("{d, date, short}"), german, date), std::string("09.11.26"));
    CHECK_EQ(format(Message("{d, date, medium}"), german, date), std::string("09.11.2026"));
    CHECK_EQ(format(Message("{d, date, long}"), german, date), std::string("9. November 2026"));
    CHECK_EQ(format(Message("{d, date, full}"), german, date), std::string("Montag, 9. November 2026"));

    CHECK_EQ(format(Message("{t, time, short}"), german, time), std::string("14:05"));
    CHECK_EQ(format(Message("{t, time, medium}"), german, time), std::string("14:05:09"));
    CHECK_EQ(format(Message("{t, time, long}"), german, time), std::string("14:05:09"));
    CHECK_EQ(format(Message("{t, time, full}"), german, time), std::string("14:05:09"));
}

TEST_CASE("message datetime: Japanese date and time styles")
{
    const Arguments date = Arguments().setDate("d", 2026, 11, 9);
    const Arguments time = Arguments().setTime("t", 14, 5, 9);

    CHECK_EQ(format(Message("{d, date, short}"), japanese, date), std::string("2026/11/09"));
    CHECK_EQ(format(Message("{d, date, medium}"), japanese, date), std::string("2026/11/09"));
    CHECK_EQ(format(Message("{d, date, long}"), japanese, date),
             std::string("2026\xE5\xB9\xB4" "11\xE6\x9C\x88" "9\xE6\x97\xA5"));
    CHECK_EQ(format(Message("{d, date, full}"), japanese, date),
             std::string("2026\xE5\xB9\xB4" "11\xE6\x9C\x88" "9\xE6\x97\xA5"
                         "\xE6\x9C\x88\xE6\x9B\x9C\xE6\x97\xA5"));

    CHECK_EQ(format(Message("{t, time, short}"), japanese, time), std::string("14:05"));
    CHECK_EQ(format(Message("{t, time, medium}"), japanese, time), std::string("14:05:09"));
    CHECK_EQ(format(Message("{t, time, long}"), japanese, time), std::string("14:05:09"));
    CHECK_EQ(format(Message("{t, time, full}"), japanese, time),
             std::string("14\xE6\x99\x82" "05\xE5\x88\x86" "09\xE7\xA7\x92"));
}

TEST_CASE("message datetime: Arabic date and time styles")
{
    const Arguments date = Arguments().setDate("d", 2026, 11, 9);
    const Arguments time = Arguments().setTime("t", 14, 5, 9);

    CHECK_EQ(format(Message("{d, date, short}"), arabic, date),
             std::string("9\xE2\x80\x8F/11\xE2\x80\x8F/2026"));
    CHECK_EQ(format(Message("{d, date, medium}"), arabic, date),
             std::string("09\xE2\x80\x8F/11\xE2\x80\x8F/2026"));
    CHECK_EQ(format(Message("{d, date, long}"), arabic, date),
             std::string("9 \xD9\x86\xD9\x88\xD9\x81\xD9\x85\xD8\xA8\xD8\xB1 2026"));
    CHECK_EQ(format(Message("{d, date, full}"), arabic, date),
             std::string("\xD8\xA7\xD9\x84\xD8\xA7\xD8\xAB\xD9\x86\xD9\x8A\xD9\x86\xD8\x8C"
                         " 9 \xD9\x86\xD9\x88\xD9\x81\xD9\x85\xD8\xA8\xD8\xB1 2026"));

    CHECK_EQ(format(Message("{t, time, short}"), arabic, time), std::string("2:05 \xD9\x85"));
    CHECK_EQ(format(Message("{t, time, medium}"), arabic, time), std::string("2:05:09 \xD9\x85"));
    CHECK_EQ(format(Message("{t, time, long}"), arabic, time), std::string("2:05:09 \xD9\x85"));
    CHECK_EQ(format(Message("{t, time, full}"), arabic, time), std::string("2:05:09 \xD9\x85"));
}

TEST_CASE("message datetime: default styles")
{
    const Arguments date = Arguments().setDate("d", 2026, 11, 9);
    const Arguments time = Arguments().setTime("t", 14, 5, 9);

    // An omitted style is CLDR's medium style, in every locale.
    CHECK_EQ(format(Message("{d, date}"), english, date), std::string("Nov 9, 2026"));
    CHECK_EQ(format(Message("{d, date}"), german, date), std::string("09.11.2026"));
    CHECK_EQ(format(Message("{d, date}"), japanese, date), std::string("2026/11/09"));
    CHECK_EQ(format(Message("{t, time}"), english, time), std::string("2:05:09\xE2\x80\xAFPM"));
    CHECK_EQ(format(Message("{t, time}"), german, time), std::string("14:05:09"));

    // An explicit medium is the same as no style at all.
    CHECK_EQ(format(Message("{d, date, medium}"), german, date), format(Message("{d, date}"), german, date));
    CHECK_EQ(format(Message("{t, time, medium}"), english, time), format(Message("{t, time}"), english, time));

    // Unknown locales fall back to English date data, not to ISO 8601.
    CHECK_EQ(format(Message("{d, date, medium}"), Locale("xx-YY"), date), std::string("Nov 9, 2026"));
}

TEST_CASE("message datetime: plain placeholders render ISO 8601")
{
    // A plain placeholder has no style to apply, so it prints locale-independent
    // ISO 8601 text, whatever the Locale is.
    const Arguments date = Arguments().setDate("d", 2026, 11, 9);
    CHECK_EQ(format(Message("{d}"), english, date), std::string("2026-11-09"));
    CHECK_EQ(format(Message("{d}"), Locale("ja"), date), std::string("2026-11-09"));

    CHECK_EQ(format(Message("{t}"), english, Arguments().setTime("t", 14, 5, 9)), std::string("14:05:09"));
    CHECK_EQ(format(Message("{t}"), english, Arguments().setTime("t", 14, 5, 9, 123000000)),
             std::string("14:05:09.123"));
    CHECK_EQ(format(Message("{t}"), english, Arguments().setTime("t", 0, 0, 59, 5)),
             std::string("00:00:59.000000005"));
    CHECK_EQ(format(Message("{t}"), english, Arguments().setTime("t", 0, 0, 0, 500000000)),
             std::string("00:00:00.5"));

    CHECK_EQ(format(Message("{x}"), english, Arguments().setDateTime("x", 2026, 11, 9, 14, 5, 9, 123456789)),
             std::string("2026-11-09T14:05:09.123456789"));
    CHECK_EQ(format(Message("{x}"), german, Arguments().setDateTime("x", 2026, 11, 9, 0, 0, 0)),
             std::string("2026-11-09T00:00:00"));

    // ISO padding: four digits inside the common range, a sign outside it.
    CHECK_EQ(format(Message("{d}"), english, Arguments().setDate("d", 999, 1, 2)), std::string("0999-01-02"));
    CHECK_EQ(format(Message("{d}"), english, Arguments().setDate("d", -5, 1, 2)), std::string("-0005-01-02"));
}

TEST_CASE("message datetime: date-time arguments serve both patterns")
{
    const Arguments value = Arguments().setDateTime("x", 2026, 11, 9, 23, 59, 58, 5);

    CHECK_EQ(format(Message("{x, date, short}"), english, value), std::string("11/9/26"));
    CHECK_EQ(format(Message("{x, time, medium}"), english, value), std::string("11:59:58\xE2\x80\xAFPM"));
    CHECK_EQ(format(Message("{x, time, full}"), german, value), std::string("23:59:58"));

    const Argument *stored = value.find("x");
    CHECK(stored != nullptr);
    CHECK(stored->type == Argument::kind::dateTime);
    CHECK(stored->hasDate());
    CHECK(stored->hasTime());
    CHECK_EQ(stored->year, 2026);
    CHECK_EQ(stored->month, 11);
    CHECK_EQ(stored->day, 9);
    CHECK_EQ(stored->hour, 23);
    CHECK_EQ(stored->minute, 59);
    CHECK_EQ(stored->second, 58);
    CHECK_EQ(stored->nanosecond, 5);

    const Arguments dateOnly = Arguments().setDate("d", 2026, 11, 9);
    CHECK(dateOnly.find("d") != nullptr);
    CHECK(dateOnly.find("d")->hasDate());
    CHECK_FALSE(dateOnly.find("d")->hasTime());
    CHECK(dateOnly.find("d")->type == Argument::kind::date);

    const Arguments timeOnly = Arguments().setTime("t", 14, 5, 9);
    CHECK(timeOnly.find("t") != nullptr);
    CHECK_FALSE(timeOnly.find("t")->hasDate());
    CHECK(timeOnly.find("t")->hasTime());
    CHECK(timeOnly.find("t")->type == Argument::kind::time);

    // setDate / setTime / setDateTime overwrite an existing argument by name.
    const Arguments replaced = Arguments().set("d", "text").setDate("d", 2026, 11, 9);
    CHECK_EQ(replaced.size(), static_cast<std::size_t>(1));
    CHECK_EQ(format(Message("{d, date, medium}"), english, replaced), std::string("Nov 9, 2026"));
}

TEST_CASE("message datetime: missing arguments stay placeholders")
{
    const Arguments empty;

    CHECK_EQ(format(Message("on {d, date, full}"), english, empty), std::string("on {d}"));
    CHECK_EQ(format(Message("at {t, time, short}"), english, empty), std::string("at {t}"));
    CHECK_EQ(format(Message("{x, date}"), english, empty), std::string("{x}"));
    CHECK_EQ(format(Message("{x, date, full} and {x, time}"), english, empty), std::string("{x} and {x}"));
}

TEST_CASE("message datetime: type mismatches are rejected")
{
    const Arguments text = Arguments().set("d", "2026-11-09");
    const Arguments integer = Arguments().set("d", 42);
    const Arguments boolean = Arguments().set("d", true);
    const Arguments real = Arguments().set("d", 1.5);
    const Arguments date = Arguments().setDate("d", 2026, 11, 9);
    const Arguments time = Arguments().setTime("t", 14, 5, 9);
    const Arguments timeAsDate = Arguments().setTime("d", 14, 5, 9);
    const Arguments dateAsTime = Arguments().setDate("t", 2026, 11, 9);

    // A date placeholder needs a date value.
    CHECK_THROWS_AS(format(Message("{d, date}"), english, text), std::invalid_argument);
    CHECK_THROWS_AS(format(Message("{d, date}"), english, integer), std::invalid_argument);
    CHECK_THROWS_AS(format(Message("{d, date}"), english, boolean), std::invalid_argument);
    CHECK_THROWS_AS(format(Message("{d, date}"), english, real), std::invalid_argument);
    CHECK_THROWS_AS(format(Message("{d, date}"), english, timeAsDate), std::invalid_argument);

    // A time placeholder needs a time value, so a date (which has no clock
    // fields) is a mismatch too.
    CHECK_THROWS_AS(format(Message("{t, time}"), english, Arguments().set("t", "14:05")), std::invalid_argument);
    CHECK_THROWS_AS(format(Message("{t, time}"), english, dateAsTime), std::invalid_argument);

    // A number placeholder rejects a date or time argument.
    CHECK_THROWS_AS(format(Message("{d, number}"), english, date), std::invalid_argument);
    CHECK_THROWS_AS(format(Message("{t, number}"), english, time), std::invalid_argument);

    // The message names the argument and what the placeholder needed.
    const std::string dateFailure = failureOf("{d, date, full}", english, text);
    CHECK(contains(dateFailure, "localisation:"));
    CHECK(contains(dateFailure, "'d'"));
    CHECK(contains(dateFailure, "{d, date}"));
    CHECK(contains(dateFailure, "must be a date argument"));

    const std::string timeFailure = failureOf("{t, time}", english, dateAsTime);
    CHECK(contains(timeFailure, "must be a time argument"));

    const std::string numberFailure = failureOf("{d, number}", english, date);
    CHECK(contains(numberFailure, "must be a number argument"));

    // Existing number behaviour is untouched: a text argument in a number
    // placeholder still renders as text instead of throwing.
    CHECK_EQ(format(Message("{n, number}"), english, Arguments().set("n", "abc")), std::string("abc"));
}

TEST_CASE("message datetime: invalid date and time styles are rejected")
{
    CHECK_THROWS_AS(Message("{d, date, huge}"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{d, date, Medium}"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{d, date, }"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{d, date, ::yyyyMMdd}"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{t, time, medium2}"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{t, time, short, extra}"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{d, date, short"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{d, date short}"), std::invalid_argument);

    // Parse errors keep the house style: prefix, offset and the pattern.
    const std::string parseFailure = failureOf("{d, date, huge}", english, Arguments());
    CHECK(contains(parseFailure, "localisation: unsupported date style 'huge'"));
    CHECK(contains(parseFailure, "at offset 14"));
    CHECK(contains(parseFailure, "in pattern \"{d, date, huge}\""));

    const std::string timeFailure = failureOf("{t, time, enormous}", english, Arguments());
    CHECK(contains(timeFailure, "unsupported time style 'enormous'"));

    // Every valid style is accepted, in both argument types.
    CHECK_EQ(format(Message("{d, date, short}"), german, Arguments().setDate("d", 2026, 11, 9)),
             std::string("09.11.26"));
    CHECK_EQ(format(Message("{d, date, long}"), german, Arguments().setDate("d", 2026, 11, 9)),
             std::string("9. November 2026"));
    CHECK_EQ(format(Message("{t, time, full}"), german, Arguments().setTime("t", 14, 5, 9)),
             std::string("14:05:09"));
}

TEST_CASE("message datetime: invalid date and time fields are rejected")
{
    // The setters validate through the calendar, so a stored value is always
    // formattable and a bad one fails at the call site instead of at format
    // time.
    CHECK_THROWS_AS(Arguments().setDate("d", 2026, 2, 30), std::invalid_argument);
    CHECK_THROWS_AS(Arguments().setDate("d", 2026, 13, 1), std::invalid_argument);
    CHECK_THROWS_AS(Arguments().setDate("d", 2026, 0, 1), std::invalid_argument);
    CHECK_THROWS_AS(Arguments().setDate("d", 10000, 1, 1), std::invalid_argument);
    CHECK_THROWS_AS(Arguments().setTime("t", 24, 0, 0), std::invalid_argument);
    CHECK_THROWS_AS(Arguments().setTime("t", 0, 60, 0), std::invalid_argument);
    CHECK_THROWS_AS(Arguments().setTime("t", 0, 0, 0, -1), std::invalid_argument);
    CHECK_THROWS_AS(Arguments().setTime("t", 0, 0, 0, 1000000000), std::invalid_argument);
    CHECK_THROWS_AS(Arguments().setDateTime("x", 2026, 11, 9, 24, 0, 0), std::invalid_argument);
    CHECK_THROWS_AS(Arguments().setDateTime("x", 2026, 11, 31, 0, 0, 0), std::invalid_argument);

    // February 29 is only rejected when the year is not a leap year.
    CHECK_EQ(format(Message("{d}"), english, Arguments().setDate("d", 2024, 2, 29)), std::string("2024-02-29"));
}

TEST_CASE("message datetime: dates combine with plural and select")
{
    const Message dated("{count, plural, one {{d, date, medium}} other {{d, date, medium}}}");
    CHECK_EQ(format(dated, english, Arguments().set("count", 1).setDate("d", 2026, 11, 9)),
             std::string("Nov 9, 2026"));
    CHECK_EQ(format(dated, german, Arguments().set("count", 3).setDate("d", 2026, 11, 9)),
             std::string("09.11.2026"));

    const Message booked("{count, plural, one {{d, date, full}: one day} other {{d, date, full}: # days}}");
    CHECK_EQ(format(booked, english, Arguments().set("count", 1).setDate("d", 2026, 11, 9)),
             std::string("Monday, November 9, 2026: one day"));
    CHECK_EQ(format(booked, english, Arguments().set("count", 5).setDate("d", 2026, 11, 9)),
             std::string("Monday, November 9, 2026: 5 days"));

    // A date is not numeric: a plural on a date argument selects "other" and
    // the pound sign is the argument's ISO text.
    CHECK_EQ(format(Message("{d, plural, one {one} other {on #}}"), english,
                    Arguments().setDate("d", 2026, 11, 9)),
             std::string("on 2026-11-09"));

    // A select compares against the same ISO text.
    CHECK_EQ(format(Message("{d, select, 2026-11-09 {today} other {another day}}"), english,
                    Arguments().setDate("d", 2026, 11, 9)),
             std::string("today"));
    CHECK_EQ(format(Message("{d, select, 2026-11-10 {tomorrow} other {another day}}"), english,
                    Arguments().setDate("d", 2026, 11, 9)),
             std::string("another day"));

    // A date placeholder nests inside a select branch.
    const Message gendered("{gender, select, female {{d, date, full}} other {{d, date, short}}}");
    CHECK_EQ(format(gendered, german, Arguments().set("gender", "female").setDate("d", 2026, 11, 9)),
             std::string("Montag, 9. November 2026"));
    CHECK_EQ(format(gendered, german, Arguments().set("gender", "male").setDate("d", 2026, 11, 9)),
             std::string("09.11.26"));
}

TEST_CASE("message datetime: catalogues translate date arguments")
{
    Catalog catalog("en");
    catalog.add(english, "shipped", "Shipped on {d, date, medium}");
    catalog.add(german, "shipped", "Versandt am {d, date, full}");
    catalog.add(japanese, "shipped", "{d, date, long}\xE3\x81\xAB\xE7\x99\xBA\xE9\x80\x81");

    const Arguments arguments = Arguments().setDate("d", 2026, 11, 9);

    CHECK_EQ(catalog.translate(english, "shipped", arguments), std::string("Shipped on Nov 9, 2026"));
    CHECK_EQ(catalog.translate(german, "shipped", arguments), std::string("Versandt am Montag, 9. November 2026"));
    CHECK_EQ(catalog.translate(Locale("de-AT"), "shipped", arguments),
             std::string("Versandt am Montag, 9. November 2026"));
    CHECK_EQ(catalog.translate(Locale("de-CH"), "shipped", arguments),
             std::string("Versandt am Montag, 9. November 2026"));
    CHECK_EQ(catalog.translate(japanese, "shipped", arguments),
             std::string("2026\xE5\xB9\xB4" "11\xE6\x9C\x88" "9\xE6\x97\xA5" "\xE3\x81\xAB\xE7\x99\xBA\xE9\x80\x81"));

    // Missing translations fall through to the source locale (en), whose date
    // pattern is used with the source locale.
    CHECK_EQ(catalog.translate(arabic, "shipped", arguments), std::string("Shipped on Nov 9, 2026"));

    // A missing key returns the key, and an explicit fallback string is
    // formatted with the requested locale.
    CHECK_EQ(catalog.translate(arabic, "unknown.key", arguments), std::string("unknown.key"));
    CHECK_EQ(catalog.translate(japanese, "unknown.key", "Sent {d, date, short}", arguments),
             std::string("Sent 2026/11/09"));
    CHECK_EQ(catalog.translate(arabic, "unknown.key", "Sent {d, date, short}", arguments),
             std::string("Sent 9\xE2\x80\x8F/11\xE2\x80\x8F/2026"));

    // A date and a time argument in one message, translated as a unit.
    Catalog times("en");
    times.add(english, "when", "{d, date, short} at {t, time, short}");
    times.add(german, "when", "{d, date, short} um {t, time, short}");
    CHECK_EQ(times.translate(german, "when", Arguments().setDate("d", 2026, 11, 9).setTime("t", 14, 5, 9)),
             std::string("09.11.26 um 14:05"));
    CHECK_EQ(times.translate(english, "when", Arguments().setDate("d", 2026, 11, 9).setTime("t", 14, 5, 9)),
             std::string("11/9/26 at 2:05\xE2\x80\xAFPM"));

    // A catalogue entry with a broken date style fails when it is added, not
    // when it is translated.
    CHECK_THROWS_AS(times.add(german, "broken", "{d, date, huge}"), std::invalid_argument);
}

TEST_CASE("message datetime: locale fallback for date patterns")
{
    const Arguments date = Arguments().setDate("d", 2026, 11, 9);

    // The date table follows the BCP 47 chain: full tag, language-script,
    // language, then English.
    CHECK_EQ(format(Message("{d, date, full}"), Locale("de-AT"), date),
             std::string("Montag, 9. November 2026"));
    CHECK_EQ(format(Message("{d, date, full}"), german, date), std::string("Montag, 9. November 2026"));
    CHECK_EQ(format(Message("{d, date, full}"), Locale("de-Latn-AT"), date),
             std::string("Montag, 9. November 2026"));
    CHECK_EQ(format(Message("{d, date, medium}"), Locale("zh-TW"), date),
             std::string("2026\xE5\xB9\xB4" "11\xE6\x9C\x88" "9\xE6\x97\xA5"));
    CHECK_EQ(format(Message("{d, date, medium}"), Locale("zh-Hant-HK"), date),
             std::string("2026\xE5\xB9\xB4" "11\xE6\x9C\x88" "9\xE6\x97\xA5"));

    // The formatter is a value: withLocale returns a copy, the original keeps
    // its locale.
    const auto formatter = localisation::DateTimeFormatter::ofLocalizedDate(FormatStyle::full);
    const auto localized = formatter.withLocale(german);
    CHECK_EQ(formatter.formatDate(2026, 11, 9), std::string("Monday, November 9, 2026"));
    CHECK_EQ(localized.formatDate(2026, 11, 9), std::string("Montag, 9. November 2026"));
    CHECK_EQ(localized.locale().tag(), std::string("de"));
    CHECK_EQ(localized.pattern(), std::string("EEEE, d. MMMM y"));
}

TEST_CASE("message datetime: Locale migration regression")
{
    // Locale and the ASCII helpers moved to locale.h; every observable
    // behaviour must be unchanged, and localisation.h must still provide them.
    CHECK_EQ(Locale("de-AT").tag(), std::string("de-AT"));
    CHECK_EQ(Locale("DE-at").tag(), std::string("de-AT"));
    CHECK_EQ(Locale("de_AT").tag(), std::string("de-AT"));
    CHECK_EQ(Locale().tag(), std::string("und"));
    CHECK_FALSE(Locale().specified());
    CHECK(Locale("de-AT").specified());
    CHECK_EQ(Locale("zh-hant_tw").tag(), std::string("zh-Hant-TW"));
    CHECK_EQ(Locale("zh-CN").tag(), std::string("zh-Hans-CN"));
    CHECK_EQ(Locale("yue-HK").tag(), std::string("yue-Hant-HK"));
    CHECK_EQ(Locale("mn-CN").tag(), std::string("mn-Mong-CN"));
    CHECK_EQ(Locale("es-419").tag(), std::string("es-419"));
    CHECK_EQ(Locale("en-US-u-ca-buddhist").tag(), std::string("en-US"));
    CHECK_EQ(Locale("fr", "", "CA").tag(), std::string("fr-CA"));
    CHECK_EQ(Locale("SR", "latn", "rs").tag(), std::string("sr-Latn-RS"));

    const std::vector<Locale> chain = Locale("de-AT").fallbacks();
    CHECK_EQ(chain.size(), static_cast<std::size_t>(2));
    CHECK_EQ(chain[0].tag(), std::string("de-AT"));
    CHECK_EQ(chain[1].tag(), std::string("de"));

    const std::vector<Locale> englishChain = Locale("en").fallbacks();
    CHECK_EQ(englishChain.size(), static_cast<std::size_t>(1));
    CHECK_EQ(englishChain[0].tag(), std::string("en"));

    const std::vector<Locale> chineseChain = Locale("zh-Hant-TW").fallbacks();
    CHECK_EQ(chineseChain.size(), static_cast<std::size_t>(3));
    CHECK_EQ(chineseChain[2].tag(), std::string("zh"));

    CHECK(Locale("de-AT") == Locale("de-at"));
    CHECK(Locale("de-AT") != Locale("de"));
    CHECK(Locale("de") < Locale("de-AT"));

    std::ostringstream stream;
    stream << Locale("de-AT");
    CHECK_EQ(stream.str(), std::string("de-AT"));

    // Negotiation and Accept-Language parsing stayed in localisation.h.
    const std::vector<Locale> available{english, german};
    CHECK_EQ(localisation::negotiate(Locale("de-AT"), available).tag(), std::string("de"));
    CHECK_EQ(localisation::negotiate(Locale("ja"), available).tag(), std::string("en"));
    const std::vector<Locale> parsed = localisation::parseAcceptLanguage("fr-CH, fr;q=0.9, en;q=0.8");
    CHECK_EQ(parsed.size(), static_cast<std::size_t>(3));
    CHECK_EQ(parsed[0].tag(), std::string("fr-CH"));

    // The detail string helpers are still available under the same names.
    CHECK(localisation::detail::isSpace('\t'));
    CHECK_FALSE(localisation::detail::isSpace('x'));
    CHECK(localisation::detail::isDigit('7'));
    CHECK(localisation::detail::isAlpha('Q'));
    CHECK(localisation::detail::isAlnum('7'));
    CHECK_FALSE(localisation::detail::isAlnum('-'));
    CHECK_EQ(localisation::detail::toLower('A'), 'a');
    CHECK_EQ(localisation::detail::toUpper('z'), 'Z');
    CHECK_EQ(localisation::detail::asciiLower("De-AT"), std::string("de-at"));
    CHECK_EQ(localisation::detail::asciiUpper("de-at"), std::string("DE-AT"));
    CHECK_EQ(localisation::detail::asciiTitle("hELLO"), std::string("Hello"));
    CHECK_EQ(localisation::detail::trimView("  de-AT \t"), std::string_view("de-AT"));
    CHECK_EQ(localisation::detail::trim("  de-AT \t"), std::string("de-AT"));
    CHECK_EQ(localisation::detail::trim("   "), std::string(""));
}
