// Tests for Bunch::compareNormalized() / Bunch::equalsNormalized().
//
// compare()/operator== keep comparing raw code point sequences. The explicit
// normalised API is the opt-in for canonical (NFC/NFD) and compatibility
// (NFKC/NFKD) equivalence. These tests pin both sides of that contract,
// including non-UTF-8 storage encodings and malformed UTF-8 input.

#include "bunch/sequence.h"

#include "test_framework.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace
{
using ::bunch::Bunch;
using ::bunch::NormalizationForm;

const std::string eAcuteUtf8 = "\xC3\xA9";       // U+00E9 LATIN SMALL LETTER E WITH ACUTE
const std::string eCombiningUtf8 = "e\xCC\x81";  // 'e' + U+0301 COMBINING ACUTE ACCENT

Bunch fromUtf8Bytes(std::string_view bytes)
{
    return Bunch(bytes, ::bunch::charset::utf8, ::bunch::charset::utf8);
}

Bunch fromUtf16LeBytes(std::string_view bytes)
{
    return Bunch(bytes, ::bunch::charset::utf16le, ::bunch::charset::utf16le);
}
} // namespace

TEST_CASE("compareNormalized: NFC and NFD canonical equivalence")
{
    const Bunch composed = fromUtf8Bytes(eAcuteUtf8);
    const Bunch decomposed = fromUtf8Bytes(eCombiningUtf8);

    CHECK_EQ(composed.size(), std::size_t(1));
    CHECK_EQ(decomposed.size(), std::size_t(2));

    // NFC is the default form.
    CHECK(composed.equalsNormalized(decomposed));
    CHECK(composed.equalsNormalized(decomposed, NormalizationForm::nfd));
    CHECK_EQ(composed.compareNormalized(decomposed), 0);
    CHECK_EQ(decomposed.compareNormalized(composed), 0);
    CHECK_EQ(decomposed.compareNormalized(composed, NormalizationForm::nfd), 0);

    // The raw sequence order is untouched: U+00E9 > 'e', and "e"+U+0301 is
    // shorter in raw text only after the shared 'e'.
    CHECK_EQ(composed.compare(decomposed), 1);
    CHECK_EQ(decomposed.compare(composed), -1);
    CHECK(decomposed < composed);
    CHECK_FALSE(composed < decomposed);
}

TEST_CASE("compareNormalized: NFKC and NFKD compatibility equivalence")
{
    const Bunch ligature("\xEF\xAC\x81"); // U+FB01 LATIN SMALL LIGATURE FI
    const Bunch plain("fi");
    CHECK_FALSE(ligature.equalsNormalized(plain)); // NFC does not map compatibility characters
    CHECK(ligature.equalsNormalized(plain, NormalizationForm::nfkc));
    CHECK(ligature.equalsNormalized(plain, NormalizationForm::nfkd));
    CHECK_EQ(ligature.compareNormalized(plain, NormalizationForm::nfkc), 0);

    const Bunch circledOne("\xE2\x91\xA0"); // U+2460 CIRCLED DIGIT ONE
    CHECK_FALSE(circledOne.equalsNormalized(Bunch("1")));
    CHECK(circledOne.equalsNormalized(Bunch("1"), NormalizationForm::nfkc));
    CHECK(circledOne.equalsNormalized(Bunch("1"), NormalizationForm::nfkd));
    CHECK_EQ(circledOne.compareNormalized(Bunch("1"), NormalizationForm::nfkc), 0);

    // Compatibility equivalence still respects code point ordering when the
    // normalised text differs.
    CHECK_EQ(circledOne.compareNormalized(Bunch("2"), NormalizationForm::nfkc), -1);
}

TEST_CASE("compareNormalized: combining marks reorder canonically")
{
    // U+0327 (CCC 202) must sort before U+0301 (CCC 230).
    const Bunch belowThenAbove("a\xCC\xA7\xCC\x81");
    const Bunch aboveThenBelow("a\xCC\x81\xCC\xA7");

    CHECK_FALSE(belowThenAbove == aboveThenBelow);
    CHECK_FALSE(belowThenAbove.equalsNormalized(aboveThenBelow, NormalizationForm::none));
    CHECK(aboveThenBelow.equalsNormalized(belowThenAbove));
    CHECK(aboveThenBelow.equalsNormalized(belowThenAbove, NormalizationForm::nfd));
    CHECK_EQ(belowThenAbove.compareNormalized(aboveThenBelow, NormalizationForm::nfd), 0);

    // Canonical ordering also applies to a run of leading non-starters.
    const Bunch marksA("\xCC\xA7\xCC\x81");
    const Bunch marksB("\xCC\x81\xCC\xA7");
    CHECK(marksA.equalsNormalized(marksB, NormalizationForm::nfd));
}

TEST_CASE("compareNormalized: ordering of unequal text")
{
    CHECK_EQ(Bunch("a").compareNormalized(Bunch("b")), -1);
    CHECK_EQ(Bunch("b").compareNormalized(Bunch("a")), 1);
    CHECK_EQ(Bunch("abc").compareNormalized(Bunch("abd")), -1);
    CHECK_EQ(Bunch("abc").compareNormalized(Bunch("ab")), 1);
    CHECK_EQ(Bunch("ab").compareNormalized(Bunch("abc")), -1);

    // Prefix comparison in normalised space: "a" is a prefix of "a"+U+0301 in
    // NFD, and "a" sorts before the composed U+00E1 in NFC.
    const Bunch plainA("a");
    const Bunch markedA("a\xCC\x81");
    CHECK_EQ(plainA.compareNormalized(markedA, NormalizationForm::nfd), -1);
    CHECK_EQ(markedA.compareNormalized(plainA, NormalizationForm::nfd), 1);
    CHECK_EQ(plainA.compareNormalized(Bunch("\xC3\xA1")), -1);

    // The chosen form changes the order: raw "e"+U+0301 sorts before "f",
    // while NFC folds it to U+00E9, which sorts after "f". NFD keeps 'e'.
    const Bunch eAcute("e\xCC\x81");
    CHECK_EQ(eAcute.compare(Bunch("f")), -1);
    CHECK_EQ(eAcute.compareNormalized(Bunch("f")), 1);
    CHECK_EQ(eAcute.compareNormalized(Bunch("f"), NormalizationForm::nfd), -1);
}

TEST_CASE("compareNormalized: empty, identical and case-sensitive text")
{
    const Bunch empty;
    CHECK(empty.equalsNormalized(Bunch()));
    CHECK_EQ(empty.compareNormalized(Bunch()), 0);
    CHECK_EQ(empty.compareNormalized(Bunch("a")), -1);
    CHECK_EQ(Bunch("a").compareNormalized(empty), 1);

    CHECK(Bunch("hello").equalsNormalized(Bunch("hello")));
    CHECK_EQ(Bunch("hello").compareNormalized(Bunch("hello")), 0);

    // Case folding is not part of this API.
    CHECK_FALSE(Bunch("A").equalsNormalized(Bunch("a")));
    CHECK_FALSE(Bunch("A").equalsNormalized(Bunch("a"), NormalizationForm::nfkc));
    CHECK(Bunch("A").equalsNormalized(Bunch("A")));
    CHECK_FALSE(Bunch("\xC3\x9F").equalsNormalized(Bunch("ss"), NormalizationForm::nfkc));
}

TEST_CASE("compareNormalized: utf16le storage encoding")
{
    const std::string composedLe("\xE9\x00", 2);          // U+00E9 in UTF-16LE
    const std::string decomposedLe("\x65\x00\x01\x03", 4); // 'e' + U+0301 in UTF-16LE
    const Bunch composed16 = fromUtf16LeBytes(composedLe);
    const Bunch decomposed16 = fromUtf16LeBytes(decomposedLe);
    const Bunch composed8 = fromUtf8Bytes(eAcuteUtf8);
    const Bunch decomposed8 = fromUtf8Bytes(eCombiningUtf8);

    CHECK_EQ(composed16.encoding(), ::bunch::charset::utf16le);
    CHECK_EQ(decomposed16.encoding(), ::bunch::charset::utf16le);
    CHECK_EQ(composed16.size(), std::size_t(1));
    CHECK_EQ(decomposed16.size(), std::size_t(2));

    CHECK(composed16.equalsNormalized(composed8));
    CHECK(composed16.equalsNormalized(decomposed16));
    CHECK(decomposed16.equalsNormalized(composed8));
    CHECK(composed8.equalsNormalized(decomposed16, NormalizationForm::nfd));
    CHECK_EQ(composed16.compareNormalized(decomposed16, NormalizationForm::nfd), 0);

    // Comparison must not rewrite the operand or its storage encoding.
    const std::vector<::bunch::Point> before = composed16.getPoints();
    CHECK_EQ(composed16.compareNormalized(decomposed8, NormalizationForm::nfd), 0);
    CHECK(composed16.getPoints() == before);
    CHECK_EQ(composed16.encoding(), ::bunch::charset::utf16le);

    // Storage encoding also must not mask real differences.
    CHECK_EQ(composed16.compareNormalized(fromUtf16LeBytes(std::string("\x66\x00", 2))), 1);
}

TEST_CASE("compareNormalized: invalid UTF-8 is replaced, not fatal")
{
    const Bunch invalid = fromUtf8Bytes("\xFF\xFE");
    CHECK_EQ(invalid.size(), std::size_t(2)); // two U+FFFD substitutions
    CHECK_EQ(invalid.compareNormalized(invalid), 0);
    CHECK(invalid.equalsNormalized(invalid));

    const Bunch replacements = fromUtf8Bytes("\xEF\xBF\xBD\xEF\xBF\xBD");
    CHECK(invalid.equalsNormalized(replacements));
    CHECK_EQ(invalid.compareNormalized(Bunch("a")), 1); // U+FFFD > 'a'
    CHECK_EQ(invalid.compareNormalized(fromUtf8Bytes("\xEF\xBF\xBD")), 1);

    const Bunch truncated = fromUtf8Bytes("\xE2\x82"); // truncated 3-byte sequence
    CHECK_EQ(truncated.size(), std::size_t(1));
    CHECK_EQ(truncated.compareNormalized(truncated, NormalizationForm::nfkc), 0);
    CHECK(truncated.equalsNormalized(fromUtf8Bytes("\xEF\xBF\xBD")));
}

TEST_CASE("compareNormalized: differs from compare and operator==")
{
    const Bunch composed("\xC3\xA9");
    const Bunch decomposed("e\xCC\x81");

    // Binary code point comparison is unchanged.
    CHECK_EQ(composed.compare(decomposed), 1);
    CHECK(composed != decomposed);
    CHECK_FALSE(composed == decomposed);

    // Normalised comparison is the explicit opt-in.
    CHECK_EQ(composed.compareNormalized(decomposed), 0);
    CHECK(composed.equalsNormalized(decomposed));

    // NormalizationForm::none is exactly compare(), including the raw order.
    CHECK_EQ(composed.compareNormalized(decomposed, NormalizationForm::none), composed.compare(decomposed));
    CHECK_EQ(decomposed.compareNormalized(composed, NormalizationForm::none), decomposed.compare(composed));
}

TEST_CASE("compareNormalized: consistent ordering across forms")
{
    const Bunch samples[] = {
        Bunch(""),
        Bunch("a"),
        Bunch("b"),
        Bunch("A"),
        Bunch("\xC3\xA9"),
        Bunch("e\xCC\x81"),
        Bunch("\xEF\xAC\x81"),
        Bunch("fi"),
        Bunch("\xE2\x91\xA0"),
        Bunch("1"),
    };
    const NormalizationForm forms[] = {
        NormalizationForm::none,
        NormalizationForm::nfc,
        NormalizationForm::nfd,
        NormalizationForm::nfkc,
        NormalizationForm::nfkd,
    };

    for (const NormalizationForm form : forms)
    {
        for (const Bunch &left : samples)
        {
            CHECK_EQ(left.compareNormalized(left, form), 0);
            CHECK(left.equalsNormalized(left, form));
            for (const Bunch &right : samples)
            {
                const int forward = left.compareNormalized(right, form);
                const int backward = right.compareNormalized(left, form);
                CHECK_EQ(forward, -backward);
                CHECK_EQ(left.equalsNormalized(right, form), forward == 0);
            }
        }
    }
}
