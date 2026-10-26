#pragma once

// ===========================================================================
// localisation/localisation.h
//
// Message localisation for Semantic-Cpp, with no dependencies beyond the C++
// standard library.  The header is deliberately stateless: there is no
// process-wide "current locale", every operation takes the Locale it should
// use.  That keeps request handling thread-safe and makes locale switching a
// value-level decision instead of a global side effect.
//
// The design is informed by the strengths and the recurring pain points of
// internationalisation in other ecosystems:
//
//   * Key-based catalogues with a fallback chain (Java ResourceBundle,
//     .NET ResourceManager) but without silent "missing resource" failures:
//     an unresolved key falls back to the source language, then to the key
//     itself, and an optional handler can report it.
//   * ICU MessageFormat-style messages, so translators can reorder arguments,
//     and so plural/select branches live in one string instead of being
//     assembled with string concatenation (Java MessageFormat, FormatJS).
//   * CLDR plural categories (zero/one/two/few/many/other) for cardinal and
//     ordinal numbers, data-driven through CLDR plural rule expressions
//     (i18next v4, ICU PluralRules).  `count == 1 ? singular : plural` is a
//     bug in most of the world's languages.
//   * msgctxt-style disambiguation of identical keys ("Open" the verb and
//     "Open" the file menu) without stuffing context into the key name.
//   * BCP 47 language tag parsing, RFC 4647 lookup (fr-CA -> fr), and
//     Accept-Language parsing (RFC 7231) for server-side negotiation.
//   * Locale-aware number rendering (grouping, decimal separator, native
//     digits) instead of printf's fixed layout.
//   * Right-to-left detection at the script level, not just the language,
//     plus top-to-bottom direction for the Traditional Mongolian script.
//   * Coverage for the CJK and Central-Asian locales that usually expose the
//     rough edges: Simplified/Traditional Chinese and their CN/SG/TW/HK/MO
//     variants, Cantonese (yue), Mongolian in both Cyrillic and Mongolian
//     script, Arabic, Tibetan (bo), Dzongkha (dz) and Uyghur (ug).  Scripts
//     are inferred from the region (zh-TW -> zh-Hant-TW, mn-CN -> mn-Mong-CN)
//     so that Traditional and Simplified catalogues fall back correctly.
//   * Patterns are parsed when a message is added to a catalogue, so a broken
//     translation fails at load time with an offset, not in production.
//   * Character-level processing is not duplicated here: UTF-8 iteration,
//     NFC/NFD/NFKC/NFKD normalisation and case mapping live in the bunch
//     module (bunch/unicode.h) and are re-exported with Locale-aware wrappers.
//
// Example:
//
//   localisation::Catalog catalog("en");
//   catalog.add(localisation::Locale("en"), "cart.items",
//               "{count, plural, =0 {Your cart is empty} one {# item} other {# items}}");
//   catalog.add(localisation::Locale("de"), "cart.items",
//               "{count, plural, =0 {Ihr Warenkorb ist leer} one {# Artikel} other {# Artikel}}");
//
//   std::cout << catalog.translate(localisation::Locale("de-AT"), "cart.items",
//                                  localisation::Arguments().set("count", 3));
//
// Everything is UTF-8 in and UTF-8 out.  Formatting covers numbers, plurals,
// ordinals, selections, Unicode normalisation and text styles; dates, times,
// currencies and collation are left to the host platform.
// ===========================================================================

#include "../bunch/charset.h"
#include "../bunch/encode.h"
#include "../bunch/unicode.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <istream>
#include <locale>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace localisation
{
namespace detail
{
inline bool isSpace(char character)
{
    return character == ' ' || character == '\t' || character == '\r' || character == '\n' || character == '\f' ||
           character == '\v';
}

inline bool isDigit(char character) { return character >= '0' && character <= '9'; }

inline bool isAlpha(char character)
{
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z');
}

inline bool isAlnum(char character) { return isAlpha(character) || isDigit(character); }

inline char toLower(char character)
{
    return (character >= 'A' && character <= 'Z') ? static_cast<char>(character - 'A' + 'a') : character;
}

inline char toUpper(char character)
{
    return (character >= 'a' && character <= 'z') ? static_cast<char>(character - 'a' + 'A') : character;
}

inline std::string asciiLower(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    for (char character : text)
    {
        result.push_back(toLower(character));
    }
    return result;
}

inline std::string asciiUpper(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    for (char character : text)
    {
        result.push_back(toUpper(character));
    }
    return result;
}

inline std::string asciiTitle(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    bool first = true;
    for (char character : text)
    {
        result.push_back(first ? toUpper(character) : toLower(character));
        first = false;
    }
    return result;
}

inline std::string_view trimView(std::string_view text)
{
    std::size_t begin = 0;
    while (begin < text.size() && isSpace(text[begin]))
    {
        ++begin;
    }
    std::size_t end = text.size();
    while (end > begin && isSpace(text[end - 1]))
    {
        --end;
    }
    return text.substr(begin, end - begin);
}

inline std::string trim(std::string_view text) { return std::string(trimView(text)); }

inline void skipSpaces(std::string_view text, std::size_t &position)
{
    while (position < text.size() && isSpace(text[position]))
    {
        ++position;
    }
}

inline double parseDecimal(std::string_view text)
{
    std::string copy(text);
    char *end = nullptr;
    const double value = std::strtod(copy.c_str(), &end);
    if (end == copy.c_str() || *end != '\0')
    {
        throw std::invalid_argument("localisation: '" + copy + "' is not a decimal number");
    }
    return value;
}

inline int hexValue(char character)
{
    if (character >= '0' && character <= '9')
    {
        return character - '0';
    }
    if (character >= 'a' && character <= 'f')
    {
        return character - 'a' + 10;
    }
    if (character >= 'A' && character <= 'F')
    {
        return character - 'A' + 10;
    }
    return -1;
}
} // namespace detail

// ---------------------------------------------------------------------------
// Locale: a BCP 47 subset (language[-script][-region])
//
// Canonicalisation lower-cases the language, title-cases the script and
// upper-cases the region.  Likely scripts are filled in where they change
// behaviour: zh-CN/zh-SG -> Hans, zh-TW/zh-HK/zh-MO -> Hant, yue -> Hant,
// mn-CN -> Mong and mn-MN -> Cyrl.
// ---------------------------------------------------------------------------

class Locale
{
  public:
    Locale() : tag_("und"), language_("und") {}

    explicit Locale(std::string_view tag)
    {
        std::vector<std::string> subtags;
        std::string current;
        for (char character : tag)
        {
            if (character == '-' || character == '_')
            {
                if (!current.empty())
                {
                    subtags.push_back(current);
                    current.clear();
                }
            }
            else
            {
                current.push_back(character);
            }
        }
        if (!current.empty())
        {
            subtags.push_back(current);
        }

        std::size_t index = 0;
        if (index < subtags.size() && isLanguageSubtag(subtags[index]))
        {
            language_ = detail::asciiLower(subtags[index]);
            ++index;
        }
        // BCP 47 extlang form: zh-yue is the Cantonese tag used by some
        // systems; canonicalise it to yue.
        if (language_ == "zh" && index < subtags.size() && detail::asciiLower(subtags[index]) == "yue")
        {
            language_ = "yue";
            ++index;
        }
        if (index < subtags.size() && isScriptSubtag(subtags[index]))
        {
            script_ = detail::asciiTitle(subtags[index]);
            ++index;
        }
        if (index < subtags.size() && isRegionSubtag(subtags[index]))
        {
            region_ = detail::asciiUpper(subtags[index]);
            ++index;
        }
        // Variants, extensions and private-use subtags are intentionally
        // ignored: they do not affect message lookup or plural selection.
        if (language_.empty())
        {
            language_ = "und";
        }
        applyLikelyScript();
        recompose();
    }

    Locale(std::string_view language, std::string_view script, std::string_view region)
    {
        language_ = detail::asciiLower(detail::trimView(language));
        script_ = detail::asciiTitle(detail::trimView(script));
        region_ = detail::asciiUpper(detail::trimView(region));
        if (language_.empty())
        {
            language_ = "und";
        }
        applyLikelyScript();
        recompose();
    }

    const std::string &tag() const noexcept { return tag_; }
    const std::string &language() const noexcept { return language_; }
    const std::string &script() const noexcept { return script_; }
    const std::string &region() const noexcept { return region_; }

    bool specified() const noexcept { return language_ != "und" && !language_.empty(); }

    // RFC 4647 lookup chain: zh-Hant-TW -> zh-Hant -> zh.
    std::vector<Locale> fallbacks() const
    {
        std::vector<Locale> result;
        result.push_back(*this);
        if (!region_.empty())
        {
            result.push_back(raw(language_, script_, std::string()));
        }
        if (!script_.empty())
        {
            result.push_back(raw(language_, std::string(), std::string()));
        }
        return result;
    }

    bool operator==(const Locale &other) const noexcept { return tag_ == other.tag_; }
    bool operator!=(const Locale &other) const noexcept { return tag_ != other.tag_; }
    bool operator<(const Locale &other) const noexcept { return tag_ < other.tag_; }

  private:
    // Builds a locale without running likely-script inference.  The fallback
    // chain must be able to produce the bare language tag (yue -> yue), which
    // inference would otherwise turn back into yue-Hant.
    static Locale raw(const std::string &language, const std::string &script, const std::string &region)
    {
        Locale result;
        result.language_ = language;
        result.script_ = script;
        result.region_ = region;
        result.recompose();
        return result;
    }

    // CLDR likely-script inference for the scripts that change catalogue
    // selection or writing direction.  Only the script subtag is added, so a
    // generic "zh" catalogue still matches every Chinese variant:
    // zh-TW -> zh-Hant-TW, zh-CN -> zh-Hans-CN, mn-CN -> mn-Mong-CN.
    void applyLikelyScript()
    {
        if (!script_.empty())
        {
            return;
        }
        if (language_ == "zh")
        {
            if (region_ == "TW" || region_ == "HK" || region_ == "MO")
            {
                script_ = "Hant";
            }
            else if (region_ == "CN" || region_ == "SG" || region_ == "MY")
            {
                script_ = "Hans";
            }
        }
        else if (language_ == "yue")
        {
            script_ = "Hant";
        }
        else if (language_ == "mn")
        {
            if (region_ == "CN")
            {
                script_ = "Mong";
            }
            else if (region_ == "MN")
            {
                script_ = "Cyrl";
            }
        }
    }

    static bool isLanguageSubtag(const std::string &subtag)
    {
        if (subtag.empty() || subtag.size() > 8)
        {
            return false;
        }
        for (char character : subtag)
        {
            if (!detail::isAlpha(character))
            {
                return false;
            }
        }
        return true;
    }

    static bool isScriptSubtag(const std::string &subtag)
    {
        if (subtag.size() != 4)
        {
            return false;
        }
        for (char character : subtag)
        {
            if (!detail::isAlpha(character))
            {
                return false;
            }
        }
        return true;
    }

    static bool isRegionSubtag(const std::string &subtag)
    {
        if (subtag.size() == 2)
        {
            return detail::isAlpha(subtag[0]) && detail::isAlpha(subtag[1]);
        }
        if (subtag.size() == 3)
        {
            return detail::isDigit(subtag[0]) && detail::isDigit(subtag[1]) && detail::isDigit(subtag[2]);
        }
        return false;
    }

    void recompose()
    {
        tag_ = language_;
        if (!script_.empty())
        {
            tag_ += '-';
            tag_ += script_;
        }
        if (!region_.empty())
        {
            tag_ += '-';
            tag_ += region_;
        }
    }

    std::string tag_;
    std::string language_;
    std::string script_;
    std::string region_;
};

inline std::ostream &operator<<(std::ostream &stream, const Locale &locale) { return stream << locale.tag(); }

// RFC 4647 lookup.  Requested locales are tried in order, each through its
// fallback chain, against the list of available tags.
inline Locale negotiate(const std::vector<Locale> &requested, const std::vector<Locale> &available,
                        const Locale &fallback = Locale("en"))
{
    for (const Locale &candidate : requested)
    {
        for (const Locale &variant : candidate.fallbacks())
        {
            for (const Locale &option : available)
            {
                if (option == variant)
                {
                    return option;
                }
            }
        }
    }
    return fallback;
}

inline Locale negotiate(const Locale &requested, const std::vector<Locale> &available,
                        const Locale &fallback = Locale("en"))
{
    return negotiate(std::vector<Locale>{requested}, available, fallback);
}

// Parses an Accept-Language header into locales ordered by descending quality.
// Wildcards and malformed tags are skipped; ties keep the header order.
inline std::vector<Locale> parseAcceptLanguage(std::string_view header)
{
    struct Weighted
    {
        Locale locale;
        double quality;
    };

    std::vector<Weighted> weighted;
    std::size_t start = 0;
    while (start <= header.size())
    {
        const std::size_t comma = header.find(',', start);
        const std::string_view item =
            header.substr(start, comma == std::string_view::npos ? std::string_view::npos : comma - start);
        std::string_view range = detail::trimView(item);

        double quality = 1.0;
        const std::size_t semicolon = range.find(';');
        if (semicolon != std::string_view::npos)
        {
            std::string_view parameters = range.substr(semicolon + 1);
            range = detail::trimView(range.substr(0, semicolon));
            std::size_t parameterStart = 0;
            while (parameterStart <= parameters.size())
            {
                const std::size_t parameterEnd = parameters.find(';', parameterStart);
                const std::string_view parameter =
                    detail::trimView(parameters.substr(parameterStart, parameterEnd == std::string_view::npos
                                                                           ? std::string_view::npos
                                                                           : parameterEnd - parameterStart));
                if (parameter.size() > 2 && (parameter[0] == 'q' || parameter[0] == 'Q') && parameter[1] == '=')
                {
                    std::istringstream stream{std::string(parameter.substr(2))};
                    stream.imbue(std::locale::classic());
                    double parsed = 0.0;
                    stream >> parsed;
                    if (stream.fail())
                    {
                        parsed = 0.0;
                    }
                    quality = parsed;
                }
                if (parameterEnd == std::string_view::npos)
                {
                    break;
                }
                parameterStart = parameterEnd + 1;
            }
        }

        if (!range.empty() && range != "*" && quality > 0.0)
        {
            Locale locale(range);
            if (locale.specified())
            {
                weighted.push_back(Weighted{locale, quality});
            }
        }

        if (comma == std::string_view::npos)
        {
            break;
        }
        start = comma + 1;
    }

    std::stable_sort(weighted.begin(), weighted.end(), [](const Weighted &left, const Weighted &right) {
        return left.quality > right.quality;
    });

    std::vector<Locale> result;
    result.reserve(weighted.size());
    for (const Weighted &entry : weighted)
    {
        result.push_back(entry.locale);
    }
    return result;
}

// ---------------------------------------------------------------------------
// Numbers: locale-aware grouping, separators and digits
// ---------------------------------------------------------------------------

struct NumberFormat
{
    int minimumFractionDigits = 0;
    int maximumFractionDigits = 3;
    bool grouping = true;
    bool percent = false;
    // Ask for the locale's native numbering system when it is not the CLDR
    // default (Tibetan digits for bo/dz, hanidec digits for zh/yue).
    bool nativeDigits = false;
};

namespace detail
{
struct NumberSymbols
{
    const char *group = ",";
    const char *decimal = ".";
    const char *digits = nullptr; // ten UTF-8 code points, or nullptr for 0-9
    int primaryGrouping = 3;
    int secondaryGrouping = 0; // 0 means "same as primary"
    const char *percent = "%";
    bool percentSpace = false;
    bool percentBefore = false;
};

struct NumberEntry
{
    const char *tag;
    NumberSymbols symbols;
};

// Separators and digits follow CLDR.  A locale whose tag and language are not
// listed uses the neutral default above (ASCII digits, comma grouping).
inline const NumberSymbols &numberSymbols(const Locale &locale)
{
    static const char arabicDigits[] = "\xD9\xA0\xD9\xA1\xD9\xA2\xD9\xA3\xD9\xA4\xD9\xA5\xD9\xA6\xD9\xA7\xD9\xA8\xD9\xA9";
    static const char persianDigits[] = "\xDB\xB0\xDB\xB1\xDB\xB2\xDB\xB3\xDB\xB4\xDB\xB5\xDB\xB6\xDB\xB7\xDB\xB8\xDB\xB9";
    static const char bengaliDigits[] = "\xE0\xA7\xA6\xE0\xA7\xA7\xE0\xA7\xA8\xE0\xA7\xA9\xE0\xA7\xAA"
                                        "\xE0\xA7\xAB\xE0\xA7\xAC\xE0\xA7\xAD\xE0\xA7\xAE\xE0\xA7\xAF";
    static const char devanagariDigits[] = "\xE0\xA5\xA6\xE0\xA5\xA7\xE0\xA5\xA8\xE0\xA5\xA9\xE0\xA5\xAA"
                                           "\xE0\xA5\xAB\xE0\xA5\xAC\xE0\xA5\xAD\xE0\xA5\xAE\xE0\xA5\xAF";
    static const char myanmarDigits[] = "\xE1\x81\x80\xE1\x81\x81\xE1\x81\x82\xE1\x81\x83\xE1\x81\x84"
                                        "\xE1\x81\x85\xE1\x81\x86\xE1\x81\x87\xE1\x81\x88\xE1\x81\x89";

    static const NumberEntry entries[] = {
        {"en", {}},
        {"en-IN", {",", ".", nullptr, 3, 2}},
        {"de", {".", ","}},
        {"de-CH", {"\xE2\x80\x99", "."}},
        {"fr", {"\xE2\x80\xAF", ",", nullptr, 3, 0, "%", true}},
        {"es", {".", ","}},
        {"es-MX", {",", "."}},
        {"es-419", {",", "."}},
        {"it", {".", ","}},
        {"pt", {".", ","}},
        {"pt-PT", {"\xC2\xA0", ","}},
        {"nl", {".", ","}},
        {"sv", {"\xC2\xA0", ","}},
        {"da", {".", ","}},
        {"dz", {",", "."}},
        {"nb", {"\xC2\xA0", ","}},
        {"fi", {"\xC2\xA0", ","}},
        {"is", {".", ","}},
        {"pl", {"\xC2\xA0", ","}},
        {"cs", {"\xC2\xA0", ","}},
        {"sk", {"\xC2\xA0", ","}},
        {"hu", {"\xC2\xA0", ","}},
        {"ro", {".", ","}},
        {"ru", {"\xC2\xA0", ","}},
        {"uk", {"\xC2\xA0", ","}},
        {"be", {"\xC2\xA0", ","}},
        {"bg", {"\xC2\xA0", ","}},
        {"el", {".", ","}},
        {"tr", {".", ",", nullptr, 3, 0, "%", false, true}},
        {"az", {".", ","}},
        {"kk", {"\xC2\xA0", ","}},
        {"he", {",", "."}},
        {"ar", {"\xD9\xAC", "\xD9\xAB", arabicDigits, 3, 0, "\xD9\xAA"}},
        {"fa", {"\xD9\xAC", "\xD9\xAB", persianDigits, 3, 0, "\xD9\xAA"}},
        {"hi", {",", ".", nullptr, 3, 2}},
        {"bn", {",", ".", bengaliDigits, 3, 2}},
        {"bo", {",", "."}},
        {"mn", {",", "."}},
        {"mr", {",", ".", devanagariDigits, 3, 2}},
        {"ne", {",", ".", devanagariDigits, 3, 2}},
        {"gu", {",", ".", nullptr, 3, 2}},
        {"kn", {",", ".", nullptr, 3, 2}},
        {"ml", {",", ".", nullptr, 3, 2}},
        {"pa", {",", ".", nullptr, 3, 2}},
        {"ta", {",", ".", nullptr, 3, 2}},
        {"te", {",", ".", nullptr, 3, 2}},
        {"zh", {",", "."}},
        {"yue", {",", "."}},
        {"ja", {",", "."}},
        {"ko", {",", "."}},
        {"vi", {".", ","}},
        {"id", {".", ","}},
        {"ms", {",", "."}},
        {"th", {",", "."}},
        {"my", {",", ".", myanmarDigits}},
        {"si", {",", "."}},
        {"ur", {",", "."}},
        {"sw", {",", "."}},
        {"af", {"\xC2\xA0", ","}},
        {"ga", {",", "."}},
        {"cy", {",", "."}},
        {"ca", {".", ","}},
        {"sr", {".", ","}},
        {"hr", {".", ","}},
        {"bs", {".", ","}},
        {"sl", {".", ","}},
    };

    for (const NumberEntry &entry : entries)
    {
        if (locale.tag() == entry.tag)
        {
            return entry.symbols;
        }
    }
    for (const NumberEntry &entry : entries)
    {
        if (locale.language() == entry.tag)
        {
            return entry.symbols;
        }
    }
    static const NumberSymbols neutral;
    return neutral;
}

inline void appendDigit(std::string &output, const char *digits, char asciiDigit)
{
    if (digits == nullptr || asciiDigit < '0' || asciiDigit > '9')
    {
        output.push_back(asciiDigit);
        return;
    }
    const int index = asciiDigit - '0';
    const char *cursor = digits;
    for (int step = 0; step < index; ++step)
    {
        ++cursor;
        while ((static_cast<unsigned char>(*cursor) & 0xC0) == 0x80)
        {
            ++cursor;
        }
    }
    const char *end = cursor + 1;
    while ((static_cast<unsigned char>(*end) & 0xC0) == 0x80)
    {
        ++end;
    }
    output.append(cursor, static_cast<std::size_t>(end - cursor));
}

inline const char *nativeDigitsFor(const Locale &locale)
{
    static const char tibetanDigits[] =
        "\xE0\xBC\xA0\xE0\xBC\xA1\xE0\xBC\xA2\xE0\xBC\xA3\xE0\xBC\xA4"
        "\xE0\xBC\xA5\xE0\xBC\xA6\xE0\xBC\xA7\xE0\xBC\xA8\xE0\xBC\xA9";
    static const char hanidecDigits[] =
        "\xE3\x80\x87\xE4\xB8\x80\xE4\xBA\x8C\xE4\xB8\x89\xE5\x9B\x9B"
        "\xE4\xBA\x94\xE5\x85\xAD\xE4\xB8\x83\xE5\x85\xAB\xE4\xB9\x9D";

    const std::string &language = locale.language();
    if (language == "bo" || language == "dz")
    {
        return tibetanDigits;
    }
    if (language == "zh" || language == "yue")
    {
        return hanidecDigits;
    }
    return nullptr;
}

inline std::string renderNumber(const std::string &integerPart, const std::string &fractionPart, bool negative,
                                const Locale &locale, const NumberFormat &format)
{
    const NumberSymbols &symbols = numberSymbols(locale);
    const char *digits = symbols.digits;
    if (format.nativeDigits)
    {
        const char *native = nativeDigitsFor(locale);
        if (native != nullptr)
        {
            digits = native;
        }
    }
    std::string body;

    const int primary = symbols.primaryGrouping > 0 ? symbols.primaryGrouping : 3;
    const int secondary = symbols.secondaryGrouping > 0 ? symbols.secondaryGrouping : primary;
    if (format.grouping && integerPart.size() > static_cast<std::size_t>(primary))
    {
        std::vector<std::size_t> boundaries;
        std::size_t position = integerPart.size();
        int size = primary;
        while (position > static_cast<std::size_t>(size))
        {
            position -= static_cast<std::size_t>(size);
            boundaries.push_back(position);
            size = secondary;
        }
        std::size_t remaining = boundaries.size();
        for (std::size_t index = 0; index < integerPart.size(); ++index)
        {
            if (remaining > 0 && index == boundaries[remaining - 1])
            {
                body += symbols.group;
                --remaining;
            }
            appendDigit(body, digits, integerPart[index]);
        }
    }
    else
    {
        for (char character : integerPart)
        {
            appendDigit(body, digits, character);
        }
    }

    if (!fractionPart.empty())
    {
        body += symbols.decimal;
        for (char character : fractionPart)
        {
            appendDigit(body, digits, character);
        }
    }

    if (format.percent)
    {
        if (symbols.percentBefore)
        {
            std::string prefix = symbols.percent;
            if (symbols.percentSpace)
            {
                prefix += "\xC2\xA0";
            }
            body = prefix + body;
        }
        else
        {
            if (symbols.percentSpace)
            {
                body += "\xC2\xA0";
            }
            body += symbols.percent;
        }
    }

    if (negative)
    {
        body.insert(body.begin(), '-');
    }
    return body;
}
} // namespace detail

inline std::string formatNumber(double value, const Locale &locale, const NumberFormat &format = NumberFormat())
{
    if (std::isnan(value))
    {
        return "NaN";
    }
    if (std::isinf(value))
    {
        return value < 0.0 ? "-inf" : "inf";
    }

    const bool negative = value < 0.0;
    double magnitude = std::fabs(value);
    if (format.percent)
    {
        magnitude *= 100.0;
    }

    int maximum = format.maximumFractionDigits;
    if (maximum < 0)
    {
        maximum = 0;
    }
    if (maximum > 15)
    {
        maximum = 15;
    }
    int minimum = format.minimumFractionDigits;
    if (minimum < 0)
    {
        minimum = 0;
    }
    if (minimum > maximum)
    {
        minimum = maximum;
    }

    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::fixed << std::setprecision(maximum) << magnitude;
    const std::string text = stream.str();

    const std::size_t dot = text.find('.');
    const std::string integerPart = dot == std::string::npos ? text : text.substr(0, dot);
    std::string fractionPart = dot == std::string::npos ? std::string() : text.substr(dot + 1);
    while (fractionPart.size() > static_cast<std::size_t>(minimum) && !fractionPart.empty() &&
           fractionPart.back() == '0')
    {
        fractionPart.pop_back();
    }

    return detail::renderNumber(integerPart, fractionPart, negative, locale, format);
}

inline std::string formatNumber(long long value, const Locale &locale, const NumberFormat &format = NumberFormat())
{
    if (format.percent)
    {
        return formatNumber(static_cast<double>(value), locale, format);
    }
    const bool negative = value < 0;
    const unsigned long long magnitude =
        negative ? 0ULL - static_cast<unsigned long long>(value) : static_cast<unsigned long long>(value);
    return detail::renderNumber(std::to_string(magnitude), std::string(), negative, locale, format);
}

template <typename Integer, typename std::enable_if<std::is_integral<Integer>::value, int>::type = 0>
std::string formatNumber(Integer value, const Locale &locale, const NumberFormat &format = NumberFormat())
{
    return formatNumber(static_cast<long long>(value), locale, format);
}

// ---------------------------------------------------------------------------
// CLDR plural rules
// ---------------------------------------------------------------------------

enum class PluralCategory
{
    zero,
    one,
    two,
    few,
    many,
    other
};

inline const char *pluralCategoryName(PluralCategory category)
{
    switch (category)
    {
    case PluralCategory::zero:
        return "zero";
    case PluralCategory::one:
        return "one";
    case PluralCategory::two:
        return "two";
    case PluralCategory::few:
        return "few";
    case PluralCategory::many:
        return "many";
    case PluralCategory::other:
        return "other";
    }
    return "other";
}

namespace detail
{
// CLDR operands.  The visible-fraction operands (v, w, f, t) are derived from
// the shortest round-trip decimal form of the value, which is what a caller
// passing a double can reasonably mean.
struct PluralOperands
{
    double n = 0.0;
    double i = 0.0;
    double f = 0.0;
    double t = 0.0;
    int v = 0;
    int w = 0;
    int e = 0;
    int c = 0;
};

inline PluralOperands makeOperands(double value)
{
    PluralOperands operands;
    operands.n = std::fabs(value);
    operands.i = std::floor(operands.n);

    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setprecision(15) << operands.n;
    const std::string text = stream.str();

    if (text.find_first_of("eE") != std::string::npos)
    {
        return operands;
    }

    const std::size_t dot = text.find('.');
    if (dot == std::string::npos)
    {
        return operands;
    }

    std::string fraction = text.substr(dot + 1);
    while (!fraction.empty() && fraction.back() == '0')
    {
        fraction.pop_back();
    }
    operands.v = static_cast<int>(fraction.size());
    operands.w = operands.v;
    for (char character : fraction)
    {
        operands.f = operands.f * 10.0 + static_cast<double>(character - '0');
        operands.t = operands.t * 10.0 + static_cast<double>(character - '0');
    }
    return operands;
}

enum class RelationOperator
{
    equal,
    notEqual,
    in,
    within
};

struct Relation
{
    char operand = 'n';
    bool hasModulus = false;
    double modulus = 0.0;
    RelationOperator op = RelationOperator::equal;
    bool negate = false;
    std::vector<std::pair<double, double>> ranges;

    double value(const PluralOperands &operands) const
    {
        switch (operand)
        {
        case 'n':
            return operands.n;
        case 'i':
            return operands.i;
        case 'f':
            return operands.f;
        case 't':
            return operands.t;
        case 'v':
            return static_cast<double>(operands.v);
        case 'w':
            return static_cast<double>(operands.w);
        case 'e':
            return static_cast<double>(operands.e);
        case 'c':
            return static_cast<double>(operands.c);
        default:
            return 0.0;
        }
    }

    bool matches(const PluralOperands &operands) const
    {
        double current = value(operands);
        if (hasModulus)
        {
            if (modulus == 0.0)
            {
                return false;
            }
            current = std::fmod(current, modulus);
        }

        bool inside = false;
        for (const std::pair<double, double> &range : ranges)
        {
            if (current >= range.first && current <= range.second)
            {
                inside = true;
                break;
            }
        }

        bool result = false;
        switch (op)
        {
        case RelationOperator::equal:
            result = inside;
            break;
        case RelationOperator::notEqual:
            result = !inside;
            break;
        case RelationOperator::in:
            result = inside && current == std::floor(current);
            break;
        case RelationOperator::within:
            result = inside;
            break;
        }
        return negate ? !result : result;
    }
};

struct Condition
{
    std::vector<Relation> relations;

    bool matches(const PluralOperands &operands) const
    {
        for (const Relation &relation : relations)
        {
            if (!relation.matches(operands))
            {
                return false;
            }
        }
        return true;
    }
};

struct Rule
{
    PluralCategory category = PluralCategory::other;
    std::vector<Condition> alternatives;

    bool matches(const PluralOperands &operands) const
    {
        for (const Condition &condition : alternatives)
        {
            if (condition.matches(operands))
            {
                return true;
            }
        }
        return false;
    }
};

struct RuleSet
{
    std::vector<Rule> rules;

    PluralCategory select(const PluralOperands &operands) const
    {
        for (const Rule &rule : rules)
        {
            if (rule.matches(operands))
            {
                return rule.category;
            }
        }
        return PluralCategory::other;
    }
};

inline std::string normalizeSpaces(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    bool pending = false;
    for (char character : text)
    {
        if (isSpace(character))
        {
            pending = !result.empty();
        }
        else
        {
            if (pending)
            {
                result.push_back(' ');
                pending = false;
            }
            result.push_back(character);
        }
    }
    return result;
}

inline PluralCategory parseCategory(std::string_view name)
{
    if (name == "zero")
    {
        return PluralCategory::zero;
    }
    if (name == "one")
    {
        return PluralCategory::one;
    }
    if (name == "two")
    {
        return PluralCategory::two;
    }
    if (name == "few")
    {
        return PluralCategory::few;
    }
    if (name == "many")
    {
        return PluralCategory::many;
    }
    if (name == "other")
    {
        return PluralCategory::other;
    }
    throw std::invalid_argument("localisation: unknown plural category '" + std::string(name) + "'");
}

inline bool isOperand(char character)
{
    return character == 'n' || character == 'i' || character == 'f' || character == 't' || character == 'v' ||
           character == 'w' || character == 'e' || character == 'c';
}

inline Relation parseRelation(std::string_view text)
{
    Relation relation;
    std::size_t position = 0;
    skipSpaces(text, position);
    if (position >= text.size() || !isOperand(text[position]))
    {
        throw std::invalid_argument("localisation: plural rule relation has no operand: '" + std::string(text) + "'");
    }
    relation.operand = text[position];
    ++position;
    skipSpaces(text, position);

    if (position < text.size() && text[position] == '%')
    {
        ++position;
        skipSpaces(text, position);
        const std::size_t begin = position;
        while (position < text.size() && (isDigit(text[position]) || text[position] == '.'))
        {
            ++position;
        }
        if (begin == position)
        {
            throw std::invalid_argument("localisation: plural rule modulus is missing");
        }
        relation.hasModulus = true;
        relation.modulus = parseDecimal(text.substr(begin, position - begin));
        if (relation.modulus == 0.0)
        {
            throw std::invalid_argument("localisation: plural rule modulus must not be zero");
        }
        skipSpaces(text, position);
    }

    const std::string_view remainder = text.substr(position);
    if (remainder.rfind("!=", 0) == 0)
    {
        relation.op = RelationOperator::notEqual;
        position += 2;
    }
    else if (remainder.rfind("=", 0) == 0)
    {
        relation.op = RelationOperator::equal;
        position += 1;
    }
    else if (remainder.rfind("not within", 0) == 0)
    {
        relation.op = RelationOperator::within;
        relation.negate = true;
        position += 10;
    }
    else if (remainder.rfind("not in", 0) == 0)
    {
        relation.op = RelationOperator::in;
        relation.negate = true;
        position += 6;
    }
    else if (remainder.rfind("is not", 0) == 0)
    {
        relation.op = RelationOperator::equal;
        relation.negate = true;
        position += 6;
    }
    else if (remainder.rfind("within", 0) == 0)
    {
        relation.op = RelationOperator::within;
        position += 6;
    }
    else if (remainder.rfind("in", 0) == 0)
    {
        relation.op = RelationOperator::in;
        position += 2;
    }
    else if (remainder.rfind("is", 0) == 0)
    {
        relation.op = RelationOperator::equal;
        position += 2;
    }
    else
    {
        throw std::invalid_argument("localisation: plural rule relation has no operator: '" + std::string(text) + "'");
    }

    std::string_view ranges = text.substr(position);
    std::size_t rangeStart = 0;
    while (rangeStart <= ranges.size())
    {
        const std::size_t comma = ranges.find(',', rangeStart);
        const std::string_view range =
            trimView(ranges.substr(rangeStart, comma == std::string_view::npos ? std::string_view::npos
                                                                               : comma - rangeStart));
        if (!range.empty())
        {
            const std::size_t dots = range.find("..");
            double low;
            double high;
            if (dots == std::string_view::npos)
            {
                low = parseDecimal(range);
                high = low;
            }
            else
            {
                low = parseDecimal(trimView(range.substr(0, dots)));
                high = parseDecimal(trimView(range.substr(dots + 2)));
            }
            relation.ranges.emplace_back(low, high);
        }
        if (comma == std::string_view::npos)
        {
            break;
        }
        rangeStart = comma + 1;
    }

    if (relation.ranges.empty())
    {
        throw std::invalid_argument("localisation: plural rule relation has no values: '" + std::string(text) + "'");
    }
    return relation;
}

inline RuleSet compileRuleSet(std::string_view text)
{
    RuleSet ruleSet;
    const std::string normalized = normalizeSpaces(text);
    std::size_t ruleStart = 0;
    while (ruleStart <= normalized.size())
    {
        const std::size_t semicolon = normalized.find(';', ruleStart);
        const std::string_view ruleText = trimView(std::string_view(normalized).substr(
            ruleStart, semicolon == std::string::npos ? std::string::npos : semicolon - ruleStart));
        if (!ruleText.empty())
        {
            const std::size_t colon = ruleText.find(':');
            if (colon == std::string_view::npos)
            {
                throw std::invalid_argument("localisation: plural rule is missing ':': '" + std::string(ruleText) +
                                            "'");
            }
            Rule rule;
            rule.category = parseCategory(trimView(ruleText.substr(0, colon)));
            const std::string_view conditionText = trimView(ruleText.substr(colon + 1));

            std::size_t alternativeStart = 0;
            while (alternativeStart <= conditionText.size())
            {
                const std::size_t alternativeEnd = conditionText.find(" or ", alternativeStart);
                const std::string_view alternative = conditionText.substr(
                    alternativeStart, alternativeEnd == std::string_view::npos ? std::string_view::npos
                                                                              : alternativeEnd - alternativeStart);
                Condition condition;
                std::size_t relationStart = 0;
                while (relationStart <= alternative.size())
                {
                    const std::size_t relationEnd = alternative.find(" and ", relationStart);
                    condition.relations.push_back(parseRelation(alternative.substr(
                        relationStart, relationEnd == std::string_view::npos ? std::string_view::npos
                                                                             : relationEnd - relationStart)));
                    if (relationEnd == std::string_view::npos)
                    {
                        break;
                    }
                    relationStart = relationEnd + 5;
                }
                rule.alternatives.push_back(std::move(condition));
                if (alternativeEnd == std::string_view::npos)
                {
                    break;
                }
                alternativeStart = alternativeEnd + 4;
            }
            ruleSet.rules.push_back(std::move(rule));
        }
        if (semicolon == std::string::npos)
        {
            break;
        }
        ruleStart = semicolon + 1;
    }
    return ruleSet;
}

struct PluralRuleEntry
{
    const char *languages; // '|'-separated language subtags
    const char *cardinal;
    const char *ordinal;
};

struct CompiledPluralRules
{
    RuleSet cardinal;
    RuleSet ordinal;
};

inline std::map<std::string, CompiledPluralRules> buildPluralRuleTable()
{
    // Cardinal rules are CLDR plural rules (integer-only operands are ignored
    // unless the rule mentions them).  Exponents (e) are always zero for the
    // double values used here; the rules that reference them still work for
    // ordinary quantities.
    static const PluralRuleEntry entries[] = {
        {"en", "one: i = 1 and v = 0",
         "one: n % 10 = 1 and n % 100 != 11; two: n % 10 = 2 and n % 100 != 12; "
         "few: n % 10 = 3 and n % 100 != 13"},
        {"de|nl|fi|et|sw", "one: i = 1 and v = 0", ""},
        {"sv", "one: i = 1 and v = 0", "one: n % 10 = 1,2 and n % 100 != 11,12"},
        {"af", "one: n = 1", ""},
        {"da", "one: n = 1 or t != 0 and i = 0,1", ""},
        {"nb|nn", "one: n = 1", ""},
        {"is", "one: t = 0 and i % 10 = 1 and i % 100 != 11 or t % 10 = 1 and t % 100 != 11", ""},
        {"fr", "one: i = 0,1; many: e = 0 and i != 0 and i % 1000000 = 0 and v = 0", "one: n = 1"},
        {"pt", "one: i = 0,1; many: e = 0 and i != 0 and i % 1000000 = 0 and v = 0", ""},
        {"es", "one: n = 1; many: e = 0 and i != 0 and i % 1000000 = 0 and v = 0", ""},
        {"it", "one: i = 1 and v = 0; many: e = 0 and i != 0 and i % 1000000 = 0 and v = 0", ""},
        {"ca", "one: i = 1 and v = 0; many: e = 0 and i != 0 and i % 1000000 = 0 and v = 0",
         "one: n = 1,3; two: n = 2; few: n = 4"},
        {"ru|uk|be",
         "one: v = 0 and i % 10 = 1 and i % 100 != 11; "
         "few: v = 0 and i % 10 = 2..4 and i % 100 != 12..14; "
         "many: v = 0 and i % 10 = 0 or v = 0 and i % 10 = 5..9 or v = 0 and i % 100 = 11..14",
         ""},
        {"pl",
         "one: i = 1 and v = 0; "
         "few: v = 0 and i % 10 = 2..4 and i % 100 != 12..14; "
         "many: v = 0 and i != 1 and i % 10 = 0..1 or v = 0 and i % 10 = 5..9 or v = 0 and i % 100 = 12..14",
         ""},
        {"cs|sk", "one: i = 1 and v = 0; few: i = 2..4 and v = 0; many: v != 0", ""},
        {"lt", "one: n % 10 = 1 and n % 100 != 11..19; few: n % 10 = 2..9 and n % 100 != 11..19; many: f != 0",
         ""},
        {"lv",
         "zero: n % 10 = 0 or n % 100 = 11..19 or v = 2 and f % 100 = 11..19; "
         "one: n % 10 = 1 and n % 100 != 11",
         ""},
        {"ga", "one: n = 1; two: n = 2; few: n = 3..6; many: n = 7..10", ""},
        {"cy", "zero: n = 0; one: n = 1; two: n = 2; few: n = 3; many: n = 6",
         "zero: n = 0,7,8,9; one: n = 1; two: n = 2; few: n = 3,4; many: n = 5,6"},
        {"ar", "zero: n = 0; one: n = 1; two: n = 2; few: n % 100 = 3..10; many: n % 100 = 11..99", ""},
        {"he", "one: i = 1 and v = 0; two: i = 2 and v = 0; many: v = 0 and n != 0..10 and n % 10 = 0", ""},
        {"hi|bn|mr|gu|kn|pa", "one: i = 0 or n = 1", ""},
        {"fa", "one: i = 0 or n = 1", ""},
        {"ta|te|ml|ne|si|ur|ps|tr|hu|el|ka|kk|uz|az|mn|sq|bg|ug|sd|ckb", "one: n = 1", ""},
        {"hy", "one: i = 0,1", ""},
        {"ro", "one: i = 1 and v = 0; few: v != 0 or n = 0 or n != 1 and n % 100 = 1..19", ""},
        {"sr|hr|bs",
         "one: v = 0 and i % 10 = 1 and i % 100 != 11 or f % 10 = 1 and f % 100 != 11; "
         "few: v = 0 and i % 10 = 2..4 and i % 100 != 12..14 or f % 10 = 2..4 and f % 100 != 12..14",
         ""},
        {"sl", "one: v = 0 and i % 100 = 1; two: v = 0 and i % 100 = 2; few: v = 0 and i % 100 = 3..4 or v != 0",
         ""},
        {"mk", "one: v = 0 and i % 10 = 1 and i % 100 != 11 or f % 10 = 1 and f % 100 != 11", ""},
        {"ja|zh|ko|vi|id|ms|th|my|km|lo|bo|dz|yue", "", ""},
        {"fil",
         "one: v = 0 and i = 1,2,3 or v = 0 and i % 10 != 4,6,9 or v != 0 and f % 10 != 4,6,9",
         ""},
    };

    std::map<std::string, CompiledPluralRules> table;
    for (const PluralRuleEntry &entry : entries)
    {
        const CompiledPluralRules compiled{compileRuleSet(entry.cardinal), compileRuleSet(entry.ordinal)};

        std::size_t start = 0;
        const std::string languages = entry.languages;
        while (start <= languages.size())
        {
            const std::size_t separator = languages.find('|', start);
            const std::string language = languages.substr(
                start, separator == std::string::npos ? std::string::npos : separator - start);
            if (!language.empty())
            {
                table[language] = compiled;
            }
            if (separator == std::string::npos)
            {
                break;
            }
            start = separator + 1;
        }
    }
    return table;
}

inline const CompiledPluralRules &rulesFor(const Locale &locale)
{
    static const std::map<std::string, CompiledPluralRules> table = buildPluralRuleTable();
    const auto found = table.find(locale.language());
    if (found != table.end())
    {
        return found->second;
    }
    static const CompiledPluralRules fallback;
    return fallback;
}
} // namespace detail

template <typename Number, typename std::enable_if<std::is_arithmetic<Number>::value, int>::type = 0>
PluralCategory pluralCategory(const Locale &locale, Number count)
{
    return detail::rulesFor(locale).cardinal.select(detail::makeOperands(static_cast<double>(count)));
}

template <typename Number, typename std::enable_if<std::is_arithmetic<Number>::value, int>::type = 0>
PluralCategory ordinalCategory(const Locale &locale, Number count)
{
    return detail::rulesFor(locale).ordinal.select(detail::makeOperands(static_cast<double>(count)));
}

// ---------------------------------------------------------------------------
// Text direction
// ---------------------------------------------------------------------------

enum class TextDirection
{
    leftToRight,
    rightToLeft,
    topToBottom // Traditional Mongolian script
};

inline TextDirection direction(const Locale &locale)
{
    if (!locale.script().empty())
    {
        const std::string &script = locale.script();
        if (script == "Mong")
        {
            return TextDirection::topToBottom;
        }
        const bool rightToLeft =
            script == "Arab" || script == "Hebr" || script == "Thaa" || script == "Syrc" || script == "Nkoo" ||
            script == "Adlm" || script == "Rohg" || script == "Mand" || script == "Samr" || script == "Merc" ||
            script == "Yezi";
        return rightToLeft ? TextDirection::rightToLeft : TextDirection::leftToRight;
    }

    static const char *rightToLeftLanguages[] = {"ar", "arc", "ckb", "dv", "fa", "he", "ks",
                                                 "ku", "ps",  "sd",  "ug", "ur", "yi"};
    for (const char *language : rightToLeftLanguages)
    {
        if (locale.language() == language)
        {
            return TextDirection::rightToLeft;
        }
    }
    return TextDirection::leftToRight;
}

inline bool isRightToLeft(const Locale &locale) { return direction(locale) == TextDirection::rightToLeft; }

// ---------------------------------------------------------------------------
// Unicode normalisation and text styles
//
// The implementation lives in bunch/unicode.h; these wrappers bind the
// character-level operations to a Locale.
// ---------------------------------------------------------------------------

using NormalizationForm = bunch::NormalizationForm;
using TextStyle = bunch::TextStyle;

using bunch::isNormalized;
using bunch::nfc;
using bunch::nfd;
using bunch::nfkc;
using bunch::nfkd;
using bunch::normalizationFormName;
using bunch::normalize;
using bunch::textStyleName;

// Both spellings are available: the bunch overload takes a language tag, the
// localisation overload takes a Locale.
using bunch::formatText;
using bunch::toLowerCase;
using bunch::toTitleCase;
using bunch::toUpperCase;

inline std::string formatText(std::string_view text, const Locale &locale, TextStyle style)
{
    return bunch::formatText(text, locale.language(), style);
}

inline std::string toLowerCase(std::string_view text, const Locale &locale = Locale("en"))
{
    return bunch::toLowerCase(text, locale.language());
}

inline std::string toUpperCase(std::string_view text, const Locale &locale = Locale("en"))
{
    return bunch::toUpperCase(text, locale.language());
}

inline std::string toTitleCase(std::string_view text, const Locale &locale = Locale("en"))
{
    return bunch::toTitleCase(text, locale.language());
}

// A small value type that bundles a locale, a style and an optional
// normalisation form, for call sites that format the same kind of text
// repeatedly.
class TextFormat
{
  public:
    TextFormat() = default;

    explicit TextFormat(const Locale &locale, TextStyle style = TextStyle::plain,
                        NormalizationForm normalization = NormalizationForm::none)
        : locale_(locale), style_(style), normalization_(normalization)
    {
    }

    TextFormat &setLocale(const Locale &locale)
    {
        locale_ = locale;
        return *this;
    }

    TextFormat &setStyle(TextStyle style)
    {
        style_ = style;
        return *this;
    }

    TextFormat &setNormalization(NormalizationForm normalization)
    {
        normalization_ = normalization;
        return *this;
    }

    const Locale &locale() const noexcept { return locale_; }
    TextStyle style() const noexcept { return style_; }
    NormalizationForm normalization() const noexcept { return normalization_; }

    std::string format(std::string_view text) const
    {
        std::string result = localisation::formatText(text, locale_, style_);
        if (normalization_ != NormalizationForm::none)
        {
            result = localisation::normalize(result, normalization_);
        }
        return result;
    }

    std::string operator()(std::string_view text) const { return format(text); }

  private:
    Locale locale_;
    TextStyle style_ = TextStyle::plain;
    NormalizationForm normalization_ = NormalizationForm::none;
};

// ---------------------------------------------------------------------------
// Message arguments
// ---------------------------------------------------------------------------

struct Argument
{
    enum class kind
    {
        text,
        integer,
        real,
        boolean
    };

    std::string name;
    kind type = kind::text;
    std::string text;
    long long integer = 0;
    double real = 0.0;
    bool boolean = false;
};

class Arguments
{
  public:
    Arguments &set(std::string_view name, std::string_view text)
    {
        Argument argument;
        argument.name = std::string(name);
        argument.type = Argument::kind::text;
        argument.text = std::string(text);
        store(std::move(argument));
        return *this;
    }

    Arguments &set(std::string_view name, const std::string &text) { return set(name, std::string_view(text)); }

    Arguments &set(std::string_view name, const char *text)
    {
        return set(name, std::string_view(text == nullptr ? "" : text));
    }

    Arguments &set(std::string_view name, bool value)
    {
        Argument argument;
        argument.name = std::string(name);
        argument.type = Argument::kind::boolean;
        argument.boolean = value;
        store(std::move(argument));
        return *this;
    }

    template <typename Integer, typename std::enable_if<std::is_integral<Integer>::value, int>::type = 0>
    Arguments &set(std::string_view name, Integer value)
    {
        Argument argument;
        argument.name = std::string(name);
        argument.type = Argument::kind::integer;
        argument.integer = static_cast<long long>(value);
        store(std::move(argument));
        return *this;
    }

    template <typename Real, typename std::enable_if<std::is_floating_point<Real>::value, int>::type = 0>
    Arguments &set(std::string_view name, Real value)
    {
        Argument argument;
        argument.name = std::string(name);
        argument.type = Argument::kind::real;
        argument.real = static_cast<double>(value);
        store(std::move(argument));
        return *this;
    }

    bool has(std::string_view name) const { return find(name) != nullptr; }

    const Argument *find(std::string_view name) const
    {
        for (const Argument &argument : arguments_)
        {
            if (argument.name == name)
            {
                return &argument;
            }
        }
        return nullptr;
    }

    std::size_t size() const noexcept { return arguments_.size(); }
    bool empty() const noexcept { return arguments_.empty(); }

  private:
    void store(Argument argument)
    {
        for (Argument &existing : arguments_)
        {
            if (existing.name == argument.name)
            {
                existing = std::move(argument);
                return;
            }
        }
        arguments_.push_back(std::move(argument));
    }

    std::vector<Argument> arguments_;
};

// ---------------------------------------------------------------------------
// Compiled message patterns
// ---------------------------------------------------------------------------

namespace detail
{
enum class PartKind
{
    text,
    pound,
    argument
};

enum class ArgumentKind
{
    plain,
    number,
    plural,
    selectordinal,
    select
};

struct part;
using parts = std::vector<part>;

struct branch
{
    std::string selector;
    bool exact = false;
    double exactValue = 0.0;
    bool categorySelector = false;
    PluralCategory category = PluralCategory::other;
    parts body;
};

struct part
{
    PartKind kind = PartKind::text;
    ArgumentKind argument = ArgumentKind::plain;
    std::string text; // literal text, or the argument name
    NumberFormat format;
    int offset = 0;
    std::vector<branch> branches;
};

class MessageParser
{
  public:
    explicit MessageParser(std::string_view pattern) : pattern_(pattern) {}

    std::vector<part> parse()
    {
        std::vector<part> result = parseParts('\0');
        if (position_ != pattern_.size())
        {
            fail("unexpected '}'");
        }
        return result;
    }

  private:
    struct DepthGuard
    {
        explicit DepthGuard(int &value) : depth(value) { ++depth; }
        ~DepthGuard() { --depth; }

        DepthGuard(const DepthGuard &) = delete;
        DepthGuard &operator=(const DepthGuard &) = delete;

        int &depth;
    };

    [[noreturn]] void fail(const std::string &message) const
    {
        std::ostringstream stream;
        stream << "localisation: " << message << " at offset " << position_ << " in pattern \"" << pattern_ << "\"";
        throw std::invalid_argument(stream.str());
    }

    bool atEnd() const noexcept { return position_ >= pattern_.size(); }

    void skipSpaces()
    {
        while (!atEnd() && isSpace(pattern_[position_]))
        {
            ++position_;
        }
    }

    void expect(char expected)
    {
        if (atEnd() || pattern_[position_] != expected)
        {
            fail(std::string("expected '") + expected + "'");
        }
        ++position_;
    }

    std::string readName()
    {
        const std::size_t begin = position_;
        while (!atEnd())
        {
            const char character = pattern_[position_];
            if (character == '}' || character == ',' || character == '{' || isSpace(character))
            {
                break;
            }
            ++position_;
        }
        if (begin == position_)
        {
            fail("expected an argument name");
        }
        return std::string(pattern_.substr(begin, position_ - begin));
    }

    std::string readWord()
    {
        const std::size_t begin = position_;
        while (!atEnd() && isAlpha(pattern_[position_]))
        {
            ++position_;
        }
        return std::string(pattern_.substr(begin, position_ - begin));
    }

    std::string readSelector()
    {
        const std::size_t begin = position_;
        while (!atEnd())
        {
            const char character = pattern_[position_];
            if (character == '{' || character == '}' || character == ',' || isSpace(character))
            {
                break;
            }
            ++position_;
        }
        if (begin == position_)
        {
            fail("expected a plural or select selector");
        }
        return std::string(pattern_.substr(begin, position_ - begin));
    }

    void readQuoted(std::string &output)
    {
        const char next = position_ + 1 < pattern_.size() ? pattern_[position_ + 1] : '\0';
        if (next == '\'')
        {
            // A pair of apostrophes always stands for one literal apostrophe.
            output.push_back('\'');
            position_ += 2;
            return;
        }
        if (next == '{' || next == '}' || next == '#')
        {
            ++position_; // opening quote
            while (!atEnd())
            {
                const char character = pattern_[position_];
                if (character == '\'')
                {
                    if (position_ + 1 < pattern_.size() && pattern_[position_ + 1] == '\'')
                    {
                        output.push_back('\'');
                        position_ += 2;
                        continue;
                    }
                    ++position_;
                    return;
                }
                output.push_back(character);
                ++position_;
            }
            return;
        }
        output.push_back('\'');
        ++position_;
    }

    NumberFormat parseNumberStyle(std::string_view style)
    {
        NumberFormat format;
        const std::string_view trimmed = trimView(style);
        if (trimmed.empty())
        {
            return format;
        }
        if (trimmed == "integer")
        {
            format.minimumFractionDigits = 0;
            format.maximumFractionDigits = 0;
            return format;
        }
        if (trimmed == "percent")
        {
            format.percent = true;
            return format;
        }
        if (trimmed.rfind("::", 0) == 0)
        {
            std::string_view skeleton = trimmed.substr(2);
            if (skeleton.rfind("percent", 0) == 0)
            {
                format.percent = true;
                skeleton.remove_prefix(7);
            }
            if (skeleton.empty())
            {
                return format;
            }
            if (skeleton.front() == '.')
            {
                skeleton.remove_prefix(1);
                int minimum = 0;
                int maximum = 0;
                for (char character : skeleton)
                {
                    if (character == '0')
                    {
                        ++minimum;
                        ++maximum;
                    }
                    else if (character == '#')
                    {
                        ++maximum;
                    }
                    else
                    {
                        fail("unsupported number skeleton");
                    }
                }
                format.minimumFractionDigits = minimum;
                format.maximumFractionDigits = maximum;
                return format;
            }
            fail("unsupported number skeleton");
        }
        fail("unsupported number style '" + std::string(trimmed) + "'");
    }

    void tryParseOffset(int &offset)
    {
        const std::size_t saved = position_;
        if (pattern_.compare(position_, 6, "offset") != 0)
        {
            return;
        }
        const std::size_t after = position_ + 6;
        if (after < pattern_.size() && (isAlnum(pattern_[after]) || pattern_[after] == '_'))
        {
            return;
        }
        position_ = after;
        skipSpaces();
        if (atEnd() || pattern_[position_] != ':')
        {
            position_ = saved;
            return;
        }
        ++position_;
        skipSpaces();
        const std::size_t begin = position_;
        int value = 0;
        while (!atEnd() && isDigit(pattern_[position_]))
        {
            value = value * 10 + (pattern_[position_] - '0');
            ++position_;
        }
        if (begin == position_)
        {
            fail("expected a plural offset");
        }
        skipSpaces();
        offset = value;
    }

    part parseArgument()
    {
        if (depth_ >= 32)
        {
            fail("message patterns may not nest more than 32 levels");
        }
        const DepthGuard guard(depth_);

        part result;
        ++position_; // '{'
        skipSpaces();
        const std::string name = readName();
        skipSpaces();
        if (atEnd())
        {
            fail("unterminated argument");
        }
        if (pattern_[position_] == '}')
        {
            ++position_;
            result.kind = PartKind::argument;
            result.argument = ArgumentKind::plain;
            result.text = name;
            return result;
        }
        if (pattern_[position_] != ',')
        {
            fail("expected ',' or '}' after the argument name");
        }
        ++position_;
        skipSpaces();
        const std::string type = readWord();
        if (type.empty())
        {
            fail("expected an argument type");
        }

        if (type == "number")
        {
            result.kind = PartKind::argument;
            result.argument = ArgumentKind::number;
            result.text = name;
            skipSpaces();
            if (!atEnd() && pattern_[position_] == ',')
            {
                ++position_;
                const std::size_t begin = position_;
                while (!atEnd() && pattern_[position_] != '}')
                {
                    ++position_;
                }
                result.format = parseNumberStyle(pattern_.substr(begin, position_ - begin));
            }
            skipSpaces();
            expect('}');
            return result;
        }

        if (type == "plural" || type == "selectordinal" || type == "select")
        {
            result.kind = PartKind::argument;
            result.argument = type == "plural"        ? ArgumentKind::plural
                              : type == "selectordinal" ? ArgumentKind::selectordinal
                                                        : ArgumentKind::select;
            result.text = name;
            skipSpaces();
            expect(',');
            if (result.argument != ArgumentKind::select)
            {
                skipSpaces();
                tryParseOffset(result.offset);
            }

            bool hasOther = false;
            std::vector<std::string> seen;
            while (true)
            {
                skipSpaces();
                if (atEnd())
                {
                    fail("unterminated " + type + " argument");
                }
                if (pattern_[position_] == '}')
                {
                    ++position_;
                    break;
                }

                branch current;
                current.selector = readSelector();
                for (const std::string &selector : seen)
                {
                    if (selector == current.selector)
                    {
                        fail("duplicate selector '" + current.selector + "'");
                    }
                }
                seen.push_back(current.selector);
                if (current.selector == "other")
                {
                    hasOther = true;
                }

                if (result.argument == ArgumentKind::select)
                {
                    if (current.selector == "other")
                    {
                        current.categorySelector = false;
                    }
                }
                else if (!current.selector.empty() && current.selector[0] == '=')
                {
                    current.exact = true;
                    current.exactValue = parseDecimal(current.selector.substr(1));
                }
                else
                {
                    current.categorySelector = true;
                    current.category = parseCategory(current.selector);
                }

                skipSpaces();
                expect('{');
                current.body = parseParts('}');
                expect('}');
                result.branches.push_back(std::move(current));
            }

            if (!hasOther)
            {
                fail(type + " argument '" + name + "' requires an 'other' branch");
            }
            return result;
        }

        fail("unknown argument type '" + type + "'");
    }

    std::vector<part> parseParts(char terminator)
    {
        std::vector<part> result;
        std::string literal;

        const auto flush = [&result, &literal]() {
            if (!literal.empty())
            {
                part text;
                text.kind = PartKind::text;
                text.text = literal;
                result.push_back(std::move(text));
                literal.clear();
            }
        };

        while (!atEnd())
        {
            const char character = pattern_[position_];
            if (terminator != '\0' && character == terminator)
            {
                break;
            }
            if (character == '{')
            {
                flush();
                result.push_back(parseArgument());
                continue;
            }
            if (character == '}')
            {
                fail("unexpected '}'");
            }
            if (character == '\'')
            {
                readQuoted(literal);
                continue;
            }
            if (character == '#')
            {
                flush();
                part pound;
                pound.kind = PartKind::pound;
                result.push_back(std::move(pound));
                ++position_;
                continue;
            }
            literal.push_back(character);
            ++position_;
        }
        flush();
        return result;
    }

    std::string_view pattern_;
    std::size_t position_ = 0;
    int depth_ = 0;
};

inline NumberFormat plainNumberFormat()
{
    NumberFormat format;
    format.grouping = false;
    return format;
}

inline bool numericValue(const Argument &argument, double &value)
{
    switch (argument.type)
    {
    case Argument::kind::integer:
        value = static_cast<double>(argument.integer);
        return true;
    case Argument::kind::real:
        value = argument.real;
        return true;
    case Argument::kind::boolean:
        value = argument.boolean ? 1.0 : 0.0;
        return true;
    case Argument::kind::text:
    {
        const std::string_view trimmed = trimView(argument.text);
        if (trimmed.empty())
        {
            return false;
        }
        const std::string copy(trimmed);
        char *end = nullptr;
        const double parsed = std::strtod(copy.c_str(), &end);
        if (end == copy.c_str() || *end != '\0')
        {
            return false;
        }
        value = parsed;
        return true;
    }
    }
    return false;
}

inline std::string textualValue(const Argument &argument, const Locale &locale)
{
    switch (argument.type)
    {
    case Argument::kind::text:
        return argument.text;
    case Argument::kind::integer:
        return formatNumber(argument.integer, locale, plainNumberFormat());
    case Argument::kind::real:
        return formatNumber(argument.real, locale, plainNumberFormat());
    case Argument::kind::boolean:
        return argument.boolean ? "true" : "false";
    }
    return std::string();
}

inline void appendValue(const Argument &argument, const Locale &locale, const NumberFormat &format,
                        std::string &output)
{
    switch (argument.type)
    {
    case Argument::kind::text:
        output += argument.text;
        break;
    case Argument::kind::integer:
        output += formatNumber(argument.integer, locale, format);
        break;
    case Argument::kind::real:
        output += formatNumber(argument.real, locale, format);
        break;
    case Argument::kind::boolean:
        output += argument.boolean ? "true" : "false";
        break;
    }
}

inline void formatParts(const std::vector<part> &messageParts, const Locale &locale, const Arguments &arguments,
                        const std::string *pound, std::string &output);

inline void formatArgument(const part &current, const Locale &locale, const Arguments &arguments,
                           const std::string *pound, std::string &output)
{
    const Argument *argument = arguments.find(current.text);

    switch (current.argument)
    {
    case ArgumentKind::plain:
    {
        if (argument == nullptr)
        {
            output += '{';
            output += current.text;
            output += '}';
            return;
        }
        appendValue(*argument, locale, plainNumberFormat(), output);
        return;
    }

    case ArgumentKind::number:
    {
        if (argument == nullptr)
        {
            output += '{';
            output += current.text;
            output += '}';
            return;
        }
        appendValue(*argument, locale, current.format, output);
        return;
    }

    case ArgumentKind::plural:
    case ArgumentKind::selectordinal:
    {
        double count = 0.0;
        const bool numeric = argument != nullptr && numericValue(*argument, count);
        std::string poundText;
        const branch *chosen = nullptr;

        if (numeric)
        {
            const double adjusted = count - static_cast<double>(current.offset);
            const PluralCategory category = current.argument == ArgumentKind::plural ? pluralCategory(locale, adjusted)
                                                                                     : ordinalCategory(locale, adjusted);

            for (const branch &candidate : current.branches)
            {
                if (candidate.exact && candidate.exactValue == adjusted)
                {
                    chosen = &candidate;
                    break;
                }
            }
            if (chosen == nullptr)
            {
                for (const branch &candidate : current.branches)
                {
                    if (candidate.categorySelector && candidate.category == category)
                    {
                        chosen = &candidate;
                        break;
                    }
                }
            }
            if (chosen == nullptr)
            {
                for (const branch &candidate : current.branches)
                {
                    if (candidate.selector == "other")
                    {
                        chosen = &candidate;
                        break;
                    }
                }
            }

            poundText = formatNumber(adjusted, locale);
        }
        else
        {
            for (const branch &candidate : current.branches)
            {
                if (candidate.selector == "other")
                {
                    chosen = &candidate;
                    break;
                }
            }
            if (argument != nullptr)
            {
                poundText = textualValue(*argument, locale);
            }
            else
            {
                poundText = '{' + current.text + '}';
            }
        }

        if (chosen == nullptr)
        {
            output += '{';
            output += current.text;
            output += '}';
            return;
        }
        formatParts(chosen->body, locale, arguments, &poundText, output);
        return;
    }

    case ArgumentKind::select:
    {
        const std::string key = argument == nullptr ? std::string() : textualValue(*argument, locale);
        const branch *chosen = nullptr;
        for (const branch &candidate : current.branches)
        {
            if (candidate.selector == key)
            {
                chosen = &candidate;
                break;
            }
        }
        if (chosen == nullptr)
        {
            for (const branch &candidate : current.branches)
            {
                if (candidate.selector == "other")
                {
                    chosen = &candidate;
                    break;
                }
            }
        }
        if (chosen == nullptr)
        {
            output += '{';
            output += current.text;
            output += '}';
            return;
        }
        formatParts(chosen->body, locale, arguments, pound, output);
        return;
    }
    }
}

inline void formatParts(const std::vector<part> &messageParts, const Locale &locale, const Arguments &arguments,
                        const std::string *pound, std::string &output)
{
    for (const part &current : messageParts)
    {
        switch (current.kind)
        {
        case PartKind::text:
            output += current.text;
            break;
        case PartKind::pound:
            output += pound == nullptr ? std::string("#") : *pound;
            break;
        case PartKind::argument:
            formatArgument(current, locale, arguments, pound, output);
            break;
        }
    }
}
} // namespace detail

class Message
{
  public:
    Message() = default;

    explicit Message(std::string_view pattern)
        : pattern_(pattern), parts_(detail::MessageParser(pattern).parse())
    {
    }

    const std::string &pattern() const noexcept { return pattern_; }
    bool empty() const noexcept { return parts_.empty(); }

    std::string format(const Locale &locale, const Arguments &arguments) const
    {
        std::string output;
        detail::formatParts(parts_, locale, arguments, nullptr, output);
        return output;
    }

    std::string format(const Locale &locale) const { return format(locale, Arguments()); }

  private:
    std::string pattern_;
    std::vector<detail::part> parts_;
};

// ---------------------------------------------------------------------------
// Catalogue: keyed messages per locale, with a fallback chain
// ---------------------------------------------------------------------------

namespace detail
{
// msgctxt is stored behind a U+0004 separator so that one flat key space can
// hold both plain and context-qualified messages.
inline std::string contextKey(std::string_view context, std::string_view key)
{
    std::string result;
    result.reserve(context.size() + key.size() + 1);
    result.append(context);
    result.push_back('\x04');
    result.append(key);
    return result;
}

inline bool hasContinuation(std::string_view line)
{
    std::size_t backslashes = 0;
    for (std::size_t index = line.size(); index > 0; --index)
    {
        if (line[index - 1] != '\\')
        {
            break;
        }
        ++backslashes;
    }
    return backslashes % 2 == 1;
}

// Java-style property escapes, including \uXXXX for anything outside ASCII.
inline std::string unescape(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    for (std::size_t index = 0; index < text.size(); ++index)
    {
        const char character = text[index];
        if (character != '\\')
        {
            result.push_back(character);
            continue;
        }
        if (index + 1 >= text.size())
        {
            result.push_back('\\');
            break;
        }
        const char escape = text[++index];
        switch (escape)
        {
        case 'n':
            result.push_back('\n');
            break;
        case 't':
            result.push_back('\t');
            break;
        case 'r':
            result.push_back('\r');
            break;
        case 'f':
            result.push_back('\f');
            break;
        case 'u':
        {
            if (index + 4 >= text.size())
            {
                result.push_back('u');
                break;
            }
            unsigned int codepoint = 0;
            bool valid = true;
            for (int digit = 0; digit < 4; ++digit)
            {
                const int value = hexValue(text[index + 1 + static_cast<std::size_t>(digit)]);
                if (value < 0)
                {
                    valid = false;
                    break;
                }
                codepoint = codepoint * 16 + static_cast<unsigned int>(value);
            }
            if (!valid)
            {
                result.push_back('u');
                break;
            }
            // Combine UTF-16 surrogate pairs written as two \u escapes.
            if (codepoint >= 0xD800 && codepoint <= 0xDBFF && index + 10 < text.size() && text[index + 5] == '\\' &&
                text[index + 6] == 'u')
            {
                unsigned int low = 0;
                bool lowValid = true;
                for (int digit = 0; digit < 4; ++digit)
                {
                    const int value = hexValue(text[index + 7 + static_cast<std::size_t>(digit)]);
                    if (value < 0)
                    {
                        lowValid = false;
                        break;
                    }
                    low = low * 16 + static_cast<unsigned int>(value);
                }
                if (lowValid && low >= 0xDC00 && low <= 0xDFFF)
                {
                    codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
                    index += 6;
                }
            }
            bunch::appendUtf8(result, codepoint);
            index += 4;
            break;
        }
        default:
            result.push_back(escape);
            break;
        }
    }
    return result;
}
} // namespace detail

class Catalog
{
  public:
    using MissingHandler = std::function<void(const Locale &, const std::string &)>;

    explicit Catalog(std::string_view sourceLocale = "en") : source_(sourceLocale) {}

    Catalog &add(const Locale &locale, std::string_view key, std::string_view pattern)
    {
        locales_[locale.tag()].insert_or_assign(std::string(key), Message(pattern));
        return *this;
    }

    // Context support mirrors gettext's msgctxt: the same identifier can have
    // different translations in different contexts.
    Catalog &add(const Locale &locale, std::string_view context, std::string_view key, std::string_view pattern)
    {
        return add(locale, detail::contextKey(context, key), pattern);
    }

    // Loads a .properties-style stream:
    //   # comment
    //   greeting = Hello, {name}!
    //   Open[menu] = &Open
    // Escapes: \n \t \r \f \\ \: \= \# \! \uXXXX; a trailing backslash
    // continues the line.  Returns the number of messages added.
    std::size_t load(const Locale &locale, std::istream &stream);

    bool contains(const Locale &locale, std::string_view key) const
    {
        const auto localeEntry = locales_.find(locale.tag());
        return localeEntry != locales_.end() && localeEntry->second.find(key) != localeEntry->second.end();
    }

    bool contains(const Locale &locale, std::string_view context, std::string_view key) const
    {
        return contains(locale, detail::contextKey(context, key));
    }

    const Message *find(const Locale &locale, std::string_view key) const { return lookup(locale, key).message; }

    const Message *find(const Locale &locale, std::string_view context, std::string_view key) const
    {
        return lookup(locale, detail::contextKey(context, key)).message;
    }

    std::string translate(const Locale &locale, std::string_view key,
                          const Arguments &arguments = Arguments()) const
    {
        const Lookup result = lookup(locale, key);
        if (result.message == nullptr)
        {
            reportMissing(locale, key);
            return std::string(key);
        }
        return result.message->format(result.locale, arguments);
    }

    std::string translate(const Locale &locale, std::string_view key, std::string_view fallback,
                          const Arguments &arguments = Arguments()) const
    {
        const Lookup result = lookup(locale, key);
        if (result.message == nullptr)
        {
            reportMissing(locale, key);
            return Message(fallback).format(locale, arguments);
        }
        return result.message->format(result.locale, arguments);
    }

    std::string translateIn(const Locale &locale, std::string_view context, std::string_view key,
                            const Arguments &arguments = Arguments()) const
    {
        const std::string composite = detail::contextKey(context, key);
        const Lookup result = lookup(locale, composite);
        if (result.message == nullptr)
        {
            reportMissing(locale, key);
            return std::string(key);
        }
        return result.message->format(result.locale, arguments);
    }

    const Locale &sourceLocale() const noexcept { return source_; }

    std::vector<Locale> locales() const
    {
        std::vector<Locale> result;
        result.reserve(locales_.size());
        for (const auto &entry : locales_)
        {
            result.emplace_back(entry.first);
        }
        return result;
    }

    std::size_t size() const
    {
        std::size_t total = 0;
        for (const auto &entry : locales_)
        {
            total += entry.second.size();
        }
        return total;
    }

    bool empty() const { return size() == 0; }

    // Called whenever a lookup falls through every locale and the source
    // language.  Useful to wire missing-translation telemetry in one place.
    void setMissingHandler(MissingHandler handler) { missing_ = std::move(handler); }

  private:
    struct Lookup
    {
        const Message *message = nullptr;
        Locale locale;
    };

    Lookup lookup(const Locale &locale, std::string_view key) const
    {
        for (const Locale &candidate : locale.fallbacks())
        {
            const auto localeEntry = locales_.find(candidate.tag());
            if (localeEntry == locales_.end())
            {
                continue;
            }
            const auto message = localeEntry->second.find(key);
            if (message != localeEntry->second.end())
            {
                return Lookup{&message->second, candidate};
            }
        }
        for (const Locale &candidate : source_.fallbacks())
        {
            const auto sourceEntry = locales_.find(candidate.tag());
            if (sourceEntry == locales_.end())
            {
                continue;
            }
            const auto message = sourceEntry->second.find(key);
            if (message != sourceEntry->second.end())
            {
                return Lookup{&message->second, candidate};
            }
        }
        return Lookup{};
    }

    void reportMissing(const Locale &locale, std::string_view key) const
    {
        if (missing_)
        {
            missing_(locale, std::string(key));
        }
    }

    std::map<std::string, std::map<std::string, Message, std::less<>>, std::less<>> locales_;
    Locale source_;
    MissingHandler missing_;
};

inline std::size_t Catalog::load(const Locale &locale, std::istream &stream)
{
    std::size_t loaded = 0;
    std::string line;
    bool firstLine = true;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (firstLine)
        {
            firstLine = false;
            if (line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
            {
                line.erase(0, 3);
            }
        }

        while (detail::hasContinuation(line))
        {
            line.pop_back();
            std::string continuation;
            if (!std::getline(stream, continuation))
            {
                break;
            }
            if (!continuation.empty() && continuation.back() == '\r')
            {
                continuation.pop_back();
            }
            std::size_t begin = 0;
            while (begin < continuation.size() && (continuation[begin] == ' ' || continuation[begin] == '\t'))
            {
                ++begin;
            }
            line += continuation.substr(begin);
        }

        std::string_view text = line;
        std::size_t begin = 0;
        while (begin < text.size() && detail::isSpace(text[begin]))
        {
            ++begin;
        }
        text.remove_prefix(begin);
        if (text.empty() || text.front() == '#' || text.front() == '!')
        {
            continue;
        }

        std::string key;
        std::size_t position = 0;
        while (position < text.size())
        {
            const char character = text[position];
            if (character == '\\')
            {
                key.push_back(character);
                if (position + 1 < text.size())
                {
                    key.push_back(text[position + 1]);
                    position += 2;
                }
                else
                {
                    ++position;
                }
                continue;
            }
            if (character == '=' || character == ':' || detail::isSpace(character))
            {
                break;
            }
            key.push_back(character);
            ++position;
        }
        while (position < text.size() && detail::isSpace(text[position]))
        {
            ++position;
        }
        if (position < text.size() && (text[position] == '=' || text[position] == ':'))
        {
            ++position;
            while (position < text.size() && detail::isSpace(text[position]))
            {
                ++position;
            }
        }

        std::string identifier = detail::unescape(key);
        if (identifier.empty())
        {
            continue;
        }
        std::string value = detail::unescape(text.substr(position));

        std::string context;
        if (identifier.size() > 2 && identifier.back() == ']')
        {
            const std::size_t open = identifier.rfind('[');
            if (open != std::string::npos && open > 0)
            {
                context = identifier.substr(open + 1, identifier.size() - open - 2);
                identifier = identifier.substr(0, open);
            }
        }

        if (context.empty())
        {
            add(locale, identifier, value);
        }
        else
        {
            add(locale, context, identifier, value);
        }
        ++loaded;
    }
    return loaded;
}

} // namespace localisation
