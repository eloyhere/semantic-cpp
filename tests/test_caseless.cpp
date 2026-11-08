#include "bunch/caseless.h"

#include "test_framework.h"

#include <string>
#include <string_view>
#include <vector>

namespace
{
// Byte sequences are written as escapes so this file stays pure ASCII and does
// not depend on the compiler's source encoding.
constexpr std::string_view kSharpS = "\xC3\x9F";                    // U+00DF LATIN SMALL LETTER SHARP S
constexpr std::string_view kCapitalSharpS = "\xE1\xBA\x9E";        // U+1E9E LATIN CAPITAL LETTER SHARP S
constexpr std::string_view kDottedCapitalI = "\xC4\xB0";           // U+0130 LATIN CAPITAL LETTER I WITH DOT ABOVE
constexpr std::string_view kDotlessSmallI = "\xC4\xB1";            // U+0131 LATIN SMALL LETTER DOTLESS I
constexpr std::string_view kCapitalSigma = "\xCE\xA3";             // U+03A3 GREEK CAPITAL LETTER SIGMA
constexpr std::string_view kSmallSigma = "\xCF\x83";               // U+03C3 GREEK SMALL LETTER SIGMA
constexpr std::string_view kFinalSigma = "\xCF\x82";               // U+03C2 GREEK SMALL LETTER FINAL SIGMA
constexpr std::string_view kFiLigature = "\xEF\xAC\x81";           // U+FB01 LATIN SMALL LIGATURE FI
constexpr std::string_view kFfLigature = "\xEF\xAC\x80";           // U+FB00 LATIN SMALL LIGATURE FF
constexpr std::string_view kFfiLigature = "\xEF\xAC\x83";          // U+FB03 LATIN SMALL LIGATURE FFI
constexpr std::string_view kCircledCapitalA = "\xE2\x92\xB6";      // U+24B6 CIRCLED LATIN CAPITAL LETTER A
constexpr std::string_view kFullwidthCapitalA = "\xEF\xBC\xA1";    // U+FF21 FULLWIDTH LATIN CAPITAL LETTER A
constexpr std::string_view kFullwidthCapitalI = "\xEF\xBC\xA9";    // U+FF29 FULLWIDTH LATIN CAPITAL LETTER I
constexpr std::string_view kMathBoldCapitalA = "\xF0\x9D\x90\x80"; // U+1D400 MATHEMATICAL BOLD CAPITAL A
constexpr std::string_view kRomanNumeralOne = "\xE2\x85\xA0";      // U+2160 ROMAN NUMERAL ONE
constexpr std::string_view kSquaredCapitalA = "\xF0\x9F\x84\xB0";  // U+1F130 SQUARED LATIN CAPITAL LETTER A
constexpr std::string_view kSoftHyphen = "\xC2\xAD";               // U+00AD SOFT HYPHEN
constexpr std::string_view kZeroWidthSpace = "\xE2\x80\x8B";       // U+200B ZERO WIDTH SPACE
constexpr std::string_view kZeroWidthJoiner = "\xE2\x80\x8D";      // U+200D ZERO WIDTH JOINER
constexpr std::string_view kWordJoiner = "\xE2\x81\xA0";           // U+2060 WORD JOINER
constexpr std::string_view kByteOrderMark = "\xEF\xBB\xBF";        // U+FEFF ZERO WIDTH NO-BREAK SPACE
constexpr std::string_view kVariationSelectorOne = "\xEF\xB8\x80"; // U+FE00 VARIATION SELECTOR-1
constexpr std::string_view kGraphemeJoiner = "\xCD\x8F";           // U+034F COMBINING GRAPHEME JOINER
constexpr std::string_view kCombiningAcute = "\xCC\x81";           // U+0301 COMBINING ACUTE ACCENT
constexpr std::string_view kCombiningDotAbove = "\xCC\x87";        // U+0307 COMBINING DOT ABOVE
constexpr std::string_view kCombiningRingAbove = "\xCC\x8A";       // U+030A COMBINING RING ABOVE
constexpr std::string_view kCombiningDotBelow = "\xCC\xA3";        // U+0323 COMBINING DOT BELOW
constexpr std::string_view kSmallEAcute = "\xC3\xA9";              // U+00E9 LATIN SMALL LETTER E WITH ACUTE
constexpr std::string_view kCapitalEAcute = "\xC3\x89";            // U+00C9 LATIN CAPITAL LETTER E WITH ACUTE
constexpr std::string_view kSmallARing = "\xC3\xA5";               // U+00E5 LATIN SMALL LETTER A WITH RING ABOVE
constexpr std::string_view kCapitalARing = "\xC3\x85";             // U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE
constexpr std::string_view kReplacement = "\xEF\xBF\xBD";          // U+FFFD REPLACEMENT CHARACTER

std::string joined(std::string_view first, std::string_view second)
{
    return std::string(first) + std::string(second);
}

std::string surrounded(std::string_view prefix, std::string_view middle, std::string_view suffix)
{
    return std::string(prefix) + std::string(middle) + std::string(suffix);
}
} // namespace

TEST_CASE("nfkcCasefold empty and ASCII text")
{
    using bunch::nfkcCasefold;

    CHECK_EQ(nfkcCasefold(""), std::string(""));
    CHECK_EQ(nfkcCasefold("", "tr"), std::string(""));
    CHECK_EQ(nfkcCasefold(std::string_view()), std::string(""));
    CHECK_EQ(nfkcCasefold("Hello, World!"), std::string("hello, world!"));
    CHECK_EQ(nfkcCasefold("ABC123"), std::string("abc123"));
    CHECK_EQ(nfkcCasefold("already folded"), std::string("already folded"));
    // U+4F60 U+597D (ni hao) has no case mapping and must pass through.
    CHECK_EQ(nfkcCasefold("\xE4\xBD\xA0\xE5\xA5\xBD"), std::string("\xE4\xBD\xA0\xE5\xA5\xBD"));
    // U+FFFD is not default-ignorable, so it survives folding untouched.
    CHECK_EQ(nfkcCasefold(kReplacement), std::string(kReplacement));
}

TEST_CASE("nfkcCasefold expands sharp s and ligatures")
{
    using bunch::nfkcCasefold;

    CHECK_EQ(nfkcCasefold(surrounded("Stra", kSharpS, "e")), std::string("strasse"));
    CHECK_EQ(nfkcCasefold(kCapitalSharpS), std::string("ss"));
    CHECK_EQ(nfkcCasefold(surrounded("STRA", kCapitalSharpS, "E")), std::string("strasse"));
    CHECK_EQ(nfkcCasefold(kFfLigature), std::string("ff"));
    CHECK_EQ(nfkcCasefold(kFiLigature), std::string("fi"));
    CHECK_EQ(nfkcCasefold(kFfiLigature), std::string("ffi"));
    CHECK(bunch::caselessMatch(surrounded("Stra", kSharpS, "e"), "STRASSE"));
}

TEST_CASE("nfkcCasefold applies compatibility decomposition")
{
    using bunch::nfkcCasefold;

    CHECK_EQ(nfkcCasefold(kCircledCapitalA), std::string("a"));
    CHECK_EQ(nfkcCasefold(kFullwidthCapitalA), std::string("a"));
    CHECK_EQ(nfkcCasefold(kMathBoldCapitalA), std::string("a"));
    CHECK_EQ(nfkcCasefold(kSquaredCapitalA), std::string("a"));
    CHECK_EQ(nfkcCasefold(kRomanNumeralOne), std::string("i"));
    // U+2126 OHM SIGN normalises to capital omega before folding.
    CHECK_EQ(nfkcCasefold("\xE2\x84\xA6"), std::string("\xCF\x89"));
    CHECK_EQ(nfkcCasefold("\xE2\x84\xAA"), std::string("k")); // U+212A KELVIN SIGN
    CHECK_EQ(nfkcCasefold("\xC2\xB2"), std::string("2"));     // U+00B2 SUPERSCRIPT TWO
    // U+00BD VULGAR FRACTION ONE HALF becomes "1" U+2044 "2".
    CHECK_EQ(nfkcCasefold("\xC2\xBD"), std::string("1\xE2\x81\x84") + "2");
    // U+1E9B normalises to long s plus dot above, the long s folds to s and the
    // result recomposes to U+1E61: folding must happen after NFKC.
    CHECK_EQ(nfkcCasefold("\xE1\xBA\x9B"), std::string("\xE1\xB9\xA1"));
}

TEST_CASE("nfkcCasefold dotted I and Greek sigma")
{
    using bunch::nfkcCasefold;

    CHECK_EQ(nfkcCasefold(kDottedCapitalI), std::string("i\xCC\x87"));
    CHECK_EQ(nfkcCasefold(std::string(kDottedCapitalI) + "stanbul"), std::string("i\xCC\x87stanbul"));
    CHECK_EQ(nfkcCasefold(kCapitalSigma), std::string(kSmallSigma));
    CHECK_EQ(nfkcCasefold(kFinalSigma), std::string(kSmallSigma));
    CHECK(bunch::caselessMatch(kCapitalSigma, kFinalSigma));
    // U+0345 COMBINING GREEK YPOGEGRAMMENI folds to U+03B9 SMALL LETTER IOTA.
    CHECK_EQ(nfkcCasefold("\xCD\x85"), std::string("\xCE\xB9"));
}

TEST_CASE("nfkcCasefold removes default-ignorable characters")
{
    using bunch::nfkcCasefold;

    CHECK_EQ(nfkcCasefold(surrounded("a", kSoftHyphen, "b")), std::string("ab"));
    CHECK_EQ(nfkcCasefold(kSoftHyphen), std::string(""));
    CHECK_EQ(nfkcCasefold(surrounded("soft", kSoftHyphen, "hyphen")), std::string("softhyphen"));
    CHECK_EQ(nfkcCasefold(std::string(kZeroWidthSpace) + std::string(kZeroWidthJoiner) + std::string(kByteOrderMark)),
             std::string(""));
    CHECK_EQ(nfkcCasefold(surrounded("a", kWordJoiner, "b")), std::string("ab"));
    CHECK_EQ(nfkcCasefold(surrounded("a", kVariationSelectorOne, "b")), std::string("ab"));
    CHECK_EQ(nfkcCasefold(surrounded("a", kGraphemeJoiner, "b")), std::string("ab"));
    CHECK_FALSE(bunch::isNfkcCasefolded(surrounded("a", kSoftHyphen, "b")));
    CHECK(bunch::caselessMatch(surrounded("a", kSoftHyphen, "b"), "AB"));
}

TEST_CASE("nfkcCasefold normalises combining sequences before comparing")
{
    using bunch::nfkcCasefold;

    CHECK_EQ(nfkcCasefold(joined("e", kCombiningAcute)), std::string(kSmallEAcute));
    CHECK_EQ(nfkcCasefold(kSmallEAcute), std::string(kSmallEAcute));
    CHECK_EQ(nfkcCasefold(joined("A", kCombiningRingAbove)), std::string(kSmallARing));
    CHECK_EQ(nfkcCasefold(kCapitalARing), std::string(kSmallARing));
    CHECK(bunch::caselessMatch(joined("cafe", kCombiningAcute), joined("CAF", kCapitalEAcute)));
    // Canonical ordering makes the two spellings of "q" with dot above and dot
    // below identical.
    CHECK(bunch::caselessMatch(joined("q", std::string(kCombiningDotAbove) + std::string(kCombiningDotBelow)),
                               joined("q", std::string(kCombiningDotBelow) + std::string(kCombiningDotAbove))));
}

TEST_CASE("nfkcCasefold is idempotent and NFKC stable")
{
    const std::vector<std::string> samples = {
        "",
        "abc",
        "Stra" + std::string(kSharpS) + "e",
        std::string(kCapitalSharpS),
        std::string(kDottedCapitalI),
        std::string(kCircledCapitalA),
        std::string(kMathBoldCapitalA),
        std::string(kRomanNumeralOne),
        std::string(kGraphemeJoiner),
        surrounded("a", kSoftHyphen, "b"),
        std::string(kCombiningAcute),
        joined("e", kCombiningAcute),
        std::string(kFinalSigma),
        std::string(kReplacement),
    };

    for (const std::string &sample : samples)
    {
        const std::string once = bunch::nfkcCasefold(sample);
        CHECK_EQ(bunch::nfkcCasefold(once), once);
        CHECK_EQ(bunch::nfkc(once), once);
        CHECK(bunch::isNfkcCasefolded(once));
    }
}

TEST_CASE("caselessMatch is symmetric and reflexive")
{
    const std::vector<std::string> samples = {
        "",
        "a",
        "A",
        "abc",
        "ABC",
        std::string(kSharpS),
        "Stra" + std::string(kSharpS) + "e",
        "STRASSE",
        std::string(kCircledCapitalA),
        std::string(kFullwidthCapitalA),
        std::string(kDottedCapitalI),
        std::string(kDotlessSmallI),
        std::string(kCombiningAcute),
        joined("i", kCombiningDotAbove),
        std::string(kSoftHyphen),
        std::string(kReplacement),
    };

    for (const std::string &left : samples)
    {
        CHECK(bunch::caselessMatch(left, left));
        for (const std::string &right : samples)
        {
            CHECK_EQ(bunch::caselessMatch(left, right), bunch::caselessMatch(right, left));
        }
    }
}

TEST_CASE("caselessMatch groups case and compatibility variants")
{
    const std::vector<std::string> strasse = {
        "strasse",
        "STRASSE",
        "Stra" + std::string(kSharpS) + "e",
        "STRA" + std::string(kCapitalSharpS) + "E",
    };
    for (const std::string &left : strasse)
    {
        for (const std::string &right : strasse)
        {
            CHECK(bunch::caselessMatch(left, right));
        }
    }

    const std::vector<std::string> letterA = {
        "a",
        "A",
        std::string(kCircledCapitalA),
        std::string(kFullwidthCapitalA),
        std::string(kMathBoldCapitalA),
        std::string(kSquaredCapitalA),
    };
    for (const std::string &left : letterA)
    {
        for (const std::string &right : letterA)
        {
            CHECK(bunch::caselessMatch(left, right));
        }
    }

    CHECK(bunch::caselessMatch(std::string(kDottedCapitalI) + "stanbul", joined("i", std::string(kCombiningDotAbove) + "stanbul")));
    // Folding is not accent-insensitive: the dot matters.
    CHECK_FALSE(bunch::caselessMatch(std::string(kDottedCapitalI) + "stanbul", "istanbul"));
}

TEST_CASE("caselessMatch rejects different text")
{
    CHECK_FALSE(bunch::caselessMatch("abc", "abd"));
    CHECK_FALSE(bunch::caselessMatch("", "a"));
    CHECK_FALSE(bunch::caselessMatch("Stra" + std::string(kSharpS) + "e", "strase"));
    CHECK_FALSE(bunch::caselessMatch("a", "b"));
    CHECK_FALSE(bunch::caselessMatch(std::string(kDotlessSmallI), "i"));
    CHECK_FALSE(bunch::caselessMatch("hello", "hello "));
}

TEST_CASE("nfkcCasefold replaces ill-formed UTF-8")
{
    const std::string replacement(kReplacement);

    CHECK_EQ(bunch::nfkcCasefold("\xFF"), replacement);
    CHECK_EQ(bunch::nfkcCasefold("\xC3"), replacement);
    CHECK_EQ(bunch::nfkcCasefold("\xE2\x82"), replacement);
    CHECK_EQ(bunch::nfkcCasefold("\xF0\x9F\x98"), replacement);
    // Each byte of an overlong two-byte sequence is its own maximal subpart.
    CHECK_EQ(bunch::nfkcCasefold("\xC0\x80"), replacement + replacement);
    CHECK_EQ(bunch::nfkcCasefold("\xED\xA0\x80"), replacement);    // UTF-16 surrogate
    CHECK_EQ(bunch::nfkcCasefold("\xF4\x90\x80\x80"), replacement); // above U+10FFFF
    CHECK(bunch::caselessMatch("\xFF", kReplacement));
    CHECK_FALSE(bunch::isNfkcCasefolded("\xFF"));
    // Ill-formed text does not stop the rest of the string from folding.
    CHECK_EQ(bunch::nfkcCasefold("\xFF\xFE" "A"), replacement + replacement + "a");
}

TEST_CASE("nfkcCasefold Turkic tailoring")
{
    using bunch::nfkcCasefold;

    CHECK_EQ(nfkcCasefold("I", "tr"), std::string(kDotlessSmallI));
    CHECK_EQ(nfkcCasefold("I", "az"), std::string(kDotlessSmallI));
    CHECK_EQ(nfkcCasefold("I"), std::string("i"));
    CHECK_EQ(nfkcCasefold(std::string(kDottedCapitalI), "tr"), std::string("i"));
    CHECK_EQ(nfkcCasefold(std::string(kDottedCapitalI)), std::string("i\xCC\x87"));
    CHECK_EQ(nfkcCasefold("Istanbul", "tr"), std::string(kDotlessSmallI) + "stanbul");
    CHECK_EQ(nfkcCasefold(std::string(kFullwidthCapitalI), "tr"), std::string(kDotlessSmallI));
    CHECK_EQ(nfkcCasefold(kRomanNumeralOne, "tr"), std::string(kDotlessSmallI));
    CHECK_EQ(nfkcCasefold("I", "de"), std::string("i"));
    CHECK_EQ(nfkcCasefold(std::string(kDotlessSmallI), "tr"), std::string(kDotlessSmallI));
    // caselessMatch stays language-neutral, so plain I never equals dotless i.
    CHECK(bunch::caselessMatch("I", "i"));
    CHECK_FALSE(bunch::caselessMatch("I", kDotlessSmallI));
}

TEST_CASE("isNfkcCasefolded detects the folded form")
{
    CHECK(bunch::isNfkcCasefolded(""));
    CHECK(bunch::isNfkcCasefolded("abc123"));
    CHECK(bunch::isNfkcCasefolded("strasse"));
    CHECK(bunch::isNfkcCasefolded(std::string(kSmallEAcute)));
    CHECK(bunch::isNfkcCasefolded("i\xCC\x87stanbul"));
    CHECK_FALSE(bunch::isNfkcCasefolded("ABC"));
    CHECK_FALSE(bunch::isNfkcCasefolded("Stra" + std::string(kSharpS) + "e"));
    CHECK_FALSE(bunch::isNfkcCasefolded(std::string(kDottedCapitalI)));
    CHECK_FALSE(bunch::isNfkcCasefolded(std::string(kCircledCapitalA)));
    CHECK_FALSE(bunch::isNfkcCasefolded("\xFF"));
}

TEST_CASE("nfkcCasefold handles long mixed text")
{
    std::string text;
    for (int index = 0; index < 64; ++index)
    {
        text += "Stra";
        text += kSharpS;
        text += "e ";
        text += kFullwidthCapitalA;
        text += kSoftHyphen;
    }

    std::string expected;
    for (int index = 0; index < 64; ++index)
    {
        expected += "strasse a";
    }

    const std::string folded = bunch::nfkcCasefold(text);
    CHECK_EQ(folded, expected);
    CHECK_EQ(bunch::nfkcCasefold(folded), folded);
    CHECK(bunch::caselessMatch(text, expected));
}
