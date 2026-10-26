#pragma once

#include "charset.h"

#include <cstddef>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

namespace bunch
{

namespace detail
{
constexpr bool isScalarValue(unsigned int codepoint) noexcept
{
    return codepoint <= 0x10FFFF && !(codepoint >= 0xD800 && codepoint <= 0xDFFF);
}
} // namespace detail

inline std::size_t sequenceLength(unsigned char byte, charset encoding)
{
    if (encoding == charset::utf8)
    {
        if (byte < 0x80)
            return 1;
        if (byte < 0xC2)
            return 0;
        if (byte < 0xE0)
            return 2;
        if (byte < 0xF0)
            return 3;
        if (byte < 0xF5)
            return 4;
        return 0;
    }
    else if (encoding == charset::utf16 || encoding == charset::utf16le || encoding == charset::utf16be)
    {
        return 2;
    }
    else if (encoding == charset::utf32 || encoding == charset::utf32le || encoding == charset::utf32be)
    {
        return 4;
    }
    return 1;
}

// Appends one code point to a UTF-8 string.  Invalid scalar values become the
// U+FFFD replacement character, like encode() does for the vector overload.
inline void appendUtf8(std::string &output, unsigned int codepoint)
{
    if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
    {
        codepoint = 0xFFFD;
    }
    if (codepoint <= 0x7F)
    {
        output.push_back(static_cast<char>(codepoint));
    }
    else if (codepoint <= 0x7FF)
    {
        output.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else if (codepoint <= 0xFFFF)
    {
        output.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else
    {
        output.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
}

inline std::vector<unsigned char> encode(unsigned int codepoint, charset encoding)
{
    std::vector<unsigned char> result;
    if (encoding == charset::utf8)
    {
        if (codepoint <= 0x7F)
        {
            result.push_back(static_cast<unsigned char>(codepoint));
        }
        else if (codepoint <= 0x7FF)
        {
            result.push_back(static_cast<unsigned char>(0xC0 | (codepoint >> 6)));
            result.push_back(static_cast<unsigned char>(0x80 | (codepoint & 0x3F)));
        }
        else if (codepoint <= 0xFFFF)
        {
            if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
            {
                result.push_back(0xEF);
                result.push_back(0xBF);
                result.push_back(0xBD);
            }
            else
            {
                result.push_back(static_cast<unsigned char>(0xE0 | (codepoint >> 12)));
                result.push_back(static_cast<unsigned char>(0x80 | ((codepoint >> 6) & 0x3F)));
                result.push_back(static_cast<unsigned char>(0x80 | (codepoint & 0x3F)));
            }
        }
        else if (codepoint <= 0x10FFFF)
        {
            result.push_back(static_cast<unsigned char>(0xF0 | (codepoint >> 18)));
            result.push_back(static_cast<unsigned char>(0x80 | ((codepoint >> 12) & 0x3F)));
            result.push_back(static_cast<unsigned char>(0x80 | ((codepoint >> 6) & 0x3F)));
            result.push_back(static_cast<unsigned char>(0x80 | (codepoint & 0x3F)));
        }
        else
        {
            result.push_back(0xEF);
            result.push_back(0xBF);
            result.push_back(0xBD);
        }
    }
    else if (encoding == charset::ascii)
    {
        if (codepoint < 0x80)
        {
            result.push_back(static_cast<unsigned char>(codepoint));
        }
        else
        {
            result.push_back('?');
        }
    }
    else if (encoding == charset::latin1)
    {
        if (codepoint < 0x100)
        {
            result.push_back(static_cast<unsigned char>(codepoint));
        }
        else
        {
            result.push_back('?');
        }
    }
    else if (encoding == charset::utf16le)
    {
        if (codepoint <= 0xFFFF)
        {
            if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
            {
                result.push_back(0xFD);
                result.push_back(0xFF);
            }
            else
            {
                result.push_back(static_cast<unsigned char>(codepoint & 0xFF));
                result.push_back(static_cast<unsigned char>((codepoint >> 8) & 0xFF));
            }
        }
        else if (codepoint <= 0x10FFFF)
        {
            unsigned int high = 0xD800 + ((codepoint - 0x10000) >> 10);
            unsigned int low = 0xDC00 + ((codepoint - 0x10000) & 0x3FF);
            result.push_back(static_cast<unsigned char>(high & 0xFF));
            result.push_back(static_cast<unsigned char>((high >> 8) & 0xFF));
            result.push_back(static_cast<unsigned char>(low & 0xFF));
            result.push_back(static_cast<unsigned char>((low >> 8) & 0xFF));
        }
        else
        {
            result.push_back(0xFD);
            result.push_back(0xFF);
        }
    }
    else if (encoding == charset::utf16 || encoding == charset::utf16be)
    {
        if (codepoint <= 0xFFFF)
        {
            if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
            {
                result.push_back(0xFF);
                result.push_back(0xFD);
            }
            else
            {
                result.push_back(static_cast<unsigned char>((codepoint >> 8) & 0xFF));
                result.push_back(static_cast<unsigned char>(codepoint & 0xFF));
            }
        }
        else if (codepoint <= 0x10FFFF)
        {
            unsigned int high = 0xD800 + ((codepoint - 0x10000) >> 10);
            unsigned int low = 0xDC00 + ((codepoint - 0x10000) & 0x3FF);
            result.push_back(static_cast<unsigned char>((high >> 8) & 0xFF));
            result.push_back(static_cast<unsigned char>(high & 0xFF));
            result.push_back(static_cast<unsigned char>((low >> 8) & 0xFF));
            result.push_back(static_cast<unsigned char>(low & 0xFF));
        }
        else
        {
            result.push_back(0xFF);
            result.push_back(0xFD);
        }
    }
    else if (encoding == charset::utf32le)
    {
        const unsigned int scalar = detail::isScalarValue(codepoint) ? codepoint : 0xFFFD;
        result.push_back(static_cast<unsigned char>(scalar & 0xFF));
        result.push_back(static_cast<unsigned char>((scalar >> 8) & 0xFF));
        result.push_back(static_cast<unsigned char>((scalar >> 16) & 0xFF));
        result.push_back(static_cast<unsigned char>((scalar >> 24) & 0xFF));
    }
    else if (encoding == charset::utf32 || encoding == charset::utf32be)
    {
        const unsigned int scalar = detail::isScalarValue(codepoint) ? codepoint : 0xFFFD;
        result.push_back(static_cast<unsigned char>((scalar >> 24) & 0xFF));
        result.push_back(static_cast<unsigned char>((scalar >> 16) & 0xFF));
        result.push_back(static_cast<unsigned char>((scalar >> 8) & 0xFF));
        result.push_back(static_cast<unsigned char>(scalar & 0xFF));
    }
    return result;
}

inline unsigned int decode(std::string_view &input, charset encoding)
{
    if (input.empty())
    {
        return 0xFFFD;
    }

    const unsigned char *data = reinterpret_cast<const unsigned char *>(input.data());
    std::size_t dataLength = input.size();

    if (encoding == charset::latin1)
    {
        unsigned char byte = data[0];
        input.remove_prefix(1);
        return byte;
    }
    else if (encoding == charset::ascii)
    {
        unsigned char byte = data[0];
        input.remove_prefix(1);
        if (byte < 0x80)
        {
            return byte;
        }
        else
        {
            return 0xFFFD;
        }
    }
    else if (encoding == charset::utf8)
    {
        const unsigned char byte = data[0];
        if (byte < 0x80)
        {
            input.remove_prefix(1);
            return byte;
        }

        std::size_t seqLen = 0;
        if (byte >= 0xC2 && byte <= 0xDF)
        {
            seqLen = 2;
        }
        else if (byte >= 0xE0 && byte <= 0xEF)
        {
            seqLen = 3;
        }
        else if (byte >= 0xF0 && byte <= 0xF4)
        {
            seqLen = 4;
        }
        else
        {
            input.remove_prefix(1);
            return 0xFFFD;
        }

        // U+FFFD substitution of maximal subparts (Unicode Standard, Section
        // 3.9): consume the lead byte plus every valid continuation byte that
        // belongs to the ill-formed sequence, but do not swallow the byte that
        // invalidated it.
        if (dataLength < 2 || (data[1] & 0xC0) != 0x80)
        {
            input.remove_prefix(1);
            return 0xFFFD;
        }
        if (seqLen == 2)
        {
            const unsigned int codepoint = ((byte & 0x1F) << 6) | (data[1] & 0x3F);
            input.remove_prefix(2);
            return codepoint < 0x80 ? 0xFFFD : codepoint;
        }

        if (dataLength < 3)
        {
            input.remove_prefix(2);
            return 0xFFFD;
        }
        if ((data[2] & 0xC0) != 0x80)
        {
            input.remove_prefix(2);
            return 0xFFFD;
        }
        if (seqLen == 3)
        {
            const unsigned int codepoint = ((byte & 0x0F) << 12) | ((data[1] & 0x3F) << 6) | (data[2] & 0x3F);
            input.remove_prefix(3);
            if (codepoint < 0x800 || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
            {
                return 0xFFFD;
            }
            return codepoint;
        }

        if (dataLength < 4)
        {
            input.remove_prefix(3);
            return 0xFFFD;
        }
        if ((data[3] & 0xC0) != 0x80)
        {
            input.remove_prefix(3);
            return 0xFFFD;
        }
        const unsigned int codepoint = ((byte & 0x07) << 18) | ((data[1] & 0x3F) << 12) | ((data[2] & 0x3F) << 6) | (data[3] & 0x3F);
        input.remove_prefix(4);
        if (codepoint < 0x10000 || codepoint > 0x10FFFF)
        {
            return 0xFFFD;
        }
        return codepoint;
    }
    else if (encoding == charset::utf16le)
    {
        if (dataLength < 2)
        {
            input.remove_prefix(dataLength);
            return 0xFFFD;
        }
        unsigned short unit = data[0] | (static_cast<unsigned short>(data[1]) << 8);
        if (unit >= 0xD800 && unit <= 0xDBFF)
        {
            if (dataLength < 4)
            {
                input.remove_prefix(dataLength);
                return 0xFFFD;
            }
            unsigned short low = data[2] | (static_cast<unsigned short>(data[3]) << 8);
            if (low < 0xDC00 || low > 0xDFFF)
            {
                input.remove_prefix(2);
                return 0xFFFD;
            }
            unsigned int codepoint = 0x10000 + ((unit - 0xD800) << 10) + (low - 0xDC00);
            input.remove_prefix(4);
            return codepoint;
        }
        else if (unit >= 0xDC00 && unit <= 0xDFFF)
        {
            input.remove_prefix(2);
            return 0xFFFD;
        }
        else
        {
            input.remove_prefix(2);
            return unit;
        }
    }
    else if (encoding == charset::utf16 || encoding == charset::utf16be)
    {
        if (dataLength < 2)
        {
            input.remove_prefix(dataLength);
            return 0xFFFD;
        }
        unsigned short unit = (static_cast<unsigned short>(data[0]) << 8) | data[1];
        if (unit >= 0xD800 && unit <= 0xDBFF)
        {
            if (dataLength < 4)
            {
                input.remove_prefix(dataLength);
                return 0xFFFD;
            }
            unsigned short low = (static_cast<unsigned short>(data[2]) << 8) | data[3];
            if (low < 0xDC00 || low > 0xDFFF)
            {
                input.remove_prefix(2);
                return 0xFFFD;
            }
            unsigned int codepoint = 0x10000 + ((unit - 0xD800) << 10) + (low - 0xDC00);
            input.remove_prefix(4);
            return codepoint;
        }
        else if (unit >= 0xDC00 && unit <= 0xDFFF)
        {
            input.remove_prefix(2);
            return 0xFFFD;
        }
        else
        {
            input.remove_prefix(2);
            return unit;
        }
    }
    else if (encoding == charset::utf32le)
    {
        if (dataLength < 4)
        {
            input.remove_prefix(dataLength);
            return 0xFFFD;
        }
        unsigned int codepoint = data[0] | (static_cast<unsigned int>(data[1]) << 8) | (static_cast<unsigned int>(data[2]) << 16) | (static_cast<unsigned int>(data[3]) << 24);
        input.remove_prefix(4);
        if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
        {
            return 0xFFFD;
        }
        return codepoint;
    }
    else if (encoding == charset::utf32 || encoding == charset::utf32be)
    {
        if (dataLength < 4)
        {
            input.remove_prefix(dataLength);
            return 0xFFFD;
        }
        unsigned int codepoint = (static_cast<unsigned int>(data[0]) << 24) | (static_cast<unsigned int>(data[1]) << 16) | (static_cast<unsigned int>(data[2]) << 8) | data[3];
        input.remove_prefix(4);
        if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
        {
            return 0xFFFD;
        }
        return codepoint;
    }
    return 0xFFFD;
}

inline std::string convert(std::string_view input, charset from, charset to)
{
    if (from == to)
    {
        return std::string(input);
    }
    std::string result;
    std::string_view remaining = input;
    while (!remaining.empty())
    {
        unsigned int codepoint = decode(remaining, from);
        auto encoded = encode(codepoint, to);
        result.insert(result.end(), encoded.begin(), encoded.end());
    }
    return result;
}

inline void convert(std::string_view input, charset from, charset to, std::vector<unsigned char> &output)
{
    if (from == to)
    {
        output.insert(output.end(), input.begin(), input.end());
        return;
    }
    std::string_view remaining = input;
    while (!remaining.empty())
    {
        unsigned int codepoint = decode(remaining, from);
        auto encoded = encode(codepoint, to);
        output.insert(output.end(), encoded.begin(), encoded.end());
    }
}

inline void convert(std::string_view input, charset from, charset to, std::deque<unsigned char> &output)
{
    if (from == to)
    {
        output.insert(output.end(), input.begin(), input.end());
        return;
    }
    std::string_view remaining = input;
    while (!remaining.empty())
    {
        unsigned int codepoint = decode(remaining, from);
        auto encoded = encode(codepoint, to);
        output.insert(output.end(), encoded.begin(), encoded.end());
    }
}
} // namespace bunch
