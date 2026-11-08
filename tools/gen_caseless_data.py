"""Generates bunch/caseless_data.h, the NFKC_Casefold mapping table.

NFKC_Casefold is the comparison form defined by UAX #15.  As described in
DerivedNormalizationProps.txt, it is constructed by applying NFKC, the full
case folding (CaseFolding.txt statuses C + F) and removal of every code point
with the Default_Ignorable_Code_Point property, repeating the process until
the result is stable.  This script derives that mapping for each code point
individually with Python's unicodedata module (Unicode 15.0.0) and reads the
Default_Ignorable_Code_Point ranges from the Unicode Character Database file
DerivedCoreProperties.txt.  When DerivedNormalizationProps.txt is available
the result is compared entry by entry with the published NFKC_CF property.
caseless.h applies the table code point by code point and normalises the
whole string to NFKC at the end, which merges sequences that only compose
across code point boundaries.

Usage:
    python tools/gen_caseless_data.py           # regenerate the header
    python tools/gen_caseless_data.py --check   # verify it is up to date
    python tools/gen_caseless_data.py --ucd PATH
    python tools/gen_caseless_data.py --normalization-props PATH

--ucd accepts either DerivedCoreProperties.txt or a directory that contains
it.  Without the option the file is looked up in $UCD_DIR and then in
tools/ucd/.  It can be downloaded from
https://www.unicode.org/Public/15.0.0/ucd/DerivedCoreProperties.txt .
DerivedNormalizationProps.txt is optional and enables an extra cross-check;
it is picked up automatically when it sits next to the DerivedCoreProperties
file.

The generated header contains only ASCII source text: code points are written
as hexadecimal numbers, never as literal characters.
"""

from __future__ import annotations

import os
import re
import sys
import unicodedata
from pathlib import Path

UNICODE_VERSION = "15.0.0"
MAX_CODE_POINT = 0x110000
SURROGATE_START = 0xD800
SURROGATE_END = 0xDFFF
MAX_HEADER_BYTES = 500 * 1024
UCD_FILE_NAME = "DerivedCoreProperties.txt"
TARGET = Path(__file__).resolve().parent.parent / "bunch" / "caseless_data.h"

# The mapping the generator must produce for a few well-known code points.
# They pin down the order of the operations: NFKC first (so U+2126 OHM SIGN
# and U+1E9B fold like the characters they normalise to), case folding second,
# NFKC again, default-ignorable removal last.
SPOT_CHECKS = {
    0x0041: (0x0061,),                  # A -> a
    0x00DF: (0x0073, 0x0073),           # sharp s -> ss
    0x1E9E: (0x0073, 0x0073),           # capital sharp s -> ss
    0x0130: (0x0069, 0x0307),           # I with dot above -> i + dot above
    0x00AD: (),                         # soft hyphen is default-ignorable
    0x0345: (0x03B9,),                  # combining ypogegrammeni -> iota
    0x1E9B: (0x1E61,),                  # long s with dot -> s with dot above
    0x2126: (0x03C9,),                  # ohm sign -> small omega
    0x212A: (0x006B,),                  # kelvin sign -> k
    0x2160: (0x0069,),                  # roman numeral one -> i
    0x24B6: (0x0061,),                  # circled capital A -> a
    0xFB01: (0x0066, 0x0069),           # fi ligature -> fi
    0x1F130: (0x0061,),                 # squared capital A -> a
}


def is_surrogate(codepoint: int) -> bool:
    return SURROGATE_START <= codepoint <= SURROGATE_END


def ucd_candidates(explicit: str | None):
    if explicit is not None:
        path = Path(explicit)
        yield path / UCD_FILE_NAME if path.is_dir() else path
        return

    environment = os.environ.get("UCD_DIR")
    if environment:
        path = Path(environment)
        yield path / UCD_FILE_NAME if path.is_dir() else path

    yield Path(__file__).resolve().parent / "ucd" / UCD_FILE_NAME


def locate_ucd(explicit: str | None) -> Path:
    tried = []
    for candidate in ucd_candidates(explicit):
        tried.append(str(candidate))
        if candidate.is_file():
            return candidate
    print(
        "error: cannot find " + UCD_FILE_NAME + "; looked in:\n  " + "\n  ".join(tried),
        file=sys.stderr,
    )
    print(
        "download it from "
        f"https://www.unicode.org/Public/{UNICODE_VERSION}/ucd/{UCD_FILE_NAME} "
        "and pass --ucd PATH",
        file=sys.stderr,
    )
    raise SystemExit(2)


def load_default_ignorable(path: Path) -> set[int]:
    """Returns the code points with Default_Ignorable_Code_Point = Yes.

    DerivedCoreProperties.txt stores the property as ranges, with comments
    after '#'.  The file header names the Unicode version, which must match
    Python's database so that the table and the case folding agree.
    """
    text = path.read_text(encoding="utf-8")
    match = re.search(r"DerivedCoreProperties-([0-9.]+)\.txt", text)
    if match is None:
        print(f"error: {path} does not look like DerivedCoreProperties.txt", file=sys.stderr)
        raise SystemExit(2)
    file_version = match.group(1)
    if file_version != UNICODE_VERSION:
        print(
            f"error: {path.name} is Unicode {file_version}, "
            f"but this generator produces Unicode {UNICODE_VERSION} data",
            file=sys.stderr,
        )
        raise SystemExit(2)

    ignorable = set()
    for line in text.splitlines():
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        fields = [field.strip() for field in line.split(";")]
        if len(fields) < 2 or fields[1] != "Default_Ignorable_Code_Point":
            continue
        span = fields[0]
        if ".." in span:
            first, last = span.split("..")
            ignorable.update(range(int(first, 16), int(last, 16) + 1))
        else:
            ignorable.add(int(span, 16))

    if not ignorable:
        print(f"error: {path} contains no Default_Ignorable_Code_Point data", file=sys.stderr)
        raise SystemExit(2)
    return ignorable


def single_codepoint_mapping(codepoint: int, ignorable: set[int]) -> tuple[int, ...]:
    """NFKC_Casefold of one code point.

    DerivedNormalizationProps.txt describes NFKC_CF as "constructed by
    applying NFKC, CaseFolding, and removal of Default_Ignorable_Code_Points",
    with the process repeated "until a stable result is produced".  One pass is
    already stable for Unicode 15.0.0; the loop makes that a checked property
    rather than an assumption.
    """
    text = chr(codepoint)
    for _ in range(8):
        previous = text
        text = unicodedata.normalize("NFKC", text)
        text = text.casefold()
        text = unicodedata.normalize("NFKC", text)
        text = "".join(character for character in text if ord(character) not in ignorable)
        if text == previous:
            break
    else:
        raise SystemExit(f"error: NFKC_Casefold(U+{codepoint:04X}) does not converge")
    return tuple(ord(character) for character in text)


def nfkc_casefold_string(text: str, ignorable: set[int]) -> str:
    """NFKC_Casefold of a whole string; used to check the table's closure."""
    text = unicodedata.normalize("NFKC", text)
    text = text.casefold()
    text = unicodedata.normalize("NFKC", text)
    return "".join(character for character in text if ord(character) not in ignorable)


def load_nfkc_cf(path: Path) -> dict[int, tuple[int, ...]]:
    """Reads the NFKC_CF entries of DerivedNormalizationProps.txt.

    Range lines expand to one entry per code point.  Entries whose mapping is
    the code point itself are dropped, matching the generated table.
    """
    entries: dict[int, tuple[int, ...]] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        fields = [field.strip() for field in line.split(";")]
        if len(fields) < 3 or fields[1] != "NFKC_CF":
            continue
        mapping = tuple(int(value, 16) for value in fields[2].split()) if fields[2] else ()
        span = fields[0]
        if ".." in span:
            first, last = span.split("..")
            codepoints = range(int(first, 16), int(last, 16) + 1)
        else:
            codepoints = (int(span, 16),)
        for codepoint in codepoints:
            if mapping != (codepoint,):
                entries[codepoint] = mapping
    return entries


def verify_against_ucd(entries, pool, official: dict[int, tuple[int, ...]]) -> None:
    """Compares the generated table with the official NFKC_CF property."""
    table = {codepoint: tuple(pool[offset:offset + length]) for codepoint, offset, length in entries}
    if table == official:
        return
    for codepoint in sorted(set(table) | set(official)):
        expected = official.get(codepoint, (codepoint,))
        actual = table.get(codepoint, (codepoint,))
        if expected != actual:
            raise SystemExit(
                f"error: NFKC_CF(U+{codepoint:04X}) is "
                f"{[f'U+{value:04X}' for value in actual]}, the UCD says "
                f"{[f'U+{value:04X}' for value in expected]}"
            )
    raise SystemExit("error: generated table differs from the UCD NFKC_CF property")


def build_table(ignorable: set[int]):
    """Returns (entries, pool, turkic_entries) for every non-identity mapping.

    entries are (codepoint, pool offset, length); a length of 0 deletes the
    code point.  turkic_entries are (codepoint, replacement) for the code
    points whose NFKC form is "I" or U+0130 LATIN CAPITAL LETTER I WITH DOT
    ABOVE, which the Turkic tailoring folds to dotless i and plain i
    respectively.
    """
    entries = []
    pool: list[int] = []
    for codepoint in range(MAX_CODE_POINT):
        if is_surrogate(codepoint):
            continue
        mapping = single_codepoint_mapping(codepoint, ignorable)
        if mapping == (codepoint,):
            continue
        entries.append((codepoint, len(pool), len(mapping)))
        pool.extend(mapping)

    turkic_entries = []
    for codepoint in range(MAX_CODE_POINT):
        if is_surrogate(codepoint):
            continue
        key = unicodedata.normalize("NFKC", chr(codepoint))
        if key == "I":
            turkic_entries.append((codepoint, 0x0131))  # LATIN SMALL LETTER DOTLESS I
        elif key == "\u0130":  # LATIN CAPITAL LETTER I WITH DOT ABOVE
            turkic_entries.append((codepoint, 0x0069))  # LATIN SMALL LETTER I
    return entries, pool, turkic_entries


def verify(entries, pool, turkic_entries, ignorable: set[int]) -> None:
    """Fails loudly if the derivation or the generated table is inconsistent."""

    def mapping_of(codepoint: int) -> tuple[int, ...]:
        for entry in entries:
            if entry[0] == codepoint:
                return tuple(pool[entry[1]:entry[1] + entry[2]])
        return (codepoint,)

    for codepoint, expected in SPOT_CHECKS.items():
        actual = mapping_of(codepoint)
        if actual != expected:
            raise SystemExit(
                f"error: NFKC_Casefold(U+{codepoint:04X}) is "
                f"{[f'U+{value:04X}' for value in actual]}, expected "
                f"{[f'U+{value:04X}' for value in expected]}"
            )

    for codepoint, _, _ in entries:
        if is_surrogate(codepoint):
            raise SystemExit(f"error: table contains surrogate U+{codepoint:04X}")

    for index in range(1, len(entries)):
        if entries[index - 1][0] >= entries[index][0]:
            raise SystemExit(
                f"error: table is not strictly ascending at U+{entries[index][0]:04X}"
            )

    # Every mapping value must already be NFKC_Casefolded: that is what makes
    # folding idempotent and lets caseless.h stop after one NFKC pass.
    for codepoint, offset, length in entries:
        value = "".join(chr(value) for value in pool[offset:offset + length])
        refolded = nfkc_casefold_string(value, ignorable)
        if refolded != value:
            raise SystemExit(
                f"error: NFKC_Casefold(U+{codepoint:04X}) is not closed under folding: "
                f"{[f'U+{ord(character):04X}' for character in value]} -> "
                f"{[f'U+{ord(character):04X}' for character in refolded]}"
            )

    turkic_codepoints = {codepoint for codepoint, _ in turkic_entries}
    for required in (0x0049, 0x0130, 0xFF29, 0x2160):
        if required not in turkic_codepoints:
            raise SystemExit(f"error: missing Turkic entry for U+{required:04X}")
    for index in range(1, len(turkic_entries)):
        if turkic_entries[index - 1][0] >= turkic_entries[index][0]:
            raise SystemExit(
                f"error: Turkic table is not strictly ascending at U+{turkic_entries[index][0]:04X}"
            )


def chunk_lines(values, per_line: int, formatter) -> list[str]:
    lines = []
    for index in range(0, len(values), per_line):
        chunk = values[index:index + per_line]
        lines.append("        " + ", ".join(formatter(value) for value in chunk) + ",")
    return lines


def binary_search(name: str, entry_type: str, count_name: str) -> list[str]:
    return [
        f"inline const {entry_type} *find{name}(std::uint32_t codepoint)",
        "{",
        "    std::size_t low = 0;",
        f"    std::size_t high = {count_name};",
        "    while (low < high)",
        "    {",
        "        const std::size_t middle = low + (high - low) / 2;",
        f"        if (k{name}Table[middle].codepoint < codepoint)",
        "        {",
        "            low = middle + 1;",
        "        }",
        "        else",
        "        {",
        "            high = middle;",
        "        }",
        "    }",
        f"    if (low < {count_name} && k{name}Table[low].codepoint == codepoint)",
        "    {",
        f"        return &k{name}Table[low];",
        "    }",
        "    return nullptr;",
        "}",
    ]


def locate_normalization_props(explicit: str | None, ucd_path: Path) -> Path | None:
    """Finds DerivedNormalizationProps.txt for the optional cross-check."""
    candidates = []
    if explicit is not None:
        candidates.append(Path(explicit))
    candidates.append(ucd_path.with_name("DerivedNormalizationProps.txt"))
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    return None


def generate(ucd_argument: str | None, normalization_props: str | None):
    ucd_path = locate_ucd(ucd_argument)
    ignorable = load_default_ignorable(ucd_path)
    entries, pool, turkic_entries = build_table(ignorable)
    verify(entries, pool, turkic_entries, ignorable)

    props_path = locate_normalization_props(normalization_props, ucd_path)
    if props_path is None:
        print(
            "note: DerivedNormalizationProps.txt not found next to "
            f"{ucd_path.name}; skipping the NFKC_CF cross-check",
            file=sys.stderr,
        )
    else:
        official = load_nfkc_cf(props_path)
        verify_against_ucd(entries, pool, official)
        print(f"verified {len(official)} NFKC_CF entries against {props_path.name}")

    out = []
    out.append("#pragma once")
    out.append("")
    out.append("// Generated by tools/gen_caseless_data.py from Python's unicodedata")
    out.append(f"// (Unicode {UNICODE_VERSION}) and DerivedCoreProperties.txt.")
    out.append("// Do not edit by hand.")
    out.append("//")
    out.append("// NFKC_Casefold (UAX #15; the NFKC_CF property of")
    out.append("// DerivedNormalizationProps.txt), constructed the way the UCD describes:")
    out.append("// NFKC, full case folding, NFKC again and removal of every")
    out.append("// Default_Ignorable_Code_Point, repeated until the result is stable.")
    out.append("// Entries are sorted by code point so caseless.h can binary-search them.")
    out.append("// A length of 0 deletes the code point; values longer than one code")
    out.append("// point live in kCaselessPool.")
    out.append("//")
    out.append("// kCaselessTurkicTable lists the code points whose NFKC form is \"I\" or")
    out.append("// U+0130 LATIN CAPITAL LETTER I WITH DOT ABOVE.  The Turkic tailoring")
    out.append("// folds those to U+0131 LATIN SMALL LETTER DOTLESS I and U+0069 LATIN")
    out.append("// SMALL LETTER I instead of their default entry.")
    out.append("")
    out.append("#include <cstddef>")
    out.append("#include <cstdint>")
    out.append("")
    out.append("namespace bunch")
    out.append("{")
    out.append("namespace detail")
    out.append("{")
    out.append("namespace unicode")
    out.append("{")
    out.append(f'inline constexpr const char *kCaselessVersion = "{UNICODE_VERSION}";')
    out.append("")
    out.append("struct CaselessEntry")
    out.append("{")
    out.append("    std::uint32_t codepoint;")
    out.append("    std::uint32_t offset;")
    out.append("    std::uint8_t length;")
    out.append("};")
    out.append("")
    out.append("inline constexpr CaselessEntry kCaselessTable[] = {")
    out.extend(chunk_lines(entries, 4, lambda entry: f"{{0x{entry[0]:X}, {entry[1]}, {entry[2]}}}"))
    out.append("};")
    out.append("")
    out.append("inline constexpr std::uint32_t kCaselessPool[] = {")
    out.extend(chunk_lines(pool, 12, lambda value: f"0x{value:X}"))
    out.append("};")
    out.append("")
    out.append("struct CaselessTurkicEntry")
    out.append("{")
    out.append("    std::uint32_t codepoint;")
    out.append("    std::uint32_t replacement;")
    out.append("};")
    out.append("")
    out.append("inline constexpr CaselessTurkicEntry kCaselessTurkicTable[] = {")
    out.extend(chunk_lines(turkic_entries, 4, lambda entry: f"{{0x{entry[0]:X}, 0x{entry[1]:X}}}"))
    out.append("};")
    out.append("")
    out.append("inline constexpr std::size_t kCaselessCount =")
    out.append("    sizeof(kCaselessTable) / sizeof(kCaselessTable[0]);")
    out.append("")
    out.append("inline constexpr std::size_t kCaselessTurkicCount =")
    out.append("    sizeof(kCaselessTurkicTable) / sizeof(kCaselessTurkicTable[0]);")
    out.append("")
    out.extend(binary_search("Caseless", "CaselessEntry", "kCaselessCount"))
    out.append("")
    out.extend(binary_search("CaselessTurkic", "CaselessTurkicEntry", "kCaselessTurkicCount"))
    out.append("")
    out.append("} // namespace unicode")
    out.append("} // namespace detail")
    out.append("} // namespace bunch")
    out.append("")
    return "\n".join(out), entries, pool, turkic_entries


def main(argv) -> int:
    check = False
    ucd_argument = None
    normalization_props = None
    arguments = argv[1:]
    index = 0
    while index < len(arguments):
        argument = arguments[index]
        if argument == "--check":
            check = True
        elif argument in ("--ucd", "-ucd"):
            index += 1
            if index >= len(arguments):
                print("error: --ucd needs a path", file=sys.stderr)
                return 2
            ucd_argument = arguments[index]
        elif argument.startswith("--ucd="):
            ucd_argument = argument.split("=", 1)[1]
        elif argument in ("--normalization-props", "-normalization-props"):
            index += 1
            if index >= len(arguments):
                print("error: --normalization-props needs a path", file=sys.stderr)
                return 2
            normalization_props = arguments[index]
        elif argument.startswith("--normalization-props="):
            normalization_props = argument.split("=", 1)[1]
        else:
            print(f"error: unknown argument {argument!r}", file=sys.stderr)
            print(
                "usage: gen_caseless_data.py [--check] [--ucd PATH] "
                "[--normalization-props PATH]",
                file=sys.stderr,
            )
            return 2
        index += 1

    if unicodedata.unidata_version != UNICODE_VERSION:
        print(
            f"error: Python's Unicode database is {unicodedata.unidata_version}, "
            f"but this generator produces Unicode {UNICODE_VERSION} data",
            file=sys.stderr,
        )
        return 2

    content, entries, pool, turkic_entries = generate(ucd_argument, normalization_props)
    if any(ord(character) > 0x7F for character in content):
        print("error: generated header would contain non-ASCII source text", file=sys.stderr)
        return 2

    encoded = content.encode("ascii")
    if len(encoded) > MAX_HEADER_BYTES:
        print(
            f"error: generated header is {len(encoded)} bytes, over the {MAX_HEADER_BYTES} byte budget",
            file=sys.stderr,
        )
        return 2

    if check:
        if not TARGET.exists():
            print(f"error: missing {TARGET}", file=sys.stderr)
            return 1
        try:
            # Compare raw ASCII bytes so a CRLF conversion or a stray non-ASCII
            # character is reported as a mismatch instead of being normalised.
            current = TARGET.read_bytes().decode("ascii")
        except UnicodeDecodeError:
            print(
                f"error: {TARGET.name} contains non-ASCII bytes; run tools/gen_caseless_data.py",
                file=sys.stderr,
            )
            return 1
        if current == content:
            print(
                f"{TARGET.name} is up to date: {len(entries)} mappings, "
                f"{len(pool)} pool code points, {len(turkic_entries)} Turkic entries"
            )
            return 0
        current_lines = current.splitlines()
        new_lines = content.splitlines()
        difference = 0
        while difference < min(len(current_lines), len(new_lines)) and (
            current_lines[difference] == new_lines[difference]
        ):
            difference += 1
        print(
            f"error: {TARGET.name} is out of date "
            f"(first difference at line {difference + 1}); run tools/gen_caseless_data.py",
            file=sys.stderr,
        )
        return 1

    TARGET.parent.mkdir(parents=True, exist_ok=True)
    TARGET.write_text(content, encoding="ascii", newline="\n")
    print(f"wrote {TARGET} ({TARGET.stat().st_size} bytes)")
    print(
        f"caseless mappings={len(entries)} pool={len(pool)} "
        f"turkic={len(turkic_entries)}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
