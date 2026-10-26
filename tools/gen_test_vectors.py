"""Generates tests/normalisation_vectors.h from Python's unicodedata.

The vectors exercise NFC/NFD/NFKC/NFKD on interesting and random strings.  The
FNV-1a hashes cover every code point that has a decomposition, a combining
class, a case mapping, a Hangul syllable, or is sampled by cp % 97 == 0, so the
C++ tests can verify the generated tables without embedding megabytes of
expected output.
"""

from __future__ import annotations

import random
import sys
import unicodedata
from pathlib import Path

MAX_CP = 0x110000
SURROGATE_START = 0xD800
SURROGATE_END = 0xDFFF
FNV_OFFSET = 14695981039346656037
FNV_PRIME = 1099511628211


def is_surrogate(cp: int) -> bool:
    return SURROGATE_START <= cp <= SURROGATE_END


def fnv_update(hash_value: int, data: bytes) -> int:
    for byte in data:
        hash_value = ((hash_value ^ byte) * FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    return hash_value


def selected(cp: int) -> bool:
    if is_surrogate(cp):
        return False
    character = chr(cp)
    return (
        bool(unicodedata.decomposition(character))
        or 0xAC00 <= cp <= 0xD7A3
        or unicodedata.combining(character) != 0
        or character.lower() != character
        or character.upper() != character
        or character.title() != character
        or cp % 97 == 0
    )


def cpp_literal(text: str) -> str:
    tokens = []
    for byte in text.encode("utf-8"):
        if byte == 0x0A:
            tokens.append("\\n")
        elif byte == 0x0D:
            tokens.append("\\r")
        elif byte == 0x09:
            tokens.append("\\t")
        elif byte == 0x22:
            tokens.append('\\"')
        elif byte == 0x5C:
            tokens.append("\\\\")
        elif 0x20 <= byte < 0x7F:
            tokens.append(chr(byte))
        else:
            tokens.append(f"\\x{byte:02X}")

    parts = []
    for token in tokens:
        if parts and parts[-1].startswith("\\x") and len(token) == 1 and token in "0123456789abcdefABCDEF":
            parts.append('""')
        parts.append(token)
    return '"' + "".join(parts) + '"'


def build_vectors():
    fixed = [
        "",
        "plain ascii",
        "Cafe\u0301",
        "\u00e9",
        "e\u0301",
        "\u212b",
        "A\u030a",
        "\u00c5",
        "\uac00",
        "\uac01",
        "\u1100\u1161",
        "\u1100\u1161\u11a8",
        "\ufb01",
        "\ufb03",
        "\uff21\uff22\uff23",
        "\u2461",
        "\u216b",
        "\u2460\u2461\u2462",
        "\ufdfa",
        "\u0915\u093c",
        "\u0958",
        "\u0f40\u0fb5",
        "\u03a3\u039f\u03a6\u039f\u03a3",
        "\u03a3",
        "\u0130stanbul",
        "Stra\u00dfe",
        "A\u030a\u0301",
        "a\u0301\u0323",
        "a\u0323\u0301",
        "a\u0301\u0323\u0308",
        "\u1e0b\u0323",
        "\u0041\u0301\u0323\u0302",
        "\U0001f1e8\U0001f1f3",
        "\U0001f469\u200d\U0001f4bb",
        "\U0001f3f3\ufe0f\u200d\U0001f308",
        "\u4e2d\u6587",
        "\u0915\u094d\u0937",
        "\u0f40\u0f72",
        "\u0e01\u0e49",
        "\u05d0\u05b7",
        "\u0627\u064e",
        "\u1f88",
        "\u1fbc",
        "\u0345",
        "\u0390",
        "\u03d3",
        "\U00010400\U00010428",
        "\U0001d15e",
        "\u00bd",
        "\u2474",
        "\u33a1",
        "\uf900",
        "\ufa0e",
        "\u2f00",
        "\u3000",
        "\u00a0",
    ]

    pools = [
        list(range(0x20, 0x7F)),
        list(range(0xA0, 0x180)),
        list(range(0x180, 0x250)),
        list(range(0x300, 0x370)),
        list(range(0x370, 0x400)),
        list(range(0x400, 0x530)),
        list(range(0x590, 0x600)),
        list(range(0x600, 0x700)),
        list(range(0x900, 0x980)),
        list(range(0x980, 0xA00)),
        list(range(0xF00, 0x1000)),
        list(range(0x1100, 0x1200)),
        list(range(0xAC00, 0xAC80)),
        list(range(0x3040, 0x3100)),
        list(range(0x4E00, 0x4E80)),
        list(range(0xFB00, 0xFB50)),
        list(range(0xFE70, 0xFF00)),
        list(range(0xFF00, 0xFFF0)),
        list(range(0x1F300, 0x1F650)),
    ]

    generator = random.Random(20261026)
    strings = list(fixed)
    for _ in range(420):
        length = generator.randint(1, 9)
        characters = []
        for _ in range(length):
            pool = generator.choice(pools)
            cp = generator.choice(pool)
            while is_surrogate(cp):
                cp = generator.choice(pool)
            characters.append(chr(cp))
        strings.append("".join(characters))

    vectors = []
    seen = set()
    for text in strings:
        if text in seen:
            continue
        seen.add(text)
        vectors.append(
            (
                text,
                unicodedata.normalize("NFC", text),
                unicodedata.normalize("NFD", text),
                unicodedata.normalize("NFKC", text),
                unicodedata.normalize("NFKD", text),
            )
        )
    return vectors


def build_hashes():
    forms = ("NFC", "NFD", "NFKC", "NFKD")
    hashes = [FNV_OFFSET] * 4
    for cp in range(MAX_CP):
        if not selected(cp):
            continue
        character = chr(cp)
        for index, form in enumerate(forms):
            hashes[index] = fnv_update(hashes[index], unicodedata.normalize(form, character).encode("utf-8"))

    case_hashes = [FNV_OFFSET] * 3
    for cp in range(MAX_CP):
        if not selected(cp):
            continue
        character = chr(cp)
        case_hashes[0] = fnv_update(case_hashes[0], character.lower().encode("utf-8"))
        case_hashes[1] = fnv_update(case_hashes[1], character.upper().encode("utf-8"))
        case_hashes[2] = fnv_update(case_hashes[2], character.title().encode("utf-8"))
    return hashes, case_hashes


def main() -> int:
    vectors = build_vectors()
    hashes, case_hashes = build_hashes()

    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("// Generated by tools/gen_test_vectors.py from Unicode "
                 f"{unicodedata.unidata_version} data. Do not edit by hand.")
    lines.append("")
    lines.append("#include <cstddef>")
    lines.append("#include <cstdint>")
    lines.append("")
    lines.append("struct NormalisationVector")
    lines.append("{")
    lines.append("    const char *input;")
    lines.append("    const char *nfc;")
    lines.append("    const char *nfd;")
    lines.append("    const char *nfkc;")
    lines.append("    const char *nfkd;")
    lines.append("};")
    lines.append("")
    lines.append("inline constexpr NormalisationVector kNormalisationVectors[] = {")
    for text, nfc_value, nfd_value, nfkc_value, nfkd_value in vectors:
        lines.append(
            "    {" + cpp_literal(text) + ", " + cpp_literal(nfc_value) + ", " + cpp_literal(nfd_value)
            + ", " + cpp_literal(nfkc_value) + ", " + cpp_literal(nfkd_value) + "},"
        )
    lines.append("};")
    lines.append("")
    lines.append("inline constexpr std::size_t kNormalisationVectorCount =")
    lines.append("    sizeof(kNormalisationVectors) / sizeof(kNormalisationVectors[0]);")
    lines.append("")
    lines.append("// FNV-1a hashes over the normalised form of every selected code point, in")
    lines.append("// ascending code point order.")
    lines.append("inline constexpr std::uint64_t kNormalisationHashes[] = {")
    lines.append("    " + ", ".join(f"0x{value:016X}ULL" for value in hashes) + ",")
    lines.append("};")
    lines.append("")
    lines.append("inline constexpr std::uint64_t kCaseHashes[] = {")
    lines.append("    " + ", ".join(f"0x{value:016X}ULL" for value in case_hashes) + ",")
    lines.append("};")
    lines.append("")

    target = Path(__file__).resolve().parent.parent / "tests" / "normalisation_vectors.h"
    target.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    print(f"wrote {target} ({target.stat().st_size} bytes)")
    print(f"vectors={len(vectors)}")
    print("normalisation hashes:", [f"0x{value:016X}" for value in hashes])
    print("case hashes:", [f"0x{value:016X}" for value in case_hashes])
    return 0


if __name__ == "__main__":
    sys.exit(main())
