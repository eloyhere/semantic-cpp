#pragma once

// ===========================================================================
// localisation/currency.h
//
// Currency formatting on top of the locale-aware number rendering in
// localisation/localisation.h.
//
// Design notes:
//
//   * CLDR is the single source of truth.  Symbols, fraction-digit counts,
//     the standard and accounting currency patterns and the currency-spacing
//     rule all live in the generated table (currency_data.h), so a locale can
//     be added by regenerating data, not by editing code.
//   * The number itself is rendered by localisation::formatNumber, so
//     grouping (including the Indian 12,34,567 pattern), the decimal
//     separator and native digits all stay consistent with plain numbers.
//     Currency formatting only wraps that number in the CLDR pattern.
//   * The pattern decides placement, not the locale: "#,##0.00<nbsp><cur>"
//     (de, fr, ru) puts the symbol after the number, "<cur>#,##0.00" (en, ja,
//     zh) puts it before, and accounting patterns such as "(<cur>#,##0.00)"
//     supply their own parentheses.  <cur> stands for the U+00A4 currency
//     sign, the placeholder CLDR uses for the substituted symbol.
//   * Currency spacing follows CLDR's currencySpacing rule for the one case
//     that matters in practice: an ISO-style alphabetic code placed directly
//     against the digits ("CHF1,234.50" -> "CHF 1,234.50").  Symbols that end
//     in a currency-sign character ("$", "CN" followed by the yen sign, the
//     han character for the yuan, ...) are left untouched, which is what
//     CLDR's [[:^S:]&[:^Z:]] currencyMatch asks for and what every reference
//     implementation produces.
//   * Negative amounts use the pattern's own negative subpattern when CLDR
//     defines one ("(<euro sign>1,234.50)", "-$1,234.50") and otherwise
//     prepend an ASCII '-'.  NaN and infinity are returned as "NaN", "inf"
//     and "-inf", exactly as formatNumber does, because no currency
//     vocabulary applies.
//
// Example:
//
//   localisation::formatCurrency(1234.5, "USD", localisation::Locale("en"))
//       -> "$1,234.50"
//   localisation::formatCurrency(1234.5, "EUR", localisation::Locale("de"))
//       -> "1.234,50" + U+00A0 + <euro sign>
//   localisation::formatCurrency(1234, "JPY", localisation::Locale("ja"))
//       -> U+FFE5 + "1,234"
//   localisation::formatCurrency(-1234.5, "USD", localisation::Locale("en"))
//       -> "-$1,234.50"
//
// Limitations (deliberate, documented):
//
//   * Amounts are formatted, never converted.  There is no exchange rate and
//     no currency arithmetic in this header.
//   * Only the fraction digits of the currency are applied; the cash-rounding
//     variant (_cashDigits) is ignored.
//   * Plural agreement of currency display names ("1 US dollar" / "2 US
//     dollars") is not implemented: that is display-name data from CLDR
//     currencies.json, which this header does not embed.
//   * Currency spacing is applied only for ASCII-letter codes, so the table
//     does not need the full Unicode symbol/space classification.
//   * Locale resolution falls back from the exact tag to the language to
//     "en"; there is no script-aware inheritance (zh-Hant does not inherit
//     from zh-Hans data beyond that rule).
// ===========================================================================

#include "currency_data.h"
#include "localisation.h"

#include <cmath>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace localisation
{
// Which CLDR currency pattern family to use.  "accounting" is the pattern
// that renders negative amounts in parentheses in the locales that do so
// (en, fr, ja, zh, ...).
enum class CurrencyStyle
{
    standard,
    accounting
};

namespace detail
{
inline auto isAsciiCurrencyLetter(char character) -> bool
{
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z');
}

// ISO 4217 codes are three ASCII letters.  Input is trimmed and upper-cased so
// that "usd", "USD" and " USD " all resolve to the same row.
inline auto normalizeCurrencyCode(std::string_view code) -> std::string
{
    const std::string_view trimmed = trimView(code);
    std::string result;
    result.reserve(trimmed.size());
    for (char character : trimmed)
    {
        result.push_back((character >= 'a' && character <= 'z') ? static_cast<char>(character - 'a' + 'A') : character);
    }
    return result;
}

inline auto currencyRecord(std::string_view code) -> const CurrencyDataEntry *
{
    if (code.size() != 3)
    {
        return nullptr;
    }
    for (const CurrencyDataEntry &entry : currencyData)
    {
        if (entry.code[0] == code[0] && entry.code[1] == code[1] && entry.code[2] == code[2])
        {
            return &entry;
        }
    }
    return nullptr;
}

// Exact tag match wins over the language match; both win over the base table,
// which is the CLDR "en" row set.  A currency that has no override in a
// matched locale keeps the base symbol rather than falling through to another
// locale, because that is what CLDR inheritance does (missing entries come
// from root, not from a sibling locale).
inline auto currencySymbolText(std::string_view code, const Locale &locale) -> std::string
{
    for (const CurrencyLocaleSymbol &entry : currencySymbols)
    {
        if (locale.tag() == entry.tag && code == entry.code)
        {
            return entry.symbol;
        }
    }
    for (const CurrencyLocaleSymbol &entry : currencySymbols)
    {
        if (locale.language() == entry.tag && code == entry.code)
        {
            return entry.symbol;
        }
    }
    if (const CurrencyDataEntry *record = currencyRecord(code); record != nullptr && record->symbol != nullptr)
    {
        return record->symbol;
    }
    return std::string(code);
}

inline auto currencyPatternRecord(const Locale &locale) -> const CurrencyPatternEntry &
{
    for (const CurrencyPatternEntry &entry : currencyPatterns)
    {
        if (locale.tag() == entry.tag)
        {
            return entry;
        }
    }
    for (const CurrencyPatternEntry &entry : currencyPatterns)
    {
        if (locale.language() == entry.tag)
        {
            return entry;
        }
    }
    return currencyPatterns[0]; // the "en" row of the generated table
}

// Split "positive;negative".  Currency patterns do not use quoted literals in
// CLDR, so the first semicolon is always the separator.
inline auto splitCurrencyPattern(std::string_view pattern) -> std::pair<std::string_view, std::string_view>
{
    const std::size_t separator = pattern.find(';');
    if (separator == std::string_view::npos)
    {
        return {pattern, std::string_view()};
    }
    return {pattern.substr(0, separator), pattern.substr(separator + 1)};
}

// Replace the currency placeholder and apply the CLDR currency-spacing rule.
// The result still contains the digit placeholders; the caller splits it into
// the literal text that surrounds the rendered number.
inline auto substituteCurrencyPlaceholder(std::string_view pattern, std::string_view symbol, std::string_view spacing)
    -> std::string
{
    const std::size_t position = pattern.find("\xC2\xA4"); // U+00A4 CURRENCY SIGN
    if (position == std::string_view::npos)
    {
        return std::string(pattern);
    }

    const std::string_view before = pattern.substr(0, position);
    const std::string_view after = pattern.substr(position + 2);

    const bool symbolLeadsNumber = !after.empty() && (after.front() == '0' || after.front() == '#');
    const bool symbolTrailsNumber = !before.empty() && (before.back() == '0' || before.back() == '#');

    const bool symbolIsCode = !symbol.empty() && isAsciiCurrencyLetter(symbol.back());
    const bool symbolStartsAsCode = !symbol.empty() && isAsciiCurrencyLetter(symbol.front());

    std::string result;
    result.reserve(pattern.size() + symbol.size() + spacing.size());
    result.append(before);
    if (symbolTrailsNumber && symbolStartsAsCode)
    {
        result.append(spacing);
    }
    result.append(symbol);
    if (symbolLeadsNumber && symbolIsCode)
    {
        result.append(spacing);
    }
    result.append(after);
    return result;
}

// Split the substituted pattern around the digit placeholders.
inline auto splitCurrencyRenderedPattern(const std::string &pattern) -> std::pair<std::string, std::string>
{
    const std::size_t first = pattern.find_first_of("#0");
    if (first == std::string::npos)
    {
        return {pattern, {}};
    }
    std::size_t last = first;
    while (last + 1 < pattern.size())
    {
        const char next = pattern[last + 1];
        if (next == '#' || next == '0' || next == ',' || next == '.')
        {
            ++last;
            continue;
        }
        break;
    }
    return {pattern.substr(0, first), pattern.substr(last + 1)};
}
} // namespace detail

// The localized symbol for a currency code, e.g. "$" for USD in "en" and
// "US$" in "fr".  A code with no symbol data falls back to the code itself.
inline auto currencySymbol(std::string_view code, const Locale &locale) -> std::string
{
    const std::string normalized = detail::normalizeCurrencyCode(code);
    if (normalized.empty())
    {
        return std::string();
    }
    return detail::currencySymbolText(normalized, locale);
}

// CLDR fraction digits for a currency: 0 for JPY, 3 for KWD, 2 by default.
inline auto currencyFractionDigits(std::string_view code) -> int
{
    const std::string normalized = detail::normalizeCurrencyCode(code);
    if (const detail::CurrencyDataEntry *record = detail::currencyRecord(normalized); record != nullptr)
    {
        return record->digits;
    }
    return 2;
}

// Format a monetary amount with the locale's currency pattern.
//
// The default style is the CLDR "standard" pattern; CurrencyStyle::accounting
// selects the accounting pattern, which is what renders negative amounts in
// parentheses for locales that use that convention.
inline auto formatCurrency(double amount, std::string_view code, const Locale &locale = Locale("en"),
                           CurrencyStyle style = CurrencyStyle::standard) -> std::string
{
    if (std::isnan(amount))
    {
        return "NaN";
    }
    if (std::isinf(amount))
    {
        return amount < 0.0 ? "-inf" : "inf";
    }

    const std::string normalized = detail::normalizeCurrencyCode(code);
    const std::string symbol = normalized.empty() ? std::string() : detail::currencySymbolText(normalized, locale);

    int digits = 2;
    if (const detail::CurrencyDataEntry *record = detail::currencyRecord(normalized); record != nullptr)
    {
        digits = record->digits;
    }

    NumberFormat format;
    format.minimumFractionDigits = digits;
    format.maximumFractionDigits = digits;
    format.grouping = true;
    const std::string number = formatNumber(std::fabs(amount), locale, format);

    const detail::CurrencyPatternEntry &record = detail::currencyPatternRecord(locale);
    const std::string_view pattern = style == CurrencyStyle::accounting ? record.accounting : record.standard;
    const auto [positive, negative] = detail::splitCurrencyPattern(pattern);

    const bool useNegativeSubpattern = amount < 0.0 && !negative.empty();
    const std::string substituted = detail::substituteCurrencyPlaceholder(
        useNegativeSubpattern ? negative : positive, symbol, record.spacing);
    const auto [prefix, suffix] = detail::splitCurrencyRenderedPattern(substituted);

    std::string result;
    result.reserve(prefix.size() + number.size() + suffix.size() + 1);
    if (amount < 0.0 && !useNegativeSubpattern)
    {
        result.push_back('-');
    }
    result.append(prefix);
    result.append(number);
    result.append(suffix);
    return result;
}
} // namespace localisation
