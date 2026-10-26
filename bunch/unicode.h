#pragma once

// ===========================================================================
// bunch/unicode.h
//
// Unicode character processing for the bunch module: NFC/NFD/NFKC/NFKD
// normalisation and case mapping (lowercase, uppercase, title case).
//
// The data tables live in unicode_data.h and are generated from Unicode 15.0
// by tools/gen_unicode.py.  Code point iteration and UTF-8 encoding are shared
// with encode.h, so this header adds no second codec.
//
// The API is locale-independent by design: the language tag is passed only
// where Unicode's special casing needs it (Turkish/Azerbaijani i, Greek final
// sigma, Dutch IJ).  localisation/localisation.h wraps these functions with its
// Locale type.
// ===========================================================================

#include "charset.h"
#include "encode.h"
#include "unicode_data.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace bunch
{
// ---------------------------------------------------------------------------
// Normalisation
// ---------------------------------------------------------------------------

enum class NormalizationForm
{
    none,
    nfc,
    nfd,
    nfkc,
    nfkd
};

inline const char *normalizationFormName(NormalizationForm form)
{
    switch (form)
    {
    case NormalizationForm::none:
        return "none";
    case NormalizationForm::nfc:
        return "nfc";
    case NormalizationForm::nfd:
        return "nfd";
    case NormalizationForm::nfkc:
        return "nfkc";
    case NormalizationForm::nfkd:
        return "nfkd";
    }
    return "none";
}

namespace detail
{
inline void decomposeCodepoint(std::uint32_t codepoint, bool compatibility, std::vector<std::uint32_t> &output)
{
    // Hangul syllables decompose algorithmically; they are not in the table.
    if (codepoint >= 0xAC00 && codepoint <= 0xD7A3)
    {
        const std::uint32_t index = codepoint - 0xAC00;
        output.push_back(0x1100 + index / 588);
        output.push_back(0x1161 + (index % 588) / 28);
        if (index % 28 != 0)
        {
            output.push_back(0x11A7 + index % 28);
        }
        return;
    }

    const unicode::Decomposition *entry = unicode::findDecomposition(codepoint);
    if (entry != nullptr && (compatibility || entry->compatibility == 0))
    {
        for (std::uint32_t index = 0; index < entry->length; ++index)
        {
            decomposeCodepoint(unicode::kDecompositionPool[entry->offset + index], compatibility, output);
        }
        return;
    }
    output.push_back(codepoint);
}

inline void orderCombiningMarks(std::vector<std::uint32_t> &codepoints)
{
    std::size_t index = 0;
    while (index < codepoints.size())
    {
        if (unicode::combiningClass(codepoints[index]) == 0)
        {
            ++index;
            continue;
        }
        std::size_t end = index;
        while (end < codepoints.size() && unicode::combiningClass(codepoints[end]) != 0)
        {
            ++end;
        }
        // Stable insertion sort: marks with equal class keep their relative
        // order, which is what canonical ordering requires.
        for (std::size_t current = index + 1; current < end; ++current)
        {
            const std::uint32_t value = codepoints[current];
            const std::uint8_t valueClass = unicode::combiningClass(value);
            std::size_t position = current;
            while (position > index && unicode::combiningClass(codepoints[position - 1]) > valueClass)
            {
                codepoints[position] = codepoints[position - 1];
                --position;
            }
            codepoints[position] = value;
        }
        index = end;
    }
}

inline bool composeHangul(std::uint32_t first, std::uint32_t second, std::uint32_t &composed)
{
    if (first >= 0x1100 && first <= 0x1112 && second >= 0x1161 && second <= 0x1175)
    {
        composed = 0xAC00 + ((first - 0x1100) * 21 + (second - 0x1161)) * 28;
        return true;
    }
    if (first >= 0xAC00 && first <= 0xD7A3 && (first - 0xAC00) % 28 == 0 && second >= 0x11A8 && second <= 0x11C2)
    {
        composed = first + (second - 0x11A7);
        return true;
    }
    return false;
}

inline std::vector<std::uint32_t> composeCodepoints(const std::vector<std::uint32_t> &codepoints)
{
    std::vector<std::uint32_t> result;
    result.reserve(codepoints.size());
    if (codepoints.empty())
    {
        return result;
    }

    result.push_back(codepoints[0]);
    std::size_t starterPosition = 0;
    std::uint32_t starter = codepoints[0];
    int lastClass = unicode::combiningClass(starter);
    if (lastClass != 0)
    {
        lastClass = 256; // a leading non-starter never absorbs a following mark
    }

    for (std::size_t index = 1; index < codepoints.size(); ++index)
    {
        const std::uint32_t current = codepoints[index];
        const std::uint8_t currentClass = unicode::combiningClass(current);
        const bool canCompose = lastClass < static_cast<int>(currentClass) || lastClass == 0;

        std::uint32_t composed = 0;
        if (canCompose && composeHangul(starter, current, composed))
        {
            result[starterPosition] = composed;
            starter = composed;
            continue;
        }
        if (canCompose && unicode::compose(starter, current, composed))
        {
            result[starterPosition] = composed;
            starter = composed;
            continue;
        }

        if (currentClass == 0)
        {
            starterPosition = result.size();
            starter = current;
        }
        lastClass = currentClass;
        result.push_back(current);
    }
    return result;
}

inline std::vector<std::uint32_t> toCodepoints(std::string_view text)
{
    std::vector<std::uint32_t> codepoints;
    codepoints.reserve(text.size());
    std::string_view remaining = text;
    while (!remaining.empty())
    {
        codepoints.push_back(decode(remaining, charset::utf8));
    }
    return codepoints;
}
} // namespace detail

inline std::string normalize(std::string_view text, NormalizationForm form)
{
    if (form == NormalizationForm::none)
    {
        return std::string(text);
    }

    const bool compatibility = form == NormalizationForm::nfkc || form == NormalizationForm::nfkd;
    const bool compose = form == NormalizationForm::nfc || form == NormalizationForm::nfkc;

    std::vector<std::uint32_t> codepoints;
    codepoints.reserve(text.size());
    std::size_t position = 0;
    while (position < text.size())
    {
        std::string_view remaining = text.substr(position);
        const unsigned int codepoint = decode(remaining, charset::utf8);
        position = text.size() - remaining.size();
        detail::decomposeCodepoint(codepoint, compatibility, codepoints);
    }
    detail::orderCombiningMarks(codepoints);
    if (compose)
    {
        codepoints = detail::composeCodepoints(codepoints);
    }

    std::string output;
    output.reserve(text.size());
    for (std::uint32_t value : codepoints)
    {
        appendUtf8(output, value);
    }
    return output;
}

inline std::string nfc(std::string_view text) { return normalize(text, NormalizationForm::nfc); }
inline std::string nfd(std::string_view text) { return normalize(text, NormalizationForm::nfd); }
inline std::string nfkc(std::string_view text) { return normalize(text, NormalizationForm::nfkc); }
inline std::string nfkd(std::string_view text) { return normalize(text, NormalizationForm::nfkd); }

inline bool isNormalized(std::string_view text, NormalizationForm form)
{
    return form == NormalizationForm::none || normalize(text, form) == text;
}

// ---------------------------------------------------------------------------
// Text styles: locale-aware case mapping and title casing
//
// Explicitly handled Unicode special cases: Turkish/Azerbaijani dotted and
// dotless i, German sharp s expansion, Greek final sigma, Dutch IJ in title
// case, and apostrophes inside words ("don't" -> "Don't", not "Don'T").
// ---------------------------------------------------------------------------

enum class TextStyle
{
    plain,
    lowercase,
    uppercase,
    capitalize, // sentence case: first cased character upper, rest lower
    title       // first cased character of each word upper, rest lower
};

inline const char *textStyleName(TextStyle style)
{
    switch (style)
    {
    case TextStyle::plain:
        return "plain";
    case TextStyle::lowercase:
        return "lowercase";
    case TextStyle::uppercase:
        return "uppercase";
    case TextStyle::capitalize:
        return "capitalize";
    case TextStyle::title:
        return "title";
    }
    return "plain";
}

namespace detail
{
inline bool isTurkicLanguage(std::string_view language) { return language == "tr" || language == "az"; }

inline void appendCaseMapping(std::string &output, const unicode::CaseMapping *mapping, const std::uint32_t *pool)
{
    for (std::uint32_t index = 0; index < mapping->length; ++index)
    {
        appendUtf8(output, pool[mapping->offset + index]);
    }
}

inline void appendLowercase(std::string &output, std::uint32_t codepoint, bool turkic)
{
    if (turkic)
    {
        if (codepoint == 'I')
        {
            appendUtf8(output, 0x0131); // dotless i
            return;
        }
        if (codepoint == 0x0130)
        {
            appendUtf8(output, 'i');
            return;
        }
    }
    const unicode::CaseMapping *mapping =
        unicode::findCaseMapping(unicode::kLowercaseMappings, unicode::kLowercaseCount, codepoint);
    if (mapping != nullptr)
    {
        appendCaseMapping(output, mapping, unicode::kLowercasePool);
    }
    else
    {
        appendUtf8(output, codepoint);
    }
}

inline void appendUppercase(std::string &output, std::uint32_t codepoint, bool turkic)
{
    if (turkic)
    {
        if (codepoint == 'i')
        {
            appendUtf8(output, 0x0130); // dotted capital I
            return;
        }
        if (codepoint == 0x0131)
        {
            appendUtf8(output, 'I');
            return;
        }
    }
    const unicode::CaseMapping *mapping =
        unicode::findCaseMapping(unicode::kUppercaseMappings, unicode::kUppercaseCount, codepoint);
    if (mapping != nullptr)
    {
        appendCaseMapping(output, mapping, unicode::kUppercasePool);
    }
    else
    {
        appendUtf8(output, codepoint);
    }
}

inline void appendTitlecase(std::string &output, std::uint32_t codepoint, bool turkic)
{
    if (turkic)
    {
        if (codepoint == 'i')
        {
            appendUtf8(output, 0x0130);
            return;
        }
        if (codepoint == 0x0131)
        {
            appendUtf8(output, 'I');
            return;
        }
    }
    const unicode::CaseMapping *mapping =
        unicode::findCaseMapping(unicode::kTitlecaseMappings, unicode::kTitlecaseCount, codepoint);
    if (mapping != nullptr)
    {
        appendCaseMapping(output, mapping, unicode::kTitlecasePool);
    }
    else
    {
        appendUtf8(output, codepoint);
    }
}

inline bool isFinalSigma(const std::vector<std::uint32_t> &codepoints, std::size_t index)
{
    // Final_Sigma in Unicode SpecialCasing: there must be a preceding cased
    // letter (through case-ignorable characters) and no following cased letter.
    bool precededByCased = false;
    for (std::size_t before = index; before > 0;)
    {
        --before;
        if (unicode::isCased(codepoints[before]))
        {
            precededByCased = true;
            break;
        }
        if (!unicode::isWord(codepoints[before]))
        {
            break;
        }
    }
    if (!precededByCased)
    {
        return false;
    }
    for (std::size_t next = index + 1; next < codepoints.size(); ++next)
    {
        if (unicode::isCased(codepoints[next]))
        {
            return false;
        }
        if (!unicode::isWord(codepoints[next]))
        {
            return true;
        }
    }
    return true;
}

inline void appendLowercaseAt(std::string &output, const std::vector<std::uint32_t> &codepoints, std::size_t index,
                              bool turkic)
{
    if (codepoints[index] == 0x03A3) // Greek capital sigma: final or medial
    {
        appendUtf8(output, isFinalSigma(codepoints, index) ? 0x03C2 : 0x03C3);
        return;
    }
    appendLowercase(output, codepoints[index], turkic);
}
} // namespace detail

inline std::string formatText(std::string_view text, std::string_view language, TextStyle style)
{
    if (style == TextStyle::plain)
    {
        return std::string(text);
    }

    const std::vector<std::uint32_t> codepoints = detail::toCodepoints(text);
    const bool turkic = detail::isTurkicLanguage(language);
    const bool dutch = language == "nl";
    std::string output;
    output.reserve(text.size() + text.size() / 4);

    if (style == TextStyle::lowercase)
    {
        for (std::size_t index = 0; index < codepoints.size(); ++index)
        {
            detail::appendLowercaseAt(output, codepoints, index, turkic);
        }
        return output;
    }

    if (style == TextStyle::uppercase)
    {
        for (std::uint32_t codepoint : codepoints)
        {
            detail::appendUppercase(output, codepoint, turkic);
        }
        return output;
    }

    if (style == TextStyle::capitalize)
    {
        bool firstCased = true;
        for (std::size_t index = 0; index < codepoints.size(); ++index)
        {
            if (firstCased && detail::unicode::isCased(codepoints[index]))
            {
                detail::appendUppercase(output, codepoints[index], turkic);
                firstCased = false;
            }
            else
            {
                detail::appendLowercaseAt(output, codepoints, index, turkic);
            }
        }
        return output;
    }

    // TextStyle::title
    bool wordStart = true;
    for (std::size_t index = 0; index < codepoints.size(); ++index)
    {
        const std::uint32_t codepoint = codepoints[index];
        if (!detail::unicode::isWord(codepoint))
        {
            // Cased symbols such as circled letters (U+24D0) still title-case.
            if (wordStart && detail::unicode::isCased(codepoint))
            {
                detail::appendTitlecase(output, codepoint, turkic);
                wordStart = false;
            }
            else
            {
                appendUtf8(output, codepoint);
                wordStart = true;
            }
            continue;
        }
        if (wordStart)
        {
            // Dutch treats "ij" as a single title-case unit: IJssel.
            if (dutch && (codepoint == 'i' || codepoint == 'I') && index + 1 < codepoints.size() &&
                (codepoints[index + 1] == 'j' || codepoints[index + 1] == 'J'))
            {
                detail::appendUppercase(output, 'I', turkic);
                detail::appendUppercase(output, 'J', turkic);
                ++index;
            }
            else
            {
                detail::appendTitlecase(output, codepoint, turkic);
            }
            wordStart = false;
        }
        else
        {
            detail::appendLowercaseAt(output, codepoints, index, turkic);
        }
    }
    return output;
}

inline std::string toLowerCase(std::string_view text, std::string_view language)
{
    return formatText(text, language, TextStyle::lowercase);
}

inline std::string toUpperCase(std::string_view text, std::string_view language)
{
    return formatText(text, language, TextStyle::uppercase);
}

inline std::string toTitleCase(std::string_view text, std::string_view language)
{
    return formatText(text, language, TextStyle::title);
}
} // namespace bunch
