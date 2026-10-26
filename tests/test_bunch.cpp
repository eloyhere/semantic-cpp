#include "bunch/bunch.h"

#include "test_framework.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <deque>
#include <functional>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
using Bytes = std::vector<unsigned char>;
using Codepoints = std::vector<unsigned int>;

Bytes makeBytes(std::initializer_list<unsigned char> values)
{
    return Bytes(values);
}

Codepoints makeCodepoints(std::initializer_list<unsigned int> values)
{
    return Codepoints(values);
}

std::string asBunch(const Bytes &bytes)
{
    return std::string(bytes.begin(), bytes.end());
}

bool same(const ::bunch::Bunch &sequence, std::string_view text)
{
    return sequence == text;
}

std::vector<unsigned int> decodeAll(std::string_view input, ::bunch::charset encoding)
{
    std::vector<unsigned int> result;
    while (!input.empty())
    {
        result.push_back(::bunch::decode(input, encoding));
    }
    return result;
}

Bytes encodeAll(std::initializer_list<unsigned int> codepoints, ::bunch::charset encoding)
{
    Bytes result;
    for (unsigned int codepoint : codepoints)
    {
        Bytes encoded = ::bunch::encode(codepoint, encoding);
        result.insert(result.end(), encoded.begin(), encoded.end());
    }
    return result;
}

std::string textOf(const ::bunch::Builder &builder)
{
    return asBunch(builder.toBunch().getBytes(::bunch::charset::utf8));
}

std::string textOf(const ::bunch::Buffer &buffer)
{
    return asBunch(buffer.data());
}

const std::string utf8Sample = std::string("A") + "\xC2\xA9" + "\xE2\x82\xAC" + "\xF0\x9F\x98\x80";
} // namespace

TEST_CASE("charset stream round trip")
{
    const std::array<std::pair<::bunch::charset, const char *>, 9> values{{
        {::bunch::charset::ascii, "ascii"},
        {::bunch::charset::utf8, "utf8"},
        {::bunch::charset::utf16, "utf16"},
        {::bunch::charset::utf16be, "utf16be"},
        {::bunch::charset::utf16le, "utf16le"},
        {::bunch::charset::utf32, "utf32"},
        {::bunch::charset::utf32be, "utf32be"},
        {::bunch::charset::utf32le, "utf32le"},
        {::bunch::charset::latin1, "latin1"},
    }};

    ::bunch::charset parsed = ::bunch::charset::ascii;
    for (const auto &entry : values)
    {
        std::ostringstream output;
        output << entry.first;
        CHECK_EQ(output.str(), std::string(entry.second));

        std::istringstream input(entry.second);
        parsed = ::bunch::charset::ascii;
        input >> parsed;
        CHECK(parsed == entry.first);
        CHECK_FALSE(input.fail());
    }

    std::istringstream invalid("utf7");
    invalid >> parsed;
    CHECK(invalid.fail());
}

TEST_CASE("sequenceLength reports code unit widths")
{
    using ::bunch::charset;
    CHECK_EQ(::bunch::sequenceLength(0x41, charset::utf8), 1u);
    CHECK_EQ(::bunch::sequenceLength(0x80, charset::utf8), 0u);
    CHECK_EQ(::bunch::sequenceLength(0xC1, charset::utf8), 0u);
    CHECK_EQ(::bunch::sequenceLength(0xC2, charset::utf8), 2u);
    CHECK_EQ(::bunch::sequenceLength(0xDF, charset::utf8), 2u);
    CHECK_EQ(::bunch::sequenceLength(0xE0, charset::utf8), 3u);
    CHECK_EQ(::bunch::sequenceLength(0xEF, charset::utf8), 3u);
    CHECK_EQ(::bunch::sequenceLength(0xF0, charset::utf8), 4u);
    CHECK_EQ(::bunch::sequenceLength(0xF4, charset::utf8), 4u);
    CHECK_EQ(::bunch::sequenceLength(0xF5, charset::utf8), 0u);
    CHECK_EQ(::bunch::sequenceLength(0xFF, charset::utf8), 0u);

    CHECK_EQ(::bunch::sequenceLength(0x00, charset::utf16), 2u);
    CHECK_EQ(::bunch::sequenceLength(0x00, charset::utf16le), 2u);
    CHECK_EQ(::bunch::sequenceLength(0x00, charset::utf16be), 2u);
    CHECK_EQ(::bunch::sequenceLength(0x00, charset::utf32), 4u);
    CHECK_EQ(::bunch::sequenceLength(0x00, charset::utf32le), 4u);
    CHECK_EQ(::bunch::sequenceLength(0x00, charset::utf32be), 4u);
    CHECK_EQ(::bunch::sequenceLength(0x00, charset::ascii), 1u);
    CHECK_EQ(::bunch::sequenceLength(0x00, charset::latin1), 1u);
}

TEST_CASE("UTF-8 encode and decode")
{
    using ::bunch::charset;
    CHECK_EQ(::bunch::encode(0x41, charset::utf8), makeBytes({0x41}));
    CHECK_EQ(::bunch::encode(0xA9, charset::utf8), makeBytes({0xC2, 0xA9}));
    CHECK_EQ(::bunch::encode(0x20AC, charset::utf8), makeBytes({0xE2, 0x82, 0xAC}));
    CHECK_EQ(::bunch::encode(0x1F600, charset::utf8), makeBytes({0xF0, 0x9F, 0x98, 0x80}));
    CHECK_EQ(::bunch::encode(0xD800, charset::utf8), makeBytes({0xEF, 0xBF, 0xBD}));
    CHECK_EQ(::bunch::encode(0x110000, charset::utf8), makeBytes({0xEF, 0xBF, 0xBD}));

    CHECK_EQ(decodeAll(utf8Sample, charset::utf8), makeCodepoints({0x41, 0xA9, 0x20AC, 0x1F600}));
    CHECK_EQ(encodeAll({0x41, 0xA9, 0x20AC, 0x1F600}, charset::utf8), makeBytes({0x41, 0xC2, 0xA9, 0xE2, 0x82, 0xAC, 0xF0, 0x9F, 0x98, 0x80}));
}

TEST_CASE("UTF-16 encode and decode")
{
    using ::bunch::charset;
    CHECK_EQ(::bunch::encode(0x41, charset::utf16le), makeBytes({0x41, 0x00}));
    CHECK_EQ(::bunch::encode(0xA9, charset::utf16le), makeBytes({0xA9, 0x00}));
    CHECK_EQ(::bunch::encode(0x20AC, charset::utf16le), makeBytes({0xAC, 0x20}));
    CHECK_EQ(::bunch::encode(0x1F600, charset::utf16le), makeBytes({0x3D, 0xD8, 0x00, 0xDE}));
    CHECK_EQ(::bunch::encode(0xD800, charset::utf16le), makeBytes({0xFD, 0xFF}));
    CHECK_EQ(::bunch::encode(0x110000, charset::utf16le), makeBytes({0xFD, 0xFF}));

    CHECK_EQ(::bunch::encode(0x41, charset::utf16be), makeBytes({0x00, 0x41}));
    CHECK_EQ(::bunch::encode(0x20AC, charset::utf16be), makeBytes({0x20, 0xAC}));
    CHECK_EQ(::bunch::encode(0x1F600, charset::utf16be), makeBytes({0xD8, 0x3D, 0xDE, 0x00}));
    CHECK_EQ(::bunch::encode(0xDC00, charset::utf16be), makeBytes({0xFF, 0xFD}));

    const Bytes littleEndian = encodeAll({0x41, 0xA9, 0x20AC, 0x1F600}, charset::utf16le);
    CHECK_EQ(decodeAll(asBunch(littleEndian), charset::utf16le), makeCodepoints({0x41, 0xA9, 0x20AC, 0x1F600}));

    const Bytes bigEndian = encodeAll({0x41, 0xA9, 0x20AC, 0x1F600}, charset::utf16be);
    CHECK_EQ(decodeAll(asBunch(bigEndian), charset::utf16be), makeCodepoints({0x41, 0xA9, 0x20AC, 0x1F600}));

    // The unmarked UTF-16 encoding scheme defaults to big-endian when no BOM
    // is present. This implementation does not strip a BOM.
    CHECK_EQ(::bunch::encode(0x41, charset::utf16), makeBytes({0x00, 0x41}));
    CHECK_EQ(::bunch::encode(0x1F600, charset::utf16), makeBytes({0xD8, 0x3D, 0xDE, 0x00}));
    CHECK_EQ(decodeAll(std::string_view("\x00\x41", 2), charset::utf16), makeCodepoints({0x41}));
}

TEST_CASE("UTF-32 encode and decode")
{
    using ::bunch::charset;
    CHECK_EQ(::bunch::encode(0x41, charset::utf32le), makeBytes({0x41, 0x00, 0x00, 0x00}));
    CHECK_EQ(::bunch::encode(0x1F600, charset::utf32le), makeBytes({0x00, 0xF6, 0x01, 0x00}));
    CHECK_EQ(::bunch::encode(0x41, charset::utf32be), makeBytes({0x00, 0x00, 0x00, 0x41}));
    CHECK_EQ(::bunch::encode(0x1F600, charset::utf32be), makeBytes({0x00, 0x01, 0xF6, 0x00}));

    const Bytes littleEndian = encodeAll({0x41, 0xA9, 0x20AC, 0x1F600}, charset::utf32le);
    CHECK_EQ(decodeAll(asBunch(littleEndian), charset::utf32le), makeCodepoints({0x41, 0xA9, 0x20AC, 0x1F600}));

    const Bytes bigEndian = encodeAll({0x41, 0xA9, 0x20AC, 0x1F600}, charset::utf32be);
    CHECK_EQ(decodeAll(asBunch(bigEndian), charset::utf32be), makeCodepoints({0x41, 0xA9, 0x20AC, 0x1F600}));

    // Unmarked UTF-32 defaults to big-endian. Invalid scalar values are
    // replaced instead of being emitted as ill-formed UTF-32.
    CHECK_EQ(::bunch::encode(0x41, charset::utf32), makeBytes({0x00, 0x00, 0x00, 0x41}));
    CHECK_EQ(::bunch::encode(0xD800, charset::utf32le), makeBytes({0xFD, 0xFF, 0x00, 0x00}));
    CHECK_EQ(::bunch::encode(0xD800, charset::utf32), makeBytes({0x00, 0x00, 0xFF, 0xFD}));
    CHECK_EQ(::bunch::encode(0x110000, charset::utf32le), makeBytes({0xFD, 0xFF, 0x00, 0x00}));
    CHECK_EQ(decodeAll(std::string_view("\x00\x00\x00\x41", 4), charset::utf32), makeCodepoints({0x41}));
}

TEST_CASE("ASCII and Latin-1 encode and decode")
{
    using ::bunch::charset;
    CHECK_EQ(::bunch::encode(0x41, charset::ascii), makeBytes({0x41}));
    CHECK_EQ(::bunch::encode(0x7F, charset::ascii), makeBytes({0x7F}));
    CHECK_EQ(::bunch::encode(0x80, charset::ascii), makeBytes({'?'}));
    CHECK_EQ(::bunch::encode(0x20AC, charset::ascii), makeBytes({'?'}));

    CHECK_EQ(::bunch::encode(0x41, charset::latin1), makeBytes({0x41}));
    CHECK_EQ(::bunch::encode(0xE9, charset::latin1), makeBytes({0xE9}));
    CHECK_EQ(::bunch::encode(0xFF, charset::latin1), makeBytes({0xFF}));
    CHECK_EQ(::bunch::encode(0x100, charset::latin1), makeBytes({'?'}));

    CHECK_EQ(decodeAll(std::string_view("A\xE9", 2), charset::latin1), makeCodepoints({0x41, 0xE9}));
    CHECK_EQ(decodeAll(std::string_view("A\x80", 2), charset::ascii), makeCodepoints({0x41, 0xFFFD}));
}

TEST_CASE("invalid byte sequences produce replacement characters")
{
    using ::bunch::charset;

    {
        std::string_view input("\x80", 1);
        CHECK_EQ(::bunch::decode(input, charset::utf8), 0xFFFDu);
        CHECK(input.empty());
    }
    {
        std::string_view input("\xE2\x82", 2);
        CHECK_EQ(::bunch::decode(input, charset::utf8), 0xFFFDu);
        CHECK(input.empty());
    }
    {
        std::string_view input("\xE2\x28\xA1", 3);
        CHECK_EQ(::bunch::decode(input, charset::utf8), 0xFFFDu);
        CHECK_EQ(input, std::string_view("\x28\xA1", 2));
    }
    {
        // Maximal subpart: consume the lead byte and the valid continuation
        // byte, leaving the offending ASCII byte for the next iteration.
        std::string_view input("\xE1\x80\x41", 3);
        CHECK_EQ(::bunch::decode(input, charset::utf8), 0xFFFDu);
        CHECK_EQ(input, std::string_view("\x41", 1));
    }
    {
        std::string_view input("\xF0\x90\x80\x41", 4);
        CHECK_EQ(::bunch::decode(input, charset::utf8), 0xFFFDu);
        CHECK_EQ(input, std::string_view("\x41", 1));
    }
    {
        std::string_view input("\xF0\x90\x41\x42", 4);
        CHECK_EQ(::bunch::decode(input, charset::utf8), 0xFFFDu);
        CHECK_EQ(input, std::string_view("\x41\x42", 2));
    }
    {
        std::string_view input("\xED\xA0\x80", 3);
        CHECK_EQ(::bunch::decode(input, charset::utf8), 0xFFFDu);
        CHECK(input.empty());
    }
    {
        std::string_view input("\xF0\x9F\x98", 3);
        CHECK_EQ(::bunch::decode(input, charset::utf8), 0xFFFDu);
        CHECK(input.empty());
    }
    {
        std::string_view input("\xC0\x80", 2);
        CHECK_EQ(::bunch::decode(input, charset::utf8), 0xFFFDu);
        CHECK_EQ(input, std::string_view("\x80", 1));
    }
    {
        std::string_view input("\x3D\xD8", 2);
        CHECK_EQ(::bunch::decode(input, charset::utf16le), 0xFFFDu);
        CHECK(input.empty());
    }
    {
        std::string_view input("\x3D\xD8\x41\x00", 4);
        CHECK_EQ(::bunch::decode(input, charset::utf16le), 0xFFFDu);
        CHECK_EQ(input, std::string_view("\x41\x00", 2));
    }
    {
        std::string_view input("\x00\xDC", 2);
        CHECK_EQ(::bunch::decode(input, charset::utf16le), 0xFFFDu);
        CHECK(input.empty());
    }
    {
        std::string_view input("\x00\x00\xD8\x00", 4);
        CHECK_EQ(::bunch::decode(input, charset::utf32le), 0xFFFDu);
        CHECK(input.empty());
    }
    {
        std::string_view input("\x00\x00\x11\x00", 4);
        CHECK_EQ(::bunch::decode(input, charset::utf32le), 0xFFFDu);
        CHECK(input.empty());
    }
    {
        std::string_view input;
        CHECK_EQ(::bunch::decode(input, charset::utf8), 0xFFFDu);
        CHECK(input.empty());
    }
}

TEST_CASE("convert writes into string, vector and deque")
{
    using ::bunch::charset;
    const std::string_view source = utf8Sample;

    CHECK_EQ(::bunch::convert(source, charset::utf8, charset::utf8), utf8Sample);
    CHECK_EQ(::bunch::convert(source, charset::utf8, charset::utf16le),
             asBunch(makeBytes({0x41, 0x00, 0xA9, 0x00, 0xAC, 0x20, 0x3D, 0xD8, 0x00, 0xDE})));

    Bytes vectorOutput;
    ::bunch::convert(source, charset::utf8, charset::utf32be, vectorOutput);
    CHECK_EQ(vectorOutput, encodeAll({0x41, 0xA9, 0x20AC, 0x1F600}, charset::utf32be));

    std::deque<unsigned char> dequeOutput;
    ::bunch::convert(asBunch(encodeAll({0x41, 0x20AC}, charset::utf16be)), charset::utf16be, charset::utf8, dequeOutput);
    CHECK_EQ(Bytes(dequeOutput.begin(), dequeOutput.end()), makeBytes({0x41, 0xE2, 0x82, 0xAC}));

    Bytes copied;
    ::bunch::convert(source, charset::utf8, charset::utf8, copied);
    CHECK_EQ(copied, Bytes(source.begin(), source.end()));
}

TEST_CASE("Meta and Point expose code point metadata")
{
    using ::bunch::Meta;
    using ::bunch::Point;

    Meta meta(7);
    CHECK_EQ(meta.getValue(), 7u);
    CHECK(meta == Meta(7));
    CHECK(meta != Meta(8));
    CHECK(Meta(7) < Meta(8));
    CHECK_EQ(std::hash<Meta>{}(meta), std::hash<Meta>{}(Meta(7)));

    Point letter(0x41);
    CHECK_EQ(letter.getValue(), 0x41u);
    CHECK_FALSE(letter.isSurrogate());
    CHECK(letter.isValidCodePoint());
    CHECK(letter == Point(0x41));
    CHECK(letter < Point(0x42));
    CHECK_EQ(std::hash<Point>{}(letter), std::hash<Point>{}(Point(0x41)));

    Point highSurrogate(0xD800);
    Point lowSurrogate(0xDFFF);
    CHECK(highSurrogate.isSurrogate());
    CHECK(lowSurrogate.isSurrogate());
    CHECK_FALSE(highSurrogate.isValidCodePoint());
    CHECK_FALSE(highSurrogate.isScalarValue());
    CHECK_FALSE(Point(0x110000).isValidCodePoint());
    CHECK_FALSE(Point(0x110000).isUnicodeCodePoint());
    CHECK(Point(0x10FFFF).isValidCodePoint());
    CHECK(Point(0x10FFFF).isScalarValue());
    CHECK(Point(0xFDD0).isNoncharacter());
    CHECK(Point(0xFFFF).isNoncharacter());
    CHECK_FALSE(Point(0x41).isNoncharacter());

    CHECK((std::is_same_v<::bunch::CodePoint, Point>));
    CHECK((std::is_class_v<::bunch::Character>));
    CHECK_FALSE((std::is_same_v<::bunch::Character, Point>));

    ::bunch::Character character(Point(0x41), ::bunch::Meta(7));
    CHECK_EQ(character.getValue(), 0x41u);
    CHECK(character.getPoint() == Point(0x41));
    CHECK_EQ(character.getMeta().getValue(), 7u);
    CHECK(character == ::bunch::Character(Point(0x41), ::bunch::Meta(7)));
    CHECK(character != ::bunch::Character(Point(0x41), ::bunch::Meta(8)));
    CHECK_EQ(std::hash<::bunch::Character>{}(character),
             std::hash<::bunch::Character>{}(::bunch::Character(Point(0x41), ::bunch::Meta(7))));
}

TEST_CASE("Bunch constructors and accessors")
{
    using ::bunch::Bunch;
    using ::bunch::charset;
    using ::bunch::Meta;
    using ::bunch::Point;

    Bunch empty;
    CHECK(empty.empty());
    CHECK_EQ(empty.size(), 0u);
    CHECK_EQ(empty.encoding(), charset::utf8);

    Bunch literal("hello");
    CHECK_EQ(literal.size(), 5u);
    CHECK(same(literal, "hello"));
    CHECK(literal == "hello");
    CHECK(literal != "world");
    CHECK(same(literal + "!", "hello!"));
    CHECK(same(">" + literal, ">hello"));

    Bunch converted(std::string_view(asBunch(makeBytes({0x41, 0x00, 0xAC, 0x20}))), charset::utf16le, charset::utf8);
    CHECK_EQ(converted.size(), 2u);
    CHECK(same(converted, "A\xE2\x82\xAC"));
    CHECK_EQ(converted.encoding(), charset::utf8);

    const Point points[]{Point(0x41), Point(0x20AC)};
    Bunch fromPoints(points, 2, charset::utf16be);
    CHECK_EQ(fromPoints.size(), 2u);
    CHECK_EQ(fromPoints.encoding(), charset::utf16be);
    CHECK_EQ(fromPoints.at(0).getValue(), 0x41u);
    CHECK_EQ(fromPoints.at(1).getValue(), 0x20ACu);

    const Meta metas[]{Meta(0x41), Meta(0x42), Meta(0x43)};
    Bunch fromMetas(metas, 3, charset::utf8);
    CHECK_EQ(fromMetas.size(), 3u);
    CHECK(same(fromMetas, "ABC"));

    const Meta utf16Metas[]{Meta(0x41), Meta(0x00), Meta(0xAC), Meta(0x20)};
    Bunch fromUtf16Metas(utf16Metas, 4, charset::utf16le, charset::utf8);
    CHECK_EQ(fromUtf16Metas.size(), 2u);
    CHECK(same(fromUtf16Metas, "A\xE2\x82\xAC"));

    CHECK_EQ(Bunch(static_cast<const ::bunch::Meta *>(nullptr), 0, charset::utf8).size(), 0u);
    CHECK_EQ(Bunch(static_cast<const Point *>(nullptr), 0).size(), 0u);

    Bunch copy(literal);
    CHECK(copy == literal);
    Bunch assigned;
    assigned = converted;
    CHECK(assigned == converted);
    Bunch moved(std::move(copy));
    CHECK(same(moved, "hello"));
    Bunch moveAssigned;
    moveAssigned = std::move(moved);
    CHECK(same(moveAssigned, "hello"));

    CHECK_EQ(literal.at(0).getValue(), 0x68u);
    CHECK_EQ(literal[1].getValue(), 0x65u);
    CHECK_THROWS_AS(literal.at(5), std::out_of_range);
    CHECK_THROWS_AS(literal[99], std::out_of_range);
}

TEST_CASE("Bunch sub repeat and concat")
{
    using ::bunch::Bunch;
    using ::bunch::charset;

    Bunch text("A\xC2\xA9\xE2\x82\xAC");
    CHECK(same(text.sub(1, 2), "\xC2\xA9\xE2\x82\xAC"));
    CHECK(text.sub(0, 99) == text);
    CHECK(text.sub(3, 1).empty());
    CHECK(text.sub(99, 2).empty());
    CHECK(same(text.sub(0, 0), ""));

    CHECK(same(text.repeat(2), "A\xC2\xA9\xE2\x82\xAC" "A\xC2\xA9\xE2\x82\xAC"));
    CHECK(text.repeat(0).empty());

    Bunch utf16(std::string_view("\x41\x00", 2), charset::utf16le, charset::utf16le);
    CHECK_EQ(utf16.sub(0, 1).encoding(), charset::utf16le);
    CHECK_EQ(utf16.repeat(2).encoding(), charset::utf16le);

    Bunch left("foo");
    Bunch right("bar");
    CHECK(same(left.concat(right), "foobar"));
    CHECK(same(left + right, "foobar"));
    CHECK(same(std::string_view("x") + left, "xfoo"));
    Bunch appended("foo");
    appended += right;
    CHECK(same(appended, "foobar"));
    appended += std::string_view("!");
    CHECK(same(appended, "foobar!"));

    CHECK_EQ(appended.encoding(), charset::utf8);
}

TEST_CASE("Bunch search operations")
{
    using ::bunch::Bunch;

    Bunch text("ababa");
    CHECK(text.startsWith(Bunch("ab")));
    CHECK(text.startsWith(std::string_view("ab")));
    CHECK(text.startsWith(Bunch()));
    CHECK_FALSE(text.startsWith(std::string_view("ba")));
    CHECK_FALSE(text.startsWith(std::string_view("ababab")));

    CHECK(text.endsWith(Bunch("ba")));
    CHECK(text.endsWith(std::string_view("ba")));
    CHECK(text.endsWith(Bunch()));
    CHECK_FALSE(text.endsWith(std::string_view("ab")));

    CHECK(text.contains(Bunch("bab")));
    CHECK(text.contains(std::string_view("bab")));
    CHECK(text.contains(Bunch()));
    CHECK_FALSE(text.contains(std::string_view("bb")));

    CHECK_EQ(text.indexOf(Bunch("ba")), 1u);
    CHECK_EQ(text.indexOf(std::string_view("ba")), 1u);
    CHECK_EQ(text.indexOf(Bunch("ba"), 2), 3u);
    CHECK_EQ(text.indexOf(std::string_view("ba"), 4), static_cast<std::size_t>(-1));
    CHECK_EQ(text.indexOf(Bunch("zz")), static_cast<std::size_t>(-1));
    CHECK_EQ(text.indexOf(Bunch()), 0u);
    CHECK_EQ(Bunch("abc").indexOf(Bunch("abcd")), static_cast<std::size_t>(-1));

    CHECK_EQ(text.lastIndexOf(Bunch("ba")), 3u);
    CHECK_EQ(text.lastIndexOf(std::string_view("ba")), 3u);
    CHECK_EQ(text.lastIndexOf(Bunch("ba"), 2), 1u);
    CHECK_EQ(text.lastIndexOf(Bunch("zz")), static_cast<std::size_t>(-1));
    CHECK_EQ(text.lastIndexOf(Bunch()), text.size());
    CHECK_EQ(text.lastIndexOf(Bunch("ba"), 99), static_cast<std::size_t>(-1));
}

TEST_CASE("Bunch replace and count")
{
    using ::bunch::Bunch;

    Bunch text("ababa");
    CHECK(same(text.replace(Bunch("ba"), Bunch("X")), "aXX"));
    CHECK(same(text.replace(Bunch("ba"), Bunch("X"), 1), "aXba"));
    CHECK(same(text.replace(Bunch("ba"), Bunch("X"), 0), "ababa"));
    CHECK(same(text.replace(Bunch(), Bunch("X")), "ababa"));
    CHECK(same(text.replace(Bunch("a"), Bunch()), "bb"));
    CHECK(same(text.replace(std::string_view("ba"), std::string_view("X")), "aXX"));
    CHECK(same(text.replace(std::string_view("ba"), std::string_view("X"), 1), "aXba"));

    CHECK_EQ(text.count(Bunch("ba")), 2u);
    CHECK_EQ(text.count(std::string_view("ba")), 2u);
    CHECK_EQ(text.count(Bunch("a")), 3u);
    CHECK_EQ(text.count(Bunch()), 0u);

    Bunch utf16(std::string_view("\x41\x00\x42\x00", 4), ::bunch::charset::utf16le, ::bunch::charset::utf16le);
    CHECK_EQ(utf16.replace(Bunch("A"), Bunch("C")).encoding(), ::bunch::charset::utf16le);
}

TEST_CASE("Bunch trim case and reverse")
{
    using ::bunch::Bunch;

    CHECK(same(Bunch(" \t abc \r\n").trim(), "abc"));
    CHECK(same(Bunch("\t\r\n ").trim(), ""));
    CHECK(same(Bunch("abc"), "abc"));
    CHECK(same(Bunch("AbC\xC3\x87"), "AbC\xC3\x87"));

    // Unicode White_Space includes NBSP (U+00A0), ideographic space (U+3000),
    // and thin space (U+2009). ZERO WIDTH SPACE (U+200B) is not White_Space.
    CHECK(same(Bunch("\xC2\xA0" "abc" "\xE3\x80\x80").trim(), "abc"));
    CHECK(same(Bunch("\xE2\x80\x89" "abc" "\xE2\x80\xA8").trim(), "abc"));
    CHECK(same(Bunch("\xE2\x80\x8B" "abc").trim(), "\xE2\x80\x8B" "abc"));

    CHECK(same(Bunch("AbC\xC3\x87").toUpperCase(), "ABC\xC3\x87"));
    CHECK(same(Bunch("AbC\xC3\x87").toLowerCase(), "abc\xC3\x87"));
    // ASCII-only case conversion: U+00DF LATIN SMALL LETTER SHARP S is not 'A'..'Z'.
    CHECK(same(Bunch("\xC3\x9F").toUpperCase(), "\xC3\x9F"));

    CHECK(same(Bunch("A\xC2\xA9\xE2\x82\xAC").reverse(), "\xE2\x82\xAC\xC2\xA9" "A"));
    CHECK(Bunch().reverse().empty());
}

TEST_CASE("Bunch split and join")
{
    using ::bunch::Bunch;

    const auto parts = Bunch("a,b,,c").split(Bunch(","));
    CHECK_EQ(parts.size(), 4u);
    CHECK(same(parts[0], "a"));
    CHECK(same(parts[1], "b"));
    CHECK(same(parts[2], ""));
    CHECK(same(parts[3], "c"));

    const auto boundaries = Bunch(",a,").split(Bunch(","));
    CHECK_EQ(boundaries.size(), 3u);
    CHECK(same(boundaries[0], ""));
    CHECK(same(boundaries[1], "a"));
    CHECK(same(boundaries[2], ""));

    const auto none = Bunch("abc").split(Bunch(","));
    CHECK_EQ(none.size(), 1u);
    CHECK(same(none[0], "abc"));
    CHECK(Bunch().split(Bunch(",")).empty());
    CHECK_THROWS_AS(Bunch("abc").split(Bunch()), std::invalid_argument);
    CHECK(Bunch().split(Bunch()).empty());

    const auto multi = Bunch("x--y--z").split(std::string_view("--"));
    CHECK_EQ(multi.size(), 3u);
    CHECK(same(multi[0], "x"));
    CHECK(same(multi[1], "y"));
    CHECK(same(multi[2], "z"));

    const std::vector<Bunch> values{Bunch("a"), Bunch("b"), Bunch("c")};
    CHECK(same(Bunch::join(values, Bunch(",")), "a,b,c"));
    CHECK(same(Bunch::join(values, std::string_view(" ")), "a b c"));
    CHECK(Bunch::join({}, Bunch(",")).empty());
}

TEST_CASE("Bunch compare and relational operators")
{
    using ::bunch::Bunch;

    CHECK_EQ(Bunch("abc").compare(Bunch("abc")), 0);
    CHECK_EQ(Bunch("abc").compare(Bunch("abd")), -1);
    CHECK_EQ(Bunch("abd").compare(Bunch("abc")), 1);
    CHECK_EQ(Bunch("abc").compare(Bunch("abcd")), -1);
    CHECK_EQ(Bunch("abcd").compare(Bunch("abc")), 1);
    CHECK_EQ(Bunch("abc").compare(std::string_view("abd")), -1);

    CHECK(Bunch("abc") == Bunch("abc"));
    CHECK(Bunch("abc") == std::string_view("abc"));
    CHECK(Bunch("abc") != Bunch("abd"));
    CHECK(Bunch("abc") != std::string_view("abd"));
    CHECK(Bunch("abc") < Bunch("abd"));
    CHECK(Bunch("abc") <= Bunch("abc"));
    CHECK(Bunch("abd") > Bunch("abc"));
    CHECK(Bunch("abd") >= Bunch("abd"));
}

TEST_CASE("Bunch byte point and stream access")
{
    using ::bunch::Bunch;
    using ::bunch::charset;

    Bunch text("A\xC2\xA9");
    CHECK_EQ(text.getBytes(), makeBytes({0x41, 0xC2, 0xA9}));
    CHECK_EQ(text.getBytes(charset::utf16le), makeBytes({0x41, 0x00, 0xA9, 0x00}));
    const std::vector<char> characters = text.getCharacters();
    CHECK_EQ(std::string(characters.begin(), characters.end()), "A\xC2\xA9");

    const std::vector<::bunch::Point> &points = text.getPoints();
    CHECK_EQ(points.size(), 2u);
    CHECK_EQ(points[1].getValue(), 0xA9u);

    const std::vector<::bunch::Meta> metas = text.getMetas();
    CHECK_EQ(metas.size(), 2u);
    CHECK_EQ(metas[1].getValue(), 0xA9u);

    std::ostringstream output;
    output << Bunch("\xE2\x82\xAC");
    CHECK_EQ(output.str(), std::string("\xE2\x82\xAC"));

    std::istringstream input("hello world\nignored");
    Bunch line;
    input >> line;
    CHECK(same(line, "hello world"));
}

TEST_CASE("Bunch iterator and hash")
{
    using ::bunch::Bunch;

    Bunch text("A\xC2\xA9\xE2\x82\xAC");
    CHECK_EQ(static_cast<std::size_t>(std::distance(text.begin(), text.end())), text.size());

    std::vector<unsigned int> values;
    for (const auto &point : text)
    {
        values.push_back(point.getValue());
    }
    CHECK_EQ(values, makeCodepoints({0x41, 0xA9, 0x20AC}));

    auto iterator = text.begin();
    CHECK_EQ(iterator->getValue(), 0x41u);
    CHECK_EQ((*iterator).getValue(), 0x41u);
    ++iterator;
    CHECK_EQ(iterator->getValue(), 0xA9u);
    iterator++;
    CHECK_EQ(iterator->getValue(), 0x20ACu);
    --iterator;
    CHECK_EQ(iterator->getValue(), 0xA9u);
    iterator--;
    CHECK(iterator == text.begin());

    auto end = text.end();
    --end;
    CHECK_EQ(end->getValue(), 0x20ACu);

    const Bunch utf16(std::string_view("\x41\x00\xA9\x00\xAC\x20", 6), ::bunch::charset::utf16le, ::bunch::charset::utf16le);
    CHECK_EQ(std::hash<Bunch>{}(text), std::hash<Bunch>{}(utf16));
    CHECK_EQ(std::hash<Bunch>{}(Bunch("abc")), std::hash<Bunch>{}(Bunch("abc")));
}

TEST_CASE("Builder construction and conversion")
{
    using ::bunch::Builder;
    using ::bunch::Bunch;
    using ::bunch::charset;
    using ::bunch::Point;

    Builder empty;
    CHECK(empty.empty());
    CHECK_EQ(empty.size(), 0u);
    CHECK_EQ(empty.encoding(), charset::utf8);

    Builder fromText(std::string_view("hello"));
    CHECK(same(fromText.toBunch(), "hello"));
    CHECK_EQ(fromText.size(), 5u);

    Builder converted(std::string_view("\xC3\xA9"), charset::utf8, charset::utf16le);
    CHECK_EQ(converted.encoding(), charset::utf16le);
    CHECK_EQ(converted.getBytes(), makeBytes({0xE9, 0x00}));
    CHECK_EQ(converted.toBunch().size(), 1u);
    CHECK(same(converted.toBunch(), "\xC3\xA9"));

    Builder custom(charset::utf32be);
    CHECK_EQ(custom.encoding(), charset::utf32be);
    custom.append(std::string_view("A"));
    CHECK_EQ(custom.getBytes(), makeBytes({0x00, 0x00, 0x00, 0x41}));

    Builder pointBuilder;
    pointBuilder.append(Point(0x20AC));
    CHECK_EQ(pointBuilder.getBytes(), makeBytes({0xE2, 0x82, 0xAC}));
    pointBuilder.prepend(Point(0x41));
    CHECK_EQ(pointBuilder.getBytes(), makeBytes({0x41, 0xE2, 0x82, 0xAC}));
}

TEST_CASE("Builder append primitive values")
{
    using ::bunch::Builder;
    using ::bunch::Bunch;

    Builder boolean;
    boolean.append(true);
    CHECK_EQ(textOf(boolean), "true");
    boolean.append(false);
    CHECK_EQ(textOf(boolean), "truefalse");

    Builder signedValues;
    signedValues.append(static_cast<short>(-2));
    signedValues.append(-3);
    signedValues.append(-4L);
    signedValues.append(-5LL);
    CHECK_EQ(textOf(signedValues), "-2-3-4-5");

    Builder unsignedValues;
    unsignedValues.append(static_cast<unsigned short>(2));
    unsignedValues.append(3u);
    unsignedValues.append(4UL);
    unsignedValues.append(5ULL);
    CHECK_EQ(textOf(unsignedValues), "2345");

    Builder floats;
    floats.append(3.5f);
    floats.append(2.25);
    floats.append(1.5L);
    CHECK_EQ(textOf(floats), "3.52.251.5");

    Builder characters;
    characters.append('A');
    characters.append(static_cast<unsigned char>('B'));
    CHECK_EQ(characters.getBytes(), makeBytes({0x41, 0x42}));

    Builder text;
    text.append(std::string_view("abc"));
    text.append(Bunch("def"));
    CHECK_EQ(textOf(text), "abcdef");

    Builder literalBuilder;
    literalBuilder.append("abc");
    literalBuilder.prepend("<<");
    literalBuilder.insert(0, ">>");
    CHECK_EQ(textOf(literalBuilder), ">><<abc");
    CHECK_EQ(textOf(literalBuilder + "!"), ">><<abc!");
    literalBuilder += "?";
    CHECK_EQ(textOf(literalBuilder), ">><<abc?");
}

TEST_CASE("Builder prepend and insert")
{
    using ::bunch::Builder;
    using ::bunch::Bunch;
    using ::bunch::Point;

    Builder prependBuilder;
    prependBuilder.append(std::string_view("world"));
    prependBuilder.prepend(std::string_view("hello "));
    CHECK_EQ(textOf(prependBuilder), "hello world");
    prependBuilder.prepend(Bunch(">> "));
    CHECK_EQ(textOf(prependBuilder), ">> hello world");
    prependBuilder.prepend(Point(0x20AC));
    CHECK_EQ(textOf(prependBuilder), "\xE2\x82\xAC" ">> hello world");
    prependBuilder.prepend('[');
    prependBuilder.prepend(static_cast<unsigned char>('<'));
    CHECK_EQ(textOf(prependBuilder), "<[\xE2\x82\xAC" ">> hello world");

    Builder insertBuilder;
    insertBuilder.append(std::string_view("abc"));
    insertBuilder.insert(1, std::string_view("XY"));
    CHECK_EQ(textOf(insertBuilder), "aXYbc");
    insertBuilder.insert(0, Bunch(">>"));
    CHECK_EQ(textOf(insertBuilder), ">>aXYbc");
    insertBuilder.insert(99, std::string_view("!"));
    CHECK_EQ(textOf(insertBuilder), ">>aXYbc!");
    insertBuilder.insert(2, '!');
    CHECK_EQ(textOf(insertBuilder), ">>!aXYbc!");
    insertBuilder.insert(0, static_cast<unsigned char>('?'));
    CHECK_EQ(textOf(insertBuilder), "?>>!aXYbc!");
    insertBuilder.insert(4, Point(0x20AC));
    CHECK_EQ(textOf(insertBuilder), "?>>!\xE2\x82\xAC" "aXYbc!");

    Builder intBuilder;
    intBuilder.insert(0, 42);
    intBuilder.insert(0, true);
    intBuilder.insert(0, 3.5);
    CHECK_EQ(textOf(intBuilder), "3.5true42");
}

TEST_CASE("Builder arithmetic stream and comparison")
{
    using ::bunch::Builder;
    using ::bunch::Bunch;

    Builder left;
    left.append(std::string_view("foo"));
    Builder right;
    right.append(std::string_view("bar"));

    CHECK_EQ(textOf(left + right), "foobar");
    CHECK_EQ(textOf(left + Bunch("!")), "foo!");
    CHECK_EQ(textOf(left + std::string_view("!")), "foo!");
    CHECK_EQ(textOf(std::string_view(">") + left), ">foo");

    Builder assigned;
    assigned = left;
    assigned += right;
    CHECK_EQ(textOf(assigned), "foobar");
    assigned += Bunch("!");
    assigned += std::string_view("?");
    CHECK_EQ(textOf(assigned), "foobar!?");

    std::ostringstream output;
    output << left;
    CHECK_EQ(output.str(), "foo");

    std::istringstream input("line\nrest");
    Builder line;
    input >> line;
    CHECK_EQ(textOf(line), "line");

    CHECK_EQ(std::hash<Builder>{}(left), std::hash<Builder>{}(Builder(std::string_view("foo"))));
    CHECK(std::less<Builder>{}(right, left));
    CHECK_FALSE(std::less<Builder>{}(left, right));
}

TEST_CASE("Buffer basic ring operations")
{
    using ::bunch::Buffer;

    Buffer empty;
    CHECK_EQ(empty.size(), 0u);
    CHECK_EQ(empty.capacity(), Buffer::defaultCapacity);
    CHECK(empty.data().empty());

    Buffer buffer(8);
    CHECK_EQ(buffer.capacity(), 8u);
    CHECK_EQ(buffer.size(), 0u);
    CHECK_EQ(buffer.write("hello", 5), 5u);
    CHECK_EQ(buffer.size(), 5u);
    CHECK_EQ(textOf(buffer), "hello");
    CHECK_EQ(buffer.peek(3), makeBytes({'h', 'e', 'l'}));
    CHECK_EQ(textOf(buffer), "hello");
    CHECK_EQ(buffer.read(2), makeBytes({'h', 'e'}));
    CHECK_EQ(buffer.size(), 3u);

    char peeked[4]{};
    CHECK_EQ(buffer.peek(peeked, 3), 3u);
    CHECK_EQ(std::string(peeked, 3), "llo");
    CHECK_EQ(buffer.size(), 3u);

    char readBuffer[4]{};
    CHECK_EQ(buffer.read(readBuffer, 2), 2u);
    CHECK_EQ(std::string(readBuffer, 2), "ll");
    CHECK_EQ(buffer.size(), 1u);

    CHECK_EQ(buffer.write(std::string_view("o")), 1u);
    CHECK_EQ(textOf(buffer), "oo");
    CHECK_EQ(buffer.read(10).size(), 2u);
    CHECK_EQ(buffer.size(), 0u);
    CHECK_EQ(buffer.read(10).size(), 0u);

    Buffer source(std::string_view("seed"));
    CHECK_EQ(textOf(source), "seed");
    CHECK_EQ(source.capacity(), 4u);

    Buffer zeroCapacity(0);
    CHECK_EQ(zeroCapacity.capacity(), Buffer::defaultCapacity);
}

TEST_CASE("Buffer wrap around growth and prepend")
{
    using ::bunch::Buffer;

    Buffer buffer(8);
    buffer.write(std::string_view("abcdef"));
    CHECK_EQ(buffer.read(4), makeBytes({'a', 'b', 'c', 'd'}));
    CHECK_EQ(buffer.write(std::string_view("ghijk")), 5u);
    CHECK_EQ(textOf(buffer), "efghijk");
    CHECK_EQ(buffer.size(), 7u);
    CHECK_EQ(buffer.read(7), makeBytes({'e', 'f', 'g', 'h', 'i', 'j', 'k'}));

    Buffer growing(8);
    growing.write(std::string_view("12345"));
    CHECK_EQ(growing.write(std::string_view("6789")), 4u);
    CHECK_EQ(growing.capacity(), 16u);
    CHECK_EQ(textOf(growing), "123456789");

    Buffer prependBuffer(8);
    prependBuffer.write(std::string_view("abc"));
    prependBuffer.prepend(std::string_view("XY"));
    CHECK_EQ(textOf(prependBuffer), "XYabc");
    prependBuffer.prepend(std::string_view("123456"));
    CHECK_EQ(textOf(prependBuffer), "123456XYabc");
    CHECK_EQ(prependBuffer.size(), 11u);

    prependBuffer.clear();
    CHECK_EQ(prependBuffer.size(), 0u);
    CHECK(prependBuffer.data().empty());

    Buffer shrink(32);
    shrink.write(std::string_view("abcd"));
    shrink.shrinkToFit();
    CHECK_EQ(shrink.size(), 4u);
    CHECK_EQ(shrink.capacity(), 4u);
    CHECK_EQ(textOf(shrink), "abcd");
    shrink.read(4);
    shrink.shrinkToFit();
    CHECK_EQ(shrink.capacity(), Buffer::defaultCapacity);
}

TEST_CASE("Buffer typed writes and stream operators")
{
    using ::bunch::Buffer;
    using ::bunch::Builder;
    using ::bunch::Bunch;

    Buffer numeric;
    numeric.write(true);
    numeric.write(static_cast<short>(-2));
    numeric.write(3);
    numeric.write(4L);
    numeric.write(5LL);
    numeric.write(3.5f);
    numeric.write(2.25);
    numeric.write(1.5L);
    numeric.write(static_cast<unsigned short>(6));
    numeric.write(7u);
    numeric.write(8UL);
    numeric.write(9ULL);
    CHECK_EQ(textOf(numeric), "true-23453.52.251.56789");

    Buffer characters;
    characters.write('A');
    characters.write(static_cast<unsigned char>('B'));
    CHECK_EQ(textOf(characters), "AB");
    characters.prepend('C');
    characters.prepend(static_cast<unsigned char>('D'));
    CHECK_EQ(textOf(characters), "DCAB");

    Buffer containers;
    containers.write(Bunch("ab"));
    containers.write(Builder(std::string_view("cd")));
    containers.write(std::vector<unsigned char>{'e', 'f'});
    CHECK_EQ(textOf(containers), "abcdef");
    containers.append(Bunch("gh"));
    containers.append(Builder(std::string_view("ij")));
    containers.append(std::vector<unsigned char>{'k', 'l'});
    CHECK_EQ(textOf(containers), "abcdefghijkl");

    CHECK_EQ(numeric.write(static_cast<const char *>(nullptr), 5), 0u);
    CHECK_EQ(numeric.prepend(static_cast<const char *>(nullptr), 5), 0u);
    CHECK_EQ(numeric.read(static_cast<char *>(nullptr), 0), 0u);

    Buffer streamed;
    streamed << "hello" << ' ' << 42 << ' ' << true;
    CHECK_EQ(textOf(streamed), "hello 42 true");

    Buffer literalBuffer;
    literalBuffer.write("abc");
    literalBuffer.append("def");
    literalBuffer.prepend("<<");
    CHECK_EQ(textOf(literalBuffer), "<<abcdef");
    literalBuffer << "!";
    CHECK_EQ(textOf(literalBuffer), "<<abcdef!");

    std::ostringstream output;
    output << streamed;
    CHECK_EQ(output.str(), "hello 42 true");

    std::string drained;
    streamed >> drained;
    CHECK_EQ(drained, "hello 42 true");
    CHECK_EQ(streamed.size(), 0u);

    std::istringstream input("from-stream\nnext");
    Buffer fromStream;
    input >> fromStream;
    CHECK_EQ(textOf(fromStream), "from-stream");
}

TEST_CASE("Buffer concatenation copy move and atomic access")
{
    using ::bunch::Buffer;

    Buffer left(std::string_view("Hello"));
    Buffer right(std::string_view("World"));
    CHECK_EQ(textOf(left + right), "HelloWorld");
    CHECK_EQ(textOf(left + std::string_view("!")), "Hello!");
    CHECK_EQ(textOf(std::string_view(">") + left), ">Hello");
    CHECK_EQ(textOf(left + left), "HelloHello");

    Buffer appended;
    appended += left;
    appended += right;
    appended += std::string_view("!");
    CHECK_EQ(textOf(appended), "HelloWorld!");

    Buffer selfAppended(std::string_view("ab"));
    selfAppended += selfAppended;
    CHECK_EQ(textOf(selfAppended), "abab");

    Buffer copied(left);
    CHECK_EQ(textOf(copied), "Hello");
    Buffer assigned;
    assigned = left;
    CHECK_EQ(textOf(assigned), "Hello");

    Buffer moved(std::move(copied));
    CHECK_EQ(textOf(moved), "Hello");
    moved.write(std::string_view("!"));
    CHECK_EQ(textOf(moved), "Hello!");
    CHECK_EQ(textOf(copied), "");

    Buffer moveAssigned;
    moveAssigned = std::move(moved);
    CHECK_EQ(textOf(moveAssigned), "Hello!");
    moveAssigned.write(std::string_view("?"));
    CHECK_EQ(textOf(moveAssigned), "Hello!?");

    Buffer atomicBuffer(std::string_view("atomic"));
    const std::size_t count = atomicBuffer.atomic([](std::vector<unsigned char> &, std::size_t &, std::size_t &, std::size_t &elementCount) -> std::size_t {
        return elementCount;
    });
    CHECK_EQ(count, 6u);

    const Buffer &constAtomicBuffer = atomicBuffer;
    const std::size_t constCount = constAtomicBuffer.atomic([](const std::vector<unsigned char> &, const std::size_t &, const std::size_t &, const std::size_t &elementCount) -> std::size_t {
        return elementCount;
    });
    CHECK_EQ(constCount, 6u);

    CHECK_EQ(std::hash<Buffer>{}(left), std::hash<Buffer>{}(Buffer(std::string_view("Hello"))));
    CHECK(std::less<Buffer>{}(Buffer(std::string_view("a")), Buffer(std::string_view("b"))));
    CHECK(std::less<Buffer>{}(Buffer(std::string_view("a")), Buffer(std::string_view("aa"))));
}

TEST_CASE("Buffer serializes concurrent writers")
{
    using ::bunch::Buffer;

    Buffer buffer(64);
    std::atomic<bool> start{false};
    std::vector<std::thread> writers;
    for (int index = 0; index < 4; ++index)
    {
        writers.emplace_back([&buffer, &start, index] {
            while (!start.load())
            {
            }
            std::string chunk(100, static_cast<char>('a' + index));
            buffer.write(chunk);
        });
    }

    start.store(true);
    for (std::thread &writer : writers)
    {
        writer.join();
    }

    CHECK_EQ(buffer.size(), 400u);
    const std::string data = textOf(buffer);
    CHECK_EQ(data.size(), 400u);
    for (char value = 'a'; value <= 'd'; ++value)
    {
        CHECK_EQ(static_cast<std::size_t>(std::count(data.begin(), data.end(), value)), 100u);
    }
}
