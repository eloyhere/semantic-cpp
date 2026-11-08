#include "bunch/segment.h"

#include "segment_vectors.h"
#include "test_framework.h"

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
using BoundaryFunction = std::vector<std::size_t> (*)(std::string_view);

std::string describeText(std::string_view text)
{
    std::ostringstream stream;
    std::string_view remaining = text;
    stream << std::hex << std::uppercase << std::setfill('0');
    while (!remaining.empty())
    {
        const unsigned int codepoint = bunch::decode(remaining, bunch::charset::utf8);
        stream << "U+" << std::setw(4) << codepoint << ' ';
    }
    return stream.str();
}

std::string describeBoundaries(const std::vector<std::size_t> &boundaries)
{
    std::ostringstream stream;
    stream << '{';
    for (std::size_t index = 0; index < boundaries.size(); ++index)
    {
        if (index != 0)
        {
            stream << ", ";
        }
        stream << boundaries[index];
    }
    stream << '}';
    return stream.str();
}

// Compares one dataset from tests/segment_vectors.h case by case.  A failure
// names the upstream test line and prints the code points, so a mismatch is
// diagnosable without re-running the generator.
void checkVectorCases(const char *dataset, const segment_vectors::BreakCase *cases, std::size_t caseCount,
                      const char *textBlob, const std::uint16_t *breaks, BoundaryFunction function)
{
    CHECK(caseCount != 0);
    for (std::size_t index = 0; index < caseCount; ++index)
    {
        const segment_vectors::BreakCase &testCase = cases[index];
        const std::string_view text(textBlob + testCase.textOffset, testCase.textLength);
        const std::vector<std::size_t> expected(breaks + testCase.breakOffset,
                                                breaks + testCase.breakOffset + testCase.breakCount);
        const std::vector<std::size_t> actual = function(text);
        if (actual == expected)
        {
            continue;
        }

        std::ostringstream message;
        message << dataset << " case " << index << " (line " << testCase.sourceLine << ") failed for "
                << describeText(text) << "\n       expected " << describeBoundaries(expected) << "\n       actual   "
                << describeBoundaries(actual);
        ::test::fail(__FILE__, __LINE__, message.str());
    }
}

void checkBoundaryContract(std::string_view text, const std::vector<std::size_t> &boundaries)
{
    CHECK(!boundaries.empty());
    CHECK_EQ(boundaries.front(), std::size_t{0});
    CHECK_EQ(boundaries.back(), text.size());
    for (std::size_t index = 1; index < boundaries.size(); ++index)
    {
        CHECK(boundaries[index - 1] < boundaries[index]);
    }
}
} // namespace

TEST_CASE("segment: grapheme boundaries match GraphemeBreakTest 15.0.0")
{
    CHECK_EQ(std::size(segment_vectors::kGraphemeCases), std::size_t{602});
    checkVectorCases("grapheme", segment_vectors::kGraphemeCases, std::size(segment_vectors::kGraphemeCases),
                     segment_vectors::kGraphemeText, segment_vectors::kGraphemeBreaks,
                     &bunch::graphemeBoundaries);
}

TEST_CASE("segment: word boundaries match WordBreakTest 15.0.0")
{
    CHECK_EQ(std::size(segment_vectors::kWordCases), std::size_t{1823});
    checkVectorCases("word", segment_vectors::kWordCases, std::size(segment_vectors::kWordCases),
                     segment_vectors::kWordText, segment_vectors::kWordBreaks, &bunch::wordBoundaries);
}

TEST_CASE("segment: sentence boundaries match SentenceBreakTest 15.0.0")
{
    CHECK_EQ(std::size(segment_vectors::kSentenceCases), std::size_t{502});
    checkVectorCases("sentence", segment_vectors::kSentenceCases, std::size(segment_vectors::kSentenceCases),
                     segment_vectors::kSentenceText, segment_vectors::kSentenceBreaks, &bunch::sentenceBoundaries);
}

TEST_CASE("segment: boundary lists are well formed")
{
    const std::string samples[] = {
        "",
        "a",
        "plain ascii text",
        "\r\n\r\n",
        "a\xCC\x81" "b\xCC\x88",
        "\xF0\x9F\x87\xA8\xF0\x9F\x87\xB3\xF0\x9F\x87\xBA\xF0\x9F\x87\xB8",
        "\xE4\xB8\xAD\xE6\x96\x87\xE6\x96\x87\xE6\x9C\xAC",
        "\xE2\x80\x8B\xE2\x80\x8B",
        "\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6",
        "one\r\ntwo\nthree",
    };
    for (const std::string &sample : samples)
    {
        checkBoundaryContract(sample, bunch::graphemeBoundaries(sample));
        checkBoundaryContract(sample, bunch::wordBoundaries(sample));
        checkBoundaryContract(sample, bunch::sentenceBoundaries(sample));
    }

    CHECK(bunch::graphemeBoundaries("").size() == 1);
    CHECK(bunch::wordBoundaries("").size() == 1);
    CHECK(bunch::sentenceBoundaries("").size() == 1);
    CHECK_EQ(bunch::graphemeCount(""), std::size_t{0});
    CHECK(bunch::graphemeClusters("").empty());
}

TEST_CASE("segment: grapheme cluster helpers")
{
    CHECK_EQ(bunch::graphemeCount("abc"), std::size_t{3});
    CHECK_EQ(bunch::graphemeCount("e\xCC\x81"), std::size_t{1});
    CHECK_EQ(bunch::graphemeCount("a\xCC\x81" "b\xCC\x88"), std::size_t{2});
    CHECK_EQ(bunch::graphemeCount("a\r\nb"), std::size_t{3});

    // Regional indicators pair up; two flags are two clusters.
    CHECK_EQ(bunch::graphemeCount("\xF0\x9F\x87\xA8\xF0\x9F\x87\xB3"), std::size_t{1});
    CHECK_EQ(bunch::graphemeCount(
                 "\xF0\x9F\x87\xA8\xF0\x9F\x87\xB3\xF0\x9F\x87\xBA\xF0\x9F\x87\xB8"),
             std::size_t{2});

    // Emoji ZWJ sequences join through GB11, including chained sequences.
    CHECK_EQ(bunch::graphemeCount(
                 "\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6"),
             std::size_t{1});

    // Hangul jamo sequences are single clusters (GB6-GB8).
    CHECK_EQ(bunch::graphemeCount("\xE1\x84\x80\xE1\x85\xA1\xE1\x86\xA8"), std::size_t{1});

    // Unicode 15.0 has no GB9c: a Devanagari virama still breaks before the
    // next consonant.  This documents the 15.0.0 rule set.
    CHECK_EQ(bunch::graphemeCount("\xE0\xA4\x95\xE0\xA5\x8D\xE0\xA4\xB7"), std::size_t{2});

    const std::vector<std::string_view> clusters = bunch::graphemeClusters("a\xCC\x81" "b");
    CHECK_EQ(clusters.size(), std::size_t{2});
    CHECK_EQ(std::string(clusters[0]), std::string("a\xCC\x81"));
    CHECK_EQ(std::string(clusters[1]), std::string("b"));
}

TEST_CASE("segment: word boundaries")
{
    CHECK_EQ(bunch::wordBoundaries("Hello, world!"),
             (std::vector<std::size_t>{0, 5, 6, 7, 12, 13}));
    CHECK_EQ(bunch::wordBoundaries("can't"), (std::vector<std::size_t>{0, 5}));
    CHECK_EQ(bunch::wordBoundaries("3.14"), (std::vector<std::size_t>{0, 4}));
    CHECK_EQ(bunch::wordBoundaries("foo_bar"), (std::vector<std::size_t>{0, 7}));
    CHECK_EQ(bunch::wordBoundaries("\xE3\x82\xAB\xE3\x82\xBF\xE3\x82\xAB\xE3\x83\x8A"),
             (std::vector<std::size_t>{0, 12}));

    // WB15/WB16 pair regional indicators and break after every second one.
    CHECK_EQ(bunch::wordBoundaries(
                 "\xF0\x9F\x87\xA8\xF0\x9F\x87\xB3\xF0\x9F\x87\xBA\xF0\x9F\x87\xB8"),
             (std::vector<std::size_t>{0, 8, 16}));
}

TEST_CASE("segment: sentence boundaries")
{
    CHECK_EQ(bunch::sentenceBoundaries("Hello. World."), (std::vector<std::size_t>{0, 7, 13}));
    CHECK_EQ(bunch::sentenceBoundaries("One! Two? Three."), (std::vector<std::size_t>{0, 5, 10, 16}));
    CHECK_EQ(bunch::sentenceBoundaries("One. Two."), (std::vector<std::size_t>{0, 5, 9}));

    // SB8 keeps a sentence together when an abbreviation is followed by a
    // lower-case word; SB11 breaks after "Mr." because "Smith" is upper case.
    CHECK_EQ(bunch::sentenceBoundaries("etc. and more"), (std::vector<std::size_t>{0, 13}));
    CHECK_EQ(bunch::sentenceBoundaries("Mr. Smith"), (std::vector<std::size_t>{0, 4, 9}));
}

TEST_CASE("segment: malformed UTF-8 is replaced without reading past the end")
{
    CHECK_EQ(bunch::graphemeBoundaries("\x80"), (std::vector<std::size_t>{0, 1}));
    CHECK_EQ(bunch::graphemeBoundaries("\xE2\x82"), (std::vector<std::size_t>{0, 2}));
    CHECK_EQ(bunch::graphemeBoundaries("\xF0\x9F\x92"), (std::vector<std::size_t>{0, 3}));
    CHECK_EQ(bunch::graphemeBoundaries("\xED\xA0\x80"), (std::vector<std::size_t>{0, 3}));
    CHECK_EQ(bunch::graphemeBoundaries("a\xFF"), (std::vector<std::size_t>{0, 1, 2}));
    CHECK_EQ(bunch::graphemeCount("\xE2\x82"), std::size_t{1});

    const std::string malformed = "a\xFF" "b\x80" "c\xF0\x9F" "d";
    const std::vector<std::size_t> graphemes = bunch::graphemeBoundaries(malformed);
    checkBoundaryContract(malformed, graphemes);
    checkBoundaryContract(malformed, bunch::wordBoundaries(malformed));
    checkBoundaryContract(malformed, bunch::sentenceBoundaries(malformed));
}
