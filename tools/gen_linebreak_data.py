"""Generates bunch/linebreak_data.h and tests/linebreak_vectors.h for UAX #14.

Both headers are build artefacts derived from the Unicode Character Database
(version 15.0.0); they are not hand-edited code.  The script is deterministic:
the same UCD input produces byte-identical output, so `--check` can be used in
CI to verify that the checked-in headers are current.

Input files (all Unicode 15.0.0):
    LineBreak.txt      Line_Break property (the main classification table)
    UnicodeData.txt    General_Category, used by LB1 to split SA into CM and AL
                       and to identify unassigned (Cn) code points
    EastAsianWidth.txt East_Asian_Width, used by LB30 to exclude East Asian
                       opening and closing punctuation from the parenthesis rule
    emoji-data.txt     Extended_Pictographic, used by the second part of LB30b
    LineBreakTest.txt  the official conformance test, converted to vectors

The data header stores four sorted range tables:

    kPropertyRanges                  Line_Break, with the @missing defaults from
                                     the LineBreak.txt header applied, so code
                                     points outside every range are XX
    kSaCombiningRanges               SA code points with GC Mn or Mc (LB1 -> CM)
    kEastAsianWideOpenCloseRanges    OP/CP code points with EAW F, W, or H
    kUnassignedPictographicRanges    Extended_Pictographic and GC Cn (LB30b)

The vector header contains every case from LineBreakTest.txt; nothing is
sampled.

The UCD directory is resolved in this order:
    --ucd DIR, $UCD_DIR, $TEMP/i18n-data/ucd, <repo>/unicode

Usage:
    python tools/gen_linebreak_data.py [--ucd DIR] [--check]
"""

from __future__ import annotations

import argparse
import os
import sys
import tempfile
from pathlib import Path

UNICODE_VERSION = "15.0.0"
EMOJI_VERSION = "15.0"
MAX_CODEPOINT = 0x10FFFF

REPOSITORY = Path(__file__).resolve().parent.parent
DATA_PATH = REPOSITORY / "bunch" / "linebreak_data.h"
VECTORS_PATH = REPOSITORY / "tests" / "linebreak_vectors.h"

BREAK = "\u00f7"  # division sign, marks a break opportunity
NO_BREAK = "\u00d7"  # multiplication sign, marks a prohibited break

# Enum members in declaration order; the integer value is the index.  The
# names must stay in sync with LineBreakClass in bunch/linebreak_data.h.
# XX comes first so that the value 0 doubles as the "code point not listed"
# default of the range tables.
LINE_BREAK_VALUES = [
    "xx", "ai", "al", "b2", "ba", "bb", "bk", "cb", "cj", "cl", "cm", "cp",
    "cr", "eb", "em", "ex", "gl", "h2", "h3", "hl", "hy", "id", "in", "is",
    "jl", "jt", "jv", "lf", "nl", "ns", "nu", "op", "po", "pr", "qu", "ri",
    "sa", "sg", "sp", "sy", "wj", "zw", "zwj",
]

# Line_Break property names in LineBreak.txt are the upper-case enum names,
# with XX spelled "XX".
PROPERTY_INDEX = {name.upper(): index for index, name in enumerate(LINE_BREAK_VALUES)}

# @missing defaults from the LineBreak.txt header: unassigned code points in
# these blocks are not XX but ID (ideographic) or PR (currency prefix).
MISSING_DEFAULTS = [
    (0x3400, 0x4DBF, "ID"),
    (0x4E00, 0x9FFF, "ID"),
    (0xF900, 0xFAFF, "ID"),
    (0x20000, 0x2FFFD, "ID"),
    (0x30000, 0x3FFFD, "ID"),
    (0x1F000, 0x1FAFF, "ID"),
    (0x1FC00, 0x1FFFD, "ID"),
    (0x20A0, 0x20CF, "PR"),
]


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def resolve_ucd_directory(argument: str | None) -> Path:
    candidates = []
    if argument:
        candidates.append(Path(argument))
    if os.environ.get("UCD_DIR"):
        candidates.append(Path(os.environ["UCD_DIR"]))
    candidates.append(Path(tempfile.gettempdir()) / "i18n-data" / "ucd")
    candidates.append(REPOSITORY / "unicode")
    for candidate in candidates:
        if (candidate / "LineBreak.txt").is_file():
            return candidate
    raise SystemExit(
        "Unicode data directory not found; pass --ucd DIR or set UCD_DIR. "
        "Expected LineBreak.txt in one of: "
        + ", ".join(str(candidate) for candidate in candidates)
    )


def check_version(path: Path, needle: str) -> None:
    head = "\n".join(read_text(path).splitlines()[:12])
    if needle not in head:
        raise SystemExit(f"{path.name}: expected {needle!r} in the header, got:\n{head}")


def split_fields(data: str, path: Path, line_number: int, expected: int) -> list[str]:
    fields = [field.strip() for field in data.split(";")]
    if len(fields) != expected:
        raise SystemExit(f"{path.name}:{line_number}: expected {expected} fields, got {data!r}")
    return fields


def parse_codepoint_range(text: str, path: Path, line_number: int) -> tuple[int, int]:
    if ".." in text:
        first_text, last_text = text.split("..", 1)
    else:
        first_text = last_text = text
    first, last = int(first_text, 16), int(last_text, 16)
    if first > last or last > MAX_CODEPOINT:
        raise SystemExit(f"{path.name}:{line_number}: invalid range {text!r}")
    return first, last


def load_line_break(path: Path) -> bytearray:
    """Line_Break one byte per code point, with the @missing defaults applied."""
    classes = bytearray([PROPERTY_INDEX["XX"]]) * (MAX_CODEPOINT + 1)
    for first, last, name in MISSING_DEFAULTS:
        classes[first:last + 1] = bytes([PROPERTY_INDEX[name]]) * (last - first + 1)
    for line_number, raw in enumerate(read_text(path).splitlines(), 1):
        data = raw.split("#", 1)[0].strip()
        if not data:
            continue
        codepoints, name = split_fields(data, path, line_number, 2)
        if name not in PROPERTY_INDEX:
            raise SystemExit(f"{path.name}:{line_number}: unknown Line_Break value {name!r}")
        first, last = parse_codepoint_range(codepoints, path, line_number)
        classes[first:last + 1] = bytes([PROPERTY_INDEX[name]]) * (last - first + 1)
    return classes


def load_general_category(path: Path) -> tuple[bytearray, bytearray]:
    """Returns (assigned, mark) flags per code point.

    assigned is 1 for every code point listed in UnicodeData.txt (including the
    First/Last ranges), mark is 1 for General_Category Mn or Mc.
    """
    assigned = bytearray(MAX_CODEPOINT + 1)
    mark = bytearray(MAX_CODEPOINT + 1)
    pending_first = None
    for line_number, raw in enumerate(read_text(path).splitlines(), 1):
        if not raw.strip():
            continue
        fields = raw.split(";")
        if len(fields) < 3:
            raise SystemExit(f"{path.name}:{line_number}: cannot parse {raw!r}")
        codepoint = int(fields[0], 16)
        name, category = fields[1], fields[2]
        if name.endswith(", First>"):
            pending_first = codepoint
            continue
        if name.endswith(", Last>"):
            if pending_first is None:
                raise SystemExit(f"{path.name}:{line_number}: Last without First")
            first, last = pending_first, codepoint
            pending_first = None
        else:
            first = last = codepoint
        assigned[first:last + 1] = b"\x01" * (last - first + 1)
        if category in ("Mn", "Mc"):
            mark[first:last + 1] = b"\x01" * (last - first + 1)
    if pending_first is not None:
        raise SystemExit(f"{path.name}: unterminated First range")
    return assigned, mark


def load_east_asian_width(path: Path) -> bytearray:
    """1 for code points whose East_Asian_Width is F, W, or H."""
    wide = bytearray(MAX_CODEPOINT + 1)
    for line_number, raw in enumerate(read_text(path).splitlines(), 1):
        data = raw.split("#", 1)[0].strip()
        if not data:
            continue
        codepoints, value = split_fields(data, path, line_number, 2)
        first, last = parse_codepoint_range(codepoints, path, line_number)
        if value in ("F", "W", "H"):
            wide[first:last + 1] = b"\x01" * (last - first + 1)
    return wide


def load_extended_pictographic(path: Path) -> bytearray:
    pictographic = bytearray(MAX_CODEPOINT + 1)
    for line_number, raw in enumerate(read_text(path).splitlines(), 1):
        data = raw.split("#", 1)[0].strip()
        if not data:
            continue
        codepoints, name = split_fields(data, path, line_number, 2)
        if name != "Extended_Pictographic":
            continue
        first, last = parse_codepoint_range(codepoints, path, line_number)
        pictographic[first:last + 1] = b"\x01" * (last - first + 1)
    return pictographic


def ranges_from_flags(flags: bytearray) -> list[tuple[int, int]]:
    ranges = []
    start = None
    for codepoint in range(MAX_CODEPOINT + 1):
        if flags[codepoint]:
            if start is None:
                start = codepoint
        elif start is not None:
            ranges.append((start, codepoint - 1))
            start = None
    if start is not None:
        ranges.append((start, MAX_CODEPOINT))
    return ranges


def collapse_classes(classes: bytearray) -> list[tuple[int, int, int]]:
    """Merges equal adjacent values and drops the default value XX (0)."""
    entries: list[tuple[int, int, int]] = []
    start = 0
    current = classes[0]
    for codepoint in range(1, MAX_CODEPOINT + 1):
        value = classes[codepoint]
        if value != current:
            if current != 0:
                entries.append((start, codepoint - 1, current))
            start = codepoint
            current = value
    if current != 0:
        entries.append((start, MAX_CODEPOINT, current))
    return entries


def single_value_ranges(flags: bytearray) -> list[tuple[int, int, int]]:
    return [(first, last, 1) for first, last in ranges_from_flags(flags)]


def format_ranges(entries: list[tuple[int, int, int]]) -> str:
    lines = []
    for index in range(0, len(entries), 4):
        chunk = entries[index:index + 4]
        rendered = ", ".join(
            f"{{0x{first:04X}, 0x{last:04X}, {value}}}" for first, last, value in chunk
        )
        lines.append("        " + rendered + ",")
    return "\n".join(lines)


def format_enum(name: str, values: list[str]) -> str:
    members = [f"    {member} = {index}" for index, member in enumerate(values)]
    return "\n".join([f"enum class {name} : std::uint8_t", "{", ",\n".join(members), "};"])


def emit_data_header(
    properties: list[tuple[int, int, int]],
    sa_combining: list[tuple[int, int, int]],
    east_asian: list[tuple[int, int, int]],
    unassigned_pictographic: list[tuple[int, int, int]],
) -> str:
    parts = [
        f"""#pragma once

// Generated by tools/gen_linebreak_data.py from Unicode {UNICODE_VERSION} data. Do not edit by hand.
//
// Code point property tables for UAX #14 line breaking.  Every table is sorted
// by code point and stores only ranges whose value is not the default, so
// lookups use binary search over at most a few thousand ranges.
//
// kPropertyRanges is the Line_Break property from LineBreak.txt with the
// @missing defaults from that file's header applied: unassigned code points in
// the CJK blocks, the Plane 1 ideographic ranges, and Planes 2 and 3 default to
// ID, and unassigned code points in the Currency Symbols block default to PR.
// Code points outside every range have the value XX.  The table is the result
// of LB1 only in the sense that it is the raw input to LB1; the resolution
// itself (AI, CJ, SA, SG, XX) happens in bunch/linebreak.h.
//
// kSaCombiningRanges holds the SA code points whose General_Category is Mn or
// Mc.  LB1 resolves those to CM and all remaining SA code points to AL; the
// split is recorded here because it depends on General_Category, which the
// algorithm does not otherwise need.
//
// kEastAsianWideOpenCloseRanges holds the OP and CP code points whose
// East_Asian_Width is F, W, or H (EastAsianWidth.txt).  LB30 only prohibits
// breaks around opening and closing punctuation that is not East Asian wide,
// so a break is allowed before U+FF08 and after U+FF09, for example.
//
// kUnassignedPictographicRanges holds unassigned (General_Category Cn) code
// points with the Extended_Pictographic property (emoji-data.txt), used by the
// second part of rule LB30b.

#include <cstddef>
#include <cstdint>

namespace bunch
{{
namespace detail
{{
namespace linebreak
{{

inline constexpr const char *kUnicodeVersion = "{UNICODE_VERSION}";
"""
    ]
    parts.append(format_enum("LineBreakClass", LINE_BREAK_VALUES))
    parts.append(
        """
struct PropertyRange
{
    std::uint32_t first;
    std::uint32_t last;
    std::uint8_t value;
};"""
    )

    def add_table(comment: str, symbol: str, entries: list[tuple[int, int, int]]) -> None:
        parts.append(
            f"\n// {comment}\n"
            f"inline constexpr PropertyRange {symbol}[] = {{\n"
            f"{format_ranges(entries)}\n"
            f"}};\n\n"
            f"inline constexpr std::size_t {symbol}Count = {len(entries)};"
        )

    add_table(
        "Line_Break (LineBreak.txt) with the @missing defaults; value 0 is XX.",
        "kPropertyRanges",
        properties,
    )
    add_table(
        "SA code points with General_Category Mn or Mc; value 1 means LB1 resolves SA to CM.",
        "kSaCombiningRanges",
        sa_combining,
    )
    add_table(
        "OP and CP code points with East_Asian_Width F, W, or H; value 1 excludes them from LB30.",
        "kEastAsianWideOpenCloseRanges",
        east_asian,
    )
    add_table(
        "Unassigned (Cn) Extended_Pictographic code points; value 1 means LB30b applies.",
        "kUnassignedPictographicRanges",
        unassigned_pictographic,
    )

    parts.append(
        """
// Binary search over a sorted range table.  Code points outside every range
// have the default value 0.
inline auto lookupProperty(const PropertyRange *ranges, std::size_t count, std::uint32_t codepoint) -> std::uint8_t
{
    std::size_t low = 0;
    std::size_t high = count;
    while (low < high)
    {
        const std::size_t middle = low + (high - low) / 2;
        if (ranges[middle].last < codepoint)
        {
            low = middle + 1;
        }
        else
        {
            high = middle;
        }
    }
    if (low < count && ranges[low].first <= codepoint)
    {
        return ranges[low].value;
    }
    return 0;
}

// Line_Break property of a code point (XX for code points outside every range).
inline auto lineBreakProperty(std::uint32_t codepoint) -> LineBreakClass
{
    return static_cast<LineBreakClass>(lookupProperty(kPropertyRanges, kPropertyRangesCount, codepoint));
}

inline auto isSaCombining(std::uint32_t codepoint) -> bool
{
    return lookupProperty(kSaCombiningRanges, kSaCombiningRangesCount, codepoint) != 0;
}

inline auto isEastAsianWideOpenClose(std::uint32_t codepoint) -> bool
{
    return lookupProperty(kEastAsianWideOpenCloseRanges, kEastAsianWideOpenCloseRangesCount, codepoint) != 0;
}

inline auto isUnassignedPictographic(std::uint32_t codepoint) -> bool
{
    return lookupProperty(kUnassignedPictographicRanges, kUnassignedPictographicRangesCount, codepoint) != 0;
}

} // namespace linebreak
} // namespace detail
} // namespace bunch
"""
    )
    return "\n".join(parts)


def parse_line_break_tests(path: Path) -> list[tuple[bytes, list[int], int]]:
    """Returns (utf8 text, expected byte offsets, source line) per case.

    Each line is a sequence of marker/code point pairs.  A break marker before a
    code point is an expected break opportunity at the current UTF-8 byte
    offset.  LB2 (sot x) means the test never reports offset 0, but
    lineBreakOpportunities always reports it, so offset 0 is prepended here.
    """
    cases = []
    for line_number, raw in enumerate(read_text(path).splitlines(), 1):
        data = raw.split("#", 1)[0].strip()
        if not data:
            continue
        text = bytearray()
        boundaries: list[int] = []
        marker = None
        for token in data.split():
            if token in (BREAK, NO_BREAK):
                if marker is not None:
                    raise SystemExit(f"{path.name}:{line_number}: two markers in a row")
                marker = token
                continue
            if marker is None:
                raise SystemExit(f"{path.name}:{line_number}: code point without a marker")
            codepoint = int(token, 16)
            if codepoint > MAX_CODEPOINT or 0xD800 <= codepoint <= 0xDFFF:
                raise SystemExit(f"{path.name}:{line_number}: invalid code point {token!r}")
            if marker == BREAK:
                boundaries.append(len(text))
            text.extend(chr(codepoint).encode("utf-8"))
            marker = None
        if marker != BREAK:
            raise SystemExit(f"{path.name}:{line_number}: test must end with {BREAK!r}")
        boundaries.append(len(text))
        if not boundaries or boundaries[0] != 0:
            boundaries.insert(0, 0)
        if boundaries != sorted(set(boundaries)):
            raise SystemExit(f"{path.name}:{line_number}: boundaries are not strictly ascending")
        if boundaries[-1] != len(text):
            raise SystemExit(f"{path.name}:{line_number}: last boundary is not the text end")
        if len(boundaries) > 0xFFFF:
            raise SystemExit(f"{path.name}:{line_number}: too many boundaries")
        if len(text) > 0xFFFF:
            raise SystemExit(f"{path.name}:{line_number}: test text is too long")
        cases.append((bytes(text), boundaries, line_number))
    if not cases:
        raise SystemExit(f"{path.name}: no test cases")
    return cases


def cpp_byte_literal(data: bytes) -> str:
    tokens = []
    for byte in data:
        if byte == 0x0A:
            token = "\\n"
        elif byte == 0x0D:
            token = "\\r"
        elif byte == 0x09:
            token = "\\t"
        elif byte == 0x22:
            token = '\\"'
        elif byte == 0x5C:
            token = "\\\\"
        elif 0x20 <= byte < 0x7F:
            token = chr(byte)
        else:
            token = f"\\x{byte:02X}"
        tokens.append(token)

    parts: list[str] = []
    for token in tokens:
        # A hexadecimal escape consumes every following hex digit, so split the
        # literal when the next character could extend the escape.
        if parts and parts[-1].startswith("\\x") and len(token) == 1 and token in "0123456789abcdefABCDEF":
            parts.append('""')
        parts.append(token)
    return '"' + "".join(parts) + '"'


def emit_vectors_header(cases: list[tuple[bytes, list[int], int]]) -> str:
    text_lines: list[str] = []
    break_values: list[int] = []
    case_lines: list[str] = []
    text_offset = 0
    break_offset = 0
    for text, boundaries, line_number in cases:
        text_lines.append(f"    // LineBreakTest.txt:{line_number} ({len(text)} bytes)")
        text_lines.append(f"    {cpp_byte_literal(text)}")
        break_values.extend(boundaries)
        case_lines.append(
            f"    {{{text_offset}, {break_offset}, {len(text)}, {len(boundaries)}, {line_number}}},"
        )
        text_offset += len(text)
        break_offset += len(boundaries)

    if text_offset > 0xFFFFFFFF:
        raise SystemExit("text blob does not fit in uint32_t")
    if break_offset > 0xFFFFFFFF:
        raise SystemExit("break blob does not fit in uint32_t")

    lines = [
        f"""#pragma once

// Generated by tools/gen_linebreak_data.py from the Unicode {UNICODE_VERSION}
// LineBreakTest.txt file.  Do not edit by hand.
//
// Every case from the file is included ({len(cases)} cases); nothing is sampled.
// The UTF-8 texts and the expected byte offsets of the break opportunities are
// concatenated into flat arrays, and each Vector describes one slice of both.
// The expected offsets match bunch::lineBreakOpportunities exactly, so they
// include 0 and text.size() even though LineBreakTest.txt itself reports no
// break at the start of text (LB2).

#include <cstdint>

namespace linebreak_vectors
{{

struct Vector
{{
    std::uint32_t textOffset;   // offset into kText
    std::uint32_t breakOffset;  // offset into kBreaks
    std::uint16_t textLength;   // length of the UTF-8 text in bytes
    std::uint16_t breakCount;   // number of expected break opportunities
    std::uint32_t sourceLine;   // line in LineBreakTest.txt, for diagnostics
}};

inline constexpr char kText[] =""",
        "\n".join(text_lines) + ";",
        "",
        "inline constexpr std::uint16_t kBreaks[] = {",
    ]
    for index in range(0, len(break_values), 20):
        chunk = break_values[index:index + 20]
        lines.append("    " + ", ".join(str(value) for value in chunk) + ",")
    lines.append("};")
    lines.append("")
    lines.append("inline constexpr Vector kVectors[] = {")
    lines.extend(case_lines)
    lines.append("};")
    lines.append("")
    lines.append("} // namespace linebreak_vectors")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ucd", help="directory containing the Unicode 15.0.0 UCD text files")
    parser.add_argument("--data", type=Path, default=DATA_PATH, help="output path for linebreak_data.h")
    parser.add_argument("--vectors", type=Path, default=VECTORS_PATH, help="output path for linebreak_vectors.h")
    parser.add_argument("--check", action="store_true", help="fail if the checked-in headers differ")
    arguments = parser.parse_args()

    ucd = resolve_ucd_directory(arguments.ucd)
    check_version(ucd / "LineBreak.txt", f"LineBreak-{UNICODE_VERSION}.txt")
    check_version(ucd / "LineBreakTest.txt", f"LineBreakTest-{UNICODE_VERSION}.txt")
    check_version(ucd / "EastAsianWidth.txt", f"EastAsianWidth-{UNICODE_VERSION}.txt")
    check_version(ucd / "emoji-data.txt", f"Emoji Version {EMOJI_VERSION}")

    classes = load_line_break(ucd / "LineBreak.txt")
    assigned, mark = load_general_category(ucd / "UnicodeData.txt")
    wide = load_east_asian_width(ucd / "EastAsianWidth.txt")
    pictographic = load_extended_pictographic(ucd / "emoji-data.txt")

    sa_combining = bytearray(MAX_CODEPOINT + 1)
    op_or_cp = bytearray(MAX_CODEPOINT + 1)
    unassigned_pictographic = bytearray(MAX_CODEPOINT + 1)
    op = PROPERTY_INDEX["OP"]
    cp = PROPERTY_INDEX["CP"]
    sa = PROPERTY_INDEX["SA"]
    for codepoint in range(MAX_CODEPOINT + 1):
        value = classes[codepoint]
        if value == sa and mark[codepoint]:
            sa_combining[codepoint] = 1
        if (value == op or value == cp) and wide[codepoint]:
            op_or_cp[codepoint] = 1
        if pictographic[codepoint] and not assigned[codepoint]:
            unassigned_pictographic[codepoint] = 1

    properties = collapse_classes(classes)
    sa_ranges = single_value_ranges(sa_combining)
    east_asian_ranges = single_value_ranges(op_or_cp)
    unassigned_ranges = single_value_ranges(unassigned_pictographic)

    cases = parse_line_break_tests(ucd / "LineBreakTest.txt")

    data_text = emit_data_header(properties, sa_ranges, east_asian_ranges, unassigned_ranges)
    vectors_text = emit_vectors_header(cases)

    if arguments.check:
        failures = []
        for path, expected in ((arguments.data, data_text), (arguments.vectors, vectors_text)):
            if not path.is_file():
                failures.append(f"{path} is missing")
            elif path.read_text(encoding="utf-8") != expected:
                failures.append(f"{path} is out of date")
        if failures:
            raise SystemExit("; ".join(failures) + "; rerun tools/gen_linebreak_data.py")
        print("line break data and vectors are up to date")
        return 0

    for path, text in ((arguments.data, data_text), (arguments.vectors, vectors_text)):
        path.write_text(text, encoding="utf-8", newline="\n")

    print(
        f"wrote {arguments.data} ({len(data_text)} bytes, {len(properties)} Line_Break + "
        f"{len(sa_ranges)} SA combining + {len(east_asian_ranges)} East Asian OP/CP + "
        f"{len(unassigned_ranges)} unassigned pictographic ranges)"
    )
    print(f"wrote {arguments.vectors} ({len(vectors_text)} bytes, {len(cases)} cases)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
