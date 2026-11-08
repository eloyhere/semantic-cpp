#include "bunch/linebreak.h"

#include "linebreak_vectors.h"
#include "test_framework.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
// Encodes code points as UTF-8 so the test data stays readable and this file
// stays ASCII.
auto utf8(std::initializer_list<std::uint32_t> codepoints) -> std::string
{
    std::string text;
    text.reserve(codepoints.size() * 2);
    for (const std::uint32_t codepoint : codepoints)
    {
        if (codepoint < 0x80U)
        {
            text.push_back(static_cast<char>(codepoint));
        }
        else if (codepoint < 0x800U)
        {
            text.push_back(static_cast<char>(0xC0U | (codepoint >> 6)));
            text.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
        }
        else if (codepoint < 0x10000U)
        {
            text.push_back(static_cast<char>(0xE0U | (codepoint >> 12)));
            text.push_back(static_cast<char>(0x80U | ((codepoint >> 6) & 0x3FU)));
            text.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
        }
        else
        {
            text.push_back(static_cast<char>(0xF0U | (codepoint >> 18)));
            text.push_back(static_cast<char>(0x80U | ((codepoint >> 12) & 0x3FU)));
            text.push_back(static_cast<char>(0x80U | ((codepoint >> 6) & 0x3FU)));
            text.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
        }
    }
    return text;
}

auto formatOffsets(const std::vector<std::size_t> &offsets) -> std::string
{
    std::ostringstream stream;
    stream << '{';
    for (std::size_t index = 0; index < offsets.size(); ++index)
    {
        if (index != 0)
        {
            stream << ", ";
        }
        stream << offsets[index];
    }
    stream << '}';
    return stream.str();
}

auto formatText(std::string_view text) -> std::string
{
    std::ostringstream stream;
    stream << std::hex << std::uppercase << std::setfill('0');
    for (std::size_t index = 0; index < text.size(); ++index)
    {
        if (index != 0)
        {
            stream << ' ';
        }
        stream << std::setw(2) << static_cast<unsigned>(static_cast<unsigned char>(text[index]));
    }
    return stream.str();
}

auto formatClasses(const std::vector<bunch::LineBreakClass> &classes) -> std::string
{
    std::ostringstream stream;
    stream << '{';
    for (std::size_t index = 0; index < classes.size(); ++index)
    {
        if (index != 0)
        {
            stream << ", ";
        }
        stream << static_cast<int>(classes[index]);
    }
    stream << '}';
    return stream.str();
}

[[noreturn]] void failStyled(const std::string &message)
{
    ::test::fail(__FILE__, __LINE__, message);
}

void checkOpportunities(const std::string &text, const std::vector<std::size_t> &expected)
{
    const std::vector<std::size_t> actual = bunch::lineBreakOpportunities(text);
    if (actual != expected)
    {
        failStyled("lineBreakOpportunities([" + formatText(text) + "]) = " + formatOffsets(actual) + ", expected " +
                   formatOffsets(expected));
    }
}

void checkMandatory(const std::string &text, const std::vector<std::size_t> &expected)
{
    const std::vector<std::size_t> actual = bunch::mandatoryLineBreaks(text);
    if (actual != expected)
    {
        failStyled("mandatoryLineBreaks([" + formatText(text) + "]) = " + formatOffsets(actual) + ", expected " +
                   formatOffsets(expected));
    }
}

void checkClass(std::uint32_t codepoint, bunch::LineBreakClass expected)
{
    const bunch::LineBreakClass actual = bunch::lineBreakClass(codepoint);
    if (actual != expected)
    {
        std::ostringstream message;
        message << "lineBreakClass(U+" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << codepoint
                << ") = " << std::dec << static_cast<int>(actual) << ", expected " << static_cast<int>(expected);
        failStyled(message.str());
    }
}

void checkClasses(const std::string &text, const std::vector<bunch::LineBreakClass> &expected)
{
    const std::vector<bunch::LineBreakClass> actual = bunch::lineBreakClasses(text);
    if (actual != expected)
    {
        failStyled("lineBreakClasses([" + formatText(text) + "]) = " + formatClasses(actual) + ", expected " +
                   formatClasses(expected));
    }
}

void checkVector(const linebreak_vectors::Vector &vector)
{
    const std::string_view text(linebreak_vectors::kText + vector.textOffset, vector.textLength);
    const std::vector<std::size_t> expected(linebreak_vectors::kBreaks + vector.breakOffset,
                                            linebreak_vectors::kBreaks + vector.breakOffset + vector.breakCount);
    const std::vector<std::size_t> actual = bunch::lineBreakOpportunities(text);
    if (actual != expected)
    {
        std::ostringstream message;
        message << "LineBreakTest.txt:" << vector.sourceLine << " [" << formatText(text) << "] = "
                << formatOffsets(actual) << ", expected " << formatOffsets(expected);
        failStyled(message.str());
    }
}
} // namespace

// ---------------------------------------------------------------------------
// Conformance
// ---------------------------------------------------------------------------

TEST_CASE("line break: UAX #14 conformance vectors")
{
    CHECK_EQ(std::size(linebreak_vectors::kVectors), std::size_t{7654});
    for (const linebreak_vectors::Vector &vector : linebreak_vectors::kVectors)
    {
        checkVector(vector);
    }
}

// ---------------------------------------------------------------------------
// LB1
// ---------------------------------------------------------------------------

TEST_CASE("line break: LB1 class resolution")
{
    checkClass(0x0041, bunch::LineBreakClass::al);
    checkClass(0x000A, bunch::LineBreakClass::lf);
    checkClass(0x000D, bunch::LineBreakClass::cr);
    checkClass(0x0009, bunch::LineBreakClass::ba);
    checkClass(0x0020, bunch::LineBreakClass::sp);
    checkClass(0x2014, bunch::LineBreakClass::b2);
    checkClass(0x00A0, bunch::LineBreakClass::gl);
    checkClass(0x2028, bunch::LineBreakClass::bk);
    checkClass(0x0308, bunch::LineBreakClass::cm);
    checkClass(0x200D, bunch::LineBreakClass::zwj);
    checkClass(0x1F469, bunch::LineBreakClass::eb);
    checkClass(0x1F3FD, bunch::LineBreakClass::em);
    checkClass(0x65E5, bunch::LineBreakClass::id);

    // AI, SG and XX resolve to AL.
    checkClass(0x00A7, bunch::LineBreakClass::al);   // AI, SECTION SIGN
    checkClass(0xD800, bunch::LineBreakClass::al);   // SG
    checkClass(0x0378, bunch::LineBreakClass::al);   // XX, unassigned
    checkClass(0x110000, bunch::LineBreakClass::al); // beyond the last code point

    // CJ resolves to NS.
    checkClass(0x3041, bunch::LineBreakClass::ns); // CJ, HIRAGANA LETTER SMALL A

    // SA resolves to CM for Mn/Mc and to AL otherwise.
    checkClass(0x0E01, bunch::LineBreakClass::al); // SA, THAI CHARACTER KO KAI
    checkClass(0x0E31, bunch::LineBreakClass::cm); // SA, THAI CHARACTER MAI HAN-AKAT

    // CB is left unresolved for LB20.
    checkClass(0xFFFC, bunch::LineBreakClass::cb);
}

TEST_CASE("line break: LB1 classes for a string")
{
    checkClasses(utf8({}), {});
    checkClasses(utf8({0x0024, 0x0031, 0x002C, 0x0032, 0x0033, 0x002C, 0x0034, 0x0025}),
                 {bunch::LineBreakClass::pr, bunch::LineBreakClass::nu, bunch::LineBreakClass::is,
                  bunch::LineBreakClass::nu, bunch::LineBreakClass::nu, bunch::LineBreakClass::is,
                  bunch::LineBreakClass::nu, bunch::LineBreakClass::po});
    // LB9/LB10 are contextual and are not applied by lineBreakClasses.
    checkClasses(utf8({0x0061, 0x0308}), {bunch::LineBreakClass::al, bunch::LineBreakClass::cm});
}

// ---------------------------------------------------------------------------
// Start, end, and mandatory breaks
// ---------------------------------------------------------------------------

TEST_CASE("line break: start, end, and empty text")
{
    checkOpportunities("", {0});
    checkMandatory("", {0});
    checkOpportunities("hello world", {0, 6, 11});
    checkMandatory("hello world", {11});

    // The end of text is always reported, also when it follows a hard break.
    checkOpportunities("\n", {0, 1});
    checkMandatory("\n", {1});
}

TEST_CASE("line break: mandatory breaks")
{
    // LB5: CR LF is kept together and yields one break, after the LF.
    checkOpportunities(utf8({0x0061, 0x000D, 0x000A, 0x0062}), {0, 3, 4});
    checkMandatory(utf8({0x0061, 0x000D, 0x000A, 0x0062}), {3, 4});
    checkOpportunities(utf8({0x0061, 0x000D, 0x0062}), {0, 2, 3});
    checkMandatory(utf8({0x0061, 0x000D, 0x0062}), {2, 3});
    checkOpportunities(utf8({0x0061, 0x000A, 0x0062}), {0, 2, 3});
    checkMandatory(utf8({0x0061, 0x000A, 0x0062}), {2, 3});

    // BK: LINE SEPARATOR.
    checkOpportunities(utf8({0x0061, 0x2028, 0x0062}), {0, 4, 5});
    checkMandatory(utf8({0x0061, 0x2028, 0x0062}), {4, 5});

    // A trailing hard break coincides with the end of text.
    checkOpportunities(utf8({0x0061, 0x000A, 0x0062, 0x000A}), {0, 2, 4});
    checkMandatory(utf8({0x0061, 0x000A, 0x0062, 0x000A}), {2, 4});
}

TEST_CASE("line break: options")
{
    const std::string text = "a b";
    CHECK_EQ(bunch::lineBreakOpportunities(text), (std::vector<std::size_t>{0, 2, 3}));

    bunch::LineBreakOptions options;
    options.includeStart = false;
    CHECK_EQ(bunch::lineBreakOpportunities(text, options), (std::vector<std::size_t>{2, 3}));

    options = {};
    options.includeEnd = false;
    CHECK_EQ(bunch::lineBreakOpportunities(text, options), (std::vector<std::size_t>{0, 2}));

    options = {};
    options.includeMandatory = false;
    CHECK_EQ(bunch::lineBreakOpportunities(text, options), (std::vector<std::size_t>{0, 2, 3}));

    // A hard break is dropped, the end of text is kept.
    const std::string hard = utf8({0x0061, 0x000A, 0x0062});
    CHECK_EQ(bunch::lineBreakOpportunities(hard, options), (std::vector<std::size_t>{0, 3}));

    // Only the space provides a soft break.
    options = {};
    options.includeStart = false;
    options.includeEnd = false;
    options.includeMandatory = false;
    CHECK_EQ(bunch::lineBreakOpportunities(text, options), (std::vector<std::size_t>{2}));

    // The single position of empty text has both the start and the end role.
    options = {};
    options.includeStart = false;
    CHECK_EQ(bunch::lineBreakOpportunities(std::string(), options), (std::vector<std::size_t>{0}));
    options.includeEnd = false;
    CHECK_EQ(bunch::lineBreakOpportunities(std::string(), options), std::vector<std::size_t>{});
}

// ---------------------------------------------------------------------------
// Spaces, combining marks, and punctuation
// ---------------------------------------------------------------------------

TEST_CASE("line break: spaces and combining marks")
{
    // LB9: the combining mark is attached to the preceding character.
    checkOpportunities(utf8({0x0061, 0x0308, 0x0020, 0x0062}), {0, 4, 5});
    checkOpportunities(utf8({0x0061, 0x0308, 0x0062}), {0, 4});

    // LB10: a combining mark without a base behaves like AL.
    checkOpportunities(utf8({0x0308, 0x0061}), {0, 3});

    // LB8a: no break after a ZWJ, and LB10 when the ZWJ follows a space.
    checkOpportunities(utf8({0x0020, 0x200D, 0x0020, 0x0062}), {0, 1, 5, 6});

    // LB18: break after spaces.
    checkOpportunities("a b", {0, 2, 3});
    checkOpportunities(utf8({0x0061, 0x0020, 0x0020, 0x0062}), {0, 3, 4});

    // LB12/LB12a: no break around NO-BREAK SPACE, except after a space.
    checkOpportunities(utf8({0x0061, 0x00A0, 0x0062}), {0, 4});
    checkOpportunities(utf8({0x0061, 0x0020, 0x00A0, 0x0062}), {0, 2, 5});
}

TEST_CASE("line break: punctuation, parentheses, and numbers")
{
    // LB19: quotation marks.
    checkOpportunities("\"a\"", {0, 3});

    // LB30: Latin parentheses do not break, East Asian ones do.
    checkOpportunities("foo(bar)", {0, 8});
    checkOpportunities(")A", {0, 2});
    checkOpportunities("A(B", {0, 3});
    checkOpportunities(utf8({0x0041, 0xFF08, 0x0042}), {0, 1, 5});
    checkOpportunities(utf8({0xFF09, 0x0041}), {0, 3, 4});

    // LB17: no break inside a pair of em dashes; LB31 around a single one.
    checkOpportunities(utf8({0x0061, 0x2014, 0x0062}), {0, 1, 4, 5});

    // LB25: numbers stay together.
    checkOpportunities(utf8({0x0033, 0x002E, 0x0031, 0x0034}), {0, 4});
    checkOpportunities("$1,234.56%", {0, 10});
    checkOpportunities(utf8({0x0028, 0x0031, 0x0032, 0x0029, 0x00A2}), {0, 6});

    // LB21b and LB13: a solidus allows a break after it.
    checkOpportunities("x/y", {0, 2, 3});
    checkOpportunities("a(b", {0, 3});

    // The Example 7 tailoring allows a break between a prefix and an opening
    // parenthesis when no digit follows.
    checkOpportunities(utf8({0x0024, 0x0028, 0x0061}), {0, 1, 3});
}

// ---------------------------------------------------------------------------
// Emoji, joiners, and regional indicators
// ---------------------------------------------------------------------------

TEST_CASE("line break: emoji, joiners, and regional indicators")
{
    // LB8a/LB30b: emoji ZWJ sequence and emoji modifier sequence.
    checkOpportunities(utf8({0x1F469, 0x200D, 0x1F469}), {0, 11});
    checkOpportunities(utf8({0x1F469, 0x1F3FD}), {0, 8});
    checkOpportunities(utf8({0x1F3FD, 0x1F469}), {0, 4, 8});

    // LB8a: no break after a ZWJ, even before an East Asian opening bracket.
    checkOpportunities(utf8({0x65E5, 0x200D, 0x672C}), {0, 9});
    checkOpportunities(utf8({0x0061, 0x200D, 0xFF08}), {0, 7});

    // LB31: ideographs break on both sides.
    checkOpportunities(utf8({0x65E5, 0x672C}), {0, 3, 6});

    // LB30a: regional indicators pair up.
    checkOpportunities(utf8({0x1F1E6, 0x1F1E7}), {0, 8});
    checkOpportunities(utf8({0x1F1E6, 0x1F1E7, 0x1F1E8}), {0, 8, 12});
    checkOpportunities(utf8({0x1F1E6, 0x1F1E7, 0x1F1E8, 0x1F1E9}), {0, 8, 16});
}

// ---------------------------------------------------------------------------
// Ill-formed UTF-8
// ---------------------------------------------------------------------------

TEST_CASE("line break: ill-formed UTF-8")
{
    // One U+FFFD per invalid byte; U+FFFD is XX, which resolves to AL.
    const std::string single(1, static_cast<char>(0xFF));
    checkClasses(single, {bunch::LineBreakClass::al});
    checkOpportunities(single, {0, 1});

    const std::string truncated("\xE2\x80", 2);
    checkClasses(truncated, {bunch::LineBreakClass::al, bunch::LineBreakClass::al});
    checkOpportunities(truncated, {0, 2});

    const std::string mixed = "a" + single + "b";
    checkOpportunities(mixed, {0, 3});
}
