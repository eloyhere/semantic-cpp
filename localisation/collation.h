#pragma once

// ===========================================================================
// localisation/collation.h
//
// Unicode Collation Algorithm (UCA, UTS #10) 15.0.0 with the Default Unicode
// Collation Element Table (DUCET), default ("non-ignorable") variant, plus the
// CLDR tailorings of the locales listed in collation_data.h.
//
// The root algorithm is the one described by UTS #10 section 7:
//
//   1. Normalise the input to NFD (canonical decomposition + canonical
//      ordering).  bunch::nfd() implements both steps from the Unicode 15.0
//      tables in bunch/unicode_data.h.
//   2. Map the code point sequence to collation elements with the DUCET:
//      contractions first (longest match, up to three code points, as in the
//      DUCET), then single code points.  Code points without an explicit
//      entry receive the implicit weights of UTS #10 section 10.1.3.
//   3. Compare the element arrays level by level (primary, secondary,
//      tertiary), skipping zero weights.  sortKey() emits the same weights
//      with a 0 separator between levels, so lexicographic comparison of two
//      sort keys agrees with compare().
//
// The table data lives in collation_data.h, generated from allkeys-15.0.0.txt
// and the CLDR collation files by tools/gen_collation_data.py.  Both files are
// Unicode 15.0.0, so the canonical decompositions in bunch/unicode.h and the
// DUCET agree.
//
// Tailoring
// ---------
// A locale with CLDR rules is compared in an expanded weight space so that
// synthetic weights fit between two adjacent root weights:
//
//   primary   = root primary   << 16
//   secondary = root secondary << 4
//   tertiary  = root tertiary  << 3
//
// A CLDR primary relation ("<") opens a slot above its anchor, a secondary
// relation ("<<") a slot above the anchor's secondary and a tertiary relation
// ("<<<") a slot above its tertiary.  Locales without rules (de, fr and ru,
// whose CLDR "standard" collation adds nothing to the root order, and every
// unknown locale) keep the compact root weights, so their sort keys are
// exactly the DUCET weights.
//
//   built in:   cs da es et fi hu lt lv nb nn no pl ro sk tr
//   root-equal: de fr ru   (their CLDR "standard" collation adds no rule)
//   native:     sv         (the hand-written rules below)
//   root only:  ja zh ko   (rule files of 33 KB, 1.1 MB and 50 KB)
//
// ja, zh and ko are deliberately not built in: their collation data is
// script-specific and far larger than the compact table this header keeps, so
// they fall back to the root order (Han characters by implicit weight, Hangul
// by its decomposition).  Every other language tag, including unknown ones,
// also falls back to root.
//
// Contractions
// ------------
// Both the DUCET contractions and the tailored ones are matched with the
// longest key first.  A contraction may also match *discontiguously* (UTS #10
// section 3.3.2): when the next code point of the key is a combining mark, the
// scan may skip intervening non-starters whose canonical combining class is
// lower, because canonical ordering already placed them before it.  The
// skipped marks are emitted after the contraction's elements, so they keep
// taking part in the secondary comparison:
//
//   "s" + U+0323 (ccc 220) + U+030C (ccc 230)  matches the Czech "š"
//   contraction, giving [š] [dot below]; "s" + U+0301 + U+030C does not,
//   because U+0301 has the same combining class as U+030C and blocks it.
//
// Limits
// ------
//   * Three levels (primary, secondary, tertiary).  There is no identical or
//     quaternary level, and no shifted/blanked variable weighting: the
//     non-ignorable variant keeps the primary weight of variable characters
//     such as spaces and punctuation.
//   * CLDR rule options are not interpreted: [import ...], [reorder ...] and
//     [caseFirst ...] are ignored by the generator, which only compiles the
//     "standard" collation of a locale.  Danish/Norwegian therefore differ
//     from CLDR in one respect: [caseFirst upper] is not applied, so case is
//     still ordered lower-first at the tertiary level.
//   * The Russian "standard" collation only contains [reorder Cyrl]; it is
//     emitted as an empty rule set and therefore uses the root order, which
//     already sorts Cyrillic after Latin.
//   * Swedish keeps its historical hand-written tailoring (a-ring,
//     a-diaeresis and o-diaeresis after z with the fixed primaries
//     0xE000..0xE002) instead of the CLDR sv rules, which would additionally
//     place ae/ae-diaeresis, oe, o-slash and friends.  The CLDR sv ordering of
//     a-ring < a-diaeresis < o-diaeresis is the same.
//   * German uses the CLDR "standard" collation, which is identical to root
//     (umlauts are secondary variants of a/o/u and sharp s expands to "ss").
//     The CLDR "phonebook" and "eor" variants are not built in.
//   * Unassigned code points use lead unit 0xFBC0 from UTS #10 Table 16 (a
//     0xFFFE base appears in some older descriptions, but the table that
//     matches DUCET 15.0.0, and that keeps the sum inside 16 bits, is
//     0xFBC0).  Unified ideographs use 0xFB40 in the BMP core Han blocks and
//     0xFB80 elsewhere, exactly as Table 16 specifies.
//   * All input is UTF-8.  Ill-formed sequences become U+FFFD through
//     bunch::decode(), which is also what the DUCET replacement character
//     weight expects.
//
// Example:
//
//   localisation::Collator collator(localisation::Locale("sv"));
//   std::vector<std::string> words{u8"\u00F6", "z", u8"\u00E5", "a", u8"\u00E4"};
//   std::sort(words.begin(), words.end(), [&](const auto &left, const auto &right) {
//       return collator.less(left, right);
//   }); // a, z, a-ring, a-diaeresis, o-diaeresis
// ===========================================================================

#include "collation_data.h"
#include "localisation.h"

#include "../bunch/unicode.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace localisation
{

// ---------------------------------------------------------------------------
// CollationStrength: how many levels take part in a comparison.
// ---------------------------------------------------------------------------

enum class CollationStrength
{
    primary,
    secondary,
    tertiary
};

namespace detail
{
namespace collation
{

// ---------------------------------------------------------------------------
// Collation elements
// ---------------------------------------------------------------------------

struct CollationElement
{
    std::uint32_t primary;
    std::uint16_t secondary;
    std::uint16_t tertiary;
};

// Shift applied to the root weights of a tailored locale.  The values must
// match the ones documented in collation_data.h: 16 bits of room for primary
// slots, 4 bits for secondary and 3 bits for tertiary ones.
struct WeightShift
{
    int primary = 0;
    int secondary = 0;
    int tertiary = 0;
};

inline auto shifted(std::uint32_t primary, std::uint16_t secondary, std::uint16_t tertiary,
                    WeightShift shift) noexcept -> CollationElement
{
    return CollationElement{
        static_cast<std::uint32_t>(primary << shift.primary),
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(secondary << shift.secondary)),
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(tertiary << shift.tertiary))};
}

inline auto unpackElement(std::uint32_t packed, WeightShift shift) noexcept -> CollationElement
{
    // Layout documented in collation_data.h: 16 primary bits, 9 secondary
    // bits and 5 tertiary bits.
    return shifted(packed >> 14, static_cast<std::uint16_t>((packed >> 5) & 0x1FF),
                   static_cast<std::uint16_t>(packed & 0x1F), shift);
}

inline auto appendSequence(std::vector<CollationElement> &output, std::uint32_t reference,
                           WeightShift shift) -> void
{
    const std::uint32_t offset = reference >> 5;
    const std::uint32_t count = reference & 0x1F;
    for (std::uint32_t index = 0; index < count; ++index)
    {
        output.push_back(unpackElement(collation_data::kCollationElements[offset + index], shift));
    }
}

// Tailored elements are stored already expanded, so no shift is applied.
inline auto appendTailored(std::vector<CollationElement> &output, std::uint32_t reference) -> void
{
    const std::uint32_t offset = reference >> 5;
    const std::uint32_t count = reference & 0x1F;
    for (std::uint32_t index = 0; index < count; ++index)
    {
        const collation_data::TailoredElement &element = collation_data::kTailoredElements[offset + index];
        output.push_back(
            CollationElement{element.primary, element.secondary, element.tertiary});
    }
}

// ---------------------------------------------------------------------------
// Tailoring selection
// ---------------------------------------------------------------------------

struct Tailoring
{
    bool swedish = false;                                        // native Swedish rules
    const collation_data::TailoredEntry *entries = nullptr;      // CLDR rules
    std::size_t entryCount = 0;
    WeightShift shift{};

    auto hasRules() const noexcept -> bool { return entries != nullptr; }
};

inline auto findTailoredLocale(std::string_view language) -> const collation_data::TailoredLocale *
{
    const collation_data::TailoredLocale *begin = collation_data::kTailoredLocales;
    const collation_data::TailoredLocale *end = begin + collation_data::kTailoredLocaleCount;
    for (const collation_data::TailoredLocale *candidate = begin; candidate != end; ++candidate)
    {
        if (language == candidate->tag)
        {
            return candidate;
        }
    }
    return nullptr;
}

inline auto tailoringFor(const Locale &locale) -> Tailoring
{
    Tailoring tailoring;
    // Language subtag only: sv, sv-SE and sv-FI all get the Swedish rules.
    if (locale.language() == "sv")
    {
        tailoring.swedish = true;
        return tailoring;
    }
    const collation_data::TailoredLocale *found = findTailoredLocale(locale.language());
    if (found != nullptr && found->entryCount != 0)
    {
        tailoring.entries = collation_data::kTailoredEntries + found->entryOffset;
        tailoring.entryCount = found->entryCount;
        tailoring.shift = WeightShift{16, 4, 3};
    }
    return tailoring;
}

// Swedish: a-ring < a-diaeresis < o-diaeresis, all after z.  Case is carried
// by the tertiary weight of the base letter (2 for lowercase, 8 for
// uppercase), as in the DUCET.
struct SwedishRule
{
    std::uint32_t base;    // lowercase ASCII base letter
    std::uint32_t mark;    // combining mark, after canonical ordering
    std::uint32_t primary; // tailored primary weight
};

inline constexpr SwedishRule kSwedishRules[] = {
    {0x0061, 0x030A, 0xE000}, // a + ring above -> a-ring
    {0x0061, 0x0308, 0xE001}, // a + diaeresis -> a-diaeresis
    {0x006F, 0x0308, 0xE002}, // o + diaeresis -> o-diaeresis
};

inline constexpr std::size_t kSwedishRuleCount = sizeof(kSwedishRules) / sizeof(kSwedishRules[0]);

// Matches a two-code-point tailored sequence at the given position.  The
// input is NFD, so precomposed a-ring/a-diaeresis/o-diaeresis have already
// become a/o plus a mark.
inline auto matchSwedish(const std::vector<std::uint32_t> &codepoints, std::size_t index,
                         CollationElement &element) -> bool
{
    if (index + 1 >= codepoints.size())
    {
        return false;
    }
    const std::uint32_t base = codepoints[index];
    const std::uint32_t mark = codepoints[index + 1];
    for (std::size_t rule = 0; rule < kSwedishRuleCount; ++rule)
    {
        const SwedishRule &candidate = kSwedishRules[rule];
        if (candidate.mark != mark)
        {
            continue;
        }
        if (base == candidate.base)
        {
            element = CollationElement{candidate.primary, 0x0020, 0x0002};
            return true;
        }
        // The rule bases are ASCII, so uppercase is base - 0x20.
        if (base == candidate.base - 0x20)
        {
            element = CollationElement{candidate.primary, 0x0020, 0x0008};
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// DUCET lookups
// ---------------------------------------------------------------------------

inline auto findScalarIndex(std::uint32_t codepoint) -> std::size_t
{
    const std::uint32_t *begin = collation_data::kScalarCodepoints;
    const std::uint32_t *end = begin + collation_data::kScalarCount;
    const std::uint32_t *position = std::lower_bound(begin, end, codepoint);
    if (position == end || *position != codepoint)
    {
        return collation_data::kScalarCount;
    }
    return static_cast<std::size_t>(position - begin);
}

inline auto isUnifiedIdeograph(std::uint32_t codepoint) -> bool
{
    const collation_data::CodepointRange *begin = collation_data::kUnifiedIdeographRanges;
    const collation_data::CodepointRange *end = begin + collation_data::kUnifiedIdeographRangeCount;
    const collation_data::CodepointRange *position = std::lower_bound(
        begin, end, codepoint, [](const collation_data::CodepointRange &range, std::uint32_t wanted) {
            return range.last < wanted;
        });
    return position != end && position->first <= codepoint;
}

inline auto isCoreHan(std::uint32_t codepoint) -> bool
{
    // UTS #10 Table 16: the CJK Unified Ideographs and CJK Compatibility
    // Ideographs blocks use the 0xFB40 lead unit; all other unified
    // ideographs (Extension A and the supplementary planes) use 0xFB80.
    return (codepoint >= 0x4E00 && codepoint <= 0x9FFF) || (codepoint >= 0xF900 && codepoint <= 0xFAFF);
}

inline auto appendImplicit(std::vector<CollationElement> &output, std::uint32_t codepoint,
                           WeightShift shift) -> void
{
    // @implicitweights ranges (Tangut, Nushu, Khitan Small Script).
    for (std::size_t index = 0; index < collation_data::kImplicitRangeCount; ++index)
    {
        const collation_data::ImplicitRange &range = collation_data::kImplicitRanges[index];
        if (codepoint >= range.first && codepoint <= range.last)
        {
            output.push_back(shifted(range.base, 0x0020, 0x0002, shift));
            output.push_back(shifted((codepoint - range.offset) | 0x8000u, 0, 0, shift));
            return;
        }
    }

    std::uint32_t base = 0xFBC0; // unassigned code points
    if (isUnifiedIdeograph(codepoint))
    {
        base = isCoreHan(codepoint) ? 0xFB40 : 0xFB80;
    }
    output.push_back(shifted(base + (codepoint >> 15), 0x0020, 0x0002, shift));
    output.push_back(shifted((codepoint & 0x7FFFu) | 0x8000u, 0, 0, shift));
}

inline auto appendSingle(std::vector<CollationElement> &output, std::uint32_t codepoint,
                         WeightShift shift) -> void
{
    const std::size_t scalarIndex = findScalarIndex(codepoint);
    if (scalarIndex != collation_data::kScalarCount)
    {
        appendSequence(output, collation_data::kScalarElementRefs[scalarIndex], shift);
    }
    else
    {
        appendImplicit(output, codepoint, shift);
    }
}

// ---------------------------------------------------------------------------
// Contraction matching, including discontiguous matches
// ---------------------------------------------------------------------------

// Number of combining marks that one contraction may skip.  The DUCET and the
// built-in CLDR tailorings never need more than one; four keeps the matcher
// allocation-free.
inline constexpr std::size_t kMaxSkippedMarks = 4;

struct ContractionMatch
{
    std::size_t next = 0;                       // index just past the match
    std::size_t skippedCount = 0;
    std::size_t skipped[kMaxSkippedMarks] = {}; // indices of skipped marks
};

// Matches key[0..length) at codepoints[index].  key[0] is already known to
// match.  An intervening code point may be skipped when it is a non-starter
// with a lower canonical combining class than the key element it precedes
// (UTS #10 section 3.3.2); anything else blocks the match.
inline auto matchKey(const std::vector<std::uint32_t> &codepoints, std::size_t index,
                     const std::uint32_t *key, std::size_t length, ContractionMatch &match) -> bool
{
    std::size_t position = index + 1;
    std::size_t skipped = 0;
    for (std::size_t part = 1; part < length; ++part)
    {
        const std::uint32_t wanted = key[part];
        const int wantedClass = bunch::detail::unicode::combiningClass(wanted);
        bool found = false;
        while (position < codepoints.size())
        {
            const std::uint32_t current = codepoints[position];
            if (current == wanted)
            {
                found = true;
                ++position;
                break;
            }
            const int currentClass = bunch::detail::unicode::combiningClass(current);
            if (wantedClass == 0 || currentClass == 0 || currentClass >= wantedClass)
            {
                return false; // blocked
            }
            if (skipped >= kMaxSkippedMarks)
            {
                return false;
            }
            match.skipped[skipped] = position;
            ++skipped;
            ++position;
        }
        if (!found)
        {
            return false;
        }
    }
    match.next = position;
    match.skippedCount = skipped;
    return true;
}

inline auto matchTailored(const std::vector<std::uint32_t> &codepoints, std::size_t index,
                          const Tailoring &tailoring, const collation_data::TailoredEntry *&matched,
                          ContractionMatch &match) -> bool
{
    if (tailoring.entries == nullptr)
    {
        return false;
    }
    const std::uint32_t first = codepoints[index];
    const collation_data::TailoredEntry *begin = tailoring.entries;
    const collation_data::TailoredEntry *end = begin + tailoring.entryCount;
    const collation_data::TailoredEntry *position =
        std::lower_bound(begin, end, first,
                         [](const collation_data::TailoredEntry &entry, std::uint32_t wanted) {
                             return entry.first < wanted;
                         });
    for (std::size_t length = 4; length >= 1; --length)
    {
        for (const collation_data::TailoredEntry *candidate = position;
             candidate != end && candidate->first == first; ++candidate)
        {
            const std::uint32_t key[4] = {candidate->first, candidate->second, candidate->third,
                                          candidate->fourth};
            std::size_t candidateLength = 4;
            while (candidateLength > 1 && key[candidateLength - 1] == 0)
            {
                --candidateLength;
            }
            if (candidateLength != length)
            {
                continue;
            }
            if (length == 1)
            {
                match.next = index + 1;
                match.skippedCount = 0;
            }
            else if (!matchKey(codepoints, index, key, length, match))
            {
                continue;
            }
            matched = candidate;
            return true;
        }
    }
    return false;
}

inline auto matchRootContraction(const std::vector<std::uint32_t> &codepoints, std::size_t index,
                                 std::uint32_t &elementRef, ContractionMatch &match) -> bool
{
    const std::uint32_t first = codepoints[index];
    const collation_data::Contraction *begin = collation_data::kContractions;
    const collation_data::Contraction *end = begin + collation_data::kContractionCount;
    const collation_data::Contraction *position =
        std::lower_bound(begin, end, first,
                         [](const collation_data::Contraction &entry, std::uint32_t wanted) {
                             return entry.first < wanted;
                         });
    for (std::size_t length = 3; length >= 2; --length)
    {
        for (const collation_data::Contraction *candidate = position;
             candidate != end && candidate->first == first; ++candidate)
        {
            const std::size_t candidateLength = candidate->third == 0 ? 2 : 3;
            if (candidateLength != length)
            {
                continue;
            }
            const std::uint32_t key[3] = {candidate->first, candidate->second, candidate->third};
            if (!matchKey(codepoints, index, key, length, match))
            {
                continue;
            }
            elementRef = candidate->elementRef;
            return true;
        }
    }
    return false;
}

inline auto appendSkipped(std::vector<CollationElement> &output,
                          const std::vector<std::uint32_t> &codepoints, const ContractionMatch &match,
                          WeightShift shift) -> void
{
    for (std::size_t skipped = 0; skipped < match.skippedCount; ++skipped)
    {
        appendSingle(output, codepoints[match.skipped[skipped]], shift);
    }
}

// ---------------------------------------------------------------------------
// Collation element generation
// ---------------------------------------------------------------------------

inline auto collationElements(std::string_view text, const Tailoring &tailoring)
    -> std::vector<CollationElement>
{
    const std::string decomposed = bunch::nfd(text);
    std::vector<std::uint32_t> codepoints = bunch::detail::toCodepoints(decomposed);
    // nfd() already performed canonical ordering; running the stable sort
    // again keeps this pipeline explicit and is idempotent.
    bunch::detail::orderCombiningMarks(codepoints);

    std::vector<CollationElement> output;
    output.reserve(codepoints.size());

    std::size_t index = 0;
    while (index < codepoints.size())
    {
        CollationElement tailored{};
        if (tailoring.swedish && matchSwedish(codepoints, index, tailored))
        {
            output.push_back(tailored);
            index += 2;
            continue;
        }

        const collation_data::TailoredEntry *entry = nullptr;
        ContractionMatch match;
        if (matchTailored(codepoints, index, tailoring, entry, match))
        {
            appendTailored(output, entry->elementRef);
            appendSkipped(output, codepoints, match, tailoring.shift);
            index = match.next;
            continue;
        }

        std::uint32_t rootRef = 0;
        if (matchRootContraction(codepoints, index, rootRef, match))
        {
            appendSequence(output, rootRef, tailoring.shift);
            appendSkipped(output, codepoints, match, tailoring.shift);
            index = match.next;
            continue;
        }

        appendSingle(output, codepoints[index], tailoring.shift);
        ++index;
    }
    return output;
}

// ---------------------------------------------------------------------------
// Comparison and sort keys
// ---------------------------------------------------------------------------

inline auto weightAt(const CollationElement &element, int level) noexcept -> std::uint32_t
{
    switch (level)
    {
    case 0:
        return element.primary;
    case 1:
        return element.secondary;
    default:
        return element.tertiary;
    }
}

inline auto levelCount(CollationStrength strength) noexcept -> int
{
    switch (strength)
    {
    case CollationStrength::primary:
        return 1;
    case CollationStrength::secondary:
        return 2;
    case CollationStrength::tertiary:
        return 3;
    }
    return 3;
}

inline auto compareAtLevel(const std::vector<CollationElement> &left,
                           const std::vector<CollationElement> &right, int level) -> int
{
    std::size_t leftIndex = 0;
    std::size_t rightIndex = 0;
    while (true)
    {
        while (leftIndex < left.size() && weightAt(left[leftIndex], level) == 0)
        {
            ++leftIndex;
        }
        while (rightIndex < right.size() && weightAt(right[rightIndex], level) == 0)
        {
            ++rightIndex;
        }
        if (leftIndex == left.size() || rightIndex == right.size())
        {
            break;
        }
        const std::uint32_t leftWeight = weightAt(left[leftIndex], level);
        const std::uint32_t rightWeight = weightAt(right[rightIndex], level);
        if (leftWeight != rightWeight)
        {
            return leftWeight < rightWeight ? -1 : 1;
        }
        ++leftIndex;
        ++rightIndex;
    }
    const bool leftDone = leftIndex == left.size();
    const bool rightDone = rightIndex == right.size();
    if (leftDone && rightDone)
    {
        return 0;
    }
    return leftDone ? -1 : 1;
}

inline auto compareElements(const std::vector<CollationElement> &left,
                            const std::vector<CollationElement> &right, CollationStrength strength) -> int
{
    const int levels = levelCount(strength);
    for (int level = 0; level < levels; ++level)
    {
        const int result = compareAtLevel(left, right, level);
        if (result != 0)
        {
            return result;
        }
    }
    return 0;
}

inline auto appendLevel(std::vector<std::uint32_t> &key, const std::vector<CollationElement> &elements,
                        int level) -> void
{
    for (const CollationElement &element : elements)
    {
        const std::uint32_t weight = weightAt(element, level);
        if (weight != 0)
        {
            key.push_back(weight);
        }
    }
    key.push_back(0); // level separator
}

inline auto buildSortKey(const std::vector<CollationElement> &elements, CollationStrength strength)
    -> std::vector<std::uint32_t>
{
    std::vector<std::uint32_t> key;
    key.reserve(elements.size() * 3 + 3);
    const int levels = levelCount(strength);
    for (int level = 0; level < levels; ++level)
    {
        appendLevel(key, elements, level);
    }
    return key;
}

} // namespace collation
} // namespace detail

// ---------------------------------------------------------------------------
// Collator: locale-bound collation at primary, secondary or tertiary strength.
// ---------------------------------------------------------------------------

class Collator
{
  public:
    explicit Collator(const Locale &locale = Locale())
        : locale_(locale), tailoring_(detail::collation::tailoringFor(locale))
    {
    }

    auto compare(std::string_view left, std::string_view right,
                 CollationStrength strength = CollationStrength::tertiary) const -> int
    {
        return detail::collation::compareElements(elementArray(left), elementArray(right), strength);
    }

    auto less(std::string_view left, std::string_view right,
              CollationStrength strength = CollationStrength::tertiary) const -> bool
    {
        return compare(left, right, strength) < 0;
    }

    auto equal(std::string_view left, std::string_view right,
               CollationStrength strength = CollationStrength::tertiary) const -> bool
    {
        return compare(left, right, strength) == 0;
    }

    auto sortKey(std::string_view text,
                 CollationStrength strength = CollationStrength::tertiary) const -> std::vector<std::uint32_t>
    {
        return detail::collation::buildSortKey(elementArray(text), strength);
    }

    auto locale() const noexcept -> const Locale & { return locale_; }

    // True when the locale actually changed the root order.
    auto tailored() const noexcept -> bool
    {
        return tailoring_.swedish || tailoring_.hasRules();
    }

  private:
    auto elementArray(std::string_view text) const -> std::vector<detail::collation::CollationElement>
    {
        return detail::collation::collationElements(text, tailoring_);
    }

    Locale locale_;
    detail::collation::Tailoring tailoring_;
};

// Convenience wrapper for one-off comparisons.
inline auto collate(std::string_view left, std::string_view right, const Locale &locale = Locale()) -> int
{
    return Collator(locale).compare(left, right);
}

} // namespace localisation
