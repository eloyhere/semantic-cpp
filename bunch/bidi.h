#pragma once

// ===========================================================================
// bunch/bidi.h
//
// Unicode Bidirectional Algorithm (UAX #9, Unicode 15.0.0) for one paragraph
// of UTF-8 text.
//
// The implementation follows the rule order of UAX #9 section 3.3:
//   - P2/P3   paragraph level detection (isolates are skipped),
//   - X1-X8   explicit levels, overrides, and isolates (directional status
//             stack plus overflow isolate, overflow embedding, and valid
//             isolate counters; maximum explicit depth 125),
//   - X9      removal of embedding/override initiators, PDF, and BN,
//   - X10     level runs and isolating run sequences (BD13, sos/eos),
//   - W1-W7   weak type resolution,
//   - N0      bracket pair resolution (BD14-BD16, Bidi_Paired_Bracket),
//   - N1/N2   neutral and isolate formatting resolution,
//   - I1/I2   implicit levels,
//   - L1      reset of separators, trailing whitespace, and isolates,
//   - L2      reordering,
//   - L3      documented only: combining mark placement belongs to shaping,
//   - L4      mirroring through bidiMirror() and bidiReorderWithMirroring().
//
// The Bidi_Class, Bidi_Paired_Bracket, and Bidi_Mirroring_Glyph tables live in
// bidi_data.h and are generated from Unicode 15.0.0 by
// tools/gen_bidi_data.py.
//
// Known limitations:
//   - L3 is a rendering/glyph-stage rule.  After L2 a combining mark applied
//     to a right-to-left base precedes its base in visual order; if the
//     shaping engine expects marks after their base, the engine reverses the
//     pair (L3).  This library returns code point order only, so it performs
//     no L3 adjustment; a renderer on top of bidiRuns() remains responsible
//     for mark placement.
//   - L4 is approximated at the code point level.  UAX #9 asks a renderer to
//     select a mirrored glyph; bidiMirror() instead substitutes the
//     Bidi_Mirroring_Glyph code point (for example "(" becomes ")").  A
//     character with Bidi_Mirrored=Yes but no Bidi_Mirroring_Glyph mapping,
//     and any character without a mapping, is returned unchanged.
//     bidiReorderWithMirroring() applies the direction condition of L4: only
//     characters whose resolved level is odd are mirrored.
//   - The text is treated as one paragraph and one line.  A paragraph
//     separator (B) is assigned the paragraph level as X8 requires, but text
//     after it is not split into a new paragraph; pass one paragraph at a
//     time.  L1 therefore treats the end of the supplied text as the line
//     end.
//   - Characters removed by X9 (LRE/RLE/LRO/RLO/PDF/BN) are marked in
//     BidiAnalysis::removed and omitted from bidiReorder output, matching
//     BidiCharacterTest.  BidiAnalysis::levels keeps the embedding level that
//     was in effect at their position.
//   - Invalid UTF-8 bytes decode to U+FFFD, one replacement per invalid byte.
// ===========================================================================

#include "bidi_data.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace bunch
{
// ---------------------------------------------------------------------------
// Results
// ---------------------------------------------------------------------------

// One maximal run of characters at a single resolved level, in logical order.
// Characters removed by X9 are not part of any run; a run therefore maps to a
// contiguous byte range of the original text.
struct BidiRun
{
    std::size_t byteOffset = 0;
    std::size_t byteLength = 0;
    int level = 0;
};

// Complete result of analysing one paragraph.
//
// codepoints holds one entry per decoded code point.  byteOffsets holds
// codepoints.size() + 1 byte positions into the original text, the last one
// being text.size().  levels holds one resolved level per code point
// (including characters marked in removed); classes holds the original
// Bidi_Class values.
struct BidiAnalysis
{
    int paragraphLevel = 0;
    std::vector<std::uint32_t> codepoints;
    std::vector<std::size_t> byteOffsets;
    std::vector<BidiClass> classes;
    std::vector<int> levels;
    std::vector<bool> removed;
};

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------

// P2/P3: returns 0 for a left-to-right paragraph and 1 for a right-to-left
// paragraph.  Strong characters between an isolate initiator and its matching
// PDI are ignored.  An empty or neutral-only text yields 0.
auto bidiParagraphLevel(std::string_view text) -> int;

// Full analysis.  paragraphLevel < 0 detects the direction with P2/P3; a
// non-negative value overrides it and is clamped to [0, 125].
auto bidiAnalyse(std::string_view text, int paragraphLevel = -1) -> BidiAnalysis;

// Resolved level for every code point (after L1).  Characters removed by X9
// keep the explicit embedding level and are flagged by bidiAnalyse().removed.
auto bidiLevels(std::string_view text, int paragraphLevel = -1) -> std::vector<int>;

// Visual order (L2) encoded as UTF-8.  Characters removed by X9 are omitted,
// as required by X9 and by the conformance test vectors.
auto bidiReorder(std::string_view text, int paragraphLevel = -1) -> std::string;

// Logical level runs with byte offsets, suitable for a renderer.
auto bidiRuns(std::string_view text, int paragraphLevel = -1) -> std::vector<BidiRun>;

// L4: the Bidi_Mirroring_Glyph of a code point, or the code point itself when
// the character has no mirror mapping.
auto bidiMirror(std::uint32_t codepoint) -> std::uint32_t;

// L4: every code point of the text mapped through bidiMirror(uint32_t), UTF-8
// in and UTF-8 out.  The substitution is context free, so a mirrored character
// in a left-to-right run is mirrored too; this overload is the low-level
// building block.  Use bidiReorderWithMirroring() when the resolved direction
// must decide whether a character is mirrored.
auto bidiMirror(std::string_view utf8) -> std::string;

// L2 then L4: visual order where each character whose resolved level is odd
// (right-to-left direction) is replaced by its Bidi_Mirroring_Glyph.
// Characters in even-level runs, and characters without a mapping (including
// combining marks), are copied unchanged.  As with bidiReorder(), characters
// removed by X9 are omitted.  L3 is not applied; see the notes at the top of
// this file.
auto bidiReorderWithMirroring(std::string_view utf8, int paragraphLevel = -1) -> std::string;

namespace detail
{
namespace bidi
{
inline constexpr int kMaxDepth = 125;
inline constexpr std::size_t kBracketStackLimit = 63;
inline constexpr std::uint32_t kReplacementCharacter = 0xFFFD;

// ---------------------------------------------------------------------------
// Small predicates
// ---------------------------------------------------------------------------

constexpr auto isIsolateInitiator(BidiClass cls) -> bool
{
    return cls == BidiClass::lri || cls == BidiClass::rli || cls == BidiClass::fsi;
}

constexpr auto isRemovedByX9(BidiClass cls) -> bool
{
    return cls == BidiClass::rle || cls == BidiClass::lre || cls == BidiClass::rlo ||
           cls == BidiClass::lro || cls == BidiClass::pdf || cls == BidiClass::bn;
}

// Neutral or isolate formatting (NI) for N1/N2.
constexpr auto isNeutralOrIsolate(BidiClass cls) -> bool
{
    return cls == BidiClass::b || cls == BidiClass::s || cls == BidiClass::ws ||
           cls == BidiClass::on || cls == BidiClass::pdi || isIsolateInitiator(cls);
}

// In the scope of W2/N0/N1 an EN or AN acts as R.
constexpr auto numericAsRight(BidiClass cls) -> BidiClass
{
    return cls == BidiClass::en || cls == BidiClass::an ? BidiClass::r : cls;
}

constexpr auto directionOfLevel(int level) -> BidiClass
{
    return level % 2 == 0 ? BidiClass::l : BidiClass::r;
}

constexpr auto leastOddGreater(int level) -> int
{
    return (level + 1) | 1;
}

constexpr auto leastEvenGreater(int level) -> int
{
    return (level + 2) & ~1;
}

// Canonical equivalence for bracket matching (BD16): U+2329/U+232A are
// canonically equivalent to U+3008/U+3009.
constexpr auto canonicalBracket(std::uint32_t codepoint) -> std::uint32_t
{
    return codepoint == 0x2329 ? 0x3008 : (codepoint == 0x232A ? 0x3009 : codepoint);
}

// ---------------------------------------------------------------------------
// UTF-8
// ---------------------------------------------------------------------------

inline auto decodeUtf8(std::string_view text, std::vector<std::uint32_t> &codepoints,
                       std::vector<std::size_t> &byteOffsets) -> void
{
    const auto *bytes = reinterpret_cast<const unsigned char *>(text.data());
    const std::size_t size = text.size();
    codepoints.clear();
    byteOffsets.clear();
    codepoints.reserve(size);
    byteOffsets.reserve(size + 1);

    std::size_t index = 0;
    while (index < size)
    {
        byteOffsets.push_back(index);
        const unsigned char lead = bytes[index];
        if (lead < 0x80)
        {
            codepoints.push_back(lead);
            ++index;
            continue;
        }

        std::size_t length = 0;
        std::uint32_t value = 0;
        std::uint32_t minimum = 0;
        if (lead >= 0xC2 && lead <= 0xDF)
        {
            length = 2;
            value = lead & 0x1FU;
            minimum = 0x80;
        }
        else if (lead >= 0xE0 && lead <= 0xEF)
        {
            length = 3;
            value = lead & 0x0FU;
            minimum = 0x800;
        }
        else if (lead >= 0xF0 && lead <= 0xF4)
        {
            length = 4;
            value = lead & 0x07U;
            minimum = 0x10000;
        }

        bool valid = length != 0 && index + length <= size;
        for (std::size_t offset = 1; valid && offset < length; ++offset)
        {
            const unsigned char continuation = bytes[index + offset];
            if ((continuation & 0xC0U) != 0x80U)
            {
                valid = false;
                break;
            }
            value = (value << 6) | (continuation & 0x3FU);
        }

        if (!valid || value < minimum || value > 0x10FFFF ||
            (value >= 0xD800 && value <= 0xDFFF))
        {
            codepoints.push_back(kReplacementCharacter);
            ++index;
            continue;
        }

        codepoints.push_back(value);
        index += length;
    }
    byteOffsets.push_back(size);
}

inline auto appendUtf8(std::string &output, std::uint32_t codepoint) -> void
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

// ---------------------------------------------------------------------------
// P2/P3
// ---------------------------------------------------------------------------

// Applies P2 to classes[begin, end) and returns the resulting level (P3).
// Characters inside an isolate are skipped; the range is analysed as if it
// were a paragraph, which is what X5c needs for FSI.
inline auto detectParagraphLevel(const std::vector<BidiClass> &classes, std::size_t begin,
                                 std::size_t end) -> int
{
    std::size_t isolateDepth = 0;
    for (std::size_t index = begin; index < end; ++index)
    {
        const BidiClass cls = classes[index];
        if (isIsolateInitiator(cls))
        {
            ++isolateDepth;
            continue;
        }
        if (cls == BidiClass::pdi)
        {
            if (isolateDepth > 0)
            {
                --isolateDepth;
            }
            continue;
        }
        if (isolateDepth > 0)
        {
            continue;
        }
        if (cls == BidiClass::l)
        {
            return 0;
        }
        if (cls == BidiClass::r || cls == BidiClass::al)
        {
            return 1;
        }
    }
    return 0;
}

// BD9: matches isolate initiators with their PDIs.  Unmatched positions stay
// -1.  The result uses original Bidi_Class values, before any X6 override.
inline auto matchIsolates(const std::vector<BidiClass> &classes, std::vector<int> &matchingPdi) -> void
{
    matchingPdi.assign(classes.size(), -1);
    std::vector<std::size_t> stack;
    for (std::size_t index = 0; index < classes.size(); ++index)
    {
        if (isIsolateInitiator(classes[index]))
        {
            stack.push_back(index);
        }
        else if (classes[index] == BidiClass::pdi && !stack.empty())
        {
            matchingPdi[stack.back()] = static_cast<int>(index);
            stack.pop_back();
        }
    }
}

// ---------------------------------------------------------------------------
// X1-X8: explicit levels and directions
// ---------------------------------------------------------------------------

struct StatusEntry
{
    int level;
    BidiClass overrideStatus; // on means neutral
    bool isolate;
};

inline auto resolveExplicitLevels(const std::vector<BidiClass> &classes, int paragraphLevel,
                                  const std::vector<int> &matchingPdi,
                                  std::vector<BidiClass> &types, std::vector<int> &levels) -> void
{
    types = classes;
    levels.assign(classes.size(), paragraphLevel);

    std::vector<StatusEntry> stack;
    stack.push_back(StatusEntry{paragraphLevel, BidiClass::on, false});

    int overflowIsolate = 0;
    int overflowEmbedding = 0;
    int validIsolate = 0;

    for (std::size_t index = 0; index < classes.size(); ++index)
    {
        const BidiClass cls = classes[index];
        const int currentLevel = stack.back().level;

        switch (cls)
        {
        case BidiClass::rle:
        case BidiClass::lre:
        case BidiClass::rlo:
        case BidiClass::lro:
        {
            levels[index] = currentLevel;
            const bool odd = cls == BidiClass::rle || cls == BidiClass::rlo;
            const int candidate = odd ? leastOddGreater(currentLevel) : leastEvenGreater(currentLevel);
            if (candidate <= kMaxDepth && overflowIsolate == 0 && overflowEmbedding == 0)
            {
                BidiClass overrideStatus = BidiClass::on;
                if (cls == BidiClass::rlo)
                {
                    overrideStatus = BidiClass::r;
                }
                else if (cls == BidiClass::lro)
                {
                    overrideStatus = BidiClass::l;
                }
                stack.push_back(StatusEntry{candidate, overrideStatus, false});
            }
            else if (overflowIsolate == 0)
            {
                ++overflowEmbedding;
            }
            break;
        }
        case BidiClass::rli:
        case BidiClass::lri:
        case BidiClass::fsi:
        {
            levels[index] = currentLevel;
            BidiClass effective = cls;
            if (cls == BidiClass::fsi)
            {
                const int matching = matchingPdi[index];
                const std::size_t end =
                    matching >= 0 ? static_cast<std::size_t>(matching) : classes.size();
                effective = detectParagraphLevel(classes, index + 1, end) == 1 ? BidiClass::rli
                                                                               : BidiClass::lri;
            }
            if (stack.back().overrideStatus != BidiClass::on)
            {
                types[index] = stack.back().overrideStatus;
            }
            const int candidate = effective == BidiClass::rli ? leastOddGreater(currentLevel)
                                                              : leastEvenGreater(currentLevel);
            if (candidate <= kMaxDepth && overflowIsolate == 0 && overflowEmbedding == 0)
            {
                ++validIsolate;
                stack.push_back(StatusEntry{candidate, BidiClass::on, true});
            }
            else
            {
                ++overflowIsolate;
            }
            break;
        }
        case BidiClass::pdi:
        {
            if (overflowIsolate > 0)
            {
                --overflowIsolate;
            }
            else if (validIsolate > 0)
            {
                overflowEmbedding = 0;
                while (!stack.back().isolate)
                {
                    stack.pop_back();
                }
                stack.pop_back();
                --validIsolate;
            }
            levels[index] = stack.back().level;
            if (stack.back().overrideStatus != BidiClass::on)
            {
                types[index] = stack.back().overrideStatus;
            }
            break;
        }
        case BidiClass::pdf:
        {
            levels[index] = currentLevel;
            if (overflowIsolate > 0)
            {
                // The PDF lies inside an overflow isolate and is ignored.
            }
            else if (overflowEmbedding > 0)
            {
                --overflowEmbedding;
            }
            else if (!stack.back().isolate && stack.size() >= 2)
            {
                stack.pop_back();
            }
            break;
        }
        case BidiClass::b:
        {
            // X8: a paragraph separator belongs to no embedding.
            levels[index] = paragraphLevel;
            break;
        }
        case BidiClass::bn:
        {
            levels[index] = currentLevel;
            break;
        }
        default:
        {
            levels[index] = currentLevel;
            if (stack.back().overrideStatus != BidiClass::on)
            {
                types[index] = stack.back().overrideStatus;
            }
            break;
        }
        }
    }
}

// ---------------------------------------------------------------------------
// X10 and W1-W7, N0-N2, I1/I2 for one isolating run sequence
// ---------------------------------------------------------------------------

struct MatchedBracket
{
    std::size_t open;  // position within the sequence
    std::size_t close; // position within the sequence
};

inline auto resolveSequence(const std::vector<std::uint32_t> &codepoints,
                            const std::vector<BidiClass> &classes,
                            const std::vector<int> &baseLevels,
                            const std::vector<std::size_t> &kept,
                            const std::vector<int> &matchingPdi, int paragraphLevel,
                            std::vector<BidiClass> &types, std::vector<int> &levels,
                            const std::vector<std::size_t> &sequence) -> void
{
    const std::size_t first = sequence.front();
    const std::size_t last = sequence.back();

    // sos/eos use the higher of the two levels at the sequence boundary and
    // the explicit levels from X1-X8, never the implicit levels added below.
    const int embeddingLevel = baseLevels[kept[first]];
    const BidiClass embedding = directionOfLevel(embeddingLevel);

    const int precedingLevel = first == 0 ? paragraphLevel : baseLevels[kept[first - 1]];
    const BidiClass sos = directionOfLevel(std::max(baseLevels[kept[first]], precedingLevel));

    const bool unmatchedIsolate =
        isIsolateInitiator(classes[kept[last]]) && matchingPdi[kept[last]] < 0;
    int followingLevel = paragraphLevel;
    if (!unmatchedIsolate && last + 1 < kept.size())
    {
        followingLevel = baseLevels[kept[last + 1]];
    }
    const BidiClass eos = directionOfLevel(std::max(baseLevels[kept[last]], followingLevel));

    // W1
    BidiClass previous = sos;
    for (std::size_t position = 0; position < sequence.size(); ++position)
    {
        BidiClass &type = types[kept[sequence[position]]];
        if (type == BidiClass::nsm)
        {
            if (position == 0)
            {
                type = sos;
            }
            else if (previous == BidiClass::lri || previous == BidiClass::rli ||
                     previous == BidiClass::fsi || previous == BidiClass::pdi)
            {
                type = BidiClass::on;
            }
            else
            {
                type = previous;
            }
        }
        previous = type;
    }

    // W2
    BidiClass strong = sos;
    for (std::size_t position = 0; position < sequence.size(); ++position)
    {
        BidiClass &type = types[kept[sequence[position]]];
        if (type == BidiClass::en && strong == BidiClass::al)
        {
            type = BidiClass::an;
        }
        if (type == BidiClass::r || type == BidiClass::l || type == BidiClass::al)
        {
            strong = type;
        }
    }

    // W3
    for (std::size_t position = 0; position < sequence.size(); ++position)
    {
        BidiClass &type = types[kept[sequence[position]]];
        if (type == BidiClass::al)
        {
            type = BidiClass::r;
        }
    }

    // W4
    for (std::size_t position = 1; position + 1 < sequence.size(); ++position)
    {
        BidiClass &type = types[kept[sequence[position]]];
        const BidiClass before = types[kept[sequence[position - 1]]];
        const BidiClass after = types[kept[sequence[position + 1]]];
        if (type == BidiClass::es)
        {
            if (before == BidiClass::en && after == BidiClass::en)
            {
                type = BidiClass::en;
            }
        }
        else if (type == BidiClass::cs)
        {
            if (before == after && (before == BidiClass::en || before == BidiClass::an))
            {
                type = before;
            }
        }
    }

    // W5
    std::size_t position = 0;
    while (position < sequence.size())
    {
        if (types[kept[sequence[position]]] != BidiClass::et)
        {
            ++position;
            continue;
        }
        const std::size_t start = position;
        while (position < sequence.size() && types[kept[sequence[position]]] == BidiClass::et)
        {
            ++position;
        }
        const bool adjacentToNumber =
            (start > 0 && types[kept[sequence[start - 1]]] == BidiClass::en) ||
            (position < sequence.size() && types[kept[sequence[position]]] == BidiClass::en);
        if (adjacentToNumber)
        {
            for (std::size_t offset = start; offset < position; ++offset)
            {
                types[kept[sequence[offset]]] = BidiClass::en;
            }
        }
    }

    // W6
    for (std::size_t offset = 0; offset < sequence.size(); ++offset)
    {
        BidiClass &type = types[kept[sequence[offset]]];
        if (type == BidiClass::et || type == BidiClass::es || type == BidiClass::cs)
        {
            type = BidiClass::on;
        }
    }

    // W7
    strong = sos;
    for (std::size_t offset = 0; offset < sequence.size(); ++offset)
    {
        BidiClass &type = types[kept[sequence[offset]]];
        if (type == BidiClass::en && strong == BidiClass::l)
        {
            type = BidiClass::l;
        }
        if (type == BidiClass::r || type == BidiClass::l)
        {
            strong = type;
        }
    }

    // N0: identify bracket pairs (BD16) in sequence order, then process them
    // in order of their opening position, as N0 requires.
    std::vector<MatchedBracket> pairs;
    std::vector<std::pair<std::uint32_t, std::size_t>> bracketStack;
    for (std::size_t offset = 0; offset < sequence.size(); ++offset)
    {
        const std::size_t index = kept[sequence[offset]];
        if (types[index] != BidiClass::on)
        {
            continue;
        }
        const auto *bracket = lookupBracket(codepoints[index]);
        if (bracket == nullptr)
        {
            continue;
        }
        if (bracket->opening)
        {
            if (bracketStack.size() >= kBracketStackLimit)
            {
                break; // BD16: no room, stop processing this sequence.
            }
            bracketStack.emplace_back(bracket->paired, offset);
        }
        else
        {
            for (std::size_t depth = bracketStack.size(); depth > 0; --depth)
            {
                if (canonicalBracket(codepoints[index]) == canonicalBracket(bracketStack[depth - 1].first))
                {
                    pairs.push_back(MatchedBracket{bracketStack[depth - 1].second, offset});
                    bracketStack.resize(depth - 1);
                    break;
                }
            }
        }
    }
    std::stable_sort(pairs.begin(), pairs.end(),
                     [](const MatchedBracket &left, const MatchedBracket &right) {
                         return left.open < right.open;
                     });

    const auto setBracket = [&](std::size_t offset, BidiClass direction) {
        types[kept[sequence[offset]]] = direction;
        // N0, last paragraph: original NSMs directly after a bracket that
        // changed direction follow the bracket.
        for (std::size_t following = offset + 1; following < sequence.size(); ++following)
        {
            const std::size_t index = kept[sequence[following]];
            if (classes[index] != BidiClass::nsm)
            {
                break;
            }
            types[index] = direction;
        }
    };

    for (const MatchedBracket &pair : pairs)
    {
        bool matchesEmbedding = false;
        bool hasStrong = false;
        for (std::size_t offset = pair.open + 1; offset < pair.close; ++offset)
        {
            const BidiClass type = numericAsRight(types[kept[sequence[offset]]]);
            if (type == embedding)
            {
                matchesEmbedding = true;
                break;
            }
            if (type == BidiClass::l || type == BidiClass::r)
            {
                hasStrong = true;
            }
        }

        if (matchesEmbedding)
        {
            setBracket(pair.open, embedding);
            setBracket(pair.close, embedding);
        }
        else if (hasStrong)
        {
            // Every strong type inside is opposite the embedding direction;
            // established context decides between that direction and the
            // embedding direction.
            const BidiClass opposite = embedding == BidiClass::l ? BidiClass::r : BidiClass::l;
            BidiClass preceding = sos;
            for (std::size_t offset = pair.open; offset > 0; --offset)
            {
                const BidiClass type = numericAsRight(types[kept[sequence[offset - 1]]]);
                if (type == BidiClass::l || type == BidiClass::r)
                {
                    preceding = type;
                    break;
                }
            }
            const BidiClass resolved = preceding == opposite ? opposite : embedding;
            setBracket(pair.open, resolved);
            setBracket(pair.close, resolved);
        }
    }

    // N1
    position = 0;
    while (position < sequence.size())
    {
        if (!isNeutralOrIsolate(types[kept[sequence[position]]]))
        {
            ++position;
            continue;
        }
        const std::size_t start = position;
        while (position < sequence.size() && isNeutralOrIsolate(types[kept[sequence[position]]]))
        {
            ++position;
        }
        const BidiClass before =
            start == 0 ? sos : numericAsRight(types[kept[sequence[start - 1]]]);
        const BidiClass after =
            position == sequence.size() ? eos : numericAsRight(types[kept[sequence[position]]]);
        if (before == after)
        {
            for (std::size_t offset = start; offset < position; ++offset)
            {
                types[kept[sequence[offset]]] = before;
            }
        }
    }

    // N2
    for (std::size_t offset = 0; offset < sequence.size(); ++offset)
    {
        BidiClass &type = types[kept[sequence[offset]]];
        if (isNeutralOrIsolate(type))
        {
            type = embedding;
        }
    }

    // I1/I2
    for (std::size_t offset = 0; offset < sequence.size(); ++offset)
    {
        const std::size_t index = kept[sequence[offset]];
        const BidiClass type = types[index];
        if (levels[index] % 2 == 0)
        {
            if (type == BidiClass::r)
            {
                ++levels[index];
            }
            else if (type == BidiClass::en || type == BidiClass::an)
            {
                levels[index] += 2;
            }
        }
        else if (type == BidiClass::l || type == BidiClass::en || type == BidiClass::an)
        {
            ++levels[index];
        }
    }
}

// ---------------------------------------------------------------------------
// X10: level runs and isolating run sequences (BD13)
// ---------------------------------------------------------------------------

struct LevelRun
{
    std::size_t start; // first position in kept
    std::size_t end;   // one past the last position in kept
    int level;
};

inline auto resolveSequences(const std::vector<std::uint32_t> &codepoints,
                             const std::vector<BidiClass> &classes,
                             const std::vector<int> &matchingPdi,
                             const std::vector<std::size_t> &kept, std::vector<BidiClass> &types,
                             std::vector<int> &levels, int paragraphLevel) -> void
{
    // Snapshot the explicit levels: rules W-N and I must not influence the
    // level runs or the sos/eos values of other sequences.
    const std::vector<int> baseLevels = levels;

    std::vector<LevelRun> runs;
    for (std::size_t position = 0; position < kept.size(); ++position)
    {
        const int level = baseLevels[kept[position]];
        if (runs.empty() || runs.back().level != level)
        {
            runs.push_back(LevelRun{position, position + 1, level});
        }
        else
        {
            runs.back().end = position + 1;
        }
    }

    std::vector<int> runOfKept(kept.size(), -1);
    for (std::size_t run = 0; run < runs.size(); ++run)
    {
        for (std::size_t position = runs[run].start; position < runs[run].end; ++position)
        {
            runOfKept[position] = static_cast<int>(run);
        }
    }

    // BD13: link a level run whose last character is an isolate initiator with
    // the run that starts with its matching PDI.
    std::vector<int> nextRun(runs.size(), -1);
    std::vector<int> previousRun(runs.size(), -1);
    std::vector<int> keptIndexOf(classes.size(), -1);
    for (std::size_t position = 0; position < kept.size(); ++position)
    {
        keptIndexOf[kept[position]] = static_cast<int>(position);
    }

    for (std::size_t run = 0; run < runs.size(); ++run)
    {
        const std::size_t lastIndex = kept[runs[run].end - 1];
        if (!isIsolateInitiator(classes[lastIndex]))
        {
            continue;
        }
        const int matching = matchingPdi[lastIndex];
        if (matching < 0)
        {
            continue;
        }
        const int matchingKept = keptIndexOf[static_cast<std::size_t>(matching)];
        if (matchingKept < 0)
        {
            continue;
        }
        const std::size_t target = static_cast<std::size_t>(runOfKept[static_cast<std::size_t>(matchingKept)]);
        if (runs[target].start != static_cast<std::size_t>(matchingKept) ||
            runs[target].level != runs[run].level)
        {
            continue;
        }
        nextRun[run] = static_cast<int>(target);
        previousRun[target] = static_cast<int>(run);
    }

    for (std::size_t head = 0; head < runs.size(); ++head)
    {
        if (previousRun[head] != -1)
        {
            continue;
        }
        std::vector<std::size_t> sequence;
        for (int run = static_cast<int>(head); run != -1;
             run = nextRun[static_cast<std::size_t>(run)])
        {
            for (std::size_t position = runs[static_cast<std::size_t>(run)].start;
                 position < runs[static_cast<std::size_t>(run)].end; ++position)
            {
                sequence.push_back(position);
            }
        }
        resolveSequence(codepoints, classes, baseLevels, kept, matchingPdi, paragraphLevel, types,
                        levels, sequence);
    }
}

// ---------------------------------------------------------------------------
// L1 and L2
// ---------------------------------------------------------------------------

inline auto applyL1(const std::vector<std::size_t> &kept, const std::vector<BidiClass> &classes,
                    int paragraphLevel, std::vector<int> &levels) -> void
{
    for (std::size_t position = 0; position < kept.size(); ++position)
    {
        const BidiClass cls = classes[kept[position]];
        if (cls != BidiClass::b && cls != BidiClass::s)
        {
            continue;
        }
        levels[kept[position]] = paragraphLevel;
        // Whitespace and isolate formatting characters before the separator
        // are reset too (L1, clause 3).
        for (std::size_t previous = position; previous > 0;)
        {
            const BidiClass previousClass = classes[kept[previous - 1]];
            if (previousClass == BidiClass::ws || previousClass == BidiClass::pdi ||
                isIsolateInitiator(previousClass))
            {
                --previous;
                levels[kept[previous]] = paragraphLevel;
            }
            else
            {
                break;
            }
        }
    }

    // L1, clause 4: trailing whitespace and isolate formatting characters.
    for (std::size_t position = kept.size(); position > 0;)
    {
        const BidiClass cls = classes[kept[position - 1]];
        if (cls == BidiClass::ws || cls == BidiClass::pdi || isIsolateInitiator(cls))
        {
            --position;
            levels[kept[position]] = paragraphLevel;
        }
        else
        {
            break;
        }
    }
}

// Returns the visual order as positions into kept (L2).
inline auto reorderIndices(const std::vector<std::size_t> &kept, const std::vector<int> &levels)
    -> std::vector<std::size_t>
{
    std::vector<std::size_t> order(kept.size());
    for (std::size_t position = 0; position < order.size(); ++position)
    {
        order[position] = position;
    }

    int highest = 0;
    int lowestOdd = std::numeric_limits<int>::max();
    for (std::size_t position = 0; position < kept.size(); ++position)
    {
        const int level = levels[kept[position]];
        highest = std::max(highest, level);
        if (level % 2 != 0)
        {
            lowestOdd = std::min(lowestOdd, level);
        }
    }
    if (lowestOdd == std::numeric_limits<int>::max())
    {
        return order;
    }

    for (int level = highest; level >= lowestOdd; --level)
    {
        std::size_t position = 0;
        while (position < order.size())
        {
            if (levels[kept[order[position]]] < level)
            {
                ++position;
                continue;
            }
            const std::size_t start = position;
            while (position < order.size() && levels[kept[order[position]]] >= level)
            {
                ++position;
            }
            std::reverse(order.begin() + static_cast<std::ptrdiff_t>(start),
                         order.begin() + static_cast<std::ptrdiff_t>(position));
        }
    }
    return order;
}

} // namespace bidi
} // namespace detail

// ---------------------------------------------------------------------------
// Public entry points
// ---------------------------------------------------------------------------

inline auto bidiParagraphLevel(std::string_view text) -> int
{
    std::vector<std::uint32_t> codepoints;
    std::vector<std::size_t> byteOffsets;
    detail::bidi::decodeUtf8(text, codepoints, byteOffsets);

    std::vector<BidiClass> classes(codepoints.size());
    for (std::size_t index = 0; index < codepoints.size(); ++index)
    {
        classes[index] = detail::bidi::lookupClass(codepoints[index]);
    }
    return detail::bidi::detectParagraphLevel(classes, 0, classes.size());
}

inline auto bidiAnalyse(std::string_view text, int paragraphLevel) -> BidiAnalysis
{
    BidiAnalysis analysis;
    detail::bidi::decodeUtf8(text, analysis.codepoints, analysis.byteOffsets);

    const std::size_t count = analysis.codepoints.size();
    analysis.classes.resize(count);
    for (std::size_t index = 0; index < count; ++index)
    {
        analysis.classes[index] = detail::bidi::lookupClass(analysis.codepoints[index]);
    }

    std::vector<int> matchingPdi;
    detail::bidi::matchIsolates(analysis.classes, matchingPdi);

    int level = paragraphLevel;
    if (level < 0)
    {
        level = detail::bidi::detectParagraphLevel(analysis.classes, 0, count);
    }
    level = std::min(level, detail::bidi::kMaxDepth);
    analysis.paragraphLevel = level;

    std::vector<BidiClass> types;
    detail::bidi::resolveExplicitLevels(analysis.classes, level, matchingPdi, types,
                                        analysis.levels);

    analysis.removed.assign(count, false);
    std::vector<std::size_t> kept;
    kept.reserve(count);
    for (std::size_t index = 0; index < count; ++index)
    {
        if (detail::bidi::isRemovedByX9(analysis.classes[index]))
        {
            analysis.removed[index] = true;
        }
        else
        {
            kept.push_back(index);
        }
    }

    detail::bidi::resolveSequences(analysis.codepoints, analysis.classes, matchingPdi, kept, types,
                                   analysis.levels, level);
    detail::bidi::applyL1(kept, analysis.classes, level, analysis.levels);
    return analysis;
}

inline auto bidiLevels(std::string_view text, int paragraphLevel) -> std::vector<int>
{
    return bidiAnalyse(text, paragraphLevel).levels;
}

inline auto bidiReorder(std::string_view text, int paragraphLevel) -> std::string
{
    const BidiAnalysis analysis = bidiAnalyse(text, paragraphLevel);

    std::vector<std::size_t> kept;
    kept.reserve(analysis.codepoints.size());
    for (std::size_t index = 0; index < analysis.codepoints.size(); ++index)
    {
        if (!analysis.removed[index])
        {
            kept.push_back(index);
        }
    }

    const std::vector<std::size_t> order = detail::bidi::reorderIndices(kept, analysis.levels);

    std::string output;
    output.reserve(text.size());
    for (const std::size_t position : order)
    {
        detail::bidi::appendUtf8(output, analysis.codepoints[kept[position]]);
    }
    return output;
}

inline auto bidiMirror(std::uint32_t codepoint) -> std::uint32_t
{
    return detail::bidi::lookupMirror(codepoint);
}

inline auto bidiMirror(std::string_view utf8) -> std::string
{
    std::vector<std::uint32_t> codepoints;
    std::vector<std::size_t> byteOffsets;
    detail::bidi::decodeUtf8(utf8, codepoints, byteOffsets);

    std::string output;
    output.reserve(utf8.size());
    for (const std::uint32_t codepoint : codepoints)
    {
        detail::bidi::appendUtf8(output, detail::bidi::lookupMirror(codepoint));
    }
    return output;
}

inline auto bidiReorderWithMirroring(std::string_view utf8, int paragraphLevel) -> std::string
{
    const BidiAnalysis analysis = bidiAnalyse(utf8, paragraphLevel);

    std::vector<std::size_t> kept;
    kept.reserve(analysis.codepoints.size());
    for (std::size_t index = 0; index < analysis.codepoints.size(); ++index)
    {
        if (!analysis.removed[index])
        {
            kept.push_back(index);
        }
    }

    const std::vector<std::size_t> order = detail::bidi::reorderIndices(kept, analysis.levels);

    std::string output;
    output.reserve(utf8.size());
    for (const std::size_t position : order)
    {
        const std::size_t index = kept[position];
        std::uint32_t codepoint = analysis.codepoints[index];
        if (analysis.levels[index] % 2 != 0)
        {
            codepoint = detail::bidi::lookupMirror(codepoint);
        }
        detail::bidi::appendUtf8(output, codepoint);
    }
    return output;
}

inline auto bidiRuns(std::string_view text, int paragraphLevel) -> std::vector<BidiRun>
{
    const BidiAnalysis analysis = bidiAnalyse(text, paragraphLevel);

    std::vector<BidiRun> runs;
    for (std::size_t index = 0; index < analysis.codepoints.size(); ++index)
    {
        if (analysis.removed[index])
        {
            continue;
        }
        const std::size_t offset = analysis.byteOffsets[index];
        const std::size_t length = analysis.byteOffsets[index + 1] - offset;
        const int level = analysis.levels[index];
        if (!runs.empty() && runs.back().level == level &&
            runs.back().byteOffset + runs.back().byteLength == offset)
        {
            runs.back().byteLength += length;
        }
        else
        {
            runs.push_back(BidiRun{offset, length, level});
        }
    }
    return runs;
}

} // namespace bunch
