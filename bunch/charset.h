#pragma once

#include <cstddef>
#include <deque>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace bunch
{
enum class charset
{
    ascii,
    utf8,
    utf16,
    utf16be,
    utf16le,
    utf32,
    utf32be,
    utf32le,
    latin1
};

inline std::ostream &operator<<(std::ostream &stream, charset cs)
{
    switch (cs)
    {
    case charset::ascii:
        stream << "ascii";
        break;
    case charset::utf8:
        stream << "utf8";
        break;
    case charset::utf16:
        stream << "utf16";
        break;
    case charset::utf16be:
        stream << "utf16be";
        break;
    case charset::utf16le:
        stream << "utf16le";
        break;
    case charset::utf32:
        stream << "utf32";
        break;
    case charset::utf32be:
        stream << "utf32be";
        break;
    case charset::utf32le:
        stream << "utf32le";
        break;
    case charset::latin1:
        stream << "latin1";
        break;
    }
    return stream;
}

inline std::istream &operator>>(std::istream &stream, charset &cs)
{
    std::string token;
    stream >> token;
    if (token == "ascii")
        cs = charset::ascii;
    else if (token == "utf8")
        cs = charset::utf8;
    else if (token == "utf16")
        cs = charset::utf16;
    else if (token == "utf16be")
        cs = charset::utf16be;
    else if (token == "utf16le")
        cs = charset::utf16le;
    else if (token == "utf32")
        cs = charset::utf32;
    else if (token == "utf32be")
        cs = charset::utf32be;
    else if (token == "utf32le")
        cs = charset::utf32le;
    else if (token == "latin1")
        cs = charset::latin1;
    else
        stream.setstate(std::ios::failbit);
    return stream;
}
} // namespace bunch
