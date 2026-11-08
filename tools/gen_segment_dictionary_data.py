#!/usr/bin/env python3
"""Generate bunch/segment_dictionary_data.h from the ICU break dictionaries.

The Thai, Lao, Khmer and Myanmar break iterators in ICU are driven by word
dictionaries that ship with the ICU data (icu4c/source/data/brkitr/dictionaries
in the ICU repository, derived from the Unicode 15.0 line-breaking data).  They
are plain text files with one word per line and '#' comments, so they can be
downloaded and fed to bunch::WordDictionary::load at run time.

This script builds the *sample* dictionaries that bunch/segment_dictionary.h
compiles in.  The sample is deliberately small (see the "limit" entries in
LANGUAGES) so the header stays within the repository's source-size budget; it is
enough to demonstrate and test dictionary segmentation, not to replace the full
data.

Usage:
    python tools/gen_segment_dictionary_data.py --icu-dir DIR
    python tools/gen_segment_dictionary_data.py            # uses ICU_DICT_DIR
                                                           # or the temp dir
    python tools/gen_segment_dictionary_data.py --limit th=1200 --limit km=900

A larger --limit makes the sample more useful and the header larger; the
default limits keep bunch/segment_dictionary_data.h just under 200 KB.  The
seed words are always kept, so a limit below the seed count has no effect.

The generated output is deterministic: the same dictionaries always produce the
same bytes, and the generated header is plain ASCII (every UTF-8 byte is written
as a \\xNN escape, which keeps the header independent of the compiler's source
and execution character sets).

Selection is not frequency based: ICU ships no frequencies, so each sample is a
curated basic-vocabulary seed list plus an evenly spaced sample of short words
(2-8 code points) drawn from the whole, lexicographically sorted dictionary.
That spreads the sample across every initial letter instead of taking the first
N entries of one block.
"""

import argparse
import os
import sys
import tempfile
import unicodedata
from pathlib import Path

REPOSITORY_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_OUTPUT = REPOSITORY_ROOT / "bunch" / "segment_dictionary_data.h"

# Script ranges, mirroring detail::dictionary::kScriptRanges in
# bunch/segment_dictionary.h.  Only letters and combining marks are kept, so
# punctuation, digits and symbols never enter a dictionary word.
LANGUAGES = [
    {
        "tag": "th",
        "name": "Thai",
        "file": "thaidict.txt",
        "table": "kThaiWords",
        "ranges": [(0x0E00, 0x0E7F)],
        "limit": 710,
        "seeds": [
            "ภาษา", "ไทย", "ประเทศ", "คน", "ดี", "มาก", "หนึ่ง", "สอง",
            "รถ", "บ้าน", "น้ำ", "ใจ", "ทำ", "งาน", "เรียน", "หนังสือ",
            "แมว", "หมา", "กิน", "ข้าว", "กรุงเทพ", "โรงเรียน", "เพื่อน",
            "ครอบครัว", "เวลา", "วัน", "เดือน", "ปี", "ใหม่", "ใหญ่",
        ],
    },
    {
        "tag": "lo",
        "name": "Lao",
        "file": "laodict.txt",
        "table": "kLaoWords",
        "ranges": [(0x0E80, 0x0EFF)],
        "limit": 670,
        "seeds": [
            "ພາສາ", "ລາວ", "ຄົນ", "ດີ", "ຫຼາຍ", "ໜຶ່ງ", "ສອງ", "ເຮືອນ",
            "ນ້ຳ", "ໃຈ", "ເຮັດ", "ວຽກ", "ຮຽນ", "ປຶ້ມ", "ແມວ", "ໝາ",
            "ກິນ", "ເຂົ້າ", "ປະເທດ", "ໂຮງຮຽນ", "ເພື່ອນ", "ຄອບຄົວ",
            "ເວລາ", "ມື້", "ເດືອນ", "ປີ", "ໃໝ່", "ໃຫຍ່",
        ],
    },
    {
        "tag": "km",
        "name": "Khmer",
        "file": "khmerdict.txt",
        "table": "kKhmerWords",
        "ranges": [(0x1780, 0x17FF), (0x19E0, 0x19FF)],
        "limit": 620,
        "seeds": [
            "ភាសា", "ខ្មែរ", "ប្រទេស", "មនុស្ស", "ល្អ", "ខ្លាំង", "មួយ", "ពីរ",
            "ផ្ទះ", "ទឹក", "ចិត្ត", "ធ្វើ", "ការងារ", "រៀន", "សៀវភៅ", "ឆ្មា",
            "ឆ្កែ", "ញ៉ាំ", "បាយ", "សាលា", "មិត្ត", "គ្រួសារ", "ពេលវេលា",
            "ថ្ងៃ", "ខែ", "ឆ្នាំ", "ថ្មី", "ធំ",
        ],
    },
    {
        "tag": "my",
        "name": "Myanmar",
        "file": "burmesedict.txt",
        "table": "kBurmeseWords",
        "ranges": [(0x1000, 0x109F), (0xA9E0, 0xA9FF), (0xAA60, 0xAA7F)],
        "limit": 570,
        "seeds": [
            "မြန်မာ", "ဘာသာ", "နိုင်ငံ", "လူ", "ကောင်း", "များ", "တစ်", "နှစ်",
            "အိမ်", "ရေ", "စိတ်", "လုပ်", "အလုပ်", "ကျောင်း", "စာအုပ်", "ကြောင်",
            "ခွေး", "စား", "ထမင်း", "မိသားစု", "ကျွန်တော်", "အချိန်", "နေ့",
            "လ", "အသစ်", "ကြီး",
        ],
    },
]

# Only short words are sampled: the sample exists to demonstrate maximal
# matching, and long compounds cost source bytes without adding coverage.
MAX_SAMPLE_CODE_POINTS = 8
MIN_SAMPLE_CODE_POINTS = 2
WORDS_PER_LINE = 4

SOURCE_NOTE = """\
// Sample word lists generated from the ICU break iterator dictionaries
// (icu4c/source/data/brkitr/dictionaries): thaidict.txt, laodict.txt,
// khmerdict.txt and burmesedict.txt, which carry the Unicode 15.0.0
// line-breaking data.  Copyright (C) 2016 and later Unicode, Inc. and others;
// distributed under the Unicode License (https://www.unicode.org/copyright.html).
//
// Each list is a deterministic sample: a curated basic-vocabulary seed list
// plus evenly spaced short entries (2-8 code points) from the whole dictionary.
// ICU ships no frequency data, so this is not a frequency list, and it is not
// the full dictionary.  Load a complete dictionary at run time with
//     bunch::WordDictionary dictionary;
//     std::ifstream stream("thaidict.txt");
//     dictionary.load(stream);
// See bunch/segment_dictionary.h for the segmentation API.\
"""


def script_codepoint(codepoint, ranges):
    """True when the code point is in one of the script ranges."""
    return any(first <= codepoint <= last for first, last in ranges)


def is_word(word, ranges):
    """A usable entry: letters/marks only, all inside the script ranges."""
    if len(word) < 2:
        return False
    for character in word:
        codepoint = ord(character)
        if not script_codepoint(codepoint, ranges):
            return False
        if unicodedata.category(character)[0] not in "LM":
            return False
    return True


def read_dictionary(path):
    """Returns the dictionary entries, in file order, with comments removed."""
    if not path.is_file():
        raise SystemExit(
            "missing ICU dictionary: %s\n"
            "Download the break dictionaries (thaidict.txt, laodict.txt,\n"
            "khmerdict.txt, burmesedict.txt) from the ICU repository, or point\n"
            "--icu-dir / ICU_DICT_DIR at an existing copy." % path
        )
    entries = []
    for line in path.read_text(encoding="utf-8-sig").splitlines():
        entry = line.strip()
        if not entry or entry.startswith("#"):
            continue
        entries.append(entry)
    return entries


def select_words(entries, language):
    """Seed words first, then an evenly spaced sample over the sorted entries."""
    unique = list(dict.fromkeys(entries))
    present = set(unique)
    missing = [seed for seed in language["seeds"] if seed not in present]
    if missing:
        raise SystemExit(
            "%s: sample seeds missing from %s: %s"
            % (language["tag"], language["file"], ", ".join(missing))
        )

    selected = list(language["seeds"])
    chosen = set(selected)
    pool = sorted(
        word
        for word in present
        if word not in chosen
        and MIN_SAMPLE_CODE_POINTS <= len(word) <= MAX_SAMPLE_CODE_POINTS
        and is_word(word, language["ranges"])
    )
    wanted = max(0, language["limit"] - len(selected))
    if wanted >= len(pool):
        selected.extend(pool)
    else:
        # i * len(pool) // wanted is strictly increasing while wanted <= len(pool),
        # so this picks `wanted` distinct entries spread over the whole alphabet.
        selected.extend(pool[(index * len(pool)) // wanted] for index in range(wanted))
    return sorted(set(selected))


def cpp_literal(word):
    """One ASCII-only C++ string literal for the UTF-8 encoding of `word`."""
    return '"' + "".join("\\x%02X" % byte for byte in word.encode("utf-8")) + '"'


def render_table(table, words):
    lines = ["inline constexpr const char *%s[] = {" % table]
    for start in range(0, len(words), WORDS_PER_LINE):
        chunk = [cpp_literal(word) + "," for word in words[start : start + WORDS_PER_LINE]]
        lines.append("    " + " ".join(chunk))
    lines.append("};")
    return lines


def render_header(language_words):
    lines = [
        "#pragma once",
        "",
        "// Generated by tools/gen_segment_dictionary_data.py.  Do not edit by hand.",
        "//",
        SOURCE_NOTE,
        "",
        "#include <cstddef>",
        "",
        "namespace bunch",
        "{",
        "namespace detail",
        "{",
        "namespace dictionary",
        "{",
        "",
    ]
    for language, words in language_words:
        byte_count = sum(len(word.encode("utf-8")) for word in words)
        lines.append(
            "// %s (%s): %d words, %d UTF-8 bytes."
            % (language["name"], language["tag"], len(words), byte_count)
        )
        lines.extend(render_table(language["table"], words))
        lines.append("")
    lines.extend(["}", "} // namespace detail", "} // namespace bunch"])
    text = "\n".join(lines) + "\n"
    return text


def default_icu_dir():
    configured = os.environ.get("ICU_DICT_DIR")
    if configured:
        return Path(configured)
    return Path(tempfile.gettempdir()) / "i18n-data" / "icu"


def apply_limits(languages, specifications):
    """Applies repeated --limit TAG=COUNT options to the language table."""
    limits = {language["tag"]: language["limit"] for language in languages}
    for specification in specifications or []:
        tag, separator, count = specification.partition("=")
        if separator == "" or tag not in limits or not count.isdigit() or int(count) < 1:
            raise SystemExit("expected --limit TAG=COUNT with TAG in %s, got %r"
                             % (", ".join(sorted(limits)), specification))
        limits[tag] = int(count)
    for language in languages:
        language["limit"] = limits[language["tag"]]
    return languages


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--icu-dir", type=Path, default=default_icu_dir(),
                        help="directory holding the ICU *dict.txt dictionaries")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT,
                        help="generated header (default: bunch/segment_dictionary_data.h)")
    parser.add_argument("--limit", action="append", metavar="TAG=COUNT",
                        help="words per language, overriding the built-in limits")
    arguments = parser.parse_args(argv)
    languages = apply_limits(LANGUAGES, arguments.limit)

    language_words = []
    for language in languages:
        entries = read_dictionary(arguments.icu_dir / language["file"])
        words = select_words(entries, language)
        language_words.append((language, words))
        print("%s %-8s %5d words from %6d entries"
              % (language["tag"], language["name"], len(words), len(entries)))

    header = render_header(language_words)
    arguments.output.write_text(header, encoding="ascii", newline="\n")
    print("wrote %s (%d bytes)" % (arguments.output, len(header.encode("ascii"))))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
