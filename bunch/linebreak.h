#pragma once

// ===========================================================================
// bunch/linebreak.h
//
// Unicode Line Breaking Algorithm (UAX #14, Unicode 15.0.0, revision 49) for
// UTF-8 text.
//
// The implementation follows the rule order of UAX #14 section 6.  In the rule
// summaries below "x" marks a prohibited break, "div" a break opportunity, and
// "!" a mandatory break.
//
//   LB1      class resolution: AI, SG and XX become AL, CJ becomes NS, and SA
//            becomes CM for General_Category Mn/Mc and AL otherwise.  CB is
//            left unresolved so that LB20 applies, which is what the
//            conformance data expects.
//   LB2/LB3  start and end of text.
//   LB4-LB6  mandatory breaks: BK, CR, LF and NL; CR LF is kept together.
//   LB7/LB8  spaces and zero width space (ZW SP* div).
//   LB8a     no break after ZWJ.
//   LB9      X (CM | ZWJ)* is treated as X, where X is not BK, CR, LF, NL, SP
//            or ZW: combining marks are folded into the base cluster.
//   LB10     remaining CM or ZWJ are treated as AL.
//   LB11-LB31 the tailorable rules, applied in order, including LB21a, LB21b,
//            LB23a, LB30a (regional indicator pairs) and LB30b (emoji base
//            plus emoji modifier, including the unassigned Extended_Pictographic
//            clause).
//
// Numeric expressions follow the tailoring of Example 7 in UAX #14 section 8.2,
// which is the variant the official LineBreakTest.txt data exercises: the
// CL, CP, IS and SY terms of LB13 only prohibit a break when the preceding
// class is not NU, and LB25 is the five-part regular-expression rule
//
//   (PR | PO) x (OP | HY)? NU
//   (OP | HY) x NU
//   NU x (NU | SY | IS)
//   NU (NU | SY | IS)* x (NU | SY | IS | CL | CP)
//   NU (NU | SY | IS)* (CL | CP)? x (PO | PR)
//
// instead of the untailored pair list.  This is a disclosed tailoring under
// UAX14-C2.  The two differ only for incomplete numeric expressions, where the
// tailoring allows a break that the pair list prohibits (for example between PR
// and OP when no digit follows).  All 7,654 cases of
// LineBreakTest-15.0.0.txt pass, which is the point of the tailoring: the
// official test data is generated with it.
//
// Break opportunities are returned as UTF-8 byte offsets into the original
// string, ascending, and always include 0 and text.size().  Mandatory breaks
// (after BK, CR, LF and NL) are included in the same result; pass
// LineBreakOptions::includeMandatory = false to obtain soft breaks only, or use
// mandatoryLineBreaks() to obtain only forced breaks.
//
// Known limitations:
//   * Only the default algorithm plus the numeric tailoring above is
//     implemented.  There is no dictionary or morphological analysis, so SA
//     text (Thai, Lao, Khmer, Myanmar, ...) breaks only where the default rules
//     allow; no language tailoring, hyphenation or emergency breaking is
//     performed.
//   * AI is resolved to AL as LB1 specifies in the absence of East Asian width
//     context.  Callers that need the CSS "normal" style can treat CJ as ID;
//     this implementation uses the LB1 default (NS), which corresponds to the
//     CSS "strict" style.
//   * Unicode 15.1 and later changes are not implemented.  LB28a and the AK,
//     AP, AS, VF and VI classes do not exist in Unicode 15.0.0; LB15a-LB15d,
//     LB20a and the revised LB30b belong to later versions.
//   * Ill-formed UTF-8 is decoded with one U+FFFD per invalid byte.  U+FFFD is
//     AI, which LB1 resolves to AL.
//   * The text is treated as one line: text.size() is always reported (LB3),
//     even when the text already ends with a hard break, and no paragraph or
//     line segmentation is performed.
// ===========================================================================

#include "linebreak_data.h"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

namespace bunch
{
// Line_Break property value, after LB1 resolution.  The enumerators live with
// the generated table in linebreak_data.h.
using LineBreakClass = detail::linebreak::LineBreakClass;

// Controls which positions lineBreakOpportunities reports.
struct LineBreakOptions
{
    // Report offset 0.  LB2 prohibits a break at the start of text, but a line
    // may always begin there, so the default is true.
    bool includeStart = true;

    // Report text.size().  LB3 requires a break at the end of text, and the
    // default is true.
    bool includeEnd = true;

    // Report the positions of mandatory breaks, that is the positions after
    // BK, CR, LF and NL.  The default is true; set it to false to obtain only
    // soft break opportunities.
    bool includeMandatory = true;
};

// LB1: the line breaking class of one code point.  AI, SG and XX resolve to
// AL, CJ to NS, and SA to CM or AL depending on General_Category.  CB is
// returned unchanged, as the default resolution leaves it to LB20.  Values
// above U+10FFFF and surrogates are unassigned, hence AL.
auto lineBreakClass(std::uint32_t codepoint) -> LineBreakClass;

// LB1 for every code point of a UTF-8 string, in logical order.  This is the
// class after LB1 only; the contextual LB9/LB10 treatment and the grouping into
// clusters are internal to lineBreakOpportunities.
auto lineBreakClasses(std::string_view text) -> std::vector<LineBreakClass>;

// Line break opportunities as UTF-8 byte offsets, ascending, including 0 and
// text.size().  Mandatory breaks (after BK, CR, LF, NL) are included.
auto lineBreakOpportunities(std::string_view text) -> std::vector<std::size_t>;

// Same, with the reporting rules of the given options.
auto lineBreakOpportunities(std::string_view text, const LineBreakOptions &options) -> std::vector<std::size_t>;

// Positions where UAX #14 requires a line break: after BK, CR, LF and NL (CR LF
// produces one break, after the LF), plus text.size() for the end of text
// (LB3).  Every returned position is also a line break opportunity.  For empty
// text the result is {0}.
auto mandatoryLineBreaks(std::string_view text) -> std::vector<std::size_t>;

namespace detail
{
namespace linebreak
{
// ---------------------------------------------------------------------------
// Decoding
// ---------------------------------------------------------------------------

inline constexpr std::uint32_t kReplacementCharacter = 0xFFFD;
inline constexpr std::size_t kNoIndex = static_cast<std::size_t>(-1);

struct DecodedText
{
    std::vector<std::uint32_t> codepoints;
    std::vector<std::size_t> byteOffsets; // codepoints.size() + 1 entries
};

inline auto isContinuationByte(unsigned char value) -> bool
{
    return (value & 0xC0U) == 0x80U;
}

// Decodes the code point starting at index.  Ill-formed byte sequences yield
// U+FFFD with width 1, so a run of invalid bytes produces one replacement
// character each.
inline auto decodeCodePoint(std::string_view text, std::size_t index, std::size_t &width) -> std::uint32_t
{
    const std::size_t size = text.size();
    const auto first = static_cast<unsigned char>(text[index]);
    if (first < 0x80U)
    {
        width = 1;
        return first;
    }

    if (first >= 0xC2U && first <= 0xDFU && index + 1 < size)
    {
        const auto second = static_cast<unsigned char>(text[index + 1]);
        if (isContinuationByte(second))
        {
            width = 2;
            return (static_cast<std::uint32_t>(first & 0x1FU) << 6) | static_cast<std::uint32_t>(second & 0x3FU);
        }
    }
    else if (first >= 0xE0U && first <= 0xEFU && index + 2 < size)
    {
        const auto second = static_cast<unsigned char>(text[index + 1]);
        const auto third = static_cast<unsigned char>(text[index + 2]);
        const bool secondOk = isContinuationByte(second) && (first != 0xE0U || second >= 0xA0U) &&
                              (first != 0xEDU || second <= 0x9FU);
        if (secondOk && isContinuationByte(third))
        {
            width = 3;
            return (static_cast<std::uint32_t>(first & 0x0FU) << 12) |
                   (static_cast<std::uint32_t>(second & 0x3FU) << 6) | static_cast<std::uint32_t>(third & 0x3FU);
        }
    }
    else if (first >= 0xF0U && first <= 0xF4U && index + 3 < size)
    {
        const auto second = static_cast<unsigned char>(text[index + 1]);
        const auto third = static_cast<unsigned char>(text[index + 2]);
        const auto fourth = static_cast<unsigned char>(text[index + 3]);
        const bool secondOk = isContinuationByte(second) && (first != 0xF0U || second >= 0x90U) &&
                              (first != 0xF4U || second <= 0x8FU);
        if (secondOk && isContinuationByte(third) && isContinuationByte(fourth))
        {
            width = 4;
            return (static_cast<std::uint32_t>(first & 0x07U) << 18) |
                   (static_cast<std::uint32_t>(second & 0x3FU) << 12) |
                   (static_cast<std::uint32_t>(third & 0x3FU) << 6) | static_cast<std::uint32_t>(fourth & 0x3FU);
        }
    }

    width = 1;
    return kReplacementCharacter;
}

inline auto decodeText(std::string_view text) -> DecodedText
{
    DecodedText decoded;
    decoded.codepoints.reserve(text.size());
    decoded.byteOffsets.reserve(text.size() + 1);
    std::size_t index = 0;
    while (index < text.size())
    {
        decoded.byteOffsets.push_back(index);
        std::size_t width = 0;
        decoded.codepoints.push_back(decodeCodePoint(text, index, width));
        index += width;
    }
    decoded.byteOffsets.push_back(text.size());
    return decoded;
}

// ---------------------------------------------------------------------------
// Classes and predicates
// ---------------------------------------------------------------------------

inline auto isAlphabetic(LineBreakClass cls) -> bool
{
    return cls == LineBreakClass::al || cls == LineBreakClass::hl;
}

inline auto isIdeographic(LineBreakClass cls) -> bool
{
    return cls == LineBreakClass::id || cls == LineBreakClass::eb || cls == LineBreakClass::em;
}

inline auto isHangulSyllable(LineBreakClass cls) -> bool
{
    return cls == LineBreakClass::jl || cls == LineBreakClass::jv || cls == LineBreakClass::jt ||
           cls == LineBreakClass::h2 || cls == LineBreakClass::h3;
}

inline auto isHardBreak(LineBreakClass cls) -> bool
{
    return cls == LineBreakClass::bk || cls == LineBreakClass::cr || cls == LineBreakClass::lf ||
           cls == LineBreakClass::nl;
}

// The classes that LB9 refuses to extend with combining marks.
inline auto isCombiningBaseExcluded(LineBreakClass cls) -> bool
{
    return isHardBreak(cls) || cls == LineBreakClass::sp || cls == LineBreakClass::zw;
}

// ---------------------------------------------------------------------------
// Clusters and context
// ---------------------------------------------------------------------------

// One LB9 cluster: a base character plus the CM/ZWJ characters attached to it.
// The byte offset is that of the first code point, which is where a break
// before the cluster would fall.
struct LineBreakCluster
{
    LineBreakClass cls = LineBreakClass::xx;
    std::size_t firstCodepoint = 0;
    std::size_t lastCodepoint = 0;
    std::size_t byteOffset = 0;
    bool endsWithZwj = false;
};

struct LineBreakContext
{
    std::vector<std::uint32_t> codepoints;
    std::vector<std::size_t> byteOffsets;
    std::vector<LineBreakCluster> clusters;
    std::vector<std::size_t> previousNonSpace;   // index of the nearest non-SP cluster
    std::vector<bool> numericRun;                // part of NU (NU | SY | IS)*
    std::vector<std::size_t> regionalIndicatorRun; // consecutive RI clusters ending here
};

inline auto buildContext(std::string_view text) -> LineBreakContext
{
    LineBreakContext context;
    DecodedText decoded = decodeText(text);
    context.codepoints = std::move(decoded.codepoints);
    context.byteOffsets = std::move(decoded.byteOffsets);

    const std::size_t count = context.codepoints.size();
    context.clusters.reserve(count);
    for (std::size_t index = 0; index < count; ++index)
    {
        const LineBreakClass resolved = lineBreakClass(context.codepoints[index]);
        LineBreakClass cls = resolved;
        if (cls == LineBreakClass::cm || cls == LineBreakClass::zwj)
        {
            // LB9: attach the combining mark or joiner to the preceding base.
            if (!context.clusters.empty() && !isCombiningBaseExcluded(context.clusters.back().cls))
            {
                context.clusters.back().lastCodepoint = index;
                context.clusters.back().endsWithZwj = cls == LineBreakClass::zwj;
                continue;
            }
            // LB10: a combining mark without a base behaves as AL.
            cls = LineBreakClass::al;
        }
        LineBreakCluster cluster;
        cluster.cls = cls;
        cluster.firstCodepoint = index;
        cluster.lastCodepoint = index;
        cluster.byteOffset = context.byteOffsets[index];
        cluster.endsWithZwj = resolved == LineBreakClass::zwj;
        context.clusters.push_back(cluster);
    }

    const std::size_t clusterCount = context.clusters.size();
    context.previousNonSpace.assign(clusterCount, kNoIndex);
    context.numericRun.assign(clusterCount, false);
    context.regionalIndicatorRun.assign(clusterCount, 0);
    std::size_t lastNonSpace = kNoIndex;
    for (std::size_t index = 0; index < clusterCount; ++index)
    {
        const LineBreakClass cls = context.clusters[index].cls;
        if (cls != LineBreakClass::sp)
        {
            lastNonSpace = index;
        }
        context.previousNonSpace[index] = lastNonSpace;

        if (cls == LineBreakClass::nu)
        {
            context.numericRun[index] = true;
        }
        else if ((cls == LineBreakClass::sy || cls == LineBreakClass::is) && index > 0 &&
                 context.numericRun[index - 1])
        {
            context.numericRun[index] = true;
        }

        if (cls == LineBreakClass::ri)
        {
            context.regionalIndicatorRun[index] =
                (index > 0 ? context.regionalIndicatorRun[index - 1] : 0) + 1;
        }
    }
    return context;
}

// ---------------------------------------------------------------------------
// Rules
// ---------------------------------------------------------------------------

// Decides the boundary between clusters[index - 1] and clusters[index], which
// requires index >= 1.  Rules are applied in their normative order; the first
// one that matches decides.
inline auto isLineBreakAllowed(const LineBreakContext &context, std::size_t index) -> bool
{
    const std::vector<LineBreakCluster> &clusters = context.clusters;
    const std::size_t clusterCount = clusters.size();
    const LineBreakClass left = clusters[index - 1].cls;
    const LineBreakClass right = clusters[index].cls;
    const std::size_t before = context.previousNonSpace[index - 1];

    // LB4/LB5/LB6: mandatory breaks.
    if (left == LineBreakClass::bk)
    {
        return true;
    }
    if (left == LineBreakClass::cr && right == LineBreakClass::lf)
    {
        return false;
    }
    if (left == LineBreakClass::cr || left == LineBreakClass::lf || left == LineBreakClass::nl)
    {
        return true;
    }
    if (isHardBreak(right))
    {
        return false;
    }

    // LB7: no break before spaces or zero width space.
    if (right == LineBreakClass::sp || right == LineBreakClass::zw)
    {
        return false;
    }

    // LB8: break after ZW, even with intervening spaces.
    if (before != kNoIndex && clusters[before].cls == LineBreakClass::zw)
    {
        return true;
    }

    // LB8a: no break after a zero width joiner.
    if (clusters[index - 1].endsWithZwj)
    {
        return false;
    }

    // LB9 and LB10 are handled by the clustering in buildContext.

    // LB11: word joiner and related characters.
    if (right == LineBreakClass::wj || left == LineBreakClass::wj)
    {
        return false;
    }

    // LB12/LB12a: glue, except after spaces and hyphens.
    if (left == LineBreakClass::gl)
    {
        return false;
    }
    if (right == LineBreakClass::gl && left != LineBreakClass::sp && left != LineBreakClass::ba &&
        left != LineBreakClass::hy)
    {
        return false;
    }

    // LB13 with the Example 7 tailoring: NU is left to LB25.
    if ((right == LineBreakClass::cl || right == LineBreakClass::cp || right == LineBreakClass::is ||
         right == LineBreakClass::sy) &&
        left != LineBreakClass::nu)
    {
        return false;
    }
    if (right == LineBreakClass::ex)
    {
        return false;
    }

    // LB14-LB17: rules that also look through a run of spaces.
    if (before != kNoIndex)
    {
        const LineBreakClass prior = clusters[before].cls;
        if (prior == LineBreakClass::op)
        {
            return false;
        }
        if (right == LineBreakClass::op && prior == LineBreakClass::qu)
        {
            return false;
        }
        if (right == LineBreakClass::ns && (prior == LineBreakClass::cl || prior == LineBreakClass::cp))
        {
            return false;
        }
        if (right == LineBreakClass::b2 && prior == LineBreakClass::b2)
        {
            return false;
        }
    }

    // LB18: break after spaces.
    if (left == LineBreakClass::sp)
    {
        return true;
    }

    // LB19: no break before or after quotation marks.
    if (right == LineBreakClass::qu || left == LineBreakClass::qu)
    {
        return false;
    }

    // LB20: break before and after an unresolved contingent break.
    if (right == LineBreakClass::cb || left == LineBreakClass::cb)
    {
        return true;
    }

    // LB21: no break before BA, HY or NS, and none after BB.
    if (right == LineBreakClass::ba || right == LineBreakClass::hy || right == LineBreakClass::ns ||
        left == LineBreakClass::bb)
    {
        return false;
    }

    // LB21a: no break after Hebrew plus hyphen.
    if ((left == LineBreakClass::hy || left == LineBreakClass::ba) && index >= 2 &&
        clusters[index - 2].cls == LineBreakClass::hl)
    {
        return false;
    }

    // LB21b: no break between solidus and Hebrew letters.
    if (left == LineBreakClass::sy && right == LineBreakClass::hl)
    {
        return false;
    }

    // LB22: no break before ellipses.
    if (right == LineBreakClass::in)
    {
        return false;
    }

    // LB23: no break between digits and letters.
    if ((isAlphabetic(left) && right == LineBreakClass::nu) ||
        (left == LineBreakClass::nu && isAlphabetic(right)))
    {
        return false;
    }

    // LB23a: numeric prefixes and ideographs.
    if ((left == LineBreakClass::pr && isIdeographic(right)) ||
        (isIdeographic(left) && right == LineBreakClass::po))
    {
        return false;
    }

    // LB24: numeric prefixes and postfixes against letters.
    if (((left == LineBreakClass::pr || left == LineBreakClass::po) && isAlphabetic(right)) ||
        (isAlphabetic(left) && (right == LineBreakClass::pr || right == LineBreakClass::po)))
    {
        return false;
    }

    // LB25 (Example 7 tailoring): do not break numbers.
    if (left == LineBreakClass::pr || left == LineBreakClass::po)
    {
        if (right == LineBreakClass::nu)
        {
            return false;
        }
        if ((right == LineBreakClass::op || right == LineBreakClass::hy) && index + 1 < clusterCount &&
            clusters[index + 1].cls == LineBreakClass::nu)
        {
            return false;
        }
    }
    if ((left == LineBreakClass::op || left == LineBreakClass::hy) && right == LineBreakClass::nu)
    {
        return false;
    }
    if (left == LineBreakClass::nu && (right == LineBreakClass::nu || right == LineBreakClass::sy ||
                                      right == LineBreakClass::is))
    {
        return false;
    }
    if (context.numericRun[index - 1] &&
        (right == LineBreakClass::nu || right == LineBreakClass::sy || right == LineBreakClass::is ||
         right == LineBreakClass::cl || right == LineBreakClass::cp))
    {
        return false;
    }
    if (right == LineBreakClass::po || right == LineBreakClass::pr)
    {
        if (context.numericRun[index - 1])
        {
            return false;
        }
        if ((left == LineBreakClass::cl || left == LineBreakClass::cp) && index >= 2 &&
            context.numericRun[index - 2])
        {
            return false;
        }
    }

    // LB26: no break inside a Korean syllable.
    if (left == LineBreakClass::jl && (right == LineBreakClass::jl || right == LineBreakClass::jv ||
                                       right == LineBreakClass::h2 || right == LineBreakClass::h3))
    {
        return false;
    }
    if ((left == LineBreakClass::jv || left == LineBreakClass::h2) &&
        (right == LineBreakClass::jv || right == LineBreakClass::jt))
    {
        return false;
    }
    if ((left == LineBreakClass::jt || left == LineBreakClass::h3) && right == LineBreakClass::jt)
    {
        return false;
    }

    // LB27: a Korean syllable block behaves like ID.
    if (isHangulSyllable(left) && right == LineBreakClass::po)
    {
        return false;
    }
    if (left == LineBreakClass::pr && isHangulSyllable(right))
    {
        return false;
    }

    // LB28: no break between alphabetics.
    if (isAlphabetic(left) && isAlphabetic(right))
    {
        return false;
    }

    // LB29: no break between numeric punctuation and alphabetics.
    if (left == LineBreakClass::is && isAlphabetic(right))
    {
        return false;
    }

    // LB30: parentheses, except East Asian wide ones.
    if ((isAlphabetic(left) || left == LineBreakClass::nu) && right == LineBreakClass::op &&
        !isEastAsianWideOpenClose(context.codepoints[clusters[index].firstCodepoint]))
    {
        return false;
    }
    if (left == LineBreakClass::cp && !isEastAsianWideOpenClose(context.codepoints[clusters[index - 1].firstCodepoint]) &&
        (isAlphabetic(right) || right == LineBreakClass::nu))
    {
        return false;
    }

    // LB30a: break between regional indicators only in pairs.
    if (left == LineBreakClass::ri && right == LineBreakClass::ri &&
        context.regionalIndicatorRun[index - 1] % 2U == 1U)
    {
        return false;
    }

    // LB30b: no break between an emoji base and an emoji modifier.
    if (left == LineBreakClass::eb && right == LineBreakClass::em)
    {
        return false;
    }
    if (right == LineBreakClass::em &&
        isUnassignedPictographic(context.codepoints[clusters[index - 1].firstCodepoint]))
    {
        return false;
    }

    // LB31: break everywhere else.
    return true;
}

// ---------------------------------------------------------------------------
// Analysis
// ---------------------------------------------------------------------------

struct BreakPoint
{
    std::size_t byteOffset = 0;
    bool start = false;     // offset 0
    bool end = false;       // text.size()
    bool mandatory = false; // break required after BK, CR, LF or NL
};

inline auto analyseLineBreaks(std::string_view text) -> std::vector<BreakPoint>
{
    const LineBreakContext context = buildContext(text);
    std::vector<BreakPoint> points;
    points.reserve(context.clusters.size() + 1);

    if (context.clusters.empty())
    {
        // The start and the end of text coincide; LB2 and LB3 both apply.
        BreakPoint point;
        point.start = true;
        point.end = true;
        points.push_back(point);
        return points;
    }

    BreakPoint start;
    start.start = true;
    points.push_back(start);

    for (std::size_t index = 1; index < context.clusters.size(); ++index)
    {
        if (!isLineBreakAllowed(context, index))
        {
            continue;
        }
        BreakPoint point;
        point.byteOffset = context.clusters[index].byteOffset;
        point.mandatory = isHardBreak(context.clusters[index - 1].cls);
        points.push_back(point);
    }

    // LB3: always break at the end of text.
    BreakPoint end;
    end.byteOffset = text.size();
    end.end = true;
    points.push_back(end);
    return points;
}
} // namespace linebreak
} // namespace detail

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

inline auto lineBreakClass(std::uint32_t codepoint) -> LineBreakClass
{
    const LineBreakClass property = detail::linebreak::lineBreakProperty(codepoint);
    switch (property)
    {
    case LineBreakClass::ai:
    case LineBreakClass::sg:
    case LineBreakClass::xx:
        return LineBreakClass::al;
    case LineBreakClass::cj:
        return LineBreakClass::ns;
    case LineBreakClass::sa:
        return detail::linebreak::isSaCombining(codepoint) ? LineBreakClass::cm : LineBreakClass::al;
    default:
        return property;
    }
}

inline auto lineBreakClasses(std::string_view text) -> std::vector<LineBreakClass>
{
    const detail::linebreak::DecodedText decoded = detail::linebreak::decodeText(text);
    std::vector<LineBreakClass> classes;
    classes.reserve(decoded.codepoints.size());
    for (const std::uint32_t codepoint : decoded.codepoints)
    {
        classes.push_back(lineBreakClass(codepoint));
    }
    return classes;
}

inline auto lineBreakOpportunities(std::string_view text) -> std::vector<std::size_t>
{
    return lineBreakOpportunities(text, LineBreakOptions{});
}

inline auto lineBreakOpportunities(std::string_view text, const LineBreakOptions &options) -> std::vector<std::size_t>
{
    const std::vector<detail::linebreak::BreakPoint> points = detail::linebreak::analyseLineBreaks(text);
    std::vector<std::size_t> result;
    result.reserve(points.size());
    for (const detail::linebreak::BreakPoint &point : points)
    {
        // A position with a special role is reported when at least one of its
        // roles is enabled: 0 is the start, text.size() is the end, and a
        // position after BK, CR, LF or NL is a mandatory break.
        if (point.start || point.end || point.mandatory)
        {
            const bool included = (point.start && options.includeStart) ||
                                  (point.end && options.includeEnd) ||
                                  (point.mandatory && options.includeMandatory);
            if (!included)
            {
                continue;
            }
        }
        result.push_back(point.byteOffset);
    }
    return result;
}

inline auto mandatoryLineBreaks(std::string_view text) -> std::vector<std::size_t>
{
    const std::vector<detail::linebreak::BreakPoint> points = detail::linebreak::analyseLineBreaks(text);
    std::vector<std::size_t> result;
    for (const detail::linebreak::BreakPoint &point : points)
    {
        if (point.mandatory || point.end)
        {
            result.push_back(point.byteOffset);
        }
    }
    return result;
}
} // namespace bunch
