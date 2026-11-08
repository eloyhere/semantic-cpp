#include "localisation/collation.h"

#include "test_framework.h"

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <random>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
using localisation::CollationStrength;
using localisation::Collator;
using localisation::Locale;

const Collator root;
const Collator swedish(Locale("sv"));

// Checks one complete sort key.  Keeping the initializer list in a function
// argument avoids commas inside the CHECK macros.
auto checkKey(const Collator &collator, std::string_view text, CollationStrength strength,
              std::initializer_list<std::uint32_t> expected) -> void
{
    CHECK(collator.sortKey(text, strength) == std::vector<std::uint32_t>(expected));
}

auto checkOrder(const Collator &collator, std::initializer_list<std::string_view> ordered) -> void
{
    const std::vector<std::string_view> values(ordered);
    for (std::size_t index = 0; index + 1 < values.size(); ++index)
    {
        CHECK(collator.less(values[index], values[index + 1]));
        CHECK(collator.compare(values[index], values[index + 1]) < 0);
        CHECK(collator.compare(values[index + 1], values[index]) > 0);
        CHECK_FALSE(collator.equal(values[index], values[index + 1]));
    }
}

auto checkEquivalent(const Collator &collator, std::string_view left, std::string_view right) -> void
{
    CHECK(collator.equal(left, right));
    CHECK(collator.compare(left, right) == 0);
    CHECK_EQ(collator.sortKey(left), collator.sortKey(right));
}
} // namespace

// ---------------------------------------------------------------------------
// Weights and levels
// ---------------------------------------------------------------------------

TEST_CASE("collation: exact sort keys at each strength")
{
    // "a" is [.20B3.0020.0002] in the DUCET; each level is written out with
    // the 0 separator that sortKey() inserts.
    checkKey(root, "a", CollationStrength::primary, {0x20B3, 0});
    checkKey(root, "a", CollationStrength::secondary, {0x20B3, 0, 0x20, 0});
    checkKey(root, "a", CollationStrength::tertiary, {0x20B3, 0, 0x20, 0, 0x2, 0});

    // Uppercase differs from lowercase only in the tertiary weight.
    checkKey(root, "A", CollationStrength::primary, {0x20B3, 0});
    checkKey(root, "A", CollationStrength::tertiary, {0x20B3, 0, 0x20, 0, 0x8, 0});

    // "e-acute" is e [.211A.0020.0002] plus U+0301 [.0000.0024.0002].
    checkKey(root, u8"\u00E9", CollationStrength::tertiary,
             {0x211A, 0, 0x20, 0x24, 0, 0x2, 0x2, 0});

    // The empty string has one separator per level and nothing else.
    checkKey(root, "", CollationStrength::primary, {0});
    checkKey(root, "", CollationStrength::tertiary, {0, 0, 0});
}

TEST_CASE("collation: primary order")
{
    checkOrder(root, {"a", "b", "c", "d", "e", "f", "z"});
    checkOrder(root, {"0", "1", "2", "9", "a"});
    checkOrder(root, {"", "a", "ab", "abc"});
    checkOrder(root, {"role", "roles", "rule"});
    checkOrder(root, {"A", "b", "B", "z"});
    checkOrder(root, {u8"\u00E5", "z"}); // a-ring is a + ring at root, so before z

    CHECK(root.equal("a", "A", CollationStrength::primary));
    CHECK(root.equal("e", u8"\u00E9", CollationStrength::primary));
    CHECK(root.equal("l", u8"l\u00B7", CollationStrength::primary));
    CHECK_FALSE(root.equal("a", "b", CollationStrength::primary));
}

TEST_CASE("collation: secondary order")
{
    // Accent order follows the DUCET secondary weights: acute 0x24, grave
    // 0x25, circumflex 0x27, diaeresis 0x2B.
    checkOrder(root, {"e", u8"\u00E9", u8"\u00E8", u8"\u00EA", u8"\u00EB", "f"});
    // UTS #10 section 1.1: cote < cot-e-acute < cot-o-circumflex <
    // cot-o-circumflex-e-acute with forward accents.
    checkOrder(root, {"cote", u8"cot\u00E9", u8"c\u00F4te", u8"c\u00F4t\u00E9"});
    // At root, a-ring/a-diaeresis sort as accented a; ring (0x29) precedes
    // diaeresis (0x2B).
    checkOrder(root, {"a", u8"\u00E5", u8"\u00E4", "b"});

    CHECK(root.equal("e", u8"\u00E9", CollationStrength::primary));
    CHECK_FALSE(root.equal("e", u8"\u00E9", CollationStrength::secondary));
    CHECK(root.less("e", u8"\u00E9", CollationStrength::secondary));
    CHECK(root.equal("a", u8"\u00E4", CollationStrength::primary));
}

TEST_CASE("collation: tertiary order")
{
    checkOrder(root, {"a", "A", "b", "B", "z", "Z"});
    checkOrder(root, {"role", "Role", u8"r\u00F4le"});
    // U+00A0 has the same primary and secondary as the ASCII space but a
    // different tertiary weight (0x1B versus 0x2).
    checkOrder(root, {" ", u8"\u00A0"});

    CHECK(root.equal("a", "A", CollationStrength::secondary));
    CHECK_FALSE(root.equal("a", "A", CollationStrength::tertiary));
    CHECK(root.equal(" ", u8"\u00A0", CollationStrength::secondary));
    CHECK_FALSE(root.equal(" ", u8"\u00A0", CollationStrength::tertiary));
    CHECK(root.less("a", "A", CollationStrength::tertiary));
}

// ---------------------------------------------------------------------------
// sortKey / compare agreement
// ---------------------------------------------------------------------------

TEST_CASE("collation: sortKey agrees with compare")
{
    const std::vector<std::string> corpus = {
        "",
        "a",
        "A",
        "ab",
        "abc",
        "b",
        "z",
        u8"\u00E5",
        u8"\u00E4",
        u8"\u00F6",
        u8"\u00E9",
        u8"\u00E8",
        u8"cote",
        u8"cot\u00E9",
        u8"c\u00F4te",
        u8"l",
        u8"l\u00B7",
        u8"m",
        u8"\u4E00",
        u8"\u3400",
        u8"\u0378",
        u8"\uFFFD",
        u8"\u0CC6\u0CC2",
        u8"\u0CC6\u0CC2\u0CD5",
        u8"a\u030A",
        u8"a\u0308",
        std::string("a\0b", 3),
        u8"a\u00ADb",
        u8"\uAC00",
    };
    const CollationStrength strengths[] = {CollationStrength::primary, CollationStrength::secondary,
                                           CollationStrength::tertiary};
    for (const std::string &left : corpus)
    {
        for (const std::string &right : corpus)
        {
            for (CollationStrength strength : strengths)
            {
                const std::vector<std::uint32_t> leftKey = root.sortKey(left, strength);
                const std::vector<std::uint32_t> rightKey = root.sortKey(right, strength);
                const int comparison = root.compare(left, right, strength);
                CHECK_EQ(comparison < 0, leftKey < rightKey);
                CHECK_EQ(comparison == 0, leftKey == rightKey);
                CHECK_EQ(comparison > 0, leftKey > rightKey);
                CHECK_EQ(comparison == 0, root.equal(left, right, strength));
                CHECK_EQ(comparison < 0, root.less(left, right, strength));
            }
        }
    }
}

TEST_CASE("collation: random strings agree with sort keys")
{
    std::mt19937 generator(20241108u);
    const std::vector<std::string> pool = {
        "a",  "e",  "z",  "A",  "Z",  " ",  "-",  u8"\u00E9", u8"\u00E5", u8"\u00E4",
        u8"\u00F6", u8"\u0301", u8"\u0308", u8"\u030A", u8"\u0323", u8"l\u00B7",
        u8"\u4E00", u8"\u20000", u8"\u17000", u8"\uAC00", u8"\u0CC6\u0CC2\u0CD5",
        u8"\u00AD", u8"\uFFFD", u8"\u0378",
    };
    std::uniform_int_distribution<std::size_t> lengthDistribution(1, 5);
    std::uniform_int_distribution<std::size_t> poolDistribution(0, pool.size() - 1);

    std::vector<std::string> values;
    values.reserve(48);
    for (int sample = 0; sample < 48; ++sample)
    {
        std::string text;
        const std::size_t length = lengthDistribution(generator);
        for (std::size_t index = 0; index < length; ++index)
        {
            text += pool[poolDistribution(generator)];
        }
        values.push_back(std::move(text));
    }

    const CollationStrength strengths[] = {CollationStrength::primary, CollationStrength::secondary,
                                           CollationStrength::tertiary};
    for (const std::string &left : values)
    {
        for (const std::string &right : values)
        {
            for (CollationStrength strength : strengths)
            {
                const int comparison = root.compare(left, right, strength);
                const std::vector<std::uint32_t> leftKey = root.sortKey(left, strength);
                const std::vector<std::uint32_t> rightKey = root.sortKey(right, strength);
                CHECK_EQ(comparison < 0, leftKey < rightKey);
                CHECK_EQ(comparison == 0, leftKey == rightKey);
                CHECK_EQ(comparison > 0, leftKey > rightKey);
            }
        }
    }

    // A comparison-based sort must also agree with the sort keys.
    std::vector<std::string> sorted = values;
    std::sort(sorted.begin(), sorted.end(),
              [](const std::string &left, const std::string &right) { return root.less(left, right); });
    for (std::size_t index = 0; index + 1 < sorted.size(); ++index)
    {
        CHECK(root.compare(sorted[index], sorted[index + 1]) <= 0);
    }
}

// ---------------------------------------------------------------------------
// Normalisation
// ---------------------------------------------------------------------------

TEST_CASE("collation: canonical equivalence")
{
    const std::pair<std::string_view, std::string_view> equivalents[] = {
        {u8"\u00E5", u8"a\u030A"},
        {u8"\u00C5", u8"A\u030A"},
        {u8"\u212B", u8"A\u030A"}, // ANGSTROM SIGN
        {u8"\u00E9", u8"e\u0301"},
        {u8"\u00E8", u8"e\u0300"},
        {u8"\u00FC", u8"u\u0308"},
        {u8"\u00C7", u8"C\u0327"},
        {u8"\u1E0A", u8"D\u0307"},
        {u8"\u1EF1", u8"u\u031B\u0323"},
        {u8"\u1EF1", u8"u\u0323\u031B"},
        {u8"\u01D5", u8"U\u0308\u0304"},
        {u8"\uAC00", u8"\u1100\u1161"},
        {u8"\uAC01", u8"\u1100\u1161\u11A8"},
        {u8"\uD7A3", u8"\u1112\u1175\u11C2"},
    };
    for (const auto &pair : equivalents)
    {
        checkEquivalent(root, pair.first, pair.second);
        CHECK(root.equal(pair.first, pair.second, CollationStrength::primary));
        CHECK(root.equal(pair.first, pair.second, CollationStrength::secondary));
        CHECK_EQ(root.sortKey(pair.first, CollationStrength::primary),
                 root.sortKey(pair.second, CollationStrength::primary));
    }

    // Canonical equivalence also holds when the characters are embedded.
    checkEquivalent(root, u8"x\u00E5y", u8"xa\u030Ay");
    checkEquivalent(root, u8"x\u212By", u8"xA\u030Ay");

    // These two are also canonically equivalent: canonical ordering puts the
    // horn (ccc 216) before the dot below (ccc 220).
    checkEquivalent(root, u8"x\u031B\u0323", u8"x\u0323\u031B");
}

// ---------------------------------------------------------------------------
// Contractions
// ---------------------------------------------------------------------------

TEST_CASE("collation: contractions and longest match")
{
    // l + middle dot is a DUCET contraction: [.21EF.0020.0002][.0000.011C.0002].
    checkKey(root, u8"l\u00B7", CollationStrength::primary, {0x21EF, 0});
    checkKey(root, u8"l\u00B7", CollationStrength::tertiary, {0x21EF, 0, 0x20, 0x11C, 0, 0x2, 0x2, 0});
    CHECK(root.equal("l", u8"l\u00B7", CollationStrength::primary));
    CHECK_FALSE(root.equal("l", u8"l\u00B7", CollationStrength::secondary));
    checkOrder(root, {"l", u8"l\u00B7", "m"});
    checkOrder(root, {u8"l\u00B7", u8"L\u00B7"}); // same primary/secondary, case at tertiary

    // Cyrillic short i is the contraction U+0438 U+0306; it is also the NFD
    // form of the precomposed U+0439.
    checkKey(root, u8"\u0438\u0306", CollationStrength::primary, {0x2525, 0});
    checkEquivalent(root, u8"\u0439", u8"\u0438\u0306");
    checkKey(root, u8"\u0439", CollationStrength::primary, {0x2525, 0});

    // U+0387 canonically decomposes to U+00B7, so this also exercises a
    // contraction reached through normalisation.
    checkEquivalent(root, u8"l\u0387", u8"l\u00B7");

    // The pair U+0CC6 U+0CC2 and the triple U+0CC6 U+0CC2 U+0CD5 coexist, so
    // the triple proves longest-match handling.
    checkKey(root, u8"\u0CC6\u0CC2", CollationStrength::primary, {0x2D58, 0});
    checkKey(root, u8"\u0CC6\u0CC2\u0CD5", CollationStrength::primary, {0x2D59, 0});
    checkKey(root, u8"\u0CC6\u0CC2\u0CD5", CollationStrength::tertiary, {0x2D59, 0, 0x20, 0, 0x2, 0});
    CHECK(root.less(u8"\u0CC6\u0CC2", u8"\u0CC6\u0CC2\u0CD5", CollationStrength::primary));

    // A third three-code-point contraction: Sinhala.
    checkKey(root, u8"\u0DD9\u0DCF\u0DCA", CollationStrength::primary, {0x2DF2, 0});
}

// ---------------------------------------------------------------------------
// Implicit weights
// ---------------------------------------------------------------------------

TEST_CASE("collation: implicit weights")
{
    // Core Han: lead unit 0xFB40, second unit (cp & 0x7FFF) | 0x8000.
    checkKey(root, u8"\u4E00", CollationStrength::tertiary,
             {0xFB40, 0xCE00, 0, 0x20, 0, 0x2, 0});
    // Extension A is "other Han" in UTS #10 Table 16: lead unit 0xFB80.
    checkKey(root, u8"\u3400", CollationStrength::tertiary,
             {0xFB80, 0xB400, 0, 0x20, 0, 0x2, 0});
    // Supplementary ideographs also use 0xFB80.
    checkKey(root, u8"\U00020000", CollationStrength::tertiary,
             {0xFB84, 0x8000, 0, 0x20, 0, 0x2, 0});
    checkKey(root, u8"\U0002A6DF", CollationStrength::tertiary,
             {0xFB85, 0xA6DF, 0, 0x20, 0, 0x2, 0});
    // Tangut: @implicitweights lead unit 0xFB00, offset 0x17000.
    checkKey(root, u8"\U00017000", CollationStrength::tertiary,
             {0xFB00, 0x8000, 0, 0x20, 0, 0x2, 0});
    checkKey(root, u8"\U00018D00", CollationStrength::tertiary,
             {0xFB00, 0x9D00, 0, 0x20, 0, 0x2, 0});
    // Unassigned code points use lead unit 0xFBC0 in UCA 15.0.0.
    checkKey(root, u8"\u0378", CollationStrength::tertiary,
             {0xFBC0, 0x8378, 0, 0x20, 0, 0x2, 0});
    checkKey(root, u8"\uFFFF", CollationStrength::tertiary,
             {0xFBC1, 0xFFFF, 0, 0x20, 0, 0x2, 0});

    checkOrder(root, {"z", u8"\u4E00", u8"\u4E01"});
    checkOrder(root, {u8"\u4E00", u8"\u3400", u8"\U00020000"});
    checkOrder(root, {u8"\U00017000", u8"\U00017001"});
    checkOrder(root, {u8"\U00017000", u8"\U00018D00"}); // Tangut Supplement offset
    checkOrder(root, {u8"\U00020000", u8"\u0378", u8"\uFFFD"});
    // The replacement character has an explicit DUCET weight, not an
    // implicit one.
    checkKey(root, u8"\uFFFD", CollationStrength::primary, {0xFFFD, 0});
}

// ---------------------------------------------------------------------------
// Tailoring
// ---------------------------------------------------------------------------

TEST_CASE("collation: Swedish tailoring")
{
    // a-ring, a-diaeresis, o-diaeresis sort after z; the tailored primaries
    // are 0xE000..0xE002.
    checkOrder(swedish, {"a", "z", u8"\u00E5", u8"\u00E4", u8"\u00F6"});
    checkKey(swedish, u8"\u00E5", CollationStrength::tertiary, {0xE000, 0, 0x20, 0, 0x2, 0});
    checkKey(swedish, u8"\u00C5", CollationStrength::tertiary, {0xE000, 0, 0x20, 0, 0x8, 0});
    checkKey(swedish, u8"\u00E4", CollationStrength::tertiary, {0xE001, 0, 0x20, 0, 0x2, 0});
    checkKey(swedish, u8"\u00F6", CollationStrength::tertiary, {0xE002, 0, 0x20, 0, 0x2, 0});

    // Root order is the opposite for z versus a-ring: a-ring sorts with a.
    CHECK(root.less(u8"\u00E5", "z"));
    CHECK(swedish.less("z", u8"\u00E5"));

    // Uppercase keeps the tailored primary and differs at the tertiary level.
    checkOrder(swedish, {"z", u8"\u00C5", u8"\u00C4", u8"\u00D6"});
    CHECK(swedish.equal(u8"\u00E5", u8"\u00C5", CollationStrength::secondary));
    CHECK(swedish.less(u8"\u00E5", u8"\u00C5", CollationStrength::tertiary));

    // The rule matches decomposed input and survives additional marks.
    checkEquivalent(swedish, u8"\u00E5", u8"a\u030A");
    checkEquivalent(swedish, u8"\u212B", u8"A\u030A");
    CHECK(swedish.equal(u8"\u00E5\u0301", u8"a\u030A\u0301"));
    CHECK_EQ(swedish.sortKey(u8"a\u030A\u0301", CollationStrength::primary)[0], 0xE000u);

    // Letters without a rule keep their root weights.
    checkKey(swedish, "a", CollationStrength::primary, {0x20B3, 0});
    CHECK_EQ(swedish.sortKey("a"), root.sortKey("a"));
}

TEST_CASE("collation: German and other locales use the root order")
{
    const Collator german(Locale("de"));
    const Collator austrian(Locale("de-AT"));
    const Collator french(Locale("fr"));

    checkOrder(german, {"a", u8"\u00E5", u8"\u00E4", "b"});
    checkOrder(german, {u8"\u00F6", "z"});
    checkOrder(austrian, {u8"\u00F6", "z"});
    checkOrder(french, {u8"\u00E4", "b"});

    for (const char *text : {"a", u8"\u00E4", u8"\u00F6", u8"\u00FC", u8"\u00DF", u8"\u00E5", "z"})
    {
        CHECK_EQ(german.sortKey(text), root.sortKey(text));
        CHECK_EQ(austrian.sortKey(text), root.sortKey(text));
        CHECK_EQ(french.sortKey(text), root.sortKey(text));
    }

    // sv-SE gets the Swedish rules through the language subtag.
    const Collator swedishFinland(Locale("sv-SE"));
    CHECK_EQ(swedishFinland.sortKey(u8"\u00E5"), swedish.sortKey(u8"\u00E5"));
    CHECK(swedishFinland.less("z", u8"\u00E5"));
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST_CASE("collation: ignorable characters, empty strings and invalid UTF-8")
{
    const std::string nul(1, '\0');
    CHECK(root.equal(nul, ""));
    CHECK(root.equal(std::string("a\0b", 3), "ab"));

    // Completely ignorable characters do not change any level.
    checkEquivalent(root, u8"a\u00ADb", "ab");   // SOFT HYPHEN
    checkEquivalent(root, u8"a\u034Fb", "ab");   // COMBINING GRAPHEME JOINER
    checkEquivalent(root, u8"a\u200Db", "ab");   // ZERO WIDTH JOINER
    checkEquivalent(root, u8"a\uFEFFb", "ab");   // ZERO WIDTH NO-BREAK SPACE

    CHECK(root.less("", "a"));
    CHECK(root.less("", "a", CollationStrength::primary));
    CHECK(root.compare("a", "") > 0);

    // Ill-formed UTF-8 is replaced with U+FFFD by bunch::decode(), which is
    // exactly the DUCET replacement character.
    CHECK(root.equal(std::string("\xFF", 1), u8"\uFFFD"));
    CHECK(root.equal(std::string("\xC3", 1), u8"\uFFFD"));
    CHECK(root.equal(std::string("\xE2\x82", 2), u8"\uFFFD"));
}

TEST_CASE("collation: collate() free function and locale binding")
{
    CHECK(localisation::collate("a", "b") < 0);
    CHECK(localisation::collate("b", "a") > 0);
    CHECK_EQ(localisation::collate("a", "a"), 0);
    CHECK(localisation::collate("z", u8"\u00E5") > 0); // root order
    CHECK(localisation::collate("z", u8"\u00E5", Locale("sv")) < 0);
    CHECK(localisation::collate(u8"\u00E4", "b", Locale("de")) < 0);

    CHECK_EQ(root.locale().tag(), std::string("und"));
    CHECK_EQ(swedish.locale().tag(), std::string("sv"));
    CHECK(swedish.locale().language() == "sv");

    // Collator() binds the root locale explicitly; the default strength is
    // tertiary, so case differences count.
    CHECK(root.less("a", "A"));
    CHECK(root.equal("a", "A", CollationStrength::primary));
}

TEST_CASE("collation: sorting a word list")
{
    std::vector<std::string> rootWords = {u8"\u00E9", "e", "f", "A", "a", "b", u8"\u00E4"};
    std::sort(rootWords.begin(), rootWords.end(),
              [](const std::string &left, const std::string &right) { return root.less(left, right); });
    const std::vector<std::string> expectedRoot = {"a", "A", u8"\u00E4", "b", "e", u8"\u00E9", "f"};
    CHECK_EQ(rootWords, expectedRoot);

    std::vector<std::string> swedishWords = {u8"\u00F6", "z", u8"\u00E5", "a", u8"\u00E4", "A", "b"};
    std::sort(swedishWords.begin(), swedishWords.end(), [](const std::string &left, const std::string &right) {
        return swedish.less(left, right);
    });
    const std::vector<std::string> expectedSwedish = {"a", "A", "b", "z", u8"\u00E5", u8"\u00E4", u8"\u00F6"};
    CHECK_EQ(swedishWords, expectedSwedish);
}

// ---------------------------------------------------------------------------
// CLDR tailorings
//
// The expected orders below are the ones the CLDR "standard" collation of
// common/collation/<locale>.xml defines; tools/gen_collation_data.py compiles
// them into localisation/collation_data.h.  Every locale with rules compared in
// an expanded weight space (root primary << 16, root secondary << 4, root
// tertiary << 3), so a tailored weight sits in a free slot between two root
// weights.  Locales without rules (de, fr, ru) keep the root order and
// therefore the exact DUCET sort keys.
// ---------------------------------------------------------------------------

namespace
{
const Collator german(Locale("de"));
const Collator spanish(Locale("es"));
const Collator french(Locale("fr"));
const Collator russian(Locale("ru"));
const Collator turkish(Locale("tr"));
const Collator czech(Locale("cs"));
const Collator danish(Locale("da"));
const Collator norwegian(Locale("nb"));
const Collator nynorsk(Locale("nn"));
const Collator finnish(Locale("fi"));
const Collator polish(Locale("pl"));
const Collator lithuanian(Locale("lt"));
const Collator latvian(Locale("lv"));
const Collator estonian(Locale("et"));
const Collator hungarian(Locale("hu"));
const Collator slovak(Locale("sk"));
const Collator romanian(Locale("ro"));
} // namespace

TEST_CASE("collation: Spanish tailoring puts n-tilde after n")
{
    // CLDR es standard: "&N<n-tilde<<<N-tilde".  The rule target is the
    // decomposed "n" + U+0303, so the tailoring itself is a contraction.
    checkOrder(spanish, {"n", u8"\u00F1", "o"});
    CHECK_FALSE(spanish.equal("n", u8"\u00F1", CollationStrength::primary));
    // At root n-tilde is n plus a combining tilde, so the primaries are equal.
    CHECK(root.equal("n", u8"\u00F1", CollationStrength::primary));
    CHECK(root.less("n", u8"\u00F1"));

    // The tailored primary is one slot above n's DUCET primary 0x2237.
    checkKey(spanish, u8"\u00F1", CollationStrength::tertiary, {0x22370001, 0, 0x200, 0, 0x10, 0});

    // Modern Spanish: ch and ll are plain c+h and l+l, not separate letters
    // (the CLDR "traditional" collation, which adds "&C<ch" and "&l<ll", is
    // not built in).  "ch" therefore sorts before "cz", not after every
    // other c word.
    checkOrder(spanish, {"c", "ch", "cz", "d"});
    checkOrder(spanish, {"l", "ll", "lz", "m"});
    CHECK_FALSE(spanish.equal("ch", "c", CollationStrength::primary));
    CHECK(spanish.less("ch", "cz"));
}

TEST_CASE("collation: Turkish tailoring (dotless i and dotted capital I)")
{
    // CLDR tr standard: "&[before 1]i<dotless-i<<<I", "&i<<<I-dot".
    checkOrder(turkish, {"h", u8"\u0131", "i", "j"});
    CHECK_FALSE(turkish.equal(u8"\u0131", "i", CollationStrength::primary));
    // The root order is the other way round: dotless i has its own primary
    // above i, so i < dotless-i at root.
    CHECK(root.less("i", u8"\u0131"));
    // "[before 1]" uses the top of the gap below the anchor: i is 0x21A6, so
    // dotless i is 0x21A5FF00.
    checkKey(turkish, u8"\u0131", CollationStrength::primary, {0x21A5FF00, 0});

    checkOrder(turkish, {"c", u8"\u00E7", "d"});
    checkOrder(turkish, {"g", u8"\u011F", "h"});
    checkOrder(turkish, {"o", u8"\u00F6", "p"});
    checkOrder(turkish, {"s", u8"\u015F", "t"});
    checkOrder(turkish, {"u", u8"\u00FC", "v"});

    // Dotted capital I is a tertiary variant of i in Turkish, while at root
    // it is i plus a combining dot above (a secondary difference).
    CHECK(turkish.equal("i", u8"\u0130", CollationStrength::secondary));
    CHECK_FALSE(root.equal("i", u8"\u0130", CollationStrength::secondary));
    checkOrder(turkish, {"i", u8"\u0130", "j"});
}

TEST_CASE("collation: Czech tailoring (c-caron, ch after h)")
{
    // CLDR cs standard: "&C<c-caron<<<C-caron", "&H<ch<<<cH<<<Ch<<<CH".
    checkOrder(czech, {"h", "ch", "i"});
    CHECK(root.less("ch", "h")); // root reads ch as c + h
    checkOrder(czech, {"ch", "cH", "Ch", "CH"});

    checkOrder(czech, {"c", u8"\u010D", "d"});
    checkOrder(czech, {"r", u8"\u0159", "s"});
    checkOrder(czech, {"s", u8"\u0161", "t"});
    checkOrder(czech, {"z", u8"\u017E"});
    // ch follows h, so it also follows every letter before h; only i and
    // later letters come after it.
    CHECK(czech.less("d", "ch"));
    CHECK(czech.less("ch", "i"));
    CHECK(czech.less("cv", "ch")); // h < v, so ch sorts before cv
}

TEST_CASE("collation: Danish and Norwegian tailoring (ae, o-slash, a-ring)")
{
    // CLDR da standard: "&[before 1]|(U+01C0)<ae<<<AE<<a-diaeresis<...
    //                     <o-slash<...<a-ring<<<aa<<<Aa<<<AA".
    checkOrder(danish, {"z", u8"\u00E6", u8"\u00E4", u8"\u00F8", u8"\u00F6", u8"\u00E5"});
    CHECK(danish.equal(u8"\u00E6", u8"\u00E4", CollationStrength::primary));
    CHECK(danish.equal(u8"\u00F8", u8"\u00F6", CollationStrength::primary));
    CHECK(root.less(u8"\u00E6", "z")); // at root ae sorts with a

    // The doubled a is a variant of a-ring: same primary, tertiary above it.
    checkOrder(danish, {"z", u8"\u00E5", "aa", "Aa", "AA"});
    CHECK(danish.equal("aa", u8"\u00E5", CollationStrength::primary));
    CHECK(danish.less(u8"\u00E5", "aa"));

    // Thorn and eth are d/t variants in both CLDR files.
    checkOrder(danish, {"t", "th", u8"\u00FE", "u"});
    checkOrder(danish, {"d", u8"\u00F0", "e"});

    // Norwegian (CLDR no.xml, inherited by nb and nn) uses ae, o-slash,
    // a-ring after z without the Danish a-diaeresis/o-diaeresis ordering.
    checkOrder(norwegian, {"z", u8"\u00E6", u8"\u00F8", u8"\u00E5"});
    CHECK(norwegian.equal("aa", u8"\u00E5", CollationStrength::primary));
    CHECK_FALSE(norwegian.equal(u8"\u00E6", "ae", CollationStrength::primary));
    // nb and nn share the no block.
    CHECK_EQ(norwegian.sortKey(u8"\u00E5"), nynorsk.sortKey(u8"\u00E5"));
    CHECK(norwegian.less(u8"z", u8"\u00E6"));
}

TEST_CASE("collation: Finnish tailoring (a-ring, a-diaeresis, o-diaeresis)")
{
    // CLDR fi standard:
    //   "&[before 1]|(U+01C0)<a-ring<<<A-ring<a-diaeresis<<<A-diaeresis
    //    <ae<<<AE<o-diaeresis<<<O-diaeresis<o-slash<<<O-slash".
    checkOrder(finnish, {"z", u8"\u00E5", u8"\u00E4", u8"\u00F6"});
    CHECK(root.less(u8"\u00E4", "z"));
    CHECK(finnish.less("z", u8"\u00E5"));

    // ae is a secondary variant of a-diaeresis in the CLDR file, so it lands
    // between a-diaeresis and o-diaeresis.
    CHECK(finnish.equal(u8"\u00E6", u8"\u00E4", CollationStrength::primary));
    checkOrder(finnish, {u8"\u00E4", u8"\u00E6", u8"\u00F6"});
    // u-diaeresis is a secondary variant of y.
    checkOrder(finnish, {"y", u8"\u00FC", "z"});
}

TEST_CASE("collation: Polish tailoring")
{
    // CLDR pl standard: "&A<a-ogonek<<<A-ogonek" and the same for c, e, l, n,
    // o, s, and "&Z<z-acute<<<Z-acute<z-dot<<<Z-dot".
    checkOrder(polish, {"a", u8"\u0105", "b"});
    checkOrder(polish, {"c", u8"\u0107", "d"});
    checkOrder(polish, {"e", u8"\u0119", "f"});
    checkOrder(polish, {"l", u8"\u0142", "m"});
    checkOrder(polish, {"n", u8"\u0144", "o"});
    checkOrder(polish, {"o", u8"\u00F3", "p"});
    checkOrder(polish, {"s", u8"\u015B", "t"});
    // Acute s and dot-above s are both after z, acute first.
    checkOrder(polish, {"z", u8"\u017A", u8"\u017C"});
    CHECK(root.equal("a", u8"\u0105", CollationStrength::primary));
    CHECK(root.equal("z", u8"\u017A", CollationStrength::primary));
}

TEST_CASE("collation: Lithuanian tailoring")
{
    // CLDR lt standard uses primary relations for c, s, z and u, secondary
    // ones for a, e and i, and rewrites y as an i variant.
    checkOrder(lithuanian, {"c", u8"\u010D", "d"});
    checkOrder(lithuanian, {"s", u8"\u0161", "t"});
    checkOrder(lithuanian, {"z", u8"\u017E"});
    checkOrder(lithuanian, {"u", u8"\u0173", u8"\u016B", "v"});

    CHECK(lithuanian.equal("a", u8"\u0105", CollationStrength::primary));
    CHECK(lithuanian.less("a", u8"\u0105", CollationStrength::secondary));
    checkOrder(lithuanian, {"a", u8"\u0105", "b"});
    // e-ogonek and e-dot are secondary variants of e, in that order.
    checkOrder(lithuanian, {"e", u8"\u0119", u8"\u0117", "f"});
    // y is a secondary variant of i, so it sorts before j; at root j < y.
    checkOrder(lithuanian, {"i", u8"\u012F", "y", "j"});
    CHECK(root.less("j", "y"));

    // "&combining-grave=combining-dot-above combining-grave": the dot above
    // rule makes i + dot above equal to the plain accented letter.
    checkEquivalent(lithuanian, u8"i\u0307\u0300", u8"\u00EC");
    CHECK_FALSE(root.equal(u8"i\u0307\u0300", u8"\u00EC"));
}

TEST_CASE("collation: Latvian tailoring")
{
    // CLDR lv standard: "&I<y<<<Y<I-macron<<<I-macron" plus primary relations
    // for a, c, e, g, k, l, n, o, r, s, u and z.
    checkOrder(latvian, {"i", "y", u8"\u012B", "j"});
    CHECK(root.less("j", "y"));
    checkOrder(latvian, {"c", u8"\u010D", "d"});
    checkOrder(latvian, {"g", u8"\u0123", "h"});
    checkOrder(latvian, {"s", u8"\u0161", "t"});
    CHECK_FALSE(latvian.equal("i", "y", CollationStrength::primary));
}

TEST_CASE("collation: Estonian tailoring moves z between s-caron and t")
{
    // CLDR et standard: "&[before 1]T<s-caron<<<S-caron<z<<<Z<z-caron<<<Z-caron"
    // and "&[before 1]X<o-tilde<<<O-tilde<a-diaeresis<...<u-diaeresis".
    checkOrder(estonian, {"s", u8"\u0161", "z", u8"\u017E", "t"});
    CHECK(root.less("t", "z")); // the opposite of the root order
    checkOrder(estonian, {"r", "s", u8"\u0161", "z", u8"\u017E", "t"});
    // The vowels o-tilde, a-diaeresis, o-diaeresis and u-diaeresis are moved
    // in front of x, in that order.
    checkOrder(estonian, {"w", u8"\u00F5", u8"\u00E4", u8"\u00F6", u8"\u00FC", "x"});
    CHECK(estonian.less("z", "x"));
}

TEST_CASE("collation: Hungarian digraphs and doubled digraphs")
{
    // CLDR hu standard: "&C<cs<<<Cs<<<CS", "&D<dz", "&DZ<dzs", "&G<gy", ...
    checkOrder(hungarian, {"c", "cs", "d"});
    checkOrder(hungarian, {"cz", "cs"}); // h < z, so cs sorts before cz
    CHECK(root.less("cs", "cz"));
    checkOrder(hungarian, {"d", "dz", "dzs", "e"});
    checkOrder(hungarian, {"g", "gy", "h"});
    checkOrder(hungarian, {"l", "ly", "m"});
    checkOrder(hungarian, {"n", "ny", "o"});
    checkOrder(hungarian, {"s", "sz", "t"});
    checkOrder(hungarian, {"z", "zs"});

    // "&cs<<<ccs/cs": the doubled digraph expands to two cs units, so it
    // sorts after cs and after the neighbouring c + s spellings.
    checkOrder(hungarian, {"cs", "ccs", "d"});
    CHECK(hungarian.equal("ccs", "cscs", CollationStrength::secondary));
    CHECK(hungarian.less("cscs", "ccs"));

    // Long umlauts differ from their short forms at the secondary level.
    checkOrder(hungarian, {"o", u8"\u00F6", u8"\u0151", "p"});
    checkOrder(hungarian, {"u", u8"\u00FC", u8"\u0171", "v"});
    CHECK(hungarian.equal(u8"\u00F6", u8"\u0151", CollationStrength::primary));
    CHECK(hungarian.less(u8"\u00F6", u8"\u0151", CollationStrength::secondary));
}

TEST_CASE("collation: Slovak tailoring")
{
    // CLDR sk standard: a-diaeresis, c-caron, "&H<ch", o-circumflex, r-caron,
    // s-caron and z-caron.
    checkOrder(slovak, {"h", "ch", "i"});
    CHECK(root.less("ch", "h"));
    checkOrder(slovak, {"a", u8"\u00E4", "b"});
    checkOrder(slovak, {"c", u8"\u010D", "d"});
    checkOrder(slovak, {"o", u8"\u00F4", "p"});
    checkOrder(slovak, {"r", u8"\u0159", "s"});
    checkOrder(slovak, {"s", u8"\u0161", "t"});
    checkOrder(slovak, {"z", u8"\u017E"});
    CHECK(root.equal("a", u8"\u00E4", CollationStrength::primary));
}

TEST_CASE("collation: Romanian tailoring and the comma-below forms")
{
    // CLDR ro standard: "&A<a-breve<<<A-breve<a-circumflex<<<A-circumflex",
    // "&I<i-circumflex", "&S<s-cedilla=s-comma-below<<<S-cedilla=S-comma-below"
    // and the same for t.
    checkOrder(romanian, {"a", u8"\u0103", u8"\u00E2", "b"});
    checkOrder(romanian, {"i", u8"\u00EE", "j"});
    checkOrder(romanian, {"s", u8"\u0219", "t"});

    // "=" makes the cedilla and comma-below spellings collation-equal, which
    // they are not at root.
    CHECK(romanian.equal(u8"\u015F", u8"\u0219"));
    CHECK(romanian.equal(u8"\u015E", u8"\u0218"));
    CHECK(romanian.equal(u8"\u0163", u8"\u021B"));
    CHECK(romanian.equal(u8"\u0162", u8"\u021A"));
    CHECK_EQ(romanian.sortKey(u8"\u015F"), romanian.sortKey(u8"\u0219"));
    CHECK_FALSE(root.equal(u8"\u015F", u8"\u0219"));
    CHECK(root.equal(u8"\u015F", u8"\u0219", CollationStrength::primary));
}

TEST_CASE("collation: German, French and Russian keep the root order")
{
    // CLDR de standard is documented as "the same as in root": umlauts are
    // secondary variants of a, o and u, and sharp s expands to ss.  CLDR fr
    // has no collation file at all, and CLDR ru standard only carries
    // [reorder Cyrl], which the root order already satisfies.
    CHECK_FALSE(german.tailored());
    CHECK_FALSE(french.tailored());
    CHECK_FALSE(russian.tailored());

    CHECK(german.equal("a", u8"\u00E4", CollationStrength::primary));
    CHECK(german.less("a", u8"\u00E4"));
    checkOrder(german, {"a", u8"\u00E4", "b"});
    checkOrder(german, {u8"\u00F6", "p"});
    // Sharp s is two s primaries plus a secondary weight of its own.
    CHECK(german.equal(u8"\u00DF", "ss", CollationStrength::primary));
    CHECK_FALSE(german.equal(u8"\u00DF", "ss", CollationStrength::secondary));
    CHECK(german.less("ss", u8"\u00DF", CollationStrength::secondary));

    CHECK(french.equal("a", u8"\u00E0", CollationStrength::primary));
    checkOrder(french, {"cote", u8"cot\u00E9", u8"c\u00F4te"});

    checkOrder(russian, {u8"\u0430", u8"\u0431", u8"\u0432", u8"\u0433"});
    CHECK(russian.equal(u8"\u0439", u8"\u0438\u0306"));

    for (const char *text : {"a", "z", u8"\u00E4", u8"\u00F6", u8"\u00FC", u8"\u00DF", u8"\u00E5",
                             u8"\u0439", u8"\u0430\u0301", "ch"})
    {
        CHECK_EQ(german.sortKey(text), root.sortKey(text));
        CHECK_EQ(french.sortKey(text), root.sortKey(text));
        CHECK_EQ(russian.sortKey(text), root.sortKey(text));
    }
}

TEST_CASE("collation: discontiguous contractions")
{
    // Czech s-caron is the tailored contraction <s, U+030C>.  U+030C has
    // combining class 230, so a lower-class mark between s and the caron
    // (U+0323, class 220) does not block the match: UTS #10 section 3.3.2
    // allows the contraction to match across it, and the skipped mark keeps
    // its own secondary weight, which is emitted after the contraction.
    CHECK(czech.equal(u8"s\u0323\u030C", u8"\u0161", CollationStrength::primary));
    CHECK(czech.less(u8"\u0161", u8"s\u0323\u030C", CollationStrength::secondary));
    CHECK_FALSE(czech.equal(u8"s\u0323\u030C", u8"\u0161"));
    // Canonical ordering makes the two spellings of the same sequence equal.
    checkEquivalent(czech, u8"s\u030C\u0323", u8"s\u0323\u030C");

    // A mark with the same combining class blocks the match instead: U+0301
    // is class 230, so the caron stays a separate accent and the primary
    // weight stays s's.
    CHECK(czech.equal(u8"s\u0301\u030C", "s", CollationStrength::primary));
    CHECK_FALSE(czech.equal(u8"s\u0301\u030C", u8"\u0161", CollationStrength::primary));

    // The same mechanism applies to the DUCET contraction of Cyrillic short i
    // (U+0438 U+0306), which is reached through normalisation of U+0439.
    CHECK(root.equal(u8"\u0438\u0323\u0306", u8"\u0439", CollationStrength::primary));
    CHECK(root.less(u8"\u0439", u8"\u0438\u0323\u0306", CollationStrength::secondary));
    CHECK_FALSE(root.equal(u8"\u0438\u0301\u0306", u8"\u0439", CollationStrength::primary));

    // A starter always blocks: "l" + U+00B7 is a contiguous DUCET
    // contraction only.
    CHECK(root.equal("l\u00B7", "l", CollationStrength::primary));
    CHECK_FALSE(root.equal("l\u0301\u00B7", "l\u00B7", CollationStrength::primary));

    // Without the Czech rules the s-caron pair is just s followed by a mark.
    CHECK(root.equal(u8"s\u0323\u030C", "s", CollationStrength::primary));
    CHECK(root.equal(u8"\u0161", "s", CollationStrength::primary));
    CHECK(root.less("s", u8"\u0161"));
}

TEST_CASE("collation: tailored sort keys agree with compare")
{
    const Collator *const collators[] = {&spanish, &turkish, &czech, &danish,
                                         &estonian, &hungarian, &lithuanian};
    const std::vector<std::string> corpus = {
        "",     "a",    "A",    u8"\u00E4", u8"\u00E5", u8"\u00E6", u8"\u00F8", u8"\u00F1",
        u8"\u010D", u8"ch", u8"Ch", u8"\u00DF", u8"ss", "i", u8"\u0131", u8"\u0130",
        u8"s\u0323\u030C", u8"\u0161", u8"cs", u8"ccs", u8"dzs", u8"\u017E", u8"\u00FE", "aa",
    };
    const CollationStrength strengths[] = {CollationStrength::primary, CollationStrength::secondary,
                                           CollationStrength::tertiary};
    for (const Collator *collator : collators)
    {
        for (const std::string &left : corpus)
        {
            for (const std::string &right : corpus)
            {
                for (CollationStrength strength : strengths)
                {
                    const std::vector<std::uint32_t> leftKey = collator->sortKey(left, strength);
                    const std::vector<std::uint32_t> rightKey = collator->sortKey(right, strength);
                    const int comparison = collator->compare(left, right, strength);
                    CHECK_EQ(comparison < 0, leftKey < rightKey);
                    CHECK_EQ(comparison == 0, leftKey == rightKey);
                    CHECK_EQ(comparison > 0, leftKey > rightKey);
                }
            }
        }
    }
}

TEST_CASE("collation: tailored word lists sort as expected")
{
    // The Czech alphabet: ch follows h, so "chata" sorts after "hrad" but
    // before "jablko"; "čaj" follows "cukr".
    std::vector<std::string> czechWords = {u8"chata", u8"hrad", u8"cukr", u8"\u010Daj",
                                           u8"jablko", u8"do"};
    std::sort(czechWords.begin(), czechWords.end(), [](const std::string &left, const std::string &right) {
        return czech.less(left, right);
    });
    const std::vector<std::string> expectedCzech = {u8"cukr", u8"\u010Daj", u8"do", u8"hrad",
                                                    u8"chata", u8"jablko"};
    CHECK_EQ(czechWords, expectedCzech);

    // Swedish keeps its own order: a-ring, a-diaeresis, o-diaeresis after z.
    std::vector<std::string> swedishWords = {u8"\u00E4pple", u8"zebra", u8"\u00E5r", u8"apa",
                                             u8"\u00F6ga"};
    std::sort(swedishWords.begin(), swedishWords.end(), [](const std::string &left, const std::string &right) {
        return swedish.less(left, right);
    });
    const std::vector<std::string> expectedSwedishWords = {u8"apa", u8"zebra", u8"\u00E5r",
                                                           u8"\u00E4pple", u8"\u00F6ga"};
    CHECK_EQ(swedishWords, expectedSwedishWords);
}

TEST_CASE("collation: unsupported locales fall back to the root order")
{
    // ja, zh and ko are not built in (their CLDR rule files are script
    // specific and far too large for the compact table), and an unknown tag
    // has no rules at all.
    const Collator japanese(Locale("ja"));
    const Collator chinese(Locale("zh-Hans"));
    const Collator korean(Locale("ko"));
    const Collator unknown(Locale("xx-YY"));
    CHECK_FALSE(japanese.tailored());
    CHECK_FALSE(chinese.tailored());
    CHECK_FALSE(korean.tailored());
    CHECK_FALSE(unknown.tailored());

    for (const char *text : {u8"\u00E4", u8"\u00E5", u8"\u00E6", u8"\u010D", "ch", u8"\u0430",
                             u8"\u4E00", u8"\uAC00", "abc"})
    {
        CHECK_EQ(japanese.sortKey(text), root.sortKey(text));
        CHECK_EQ(chinese.sortKey(text), root.sortKey(text));
        CHECK_EQ(korean.sortKey(text), root.sortKey(text));
        CHECK_EQ(unknown.sortKey(text), root.sortKey(text));
    }

    // Han and Hangul keep their root order in every collator.  A tailored
    // collator scales the implicit weights with its expanded weight space, so
    // U+4E00 keeps the 0xFB40 lead unit but shifted by 16 bits.
    checkOrder(spanish, {"z", u8"\u4E00", u8"\u4E01"});
    checkKey(spanish, u8"\u4E00", CollationStrength::primary, {0xFB400000, 0xCE000000, 0});
    checkOrder(japanese, {u8"\u4E00", u8"\u4E01"});
    checkOrder(chinese, {"z", u8"\u4E00"});

    // The language subtag selects the tailoring, so cs-CZ and tr-TR work too.
    CHECK_EQ(Collator(Locale("cs-CZ")).sortKey(u8"\u010D"), czech.sortKey(u8"\u010D"));
    CHECK_EQ(Collator(Locale("tr-TR")).sortKey(u8"\u0131"), turkish.sortKey(u8"\u0131"));
    CHECK_EQ(Collator(Locale("nb-NO")).sortKey(u8"\u00E6"), norwegian.sortKey(u8"\u00E6"));
    CHECK(Locale("cs-CZ").language() == "cs");
}

TEST_CASE("collation: collate() honours the tailoring")
{
    CHECK(localisation::collate("ch", "h") < 0);              // root: c + h
    CHECK(localisation::collate("ch", "h", Locale("cs")) > 0); // Czech: ch after h
    CHECK(localisation::collate(u8"\u00E6", "z", Locale("da")) > 0);
    CHECK(localisation::collate(u8"\u00E6", "z") < 0);
    CHECK_EQ(localisation::collate("a", "a", Locale("hu")), 0);
}
