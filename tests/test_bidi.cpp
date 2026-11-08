#include "bunch/bidi.h"

#include "bidi_vectors.h"
#include "test_framework.h"

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
using bunch::BidiAnalysis;
using bunch::BidiRun;

auto testAppendUtf8(std::string &output, std::uint32_t codepoint) -> void
{
    if (codepoint <= 0x7F)
    {
        output.push_back(static_cast<char>(codepoint));
    }
    else if (codepoint <= 0x7FF)
    {
        output.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else if (codepoint <= 0xFFFF)
    {
        output.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else
    {
        output.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
}

auto encode(const std::vector<std::uint32_t> &codepoints) -> std::string
{
    std::string text;
    for (const std::uint32_t codepoint : codepoints)
    {
        testAppendUtf8(text, codepoint);
    }
    return text;
}

auto splitFields(const std::string &line) -> std::vector<std::string_view>
{
    std::vector<std::string_view> fields;
    std::string_view view(line);
    std::size_t start = 0;
    while (true)
    {
        const std::size_t separator = view.find(';', start);
        if (separator == std::string_view::npos)
        {
            fields.push_back(view.substr(start));
            break;
        }
        fields.push_back(view.substr(start, separator - start));
        start = separator + 1;
    }
    return fields;
}

auto splitTokens(std::string_view field) -> std::vector<std::string_view>
{
    std::vector<std::string_view> tokens;
    std::size_t index = 0;
    while (index < field.size())
    {
        while (index < field.size() && field[index] == ' ')
        {
            ++index;
        }
        const std::size_t start = index;
        while (index < field.size() && field[index] != ' ')
        {
            ++index;
        }
        if (index > start)
        {
            tokens.push_back(field.substr(start, index - start));
        }
    }
    return tokens;
}

auto parseHex(std::string_view token) -> std::uint32_t
{
    std::uint32_t value = 0;
    for (const char character : token)
    {
        value *= 16;
        if (character >= '0' && character <= '9')
        {
            value += static_cast<std::uint32_t>(character - '0');
        }
        else if (character >= 'A' && character <= 'F')
        {
            value += static_cast<std::uint32_t>(character - 'A' + 10);
        }
        else if (character >= 'a' && character <= 'f')
        {
            value += static_cast<std::uint32_t>(character - 'a' + 10);
        }
        else
        {
            throw std::runtime_error("invalid hexadecimal code point");
        }
    }
    return value;
}

auto parseDecimal(std::string_view token) -> int
{
    int value = 0;
    for (const char character : token)
    {
        if (character < '0' || character > '9')
        {
            throw std::runtime_error("invalid decimal value");
        }
        value = value * 10 + (character - '0');
    }
    return value;
}

struct VectorLine
{
    std::vector<std::uint32_t> codepoints;
    int direction = 0;
    int paragraphLevel = 0;
    std::vector<int> levels; // -1 marks 'x' (removed by X9)
    std::vector<std::size_t> order;
};

auto parseVector(const std::string &line) -> VectorLine
{
    const std::vector<std::string_view> fields = splitFields(line);
    if (fields.size() != 5)
    {
        throw std::runtime_error("expected five vector fields");
    }
    VectorLine vector;
    for (const std::string_view token : splitTokens(fields[0]))
    {
        vector.codepoints.push_back(parseHex(token));
    }
    vector.direction = parseDecimal(fields[1]);
    vector.paragraphLevel = parseDecimal(fields[2]);
    for (const std::string_view token : splitTokens(fields[3]))
    {
        vector.levels.push_back(token == "x" ? -1 : parseDecimal(token));
    }
    for (const std::string_view token : splitTokens(fields[4]))
    {
        vector.order.push_back(static_cast<std::size_t>(parseDecimal(token)));
    }
    if (vector.levels.size() != vector.codepoints.size())
    {
        throw std::runtime_error("level count does not match code point count");
    }
    return vector;
}

[[noreturn]] auto failVector(std::size_t index, const std::string &message) -> void
{
    std::ostringstream stream;
    stream << "vector " << index << ": " << message << "\n       " << bidi_test_vectors::kLines[index];
    ::test::fail(__FILE__, __LINE__, stream.str());
}

auto expectedVisual(const VectorLine &vector) -> std::string
{
    std::string visual;
    for (const std::size_t index : vector.order)
    {
        if (index >= vector.codepoints.size())
        {
            throw std::runtime_error("visual order index out of range");
        }
        testAppendUtf8(visual, vector.codepoints[index]);
    }
    return visual;
}

auto expectLevels(std::string_view text, const std::vector<int> &expected,
                  int paragraphLevel = -1) -> void
{
    const std::vector<int> actual = bunch::bidiLevels(text, paragraphLevel);
    CHECK_EQ(actual.size(), expected.size());
    for (std::size_t index = 0; index < expected.size(); ++index)
    {
        if (actual[index] != expected[index])
        {
            std::ostringstream stream;
            stream << "level " << index << ": expected " << expected[index] << ", got "
                   << actual[index];
            ::test::fail(__FILE__, __LINE__, stream.str());
        }
    }
}
} // namespace

// ---------------------------------------------------------------------------
// Conformance: stratified sample of BidiCharacterTest-15.0.0.txt
// ---------------------------------------------------------------------------

TEST_CASE("BidiCharacterTest sample matches UAX #9")
{
    using namespace bidi_test_vectors;

    CHECK(kLineCount > 15000);
    CHECK(kLineCount < 20000);
    CHECK(kSignatureCount == kTotalSignatureCount);
    CHECK(kPureLtrCount > 1000);
    CHECK(kPureRtlCount > 1000);
    CHECK(kNumbersCount > 100);
    CHECK(kNeutralsCount > 1000);
    CHECK(kBracketsCount > 1000);
    CHECK(kExplicitCount > 50);
    CHECK(kIsolatesCount > 30);
    CHECK(kAutoCount > 20);
    CHECK(kNsmCount > 10);

    for (std::size_t vectorIndex = 0; vectorIndex < kLineCount; ++vectorIndex)
    {
        const VectorLine vector = parseVector(kLines[vectorIndex]);
        const std::string text = encode(vector.codepoints);
        const int requested = vector.direction == 2 ? -1 : vector.direction;

        const BidiAnalysis analysis = bunch::bidiAnalyse(text, requested);
        if (analysis.paragraphLevel != vector.paragraphLevel)
        {
            failVector(vectorIndex, "paragraph level: expected " +
                                        std::to_string(vector.paragraphLevel) + ", got " +
                                        std::to_string(analysis.paragraphLevel));
        }
        if (vector.direction == 2 && bunch::bidiParagraphLevel(text) != vector.paragraphLevel)
        {
            failVector(vectorIndex, "P2/P3 detection: expected " +
                                        std::to_string(vector.paragraphLevel) + ", got " +
                                        std::to_string(bunch::bidiParagraphLevel(text)));
        }

        const std::vector<int> levels = bunch::bidiLevels(text, requested);
        if (levels != analysis.levels)
        {
            failVector(vectorIndex, "bidiLevels disagrees with bidiAnalyse");
        }

        for (std::size_t index = 0; index < vector.codepoints.size(); ++index)
        {
            const bool expectedRemoved = vector.levels[index] < 0;
            if (analysis.removed[index] != expectedRemoved)
            {
                failVector(vectorIndex, "removal flag at " + std::to_string(index) +
                                            ": expected " + (expectedRemoved ? "x" : "kept"));
            }
            if (!expectedRemoved && analysis.levels[index] != vector.levels[index])
            {
                failVector(vectorIndex, "level " + std::to_string(index) + ": expected " +
                                            std::to_string(vector.levels[index]) + ", got " +
                                            std::to_string(analysis.levels[index]));
            }
        }

        const std::string reordered = bunch::bidiReorder(text, requested);
        const std::string expected = expectedVisual(vector);
        if (reordered != expected)
        {
            failVector(vectorIndex, "visual order mismatch: expected " + expected + ", got " +
                                        reordered);
        }
    }
}

// ---------------------------------------------------------------------------
// P2/P3 and paragraph direction
// ---------------------------------------------------------------------------

TEST_CASE("bidiParagraphLevel follows P2/P3")
{
    CHECK_EQ(bunch::bidiParagraphLevel(""), 0);
    CHECK_EQ(bunch::bidiParagraphLevel("hello"), 0);
    CHECK_EQ(bunch::bidiParagraphLevel(u8"\u05D0\u05D1\u05D2"), 1);
    CHECK_EQ(bunch::bidiParagraphLevel(u8"\u0627\u0628\u062C"), 1);
    CHECK_EQ(bunch::bidiParagraphLevel("123 456"), 0);
    CHECK_EQ(bunch::bidiParagraphLevel(u8"123 \u05D0"), 1);
    CHECK_EQ(bunch::bidiParagraphLevel(u8"\u2067\u05D0\u05D1\u2069"), 0);
    CHECK_EQ(bunch::bidiParagraphLevel(u8"\u2067\u05D0\u05D1"), 0);
    CHECK_EQ(bunch::bidiParagraphLevel(u8"a\u2067\u05D0\u05D1\u2069"), 0);
    CHECK_EQ(bunch::bidiParagraphLevel(u8"\u2067\u05D0\u2069\u05D1"), 1);
    CHECK_EQ(bunch::bidiParagraphLevel(u8"\u2068\u05D0\u05D1\u2069"), 0);
}

TEST_CASE("automatic paragraph direction")
{
    const BidiAnalysis rtl = bunch::bidiAnalyse(u8"\u05D0\u05D1", -1);
    CHECK_EQ(rtl.paragraphLevel, 1);
    CHECK_EQ(rtl.levels[0], 1);

    const BidiAnalysis ltr = bunch::bidiAnalyse(u8"\u05D0\u05D1", 0);
    CHECK_EQ(ltr.paragraphLevel, 0);
    CHECK_EQ(ltr.levels[0], 1);

    const BidiAnalysis forcedRtl = bunch::bidiAnalyse("abc", 1);
    CHECK_EQ(forcedRtl.paragraphLevel, 1);
    CHECK_EQ(forcedRtl.levels[0], 2);
    CHECK_EQ(bunch::bidiReorder("abc", 1), "abc");
}

// ---------------------------------------------------------------------------
// Basic directionality, numbers, neutrals
// ---------------------------------------------------------------------------

TEST_CASE("pure left-to-right text")
{
    const std::string text = "Hello, world!";
    const std::vector<int> levels = bunch::bidiLevels(text);
    CHECK_EQ(levels.size(), text.size());
    for (const int level : levels)
    {
        CHECK_EQ(level, 0);
    }
    CHECK_EQ(bunch::bidiReorder(text), text);
}

TEST_CASE("pure right-to-left text")
{
    const std::string text = u8"\u05D0\u05D1\u05D2";
    expectLevels(text, {1, 1, 1});
    CHECK_EQ(bunch::bidiReorder(text), u8"\u05D2\u05D1\u05D0");
}

TEST_CASE("European and Arabic numbers")
{
    // W7: a European number after left-to-right text becomes L.
    expectLevels("abc 123", {0, 0, 0, 0, 0, 0, 0});
    CHECK_EQ(bunch::bidiReorder("abc 123"), "abc 123");

    // After a right-to-left run the EN stays a number and rises to level 2.
    const std::string hebrewNumbers = u8"\u05D0 123";
    expectLevels(hebrewNumbers, {1, 1, 2, 2, 2}, 0);
    CHECK_EQ(bunch::bidiReorder(hebrewNumbers, 0), u8"123 \u05D0");

    const std::string rtlNumbers = u8"\u05D0\u05D1\u05D2 123";
    expectLevels(rtlNumbers, {1, 1, 1, 1, 2, 2, 2});
    CHECK_EQ(bunch::bidiReorder(rtlNumbers), u8"123 \u05D2\u05D1\u05D0");

    const std::string arabic = u8"abc \u0661\u0662\u0663";
    expectLevels(arabic, {0, 0, 0, 0, 2, 2, 2});
    CHECK_EQ(bunch::bidiReorder(arabic), arabic);
}

TEST_CASE("neutral resolution")
{
    expectLevels("a, b", {0, 0, 0, 0});
    const std::string rtl = u8"\u05D0, \u05D1";
    expectLevels(rtl, {1, 1, 1, 1});
    CHECK_EQ(bunch::bidiReorder(rtl), u8"\u05D1 ,\u05D0");
    expectLevels("...", {0, 0, 0});
}

// ---------------------------------------------------------------------------
// N0: bracket pairs
// ---------------------------------------------------------------------------

TEST_CASE("bracket pairs resolve as a unit (N0)")
{
    // The enclosed "abc" matches the LTR embedding, so both brackets become L
    // even though the surrounding Hebrew is RTL.
    const std::string text = u8"\u05D0\u05D1\u05D2(abc)\u05D0\u05D1\u05D2";
    expectLevels(text, {1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1}, 0);
    CHECK_EQ(bunch::bidiReorder(text, 0), u8"\u05D2\u05D1\u05D0(abc)\u05D2\u05D1\u05D0");
}

TEST_CASE("bracket canonical equivalence")
{
    // U+2329/U+232A are canonically equivalent to U+3008/U+3009, so the pair
    // below must match.  Without matching, the ON brackets would resolve to R
    // between the surrounding Hebrew and reach level 1.
    const std::string text = u8"\u05D0\u05D1\u05D2\u2329abc\u3009\u05D0\u05D1\u05D2";
    expectLevels(text, {1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1}, 0);
}

TEST_CASE("NSM after a bracket follows the bracket")
{
    // Inside the pair is RTL while the embedding is LTR: context makes the
    // brackets R.  The combining mark directly after the closing bracket must
    // become R as well instead of falling back to the embedding direction.
    const std::string text = u8"\u05D0\u05D1\u05D2(\u05D0\u05D1\u05D2)\u0301a";
    expectLevels(text, {1, 1, 1, 1, 1, 1, 1, 1, 1, 0}, 0);
}

// ---------------------------------------------------------------------------
// X1-X9: explicit embeddings, overrides, and isolates
// ---------------------------------------------------------------------------

TEST_CASE("explicit embeddings and overrides")
{
    const std::string embedding = u8"a\u202B\u05D0\u05D1\u202Cb";
    const BidiAnalysis embeddingAnalysis = bunch::bidiAnalyse(embedding);
    CHECK(embeddingAnalysis.removed[1]);
    CHECK(embeddingAnalysis.removed[4]);
    CHECK_FALSE(embeddingAnalysis.removed[0]);
    expectLevels(embedding, {0, 0, 1, 1, 1, 0});
    CHECK_EQ(bunch::bidiReorder(embedding), u8"a\u05D1\u05D0b");

    const std::string overrideText = u8"a\u202Eabc\u202Cb";
    const BidiAnalysis overrideAnalysis = bunch::bidiAnalyse(overrideText);
    CHECK_EQ(overrideAnalysis.codepoints.size(), static_cast<std::size_t>(7));
    CHECK(overrideAnalysis.removed[1]);
    CHECK(overrideAnalysis.removed[5]);
    CHECK_EQ(overrideAnalysis.levels[0], 0);
    CHECK_EQ(overrideAnalysis.levels[2], 1);
    CHECK_EQ(overrideAnalysis.levels[3], 1);
    CHECK_EQ(overrideAnalysis.levels[4], 1);
    CHECK_EQ(overrideAnalysis.levels[6], 0);
    CHECK_EQ(bunch::bidiReorder(overrideText), "acbab");

    const std::string lro = u8"\u05D0\u202Dabc\u202C\u05D1";
    expectLevels(lro, {1, 1, 2, 2, 2, 2, 1});
    CHECK_EQ(bunch::bidiReorder(lro), u8"\u05D1abc\u05D0");
}

TEST_CASE("isolate initiators, PDI, and FSI")
{
    const std::string rli = u8"a\u2067\u05D0\u05D1\u2069b";
    expectLevels(rli, {0, 0, 1, 1, 0, 0});
    CHECK_EQ(bunch::bidiReorder(rli), u8"a\u2067\u05D1\u05D0\u2069b");

    const std::string fsiRtl = u8"a\u2068\u05D0\u05D1\u2069b";
    expectLevels(fsiRtl, {0, 0, 1, 1, 0, 0});
    CHECK_EQ(bunch::bidiReorder(fsiRtl), u8"a\u2068\u05D1\u05D0\u2069b");

    const std::string fsiLtr = u8"a\u2068bc\u2069d";
    expectLevels(fsiLtr, {0, 0, 2, 2, 0, 0});
    CHECK_EQ(bunch::bidiReorder(fsiLtr), fsiLtr);

    const std::string unmatched = u8"a\u2069b";
    expectLevels(unmatched, {0, 0, 0});
    CHECK_EQ(bunch::bidiReorder(unmatched), unmatched);
}

TEST_CASE("L1 resets trailing whitespace and isolates")
{
    const std::string embeddedSpace = u8"a\u202B \u202C";
    expectLevels(embeddedSpace, {0, 0, 0, 1});
    CHECK_EQ(bunch::bidiLevels(embeddedSpace)[2], 0);

    const std::string isolateSpace = u8"a\u2067 ";
    expectLevels(isolateSpace, {0, 0, 0});
}

// ---------------------------------------------------------------------------
// Robustness and byte offsets
// ---------------------------------------------------------------------------

TEST_CASE("invalid UTF-8 becomes U+FFFD")
{
    const std::string dangling(1, static_cast<char>(0xFF));
    const BidiAnalysis danglingAnalysis = bunch::bidiAnalyse(dangling);
    CHECK_EQ(danglingAnalysis.codepoints.size(), static_cast<std::size_t>(1));
    CHECK(danglingAnalysis.codepoints[0] == 0xFFFDu);
    CHECK_EQ(danglingAnalysis.levels.size(), static_cast<std::size_t>(1));
    CHECK_EQ(danglingAnalysis.levels[0], 0);
    CHECK_FALSE(danglingAnalysis.removed[0]);

    const std::string truncated = "\xE2\x82";
    CHECK_EQ(bunch::bidiAnalyse(truncated).codepoints.size(), static_cast<std::size_t>(2));

    const std::string overlong = "\xC0\xAF";
    CHECK_EQ(bunch::bidiAnalyse(overlong).codepoints.size(), static_cast<std::size_t>(2));

    const std::string surrogate = "\xED\xA0\x80";
    CHECK_EQ(bunch::bidiAnalyse(surrogate).codepoints.size(), static_cast<std::size_t>(3));

    const std::string emoji = "\xF0\x9F\x98\x80";
    const BidiAnalysis emojiAnalysis = bunch::bidiAnalyse(emoji);
    CHECK_EQ(emojiAnalysis.codepoints.size(), static_cast<std::size_t>(1));
    CHECK(emojiAnalysis.codepoints[0] == 0x1F600u);
    CHECK_EQ(emojiAnalysis.levels[0], 0);
    CHECK_EQ(bunch::bidiReorder(emoji), emoji);
}

TEST_CASE("byte offsets and level runs")
{
    const std::string text = u8"a\u05D0\u05D1b";
    const BidiAnalysis analysis = bunch::bidiAnalyse(text);
    CHECK_EQ(analysis.codepoints.size(), static_cast<std::size_t>(4));
    CHECK_EQ(analysis.byteOffsets.size(), static_cast<std::size_t>(5));
    CHECK_EQ(analysis.byteOffsets[0], static_cast<std::size_t>(0));
    CHECK_EQ(analysis.byteOffsets[1], static_cast<std::size_t>(1));
    CHECK_EQ(analysis.byteOffsets[2], static_cast<std::size_t>(3));
    CHECK_EQ(analysis.byteOffsets[3], static_cast<std::size_t>(5));
    CHECK_EQ(analysis.byteOffsets[4], static_cast<std::size_t>(6));

    const std::vector<BidiRun> runs = bunch::bidiRuns(text);
    CHECK_EQ(runs.size(), static_cast<std::size_t>(3));
    CHECK_EQ(runs[0].byteOffset, static_cast<std::size_t>(0));
    CHECK_EQ(runs[0].byteLength, static_cast<std::size_t>(1));
    CHECK_EQ(runs[0].level, 0);
    CHECK_EQ(runs[1].byteOffset, static_cast<std::size_t>(1));
    CHECK_EQ(runs[1].byteLength, static_cast<std::size_t>(4));
    CHECK_EQ(runs[1].level, 1);
    CHECK_EQ(runs[2].byteOffset, static_cast<std::size_t>(5));
    CHECK_EQ(runs[2].byteLength, static_cast<std::size_t>(1));
    CHECK_EQ(runs[2].level, 0);

    const std::string removed = u8"a\u202Bb";
    const std::vector<BidiRun> removedRuns = bunch::bidiRuns(removed);
    CHECK_EQ(removedRuns.size(), static_cast<std::size_t>(2));
    CHECK_EQ(removedRuns[0].byteLength, static_cast<std::size_t>(1));
    CHECK_EQ(removedRuns[1].byteOffset, static_cast<std::size_t>(4));
}

TEST_CASE("depth limit and overflow counters")
{
    std::string embeddings;
    for (int index = 0; index < 130; ++index)
    {
        embeddings += u8"\u202B";
    }
    embeddings += "a";
    for (int index = 0; index < 130; ++index)
    {
        embeddings += u8"\u202C";
    }
    embeddings += "b";
    const std::vector<int> levels = bunch::bidiLevels(embeddings);
    CHECK_EQ(levels.size(), static_cast<std::size_t>(262));
    CHECK_EQ(levels[130], 126);
    CHECK_EQ(levels[261], 0);
    for (const int level : levels)
    {
        CHECK(level >= 0);
        CHECK(level <= 126);
    }

    std::string isolates;
    for (int index = 0; index < 130; ++index)
    {
        isolates += u8"\u2067";
    }
    isolates += "a";
    for (int index = 0; index < 130; ++index)
    {
        isolates += u8"\u2069";
    }
    const std::vector<int> isolateLevels = bunch::bidiLevels(isolates);
    CHECK_EQ(isolateLevels.size(), static_cast<std::size_t>(261));
    CHECK_EQ(isolateLevels[130], 126);
    CHECK_EQ(isolateLevels[260], 0);

    std::string brackets;
    for (int index = 0; index < 70; ++index)
    {
        brackets += "(";
    }
    brackets += "a";
    for (int index = 0; index < 70; ++index)
    {
        brackets += ")";
    }
    CHECK_EQ(bunch::bidiAnalyse(brackets).levels.size(), static_cast<std::size_t>(141));
}

TEST_CASE("empty text")
{
    CHECK_EQ(bunch::bidiParagraphLevel(""), 0);
    CHECK(bunch::bidiLevels("").empty());
    CHECK(bunch::bidiReorder("").empty());
    CHECK(bunch::bidiRuns("").empty());
    CHECK_EQ(bunch::bidiMirror(""), std::string{});
    CHECK_EQ(bunch::bidiReorderWithMirroring(""), std::string{});

    const BidiAnalysis analysis = bunch::bidiAnalyse("");
    CHECK_EQ(analysis.paragraphLevel, 0);
    CHECK_EQ(analysis.byteOffsets.size(), static_cast<std::size_t>(1));
    CHECK_EQ(analysis.byteOffsets[0], static_cast<std::size_t>(0));
}

// ---------------------------------------------------------------------------
// L4: Bidi_Mirroring_Glyph
// ---------------------------------------------------------------------------

TEST_CASE("bidiMirror maps paired code points")
{
    CHECK_EQ(bunch::bidiMirror(0x0028u), 0x0029u); // U+0028/U+0029 parentheses
    CHECK_EQ(bunch::bidiMirror(0x0029u), 0x0028u);
    CHECK_EQ(bunch::bidiMirror(0x005Bu), 0x005Du); // U+005B/U+005D square brackets
    CHECK_EQ(bunch::bidiMirror(0x005Du), 0x005Bu);
    CHECK_EQ(bunch::bidiMirror(0x007Bu), 0x007Du); // U+007B/U+007D curly brackets
    CHECK_EQ(bunch::bidiMirror(0x007Du), 0x007Bu);
    CHECK_EQ(bunch::bidiMirror(0x003Cu), 0x003Eu); // U+003C/U+003E less/greater than
    CHECK_EQ(bunch::bidiMirror(0x003Eu), 0x003Cu);
    CHECK_EQ(bunch::bidiMirror(0x00ABu), 0x00BBu); // U+00AB/U+00BB guillemets
    CHECK_EQ(bunch::bidiMirror(0x00BBu), 0x00ABu);
    CHECK_EQ(bunch::bidiMirror(0x2039u), 0x203Au); // U+2039/U+203A single guillemets
    CHECK_EQ(bunch::bidiMirror(0x203Au), 0x2039u);
    CHECK_EQ(bunch::bidiMirror(0x2264u), 0x2265u); // U+2264/U+2265 less/greater-or-equal
    CHECK_EQ(bunch::bidiMirror(0x2265u), 0x2264u);
    CHECK_EQ(bunch::bidiMirror(0x2045u), 0x2046u); // U+2045/U+2046 square bracket with quill
    CHECK_EQ(bunch::bidiMirror(0x2046u), 0x2045u);
    CHECK_EQ(bunch::bidiMirror(0xFF08u), 0xFF09u); // U+FF08/U+FF09 fullwidth parentheses
    CHECK_EQ(bunch::bidiMirror(0xFF09u), 0xFF08u);

    // The bracket table also maps U+2329/U+232A and U+3008/U+3009, but the
    // mirror table keeps them as separate pairs.
    CHECK_EQ(bunch::bidiMirror(0x2329u), 0x232Au);
    CHECK_EQ(bunch::bidiMirror(0x3008u), 0x3009u);

    // Characters without a Bidi_Mirroring_Glyph mapping are unchanged.
    CHECK_EQ(bunch::bidiMirror(0x0000u), 0x0000u);
    CHECK_EQ(bunch::bidiMirror(0x0041u), 0x0041u);
    CHECK_EQ(bunch::bidiMirror(0x05D0u), 0x05D0u);
    CHECK_EQ(bunch::bidiMirror(0x0627u), 0x0627u);
    CHECK_EQ(bunch::bidiMirror(0x1F600u), 0x1F600u);
    CHECK_EQ(bunch::bidiMirror(0x10FFFFu), 0x10FFFFu);
}

TEST_CASE("bidiMirror maps UTF-8 text per code point")
{
    const std::string mixed =
        u8"a(b) \u05D0[\u05D1] \u0627{\u0628} \u2264 \uFF08\uFF09";
    const std::string expected =
        u8"a)b( \u05D0]\u05D1[ \u0627}\u0628{ \u2265 \uFF09\uFF08";
    CHECK_EQ(bunch::bidiMirror(mixed), expected);

    // Text without mirrored characters is returned byte for byte.
    CHECK_EQ(bunch::bidiMirror("Hello, world!"), "Hello, world!");
    CHECK_EQ(bunch::bidiMirror(u8"\u05D0\u05D1\u05D2"), u8"\u05D0\u05D1\u05D2");

    // The string overload decodes invalid bytes to U+FFFD, like bidiAnalyse.
    const std::string invalid(1, static_cast<char>(0xFF));
    CHECK_EQ(bunch::bidiMirror(invalid), "\xEF\xBF\xBD");
}

TEST_CASE("mirror table is sorted and reciprocal")
{
    using bunch::detail::bidi::kMirrorPairCount;
    using bunch::detail::bidi::kMirrorPairs;

    // Unicode 15.0.0 BidiMirroring.txt contains 428 mappings.
    CHECK_EQ(kMirrorPairCount, static_cast<std::size_t>(428));
    for (std::size_t index = 0; index < kMirrorPairCount; ++index)
    {
        const auto &pair = kMirrorPairs[index];
        CHECK(pair.codepoint != pair.mirrored);
        CHECK(pair.codepoint <= 0x10FFFFu);
        CHECK(pair.mirrored <= 0x10FFFFu);
        if (index > 0)
        {
            CHECK(kMirrorPairs[index - 1].codepoint < pair.codepoint);
        }
        CHECK_EQ(bunch::bidiMirror(pair.codepoint), pair.mirrored);
        CHECK_EQ(bunch::bidiMirror(pair.mirrored), pair.codepoint);
    }
}

TEST_CASE("bidiReorderWithMirroring applies L2 then L4")
{
    const std::string text = u8"\u05D0(\u05D1)\u05D2";

    // L2 reverses the right-to-left run; the parentheses are not yet mirrored.
    CHECK_EQ(bunch::bidiReorder(text), u8"\u05D2)\u05D1(\u05D0");

    // L4 then substitutes the mirrored parentheses.  For an all-RTL paragraph
    // this equals applying bidiMirror to the L2 output.
    CHECK_EQ(bunch::bidiMirror(bunch::bidiReorder(text)), u8"\u05D2(\u05D1)\u05D0");
    CHECK_EQ(bunch::bidiReorderWithMirroring(text), u8"\u05D2(\u05D1)\u05D0");
}

TEST_CASE("L4 mirrors only right-to-left characters")
{
    // The parentheses resolve to L in this LTR paragraph, so L4 leaves them
    // alone even though bidiMirror() on its own would swap them.  Only the
    // Hebrew run is reversed by L2.
    const std::string ltr = u8"abc (\u05D0\u05D1\u05D2)";
    const std::string reorderedLtr = u8"abc (\u05D2\u05D1\u05D0)";
    CHECK_EQ(bunch::bidiReorder(ltr), reorderedLtr);
    CHECK_EQ(bunch::bidiReorderWithMirroring(ltr), reorderedLtr);
    CHECK_EQ(bunch::bidiMirror(bunch::bidiReorder(ltr)), u8"abc )\u05D2\u05D1\u05D0(");

    // The parenthesis below sits in an even (LTR) embedding level inside an
    // RTL paragraph, so it is not mirrored either.
    const std::string embedded = u8"\u05D0\u202A(\u202C\u05D1";
    CHECK_EQ(bunch::bidiReorder(embedded), u8"\u05D1(\u05D0");
    CHECK_EQ(bunch::bidiReorderWithMirroring(embedded), u8"\u05D1(\u05D0");
}

// ---------------------------------------------------------------------------
// L3: combining marks are a shaping concern
// ---------------------------------------------------------------------------

TEST_CASE("L3 combining marks remain in L2 order")
{
    // UAX #9 L3 is applied by the shaping engine, not by this code point level
    // library.  L2 already puts the mark before its RTL base in visual order;
    // the library deliberately does not move it back after the base.
    const std::string text = u8"\u05D0\u05B0"; // alef + sheva
    expectLevels(text, {1, 1});
    CHECK_EQ(bunch::bidiReorder(text), u8"\u05B0\u05D0");
    CHECK_EQ(bunch::bidiReorderWithMirroring(text), u8"\u05B0\u05D0");
}
