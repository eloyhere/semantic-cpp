#pragma once

// ===========================================================================
// bunch/segment_dictionary.h
//
// Dictionary-based word segmentation (UAX #29 section 4.1) for the
// Complex_Context scripts Thai, Lao, Khmer and Myanmar.
//
// UAX #29 leaves the SA (Complex_Context) scripts to a dictionary: the default
// word rules cannot find word boundaries inside a run of Thai letters, because
// every letter has Word_Break=Other, so bunch::wordBoundaries() returns one
// boundary per grapheme cluster instead of one per word.  This header adds the
// missing dictionary step:
//
//   1. The text is decoded with the same strict UTF-8 policy as segment.h.
//   2. It is split into runs of one and the same Complex_Context script.  A run
//      ends at any code point that is outside that script's Unicode blocks or
//      is Word_Break=Numeric, which includes the Thai U+0E50..U+0E59 digits and
//      their Lao, Khmer and Myanmar counterparts, so words never span scripts,
//      digits, spaces, punctuation or Latin text.
//   3. Inside a run the extended grapheme cluster boundaries are computed (so a
//      combining mark is never split from its base) and the segmenter greedily
//      picks the longest dictionary word that starts at the current cluster.
//      If no entry matches, one cluster becomes the segment, which keeps the
//      result well formed and total for unknown text.
//   4. Outside the runs the boundaries from bunch::wordBoundaries() are kept;
//      the boundaries inside a run are replaced by the dictionary boundaries.
//
// The algorithm is deterministic (greedy longest match, left to right) and
// runs in O(clusters * longest_entry * log(dictionary)) time, with the
// look-ahead bounded by WordDictionary::maxWordLength().
//
// Usage:
//
//     bunch::WordDictionary dictionary;
//     std::ifstream stream("thaidict.txt");          // one word per line
//     dictionary.load(stream);                       // appends, skips '#' lines
//     const std::vector<std::size_t> boundaries =
//         bunch::dictionaryWordBoundaries("ภาษาไทย", dictionary);
//
// The bundled sample dictionaries in segment_dictionary_data.h hold 710 Thai,
// 670 Lao, 620 Khmer and 570 Myanmar words (2,570 entries in total) and are
// selected by language tag:
//
//     bunch::dictionaryWordBoundaries("ภาษาไทย", "th");   // {"th", "lo", "km", "my"}
//
// A language tag with another primary subtag (for example "en") falls back to
// bunch::wordBoundaries().  The sample lists are small on purpose; production
// code should load the complete ICU break dictionaries at run time with
// WordDictionary::load(), which accepts their native format unchanged.
//
// Known limitations:
//   * No ambiguity resolution and no part-of-speech information.  Like every
//     maximal-matching segmenter this one cannot resolve genuinely ambiguous
//     strings; ICU's break iterators apply additional heuristics that this
//     implementation deliberately omits.
//   * Only Thai, Lao, Khmer and Myanmar are recognised as Complex_Context.
//     Other SA scripts (Tai Tham, Tai Viet, ...) keep the rule-based
//     boundaries, and Complex_Context handling is not locale-tailored.
//   * dictionaryWordBoundaries(text, language) pairs the whole text with the
//     one bundled dictionary of that language, so runs in another
//     Complex_Context script fall back to cluster segmentation; pass a
//     WordDictionary holding all the scripts you need to the other overload.
//   * An unmatched run falls back to one extended grapheme cluster per
//     segment rather than one code point, so combining marks always stay with
//     their base letter.
//   * The bundled dictionaries are deterministic samples, not frequency lists
//     and not the full ICU data (ICU ships no word frequencies).
//   * Words are matched byte-exactly: there is no normalisation, no case
//     folding and no dictionary word that ends in the middle of a grapheme
//     cluster can match.
// ===========================================================================

#include "segment.h"
#include "segment_dictionary_data.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <istream>
#include <iterator>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace bunch
{
namespace detail
{
namespace dictionary
{
// ---------------------------------------------------------------------------
// Complex_Context scripts
// ---------------------------------------------------------------------------

enum class Script : std::uint8_t
{
    none = 0,
    thai = 1,
    lao = 2,
    khmer = 3,
    myanmar = 4
};

// The four supported scripts.  The ranges also cover the Khmer Symbols block
// and the Myanmar Extended-A/B blocks, which contain letters and marks.  Thai
// and Lao have no extension blocks in Unicode 15.0.
struct ScriptRange
{
    std::uint32_t first;
    std::uint32_t last;
    Script script;
};

inline constexpr ScriptRange kScriptRanges[] = {
    {0x0E00, 0x0E7F, Script::thai},    {0x0E80, 0x0EFF, Script::lao},
    {0x1000, 0x109F, Script::myanmar}, {0x1780, 0x17FF, Script::khmer},
    {0x19E0, 0x19FF, Script::khmer},   {0xA9E0, 0xA9FF, Script::myanmar},
    {0xAA60, 0xAA7F, Script::myanmar},
};

// Returns the Complex_Context script of `codepoint`, or Script::none.
// Digits are reported as none even though they sit inside the script blocks:
// Word_Break=Numeric makes them behave like any other digit for segmentation.
inline auto scriptOf(std::uint32_t codepoint) -> Script
{
    for (const ScriptRange &range : kScriptRanges)
    {
        if (codepoint < range.first || codepoint > range.last)
        {
            continue;
        }
        if (segment::wordBreak(codepoint) == segment::WordBreak::numeric)
        {
            return Script::none;
        }
        return range.script;
    }
    return Script::none;
}

// Number of code points in a UTF-8 string, counted with the same decoder as
// segment.h so that ill-formed bytes count as one replacement character.
inline auto codepointCount(std::string_view text) -> std::size_t
{
    std::size_t count = 0;
    std::string_view remaining = text;
    while (!remaining.empty())
    {
        (void)decode(remaining, charset::utf8);
        ++count;
    }
    return count;
}

inline auto asciiLower(char character) -> char
{
    return character >= 'A' && character <= 'Z' ? static_cast<char>(character - 'A' + 'a') : character;
}

// Maps a BCP 47 language tag to a bundled dictionary.  Only the primary subtag
// matters, so "th", "th-TH" and "th_TH" all select Thai; any other language
// returns Script::none.
inline auto languageScript(std::string_view language) -> Script
{
    std::size_t length = 0;
    while (length < language.size() && language[length] != '-' && language[length] != '_')
    {
        ++length;
    }
    if (length != 2)
    {
        return Script::none;
    }
    const char first = asciiLower(language[0]);
    const char second = asciiLower(language[1]);
    if (first == 't' && second == 'h')
    {
        return Script::thai;
    }
    if (first == 'l' && second == 'o')
    {
        return Script::lao;
    }
    if (first == 'k' && second == 'm')
    {
        return Script::khmer;
    }
    if (first == 'm' && second == 'y')
    {
        return Script::myanmar;
    }
    return Script::none;
}

// A maximal run of code points that belong to one Complex_Context script.
struct Run
{
    std::size_t begin; // first code point index
    std::size_t end;   // one past the last code point index
    Script script;
};

inline auto findRuns(const std::vector<std::uint32_t> &codepoints) -> std::vector<Run>
{
    std::vector<Run> runs;
    std::size_t index = 0;
    while (index < codepoints.size())
    {
        const Script script = scriptOf(codepoints[index]);
        if (script == Script::none)
        {
            ++index;
            continue;
        }
        std::size_t end = index + 1;
        while (end < codepoints.size() && scriptOf(codepoints[end]) == script)
        {
            ++end;
        }
        runs.push_back(Run{index, end, script});
        index = end;
    }
    return runs;
}
} // namespace dictionary
} // namespace detail

// ---------------------------------------------------------------------------
// WordDictionary
// ---------------------------------------------------------------------------

// A word set for dictionaryWordBoundaries().  Words are UTF-8 byte strings and
// are stored exactly as added; adding a word twice is a no-op.  One filled
// dictionary may be shared between threads: the queries are const, only add()
// and load() mutate.
class WordDictionary
{
  public:
    // Adds one word.  Empty words are ignored.
    void add(std::string_view word);

    // Appends the words of `input`, one per line, skipping blank lines and
    // lines whose first non-blank character is '#'.  This is exactly the ICU
    // break-dictionary format, so thaidict.txt, laodict.txt, khmerdict.txt and
    // burmesedict.txt can be loaded unchanged.  CR-LF line endings and a
    // leading UTF-8 byte order mark are accepted, and entries already in the
    // dictionary are kept.
    void load(std::istream &input);

    // True when this exact UTF-8 byte sequence was added.
    auto contains(std::string_view word) const -> bool;

    // Number of distinct words.
    auto size() const -> std::size_t;

    // Length in code points of the longest word, or 0 when the dictionary is
    // empty.  The segmenter uses it to bound its look-ahead.
    auto maxWordLength() const -> std::size_t;

  private:
    std::set<std::string, std::less<>> words_;
    std::size_t maxWordLength_ = 0;
};

inline void WordDictionary::add(std::string_view word)
{
    if (word.empty())
    {
        return;
    }
    const std::size_t length = detail::dictionary::codepointCount(word);
    if (!words_.insert(std::string(word)).second)
    {
        return;
    }
    maxWordLength_ = std::max(maxWordLength_, length);
}

inline void WordDictionary::load(std::istream &input)
{
    std::string line;
    bool firstLine = true;
    while (std::getline(input, line))
    {
        std::string_view entry(line);
        if (firstLine)
        {
            // Tolerate a UTF-8 byte order mark written by Windows editors.
            firstLine = false;
            if (entry.size() >= 3 && entry.substr(0, 3) == "\xEF\xBB\xBF")
            {
                entry.remove_prefix(3);
            }
        }
        if (!entry.empty() && entry.back() == '\r')
        {
            entry.remove_suffix(1);
        }
        while (!entry.empty() && (entry.front() == ' ' || entry.front() == '\t'))
        {
            entry.remove_prefix(1);
        }
        while (!entry.empty() && (entry.back() == ' ' || entry.back() == '\t'))
        {
            entry.remove_suffix(1);
        }
        if (entry.empty() || entry.front() == '#')
        {
            continue;
        }
        add(entry);
    }
}

inline auto WordDictionary::contains(std::string_view word) const -> bool
{
    return words_.find(word) != words_.end();
}

inline auto WordDictionary::size() const -> std::size_t
{
    return words_.size();
}

inline auto WordDictionary::maxWordLength() const -> std::size_t
{
    return maxWordLength_;
}

namespace detail
{
namespace dictionary
{
// Builds a dictionary from a generated word table.
inline auto makeDictionary(const char *const *words, std::size_t count) -> WordDictionary
{
    WordDictionary dictionary;
    for (std::size_t index = 0; index < count; ++index)
    {
        dictionary.add(words[index]);
    }
    return dictionary;
}

// Returns the bundled sample dictionary for `script`, or nullptr when no
// bundled data exists.  The dictionaries are built on first use and are
// immutable afterwards (thread-safe function-local statics).
inline auto builtinDictionary(Script script) -> const WordDictionary *
{
    switch (script)
    {
    case Script::thai:
    {
        static const WordDictionary dictionary = makeDictionary(kThaiWords, std::size(kThaiWords));
        return &dictionary;
    }
    case Script::lao:
    {
        static const WordDictionary dictionary = makeDictionary(kLaoWords, std::size(kLaoWords));
        return &dictionary;
    }
    case Script::khmer:
    {
        static const WordDictionary dictionary = makeDictionary(kKhmerWords, std::size(kKhmerWords));
        return &dictionary;
    }
    case Script::myanmar:
    {
        static const WordDictionary dictionary = makeDictionary(kBurmeseWords, std::size(kBurmeseWords));
        return &dictionary;
    }
    case Script::none:
        break;
    }
    return nullptr;
}
} // namespace dictionary
} // namespace detail

// ---------------------------------------------------------------------------
// Dictionary segmentation
// ---------------------------------------------------------------------------

// Byte offsets (into `text`) at which dictionary words start or end.  The
// offsets are ascending and include 0 and text.size(); empty text yields {0}.
//
// Inside a run of Thai, Lao, Khmer or Myanmar code points the boundaries are
// the greedy longest-match dictionary boundaries; everywhere else the default
// UAX #29 word boundaries from bunch::wordBoundaries() are kept.  An empty
// dictionary therefore behaves exactly like bunch::wordBoundaries().
inline auto dictionaryWordBoundaries(std::string_view text, const WordDictionary &dictionary)
    -> std::vector<std::size_t>
{
    std::vector<std::size_t> boundaries = wordBoundaries(text);
    if (text.empty() || dictionary.size() == 0)
    {
        return boundaries;
    }

    const detail::segment::DecodedText decoded = detail::segment::decodeUtf8(text);
    const std::vector<detail::dictionary::Run> runs = detail::dictionary::findRuns(decoded.codepoints);
    if (runs.empty())
    {
        return boundaries;
    }

    // keep[offset] marks every byte offset that is a boundary in the result.
    std::vector<unsigned char> keep(text.size() + 1, 0);
    for (const std::size_t offset : boundaries)
    {
        keep[offset] = 1;
    }

    for (const detail::dictionary::Run &run : runs)
    {
        // Drop the rule-based boundaries strictly inside the run; they only
        // separate clusters, not words.
        for (std::size_t index = run.begin + 1; index < run.end; ++index)
        {
            keep[decoded.offsets[index]] = 0;
        }

        const std::size_t runStart = decoded.offsets[run.begin];
        const std::size_t runEnd =
            run.end == decoded.codepoints.size() ? text.size() : decoded.offsets[run.end];
        const std::string_view runText = text.substr(runStart, runEnd - runStart);
        const std::vector<std::size_t> clusters = graphemeBoundaries(runText);

        std::size_t position = 0;
        while (position + 1 < clusters.size())
        {
            // A match of n clusters uses at least n code points, so the
            // longest entry bounds the look-ahead.
            const std::size_t limit = std::min(clusters.size() - 1, position + dictionary.maxWordLength());
            std::size_t next = position + 1; // fallback: one cluster
            for (std::size_t candidate = limit; candidate > position + 1; --candidate)
            {
                const std::string_view word =
                    runText.substr(clusters[position], clusters[candidate] - clusters[position]);
                if (dictionary.contains(word))
                {
                    next = candidate;
                    break;
                }
            }
            keep[runStart + clusters[next]] = 1;
            position = next;
        }
    }

    boundaries.clear();
    boundaries.reserve(text.size() / 2 + 2);
    for (std::size_t offset = 0; offset <= text.size(); ++offset)
    {
        if (keep[offset] != 0)
        {
            boundaries.push_back(offset);
        }
    }
    return boundaries;
}

// Byte offsets from the bundled sample dictionary selected by `language`,
// whose primary subtag must be "th", "lo", "km" or "my" ("th-TH" also works).
// Any other language falls back to bunch::wordBoundaries().
inline auto dictionaryWordBoundaries(std::string_view text, std::string_view language)
    -> std::vector<std::size_t>
{
    const WordDictionary *dictionary =
        detail::dictionary::builtinDictionary(detail::dictionary::languageScript(language));
    if (dictionary == nullptr)
    {
        return wordBoundaries(text);
    }
    return dictionaryWordBoundaries(text, *dictionary);
}

// The segments between consecutive dictionaryWordBoundaries() offsets, each
// one a slice of `text`.  On text outside the Complex_Context scripts these
// are the UAX #29 words; inside the runs they are the matched dictionary
// words.  Empty text yields an empty list.
inline auto dictionaryWords(std::string_view text, const WordDictionary &dictionary)
    -> std::vector<std::string_view>
{
    const std::vector<std::size_t> boundaries = dictionaryWordBoundaries(text, dictionary);
    std::vector<std::string_view> words;
    if (boundaries.size() < 2)
    {
        return words;
    }
    words.reserve(boundaries.size() - 1);
    for (std::size_t index = 0; index + 1 < boundaries.size(); ++index)
    {
        words.push_back(text.substr(boundaries[index], boundaries[index + 1] - boundaries[index]));
    }
    return words;
}

// Same as above, using the bundled sample dictionary for `language`; other
// languages yield the UAX #29 words.
inline auto dictionaryWords(std::string_view text, std::string_view language) -> std::vector<std::string_view>
{
    const WordDictionary *dictionary =
        detail::dictionary::builtinDictionary(detail::dictionary::languageScript(language));
    static const WordDictionary empty;
    return dictionaryWords(text, dictionary != nullptr ? *dictionary : empty);
}
} // namespace bunch
