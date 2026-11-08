#include "localisation/number_format.h"

#include "test_framework.h"

#include <cmath>
#include <string>

namespace
{
using localisation::Locale;

const Locale english("en");
const Locale german("de");
const Locale french("fr");
const Locale japanese("ja");
const Locale chinese("zh");
const Locale hindi("hi");
const Locale arabic("ar");
const Locale russian("ru");

// Test sources stay pure ASCII, like the rest of the repository.
const std::string nbsp = "\xC2\xA0";      // U+00A0 NO-BREAK SPACE
const std::string narrowNbsp = "\xE2\x80\xAF"; // U+202F NARROW NO-BREAK SPACE
const std::string arabicGroup = "\xD9\xAC";    // U+066C ARABIC THOUSANDS SEPARATOR
const std::string arabicDecimal = "\xD9\xAB";  // U+066B ARABIC DECIMAL SEPARATOR

const std::string arabicThousandWord = "\xD8\xA3\xD9\x84\xD9\x81";                     // U+0623 U+0644 U+0641
const std::string arabicThousandsWord = "\xD8\xA2\xD9\x84\xD8\xA7\xD9\x81";            // U+0622 U+0644 U+0627 U+0641
const std::string arabicMillionWord = "\xD9\x85\xD9\x84\xD9\x8A\xD9\x88\xD9\x86";      // U+0645 U+0644 U+064A U+0648 U+0646
const std::string arabicBillionWord = "\xD9\x85\xD9\x84\xD9\x8A\xD8\xA7\xD8\xB1";      // U+0645 U+0644 U+064A U+0627 U+0631
const std::string hindiThousandWord = "\xE0\xA4\xB9\xE0\xA4\x9C\xE0\xA4\xBC\xE0\xA4\xBE\xE0\xA4\xB0";
const std::string hindiLakhWord = "\xE0\xA4\xB2\xE0\xA4\xBE\xE0\xA4\x96";              // U+0932 U+093E U+0916
const std::string hindiCroreWord = "\xE0\xA4\x85\xE0\xA5\xB0";                         // U+0905 U+0970
const std::string hanTenThousand = "\xE4\xB8\x87";                                     // U+4E07
const std::string hanHundredMillion = "\xE5\x84\x84";                                  // U+5104
const std::string hanHundredMillionSimplified = "\xE4\xBA\xBF";                        // U+4EBF
const std::string russianMillionWord = "\xD0\xBC\xD0\xBB\xD0\xBD";                     // U+043C U+043B U+043D

// Map an ASCII number such as "1,230" to Arabic-Indic digits (U+0660..U+0669).
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
} // namespace

TEST_CASE("number_format: compact notation in English")
{
    CHECK_EQ(localisation::formatCompactNumber(999, english), std::string("999"));
    CHECK_EQ(localisation::formatCompactNumber(1000, english), std::string("1K"));
    CHECK_EQ(localisation::formatCompactNumber(1500, english), std::string("1.5K"));
    CHECK_EQ(localisation::formatCompactNumber(1200000, english), std::string("1.2M"));
    CHECK_EQ(localisation::formatCompactNumber(1.2e9, english), std::string("1.2B"));
    CHECK_EQ(localisation::formatCompactNumber(1.2e12, english), std::string("1.2T"));

    // Two significant digits, and a growing digit count as the magnitude grows.
    CHECK_EQ(localisation::formatCompactNumber(12345, english), std::string("12K"));
    CHECK_EQ(localisation::formatCompactNumber(123456, english), std::string("120K"));
    CHECK_EQ(localisation::formatCompactNumber(12345678, english), std::string("12M"));

    // Values below the first magnitude stay plain.
    CHECK_EQ(localisation::formatCompactNumber(0.5, english), std::string("0.5"));
    CHECK_EQ(localisation::formatCompactNumber(0, english), std::string("0"));
    CHECK_EQ(localisation::formatCompactNumber(-1500, english), std::string("-1.5K"));
}

TEST_CASE("number_format: compact notation in German and French")
{
    // German does not abbreviate below one million, so CLDR's "0" pattern
    // means "print the plain number".
    CHECK_EQ(localisation::formatCompactNumber(999, german), std::string("999"));
    CHECK_EQ(localisation::formatCompactNumber(1000, german), std::string("1.000"));
    CHECK_EQ(localisation::formatCompactNumber(1500, german), std::string("1.500"));
    CHECK_EQ(localisation::formatCompactNumber(1200000, german), "1,2" + nbsp + "Mio.");
    CHECK_EQ(localisation::formatCompactNumber(1.2e9, german), "1,2" + nbsp + "Mrd.");
    CHECK_EQ(localisation::formatCompactNumber(-1500, german), std::string("-1.500"));

    // French uses a comma decimal separator and a no-break space before the
    // abbreviation.
    CHECK_EQ(localisation::formatCompactNumber(1000, french), "1" + nbsp + "k");
    CHECK_EQ(localisation::formatCompactNumber(1500, french), "1,5" + nbsp + "k");
    CHECK_EQ(localisation::formatCompactNumber(1200000, french), "1,2" + nbsp + "M");
    CHECK_EQ(localisation::formatCompactNumber(1.2e9, french), "1,2" + nbsp + "Md");
    CHECK_EQ(localisation::formatCompactNumber(0.5, french), std::string("0,5"));
}

TEST_CASE("number_format: compact notation in CJK and Hindi")
{
    // Japanese and Chinese abbreviate at 10^4, not at 10^3.
    CHECK_EQ(localisation::formatCompactNumber(1500, japanese), std::string("1,500"));
    CHECK_EQ(localisation::formatCompactNumber(12345, japanese), "1.2" + hanTenThousand);
    CHECK_EQ(localisation::formatCompactNumber(1200000, japanese), "120" + hanTenThousand);
    CHECK_EQ(localisation::formatCompactNumber(1.2e9, japanese), "12" + hanHundredMillion);

    CHECK_EQ(localisation::formatCompactNumber(1500, chinese), std::string("1,500"));
    CHECK_EQ(localisation::formatCompactNumber(1200000, chinese), "120" + hanTenThousand);
    CHECK_EQ(localisation::formatCompactNumber(1.2e9, chinese), "12" + hanHundredMillionSimplified);

    // Hindi keeps the Indian grouping scale (thousand/lakh/crore).
    CHECK_EQ(localisation::formatCompactNumber(1000, hindi), "1" + nbsp + hindiThousandWord);
    CHECK_EQ(localisation::formatCompactNumber(1500, hindi), "1.5" + nbsp + hindiThousandWord);
    CHECK_EQ(localisation::formatCompactNumber(1200000, hindi), "12" + nbsp + hindiLakhWord);
    CHECK_EQ(localisation::formatCompactNumber(1.2e9, hindi), "1.2" + nbsp + hindiCroreWord);
}

TEST_CASE("number_format: compact notation uses plural categories")
{
    // Arabic distinguishes the "one" word for a thousand from the "few" word.
    CHECK_EQ(localisation::formatCompactNumber(1000, arabic), arabicNumber("1") + nbsp + arabicThousandWord);
    CHECK_EQ(localisation::formatCompactNumber(3000, arabic), arabicNumber("3") + nbsp + arabicThousandsWord);
    CHECK_EQ(localisation::formatCompactNumber(1200000, arabic), arabicNumber("1.2") + nbsp + arabicMillionWord);
    CHECK_EQ(localisation::formatCompactNumber(1.2e9, arabic), arabicNumber("1.2") + nbsp + arabicBillionWord);

    CHECK_EQ(localisation::formatCompactNumber(1200000, russian), "1,2" + nbsp + russianMillionWord);
}

TEST_CASE("number_format: compact long form")
{
    // The long forms spell the magnitude out.  Unlike the short forms, CLDR
    // separates them with an ordinary space rather than a no-break space.
    CHECK_EQ(localisation::formatCompactNumber(1200000, english, false), std::string("1.2 million"));
    CHECK_EQ(localisation::formatCompactNumber(1200000, german, false), std::string("1,2 Millionen"));
    CHECK_EQ(localisation::formatCompactNumber(1200000, french, false), std::string("1,2 million"));
    CHECK_EQ(localisation::formatCompactNumber(1500, english, false), std::string("1.5 thousand"));
}

TEST_CASE("number_format: compact notation falls back to plain numbers")
{
    // Below 1000, above the tabulated magnitudes, and NaN/infinity are all
    // delegated to formatNumber.
    CHECK_EQ(localisation::formatCompactNumber(999, english), std::string("999"));
    CHECK_EQ(localisation::formatCompactNumber(0.25, english), std::string("0.25"));
    CHECK_EQ(localisation::formatCompactNumber(1e15, english), std::string("1,000,000,000,000,000"));
    CHECK_EQ(localisation::formatCompactNumber(std::nan(""), english), std::string("NaN"));
    CHECK_EQ(localisation::formatCompactNumber(1.0 / 0.0, english), std::string("inf"));
    CHECK_EQ(localisation::formatCompactNumber(-1.0 / 0.0, english), std::string("-inf"));
}

TEST_CASE("number_format: scientific notation")
{
    CHECK_EQ(localisation::formatScientificNumber(0, english), std::string("0E+00"));
    CHECK_EQ(localisation::formatScientificNumber(1234.5, english), std::string("1.234E+03"));
    CHECK_EQ(localisation::formatScientificNumber(-0.00042, english), std::string("-4.2E-04"));
    CHECK_EQ(localisation::formatScientificNumber(1, english), std::string("1E+00"));
    CHECK_EQ(localisation::formatScientificNumber(-1234.5, english), std::string("-1.234E+03"));
    CHECK_EQ(localisation::formatScientificNumber(1e-9, english), std::string("1E-09"));

    // Rounding the mantissa can carry into the next decade.
    CHECK_EQ(localisation::formatScientificNumber(999999, english), std::string("1E+06"));

    // Fraction-digit limits are honoured on both ends.
    CHECK_EQ(localisation::formatScientificNumber(1234.5, english, 2, 5), std::string("1.2345E+03"));
    CHECK_EQ(localisation::formatScientificNumber(0, english, 2, 2), std::string("0.00E+00"));

    CHECK_EQ(localisation::formatScientificNumber(std::nan(""), english), std::string("NaN"));
    CHECK_EQ(localisation::formatScientificNumber(-1.0 / 0.0, english), std::string("-inf"));
}

TEST_CASE("number_format: scientific notation uses locale digits")
{
    CHECK_EQ(localisation::formatScientificNumber(1234.5, german), std::string("1,234E+03"));
    CHECK_EQ(localisation::formatScientificNumber(-0.00042, german), std::string("-4,2E-04"));
    CHECK_EQ(localisation::formatScientificNumber(1234.5, french), std::string("1,234E+03"));
    CHECK_EQ(localisation::formatScientificNumber(1234.5, arabic), arabicNumber("1.234") + "E+03");
    CHECK_EQ(localisation::formatScientificNumber(-0.00042, arabic), "-" + arabicNumber("4.2") + "E-04");
    CHECK_EQ(localisation::formatScientificNumber(0, arabic), arabicNumber("0") + "E+00");
}

TEST_CASE("number_format: significant digits")
{
    CHECK_EQ(localisation::formatSignificantNumber(1234.5, english, 1, 3), std::string("1,230"));
    CHECK_EQ(localisation::formatSignificantNumber(1234.5, german, 1, 3), std::string("1.230"));
    CHECK_EQ(localisation::formatSignificantNumber(1234.5, arabic, 1, 3), arabicNumber("1,230"));
    CHECK_EQ(localisation::formatSignificantNumber(-1234.5, english, 1, 3), std::string("-1,230"));

    CHECK_EQ(localisation::formatSignificantNumber(123456, english, 1, 3), std::string("123,000"));
    CHECK_EQ(localisation::formatSignificantNumber(1.2345, english, 1, 3), std::string("1.23"));
    CHECK_EQ(localisation::formatSignificantNumber(9.999, english, 1, 3), std::string("10"));
    CHECK_EQ(localisation::formatSignificantNumber(0.00012345, english, 1, 3), std::string("0.000123"));
    CHECK_EQ(localisation::formatSignificantNumber(0, english, 1, 3), std::string("0"));

    // A larger maximum keeps more digits, but still rounds.
    CHECK_EQ(localisation::formatSignificantNumber(1234.5, english, 1, 6), std::string("1,234.5"));
}

TEST_CASE("number_format: minimum significant digits")
{
    // Trailing zeros appear until the minimum is met, then they are dropped.
    CHECK_EQ(localisation::formatSignificantNumber(1.2, english, 3, 3), std::string("1.20"));
    CHECK_EQ(localisation::formatSignificantNumber(1.2, german, 3, 3), std::string("1,20"));
    CHECK_EQ(localisation::formatSignificantNumber(1234.5, english, 3, 3), std::string("1,230"));
    CHECK_EQ(localisation::formatSignificantNumber(0, english, 3, 3), std::string("0.00"));
    CHECK_EQ(localisation::formatSignificantNumber(1.2345, english, 2, 4), std::string("1.234"));

    CHECK_EQ(localisation::formatSignificantNumber(std::nan(""), english), std::string("NaN"));
    CHECK_EQ(localisation::formatSignificantNumber(-1.0 / 0.0, english), std::string("-inf"));
}
