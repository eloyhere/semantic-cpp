#!/usr/bin/env python3
"""Generate localisation/datetime_data.h from Unicode CLDR Gregorian calendar data.

The table holds the CLDR data that locale-aware date and time formatting needs:

  * the short/medium/long/full date, time and date-time patterns,
  * month names (wide, abbreviated, narrow),
  * quarter names (wide, abbreviated, narrow),
  * weekday names (wide, abbreviated, short, narrow),
  * day period names (am, pm, midnight, noon in three widths),
  * era names (abbreviated, wide, narrow).

Input is the ``ca-gregorian.json`` file of every locale, as published by the
``cldr-json`` project:

  https://raw.githubusercontent.com/unicode-org/cldr-json/main/cldr-json/cldr-dates-full/main/<locale>/ca-gregorian.json

The default input directory is ``%TEMP%/i18n-data/cldr/<locale>/ca-gregorian.json``
(``$TMPDIR`` on POSIX); pass ``--input`` to point at a different tree.

Regenerate with:

  python tools/gen_datetime_data.py

Pass ``--check`` to verify that the checked-in header is exactly what the
generator would write, without touching the file (useful in CI). Pass
``--input`` to point at a different CLDR tree.

The output is deterministic: the locale list is fixed, JSON keys are visited in
a fixed order, and every string is emitted as an ASCII-only C++ literal
(non-ASCII text becomes ``\\xNN`` UTF-8 bytes), so the header compiles the same
way under MSVC, GCC and Clang regardless of the compiler's source and execution
character sets.
"""

import argparse
import json
import os
import sys
import tempfile

# Locales in the table. "en" is first on purpose: it is the terminal fallback in
# localisation/datetime.h, which indexes the first row directly when no tag and
# no language matches.
LOCALES = [
    "en",
    "ar",
    "de",
    "es",
    "fr",
    "he",
    "hi",
    "it",
    "ja",
    "ko",
    "pt",
    "ru",
    "tr",
    "zh",
    "zh-Hant",
]

# CLDR release the checked-in data was generated from.
CLDR_VERSION = "49.0.0"

STYLES = ["short", "medium", "long", "full"]
MONTH_KEYS = [str(number) for number in range(1, 13)]
QUARTER_KEYS = [str(number) for number in range(1, 5)]
WEEKDAY_KEYS = ["sun", "mon", "tue", "wed", "thu", "fri", "sat"]
PERIOD_KEYS = ["am", "pm", "midnight", "noon"]
MONTH_WIDTHS = ["wide", "abbreviated", "narrow"]
QUARTER_WIDTHS = ["wide", "abbreviated", "narrow"]
WEEKDAY_WIDTHS = ["wide", "abbreviated", "short", "narrow"]
PERIOD_WIDTHS = ["abbreviated", "wide", "narrow"]
ERA_WIDTHS = [("abbreviated", "eraAbbr"), ("wide", "eraNames"), ("narrow", "eraNarrow")]


def empty_names():
    """An all-empty name table, used as the English baseline and as a fallback."""
    return {
        "months": {width: [""] * 12 for width in MONTH_WIDTHS},
        "quarters": {width: [""] * 4 for width in QUARTER_WIDTHS},
        "weekdays": {width: [""] * 7 for width in WEEKDAY_WIDTHS},
        "periods": {width: [""] * 4 for width in PERIOD_WIDTHS},
        "eras": {width: ["", ""] for width, _ in ERA_WIDTHS},
        "patterns": {"date": [""] * 4, "time": [""] * 4, "dateTime": [""] * 4},
    }


def load_calendar(directory, locale):
    """Reads one locale's Gregorian calendar node."""
    path = os.path.join(directory, locale, "ca-gregorian.json")
    with open(path, encoding="utf-8") as handle:
        document = json.load(handle)
    return document["main"][locale]["dates"]["calendars"]["gregorian"]


def lookup(calendar, path):
    """Walks a nested dict and returns a string leaf, or an empty string."""
    node = calendar
    for step in path:
        if not isinstance(node, dict) or step not in node:
            return ""
        node = node[step]
    return node if isinstance(node, str) else ""


def series(calendar, section, width, keys):
    return [lookup(calendar, (section, "format", width, key)) for key in keys]


def period_series(calendar, width):
    """Reads day period names; ``format`` first, then ``stand-alone``."""
    values = []
    for key in PERIOD_KEYS:
        value = lookup(calendar, ("dayPeriods", "format", width, key))
        if not value:
            value = lookup(calendar, ("dayPeriods", "stand-alone", width, key))
        values.append(value)
    return values


def collect(calendar, english):
    """Normalises one locale, filling gaps from the English baseline."""
    months = {}
    for width in MONTH_WIDTHS:
        values = series(calendar, "months", width, MONTH_KEYS)
        months[width] = [value or english["months"][width][index] for index, value in enumerate(values)]

    quarters = {}
    for width in QUARTER_WIDTHS:
        values = series(calendar, "quarters", width, QUARTER_KEYS)
        quarters[width] = [value or english["quarters"][width][index] for index, value in enumerate(values)]

    weekdays = {}
    for width in WEEKDAY_WIDTHS:
        values = series(calendar, "days", width, WEEKDAY_KEYS)
        fallback = english["weekdays"][width]
        weekdays[width] = [value or fallback[index] for index, value in enumerate(values)]
    for width in WEEKDAY_WIDTHS:
        # A locale without a width (for example without short weekday names)
        # borrows its abbreviated names before falling back to English.
        if not all(weekdays[width]):
            weekdays[width] = [value or weekdays["abbreviated"][index] for index, value in enumerate(weekdays[width])]

    periods = {}
    for width in PERIOD_WIDTHS:
        values = period_series(calendar, width)
        periods[width] = [value or english["periods"][width][index] for index, value in enumerate(values)]

    eras = {}
    for width, section in ERA_WIDTHS:
        values = [lookup(calendar, ("eras", section, "0")), lookup(calendar, ("eras", section, "1"))]
        eras[width] = [value or english["eras"][width][index] for index, value in enumerate(values)]

    patterns = {
        "date": [lookup(calendar, ("dateFormats", style)) for style in STYLES],
        "time": [lookup(calendar, ("timeFormats", style)) for style in STYLES],
        "dateTime": [lookup(calendar, ("dateTimeFormats", style)) for style in STYLES],
    }
    return {
        "months": months,
        "quarters": quarters,
        "weekdays": weekdays,
        "periods": periods,
        "eras": eras,
        "patterns": patterns,
    }


def cpp_string(text):
    """Formats a Python string as an ASCII-only C++ string literal."""
    chunks = []
    current = []
    after_escape = False
    for character in text:
        code = ord(character)
        if character == '"':
            current.append('\\"')
            after_escape = False
        elif character == "\\":
            current.append("\\\\")
            after_escape = False
        elif character == "\n":
            current.append("\\n")
            after_escape = False
        elif character == "\t":
            current.append("\\t")
            after_escape = False
        elif 0x20 <= code < 0x7F:
            # A literal hex digit directly after a \xNN escape would be absorbed
            # into the escape, so close the literal and start a new one.
            if after_escape and character in "0123456789abcdefABCDEF":
                chunks.append('"' + "".join(current) + '"')
                current = []
            current.append(character)
            after_escape = False
        else:
            for byte in character.encode("utf-8"):
                current.append("\\x%02X" % byte)
            after_escape = True
    chunks.append('"' + "".join(current) + '"')
    return " ".join(chunks)


def joined(values):
    return ", ".join(cpp_string(value) for value in values)


def emit_header(rows, version):
    """Builds the generated header as a list of lines."""
    lines = []
    add = lines.append

    add("// ===========================================================================")
    add("// localisation/datetime_data.h")
    add("//")
    add("// GENERATED FILE - do not edit by hand.")
    add("//")
    add("// Generator  : tools/gen_datetime_data.py")
    add("// Source     : Unicode CLDR %s (cldr-json %s)" % (version, version))
    add("// Input      : ca-gregorian.json of %d locales" % len(rows))
    add("//")
    add("// The generator reads one CLDR ``ca-gregorian.json`` file per locale from")
    add("//")
    add("//   https://github.com/unicode-org/cldr-json/tree/main/cldr-json/cldr-dates-full/main")
    add("//")
    add("// and writes the date/time patterns, month names, quarter names, weekday names,")
    add("// day periods and era names that localisation/datetime.h needs. Strings are stored")
    add("// as UTF-8 bytes in plain C string literals, so this file stays pure ASCII and")
    add("// the runtime representation is UTF-8.")
    add("//")
    add("// Regenerate with:")
    add("//")
    add("//   python tools/gen_datetime_data.py")
    add("//")
    add("// or point it at another CLDR tree with ``--input <dir>``.")
    add("//")
    add("// Layout:")
    add("//")
    add("//   * pattern arrays are indexed like DateStyle/TimeStyle:")
    add("//     0 = short, 1 = medium, 2 = long, 3 = full;")
    add("//   * month arrays run January (0) .. December (11);")
    add("//   * quarter arrays run Q1 (0) .. Q4 (3);")
    add("//   * weekday arrays run Sunday (0) .. Saturday (6), the CLDR order;")
    add("//   * dateTime patterns keep CLDR's {1} = date, {0} = time placeholders;")
    add("//   * rows start with \"en\", the terminal fallback of the lookup chain.")
    add("// ===========================================================================")
    add("")
    add("#pragma once")
    add("")
    add("#include <cstddef>")
    add("")
    add("namespace localisation::detail::cldr")
    add("{")
    add("")
    add("// CLDR release the table was generated from.")
    add('inline constexpr const char *version = "%s";' % version)
    add("")
    add("inline constexpr int monthCount = 12;")
    add("inline constexpr int quarterCount = 4;")
    add("inline constexpr int weekdayCount = 7;")
    add("inline constexpr int styleCount = 4;")
    add("")
    add("// Month names in the CLDR format context (the one date patterns use).")
    add("struct MonthNames")
    add("{")
    add("    const char *wide[monthCount];")
    add("    const char *abbreviated[monthCount];")
    add("    const char *narrow[monthCount];")
    add("};")
    add("")
    add("// Quarter names, first quarter first, also from the format context.")
    add("struct QuarterNames")
    add("{")
    add("    const char *wide[quarterCount];")
    add("    const char *abbreviated[quarterCount];")
    add("    const char *narrow[quarterCount];")
    add("};")
    add("")
    add("// Weekday names, Sunday first.")
    add("struct WeekdayNames")
    add("{")
    add("    const char *wide[weekdayCount];")
    add("    const char *abbreviated[weekdayCount];")
    add("    const char *shortName[weekdayCount];")
    add("    const char *narrow[weekdayCount];")
    add("};")
    add("")
    add("// AM/PM and the fixed midnight/noon day periods, in three widths.")
    add("struct DayPeriodNames")
    add("{")
    add("    const char *abbreviatedAm;")
    add("    const char *abbreviatedPm;")
    add("    const char *wideAm;")
    add("    const char *widePm;")
    add("    const char *narrowAm;")
    add("    const char *narrowPm;")
    add("    const char *abbreviatedMidnight;")
    add("    const char *abbreviatedNoon;")
    add("    const char *wideMidnight;")
    add("    const char *wideNoon;")
    add("    const char *narrowMidnight;")
    add("    const char *narrowNoon;")
    add("};")
    add("")
    add("// Era names: index 0 is BCE, index 1 is CE.")
    add("struct EraNames")
    add("{")
    add("    const char *abbreviated[2];")
    add("    const char *wide[2];")
    add("    const char *narrow[2];")
    add("};")
    add("")
    add("// The CLDR date/time/date-time patterns, indexed by style.")
    add("struct Patterns")
    add("{")
    add("    const char *date[styleCount];")
    add("    const char *time[styleCount];")
    add("    const char *dateTime[styleCount];")
    add("};")
    add("")
    add("// One locale row. ``tag`` is the canonical BCP 47 tag of the CLDR locale.")
    add("struct LocaleData")
    add("{")
    add("    const char *tag;")
    add("    Patterns patterns;")
    add("    MonthNames months;")
    add("    QuarterNames quarters;")
    add("    WeekdayNames weekdays;")
    add("    DayPeriodNames dayPeriods;")
    add("    EraNames eras;")
    add("};")
    add("")
    add("inline constexpr LocaleData localeTable[] = {")

    for tag, value in rows:
        add("    {")
        add('        "%s",' % tag)
        add("        // date / time / date-time patterns: short, medium, long, full")
        add("        {")
        for key in ("date", "time", "dateTime"):
            add("            {%s}," % joined(value["patterns"][key]))
        add("        },")
        add("        // month names: wide, abbreviated, narrow")
        add("        {")
        for width in MONTH_WIDTHS:
            add("            {%s}," % joined(value["months"][width]))
        add("        },")
        add("        // quarter names: wide, abbreviated, narrow (Q1 .. Q4)")
        add("        {")
        for width in QUARTER_WIDTHS:
            add("            {%s}," % joined(value["quarters"][width]))
        add("        },")
        add("        // weekday names: wide, abbreviated, short, narrow (Sunday first)")
        add("        {")
        for width in WEEKDAY_WIDTHS:
            add("            {%s}," % joined(value["weekdays"][width]))
        add("        },")
        add("        // day periods: am/pm in three widths, then midnight/noon")
        add("        {")
        add("            %s," % joined([value["periods"][width][index] for width in PERIOD_WIDTHS for index in (0, 1)]))
        add("            %s," % joined([value["periods"][width][index] for width in PERIOD_WIDTHS for index in (2, 3)]))
        add("        },")
        add("        // era names: BCE, CE")
        add("        {")
        for width in ("abbreviated", "wide", "narrow"):
            add("            {%s}," % joined(value["eras"][width]))
        add("        },")
        add("    },")

    add("};")
    add("")
    add("inline constexpr std::size_t localeCount = sizeof(localeTable) / sizeof(localeTable[0]);")
    add("")
    add("} // namespace localisation::detail::cldr")
    add("")
    return lines


def main():
    script_directory = os.path.dirname(os.path.abspath(__file__))
    repository = os.path.dirname(script_directory)

    parser = argparse.ArgumentParser(description="Generate localisation/datetime_data.h from CLDR data.")
    parser.add_argument(
        "--input",
        default=os.environ.get("CLDR_DATA_DIR", os.path.join(tempfile.gettempdir(), "i18n-data", "cldr")),
        help="directory holding <locale>/ca-gregorian.json files",
    )
    parser.add_argument(
        "--output",
        default=os.path.join(repository, "localisation", "datetime_data.h"),
        help="generated header path",
    )
    parser.add_argument("--version", default=CLDR_VERSION, help="CLDR version recorded in the header")
    parser.add_argument(
        "--check",
        action="store_true",
        help="verify that --output is up to date instead of rewriting it",
    )
    arguments = parser.parse_args()

    english = collect(load_calendar(arguments.input, "en"), empty_names())
    rows = [("en", english)]
    for locale in LOCALES:
        if locale != "en":
            rows.append((locale, collect(load_calendar(arguments.input, locale), english)))

    text = "\n".join(emit_header(rows, arguments.version))
    if arguments.check:
        try:
            with open(arguments.output, encoding="utf-8", newline="\n") as handle:
                existing = handle.read()
        except FileNotFoundError:
            sys.exit("check failed: %s does not exist; run without --check" % arguments.output)
        if existing != text:
            sys.exit("check failed: %s is out of date; run without --check" % arguments.output)
        print("checked %s (%d locales, CLDR %s): up to date" % (arguments.output, len(rows), arguments.version))
        return

    with open(arguments.output, "w", encoding="utf-8", newline="\n") as handle:
        handle.write(text)

    size = os.path.getsize(arguments.output)
    missing = [tag for tag, value in rows if not all(value["patterns"]["date"]) or not all(value["patterns"]["time"])]
    if missing:
        sys.exit("missing CLDR date/time patterns for: %s" % ", ".join(missing))
    print("wrote %s (%d bytes, %d locales, CLDR %s)" % (arguments.output, size, len(rows), arguments.version))
    if size > 1500000:
        sys.exit("generated header exceeds the 1.5 MB budget")


if __name__ == "__main__":
    main()
