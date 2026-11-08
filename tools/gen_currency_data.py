#!/usr/bin/env python3
"""Regenerate ``localisation/currency_data.h`` from the CLDR JSON distribution.

The library stores no locale data in executable code: every currency symbol,
currency pattern, fraction-digit count and compact-decimal pattern lives in
the generated table.  This script is the single source of truth for that
table, so regenerating it is a mechanical, reviewable step.

Usage::

    python tools/gen_currency_data.py
    python tools/gen_currency_data.py --cldr /path/to/cldr --out localisation/currency_data.h

The default CLDR directory is ``%TEMP%/i18n-data/cldr`` on Windows and
``$TMPDIR/i18n-data/cldr`` elsewhere; each locale needs ``numbers.json`` and
``currencies.json``.  ``supplemental/currencyData.json`` is needed for the
per-currency fraction digits and is downloaded on demand when it is missing.

The generated header is pure ASCII: every non-ASCII code point is written as
a C++ universal character name (``\\u00a0``), so the table is independent of
the compiler's source and execution character sets.
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import urllib.request
from pathlib import Path

# Locales with a hand-picked, test-covered data set.  The first entry is the
# base locale: its currency symbols and fraction digits form the fallback
# table every other locale is diffed against.
LOCALES = ["en", "de", "fr", "es", "it", "pt", "ru", "tr", "ar", "he", "hi", "ja", "ko", "zh", "zh-Hant"]

# Compact decimal magnitudes, in powers of ten.  Index i is 10 ** (3 + i).
MAGNITUDES = [10 ** exponent for exponent in range(3, 15)]

# CLDR plural categories, in the order of localisation::PluralCategory.
CATEGORIES = ["zero", "one", "two", "few", "many", "other"]

SUPPLEMENTAL_URL = (
    "https://raw.githubusercontent.com/unicode-org/cldr-json/main/"
    "cldr-json/cldr-core/supplemental/currencyData.json"
)


def default_cldr_root() -> Path:
    base = os.environ.get("I18N_DATA") or os.path.join(
        os.environ.get("TEMP") or os.environ.get("TMPDIR") or "/tmp", "i18n-data"
    )
    return Path(base) / "cldr"


def read_json(path: Path):
    with path.open(encoding="utf-8") as handle:
        return json.load(handle)


def load_numbers(cldr: Path, locale: str) -> dict:
    return read_json(cldr / locale / "numbers.json")["main"][locale]["numbers"]


def load_currencies(cldr: Path, locale: str) -> dict:
    return read_json(cldr / locale / "currencies.json")["main"][locale]["numbers"]["currencies"]


def numbered_section(numbers: dict, family: str) -> dict:
    """Return ``<family>-numberSystem-<system>`` preferring the Latin system."""
    prefix = family + "-numberSystem-"
    for key in sorted(numbers):
        if key.startswith(prefix) and key.endswith("latn"):
            return numbers[key]
    for key in sorted(numbers):
        if key.startswith(prefix):
            return numbers[key]
    raise KeyError(family)


def load_fraction_digits(cldr: Path) -> tuple[dict, str]:
    """Per-currency fraction digits from supplemental currencyData."""
    path = cldr / "supplemental" / "currencyData.json"
    if not path.exists():
        path.parent.mkdir(parents=True, exist_ok=True)
        print(f"downloading {SUPPLEMENTAL_URL}")
        with urllib.request.urlopen(SUPPLEMENTAL_URL, timeout=60) as response:
            path.write_bytes(response.read())

    data = read_json(path)
    supplemental = data["supplemental"]
    version = supplemental.get("version", {})
    fractions = supplemental["currencyData"]["fractions"]

    digits = {}
    for code, entry in fractions.items():
        value = entry.get("_digits")
        digits[code] = int(value) if value is not None else 2
    return digits, str(version.get("_cldrVersion", "unknown"))


def split_currency_pattern(pattern: str) -> tuple[str, str]:
    """Split ``positive;negative`` on the first unquoted semicolon."""
    quoted = False
    for index, character in enumerate(pattern):
        if character == "'":
            quoted = not quoted
        elif character == ";" and not quoted:
            return pattern[:index], pattern[index + 1 :]
    return pattern, ""


def first_number_digits(pattern: str) -> int:
    """Number of ``0`` integer placeholders in the first digit run."""
    run = 0
    started = False
    for character in pattern:
        if character in "0#":
            started = True
            if character == "0":
                run += 1
        elif started and character in ",.":
            started = False
            break
        elif started:
            break
    return max(run, 1)


def compact_table(numbers: dict, form: str) -> dict:
    """Return ``{magnitude: {category: pattern}}`` for one compact form."""
    section = numbered_section(numbers, "decimalFormats")
    form_data = section.get(form, {})
    patterns = form_data.get("decimalFormat", {})

    table = {magnitude: {} for magnitude in MAGNITUDES}
    for key, pattern in patterns.items():
        if "-count-" not in key:
            continue
        size, category = key.split("-count-", 1)
        if category not in CATEGORIES:
            continue  # alt-alphaNextToNumber and other alternates
        magnitude = int(size)
        if magnitude not in table:
            continue
        table[magnitude][category] = pattern
    return table


def cpp_literal(text: str) -> str:
    """Render ``text`` as a C++ string literal, pure ASCII."""
    pieces = ['"']
    for character in text:
        code_point = ord(character)
        if character == '"':
            pieces.append('\\"')
        elif character == "\\":
            pieces.append("\\\\")
        elif 0x20 <= code_point < 0x7F:
            pieces.append(character)
        elif code_point <= 0xFFFF:
            pieces.append(f"\\u{code_point:04x}")
        else:
            pieces.append(f"\\U{code_point:08x}")
    pieces.append('"')
    return "".join(pieces)


def cpp_or_null(text):
    return "nullptr" if text is None else cpp_literal(text)


def build(cldr: Path) -> tuple[list, list, list, list, str]:
    digits, cldr_version = load_fraction_digits(cldr)

    base_symbols = {}
    for code, entry in load_currencies(cldr, "en").items():
        base_symbols[code] = entry.get("symbol") or code

    base = []
    for code in sorted(base_symbols):
        symbol = base_symbols[code]
        base.append((code, None if symbol == code else symbol, digits.get(code, 2)))

    overrides = []
    patterns = []
    compact = []

    for locale in LOCALES:
        numbers = load_numbers(cldr, locale)
        currencies = load_currencies(cldr, locale)

        for code in sorted(base_symbols):
            entry = currencies.get(code)
            symbol = entry.get("symbol") if entry else None
            if not symbol:
                continue
            if symbol == base_symbols[code]:
                continue
            overrides.append((locale, code, symbol))

        currency_formats = numbered_section(numbers, "currencyFormats")
        spacing = "\u00a0"
        spacing_data = currency_formats.get("currencySpacing", {})
        for key in ("beforeCurrency", "afterCurrency"):
            candidate = spacing_data.get(key, {}).get("insertBetween")
            if candidate:
                spacing = candidate
                break
        patterns.append(
            (
                locale,
                currency_formats.get("standard", "#,##0.00"),
                currency_formats.get("accounting", currency_formats.get("standard", "#,##0.00")),
                spacing,
            )
        )

        short_table = compact_table(numbers, "short")
        long_table = compact_table(numbers, "long")
        compact.append((locale, short_table, long_table))

    return base, overrides, patterns, compact, cldr_version


HEADER = '''#pragma once

// ===========================================================================
// localisation/currency_data.h
//
// GENERATED FILE -- do not edit by hand.  Regenerate with:
//
//     python tools/gen_currency_data.py
//
// Source: Unicode CLDR {version} (cldr-json, https://github.com/unicode-org/cldr-json).
//
// The table holds, for the {locale_count} locales covered by the generated
// data set:
//
//   * currencyData      base currency metadata from CLDR ``en``: the symbol
//                       (nullptr when it equals the code) and the number of
//                       fraction digits used for amounts of that currency.
//   * currencySymbols   per-locale symbol overrides, present only where the
//                       locale's symbol differs from the base table.
//   * currencyPatterns  the CLDR ``standard`` and ``accounting`` currency
//                       patterns as ``positive;negative`` pairs, plus the
//                       currency-spacing string inserted between an
//                       alphabetic currency code and a number.
//   * compactPatterns   the CLDR short and long compact decimal patterns for
//                       the magnitudes 1e3 .. 1e14.  Row ``i`` of each table
//                       is the pattern for 10^(3 + i); the six columns are the
//                       plural categories zero/one/two/few/many/other, and a
//                       nullptr column means the category is not defined for
//                       that magnitude.
//
// The file is intentionally pure ASCII: non-ASCII code points are written as
// C++ universal character names so the table does not depend on the compiler
// source or execution character set.
// ===========================================================================

namespace localisation
{{
namespace detail
{{

// One row of the base currency table.  ``symbol`` is nullptr when the symbol
// is the ISO code itself; ``digits`` is the CLDR fraction-digit count.
struct CurrencyDataEntry
{{
    const char *code;
    const char *symbol;
    int digits;
}};

// A locale-specific symbol for one currency.
struct CurrencyLocaleSymbol
{{
    const char *tag;
    const char *code;
    const char *symbol;
}};

// A locale's currency patterns.  ``standard`` and ``accounting`` are CLDR
// ``positive;negative`` pattern pairs (the negative half is empty when CLDR
// defines no separate negative subpattern).  ``spacing`` is inserted between
// an alphabetic currency code and the digits.
struct CurrencyPatternEntry
{{
    const char *tag;
    const char *standard;
    const char *accounting;
    const char *spacing;
}};

// Number of magnitudes covered by CompactPatternEntry (1e3 .. 1e14).
inline constexpr int compactMagnitudeCount = {magnitude_count};

// Compact decimal patterns for one locale.  ``shortForms[i]`` and
// ``longForms[i]`` describe 10^(3 + i).
struct CompactPatternEntry
{{
    const char *tag;
    const char *shortForms[compactMagnitudeCount][6];
    const char *longForms[compactMagnitudeCount][6];
}};

inline const CurrencyDataEntry currencyData[] = {{
{base_table}}};

inline const CurrencyLocaleSymbol currencySymbols[] = {{
{override_table}}};

inline const CurrencyPatternEntry currencyPatterns[] = {{
{pattern_table}}};

inline const CompactPatternEntry compactPatterns[] = {{
{compact_table}}};

}} // namespace detail
}} // namespace localisation
'''


def render_base(base) -> str:
    lines = []
    for code, symbol, digit_count in base:
        lines.append(f'    {{"{code}", {cpp_or_null(symbol)}, {digit_count}}},\n')
    return "".join(lines)


def render_overrides(overrides) -> str:
    lines = []
    for locale, code, symbol in overrides:
        lines.append(f'    {{"{locale}", "{code}", {cpp_literal(symbol)}}},\n')
    return "".join(lines)


def render_patterns(patterns) -> str:
    lines = []
    for locale, standard, accounting, spacing in patterns:
        lines.append(
            f'    {{"{locale}", {cpp_literal(standard)}, {cpp_literal(accounting)}, '
            f"{cpp_literal(spacing)}}},\n"
        )
    return "".join(lines)


def render_compact(compact) -> str:
    lines = []
    for index, (locale, short_table, long_table) in enumerate(compact):
        body = []
        body.append('    {"' + locale + '",\n')
        for table in (short_table, long_table):
            body.append("      {\n")
            for magnitude in MAGNITUDES:
                cells = [cpp_or_null(table[magnitude].get(category)) for category in CATEGORIES]
                body.append("        {" + ", ".join(cells) + "}, // 1e" + str(len(str(magnitude)) - 1) + "\n")
            body.append("      },\n")
        body.append("    }" + (",\n" if index + 1 < len(compact) else "\n"))
        lines.append("".join(body))
    return "".join(lines)


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cldr", type=Path, default=default_cldr_root(), help="CLDR JSON root directory")
    parser.add_argument(
        "--out",
        type=Path,
        default=Path(__file__).resolve().parent.parent / "localisation" / "currency_data.h",
        help="output header",
    )
    arguments = parser.parse_args(argv)

    if not arguments.cldr.is_dir():
        print(f"error: CLDR directory not found: {arguments.cldr}", file=sys.stderr)
        return 1
    missing = [locale for locale in LOCALES if not (arguments.cldr / locale / "numbers.json").exists()]
    if missing:
        print(f"error: missing numbers.json for: {', '.join(missing)}", file=sys.stderr)
        return 1

    base, overrides, patterns, compact, cldr_version = build(arguments.cldr)
    text = HEADER.format(
        version=cldr_version,
        locale_count=len(LOCALES),
        magnitude_count=len(MAGNITUDES),
        base_table=render_base(base),
        override_table=render_overrides(overrides),
        pattern_table=render_patterns(patterns),
        compact_table=render_compact(compact),
    )
    arguments.out.write_text(text, encoding="ascii", newline="\n")

    print(
        f"wrote {arguments.out}: {len(base)} currencies, {len(overrides)} symbol overrides, "
        f"{len(patterns)} currency patterns, {len(compact)} compact locales, CLDR {cldr_version}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
