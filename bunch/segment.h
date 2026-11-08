#pragma once

// ===========================================================================
// bunch/segment.h
//
// UAX #29 text segmentation for Unicode 15.0.0: extended grapheme clusters,
// words and sentences.
//
// The input is UTF-8.  It is decoded into code points first (ill-formed byte
// sequences become U+FFFD without reading past the end of the string, using the
// same maximal-subpart policy as bunch::decode), and the default UAX #29 rules
// are then applied to every adjacent code point pair:
//
//   graphemes  GB1-GB13   (GB9c is a Unicode 15.1 addition and is not applied)
//   words      WB1-WB999  (including the WB4 ignore rule for Extend/Format/ZWJ)
//   sentences  SB1-SB999  (including the SB5 ignore rule and SB998 default)
//
// Boundaries are returned as byte offsets into the original UTF-8 string, in
// ascending order, always including 0 and text.size().  The tables in
// segment_data.h are generated from the Unicode 15.0.0 property files by
// tools/gen_segment_data.py and are searched with binary search.
//
// Known limitations:
//   * Only the default (non-tailored) rules are implemented.  UAX #29 permits
//     locale tailoring, and TR29 notes that dictionary-based segmentation
//     (Thai, Lao, Khmer, Myanmar, ...) is beyond the default algorithm; such
//     scripts therefore use the rule-based boundaries only.
//   * Unicode 15.1's GB9c (Indic conjunct clusters) is intentionally absent so
//     the implementation matches the Unicode 15.0.0 test files.
//   * The text is segmented as one paragraph/sentence stream; higher-level
//     paragraph separators are treated by the SB rules only.
//   * Decoding is strict UTF-8: overlong forms, surrogates and values above
//     U+10FFFF are replaced by U+FFFD and are never treated as valid text.
// ===========================================================================

#include "charset.h"
#include "encode.h"
#include "segment_data.h"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

namespace bunch
{
namespace detail
{
namespace segment
{
// ---------------------------------------------------------------------------
// Decoding
// ---------------------------------------------------------------------------

struct DecodedText
{
    std::vector<std::uint32_t> codepoints;
    std::vector<std::size_t> offsets; // byte offset of each code point
};

inline auto decodeUtf8(std::string_view text) -> DecodedText
{
    DecodedText decoded;
    decoded.codepoints.reserve(text.size());
    decoded.offsets.reserve(text.size());
    std::string_view remaining = text;
    while (!remaining.empty())
    {
        decoded.offsets.push_back(text.size() - remaining.size());
        decoded.codepoints.push_back(decode(remaining, charset::utf8));
    }
    return decoded;
}

inline auto isGraphemeControl(GraphemeBreak value) -> bool
{
    return value == GraphemeBreak::control || value == GraphemeBreak::cr || value == GraphemeBreak::lf;
}

inline auto isHangulL(GraphemeBreak value) -> bool
{
    return value == GraphemeBreak::l;
}

// GB11: \p{Extended_Pictographic} Extend* ZWJ.  The ZWJ is at `position`.
inline auto hasPictographicBeforeZwj(const std::vector<GraphemeBreak> &properties,
                                     const std::vector<unsigned char> &pictographic,
                                     std::size_t position) -> bool
{
    std::size_t index = position;
    while (index > 0)
    {
        --index;
        if (properties[index] == GraphemeBreak::extend)
        {
            continue;
        }
        return pictographic[index] != 0;
    }
    return false;
}

// Returns true when there is a grapheme cluster boundary between index - 1 and
// index, where index is in [1, codepoints.size()).
inline auto isGraphemeBoundary(const std::vector<GraphemeBreak> &properties,
                               const std::vector<unsigned char> &pictographic,
                               const std::vector<std::size_t> &regionalRuns,
                               std::size_t index) -> bool
{
    const GraphemeBreak left = properties[index - 1];
    const GraphemeBreak right = properties[index];

    if (left == GraphemeBreak::cr && right == GraphemeBreak::lf)
    {
        return false; // GB3
    }
    if (isGraphemeControl(left))
    {
        return true; // GB4
    }
    if (isGraphemeControl(right))
    {
        return true; // GB5
    }
    if (isHangulL(left) &&
        (right == GraphemeBreak::l || right == GraphemeBreak::v || right == GraphemeBreak::lv ||
         right == GraphemeBreak::lvt))
    {
        return false; // GB6
    }
    if ((left == GraphemeBreak::lv || left == GraphemeBreak::v) &&
        (right == GraphemeBreak::v || right == GraphemeBreak::t))
    {
        return false; // GB7
    }
    if ((left == GraphemeBreak::lvt || left == GraphemeBreak::t) && right == GraphemeBreak::t)
    {
        return false; // GB8
    }
    if (right == GraphemeBreak::extend || right == GraphemeBreak::zwj)
    {
        return false; // GB9
    }
    if (right == GraphemeBreak::spacingMark)
    {
        return false; // GB9a
    }
    if (left == GraphemeBreak::prepend)
    {
        return false; // GB9b
    }
    if (left == GraphemeBreak::zwj && pictographic[index] != 0 &&
        hasPictographicBeforeZwj(properties, pictographic, index - 1))
    {
        return false; // GB11
    }
    if (left == GraphemeBreak::regionalIndicator && right == GraphemeBreak::regionalIndicator)
    {
        return regionalRuns[index - 1] % 2 == 0; // GB12, GB13: no break after an odd run
    }
    return true; // GB999
}

// ---------------------------------------------------------------------------
// Word properties and helpers
// ---------------------------------------------------------------------------

inline auto isWordIgnore(WordBreak value) -> bool
{
    return value == WordBreak::extend || value == WordBreak::format || value == WordBreak::zwj;
}

inline auto isAhLetter(WordBreak value) -> bool
{
    return value == WordBreak::aLetter || value == WordBreak::hebrewLetter;
}

inline auto isMidLetterOrQuote(WordBreak value) -> bool
{
    return value == WordBreak::midLetter || value == WordBreak::midNumLet || value == WordBreak::singleQuote;
}

inline auto isMidNumOrQuote(WordBreak value) -> bool
{
    return value == WordBreak::midNum || value == WordBreak::midNumLet || value == WordBreak::singleQuote;
}

inline auto isWordLetterNumericOrKatakana(WordBreak value) -> bool
{
    return isAhLetter(value) || value == WordBreak::numeric || value == WordBreak::katakana;
}

inline auto wordPropertyAt(const std::vector<WordBreak> &properties, const std::vector<std::size_t> &filtered,
                           std::size_t rank) -> WordBreak
{
    return properties[filtered[rank]];
}

// Returns true when there is a word boundary between index - 1 and index.
// `filtered` lists the indices of the non-ignorable characters (WB4) and
// `rank` maps a code point index to its position in `filtered` (0 for ignored
// characters).
inline auto isWordBoundary(const std::vector<WordBreak> &properties,
                           const std::vector<unsigned char> &pictographic,
                           const std::vector<std::size_t> &filtered, const std::vector<std::size_t> &rank,
                           const std::vector<std::size_t> &regionalRuns, std::size_t index) -> bool
{
    const WordBreak left = properties[index - 1];
    const WordBreak right = properties[index];

    if (left == WordBreak::cr && right == WordBreak::lf)
    {
        return false; // WB3
    }
    if (left == WordBreak::newline || left == WordBreak::cr || left == WordBreak::lf)
    {
        return true; // WB3a
    }
    if (right == WordBreak::newline || right == WordBreak::cr || right == WordBreak::lf)
    {
        return true; // WB3b
    }
    if (left == WordBreak::zwj && pictographic[index] != 0)
    {
        return false; // WB3c
    }
    if (left == WordBreak::wSegSpace && right == WordBreak::wSegSpace)
    {
        return false; // WB3d
    }
    if (isWordIgnore(right))
    {
        return false; // WB4: ignore Extend/Format/ZWJ, except after sot and newlines (handled above)
    }

    const std::size_t rightRank = rank[index];
    if (rightRank == 0)
    {
        return true; // WB999: no left context at all
    }
    const std::size_t leftRank = rightRank - 1;
    const WordBreak beforeLeft = leftRank > 0 ? wordPropertyAt(properties, filtered, leftRank - 1) : WordBreak::other;
    const bool hasBeforeLeft = leftRank > 0;
    const WordBreak afterRight =
        rightRank + 1 < filtered.size() ? wordPropertyAt(properties, filtered, rightRank + 1) : WordBreak::other;
    const bool hasAfterRight = rightRank + 1 < filtered.size();
    const WordBreak leftValue = wordPropertyAt(properties, filtered, leftRank);

    if (isAhLetter(leftValue) && isAhLetter(right))
    {
        return false; // WB5
    }
    if (isAhLetter(leftValue) && isMidLetterOrQuote(right) && hasAfterRight && isAhLetter(afterRight))
    {
        return false; // WB6
    }
    if (isMidLetterOrQuote(leftValue) && isAhLetter(right) && hasBeforeLeft && isAhLetter(beforeLeft))
    {
        return false; // WB7
    }
    if (leftValue == WordBreak::hebrewLetter && right == WordBreak::singleQuote)
    {
        return false; // WB7a
    }
    if (leftValue == WordBreak::hebrewLetter && right == WordBreak::doubleQuote && hasAfterRight &&
        afterRight == WordBreak::hebrewLetter)
    {
        return false; // WB7b
    }
    if (leftValue == WordBreak::doubleQuote && right == WordBreak::hebrewLetter && hasBeforeLeft &&
        beforeLeft == WordBreak::hebrewLetter)
    {
        return false; // WB7c
    }
    if (leftValue == WordBreak::numeric && right == WordBreak::numeric)
    {
        return false; // WB8
    }
    if (isAhLetter(leftValue) && right == WordBreak::numeric)
    {
        return false; // WB9
    }
    if (leftValue == WordBreak::numeric && isAhLetter(right))
    {
        return false; // WB10
    }
    if (isMidNumOrQuote(leftValue) && right == WordBreak::numeric && hasBeforeLeft &&
        beforeLeft == WordBreak::numeric)
    {
        return false; // WB11
    }
    if (leftValue == WordBreak::numeric && isMidNumOrQuote(right) && hasAfterRight && afterRight == WordBreak::numeric)
    {
        return false; // WB12
    }
    if (leftValue == WordBreak::katakana && right == WordBreak::katakana)
    {
        return false; // WB13
    }
    if ((isWordLetterNumericOrKatakana(leftValue) || leftValue == WordBreak::extendNumLet) &&
        right == WordBreak::extendNumLet)
    {
        return false; // WB13a
    }
    if (leftValue == WordBreak::extendNumLet && isWordLetterNumericOrKatakana(right))
    {
        return false; // WB13b
    }
    if (leftValue == WordBreak::regionalIndicator && right == WordBreak::regionalIndicator)
    {
        return regionalRuns[rightRank - 1] % 2 == 0; // WB15, WB16
    }
    return true; // WB999
}

// ---------------------------------------------------------------------------
// Sentence properties and helpers
// ---------------------------------------------------------------------------

inline auto isSentenceIgnore(SentenceBreak value) -> bool
{
    return value == SentenceBreak::extend || value == SentenceBreak::format;
}

inline auto isSentenceParaSeparator(SentenceBreak value) -> bool
{
    return value == SentenceBreak::sep || value == SentenceBreak::cr || value == SentenceBreak::lf;
}

inline auto isSentenceTerminator(SentenceBreak value) -> bool
{
    return value == SentenceBreak::aTerm || value == SentenceBreak::sTerm;
}

inline auto sentencePropertyAt(const std::vector<SentenceBreak> &properties, const std::vector<std::size_t> &filtered,
                               std::size_t rank) -> SentenceBreak
{
    return properties[filtered[rank]];
}

// SATerm Close* Sp*, scanned backwards from `rank` (the last significant item
// before the boundary).  `terminator` selects ATerm for SB8 or SATerm for the
// other rules.
inline auto endsWithTerminatorCloseStarSpStar(const std::vector<SentenceBreak> &properties,
                                              const std::vector<std::size_t> &filtered, std::size_t rank,
                                              bool satTerm) -> bool
{
    std::size_t position = rank + 1;
    while (position > 0 && sentencePropertyAt(properties, filtered, position - 1) == SentenceBreak::sp)
    {
        --position;
    }
    while (position > 0 && sentencePropertyAt(properties, filtered, position - 1) == SentenceBreak::close)
    {
        --position;
    }
    if (position == 0)
    {
        return false;
    }
    const SentenceBreak value = sentencePropertyAt(properties, filtered, position - 1);
    return satTerm ? isSentenceTerminator(value) : value == SentenceBreak::aTerm;
}

// SATerm Close*, scanned backwards from `rank`.
inline auto endsWithTerminatorCloseStar(const std::vector<SentenceBreak> &properties,
                                        const std::vector<std::size_t> &filtered, std::size_t rank) -> bool
{
    std::size_t position = rank + 1;
    while (position > 0 && sentencePropertyAt(properties, filtered, position - 1) == SentenceBreak::close)
    {
        --position;
    }
    if (position == 0)
    {
        return false;
    }
    return isSentenceTerminator(sentencePropertyAt(properties, filtered, position - 1));
}

// SATerm Close* Sp* ParaSep?, scanned backwards from `rank`.
inline auto endsWithTerminatorCloseStarSpStarParaSeparator(const std::vector<SentenceBreak> &properties,
                                                           const std::vector<std::size_t> &filtered, std::size_t rank)
    -> bool
{
    std::size_t position = rank + 1;
    if (position > 0 && isSentenceParaSeparator(sentencePropertyAt(properties, filtered, position - 1)))
    {
        --position;
    }
    while (position > 0 && sentencePropertyAt(properties, filtered, position - 1) == SentenceBreak::sp)
    {
        --position;
    }
    while (position > 0 && sentencePropertyAt(properties, filtered, position - 1) == SentenceBreak::close)
    {
        --position;
    }
    if (position == 0)
    {
        return false;
    }
    return isSentenceTerminator(sentencePropertyAt(properties, filtered, position - 1));
}

// SB8 lookahead: ( not (OLetter | Upper | Lower | ParaSep | SATerm) )* Lower.
inline auto lowerFollows(const std::vector<SentenceBreak> &properties, const std::vector<std::size_t> &filtered,
                         std::size_t rank) -> bool
{
    for (std::size_t position = rank; position < filtered.size(); ++position)
    {
        const SentenceBreak value = sentencePropertyAt(properties, filtered, position);
        if (value == SentenceBreak::lower)
        {
            return true;
        }
        if (value == SentenceBreak::oLetter || value == SentenceBreak::upper || isSentenceParaSeparator(value) ||
            isSentenceTerminator(value))
        {
            return false;
        }
    }
    return false;
}

// Returns true when there is a sentence boundary between index - 1 and index.
inline auto isSentenceBoundary(const std::vector<SentenceBreak> &properties,
                               const std::vector<std::size_t> &filtered, const std::vector<std::size_t> &rank,
                               std::size_t index) -> bool
{
    const SentenceBreak left = properties[index - 1];
    const SentenceBreak right = properties[index];

    if (left == SentenceBreak::cr && right == SentenceBreak::lf)
    {
        return false; // SB3
    }
    if (isSentenceParaSeparator(left))
    {
        return true; // SB4
    }
    if (isSentenceIgnore(right))
    {
        return false; // SB5: ignore Extend/Format, except after sot/paragraph separators
    }

    const std::size_t rightRank = rank[index];
    if (rightRank == 0)
    {
        return false; // SB998: no left context
    }
    const std::size_t leftRank = rightRank - 1;
    const SentenceBreak leftValue = sentencePropertyAt(properties, filtered, leftRank);

    if (leftValue == SentenceBreak::aTerm && right == SentenceBreak::numeric)
    {
        return false; // SB6
    }
    if (leftValue == SentenceBreak::aTerm && right == SentenceBreak::upper && leftRank > 0)
    {
        const SentenceBreak beforeTerm = sentencePropertyAt(properties, filtered, leftRank - 1);
        if (beforeTerm == SentenceBreak::upper || beforeTerm == SentenceBreak::lower)
        {
            return false; // SB7
        }
    }
    if (endsWithTerminatorCloseStarSpStar(properties, filtered, leftRank, false) &&
        lowerFollows(properties, filtered, rightRank))
    {
        return false; // SB8
    }
    if (endsWithTerminatorCloseStarSpStar(properties, filtered, leftRank, true) &&
        (right == SentenceBreak::sContinue || isSentenceTerminator(right)))
    {
        return false; // SB8a
    }
    if (endsWithTerminatorCloseStar(properties, filtered, leftRank) &&
        (right == SentenceBreak::close || right == SentenceBreak::sp || isSentenceParaSeparator(right)))
    {
        return false; // SB9
    }
    if (endsWithTerminatorCloseStarSpStar(properties, filtered, leftRank, true) &&
        (right == SentenceBreak::sp || isSentenceParaSeparator(right)))
    {
        return false; // SB10
    }
    if (endsWithTerminatorCloseStarSpStarParaSeparator(properties, filtered, leftRank))
    {
        return true; // SB11
    }
    return false; // SB998
}

// Builds the significant-character list (WB4/SB5) and its inverse ranking.
// Ignored characters attach to the preceding character, matching the standard
// replacement form of the ignore rules.  `ignored` already accounts for the
// "except after sot / CR / LF / Sep / Newline" exception.
inline auto buildSignificant(const std::vector<unsigned char> &ignored)
    -> std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
{
    std::vector<std::size_t> filtered;
    filtered.reserve(ignored.size());
    std::vector<std::size_t> rank(ignored.size(), 0);
    for (std::size_t index = 0; index < ignored.size(); ++index)
    {
        if (ignored[index] != 0)
        {
            rank[index] = 0;
            continue;
        }
        rank[index] = filtered.size();
        filtered.push_back(index);
    }
    return {std::move(filtered), std::move(rank)};
}
} // namespace segment
} // namespace detail

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

// Byte offsets (into `text`) at which extended grapheme clusters start or end.
// Ascending, includes 0 and text.size(); empty text yields {0}.
inline auto graphemeBoundaries(std::string_view text) -> std::vector<std::size_t>
{
    using namespace detail::segment;

    const DecodedText decoded = decodeUtf8(text);
    std::vector<std::size_t> boundaries;
    boundaries.reserve(decoded.codepoints.size() + 1);
    boundaries.push_back(0); // GB1

    const std::size_t count = decoded.codepoints.size();
    if (count > 1)
    {
        std::vector<GraphemeBreak> properties;
        std::vector<unsigned char> pictographic;
        std::vector<std::size_t> regionalRuns;
        properties.reserve(count);
        pictographic.reserve(count);
        regionalRuns.reserve(count);
        std::size_t run = 0;
        for (std::size_t index = 0; index < count; ++index)
        {
            const GraphemeBreak value = graphemeBreak(decoded.codepoints[index]);
            properties.push_back(value);
            pictographic.push_back(isExtendedPictographic(decoded.codepoints[index]) ? 1 : 0);
            run = value == GraphemeBreak::regionalIndicator ? run + 1 : 0;
            regionalRuns.push_back(run);
        }
        for (std::size_t index = 1; index < count; ++index)
        {
            if (isGraphemeBoundary(properties, pictographic, regionalRuns, index))
            {
                boundaries.push_back(decoded.offsets[index]);
            }
        }
    }

    if (boundaries.back() != text.size())
    {
        boundaries.push_back(text.size()); // GB2
    }
    return boundaries;
}

// Number of extended grapheme clusters in `text` (0 for empty text).
inline auto graphemeCount(std::string_view text) -> std::size_t
{
    const std::vector<std::size_t> boundaries = graphemeBoundaries(text);
    return boundaries.empty() ? 0 : boundaries.size() - 1;
}

// Convenience view over the clusters: each element is a slice of `text`.
inline auto graphemeClusters(std::string_view text) -> std::vector<std::string_view>
{
    const std::vector<std::size_t> boundaries = graphemeBoundaries(text);
    std::vector<std::string_view> clusters;
    if (boundaries.size() < 2)
    {
        return clusters;
    }
    clusters.reserve(boundaries.size() - 1);
    for (std::size_t index = 0; index + 1 < boundaries.size(); ++index)
    {
        clusters.push_back(text.substr(boundaries[index], boundaries[index + 1] - boundaries[index]));
    }
    return clusters;
}

// Byte offsets at which words start or end, per the default UAX #29 rules.
// Ascending, includes 0 and text.size(); empty text yields {0}.
inline auto wordBoundaries(std::string_view text) -> std::vector<std::size_t>
{
    using namespace detail::segment;

    const DecodedText decoded = decodeUtf8(text);
    std::vector<std::size_t> boundaries;
    boundaries.reserve(decoded.codepoints.size() + 1);
    boundaries.push_back(0); // WB1

    const std::size_t count = decoded.codepoints.size();
    if (count > 1)
    {
        std::vector<WordBreak> properties;
        std::vector<unsigned char> pictographic;
        properties.reserve(count);
        pictographic.reserve(count);
        for (std::size_t index = 0; index < count; ++index)
        {
            properties.push_back(wordBreak(decoded.codepoints[index]));
            pictographic.push_back(isExtendedPictographic(decoded.codepoints[index]) ? 1 : 0);
        }
        std::vector<unsigned char> ignored(count, 0);
        for (std::size_t index = 0; index < count; ++index)
        {
            if (!isWordIgnore(properties[index]))
            {
                continue;
            }
            // WB4: ignore Format/Extend/ZWJ except after sot, CR, LF or Newline.
            const bool afterBreak = index == 0 || properties[index - 1] == WordBreak::cr ||
                                    properties[index - 1] == WordBreak::lf ||
                                    properties[index - 1] == WordBreak::newline;
            ignored[index] = afterBreak ? 0 : 1;
        }
        const auto significant = buildSignificant(ignored);
        const std::vector<std::size_t> &filtered = significant.first;
        const std::vector<std::size_t> &rank = significant.second;

        std::vector<std::size_t> regionalRuns(filtered.size(), 0);
        std::size_t run = 0;
        for (std::size_t position = 0; position < filtered.size(); ++position)
        {
            run = properties[filtered[position]] == WordBreak::regionalIndicator ? run + 1 : 0;
            regionalRuns[position] = run;
        }
        for (std::size_t index = 1; index < count; ++index)
        {
            if (isWordBoundary(properties, pictographic, filtered, rank, regionalRuns, index))
            {
                boundaries.push_back(decoded.offsets[index]);
            }
        }
    }

    if (boundaries.back() != text.size())
    {
        boundaries.push_back(text.size()); // WB2
    }
    return boundaries;
}

// Byte offsets at which sentences start or end, per the default UAX #29 rules.
// Ascending, includes 0 and text.size(); empty text yields {0}.
inline auto sentenceBoundaries(std::string_view text) -> std::vector<std::size_t>
{
    using namespace detail::segment;

    const DecodedText decoded = decodeUtf8(text);
    std::vector<std::size_t> boundaries;
    boundaries.reserve(decoded.codepoints.size() + 1);
    boundaries.push_back(0); // SB1

    const std::size_t count = decoded.codepoints.size();
    if (count > 1)
    {
        std::vector<SentenceBreak> properties;
        properties.reserve(count);
        for (std::size_t index = 0; index < count; ++index)
        {
            properties.push_back(sentenceBreak(decoded.codepoints[index]));
        }
        std::vector<unsigned char> ignored(count, 0);
        for (std::size_t index = 0; index < count; ++index)
        {
            if (!isSentenceIgnore(properties[index]))
            {
                continue;
            }
            // SB5: ignore Format/Extend except after sot, CR, LF or Sep.
            const bool afterBreak = index == 0 || isSentenceParaSeparator(properties[index - 1]);
            ignored[index] = afterBreak ? 0 : 1;
        }
        const auto significant = buildSignificant(ignored);
        const std::vector<std::size_t> &filtered = significant.first;
        const std::vector<std::size_t> &rank = significant.second;
        for (std::size_t index = 1; index < count; ++index)
        {
            if (isSentenceBoundary(properties, filtered, rank, index))
            {
                boundaries.push_back(decoded.offsets[index]);
            }
        }
    }

    if (boundaries.back() != text.size())
    {
        boundaries.push_back(text.size()); // SB2
    }
    return boundaries;
}
} // namespace bunch
