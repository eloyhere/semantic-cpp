"""Generates localisation/collation_data.h from the Unicode DUCET and CLDR.

The Default Unicode Collation Element Table (allkeys.txt) is published by the
Unicode Consortium for the Unicode Collation Algorithm (UTS #10).  This script
turns it into a compact, sorted C++17 table:

  * every collation-element sequence is stored once in a shared pool;
  * single-code-point mappings are two parallel sorted arrays, so lookups use
    binary search;
  * multi-code-point mappings (contractions, at most three code points in the
    DUCET) are a sorted array of keys;
  * the @implicitweights ranges of the source file and the Unicode
    Unified_Ideograph ranges of the matching Unicode version are emitted
    alongside the entries.

On top of the root table the script compiles the CLDR tailorings of the
locales listed in ``CLDR_LOCALES``: the ``standard`` collation of
``common/collation/<locale>.xml`` becomes a table of synthetic collation
elements, one per tailored contraction key.

Rule subset
-----------
Supported (everything the built-in locales need):

  * resets ``&X``, ``&[before 1]X`` (levels 1..3) and ``&[last regular]``
    (the DUCET has no Han boundary element, so the last explicit primary
    below the implicit lead unit 0xFB40 stands in for it);
  * relations ``<`` (primary), ``<<`` (secondary), ``<<<`` (tertiary), ``=``;
  * expansions (``/``), e.g. ``&cs<<<ccs/cs`` or ``&t<<<þ/h``;
  * contractions of one to three code points (``&H<ch``, ``&DZ<dzs``);
  * ``\\uXXXX`` escapes in rule text (used by fi for U+0335);
  * standalone ``[import ...]``, ``[reorder ...]`` and ``[caseFirst ...]``
    directives are ignored, as documented in the generated header.

Not supported, and either rejected loudly or excluded by locale choice:
``[import]`` payloads of other locales (re-importing would need the full CLDR
inheritance graph), variable weighting, ``[backwards 2]`` and script
reordering.  The ``ja``, ``zh`` and ``ko`` collations are intentionally not
built in: their rule files are 33 KB, 1.1 MB and 50 KB of script-specific
data, far outside the compact table this header keeps.

Usage:
    python tools/gen_collation_data.py [allkeys.txt] [output.h] [--check]
        [--collation-dir DIR] [--no-tailoring]

Without arguments the script reads ``%TEMP%/i18n-data/uca/allkeys.txt`` and
``%TEMP%/i18n-data/cldr-collation/`` and writes ``localisation/collation_data.h``
relative to the repository root.  The ``UCA_ALLKEYS`` and ``CLDR_COLLATION``
environment variables override the two input paths.  ``--check`` regenerates
into memory and fails when the checked-in header is out of date.

The output is deterministic: entries are sorted and every count is derived
from the input, so re-running the script on the same inputs produces a
byte-identical header.
"""

from __future__ import annotations

import argparse
import os
import re
import sys
import tempfile
import unicodedata
import xml.etree.ElementTree as ElementTree
from pathlib import Path

# ---------------------------------------------------------------------------
# Constants that describe the packed representation.  They must stay in sync
# with the decoding code in localisation/collation.h.
# ---------------------------------------------------------------------------

COUNT_BITS = 5
COUNT_MASK = (1 << COUNT_BITS) - 1
SECONDARY_BITS = 9
TERTIARY_BITS = 5
SECONDARY_MASK = (1 << SECONDARY_BITS) - 1
TERTIARY_MASK = (1 << TERTIARY_BITS) - 1
PRIMARY_SHIFT = SECONDARY_BITS + TERTIARY_BITS
SECONDARY_SHIFT = TERTIARY_BITS

MAX_CODE_POINTS = 0x110000
MAX_CONTRACTION_LENGTH = 3

# Tailored keys may be one code point longer than the DUCET contractions: the
# Hungarian doubled trigraph rules ("&dzs<<<ddzs/dzs") target four code points.
MAX_TAILORED_LENGTH = 4

# UTS #10 Table 16: for the siniform scripts the second primary weight is
# (CP - block start) | 0x8000, while the first is the constant lead unit that
# allkeys.txt gives on its @implicitweights line.
IMPLICIT_OFFSETS = {
    0xFB00: 0x17000,  # Tangut, Tangut Components, Tangut Supplement
    0xFB01: 0x1B170,  # Nushu
    0xFB02: 0x18B00,  # Khitan Small Script
}

# UTS #10 Table 16, Han rows: core Han ideographs live in the BMP blocks
# CJK_Unified_Ideographs and CJK_Compatibility_Ideographs and use 0xFB40 as
# their lead unit; every other unified ideograph uses 0xFB80.  Unassigned
# code points use 0xFBC0.
CORE_HAN_BLOCKS = ((0x4E00, 0x9FFF), (0xF900, 0xFAFF))
CORE_HAN_BASE = 0xFB40
OTHER_HAN_BASE = 0xFB80
UNASSIGNED_BASE = 0xFBC0

ENTRY_CE = re.compile(
    r"\[\s*([.*@])\s*([0-9A-Fa-f]{4,6})\.([0-9A-Fa-f]{4,6})\.([0-9A-Fa-f]{4,6})"
    r"(?:\.([0-9A-Fa-f]{4,6}))?\s*\]"
)
IMPLICIT_LINE = re.compile(
    r"^@implicitweights\s+([0-9A-Fa-f]{4,6})\.\.([0-9A-Fa-f]{4,6})\s*;\s*([0-9A-Fa-f]{4,6})\b"
)

# ---------------------------------------------------------------------------
# Tailoring constants.  A tailored locale compares in an expanded weight space
# so that synthetic weights fit between two adjacent root weights; the root
# data itself stays in the compact 16/9/5-bit space.  See collation_data.h.
# ---------------------------------------------------------------------------

TAILORING_PRIMARY_SHIFT = 16
TAILORING_SECONDARY_SHIFT = 4
TAILORING_TERTIARY_SHIFT = 3

DEFAULT_SECONDARY = 0x20 << TAILORING_SECONDARY_SHIFT      # 0x0200
LOWER_TERTIARY = 0x02 << TAILORING_TERTIARY_SHIFT          # 0x0010
UPPER_TERTIARY = 0x08 << TAILORING_TERTIARY_SHIFT          # 0x0040
SECONDARY_STEP = 1
TERTIARY_STEP = 0x08
BEFORE_PRIMARY_GAP = 0x100
BEFORE_SECONDARY_STEP = 1
BEFORE_TERTIARY_STEP = 0x08

# CLDR locales whose "standard" collation is compiled into the header, sorted.
# sv is deliberately absent: it keeps the hand-written Swedish tailoring of
# collation.h (a-ring, a-diaeresis, o-diaeresis after z), which predates the
# CLDR rule compiler and whose sort-key weights are part of the public tests.
CLDR_LOCALES = (
    "cs",
    "da",
    "de",
    "es",
    "et",
    "fi",
    "fr",
    "hu",
    "lt",
    "lv",
    "nb",
    "nn",
    "no",
    "pl",
    "ro",
    "ru",
    "sk",
    "tr",
)

# CLDR inheritance: nb and nn inherit the Norwegian collation of no.
CLDR_PARENTS = {"nb": "no", "nn": "no"}

# Rule directives accepted on a reset.  Any other [option] is an error: better
# to fail than to misread a rule.
IGNORED_DIRECTIVES = ("import", "reorder", "caseFirst", "backwards", "normalization")


class CollationDataError(RuntimeError):
    """Raised when the collation sources cannot be parsed without ambiguity."""


# ---------------------------------------------------------------------------
# Parsing: DUCET
# ---------------------------------------------------------------------------


def parse_allkeys(path: Path):
    """Returns (version, implicit_ranges, entries, variable_count).

    ``entries`` maps a tuple of code points to a tuple of (primary, secondary,
    tertiary) tuples.  ``implicit_ranges`` is a list of
    (first, last, base, offset) tuples.
    """
    version = None
    implicit_ranges = []
    entries = {}
    variable_count = 0

    with path.open("r", encoding="utf-8") as source:
        for line_number, raw_line in enumerate(source, 1):
            line = raw_line.split("#", 1)[0].strip()
            if not line:
                continue
            if line.startswith("@"):
                if line.startswith("@version"):
                    parts = line.split()
                    if len(parts) != 2:
                        raise CollationDataError(f"{path}:{line_number}: malformed @version line")
                    version = parts[1]
                elif line.startswith("@implicitweights"):
                    match = IMPLICIT_LINE.match(line)
                    if match is None:
                        raise CollationDataError(f"{path}:{line_number}: malformed @implicitweights line")
                    first = int(match.group(1), 16)
                    last = int(match.group(2), 16)
                    base = int(match.group(3), 16)
                    if base not in IMPLICIT_OFFSETS:
                        raise CollationDataError(
                            f"{path}:{line_number}: unsupported implicit-weight base 0x{base:04X}"
                        )
                    if first > last or last >= MAX_CODE_POINTS:
                        raise CollationDataError(f"{path}:{line_number}: invalid @implicitweights range")
                    implicit_ranges.append((first, last, base, IMPLICIT_OFFSETS[base]))
                # Other @-directives are intentionally ignored; the DUCET
                # currently defines only @version and @implicitweights.
                continue

            if ";" not in line:
                raise CollationDataError(f"{path}:{line_number}: missing ';' separator")
            codepoint_text, element_text = line.split(";", 1)
            codepoints = tuple(int(part, 16) for part in codepoint_text.split())
            if not codepoints:
                raise CollationDataError(f"{path}:{line_number}: empty code point list")
            if len(codepoints) > MAX_CONTRACTION_LENGTH:
                raise CollationDataError(
                    f"{path}:{line_number}: {len(codepoints)}-code-point contractions are not supported"
                )
            if any(cp >= MAX_CODE_POINTS for cp in codepoints):
                raise CollationDataError(f"{path}:{line_number}: code point out of range")

            matches = list(ENTRY_CE.finditer(element_text))
            if not matches:
                raise CollationDataError(f"{path}:{line_number}: no collation elements")
            remainder = ENTRY_CE.sub("", element_text).strip()
            if remainder:
                raise CollationDataError(f"{path}:{line_number}: unparsed collation element text {remainder!r}")
            if any(match.group(5) is not None for match in matches):
                raise CollationDataError(
                    f"{path}:{line_number}: four-level collation elements are not supported"
                )

            elements = []
            variable = False
            for match in matches:
                marker = match.group(1)
                if marker in ("*", "@"):
                    variable = True
                elements.append(
                    (int(match.group(2), 16), int(match.group(3), 16), int(match.group(4), 16))
                )
            if variable:
                variable_count += 1

            if codepoints in entries:
                raise CollationDataError(f"{path}:{line_number}: duplicate entry for {codepoints!r}")
            entries[codepoints] = tuple(elements)

    if version is None:
        raise CollationDataError(f"{path}: missing @version line")
    if not entries:
        raise CollationDataError(f"{path}: no collation element entries")

    implicit_ranges.sort(key=lambda item: item[0])
    for previous, current in zip(implicit_ranges, implicit_ranges[1:]):
        if current[0] <= previous[1]:
            raise CollationDataError(f"{path}: overlapping @implicitweights ranges")
    return version, implicit_ranges, entries, variable_count


# ---------------------------------------------------------------------------
# Parsing: CLDR collation rules
# ---------------------------------------------------------------------------

RULE_ESCAPE = re.compile(r"\\u([0-9A-Fa-f]{4})")
BEFORE_DIRECTIVE = re.compile(r"before\s+([1-3])")
LAST_REGULAR_DIRECTIVE = re.compile(r"last\s+regular")


def expand_escapes(text: str) -> str:
    """Replaces the \\uXXXX escapes that CLDR rule files use for marks."""
    return RULE_ESCAPE.sub(lambda match: chr(int(match.group(1), 16)), text)


def strip_comments(text: str) -> str:
    return "\n".join(line.split("#", 1)[0] for line in text.splitlines())


def split_rules(source: str):
    """Parses a CLDR <cr> rule body.

    Returns a list of (before_level, reset_text, [(relation, target_text), ...]).
    ``before_level`` is 0 unless the reset carried a ``[before N]`` option.
    Directive-only text (``[import ...]``) yields no rules.
    """
    text = strip_comments(source)
    length = len(text)
    index = 0
    rules = []

    def skip_space(position: int) -> int:
        while position < length and text[position].isspace():
            position += 1
        return position

    def read_text(position: int) -> tuple[str, int]:
        start = position
        while position < length and text[position] not in "<=&[":
            position += 1
        return text[start:position].strip(), position

    while index < length:
        index = skip_space(index)
        if index >= length:
            break
        if text[index] == "[":
            end = text.find("]", index)
            if end < 0:
                raise CollationDataError(f"unterminated directive in rule text: {text[index:index + 40]!r}")
            index = end + 1
            continue
        if text[index] != "&":
            raise CollationDataError(f"unexpected rule text {text[index:index + 40]!r}")
        index += 1
        before = 0
        logical = ""
        while True:
            index = skip_space(index)
            if index >= length or text[index] != "[":
                break
            end = text.find("]", index)
            if end < 0:
                raise CollationDataError("unterminated directive after '&'")
            directive = text[index + 1:end].strip()
            match = BEFORE_DIRECTIVE.fullmatch(directive)
            if match is not None:
                before = int(match.group(1))
            elif LAST_REGULAR_DIRECTIVE.fullmatch(directive):
                logical = "last regular"
            elif directive.startswith(IGNORED_DIRECTIVES):
                pass
            else:
                raise CollationDataError(f"unsupported reset option [{directive}]")
            index = end + 1

        index = skip_space(index)
        reset, index = read_text(index)
        if not reset and not logical:
            raise CollationDataError("reset without text")

        relations = []
        while True:
            index = skip_space(index)
            if index >= length or text[index] not in "<=":
                break
            if text[index] == "=":
                relation = "="
                index += 1
            else:
                relation = ""
                while index < length and text[index] == "<" and len(relation) < 3:
                    relation += "<"
                    index += 1
            target, index = read_text(index)
            if not target:
                raise CollationDataError(f"empty target after relation {relation!r}")
            relations.append((relation, target))
        if not relations:
            raise CollationDataError(f"reset {reset!r} without a relation")
        rules.append(
            (before, logical, expand_escapes(reset), [(rel, expand_escapes(t)) for rel, t in relations])
        )
    return rules


def load_locale_rules(path: Path):
    """Returns the compiled rule list of a locale's default collation type."""
    try:
        tree = ElementTree.parse(path)
    except ElementTree.ParseError as error:
        raise CollationDataError(f"{path}: {error}") from error
    root = tree.getroot()
    default_type = "standard"
    for element in root.iter("defaultCollation"):
        default_type = (element.text or "standard").strip()
    for collation in root.iter("collation"):
        if collation.get("type") != default_type or collation.get("alt") is not None:
            continue
        body = collation.find("cr")
        if body is None or not body.text:
            return []
        return split_rules(body.text)
    return []


def inheritance_chain(locale: str):
    """Returns the locale followed by its CLDR parents."""
    chain = []
    current = locale
    while current not in chain:
        chain.append(current)
        parent = CLDR_PARENTS.get(current)
        if parent is None:
            break
        current = parent
    return chain


def resolve_locale_rules(locale: str, directory: Path):
    """Returns (source path or None, rules) following CLDR inheritance.

    A locale file may exist and still carry no rules (nb.xml is empty and
    inherits no.xml), so every file of the chain is inspected.  A locale with
    no file at all is an error: silently falling back to root would hide a
    missing download.
    """
    found_file = None
    for candidate in inheritance_chain(locale):
        path = directory / f"{candidate}.xml"
        if not path.is_file():
            continue
        if found_file is None:
            found_file = path
        rules = load_locale_rules(path)
        if rules:
            return path, rules
    if found_file is None:
        raise CollationDataError(
            f"no CLDR collation file for {locale} in {directory} (looked for "
            + ", ".join(f"{name}.xml" for name in inheritance_chain(locale))
            + ")"
        )
    return found_file, []


# ---------------------------------------------------------------------------
# Compiling CLDR rules into synthetic collation elements
# ---------------------------------------------------------------------------


def expand_root_element(element) -> tuple[int, int, int]:
    primary, secondary, tertiary = element
    return (
        primary << TAILORING_PRIMARY_SHIFT,
        secondary << TAILORING_SECONDARY_SHIFT,
        tertiary << TAILORING_TERTIARY_SHIFT,
    )


def root_index(entries):
    """Maps a first code point to its root keys, longest first."""
    index = {}
    for codepoints in entries:
        index.setdefault(codepoints[0], []).append(codepoints)
    for value in index.values():
        value.sort(key=lambda item: (-len(item), item))
    return index


def resolve_text(text: str, tailored, index, entries):
    """Maps a rule text to the expanded collation elements it collates as."""
    codepoints = tuple(ord(char) for char in unicodedata.normalize("NFD", text))
    elements = []
    position = 0
    while position < len(codepoints):
        matched = None
        for length in (MAX_TAILORED_LENGTH, 3, 2, 1):
            key = codepoints[position:position + length]
            if len(key) == length and key in tailored:
                matched = key
                break
        if matched is None:
            for key in index.get(codepoints[position], ()):
                if codepoints[position:position + len(key)] == key:
                    matched = key
                    break
        if matched is None:
            raise CollationDataError(
                f"cannot resolve U+{codepoints[position]:04X} of rule text {text!r} in the root table"
            )
        if matched in tailored:
            elements.extend(tailored[matched])
        else:
            elements.extend(expand_root_element(element) for element in entries[matched])
        position += len(matched)
    if not elements:
        raise CollationDataError(f"rule text {text!r} has no collation elements")
    return elements


def case_tertiary(text: str) -> int:
    for char in text:
        category = unicodedata.category(char)
        if category in ("Lu", "Lt"):
            return UPPER_TERTIARY
        if category == "Ll":
            return LOWER_TERTIARY
    return LOWER_TERTIARY


def compute_element(relation: str, anchor, before_level: int, tertiary: int):
    """Builds the synthetic element of one relation.

    ``anchor`` is already in the expanded space.  A primary relation opens a new
    slot right above the anchor's primary; "before 1" uses the top of the gap
    below the anchor's primary so that a chain of "<" relations still ends up
    below the anchor.
    """
    primary, secondary, anchor_tertiary = anchor
    if relation == "=":
        return anchor
    if primary == 0:
        raise CollationDataError("a primary-ignorable reset cannot anchor a primary or secondary relation")
    if before_level == 1 and relation == "<":
        step = BEFORE_PRIMARY_GAP if primary % (1 << TAILORING_PRIMARY_SHIFT) == 0 else 1
        if primary < step:
            raise CollationDataError("cannot place a weight before the first primary")
        return (primary - step, DEFAULT_SECONDARY, tertiary)
    if before_level == 2 and relation == "<<":
        return (primary, secondary - BEFORE_SECONDARY_STEP, tertiary)
    if before_level == 3 and relation == "<<<":
        return (primary, secondary, anchor_tertiary - BEFORE_TERTIARY_STEP)
    if relation == "<":
        return (primary + 1, DEFAULT_SECONDARY, tertiary)
    if relation == "<<":
        return (primary, secondary + SECONDARY_STEP, tertiary)
    if relation == "<<<":
        return (primary, secondary, anchor_tertiary + TERTIARY_STEP)
    raise CollationDataError(f"unsupported relation {relation!r}")


def last_regular_element(entries):
    """Returns the CE of the last regular (non-implicit) root character.

    CLDR's "[last regular]" is the boundary collation element before the Han
    implicit weights.  The DUCET has no such boundary element, so the last
    explicit primary below the first implicit lead unit 0xFB40 is used; the
    two are the same position for every code point this library assigns an
    explicit weight to.
    """
    best = None
    for codepoints, elements in entries.items():
        if len(codepoints) != 1:
            continue
        primary = elements[0][0]
        if primary >= 0xFB40 or primary in (0xFFFD, 0xFFFF):
            continue
        if best is None or primary > best[0]:
            best = (primary, elements[0])
    if best is None:
        raise CollationDataError("the root table has no regular collation element")
    return expand_root_element(best[1])


def compile_rules(rules, index, entries):
    """Turns parsed rules into {code point tuple: element tuple}."""
    tailored = {}
    for before, logical, reset_text, relations in rules:
        # A '/' in a reset would introduce a reset-side expansion, which no
        # built-in locale uses; the text before the slash is the reset item.
        if logical:
            anchor_elements = [last_regular_element(entries)]
        else:
            anchor_elements = resolve_text(reset_text.split("/", 1)[0], tailored, index, entries)
        # Trailing primary-ignorable elements (a combining mark in the reset,
        # as in fi's "&D\u0335<<đ") are decoration, not the anchor.  A reset
        # that is itself primary-ignorable is only usable with "=" (lt's
        # "&̀=̇̀"), which is checked in compute_element().
        trimmed = list(anchor_elements)
        while len(trimmed) > 1 and trimmed[-1][0] == 0:
            trimmed.pop()
        anchors = [element for element in trimmed if element[0] != 0] or trimmed
        previous = anchors[0] if before else anchors[-1]
        # A multi-element reset is an expansion: the target keeps the leading
        # elements of the reset and replaces the last one.  That is how
        # "&th<<<þ" makes thorn a tertiary variant of the two-element sequence
        # "th" and "&oe<<œ" keeps the o of "oe".
        prefix = trimmed[:-1]
        for position, (relation, target) in enumerate(relations):
            text, _, expansion = target.partition("/")
            codepoints = tuple(ord(char) for char in unicodedata.normalize("NFD", text))
            if not codepoints:
                raise CollationDataError(f"empty target {target!r}")
            if len(codepoints) > MAX_TAILORED_LENGTH:
                raise CollationDataError(
                    f"target {target!r} has {len(codepoints)} code points; at most "
                    f"{MAX_TAILORED_LENGTH} are supported"
                )
            element = compute_element(
                relation,
                previous,
                before if position == 0 else 0,
                case_tertiary(text),
            )
            elements = prefix + [element]
            if expansion:
                elements.extend(resolve_text(expansion, tailored, index, entries))
            if codepoints in tailored:
                raise CollationDataError(
                    "duplicate tailoring for " + " ".join(f"U+{cp:04X}" for cp in codepoints)
                )
            tailored[codepoints] = tuple(elements)
            previous = element
    return tailored


def build_tailorings(entries, collation_dir: Path, locales):
    """Compiles the tailorings of ``locales`` into emission-ready rows.

    Returns (element_pool, entry_rows, locale_rows).  ``element_pool`` holds the
    expanded (primary, secondary, tertiary) triples; ``entry_rows`` holds
    (padded key, element reference) grouped by locale; ``locale_rows`` holds
    (tag, entry offset, entry count).  Locales that resolve to the same CLDR
    file (nb, nn and no) share one entry block.
    """
    index = root_index(entries)
    element_pool = []
    entry_rows = []
    locale_rows = []
    blocks = {}
    for locale in locales:
        path, rules = resolve_locale_rules(locale, collation_dir)
        if not rules:
            locale_rows.append((locale, len(entry_rows), 0))
            continue
        block = blocks.get(path)
        if block is None:
            tailored = compile_rules(rules, index, entries)
            offset = len(entry_rows)
            for codepoints in sorted(tailored):
                sequence = tailored[codepoints]
                padded = codepoints + (0,) * (MAX_TAILORED_LENGTH - len(codepoints))
                entry_rows.append((padded, pack_reference(len(element_pool), len(sequence))))
                element_pool.extend(sequence)
            block = (offset, len(entry_rows) - offset)
            blocks[path] = block
        locale_rows.append((locale, block[0], block[1]))
    return element_pool, entry_rows, locale_rows


# ---------------------------------------------------------------------------
# Packing (root table)
# ---------------------------------------------------------------------------


def pack_element(primary: int, secondary: int, tertiary: int) -> int:
    """Packs one collation element into a single 32-bit word."""
    if not 0 <= primary <= 0xFFFF:
        raise CollationDataError(f"primary weight 0x{primary:X} does not fit in 16 bits")
    if not 0 <= secondary <= SECONDARY_MASK:
        raise CollationDataError(f"secondary weight 0x{secondary:X} does not fit in {SECONDARY_BITS} bits")
    if not 0 <= tertiary <= TERTIARY_MASK:
        raise CollationDataError(f"tertiary weight 0x{tertiary:X} does not fit in {TERTIARY_BITS} bits")
    return (primary << PRIMARY_SHIFT) | (secondary << SECONDARY_SHIFT) | tertiary


def pack_reference(offset: int, count: int) -> int:
    """Packs a pool offset and element count into one 32-bit reference."""
    if not 1 <= count <= COUNT_MASK:
        raise CollationDataError(f"collation element count {count} does not fit in {COUNT_BITS} bits")
    reference = (offset << COUNT_BITS) | count
    if reference > 0xFFFFFFFF:
        raise CollationDataError("collation element reference does not fit in 32 bits")
    return reference


def build_body(entries):
    """Builds the deduplicated pool and both indexes."""
    pool = []
    offsets = {}
    scalar_codepoints = []
    scalar_references = []
    contractions = []

    def intern(elements) -> int:
        packed = tuple(pack_element(*element) for element in elements)
        offset = offsets.get(packed)
        if offset is None:
            offset = len(pool)
            offsets[packed] = offset
            pool.extend(packed)
        return pack_reference(offset, len(packed))

    for codepoints in sorted(entries):
        elements = entries[codepoints]
        reference = intern(elements)
        if len(codepoints) == 1:
            scalar_codepoints.append(codepoints[0])
            scalar_references.append(reference)
        else:
            padded = codepoints + (0,) * (MAX_CONTRACTION_LENGTH - len(codepoints))
            contractions.append((padded, reference))

    contractions.sort(key=lambda item: item[0])
    return pool, scalar_codepoints, scalar_references, contractions, len(offsets)


# ---------------------------------------------------------------------------
# Unicode property data needed for implicit weights
# ---------------------------------------------------------------------------


def unified_ideograph_ranges():
    """Returns the Unified_Ideograph ranges of the running Unicode version.

    Python does not expose the Unified_Ideograph property directly, but every
    code point with that property has a name starting with "CJK UNIFIED
    IDEOGRAPH-" or "CJK COMPATIBILITY IDEOGRAPH-".  That yields exactly the
    twelve contiguous ranges of Unicode 15.0.0.
    """
    ranges = []
    start = None
    previous = None
    for codepoint in range(MAX_CODE_POINTS):
        name = unicodedata.name(chr(codepoint), "")
        unified = name.startswith("CJK UNIFIED IDEOGRAPH-") or name.startswith(
            "CJK COMPATIBILITY IDEOGRAPH-"
        )
        if not unified:
            if start is not None:
                ranges.append((start, previous))
                start = None
            continue
        if start is None:
            start = codepoint
        elif codepoint != previous + 1:
            ranges.append((start, previous))
            start = codepoint
        previous = codepoint
    if start is not None:
        ranges.append((start, previous))
    return ranges


def is_core_han(codepoint: int) -> bool:
    return any(first <= codepoint <= last for first, last in CORE_HAN_BLOCKS)


# ---------------------------------------------------------------------------
# Emission
# ---------------------------------------------------------------------------


def format_int(value: int) -> str:
    return f"0x{value:X}"


def format_values(values, per_line: int, indent: str = "        ") -> str:
    lines = []
    for index in range(0, len(values), per_line):
        chunk = values[index:index + per_line]
        lines.append(indent + ", ".join(chunk))
    return ",\n".join(lines)


def render_tailoring(element_pool, entry_rows, locale_rows):
    """Renders the three CLDR tailoring arrays."""
    if element_pool:
        lines = ",\n".join(
            "        {" + ", ".join(format_int(value) for value in element) + "}"
            for element in element_pool
        )
        elements = f"inline constexpr TailoredElement kTailoredElements[] = {{\n{lines}\n}};\n"
    else:
        elements = "inline constexpr TailoredElement kTailoredElements[1] = {{0, 0, 0}};\n"

    if entry_rows:
        lines = ",\n".join(
            "        {" + ", ".join(format_int(value) for value in key) + f", {format_int(reference)}}}"
            for key, reference in entry_rows
        )
        entries = f"inline constexpr TailoredEntry kTailoredEntries[] = {{\n{lines}\n}};\n"
    else:
        entries = "inline constexpr TailoredEntry kTailoredEntries[1] = {{0, 0, 0, 0}};\n"

    locale_lines = ",\n".join(
        f'        {{"{tag}", {offset}, {count}}}' for tag, offset, count in locale_rows
    )
    locales = f"inline constexpr TailoredLocale kTailoredLocales[] = {{\n{locale_lines}\n}};\n"
    return elements, entries, locales


def render(
    version,
    implicit_ranges,
    entries,
    variable_count,
    pool,
    scalar_codepoints,
    scalar_references,
    contractions,
    distinct_sequences,
    unified_ranges,
    tailored_elements,
    tailored_entries,
    tailored_locales,
    tailored_locale_count,
    tailored_element_count,
    tailored_entry_count,
):
    scalar_count = len(scalar_codepoints)
    pair_count = sum(1 for padded, _ in contractions if padded[2] == 0)
    triple_count = len(contractions) - pair_count
    total_elements = sum(len(elements) for elements in entries.values())
    max_elements = max(len(elements) for elements in entries.values())
    max_primary = max(primary for elements in entries.values() for primary, _, _ in elements)
    max_secondary = max(secondary for elements in entries.values() for _, secondary, _ in elements)
    max_tertiary = max(tertiary for elements in entries.values() for _, _, tertiary in elements)
    max_codepoint = max(codepoints[0] for codepoints in entries if len(codepoints) == 1)

    if unicodedata.unidata_version.split(".") != version.split("."):
        print(
            f"warning: the generated header records UCA {version}, but this Python reads "
            f"Unicode {unicodedata.unidata_version} data; the Unified_Ideograph ranges may differ",
            file=sys.stderr,
        )
    for _padded, element_ref in contractions:
        offset = element_ref >> COUNT_BITS
        count = element_ref & COUNT_MASK
        if offset + count > len(pool):
            raise CollationDataError("internal error: contraction points outside the pool")
    if scalar_count + len(contractions) != len(entries):
        raise CollationDataError("internal error: entry count mismatch")
    if max_elements > COUNT_MASK:
        raise CollationDataError("internal error: element count does not fit the reference")

    pool_lines = format_values([format_int(value) for value in pool], 10)
    scalar_codepoint_lines = format_values([format_int(value) for value in scalar_codepoints], 10)
    scalar_reference_lines = format_values([format_int(value) for value in scalar_references], 8)
    contraction_lines = ",\n".join(
        "        {" + ", ".join(format_int(value) for value in padded) + f", {format_int(reference)}}}"
        for padded, reference in contractions
    )
    implicit_lines = ",\n".join(
        "        {" + ", ".join(format_int(value) for value in (first, last, base, offset)) + "}"
        for first, last, base, offset in implicit_ranges
    )
    unified_lines = ",\n".join(
        "        {" + ", ".join(format_int(value) for value in (first, last)) + "}"
        for first, last in unified_ranges
    )

    return f"""#pragma once

// ===========================================================================
// localisation/collation_data.h
//
// GENERATED FILE - DO NOT EDIT BY HAND.
//
// Default Unicode Collation Element Table (DUCET) of the Unicode Collation
// Algorithm (UTS #10), version {version} (Unicode {unicodedata.unidata_version}),
// plus the CLDR tailorings compiled by tools/gen_collation_data.py.
//
//   Source:    allkeys-{version}.txt
//              https://www.unicode.org/Public/UCA/{version}/allkeys.txt
//              common/collation/<locale>.xml of CLDR
//   Generator: tools/gen_collation_data.py <allkeys.txt> <output>
//
// Contents
// --------
//   * {len(entries)} DUCET entries: {scalar_count} single code points and
//     {len(contractions)} contractions ({pair_count} pairs, {triple_count} triples).
//   * {distinct_sequences} distinct collation-element sequences in the pool
//     ({total_elements} elements in total, at most {max_elements} per entry).
//   * {variable_count} entries are marked variable ("*") in allkeys.txt.  This
//     library implements the default, non-ignorable UCA variant, in which
//     variable elements keep their primary weight, so the marker is not stored.
//   * {tailored_locale_count} CLDR locales, {tailored_element_count} tailored
//     collation elements and {tailored_entry_count} tailored contraction keys
//     (see "Tailorings" below).
//   * Primary weights 0x0000..0x{max_primary:04X}, secondary weights
//     0x0000..0x{max_secondary:04X}, tertiary weights 0x0000..0x{max_tertiary:04X}.
//     The largest explicit code point is U+{max_codepoint:04X}.
//
// Representation
// --------------
// A root collation element is packed into one 32-bit word:
//
//   bits 29..14  primary weight   (0x0000..0xFFFF)
//   bits 13..5   secondary weight (0x0000..0x01FF)
//   bits  4..0   tertiary weight  (0x0000..0x001F)
//
// An element reference packs a pool offset and an element count:
//
//   bits 31..5   offset into kCollationElements
//   bits  4..0   number of elements (1..{max_elements})
//
// kScalarCodepoints is sorted ascending, so a single-code-point mapping is a
// binary search followed by a pool lookup.  kContractions is sorted by
// (first, second, third); two-code-point entries store 0 as their third key,
// which keeps three- and two-code-point lookups independent.
//
// Code points without an explicit entry use the UTS #10 Table 16 implicit
// weights.  kImplicitRanges holds the @implicitweights lines of allkeys.txt
// (Tangut, Nushu, Khitan Small Script) and kUnifiedIdeographRanges holds the
// Unified_Ideograph ranges of Unicode {unicodedata.unidata_version}; core Han
// ideographs (CJK Unified Ideographs and CJK Compatibility Ideographs blocks)
// use lead unit 0x{CORE_HAN_BASE:04X}, other unified ideographs 0x{OTHER_HAN_BASE:04X}, and
// unassigned code points 0x{UNASSIGNED_BASE:04X}.
//
// Tailorings
// ----------
// The CLDR "standard" collation of a locale is compiled into synthetic
// collation elements.  A collator whose locale has at least one rule compares
// in an expanded weight space, so that synthetic weights fit between two root
// weights:
//
//   primary   = rootPrimary << {TAILORING_PRIMARY_SHIFT}
//   secondary = rootSecondary << {TAILORING_SECONDARY_SHIFT}
//   tertiary  = rootTertiary << {TAILORING_TERTIARY_SHIFT}
//
// A primary relation (CLDR "<") inserts a weight above its anchor: the first
// one after root primary P is (P << {TAILORING_PRIMARY_SHIFT}) + 1, the next
// (P << {TAILORING_PRIMARY_SHIFT}) + 2, and so on.  "[before 1]" reserves the
// top of the gap below the anchor, ((P - 1) << {TAILORING_PRIMARY_SHIFT}) +
// 0x{BEFORE_PRIMARY_GAP:02X}FF, so a chain that continues with "<" still ends up
// below the anchor.  Secondary and tertiary relations use the same idea one
// level down, with steps of {SECONDARY_STEP} and 0x{TERTIARY_STEP:02X} in the expanded space.
// A tailored element already stores the expanded weights; root elements are
// shifted when a tailored collator builds them, and locales without rules keep
// the compact root weights, so their sort keys are exactly the DUCET weights.
//
// kTailoredEntries is grouped by locale (kTailoredLocale.entryOffset /
// .entryCount) and sorted inside each group by (first, second, third, fourth).
// Unused key positions hold 0, so a single code point has second == third ==
// fourth == 0 and a pair has third == fourth == 0.  An entry count of 0 means
// "this locale is identical to the root order" (de, fr and ru: their CLDR
// standard collation adds no rules).
// nb, nn and no share one block, because nb.xml and nn.xml inherit no.xml.
// The compiled locales are cs da de es et fi fr hu lt lv nb nn no pl ro ru sk
// tr.  sv is not emitted: it keeps the native Swedish tailoring of
// collation.h.  de, fr and ru resolve to an empty rule set, because their
// CLDR "standard" collation is the root order.  ja, zh and ko are
// not built in at all (33 KB, 1.1 MB and 50 KB of script-specific rules), so
// they fall back to the root order.
//
// Regenerate with:
//   python tools/gen_collation_data.py allkeys.txt localisation/collation_data.h
// ===========================================================================

#include <cstddef>
#include <cstdint>

namespace localisation
{{
namespace detail
{{
namespace collation_data
{{

inline constexpr const char *kVersion = "{version}";

struct Contraction
{{
    std::uint32_t first;
    std::uint32_t second;
    std::uint32_t third;
    std::uint32_t elementRef;
}};

struct ImplicitRange
{{
    std::uint32_t first;
    std::uint32_t last;
    std::uint32_t base;
    std::uint32_t offset;
}};

struct CodepointRange
{{
    std::uint32_t first;
    std::uint32_t last;
}};

struct TailoredElement
{{
    std::uint32_t primary;
    std::uint16_t secondary;
    std::uint16_t tertiary;
}};

struct TailoredEntry
{{
    std::uint32_t first;
    std::uint32_t second;
    std::uint32_t third;
    std::uint32_t fourth;
    std::uint32_t elementRef;
}};

struct TailoredLocale
{{
    const char *tag;
    std::uint32_t entryOffset;
    std::uint32_t entryCount;
}};

// Pool of packed collation elements.  Sequences are deduplicated: several
// entries (most notably the {sum(1 for elements in entries.values() if elements == ((0, 0, 0),))} completely
// ignorable code points) share the same pool range.
inline constexpr std::uint32_t kCollationElements[] = {{
{pool_lines}
}};

// Single-code-point mappings, sorted by code point.
inline constexpr std::uint32_t kScalarCodepoints[] = {{
{scalar_codepoint_lines}
}};

// Pool reference for the corresponding entry in kScalarCodepoints.
inline constexpr std::uint32_t kScalarElementRefs[] = {{
{scalar_reference_lines}
}};

// Multi-code-point mappings, sorted by (first, second, third).
inline constexpr Contraction kContractions[] = {{
{contraction_lines}
}};

// @implicitweights ranges: [first, last] maps to a constant lead unit and a
// per-code-point second unit of (codePoint - offset) | 0x8000.
inline constexpr ImplicitRange kImplicitRanges[] = {{
{implicit_lines}
}};

// Unified_Ideograph ranges of Unicode {unicodedata.unidata_version}.
inline constexpr CodepointRange kUnifiedIdeographRanges[] = {{
{unified_lines}
}};

// ---------------------------------------------------------------------------
// CLDR tailorings
// ---------------------------------------------------------------------------

// Expanded (primary, secondary, tertiary) weights of the tailored elements.
// The references below index this array.
{tailored_elements}
// Tailored contraction keys, grouped by locale and sorted inside each group.
{tailored_entries}
// Compiled locales, sorted by language subtag.
{tailored_locales}
inline constexpr std::size_t kScalarCount = sizeof(kScalarCodepoints) / sizeof(kScalarCodepoints[0]);
inline constexpr std::size_t kContractionCount = sizeof(kContractions) / sizeof(kContractions[0]);
inline constexpr std::size_t kImplicitRangeCount = sizeof(kImplicitRanges) / sizeof(kImplicitRanges[0]);
inline constexpr std::size_t kUnifiedIdeographRangeCount =
    sizeof(kUnifiedIdeographRanges) / sizeof(kUnifiedIdeographRanges[0]);
inline constexpr std::size_t kCollationElementCount =
    sizeof(kCollationElements) / sizeof(kCollationElements[0]);
inline constexpr std::size_t kTailoredElementCount =
    sizeof(kTailoredElements) / sizeof(kTailoredElements[0]);
inline constexpr std::size_t kTailoredEntryCount = sizeof(kTailoredEntries) / sizeof(kTailoredEntries[0]);
inline constexpr std::size_t kTailoredLocaleCount = sizeof(kTailoredLocales) / sizeof(kTailoredLocales[0]);

}} // namespace collation_data
}} // namespace detail
}} // namespace localisation
"""


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------


def default_allkeys_path() -> Path:
    override = os.environ.get("UCA_ALLKEYS")
    if override:
        return Path(override)
    return Path(tempfile.gettempdir()) / "i18n-data" / "uca" / "allkeys.txt"


def default_collation_dir() -> Path:
    override = os.environ.get("CLDR_COLLATION")
    if override:
        return Path(override)
    return Path(tempfile.gettempdir()) / "i18n-data" / "cldr-collation"


def default_output_path() -> Path:
    return Path(__file__).resolve().parent.parent / "localisation" / "collation_data.h"


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description="Generate localisation/collation_data.h from allkeys.txt")
    parser.add_argument("input", nargs="?", type=Path, default=None, help="path to allkeys.txt")
    parser.add_argument("output", nargs="?", type=Path, default=None, help="path to collation_data.h")
    parser.add_argument(
        "--check",
        action="store_true",
        help="do not write; fail when the output file is missing or out of date",
    )
    parser.add_argument(
        "--collation-dir",
        type=Path,
        default=None,
        help="directory holding the CLDR <locale>.xml collation files",
    )
    parser.add_argument(
        "--no-tailoring",
        action="store_true",
        help="emit an empty CLDR tailoring section (root order only)",
    )
    arguments = parser.parse_args(argv)

    source_path = arguments.input if arguments.input is not None else default_allkeys_path()
    output_path = arguments.output if arguments.output is not None else default_output_path()
    if not source_path.is_file():
        print(f"error: allkeys file not found: {source_path}", file=sys.stderr)
        print(
            "download it from https://www.unicode.org/Public/UCA/15.0.0/allkeys.txt "
            "or pass the path as the first argument",
            file=sys.stderr,
        )
        return 1

    version, implicit_ranges, entries, variable_count = parse_allkeys(source_path)
    pool, scalar_codepoints, scalar_references, contractions, distinct_sequences = build_body(entries)
    unified_ranges = unified_ideograph_ranges()

    if arguments.no_tailoring:
        tailored_elements, tailored_entry_rows, tailored_locale_rows = [], [], []
    else:
        collation_dir = arguments.collation_dir if arguments.collation_dir is not None else default_collation_dir()
        if not collation_dir.is_dir():
            print(f"error: CLDR collation directory not found: {collation_dir}", file=sys.stderr)
            print(
                "download common/collation/<locale>.xml from https://github.com/unicode-org/cldr, "
                "or pass --collation-dir, or use --no-tailoring",
                file=sys.stderr,
            )
            return 1
        tailored_elements, tailored_entry_rows, tailored_locale_rows = build_tailorings(
            entries, collation_dir, CLDR_LOCALES
        )

    # Sanity check for the implicit-weight tables: no explicit entry may fall
    # inside an implicit range, and every core-Han range must be covered by
    # the Unified_Ideograph table.
    for first, last, base, offset in implicit_ranges:
        for codepoints in entries:
            if len(codepoints) == 1 and first <= codepoints[0] <= last:
                raise CollationDataError(
                    f"explicit entry U+{codepoints[0]:04X} overlaps @implicitweights range "
                    f"0x{first:04X}..0x{last:04X}"
                )
        if base == 0xFB00 and offset != 0x17000:
            raise CollationDataError("Tangut implicit range must use offset 0x17000")
    core_han_ranges = [item for item in unified_ranges if is_core_han(item[0])]
    if not core_han_ranges:
        raise CollationDataError("Unified_Ideograph table has no core Han ranges")

    tailored_elements_text, tailored_entries_text, tailored_locales_text = render_tailoring(
        tailored_elements, tailored_entry_rows, tailored_locale_rows
    )
    text = render(
        version,
        implicit_ranges,
        entries,
        variable_count,
        pool,
        scalar_codepoints,
        scalar_references,
        contractions,
        distinct_sequences,
        unified_ranges,
        tailored_elements_text,
        tailored_entries_text,
        tailored_locales_text,
        len(tailored_locale_rows),
        len(tailored_elements),
        len(tailored_entry_rows),
    )
    encoded = text.encode("utf-8")

    if arguments.check:
        if not output_path.is_file():
            print(f"error: {output_path} does not exist", file=sys.stderr)
            return 1
        if output_path.read_bytes() != encoded:
            print(f"error: {output_path} is out of date; regenerate it", file=sys.stderr)
            return 1
        print(f"{output_path} is up to date ({len(encoded)} bytes)")
        return 0

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(encoded)
    print(
        f"wrote {output_path} ({len(encoded)} bytes): "
        f"{len(entries)} entries, {len(pool)} pooled elements, "
        f"{len(contractions)} contractions, {len(implicit_ranges)} implicit ranges, "
        f"{len(unified_ranges)} unified-ideograph ranges, "
        f"{len(tailored_locale_rows)} locales with {len(tailored_entry_rows)} tailored keys"
    )
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except CollationDataError as error:
        print(f"error: {error}", file=sys.stderr)
        sys.exit(1)
