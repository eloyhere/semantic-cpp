#include "localisation/po.h"

#include "test_framework.h"

#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
using localisation::Arguments;
using localisation::Catalog;
using localisation::Locale;
using localisation::PoEntry;

const Locale english("en");
const Locale german("de");
const Locale french("fr");
const Locale russian("ru");
const Locale arabic("ar");
const Locale chinese("zh");

std::string translate(const Catalog &catalog, const Locale &locale, const std::string &key, long long count)
{
    return catalog.translate(locale, key, Arguments().set("count", count));
}
} // namespace

TEST_CASE("po: parses header, comments, contexts and fuzzy flags")
{
    std::istringstream input(R"PO(# translator comment
#. extracted comment
#: src/app.cpp:10
#| msgid "Previous"
msgid ""
msgstr ""
"Project-Id-Version: semantic-cpp demo\n"
"Language: de\n"
"Plural-Forms: nplurals=2; plural=(n != 1);\n"

msgid "Hello"
msgstr "Hallo"

msgctxt "menu"
msgid "Open"
msgstr "Öffnen"

#, fuzzy, c-format
msgid "Save"
msgstr "Speichern"

#~ msgid "Obsolete"
#~ msgstr "Veraltet"
)PO");

    const std::vector<PoEntry> entries = localisation::parsePo(input);
    CHECK_EQ(entries.size(), static_cast<std::size_t>(4));

    // The header entry is returned verbatim; loadPo is the one that skips it.
    CHECK(entries[0].id.empty());
    CHECK(entries[0].context.empty());
    CHECK_EQ(entries[0].forms.size(), static_cast<std::size_t>(1));
    CHECK(entries[0].forms[0].find("Plural-Forms: nplurals=2") != std::string::npos);
    CHECK_FALSE(entries[0].fuzzy);

    CHECK_EQ(entries[1].id, std::string("Hello"));
    CHECK_EQ(entries[1].forms[0], std::string("Hallo"));
    CHECK_FALSE(entries[1].fuzzy);
    CHECK_FALSE(entries[1].plural);

    CHECK_EQ(entries[2].context, std::string("menu"));
    CHECK_EQ(entries[2].id, std::string("Open"));
    CHECK_EQ(entries[2].forms[0], std::string("Öffnen"));

    CHECK(entries[3].fuzzy);
    CHECK_EQ(entries[3].id, std::string("Save"));
    CHECK_EQ(entries[3].forms[0], std::string("Speichern"));
}

TEST_CASE("po: multi-line strings and escape sequences")
{
    std::istringstream input(R"PO(msgid "escapes"
msgstr ""
"line one\n"
"line two\twith tab\r\n"
"quote \" backslash \\ bell \a hex \x41 octal \101"
" and UTF-8: 日本語"
)PO");

    const std::vector<PoEntry> entries = localisation::parsePo(input);
    CHECK_EQ(entries.size(), static_cast<std::size_t>(1));
    CHECK_EQ(entries[0].id, std::string("escapes"));

    const std::string expected = "line one\nline two\twith tab\r\nquote \" backslash \\ bell \a hex A octal A"
                                 " and UTF-8: 日本語";
    CHECK_EQ(entries[0].forms[0], expected);

    // The parsed text survives an ordinary catalogue round trip.
    std::istringstream loadInput(R"PO(msgid "escapes"
msgstr ""
"line one\n"
"line two\twith tab\r\n"
"quote \" backslash \\ bell \a hex \x41 octal \101"
" and UTF-8: 日本語"
)PO");
    Catalog catalog("en");
    CHECK_EQ(localisation::loadPo(catalog, english, loadInput), static_cast<std::size_t>(1));
    CHECK_EQ(catalog.translate(english, "escapes"), expected);
}

TEST_CASE("po: loadPo resolves singular messages with context and fallback")
{
    std::istringstream input(R"PO(msgid ""
msgstr ""
"Language: de\n"
"Content-Type: text/plain; charset=UTF-8\n"

#: src/greeting.cpp:12
msgid "Hello"
msgstr "Hallo"

msgctxt "menu"
msgid "Open"
msgstr "Öffnen"

msgid "Welcome"
msgstr "Willkommen, {name}!"
)PO");

    Catalog catalog("en");
    CHECK_EQ(localisation::loadPo(catalog, german, input), static_cast<std::size_t>(3));
    CHECK_EQ(catalog.size(), static_cast<std::size_t>(3));

    CHECK_EQ(catalog.translate(german, "Hello"), std::string("Hallo"));
    CHECK_EQ(catalog.translateIn(german, "menu", "Open"), std::string("Öffnen"));
    CHECK_EQ(catalog.translate(german, "Welcome", Arguments().set("name", "Ada")),
             std::string("Willkommen, Ada!"));

    // A missing key falls through to the source language and then the key.
    CHECK_EQ(catalog.translate(german, "Missing"), std::string("Missing"));
    CHECK_FALSE(catalog.contains(german, "Missing"));
    CHECK(catalog.contains(german, "Hello"));
    CHECK(catalog.contains(german, "menu", "Open"));
}

TEST_CASE("po: loadPo skips fuzzy and untranslated entries")
{
    std::istringstream input(R"PO(msgid ""
msgstr ""
"Plural-Forms: nplurals=2; plural=(n != 1);\n"

#, fuzzy
msgid "Save"
msgstr "Speichern"

msgid "Close"
msgstr ""

msgid "Quit"
msgstr "Beenden"
)PO");

    Catalog catalog("en");
    CHECK_EQ(localisation::loadPo(catalog, german, input), static_cast<std::size_t>(1));
    CHECK_EQ(catalog.size(), static_cast<std::size_t>(1));
    CHECK_EQ(catalog.translate(german, "Save"), std::string("Save"));
    CHECK_EQ(catalog.translate(german, "Close"), std::string("Close"));
    CHECK_EQ(catalog.translate(german, "Quit"), std::string("Beenden"));
}

TEST_CASE("po: English plural forms")
{
    const std::string po = R"PO(msgid ""
msgstr ""
"Plural-Forms: nplurals=2; plural=(n != 1);\n"

msgid "%d file"
msgid_plural "%d files"
msgstr[0] "%d file"
msgstr[1] "%d files"

msgctxt "cart"
msgid "%d item"
msgid_plural "%d items"
msgstr[0] "%d item"
msgstr[1] "%d items"
)PO";

    std::istringstream parseInput(po);
    const std::vector<PoEntry> entries = localisation::parsePo(parseInput);
    CHECK_EQ(entries.size(), static_cast<std::size_t>(3));
    CHECK(entries[1].plural);
    CHECK_EQ(entries[1].id, std::string("%d file"));
    CHECK_EQ(entries[1].forms.size(), static_cast<std::size_t>(2));
    CHECK_EQ(entries[1].forms[0], std::string("%d file"));
    CHECK_EQ(entries[1].forms[1], std::string("%d files"));

    Catalog catalog("en");
    std::istringstream loadInput(po);
    CHECK_EQ(localisation::loadPo(catalog, english, loadInput), static_cast<std::size_t>(2));

    CHECK_EQ(translate(catalog, english, "%d file", 0), std::string("0 files"));
    CHECK_EQ(translate(catalog, english, "%d file", 1), std::string("1 file"));
    CHECK_EQ(translate(catalog, english, "%d file", 2), std::string("2 files"));
    CHECK_EQ(translate(catalog, english, "%d file", 5), std::string("5 files"));
    CHECK_EQ(translate(catalog, english, "%d file", 21), std::string("21 files"));
    CHECK_EQ(translate(catalog, english, "%d file", 1000), std::string("1,000 files"));

    CHECK_EQ(catalog.translateIn(english, "cart", "%d item", Arguments().set("count", 1)),
             std::string("1 item"));
    CHECK_EQ(catalog.translateIn(english, "cart", "%d item", Arguments().set("count", 3)),
             std::string("3 items"));
}

TEST_CASE("po: Russian plural forms")
{
    const std::string po = R"PO(msgid ""
msgstr ""
"Plural-Forms: nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);\n"

msgid "%d file"
msgid_plural "%d files"
msgstr[0] "%d файл"
msgstr[1] "%d файла"
msgstr[2] "%d файлов"
)PO";

    Catalog catalog("en");
    std::istringstream input(po);
    CHECK_EQ(localisation::loadPo(catalog, russian, input), static_cast<std::size_t>(1));

    // one -> msgstr[0], few -> msgstr[1], many -> msgstr[2].
    CHECK_EQ(translate(catalog, russian, "%d file", 0), std::string("0 файлов"));
    CHECK_EQ(translate(catalog, russian, "%d file", 1), std::string("1 файл"));
    CHECK_EQ(translate(catalog, russian, "%d file", 2), std::string("2 файла"));
    CHECK_EQ(translate(catalog, russian, "%d file", 5), std::string("5 файлов"));
    CHECK_EQ(translate(catalog, russian, "%d file", 11), std::string("11 файлов"));
    CHECK_EQ(translate(catalog, russian, "%d file", 21), std::string("21 файл"));
    CHECK_EQ(translate(catalog, russian, "%d file", 22), std::string("22 файла"));
    CHECK_EQ(translate(catalog, russian, "%d file", 25), std::string("25 файлов"));
    CHECK_EQ(translate(catalog, russian, "%d file", 101), std::string("101 файл"));
    CHECK_EQ(translate(catalog, russian, "%d file", 111), std::string("111 файлов"));
}

TEST_CASE("po: Arabic plural categories")
{
    const std::string po = R"PO(msgid ""
msgstr ""
"Plural-Forms: nplurals=6; plural=(n==0 ? 0 : n==1 ? 1 : n==2 ? 2 : n%100>=3 && n%100<=10 ? 3 : n%100>=11 ? 4 : 5);\n"

msgid "%d file"
msgid_plural "%d files"
msgstr[0] "zero:%d"
msgstr[1] "one:%d"
msgstr[2] "two:%d"
msgstr[3] "few:%d"
msgstr[4] "many:%d"
msgstr[5] "other:%d"
)PO";

    Catalog catalog("en");
    std::istringstream input(po);
    CHECK_EQ(localisation::loadPo(catalog, arabic, input), static_cast<std::size_t>(1));

    // Arabic formats numbers with Arabic-Indic digits, so ICU's '#' prints
    // the count in native digits.
    CHECK_EQ(translate(catalog, arabic, "%d file", 0), std::string("zero:٠"));
    CHECK_EQ(translate(catalog, arabic, "%d file", 1), std::string("one:١"));
    CHECK_EQ(translate(catalog, arabic, "%d file", 2), std::string("two:٢"));
    CHECK_EQ(translate(catalog, arabic, "%d file", 3), std::string("few:٣"));
    CHECK_EQ(translate(catalog, arabic, "%d file", 5), std::string("few:٥"));
    CHECK_EQ(translate(catalog, arabic, "%d file", 11), std::string("many:١١"));
    CHECK_EQ(translate(catalog, arabic, "%d file", 21), std::string("many:٢١"));
    CHECK_EQ(translate(catalog, arabic, "%d file", 100), std::string("other:١٠٠"));
}

TEST_CASE("po: plural defaults without a Plural-Forms header")
{
    const std::string po = R"PO(msgid "%d apple"
msgid_plural "%d apples"
msgstr[0] "%d apple"
msgstr[1] "%d apples"
)PO";

    Catalog catalog("en");
    std::istringstream input(po);
    CHECK_EQ(localisation::loadPo(catalog, english, input), static_cast<std::size_t>(1));

    CHECK_EQ(translate(catalog, english, "%d apple", 0), std::string("0 apples"));
    CHECK_EQ(translate(catalog, english, "%d apple", 1), std::string("1 apple"));
    CHECK_EQ(translate(catalog, english, "%d apple", 2), std::string("2 apples"));
    CHECK_EQ(translate(catalog, english, "%d apple", 21), std::string("21 apples"));
}

TEST_CASE("po: nplurals=1 catalogue")
{
    const std::string po = R"PO(msgid ""
msgstr ""
"Plural-Forms: nplurals=1; plural=0;\n"

msgid "%d file"
msgid_plural "%d files"
msgstr[0] "%d 个文件"
)PO";

    std::istringstream parseInput(po);
    const std::vector<PoEntry> entries = localisation::parsePo(parseInput);
    CHECK_EQ(entries.size(), static_cast<std::size_t>(2));
    CHECK(entries[1].plural);
    CHECK_EQ(entries[1].forms.size(), static_cast<std::size_t>(1));

    Catalog catalog("en");
    std::istringstream loadInput(po);
    CHECK_EQ(localisation::loadPo(catalog, chinese, loadInput), static_cast<std::size_t>(1));
    CHECK_EQ(translate(catalog, chinese, "%d file", 0), std::string("0 个文件"));
    CHECK_EQ(translate(catalog, chinese, "%d file", 1), std::string("1 个文件"));
    CHECK_EQ(translate(catalog, chinese, "%d file", 5), std::string("5 个文件"));
}

TEST_CASE("po: printf count placeholders become ICU pound signs")
{
    const std::string po = R"PO(msgid ""
msgstr ""
"Plural-Forms: nplurals=2; plural=(n != 1);\n"

msgid "%d percent"
msgid_plural "%d percents"
msgstr[0] "one: %d%%"
msgstr[1] "many: %1$d%%"
)PO";

    Catalog catalog("en");
    std::istringstream input(po);
    CHECK_EQ(localisation::loadPo(catalog, english, input), static_cast<std::size_t>(1));

    CHECK_EQ(translate(catalog, english, "%d percent", 1), std::string("one: 1%"));
    CHECK_EQ(translate(catalog, english, "%d percent", 5), std::string("many: 5%"));
}

TEST_CASE("po: plural categories outside the sampled range fall back to other")
{
    const std::string po = R"PO(msgid ""
msgstr ""
"Plural-Forms: nplurals=2; plural=(n != 1);\n"

msgid "%d item"
msgid_plural "%d items"
msgstr[0] "%d item"
msgstr[1] "%d items"
)PO";

    Catalog catalog("en");
    std::istringstream input(po);
    CHECK_EQ(localisation::loadPo(catalog, french, input), static_cast<std::size_t>(1));

    CHECK_EQ(translate(catalog, french, "%d item", 5), std::string("5 items"));

    // French only reaches the CLDR "many" category at multiples of one
    // million, which the n = 0..200 sampling never sees.  The generated
    // pattern has no "many" branch, so ICU falls back to "other".  French
    // groups digits with U+202F (narrow no-break space).
    const std::string million = "1\xE2\x80\xAF" "000\xE2\x80\xAF" "000";
    CHECK_EQ(translate(catalog, french, "%d item", 1000000), million + " items");
}

TEST_CASE("po: literal hash and percent in plural forms stay literal")
{
    const std::string po = R"PO(msgid ""
msgstr ""
"Plural-Forms: nplurals=2; plural=(n != 1);\n"

msgid "%d file #"
msgid_plural "%d files #"
msgstr[0] "%d file #%"
msgstr[1] "%d files #%"
)PO";

    Catalog catalog("en");
    std::istringstream input(po);
    CHECK_EQ(localisation::loadPo(catalog, english, input), static_cast<std::size_t>(1));

    // The '%d' becomes the count, the literal '#' is quoted for ICU and the
    // lone '%' stays a percent sign.
    CHECK_EQ(translate(catalog, english, "%d file #", 1), std::string("1 file #%"));
    CHECK_EQ(translate(catalog, english, "%d file #", 5), std::string("5 files #%"));
}

TEST_CASE("po: a broken Plural-Forms expression falls back to the gettext default")
{
    const std::string po = R"PO(msgid ""
msgstr ""
"Plural-Forms: nplurals=2; plural=(n ;\n"

msgid "%d file"
msgid_plural "%d files"
msgstr[0] "%d file"
msgstr[1] "%d files"
)PO";

    Catalog catalog("en");
    std::istringstream input(po);
    CHECK_EQ(localisation::loadPo(catalog, english, input), static_cast<std::size_t>(1));
    CHECK_EQ(translate(catalog, english, "%d file", 1), std::string("1 file"));
    CHECK_EQ(translate(catalog, english, "%d file", 2), std::string("2 files"));
}

TEST_CASE("po: singular translations that are not valid ICU patterns load as literals")
{
    std::istringstream input(R"PO(msgid "Left brace"
msgstr "a { b"

msgid "Apostrophe"
msgstr "l'été"

msgid "Brace and apostrophe"
msgstr "d'accord {x}"
)PO");

    Catalog catalog("en");
    CHECK_EQ(localisation::loadPo(catalog, french, input), static_cast<std::size_t>(3));
    CHECK_EQ(catalog.translate(french, "Left brace"), std::string("a { b"));
    CHECK_EQ(catalog.translate(french, "Apostrophe"), std::string("l'été"));
    CHECK_EQ(catalog.translate(french, "Brace and apostrophe"), std::string("d'accord {x}"));
}

TEST_CASE("po: malformed input is rejected with std::invalid_argument")
{
    std::istringstream missingMsgstr(R"PO(msgid "lonely"
)PO");
    CHECK_THROWS_AS(localisation::parsePo(missingMsgstr), std::invalid_argument);

    std::istringstream badIndex(R"PO(msgid "x"
msgstr[x] "y"
)PO");
    CHECK_THROWS_AS(localisation::parsePo(badIndex), std::invalid_argument);

    std::istringstream negativeIndex(R"PO(msgid "x"
msgstr[-1] "y"
)PO");
    CHECK_THROWS_AS(localisation::parsePo(negativeIndex), std::invalid_argument);

    std::istringstream oversizedIndex(R"PO(msgid "x"
msgstr[256] "y"
)PO");
    CHECK_THROWS_AS(localisation::parsePo(oversizedIndex), std::invalid_argument);

    std::istringstream unterminated(R"PO(msgid "x
msgstr "y"
)PO");
    CHECK_THROWS_AS(localisation::parsePo(unterminated), std::invalid_argument);

    std::istringstream unknownKeyword(R"PO(msgid "x"
msgstr "y"
msgfoo "z"
)PO");
    CHECK_THROWS_AS(localisation::parsePo(unknownKeyword), std::invalid_argument);

    std::istringstream duplicateMsgid(R"PO(msgid "x"
msgstr "y"
msgid "z"
msgstr "w"
)PO");
    CHECK_THROWS_AS(localisation::parsePo(duplicateMsgid), std::invalid_argument);

    // loadPo uses the same parser, so it reports the same error.
    Catalog catalog("en");
    std::istringstream loadInput(R"PO(msgid "lonely"
)PO");
    CHECK_THROWS_AS(localisation::loadPo(catalog, english, loadInput), std::invalid_argument);
}

TEST_CASE("po: UTF-8 BOM and CRLF line endings")
{
    const std::string text = "\xEF\xBB\xBFmsgid \"Hello\"\r\nmsgstr \"Hallo\"\r\n";
    std::istringstream input(text);
    const std::vector<PoEntry> entries = localisation::parsePo(input);
    CHECK_EQ(entries.size(), static_cast<std::size_t>(1));
    CHECK_EQ(entries[0].id, std::string("Hello"));
    CHECK_EQ(entries[0].forms[0], std::string("Hallo"));
}

TEST_CASE("po: empty input loads nothing")
{
    std::istringstream parseInput;
    CHECK(localisation::parsePo(parseInput).empty());

    Catalog catalog("en");
    std::istringstream loadInput;
    CHECK_EQ(localisation::loadPo(catalog, english, loadInput), static_cast<std::size_t>(0));
    CHECK(catalog.empty());
}
