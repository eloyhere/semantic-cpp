#include "bunch/unicode.h"
#include "localisation/localisation.h"

#include "normalisation_vectors.h"
#include "test_framework.h"

#include <cstdint>
#include <string>

namespace
{
using localisation::Locale;
using localisation::NormalizationForm;
using localisation::TextFormat;
using localisation::TextStyle;

constexpr std::uint64_t fnvOffset = 14695981039346656037ULL;
constexpr std::uint64_t fnvPrime = 1099511628211ULL;

std::uint64_t hashText(const std::string &text, std::uint64_t hash)
{
    for (unsigned char byte : text)
    {
        hash ^= static_cast<std::uint64_t>(byte);
        hash *= fnvPrime;
    }
    return hash;
}

std::string codepointToUtf8(std::uint32_t codepoint)
{
    std::string text;
    bunch::appendUtf8(text, codepoint);
    return text;
}

bool selectedForHash(std::uint32_t codepoint)
{
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
    {
        return false;
    }
    using namespace bunch::detail;
    const bool hasDecomposition = unicode::findDecomposition(codepoint) != nullptr;
    const bool isHangulSyllable = codepoint >= 0xAC00 && codepoint <= 0xD7A3;
    const bool hasCombiningClass = unicode::combiningClass(codepoint) != 0;
    const bool hasCaseMapping =
        unicode::findCaseMapping(unicode::kLowercaseMappings, unicode::kLowercaseCount, codepoint) != nullptr ||
        unicode::findCaseMapping(unicode::kUppercaseMappings, unicode::kUppercaseCount, codepoint) != nullptr ||
        unicode::findCaseMapping(unicode::kTitlecaseMappings, unicode::kTitlecaseCount, codepoint) != nullptr;
    return hasDecomposition || isHangulSyllable || hasCombiningClass || hasCaseMapping || codepoint % 97 == 0;
}
} // namespace

TEST_CASE("unicode: bunch character helpers are reusable")
{
    // The character-level API lives in bunch; localisation only adds Locale.
    CHECK_EQ(bunch::nfc("e\xCC\x81"), std::string("\xC3\xA9"));
    CHECK_EQ(bunch::nfd("\xC3\xA9"), std::string("e\xCC\x81"));
    CHECK_EQ(bunch::nfkc("\xEF\xBC\xA1"), std::string("A"));
    CHECK(bunch::isNormalized("\xC3\xA9", bunch::NormalizationForm::nfc));

    CHECK_EQ(bunch::formatText("hello world", "en", bunch::TextStyle::title), std::string("Hello World"));
    CHECK_EQ(bunch::formatText("istanbul", "tr", bunch::TextStyle::uppercase),
             std::string("\xC4\xB0STANBUL"));
    CHECK_EQ(bunch::toLowerCase("ABC", "en"), std::string("abc"));

    // The localisation wrappers expose the same values under the Locale API.
    CHECK_EQ(localisation::nfc("e\xCC\x81"), bunch::nfc("e\xCC\x81"));
    CHECK_EQ(localisation::formatText("hello", Locale("en"), TextStyle::title), std::string("Hello"));
}

TEST_CASE("unicode: generated normalisation vectors")
{
    for (std::size_t index = 0; index < kNormalisationVectorCount; ++index)
    {
        const NormalisationVector &vector = kNormalisationVectors[index];
        const std::string input = vector.input;

        CHECK_EQ(localisation::normalize(input, NormalizationForm::nfc), std::string(vector.nfc));
        CHECK_EQ(localisation::normalize(input, NormalizationForm::nfd), std::string(vector.nfd));
        CHECK_EQ(localisation::normalize(input, NormalizationForm::nfkc), std::string(vector.nfkc));
        CHECK_EQ(localisation::normalize(input, NormalizationForm::nfkd), std::string(vector.nfkd));

        CHECK(localisation::isNormalized(vector.nfc, NormalizationForm::nfc));
        CHECK(localisation::isNormalized(vector.nfd, NormalizationForm::nfd));
        CHECK(localisation::isNormalized(vector.nfkc, NormalizationForm::nfkc));
        CHECK(localisation::isNormalized(vector.nfkd, NormalizationForm::nfkd));
    }
}

TEST_CASE("unicode: normalisation hashes cover the data tables")
{
    std::uint64_t hashes[4] = {fnvOffset, fnvOffset, fnvOffset, fnvOffset};
    const NormalizationForm forms[4] = {NormalizationForm::nfc, NormalizationForm::nfd, NormalizationForm::nfkc,
                                       NormalizationForm::nfkd};

    for (std::uint32_t codepoint = 0; codepoint <= 0x10FFFF; ++codepoint)
    {
        if (!selectedForHash(codepoint))
        {
            continue;
        }
        const std::string input = codepointToUtf8(codepoint);
        for (int form = 0; form < 4; ++form)
        {
            hashes[form] = hashText(localisation::normalize(input, forms[form]), hashes[form]);
        }
    }

    for (int form = 0; form < 4; ++form)
    {
        CHECK_EQ(hashes[form], kNormalisationHashes[form]);
    }
}

TEST_CASE("unicode: case mapping hashes cover the data tables")
{
    std::uint64_t hashes[3] = {fnvOffset, fnvOffset, fnvOffset};
    const TextStyle styles[3] = {TextStyle::lowercase, TextStyle::uppercase, TextStyle::title};

    for (std::uint32_t codepoint = 0; codepoint <= 0x10FFFF; ++codepoint)
    {
        if (!selectedForHash(codepoint))
        {
            continue;
        }
        const std::string input = codepointToUtf8(codepoint);
        for (int style = 0; style < 3; ++style)
        {
            hashes[style] = hashText(localisation::formatText(input, Locale("en"), styles[style]), hashes[style]);
        }
    }

    for (int style = 0; style < 3; ++style)
    {
        CHECK_EQ(hashes[style], kCaseHashes[style]);
    }
}

TEST_CASE("unicode: normalisation examples")
{
    using localisation::normalize;

    CHECK_EQ(localisation::nfd("\xC3\xA9"), std::string("e\xCC\x81"));
    CHECK_EQ(localisation::nfc("e\xCC\x81"), std::string("\xC3\xA9"));
    CHECK_EQ(localisation::nfc("\xE2\x84\xAB"), std::string("\xC3\x85"));
    CHECK_EQ(localisation::nfd("\xE2\x84\xAB"), std::string("A\xCC\x8A"));

    CHECK_EQ(localisation::nfd("\xEA\xB0\x81"), std::string("\xE1\x84\x80\xE1\x85\xA1\xE1\x86\xA8"));
    CHECK_EQ(localisation::nfc("\xE1\x84\x80\xE1\x85\xA1"), std::string("\xEA\xB0\x80"));

    CHECK_EQ(localisation::nfkc("\xEF\xAC\x81"), std::string("fi"));
    CHECK_EQ(localisation::nfkc("\xEF\xBC\xA1"), std::string("A"));
    CHECK_EQ(localisation::nfkc("\xE2\x91\xA1"), std::string("2"));
    CHECK_EQ(localisation::nfkc("\xE2\x85\xAB"), std::string("XII"));
    CHECK_EQ(localisation::nfkd("\xE2\x85\xAB"), std::string("XII"));

    CHECK_EQ(localisation::nfd("a\xCC\x81\xCC\xA3"), std::string("a\xCC\xA3\xCC\x81"));
    CHECK_EQ(localisation::nfc("a\xCC\x81\xCC\xA3"), std::string("\xE1\xBA\xA1\xCC\x81"));
    CHECK_EQ(localisation::nfc("\xE0\xA5\x98"), std::string("\xE0\xA4\x95\xE0\xA4\xBC"));

    CHECK_EQ(normalize("plain", NormalizationForm::none), std::string("plain"));
    CHECK(localisation::isNormalized(localisation::nfc("\xE2\x84\xAB"), NormalizationForm::nfc));
    CHECK_FALSE(localisation::isNormalized("\xE2\x84\xAB", NormalizationForm::nfc));
    CHECK(localisation::isNormalized("plain", NormalizationForm::none));
    CHECK_EQ(std::string(localisation::normalizationFormName(NormalizationForm::nfkc)), std::string("nfkc"));
}

TEST_CASE("unicode: locale-aware text styles")
{
    const Locale english("en");
    const Locale turkish("tr");
    const Locale azerbaijani("az");
    const Locale greek("el");
    const Locale dutch("nl");
    const Locale chinese("zh-Hant-TW");

    CHECK_EQ(localisation::formatText("stra\xC3\x9F" "e", english, TextStyle::uppercase),
             std::string("STRASSE"));
    CHECK_EQ(localisation::formatText("istanbul", turkish, TextStyle::uppercase),
             std::string("\xC4\xB0STANBUL"));
    CHECK_EQ(localisation::formatText("ISTANBUL", turkish, TextStyle::lowercase),
             std::string("\xC4\xB1stanbul"));
    CHECK_EQ(localisation::formatText("I", azerbaijani, TextStyle::lowercase), std::string("\xC4\xB1"));
    CHECK_EQ(localisation::formatText("i", azerbaijani, TextStyle::uppercase), std::string("\xC4\xB0"));

    // Greek final sigma: word-final sigma is U+03C2, a standalone sigma is not.
    CHECK_EQ(localisation::formatText("\xCE\xA3\xCE\x9F\xCE\xA6\xCE\x9F\xCE\xA3", greek, TextStyle::lowercase),
             std::string("\xCF\x83\xCE\xBF\xCF\x86\xCE\xBF\xCF\x82"));
    CHECK_EQ(localisation::formatText("\xCE\xA3", greek, TextStyle::lowercase), std::string("\xCF\x83"));

    CHECK_EQ(localisation::formatText("don't stop", english, TextStyle::title), std::string("Don't Stop"));
    CHECK_EQ(localisation::formatText("they're here", english, TextStyle::title), std::string("They're Here"));
    CHECK_EQ(localisation::formatText("hello world", english, TextStyle::title), std::string("Hello World"));
    CHECK_EQ(localisation::formatText("well-known", english, TextStyle::title), std::string("Well-Known"));
    CHECK_EQ(localisation::formatText("123abc", english, TextStyle::title), std::string("123abc"));
    CHECK_EQ(localisation::formatText("ijssel", dutch, TextStyle::title), std::string("IJssel"));
    CHECK_EQ(localisation::formatText("HELLO WORLD", english, TextStyle::capitalize),
             std::string("Hello world"));
    CHECK_EQ(localisation::formatText("\xE4\xB8\xAD\xE6\x96\x87", chinese, TextStyle::title),
             std::string("\xE4\xB8\xAD\xE6\x96\x87"));

    const TextFormat format(english, TextStyle::title, NormalizationForm::nfc);
    CHECK_EQ(format.format("cafe\xCC\x81 au lait"), std::string("Caf\xC3\xA9 Au Lait"));
    CHECK_EQ(format("cafe\xCC\x81"), std::string("Caf\xC3\xA9"));
    CHECK_EQ(std::string(localisation::textStyleName(TextStyle::title)), std::string("title"));

    CHECK_EQ(localisation::toLowerCase("ABC"), std::string("abc"));
    CHECK_EQ(localisation::toTitleCase("hello world"), std::string("Hello World"));
}
