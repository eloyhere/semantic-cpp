"""Generates bunch/properties_data.h from the Unicode Character Database.

The generated header holds compressed, sorted range tables for three
enumerated properties (General_Category, Script, East_Asian_Width) and five
binary properties (Alphabetic, White_Space, Uppercase, Lowercase,
Default_Ignorable_Code_Point), plus the matching name tables.
bunch/properties.h declares the enumerations and the binary searches; this
script only emits data.

Inputs (Unicode 15.0.0, all from https://www.unicode.org/Public/15.0.0/ucd/):

    UnicodeData.txt             General_Category
    Scripts.txt                 Script
    EastAsianWidth.txt          East_Asian_Width (with the UAX #11 @missing
                                defaults for unassigned CJK code points)
    DerivedCoreProperties.txt   Alphabetic, Uppercase, Lowercase,
                                Default_Ignorable_Code_Point
    PropList.txt                White_Space

Usage:
    python tools/gen_properties_data.py [--ucd DIR] [--check]

--ucd defaults to $BUNCH_UCD_DIR, else <temp>/i18n-data/ucd.  --check verifies
that the committed header is up to date instead of rewriting it.

The generator reads the enumerator order from bunch/properties.h and refuses
to run when the two disagree, so the public enumerations and the generated
range values cannot drift apart silently.  The output contains only ASCII
source text, uses LF line endings, and is deterministic: no timestamps, no
dictionary iteration order, code points always written as hexadecimal.
"""

from __future__ import annotations

import argparse
import os
import re
import sys
import tempfile
from pathlib import Path

UNICODE_VERSION = "15.0.0"
MAX_CP = 0x110000
MAX_HEADER_BYTES = 2 * 1024 * 1024

ROOT = Path(__file__).resolve().parent.parent
TARGET = ROOT / "bunch" / "properties_data.h"
PUBLIC_HEADER = ROOT / "bunch" / "properties.h"

UCD_FILES = (
    "UnicodeData.txt",
    "Scripts.txt",
    "EastAsianWidth.txt",
    "DerivedCoreProperties.txt",
    "PropList.txt",
)

# Canonical Unicode ordering of General_Category values: short UCD name and
# the enumerator used by bunch::GeneralCategory.  The position in this tuple is
# the value stored in the generated table.
GENERAL_CATEGORIES = (
    ("Lu", "upperLetter"),
    ("Ll", "lowerLetter"),
    ("Lt", "titlecaseLetter"),
    ("Lm", "modifierLetter"),
    ("Lo", "otherLetter"),
    ("Mn", "nonspacingMark"),
    ("Mc", "spacingMark"),
    ("Me", "enclosingMark"),
    ("Nd", "decimalNumber"),
    ("Nl", "letterNumber"),
    ("No", "otherNumber"),
    ("Pc", "connectorPunctuation"),
    ("Pd", "dashPunctuation"),
    ("Ps", "openPunctuation"),
    ("Pe", "closePunctuation"),
    ("Pi", "initialPunctuation"),
    ("Pf", "finalPunctuation"),
    ("Po", "otherPunctuation"),
    ("Sm", "mathSymbol"),
    ("Sc", "currencySymbol"),
    ("Sk", "modifierSymbol"),
    ("So", "otherSymbol"),
    ("Zs", "spaceSeparator"),
    ("Zl", "lineSeparator"),
    ("Zp", "paragraphSeparator"),
    ("Cc", "control"),
    ("Cf", "format"),
    ("Cs", "surrogate"),
    ("Co", "privateUse"),
    ("Cn", "unassigned"),
)

# short UCD name, enumerator, name returned by eastAsianWidthName().
EAST_ASIAN_WIDTHS = (
    ("N", "neutral", "Neutral"),
    ("Na", "narrow", "Narrow"),
    ("W", "wide", "Wide"),
    ("F", "fullwidth", "Fullwidth"),
    ("H", "halfwidth", "Halfwidth"),
    ("A", "ambiguous", "Ambiguous"),
)

# UAX #11: unassigned code points in these blocks default to Wide.  The
# defaults are applied before the explicit records of EastAsianWidth.txt, which
# override them for assigned code points.
EAST_ASIAN_WIDTH_WIDE_DEFAULTS = (
    (0x3400, 0x4DBF),   # CJK Unified Ideographs Extension A
    (0x4E00, 0x9FFF),   # CJK Unified Ideographs
    (0xF900, 0xFAFF),   # CJK Compatibility Ideographs
    (0x20000, 0x2FFFD), # Plane 2
    (0x30000, 0x3FFFD), # Plane 3
)

# Property name as written in the UCD, generated table, generated count.
BINARY_PROPERTIES = (
    ("Alphabetic", "kAlphabeticRanges", "kAlphabeticRangeCount"),
    ("White_Space", "kWhiteSpaceRanges", "kWhiteSpaceRangeCount"),
    ("Uppercase", "kUppercaseRanges", "kUppercaseRangeCount"),
    ("Lowercase", "kLowercaseRanges", "kLowercaseRangeCount"),
    ("Default_Ignorable_Code_Point", "kDefaultIgnorableRanges",
     "kDefaultIgnorableRangeCount"),
)

# Sanity checks pinned to Unicode 15.0.0.  They catch a swapped UCD directory
# (for example 15.1 data) before it is baked into the committed header.
EXPECTED_ASSIGNED = 288767
EXPECTED_CATEGORY_COUNTS = {
    "Lu": 1831,
    "Ll": 2233,
    "Lo": 131612,
    "Nd": 680,
    "Mn": 1985,
    "Cs": 2048,
    "Co": 137468,
    "Cc": 65,
}

CATEGORY_IDS = {short: index for index, (short, _) in enumerate(GENERAL_CATEGORIES)}
CATEGORY_TOKENS = [token for _, token in GENERAL_CATEGORIES]
CATEGORY_SHORT_NAMES = [short for short, _ in GENERAL_CATEGORIES]

WIDTH_IDS = {short: index for index, (short, _, _) in enumerate(EAST_ASIAN_WIDTHS)}
WIDTH_TOKENS = [token for _, token, _ in EAST_ASIAN_WIDTHS]
WIDTH_NAMES = [name for _, _, name in EAST_ASIAN_WIDTHS]

# Script long names that the enumeration puts first and last; everything in
# between is sorted alphabetically.
SCRIPT_PREFIX = ("Common", "Inherited")
SCRIPT_SUFFIX = ("Unknown",)


class GenerationError(Exception):
    """A reproducible, actionable problem with the inputs or the header."""


def script_token(long_name: str) -> str:
    """Converts a UCD script long name such as Old_Italic to oldItalic."""
    parts = long_name.split("_")
    token = parts[0].lower()
    for part in parts[1:]:
        token += part[:1].upper() + part[1:].lower()
    return token


def default_ucd_directory() -> Path:
    configured = os.environ.get("BUNCH_UCD_DIR")
    if configured:
        return Path(configured)
    return Path(tempfile.gettempdir()) / "i18n-data" / "ucd"


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except FileNotFoundError as error:
        raise GenerationError(
            f"missing UCD file {path}; download the Unicode {UNICODE_VERSION} "
            f"files from https://www.unicode.org/Public/{UNICODE_VERSION}/ucd/ "
            "or pass --ucd DIR"
        ) from error
    except UnicodeDecodeError as error:
        raise GenerationError(f"{path} is not valid UTF-8: {error}") from error


def check_version(path: Path, text: str) -> None:
    """Confirms that the file header names the version this script emits."""
    if path.name == "UnicodeData.txt":
        # UnicodeData.txt has no header comment; EXPECTED_ASSIGNED checks it.
        return
    header = "\n".join(text.splitlines()[:15])
    if UNICODE_VERSION not in header:
        raise GenerationError(
            f"{path.name} does not look like Unicode {UNICODE_VERSION} data; "
            "the generated header would mix versions"
        )


def load_ranges(path: Path):
    """Parses a semicolon-separated UCD file into (first, last, value) records."""
    text = read_text(path)
    check_version(path, text)
    records = []
    for line_number, raw in enumerate(text.splitlines(), 1):
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        fields = [field.strip() for field in line.split(";")]
        if len(fields) < 2:
            raise GenerationError(
                f"{path.name}:{line_number}: expected a semicolon-separated record"
            )
        span = fields[0]
        try:
            if ".." in span:
                first_text, last_text = span.split("..")
                first = int(first_text, 16)
                last = int(last_text, 16)
            else:
                first = last = int(span, 16)
        except ValueError as error:
            raise GenerationError(
                f"{path.name}:{line_number}: bad code point range {span!r}"
            ) from error
        if not 0 <= first <= last < MAX_CP:
            raise GenerationError(
                f"{path.name}:{line_number}: range {span!r} is outside the code space"
            )
        records.append((first, last, fields[1]))
    if not records:
        raise GenerationError(f"{path.name}: no property records found")
    return records


def group_by_property(records):
    grouped = {}
    for first, last, name in records:
        grouped.setdefault(name, []).append((first, last))
    return grouped


def fill(values: bytearray, first: int, last: int, value: int) -> None:
    values[first:last + 1] = bytearray([value]) * (last - first + 1)


def build_general_categories(path: Path):
    """Returns (category id per code point, assigned code point count)."""
    text = read_text(path)
    values = bytearray([CATEGORY_IDS["Cn"]]) * MAX_CP
    assigned = 0
    pending = None
    for line_number, raw in enumerate(text.splitlines(), 1):
        if not raw or raw.startswith("#"):
            continue
        fields = raw.split(";")
        if len(fields) < 3:
            raise GenerationError(f"{path.name}:{line_number}: fewer than 3 fields")
        try:
            codepoint = int(fields[0], 16)
        except ValueError as error:
            raise GenerationError(
                f"{path.name}:{line_number}: bad code point {fields[0]!r}"
            ) from error
        name = fields[1]
        short_name = fields[2]
        if short_name not in CATEGORY_IDS:
            raise GenerationError(
                f"{path.name}:{line_number}: unknown General_Category {short_name!r}"
            )
        category = CATEGORY_IDS[short_name]
        if name.endswith(", First>"):
            if pending is not None:
                raise GenerationError(f"{path.name}:{line_number}: nested First entry")
            pending = (codepoint, category, short_name)
        elif name.endswith(", Last>"):
            if pending is None:
                raise GenerationError(
                    f"{path.name}:{line_number}: Last entry without First"
                )
            first, first_category, first_short_name = pending
            if first_category != category:
                raise GenerationError(
                    f"{path.name}:{line_number}: range {first_short_name}/{short_name} "
                    "changes category in the middle"
                )
            fill(values, first, codepoint, category)
            assigned += codepoint - first + 1
            pending = None
        else:
            if pending is not None:
                raise GenerationError(
                    f"{path.name}:{line_number}: single entry inside a First/Last range"
                )
            values[codepoint] = category
            assigned += 1
    if pending is not None:
        raise GenerationError(f"{path.name}: unterminated First/Last range")
    return values, assigned


def script_order(scripts_grouped):
    """Returns the Script enumeration order implied by the UCD data."""
    names = set(scripts_grouped)
    if "Common" not in names or "Inherited" not in names:
        raise GenerationError("Scripts.txt is missing the Common or Inherited script")
    if "Unknown" in names:
        raise GenerationError(
            "Scripts.txt lists Unknown explicitly; the @missing default is "
            "supposed to cover unassigned code points"
        )
    specific = sorted(names - set(SCRIPT_PREFIX))
    return list(SCRIPT_PREFIX) + specific + list(SCRIPT_SUFFIX)


def build_scripts(scripts_grouped, order):
    ids = {name: index for index, name in enumerate(order)}
    values = bytearray([ids["Unknown"]]) * MAX_CP
    for name, ranges in scripts_grouped.items():
        if name not in ids:
            raise GenerationError(
                f"script {name!r} has no enumerator; add it to bunch/properties.h "
                "and rerun the generator"
            )
        for first, last in ranges:
            fill(values, first, last, ids[name])
    return values


def build_east_asian_widths(widths_grouped):
    values = bytearray([WIDTH_IDS["N"]]) * MAX_CP
    # UAX #11 @missing rule: unassigned code points in the CJK blocks are
    # Wide.  Applying the defaults first lets explicit records win.
    for first, last in EAST_ASIAN_WIDTH_WIDE_DEFAULTS:
        fill(values, first, last, WIDTH_IDS["W"])
    for name, ranges in widths_grouped.items():
        if name not in WIDTH_IDS:
            raise GenerationError(f"unknown East_Asian_Width value {name!r}")
        for first, last in ranges:
            fill(values, first, last, WIDTH_IDS[name])
    return values


def build_binary_flags(properties, property_name: str) -> bytearray:
    if property_name not in properties:
        raise GenerationError(f"property {property_name!r} not found in the UCD files")
    flags = bytearray(MAX_CP)
    for first, last in properties[property_name]:
        fill(flags, first, last, 1)
    return flags


def merge_ranges(values: bytearray):
    """Merges equal neighbours into (first, last, value) ranges."""
    ranges = []
    start = 0
    current = values[0]
    for codepoint in range(1, MAX_CP):
        value = values[codepoint]
        if value != current:
            ranges.append((start, codepoint - 1, current))
            start = codepoint
            current = value
    ranges.append((start, MAX_CP - 1, current))
    return ranges


def true_ranges(flags: bytearray):
    """Merges set flags into (first, last) ranges, dropping the empty ones."""
    ranges = []
    start = None
    for codepoint, flag in enumerate(flags):
        if flag:
            if start is None:
                start = codepoint
        elif start is not None:
            ranges.append((start, codepoint - 1))
            start = None
    if start is not None:
        ranges.append((start, MAX_CP - 1))
    return ranges


def parse_enumeration(source: str, type_name: str):
    pattern = re.compile(
        r"enum\s+class\s+" + re.escape(type_name) + r"\s*:\s*std::uint8_t\s*\{(.*?)\};",
        re.DOTALL,
    )
    match = pattern.search(source)
    if match is None:
        raise GenerationError(
            f"bunch/properties.h does not declare enum class {type_name} : std::uint8_t"
        )
    body = re.sub(r"//[^\n]*", "", match.group(1))
    tokens = []
    for raw in body.split(","):
        token = raw.strip()
        if not token:
            continue
        if not re.fullmatch(r"[a-z][A-Za-z0-9]*", token):
            raise GenerationError(
                f"bunch/properties.h: unexpected {type_name} enumerator {token!r}; "
                "the generator only understands plain camelCase enumerators"
            )
        tokens.append(token)
    return tokens


def compare_enumeration(type_name: str, actual, expected) -> None:
    if actual == expected:
        return
    for index in range(min(len(actual), len(expected))):
        if actual[index] != expected[index]:
            raise GenerationError(
                f"bunch/properties.h: enum class {type_name} is out of sync with "
                f"the UCD data at position {index}: got {actual[index]!r}, expected "
                f"{expected[index]!r}"
            )
    raise GenerationError(
        f"bunch/properties.h: enum class {type_name} has {len(actual)} enumerators "
        f"but the UCD data needs {len(expected)}"
    )


def verify_enumerations(order):
    """Checks that the public enumerations match the UCD-derived order."""
    try:
        source = PUBLIC_HEADER.read_text(encoding="utf-8")
    except FileNotFoundError as error:
        raise GenerationError(
            f"missing {PUBLIC_HEADER}; the enumerator order is read from it"
        ) from error
    compare_enumeration(
        "GeneralCategory", parse_enumeration(source, "GeneralCategory"), CATEGORY_TOKENS)
    compare_enumeration(
        "EastAsianWidth", parse_enumeration(source, "EastAsianWidth"), WIDTH_TOKENS)
    compare_enumeration(
        "Script", parse_enumeration(source, "Script"),
        [script_token(name) for name in order])


def validate(category, script, width, binary_flags, order, assigned) -> None:
    """Cross-checks the built tables against known Unicode 15.0.0 facts."""
    if assigned != EXPECTED_ASSIGNED:
        raise GenerationError(
            f"UnicodeData.txt assigns {assigned} code points, expected "
            f"{EXPECTED_ASSIGNED} for Unicode {UNICODE_VERSION}"
        )

    counts = {}
    for value in category:
        counts[value] = counts.get(value, 0) + 1
    for short_name, expected in EXPECTED_CATEGORY_COUNTS.items():
        actual = counts.get(CATEGORY_IDS[short_name], 0)
        if actual != expected:
            raise GenerationError(
                f"General_Category {short_name} covers {actual} code points, "
                f"expected {expected} for Unicode {UNICODE_VERSION}"
            )

    if len(order) != 164:
        raise GenerationError(
            f"expected 164 Script values for Unicode {UNICODE_VERSION}, "
            f"found {len(order)}"
        )

    category_checks = {
        0x41: "Lu", 0x61: "Ll", 0x31: "Nd", 0x20: "Zs", 0xE9: "Ll",
        0x3A9: "Lu", 0x4E2D: "Lo", 0x1F600: "So", 0x301: "Mn", 0x200B: "Cf",
        0xE000: "Co", 0x378: "Cn", 0xD800: "Cs", 0x10FFFF: "Cn",
    }
    for codepoint, short_name in category_checks.items():
        actual = CATEGORY_SHORT_NAMES[category[codepoint]]
        if actual != short_name:
            raise GenerationError(
                f"General_Category U+{codepoint:04X} is {actual}, expected {short_name}"
            )

    script_checks = {
        0x41: "Latin", 0x61: "Latin", 0xE9: "Latin", 0x3A9: "Greek",
        0x4E2D: "Han", 0x1F600: "Common", 0x301: "Inherited", 0x200B: "Common",
        0xE000: "Unknown", 0x378: "Unknown", 0xD800: "Unknown",
    }
    for codepoint, name in script_checks.items():
        actual = order[script[codepoint]]
        if actual != name:
            raise GenerationError(
                f"Script U+{codepoint:04X} is {actual}, expected {name}"
            )

    width_short_names = [short for short, _, _ in EAST_ASIAN_WIDTHS]
    width_checks = {
        0x41: "Na", 0x61: "Na", 0x31: "Na", 0x20: "Na", 0xE9: "A",
        0x3A9: "A", 0x4E2D: "W", 0x1F600: "W", 0x301: "A", 0x200B: "N",
        0xE000: "A", 0x378: "N", 0xD800: "N", 0x3000: "F", 0x2FFFD: "W",
    }
    for codepoint, short_name in width_checks.items():
        actual = width_short_names[width[codepoint]]
        if actual != short_name:
            raise GenerationError(
                f"East_Asian_Width U+{codepoint:04X} is {actual}, expected {short_name}"
            )

    binary_checks = {
        "Alphabetic": ((0x41, True), (0x61, True), (0x31, False), (0x4E2D, True)),
        "White_Space": ((0x20, True), (0x200B, False), (0x3000, True)),
        "Uppercase": ((0x41, True), (0x61, False), (0x2160, True)),
        "Lowercase": ((0x61, True), (0x41, False), (0xDF, True)),
        "Default_Ignorable_Code_Point": ((0xAD, True), (0x200B, True),
                                         (0x41, False)),
    }
    for property_name, checks in binary_checks.items():
        flags = binary_flags[property_name]
        for codepoint, expected in checks:
            actual = bool(flags[codepoint])
            if actual != expected:
                raise GenerationError(
                    f"{property_name} U+{codepoint:04X} is {actual}, expected {expected}"
                )


def emit_enum_range_table(name: str, kind: str, ranges, tokens) -> list:
    """Emits a range table whose value is the numeric enumerator value.

    The enumerator name is kept as a trailing comment so the generated data can
    be reviewed without counting enumerators in bunch/properties.h.
    """
    lines = [f"inline constexpr {kind} {name}[] = {{"]
    for first, last, value in ranges:
        lines.append(f"        {{0x{first:04X}, 0x{last:04X}, {value}}}, // {tokens[value]}")
    lines.append("};")
    lines.append("")
    return lines


def emit_binary_range_table(name: str, ranges) -> list:
    lines = [f"inline constexpr PropertyRange {name}[] = {{"]
    for index in range(0, len(ranges), 4):
        entries = [f"{{0x{first:04X}, 0x{last:04X}}}"
                   for first, last in ranges[index:index + 4]]
        lines.append("        " + ", ".join(entries) + ",")
    lines.append("};")
    lines.append("")
    return lines


def emit_count(name: str, table: str) -> list:
    return [
        f"inline constexpr std::size_t {name} =",
        f"    sizeof({table}) / sizeof({table}[0]);",
        "",
    ]


def emit_name_table(name: str, values) -> list:
    lines = [f"inline constexpr const char *{name}[] = {{"]
    for index in range(0, len(values), 5):
        chunk = values[index:index + 5]
        lines.append("        " + ", ".join(f'"{value}"' for value in chunk) + ",")
    lines.append("};")
    lines.append("")
    return lines


def emit_header(category_ranges, script_ranges, width_ranges, binary_ranges,
                order) -> str:
    out = []
    out.append("#pragma once")
    out.append("")
    out.append("// ===========================================================================")
    out.append("// bunch/properties_data.h")
    out.append("//")
    out.append(f"// Generated by tools/gen_properties_data.py from the Unicode {UNICODE_VERSION}")
    out.append("// Character Database.  Do not edit this file by hand; rerun the")
    out.append("// generator instead.")
    out.append("//")
    out.append("// Contents:")
    out.append("//   - General_Category, Script, and East_Asian_Width as sorted,")
    out.append("//     non-overlapping ranges that cover 0..10FFFF without gaps,")
    out.append("//   - Alphabetic, White_Space, Uppercase, Lowercase, and")
    out.append("//     Default_Ignorable_Code_Point as sorted, non-overlapping ranges")
    out.append("//     that contain only the code points with the property.")
    out.append("//")
    out.append("// The values in the General_Category, Script, and East_Asian_Width")
    out.append("// tables are the numeric values of the enumerators declared in")
    out.append("// bunch/properties.h, in the order of that enumeration; the trailing")
    out.append("// comment repeats the enumerator name.  The generator reads the")
    out.append("// enumerator order from bunch/properties.h, so the two cannot drift")
    out.append("// apart silently.  This header depends only on the standard library and")
    out.append("// can be compiled on its own; bunch/properties.h is the public API.")
    out.append("//")
    out.append("// Inputs: UnicodeData.txt, Scripts.txt, EastAsianWidth.txt,")
    out.append("// DerivedCoreProperties.txt, and PropList.txt from")
    out.append(f"// https://www.unicode.org/Public/{UNICODE_VERSION}/ucd/.")
    out.append("// ===========================================================================")
    out.append("")
    out.append("#include <cstddef>")
    out.append("#include <cstdint>")
    out.append("")
    out.append("namespace bunch")
    out.append("{")
    out.append("namespace detail")
    out.append("{")
    out.append("namespace properties")
    out.append("{")
    out.append(f'inline constexpr const char *kUnicodeVersion = "{UNICODE_VERSION}";')
    out.append("")
    out.append("struct CategoryRange")
    out.append("{")
    out.append("    std::uint32_t first;")
    out.append("    std::uint32_t last;")
    out.append("    std::uint8_t category; // GeneralCategory")
    out.append("};")
    out.append("")
    out.append("struct ScriptRange")
    out.append("{")
    out.append("    std::uint32_t first;")
    out.append("    std::uint32_t last;")
    out.append("    std::uint8_t script; // Script")
    out.append("};")
    out.append("")
    out.append("struct WidthRange")
    out.append("{")
    out.append("    std::uint32_t first;")
    out.append("    std::uint32_t last;")
    out.append("    std::uint8_t width; // EastAsianWidth")
    out.append("};")
    out.append("")
    out.append("struct PropertyRange")
    out.append("{")
    out.append("    std::uint32_t first;")
    out.append("    std::uint32_t last;")
    out.append("};")
    out.append("")

    out.extend(emit_enum_range_table(
        "kCategoryRanges", "CategoryRange", category_ranges, CATEGORY_TOKENS))
    out.extend(emit_count("kCategoryRangeCount", "kCategoryRanges"))
    out.extend(emit_enum_range_table(
        "kScriptRanges", "ScriptRange", script_ranges,
        [script_token(name) for name in order]))
    out.extend(emit_count("kScriptRangeCount", "kScriptRanges"))
    out.extend(emit_enum_range_table(
        "kWidthRanges", "WidthRange", width_ranges, WIDTH_TOKENS))
    out.extend(emit_count("kWidthRangeCount", "kWidthRanges"))

    for property_name, table, count in BINARY_PROPERTIES:
        out.extend(emit_binary_range_table(table, binary_ranges[property_name]))
        out.extend(emit_count(count, table))

    out.extend(emit_name_table("kCategoryNames", CATEGORY_SHORT_NAMES))
    out.extend(emit_count("kCategoryNameCount", "kCategoryNames"))
    out.extend(emit_name_table("kScriptNames", order))
    out.extend(emit_count("kScriptNameCount", "kScriptNames"))
    out.extend(emit_name_table("kWidthNames", WIDTH_NAMES))
    out.extend(emit_count("kWidthNameCount", "kWidthNames"))

    out.append("} // namespace properties")
    out.append("} // namespace detail")
    out.append("} // namespace bunch")
    out.append("")
    return "\n".join(out)


def generate(ucd_directory: Path):
    paths = {name: ucd_directory / name for name in UCD_FILES}
    scripts_grouped = group_by_property(load_ranges(paths["Scripts.txt"]))
    order = script_order(scripts_grouped)
    verify_enumerations(order)

    category, assigned = build_general_categories(paths["UnicodeData.txt"])
    script = build_scripts(scripts_grouped, order)
    widths_grouped = group_by_property(load_ranges(paths["EastAsianWidth.txt"]))
    width = build_east_asian_widths(widths_grouped)

    properties = {}
    for name in ("DerivedCoreProperties.txt", "PropList.txt"):
        for property_name, ranges in group_by_property(load_ranges(paths[name])).items():
            properties.setdefault(property_name, []).extend(ranges)
    binary_flags = {
        name: build_binary_flags(properties, name) for name, _, _ in BINARY_PROPERTIES
    }
    validate(category, script, width, binary_flags, order, assigned)

    category_ranges = merge_ranges(category)
    script_ranges = merge_ranges(script)
    width_ranges = merge_ranges(width)
    binary_ranges = {
        name: true_ranges(flags) for name, flags in binary_flags.items()
    }
    content = emit_header(category_ranges, script_ranges, width_ranges,
                          binary_ranges, order)
    stats = {
        "categories": len(category_ranges),
        "scripts": len(script_ranges),
        "widths": len(width_ranges),
        "alphabetic": len(binary_ranges["Alphabetic"]),
        "whiteSpace": len(binary_ranges["White_Space"]),
        "uppercase": len(binary_ranges["Uppercase"]),
        "lowercase": len(binary_ranges["Lowercase"]),
        "defaultIgnorable": len(binary_ranges["Default_Ignorable_Code_Point"]),
        "bytes": len(content.encode("ascii")),
    }
    return content, stats


def main(argv) -> int:
    parser = argparse.ArgumentParser(
        description="Generate bunch/properties_data.h from Unicode 15.0.0 UCD files.")
    parser.add_argument("--ucd", type=Path, default=default_ucd_directory(),
                        help="directory containing the five UCD text files")
    parser.add_argument("--check", action="store_true",
                        help="verify the committed header instead of writing it")
    arguments = parser.parse_args(argv[1:])

    try:
        content, stats = generate(arguments.ucd)
    except GenerationError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    encoded = content.encode("ascii")
    if len(encoded) > MAX_HEADER_BYTES:
        print(
            f"error: generated header is {len(encoded)} bytes, over the "
            f"{MAX_HEADER_BYTES} byte budget",
            file=sys.stderr,
        )
        return 2

    if arguments.check:
        if not TARGET.exists():
            print(f"error: missing {TARGET}", file=sys.stderr)
            return 1
        try:
            current = TARGET.read_bytes()
        except OSError as error:
            print(f"error: cannot read {TARGET}: {error}", file=sys.stderr)
            return 1
        if current == encoded:
            print(f"{TARGET.name} is up to date ({len(encoded)} bytes)")
            return 0
        current_lines = current.decode("ascii", "replace").splitlines()
        new_lines = content.splitlines()
        difference = 0
        while (difference < len(current_lines) and difference < len(new_lines)
               and current_lines[difference] == new_lines[difference]):
            difference += 1
        print(
            f"error: {TARGET.name} is out of date (first difference at line "
            f"{difference + 1}); run tools/gen_properties_data.py",
            file=sys.stderr,
        )
        return 1

    TARGET.parent.mkdir(parents=True, exist_ok=True)
    TARGET.write_bytes(encoded)
    print(f"wrote {TARGET} ({len(encoded)} bytes)")
    print("ranges: " + ", ".join(f"{name}={count}" for name, count in stats.items()))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
