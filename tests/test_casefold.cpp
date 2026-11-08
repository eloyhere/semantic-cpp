#include "bunch/casefold.h"

#include "bunch/sequence.h"

#include "test_framework.h"

#include <string>
#include <string_view>

namespace
{
// Byte sequences are written as escapes so this file stays pure ASCII and does
// not depend on the compiler's source encoding.
constexpr std::string_view kSharpS = "\xC3\x9F";               // U+00DF LATIN SMALL LETTER SHARP S
constexpr std::string_view kCapitalSharpS = "\xE1\xBA\x9E";    // U+1E9E LATIN CAPITAL LETTER SHARP S
constexpr std::string_view kDottedCapitalI = "\xC4\xB0";       // U+0130 LATIN CAPITAL LETTER I WITH DOT ABOVE
constexpr std::string_view kDotlessSmallI = "\xC4\xB1";        // U+0131 LATIN SMALL LETTER DOTLESS I
constexpr std::string_view kCapitalSigma = "\xCE\xA3";         // U+03A3 GREEK CAPITAL LETTER SIGMA
constexpr std::string_view kSmallSigma = "\xCF\x83";           // U+03C3 GREEK SMALL LETTER SIGMA
constexpr std::string_view kFinalSigma = "\xCF\x82";           // U+03C2 GREEK SMALL LETTER FINAL SIGMA
constexpr std::string_view kFiLigature = "\xEF\xAC\x81";       // U+FB01 LATIN SMALL LIGATURE FI
constexpr std::string_view kFfiLigature = "\xEF\xAC\x83";      // U+FB03 LATIN SMALL LIGATURE FFI
constexpr std::string_view kTitleDz = "\xC7\x85";              // U+01C5 LATIN CAPITAL D + SMALL Z WITH CARON
constexpr std::string_view kLowerDz = "\xC7\x86";              // U+01C6 LATIN SMALL LETTER DZ WITH CARON
constexpr std::string_view kUpperDz = "\xC7\x84";              // U+01C4 LATIN CAPITAL LETTER DZ WITH CARON
constexpr std::string_view kReplacement = "\xEF\xBF\xBD";      // U+FFFD REPLACEMENT CHARACTER

std::string bytesOf(const bunch::Bunch &value)
{
    const auto bytes = value.getBytes();
    return std::string(bytes.begin(), bytes.end());
}
} // namespace

TEST_CASE("casefold ASCII empty and uncased text")
{
    using bunch::casefold;

    CHECK_EQ(casefold(""), std::string(""));
    CHECK_EQ(casefold("", "tr"), std::string(""));
    CHECK_EQ(casefold("Hello, World!"), std::string("hello, world!"));
    CHECK_EQ(casefold("ABC123"), std::string("abc123"));
    CHECK_EQ(casefold("already folded"), std::string("already folded"));
    // U+4F60 U+597D (ni hao) has no case mapping and must pass through.
    CHECK_EQ(casefold("\xE4\xBD\xA0\xE5\xA5\xBD"), std::string("\xE4\xBD\xA0\xE5\xA5\xBD"));
    CHECK_EQ(casefold(std::string_view()), std::string(""));
}

TEST_CASE("casefold full expansions")
{
    using bunch::casefold;

    // Both sharp s characters fold to the same "ss".
    CHECK_EQ(casefold(kSharpS), std::string("ss"));
    CHECK_EQ(casefold(kCapitalSharpS), std::string("ss"));
    CHECK_EQ(casefold("Stra" + std::string(kSharpS) + "e"), std::string("strasse"));
    CHECK_EQ(casefold("STRASSE"), std::string("strasse"));
    CHECK_EQ(casefold("Stra" + std::string(kSharpS) + "e"), casefold("STRASSE"));

    // U+0130 folds to "i" plus U+0307 COMBINING DOT ABOVE: two code points.
    CHECK_EQ(casefold(kDottedCapitalI), std::string("i\xCC\x87"));
    CHECK_EQ(casefold(std::string(kDottedCapitalI) + "stanbul"), std::string("i\xCC\x87stanbul"));

    // Compatibility ligatures expand to their letter sequences.
    CHECK_EQ(casefold(kFiLigature), std::string("fi"));
    CHECK_EQ(casefold(kFfiLigature), std::string("ffi"));

    // Titlecase digraphs fold to the lowercase digraph.
    CHECK_EQ(casefold(kTitleDz), std::string(kLowerDz));
    CHECK_EQ(casefold(kUpperDz), std::string(kLowerDz));
    CHECK_EQ(casefold(kLowerDz), std::string(kLowerDz));
}

TEST_CASE("casefold Greek sigma")
{
    using bunch::casefold;

    // Capital sigma and final sigma both fold to medial sigma (U+03C3).
    CHECK_EQ(casefold(kCapitalSigma), std::string(kSmallSigma));
    CHECK_EQ(casefold(kFinalSigma), std::string(kSmallSigma));
    CHECK_EQ(casefold(kSmallSigma), std::string(kSmallSigma));

    // Folding is context-free, so a word-final sigma becomes medial sigma.
    CHECK_EQ(casefold("\xCE\x9F\xCE\x94\xCE\x9F\xCE\xA3"), std::string("\xCE\xBF\xCE\xB4\xCE\xBF\xCF\x83"));
    CHECK_EQ(casefold("\xCE\xA3\xCE\x9F\xCE\xA6\xCE\x9F\xCE\xA3"),
             std::string("\xCF\x83\xCE\xBF\xCF\x86\xCE\xBF\xCF\x83"));
}

TEST_CASE("casefold Turkic tailoring")
{
    using bunch::casefold;

    // Default and non-Turkic: capital I folds to plain i.
    CHECK_EQ(casefold("I"), std::string("i"));
    CHECK_EQ(casefold("I", "en"), std::string("i"));
    // Turkic T mapping: capital I folds to dotless i.
    CHECK_EQ(casefold("I", "tr"), std::string(kDotlessSmallI));
    CHECK_EQ(casefold("I", "az"), std::string(kDotlessSmallI));

    // Default: dotted capital I folds to i + combining dot.
    CHECK_EQ(casefold(kDottedCapitalI), std::string("i\xCC\x87"));
    CHECK_EQ(casefold(kDottedCapitalI, "en"), std::string("i\xCC\x87"));
    // Turkic T mapping: dotted capital I folds to plain i.
    CHECK_EQ(casefold(kDottedCapitalI, "tr"), std::string("i"));
    CHECK_EQ(casefold(kDottedCapitalI, "az"), std::string("i"));

    // Dotless i has no case folding, with or without the Turkic tailoring.
    CHECK_EQ(casefold(kDotlessSmallI), std::string(kDotlessSmallI));
    CHECK_EQ(casefold(kDotlessSmallI, "tr"), std::string(kDotlessSmallI));

    CHECK_EQ(casefold("TITLE", "tr"), std::string("t\xC4\xB1tle"));
    CHECK_EQ(casefold("D\xC4\xB0YARBAKIR", "tr"), std::string("diyarbak\xC4\xB1r"));
    // Other language tags keep the default mappings.
    CHECK_EQ(casefold("I", "de"), std::string("i"));
    CHECK_EQ(casefold(kDottedCapitalI, "de"), std::string("i\xCC\x87"));
}

TEST_CASE("casefold idempotence")
{
    using bunch::casefold;

    const std::string_view samples[] = {
        "",
        "Hello, World!",
        "Stra\xC3\x9F" "e",
        "STRASSE",
        "\xC4\xB0STANBUL",
        "\xCE\x9F\xCE\x94\xCE\x9F\xCE\xA3",
        "\xCE\xA3\xCE\x9F\xCE\xA6\xCE\x9F\xCE\xA3",
        "\xEF\xAC\x81\xEF\xAC\x83",
        "\xC7\x85\xC7\x84\xC7\x86",
        "\xE1\xBA\x9E",
    };
    for (std::string_view sample : samples)
    {
        const std::string once = casefold(sample);
        CHECK_EQ(casefold(once), once);
        CHECK_EQ(casefold(casefold(once)), once);
    }

    // The Turkic tailoring is idempotent as well.
    const std::string turkicOnce = casefold("D\xC4\xB0YARBAKIR TITLE", "tr");
    CHECK_EQ(casefold(turkicOnce, "tr"), turkicOnce);

    // A mixed sentence: expansion, Greek sigma and a ligature in one string.
    const std::string mixed = "Stra\xC3\x9F" "e \xCE\x9F\xCE\x94\xCE\x9F\xCE\xA3 \xEF\xAC\x81";
    const std::string folded = casefold(mixed);
    CHECK_EQ(folded, std::string("strasse \xCE\xBF\xCE\xB4\xCE\xBF\xCF\x83 fi"));
    CHECK_EQ(casefold(folded), folded);
}

TEST_CASE("casefold ill-formed UTF-8")
{
    using bunch::casefold;

    // Each ill-formed sequence becomes one U+FFFD; no read past the end.
    CHECK_EQ(casefold("\xFF"), std::string(kReplacement));
    CHECK_EQ(casefold("\x80"), std::string(kReplacement));
    CHECK_EQ(casefold("A\xFF" "B"), std::string("a") + std::string(kReplacement) + "b");
    CHECK_EQ(casefold("\xE2\x82"), std::string(kReplacement));             // truncated three-byte
    CHECK_EQ(casefold("\xF0\x9F\x92"), std::string(kReplacement));         // truncated four-byte
    CHECK_EQ(casefold("\xED\xA0\x80"), std::string(kReplacement));         // encoded surrogate
    CHECK_EQ(casefold("\xC0\xAF"), std::string(kReplacement) + std::string(kReplacement)); // overlong
    CHECK_EQ(casefold("\xEF\xBF\xBD"), std::string(kReplacement));         // already U+FFFD
    // Valid text around the bad bytes is still folded.
    CHECK_EQ(casefold("\xC3\x9F\xFF"), std::string("ss") + std::string(kReplacement));
    // Lead byte with a missing continuation at end of input.
    CHECK_EQ(casefold("\xC3"), std::string(kReplacement));
}

TEST_CASE("Bunch full Unicode case conversion")
{
    using bunch::Bunch;

    CHECK_EQ(bytesOf(Bunch("stra\xC3\x9F" "e").toUpperCase()), std::string("STRASSE"));
    CHECK_EQ(bytesOf(Bunch("Stra\xC3\x9F" "e").toUpperCase()), std::string("STRASSE"));
    CHECK_EQ(bytesOf(Bunch("STRASSE").toLowerCase()), std::string("strasse"));
    CHECK_EQ(bytesOf(Bunch("AbC\xC3\x87").toUpperCase()), std::string("ABC\xC3\x87"));
    CHECK_EQ(bytesOf(Bunch("AbC\xC3\x87").toLowerCase()), std::string("abc\xC3\xA7"));
    CHECK_EQ(bytesOf(Bunch("AbC").toUpperCase()), std::string("ABC"));
    CHECK_EQ(bytesOf(Bunch("AbC").toLowerCase()), std::string("abc"));

    // Lowercasing dotted capital I keeps the combining dot.
    CHECK_EQ(bytesOf(Bunch("\xC4\xB0STANBUL").toLowerCase()), std::string("i\xCC\x87stanbul"));
    // Without a language tag Bunch uses non-Turkic mappings: I lowers to i.
    CHECK_EQ(bytesOf(Bunch("I").toLowerCase()), std::string("i"));

    // Greek final sigma is context-sensitive: word-final sigma becomes U+03C2.
    CHECK_EQ(bytesOf(Bunch("\xCE\x9F\xCE\x94\xCE\x9F\xCE\xA3").toLowerCase()),
             std::string("\xCE\xBF\xCE\xB4\xCE\xBF\xCF\x82"));
    CHECK_EQ(bytesOf(Bunch("\xCE\x9F\xCE\x94\xCE\x9F\xCE\xA3\xCE\x91").toLowerCase()),
             std::string("\xCE\xBF\xCE\xB4\xCE\xBF\xCF\x83\xCE\xB1"));

    // Uppercase expands small sharp s; capital sharp s is already uppercase and
    // only lowercases back to small sharp s.
    CHECK_EQ(bytesOf(Bunch(kSharpS).toUpperCase()), std::string("SS"));
    CHECK_EQ(bytesOf(Bunch(kCapitalSharpS).toUpperCase()), std::string(kCapitalSharpS));
    CHECK_EQ(bytesOf(Bunch(kCapitalSharpS).toLowerCase()), std::string(kSharpS));
    CHECK_EQ(bytesOf(Bunch(kFiLigature).toUpperCase()), std::string("FI"));
    CHECK_EQ(bytesOf(Bunch("\xC7\x86").toUpperCase()), std::string(kUpperDz));
    CHECK_EQ(bytesOf(Bunch("\xC7\x86").toLowerCase()), std::string(kLowerDz));

    // Empty input stays empty and keeps its storage encoding.
    CHECK(Bunch().toUpperCase().empty());
    CHECK(Bunch().toLowerCase().empty());
    CHECK(Bunch("").toUpperCase().empty());
    CHECK_EQ(Bunch().toUpperCase().encoding(), ::bunch::charset::utf8);

    // storageEncoding is independent of the code points and must be preserved.
    const Bunch utf16(std::string_view("AbC"), ::bunch::charset::utf8, ::bunch::charset::utf16le);
    CHECK_EQ(utf16.toUpperCase().encoding(), ::bunch::charset::utf16le);
    CHECK_EQ(utf16.toLowerCase().encoding(), ::bunch::charset::utf16le);

    // Ill-formed stored input was already replaced by the codec, and folding
    // keeps the replacement character.
    CHECK_EQ(bytesOf(Bunch("\xFF").toUpperCase()), std::string(kReplacement));
    CHECK_EQ(bytesOf(Bunch("A\xFF" "B").toLowerCase()), std::string("a") + std::string(kReplacement) + "b");

    // The mapped result is stable under a second application.
    const Bunch upper = Bunch("Stra\xC3\x9F" "e").toUpperCase();
    CHECK_EQ(bytesOf(upper.toUpperCase()), bytesOf(upper));
    const Bunch lower = Bunch("\xC4\xB0STANBUL").toLowerCase();
    CHECK_EQ(bytesOf(lower.toLowerCase()), bytesOf(lower));
}
