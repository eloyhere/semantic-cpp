#pragma once

// ===========================================================================
// bunch/caseless.h
//
// NFKC_Casefold (UAX #15) and caseless comparison.
//
// NFKC_Casefold is the canonical form for comparing text without case.  UAX
// #15 and DerivedNormalizationProps.txt define it as the result of applying
// NFKC, the full case folding (CaseFolding.txt statuses C + F) and the removal
// of every Default_Ignorable_Code_Point, repeated until the result is stable.
// Text that differs only in case, in compatibility variants (fullwidth,
// circled, squared, mathematical alphabets, ligatures) or in default-ignorable
// characters therefore folds to identical bytes:
//
//     "Stra" U+00DF "e" -> "strasse"    ("STRASSE" folds to the same)
//     U+1E9E            -> "ss"         (LATIN CAPITAL LETTER SHARP S)
//     U+FB00            -> "ff"         (LATIN SMALL LIGATURE FF)
//     U+24B6            -> "a"          (CIRCLED LATIN CAPITAL LETTER A)
//     U+FF21            -> "a"          (FULLWIDTH LATIN CAPITAL LETTER A)
//     U+1D400           -> "a"          (MATHEMATICAL BOLD CAPITAL A)
//     U+2160            -> "i"          (ROMAN NUMERAL ONE)
//     U+0130            -> "i" + U+0307 (CAPITAL I WITH DOT ABOVE)
//     U+00AD            -> deleted      (SOFT HYPHEN)
//
// The per-code-point mappings live in caseless_data.h and are generated from
// Python's Unicode database plus the Default_Ignorable_Code_Point ranges of
// DerivedCoreProperties.txt; they are exactly the published NFKC_CF values
// (the generator cross-checks all 10,491 of them).  Applying the mapping per
// code point is not enough on its own, because characters interact across
// code point boundaries, so nfkcCasefold() normalises the whole string to
// NFKC afterwards: "e" + U+0301 COMBINING ACUTE ACCENT composes to U+00E9.
//
// The language tag only selects the Turkic tailoring, exactly as in
// casefold(): "tr" and "az" map U+0049 LATIN CAPITAL LETTER I (and the
// compatibility characters whose NFKC form is "I", such as U+FF29 or
// U+2160) to U+0131 LATIN SMALL LETTER DOTLESS I, and U+0130 LATIN CAPITAL
// LETTER I WITH DOT ABOVE to plain "i".  Every other language, including the
// empty default, uses the language-neutral mapping.
//
// Text enters and leaves as UTF-8.  Ill-formed input is replaced with U+FFFD
// under the same maximal-subpart rules as the rest of the bunch codec, so no
// function reads past the end of the buffer.  The empty string folds to the
// empty string, and folding is idempotent.
//
// This header is not a UTS #46 (IDNA) implementation.  It performs no
// transitional (IDNA2003) handling, no deviation-character mapping beyond the
// ordinary fold, no punycode conversion, and no label, length or Bidi rule
// validation; use it as the comparison layer of such a profile, never as a
// replacement for one.  Default-ignorable removal is unconditional and meant
// for comparison; it is not sanitising for display, so BIDI control characters
// and other invisibles disappear from the folded form by design.
// ===========================================================================

#include "caseless_data.h"
#include "casefold.h"
#include "unicode.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace bunch
{
namespace detail
{
// Appends the NFKC_Casefold mapping of one code point.  The Turkic tailoring
// is a separate, much smaller table because it only affects the code points
// whose NFKC form is "I" or U+0130.
inline auto appendNfkcCasefolded(std::string &output, std::uint32_t codepoint, bool turkic) -> void
{
    if (turkic)
    {
        const unicode::CaselessTurkicEntry *turkicEntry = unicode::findCaselessTurkic(codepoint);
        if (turkicEntry != nullptr)
        {
            appendUtf8(output, turkicEntry->replacement);
            return;
        }
    }

    const unicode::CaselessEntry *entry = unicode::findCaseless(codepoint);
    if (entry == nullptr)
    {
        appendUtf8(output, codepoint);
        return;
    }
    for (std::uint32_t index = 0; index < entry->length; ++index)
    {
        appendUtf8(output, unicode::kCaselessPool[entry->offset + index]);
    }
}

inline auto isAsciiText(std::string_view text) -> bool
{
    for (const char character : text)
    {
        if (static_cast<unsigned char>(character) >= 0x80)
        {
            return false;
        }
    }
    return true;
}

// ASCII has no compatibility decompositions and no default-ignorable
// characters, so its NFKC_Casefold mapping is just A-Z to a-z.
inline auto appendAsciiCaseFold(std::string &output, std::string_view text) -> void
{
    for (const char character : text)
    {
        const bool uppercase = character >= 'A' && character <= 'Z';
        output.push_back(uppercase ? static_cast<char>(character - 'A' + 'a') : character);
    }
}

inline auto foldAsciiCharacter(char character) -> char
{
    const bool uppercase = character >= 'A' && character <= 'Z';
    return uppercase ? static_cast<char>(character - 'A' + 'a') : character;
}

inline auto asciiCaselessEqual(std::string_view left, std::string_view right) -> bool
{
    if (left.size() != right.size())
    {
        return false;
    }
    for (std::size_t index = 0; index < left.size(); ++index)
    {
        if (foldAsciiCharacter(left[index]) != foldAsciiCharacter(right[index]))
        {
            return false;
        }
    }
    return true;
}
} // namespace detail

// Folds text to its NFKC_Casefold form, the form used for caseless comparison.
// The result is NFKC-normalised, free of Default_Ignorable_Code_Point
// characters, and stable: folding it again returns the same bytes.
inline auto nfkcCasefold(std::string_view text, std::string_view language = "") -> std::string
{
    const bool turkic = detail::isTurkicCaseFoldLanguage(language);

    std::string output;
    output.reserve(text.size() + text.size() / 4 + 4);

    // Fast path: nothing in ASCII needs compatibility decomposition or
    // ignorable removal, so the mapping is a straight A-Z to a-z fold.  The
    // Turkic rules can produce U+0131, so they take the general path.
    if (!turkic && detail::isAsciiText(text))
    {
        detail::appendAsciiCaseFold(output, text);
        return output;
    }

    std::string_view remaining = text;
    while (!remaining.empty())
    {
        detail::appendNfkcCasefolded(output, decode(remaining, charset::utf8), turkic);
    }
    if (output.empty())
    {
        return output;
    }
    return nfkc(output);
}

// Compares two strings for caseless equality by folding both to NFKC_Casefold.
// The relation is symmetric and reflexive; it is a plain code point sequence
// comparison, not a collation, so it does not order text.
inline auto caselessMatch(std::string_view left, std::string_view right) -> bool
{
    if (left == right)
    {
        return true;
    }
    if (detail::isAsciiText(left) && detail::isAsciiText(right))
    {
        return detail::asciiCaselessEqual(left, right);
    }
    return nfkcCasefold(left) == nfkcCasefold(right);
}

// True when text is already in NFKC_Casefold form, i.e. when folding it again
// would not change it.  Ill-formed UTF-8 is replaced with U+FFFD by the fold,
// so text containing it is never considered folded.
inline auto isNfkcCasefolded(std::string_view text) -> bool
{
    const std::string folded = nfkcCasefold(text);
    return std::string_view(folded) == text;
}
} // namespace bunch
