#pragma once

// ===========================================================================
// bunch/encoding_detect.h
//
// Byte order mark (BOM) identification and conservative encoding sniffing for
// the bunch codec.  decode() needs to be told an encoding before it can read
// anything; the helpers here answer the two questions that come first: does
// the buffer start with a BOM, and, when it does not, which encoding is the
// least surprising guess for it?
//
// A BOM is checked in the only order that is unambiguous:
//
//     FF FE 00 00   UTF-32 LE
//     00 00 FE FF   UTF-32 BE
//     EF BB BF      UTF-8
//     FF FE         UTF-16 LE
//     FE FF         UTF-16 BE
//
// UTF-32 must be tested before UTF-16: FF FE 00 00 starts with the UTF-16 LE
// BOM, so testing UTF-16 first would report UTF-16 LE and leave two U+0000
// code units behind after stripBom().  A BOM is the only fully reliable signal
// here and the only answer reported with certain == true.
//
// Without a BOM the detector is deliberately conservative:
//
//   1. Empty input is ambiguous.  It is reported as latin1 with bomLength == 0
//      and certain == false; an empty buffer decodes to an empty string under
//      every encoding, so callers that require UTF-8 can simply treat it as
//      UTF-8 themselves.
//   2. UTF-16 evidence: a zero byte on one side of (almost) every 16-bit unit,
//      at odd offsets for UTF-16 LE and at even offsets for UTF-16 BE.  At
//      least two whole units are required and the text must not contain
//      unpaired surrogates.  This test runs before the UTF-8 test on purpose:
//      BOM-less UTF-16 LE "A\0B\0" is also well-formed UTF-8 (U+0000 is a
//      valid character), so a pure UTF-8 validity check would claim it.
//   3. Well-formed UTF-8, including pure ASCII, is reported as charset::utf8.
//      ASCII is never reported as charset::ascii: it is a strict subset of
//      UTF-8, and decoding ASCII as UTF-8 is always correct.
//   4. Everything else is reported as charset::latin1.  Every byte string is
//      valid Latin-1, so the fallback cannot fail; it is the honest "unknown
//      single-byte encoding" answer, always with certain == false.
//
// Known risks, all signalled by certain == false:
//
//   - Short inputs carry little evidence.  A two-byte "A\0" is valid UTF-8 and
//     is below the UTF-16 threshold, and a lone 0xE9 byte is both incomplete
//     UTF-8 and a plausible Latin-1 'e' with an acute accent.
//   - UTF-16 text whose code units contain no zero byte (most CJK text, all
//     non-BMP text) is invisible to the NUL test and may be reported as UTF-8
//     or Latin-1 instead.  The reverse also holds: text made almost entirely
//     of surrogate pairs can put the NUL on the unexpected side of the pair
//     and invert the reported byte order.  Only a BOM or an out-of-band label
//     can settle such cases.
//   - Binary data can satisfy the UTF-16 pattern by chance.  The thresholds
//     below are chosen so that random bytes do not, but a structured binary
//     format might.
//   - UTF-32 without a BOM is not guessed at all.  Its NUL pattern overlaps
//     UTF-16 and single-byte encodings, so such input surfaces as UTF-8 or
//     Latin-1; a caller that needs UTF-32 support must supply the encoding.
//
// None of these functions allocates, reads outside the passed view, or changes
// the bytes.  stripBom() returns a view into the same buffer.
// ===========================================================================

#include "charset.h"

#include <cstddef>
#include <optional>
#include <string_view>

namespace bunch
{
// Result of detectEncoding().  bomLength is the size of the BOM that was found
// (0 when there is none) and certain is true only when a BOM decided the
// answer, i.e. never for a heuristic result.
struct EncodingDetection
{
    charset encoding;
    std::size_t bomLength;
    bool certain;
};

namespace detail
{
inline constexpr unsigned char bomUtf32Le[4] = {0xFF, 0xFE, 0x00, 0x00};
inline constexpr unsigned char bomUtf32Be[4] = {0x00, 0x00, 0xFE, 0xFF};
inline constexpr unsigned char bomUtf8[3] = {0xEF, 0xBB, 0xBF};
inline constexpr unsigned char bomUtf16Le[2] = {0xFF, 0xFE};
inline constexpr unsigned char bomUtf16Be[2] = {0xFE, 0xFF};

inline auto startsWith(std::string_view bytes, const unsigned char *prefix, std::size_t prefixLength) noexcept -> bool
{
    if (bytes.size() < prefixLength)
    {
        return false;
    }
    for (std::size_t index = 0; index < prefixLength; ++index)
    {
        if (static_cast<unsigned char>(bytes[index]) != prefix[index])
        {
            return false;
        }
    }
    return true;
}

constexpr auto isContinuationByte(unsigned char byte) noexcept -> bool
{
    return (byte & 0xC0) == 0x80;
}

// Strict UTF-8 well-formedness: rejects stray continuation bytes, overlong
// forms, surrogates and values above U+10FFFF.  NUL (0x00) is a valid UTF-8
// character and is accepted here; the caller decides what NULs mean.
inline auto isValidUtf8(std::string_view bytes) noexcept -> bool
{
    const auto *data = reinterpret_cast<const unsigned char *>(bytes.data());
    const std::size_t size = bytes.size();
    std::size_t index = 0;
    while (index < size)
    {
        const unsigned char lead = data[index];
        if (lead < 0x80)
        {
            ++index;
            continue;
        }

        std::size_t sequenceLength = 0;
        unsigned int codepoint = 0;
        if (lead >= 0xC2 && lead <= 0xDF)
        {
            sequenceLength = 2;
            codepoint = lead & 0x1F;
        }
        else if (lead >= 0xE0 && lead <= 0xEF)
        {
            sequenceLength = 3;
            codepoint = lead & 0x0F;
        }
        else if (lead >= 0xF0 && lead <= 0xF4)
        {
            sequenceLength = 4;
            codepoint = lead & 0x07;
        }
        else
        {
            return false;
        }

        if (size - index < sequenceLength)
        {
            return false;
        }
        for (std::size_t offset = 1; offset < sequenceLength; ++offset)
        {
            const unsigned char continuation = data[index + offset];
            if (!isContinuationByte(continuation))
            {
                return false;
            }
            codepoint = (codepoint << 6) | (continuation & 0x3F);
        }
        if ((sequenceLength == 2 && codepoint < 0x80) || (sequenceLength == 3 && codepoint < 0x800) ||
            (sequenceLength == 4 && codepoint < 0x10000) || codepoint > 0x10FFFF ||
            (codepoint >= 0xD800 && codepoint <= 0xDFFF))
        {
            return false;
        }
        index += sequenceLength;
    }
    return true;
}

// Reads one UTF-16 code unit from two bytes.  data must point at two bytes.
inline auto utf16Unit(const unsigned char *data, bool littleEndian) noexcept -> unsigned int
{
    if (littleEndian)
    {
        return static_cast<unsigned int>(data[0]) | (static_cast<unsigned int>(data[1]) << 8);
    }
    return (static_cast<unsigned int>(data[0]) << 8) | static_cast<unsigned int>(data[1]);
}

// Rejects unpaired surrogates, so a candidate is only accepted when the whole
// buffer parses as UTF-16.  bytes must have an even size.
inline auto isWellFormedUtf16(std::string_view bytes, bool littleEndian) noexcept -> bool
{
    const auto *data = reinterpret_cast<const unsigned char *>(bytes.data());
    const std::size_t unitCount = bytes.size() / 2;
    for (std::size_t index = 0; index < unitCount; ++index)
    {
        const unsigned int unit = utf16Unit(data + index * 2, littleEndian);
        if (unit >= 0xD800 && unit <= 0xDBFF)
        {
            if (index + 1 >= unitCount)
            {
                return false;
            }
            const unsigned int low = utf16Unit(data + (index + 1) * 2, littleEndian);
            if (low < 0xDC00 || low > 0xDFFF)
            {
                return false;
            }
            ++index;
        }
        else if (unit >= 0xDC00 && unit <= 0xDFFF)
        {
            return false;
        }
    }
    return true;
}

// Returns the byte order named by NUL positions, if any.  Latin-script UTF-16
// keeps one byte of every code unit at zero, so the side that holds the zero
// bytes identifies the byte order.  A minimum of two units and two zero bytes
// keeps stray NULs in short or binary input from tipping the decision.
inline auto sniffUtf16(std::string_view bytes) noexcept -> std::optional<charset>
{
    constexpr std::size_t minimumUnitCount = 2;
    constexpr std::size_t minimumZeroCount = 2;
    constexpr double zeroRatioThreshold = 0.5;
    constexpr double oppositeZeroRatioLimit = 0.1;

    if (bytes.size() < minimumUnitCount * 2 || bytes.size() % 2 != 0)
    {
        return std::nullopt;
    }

    const auto *data = reinterpret_cast<const unsigned char *>(bytes.data());
    const std::size_t unitCount = bytes.size() / 2;
    std::size_t evenZeroCount = 0;
    std::size_t oddZeroCount = 0;
    for (std::size_t index = 0; index < bytes.size(); index += 2)
    {
        if (data[index] == 0)
        {
            ++evenZeroCount;
        }
        if (data[index + 1] == 0)
        {
            ++oddZeroCount;
        }
    }

    const double evenZeroRatio = static_cast<double>(evenZeroCount) / static_cast<double>(unitCount);
    const double oddZeroRatio = static_cast<double>(oddZeroCount) / static_cast<double>(unitCount);

    // UTF-16 LE stores the high byte of a BMP code unit second, so Latin text
    // has its zero bytes at odd offsets; UTF-16 BE mirrors that.
    if (oddZeroCount >= minimumZeroCount && oddZeroRatio >= zeroRatioThreshold &&
        evenZeroRatio <= oppositeZeroRatioLimit && isWellFormedUtf16(bytes, true))
    {
        return charset::utf16le;
    }
    if (evenZeroCount >= minimumZeroCount && evenZeroRatio >= zeroRatioThreshold &&
        oddZeroRatio <= oppositeZeroRatioLimit && isWellFormedUtf16(bytes, false))
    {
        return charset::utf16be;
    }
    return std::nullopt;
}
} // namespace detail

// Returns the length of the complete BOM at the start of bytes, or 0 when
// there is none.  Truncated prefixes such as EF BB are not a BOM.
inline auto bomLength(std::string_view bytes) noexcept -> std::size_t
{
    // UTF-32 first: FF FE 00 00 also starts with the UTF-16 LE BOM.
    if (detail::startsWith(bytes, detail::bomUtf32Le, 4))
    {
        return 4;
    }
    if (detail::startsWith(bytes, detail::bomUtf32Be, 4))
    {
        return 4;
    }
    if (detail::startsWith(bytes, detail::bomUtf8, 3))
    {
        return 3;
    }
    if (detail::startsWith(bytes, detail::bomUtf16Le, 2))
    {
        return 2;
    }
    if (detail::startsWith(bytes, detail::bomUtf16Be, 2))
    {
        return 2;
    }
    return 0;
}

// True when bytes starts with any complete BOM recognised by bomLength().
inline auto hasBom(std::string_view bytes) noexcept -> bool
{
    return bomLength(bytes) != 0;
}

// Returns a view of bytes without its BOM.  The view points into the caller's
// buffer and the input is untouched; when there is no BOM the view is bytes
// itself.
inline auto stripBom(std::string_view bytes) noexcept -> std::string_view
{
    bytes.remove_prefix(bomLength(bytes));
    return bytes;
}

// Identifies the encoding of bytes, preferring a BOM and otherwise applying
// the conservative heuristics documented at the top of this header.
inline auto detectEncoding(std::string_view bytes) noexcept -> EncodingDetection
{
    const std::size_t length = bomLength(bytes);
    if (length != 0)
    {
        charset encoding = charset::utf8;
        if (length == 4)
        {
            // Both UTF-32 BOMs are four bytes: the little-endian one starts
            // with FF, the big-endian one with 00.
            encoding = static_cast<unsigned char>(bytes[0]) == 0xFF ? charset::utf32le : charset::utf32be;
        }
        else if (length == 2)
        {
            // The same split separates the two UTF-16 BOMs.
            encoding = static_cast<unsigned char>(bytes[0]) == 0xFF ? charset::utf16le : charset::utf16be;
        }
        // length == 3 is EF BB BF: UTF-8.
        return EncodingDetection{encoding, length, true};
    }

    if (bytes.empty())
    {
        // Nothing to look at; see the empty-input note above.
        return EncodingDetection{charset::latin1, 0, false};
    }

    if (const auto utf16 = detail::sniffUtf16(bytes))
    {
        return EncodingDetection{*utf16, 0, false};
    }

    if (detail::isValidUtf8(bytes))
    {
        return EncodingDetection{charset::utf8, 0, false};
    }

    return EncodingDetection{charset::latin1, 0, false};
}

// detectEncoding() plus stripBom() in one call.  detection is always assigned,
// including when no BOM is present.
inline auto detectAndStrip(std::string_view bytes, EncodingDetection &detection) noexcept -> std::string_view
{
    detection = detectEncoding(bytes);
    bytes.remove_prefix(detection.bomLength);
    return bytes;
}
} // namespace bunch
