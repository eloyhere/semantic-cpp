#pragma once

#include <cstddef>
#include <functional>
#include <iterator>
#include <vector>

namespace bunch
{

// A numeric Unicode code point in the codespace U+0000..U+10FFFF.
// A code point is not an abstract character or a user-perceived character
// (grapheme cluster); see localisation/unicode.md.
class Point
{
  public:
    Point() : code(0) {}
    explicit Point(unsigned int codepoint) : code(codepoint) {}
    Point(const Point &other) : code(other.code) {}
    Point(Point &&other) noexcept : code(other.code) { other.code = 0; }

    Point &operator=(const Point &other)
    {
        if (this != &other)
        {
            code = other.code;
        }
        return *this;
    }

    Point &operator=(Point &&other) noexcept
    {
        if (this != &other)
        {
            code = other.code;
            other.code = 0;
        }
        return *this;
    }

    bool operator==(const Point &other) const { return code == other.code; }
    bool operator!=(const Point &other) const { return code != other.code; }
    bool operator<(const Point &other) const { return code < other.code; }
    unsigned int getValue() const { return code; }
    bool isSurrogate() const { return code >= 0xD800 && code <= 0xDFFF; }
    bool isUnicodeCodePoint() const { return code <= 0x10FFFF; }
    bool isScalarValue() const { return isUnicodeCodePoint() && !isSurrogate(); }
    bool isNoncharacter() const
    {
        return isUnicodeCodePoint() && ((code >= 0xFDD0 && code <= 0xFDEF) || (code & 0xFFFE) == 0xFFFE);
    }
    // Historical name: this actually reports whether the value is a Unicode
    // scalar value (a code point that is not a surrogate).
    bool isValidCodePoint() const { return isScalarValue(); }

  private:
    unsigned int code;
};

class PointIterator
{
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = Point;
    using difference_type = std::ptrdiff_t;
    using pointer = const Point *;
    using reference = const Point &;

    explicit PointIterator(std::vector<Point>::const_iterator it) : baseIterator(it) {}

    reference operator*() const { return *baseIterator; }
    pointer operator->() const { return &(*baseIterator); }

    PointIterator &operator++()
    {
        ++baseIterator;
        return *this;
    }
    PointIterator operator++(int)
    {
        PointIterator temp = *this;
        ++baseIterator;
        return temp;
    }
    PointIterator &operator--()
    {
        --baseIterator;
        return *this;
    }
    PointIterator operator--(int)
    {
        PointIterator temp = *this;
        --baseIterator;
        return temp;
    }

    bool operator==(const PointIterator &other) const { return baseIterator == other.baseIterator; }
    bool operator!=(const PointIterator &other) const { return baseIterator != other.baseIterator; }

  private:
    std::vector<Point>::const_iterator baseIterator;
};

} // namespace bunch

namespace std
{
template <>
struct hash<::bunch::Point>
{
    size_t operator()(const ::bunch::Point &point) const noexcept
    {
        return hash<unsigned int>()(point.getValue());
    }
};
} // namespace std
