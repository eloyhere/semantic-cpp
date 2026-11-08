#include "localisation/currency.h"

#include "test_framework.h"

#include <cmath>
#include <string>
#include <string_view>

namespace
{
using localisation::CurrencyStyle;
using localisation::Locale;

const Locale english("en");
const Locale german("de");
const Locale french("fr");
const Locale japanese("ja");
const Locale chinese("zh");
const Locale hindi("hi");
const Locale arabic("ar");
const Locale unknown("xx");

// Test sources stay pure ASCII, like the rest of the repository, so every
// non-ASCII code point is spelled out as its UTF-8 bytes.
const std::string nbsp = "\xC2\xA0";           // U+00A0 NO-BREAK SPACE
const std::string narrowNbsp = "\xE2\x80\xAF"; // U+202F NARROW NO-BREAK SPACE
const std::string euro = "\xE2\x82\xAC";       // U+20AC EURO SIGN
const std::string pound = "\xC2\xA3";          // U+00A3 POUND SIGN
const std::string yen = "\xC2\xA5";            // U+00A5 YEN SIGN
const std::string fullwidthYen = "\xEF\xBF\xA5"; // U+FFE5 FULLWIDTH YEN SIGN
const std::string rupee = "\xE2\x82\xB9";      // U+20B9 INDIAN RUPEE SIGN
const std::string yuanHan = "\xE5\x85\x83";    // U+5143 CJK UNIFIED IDEOGRAPH
const std::string rlm = "\xE2\x80\x8F";        // U+200F RIGHT-TO-LEFT MARK
const std::string alm = "\xD8\x9C";            // U+061C ARABIC LETTER MARK
const std::string arabicGroup = "\xD9\xAC";    // U+066C ARABIC THOUSANDS SEPARATOR
const std::string arabicDecimal = "\xD9\xAB";  // U+066B ARABIC DECIMAL SEPARATOR

// Map an ASCII number such as "1,234.50" to Arabic-Indic digits and Arabic
// separators: U+0660..U+0669 for the digits.
std::string arabicNumber(const std::string &ascii)
{
    std::string result;
    for (char character : ascii)
    {
        if (character >= '0' && character <= '9')
        {
            result.push_back(static_cast<char>(0xD9));
            result.push_back(static_cast<char>(0xA0 + (character - '0')));
        }
        else if (character == ',')
        {
            result += arabicGroup;
        }
        else if (character == '.')
        {
            result += arabicDecimal;
        }
        else
        {
            result.push_back(character);
        }
    }
    return result;
}

std::string escapeBytes(const std::string &text)
{
    static const char *hex = "0123456789ABCDEF";
    std::string result;
    for (unsigned char character : text)
    {
        if (character >= 0x20 && character < 0x7F && character != '"' && character != '\\')
        {
            result.push_back(static_cast<char>(character));
        }
        else
        {
            result += "\\x";
            result.push_back(hex[character >> 4]);
            result.push_back(hex[character & 0x0F]);
        }
    }
    return result;
}

void checkCurrencyAt(double amount, std::string_view code, const Locale &locale, const std::string &expected,
                     const char *file, int line)
{
    const std::string actual = localisation::formatCurrency(amount, code, locale);
    if (actual != expected)
    {
        ::test::fail(file, line, "expected \"" + escapeBytes(expected) + "\" but got \"" + escapeBytes(actual) +
                                    "\" for " + std::string(code) + " in " + locale.tag());
    }
}

void checkAccountingAt(double amount, std::string_view code, const Locale &locale, const std::string &expected,
                       const char *file, int line)
{
    const std::string actual = localisation::formatCurrency(amount, code, locale, CurrencyStyle::accounting);
    if (actual != expected)
    {
        ::test::fail(file, line, "expected \"" + escapeBytes(expected) + "\" but got \"" + escapeBytes(actual) +
                                    "\" for " + std::string(code) + " in " + locale.tag() + " (accounting)");
    }
}

#define CHECK_CURRENCY(amount, code, locale, expected) \
    checkCurrencyAt((amount), (code), (locale), (expected), __FILE__, __LINE__)

#define CHECK_ACCOUNTING(amount, code, locale, expected) \
    checkAccountingAt((amount), (code), (locale), (expected), __FILE__, __LINE__)
} // namespace

TEST_CASE("currency: CLDR fraction digits")
{
    CHECK_EQ(localisation::currencyFractionDigits("USD"), 2);
    CHECK_EQ(localisation::currencyFractionDigits("EUR"), 2);
    CHECK_EQ(localisation::currencyFractionDigits("JPY"), 0);
    CHECK_EQ(localisation::currencyFractionDigits("KRW"), 0);
    CHECK_EQ(localisation::currencyFractionDigits("KWD"), 3);
    CHECK_EQ(localisation::currencyFractionDigits("BHD"), 3);
    CHECK_EQ(localisation::currencyFractionDigits("CLF"), 4);

    // Codes outside the table keep the two-digit default, and input is
    // normalised so lower-case and padded codes resolve too.
    CHECK_EQ(localisation::currencyFractionDigits("XYZ"), 2);
    CHECK_EQ(localisation::currencyFractionDigits("jpy"), 0);
    CHECK_EQ(localisation::currencyFractionDigits(" KWD "), 3);
}

TEST_CASE("currency: localised symbols")
{
    CHECK_EQ(localisation::currencySymbol("USD", english), std::string("$"));
    CHECK_EQ(localisation::currencySymbol("USD", german), std::string("$"));
    CHECK_EQ(localisation::currencySymbol("USD", french), std::string("$US"));
    CHECK_EQ(localisation::currencySymbol("USD", chinese), std::string("US$"));
    CHECK_EQ(localisation::currencySymbol("USD", japanese), std::string("$"));

    CHECK_EQ(localisation::currencySymbol("EUR", english), euro);
    CHECK_EQ(localisation::currencySymbol("EUR", japanese), euro);

    // Japanese uses the fullwidth yen sign, Chinese the Latin one, and the
    // Japanese symbol for the yuan is the han character.
    CHECK_EQ(localisation::currencySymbol("JPY", japanese), fullwidthYen);
    CHECK_EQ(localisation::currencySymbol("JPY", chinese), "JP" + yen);
    CHECK_EQ(localisation::currencySymbol("CNY", chinese), yen);
    CHECK_EQ(localisation::currencySymbol("CNY", japanese), yuanHan);

    // A currency with no symbol in the locale falls back to the code, never to
    // another locale's symbol.
    CHECK_EQ(localisation::currencySymbol("KWD", german), std::string("KWD"));
    CHECK_EQ(localisation::currencySymbol("XYZ", german), std::string("XYZ"));
    CHECK_EQ(localisation::currencySymbol("usd", english), std::string("$"));
    CHECK_EQ(localisation::currencySymbol("", english), std::string());
}

TEST_CASE("currency: English, Japanese and Chinese patterns")
{
    CHECK_CURRENCY(1234.4, "USD", english, "$1,234.40");
    CHECK_CURRENCY(1234.4, "EUR", english, euro + "1,234.40");
    CHECK_CURRENCY(1234.4, "JPY", english, yen + "1,234");
    CHECK_CURRENCY(1234.4, "CNY", english, "CN" + yen + "1,234.40");
    CHECK_CURRENCY(1234.4, "INR", english, rupee + "1,234.40");
    CHECK_CURRENCY(1234.4, "GBP", english, pound + "1,234.40");
    CHECK_CURRENCY(1234.4, "KWD", english, "KWD" + nbsp + "1,234.400");

    CHECK_CURRENCY(1234.4, "USD", japanese, "$1,234.40");
    CHECK_CURRENCY(1234.4, "EUR", japanese, euro + "1,234.40");
    CHECK_CURRENCY(1234.4, "JPY", japanese, fullwidthYen + "1,234");
    CHECK_CURRENCY(1234.4, "CNY", japanese, yuanHan + "1,234.40");
    CHECK_CURRENCY(1234.4, "INR", japanese, rupee + "1,234.40");
    CHECK_CURRENCY(1234.4, "GBP", japanese, pound + "1,234.40");
    CHECK_CURRENCY(1234.4, "KWD", japanese, "KWD" + nbsp + "1,234.400");

    CHECK_CURRENCY(1234.4, "USD", chinese, "US$1,234.40");
    CHECK_CURRENCY(1234.4, "EUR", chinese, euro + "1,234.40");
    CHECK_CURRENCY(1234.4, "JPY", chinese, "JP" + yen + "1,234");
    CHECK_CURRENCY(1234.4, "CNY", chinese, yen + "1,234.40");
    CHECK_CURRENCY(1234.4, "INR", chinese, rupee + "1,234.40");
    CHECK_CURRENCY(1234.4, "GBP", chinese, pound + "1,234.40");
    CHECK_CURRENCY(1234.4, "KWD", chinese, "KWD" + nbsp + "1,234.400");
}

TEST_CASE("currency: German and French symbol placement")
{
    // CLDR places the symbol after the amount with a no-break space in these
    // locales, unlike the English prefix form.
    CHECK_CURRENCY(1234.4, "USD", german, "1.234,40" + nbsp + "$");
    CHECK_CURRENCY(1234.4, "EUR", german, "1.234,40" + nbsp + euro);
    CHECK_CURRENCY(1234.4, "JPY", german, "1.234" + nbsp + yen);
    CHECK_CURRENCY(1234.4, "CNY", german, "1.234,40" + nbsp + "CN" + yen);
    CHECK_CURRENCY(1234.4, "INR", german, "1.234,40" + nbsp + rupee);
    CHECK_CURRENCY(1234.4, "GBP", german, "1.234,40" + nbsp + pound);
    CHECK_CURRENCY(1234.4, "KWD", german, "1.234,400" + nbsp + "KWD");

    CHECK_CURRENCY(1234.4, "USD", french, "1" + narrowNbsp + "234,40" + nbsp + "$US");
    CHECK_CURRENCY(1234.4, "EUR", french, "1" + narrowNbsp + "234,40" + nbsp + euro);
    CHECK_CURRENCY(1234.4, "JPY", french, "1" + narrowNbsp + "234" + nbsp + "JPY");
    CHECK_CURRENCY(1234.4, "CNY", french, "1" + narrowNbsp + "234,40" + nbsp + "CNY");
    CHECK_CURRENCY(1234.4, "INR", french, "1" + narrowNbsp + "234,40" + nbsp + rupee);
    CHECK_CURRENCY(1234.4, "GBP", french, "1" + narrowNbsp + "234,40" + nbsp + pound + "GB");
    CHECK_CURRENCY(1234.4, "KWD", french, "1" + narrowNbsp + "234,400" + nbsp + "KWD");
}

TEST_CASE("currency: Hindi grouping and symbols")
{
    CHECK_CURRENCY(1234.4, "USD", hindi, "$1,234.40");
    CHECK_CURRENCY(1234.4, "EUR", hindi, euro + "1,234.40");
    CHECK_CURRENCY(1234.4, "JPY", hindi, "JP" + yen + "1,234");
    CHECK_CURRENCY(1234.4, "CNY", hindi, "CN" + yen + "1,234.40");
    CHECK_CURRENCY(1234.4, "INR", hindi, rupee + "1,234.40");
    CHECK_CURRENCY(1234.4, "GBP", hindi, pound + "1,234.40");
    CHECK_CURRENCY(1234.4, "KWD", hindi, "KWD" + nbsp + "1,234.400");

    // The Indian grouping pattern comes from formatNumber, not from currency.h.
    CHECK_CURRENCY(1234567.89, "INR", hindi, rupee + "12,34,567.89");
}

TEST_CASE("currency: Arabic digits and direction marks")
{
    CHECK_CURRENCY(1234.4, "USD", arabic, rlm + arabicNumber("1,234.40") + nbsp + "US$");
    CHECK_CURRENCY(1234.4, "EUR", arabic, rlm + arabicNumber("1,234.40") + nbsp + euro);
    CHECK_CURRENCY(1234.4, "JPY", arabic, rlm + arabicNumber("1,234") + nbsp + "JP" + yen);
    CHECK_CURRENCY(1234.4, "CNY", arabic, rlm + arabicNumber("1,234.40") + nbsp + "CN" + yen);
    CHECK_CURRENCY(1234.4, "INR", arabic, rlm + arabicNumber("1,234.40") + nbsp + rupee);
    CHECK_CURRENCY(1234.4, "GBP", arabic, rlm + arabicNumber("1,234.40") + nbsp + "UK" + pound);
    CHECK_CURRENCY(1234.4, "KWD", arabic,
                   rlm + arabicNumber("1,234.400") + nbsp + "\xD8\xAF.\xD9\x83." + rlm);
}

TEST_CASE("currency: negative amounts")
{
    CHECK_CURRENCY(-1234.5, "USD", english, "-$1,234.50");
    CHECK_CURRENCY(-1234.5, "USD", german, "-1.234,50" + nbsp + "$");
    CHECK_CURRENCY(-1234.5, "USD", french, "-1" + narrowNbsp + "234,50" + nbsp + "$US");
    CHECK_CURRENCY(-1234.5, "USD", japanese, "-$1,234.50");
    CHECK_CURRENCY(-1234.5, "USD", chinese, "-US$1,234.50");
    CHECK_CURRENCY(-1234.5, "USD", hindi, "-$1,234.50");
    CHECK_CURRENCY(-1234.5, "USD", arabic, rlm + "-" + arabicNumber("1,234.50") + nbsp + "US$");

    CHECK_CURRENCY(-1234, "JPY", english, "-" + yen + "1,234");
    CHECK_CURRENCY(-1234, "JPY", german, "-1.234" + nbsp + yen);
    CHECK_CURRENCY(-1234, "JPY", japanese, "-" + fullwidthYen + "1,234");
    CHECK_CURRENCY(-1234, "JPY", chinese, "-JP" + yen + "1,234");
    CHECK_CURRENCY(-1234, "JPY", hindi, "-JP" + yen + "1,234");
    CHECK_CURRENCY(-1234, "JPY", arabic, rlm + "-" + arabicNumber("1,234") + nbsp + "JP" + yen);
}

TEST_CASE("currency: accounting patterns")
{
    CHECK_ACCOUNTING(-1234.5, "USD", english, "($1,234.50)");
    CHECK_ACCOUNTING(-1234.5, "USD", japanese, "($1,234.50)");
    CHECK_ACCOUNTING(-1234.5, "USD", chinese, "(US$1,234.50)");
    CHECK_ACCOUNTING(-1234.5, "USD", french, "(1" + narrowNbsp + "234,50" + nbsp + "$US)");
    CHECK_ACCOUNTING(-1234.5, "USD", arabic, "(" + alm + arabicNumber("1,234.50") + nbsp + "US$)");

    // Locales without a separate accounting pattern keep the standard form.
    CHECK_ACCOUNTING(-1234.5, "USD", german, "-1.234,50" + nbsp + "$");
    CHECK_ACCOUNTING(-1234.5, "USD", hindi, "-$1,234.50");

    // Accounting does not change positive amounts.
    CHECK_ACCOUNTING(1234.5, "USD", english, "$1,234.50");
}

TEST_CASE("currency: unknown codes fall back to the code")
{
    CHECK_CURRENCY(1234.5, "XYZ", english, "XYZ" + nbsp + "1,234.50");
    CHECK_CURRENCY(1234.5, "XYZ", german, "1.234,50" + nbsp + "XYZ");
    CHECK_CURRENCY(1234.5, "XYZ", french, "1" + narrowNbsp + "234,50" + nbsp + "XYZ");
    CHECK_CURRENCY(1234.5, "XYZ", japanese, "XYZ" + nbsp + "1,234.50");

    // Lower-case input is normalised to the ISO-style upper-case code.
    CHECK_CURRENCY(1234.5, "usd", english, "$1,234.50");

    // Alphabetic codes get the CLDR currency-spacing no-break space; symbols
    // ending in a currency sign do not.
    CHECK_CURRENCY(1234.5, "CHF", english, "CHF" + nbsp + "1,234.50");
    CHECK_CURRENCY(1234.5, "CHF", german, "1.234,50" + nbsp + "CHF");
    CHECK_CURRENCY(1234.5, "CHF", japanese, "CHF" + nbsp + "1,234.50");
    CHECK_CURRENCY(1234.5, "CNY", english, "CN" + yen + "1,234.50");
}

TEST_CASE("currency: unknown locales and special values")
{
    CHECK_CURRENCY(1234.5, "USD", unknown, "$1,234.50");
    CHECK_CURRENCY(0.0, "USD", english, "$0.00");
    CHECK_CURRENCY(0.0, "JPY", japanese, fullwidthYen + "0");

    CHECK_EQ(localisation::formatCurrency(std::nan(""), "USD", english), std::string("NaN"));
    CHECK_EQ(localisation::formatCurrency(1.0 / 0.0, "USD", english), std::string("inf"));
    CHECK_EQ(localisation::formatCurrency(-1.0 / 0.0, "USD", english), std::string("-inf"));
}
