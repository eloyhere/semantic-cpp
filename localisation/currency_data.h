#pragma once

// ===========================================================================
// localisation/currency_data.h
//
// GENERATED FILE -- do not edit by hand.  Regenerate with:
//
//     python tools/gen_currency_data.py
//
// Source: Unicode CLDR 49 (cldr-json, https://github.com/unicode-org/cldr-json).
//
// The table holds, for the 15 locales covered by the generated
// data set:
//
//   * currencyData      base currency metadata from CLDR ``en``: the symbol
//                       (nullptr when it equals the code) and the number of
//                       fraction digits used for amounts of that currency.
//   * currencySymbols   per-locale symbol overrides, present only where the
//                       locale's symbol differs from the base table.
//   * currencyPatterns  the CLDR ``standard`` and ``accounting`` currency
//                       patterns as ``positive;negative`` pairs, plus the
//                       currency-spacing string inserted between an
//                       alphabetic currency code and a number.
//   * compactPatterns   the CLDR short and long compact decimal patterns for
//                       the magnitudes 1e3 .. 1e14.  Row ``i`` of each table
//                       is the pattern for 10^(3 + i); the six columns are the
//                       plural categories zero/one/two/few/many/other, and a
//                       nullptr column means the category is not defined for
//                       that magnitude.
//
// The file is intentionally pure ASCII: non-ASCII code points are written as
// C++ universal character names so the table does not depend on the compiler
// source or execution character set.
// ===========================================================================

namespace localisation
{
namespace detail
{

// One row of the base currency table.  ``symbol`` is nullptr when the symbol
// is the ISO code itself; ``digits`` is the CLDR fraction-digit count.
struct CurrencyDataEntry
{
    const char *code;
    const char *symbol;
    int digits;
};

// A locale-specific symbol for one currency.
struct CurrencyLocaleSymbol
{
    const char *tag;
    const char *code;
    const char *symbol;
};

// A locale's currency patterns.  ``standard`` and ``accounting`` are CLDR
// ``positive;negative`` pattern pairs (the negative half is empty when CLDR
// defines no separate negative subpattern).  ``spacing`` is inserted between
// an alphabetic currency code and the digits.
struct CurrencyPatternEntry
{
    const char *tag;
    const char *standard;
    const char *accounting;
    const char *spacing;
};

// Number of magnitudes covered by CompactPatternEntry (1e3 .. 1e14).
inline constexpr int compactMagnitudeCount = 12;

// Compact decimal patterns for one locale.  ``shortForms[i]`` and
// ``longForms[i]`` describe 10^(3 + i).
struct CompactPatternEntry
{
    const char *tag;
    const char *shortForms[compactMagnitudeCount][6];
    const char *longForms[compactMagnitudeCount][6];
};

inline const CurrencyDataEntry currencyData[] = {
    {"ADP", nullptr, 0},
    {"AED", nullptr, 2},
    {"AFA", nullptr, 2},
    {"AFN", nullptr, 0},
    {"ALK", nullptr, 2},
    {"ALL", nullptr, 0},
    {"AMD", nullptr, 2},
    {"ANG", nullptr, 2},
    {"AOA", nullptr, 2},
    {"AOK", nullptr, 2},
    {"AON", nullptr, 2},
    {"AOR", nullptr, 2},
    {"ARA", nullptr, 2},
    {"ARL", nullptr, 2},
    {"ARM", nullptr, 2},
    {"ARP", nullptr, 2},
    {"ARS", nullptr, 2},
    {"ATS", nullptr, 2},
    {"AUD", "A$", 2},
    {"AWG", nullptr, 2},
    {"AZM", nullptr, 2},
    {"AZN", nullptr, 2},
    {"BAD", nullptr, 2},
    {"BAM", nullptr, 2},
    {"BAN", nullptr, 2},
    {"BBD", nullptr, 2},
    {"BDT", nullptr, 2},
    {"BEC", nullptr, 2},
    {"BEF", nullptr, 2},
    {"BEL", nullptr, 2},
    {"BGL", nullptr, 2},
    {"BGM", nullptr, 2},
    {"BGN", nullptr, 2},
    {"BGO", nullptr, 2},
    {"BHD", nullptr, 3},
    {"BIF", nullptr, 0},
    {"BMD", nullptr, 2},
    {"BND", nullptr, 2},
    {"BOB", nullptr, 2},
    {"BOL", nullptr, 2},
    {"BOP", nullptr, 2},
    {"BOV", nullptr, 2},
    {"BRB", nullptr, 2},
    {"BRC", nullptr, 2},
    {"BRE", nullptr, 2},
    {"BRL", "R$", 2},
    {"BRN", nullptr, 2},
    {"BRR", nullptr, 2},
    {"BRZ", nullptr, 2},
    {"BSD", nullptr, 2},
    {"BTN", nullptr, 2},
    {"BUK", nullptr, 2},
    {"BWP", nullptr, 2},
    {"BYB", nullptr, 2},
    {"BYN", nullptr, 2},
    {"BYR", nullptr, 0},
    {"BZD", nullptr, 2},
    {"CAD", "CA$", 2},
    {"CDF", nullptr, 2},
    {"CHE", nullptr, 2},
    {"CHF", nullptr, 2},
    {"CHW", nullptr, 2},
    {"CLE", nullptr, 2},
    {"CLF", nullptr, 4},
    {"CLP", nullptr, 0},
    {"CNH", nullptr, 2},
    {"CNY", "CN\u00a5", 2},
    {"COP", nullptr, 0},
    {"COU", nullptr, 2},
    {"CRC", nullptr, 2},
    {"CSD", nullptr, 2},
    {"CSK", nullptr, 2},
    {"CUC", nullptr, 2},
    {"CUP", nullptr, 2},
    {"CVE", nullptr, 2},
    {"CYP", nullptr, 2},
    {"CZK", nullptr, 2},
    {"DDM", nullptr, 2},
    {"DEM", nullptr, 2},
    {"DJF", nullptr, 0},
    {"DKK", nullptr, 2},
    {"DOP", nullptr, 2},
    {"DZD", nullptr, 2},
    {"ECS", nullptr, 2},
    {"ECV", nullptr, 2},
    {"EEK", nullptr, 2},
    {"EGP", nullptr, 2},
    {"ERN", nullptr, 2},
    {"ESA", nullptr, 2},
    {"ESB", nullptr, 2},
    {"ESP", nullptr, 0},
    {"ETB", nullptr, 2},
    {"EUR", "\u20ac", 2},
    {"FIM", nullptr, 2},
    {"FJD", nullptr, 2},
    {"FKP", nullptr, 2},
    {"FRF", nullptr, 2},
    {"GBP", "\u00a3", 2},
    {"GEK", nullptr, 2},
    {"GEL", nullptr, 2},
    {"GHC", nullptr, 2},
    {"GHS", nullptr, 2},
    {"GIP", nullptr, 2},
    {"GMD", nullptr, 2},
    {"GNF", nullptr, 0},
    {"GNS", nullptr, 2},
    {"GQE", nullptr, 2},
    {"GRD", nullptr, 2},
    {"GTQ", nullptr, 2},
    {"GWE", nullptr, 2},
    {"GWP", nullptr, 2},
    {"GYD", nullptr, 2},
    {"HKD", "HK$", 2},
    {"HNL", nullptr, 2},
    {"HRD", nullptr, 2},
    {"HRK", nullptr, 2},
    {"HTG", nullptr, 2},
    {"HUF", nullptr, 0},
    {"IDR", nullptr, 0},
    {"IEP", nullptr, 2},
    {"ILP", nullptr, 2},
    {"ILR", nullptr, 2},
    {"ILS", "\u20aa", 2},
    {"INR", "\u20b9", 2},
    {"IQD", nullptr, 0},
    {"IRR", nullptr, 0},
    {"ISJ", nullptr, 2},
    {"ISK", nullptr, 0},
    {"ITL", nullptr, 0},
    {"JMD", nullptr, 2},
    {"JOD", nullptr, 3},
    {"JPY", "\u00a5", 0},
    {"KES", nullptr, 2},
    {"KGS", nullptr, 2},
    {"KHR", nullptr, 2},
    {"KMF", nullptr, 0},
    {"KPW", nullptr, 0},
    {"KRH", nullptr, 2},
    {"KRO", nullptr, 2},
    {"KRW", "\u20a9", 0},
    {"KWD", nullptr, 3},
    {"KYD", nullptr, 2},
    {"KZT", nullptr, 2},
    {"LAK", nullptr, 0},
    {"LBP", nullptr, 0},
    {"LKR", nullptr, 2},
    {"LRD", nullptr, 2},
    {"LSL", nullptr, 2},
    {"LTL", nullptr, 2},
    {"LTT", nullptr, 2},
    {"LUC", nullptr, 2},
    {"LUF", nullptr, 0},
    {"LUL", nullptr, 2},
    {"LVL", nullptr, 2},
    {"LVR", nullptr, 2},
    {"LYD", nullptr, 3},
    {"MAD", nullptr, 2},
    {"MAF", nullptr, 2},
    {"MCF", nullptr, 2},
    {"MDC", nullptr, 2},
    {"MDL", nullptr, 2},
    {"MGA", nullptr, 0},
    {"MGF", nullptr, 0},
    {"MKD", nullptr, 2},
    {"MKN", nullptr, 2},
    {"MLF", nullptr, 2},
    {"MMK", nullptr, 0},
    {"MNT", nullptr, 2},
    {"MOP", nullptr, 2},
    {"MRO", nullptr, 0},
    {"MRU", nullptr, 2},
    {"MTL", nullptr, 2},
    {"MTP", nullptr, 2},
    {"MUR", nullptr, 2},
    {"MVP", nullptr, 2},
    {"MVR", nullptr, 2},
    {"MWK", nullptr, 2},
    {"MXN", "MX$", 2},
    {"MXP", nullptr, 2},
    {"MXV", nullptr, 2},
    {"MYR", nullptr, 2},
    {"MZE", nullptr, 2},
    {"MZM", nullptr, 2},
    {"MZN", nullptr, 2},
    {"NAD", nullptr, 2},
    {"NGN", nullptr, 2},
    {"NIC", nullptr, 2},
    {"NIO", nullptr, 2},
    {"NLG", nullptr, 2},
    {"NOK", nullptr, 2},
    {"NPR", nullptr, 2},
    {"NZD", "NZ$", 2},
    {"OMR", nullptr, 3},
    {"PAB", nullptr, 2},
    {"PEI", nullptr, 2},
    {"PEN", nullptr, 2},
    {"PES", nullptr, 2},
    {"PGK", nullptr, 2},
    {"PHP", "\u20b1", 2},
    {"PKR", nullptr, 2},
    {"PLN", nullptr, 2},
    {"PLZ", nullptr, 2},
    {"PTE", nullptr, 2},
    {"PYG", nullptr, 0},
    {"QAR", nullptr, 2},
    {"RHD", nullptr, 2},
    {"ROL", nullptr, 2},
    {"RON", nullptr, 2},
    {"RSD", nullptr, 2},
    {"RUB", nullptr, 2},
    {"RUR", nullptr, 2},
    {"RWF", nullptr, 0},
    {"SAR", nullptr, 2},
    {"SBD", nullptr, 2},
    {"SCR", nullptr, 2},
    {"SDD", nullptr, 2},
    {"SDG", nullptr, 2},
    {"SDP", nullptr, 2},
    {"SEK", nullptr, 2},
    {"SGD", nullptr, 2},
    {"SHP", nullptr, 2},
    {"SIT", nullptr, 2},
    {"SKK", nullptr, 2},
    {"SLE", nullptr, 2},
    {"SLL", nullptr, 0},
    {"SOS", nullptr, 0},
    {"SRD", nullptr, 2},
    {"SRG", nullptr, 2},
    {"SSP", nullptr, 2},
    {"STD", nullptr, 0},
    {"STN", nullptr, 2},
    {"SUR", nullptr, 2},
    {"SVC", nullptr, 2},
    {"SYP", nullptr, 0},
    {"SZL", nullptr, 2},
    {"THB", nullptr, 2},
    {"TJR", nullptr, 2},
    {"TJS", nullptr, 2},
    {"TMM", nullptr, 0},
    {"TMT", nullptr, 2},
    {"TND", nullptr, 3},
    {"TOP", nullptr, 2},
    {"TPE", nullptr, 2},
    {"TRL", nullptr, 0},
    {"TRY", nullptr, 2},
    {"TTD", nullptr, 2},
    {"TWD", "NT$", 2},
    {"TZS", nullptr, 2},
    {"UAH", nullptr, 2},
    {"UAK", nullptr, 2},
    {"UGS", nullptr, 2},
    {"UGX", nullptr, 0},
    {"USD", "$", 2},
    {"USN", nullptr, 2},
    {"USS", nullptr, 2},
    {"UYI", nullptr, 0},
    {"UYP", nullptr, 2},
    {"UYU", nullptr, 2},
    {"UYW", nullptr, 4},
    {"UZS", nullptr, 2},
    {"VEB", nullptr, 2},
    {"VED", nullptr, 2},
    {"VEF", nullptr, 2},
    {"VES", nullptr, 2},
    {"VND", "\u20ab", 0},
    {"VNN", nullptr, 2},
    {"VUV", nullptr, 0},
    {"WST", nullptr, 2},
    {"XAF", "FCFA", 0},
    {"XAG", nullptr, 2},
    {"XAU", nullptr, 2},
    {"XBA", nullptr, 2},
    {"XBB", nullptr, 2},
    {"XBC", nullptr, 2},
    {"XBD", nullptr, 2},
    {"XCD", "EC$", 2},
    {"XCG", "Cg.", 2},
    {"XDR", nullptr, 2},
    {"XEU", nullptr, 2},
    {"XFO", nullptr, 2},
    {"XFU", nullptr, 2},
    {"XOF", "F\u202fCFA", 0},
    {"XPD", nullptr, 2},
    {"XPF", "CFPF", 0},
    {"XPT", nullptr, 2},
    {"XRE", nullptr, 2},
    {"XSU", nullptr, 2},
    {"XTS", nullptr, 2},
    {"XUA", nullptr, 2},
    {"XXX", "\u00a4", 2},
    {"YDD", nullptr, 2},
    {"YER", nullptr, 0},
    {"YUD", nullptr, 2},
    {"YUM", nullptr, 2},
    {"YUN", nullptr, 2},
    {"YUR", nullptr, 2},
    {"ZAL", nullptr, 2},
    {"ZAR", nullptr, 2},
    {"ZMK", nullptr, 0},
    {"ZMW", nullptr, 2},
    {"ZRN", nullptr, 2},
    {"ZRZ", nullptr, 2},
    {"ZWD", nullptr, 0},
    {"ZWG", nullptr, 2},
    {"ZWL", nullptr, 2},
    {"ZWR", nullptr, 2},
};

inline const CurrencyLocaleSymbol currencySymbols[] = {
    {"de", "ATS", "\u00f6S"},
    {"de", "AUD", "AU$"},
    {"de", "BGM", "BGK"},
    {"de", "BGO", "BGJ"},
    {"de", "DEM", "DM"},
    {"de", "PHP", "PHP"},
    {"de", "THB", "\u0e3f"},
    {"de", "XXX", "XXX"},
    {"fr", "ARS", "$AR"},
    {"fr", "AUD", "$AU"},
    {"fr", "BEF", "FB"},
    {"fr", "BMD", "$BM"},
    {"fr", "BND", "$BN"},
    {"fr", "BZD", "$BZ"},
    {"fr", "CAD", "$CA"},
    {"fr", "CLP", "$CL"},
    {"fr", "CNY", "CNY"},
    {"fr", "COP", "$CO"},
    {"fr", "CYP", "\u00a3CY"},
    {"fr", "FJD", "$FJ"},
    {"fr", "FKP", "\u00a3FK"},
    {"fr", "FRF", "F"},
    {"fr", "GBP", "\u00a3GB"},
    {"fr", "GIP", "\u00a3GI"},
    {"fr", "HKD", "HKD"},
    {"fr", "IEP", "\u00a3IE"},
    {"fr", "ILP", "\u00a3IL"},
    {"fr", "ITL", "\u20a4IT"},
    {"fr", "JPY", "JPY"},
    {"fr", "LBP", "\u00a3LB"},
    {"fr", "MTP", "\u00a3MT"},
    {"fr", "MXN", "$MX"},
    {"fr", "NAD", "$NA"},
    {"fr", "NZD", "$NZ"},
    {"fr", "PHP", "PHP"},
    {"fr", "RHD", "$RH"},
    {"fr", "SBD", "$SB"},
    {"fr", "SGD", "$SG"},
    {"fr", "SRD", "$SR"},
    {"fr", "TTD", "$TT"},
    {"fr", "TWD", "TWD"},
    {"fr", "USD", "$US"},
    {"fr", "UYU", "$UY"},
    {"fr", "WST", "$WS"},
    {"fr", "XCD", "XCD"},
    {"fr", "XPF", "FCFP"},
    {"es", "AUD", "AUD"},
    {"es", "BRL", "BRL"},
    {"es", "CAD", "CAD"},
    {"es", "CNY", "CNY"},
    {"es", "ESP", "\u20a7"},
    {"es", "GBP", "GBP"},
    {"es", "HKD", "HKD"},
    {"es", "ILS", "ILS"},
    {"es", "INR", "INR"},
    {"es", "JPY", "JPY"},
    {"es", "KRW", "KRW"},
    {"es", "MXN", "MXN"},
    {"es", "NZD", "NZD"},
    {"es", "PHP", "PHP"},
    {"es", "THB", "\u0e3f"},
    {"es", "TWD", "TWD"},
    {"es", "USD", "US$"},
    {"es", "XAF", "XAF"},
    {"es", "XCD", "XCD"},
    {"es", "XOF", "XOF"},
    {"it", "BRL", "BRL"},
    {"it", "HKD", "HKD"},
    {"it", "INR", "INR"},
    {"it", "JPY", "JPY"},
    {"it", "KRW", "KRW"},
    {"it", "MXN", "MXN"},
    {"it", "THB", "\u0e3f"},
    {"it", "TWD", "TWD"},
    {"it", "USD", "USD"},
    {"it", "VND", "VND"},
    {"it", "XCG", "Cf"},
    {"pt", "AUD", "AU$"},
    {"pt", "JPY", "JP\u00a5"},
    {"pt", "PHP", "PHP"},
    {"pt", "PTE", "Esc."},
    {"pt", "THB", "\u0e3f"},
    {"pt", "USD", "US$"},
    {"ru", "PHP", "PHP"},
    {"ru", "RUB", "\u20bd"},
    {"ru", "RUR", "\u0440."},
    {"ru", "THB", "\u0e3f"},
    {"ru", "TMT", "\u0422\u041c\u0422"},
    {"ru", "UAH", "\u20b4"},
    {"ru", "XCG", "Cg"},
    {"ru", "XXX", "XXXX"},
    {"tr", "AUD", "AU$"},
    {"tr", "PHP", "PHP"},
    {"tr", "THB", "\u0e3f"},
    {"tr", "TRY", "\u20ba"},
    {"ar", "AED", "\u062f.\u0625.\u200f"},
    {"ar", "AUD", "AU$"},
    {"ar", "BHD", "\u062f.\u0628.\u200f"},
    {"ar", "DZD", "\u062f.\u062c.\u200f"},
    {"ar", "EGP", "\u062c.\u0645.\u200f"},
    {"ar", "GBP", "UK\u00a3"},
    {"ar", "IQD", "\u062f.\u0639.\u200f"},
    {"ar", "IRR", "\u0631.\u0625."},
    {"ar", "JOD", "\u062f.\u0623.\u200f"},
    {"ar", "JPY", "JP\u00a5"},
    {"ar", "KWD", "\u062f.\u0643.\u200f"},
    {"ar", "LBP", "\u0644.\u0644.\u200f"},
    {"ar", "LYD", "\u062f.\u0644.\u200f"},
    {"ar", "MAD", "\u062f.\u0645.\u200f"},
    {"ar", "MRU", "\u0623.\u0645."},
    {"ar", "OMR", "\u0631.\u0639.\u200f"},
    {"ar", "PHP", "PHP"},
    {"ar", "QAR", "\u0631.\u0642.\u200f"},
    {"ar", "SAR", "\u0631.\u0633.\u200f"},
    {"ar", "SDD", "\u062f.\u0633.\u200f"},
    {"ar", "SDG", "\u062c.\u0633."},
    {"ar", "SYP", "\u0644.\u0633.\u200f"},
    {"ar", "THB", "\u0e3f"},
    {"ar", "TND", "\u062f.\u062a.\u200f"},
    {"ar", "USD", "US$"},
    {"ar", "YER", "\u0631.\u064a.\u200f"},
    {"he", "CNY", "\u200eCN\u00a5\u200e"},
    {"he", "ILP", "\u05dc\u05f4\u05d9"},
    {"he", "PHP", "PHP"},
    {"he", "THB", "\u0e3f"},
    {"hi", "JPY", "JP\u00a5"},
    {"hi", "PHP", "PHP"},
    {"hi", "THB", "\u0e3f"},
    {"ja", "CNY", "\u5143"},
    {"ja", "JPY", "\uffe5"},
    {"ja", "PHP", "PHP"},
    {"ja", "XCG", "Cg"},
    {"ja", "XXX", "XXX"},
    {"ko", "AUD", "AU$"},
    {"ko", "JPY", "JP\u00a5"},
    {"ko", "PHP", "PHP"},
    {"ko", "USD", "US$"},
    {"zh", "AUD", "AU$"},
    {"zh", "CNY", "\u00a5"},
    {"zh", "ILR", "ILS"},
    {"zh", "JPY", "JP\u00a5"},
    {"zh", "PHP", "PHP"},
    {"zh", "USD", "US$"},
    {"zh", "XXX", "XXX"},
    {"zh-Hant", "AUD", "AU$"},
    {"zh-Hant", "KRW", "\uffe6"},
    {"zh-Hant", "PHP", "PHP"},
    {"zh-Hant", "TWD", "$"},
    {"zh-Hant", "USD", "US$"},
    {"zh-Hant", "XXX", "XXX"},
};

inline const CurrencyPatternEntry currencyPatterns[] = {
    {"en", "\u00a4#,##0.00", "\u00a4#,##0.00;(\u00a4#,##0.00)", "\u00a0"},
    {"de", "#,##0.00\u00a0\u00a4", "#,##0.00\u00a0\u00a4", "\u00a0"},
    {"fr", "#,##0.00\u00a0\u00a4", "#,##0.00\u00a0\u00a4;(#,##0.00\u00a0\u00a4)", "\u00a0"},
    {"es", "#,##0.00\u00a0\u00a4", "#,##0.00\u00a0\u00a4", "\u00a0"},
    {"it", "#,##0.00\u00a0\u00a4", "#,##0.00\u00a0\u00a4", "\u00a0"},
    {"pt", "\u00a4\u00a0#,##0.00", "\u00a4\u00a0#,##0.00", "\u00a0"},
    {"ru", "#,##0.00\u00a0\u00a4", "#,##0.00\u00a0\u00a4", "\u00a0"},
    {"tr", "\u00a4#,##0.00", "\u00a4#,##0.00;(\u00a4#,##0.00)", "\u00a0"},
    {"ar", "\u200f#,##0.00\u00a0\u00a4;\u200f-#,##0.00\u00a0\u00a4", "\u061c#,##0.00\u00a4;(\u061c#,##0.00\u00a4)", "\u00a0"},
    {"he", "\u200f#,##0.00\u00a0\u200f\u00a4;\u200f-#,##0.00\u00a0\u200f\u00a4", "\u200f#,##0.00\u00a0\u200f\u00a4;\u200f-#,##0.00\u00a0\u200f\u00a4", "\u00a0"},
    {"hi", "\u00a4#,##,##0.00", "\u00a4#,##,##0.00", "\u00a0"},
    {"ja", "\u00a4#,##0.00", "\u00a4#,##0.00;(\u00a4#,##0.00)", "\u00a0"},
    {"ko", "\u00a4#,##0.00", "\u00a4#,##0.00;(\u00a4#,##0.00)", "\u00a0"},
    {"zh", "\u00a4#,##0.00", "\u00a4#,##0.00;(\u00a4#,##0.00)", "\u00a0"},
    {"zh-Hant", "\u00a4#,##0.00", "\u00a4#,##0.00;(\u00a4#,##0.00)", "\u00a0"},
};

inline const CompactPatternEntry compactPatterns[] = {
    {"en",
      {
        {nullptr, "0K", nullptr, nullptr, nullptr, "0K"}, // 1e3
        {nullptr, "00K", nullptr, nullptr, nullptr, "00K"}, // 1e4
        {nullptr, "000K", nullptr, nullptr, nullptr, "000K"}, // 1e5
        {nullptr, "0M", nullptr, nullptr, nullptr, "0M"}, // 1e6
        {nullptr, "00M", nullptr, nullptr, nullptr, "00M"}, // 1e7
        {nullptr, "000M", nullptr, nullptr, nullptr, "000M"}, // 1e8
        {nullptr, "0B", nullptr, nullptr, nullptr, "0B"}, // 1e9
        {nullptr, "00B", nullptr, nullptr, nullptr, "00B"}, // 1e10
        {nullptr, "000B", nullptr, nullptr, nullptr, "000B"}, // 1e11
        {nullptr, "0T", nullptr, nullptr, nullptr, "0T"}, // 1e12
        {nullptr, "00T", nullptr, nullptr, nullptr, "00T"}, // 1e13
        {nullptr, "000T", nullptr, nullptr, nullptr, "000T"}, // 1e14
      },
      {
        {nullptr, "0 thousand", nullptr, nullptr, nullptr, "0 thousand"}, // 1e3
        {nullptr, "00 thousand", nullptr, nullptr, nullptr, "00 thousand"}, // 1e4
        {nullptr, "000 thousand", nullptr, nullptr, nullptr, "000 thousand"}, // 1e5
        {nullptr, "0 million", nullptr, nullptr, nullptr, "0 million"}, // 1e6
        {nullptr, "00 million", nullptr, nullptr, nullptr, "00 million"}, // 1e7
        {nullptr, "000 million", nullptr, nullptr, nullptr, "000 million"}, // 1e8
        {nullptr, "0 billion", nullptr, nullptr, nullptr, "0 billion"}, // 1e9
        {nullptr, "00 billion", nullptr, nullptr, nullptr, "00 billion"}, // 1e10
        {nullptr, "000 billion", nullptr, nullptr, nullptr, "000 billion"}, // 1e11
        {nullptr, "0 trillion", nullptr, nullptr, nullptr, "0 trillion"}, // 1e12
        {nullptr, "00 trillion", nullptr, nullptr, nullptr, "00 trillion"}, // 1e13
        {nullptr, "000 trillion", nullptr, nullptr, nullptr, "000 trillion"}, // 1e14
      },
    },
    {"de",
      {
        {nullptr, "0", nullptr, nullptr, nullptr, "0"}, // 1e3
        {nullptr, "0", nullptr, nullptr, nullptr, "0"}, // 1e4
        {nullptr, "0", nullptr, nullptr, nullptr, "0"}, // 1e5
        {nullptr, "0\u00a0Mio'.'", nullptr, nullptr, nullptr, "0\u00a0Mio'.'"}, // 1e6
        {nullptr, "00\u00a0Mio'.'", nullptr, nullptr, nullptr, "00\u00a0Mio'.'"}, // 1e7
        {nullptr, "000\u00a0Mio'.'", nullptr, nullptr, nullptr, "000\u00a0Mio'.'"}, // 1e8
        {nullptr, "0\u00a0Mrd'.'", nullptr, nullptr, nullptr, "0\u00a0Mrd'.'"}, // 1e9
        {nullptr, "00\u00a0Mrd'.'", nullptr, nullptr, nullptr, "00\u00a0Mrd'.'"}, // 1e10
        {nullptr, "000\u00a0Mrd'.'", nullptr, nullptr, nullptr, "000\u00a0Mrd'.'"}, // 1e11
        {nullptr, "0\u00a0Bio'.'", nullptr, nullptr, nullptr, "0\u00a0Bio'.'"}, // 1e12
        {nullptr, "00\u00a0Bio'.'", nullptr, nullptr, nullptr, "00\u00a0Bio'.'"}, // 1e13
        {nullptr, "000\u00a0Bio'.'", nullptr, nullptr, nullptr, "000\u00a0Bio'.'"}, // 1e14
      },
      {
        {nullptr, "0 Tausend", nullptr, nullptr, nullptr, "0 Tausend"}, // 1e3
        {nullptr, "00 Tausend", nullptr, nullptr, nullptr, "00 Tausend"}, // 1e4
        {nullptr, "000 Tausend", nullptr, nullptr, nullptr, "000 Tausend"}, // 1e5
        {nullptr, "0 Million", nullptr, nullptr, nullptr, "0 Millionen"}, // 1e6
        {nullptr, "00 Millionen", nullptr, nullptr, nullptr, "00 Millionen"}, // 1e7
        {nullptr, "000 Millionen", nullptr, nullptr, nullptr, "000 Millionen"}, // 1e8
        {nullptr, "0 Milliarde", nullptr, nullptr, nullptr, "0 Milliarden"}, // 1e9
        {nullptr, "00 Milliarden", nullptr, nullptr, nullptr, "00 Milliarden"}, // 1e10
        {nullptr, "000 Milliarden", nullptr, nullptr, nullptr, "000 Milliarden"}, // 1e11
        {nullptr, "0 Billion", nullptr, nullptr, nullptr, "0 Billionen"}, // 1e12
        {nullptr, "00 Billionen", nullptr, nullptr, nullptr, "00 Billionen"}, // 1e13
        {nullptr, "000 Billionen", nullptr, nullptr, nullptr, "000 Billionen"}, // 1e14
      },
    },
    {"fr",
      {
        {nullptr, "0\u00a0k", nullptr, nullptr, nullptr, "0\u00a0k"}, // 1e3
        {nullptr, "00\u00a0k", nullptr, nullptr, nullptr, "00\u00a0k"}, // 1e4
        {nullptr, "000\u00a0k", nullptr, nullptr, nullptr, "000\u00a0k"}, // 1e5
        {nullptr, "0\u00a0M", nullptr, nullptr, nullptr, "0\u00a0M"}, // 1e6
        {nullptr, "00\u00a0M", nullptr, nullptr, nullptr, "00\u00a0M"}, // 1e7
        {nullptr, "000\u00a0M", nullptr, nullptr, nullptr, "000\u00a0M"}, // 1e8
        {nullptr, "0\u00a0Md", nullptr, nullptr, nullptr, "0\u00a0Md"}, // 1e9
        {nullptr, "00\u00a0Md", nullptr, nullptr, nullptr, "00\u00a0Md"}, // 1e10
        {nullptr, "000\u00a0Md", nullptr, nullptr, nullptr, "000\u00a0Md"}, // 1e11
        {nullptr, "0\u00a0Bn", nullptr, nullptr, nullptr, "0\u00a0Bn"}, // 1e12
        {nullptr, "00\u00a0Bn", nullptr, nullptr, nullptr, "00\u00a0Bn"}, // 1e13
        {nullptr, "000\u00a0Bn", nullptr, nullptr, nullptr, "000\u00a0Bn"}, // 1e14
      },
      {
        {nullptr, "0 millier", nullptr, nullptr, nullptr, "0 mille"}, // 1e3
        {nullptr, "00 mille", nullptr, nullptr, nullptr, "00 mille"}, // 1e4
        {nullptr, "000 mille", nullptr, nullptr, nullptr, "000 mille"}, // 1e5
        {nullptr, "0 million", nullptr, nullptr, nullptr, "0 millions"}, // 1e6
        {nullptr, "00 million", nullptr, nullptr, nullptr, "00 millions"}, // 1e7
        {nullptr, "000 million", nullptr, nullptr, nullptr, "000 millions"}, // 1e8
        {nullptr, "0 milliard", nullptr, nullptr, nullptr, "0 milliards"}, // 1e9
        {nullptr, "00 milliard", nullptr, nullptr, nullptr, "00 milliards"}, // 1e10
        {nullptr, "000 milliard", nullptr, nullptr, nullptr, "000 milliards"}, // 1e11
        {nullptr, "0 billion", nullptr, nullptr, nullptr, "0 billions"}, // 1e12
        {nullptr, "00 billion", nullptr, nullptr, nullptr, "00 billions"}, // 1e13
        {nullptr, "000 billion", nullptr, nullptr, nullptr, "000 billions"}, // 1e14
      },
    },
    {"es",
      {
        {nullptr, "0\u00a0mil", nullptr, nullptr, nullptr, "0\u00a0mil"}, // 1e3
        {nullptr, "00\u00a0mil", nullptr, nullptr, nullptr, "00\u00a0mil"}, // 1e4
        {nullptr, "000\u00a0mil", nullptr, nullptr, nullptr, "000\u00a0mil"}, // 1e5
        {nullptr, "0\u00a0M", nullptr, nullptr, nullptr, "0\u00a0M"}, // 1e6
        {nullptr, "00\u00a0M", nullptr, nullptr, nullptr, "00\u00a0M"}, // 1e7
        {nullptr, "000\u00a0M", nullptr, nullptr, nullptr, "000\u00a0M"}, // 1e8
        {nullptr, "0000\u00a0M", nullptr, nullptr, nullptr, "0000\u00a0M"}, // 1e9
        {nullptr, "00\u00a0mil\u00a0M", nullptr, nullptr, nullptr, "00\u00a0mil\u00a0M"}, // 1e10
        {nullptr, "000\u00a0mil\u00a0M", nullptr, nullptr, nullptr, "000\u00a0mil\u00a0M"}, // 1e11
        {nullptr, "0\u00a0B", nullptr, nullptr, nullptr, "0\u00a0B"}, // 1e12
        {nullptr, "00\u00a0B", nullptr, nullptr, nullptr, "00\u00a0B"}, // 1e13
        {nullptr, "000\u00a0B", nullptr, nullptr, nullptr, "000\u00a0B"}, // 1e14
      },
      {
        {nullptr, "0 mil", nullptr, nullptr, nullptr, "0 mil"}, // 1e3
        {nullptr, "00 mil", nullptr, nullptr, nullptr, "00 mil"}, // 1e4
        {nullptr, "000 mil", nullptr, nullptr, nullptr, "000 mil"}, // 1e5
        {nullptr, "0 mill\u00f3n", nullptr, nullptr, nullptr, "0 millones"}, // 1e6
        {nullptr, "00 millones", nullptr, nullptr, nullptr, "00 millones"}, // 1e7
        {nullptr, "000 millones", nullptr, nullptr, nullptr, "000 millones"}, // 1e8
        {nullptr, "0 mil millones", nullptr, nullptr, nullptr, "0 mil millones"}, // 1e9
        {nullptr, "00 mil millones", nullptr, nullptr, nullptr, "00 mil millones"}, // 1e10
        {nullptr, "000 mil millones", nullptr, nullptr, nullptr, "000 mil millones"}, // 1e11
        {nullptr, "0 bill\u00f3n", nullptr, nullptr, nullptr, "0 billones"}, // 1e12
        {nullptr, "00 billones", nullptr, nullptr, nullptr, "00 billones"}, // 1e13
        {nullptr, "000 billones", nullptr, nullptr, nullptr, "000 billones"}, // 1e14
      },
    },
    {"it",
      {
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0K"}, // 1e3
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00K"}, // 1e4
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000K"}, // 1e5
        {nullptr, "0\u00a0Mln", nullptr, nullptr, nullptr, "0\u00a0Mln"}, // 1e6
        {nullptr, "00\u00a0Mln", nullptr, nullptr, nullptr, "00\u00a0Mln"}, // 1e7
        {nullptr, "000\u00a0Mln", nullptr, nullptr, nullptr, "000\u00a0Mln"}, // 1e8
        {nullptr, "0\u00a0Mld", nullptr, nullptr, nullptr, "0\u00a0Mld"}, // 1e9
        {nullptr, "00\u00a0Mld", nullptr, nullptr, nullptr, "00\u00a0Mld"}, // 1e10
        {nullptr, "000\u00a0Mld", nullptr, nullptr, nullptr, "000\u00a0Mld"}, // 1e11
        {nullptr, "0\u00a0Bln", nullptr, nullptr, nullptr, "0\u00a0Bln"}, // 1e12
        {nullptr, "00\u00a0Bln", nullptr, nullptr, nullptr, "00\u00a0Bln"}, // 1e13
        {nullptr, "000\u00a0Bln", nullptr, nullptr, nullptr, "000\u00a0Bln"}, // 1e14
      },
      {
        {nullptr, "mille", nullptr, nullptr, nullptr, "0 mila"}, // 1e3
        {nullptr, "00 mila", nullptr, nullptr, nullptr, "00 mila"}, // 1e4
        {nullptr, "000 mila", nullptr, nullptr, nullptr, "000 mila"}, // 1e5
        {nullptr, "0 milione", nullptr, nullptr, nullptr, "0 milioni"}, // 1e6
        {nullptr, "00 milioni", nullptr, nullptr, nullptr, "00 milioni"}, // 1e7
        {nullptr, "000 milioni", nullptr, nullptr, nullptr, "000 milioni"}, // 1e8
        {nullptr, "0 miliardo", nullptr, nullptr, nullptr, "0 miliardi"}, // 1e9
        {nullptr, "00 miliardi", nullptr, nullptr, nullptr, "00 miliardi"}, // 1e10
        {nullptr, "000 miliardi", nullptr, nullptr, nullptr, "000 miliardi"}, // 1e11
        {nullptr, "0 mille miliardi", nullptr, nullptr, nullptr, "0 mila miliardi"}, // 1e12
        {nullptr, "00 mila miliardi", nullptr, nullptr, nullptr, "00 mila miliardi"}, // 1e13
        {nullptr, "000 mila miliardi", nullptr, nullptr, nullptr, "000 mila miliardi"}, // 1e14
      },
    },
    {"pt",
      {
        {nullptr, "0\u00a0mil", nullptr, nullptr, nullptr, "0\u00a0mil"}, // 1e3
        {nullptr, "00\u00a0mil", nullptr, nullptr, nullptr, "00\u00a0mil"}, // 1e4
        {nullptr, "000\u00a0mil", nullptr, nullptr, nullptr, "000\u00a0mil"}, // 1e5
        {nullptr, "0\u00a0mi", nullptr, nullptr, nullptr, "0\u00a0mi"}, // 1e6
        {nullptr, "00\u00a0mi", nullptr, nullptr, nullptr, "00\u00a0mi"}, // 1e7
        {nullptr, "000\u00a0mi", nullptr, nullptr, nullptr, "000\u00a0mi"}, // 1e8
        {nullptr, "0\u00a0bi", nullptr, nullptr, nullptr, "0\u00a0bi"}, // 1e9
        {nullptr, "00\u00a0bi", nullptr, nullptr, nullptr, "00\u00a0bi"}, // 1e10
        {nullptr, "000\u00a0bi", nullptr, nullptr, nullptr, "000\u00a0bi"}, // 1e11
        {nullptr, "0\u00a0tri", nullptr, nullptr, nullptr, "0\u00a0tri"}, // 1e12
        {nullptr, "00\u00a0tri", nullptr, nullptr, nullptr, "00\u00a0tri"}, // 1e13
        {nullptr, "000\u00a0tri", nullptr, nullptr, nullptr, "000\u00a0tri"}, // 1e14
      },
      {
        {nullptr, "0 mil", nullptr, nullptr, nullptr, "0 mil"}, // 1e3
        {nullptr, "00 mil", nullptr, nullptr, nullptr, "00 mil"}, // 1e4
        {nullptr, "000 mil", nullptr, nullptr, nullptr, "000 mil"}, // 1e5
        {nullptr, "0 milh\u00e3o", nullptr, nullptr, nullptr, "0 milh\u00f5es"}, // 1e6
        {nullptr, "00 milh\u00e3o", nullptr, nullptr, nullptr, "00 milh\u00f5es"}, // 1e7
        {nullptr, "000 milh\u00e3o", nullptr, nullptr, nullptr, "000 milh\u00f5es"}, // 1e8
        {nullptr, "0 bilh\u00e3o", nullptr, nullptr, nullptr, "0 bilh\u00f5es"}, // 1e9
        {nullptr, "00 bilh\u00e3o", nullptr, nullptr, nullptr, "00 bilh\u00f5es"}, // 1e10
        {nullptr, "000 bilh\u00e3o", nullptr, nullptr, nullptr, "000 bilh\u00f5es"}, // 1e11
        {nullptr, "0 trilh\u00e3o", nullptr, nullptr, nullptr, "0 trilh\u00f5es"}, // 1e12
        {nullptr, "00 trilh\u00e3o", nullptr, nullptr, nullptr, "00 trilh\u00f5es"}, // 1e13
        {nullptr, "000 trilh\u00e3o", nullptr, nullptr, nullptr, "000 trilh\u00f5es"}, // 1e14
      },
    },
    {"ru",
      {
        {nullptr, "0\u00a0\u0442\u044b\u0441'.'", nullptr, "0\u00a0\u0442\u044b\u0441'.'", "0\u00a0\u0442\u044b\u0441'.'", "0\u00a0\u0442\u044b\u0441'.'"}, // 1e3
        {nullptr, "00\u00a0\u0442\u044b\u0441'.'", nullptr, "00\u00a0\u0442\u044b\u0441'.'", "00\u00a0\u0442\u044b\u0441'.'", "00\u00a0\u0442\u044b\u0441'.'"}, // 1e4
        {nullptr, "000\u00a0\u0442\u044b\u0441'.'", nullptr, "000\u00a0\u0442\u044b\u0441'.'", "000\u00a0\u0442\u044b\u0441'.'", "000\u00a0\u0442\u044b\u0441'.'"}, // 1e5
        {nullptr, "0\u00a0\u043c\u043b\u043d", nullptr, "0\u00a0\u043c\u043b\u043d", "0\u00a0\u043c\u043b\u043d", "0\u00a0\u043c\u043b\u043d"}, // 1e6
        {nullptr, "00\u00a0\u043c\u043b\u043d", nullptr, "00\u00a0\u043c\u043b\u043d", "00\u00a0\u043c\u043b\u043d", "00\u00a0\u043c\u043b\u043d"}, // 1e7
        {nullptr, "000\u00a0\u043c\u043b\u043d", nullptr, "000\u00a0\u043c\u043b\u043d", "000\u00a0\u043c\u043b\u043d", "000\u00a0\u043c\u043b\u043d"}, // 1e8
        {nullptr, "0\u00a0\u043c\u043b\u0440\u0434", nullptr, "0\u00a0\u043c\u043b\u0440\u0434", "0\u00a0\u043c\u043b\u0440\u0434", "0\u00a0\u043c\u043b\u0440\u0434"}, // 1e9
        {nullptr, "00\u00a0\u043c\u043b\u0440\u0434", nullptr, "00\u00a0\u043c\u043b\u0440\u0434", "00\u00a0\u043c\u043b\u0440\u0434", "00\u00a0\u043c\u043b\u0440\u0434"}, // 1e10
        {nullptr, "000\u00a0\u043c\u043b\u0440\u0434", nullptr, "000\u00a0\u043c\u043b\u0440\u0434", "000\u00a0\u043c\u043b\u0440\u0434", "000\u00a0\u043c\u043b\u0440\u0434"}, // 1e11
        {nullptr, "0\u00a0\u0442\u0440\u043b\u043d", nullptr, "0\u00a0\u0442\u0440\u043b\u043d", "0\u00a0\u0442\u0440\u043b\u043d", "0\u00a0\u0442\u0440\u043b\u043d"}, // 1e12
        {nullptr, "00\u00a0\u0442\u0440\u043b\u043d", nullptr, "00\u00a0\u0442\u0440\u043b\u043d", "00\u00a0\u0442\u0440\u043b\u043d", "00\u00a0\u0442\u0440\u043b\u043d"}, // 1e13
        {nullptr, "000\u00a0\u0442\u0440\u043b\u043d", nullptr, "000\u00a0\u0442\u0440\u043b\u043d", "000\u00a0\u0442\u0440\u043b\u043d", "000\u00a0\u0442\u0440\u043b\u043d"}, // 1e14
      },
      {
        {nullptr, "0 \u0442\u044b\u0441\u044f\u0447\u0430", nullptr, "0 \u0442\u044b\u0441\u044f\u0447\u0438", "0 \u0442\u044b\u0441\u044f\u0447", "0 \u0442\u044b\u0441\u044f\u0447\u0438"}, // 1e3
        {nullptr, "00 \u0442\u044b\u0441\u044f\u0447\u0430", nullptr, "00 \u0442\u044b\u0441\u044f\u0447\u0438", "00 \u0442\u044b\u0441\u044f\u0447", "00 \u0442\u044b\u0441\u044f\u0447\u0438"}, // 1e4
        {nullptr, "000 \u0442\u044b\u0441\u044f\u0447\u0430", nullptr, "000 \u0442\u044b\u0441\u044f\u0447\u0438", "000 \u0442\u044b\u0441\u044f\u0447", "000 \u0442\u044b\u0441\u044f\u0447\u0438"}, // 1e5
        {nullptr, "0 \u043c\u0438\u043b\u043b\u0438\u043e\u043d", nullptr, "0 \u043c\u0438\u043b\u043b\u0438\u043e\u043d\u0430", "0 \u043c\u0438\u043b\u043b\u0438\u043e\u043d\u043e\u0432", "0 \u043c\u0438\u043b\u043b\u0438\u043e\u043d\u0430"}, // 1e6
        {nullptr, "00 \u043c\u0438\u043b\u043b\u0438\u043e\u043d", nullptr, "00 \u043c\u0438\u043b\u043b\u0438\u043e\u043d\u0430", "00 \u043c\u0438\u043b\u043b\u0438\u043e\u043d\u043e\u0432", "00 \u043c\u0438\u043b\u043b\u0438\u043e\u043d\u0430"}, // 1e7
        {nullptr, "000 \u043c\u0438\u043b\u043b\u0438\u043e\u043d", nullptr, "000 \u043c\u0438\u043b\u043b\u0438\u043e\u043d\u0430", "000 \u043c\u0438\u043b\u043b\u0438\u043e\u043d\u043e\u0432", "000 \u043c\u0438\u043b\u043b\u0438\u043e\u043d\u0430"}, // 1e8
        {nullptr, "0 \u043c\u0438\u043b\u043b\u0438\u0430\u0440\u0434", nullptr, "0 \u043c\u0438\u043b\u043b\u0438\u0430\u0440\u0434\u0430", "0 \u043c\u0438\u043b\u043b\u0438\u0430\u0440\u0434\u043e\u0432", "0 \u043c\u0438\u043b\u043b\u0438\u0430\u0440\u0434\u0430"}, // 1e9
        {nullptr, "00 \u043c\u0438\u043b\u043b\u0438\u0430\u0440\u0434", nullptr, "00 \u043c\u0438\u043b\u043b\u0438\u0430\u0440\u0434\u0430", "00 \u043c\u0438\u043b\u043b\u0438\u0430\u0440\u0434\u043e\u0432", "00 \u043c\u0438\u043b\u043b\u0438\u0430\u0440\u0434\u0430"}, // 1e10
        {nullptr, "000 \u043c\u0438\u043b\u043b\u0438\u0430\u0440\u0434", nullptr, "000 \u043c\u0438\u043b\u043b\u0438\u0430\u0440\u0434\u0430", "000 \u043c\u0438\u043b\u043b\u0438\u0430\u0440\u0434\u043e\u0432", "000 \u043c\u0438\u043b\u043b\u0438\u0430\u0440\u0434\u0430"}, // 1e11
        {nullptr, "0 \u0442\u0440\u0438\u043b\u043b\u0438\u043e\u043d", nullptr, "0 \u0442\u0440\u0438\u043b\u043b\u0438\u043e\u043d\u0430", "0 \u0442\u0440\u0438\u043b\u043b\u0438\u043e\u043d\u043e\u0432", "0 \u0442\u0440\u0438\u043b\u043b\u0438\u043e\u043d\u0430"}, // 1e12
        {nullptr, "00 \u0442\u0440\u0438\u043b\u043b\u0438\u043e\u043d", nullptr, "00 \u0442\u0440\u0438\u043b\u043b\u0438\u043e\u043d\u0430", "00 \u0442\u0440\u0438\u043b\u043b\u0438\u043e\u043d\u043e\u0432", "00 \u0442\u0440\u0438\u043b\u043b\u0438\u043e\u043d\u0430"}, // 1e13
        {nullptr, "000 \u0442\u0440\u0438\u043b\u043b\u0438\u043e\u043d", nullptr, "000 \u0442\u0440\u0438\u043b\u043b\u0438\u043e\u043d\u0430", "000 \u0442\u0440\u0438\u043b\u043b\u0438\u043e\u043d\u043e\u0432", "000 \u0442\u0440\u0438\u043b\u043b\u0438\u043e\u043d\u0430"}, // 1e14
      },
    },
    {"tr",
      {
        {nullptr, "0\u00a0B", nullptr, nullptr, nullptr, "0\u00a0B"}, // 1e3
        {nullptr, "00\u00a0B", nullptr, nullptr, nullptr, "00\u00a0B"}, // 1e4
        {nullptr, "000\u00a0B", nullptr, nullptr, nullptr, "000\u00a0B"}, // 1e5
        {nullptr, "0\u00a0Mn", nullptr, nullptr, nullptr, "0\u00a0Mn"}, // 1e6
        {nullptr, "00\u00a0Mn", nullptr, nullptr, nullptr, "00\u00a0Mn"}, // 1e7
        {nullptr, "000\u00a0Mn", nullptr, nullptr, nullptr, "000\u00a0Mn"}, // 1e8
        {nullptr, "0\u00a0Mr", nullptr, nullptr, nullptr, "0\u00a0Mr"}, // 1e9
        {nullptr, "00\u00a0Mr", nullptr, nullptr, nullptr, "00\u00a0Mr"}, // 1e10
        {nullptr, "000\u00a0Mr", nullptr, nullptr, nullptr, "000\u00a0Mr"}, // 1e11
        {nullptr, "0\u00a0Tn", nullptr, nullptr, nullptr, "0\u00a0Tn"}, // 1e12
        {nullptr, "00\u00a0Tn", nullptr, nullptr, nullptr, "00\u00a0Tn"}, // 1e13
        {nullptr, "000\u00a0Tn", nullptr, nullptr, nullptr, "000\u00a0Tn"}, // 1e14
      },
      {
        {nullptr, "0 bin", nullptr, nullptr, nullptr, "0 bin"}, // 1e3
        {nullptr, "00 bin", nullptr, nullptr, nullptr, "00 bin"}, // 1e4
        {nullptr, "000 bin", nullptr, nullptr, nullptr, "000 bin"}, // 1e5
        {nullptr, "0 milyon", nullptr, nullptr, nullptr, "0 milyon"}, // 1e6
        {nullptr, "00 milyon", nullptr, nullptr, nullptr, "00 milyon"}, // 1e7
        {nullptr, "000 milyon", nullptr, nullptr, nullptr, "000 milyon"}, // 1e8
        {nullptr, "0 milyar", nullptr, nullptr, nullptr, "0 milyar"}, // 1e9
        {nullptr, "00 milyar", nullptr, nullptr, nullptr, "00 milyar"}, // 1e10
        {nullptr, "000 milyar", nullptr, nullptr, nullptr, "000 milyar"}, // 1e11
        {nullptr, "0 trilyon", nullptr, nullptr, nullptr, "0 trilyon"}, // 1e12
        {nullptr, "00 trilyon", nullptr, nullptr, nullptr, "00 trilyon"}, // 1e13
        {nullptr, "000 trilyon", nullptr, nullptr, nullptr, "000 trilyon"}, // 1e14
      },
    },
    {"ar",
      {
        {"0\u00a0\u0623\u0644\u0641", "0\u00a0\u0623\u0644\u0641", "0\u00a0\u0623\u0644\u0641", "0\u00a0\u0622\u0644\u0627\u0641", "0\u00a0\u0623\u0644\u0641", "0\u00a0\u0623\u0644\u0641"}, // 1e3
        {"00\u00a0\u0623\u0644\u0641", "00\u00a0\u0623\u0644\u0641", "00\u00a0\u0623\u0644\u0641", "00\u00a0\u0623\u0644\u0641", "00\u00a0\u0623\u0644\u0641", "00\u00a0\u0623\u0644\u0641"}, // 1e4
        {"000\u00a0\u0623\u0644\u0641", "000\u00a0\u0623\u0644\u0641", "000\u00a0\u0623\u0644\u0641", "000\u00a0\u0623\u0644\u0641", "000\u00a0\u0623\u0644\u0641", "000\u00a0\u0623\u0644\u0641"}, // 1e5
        {"0\u00a0\u0645\u0644\u064a\u0648\u0646", "0\u00a0\u0645\u0644\u064a\u0648\u0646", "0\u00a0\u0645\u0644\u064a\u0648\u0646", "0\u00a0\u0645\u0644\u064a\u0648\u0646", "0\u00a0\u0645\u0644\u064a\u0648\u0646", "0\u00a0\u0645\u0644\u064a\u0648\u0646"}, // 1e6
        {"00\u00a0\u0645\u0644\u064a\u0648\u0646", "00\u00a0\u0645\u0644\u064a\u0648\u0646", "00\u00a0\u0645\u0644\u064a\u0648\u0646", "00\u00a0\u0645\u0644\u064a\u0648\u0646", "00\u00a0\u0645\u0644\u064a\u0648\u0646", "00\u00a0\u0645\u0644\u064a\u0648\u0646"}, // 1e7
        {"000\u00a0\u0645\u0644\u064a\u0648\u0646", "000\u00a0\u0645\u0644\u064a\u0648\u0646", "000\u00a0\u0645\u0644\u064a\u0648\u0646", "000\u00a0\u0645\u0644\u064a\u0648\u0646", "000\u00a0\u0645\u0644\u064a\u0648\u0646", "000\u00a0\u0645\u0644\u064a\u0648\u0646"}, // 1e8
        {"0\u00a0\u0645\u0644\u064a\u0627\u0631", "0\u00a0\u0645\u0644\u064a\u0627\u0631", "0\u00a0\u0645\u0644\u064a\u0627\u0631", "0\u00a0\u0645\u0644\u064a\u0627\u0631", "0\u00a0\u0645\u0644\u064a\u0627\u0631", "0\u00a0\u0645\u0644\u064a\u0627\u0631"}, // 1e9
        {"00\u00a0\u0645\u0644\u064a\u0627\u0631", "00\u00a0\u0645\u0644\u064a\u0627\u0631", "00\u00a0\u0645\u0644\u064a\u0627\u0631", "00\u00a0\u0645\u0644\u064a\u0627\u0631", "00\u00a0\u0645\u0644\u064a\u0627\u0631", "00\u00a0\u0645\u0644\u064a\u0627\u0631"}, // 1e10
        {"000\u00a0\u0645\u0644\u064a\u0627\u0631", "000\u00a0\u0645\u0644\u064a\u0627\u0631", "000\u00a0\u0645\u0644\u064a\u0627\u0631", "000\u00a0\u0645\u0644\u064a\u0627\u0631", "000\u00a0\u0645\u0644\u064a\u0627\u0631", "000\u00a0\u0645\u0644\u064a\u0627\u0631"}, // 1e11
        {"0\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "0\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "0\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "0\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "0\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "0\u00a0\u062a\u0631\u0644\u064a\u0648\u0646"}, // 1e12
        {"00\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "00\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "00\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "00\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "00\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "00\u00a0\u062a\u0631\u0644\u064a\u0648\u0646"}, // 1e13
        {"000\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "000\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "000\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "000\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "000\u00a0\u062a\u0631\u0644\u064a\u0648\u0646", "000\u00a0\u062a\u0631\u0644\u064a\u0648\u0646"}, // 1e14
      },
      {
        {"0 \u0623\u0644\u0641", "0 \u0623\u0644\u0641", "0 \u0623\u0644\u0641", "0 \u0622\u0644\u0627\u0641", "0 \u0623\u0644\u0641", "0 \u0623\u0644\u0641"}, // 1e3
        {"00 \u0623\u0644\u0641", "00 \u0623\u0644\u0641", "00 \u0623\u0644\u0641", "00 \u0623\u0644\u0641", "00 \u0623\u0644\u0641", "00 \u0623\u0644\u0641"}, // 1e4
        {"000 \u0623\u0644\u0641", "000 \u0623\u0644\u0641", "000 \u0623\u0644\u0641", "000 \u0623\u0644\u0641", "000 \u0623\u0644\u0641", "000 \u0623\u0644\u0641"}, // 1e5
        {"0 \u0645\u0644\u064a\u0648\u0646", "0 \u0645\u0644\u064a\u0648\u0646", "0 \u0645\u0644\u064a\u0648\u0646", "0 \u0645\u0644\u0627\u064a\u064a\u0646", "0 \u0645\u0644\u064a\u0648\u0646", "0 \u0645\u0644\u064a\u0648\u0646"}, // 1e6
        {"00 \u0645\u0644\u064a\u0648\u0646", "00 \u0645\u0644\u064a\u0648\u0646", "00 \u0645\u0644\u064a\u0648\u0646", "00 \u0645\u0644\u0627\u064a\u064a\u0646", "00 \u0645\u0644\u064a\u0648\u0646", "00 \u0645\u0644\u064a\u0648\u0646"}, // 1e7
        {"000 \u0645\u0644\u064a\u0648\u0646", "000 \u0645\u0644\u064a\u0648\u0646", "000 \u0645\u0644\u064a\u0648\u0646", "000 \u0645\u0644\u064a\u0648\u0646", "000 \u0645\u0644\u064a\u0648\u0646", "000 \u0645\u0644\u064a\u0648\u0646"}, // 1e8
        {"0 \u0645\u0644\u064a\u0627\u0631", "0 \u0645\u0644\u064a\u0627\u0631", "0 \u0645\u0644\u064a\u0627\u0631", "0 \u0645\u0644\u064a\u0627\u0631", "0 \u0645\u0644\u064a\u0627\u0631", "0 \u0645\u0644\u064a\u0627\u0631"}, // 1e9
        {"00 \u0645\u0644\u064a\u0627\u0631", "00 \u0645\u0644\u064a\u0627\u0631", "00 \u0645\u0644\u064a\u0627\u0631", "00 \u0645\u0644\u064a\u0627\u0631", "00 \u0645\u0644\u064a\u0627\u0631", "00 \u0645\u0644\u064a\u0627\u0631"}, // 1e10
        {"000 \u0645\u0644\u064a\u0627\u0631", "000 \u0645\u0644\u064a\u0627\u0631", "000 \u0645\u0644\u064a\u0627\u0631", "000 \u0645\u0644\u064a\u0627\u0631", "000 \u0645\u0644\u064a\u0627\u0631", "000 \u0645\u0644\u064a\u0627\u0631"}, // 1e11
        {"0 \u062a\u0631\u0644\u064a\u0648\u0646", "0 \u062a\u0631\u0644\u064a\u0648\u0646", "0 \u062a\u0631\u0644\u064a\u0648\u0646", "0 \u062a\u0631\u0644\u064a\u0648\u0646", "0 \u062a\u0631\u0644\u064a\u0648\u0646", "0 \u062a\u0631\u0644\u064a\u0648\u0646"}, // 1e12
        {"00 \u062a\u0631\u0644\u064a\u0648\u0646", "00 \u062a\u0631\u0644\u064a\u0648\u0646", "00 \u062a\u0631\u0644\u064a\u0648\u0646", "00 \u062a\u0631\u0644\u064a\u0648\u0646", "00 \u062a\u0631\u0644\u064a\u0648\u0646", "00 \u062a\u0631\u0644\u064a\u0648\u0646"}, // 1e13
        {"000 \u062a\u0631\u0644\u064a\u0648\u0646", "000 \u062a\u0631\u0644\u064a\u0648\u0646", "000 \u062a\u0631\u0644\u064a\u0648\u0646", "000 \u062a\u0631\u0644\u064a\u0648\u0646", "000 \u062a\u0631\u0644\u064a\u0648\u0646", "000 \u062a\u0631\u0644\u064a\u0648\u0646"}, // 1e14
      },
    },
    {"he",
      {
        {nullptr, "0K\u200f", "0K\u200f", nullptr, nullptr, "0K\u200f"}, // 1e3
        {nullptr, "00K\u200f", "00K\u200f", nullptr, nullptr, "00K\u200f"}, // 1e4
        {nullptr, "000K\u200f", "000K\u200f", nullptr, nullptr, "000K\u200f"}, // 1e5
        {nullptr, "0M\u200f", "0M\u200f", nullptr, nullptr, "0M\u200f"}, // 1e6
        {nullptr, "00M\u200f", "00M\u200f", nullptr, nullptr, "00M\u200f"}, // 1e7
        {nullptr, "000M\u200f", "000M\u200f", nullptr, nullptr, "000M\u200f"}, // 1e8
        {nullptr, "0B\u200f", "0B\u200f", nullptr, nullptr, "0B\u200f"}, // 1e9
        {nullptr, "00B\u200f", "00B\u200f", nullptr, nullptr, "00B\u200f"}, // 1e10
        {nullptr, "000B\u200f", "000B\u200f", nullptr, nullptr, "000B\u200f"}, // 1e11
        {nullptr, "0T\u200f", "0T\u200f", nullptr, nullptr, "0T\u200f"}, // 1e12
        {nullptr, "00T\u200f", "00T\u200f", nullptr, nullptr, "00T\u200f"}, // 1e13
        {nullptr, "000T\u200f", "000T\u200f", nullptr, nullptr, "000T\u200f"}, // 1e14
      },
      {
        {nullptr, "\u200f0 \u05d0\u05dc\u05e3", "\u200f0 \u05d0\u05dc\u05e3", nullptr, "\u200f0 \u05d0\u05dc\u05e3", "\u200f0 \u05d0\u05dc\u05e3"}, // 1e3
        {nullptr, "\u200f00 \u05d0\u05dc\u05e3", "\u200f00 \u05d0\u05dc\u05e3", nullptr, "\u200f00 \u05d0\u05dc\u05e3", "\u200f00 \u05d0\u05dc\u05e3"}, // 1e4
        {nullptr, "\u200f000 \u05d0\u05dc\u05e3", "\u200f000 \u05d0\u05dc\u05e3", nullptr, "\u200f000 \u05d0\u05dc\u05e3", "\u200f000 \u05d0\u05dc\u05e3"}, // 1e5
        {nullptr, "\u200f0 \u05de\u05d9\u05dc\u05d9\u05d5\u05df", "\u200f0 \u05de\u05d9\u05dc\u05d9\u05d5\u05df", nullptr, "\u200f0 \u05de\u05d9\u05dc\u05d9\u05d5\u05df", "\u200f0 \u05de\u05d9\u05dc\u05d9\u05d5\u05df"}, // 1e6
        {nullptr, "\u200f00 \u05de\u05d9\u05dc\u05d9\u05d5\u05df", "\u200f00 \u05de\u05d9\u05dc\u05d9\u05d5\u05df", nullptr, "\u200f00 \u05de\u05d9\u05dc\u05d9\u05d5\u05df", "\u200f00 \u05de\u05d9\u05dc\u05d9\u05d5\u05df"}, // 1e7
        {nullptr, "\u200f000 \u05de\u05d9\u05dc\u05d9\u05d5\u05df", "\u200f000 \u05de\u05d9\u05dc\u05d9\u05d5\u05df", nullptr, "\u200f000 \u05de\u05d9\u05dc\u05d9\u05d5\u05df", "\u200f000 \u05de\u05d9\u05dc\u05d9\u05d5\u05df"}, // 1e8
        {nullptr, "\u200f0 \u05de\u05d9\u05dc\u05d9\u05d0\u05e8\u05d3", "\u200f0 \u05de\u05d9\u05dc\u05d9\u05d0\u05e8\u05d3", nullptr, "\u200f0 \u05de\u05d9\u05dc\u05d9\u05d0\u05e8\u05d3", "\u200f0 \u05de\u05d9\u05dc\u05d9\u05d0\u05e8\u05d3"}, // 1e9
        {nullptr, "\u200f00 \u05de\u05d9\u05dc\u05d9\u05d0\u05e8\u05d3", "\u200f00 \u05de\u05d9\u05dc\u05d9\u05d0\u05e8\u05d3", nullptr, "\u200f00 \u05de\u05d9\u05dc\u05d9\u05d0\u05e8\u05d3", "\u200f00 \u05de\u05d9\u05dc\u05d9\u05d0\u05e8\u05d3"}, // 1e10
        {nullptr, "\u200f000 \u05de\u05d9\u05dc\u05d9\u05d0\u05e8\u05d3", "\u200f000 \u05de\u05d9\u05dc\u05d9\u05d0\u05e8\u05d3", nullptr, "\u200f000 \u05de\u05d9\u05dc\u05d9\u05d0\u05e8\u05d3", "\u200f000 \u05de\u05d9\u05dc\u05d9\u05d0\u05e8\u05d3"}, // 1e11
        {nullptr, "\u200f0 \u05d8\u05e8\u05d9\u05dc\u05d9\u05d5\u05df", "\u200f0 \u05d8\u05e8\u05d9\u05dc\u05d9\u05d5\u05df", nullptr, "\u200f0 \u05d8\u05e8\u05d9\u05dc\u05d9\u05d5\u05df", "\u200f0 \u05d8\u05e8\u05d9\u05dc\u05d9\u05d5\u05df"}, // 1e12
        {nullptr, "\u200f00 \u05d8\u05e8\u05d9\u05dc\u05d9\u05d5\u05df", "\u200f00 \u05d8\u05e8\u05d9\u05dc\u05d9\u05d5\u05df", nullptr, "\u200f00 \u05d8\u05e8\u05d9\u05dc\u05d9\u05d5\u05df", "\u200f00 \u05d8\u05e8\u05d9\u05dc\u05d9\u05d5\u05df"}, // 1e13
        {nullptr, "\u200f000 \u05d8\u05e8\u05d9\u05dc\u05d9\u05d5\u05df", "\u200f000 \u05d8\u05e8\u05d9\u05dc\u05d9\u05d5\u05df", nullptr, "\u200f000 \u05d8\u05e8\u05d9\u05dc\u05d9\u05d5\u05df", "\u200f000 \u05d8\u05e8\u05d9\u05dc\u05d9\u05d5\u05df"}, // 1e14
      },
    },
    {"hi",
      {
        {nullptr, "0\u00a0\u0939\u091c\u093c\u093e\u0930", nullptr, nullptr, nullptr, "0\u00a0\u0939\u091c\u093c\u093e\u0930"}, // 1e3
        {nullptr, "00\u00a0\u0939\u091c\u093c\u093e\u0930", nullptr, nullptr, nullptr, "00\u00a0\u0939\u091c\u093c\u093e\u0930"}, // 1e4
        {nullptr, "0\u00a0\u0932\u093e\u0916", nullptr, nullptr, nullptr, "0\u00a0\u0932\u093e\u0916"}, // 1e5
        {nullptr, "00\u00a0\u0932\u093e\u0916", nullptr, nullptr, nullptr, "00\u00a0\u0932\u093e\u0916"}, // 1e6
        {nullptr, "0\u00a0\u0915\u0970", nullptr, nullptr, nullptr, "0\u00a0\u0915\u0970"}, // 1e7
        {nullptr, "00\u00a0\u0915\u0970", nullptr, nullptr, nullptr, "00\u00a0\u0915\u0970"}, // 1e8
        {nullptr, "0\u00a0\u0905\u0970", nullptr, nullptr, nullptr, "0\u00a0\u0905\u0970"}, // 1e9
        {nullptr, "00\u00a0\u0905\u0970", nullptr, nullptr, nullptr, "00\u00a0\u0905\u0970"}, // 1e10
        {nullptr, "0\u00a0\u0916\u0970", nullptr, nullptr, nullptr, "0\u00a0\u0916\u0970"}, // 1e11
        {nullptr, "00\u00a0\u0916\u0970", nullptr, nullptr, nullptr, "00\u00a0\u0916\u0970"}, // 1e12
        {nullptr, "0\u00a0\u0928\u0940\u0932", nullptr, nullptr, nullptr, "0\u00a0\u0928\u0940\u0932"}, // 1e13
        {nullptr, "00\u00a0\u0928\u0940\u0932", nullptr, nullptr, nullptr, "00\u00a0\u0928\u0940\u0932"}, // 1e14
      },
      {
        {nullptr, "0 \u0939\u091c\u093c\u093e\u0930", nullptr, nullptr, nullptr, "0 \u0939\u091c\u093c\u093e\u0930"}, // 1e3
        {nullptr, "00 \u0939\u091c\u093c\u093e\u0930", nullptr, nullptr, nullptr, "00 \u0939\u091c\u093c\u093e\u0930"}, // 1e4
        {nullptr, "0 \u0932\u093e\u0916", nullptr, nullptr, nullptr, "0 \u0932\u093e\u0916"}, // 1e5
        {nullptr, "00 \u0932\u093e\u0916", nullptr, nullptr, nullptr, "00 \u0932\u093e\u0916"}, // 1e6
        {nullptr, "0 \u0915\u0930\u094b\u0921\u093c", nullptr, nullptr, nullptr, "0 \u0915\u0930\u094b\u0921\u093c"}, // 1e7
        {nullptr, "00 \u0915\u0930\u094b\u0921\u093c", nullptr, nullptr, nullptr, "00 \u0915\u0930\u094b\u0921\u093c"}, // 1e8
        {nullptr, "0 \u0905\u0930\u092c", nullptr, nullptr, nullptr, "0 \u0905\u0930\u092c"}, // 1e9
        {nullptr, "00 \u0905\u0930\u092c", nullptr, nullptr, nullptr, "00 \u0905\u0930\u092c"}, // 1e10
        {nullptr, "0 \u0916\u0930\u092c", nullptr, nullptr, nullptr, "0 \u0916\u0930\u092c"}, // 1e11
        {nullptr, "00 \u0916\u0930\u092c", nullptr, nullptr, nullptr, "00 \u0916\u0930\u092c"}, // 1e12
        {nullptr, "000 \u0916\u0930\u092c", nullptr, nullptr, nullptr, "000 \u0916\u0930\u092c"}, // 1e13
        {nullptr, "0000 \u0916\u0930\u092c", nullptr, nullptr, nullptr, "0000 \u0916\u0930\u092c"}, // 1e14
      },
    },
    {"ja",
      {
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0"}, // 1e3
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u4e07"}, // 1e4
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u4e07"}, // 1e5
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u4e07"}, // 1e6
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\u4e07"}, // 1e7
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u5104"}, // 1e8
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u5104"}, // 1e9
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u5104"}, // 1e10
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\u5104"}, // 1e11
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u5146"}, // 1e12
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u5146"}, // 1e13
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u5146"}, // 1e14
      },
      {
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0"}, // 1e3
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u4e07"}, // 1e4
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u4e07"}, // 1e5
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u4e07"}, // 1e6
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\u4e07"}, // 1e7
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u5104"}, // 1e8
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u5104"}, // 1e9
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u5104"}, // 1e10
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\u5104"}, // 1e11
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u5146"}, // 1e12
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u5146"}, // 1e13
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u5146"}, // 1e14
      },
    },
    {"ko",
      {
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\ucc9c"}, // 1e3
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\ub9cc"}, // 1e4
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\ub9cc"}, // 1e5
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\ub9cc"}, // 1e6
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\ub9cc"}, // 1e7
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\uc5b5"}, // 1e8
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\uc5b5"}, // 1e9
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\uc5b5"}, // 1e10
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\uc5b5"}, // 1e11
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\uc870"}, // 1e12
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\uc870"}, // 1e13
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\uc870"}, // 1e14
      },
      {
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\ucc9c"}, // 1e3
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\ub9cc"}, // 1e4
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\ub9cc"}, // 1e5
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\ub9cc"}, // 1e6
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\ub9cc"}, // 1e7
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\uc5b5"}, // 1e8
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\uc5b5"}, // 1e9
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\uc5b5"}, // 1e10
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\uc5b5"}, // 1e11
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\uc870"}, // 1e12
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\uc870"}, // 1e13
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\uc870"}, // 1e14
      },
    },
    {"zh",
      {
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0"}, // 1e3
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u4e07"}, // 1e4
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u4e07"}, // 1e5
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u4e07"}, // 1e6
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\u4e07"}, // 1e7
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u4ebf"}, // 1e8
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u4ebf"}, // 1e9
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u4ebf"}, // 1e10
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\u4ebf"}, // 1e11
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u4e07\u4ebf"}, // 1e12
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u4e07\u4ebf"}, // 1e13
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u4e07\u4ebf"}, // 1e14
      },
      {
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0"}, // 1e3
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u4e07"}, // 1e4
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u4e07"}, // 1e5
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u4e07"}, // 1e6
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\u4e07"}, // 1e7
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u4ebf"}, // 1e8
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u4ebf"}, // 1e9
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u4ebf"}, // 1e10
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\u4ebf"}, // 1e11
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u4e07\u4ebf"}, // 1e12
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u4e07\u4ebf"}, // 1e13
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u4e07\u4ebf"}, // 1e14
      },
    },
    {"zh-Hant",
      {
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0"}, // 1e3
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u842c"}, // 1e4
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u842c"}, // 1e5
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u842c"}, // 1e6
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\u842c"}, // 1e7
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u5104"}, // 1e8
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u5104"}, // 1e9
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u5104"}, // 1e10
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\u5104"}, // 1e11
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u5146"}, // 1e12
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u5146"}, // 1e13
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u5146"}, // 1e14
      },
      {
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0"}, // 1e3
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u842c"}, // 1e4
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u842c"}, // 1e5
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u842c"}, // 1e6
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\u842c"}, // 1e7
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u5104"}, // 1e8
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u5104"}, // 1e9
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u5104"}, // 1e10
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0000\u5104"}, // 1e11
        {nullptr, nullptr, nullptr, nullptr, nullptr, "0\u5146"}, // 1e12
        {nullptr, nullptr, nullptr, nullptr, nullptr, "00\u5146"}, // 1e13
        {nullptr, nullptr, nullptr, nullptr, nullptr, "000\u5146"}, // 1e14
      },
    }
};

} // namespace detail
} // namespace localisation
