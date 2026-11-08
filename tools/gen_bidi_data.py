"""Generates bunch/bidi_data.h and tests/bidi_vectors.h for UAX #9.

The data header contains the Bidi_Class property for every code point (merged
into sorted ranges), the Bidi_Paired_Bracket property used by rule N0, and the
Bidi_Mirroring_Glyph property used by rule L4.  Classes come from Python's
unicodedata (Unicode 15.0.0); unassigned code points get the @missing defaults
from DerivedBidiClass-15.0.0.txt instead of the empty string unicodedata
reports for them.

The vector header is a stratified sample of BidiCharacterTest-15.0.0.txt.  The
official file has 91,707 paragraphs (about 7 MB), far more than is useful for
a unit test, so the generator keeps:

  * every paragraph that exercises numbers, NSMs, explicit formatting
    characters, isolates, or automatic paragraph direction detection;
  * one paragraph for every distinct (paragraph direction, Bidi_Class
    sequence) signature in the file, so every class pattern is represented;
  * at least thirty paragraphs for every distinct code point that occurs in
    the file, so bracket characters, digits, and combining marks are covered;
  * a uniform stride over the remaining paragraphs until the byte budget is
    reached.

The selection is deterministic.  The generated header records the category
counts so the C++ test can assert that all required categories are present.

The data header can be regenerated on its own with ``--data-only`` (the test
vectors are left untouched) and verified with ``--check``, which rebuilds both
outputs in memory and compares them with the working tree.  A successful
``--check`` therefore also proves that generation is reproducible.
"""

from __future__ import annotations

import argparse
import os
import sys
import unicodedata
from pathlib import Path

MAX_CP = 0x110000
ROOT = Path(__file__).resolve().parent.parent
DEFAULT_VECTOR_BUDGET = 1_400_000

# Enum order is mirrored by BidiClass in bunch/bidi_data.h.  Only the
# enumerator names matter to the C++ code, not the order.
CLASS_NAMES = [
    "L", "R", "AL", "EN", "ES", "ET", "AN", "CS", "NSM", "BN", "B", "S",
    "WS", "ON", "LRE", "LRO", "RLE", "RLO", "PDF", "LRI", "RLI", "FSI", "PDI",
]
NAME_TO_TOKEN = {name: name.lower() for name in CLASS_NAMES}

# @missing defaults from DerivedBidiClass-15.0.0.txt.  The general default
# comes first; later, narrower ranges override it for unassigned code points
# in the RTL scripts.  unicodedata.bidirectional() returns "" for those.
MISSING_DEFAULTS = [
    (0x0000, 0x10FFFF, "L"),
    (0x0590, 0x05FF, "R"),
    (0x0600, 0x07BF, "AL"),
    (0x07C0, 0x085F, "R"),
    (0x0860, 0x08FF, "AL"),
    (0x20A0, 0x20CF, "ET"),
    (0xFB1D, 0xFB4F, "R"),
    (0xFB50, 0xFDCF, "AL"),
    (0xFDF0, 0xFDFF, "AL"),
    (0xFE70, 0xFEFF, "AL"),
    (0x10800, 0x10CFF, "R"),
    (0x10D00, 0x10D3F, "AL"),
    (0x10D40, 0x10EBF, "R"),
    (0x10EC0, 0x10EFF, "AL"),
    (0x10F00, 0x10F2F, "R"),
    (0x10F30, 0x10F6F, "AL"),
    (0x10F70, 0x10FFF, "R"),
    (0x1E800, 0x1EC6F, "R"),
    (0x1EC70, 0x1ECBF, "AL"),
    (0x1ECC0, 0x1ECFF, "R"),
    (0x1ED00, 0x1ED4F, "AL"),
    (0x1ED50, 0x1EDFF, "R"),
    (0x1EE00, 0x1EEFF, "AL"),
    (0x1EF00, 0x1EFFF, "R"),
]

ISOLATE_CODEPOINTS = {0x2066, 0x2067, 0x2068, 0x2069}
EXPLICIT_CODEPOINTS = {0x202A, 0x202B, 0x202C, 0x202D, 0x202E}


def default_class(codepoint: int) -> str:
    """Returns the @missing Bidi_Class name for an unassigned code point."""
    result = "L"
    for first, last, name in MISSING_DEFAULTS:
        if first <= codepoint <= last:
            result = name
    return result


def class_of(character: str) -> str:
    name = unicodedata.bidirectional(character)
    if not name:
        name = default_class(ord(character))
    return name


def build_class_ranges():
    """Merges consecutive code points with the same Bidi_Class into ranges."""
    ranges = []
    start = 0
    current = None
    for codepoint in range(MAX_CP):
        name = unicodedata.bidirectional(chr(codepoint))
        if not name:
            name = default_class(codepoint)
        if name != current:
            if current is not None:
                ranges.append((start, codepoint - 1, current))
            start = codepoint
            current = name
    ranges.append((start, MAX_CP - 1, current))
    return ranges


def build_brackets(path: Path):
    """Parses BidiBrackets.txt into sorted (code point, pair, opening)."""
    entries = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        parts = [part.strip() for part in line.split(";")]
        if len(parts) < 3:
            continue
        codepoint = int(parts[0], 16)
        paired = int(parts[1], 16)
        opening = parts[2] == "o"
        if parts[2] not in ("o", "c"):
            raise ValueError(f"unexpected bracket type in {path}: {raw!r}")
        entries.append((codepoint, paired, opening))
    entries.sort()
    return entries


def build_mirrors(path: Path):
    """Parses BidiMirroring.txt into sorted (code point, mirror) pairs.

    The file has two hexadecimal fields per mapping line:
    ``0028; 0029 # LEFT PARENTHESIS``.  Every line either maps a code point
    with Bidi_Mirrored=Yes to another code point or is blank/comment.  The
    Unicode 15.0.0 mappings are symmetric, but the generator does not rely on
    that; it only validates the ranges and rejects duplicates.
    """
    entries = []
    seen = set()
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        parts = [part.strip() for part in line.split(";")]
        if len(parts) < 2:
            raise ValueError(f"malformed mirror line in {path}: {raw!r}")
        codepoint = int(parts[0], 16)
        mirrored = int(parts[1], 16)
        for value in (codepoint, mirrored):
            if not 0 <= value <= 0x10FFFF or 0xD800 <= value <= 0xDFFF:
                raise ValueError(f"invalid code point in {path}: {raw!r}")
        if codepoint in seen:
            raise ValueError(f"duplicate mirror entry in {path}: {raw!r}")
        seen.add(codepoint)
        if codepoint == mirrored:
            raise ValueError(f"self mirror in {path}: {raw!r}")
        entries.append((codepoint, mirrored))
    entries.sort()
    return entries


def emit_data_header(ranges, brackets, mirrors) -> str:
    out = []
    out.append("#pragma once")
    out.append("")
    out.append("// ===========================================================================")
    out.append("// bunch/bidi_data.h")
    out.append("//")
    out.append("// Generated by tools/gen_bidi_data.py from Unicode 15.0.0:")
    out.append("//   - Bidi_Class for every code point (sorted ranges, binary search),")
    out.append("//   - Bidi_Paired_Bracket and Bidi_Paired_Bracket_Type (rule N0),")
    out.append("//   - Bidi_Mirroring_Glyph (rule L4, sorted pairs, binary search).")
    out.append("//")
    out.append("// Unassigned code points use the @missing defaults from")
    out.append("// DerivedBidiClass-15.0.0.txt.  Do not edit this file by hand.")
    out.append("// ===========================================================================")
    out.append("")
    out.append("#include <cstddef>")
    out.append("#include <cstdint>")
    out.append("")
    out.append("namespace bunch")
    out.append("{")
    out.append("enum class BidiClass : std::uint8_t")
    out.append("{")
    out.append("    " + ", ".join(NAME_TO_TOKEN[name] for name in CLASS_NAMES))
    out.append("};")
    out.append("")
    out.append("namespace detail")
    out.append("{")
    out.append("namespace bidi")
    out.append("{")
    out.append('inline constexpr const char *kVersion = "15.0.0";')
    out.append("")
    out.append("struct ClassRange")
    out.append("{")
    out.append("    std::uint32_t first;")
    out.append("    std::uint32_t last;")
    out.append("    BidiClass cls;")
    out.append("};")
    out.append("")
    out.append("struct BracketPair")
    out.append("{")
    out.append("    std::uint32_t codepoint;")
    out.append("    std::uint32_t paired;")
    out.append("    bool opening;")
    out.append("};")
    out.append("")
    out.append("struct MirrorPair")
    out.append("{")
    out.append("    std::uint32_t codepoint;")
    out.append("    std::uint32_t mirrored;")
    out.append("};")
    out.append("")
    out.append("inline constexpr ClassRange kClassRanges[] = {")
    for index in range(0, len(ranges), 3):
        chunk = ranges[index:index + 3]
        entries = []
        for first, last, name in chunk:
            entries.append(
                f"{{0x{first:04X}, 0x{last:04X}, BidiClass::{NAME_TO_TOKEN[name]}}}")
        out.append("        " + ", ".join(entries) + ",")
    out.append("};")
    out.append("")
    out.append("inline constexpr std::size_t kClassRangeCount =")
    out.append("    sizeof(kClassRanges) / sizeof(kClassRanges[0]);")
    out.append("")
    out.append("inline constexpr BracketPair kBracketPairs[] = {")
    for index in range(0, len(brackets), 2):
        chunk = brackets[index:index + 2]
        entries = []
        for codepoint, paired, opening in chunk:
            entries.append(
                f"{{0x{codepoint:04X}, 0x{paired:04X}, {'true' if opening else 'false'}}}")
        out.append("        " + ", ".join(entries) + ",")
    out.append("};")
    out.append("")
    out.append("inline constexpr std::size_t kBracketPairCount =")
    out.append("    sizeof(kBracketPairs) / sizeof(kBracketPairs[0]);")
    out.append("")
    out.append("inline constexpr MirrorPair kMirrorPairs[] = {")
    for index in range(0, len(mirrors), 2):
        chunk = mirrors[index:index + 2]
        entries = []
        for codepoint, mirrored in chunk:
            entries.append(f"{{0x{codepoint:04X}, 0x{mirrored:04X}}}")
        out.append("        " + ", ".join(entries) + ",")
    out.append("};")
    out.append("")
    out.append("inline constexpr std::size_t kMirrorPairCount =")
    out.append("    sizeof(kMirrorPairs) / sizeof(kMirrorPairs[0]);")
    out.append("")
    out.append("// Returns the Bidi_Class of a Unicode scalar value.  The table covers")
    out.append("// 0..10FFFF, so the fallback only guards against corrupt input.")
    out.append("inline auto lookupClass(std::uint32_t codepoint) -> BidiClass")
    out.append("{")
    out.append("    std::size_t low = 0;")
    out.append("    std::size_t high = kClassRangeCount;")
    out.append("    while (low < high)")
    out.append("    {")
    out.append("        const std::size_t middle = low + (high - low) / 2;")
    out.append("        if (kClassRanges[middle].last < codepoint)")
    out.append("        {")
    out.append("            low = middle + 1;")
    out.append("        }")
    out.append("        else")
    out.append("        {")
    out.append("            high = middle;")
    out.append("        }")
    out.append("    }")
    out.append("    if (low < kClassRangeCount && kClassRanges[low].first <= codepoint)")
    out.append("    {")
    out.append("        return kClassRanges[low].cls;")
    out.append("    }")
    out.append("    return BidiClass::l;")
    out.append("}")
    out.append("")
    out.append("// Returns the bracket properties for a code point, or nullptr when the")
    out.append("// code point is not an opening or closing paired bracket.")
    out.append("inline auto lookupBracket(std::uint32_t codepoint) -> const BracketPair *")
    out.append("{")
    out.append("    std::size_t low = 0;")
    out.append("    std::size_t high = kBracketPairCount;")
    out.append("    while (low < high)")
    out.append("    {")
    out.append("        const std::size_t middle = low + (high - low) / 2;")
    out.append("        if (kBracketPairs[middle].codepoint < codepoint)")
    out.append("        {")
    out.append("            low = middle + 1;")
    out.append("        }")
    out.append("        else")
    out.append("        {")
    out.append("            high = middle;")
    out.append("        }")
    out.append("    }")
    out.append("    if (low < kBracketPairCount && kBracketPairs[low].codepoint == codepoint)")
    out.append("    {")
    out.append("        return &kBracketPairs[low];")
    out.append("    }")
    out.append("    return nullptr;")
    out.append("}")
    out.append("")
    out.append("// Returns the Bidi_Mirroring_Glyph of a code point, or the code point")
    out.append("// itself when the character has no mirror mapping.")
    out.append("inline auto lookupMirror(std::uint32_t codepoint) -> std::uint32_t")
    out.append("{")
    out.append("    std::size_t low = 0;")
    out.append("    std::size_t high = kMirrorPairCount;")
    out.append("    while (low < high)")
    out.append("    {")
    out.append("        const std::size_t middle = low + (high - low) / 2;")
    out.append("        if (kMirrorPairs[middle].codepoint < codepoint)")
    out.append("        {")
    out.append("            low = middle + 1;")
    out.append("        }")
    out.append("        else")
    out.append("        {")
    out.append("            high = middle;")
    out.append("        }")
    out.append("    }")
    out.append("    if (low < kMirrorPairCount && kMirrorPairs[low].codepoint == codepoint)")
    out.append("    {")
    out.append("        return kMirrorPairs[low].mirrored;")
    out.append("    }")
    out.append("    return codepoint;")
    out.append("}")
    out.append("")
    out.append("} // namespace bidi")
    out.append("} // namespace detail")
    out.append("} // namespace bunch")
    out.append("")
    return "\n".join(out)


def parse_test_lines(path: Path):
    records = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        fields = line.split(";")
        if len(fields) != 5:
            raise ValueError(f"expected five fields: {line[:80]!r}")
        codepoints = [int(token, 16) for token in fields[0].split()]
        direction = int(fields[1])
        levels = fields[3].split()
        order = fields[4].split()
        # The visual order lists only the characters that survive X9, so its
        # length matches the number of levels that are not 'x'.
        if len(levels) != len(codepoints) or len(order) != sum(
                token != "x" for token in levels):
            raise ValueError(f"malformed test line: {line[:80]!r}")
        classes = [class_of(chr(codepoint)) for codepoint in codepoints]
        records.append({
            "line": line,
            "codepoints": codepoints,
            "classes": classes,
            "direction": direction,
        })
    return records


def is_pure_ltr(classes) -> bool:
    has_l = "L" in classes
    has_rtl = "R" in classes or "AL" in classes
    return has_l and not has_rtl


def is_pure_rtl(classes) -> bool:
    has_rtl = "R" in classes or "AL" in classes
    return has_rtl and "L" not in classes


def select_vectors(records, brackets, budget):
    chosen = [False] * len(records)
    payload = 0
    bracket_codepoints = {entry[0] for entry in brackets} | {entry[1] for entry in brackets}

    def add(index: int) -> bool:
        nonlocal payload
        if chosen[index]:
            return True
        cost = len(records[index]["line"].encode("utf-8")) + 8
        if payload + cost > budget:
            return False
        chosen[index] = True
        payload += cost
        return True

    # Layer 1: every paragraph with a feature that is rare in the file.
    for index, record in enumerate(records):
        classes = record["classes"]
        special = (
            record["direction"] == 2
            or any(codepoint in EXPLICIT_CODEPOINTS for codepoint in record["codepoints"])
            or any(codepoint in ISOLATE_CODEPOINTS for codepoint in record["codepoints"])
            or any(name in ("EN", "AN", "NSM") for name in classes)
        )
        if special:
            add(index)

    # Layer 2: one paragraph per distinct class signature.
    signatures = {}
    for index, record in enumerate(records):
        signatures.setdefault((record["direction"], tuple(record["classes"])), index)
    for index in signatures.values():
        add(index)

    # Layer 3: at least thirty paragraphs per distinct code point.
    occurrences = {}
    for index, record in enumerate(records):
        for codepoint in set(record["codepoints"]):
            occurrences.setdefault(codepoint, []).append(index)
    for indices in occurrences.values():
        selected = sum(1 for index in indices if chosen[index])
        if selected >= 30:
            continue
        needed = 30 - selected
        step = max(1, len(indices) // 30)
        for offset in range(0, len(indices), step):
            if needed == 0:
                break
            if add(indices[offset]):
                needed -= 1

    # Layer 4: uniform stride over everything that is left.
    stride = 7
    for index in range(0, len(records), stride):
        if payload >= budget - 8192:
            break
        add(index)

    return chosen, payload, signatures, bracket_codepoints, occurrences


def emit_vector_header(records, chosen, budget, signatures, bracket_codepoints) -> str:
    selected = [index for index, keep in enumerate(chosen) if keep]
    counts = {
        "pure_ltr": 0,
        "pure_rtl": 0,
        "numbers": 0,
        "neutrals": 0,
        "brackets": 0,
        "explicit": 0,
        "isolates": 0,
        "auto": 0,
        "nsm": 0,
    }
    for index in selected:
        record = records[index]
        classes = record["classes"]
        codepoints = record["codepoints"]
        if is_pure_ltr(classes):
            counts["pure_ltr"] += 1
        if is_pure_rtl(classes):
            counts["pure_rtl"] += 1
        if any(name in ("EN", "AN") for name in classes):
            counts["numbers"] += 1
        if any(name in ("WS", "ON", "S", "B") for name in classes):
            counts["neutrals"] += 1
        if any(codepoint in bracket_codepoints for codepoint in codepoints):
            counts["brackets"] += 1
        if any(codepoint in EXPLICIT_CODEPOINTS for codepoint in codepoints):
            counts["explicit"] += 1
        if any(codepoint in ISOLATE_CODEPOINTS for codepoint in codepoints):
            counts["isolates"] += 1
        if record["direction"] == 2:
            counts["auto"] += 1
        if "NSM" in classes:
            counts["nsm"] += 1

    total_payload = sum(len(records[i]["line"].encode("utf-8")) + 8 for i in selected)
    covered_signatures = {
        (records[i]["direction"], tuple(records[i]["classes"])) for i in selected
    }
    out = []
    out.append("#pragma once")
    out.append("")
    out.append("// ===========================================================================")
    out.append("// tests/bidi_vectors.h")
    out.append("//")
    out.append("// Generated by tools/gen_bidi_data.py from BidiCharacterTest-15.0.0.txt.")
    out.append("// Do not edit this file by hand.")
    out.append("//")
    out.append(f"// Stratified sample: {len(selected)} of {len(records)} official paragraphs,")
    out.append(f"// {total_payload} bytes of vector payload (budget {budget}).  Layers:")
    out.append("//   1. every paragraph with numbers, NSMs, explicit formatting, isolates,")
    out.append("//      or automatic paragraph direction;")
    out.append(f"//   2. one paragraph for each of the {len(signatures)} distinct")
    out.append("//      (direction, Bidi_Class sequence) signatures in the file;")
    out.append("//   3. at least 30 paragraphs for every distinct code point in the file;")
    out.append("//   4. a uniform stride over the remaining paragraphs up to the budget.")
    out.append("//")
    out.append("// Each line keeps the official format")
    out.append("//   code points; paragraph direction; resolved paragraph level;")
    out.append("//   resolved levels ('x' for X9-removed characters); visual order.")
    out.append("// ===========================================================================")
    out.append("")
    out.append("#include <cstddef>")
    out.append("")
    out.append("namespace bidi_test_vectors")
    out.append("{")
    out.append("inline constexpr const char *const kLines[] = {")
    for index in selected:
        line = records[index]["line"]
        if any(character in line for character in ('"', "\\")):
            raise ValueError(f"vector line needs escaping: {line!r}")
        out.append('    "' + line + '",')
    out.append("};")
    out.append("")
    out.append("inline constexpr std::size_t kLineCount = sizeof(kLines) / sizeof(kLines[0]);")
    out.append("")
    out.append("// Category coverage asserted by tests/test_bidi.cpp.")
    for key, value in counts.items():
        identifier = "k" + "".join(part.capitalize() for part in key.split("_")) + "Count"
        out.append(f"inline constexpr std::size_t {identifier} = {value};")
    out.append(f"inline constexpr std::size_t kSignatureCount = {len(covered_signatures)};")
    out.append(f"inline constexpr std::size_t kTotalSignatureCount = {len(signatures)};")
    out.append("")
    out.append("} // namespace bidi_test_vectors")
    out.append("")
    return "\n".join(out)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    temp = Path(os.environ.get("TEMP", "/tmp"))
    parser.add_argument("--ucd", type=Path, default=temp / "i18n-data" / "ucd",
                        help="directory containing BidiCharacterTest.txt")
    parser.add_argument("--brackets", type=Path, default=None,
                        help="BidiBrackets.txt (default: --ucd/BidiBrackets.txt, then TEMP)")
    parser.add_argument("--mirrors", type=Path, default=None,
                        help="BidiMirroring.txt (default: --ucd/BidiMirroring.txt, then TEMP)")
    parser.add_argument("--output", type=Path, default=ROOT / "bunch" / "bidi_data.h")
    parser.add_argument("--vectors", type=Path, default=ROOT / "tests" / "bidi_vectors.h")
    parser.add_argument("--vectors-budget", type=int, default=DEFAULT_VECTOR_BUDGET)
    parser.add_argument("--data-only", action="store_true",
                        help="write only the data header and leave the test vectors alone")
    parser.add_argument("--check", action="store_true",
                        help="verify that the generated files match the working tree; write nothing")
    args = parser.parse_args()

    brackets_path = args.brackets
    if brackets_path is None:
        candidate = args.ucd / "BidiBrackets.txt"
        if candidate.exists():
            brackets_path = candidate
        else:
            brackets_path = temp / "BidiBrackets.txt"
    mirrors_path = args.mirrors
    if mirrors_path is None:
        candidate = args.ucd / "BidiMirroring.txt"
        if candidate.exists():
            mirrors_path = candidate
        else:
            mirrors_path = temp / "BidiMirroring.txt"
    test_path = args.ucd / "BidiCharacterTest.txt"

    required = [brackets_path, mirrors_path]
    if not args.data_only:
        required.append(test_path)
    for path in required:
        if not path.exists():
            parser.error(f"missing input file: {path}")

    ranges = build_class_ranges()
    brackets = build_brackets(brackets_path)
    mirrors = build_mirrors(mirrors_path)
    data_header = emit_data_header(ranges, brackets, mirrors)
    outputs = [(args.output, data_header)]
    record_count = 0
    payload = 0
    chosen = []

    if not args.data_only:
        records = parse_test_lines(test_path)
        chosen, payload, signatures, bracket_codepoints, _ = select_vectors(
            records, brackets, args.vectors_budget)
        vector_header = emit_vector_header(
            records, chosen, args.vectors_budget, signatures, bracket_codepoints)
        outputs.append((args.vectors, vector_header))
        record_count = len(records)

    if args.check:
        stale = []
        for path, content in outputs:
            if not path.exists() or path.read_text(encoding="utf-8") != content:
                stale.append(path)
        if stale:
            for path in stale:
                print(f"stale: {path}", file=sys.stderr)
            return 1
        print("up to date: " + ", ".join(str(path) for path, _ in outputs))
        return 0

    for path, content in outputs:
        path.write_text(content, encoding="utf-8", newline="\n")

    for path, _ in outputs:
        if path.stat().st_size >= 1_500_000:
            print(f"error: {path} exceeds the 1.5 MB limit", file=sys.stderr)
            return 1

    print(f"wrote {args.output} ({args.output.stat().st_size} bytes, "
          f"{len(ranges)} class ranges, {len(brackets)} bracket pairs, "
          f"{len(mirrors)} mirror pairs)")
    if not args.data_only:
        print(f"wrote {args.vectors} ({args.vectors.stat().st_size} bytes, "
              f"{len(chosen)} of {record_count} paragraphs, {payload} bytes of payload)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
