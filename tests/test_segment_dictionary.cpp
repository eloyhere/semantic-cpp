#include "bunch/segment_dictionary.h"

#include "test_framework.h"

#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
using Boundaries = std::vector<std::size_t>;
using Segments = std::vector<std::string>;

std::string describe(const Boundaries &values)
{
    std::ostringstream stream;
    stream << '{';
    for (std::size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
        {
            stream << ", ";
        }
        stream << values[index];
    }
    stream << '}';
    return stream.str();
}

std::string describe(const Segments &values)
{
    std::ostringstream stream;
    stream << '{';
    for (std::size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
        {
            stream << ", ";
        }
        stream << '[' << values[index] << ']';
    }
    stream << '}';
    return stream.str();
}

Segments segments(std::string_view text, std::string_view language)
{
    Segments result;
    for (std::string_view word : bunch::dictionaryWords(text, language))
    {
        result.emplace_back(word);
    }
    return result;
}

Segments segments(std::string_view text, const bunch::WordDictionary &dictionary)
{
    Segments result;
    for (std::string_view word : bunch::dictionaryWords(text, dictionary))
    {
        result.emplace_back(word);
    }
    return result;
}

void checkBoundaries(std::string_view text, std::string_view language, const Boundaries &expected)
{
    const Boundaries actual = bunch::dictionaryWordBoundaries(text, language);
    if (actual == expected)
    {
        return;
    }
    ::test::fail(__FILE__, __LINE__, "unexpected boundaries for \"" + std::string(text) + "\": got " +
                                         describe(actual) + ", expected " + describe(expected));
}

void checkBoundaries(std::string_view text, const bunch::WordDictionary &dictionary, const Boundaries &expected)
{
    const Boundaries actual = bunch::dictionaryWordBoundaries(text, dictionary);
    if (actual == expected)
    {
        return;
    }
    ::test::fail(__FILE__, __LINE__, "unexpected boundaries for \"" + std::string(text) + "\": got " +
                                         describe(actual) + ", expected " + describe(expected));
}

void checkSegments(std::string_view text, std::string_view language, const Segments &expected)
{
    const Segments actual = segments(text, language);
    if (actual == expected)
    {
        return;
    }
    ::test::fail(__FILE__, __LINE__,
                 "unexpected segments for \"" + std::string(text) + "\": got " + describe(actual) +
                     ", expected " + describe(expected));
}

void checkSegments(std::string_view text, const bunch::WordDictionary &dictionary, const Segments &expected)
{
    const Segments actual = segments(text, dictionary);
    if (actual == expected)
    {
        return;
    }
    ::test::fail(__FILE__, __LINE__,
                 "unexpected segments for \"" + std::string(text) + "\": got " + describe(actual) +
                     ", expected " + describe(expected));
}

// Boundary lists must be strictly ascending, start at 0 and end at text.size().
void checkContract(std::string_view text, const Boundaries &boundaries)
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

// The bundled Thai sample dictionary splits the language name into its two
// words; the base UAX #29 rules alone would break between every letter.
TEST_CASE("segment dictionary: bundled Thai dictionary")
{
    checkBoundaries("ภาษาไทย", "th", {0, 12, 21});
    checkSegments("ประเทศไทย", "th", {"ประเทศ", "ไทย"});
    checkSegments("คนดีมาก", "th", {"คน", "ดี", "มาก"});

    // The same words reached through an explicitly built dictionary.
    bunch::WordDictionary thai;
    thai.add("ภาษา");
    thai.add("ไทย");
    CHECK_EQ(bunch::dictionaryWordBoundaries("ภาษาไทย", thai), (Boundaries{0, 12, 21}));
    checkSegments("ภาษาไทย", thai, {"ภาษา", "ไทย"});
}

TEST_CASE("segment dictionary: bundled Lao dictionary")
{
    checkBoundaries("ພາສາລາວ", "lo", {0, 12, 21});
    checkSegments("ຄົນດີຫຼາຍ", "lo", {"ຄົນ", "ດີ", "ຫຼາຍ"});
    checkSegments("ພາສາລາວ 123", "lo", {"ພາສາ", "ລາວ", " ", "123"});
}

TEST_CASE("segment dictionary: bundled Khmer dictionary")
{
    // Khmer stacks consonants with U+17D2 COENG, so the matches end on
    // grapheme cluster boundaries: 12 bytes for 4 code points, 27 bytes total.
    checkBoundaries("ភាសាខ្មែរ", "km", {0, 12, 27});
    checkSegments("ប្រទេសខ្មែរ", "km", {"ប្រទេស", "ខ្មែរ"});
    checkSegments("ភាសា 123", "km", {"ភាសា", " ", "123"});
}

TEST_CASE("segment dictionary: bundled Myanmar dictionary")
{
    checkBoundaries("မြန်မာဘာသာ", "my", {0, 18, 30});
    checkSegments("ကျောင်းကြီး", "my", {"ကျောင်း", "ကြီး"});
    checkSegments("မြန်မာ 123", "my", {"မြန်မာ", " ", "123"});
}

TEST_CASE("segment dictionary: mixed scripts, Latin, digits and punctuation")
{
    checkSegments("ภาษาไทย 123 abc!", "th", {"ภาษา", "ไทย", " ", "123", " ", "abc", "!"});
    checkSegments("abcภาษาไทยxyz", "th", {"abc", "ภาษา", "ไทย", "xyz"});
    checkSegments("ภาษา ไทย", "th", {"ภาษา", " ", "ไทย"});
    checkSegments("ภาษา\nไทย", "th", {"ภาษา", "\n", "ไทย"});

    // Thai digits are Word_Break=Numeric, so they end the script run and are
    // grouped by the numeric rule instead of the dictionary.
    checkSegments("ภาษา๑๒๓", "th", {"ภาษา", "๑๒๓"});

    // A run never spans two Complex_Context scripts, so a dictionary holding
    // words of both scripts still segments them independently.
    bunch::WordDictionary asian;
    asian.add("ภาษา");
    asian.add("ไทย");
    asian.add("ພາສາ");
    asian.add("ລາວ");
    checkSegments("ภาษาไทยພາສາລາວ", asian, {"ภาษา", "ไทย", "ພາສາ", "ລາວ"});
    checkBoundaries("ภาษาไทยພາສາລາວ", asian, {0, 12, 21, 33, 42});
}

TEST_CASE("segment dictionary: empty text and unknown character sequences")
{
    checkBoundaries("", "th", {0});
    checkBoundaries("", "xx", {0});
    CHECK(bunch::dictionaryWords("", "th").empty());
    const bunch::WordDictionary empty;
    CHECK(bunch::dictionaryWords("", empty).empty());

    // No entry matches these runs, so each grapheme cluster becomes a segment
    // (one code point here) - the fallback is total, it never drops text.
    checkBoundaries("ฅฅฅ", "th", {0, 3, 6, 9});
    checkBoundaries("ຆຆຆ", "lo", {0, 3, 6, 9});
    checkBoundaries("ឣឣឣ", "km", {0, 3, 6, 9});
    checkBoundaries("ဣဣဣ", "my", {0, 3, 6, 9});
}

TEST_CASE("segment dictionary: language tags and fallback")
{
    checkBoundaries("ภาษาไทย", "TH", {0, 12, 21});
    checkBoundaries("ภาษาไทย", "th-TH", {0, 12, 21});
    checkBoundaries("ภาษาไทย", "th_TH", {0, 12, 21});

    // Unknown languages keep the rule-based UAX #29 boundaries.
    CHECK_EQ(bunch::dictionaryWordBoundaries("ภาษาไทย", "xx"), bunch::wordBoundaries("ภาษาไทย"));
    CHECK_EQ(bunch::dictionaryWordBoundaries("hello", "en"), bunch::wordBoundaries("hello"));
    CHECK_EQ(bunch::dictionaryWordBoundaries("hello", ""), bunch::wordBoundaries("hello"));
}

TEST_CASE("segment dictionary: explicit dictionary and maximal matching")
{
    // Longest match wins even when a shorter entry also matches at the same
    // position: "กกก" is preferred over "กก" in "กกกก", leaving one cluster.
    bunch::WordDictionary overlapping;
    overlapping.add("กก");
    overlapping.add("กกก");
    CHECK_EQ(bunch::dictionaryWordBoundaries("กกกก", overlapping), (Boundaries{0, 9, 12}));
    CHECK_EQ(bunch::dictionaryWordBoundaries("กกก", overlapping), (Boundaries{0, 9}));
    CHECK_EQ(bunch::dictionaryWordBoundaries("ก", overlapping), (Boundaries{0, 3}));

    // An empty dictionary is exactly bunch::wordBoundaries().
    const bunch::WordDictionary empty;
    CHECK_EQ(bunch::dictionaryWordBoundaries("ภาษาไทย", empty), bunch::wordBoundaries("ภาษาไทย"));
}

TEST_CASE("segment dictionary: WordDictionary container and load")
{
    bunch::WordDictionary dictionary;
    CHECK_EQ(dictionary.size(), std::size_t{0});
    CHECK_EQ(dictionary.maxWordLength(), std::size_t{0});
    CHECK_FALSE(dictionary.contains("abc"));

    dictionary.add("abc");
    dictionary.add("abcd");
    dictionary.add("abc"); // duplicate
    dictionary.add("");    // ignored
    CHECK_EQ(dictionary.size(), std::size_t{2});
    CHECK(dictionary.contains("abc"));
    CHECK(dictionary.contains("abcd"));
    CHECK_FALSE(dictionary.contains("ab"));
    CHECK_EQ(dictionary.maxWordLength(), std::size_t{4});

    // "abc"/"abcd" only exercise the container: dictionary segmentation is
    // limited to Complex_Context runs, so Latin text keeps its UAX #29
    // boundaries even with a matching entry (see the next test).
    CHECK_EQ(bunch::dictionaryWordBoundaries("abcd", dictionary), bunch::wordBoundaries("abcd"));

    // The ICU dictionary format: comments, blank lines, padding and CR-LF.
    std::istringstream input("# comment\r\n\r\n  abc  \r\nabcd\nabc\n\ttabbed\t\n");
    bunch::WordDictionary loaded;
    loaded.add("existing");
    loaded.load(input);
    CHECK_EQ(loaded.size(), std::size_t{4});
    CHECK(loaded.contains("existing"));
    CHECK(loaded.contains("abc"));
    CHECK(loaded.contains("abcd"));
    CHECK(loaded.contains("tabbed"));
    CHECK_FALSE(loaded.contains("# comment"));
    CHECK_EQ(loaded.maxWordLength(), std::size_t{8}); // "existing"

    // A UTF-8 byte order mark on the first line is ignored.
    std::istringstream withBom("\xEF\xBB\xBF" "abc\n");
    bunch::WordDictionary bomDictionary;
    bomDictionary.load(withBom);
    CHECK_EQ(bomDictionary.size(), std::size_t{1});
    CHECK(bomDictionary.contains("abc"));

    // Loading an empty stream changes nothing.
    std::istringstream emptyInput;
    loaded.load(emptyInput);
    CHECK_EQ(loaded.size(), std::size_t{4});
}

TEST_CASE("segment dictionary: cluster integrity and malformed UTF-8")
{
    // A dictionary entry that would end in the middle of a grapheme cluster
    // cannot match: the Thai tone mark stays with its base letter.
    bunch::WordDictionary marks;
    marks.add("ก");
    CHECK_EQ(bunch::dictionaryWordBoundaries("ก้", marks), (Boundaries{0, 6}));

    // Truncated UTF-8 becomes one U+FFFD and one segment, like segment.h.
    checkBoundaries("\xE0\xB8", "th", {0, 2});

    const std::string malformed = "a\xFF" "ภาษา\xF0\x9F" "d";
    checkContract(malformed, bunch::dictionaryWordBoundaries(malformed, "th"));
    checkContract(malformed, bunch::dictionaryWordBoundaries(malformed, "my"));
    const bunch::WordDictionary empty;
    checkContract(malformed, bunch::dictionaryWordBoundaries(malformed, empty));
}

TEST_CASE("segment dictionary: matches wordBoundaries outside Complex_Context")
{
    const std::string samples[] = {
        "Hello, world!", "can't", "3.14", "foo_bar", "abc", "カタカナ", "日本語のテキスト", "one\ntwo", "\r\n",
    };
    bunch::WordDictionary latin;
    latin.add("abc");
    latin.add("abcd");
    const bunch::WordDictionary empty;

    for (const std::string &sample : samples)
    {
        CHECK_EQ(bunch::dictionaryWordBoundaries(sample, "th"), bunch::wordBoundaries(sample));
        CHECK_EQ(bunch::dictionaryWordBoundaries(sample, "en"), bunch::wordBoundaries(sample));
        CHECK_EQ(bunch::dictionaryWordBoundaries(sample, latin), bunch::wordBoundaries(sample));
        CHECK_EQ(bunch::dictionaryWordBoundaries(sample, empty), bunch::wordBoundaries(sample));
    }
}

TEST_CASE("segment dictionary: boundary lists are well formed")
{
    const std::string samples[] = {
        "",
        "ภาษาไทย",
        "abcภาษาไทยxyz",
        "ພາສາລາວ 123",
        "ភាសាខ្មែរ",
        "မြန်မာဘာသာ",
        "ภาษา\nไทย",
        "\xE0\xB8",
        "a\xFF" "b",
    };
    const char *languages[] = {"th", "lo", "km", "my", "en"};

    for (const std::string &sample : samples)
    {
        for (const char *language : languages)
        {
            const Boundaries boundaries = bunch::dictionaryWordBoundaries(sample, language);
            checkContract(sample, boundaries);

            // The segments are consecutive slices, so they rebuild the input.
            std::string joined;
            std::size_t previous = 0;
            for (std::string_view word : bunch::dictionaryWords(sample, language))
            {
                joined.append(word);
                previous += word.size();
            }
            CHECK_EQ(joined, sample);
            CHECK_EQ(previous, sample.size());
        }
    }
}
