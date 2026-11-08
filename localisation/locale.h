#pragma once

// ===========================================================================
// localisation/locale.h
//
// BCP 47 locale tags and the small ASCII string helpers they are built from.
//
// This header exists to break a dependency cycle.  The message layer
// (localisation.h) needs to format dates and times, so it calls the
// DateTimeFormatter of localisation/datetime.h; the date/time layer only needs
// to know what a Locale is.  While Locale lived in localisation.h, datetime.h
// had to include the whole message/catalogue layer, and localisation.h could
// not include datetime.h back.  Locale and its string helpers therefore live
// here, both other headers include this one, and localisation.h is the only
// one of the three that sees the other two at once:
//
//   locale.h        <- datetime.h <- localisation.h
//
// The surface is unchanged: localisation::Locale, localisation::detail
// isSpace/isDigit/isAlpha/isAlnum/toLower/toUpper/asciiLower/asciiUpper/
// asciiTitle/trimView/trim and operator<< behave exactly as they did as part
// of localisation.h, and including localisation.h still provides all of them.
//
// A Locale is a BCP 47 subset: language[-script][-region].  Canonicalisation
// lower-cases the language, title-cases the script and upper-cases the region.
// Likely scripts are filled in where they change behaviour: zh-CN/zh-SG ->
// Hans, zh-TW/zh-HK/zh-MO -> Hant, yue -> Hant, mn-CN -> Mong and mn-MN ->
// Cyrl.  Variants, extensions and private-use subtags are ignored.
//
// Example:
//
//   const localisation::Locale locale("zh-hant_tw");   // zh-Hant-TW
//   for (const localisation::Locale &candidate : locale.fallbacks())
//   {
//       std::cout << candidate << '\n';   // zh-Hant-TW, zh-Hant, zh
//   }
// ===========================================================================

#include <cstddef>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace localisation
{
namespace detail
{
inline auto isSpace(char character) -> bool
{
    return character == ' ' || character == '\t' || character == '\r' || character == '\n' || character == '\f' ||
           character == '\v';
}

inline auto isDigit(char character) -> bool { return character >= '0' && character <= '9'; }

inline auto isAlpha(char character) -> bool
{
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z');
}

inline auto isAlnum(char character) -> bool { return isAlpha(character) || isDigit(character); }

inline auto toLower(char character) -> char
{
    return (character >= 'A' && character <= 'Z') ? static_cast<char>(character - 'A' + 'a') : character;
}

inline auto toUpper(char character) -> char
{
    return (character >= 'a' && character <= 'z') ? static_cast<char>(character - 'a' + 'A') : character;
}

inline auto asciiLower(std::string_view text) -> std::string
{
    std::string result;
    result.reserve(text.size());
    for (char character : text)
    {
        result.push_back(toLower(character));
    }
    return result;
}

inline auto asciiUpper(std::string_view text) -> std::string
{
    std::string result;
    result.reserve(text.size());
    for (char character : text)
    {
        result.push_back(toUpper(character));
    }
    return result;
}

inline auto asciiTitle(std::string_view text) -> std::string
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

inline auto trimView(std::string_view text) -> std::string_view
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

inline auto trim(std::string_view text) -> std::string { return std::string(trimView(text)); }
} // namespace detail

// ---------------------------------------------------------------------------
// Locale: a BCP 47 subset (language[-script][-region])
//
// The tag is immutable: every operation returns a new Locale, and there is no
// process-wide "current locale", so a Locale can be shared across threads and
// passed by value without synchronisation.
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

    auto tag() const noexcept -> const std::string & { return tag_; }
    auto language() const noexcept -> const std::string & { return language_; }
    auto script() const noexcept -> const std::string & { return script_; }
    auto region() const noexcept -> const std::string & { return region_; }

    auto specified() const noexcept -> bool { return language_ != "und" && !language_.empty(); }

    // RFC 4647 lookup chain: zh-Hant-TW -> zh-Hant -> zh.
    auto fallbacks() const -> std::vector<Locale>
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

    auto operator==(const Locale &other) const noexcept -> bool { return tag_ == other.tag_; }
    auto operator!=(const Locale &other) const noexcept -> bool { return tag_ != other.tag_; }
    auto operator<(const Locale &other) const noexcept -> bool { return tag_ < other.tag_; }

  private:
    // Builds a locale without running likely-script inference.  The fallback
    // chain must be able to produce the bare language tag (yue -> yue), which
    // inference would otherwise turn back into yue-Hant.
    static auto raw(const std::string &language, const std::string &script, const std::string &region) -> Locale
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
    auto applyLikelyScript() -> void
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

    static auto isLanguageSubtag(const std::string &subtag) -> bool
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

    static auto isScriptSubtag(const std::string &subtag) -> bool
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

    static auto isRegionSubtag(const std::string &subtag) -> bool
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

    auto recompose() -> void
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

inline auto operator<<(std::ostream &stream, const Locale &locale) -> std::ostream & { return stream << locale.tag(); }

} // namespace localisation
