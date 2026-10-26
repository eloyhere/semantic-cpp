#include "localisation/localisation.h"

#include "test_framework.h"

#include <sstream>
#include <string>
#include <vector>

namespace
{
using localisation::Arguments;
using localisation::Catalog;
using localisation::Locale;
using localisation::Message;
using localisation::NumberFormat;
using localisation::PluralCategory;
using localisation::TextDirection;

const Locale english("en");
const Locale german("de");
const Locale french("fr");
const Locale russian("ru");
const Locale japanese("ja");

std::string format(const Message &message, const Locale &locale, const Arguments &arguments)
{
    return message.format(locale, arguments);
}
} // namespace

TEST_CASE("localisation: locale canonicalisation")
{
    CHECK_EQ(Locale("zh_hant_tw").tag(), std::string("zh-Hant-TW"));
    CHECK_EQ(Locale("EN-us").tag(), std::string("en-US"));
    CHECK_EQ(Locale("es-419").tag(), std::string("es-419"));
    CHECK_EQ(Locale("en-US-u-ca-buddhist").tag(), std::string("en-US"));
    CHECK_EQ(Locale("en_Latn_GB").tag(), std::string("en-Latn-GB"));
    CHECK_EQ(Locale().tag(), std::string("und"));
    CHECK_FALSE(Locale().specified());
    CHECK_FALSE(Locale("not a tag").specified());
    CHECK(Locale("fr").specified());
    CHECK_EQ(Locale("fr", "", "CA").tag(), std::string("fr-CA"));
    CHECK_EQ(Locale("", "", "CA").tag(), std::string("und-CA"));
    CHECK_EQ(Locale("", "", "").tag(), std::string("und"));
    CHECK_EQ(Locale("SR", "latn", "rs").tag(), std::string("sr-Latn-RS"));

    // English variants stay distinct so catalogues can override spelling.
    CHECK_EQ(Locale("en-GB").tag(), std::string("en-GB"));
    CHECK_EQ(Locale("en-US").tag(), std::string("en-US"));
    CHECK_EQ(Locale("en").tag(), std::string("en"));

    // Chinese script inference and the Cantonese extlang tag.
    CHECK_EQ(Locale("zh-CN").tag(), std::string("zh-Hans-CN"));
    CHECK_EQ(Locale("zh-SG").tag(), std::string("zh-Hans-SG"));
    CHECK_EQ(Locale("zh-TW").tag(), std::string("zh-Hant-TW"));
    CHECK_EQ(Locale("zh-HK").tag(), std::string("zh-Hant-HK"));
    CHECK_EQ(Locale("zh-MO").tag(), std::string("zh-Hant-MO"));
    CHECK_EQ(Locale("zh-Hant").tag(), std::string("zh-Hant"));
    CHECK_EQ(Locale("zh-Hans").tag(), std::string("zh-Hans"));
    CHECK_EQ(Locale("yue").tag(), std::string("yue-Hant"));
    CHECK_EQ(Locale("yue-HK").tag(), std::string("yue-Hant-HK"));
    CHECK_EQ(Locale("zh-yue-HK").tag(), std::string("yue-Hant-HK"));

    // Mongolian keeps its script: Cyrillic in Mongolia, Mongolian script in
    // China (and therefore a different writing direction).
    CHECK_EQ(Locale("mn").tag(), std::string("mn"));
    CHECK_EQ(Locale("mn-MN").tag(), std::string("mn-Cyrl-MN"));
    CHECK_EQ(Locale("mn-CN").tag(), std::string("mn-Mong-CN"));
}

TEST_CASE("localisation: locale fallbacks")
{
    const std::vector<Locale> chain = Locale("zh-Hant-TW").fallbacks();
    CHECK_EQ(chain.size(), static_cast<std::size_t>(3));
    CHECK_EQ(chain[0].tag(), std::string("zh-Hant-TW"));
    CHECK_EQ(chain[1].tag(), std::string("zh-Hant"));
    CHECK_EQ(chain[2].tag(), std::string("zh"));

    const std::vector<Locale> shortChain = Locale("en").fallbacks();
    CHECK_EQ(shortChain.size(), static_cast<std::size_t>(1));
    CHECK_EQ(shortChain[0].tag(), std::string("en"));

    const std::vector<Locale> traditional = Locale("zh-HK").fallbacks();
    CHECK_EQ(traditional.size(), static_cast<std::size_t>(3));
    CHECK_EQ(traditional[0].tag(), std::string("zh-Hant-HK"));
    CHECK_EQ(traditional[1].tag(), std::string("zh-Hant"));
    CHECK_EQ(traditional[2].tag(), std::string("zh"));

    const std::vector<Locale> cantonese = Locale("yue-HK").fallbacks();
    CHECK_EQ(cantonese.size(), static_cast<std::size_t>(3));
    CHECK_EQ(cantonese[0].tag(), std::string("yue-Hant-HK"));
    CHECK_EQ(cantonese[1].tag(), std::string("yue-Hant"));
    CHECK_EQ(cantonese[2].tag(), std::string("yue"));
}

TEST_CASE("localisation: language negotiation")
{
    const std::vector<Locale> available{english, french, german};

    CHECK_EQ(localisation::negotiate(Locale("fr-CA"), available).tag(), std::string("fr"));
    CHECK_EQ(localisation::negotiate(Locale("de-AT"), available).tag(), std::string("de"));
    CHECK_EQ(localisation::negotiate(Locale("ja"), available).tag(), std::string("en"));
    CHECK_EQ(localisation::negotiate(Locale("ja"), available, Locale("fr")).tag(), std::string("fr"));
    CHECK_EQ(localisation::negotiate(std::vector<Locale>{Locale("it"), Locale("fr-CH")}, available).tag(),
             std::string("fr"));
    CHECK_EQ(localisation::negotiate(Locale("de-CH"), std::vector<Locale>{Locale("de-CH"), Locale("de")}).tag(),
             std::string("de-CH"));
}

TEST_CASE("localisation: accept language")
{
    const std::vector<Locale> parsed =
        localisation::parseAcceptLanguage("fr-CH, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5");
    CHECK_EQ(parsed.size(), static_cast<std::size_t>(4));
    CHECK_EQ(parsed[0].tag(), std::string("fr-CH"));
    CHECK_EQ(parsed[1].tag(), std::string("fr"));
    CHECK_EQ(parsed[2].tag(), std::string("en"));
    CHECK_EQ(parsed[3].tag(), std::string("de"));

    const std::vector<Locale> unacceptable = localisation::parseAcceptLanguage("de;q=0, fr");
    CHECK_EQ(unacceptable.size(), static_cast<std::size_t>(1));
    CHECK_EQ(unacceptable[0].tag(), std::string("fr"));

    const std::vector<Locale> tied = localisation::parseAcceptLanguage("en;q=0.8, fr;q=0.8");
    CHECK_EQ(tied.size(), static_cast<std::size_t>(2));
    CHECK_EQ(tied[0].tag(), std::string("en"));
    CHECK_EQ(tied[1].tag(), std::string("fr"));

    CHECK(localisation::parseAcceptLanguage("").empty());
}

TEST_CASE("localisation: number formatting")
{
    CHECK_EQ(localisation::formatNumber(1234567.891, english), std::string("1,234,567.891"));
    CHECK_EQ(localisation::formatNumber(1234.5, german), std::string("1.234,5"));
    CHECK_EQ(localisation::formatNumber(1234.5, Locale("de-CH")), std::string("1\xE2\x80\x99" "234.5"));
    CHECK_EQ(localisation::formatNumber(1234.5, french), std::string("1\xE2\x80\xAF" "234,5"));
    CHECK_EQ(localisation::formatNumber(12345678, Locale("en-IN")), std::string("1,23,45,678"));
    CHECK_EQ(localisation::formatNumber(12345678, Locale("hi")), std::string("1,23,45,678"));
    CHECK_EQ(localisation::formatNumber(-1234.5, german), std::string("-1.234,5"));
    CHECK_EQ(localisation::formatNumber(0, english), std::string("0"));
    CHECK_EQ(localisation::formatNumber(1000, english), std::string("1,000"));

    NumberFormat fixed;
    fixed.minimumFractionDigits = 2;
    fixed.maximumFractionDigits = 2;
    CHECK_EQ(localisation::formatNumber(1.2, english, fixed), std::string("1.20"));

    NumberFormat integer;
    integer.maximumFractionDigits = 0;
    CHECK_EQ(localisation::formatNumber(1234.6, english, integer), std::string("1,235"));

    NumberFormat percent;
    percent.percent = true;
    CHECK_EQ(localisation::formatNumber(0.5, english, percent), std::string("50%"));
    CHECK_EQ(localisation::formatNumber(0.5, french, percent), std::string("50\xC2\xA0%"));
    CHECK_EQ(localisation::formatNumber(0.5, Locale("tr"), percent), std::string("%50"));
    CHECK_EQ(localisation::formatNumber(0.5, Locale("ar"), percent), std::string("\xD9\xA5\xD9\xA0\xD9\xAA"));

    // English variants share numeric symbols but remain distinct locales.
    CHECK_EQ(localisation::formatNumber(1234567.5, Locale("en-GB")), std::string("1,234,567.5"));
    CHECK_EQ(localisation::formatNumber(1234567.5, Locale("en-US")), std::string("1,234,567.5"));

    CHECK_EQ(localisation::formatNumber(1234.5, Locale("es-419")), std::string("1,234.5"));

    // Chinese variants share the Latin numbering system and 3-digit grouping.
    CHECK_EQ(localisation::formatNumber(1234567.5, Locale("zh-CN")), std::string("1,234,567.5"));
    CHECK_EQ(localisation::formatNumber(1234567.5, Locale("zh-SG")), std::string("1,234,567.5"));
    CHECK_EQ(localisation::formatNumber(1234567.5, Locale("zh-TW")), std::string("1,234,567.5"));
    CHECK_EQ(localisation::formatNumber(1234567.5, Locale("zh-HK")), std::string("1,234,567.5"));
    CHECK_EQ(localisation::formatNumber(1234567.5, Locale("zh-MO")), std::string("1,234,567.5"));
    CHECK_EQ(localisation::formatNumber(1234.5, Locale("yue")), std::string("1,234.5"));

    // Mongolian, Tibetan and Dzongkha default to the Latin numbering system.
    CHECK_EQ(localisation::formatNumber(1234.5, Locale("mn-MN")), std::string("1,234.5"));
    CHECK_EQ(localisation::formatNumber(1234.5, Locale("bo")), std::string("1,234.5"));
    CHECK_EQ(localisation::formatNumber(1234.5, Locale("dz")), std::string("1,234.5"));

    // Arabic-Indic digits with U+066C grouping and U+066B decimal separator.
    const std::string arabicNumber = "\xD9\xA1\xD9\xAC\xD9\xA2\xD9\xA3\xD9\xA4\xD9\xAB\xD9\xA5";
    CHECK_EQ(localisation::formatNumber(1234.5, Locale("ar")), arabicNumber);
    CHECK_EQ(localisation::formatNumber(1234.5, Locale("ar-EG")), arabicNumber);
    CHECK_EQ(localisation::formatNumber(1234.5, Locale("ar-SA")), arabicNumber);

    // Bengali digits.
    CHECK_EQ(localisation::formatNumber(1234.5, Locale("bn")),
             std::string("\xE0\xA7\xA7,\xE0\xA7\xA8\xE0\xA7\xA9\xE0\xA7\xAA."
                         "\xE0\xA7\xAB"));

    // Native-digit opt-in: Tibetan digits for bo/dz, hanidec digits for zh/yue.
    NumberFormat native;
    native.nativeDigits = true;
    CHECK_EQ(localisation::formatNumber(1234.5, Locale("bo"), native),
             std::string("\xE0\xBC\xA1,\xE0\xBC\xA2\xE0\xBC\xA3\xE0\xBC\xA4."
                         "\xE0\xBC\xA5"));
    CHECK_EQ(localisation::formatNumber(1234.5, Locale("dz"), native),
             std::string("\xE0\xBC\xA1,\xE0\xBC\xA2\xE0\xBC\xA3\xE0\xBC\xA4."
                         "\xE0\xBC\xA5"));
    CHECK_EQ(localisation::formatNumber(1234.5, Locale("zh-TW"), native),
             std::string("\xE4\xB8\x80,\xE4\xBA\x8C\xE4\xB8\x89\xE5\x9B\x9B."
                         "\xE4\xBA\x94"));
}

TEST_CASE("localisation: cardinal plural categories")
{
    CHECK(localisation::pluralCategory(english, 0) == PluralCategory::other);
    CHECK(localisation::pluralCategory(english, 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(english, 2) == PluralCategory::other);
    CHECK(localisation::pluralCategory(english, 1.5) == PluralCategory::other);

    CHECK(localisation::pluralCategory(french, 0) == PluralCategory::one);
    CHECK(localisation::pluralCategory(french, 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(french, 1.5) == PluralCategory::one);
    CHECK(localisation::pluralCategory(french, 2) == PluralCategory::other);
    CHECK(localisation::pluralCategory(french, 1000000) == PluralCategory::many);

    CHECK(localisation::pluralCategory(russian, 0) == PluralCategory::many);
    CHECK(localisation::pluralCategory(russian, 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(russian, 2) == PluralCategory::few);
    CHECK(localisation::pluralCategory(russian, 5) == PluralCategory::many);
    CHECK(localisation::pluralCategory(russian, 11) == PluralCategory::many);
    CHECK(localisation::pluralCategory(russian, 21) == PluralCategory::one);
    CHECK(localisation::pluralCategory(russian, 22) == PluralCategory::few);
    CHECK(localisation::pluralCategory(russian, 1.5) == PluralCategory::other);

    CHECK(localisation::pluralCategory(Locale("pl"), 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(Locale("pl"), 2) == PluralCategory::few);
    CHECK(localisation::pluralCategory(Locale("pl"), 5) == PluralCategory::many);
    CHECK(localisation::pluralCategory(Locale("pl"), 12) == PluralCategory::many);
    CHECK(localisation::pluralCategory(Locale("pl"), 22) == PluralCategory::few);
    CHECK(localisation::pluralCategory(Locale("pl"), 0) == PluralCategory::many);

    CHECK(localisation::pluralCategory(Locale("cs"), 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(Locale("cs"), 2) == PluralCategory::few);
    CHECK(localisation::pluralCategory(Locale("cs"), 5) == PluralCategory::other);
    CHECK(localisation::pluralCategory(Locale("cs"), 1.5) == PluralCategory::many);

    CHECK(localisation::pluralCategory(Locale("ar"), 0) == PluralCategory::zero);
    CHECK(localisation::pluralCategory(Locale("ar"), 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(Locale("ar"), 2) == PluralCategory::two);
    CHECK(localisation::pluralCategory(Locale("ar"), 3) == PluralCategory::few);
    CHECK(localisation::pluralCategory(Locale("ar"), 11) == PluralCategory::many);
    CHECK(localisation::pluralCategory(Locale("ar"), 100) == PluralCategory::other);

    CHECK(localisation::pluralCategory(Locale("cy"), 0) == PluralCategory::zero);
    CHECK(localisation::pluralCategory(Locale("cy"), 3) == PluralCategory::few);
    CHECK(localisation::pluralCategory(Locale("cy"), 6) == PluralCategory::many);
    CHECK(localisation::pluralCategory(Locale("cy"), 4) == PluralCategory::other);

    CHECK(localisation::pluralCategory(Locale("ga"), 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(Locale("ga"), 2) == PluralCategory::two);
    CHECK(localisation::pluralCategory(Locale("ga"), 3) == PluralCategory::few);
    CHECK(localisation::pluralCategory(Locale("ga"), 7) == PluralCategory::many);
    CHECK(localisation::pluralCategory(Locale("ga"), 11) == PluralCategory::other);

    CHECK(localisation::pluralCategory(Locale("he"), 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(Locale("he"), 2) == PluralCategory::two);
    CHECK(localisation::pluralCategory(Locale("he"), 20) == PluralCategory::many);
    CHECK(localisation::pluralCategory(Locale("he"), 10) == PluralCategory::other);

    CHECK(localisation::pluralCategory(Locale("lt"), 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(Locale("lt"), 2) == PluralCategory::few);
    CHECK(localisation::pluralCategory(Locale("lt"), 11) == PluralCategory::other);
    CHECK(localisation::pluralCategory(Locale("lt"), 1.5) == PluralCategory::many);

    CHECK(localisation::pluralCategory(japanese, 0) == PluralCategory::other);
    CHECK(localisation::pluralCategory(japanese, 1) == PluralCategory::other);
    CHECK(localisation::pluralCategory(japanese, 100) == PluralCategory::other);

    // Chinese has no grammatical plural, in either script or any region.
    CHECK(localisation::pluralCategory(Locale("zh-CN"), 1) == PluralCategory::other);
    CHECK(localisation::pluralCategory(Locale("zh-TW"), 2) == PluralCategory::other);
    CHECK(localisation::pluralCategory(Locale("zh-HK"), 100) == PluralCategory::other);
    CHECK(localisation::pluralCategory(Locale("yue"), 1) == PluralCategory::other);

    // Mongolian has one/other; Tibetan and Dzongkha only have other.
    CHECK(localisation::pluralCategory(Locale("mn"), 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(Locale("mn"), 2) == PluralCategory::other);
    CHECK(localisation::pluralCategory(Locale("mn-Mong-CN"), 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(Locale("bo"), 1) == PluralCategory::other);
    CHECK(localisation::pluralCategory(Locale("dz"), 2) == PluralCategory::other);

    // Arabic categories apply to every regional variant.
    CHECK(localisation::pluralCategory(Locale("ar-SA"), 0) == PluralCategory::zero);
    CHECK(localisation::pluralCategory(Locale("ar-EG"), 2) == PluralCategory::two);
    CHECK(localisation::pluralCategory(Locale("ar-SA"), 3) == PluralCategory::few);
    CHECK(localisation::pluralCategory(Locale("ar-EG"), 11) == PluralCategory::many);

    CHECK(localisation::pluralCategory(Locale("ug"), 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(Locale("sd"), 1) == PluralCategory::one);
    CHECK(localisation::pluralCategory(Locale("ckb"), 1) == PluralCategory::one);
}

TEST_CASE("localisation: ordinal plural categories")
{
    CHECK(localisation::ordinalCategory(english, 1) == PluralCategory::one);
    CHECK(localisation::ordinalCategory(english, 2) == PluralCategory::two);
    CHECK(localisation::ordinalCategory(english, 3) == PluralCategory::few);
    CHECK(localisation::ordinalCategory(english, 4) == PluralCategory::other);
    CHECK(localisation::ordinalCategory(english, 11) == PluralCategory::other);
    CHECK(localisation::ordinalCategory(english, 21) == PluralCategory::one);
    CHECK(localisation::ordinalCategory(english, 101) == PluralCategory::one);
    CHECK(localisation::ordinalCategory(english, 111) == PluralCategory::other);

    CHECK(localisation::ordinalCategory(Locale("sv"), 1) == PluralCategory::one);
    CHECK(localisation::ordinalCategory(Locale("sv"), 2) == PluralCategory::one);
    CHECK(localisation::ordinalCategory(Locale("sv"), 11) == PluralCategory::other);
    CHECK(localisation::ordinalCategory(Locale("sv"), 21) == PluralCategory::one);

    CHECK(localisation::ordinalCategory(Locale("cy"), 0) == PluralCategory::zero);
    CHECK(localisation::ordinalCategory(Locale("cy"), 3) == PluralCategory::few);
    CHECK(localisation::ordinalCategory(Locale("cy"), 5) == PluralCategory::many);
    CHECK(localisation::ordinalCategory(Locale("cy"), 7) == PluralCategory::zero);

    CHECK(localisation::ordinalCategory(french, 1) == PluralCategory::one);
    CHECK(localisation::ordinalCategory(french, 2) == PluralCategory::other);
}

TEST_CASE("localisation: message arguments and numbers")
{
    const Arguments arguments = Arguments().set("name", "Ada").set("count", 3).set("ratio", 0.25);

    CHECK_EQ(format(Message("Hello, {name}!"), english, arguments), std::string("Hello, Ada!"));
    CHECK_EQ(format(Message("{count} and {ratio}"), english, arguments), std::string("3 and 0.25"));
    CHECK_EQ(format(Message("{name} {name}"), english, arguments), std::string("Ada Ada"));
    CHECK_EQ(format(Message("no arguments"), english, Arguments()), std::string("no arguments"));
    CHECK_EQ(format(Message("{n, number}"), german, Arguments().set("n", 1234.5)), std::string("1.234,5"));
    CHECK_EQ(format(Message("{n, number, integer}"), english, Arguments().set("n", 1234.6)),
             std::string("1,235"));
    CHECK_EQ(format(Message("{n, number, percent}"), english, Arguments().set("n", 0.25)), std::string("25%"));
    CHECK_EQ(format(Message("{n, number, ::.##}"), english, Arguments().set("n", 1.23456)), std::string("1.23"));
    CHECK_EQ(format(Message("{n, number, ::.00}"), english, Arguments().set("n", 1.2)), std::string("1.20"));
    CHECK_EQ(format(Message("{n}"), english, Arguments().set("n", 1000000)), std::string("1000000"));
}

TEST_CASE("localisation: message escaping")
{
    CHECK_EQ(format(Message("This '{isn''t}' obvious"), english, Arguments()),
             std::string("This {isn't} obvious"));
    CHECK_EQ(format(Message("It''s fine"), english, Arguments()), std::string("It's fine"));
    CHECK_EQ(format(Message("'{'name'}'"), english, Arguments()), std::string("{name}"));
    CHECK_EQ(format(Message("Issue #42"), english, Arguments()), std::string("Issue #42"));
    CHECK_EQ(format(Message("{count, plural, one {'#' # item} other {'#' # items}}"), english,
                     Arguments().set("count", 1)),
             std::string("# 1 item"));
}

TEST_CASE("localisation: message plurals and selections")
{
    const Message items("{count, plural, =0 {no items} one {# item} other {# items}}");
    CHECK_EQ(format(items, english, Arguments().set("count", 0)), std::string("no items"));
    CHECK_EQ(format(items, english, Arguments().set("count", 1)), std::string("1 item"));
    CHECK_EQ(format(items, english, Arguments().set("count", 5)), std::string("5 items"));
    CHECK_EQ(format(items, english, Arguments().set("count", 1000)), std::string("1,000 items"));
    CHECK_EQ(format(items, english, Arguments().set("count", "3")), std::string("3 items"));

    const Message offset(
        "{count, plural, offset:1 =0 {nobody} =1 {just you} one {you and # other} other {you and # others}}");
    CHECK_EQ(format(offset, english, Arguments().set("count", 1)), std::string("nobody"));
    CHECK_EQ(format(offset, english, Arguments().set("count", 2)), std::string("just you"));
    CHECK_EQ(format(offset, english, Arguments().set("count", 4)), std::string("you and 3 others"));

    const Message gender("{gender, select, female {She} male {He} other {They}}");
    CHECK_EQ(format(gender, english, Arguments().set("gender", "female")), std::string("She"));
    CHECK_EQ(format(gender, english, Arguments().set("gender", "male")), std::string("He"));
    CHECK_EQ(format(gender, english, Arguments().set("gender", "other")), std::string("They"));
    CHECK_EQ(format(gender, english, Arguments().set("gender", "unknown")), std::string("They"));
    CHECK_EQ(format(gender, english, Arguments()), std::string("They"));

    const Message nested(
        "{count, plural, one {{gender, select, female {She has # item} other {They have # item}}} "
        "other {{gender, select, female {She has # items} other {They have # items}}}}");
    CHECK_EQ(format(nested, english, Arguments().set("count", 1).set("gender", "female")),
             std::string("She has 1 item"));
    CHECK_EQ(format(nested, english, Arguments().set("count", 3).set("gender", "male")),
             std::string("They have 3 items"));

    const Message ordinal("{place, selectordinal, one {#st} two {#nd} few {#rd} other {#th}}");
    CHECK_EQ(format(ordinal, english, Arguments().set("place", 1)), std::string("1st"));
    CHECK_EQ(format(ordinal, english, Arguments().set("place", 2)), std::string("2nd"));
    CHECK_EQ(format(ordinal, english, Arguments().set("place", 3)), std::string("3rd"));
    CHECK_EQ(format(ordinal, english, Arguments().set("place", 11)), std::string("11th"));
    CHECK_EQ(format(ordinal, english, Arguments().set("place", 21)), std::string("21st"));

    const Message nestedPlural(
        "{outer, plural, other {outer #, inner {inner, plural, other {#}}}}");
    CHECK_EQ(format(nestedPlural, english, Arguments().set("outer", 1).set("inner", 2)),
             std::string("outer 1, inner 2"));

    const Message russianItems("{count, plural, one {# file} few {# files} many {# files} other {# files}}");
    CHECK_EQ(format(russianItems, russian, Arguments().set("count", 1)), std::string("1 file"));
    CHECK_EQ(format(russianItems, russian, Arguments().set("count", 2)), std::string("2 files"));
    CHECK_EQ(format(russianItems, russian, Arguments().set("count", 5)), std::string("5 files"));
}

TEST_CASE("localisation: message placeholders when arguments are missing")
{
    CHECK_EQ(format(Message("Hi {name}"), english, Arguments()), std::string("Hi {name}"));
    CHECK_EQ(format(Message("{count, plural, one {one} other {many}}"), english, Arguments()),
             std::string("many"));
    CHECK_EQ(format(Message("{count, plural, one {#} other {#}}"), english, Arguments()),
             std::string("{count}"));
}

TEST_CASE("localisation: invalid patterns are rejected")
{
    CHECK_THROWS_AS(Message("Hello {name"), std::invalid_argument);
    CHECK_THROWS_AS(Message("Hello }"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{a, plural, one {x}}"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{a, select, x {y}}"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{a, plural, one {x} one {y} other {z}}"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{a, number, currency}"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{a, wat, x {y}}"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{a, plural, wat {x} other {y}}"), std::invalid_argument);
    CHECK_THROWS_AS(Message("{a, plural, one {x}"), std::invalid_argument);

    std::string deep;
    for (int level = 0; level < 33; ++level)
    {
        deep += "{count, plural, other {";
    }
    deep += "x";
    for (int level = 0; level < 33; ++level)
    {
        deep += "}}";
    }
    CHECK_THROWS_AS(Message(deep), std::invalid_argument);
}

TEST_CASE("localisation: catalogue fallback chain")
{
    Catalog catalog("en");
    catalog.add(english, "greeting", "Hello, {name}!");
    catalog.add(german, "greeting", "Hallo, {name}!");
    catalog.add(english, "items", "{count, plural, one {# item} other {# items}}");
    catalog.add(german, "items", "{count, plural, one {# Artikel} other {# Artikel}}");

    CHECK_EQ(catalog.translate(Locale("de-AT"), "greeting", Arguments().set("name", "Ada")),
             std::string("Hallo, Ada!"));
    CHECK_EQ(catalog.translate(japanese, "greeting", Arguments().set("name", "Ada")),
             std::string("Hello, Ada!"));
    CHECK_EQ(catalog.translate(japanese, "missing.key"), std::string("missing.key"));
    CHECK_EQ(catalog.size(), static_cast<std::size_t>(4));
    CHECK(catalog.contains(german, "greeting"));
    CHECK_FALSE(catalog.contains(Locale("de-AT"), "greeting"));
    CHECK(catalog.find(Locale("de-AT"), "greeting") != nullptr);
    CHECK(catalog.find(Locale("de-AT"), "absent") == nullptr);

    // The fallback message is formatted with the locale that supplied it, so
    // English plural rules stay in effect for English text shown in Japan.
    CHECK_EQ(catalog.translate(japanese, "items", Arguments().set("count", 1)), std::string("1 item"));
    CHECK_EQ(catalog.translate(german, "items", Arguments().set("count", 1000)),
             std::string("1.000 Artikel"));

    CHECK_EQ(catalog.translate(german, "unknown", "Hi {name}", Arguments().set("name", "Bob")),
             std::string("Hi Bob"));

    // A regional source locale falls back to its language for source text.
    Catalog regionalSource("en-US");
    regionalSource.add(english, "hello", "Hello");
    CHECK_EQ(regionalSource.translate(french, "hello"), std::string("Hello"));

    std::vector<std::string> missing;
    catalog.setMissingHandler(
        [&missing](const Locale &, const std::string &key) { missing.push_back(key); });
    catalog.translate(japanese, "nope");
    CHECK_EQ(missing.size(), static_cast<std::size_t>(1));
    CHECK_EQ(missing[0], std::string("nope"));
}

TEST_CASE("localisation: Chinese scripts and English variants")
{
    Catalog catalog("en");
    catalog.add(Locale("zh-Hans"), "script", "Hans");
    catalog.add(Locale("zh-Hant"), "script", "Hant");
    catalog.add(Locale("yue-Hant"), "cantonese", "Yue");
    catalog.add(Locale("en"), "colour", "colour");
    catalog.add(Locale("en-GB"), "colour", "colour (UK)");
    catalog.add(Locale("en-US"), "colour", "color (US)");

    // Simplified Chinese regions fall back to zh-Hans, Traditional regions to
    // zh-Hant, including Traditional Chinese in mainland China.
    CHECK_EQ(catalog.translate(Locale("zh-CN"), "script"), std::string("Hans"));
    CHECK_EQ(catalog.translate(Locale("zh-SG"), "script"), std::string("Hans"));
    CHECK_EQ(catalog.translate(Locale("zh-Hant-CN"), "script"), std::string("Hant"));
    CHECK_EQ(catalog.translate(Locale("zh-TW"), "script"), std::string("Hant"));
    CHECK_EQ(catalog.translate(Locale("zh-HK"), "script"), std::string("Hant"));
    CHECK_EQ(catalog.translate(Locale("zh-MO"), "script"), std::string("Hant"));

    // Cantonese (Guangdong/Hong Kong/Macao) is served by yue-Hant.
    CHECK_EQ(catalog.translate(Locale("yue-HK"), "cantonese"), std::string("Yue"));
    CHECK_EQ(catalog.translate(Locale("yue-CN"), "cantonese"), std::string("Yue"));
    CHECK_EQ(catalog.translate(Locale("zh-yue-MO"), "cantonese"), std::string("Yue"));

    // British and American English are distinct catalogue entries, while
    // en-AU falls back to the generic English source text.
    CHECK_EQ(catalog.translate(Locale("en-GB"), "colour"), std::string("colour (UK)"));
    CHECK_EQ(catalog.translate(Locale("en-US"), "colour"), std::string("color (US)"));
    CHECK_EQ(catalog.translate(Locale("en-AU"), "colour"), std::string("colour"));
}

TEST_CASE("localisation: catalogue contexts")
{
    Catalog catalog("en");
    catalog.add(english, "menu", "Open", "&Open");
    catalog.add(english, "dialog", "Open", "Open file");
    catalog.add(english, "Open", "&Open");

    CHECK_EQ(catalog.translateIn(english, "menu", "Open"), std::string("&Open"));
    CHECK_EQ(catalog.translateIn(english, "dialog", "Open"), std::string("Open file"));
    CHECK_EQ(catalog.translate(english, "Open"), std::string("&Open"));
    CHECK(catalog.contains(english, "menu", "Open"));
    CHECK_FALSE(catalog.contains(english, "absent", "Open"));
}

TEST_CASE("localisation: properties catalogue loading")
{
    Catalog catalog("en");
    const std::string properties =
        "\xEF\xBB\xBF# comment\n"
        "greeting = Bonjour, {name} !\n"
        "farewell: Au revoir\n"
        "escaped = a\\tb\\nc\\u00e9\n"
        "continued = one\\\n"
        "    two\n"
        "Open[menu] = &Ouvrir\n"
        "emoji = \\uD83D\\uDE00\n"
        "bare\n"
        "! bang comment\n"
        "\n";

    std::istringstream stream(properties);
    CHECK_EQ(catalog.load(french, stream), static_cast<std::size_t>(7));
    CHECK_EQ(catalog.translate(french, "greeting", Arguments().set("name", "Ada")),
             std::string("Bonjour, Ada !"));
    CHECK_EQ(catalog.translate(french, "farewell"), std::string("Au revoir"));
    CHECK_EQ(catalog.translate(french, "escaped"), std::string("a\tb\nc\xC3\xA9"));
    CHECK_EQ(catalog.translate(french, "continued"), std::string("onetwo"));
    CHECK_EQ(catalog.translate(french, "emoji"), std::string("\xF0\x9F\x98\x80"));
    CHECK_EQ(catalog.translate(french, "bare"), std::string());
    CHECK_EQ(catalog.translateIn(french, "menu", "Open"), std::string("&Ouvrir"));
}

TEST_CASE("localisation: plural rule data for every supported language")
{
    // Compiling every rule set catches malformed CLDR expressions at test
    // time instead of on the first request in a rarely used language.
    const char *languages[] = {"en", "de", "nl", "sv", "fi", "et", "sw", "af", "da", "nb", "nn", "is",
                               "fr", "pt", "es", "it", "ca", "ru", "uk", "be", "pl", "cs", "sk", "lt",
                               "lv", "ga", "cy", "ar", "he", "hi", "bn", "mr", "gu", "kn", "pa", "fa",
                               "ta", "te", "ml", "ne", "si", "ur", "ps", "tr", "hu", "el", "ka", "kk",
                               "uz", "az", "mn", "sq", "bg", "hy", "ro", "sr", "hr", "bs", "sl", "mk",
                               "ja", "zh", "ko", "vi", "id", "ms", "th", "my", "km", "lo", "fil", "bo",
                               "dz", "yue", "ug", "sd", "ckb"};

    for (const char *language : languages)
    {
        const Locale locale(language);
        for (int count = 0; count <= 25; ++count)
        {
            const PluralCategory cardinal = localisation::pluralCategory(locale, count);
            const PluralCategory ordinal = localisation::ordinalCategory(locale, count);
            CHECK(cardinal == PluralCategory::zero || cardinal == PluralCategory::one ||
                  cardinal == PluralCategory::two || cardinal == PluralCategory::few ||
                  cardinal == PluralCategory::many || cardinal == PluralCategory::other);
            CHECK(ordinal == PluralCategory::zero || ordinal == PluralCategory::one ||
                  ordinal == PluralCategory::two || ordinal == PluralCategory::few ||
                  ordinal == PluralCategory::many || ordinal == PluralCategory::other);
        }
    }
}

TEST_CASE("localisation: text direction")
{
    CHECK(localisation::direction(english) == TextDirection::leftToRight);
    CHECK(localisation::direction(japanese) == TextDirection::leftToRight);
    CHECK(localisation::direction(Locale("ar")) == TextDirection::rightToLeft);
    CHECK(localisation::direction(Locale("he")) == TextDirection::rightToLeft);
    CHECK(localisation::direction(Locale("fa")) == TextDirection::rightToLeft);
    CHECK(localisation::direction(Locale("ur")) == TextDirection::rightToLeft);
    CHECK(localisation::direction(Locale("az")) == TextDirection::leftToRight);
    CHECK(localisation::direction(Locale("az-Arab")) == TextDirection::rightToLeft);
    CHECK(localisation::direction(Locale("ar-Latn")) == TextDirection::leftToRight);

    // Traditional Mongolian is written top-to-bottom; Cyrillic Mongolian is not.
    CHECK(localisation::direction(Locale("mn-MN")) == TextDirection::leftToRight);
    CHECK(localisation::direction(Locale("mn")) == TextDirection::leftToRight);
    CHECK(localisation::direction(Locale("mn-CN")) == TextDirection::topToBottom);
    CHECK(localisation::direction(Locale("mn-Mong")) == TextDirection::topToBottom);

    CHECK(localisation::direction(Locale("bo")) == TextDirection::leftToRight);
    CHECK(localisation::direction(Locale("dz")) == TextDirection::leftToRight);
    CHECK(localisation::direction(Locale("zh-Hant-TW")) == TextDirection::leftToRight);
    CHECK(localisation::direction(Locale("ar-EG")) == TextDirection::rightToLeft);

    CHECK(localisation::isRightToLeft(Locale("he")));
    CHECK_FALSE(localisation::isRightToLeft(Locale("mn-CN")));
}
