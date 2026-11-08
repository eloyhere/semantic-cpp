#include "bunch/properties.h"

#include "test_framework.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace
{
using bunch::EastAsianWidth;
using bunch::GeneralCategory;
using bunch::Script;

struct CategoryCase
{
    std::uint32_t codepoint;
    GeneralCategory category;
    const char *name;
};

constexpr CategoryCase kCategoryCases[] = {
    {0x0041, GeneralCategory::upperLetter, "Lu"},          // LATIN CAPITAL LETTER A
    {0x0061, GeneralCategory::lowerLetter, "Ll"},          // LATIN SMALL LETTER A
    {0x01C5, GeneralCategory::titlecaseLetter, "Lt"},      // LATIN CAPITAL D, SMALL Z
    {0x02B0, GeneralCategory::modifierLetter, "Lm"},       // MODIFIER LETTER SMALL H
    {0x4E2D, GeneralCategory::otherLetter, "Lo"},          // CJK UNIFIED IDEOGRAPH-4E2D
    {0x0301, GeneralCategory::nonspacingMark, "Mn"},       // COMBINING ACUTE ACCENT
    {0x0903, GeneralCategory::spacingMark, "Mc"},          // DEVANAGARI SIGN VISARGA
    {0x0488, GeneralCategory::enclosingMark, "Me"},        // CYRILLIC HUNDRED THOUSANDS
    {0x0031, GeneralCategory::decimalNumber, "Nd"},        // DIGIT ONE
    {0x2160, GeneralCategory::letterNumber, "Nl"},         // ROMAN NUMERAL ONE
    {0x00B2, GeneralCategory::otherNumber, "No"},          // SUPERSCRIPT TWO
    {0x203F, GeneralCategory::connectorPunctuation, "Pc"}, // UNDERTIE
    {0x2010, GeneralCategory::dashPunctuation, "Pd"},      // HYPHEN
    {0x0028, GeneralCategory::openPunctuation, "Ps"},      // LEFT PARENTHESIS
    {0x0029, GeneralCategory::closePunctuation, "Pe"},     // RIGHT PARENTHESIS
    {0x00AB, GeneralCategory::initialPunctuation, "Pi"},   // LEFT-POINTING ANGLE
    {0x00BB, GeneralCategory::finalPunctuation, "Pf"},     // RIGHT-POINTING ANGLE
    {0x0021, GeneralCategory::otherPunctuation, "Po"},     // EXCLAMATION MARK
    {0x002B, GeneralCategory::mathSymbol, "Sm"},           // PLUS SIGN
    {0x0024, GeneralCategory::currencySymbol, "Sc"},       // DOLLAR SIGN
    {0x005E, GeneralCategory::modifierSymbol, "Sk"},       // CIRCUMFLEX ACCENT
    {0x1F600, GeneralCategory::otherSymbol, "So"},         // GRINNING FACE
    {0x0020, GeneralCategory::spaceSeparator, "Zs"},       // SPACE
    {0x2028, GeneralCategory::lineSeparator, "Zl"},        // LINE SEPARATOR
    {0x2029, GeneralCategory::paragraphSeparator, "Zp"},   // PARAGRAPH SEPARATOR
    {0x0000, GeneralCategory::control, "Cc"},              // NULL
    {0x200B, GeneralCategory::format, "Cf"},               // ZERO WIDTH SPACE
    {0xD800, GeneralCategory::surrogate, "Cs"},            // high surrogate
    {0xE000, GeneralCategory::privateUse, "Co"},           // private use
    {0x0378, GeneralCategory::unassigned, "Cn"},           // unassigned
    {0x00E9, GeneralCategory::lowerLetter, "Ll"},          // LATIN SMALL E ACUTE
    {0x03A9, GeneralCategory::upperLetter, "Lu"},          // GREEK CAPITAL OMEGA
};

struct CategoryNameCase
{
    GeneralCategory category;
    const char *name;
};

constexpr CategoryNameCase kCategoryNames[] = {
    {GeneralCategory::upperLetter, "Lu"},
    {GeneralCategory::lowerLetter, "Ll"},
    {GeneralCategory::titlecaseLetter, "Lt"},
    {GeneralCategory::modifierLetter, "Lm"},
    {GeneralCategory::otherLetter, "Lo"},
    {GeneralCategory::nonspacingMark, "Mn"},
    {GeneralCategory::spacingMark, "Mc"},
    {GeneralCategory::enclosingMark, "Me"},
    {GeneralCategory::decimalNumber, "Nd"},
    {GeneralCategory::letterNumber, "Nl"},
    {GeneralCategory::otherNumber, "No"},
    {GeneralCategory::connectorPunctuation, "Pc"},
    {GeneralCategory::dashPunctuation, "Pd"},
    {GeneralCategory::openPunctuation, "Ps"},
    {GeneralCategory::closePunctuation, "Pe"},
    {GeneralCategory::initialPunctuation, "Pi"},
    {GeneralCategory::finalPunctuation, "Pf"},
    {GeneralCategory::otherPunctuation, "Po"},
    {GeneralCategory::mathSymbol, "Sm"},
    {GeneralCategory::currencySymbol, "Sc"},
    {GeneralCategory::modifierSymbol, "Sk"},
    {GeneralCategory::otherSymbol, "So"},
    {GeneralCategory::spaceSeparator, "Zs"},
    {GeneralCategory::lineSeparator, "Zl"},
    {GeneralCategory::paragraphSeparator, "Zp"},
    {GeneralCategory::control, "Cc"},
    {GeneralCategory::format, "Cf"},
    {GeneralCategory::surrogate, "Cs"},
    {GeneralCategory::privateUse, "Co"},
    {GeneralCategory::unassigned, "Cn"},
};

struct ScriptCase
{
    std::uint32_t codepoint;
    Script script;
    const char *name;
};

constexpr ScriptCase kScriptCases[] = {
    {0x0041, Script::latin, "Latin"},
    {0x0061, Script::latin, "Latin"},
    {0x00E9, Script::latin, "Latin"},
    {0x03A9, Script::greek, "Greek"},
    {0x0410, Script::cyrillic, "Cyrillic"},
    {0x4E2D, Script::han, "Han"},
    {0x4E00, Script::han, "Han"},
    {0x3042, Script::hiragana, "Hiragana"},
    {0x30A2, Script::katakana, "Katakana"},
    {0xAC00, Script::hangul, "Hangul"},
    {0x0627, Script::arabic, "Arabic"},
    {0x05D0, Script::hebrew, "Hebrew"},
    {0x0915, Script::devanagari, "Devanagari"},
    {0x0E01, Script::thai, "Thai"},
    {0x1680, Script::ogham, "Ogham"},
    {0x1F600, Script::common, "Common"},
    {0x200B, Script::common, "Common"},
    {0x0301, Script::inherited, "Inherited"},
    {0x200D, Script::inherited, "Inherited"},
    {0xE000, Script::unknown, "Unknown"},
    {0x0378, Script::unknown, "Unknown"},
    {0xD800, Script::unknown, "Unknown"},
};

struct WidthCase
{
    std::uint32_t codepoint;
    EastAsianWidth width;
    const char *name;
};

constexpr WidthCase kWidthCases[] = {
    {0x0041, EastAsianWidth::narrow, "Narrow"},
    {0x0061, EastAsianWidth::narrow, "Narrow"},
    {0x0031, EastAsianWidth::narrow, "Narrow"},
    {0x0020, EastAsianWidth::narrow, "Narrow"},
    {0x00E9, EastAsianWidth::ambiguous, "Ambiguous"},
    {0x03A9, EastAsianWidth::ambiguous, "Ambiguous"},
    {0x0301, EastAsianWidth::ambiguous, "Ambiguous"},
    {0x4E2D, EastAsianWidth::wide, "Wide"},
    {0x1F600, EastAsianWidth::wide, "Wide"},
    {0x200B, EastAsianWidth::neutral, "Neutral"},
    {0x0378, EastAsianWidth::neutral, "Neutral"},
    {0xE000, EastAsianWidth::ambiguous, "Ambiguous"},
    {0x3000, EastAsianWidth::fullwidth, "Fullwidth"},
    {0xFF01, EastAsianWidth::fullwidth, "Fullwidth"},
    {0xFF66, EastAsianWidth::halfwidth, "Halfwidth"},
    // Unassigned code points use the UAX #11 @missing defaults.
    {0x2FFFD, EastAsianWidth::wide, "Wide"},
    {0x3FFFD, EastAsianWidth::wide, "Wide"},
    {0x3FFFE, EastAsianWidth::neutral, "Neutral"},
    {0xFA6E, EastAsianWidth::wide, "Wide"},
};

struct BinaryCase
{
    std::uint32_t codepoint;
    bool alphabetic;
    bool whiteSpace;
    bool uppercase;
    bool lowercase;
    bool defaultIgnorable;
};

constexpr BinaryCase kBinaryCases[] = {
    {0x0041, true, false, true, false, false},   // A
    {0x0061, true, false, false, true, false},   // a
    {0x0031, false, false, false, false, false}, // 1
    {0x0009, false, true, false, false, false},  // TAB
    {0x000D, false, true, false, false, false},  // CR
    {0x0020, false, true, false, false, false},  // SPACE
    {0x0085, false, true, false, false, false},  // NEL
    {0x00A0, false, true, false, false, false},  // NO-BREAK SPACE
    {0x1680, false, true, false, false, false},  // OGHAM SPACE MARK
    {0x2000, false, true, false, false, false},  // EN QUAD
    {0x2028, false, true, false, false, false},  // LINE SEPARATOR
    {0x2029, false, true, false, false, false},  // PARAGRAPH SEPARATOR
    {0x3000, false, true, false, false, false},  // IDEOGRAPHIC SPACE
    {0x200B, false, false, false, false, true},  // ZERO WIDTH SPACE
    {0x200D, false, false, false, false, true},  // ZERO WIDTH JOINER
    {0x2060, false, false, false, false, true},  // WORD JOINER
    {0x00AD, false, false, false, false, true},  // SOFT HYPHEN
    {0xFE00, false, false, false, false, true},  // VARIATION SELECTOR-1
    {0xFEFF, false, false, false, false, true},  // ZERO WIDTH NO-BREAK SPACE
    {0x180E, false, false, false, false, true},  // MONGOLIAN VOWEL SEPARATOR
    {0x0301, false, false, false, false, false}, // COMBINING ACUTE ACCENT
    {0x00E9, true, false, false, true, false},   // e ACUTE
    {0x00DF, true, false, false, true, false},   // SHARP S
    {0x0345, true, false, false, true, false},   // COMBINING YPOGEGRAMMENI
    {0x03A9, true, false, true, false, false},   // GREEK CAPITAL OMEGA
    {0x2160, true, false, true, false, false},   // ROMAN NUMERAL ONE (Nl)
    {0x24B6, true, false, true, false, false},   // CIRCLED CAPITAL A (So)
    {0x1D400, true, false, true, false, false},  // MATH BOLD CAPITAL A
    {0x4E2D, true, false, false, false, false},  // CJK 4E2D
    {0x1F600, false, false, false, false, false}, // emoji, not Alphabetic
    {0xE000, false, false, false, false, false},  // private use
    {0x0378, false, false, false, false, false},  // unassigned
};

using bunch::detail::properties::PropertyRange;

constexpr PropertyRange kMaximumCodePoint = {0x10FFFF, 0x10FFFF};

template <typename Range>
auto checkCompleteCoverage(const Range *ranges, std::size_t count) -> void
{
    CHECK(count > 0);
    CHECK_EQ(ranges[0].first, 0u);
    for (std::size_t index = 0; index < count; ++index)
    {
        CHECK(ranges[index].first <= ranges[index].last);
        if (index > 0)
        {
            CHECK_EQ(ranges[index].first, ranges[index - 1].last + 1);
        }
    }
    CHECK_EQ(ranges[count - 1].last, kMaximumCodePoint.first);
}

template <typename Range>
auto checkSparseCoverage(const Range *ranges, std::size_t count) -> void
{
    for (std::size_t index = 0; index < count; ++index)
    {
        CHECK(ranges[index].first <= ranges[index].last);
        CHECK(ranges[index].last <= kMaximumCodePoint.last);
        if (index > 0)
        {
            CHECK(ranges[index].first > ranges[index - 1].last);
        }
    }
}

template <typename Range, typename ValueAccessor, typename Lookup>
auto checkEnumRangeBoundaries(const Range *ranges, std::size_t count,
                              ValueAccessor value, Lookup lookup) -> void
{
    for (std::size_t index = 0; index < count; ++index)
    {
        const Range &range = ranges[index];
        const auto expected = value(range);
        CHECK(lookup(range.first) == expected);
        CHECK(lookup(range.first + (range.last - range.first) / 2) == expected);
        CHECK(lookup(range.last) == expected);
        if (range.first > 0)
        {
            CHECK(!(lookup(range.first - 1) == expected));
        }
    }
}

template <typename Predicate>
auto checkBinaryRangeBoundaries(const PropertyRange *ranges, std::size_t count,
                                Predicate predicate) -> void
{
    for (std::size_t index = 0; index < count; ++index)
    {
        const PropertyRange &range = ranges[index];
        CHECK(predicate(range.first));
        CHECK(predicate(range.first + (range.last - range.first) / 2));
        CHECK(predicate(range.last));
        if (range.first > 0)
        {
            CHECK(!predicate(range.first - 1));
        }
        if (range.last < kMaximumCodePoint.last)
        {
            CHECK(!predicate(range.last + 1));
        }
    }
}

// The lookups are constexpr; these fail to compile if that stops being true.
static_assert(bunch::generalCategory(0x0041) == GeneralCategory::upperLetter,
              "generalCategory must be usable in a constant expression");
static_assert(bunch::generalCategory(0x0378) == GeneralCategory::unassigned,
              "unassigned code points must report Cn");
static_assert(bunch::script(0x4E2D) == Script::han,
              "script must be usable in a constant expression");
static_assert(bunch::eastAsianWidth(0x4E2D) == EastAsianWidth::wide,
              "eastAsianWidth must be usable in a constant expression");
static_assert(bunch::isAlphabetic(0x0041) && !bunch::isAlphabetic(0x0031),
              "isAlphabetic must match the UCD");
static_assert(bunch::isWhiteSpace(0x0020) && !bunch::isWhiteSpace(0x200B),
              "isWhiteSpace must match the UCD");
static_assert(bunch::isDefaultIgnorable(0x00AD) && !bunch::isDefaultIgnorable(0x0041),
              "isDefaultIgnorable must match the UCD");
static_assert(bunch::generalCategoryName(GeneralCategory::upperLetter)[0] == 'L',
              "generalCategoryName must be usable in a constant expression");
static_assert(bunch::scriptName(Script::oldItalic)[0] == 'O',
              "scriptName must be usable in a constant expression");
} // namespace

TEST_CASE("General_Category of known code points")
{
    for (const CategoryCase &entry : kCategoryCases)
    {
        CHECK(bunch::generalCategory(entry.codepoint) == entry.category);
        CHECK_EQ(std::string(bunch::generalCategoryName(bunch::generalCategory(entry.codepoint))),
                 std::string(entry.name));
    }
    // The table above has at least one code point for every category, so a
    // wrong enumerator value cannot hide behind untested values.
    for (const CategoryNameCase &nameCase : kCategoryNames)
    {
        bool found = false;
        for (const CategoryCase &categoryCase : kCategoryCases)
        {
            found = found || categoryCase.category == nameCase.category;
        }
        CHECK(found);
    }
}

TEST_CASE("General_Category names cover every value")
{
    CHECK_EQ(static_cast<std::size_t>(bunch::detail::properties::kCategoryNameCount),
             sizeof(kCategoryNames) / sizeof(kCategoryNames[0]));
    for (const CategoryNameCase &entry : kCategoryNames)
    {
        const std::string name = bunch::generalCategoryName(entry.category);
        CHECK_EQ(name, std::string(entry.name));
        CHECK_EQ(name.size(), std::size_t{2});
    }
    // The enumeration must stay consecutive and end at unassigned.
    CHECK_EQ(static_cast<std::size_t>(GeneralCategory::upperLetter), std::size_t{0});
    CHECK_EQ(static_cast<std::size_t>(GeneralCategory::unassigned) + 1,
             static_cast<std::size_t>(bunch::detail::properties::kCategoryNameCount));
}

TEST_CASE("Script of known code points")
{
    for (const ScriptCase &entry : kScriptCases)
    {
        CHECK(bunch::script(entry.codepoint) == entry.script);
        CHECK_EQ(std::string(bunch::scriptName(bunch::script(entry.codepoint))),
                 std::string(entry.name));
    }
}

TEST_CASE("Script names cover every value")
{
    const std::size_t count = bunch::detail::properties::kScriptNameCount;
    CHECK_EQ(count, std::size_t{164});
    for (std::size_t index = 0; index < count; ++index)
    {
        const std::string name = bunch::scriptName(static_cast<Script>(index));
        CHECK(!name.empty());
        CHECK(name[0] >= 'A' && name[0] <= 'Z');
    }
    CHECK_EQ(std::string(bunch::scriptName(Script::common)), std::string("Common"));
    CHECK_EQ(std::string(bunch::scriptName(Script::inherited)), std::string("Inherited"));
    CHECK_EQ(std::string(bunch::scriptName(Script::unknown)), std::string("Unknown"));
    CHECK_EQ(std::string(bunch::scriptName(Script::latin)), std::string("Latin"));
    CHECK_EQ(std::string(bunch::scriptName(Script::han)), std::string("Han"));
    CHECK_EQ(std::string(bunch::scriptName(Script::oldItalic)), std::string("Old_Italic"));
    CHECK_EQ(std::string(bunch::scriptName(Script::warangCiti)), std::string("Warang_Citi"));
    CHECK_EQ(static_cast<std::size_t>(Script::common), std::size_t{0});
    CHECK_EQ(static_cast<std::size_t>(Script::inherited), std::size_t{1});
    CHECK_EQ(static_cast<std::size_t>(Script::unknown) + 1, count);
}

TEST_CASE("East_Asian_Width of known code points")
{
    for (const WidthCase &entry : kWidthCases)
    {
        CHECK(bunch::eastAsianWidth(entry.codepoint) == entry.width);
        CHECK_EQ(std::string(bunch::eastAsianWidthName(bunch::eastAsianWidth(entry.codepoint))),
                 std::string(entry.name));
    }
    CHECK_EQ(static_cast<std::size_t>(bunch::detail::properties::kWidthNameCount),
             std::size_t{6});
    CHECK_EQ(static_cast<std::size_t>(EastAsianWidth::ambiguous) + 1,
             static_cast<std::size_t>(bunch::detail::properties::kWidthNameCount));
}

TEST_CASE("Binary properties of known code points")
{
    for (const BinaryCase &entry : kBinaryCases)
    {
        CHECK_EQ(bunch::isAlphabetic(entry.codepoint), entry.alphabetic);
        CHECK_EQ(bunch::isWhiteSpace(entry.codepoint), entry.whiteSpace);
        CHECK_EQ(bunch::isUppercase(entry.codepoint), entry.uppercase);
        CHECK_EQ(bunch::isLowercase(entry.codepoint), entry.lowercase);
        CHECK_EQ(bunch::isDefaultIgnorable(entry.codepoint), entry.defaultIgnorable);
    }
}

TEST_CASE("Range tables cover the code space")
{
    using namespace bunch::detail::properties;
    checkCompleteCoverage(kCategoryRanges, kCategoryRangeCount);
    checkCompleteCoverage(kScriptRanges, kScriptRangeCount);
    checkCompleteCoverage(kWidthRanges, kWidthRangeCount);
    checkSparseCoverage(kAlphabeticRanges, kAlphabeticRangeCount);
    checkSparseCoverage(kWhiteSpaceRanges, kWhiteSpaceRangeCount);
    checkSparseCoverage(kUppercaseRanges, kUppercaseRangeCount);
    checkSparseCoverage(kLowercaseRanges, kLowercaseRangeCount);
    checkSparseCoverage(kDefaultIgnorableRanges, kDefaultIgnorableRangeCount);
}

TEST_CASE("Range boundaries resolve to their range value")
{
    using namespace bunch::detail::properties;
    checkEnumRangeBoundaries(kCategoryRanges, kCategoryRangeCount,
                             [](const CategoryRange &range) {
                                 return static_cast<GeneralCategory>(range.category);
                             },
                             [](std::uint32_t codepoint) { return bunch::generalCategory(codepoint); });
    checkEnumRangeBoundaries(kScriptRanges, kScriptRangeCount,
                             [](const ScriptRange &range) {
                                 return static_cast<Script>(range.script);
                             },
                             [](std::uint32_t codepoint) { return bunch::script(codepoint); });
    checkEnumRangeBoundaries(kWidthRanges, kWidthRangeCount,
                             [](const WidthRange &range) {
                                 return static_cast<EastAsianWidth>(range.width);
                             },
                             [](std::uint32_t codepoint) { return bunch::eastAsianWidth(codepoint); });

    checkBinaryRangeBoundaries(kAlphabeticRanges, kAlphabeticRangeCount,
                               [](std::uint32_t codepoint) { return bunch::isAlphabetic(codepoint); });
    checkBinaryRangeBoundaries(kWhiteSpaceRanges, kWhiteSpaceRangeCount,
                               [](std::uint32_t codepoint) { return bunch::isWhiteSpace(codepoint); });
    checkBinaryRangeBoundaries(kUppercaseRanges, kUppercaseRangeCount,
                               [](std::uint32_t codepoint) { return bunch::isUppercase(codepoint); });
    checkBinaryRangeBoundaries(kLowercaseRanges, kLowercaseRangeCount,
                               [](std::uint32_t codepoint) { return bunch::isLowercase(codepoint); });
    checkBinaryRangeBoundaries(kDefaultIgnorableRanges, kDefaultIgnorableRangeCount,
                               [](std::uint32_t codepoint) {
                                   return bunch::isDefaultIgnorable(codepoint);
                               });
}

TEST_CASE("Every property value occurs and category counts match Unicode 15.0.0")
{
    using bunch::detail::properties::kCategoryNameCount;
    using bunch::detail::properties::kScriptNameCount;
    using bunch::detail::properties::kWidthNameCount;

    std::array<std::size_t, kCategoryNameCount> categoryCounts{};
    std::array<std::size_t, kScriptNameCount> scriptCounts{};
    std::array<std::size_t, kWidthNameCount> widthCounts{};
    std::size_t assigned = 0;

    for (std::uint32_t codepoint = 0; codepoint <= 0x10FFFF; ++codepoint)
    {
        const GeneralCategory category = bunch::generalCategory(codepoint);
        categoryCounts[static_cast<std::size_t>(category)] += 1;
        scriptCounts[static_cast<std::size_t>(bunch::script(codepoint))] += 1;
        widthCounts[static_cast<std::size_t>(bunch::eastAsianWidth(codepoint))] += 1;
        if (category != GeneralCategory::unassigned)
        {
            ++assigned;
        }
    }

    CHECK_EQ(assigned, std::size_t{288767});
    CHECK_EQ(categoryCounts[static_cast<std::size_t>(GeneralCategory::unassigned)],
             std::size_t{825345});
    CHECK_EQ(categoryCounts[static_cast<std::size_t>(GeneralCategory::upperLetter)],
             std::size_t{1831});
    CHECK_EQ(categoryCounts[static_cast<std::size_t>(GeneralCategory::lowerLetter)],
             std::size_t{2233});
    CHECK_EQ(categoryCounts[static_cast<std::size_t>(GeneralCategory::otherLetter)],
             std::size_t{131612});
    CHECK_EQ(categoryCounts[static_cast<std::size_t>(GeneralCategory::surrogate)],
             std::size_t{2048});
    CHECK_EQ(categoryCounts[static_cast<std::size_t>(GeneralCategory::privateUse)],
             std::size_t{137468});
    for (std::size_t count : categoryCounts)
    {
        CHECK(count > 0);
    }
    for (std::size_t count : scriptCounts)
    {
        CHECK(count > 0);
    }
    for (std::size_t count : widthCounts)
    {
        CHECK(count > 0);
    }
}

TEST_CASE("Invalid code points and surrogates are safe")
{
    constexpr std::uint32_t invalid[] = {0x110000, 0x00200000, 0xFFFFFFFE, 0xFFFFFFFF};
    for (std::uint32_t codepoint : invalid)
    {
        CHECK(bunch::generalCategory(codepoint) == GeneralCategory::unassigned);
        CHECK(bunch::script(codepoint) == Script::unknown);
        CHECK(bunch::eastAsianWidth(codepoint) == EastAsianWidth::neutral);
        CHECK_FALSE(bunch::isAlphabetic(codepoint));
        CHECK_FALSE(bunch::isWhiteSpace(codepoint));
        CHECK_FALSE(bunch::isUppercase(codepoint));
        CHECK_FALSE(bunch::isLowercase(codepoint));
        CHECK_FALSE(bunch::isDefaultIgnorable(codepoint));
        CHECK_EQ(std::string(bunch::generalCategoryName(bunch::generalCategory(codepoint))),
                 std::string("Cn"));
        CHECK_EQ(std::string(bunch::scriptName(bunch::script(codepoint))),
                 std::string("Unknown"));
    }

    constexpr std::uint32_t surrogates[] = {0xD800, 0xDBFF, 0xDC00, 0xDFFF};
    for (std::uint32_t codepoint : surrogates)
    {
        CHECK(bunch::generalCategory(codepoint) == GeneralCategory::surrogate);
        CHECK(bunch::script(codepoint) == Script::unknown);
        CHECK(bunch::eastAsianWidth(codepoint) == EastAsianWidth::neutral);
        CHECK_FALSE(bunch::isAlphabetic(codepoint));
        CHECK_FALSE(bunch::isWhiteSpace(codepoint));
        CHECK_FALSE(bunch::isUppercase(codepoint));
        CHECK_FALSE(bunch::isLowercase(codepoint));
        CHECK_FALSE(bunch::isDefaultIgnorable(codepoint));
    }

    // Values outside the enumeration can only be produced by a cast; the name
    // functions document and return their fallback instead of reading past
    // the generated tables.
    CHECK_EQ(std::string(bunch::generalCategoryName(static_cast<GeneralCategory>(200))),
             std::string("Cn"));
    CHECK_EQ(std::string(bunch::scriptName(static_cast<Script>(200))),
             std::string("Unknown"));
    CHECK_EQ(std::string(bunch::eastAsianWidthName(static_cast<EastAsianWidth>(200))),
             std::string("Neutral"));

    CHECK(bunch::generalCategory(0) == GeneralCategory::control);
    CHECK(bunch::generalCategory(0x10FFFF) == GeneralCategory::unassigned);
    CHECK(!bunch::isAlphabetic(0x10FFFF));
}

TEST_CASE("Data version is Unicode 15.0.0")
{
    CHECK_EQ(std::string(bunch::detail::properties::kUnicodeVersion),
             std::string("15.0.0"));
}
