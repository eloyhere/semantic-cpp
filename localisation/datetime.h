#pragma once

// ===========================================================================
// localisation/datetime.h
//
// Locale-aware date, time and date-time formatting in the style of
// java.time.format.DateTimeFormatter, driven by Unicode CLDR data.
//
// The public surface has two layers:
//
//   * localisation::DateTimeFormatter - the Java-aligned formatter: an
//     immutable value built from a pattern (ofPattern) or from a CLDR style
//     (ofLocalizedDate / ofLocalizedTime / ofLocalizedDateTime), with
//     withLocale, pattern, locale, format and toString.
//   * The original free functions (formatDate, formatTime, formatDateTime and
//     formatDatePattern / formatTimePattern / formatDateTimePattern). Their
//     signatures are unchanged; they are now thin wrappers over
//     DateTimeFormatter, so both layers always agree.
//
// Example:
//
//   const localisation::Locale german("de");
//   const auto date = temporal::gregory::Date::of(temporal::gregory::Year(2024),
//                                                 temporal::gregory::Month::August, 15);
//   const auto time = temporal::LocalGregoryTime::of(15, 4, 5);
//
//   const auto iso = localisation::DateTimeFormatter::ofPattern(
//       "uuuu-MM-dd'T'HH:mm:ss", localisation::Locale("en"));
//   std::cout << iso.format(temporal::LocalGregoryDateTime(date, time));
//   // 2024-08-15T15:04:05
//
//   std::cout << localisation::DateTimeFormatter::ofLocalizedDate(
//                    localisation::FormatStyle::full)
//                    .withLocale(german)
//                    .format(date);
//   // Donnerstag, 15. August 2024
//
// The header is stateless: there is no process-wide "current locale", every
// formatter carries the Locale it was built with, and the data table is
// immutable constexpr state. A DateTimeFormatter is an immutable value: every
// with* operation returns a copy, so formatters can be shared freely and
// formatting is thread-safe.
//
// Dependencies
// ------------
// Formatting needs the Locale type and nothing else from the localisation
// module, so this header includes locale.h instead of localisation.h.  That
// keeps the include chain acyclic - locale.h <- datetime.h <- localisation.h -
// and lets the message layer render its "{d, date}" and "{t, time}" arguments
// with the CLDR patterns below (see ArgumentKind::date/time in
// localisation.h).  datetime.h is usable on its own: it only needs locale.h,
// the temporal value types and its generated data table.
//
// Data
// ----
// Patterns and the month, quarter, weekday, day-period and era names come from
// localisation/datetime_data.h, which tools/gen_datetime_data.py generates from
// Unicode CLDR 49.0.0 (cldr-json 49.0.0) ca-gregorian.json files. The table
// covers 15 locales:
//
//   en  de  fr  es  it  pt  ru  tr  ar  he  hi  ja  ko  zh  zh-Hant
//
// Locale lookup follows a CLDR-style chain: full tag, then language-script,
// then language, then "en". zh-TW therefore resolves to zh-Hant data, de-AT and
// de-CH to de, and any unknown tag to English. Locale itself does the BCP 47
// canonicalisation, so "ZH-hant" and "zh-Hant" select the same data.
//
// Pattern language
// ----------------
// The engine implements the java.time.format.DateTimeFormatter pattern
// language:
//
//   G  era                     text (1-3 abbreviated, 4 full, 5 narrow)
//   u  year                    number (2 = reduced two-digit, <4 normal sign,
//                                     >=4 sign only when the pad width is
//                                     exceeded, like SignStyle.EXCEEDS_PAD)
//   y  year-of-era             same widths; never negative (year 0 is 1 BCE)
//   Y  week-based-year         same widths; ISO 8601 week rules (see below)
//   Q  quarter-of-year         number (1-2) or text (3 abbreviated, 4 full,
//   q                         5 narrow); q is the stand-alone form, which this
//                             data set does not distinguish from Q
//   M  month-of-year           number (1-2) or text (3/4/5); L is stand-alone
//   L
//   w  week-of-week-based-year number (1-2), ISO 8601
//   W  week-of-month           number (1), ISO 8601
//   D  day-of-year             number (1-3)
//   d  day-of-month            number (1-2)
//   F  day-of-week-in-month    number (1), the n-th weekday of the month
//   E  day-of-week             text (1-3 abbreviated, 4 full, 5 narrow,
//                                    6 short; 6 is a CLDR extension)
//   e  localized day-of-week   number (1-2) or text (3/4/5, 6 short)
//   c  localized day-of-week   number (1) or text (3/4/5, 6 short)
//   a  am/pm                   text (1-3 abbreviated, 4 wide, 5 narrow)
//   b  am/pm/noon/midnight     text, like a; "b" is a CLDR letter Java lacks
//   B  period-of-day           text, approximated with am/pm (see below)
//   h  clock-hour-of-am-pm     number (1-2), 1..12
//   K  hour-of-am-pm           number (1-2), 0..11
//   k  clock-hour-of-day       number (1-2), 1..24 (midnight is 24)
//   H  hour-of-day             number (1-2), 0..23
//   m  minute-of-hour          number (1-2)
//   s  second-of-minute        number (1-2)
//   S  fraction-of-second      fraction (1..9 digits, most significant first)
//   A  milli-of-day            number
//   n  nano-of-second          number
//   N  nano-of-day             number
//   z  time-zone name          no name data: renders nothing (see limitations)
//   v  generic zone name       no name data: renders nothing (count 1 or 4)
//   V  time-zone id            a zoned value's temporal::Zone identifier
//   Z  zone-offset             offset-Z widths 1-3, 4 (localized), 5
//   O  localized zone-offset   offset-O widths 1 and 4, "GMT+8" style
//   X  zone-offset, Z for zero  offset-X widths 1-5
//   x  zone-offset             offset-x widths 1-5
//   p  pad next                pads the following field with spaces
//   '  quoted literal          '' is a single apostrophe
//   [  ]                       optional section, dropped when it renders empty
//
// Numeric fields follow Java's rules: a single letter is the minimum width
// without padding, two or more letters zero-pad to that width. Text fields
// follow the CLDR widths (abbreviated / wide / narrow) and reject counts the
// data cannot satisfy. Patterns are validated when the formatter is built, so
// an unknown pattern letter, a reserved character ('{', '}', '#'), an
// unterminated quoted literal or an unsupported repetition count throws
// std::invalid_argument from ofPattern instead of failing during formatting.
//
// Optional sections
// -----------------
// "[ ... ]" works as in Java: a section whose fields the value cannot supply is
// omitted whole (so "uuuu[ HH:mm]" formats a date without complaining about the
// missing time), and a section that renders nothing between its markers is
// dropped as well, which is what happens to a zone token when no zone data
// exists. Parentheses are treated the same way, which is not Java but is
// required by CLDR patterns such as the Spanish full time "H:mm:ss (zzzz)":
// dropping the empty zone must remove the parentheses too rather than leave
// "()" behind.
//
// Field availability
// ------------------
// A pattern field outside every optional section must be supplied by the value:
// formatting a LocalGregoryDate with "HH:mm", or a LocalGregoryTime with
// "uuuu", throws std::invalid_argument. Java raises
// UnsupportedTemporalTypeException there; this library uses the single
// exception type it documents for invalid formatting requests.
//
// Localized styles
// ----------------
// ofLocalizedDate / ofLocalizedTime / ofLocalizedDateTime select a row of the
// generated CLDR dateFormats / timeFormats / dateTimeFormats tables through
// FormatStyle (full, long_, medium, short_). The formatter they return carries
// the style and an English locale by default; withLocale switches the locale
// and re-resolves the pattern, exactly like Java's lazily localized
// formatters. pattern() and toString() return the resolved CLDR pattern of the
// current locale (Java's localized formatters print a description instead,
// because a single Java formatter can carry several patterns).
//
// Exceptions
// ----------
// The temporal value types validate their own fields, so an invalid date
// (month 13, 30 February) or time (24:00) cannot reach the formatter, and the
// field-taking conveniences DateTimeFormatter::formatDate / formatTime throw
// std::invalid_argument for exactly those inputs. Invalid patterns are rejected
// by ofPattern (see above), and a pattern field that the value cannot supply
// throws std::invalid_argument unless it sits in an optional section.
//
// Java compatibility
// ------------------
// The pattern language is Java's, with these deliberate supersets of it:
//
//   * 'b' (am/pm/noon/midnight) and 'v' (generic zone name) are CLDR letters
//     that Java rejects; they are accepted because CLDR patterns use them.
//   * 'a' and 'b' accept 4 (wide) and 5 (narrow) instead of Java's single
//     letter, and E / e / c accept 6 (CLDR's "short" weekday width). Java
//     rejects those counts.
//   * Parentheses act as optional sections, as described above.
//   * A pattern letter that this library cannot render (z and v always, V
//     without a zone, or an offset letter on a value without an offset)
//     renders nothing instead of throwing; Java would throw when the field is
//     mandatory.
//
// Known limitations
// -----------------
//   * Gregorian calendar only. The patterns and names are CLDR's "gregorian"
//     calendar set; non-Gregorian calendars (islamic, japanese, buddhist, ...)
//     and their year numbering are not implemented.
//   * Time zone names are not implemented. The library carries no zone name
//     data, so z and v render an empty string (and an enclosing optional
//     section disappears with them). V prints the identifier of the value's
//     temporal::Zone, which is real data; for the Local* and Offset* values,
//     which carry no zone, it renders empty too. Z, O, X and x render a real
//     offset when the value has one (the OffsetGregoryDateTime and
//     ZonedGregoryDateTime overloads); a LocalGregoryDate/Time/DateTime
//     carries no offset, so those letters render empty for it.
//   * The localized offset text is the CLDR root "GMT": CLDR's localised
//     gmtZeroFormat lives in the supplemental time zone name data, which this
//     table does not bundle.
//   * Week-based fields use the ISO 8601 week rules (Monday first, week 1
//     contains 4 January) for w, W and Y, and ISO weekday numbering (Monday =
//     1 .. Sunday = 7) for e and c. Java uses each locale's CLDR weekData
//     (first day of week, minimal days), which is supplemental data that the
//     per-locale ca-gregorian.json files do not contain.
//   * "F" is the day-of-week-in-month as defined by CLDR and
//     SimpleDateFormat, (day - 1) / 7 + 1. Java maps the letter to
//     ChronoField.ALIGNED_DAY_OF_WEEK_IN_MONTH instead, which yields
//     (day - 1) % 7 + 1.
//   * Relative time ("yesterday", "in 3 days"), interval formatting, skeleton
//     matching, parsing and the resolver are not implemented: this is a
//     formatter only.
//   * Flexible day periods ("B") are approximated with the am/pm names because
//     CLDR's dayPeriodRules are not part of ca-gregorian.json; "b" uses the
//     midnight/noon names when the hour is exactly 0 or 12.
//   * Native digits are not implemented: numeric fields always use ASCII
//     digits, and the data snapshot declares latn as the numbering system of
//     every covered locale.
//   * Java's newer "g" (modified Julian day) is not implemented and is
//     rejected as an unknown pattern letter.
//   * Only the 15 locales above have data. Every other tag falls back to
//     English instead of to a locale-independent ISO 8601 format.
// ===========================================================================

#include "datetime_data.h"
#include "locale.h"

#include "../temporal/field.h"
#include "../temporal/gregory/gregory.h"
#include "../temporal/local_date.h"
#include "../temporal/local_date_time.h"
#include "../temporal/local_time.h"
#include "../temporal/offset.h"
#include "../temporal/offset_date_time.h"
#include "../temporal/zone.h"
#include "../temporal/zone_temporal.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace localisation
{

// The generated data table lives under detail; the short alias keeps the
// formatting code readable.
namespace cldr = detail::cldr;

// The four CLDR date/time styles, named and ordered as in
// java.time.format.FormatStyle (FULL, LONG, MEDIUM, SHORT). The enumerator
// values are the Java order and are mapped onto the generated pattern rows by
// styleIndex(); the trailing underscores keep long_ and short_ valid C++
// identifiers.
enum class FormatStyle
{
    full,
    long_,
    medium,
    short_
};

// The original style selectors are kept so that the free-function signatures
// do not change. Their values are the indices of the generated pattern arrays
// (short, medium, long, full).
enum class DateStyle
{
    short_ = 0,
    medium = 1,
    long_ = 2,
    full = 3
};

enum class TimeStyle
{
    short_ = 0,
    medium = 1,
    long_ = 2,
    full = 3
};

static_assert(cldr::localeCount > 0,
              "localisation: the CLDR date/time table is empty; run tools/gen_datetime_data.py");

namespace detail
{

// FormatStyle is declared in Java order (full, long_, medium, short_) while the
// generated tables are indexed short..full, so the two are related by a
// reversal rather than a cast.
constexpr auto styleIndex(FormatStyle style) noexcept -> std::size_t
{
    return 3 - static_cast<std::size_t>(style);
}

// The legacy style selectors use the table order directly, so converting one to
// a FormatStyle is a small switch rather than a cast.
constexpr auto formatStyleOf(DateStyle style) noexcept -> FormatStyle
{
    switch (style)
    {
    case DateStyle::short_:
        return FormatStyle::short_;
    case DateStyle::medium:
        return FormatStyle::medium;
    case DateStyle::long_:
        return FormatStyle::long_;
    case DateStyle::full:
        return FormatStyle::full;
    }
    return FormatStyle::medium;
}

constexpr auto formatStyleOf(TimeStyle style) noexcept -> FormatStyle
{
    switch (style)
    {
    case TimeStyle::short_:
        return FormatStyle::short_;
    case TimeStyle::medium:
        return FormatStyle::medium;
    case TimeStyle::long_:
        return FormatStyle::long_;
    case TimeStyle::full:
        return FormatStyle::full;
    }
    return FormatStyle::medium;
}

// ---------------------------------------------------------------------------
// The decoded value a pattern renders
// ---------------------------------------------------------------------------

// Every field a pattern can ask for, resolved once per value so that a pattern
// such as "EEEE, MMMM d, uuuu" does not decode the date four times. The
// has* flags record which parts of a value exist, so a pattern field that the
// value cannot supply is reported instead of silently rendering a zero.
struct DateTimeParts
{
    std::int32_t year = 0;  // proleptic Gregorian year, may be negative
    std::int32_t weekYear = 0; // ISO week-based year
    int month = 1;          // 1..12
    int day = 1;            // 1..31
    int dayOfYear = 1;      // 1..366
    int isoWeekday = 1;     // ISO 8601: Monday = 1 .. Sunday = 7
    int weekOfWeekYear = 1; // ISO 8601: 1..53
    int weekOfMonth = 1;    // ISO 8601: week 1 contains the 1st
    int dayOfWeekInMonth = 1; // n-th weekday of the month, 1..5
    int quarter = 1;        // 1..4
    int hour = 0;           // 0..23
    int minute = 0;         // 0..59
    int second = 0;         // 0..59
    int nanosecond = 0;     // 0..999999999
    int offsetSeconds = 0;  // seconds east of UTC
    std::string_view zoneId; // IANA-style identifier, when the value has a zone
    bool hasDate = false;
    bool hasTime = false;
    bool hasOffset = false;
    bool hasZone = false;
};

// The week fields, which a date cannot answer on its own: the ISO week and the
// week-based year come from the calendar policy, and the ISO week-of-month is
// derived from the weekday of the first day of the month.
inline auto fillWeekParts(DateTimeParts &parts, const temporal::DayNumber day) -> void
{
    parts.weekYear = static_cast<std::int32_t>(
        temporal::gregory::Calendar::getField(day, temporal::Field::WeekYear));
    parts.weekOfWeekYear = static_cast<int>(
        temporal::gregory::Calendar::getField(day, temporal::Field::WeekOfWeekYear));
    const temporal::DayNumber firstOfMonth = day - temporal::Days(parts.day - 1);
    const int firstWeekday = temporal::gregory::isoNumber(temporal::gregory::Calendar::getWeekday(firstOfMonth));
    parts.weekOfMonth = (parts.day + firstWeekday - 2) / 7 + 1;
    parts.dayOfWeekInMonth = (parts.day - 1) / 7 + 1;
    parts.quarter = (parts.month - 1) / 3 + 1;
}

inline auto partsOf(const temporal::LocalGregoryDate &date) -> DateTimeParts
{
    DateTimeParts parts;
    parts.hasDate = true;
    parts.year = date.getYear().getValue();
    parts.month = temporal::gregory::number(date.getMonth());
    parts.day = date.getDay();
    parts.dayOfYear = date.getDayOfYear();
    parts.isoWeekday = temporal::gregory::isoNumber(date.getWeekday());
    fillWeekParts(parts, date.getDayNumber());
    return parts;
}

inline auto partsOf(const temporal::LocalGregoryTime &time) -> DateTimeParts
{
    DateTimeParts parts;
    parts.hasTime = true;
    parts.hour = time.getHour();
    parts.minute = time.getMinute();
    parts.second = time.getSecond();
    parts.nanosecond = time.getNanosecond();
    return parts;
}

inline auto partsOf(const temporal::LocalGregoryDateTime &value) -> DateTimeParts
{
    DateTimeParts parts = partsOf(value.getDate());
    parts.hasTime = true;
    parts.hour = value.getHour();
    parts.minute = value.getMinute();
    parts.second = value.getSecond();
    parts.nanosecond = value.getNanosecond();
    return parts;
}

inline auto partsOf(const temporal::OffsetGregoryDateTime &value) -> DateTimeParts
{
    DateTimeParts parts = partsOf(value.getDateTime());
    parts.hasOffset = true;
    parts.offsetSeconds = value.getOffset().getSeconds();
    return parts;
}

inline auto partsOf(const temporal::ZonedGregoryDateTime &value) -> DateTimeParts
{
    DateTimeParts parts = partsOf(value.getDateTime());
    parts.hasOffset = true;
    parts.offsetSeconds = value.getOffset().getSeconds();
    parts.hasZone = true;
    parts.zoneId = value.getZone().getIdentifier();
    return parts;
}

// ---------------------------------------------------------------------------
// Small formatting helpers
// ---------------------------------------------------------------------------

inline bool isAsciiLetter(char character) noexcept
{
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z');
}

inline bool isSpaceOrTab(char character) noexcept
{
    return character == ' ' || character == '\t';
}

// Java's SignStyle, reduced to the three variants the pattern language needs.
enum class SignStyle
{
    normal,     // SignStyle.NORMAL: sign only for negative values
    exceedsPad, // SignStyle.EXCEEDS_PAD: also '+' when the pad width is exceeded
    notNegative // SignStyle.NOT_NEGATIVE: negative values are a formatting error
};

inline auto appendTwoDigits(std::string &output, int value) -> void
{
    output.push_back(static_cast<char>('0' + value / 10 % 10));
    output.push_back(static_cast<char>('0' + value % 10));
}

// Zero-padded integer with Java's sign rules. The sign is written separately
// from the zero padding, so year -5 with "uuuu" renders as "-0005".
inline auto appendNumber(std::int64_t value, std::size_t width, SignStyle style, std::string &output) -> void
{
    const bool negative = value < 0;
    if (negative && style == SignStyle::notNegative)
    {
        throw std::invalid_argument("localisation: a non-negative pattern field received a negative value");
    }
    std::uint64_t magnitude =
        negative ? static_cast<std::uint64_t>(-(value + 1)) + 1 : static_cast<std::uint64_t>(value);
    char digits[20];
    std::size_t length = 0;
    do
    {
        digits[length] = static_cast<char>('0' + magnitude % 10);
        ++length;
        magnitude /= 10;
    } while (magnitude != 0);

    const bool sign = negative || (style == SignStyle::exceedsPad && length > width);
    if (sign)
    {
        output.push_back(negative ? '-' : '+');
    }
    for (std::size_t index = length; index < width; ++index)
    {
        output.push_back('0');
    }
    while (length > 0)
    {
        --length;
        output.push_back(digits[length]);
    }
}

inline auto appendText(std::string &output, const char *text) -> void
{
    if (text != nullptr)
    {
        output += text;
    }
}

// ---------------------------------------------------------------------------
// The compiled pattern
// ---------------------------------------------------------------------------

enum class TokenKind : std::uint8_t
{
    literal,
    field,
    optionalStart,
    optionalEnd,
    parenthesisStart,
    parenthesisEnd
};

// One compiled pattern element. Literals carry their text; fields carry the
// pattern letter, its repetition count and the width of a preceding 'p' pad
// modifier (0 means "no padding").
struct Token
{
    TokenKind kind = TokenKind::literal;
    std::string literal;
    char letter = 0;
    std::size_t count = 0;
    std::size_t pad = 0;
};

// Optional sections may nest; CLDR uses at most two levels and Java has no
// limit at all. A fixed ceiling keeps rendering allocation-free.
inline constexpr std::size_t maximumGroupDepth = 16;

inline auto invalidCount(char letter, std::size_t count) -> std::invalid_argument
{
    return std::invalid_argument("localisation: invalid pattern: '" + std::string(1, letter) + "' repeated " +
                                 std::to_string(count) + " times");
}

// Rejects the pattern-letter repetitions that Java also rejects, so a typo
// fails when the formatter is built instead of producing misleading text.
inline auto validateField(char letter, std::size_t count) -> void
{
    switch (letter)
    {
    case 'G':
        if (count < 1 || count > 5)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'M':
    case 'L':
    case 'Q':
    case 'q':
        if (count < 1 || count > 5)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'E':
    case 'e':
        // Six letters select CLDR's "short" weekday width; Java stops at five.
        if (count < 1 || count > 6)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'c':
        // "cc" is invalid in Java as well; six selects CLDR's short width.
        if (count == 2 || count < 1 || count > 6)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'a':
    case 'b':
    case 'B':
        if (count < 1 || count > 5)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'D':
        if (count < 1 || count > 3)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'd':
    case 'H':
    case 'h':
    case 'K':
    case 'k':
    case 'm':
    case 's':
        if (count < 1 || count > 2)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'F':
    case 'W':
        if (count != 1)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'w':
        if (count < 1 || count > 2)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'S':
        if (count < 1 || count > 9)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'z':
        if (count < 1 || count > 4)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'V':
        if (count != 2)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'v':
        if (count != 1 && count != 4)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'Z':
        if (count < 1 || count > 5)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'O':
        if (count != 1 && count != 4)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'X':
    case 'x':
        if (count < 1 || count > 5)
        {
            throw invalidCount(letter, count);
        }
        return;
    case 'u':
    case 'y':
    case 'Y':
    case 'A':
    case 'n':
    case 'N':
        if (count < 1)
        {
            throw invalidCount(letter, count);
        }
        return;
    default:
        throw std::invalid_argument("localisation: unknown pattern letter: '" + std::string(1, letter) + "'");
    }
}

// Renders one compiled pattern into a string. The renderer is a short-lived
// helper; the immutable Pattern it is used from is what callers share.
//
// A field that the value cannot supply raises MissingField internally: inside an
// optional section that means "omit the whole section", which is what Java's
// optionalStart/optionalEnd do, while a field outside every section becomes the
// std::invalid_argument that format() reports.
struct MissingField : std::exception
{
    explicit MissingField(std::string text) : message(std::move(text)) {}

    auto what() const noexcept -> const char * override { return message.c_str(); }

    std::string message;
};

class PatternRenderer
{
  public:
    PatternRenderer(const cldr::LocaleData &data, const DateTimeParts &parts, std::string &output) noexcept
        : data_(data), parts_(parts), output_(output)
    {
    }

    auto render(const std::vector<Token> &tokens) -> void
    {
        depth_ = 0;
        std::size_t index = 0;
        while (index < tokens.size())
        {
            const Token &token = tokens[index];
            switch (token.kind)
            {
            case TokenKind::literal:
                writeLiteral(token.literal);
                ++index;
                break;
            case TokenKind::optionalStart:
                groups_[depth_] = Group{output_.size(), output_.size(), false};
                ++depth_;
                ++index;
                break;
            case TokenKind::parenthesisStart:
                // The parenthesis is written eagerly so that closing the group
                // can drop it together with the empty content.
                groups_[depth_] = Group{output_.size(), output_.size() + 1, true};
                ++depth_;
                output_.push_back('(');
                ++index;
                break;
            case TokenKind::optionalEnd:
                closeGroup(false);
                ++index;
                break;
            case TokenKind::parenthesisEnd:
                closeGroup(true);
                ++index;
                break;
            case TokenKind::field:
                try
                {
                    writeField(token);
                }
                catch (const MissingField &missing)
                {
                    if (depth_ == 0)
                    {
                        throw std::invalid_argument(missing.message);
                    }
                    index = skipOptionalSection(tokens, index);
                    break;
                }
                ++index;
                break;
            }
        }
        // An unbalanced '(' keeps its content: it is literal punctuation that
        // the pattern never asked to be optional in the Java sense.
    }

  private:
    struct Group
    {
        std::size_t openPosition;   // index in the output where the group starts
        std::size_t contentStart;   // index in the output where the content starts
        bool parenthesis;           // '(' .. ')' rather than '[' .. ']'
    };

    // Abandons the optional section that contains tokens[index]: every token up
    // to and including the group's closing marker is skipped and the text the
    // section produced so far is removed.
    auto skipOptionalSection(const std::vector<Token> &tokens, std::size_t index) -> std::size_t
    {
        const std::size_t targetDepth = depth_;
        std::size_t cursor = index;
        std::size_t localDepth = depth_;
        while (cursor < tokens.size())
        {
            const Token &token = tokens[cursor];
            if (token.kind == TokenKind::optionalStart || token.kind == TokenKind::parenthesisStart)
            {
                ++localDepth;
            }
            else if (token.kind == TokenKind::optionalEnd || token.kind == TokenKind::parenthesisEnd)
            {
                --localDepth;
                if (localDepth + 1 == targetDepth)
                {
                    break;
                }
            }
            ++cursor;
        }
        const Group group = groups_[depth_ - 1];
        depth_ = targetDepth - 1;
        if (group.openPosition <= output_.size())
        {
            output_.resize(group.openPosition);
        }
        pendingSeparator_ = false;
        dropSeparator();
        return cursor < tokens.size() ? cursor + 1 : tokens.size();
    }

    auto closeGroup(bool parenthesis) -> void
    {
        if (depth_ == 0 || groups_[depth_ - 1].parenthesis != parenthesis)
        {
            return;
        }
        const Group group = groups_[depth_ - 1];
        --depth_;
        if (isBlankFrom(group.contentStart))
        {
            if (group.openPosition <= output_.size())
            {
                output_.resize(group.openPosition);
            }
            // Anything the dropped content asked for is gone with it.
            pendingSeparator_ = false;
            dropSeparator();
            return;
        }
        if (group.parenthesis)
        {
            output_.push_back(')');
        }
    }

    auto isBlankFrom(std::size_t start) const noexcept -> bool
    {
        for (std::size_t index = start; index < output_.size(); ++index)
        {
            if (!isSpaceOrTab(output_[index]))
            {
                return false;
            }
        }
        return true;
    }

    // A dropped element leaves a separating space behind in most CLDR patterns
    // ("h:mm:ss a zzzz", "H:mm:ss (zzzz)", "z HH:mm:ss"). The renderer removes
    // exactly one separator: the space in front of the element when there is
    // one, otherwise the space that follows it (pendingSeparator_). That keeps
    // the result tidy without trimming spaces that the pattern asks for, which
    // matters for Java's 'p' pad (a left-padded field starts with spaces).
    auto dropSeparator() -> void
    {
        // Never eat a space that belongs to an enclosing group or to the
        // literal text before it: only a separator inside the current group may
        // be removed, otherwise the caller's spacing disappears.
        const std::size_t floor = depth_ > 0 ? groups_[depth_ - 1].contentStart : 0;
        if (output_.size() > floor && isSpaceOrTab(output_.back()))
        {
            output_.pop_back();
            pendingSeparator_ = false;
            return;
        }
        // A dropped element at the very start of its group leaves the separator
        // that used to follow it, as in "z HH:mm:ss"; remove that one instead.
        // A separator in the middle of a pattern belongs to the caller's text
        // (as in "HH:mm[ z] UTC") and is kept.
        pendingSeparator_ = output_.size() == floor;
    }

    auto writeLiteral(std::string_view text) -> void
    {
        if (pendingSeparator_)
        {
            pendingSeparator_ = false;
            if (!text.empty() && isSpaceOrTab(text.front()))
            {
                text.remove_prefix(1);
            }
        }
        output_ += text;
    }

    auto writeField(const Token &token) -> void
    {
        pendingSeparator_ = false;
        if (token.pad == 0)
        {
            writeField(token.letter, token.count, output_);
            return;
        }
        // Java's 'p' is a fixed-width pad: the field is rendered first and then
        // left-padded with spaces. A field wider than the pad width is an error
        // in Java too.
        std::string padded;
        writeField(token.letter, token.count, padded);
        if (padded.size() > token.pad)
        {
            throw std::invalid_argument("localisation: padded pattern field is wider than its 'p' pad width");
        }
        output_.append(token.pad - padded.size(), ' ');
        output_ += padded;
    }

    auto requireDate(char letter) const -> void
    {
        if (!parts_.hasDate)
        {
            throw MissingField("localisation: pattern field '" + std::string(1, letter) +
                               "' needs a date, but the value has no date");
        }
    }

    auto requireTime(char letter) const -> void
    {
        if (!parts_.hasTime)
        {
            throw MissingField("localisation: pattern field '" + std::string(1, letter) +
                               "' needs a time, but the value has no time");
        }
    }

    auto yearOfEra() const noexcept -> std::int64_t
    {
        return parts_.year > 0 ? parts_.year : 1 - static_cast<std::int64_t>(parts_.year);
    }

    auto hourOfHalfDay() const noexcept -> int
    {
        const int hour = parts_.hour % 12;
        return hour == 0 ? 12 : hour;
    }

    auto milliOfDay() const noexcept -> std::int64_t
    {
        const std::int64_t seconds =
            static_cast<std::int64_t>(parts_.hour) * 3600 + parts_.minute * 60 + parts_.second;
        return seconds * 1000 + parts_.nanosecond / 1000000;
    }

    auto nanoOfDay() const noexcept -> std::int64_t
    {
        const std::int64_t seconds =
            static_cast<std::int64_t>(parts_.hour) * 3600 + parts_.minute * 60 + parts_.second;
        return seconds * 1000000000 + parts_.nanosecond;
    }

    auto writeYear(std::int64_t value, std::size_t count, std::string &output) -> void
    {
        if (count == 2)
        {
            // Java's reduced two-digit form: the rightmost two digits, padding
            // included, and no sign.
            const std::int64_t magnitude = value < 0 ? -value : value;
            appendNumber(magnitude % 100, 2, SignStyle::notNegative, output);
            return;
        }
        appendNumber(value, count, count < 4 ? SignStyle::normal : SignStyle::exceedsPad, output);
    }

    auto writeQuarter(std::size_t count, std::string &output) -> void
    {
        if (count <= 2)
        {
            appendNumber(parts_.quarter, count, SignStyle::notNegative, output);
            return;
        }
        const cldr::QuarterNames &quarters = data_.quarters;
        const int index = parts_.quarter - 1;
        if (index < 0 || index >= cldr::quarterCount)
        {
            return;
        }
        appendText(output, count == 3 ? quarters.abbreviated[index]
                          : count == 4 ? quarters.wide[index]
                                       : quarters.narrow[index]);
    }

    auto writeMonth(std::size_t count, std::string &output) -> void
    {
        if (count <= 2)
        {
            appendNumber(parts_.month, count, SignStyle::notNegative, output);
            return;
        }
        const cldr::MonthNames &months = data_.months;
        const int index = parts_.month - 1;
        if (index < 0 || index >= cldr::monthCount)
        {
            return;
        }
        appendText(output, count == 3 ? months.abbreviated[index]
                          : count == 4 ? months.wide[index]
                                       : months.narrow[index]);
    }

    auto writeWeekday(char letter, std::size_t count, std::string &output) -> void
    {
        if ((letter == 'e' || letter == 'c') && count <= 2)
        {
            appendNumber(parts_.isoWeekday, count, SignStyle::notNegative, output);
            return;
        }
        const cldr::WeekdayNames &weekdays = data_.weekdays;
        const int index = parts_.isoWeekday % 7; // ISO 1..7 -> Sunday = 0
        if (index < 0 || index >= cldr::weekdayCount)
        {
            return;
        }
        appendText(output, count == 6 ? weekdays.shortName[index]
                          : count == 5 ? weekdays.narrow[index]
                          : count == 4 ? weekdays.wide[index]
                                       : weekdays.abbreviated[index]);
    }

    auto writeEra(std::size_t count, std::string &output) -> void
    {
        const cldr::EraNames &eras = data_.eras;
        const int index = parts_.year > 0 ? 1 : 0;
        appendText(output, count == 5 ? eras.narrow[index] : count == 4 ? eras.wide[index] : eras.abbreviated[index]);
    }

    auto writeFraction(std::size_t count, std::string &output) -> void
    {
        int value = parts_.nanosecond;
        char buffer[9];
        for (int index = 8; index >= 0; --index)
        {
            buffer[index] = static_cast<char>('0' + value % 10);
            value /= 10;
        }
        output.append(buffer, count);
    }

    auto writeDayPeriod(char letter, std::size_t count, std::string &output) -> void
    {
        const cldr::DayPeriodNames &periods = data_.dayPeriods;
        const bool wide = count == 4;
        const bool narrow = count >= 5;
        if (letter == 'b' && parts_.hour == 0)
        {
            appendText(output, wide ? periods.wideMidnight
                              : narrow ? periods.narrowMidnight
                                       : periods.abbreviatedMidnight);
            return;
        }
        if (letter == 'b' && parts_.hour == 12)
        {
            appendText(output, wide ? periods.wideNoon : narrow ? periods.narrowNoon : periods.abbreviatedNoon);
            return;
        }
        const bool morning = parts_.hour < 12;
        appendText(output, wide ? (morning ? periods.wideAm : periods.widePm)
                          : narrow ? (morning ? periods.narrowAm : periods.narrowPm)
                                   : (morning ? periods.abbreviatedAm : periods.abbreviatedPm));
        // "B" (period-of-day) reaches this branch too: without CLDR's
        // dayPeriodRules it degrades to the am/pm vocabulary.
    }

    // The offset styles used by Z, O, X and x. They mirror the pattern strings
    // of Java's OffsetIdPrinterParser: an upper-case component is mandatory, a
    // lower-case one is only printed when it is non-zero.
    enum class OffsetStyle : std::uint8_t
    {
        hourOptionalMinute,      // "+HHmm"
        hourMinute,              // "+HHMM"
        hourMinuteColon,         // "+HH:MM"
        hourMinuteSecond,        // "+HHMMss"
        hourMinuteSecondColon    // "+HH:MM:ss"
    };

    auto writeOffset(char letter, std::size_t count, std::string &output) -> void
    {
        if (!parts_.hasOffset)
        {
            // No offset on the value: behave like an absent zone token, which is
            // what makes "HH:mm:ss[Z]" collapse cleanly.
            dropSeparator();
            return;
        }
        switch (letter)
        {
        case 'Z':
            if (count == 4)
            {
                writeLocalizedOffset(true, output);
            }
            else if (count == 5)
            {
                writeNumericOffset(OffsetStyle::hourMinuteSecondColon, "Z", output);
            }
            else
            {
                writeNumericOffset(OffsetStyle::hourMinute, "+0000", output);
            }
            return;
        case 'O':
            writeLocalizedOffset(count == 4, output);
            return;
        case 'X':
            // Pattern letter X prints 'Z' for a zero offset at every width.
            writeNumericOffset(offsetStyleOf(count), "Z", output);
            return;
        case 'x':
            writeNumericOffset(offsetStyleOf(count), count == 1 ? "+00" : (count % 2 == 0 ? "+0000" : "+00:00"),
                               output);
            return;
        default:
            return;
        }
    }

    static auto offsetStyleOf(std::size_t count) noexcept -> OffsetStyle
    {
        switch (count)
        {
        case 1:
            return OffsetStyle::hourOptionalMinute;
        case 2:
            return OffsetStyle::hourMinute;
        case 3:
            return OffsetStyle::hourMinuteColon;
        case 4:
            return OffsetStyle::hourMinuteSecond;
        default:
            return OffsetStyle::hourMinuteSecondColon;
        }
    }

    auto writeNumericOffset(OffsetStyle style, const char *zeroText, std::string &output) -> void
    {
        const int total = parts_.offsetSeconds;
        if (total == 0)
        {
            output += zeroText;
            return;
        }
        const int magnitude = total < 0 ? -total : total;
        const int hours = magnitude / 3600 % 100;
        const int minutes = magnitude / 60 % 60;
        const int seconds = magnitude % 60;
        const bool mandatoryMinutes = style == OffsetStyle::hourMinute || style == OffsetStyle::hourMinuteColon ||
                                      style == OffsetStyle::hourMinuteSecond ||
                                      style == OffsetStyle::hourMinuteSecondColon;
        const bool withMinutes = mandatoryMinutes || minutes != 0;
        if (hours == 0 && !withMinutes)
        {
            // Java prints the zero text for an offset whose hour and minute both
            // render as zero (for example +00:00:30 with a one-letter X).
            output += zeroText;
            return;
        }
        output.push_back(total < 0 ? '-' : '+');
        appendTwoDigits(output, hours);
        if (!withMinutes)
        {
            return;
        }
        const bool colon = style == OffsetStyle::hourMinuteColon || style == OffsetStyle::hourMinuteSecondColon;
        if (colon)
        {
            output.push_back(':');
        }
        appendTwoDigits(output, minutes);
        const bool optionalSeconds = style == OffsetStyle::hourMinuteSecond ||
                                     style == OffsetStyle::hourMinuteSecondColon;
        if (optionalSeconds && seconds != 0)
        {
            if (colon)
            {
                output.push_back(':');
            }
            appendTwoDigits(output, seconds);
        }
    }

    // Z(4) and O: the localized offset, "GMT+8" (short) or "GMT+08:00" (full).
    // CLDR's localized gmtZeroFormat is not part of the bundled data, so the
    // CLDR root text "GMT" is used.
    auto writeLocalizedOffset(bool full, std::string &output) -> void
    {
        output += "GMT";
        const int total = parts_.offsetSeconds;
        if (total == 0)
        {
            return;
        }
        const int magnitude = total < 0 ? -total : total;
        const int hours = magnitude / 3600 % 100;
        const int minutes = magnitude / 60 % 60;
        const int seconds = magnitude % 60;
        output.push_back(total < 0 ? '-' : '+');
        if (full)
        {
            appendTwoDigits(output, hours);
            output.push_back(':');
            appendTwoDigits(output, minutes);
            if (seconds != 0)
            {
                output.push_back(':');
                appendTwoDigits(output, seconds);
            }
            return;
        }
        if (hours >= 10)
        {
            output.push_back(static_cast<char>('0' + hours / 10));
        }
        output.push_back(static_cast<char>('0' + hours % 10));
        if (minutes != 0 || seconds != 0)
        {
            output.push_back(':');
            appendTwoDigits(output, minutes);
            if (seconds != 0)
            {
                output.push_back(':');
                appendTwoDigits(output, seconds);
            }
        }
    }

    auto writeField(char letter, std::size_t count, std::string &output) -> void
    {
        switch (letter)
        {
        case 'G':
            requireDate(letter);
            writeEra(count, output);
            break;
        case 'u':
            requireDate(letter);
            writeYear(parts_.year, count, output);
            break;
        case 'y':
            requireDate(letter);
            writeYear(yearOfEra(), count, output);
            break;
        case 'Y':
            requireDate(letter);
            writeYear(parts_.weekYear, count, output);
            break;
        case 'Q':
        case 'q':
            requireDate(letter);
            writeQuarter(count, output);
            break;
        case 'M':
        case 'L':
            requireDate(letter);
            writeMonth(count, output);
            break;
        case 'w':
            requireDate(letter);
            appendNumber(parts_.weekOfWeekYear, count, SignStyle::notNegative, output);
            break;
        case 'W':
            requireDate(letter);
            appendNumber(parts_.weekOfMonth, count, SignStyle::notNegative, output);
            break;
        case 'D':
            requireDate(letter);
            appendNumber(parts_.dayOfYear, count, SignStyle::notNegative, output);
            break;
        case 'd':
            requireDate(letter);
            appendNumber(parts_.day, count, SignStyle::notNegative, output);
            break;
        case 'F':
            requireDate(letter);
            appendNumber(parts_.dayOfWeekInMonth, count, SignStyle::notNegative, output);
            break;
        case 'E':
        case 'e':
        case 'c':
            requireDate(letter);
            writeWeekday(letter, count, output);
            break;
        case 'a':
        case 'b':
        case 'B':
            requireTime(letter);
            writeDayPeriod(letter, count, output);
            break;
        case 'h':
            requireTime(letter);
            appendNumber(hourOfHalfDay(), count, SignStyle::notNegative, output);
            break;
        case 'K':
            requireTime(letter);
            appendNumber(parts_.hour % 12, count, SignStyle::notNegative, output);
            break;
        case 'k':
            requireTime(letter);
            appendNumber(parts_.hour == 0 ? 24 : parts_.hour, count, SignStyle::notNegative, output);
            break;
        case 'H':
            requireTime(letter);
            appendNumber(parts_.hour, count, SignStyle::notNegative, output);
            break;
        case 'm':
            requireTime(letter);
            appendNumber(parts_.minute, count, SignStyle::notNegative, output);
            break;
        case 's':
            requireTime(letter);
            appendNumber(parts_.second, count, SignStyle::notNegative, output);
            break;
        case 'S':
            requireTime(letter);
            writeFraction(count, output);
            break;
        case 'A':
            requireTime(letter);
            appendNumber(milliOfDay(), count, SignStyle::notNegative, output);
            break;
        case 'n':
            requireTime(letter);
            appendNumber(parts_.nanosecond, count, SignStyle::notNegative, output);
            break;
        case 'N':
            requireTime(letter);
            appendNumber(nanoOfDay(), count, SignStyle::notNegative, output);
            break;
        case 'z':
        case 'v':
            // Zone *names* need CLDR's timeZoneNames data (see the limitations).
            dropSeparator();
            break;
        case 'V':
            // The zone ID is real data: a temporal::Zone carries its identifier,
            // so a zoned value can print it exactly as Java's V does.
            if (parts_.hasZone && !parts_.zoneId.empty())
            {
                output.append(parts_.zoneId.data(), parts_.zoneId.size());
            }
            else
            {
                dropSeparator();
            }
            break;
        case 'Z':
        case 'O':
        case 'X':
        case 'x':
            writeOffset(letter, count, output);
            break;
        default:
            // Unreachable: compile() validates every field letter.
            break;
        }
    }

    const cldr::LocaleData &data_;
    const DateTimeParts &parts_;
    std::string &output_;
    std::array<Group, maximumGroupDepth> groups_{};
    std::size_t depth_ = 0;
    bool pendingSeparator_ = false;
};

// An immutable compiled pattern: the text as written, plus the tokens the
// renderer walks. Compilation is where invalid patterns are rejected, so a
// formatter can be formatted from any number of threads afterwards.
class Pattern
{
  public:
    Pattern() = default;

    static auto compile(std::string_view text) -> Pattern
    {
        Pattern pattern;
        if (!text.empty())
        {
            pattern.text_.assign(text.data(), text.size());
        }
        pattern.parse();
        return pattern;
    }

    auto text() const noexcept -> std::string_view { return text_; }

    auto render(const cldr::LocaleData &data, const DateTimeParts &parts) const -> std::string
    {
        std::string output;
        output.reserve(text_.size() + 24);
        PatternRenderer renderer(data, parts, output);
        renderer.render(tokens_);
        return output;
    }

  private:
    auto appendLiteral(std::string_view text) -> void
    {
        if (!tokens_.empty() && tokens_.back().kind == TokenKind::literal)
        {
            if (!text.empty())
            {
                tokens_.back().literal.append(text.data(), text.size());
            }
            return;
        }
        Token token;
        token.kind = TokenKind::literal;
        if (!text.empty())
        {
            token.literal.assign(text.data(), text.size());
        }
        tokens_.push_back(std::move(token));
    }

    auto parseQuoted(std::size_t &position) -> void
    {
        ++position; // the opening apostrophe
        std::string literal;
        while (position < text_.size())
        {
            const char character = text_[position];
            if (character == '\'')
            {
                if (position + 1 < text_.size() && text_[position + 1] == '\'')
                {
                    literal.push_back('\'');
                    position += 2;
                    continue;
                }
                ++position;
                if (literal.empty())
                {
                    // "''" is a single apostrophe in Java as well.
                    literal.push_back('\'');
                }
                appendLiteral(literal);
                return;
            }
            literal.push_back(character);
            ++position;
        }
        throw std::invalid_argument("localisation: pattern ends with an unterminated quoted literal");
    }

    auto parseField(std::size_t &position) -> void
    {
        char letter = text_[position];
        std::size_t pad = 0;
        if (letter == 'p')
        {
            const std::size_t padStart = position;
            while (position < text_.size() && text_[position] == 'p')
            {
                ++position;
            }
            if (position >= text_.size() || !isAsciiLetter(text_[position]))
            {
                throw std::invalid_argument(
                    "localisation: pattern letter 'p' must be followed by a field letter");
            }
            pad = position - padStart;
            letter = text_[position];
        }
        const std::size_t start = position;
        ++position;
        while (position < text_.size() && text_[position] == letter)
        {
            ++position;
        }
        const std::size_t count = position - start;
        validateField(letter, count);
        Token token;
        token.kind = TokenKind::field;
        token.letter = letter;
        token.count = count;
        token.pad = pad;
        tokens_.push_back(std::move(token));
    }

    auto parse() -> void
    {
        std::array<char, maximumGroupDepth> groups{};
        std::size_t depth = 0;
        std::size_t position = 0;
        while (position < text_.size())
        {
            const char character = text_[position];
            if (character == '\'')
            {
                parseQuoted(position);
                continue;
            }
            if (character == '[' || character == '(')
            {
                if (depth == maximumGroupDepth)
                {
                    throw std::invalid_argument("localisation: optional sections are nested too deeply");
                }
                Token token;
                token.kind = character == '[' ? TokenKind::optionalStart : TokenKind::parenthesisStart;
                tokens_.push_back(std::move(token));
                groups[depth] = character;
                ++depth;
                ++position;
                continue;
            }
            if (character == ']' || character == ')')
            {
                const char opening = character == ']' ? '[' : '(';
                if (depth > 0 && groups[depth - 1] == opening)
                {
                    --depth;
                    Token token;
                    token.kind = character == ']' ? TokenKind::optionalEnd : TokenKind::parenthesisEnd;
                    tokens_.push_back(std::move(token));
                }
                else if (character == ']')
                {
                    // Java rejects an unmatched ']'; a stray ')' stays literal
                    // because parentheses are not structural in Java.
                    throw std::invalid_argument("localisation: pattern has a ']' without a matching '['");
                }
                else
                {
                    appendLiteral(")");
                }
                ++position;
                continue;
            }
            if (character == '{' || character == '}' || character == '#')
            {
                throw std::invalid_argument("localisation: pattern includes the reserved character '" +
                                            std::string(1, character) + "'");
            }
            if (isAsciiLetter(character))
            {
                parseField(position);
                continue;
            }
            // Any other character is literal text; a whole run becomes one
            // token so that tokenising stays proportional to the pattern.
            const std::size_t start = position;
            while (position < text_.size())
            {
                const char current = text_[position];
                if (current == '\'' || current == '[' || current == ']' || current == '(' || current == ')' ||
                    current == '{' || current == '}' || current == '#' || isAsciiLetter(current))
                {
                    break;
                }
                ++position;
            }
            appendLiteral(std::string_view(text_).substr(start, position - start));
        }
        for (std::size_t index = 0; index < depth; ++index)
        {
            if (groups[index] == '[')
            {
                throw std::invalid_argument("localisation: pattern has a '[' without a matching ']'");
            }
        }
    }

    std::string text_;
    std::vector<Token> tokens_;
};

// ---------------------------------------------------------------------------
// Locale data lookup and the localized patterns
// ---------------------------------------------------------------------------

// Replaces CLDR's {1} (date) and {0} (time) placeholders of a date-time glue
// pattern with the date and time patterns, so a combined value is rendered in a
// single pass. That keeps quoting, optional groups and the locale's date/time
// order intact (fr glue "{1} {0}", ar glue "{1}\u060C {0}", ...).
inline auto composePatterns(std::string_view glue, std::string_view datePattern, std::string_view timePattern)
    -> std::string
{
    std::string pattern;
    pattern.reserve(glue.size() + datePattern.size() + timePattern.size());
    bool replaced = false;
    std::size_t position = 0;
    while (position < glue.size())
    {
        if (position + 2 < glue.size() && glue[position] == '{' && glue[position + 2] == '}')
        {
            const char selector = glue[position + 1];
            if (selector == '0')
            {
                pattern.append(timePattern);
                position += 3;
                replaced = true;
                continue;
            }
            if (selector == '1')
            {
                pattern.append(datePattern);
                position += 3;
                replaced = true;
                continue;
            }
        }
        pattern.push_back(glue[position]);
        ++position;
    }
    if (!replaced)
    {
        // No usable glue pattern (a CLDR row without dateTimeFormats): fall back
        // to the conventional date-then-time order.
        pattern.assign(datePattern);
        if (!pattern.empty() && !timePattern.empty())
        {
            pattern.push_back(' ');
        }
        pattern.append(timePattern);
    }
    return pattern;
}

inline auto findByTag(std::string_view tag) noexcept -> const cldr::LocaleData *
{
    for (std::size_t index = 0; index < cldr::localeCount; ++index)
    {
        if (tag == cldr::localeTable[index].tag)
        {
            return &cldr::localeTable[index];
        }
    }
    return nullptr;
}

// CLDR-style lookup chain: full tag, language-script, language, then English.
inline auto findLocaleData(const Locale &locale) -> const cldr::LocaleData *
{
    if (const cldr::LocaleData *found = findByTag(locale.tag()))
    {
        return found;
    }
    if (!locale.script().empty())
    {
        const std::string languageAndScript = locale.language() + "-" + locale.script();
        if (const cldr::LocaleData *found = findByTag(languageAndScript))
        {
            return found;
        }
    }
    if (const cldr::LocaleData *found = findByTag(locale.language()))
    {
        return found;
    }
    if (const cldr::LocaleData *found = findByTag("en"))
    {
        return found;
    }
    // The generator always writes "en" as the first row, so this is the last
    // resort rather than a reachable miss.
    return cldr::localeTable;
}

// The three localized forms, resolved to the concrete CLDR pattern of a
// locale. DateTimeFormatter stores this text (and the styles it came from) so
// that withLocale can re-resolve it.
enum class LocalizedKind : std::uint8_t
{
    date,
    time,
    dateTime
};

inline auto localizedPatternText(LocalizedKind kind, FormatStyle dateStyle, FormatStyle timeStyle,
                                 const Locale &locale) -> std::string
{
    const cldr::LocaleData &data = *findLocaleData(locale);
    const std::size_t dateIndex = styleIndex(dateStyle);
    const std::size_t timeIndex = styleIndex(timeStyle);
    switch (kind)
    {
    case LocalizedKind::date:
        return data.patterns.date[dateIndex];
    case LocalizedKind::time:
        return data.patterns.time[timeIndex];
    case LocalizedKind::dateTime:
        return composePatterns(data.patterns.dateTime[dateIndex], data.patterns.date[dateIndex],
                               data.patterns.time[timeIndex]);
    }
    return std::string();
}

// ---------------------------------------------------------------------------
// Field-taking helpers shared with the legacy free functions
// ---------------------------------------------------------------------------

inline auto dateFromFields(int year, int month, int day) -> temporal::LocalGregoryDate
{
    if (month < 1 || month > 12)
    {
        throw std::invalid_argument("localisation: month outside 1..12");
    }
    return temporal::gregory::Date::of(temporal::gregory::Year(static_cast<std::int32_t>(year)),
                                       static_cast<temporal::gregory::Month>(month), day);
}

inline auto timeFromFields(int hour, int minute, int second, int nanosecond = 0) -> temporal::LocalGregoryTime
{
    return temporal::LocalGregoryTime::of(hour, minute, second, nanosecond);
}

} // namespace detail

// ===========================================================================
// DateTimeFormatter
//
// The C++ counterpart of java.time.format.DateTimeFormatter. It is an
// immutable value: all formatting state (pattern, locale, localized style) is
// const, the with* operations return copies, and copies share nothing mutable,
// so concurrent formatting needs no locking.
//
// Three deliberate C++ additions follow from C++ not having Java's
// TemporalAccessor hierarchy: format(const OffsetGregoryDateTime &) and
// format(const ZonedGregoryDateTime &) make a real offset (and, for the zoned
// value, a zone identifier) available to Z, O, X, x and V, while formatDate /
// formatTime accept proleptic Gregorian fields directly so that an out-of-range
// month, day or clock field is reported as std::invalid_argument at the call
// site.
// ===========================================================================
class DateTimeFormatter
{
  public:
    DateTimeFormatter() = delete;

    // Compiles a pattern for a locale. Throws std::invalid_argument for an
    // unknown pattern letter, a reserved character, an unterminated quoted
    // literal, an unbalanced '[' or an unsupported repetition count.
    static auto ofPattern(std::string_view pattern, const Locale &locale) -> DateTimeFormatter
    {
        return DateTimeFormatter(detail::Pattern::compile(pattern), locale);
    }

    // Java's ofPattern(String) uses the default locale; this library has no
    // process-wide locale, so the neutral default is English.
    static auto ofPattern(std::string_view pattern) -> DateTimeFormatter
    {
        return ofPattern(pattern, Locale("en"));
    }

    // The localized formatters start in English and re-resolve their CLDR
    // pattern when withLocale changes the locale.
    static auto ofLocalizedDate(FormatStyle style) -> DateTimeFormatter
    {
        return DateTimeFormatter(detail::LocalizedKind::date, style, FormatStyle::medium, Locale("en"));
    }

    static auto ofLocalizedTime(FormatStyle style) -> DateTimeFormatter
    {
        return DateTimeFormatter(detail::LocalizedKind::time, FormatStyle::medium, style, Locale("en"));
    }

    static auto ofLocalizedDateTime(FormatStyle dateStyle, FormatStyle timeStyle) -> DateTimeFormatter
    {
        return DateTimeFormatter(detail::LocalizedKind::dateTime, dateStyle, timeStyle, Locale("en"));
    }

    static auto ofLocalizedDateTime(FormatStyle style) -> DateTimeFormatter
    {
        return ofLocalizedDateTime(style, style);
    }

    // Returns a copy of this formatter bound to another locale. A
    // pattern-based formatter keeps its pattern; a localized one looks its
    // pattern up again, exactly like Java's lazily localized formatters.
    auto withLocale(const Locale &locale) const -> DateTimeFormatter
    {
        if (!localized_)
        {
            return DateTimeFormatter(pattern_, locale);
        }
        return DateTimeFormatter(localizedKind_, dateStyle_, timeStyle_, locale);
    }

    // The pattern text this formatter renders. For a localized formatter this
    // is the CLDR pattern resolved for the current locale.
    auto pattern() const noexcept -> std::string_view { return pattern_.text(); }

    auto locale() const noexcept -> const Locale & { return locale_; }

    auto format(const temporal::LocalGregoryDate &value) const -> std::string
    {
        return render(detail::partsOf(value));
    }

    auto format(const temporal::LocalGregoryTime &value) const -> std::string
    {
        return render(detail::partsOf(value));
    }

    auto format(const temporal::LocalGregoryDateTime &value) const -> std::string
    {
        return render(detail::partsOf(value));
    }

    // C++ addition: a value that carries a real UTC offset, so that Z, O, X
    // and x have something to print.
    auto format(const temporal::OffsetGregoryDateTime &value) const -> std::string
    {
        return render(detail::partsOf(value));
    }

    // C++ addition: a zoned value, so that V prints the temporal::Zone
    // identifier (z and v still need CLDR zone names; see the limitations).
    auto format(const temporal::ZonedGregoryDateTime &value) const -> std::string
    {
        return render(detail::partsOf(value));
    }

    // C++ addition: field-taking conveniences. An invalid date (month 13,
    // 30 February) or time (24:00) throws std::invalid_argument, which is the
    // same type the temporal value types throw when they reject those fields.
    auto formatDate(int year, int month, int day) const -> std::string
    {
        return format(detail::dateFromFields(year, month, day));
    }

    auto formatTime(int hour, int minute, int second, int nanosecond = 0) const -> std::string
    {
        return format(detail::timeFromFields(hour, minute, second, nanosecond));
    }

    // Java's toString prints a description of a formatter; here pattern() is
    // the only stable identity a localized formatter has, so toString returns
    // the resolved pattern.
    auto toString() const -> std::string { return std::string(pattern_.text()); }

  private:
    DateTimeFormatter(detail::Pattern pattern, const Locale &locale)
        : localized_(false), locale_(locale), pattern_(std::move(pattern))
    {
    }

    DateTimeFormatter(detail::LocalizedKind kind, FormatStyle dateStyle, FormatStyle timeStyle, const Locale &locale)
        : localized_(true),
          localizedKind_(kind),
          dateStyle_(dateStyle),
          timeStyle_(timeStyle),
          locale_(locale),
          pattern_(detail::Pattern::compile(detail::localizedPatternText(kind, dateStyle, timeStyle, locale)))
    {
    }

    auto render(const detail::DateTimeParts &parts) const -> std::string
    {
        return pattern_.render(*detail::findLocaleData(locale_), parts);
    }

    bool localized_ = false;
    detail::LocalizedKind localizedKind_ = detail::LocalizedKind::date;
    FormatStyle dateStyle_ = FormatStyle::medium;
    FormatStyle timeStyle_ = FormatStyle::medium;
    Locale locale_;
    detail::Pattern pattern_;
};

// ---------------------------------------------------------------------------
// The original free functions, kept as thin wrappers
//
// Their signatures and observable results are unchanged. They now go through
// DateTimeFormatter, so pattern validation, the field-availability rules and
// the localized styles behave identically whether callers use the class or
// these helpers.
// ---------------------------------------------------------------------------

// Formats a date with the CLDR date pattern of the requested style.
inline auto formatDate(const temporal::LocalGregoryDate &date, const Locale &locale,
                       DateStyle style = DateStyle::medium) -> std::string
{
    return DateTimeFormatter::ofLocalizedDate(detail::formatStyleOf(style)).withLocale(locale).format(date);
}

// Formats a date given as proleptic Gregorian fields. Throws std::invalid_argument
// for a month outside 1..12 or a day that does not exist in that month.
inline auto formatDate(int year, int month, int day, const Locale &locale, DateStyle style = DateStyle::medium)
    -> std::string
{
    return formatDate(detail::dateFromFields(year, month, day), locale, style);
}

// Formats a time of day with the CLDR time pattern of the requested style.
inline auto formatTime(const temporal::LocalGregoryTime &time, const Locale &locale,
                       TimeStyle style = TimeStyle::medium) -> std::string
{
    return DateTimeFormatter::ofLocalizedTime(detail::formatStyleOf(style)).withLocale(locale).format(time);
}

// Formats a time given as wall-clock fields. Throws std::invalid_argument when
// an hour, minute or second is out of range.
inline auto formatTime(int hour, int minute, int second, const Locale &locale,
                       TimeStyle style = TimeStyle::medium) -> std::string
{
    return formatTime(detail::timeFromFields(hour, minute, second), locale, style);
}

// Formats a date-time by rendering the locale's CLDR date-time glue pattern with
// the date pattern of dateStyle and the time pattern of timeStyle substituted,
// which is how CLDR composes the two (the glue row is selected by the date
// style).
inline auto formatDateTime(const temporal::LocalGregoryDateTime &value, const Locale &locale,
                           DateStyle dateStyle = DateStyle::medium, TimeStyle timeStyle = TimeStyle::medium)
    -> std::string
{
    return DateTimeFormatter::ofLocalizedDateTime(detail::formatStyleOf(dateStyle), detail::formatStyleOf(timeStyle))
        .withLocale(locale)
        .format(value);
}

// Formats a date-time given as proleptic Gregorian fields.
inline auto formatDateTime(int year, int month, int day, int hour, int minute, int second, const Locale &locale,
                           DateStyle dateStyle = DateStyle::medium, TimeStyle timeStyle = TimeStyle::medium)
    -> std::string
{
    const temporal::LocalGregoryDateTime value(detail::dateFromFields(year, month, day),
                                               detail::timeFromFields(hour, minute, second));
    return formatDateTime(value, locale, dateStyle, timeStyle);
}

// ---------------------------------------------------------------------------
// Pattern escape hatch
//
// The three functions below format an arbitrary pattern for one value. They
// are the way to test or use individual pattern letters (for example "EEEE")
// without adding a style to the table; they compile the pattern on every call.
// ---------------------------------------------------------------------------

inline auto formatDatePattern(std::string_view pattern, const temporal::LocalGregoryDate &date,
                              const Locale &locale) -> std::string
{
    return DateTimeFormatter::ofPattern(pattern, locale).format(date);
}

inline auto formatTimePattern(std::string_view pattern, const temporal::LocalGregoryTime &time,
                              const Locale &locale) -> std::string
{
    return DateTimeFormatter::ofPattern(pattern, locale).format(time);
}

inline auto formatDateTimePattern(std::string_view pattern, const temporal::LocalGregoryDateTime &value,
                                  const Locale &locale) -> std::string
{
    return DateTimeFormatter::ofPattern(pattern, locale).format(value);
}

} // namespace localisation
