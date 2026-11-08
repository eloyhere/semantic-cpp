#include "bunch/encoding_detect.h"

#include "test_framework.h"

#include <cstddef>
#include <initializer_list>
#include <string>
#include <string_view>

namespace
{
using bunch::charset;
using bunch::EncodingDetection;

auto makeBytes(std::initializer_list<unsigned char> values) -> std::string
{
    std::string bytes;
    bytes.reserve(values.size());
    for (const unsigned char value : values)
    {
        bytes.push_back(static_cast<char>(value));
    }
    return bytes;
}

auto makeUtf16Le(std::string_view text) -> std::string
{
    std::string bytes;
    bytes.reserve(text.size() * 2);
    for (const char character : text)
    {
        bytes.push_back(character);
        bytes.push_back('\0');
    }
    return bytes;
}

auto makeUtf16Be(std::string_view text) -> std::string
{
    std::string bytes;
    bytes.reserve(text.size() * 2);
    for (const char character : text)
    {
        bytes.push_back('\0');
        bytes.push_back(character);
    }
    return bytes;
}
} // namespace

TEST_CASE("encoding_detect: UTF-8 BOM")
{
    const std::string payload = "hello";
    const std::string bytes = makeBytes({0xEF, 0xBB, 0xBF}) + payload;

    const EncodingDetection detection = bunch::detectEncoding(bytes);
    CHECK_EQ(detection.encoding, charset::utf8);
    CHECK_EQ(detection.bomLength, std::size_t{3});
    CHECK(detection.certain);
    CHECK(bunch::hasBom(bytes));
    CHECK_EQ(bunch::bomLength(bytes), std::size_t{3});

    const std::string_view stripped = bunch::stripBom(bytes);
    CHECK_EQ(stripped, std::string_view(payload));
    CHECK_EQ(stripped.data(), bytes.data() + 3);
    CHECK_EQ(bytes, makeBytes({0xEF, 0xBB, 0xBF}) + payload);

    const std::string bomOnly = makeBytes({0xEF, 0xBB, 0xBF});
    CHECK(bunch::hasBom(bomOnly));
    CHECK(bunch::stripBom(bomOnly).empty());
}

TEST_CASE("encoding_detect: UTF-16 BOMs")
{
    const std::string payload = makeBytes({0x68, 0x00, 0x69, 0x00});
    const std::string littleEndian = makeBytes({0xFF, 0xFE}) + payload;
    const EncodingDetection le = bunch::detectEncoding(littleEndian);
    CHECK_EQ(le.encoding, charset::utf16le);
    CHECK_EQ(le.bomLength, std::size_t{2});
    CHECK(le.certain);
    CHECK_EQ(bunch::stripBom(littleEndian), std::string_view(payload));

    const std::string bigEndianPayload = makeBytes({0x00, 0x68, 0x00, 0x69});
    const std::string bigEndian = makeBytes({0xFE, 0xFF}) + bigEndianPayload;
    const EncodingDetection be = bunch::detectEncoding(bigEndian);
    CHECK_EQ(be.encoding, charset::utf16be);
    CHECK_EQ(be.bomLength, std::size_t{2});
    CHECK(be.certain);
    CHECK_EQ(bunch::stripBom(bigEndian), std::string_view(bigEndianPayload));
}

TEST_CASE("encoding_detect: UTF-32 BOMs win over UTF-16")
{
    const std::string lePayload = makeBytes({'a', 0x00, 0x00, 0x00});
    const std::string leBytes = makeBytes({0xFF, 0xFE, 0x00, 0x00}) + lePayload;
    const EncodingDetection le = bunch::detectEncoding(leBytes);
    CHECK_EQ(le.encoding, charset::utf32le);
    CHECK(le.encoding != charset::utf16le);
    CHECK_EQ(le.bomLength, std::size_t{4});
    CHECK(le.certain);
    CHECK_EQ(bunch::bomLength(leBytes), std::size_t{4});
    CHECK_EQ(bunch::stripBom(leBytes), std::string_view(lePayload));

    const std::string bePayload = makeBytes({0x00, 0x00, 0x00, 'a'});
    const std::string beBytes = makeBytes({0x00, 0x00, 0xFE, 0xFF}) + bePayload;
    const EncodingDetection be = bunch::detectEncoding(beBytes);
    CHECK_EQ(be.encoding, charset::utf32be);
    CHECK(be.encoding != charset::utf16be);
    CHECK_EQ(be.bomLength, std::size_t{4});
    CHECK(be.certain);
    CHECK_EQ(bunch::stripBom(beBytes), std::string_view(bePayload));

    // A UTF-32 BOM on its own is stripped completely.
    CHECK(bunch::stripBom(makeBytes({0xFF, 0xFE, 0x00, 0x00})).empty());
    CHECK(bunch::stripBom(makeBytes({0x00, 0x00, 0xFE, 0xFF})).empty());
}

TEST_CASE("encoding_detect: partial BOM prefixes are not BOMs")
{
    const std::string utf8Prefix = makeBytes({0xEF, 0xBB});
    CHECK_FALSE(bunch::hasBom(utf8Prefix));
    CHECK_EQ(bunch::bomLength(utf8Prefix), std::size_t{0});
    const EncodingDetection utf8PrefixDetection = bunch::detectEncoding(utf8Prefix);
    CHECK_EQ(utf8PrefixDetection.encoding, charset::latin1);
    CHECK_EQ(utf8PrefixDetection.bomLength, std::size_t{0});
    CHECK_FALSE(utf8PrefixDetection.certain);

    const std::string utf32BePrefix = makeBytes({0x00, 0x00, 0xFE});
    CHECK_FALSE(bunch::hasBom(utf32BePrefix));
    CHECK_EQ(bunch::detectEncoding(utf32BePrefix).encoding, charset::latin1);

    // Three bytes of the UTF-32 LE BOM are a complete UTF-16 LE BOM followed
    // by one payload byte: the shorter BOM wins, as the BOM order requires.
    const std::string truncatedUtf32Le = makeBytes({0xFF, 0xFE, 0x00});
    CHECK_EQ(bunch::bomLength(truncatedUtf32Le), std::size_t{2});
    CHECK_EQ(bunch::detectEncoding(truncatedUtf32Le).encoding, charset::utf16le);
    CHECK_EQ(bunch::stripBom(truncatedUtf32Le), std::string_view("\x00", 1));
}

TEST_CASE("encoding_detect: BOM-less ASCII and UTF-8")
{
    const std::string ascii = "plain ASCII text";
    const EncodingDetection asciiDetection = bunch::detectEncoding(ascii);
    CHECK_EQ(asciiDetection.encoding, charset::utf8);
    CHECK_EQ(asciiDetection.bomLength, std::size_t{0});
    CHECK_FALSE(asciiDetection.certain);
    CHECK_FALSE(bunch::hasBom(ascii));
    CHECK_EQ(bunch::stripBom(ascii), std::string_view(ascii));

    const std::string utf8 = makeBytes({0x68, 0xC3, 0xA9, 0x6C, 0x6C, 0x6F});
    const EncodingDetection utf8Detection = bunch::detectEncoding(utf8);
    CHECK_EQ(utf8Detection.encoding, charset::utf8);
    CHECK_FALSE(utf8Detection.certain);

    const std::string emoji = makeBytes({0xF0, 0x9F, 0x98, 0x80});
    CHECK_EQ(bunch::detectEncoding(emoji).encoding, charset::utf8);
}

TEST_CASE("encoding_detect: BOM-less UTF-16 from NUL distribution")
{
    const std::string littleEndian = makeUtf16Le("Hello");
    const EncodingDetection le = bunch::detectEncoding(littleEndian);
    CHECK_EQ(le.encoding, charset::utf16le);
    CHECK_EQ(le.bomLength, std::size_t{0});
    CHECK_FALSE(le.certain);

    const std::string bigEndian = makeUtf16Be("Hello");
    const EncodingDetection be = bunch::detectEncoding(bigEndian);
    CHECK_EQ(be.encoding, charset::utf16be);
    CHECK_EQ(be.bomLength, std::size_t{0});
    CHECK_FALSE(be.certain);

    // A NUL on both sides of a unit (here BOM-less UTF-32 LE "a") shows no
    // byte order and must not be forced into UTF-16.
    const std::string utf32Le = makeBytes({'a', 0x00, 0x00, 0x00});
    const EncodingDetection utf32Guess = bunch::detectEncoding(utf32Le);
    CHECK(utf32Guess.encoding != charset::utf16le);
    CHECK(utf32Guess.encoding != charset::utf16be);
}

TEST_CASE("encoding_detect: single-byte fallback")
{
    const std::string latin1 = makeBytes({0x63, 0x61, 0x66, 0xE9});
    const EncodingDetection latin1Detection = bunch::detectEncoding(latin1);
    CHECK_EQ(latin1Detection.encoding, charset::latin1);
    CHECK_EQ(latin1Detection.bomLength, std::size_t{0});
    CHECK_FALSE(latin1Detection.certain);

    // GBK "ni hao" (C4 E3 BA C3) is not well-formed UTF-8.
    const std::string gbk = makeBytes({0xC4, 0xE3, 0xBA, 0xC3});
    CHECK_EQ(bunch::detectEncoding(gbk).encoding, charset::latin1);

    const std::string highByte = makeBytes({0x80});
    CHECK_EQ(bunch::detectEncoding(highByte).encoding, charset::latin1);
}

TEST_CASE("encoding_detect: empty and short inputs")
{
    const std::string empty;
    const EncodingDetection emptyDetection = bunch::detectEncoding(empty);
    CHECK_EQ(emptyDetection.encoding, charset::latin1);
    CHECK_EQ(emptyDetection.bomLength, std::size_t{0});
    CHECK_FALSE(emptyDetection.certain);
    CHECK_FALSE(bunch::hasBom(empty));
    CHECK_EQ(bunch::bomLength(empty), std::size_t{0});
    CHECK(bunch::stripBom(empty).empty());

    const std::string oneAscii = makeBytes({0x41});
    CHECK_EQ(bunch::detectEncoding(oneAscii).encoding, charset::utf8);
    CHECK_FALSE(bunch::detectEncoding(oneAscii).certain);

    // Two bytes are below the UTF-16 evidence threshold, and NUL is valid
    // UTF-8, so "A\0" is reported as UTF-8 rather than guessed as UTF-16.
    const std::string asciiThenNul = makeBytes({0x41, 0x00});
    CHECK_EQ(bunch::detectEncoding(asciiThenNul).encoding, charset::utf8);

    const std::string oneLead = makeBytes({0xEF});
    CHECK_EQ(bunch::detectEncoding(oneLead).encoding, charset::latin1);

    const std::string partialEuro = makeBytes({0xE2, 0x82});
    CHECK_EQ(bunch::detectEncoding(partialEuro).encoding, charset::latin1);

    const std::string brokenThree = makeBytes({0xEF, 0xBB, 0x00});
    CHECK_EQ(bunch::detectEncoding(brokenThree).encoding, charset::latin1);
}

TEST_CASE("encoding_detect: detectAndStrip fills the detection")
{
    const std::string payload = "text";
    const std::string bytes = makeBytes({0xEF, 0xBB, 0xBF}) + payload;

    EncodingDetection detection{charset::latin1, 0, false};
    const std::string_view stripped = bunch::detectAndStrip(bytes, detection);
    CHECK_EQ(detection.encoding, charset::utf8);
    CHECK_EQ(detection.bomLength, std::size_t{3});
    CHECK(detection.certain);
    CHECK_EQ(stripped, std::string_view(payload));

    const std::string plain = "plain";
    const std::string_view unchanged = bunch::detectAndStrip(plain, detection);
    CHECK_EQ(detection.encoding, charset::utf8);
    CHECK_EQ(detection.bomLength, std::size_t{0});
    CHECK_FALSE(detection.certain);
    CHECK_EQ(unchanged.data(), plain.data());
    CHECK_EQ(unchanged.size(), plain.size());
}

TEST_CASE("encoding_detect: bom helpers agree with detection")
{
    const std::string samples[] = {
        makeBytes({}),
        makeBytes({0x00}),
        makeBytes({0xFF}),
        makeBytes({0xEF, 0xBB}),
        makeBytes({0xFF, 0xFE}),
        makeBytes({0xFE, 0xFF}),
        makeBytes({0xEF, 0xBB, 0xBF}),
        makeBytes({0xFF, 0xFE, 0x00}),
        makeBytes({0xFF, 0xFE, 0x00, 0x00}),
        makeBytes({0x00, 0x00, 0xFE, 0xFF}),
        makeBytes({0x00, 0x00, 0xFE}),
        makeBytes({0x41, 0x00, 0x42, 0x00}),
        "plain text",
    };

    for (const std::string &sample : samples)
    {
        const EncodingDetection detection = bunch::detectEncoding(sample);
        const std::size_t length = bunch::bomLength(sample);
        CHECK_EQ(length, detection.bomLength);
        CHECK_EQ(bunch::hasBom(sample), length != 0);
        CHECK_EQ(detection.certain, length != 0);

        const std::string_view stripped = bunch::stripBom(sample);
        CHECK_EQ(stripped.size(), sample.size() - length);
        if (!sample.empty())
        {
            CHECK_EQ(stripped.data(), sample.data() + length);
        }
    }
}

TEST_CASE("encoding_detect: input buffer is not modified")
{
    const std::string original = makeBytes({0xFF, 0xFE, 0x00, 0x00, 0x61, 0x00, 0x00, 0x00});
    std::string bytes = original;

    (void)bunch::detectEncoding(bytes);
    (void)bunch::stripBom(bytes);
    CHECK_EQ(bytes, original);
}
