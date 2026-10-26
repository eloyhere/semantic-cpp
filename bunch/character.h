#pragma once

#include "meta.h"
#include "point.h"

#include <cstddef>
#include <functional>

namespace bunch
{

// A character value combines a Unicode code point with application metadata.
// It is an encoded-character-like value, not an abstract character or a
// grapheme cluster; a user-perceived character may consist of several code
// points. Use Point when only the numeric code point is needed.
class Character
{
  public:
    Character() : pointValue(), metaValue() {}
    explicit Character(unsigned int codepoint) : pointValue(codepoint), metaValue() {}
    explicit Character(Point point) : pointValue(point), metaValue() {}
    Character(Point point, Meta meta) : pointValue(point), metaValue(meta) {}

    Character(const Character &other) : pointValue(other.pointValue), metaValue(other.metaValue) {}
    Character(Character &&other) noexcept : pointValue(other.pointValue), metaValue(other.metaValue)
    {
        other.pointValue = Point();
        other.metaValue = Meta();
    }

    Character &operator=(const Character &other)
    {
        if (this != &other)
        {
            pointValue = other.pointValue;
            metaValue = other.metaValue;
        }
        return *this;
    }

    Character &operator=(Character &&other) noexcept
    {
        if (this != &other)
        {
            pointValue = other.pointValue;
            metaValue = other.metaValue;
            other.pointValue = Point();
            other.metaValue = Meta();
        }
        return *this;
    }

    const Point &getPoint() const { return pointValue; }
    const Meta &getMeta() const { return metaValue; }
    unsigned int getValue() const { return pointValue.getValue(); }
    bool isSurrogate() const { return pointValue.isSurrogate(); }
    bool isUnicodeCodePoint() const { return pointValue.isUnicodeCodePoint(); }
    bool isScalarValue() const { return pointValue.isScalarValue(); }
    bool isNoncharacter() const { return pointValue.isNoncharacter(); }
    bool isValidCodePoint() const { return pointValue.isValidCodePoint(); }

    bool operator==(const Character &other) const
    {
        return pointValue == other.pointValue && metaValue == other.metaValue;
    }
    bool operator!=(const Character &other) const { return !(*this == other); }
    bool operator<(const Character &other) const
    {
        if (pointValue != other.pointValue)
        {
            return pointValue < other.pointValue;
        }
        return metaValue < other.metaValue;
    }

  private:
    Point pointValue;
    Meta metaValue;
};

using CodePoint = Point;

} // namespace bunch

namespace std
{
template <>
struct hash<::bunch::Character>
{
    size_t operator()(const ::bunch::Character &character) const noexcept
    {
        size_t seed = hash<unsigned int>()(character.getPoint().getValue());
        seed ^= hash<unsigned int>()(character.getMeta().getValue()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
} // namespace std
