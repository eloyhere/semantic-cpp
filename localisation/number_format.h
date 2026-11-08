#pragma once

// ===========================================================================
// localisation/number_format.h
//
// Three number presentations that plain localisation::formatNumber does not
// cover: compact (shortened) notation, scientific notation and rounding to
// significant digits.  All three keep the locale's separators, grouping and
// native digits because they render the digits through formatNumber rather
// than building strings out of ASCII characters.
//
// Compact notation follows the CLDR short and long compact decimal patterns:
//
//   * A magnitude is chosen from the pattern table, and the pattern itself
//     decides the divisor.  "0K" divides by 1000, while the Japanese pattern
//     "000" followed by U+4E07 (the ten-thousand character) divides by 10^4
//     and prints three integer digits, so 1200000 becomes "120" + U+4E07 and
//     not "1.2" + U+767E + U+4E07.
//   * CLDR stores a "0" pattern for magnitudes that are not abbreviated
//     (German below one million, Japanese below ten thousand).  A bare "0"
//     means "show the plain number", so 1500 is "1.500" in German and
//     "1,500" in Japanese.
//   * Two significant digits are kept, which is the CLDR/ICU compact
//     default: 1500 -> "1.5K", 12345 -> "12K", 1200000 -> "1.2M".
//   * The plural category is selected from the rounded, scaled value, so
//     Arabic picks the plural "few" word (U+0622 U+0644 U+0627 U+0641) for
//     3000 and the "one" word (U+0623 U+0644 U+0641) for 1000.
//
// Scientific notation renders a normalized mantissa (1 <= |m| < 10) with the
// locale's decimal separator and a two-digit minimum exponent: "1.23E+06".
//
// Significant-digit rounding rounds first and renders afterwards, so
// 1234.5 to three significant digits is "1,230" in English rather than
// "1,234.5" truncated.
//
// Example:
//
//   localisation::formatCompactNumber(1200000, localisation::Locale("en"))
//       -> "1.2M"
//   localisation::formatCompactNumber(1200000, localisation::Locale("de"))
//       -> "1,2 Mio."
//   localisation::formatScientificNumber(1234.5, localisation::Locale("en"))
//       -> "1.234E+03"
//   localisation::formatSignificantNumber(1234.5, localisation::Locale("en"), 1, 3)
//       -> "1,230"
//
// Limitations (deliberate, documented):
//
//   * Compact notation covers the CLDR magnitudes 1e3 .. 1e14.  Anything
//     smaller than 1000 or at least 1e15 is returned as a plain number.
//   * The "-alt-alphaNextToNumber" compact variants (used when an alphabetic
//     currency code sits next to the number) are not applied; this header
//     never formats a currency together with a compact number.
//   * Only the Latin numbering system's compact patterns are embedded, which
//     matches how localisation.h renders numbers for these locales.
// ===========================================================================

#include "currency_data.h"
#include "localisation.h"

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace localisation
{
namespace detail
{
// CLDR compact notation keeps two significant digits in both the short and
// the long form.
inline constexpr int compactSignificantDigits = 2;

// Decimal places that hold two significant digits for a value of this
// magnitude; also reports how many integer digits the rounded value has so the
// caller can pick the matching CLDR pattern.
struct SignificantRoundedDecimal
{
    std::string text; // plain ASCII fixed notation without trailing zeros
    double value = 0.0;
    int fractionDigits = 0;
    int integerDigits = 1;
};

inline auto countDecimalIntegerDigits(double value) -> int
{
    const double magnitude = std::fabs(value);
    if (magnitude < 1.0)
    {
        return 1;
    }
    return static_cast<int>(std::floor(std::log10(magnitude))) + 1;
}

// floor(log10(magnitude)), corrected so that an exact power of ten always maps
// to its own exponent even when log10 is one unit in the last place low.
inline auto decimalExponent(double magnitude) -> int
{
    if (!(magnitude > 0.0))
    {
        return 0;
    }
    int exponent = static_cast<int>(std::floor(std::log10(magnitude)));
    if (exponent > 308)
    {
        exponent = 308;
    }
    if (exponent < -323)
    {
        exponent = -323;
    }
    const double boundary = std::pow(10.0, static_cast<double>(exponent));
    if (magnitude < boundary)
    {
        --exponent;
    }
    else if (magnitude >= boundary * 10.0)
    {
        ++exponent;
    }
    return exponent;
}

inline auto roundSignificantDecimal(double value, int significantDigits) -> SignificantRoundedDecimal
{
    SignificantRoundedDecimal result;
    if (significantDigits < 1)
    {
        significantDigits = 1;
    }
    if (value == 0.0)
    {
        result.text = "0";
        return result;
    }

    int exponent = decimalExponent(std::fabs(value));
    int fractionDigits = significantDigits - 1 - exponent;

    double rounded = value;
    if (fractionDigits < 0)
    {
        // The value carries more integer digits than the budget allows, so the
        // rounding happens at a power of ten (1234.5 -> 1200 with two digits).
        // nearbyint honours the default round-half-even mode that CLDR asks for.
        const double scale = std::pow(10.0, static_cast<double>(-fractionDigits));
        rounded = std::nearbyint(value / scale) * scale;
        fractionDigits = 0;
    }
    if (fractionDigits > 15)
    {
        fractionDigits = 15;
    }

    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::fixed << std::setprecision(fractionDigits) << rounded;
    std::string text = stream.str();

    if (text.find('.') != std::string::npos)
    {
        while (!text.empty() && text.back() == '0')
        {
            text.pop_back();
        }
        if (!text.empty() && text.back() == '.')
        {
            text.pop_back();
        }
    }

    result.text = text;
    result.value = std::strtod(text.c_str(), nullptr);
    result.fractionDigits = fractionDigits;
    result.integerDigits = countDecimalIntegerDigits(result.value);
    return result;
}

// A CLDR compact pattern split around its digit placeholders.
struct CompactPatternParts
{
    std::string prefix;
    std::string suffix;
    int integerDigits = 0;
    bool plain = false; // a bare "0": CLDR's "do not abbreviate" marker
};

inline auto parseCompactDecimalPattern(std::string_view pattern) -> CompactPatternParts
{
    CompactPatternParts parts;
    std::string prefix;
    std::string suffix;
    std::size_t index = 0;
    bool quoted = false;
    int stage = 0; // 0 before the number, 1 inside it, 2 after it

    while (index < pattern.size())
    {
        const char character = pattern[index];
        if (quoted)
        {
            if (character == '\'')
            {
                quoted = false;
            }
            else
            {
                (stage == 2 ? suffix : prefix).push_back(character);
            }
            ++index;
            continue;
        }
        if (character == '\'')
        {
            quoted = true;
            ++index;
            continue;
        }
        if (stage < 2 && (character == '0' || character == '#'))
        {
            stage = 1;
            if (character == '0')
            {
                ++parts.integerDigits;
            }
            ++index;
            continue;
        }
        if (stage == 1 && (character == ',' || character == '.'))
        {
            ++index;
            continue;
        }
        if (stage == 1)
        {
            stage = 2;
        }
        (stage == 2 ? suffix : prefix).push_back(character);
        ++index;
    }

    if (parts.integerDigits < 1)
    {
        parts.integerDigits = 1;
    }
    parts.prefix = std::move(prefix);
    parts.suffix = std::move(suffix);
    parts.plain = parts.prefix.empty() && parts.suffix.empty() && parts.integerDigits == 1;
    return parts;
}

inline auto compactPatternRecord(const Locale &locale) -> const CompactPatternEntry &
{
    for (const CompactPatternEntry &entry : compactPatterns)
    {
        if (locale.tag() == entry.tag)
        {
            return entry;
        }
    }
    for (const CompactPatternEntry &entry : compactPatterns)
    {
        if (locale.language() == entry.tag)
        {
            return entry;
        }
    }
    return compactPatterns[0]; // the "en" row of the generated table
}

// The digit count of a magnitude is the same for every plural category, so any
// non-null cell of the row describes the pattern shape.
inline auto compactRawPattern(const CompactPatternEntry &entry, int magnitudeIndex, bool shortForm, int category)
    -> const char *
{
    if (magnitudeIndex < 0 || magnitudeIndex >= compactMagnitudeCount)
    {
        return nullptr;
    }
    const char *const *row = shortForm ? entry.shortForms[magnitudeIndex] : entry.longForms[magnitudeIndex];

    if (category >= 0 && category < 6 && row[category] != nullptr)
    {
        return row[category];
    }
    if (row[5] != nullptr)
    {
        return row[5];
    }
    for (int column = 0; column < 6; ++column)
    {
        if (row[column] != nullptr)
        {
            return row[column];
        }
    }
    return nullptr;
}
} // namespace detail

// Shortened notation, e.g. "1.2M".  ``shortForm`` selects the CLDR short
// patterns; false selects the long ones ("1.2 million").
inline auto formatCompactNumber(double value, const Locale &locale, bool shortForm = true) -> std::string
{
    if (!std::isfinite(value))
    {
        return formatNumber(value, locale);
    }

    const double magnitude = std::fabs(value);
    if (magnitude < 1000.0 || magnitude >= 1e15)
    {
        return formatNumber(value, locale);
    }

    const detail::CompactPatternEntry &entry = detail::compactPatternRecord(locale);

    int magnitudeIndex = 0;
    double boundary = 1000.0;
    for (int index = 0; index < detail::compactMagnitudeCount; ++index)
    {
        if (magnitude >= boundary)
        {
            magnitudeIndex = index;
        }
        boundary *= 10.0;
    }

    // Refine the magnitude: the pattern decides the divisor, and the rounded
    // quotient may need more digits than the pattern at this magnitude
    // provides.  Widening the magnitude by one step keeps the same unit and
    // adds a digit, so the loop settles after one or two passes.
    for (int guard = 0; guard < 4; ++guard)
    {
        const char *raw = detail::compactRawPattern(entry, magnitudeIndex, shortForm, 5);
        if (raw == nullptr)
        {
            return formatNumber(value, locale);
        }
        const detail::CompactPatternParts parts = detail::parseCompactDecimalPattern(raw);
        if (parts.plain)
        {
            // CLDR uses a bare "0" to mean "this magnitude is not abbreviated".
            return formatNumber(value, locale);
        }

        const int unitExponent = magnitudeIndex + 3 - (parts.integerDigits - 1);
        const double unit = std::pow(10.0, static_cast<double>(unitExponent));
        const detail::SignificantRoundedDecimal scaled =
            detail::roundSignificantDecimal(magnitude / unit, detail::compactSignificantDigits);

        const int targetIndex = unitExponent + (scaled.integerDigits - 1) - 3;
        if (targetIndex == magnitudeIndex || targetIndex < 0 || targetIndex >= detail::compactMagnitudeCount)
        {
            break;
        }
        magnitudeIndex = targetIndex;
    }

    const char *shape = detail::compactRawPattern(entry, magnitudeIndex, shortForm, 5);
    if (shape == nullptr)
    {
        return formatNumber(value, locale);
    }
    const detail::CompactPatternParts shapeParts = detail::parseCompactDecimalPattern(shape);
    if (shapeParts.plain)
    {
        return formatNumber(value, locale);
    }

    const int unitExponent = magnitudeIndex + 3 - (shapeParts.integerDigits - 1);
    const double unit = std::pow(10.0, static_cast<double>(unitExponent));
    detail::SignificantRoundedDecimal scaled =
        detail::roundSignificantDecimal(magnitude / unit, detail::compactSignificantDigits);

    // The plural category comes from the rounded, scaled value.
    const int category = static_cast<int>(pluralCategory(locale, scaled.value));
    const char *selected = detail::compactRawPattern(entry, magnitudeIndex, shortForm, category);
    if (selected == nullptr)
    {
        return formatNumber(value, locale);
    }
    const detail::CompactPatternParts parts = detail::parseCompactDecimalPattern(selected);

    if (value < 0.0)
    {
        scaled.value = -scaled.value;
    }

    NumberFormat format;
    format.minimumFractionDigits = 0;
    format.maximumFractionDigits = scaled.fractionDigits;
    format.grouping = false;
    const std::string number = formatNumber(scaled.value, locale, format);

    std::string result;
    result.reserve(parts.prefix.size() + number.size() + parts.suffix.size());
    result.append(parts.prefix);
    result.append(number);
    result.append(parts.suffix);
    return result;
}

// Scientific notation with a normalized mantissa.
inline auto formatScientificNumber(double value, const Locale &locale, int minimumFractionDigits = 0,
                                   int maximumFractionDigits = 3) -> std::string
{
    if (std::isnan(value))
    {
        return "NaN";
    }
    if (std::isinf(value))
    {
        return value < 0.0 ? "-inf" : "inf";
    }

    int minimum = minimumFractionDigits;
    if (minimum < 0)
    {
        minimum = 0;
    }
    if (minimum > 15)
    {
        minimum = 15;
    }
    int maximum = maximumFractionDigits;
    if (maximum < minimum)
    {
        maximum = minimum;
    }
    if (maximum > 15)
    {
        maximum = 15;
    }

    int exponent = 0;
    double mantissa = value;
    if (mantissa != 0.0)
    {
        exponent = detail::decimalExponent(std::fabs(mantissa));
        mantissa /= std::pow(10.0, static_cast<double>(exponent));
        while (std::fabs(mantissa) >= 10.0)
        {
            mantissa /= 10.0;
            ++exponent;
        }
        while (std::fabs(mantissa) < 1.0)
        {
            mantissa *= 10.0;
            --exponent;
        }
        // Rounding the mantissa can carry into the next decade (9.99 -> 10).
        if (std::fabs(detail::roundSignificantDecimal(mantissa, maximum + 1).value) >= 10.0)
        {
            mantissa /= 10.0;
            ++exponent;
        }
    }

    NumberFormat format;
    format.minimumFractionDigits = minimum;
    format.maximumFractionDigits = maximum;
    format.grouping = false;
    std::string result = formatNumber(mantissa, locale, format);

    result.push_back('E');
    result.push_back(exponent < 0 ? '-' : '+');
    std::string digits = std::to_string(exponent < 0 ? -exponent : exponent);
    while (digits.size() < 2)
    {
        digits.insert(digits.begin(), '0');
    }
    result.append(digits);
    return result;
}

// Round to significant digits, then render with the locale's digits.
inline auto formatSignificantNumber(double value, const Locale &locale, int minimumSignificantDigits = 1,
                                    int maximumSignificantDigits = 3) -> std::string
{
    if (std::isnan(value))
    {
        return "NaN";
    }
    if (std::isinf(value))
    {
        return value < 0.0 ? "-inf" : "inf";
    }

    int minimum = minimumSignificantDigits;
    if (minimum < 1)
    {
        minimum = 1;
    }
    if (minimum > 15)
    {
        minimum = 15;
    }
    int maximum = maximumSignificantDigits;
    if (maximum < minimum)
    {
        maximum = minimum;
    }
    if (maximum > 15)
    {
        maximum = 15;
    }

    if (value == 0.0)
    {
        int fractionDigits = minimum - 1;
        if (fractionDigits > 15)
        {
            fractionDigits = 15;
        }
        NumberFormat format;
        format.minimumFractionDigits = fractionDigits;
        format.maximumFractionDigits = fractionDigits;
        return formatNumber(0.0, locale, format);
    }

    const int exponent = detail::decimalExponent(std::fabs(value));

    const int maximumFractionDigits = maximum - 1 - exponent;
    if (maximumFractionDigits >= 0)
    {
        int minimumFractionDigits = minimum - 1 - exponent;
        if (minimumFractionDigits < 0)
        {
            minimumFractionDigits = 0;
        }
        if (minimumFractionDigits > maximumFractionDigits)
        {
            minimumFractionDigits = maximumFractionDigits;
        }
        NumberFormat format;
        format.minimumFractionDigits = minimumFractionDigits;
        format.maximumFractionDigits = maximumFractionDigits > 15 ? 15 : maximumFractionDigits;
        return formatNumber(value, locale, format);
    }

    // The value is larger than the significant-digit budget, so rounding
    // happens above the decimal point (1234.5 -> 1230 with three digits).
    // nearbyint keeps the round-half-even behaviour CLDR asks for.
    int power = -maximumFractionDigits;
    if (power > 300)
    {
        power = 300;
    }
    const double scale = std::pow(10.0, static_cast<double>(power));
    const double rounded = std::nearbyint(value / scale) * scale;

    NumberFormat format;
    format.minimumFractionDigits = 0;
    format.maximumFractionDigits = 0;
    return formatNumber(rounded, locale, format);
}
} // namespace localisation
