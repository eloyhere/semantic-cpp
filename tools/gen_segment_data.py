"""Generates bunch/segment_data.h and tests/segment_vectors.h.

Both headers are build artefacts derived from the Unicode Character Database
(version 15.0.0); they are not hand-edited code.  The script is deterministic:
the same UCD input produces byte-identical output, so `--check` can be used in
CI to verify that the checked-in headers are current.

Input files (all Unicode 15.0.0):
    GraphemeBreakProperty.txt, WordBreakProperty.txt,
    SentenceBreakProperty.txt, emoji-data.txt,
    GraphemeBreakTest.txt, WordBreakTest.txt, SentenceBreakTest.txt

The UCD directory is resolved in this order:
    --ucd DIR, $UCD_DIR, $TEMP/i18n-data/ucd, <repo>/unicode

Usage:
    python tools/gen_segment_data.py [--ucd DIR] [--check]
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
DATA_PATH = REPOSITORY / "bunch" / "segment_data.h"
VECTORS_PATH = REPOSITORY / "tests" / "segment_vectors.h"

BREAK = "\u00f7"  # division sign, marks a boundary
NO_BREAK = "\u00d7"  # multiplication sign, marks a joined pair

# Enum members in declaration order; the integer value is the index.  These
# names must stay in sync with bunch/segment_data.h.
GRAPHEME_VALUES = [
    ("other", None),
    ("cr", "CR"),
    ("lf", "LF"),
    ("control", "Control"),
    ("extend", "Extend"),
    ("zwj", "ZWJ"),
    ("regionalIndicator", "Regional_Indicator"),
    ("prepend", "Prepend"),
    ("spacingMark", "SpacingMark"),
    ("l", "L"),
    ("v", "V"),
    ("t", "T"),
    ("lv", "LV"),
    ("lvt", "LVT"),
]

WORD_VALUES = [
    ("other", None),
    ("cr", "CR"),
    ("lf", "LF"),
    ("newline", "Newline"),
    ("extend", "Extend"),
    ("format", "Format"),
    ("katakana", "Katakana"),
    ("hebrewLetter", "Hebrew_Letter"),
    ("aLetter", "ALetter"),
    ("singleQuote", "Single_Quote"),
    ("doubleQuote", "Double_Quote"),
    ("midNumLet", "MidNumLet"),
    ("midLetter", "MidLetter"),
    ("midNum", "MidNum"),
    ("numeric", "Numeric"),
    ("extendNumLet", "ExtendNumLet"),
    ("zwj", "ZWJ"),
    ("wSegSpace", "WSegSpace"),
    ("regionalIndicator", "Regional_Indicator"),
]

SENTENCE_VALUES = [
    ("other", None),
    ("cr", "CR"),
    ("lf", "LF"),
    ("extend", "Extend"),
    ("sep", "Sep"),
    ("format", "Format"),
    ("sp", "Sp"),
    ("lower", "Lower"),
    ("upper", "Upper"),
    ("oLetter", "OLetter"),
    ("numeric", "Numeric"),
    ("aTerm", "ATerm"),
    ("sTerm", "STerm"),
    ("close", "Close"),
    ("sContinue", "SContinue"),
]


def property_index(values) -> dict[str, int]:
    return {name: index for index, (_, name) in enumerate(values) if name is not None}


GRAPHEME_INDEX = property_index(GRAPHEME_VALUES)
WORD_INDEX = property_index(WORD_VALUES)
SENTENCE_INDEX = property_index(SENTENCE_VALUES)


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
        if (candidate / "GraphemeBreakProperty.txt").is_file():
            return candidate
    raise SystemExit(
        "Unicode data directory not found; pass --ucd DIR or set UCD_DIR. "
        "Expected GraphemeBreakProperty.txt in one of: "
        + ", ".join(str(candidate) for candidate in candidates)
    )


def check_version(path: Path, needle: str) -> None:
    head = "\n".join(read_text(path).splitlines()[:12])
    if needle not in head:
        raise SystemExit(f"{path.name}: expected {needle!r} in the header, got:\n{head}")


def parse_properties(
    path: Path,
    allowed: dict[str, int],
    check_overlaps: bool = True,
) -> dict[str, list[tuple[int, int]]]:
    """Parses a UCD property file into {property name: [ranges]}.

    The UCD files are not required to be sorted and may split adjacent code
    points into several lines, so the ranges are merged here.  Missing code
    points have the property's default value (Other), which is not stored.
    """
    ranges: dict[str, list[tuple[int, int]]] = {}
    for line_number, raw in enumerate(read_text(path).splitlines(), 1):
        if raw.lstrip().startswith("#"):
            continue
        data = raw.split("#", 1)[0].strip()
        if not data:
            continue
        fields = [field.strip() for field in data.split(";")]
        if len(fields) != 2:
            raise SystemExit(f"{path.name}:{line_number}: cannot parse {raw!r}")
        codepoints, name = fields
        if name not in allowed:
            raise SystemExit(f"{path.name}:{line_number}: unknown property {name!r}")
        if ".." in codepoints:
            first_text, last_text = codepoints.split("..", 1)
        else:
            first_text = last_text = codepoints
        first, last = int(first_text, 16), int(last_text, 16)
        if first > last or last > MAX_CODEPOINT:
            raise SystemExit(f"{path.name}:{line_number}: invalid range {codepoints!r}")
        ranges.setdefault(name, []).append((first, last))

    merged: dict[str, list[tuple[int, int]]] = {}
    for name, entries in ranges.items():
        entries.sort()
        result: list[tuple[int, int]] = []
        for first, last in entries:
            if result and first <= result[-1][1] + 1:
                previous_first, previous_last = result[-1]
                result[-1] = (previous_first, max(previous_last, last))
            else:
                result.append((first, last))
        merged[name] = result

    if check_overlaps:
        flat = sorted((first, last, name) for name, entries in merged.items() for first, last in entries)
        for index in range(1, len(flat)):
            if flat[index][0] <= flat[index - 1][1]:
                raise SystemExit(
                    f"{path.name}: overlapping ranges {flat[index - 1]!r} and {flat[index]!r}"
                )
    return merged


def flatten_properties(merged: dict[str, list[tuple[int, int]]], values) -> list[tuple[int, int, int]]:
    index = property_index(values)
    entries = [
        (first, last, index[name])
        for name in merged
        for first, last in merged[name]
        if name != "other"
    ]
    entries.sort()
    return entries


def parse_break_tests(path: Path) -> list[tuple[str, list[int], int]]:
    """Returns (utf8 text, expected byte boundaries, source line) per case.

    Each line is a sequence of ÷/× markers interleaved with hexadecimal code
    points.  A ÷ marker is an expected boundary at the current UTF-8 byte
    offset; the marker before the first code point is always a boundary and the
    trailing marker gives the final boundary at the end of the text.
    """
    cases = []
    lines = read_text(path).splitlines()
    name = path.name
    for line_number, raw in enumerate(lines, 1):
        if not raw.strip() or raw.lstrip().startswith("#"):
            continue
        data = raw.split("#", 1)[0].strip()
        if not data:
            continue
        tokens = data.split()
        text = bytearray()
        boundaries: list[int] = []
        marker = None
        for token in tokens:
            if token in (BREAK, NO_BREAK):
                if marker is not None:
                    raise SystemExit(f"{name}:{line_number}: two markers in a row")
                marker = token
                continue
            if marker is None:
                raise SystemExit(f"{name}:{line_number}: code point without a marker")
            codepoint = int(token, 16)
            if codepoint > MAX_CODEPOINT or 0xD800 <= codepoint <= 0xDFFF:
                raise SystemExit(f"{name}:{line_number}: invalid code point {token!r}")
            if marker == BREAK:
                if boundaries and boundaries[-1] == len(text):
                    raise SystemExit(f"{name}:{line_number}: duplicate boundary")
                boundaries.append(len(text))
            text.extend(chr(codepoint).encode("utf-8"))
            marker = None
        if marker != BREAK:
            raise SystemExit(f"{name}:{line_number}: test must end with {BREAK!r}")
        boundaries.append(len(text))
        if boundaries[0] != 0:
            raise SystemExit(f"{name}:{line_number}: first boundary is not zero")
        if boundaries[-1] != len(text):
            raise SystemExit(f"{name}:{line_number}: last boundary is not the text end")
        if boundaries != sorted(set(boundaries)):
            raise SystemExit(f"{name}:{line_number}: boundaries are not strictly ascending")
        if len(boundaries) - 1 > 255:
            raise SystemExit(f"{name}:{line_number}: more than 255 clusters")
        if len(text) > 0xFFFF:
            raise SystemExit(f"{name}:{line_number}: test text is too long")
        cases.append((bytes(text), boundaries, line_number))
    if not cases:
        raise SystemExit(f"{name}: no test cases")
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


def format_ranges(entries: list[tuple[int, int, int]]) -> str:
    lines = []
    for index in range(0, len(entries), 4):
        chunk = entries[index:index + 4]
        rendered = ", ".join(
            f"{{0x{first:04X}, 0x{last:04X}, {value}}}" for first, last, value in chunk
        )
        lines.append("        " + rendered + ",")
    return "\n".join(lines)


def format_enum(name: str, values) -> str:
    lines = [f"enum class {name} : std::uint8_t", "{"]
    members = []
    for index, (member, ucd_name) in enumerate(values):
        if ucd_name is None:
            members.append(f"    {member} = 0")
        else:
            members.append(f"    {member} = {index}")
    lines.append(",\n".join(members))
    lines.append("};")
    return "\n".join(lines)


def emit_data_header(
    grapheme: list[tuple[int, int, int]],
    word: list[tuple[int, int, int]],
    sentence: list[tuple[int, int, int]],
    pictographic: list[tuple[int, int, int]],
) -> str:
    parts = []
    parts.append(
        """#pragma once

// Generated by tools/gen_segment_data.py from Unicode 15.0.0 data. Do not edit by hand.
//
// Code point property tables for UAX #29 text segmentation.  Each table is
// sorted by code point and stores only ranges whose value is not the default
// (Other), so lookups use binary search over at most a few thousand ranges.

#include <cstddef>
#include <cstdint>

namespace bunch
{
namespace detail
{
namespace segment
{

inline constexpr const char *kUnicodeVersion = "15.0.0";
"""
    )
    parts.append(format_enum("GraphemeBreak", GRAPHEME_VALUES))
    parts.append("")
    parts.append(format_enum("WordBreak", WORD_VALUES))
    parts.append("")
    parts.append(format_enum("SentenceBreak", SENTENCE_VALUES))
    parts.append(
        """
struct PropertyRange
{
    std::uint32_t first;
    std::uint32_t last;
    std::uint8_t value;
};
"""
    )

    def add_table(comment: str, symbol: str, entries: list[tuple[int, int, int]]) -> None:
        parts.append(
            f"\n// {comment}\n"
            f"inline constexpr PropertyRange {symbol}[] = {{\n"
            f"{format_ranges(entries)}\n"
            f"}};\n\n"
            f"inline constexpr std::size_t {symbol}Count = {len(entries)};"
        )

    add_table("Grapheme_Cluster_Break (GraphemeBreakProperty.txt).", "kGraphemeRanges", grapheme)
    add_table("Word_Break (WordBreakProperty.txt).", "kWordRanges", word)
    add_table("Sentence_Break (SentenceBreakProperty.txt).", "kSentenceRanges", sentence)
    add_table(
        "Extended_Pictographic (emoji-data.txt); the range value is always 1.",
        "kExtendedPictographicRanges",
        pictographic,
    )

    parts.append(
        """
// Binary search over a sorted range table.  Code points outside every range
// have the default value 0 (Other).
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

inline auto graphemeBreak(std::uint32_t codepoint) -> GraphemeBreak
{
    return static_cast<GraphemeBreak>(lookupProperty(kGraphemeRanges, kGraphemeRangesCount, codepoint));
}

inline auto wordBreak(std::uint32_t codepoint) -> WordBreak
{
    return static_cast<WordBreak>(lookupProperty(kWordRanges, kWordRangesCount, codepoint));
}

inline auto sentenceBreak(std::uint32_t codepoint) -> SentenceBreak
{
    return static_cast<SentenceBreak>(lookupProperty(kSentenceRanges, kSentenceRangesCount, codepoint));
}

inline auto isExtendedPictographic(std::uint32_t codepoint) -> bool
{
    return lookupProperty(kExtendedPictographicRanges, kExtendedPictographicRangesCount, codepoint) != 0;
}

} // namespace segment
} // namespace detail
} // namespace bunch
"""
    )
    return "\n".join(parts)


def emit_case_data(
    name: str,
    cases: list[tuple[bytes, list[int], int]],
) -> str:
    text_lines = []
    break_values: list[int] = []
    case_lines = []
    text_offset = 0
    break_offset = 0
    for text, boundaries, line_number in cases:
        text_lines.append(f"    // {name}BreakTest.txt:{line_number} ({len(text)} bytes)")
        text_lines.append(f"    {cpp_byte_literal(text)}")
        for boundary in boundaries:
            if boundary > 0xFFFF:
                raise SystemExit("boundary offset does not fit in uint16_t")
            break_values.append(boundary)
        case_lines.append(
            f"    {{{text_offset}, {break_offset}, {len(text)}, {line_number}, {len(boundaries)}}},"
        )
        text_offset += len(text)
        break_offset += len(boundaries)

    if text_offset > 0xFFFFFFFF:
        raise SystemExit("text blob does not fit in uint32_t")

    lines = [
        f"inline constexpr char k{name}Text[] =",
        "\n".join(text_lines) + ";",
        "",
        f"inline constexpr std::uint16_t k{name}Breaks[] = {{",
    ]
    for index in range(0, len(break_values), 20):
        chunk = break_values[index:index + 20]
        lines.append("    " + ", ".join(str(value) for value in chunk) + ",")
    lines.append("};")
    lines.append("")
    lines.append(f"inline constexpr BreakCase k{name}Cases[] = {{")
    lines.extend(case_lines)
    lines.append("};")
    return "\n".join(lines)


def emit_vectors_header(grapheme_cases, word_cases, sentence_cases) -> str:
    total = len(grapheme_cases) + len(word_cases) + len(sentence_cases)
    parts = [
        f"""#pragma once

// Generated by tools/gen_segment_data.py from the Unicode 15.0.0
// GraphemeBreakTest.txt, WordBreakTest.txt and SentenceBreakTest.txt files.
// Do not edit by hand.
//
// Every case from all three files is included ({total} cases: {len(grapheme_cases)}
// grapheme, {len(word_cases)} word, {len(sentence_cases)} sentence); nothing is sampled.
// The UTF-8 texts and the expected byte boundaries are concatenated into flat
// arrays, and each BreakCase describes one slice of both.

#include <cstdint>

namespace segment_vectors
{{

struct BreakCase
{{
    std::uint32_t textOffset;   // offset into the matching k*Text array
    std::uint32_t breakOffset;  // offset into the matching k*Breaks array
    std::uint16_t textLength;   // length of the UTF-8 text in bytes
    std::uint16_t sourceLine;   // line in the upstream test file, for diagnostics
    std::uint8_t breakCount;    // number of expected boundaries
}};
"""
    ]
    parts.append(emit_case_data("Grapheme", grapheme_cases))
    parts.append("")
    parts.append(emit_case_data("Word", word_cases))
    parts.append("")
    parts.append(emit_case_data("Sentence", sentence_cases))
    parts.append("\n} // namespace segment_vectors\n")
    return "\n".join(parts)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ucd", help="directory containing the Unicode 15.0.0 UCD text files")
    parser.add_argument("--data", type=Path, default=DATA_PATH, help="output path for segment_data.h")
    parser.add_argument("--vectors", type=Path, default=VECTORS_PATH, help="output path for segment_vectors.h")
    parser.add_argument("--check", action="store_true", help="fail if the checked-in headers differ")
    arguments = parser.parse_args()

    ucd = resolve_ucd_directory(arguments.ucd)
    property_files = {
        "GraphemeBreakProperty.txt": f"GraphemeBreakProperty-{UNICODE_VERSION}.txt",
        "WordBreakProperty.txt": f"WordBreakProperty-{UNICODE_VERSION}.txt",
        "SentenceBreakProperty.txt": f"SentenceBreakProperty-{UNICODE_VERSION}.txt",
        "emoji-data.txt": f"Emoji Version {EMOJI_VERSION}",
        "GraphemeBreakTest.txt": f"GraphemeBreakTest-{UNICODE_VERSION}.txt",
        "WordBreakTest.txt": f"WordBreakTest-{UNICODE_VERSION}.txt",
        "SentenceBreakTest.txt": f"SentenceBreakTest-{UNICODE_VERSION}.txt",
    }
    for name, needle in property_files.items():
        check_version(ucd / name, needle)

    grapheme = flatten_properties(parse_properties(ucd / "GraphemeBreakProperty.txt", GRAPHEME_INDEX), GRAPHEME_VALUES)
    word = flatten_properties(parse_properties(ucd / "WordBreakProperty.txt", WORD_INDEX), WORD_VALUES)
    sentence = flatten_properties(parse_properties(ucd / "SentenceBreakProperty.txt", SENTENCE_INDEX), SENTENCE_VALUES)
    emoji = parse_properties(
        ucd / "emoji-data.txt",
        {
            "Emoji": 0,
            "Emoji_Presentation": 0,
            "Emoji_Modifier": 0,
            "Emoji_Modifier_Base": 0,
            "Emoji_Component": 0,
            "Extended_Pictographic": 0,
        },
        check_overlaps=False,
    )
    pictographic = [(first, last, 1) for first, last in emoji["Extended_Pictographic"]]
    if not pictographic:
        raise SystemExit("emoji-data.txt: no Extended_Pictographic ranges")

    grapheme_cases = parse_break_tests(ucd / "GraphemeBreakTest.txt")
    word_cases = parse_break_tests(ucd / "WordBreakTest.txt")
    sentence_cases = parse_break_tests(ucd / "SentenceBreakTest.txt")

    data_text = emit_data_header(grapheme, word, sentence, pictographic)
    vectors_text = emit_vectors_header(grapheme_cases, word_cases, sentence_cases)

    if arguments.check:
        failures = []
        for path, expected in ((arguments.data, data_text), (arguments.vectors, vectors_text)):
            if not path.is_file():
                failures.append(f"{path} is missing")
            elif path.read_text(encoding="utf-8") != expected:
                failures.append(f"{path} is out of date")
        if failures:
            raise SystemExit("; ".join(failures) + "; rerun tools/gen_segment_data.py")
        print("segment data and vectors are up to date")
        return 0

    for path, text in ((arguments.data, data_text), (arguments.vectors, vectors_text)):
        path.write_text(text, encoding="utf-8", newline="\n")

    print(
        f"wrote {arguments.data} ({len(data_text)} bytes, {len(grapheme)} grapheme + "
        f"{len(word)} word + {len(sentence)} sentence + {len(pictographic)} pictographic ranges)"
    )
    print(
        f"wrote {arguments.vectors} ({len(vectors_text)} bytes, {len(grapheme_cases)} grapheme + "
        f"{len(word_cases)} word + {len(sentence_cases)} sentence cases)"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
