#pragma once

#include "charset.h"
#include "encode.h"
#include "meta.h"
#include "point.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace bunch
{

class Bunch;

class Bunch
{
  public:
    // A Bunch stores a sequence of Unicode code points in an
    // encoding-independent form. storageEncoding is the preferred encoding
    // used by getBytes()/operator<<; it does not change the stored values.
    Bunch() : pointStorage(), storageEncoding(charset::utf8) {}

    Bunch(const char *str) : Bunch(std::string_view(str), charset::utf8, charset::utf8) {}

    explicit Bunch(charset encoding) : pointStorage(), storageEncoding(encoding) {}

    Bunch(std::string_view source, charset sourceEncoding = charset::utf8, charset targetEncoding = charset::utf8)
        : pointStorage(), storageEncoding(targetEncoding)
    {
        if (sourceEncoding == targetEncoding)
        {
            std::string_view remaining = source;
            while (!remaining.empty())
            {
                pointStorage.push_back(Point(decode(remaining, targetEncoding)));
            }
        }
        else
        {
            std::string converted = convert(source, sourceEncoding, targetEncoding);
            std::string_view remaining = converted;
            while (!remaining.empty())
            {
                pointStorage.push_back(Point(decode(remaining, targetEncoding)));
            }
        }
    }

    Bunch(const Meta *source, std::size_t length, charset sourceEncoding, charset targetEncoding = charset::utf8)
        : pointStorage(), storageEncoding(targetEncoding)
    {
        if (!source || length == 0)
        {
            return;
        }
        std::string temp;
        temp.reserve(length);
        for (std::size_t i = 0; i < length; ++i)
        {
            temp.push_back(static_cast<char>(source[i].getValue() & 0xFF));
        }
        if (sourceEncoding == targetEncoding)
        {
            std::string_view remaining = temp;
            while (!remaining.empty())
            {
                pointStorage.push_back(Point(decode(remaining, targetEncoding)));
            }
        }
        else
        {
            std::string converted = convert(std::string_view(temp), sourceEncoding, targetEncoding);
            std::string_view remaining = converted;
            while (!remaining.empty())
            {
                pointStorage.push_back(Point(decode(remaining, targetEncoding)));
            }
        }
    }

    Bunch(const Point *source, std::size_t length, charset targetEncoding = charset::utf8)
        : pointStorage(), storageEncoding(targetEncoding)
    {
        if (!source || length == 0)
        {
            return;
        }
        for (std::size_t i = 0; i < length; ++i)
        {
            pointStorage.push_back(source[i]);
        }
    }

    Bunch(const Bunch &other)
        : pointStorage(other.pointStorage), storageEncoding(other.storageEncoding) {}

    Bunch(Bunch &&other) noexcept
        : pointStorage(std::move(other.pointStorage)), storageEncoding(other.storageEncoding)
    {
        other.storageEncoding = charset::utf8;
    }

    Bunch &operator=(const Bunch &other)
    {
        if (this != &other)
        {
            pointStorage = other.pointStorage;
            storageEncoding = other.storageEncoding;
        }
        return *this;
    }

    Bunch &operator=(Bunch &&other) noexcept
    {
        if (this != &other)
        {
            pointStorage = std::move(other.pointStorage);
            storageEncoding = other.storageEncoding;
            other.storageEncoding = charset::utf8;
        }
        return *this;
    }

    // Number of Unicode code points, not grapheme clusters. Combining marks
    // and emoji ZWJ sequences may therefore count as multiple units.
    std::size_t size() const { return pointStorage.size(); }
    bool empty() const { return pointStorage.empty(); }

    Point at(std::size_t index) const
    {
        if (index >= pointStorage.size())
        {
            throw std::out_of_range("Index out of range");
        }
        return pointStorage[index];
    }

    charset encoding() const { return storageEncoding; }

    Bunch sub(std::size_t start, std::size_t length) const
    {
        std::size_t totalPoints = pointStorage.size();
        if (start > totalPoints)
        {
            start = totalPoints;
        }
        if (length > totalPoints - start)
        {
            length = totalPoints - start;
        }
        if (length == 0)
        {
            Bunch result;
            result.storageEncoding = storageEncoding;
            return result;
        }
        Bunch result;
        result.storageEncoding = storageEncoding;
        for (std::size_t i = start; i < start + length && i < totalPoints; ++i)
        {
            result.pointStorage.push_back(pointStorage[i]);
        }
        return result;
    }

    Bunch repeat(std::size_t count) const
    {
        if (count == 0)
        {
            Bunch result;
            result.storageEncoding = storageEncoding;
            return result;
        }
        if (count > 1 && pointStorage.size() > std::numeric_limits<std::size_t>::max() / count)
        {
            throw std::overflow_error("repeat size overflow");
        }
        Bunch result;
        result.storageEncoding = storageEncoding;
        result.pointStorage.reserve(pointStorage.size() * count);
        for (std::size_t i = 0; i < count; ++i)
        {
            result.pointStorage.insert(result.pointStorage.end(), pointStorage.begin(), pointStorage.end());
        }
        return result;
    }

    Bunch concat(const Bunch &other) const
    {
        Bunch result;
        result.storageEncoding = storageEncoding;
        result.pointStorage.reserve(pointStorage.size() + other.pointStorage.size());
        result.pointStorage.insert(result.pointStorage.end(), pointStorage.begin(), pointStorage.end());
        result.pointStorage.insert(result.pointStorage.end(), other.pointStorage.begin(), other.pointStorage.end());
        return result;
    }

    bool startsWith(const Bunch &other) const
    {
        if (other.pointStorage.size() > pointStorage.size())
        {
            return false;
        }
        for (std::size_t i = 0; i < other.pointStorage.size(); ++i)
        {
            if (pointStorage[i].getValue() != other.pointStorage[i].getValue())
            {
                return false;
            }
        }
        return true;
    }

    bool startsWith(std::string_view str, charset strEncoding = charset::utf8) const
    {
        Bunch temp(str, strEncoding, storageEncoding);
        return startsWith(temp);
    }

    bool endsWith(const Bunch &other) const
    {
        if (other.pointStorage.size() > pointStorage.size())
        {
            return false;
        }
        std::size_t offset = pointStorage.size() - other.pointStorage.size();
        for (std::size_t i = 0; i < other.pointStorage.size(); ++i)
        {
            if (pointStorage[offset + i].getValue() != other.pointStorage[i].getValue())
            {
                return false;
            }
        }
        return true;
    }

    bool endsWith(std::string_view str, charset strEncoding = charset::utf8) const
    {
        Bunch temp(str, strEncoding, storageEncoding);
        return endsWith(temp);
    }

    bool contains(const Bunch &other) const
    {
        return indexOf(other) != static_cast<std::size_t>(-1);
    }

    bool contains(std::string_view str, charset strEncoding = charset::utf8) const
    {
        Bunch temp(str, strEncoding, storageEncoding);
        return contains(temp);
    }

    std::size_t indexOf(const Bunch &other, std::size_t fromCodePoint = 0) const
    {
        const auto &otherPoints = other.pointStorage;
        if (otherPoints.empty())
        {
            return 0;
        }
        if (fromCodePoint >= pointStorage.size())
        {
            return static_cast<std::size_t>(-1);
        }
        if (otherPoints.size() > pointStorage.size())
        {
            return static_cast<std::size_t>(-1);
        }
        for (std::size_t i = fromCodePoint; i <= pointStorage.size() - otherPoints.size(); ++i)
        {
            bool found = true;
            for (std::size_t j = 0; j < otherPoints.size(); ++j)
            {
                if (pointStorage[i + j].getValue() != otherPoints[j].getValue())
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                return i;
            }
        }
        return static_cast<std::size_t>(-1);
    }

    std::size_t indexOf(std::string_view str, std::size_t fromCodePoint = 0, charset strEncoding = charset::utf8) const
    {
        Bunch temp(str, strEncoding, storageEncoding);
        return indexOf(temp, fromCodePoint);
    }

    std::size_t lastIndexOf(const Bunch &other, std::size_t fromCodePoint = static_cast<std::size_t>(-1)) const
    {
        const auto &otherPoints = other.pointStorage;
        if (otherPoints.empty())
        {
            return pointStorage.size();
        }
        if (otherPoints.size() > pointStorage.size())
        {
            return static_cast<std::size_t>(-1);
        }
        std::size_t startPosition = pointStorage.size() - otherPoints.size();
        if (fromCodePoint != static_cast<std::size_t>(-1))
        {
            if (fromCodePoint >= pointStorage.size())
            {
                return static_cast<std::size_t>(-1);
            }
            if (fromCodePoint < startPosition)
            {
                startPosition = fromCodePoint;
            }
        }
        for (std::size_t i = startPosition + 1; i > 0; --i)
        {
            std::size_t currentIndex = i - 1;
            bool found = true;
            for (std::size_t j = 0; j < otherPoints.size(); ++j)
            {
                if (pointStorage[currentIndex + j].getValue() != otherPoints[j].getValue())
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                return currentIndex;
            }
        }
        return static_cast<std::size_t>(-1);
    }

    std::size_t lastIndexOf(std::string_view str, std::size_t fromCodePoint = static_cast<std::size_t>(-1), charset strEncoding = charset::utf8) const
    {
        Bunch temp(str, strEncoding, storageEncoding);
        return lastIndexOf(temp, fromCodePoint);
    }

    Bunch replace(const Bunch &target, const Bunch &replacement) const
    {
        if (target.pointStorage.empty())
        {
            return *this;
        }
        Bunch result;
        result.storageEncoding = storageEncoding;
        std::size_t pos = 0;
        while (pos < pointStorage.size())
        {
            std::size_t found = indexOf(target, pos);
            if (found == static_cast<std::size_t>(-1))
            {
                for (std::size_t i = pos; i < pointStorage.size(); ++i)
                {
                    result.pointStorage.push_back(pointStorage[i]);
                }
                break;
            }
            for (std::size_t i = pos; i < found; ++i)
            {
                result.pointStorage.push_back(pointStorage[i]);
            }
            for (const auto &p : replacement.pointStorage)
            {
                result.pointStorage.push_back(p);
            }
            pos = found + target.pointStorage.size();
        }
        return result;
    }

    Bunch replace(std::string_view target, std::string_view replacement, charset targetEncoding = charset::utf8, charset replacementEncoding = charset::utf8) const
    {
        Bunch targetCs(target, targetEncoding, storageEncoding);
        Bunch replacementCs(replacement, replacementEncoding, storageEncoding);
        return replace(targetCs, replacementCs);
    }

    Bunch replace(const Bunch &target, const Bunch &replacement, std::size_t maxCount) const
    {
        if (target.pointStorage.empty() || maxCount == 0)
        {
            return *this;
        }
        Bunch result;
        result.storageEncoding = storageEncoding;
        std::size_t pos = 0;
        std::size_t count = 0;
        while (pos < pointStorage.size() && count < maxCount)
        {
            std::size_t found = indexOf(target, pos);
            if (found == static_cast<std::size_t>(-1))
            {
                for (std::size_t i = pos; i < pointStorage.size(); ++i)
                {
                    result.pointStorage.push_back(pointStorage[i]);
                }
                break;
            }
            for (std::size_t i = pos; i < found; ++i)
            {
                result.pointStorage.push_back(pointStorage[i]);
            }
            for (const auto &p : replacement.pointStorage)
            {
                result.pointStorage.push_back(p);
            }
            pos = found + target.pointStorage.size();
            count++;
        }
        if (pos < pointStorage.size())
        {
            for (std::size_t i = pos; i < pointStorage.size(); ++i)
            {
                result.pointStorage.push_back(pointStorage[i]);
            }
        }
        return result;
    }

    Bunch replace(std::string_view target, std::string_view replacement, std::size_t maxCount, charset targetEncoding = charset::utf8, charset replacementEncoding = charset::utf8) const
    {
        Bunch targetCs(target, targetEncoding, storageEncoding);
        Bunch replacementCs(replacement, replacementEncoding, storageEncoding);
        return replace(targetCs, replacementCs, maxCount);
    }

    std::size_t count(const Bunch &target) const
    {
        if (target.pointStorage.empty())
        {
            return 0;
        }
        std::size_t result = 0;
        std::size_t pos = 0;
        while (true)
        {
            std::size_t found = indexOf(target, pos);
            if (found == static_cast<std::size_t>(-1))
            {
                break;
            }
            result++;
            pos = found + target.pointStorage.size();
        }
        return result;
    }

    std::size_t count(std::string_view str, charset strEncoding = charset::utf8) const
    {
        Bunch temp(str, strEncoding, storageEncoding);
        return count(temp);
    }

    // Trims Unicode White_Space code points (UCD White_Space property), not
    // just ASCII whitespace.
    Bunch trim() const
    {
        std::size_t start = 0;
        while (start < pointStorage.size() && isWhitespace(pointStorage[start].getValue()))
        {
            start++;
        }
        std::size_t end = pointStorage.size();
        while (end > start && isWhitespace(pointStorage[end - 1].getValue()))
        {
            end--;
        }
        return sub(start, end - start);
    }

    // ASCII-only simple case conversion. Full Unicode case conversion is
    // context-sensitive and may change string length (for example, U+00DF
    // LATIN SMALL LETTER SHARP S maps to "SS" in uppercase).
    Bunch toUpperCase() const
    {
        Bunch result;
        result.storageEncoding = storageEncoding;
        result.pointStorage.reserve(pointStorage.size());
        for (const auto &p : pointStorage)
        {
            unsigned int cp = p.getValue();
            if (cp >= 'a' && cp <= 'z')
            {
                result.pointStorage.push_back(Point(cp - 32));
            }
            else
            {
                result.pointStorage.push_back(p);
            }
        }
        return result;
    }

    // ASCII-only simple case conversion. See toUpperCase().
    Bunch toLowerCase() const
    {
        Bunch result;
        result.storageEncoding = storageEncoding;
        result.pointStorage.reserve(pointStorage.size());
        for (const auto &p : pointStorage)
        {
            unsigned int cp = p.getValue();
            if (cp >= 'A' && cp <= 'Z')
            {
                result.pointStorage.push_back(Point(cp + 32));
            }
            else
            {
                result.pointStorage.push_back(p);
            }
        }
        return result;
    }

    // Reverses code points, not grapheme clusters. This can split a base
    // character from its combining marks; use a grapheme-aware algorithm if
    // visual character order is required.
    Bunch reverse() const
    {
        Bunch result;
        result.storageEncoding = storageEncoding;
        result.pointStorage.reserve(pointStorage.size());
        for (auto it = pointStorage.rbegin(); it != pointStorage.rend(); ++it)
        {
            result.pointStorage.push_back(*it);
        }
        return result;
    }

    std::vector<Bunch> split(const Bunch &delimiter) const
    {
        std::vector<Bunch> result;
        if (pointStorage.empty())
        {
            return result;
        }
        if (delimiter.empty())
        {
            throw std::invalid_argument("split delimiter must not be empty");
        }
        std::size_t pos = 0;
        while (pos <= pointStorage.size())
        {
            std::size_t found = indexOf(delimiter, pos);
            if (found == static_cast<std::size_t>(-1))
            {
                result.push_back(sub(pos, pointStorage.size() - pos));
                break;
            }
            result.push_back(sub(pos, found - pos));
            pos = found + delimiter.pointStorage.size();
        }
        return result;
    }

    std::vector<Bunch> split(std::string_view delimiter, charset delimiterEncoding = charset::utf8) const
    {
        Bunch delim(delimiter, delimiterEncoding, storageEncoding);
        return split(delim);
    }

    static Bunch join(const std::vector<Bunch> &parts, const Bunch &separator)
    {
        if (parts.empty())
        {
            return Bunch();
        }
        Bunch result = parts[0];
        for (std::size_t i = 1; i < parts.size(); ++i)
        {
            result = result.concat(separator).concat(parts[i]);
        }
        return result;
    }

    static Bunch join(const std::vector<Bunch> &parts, std::string_view separator, charset separatorEncoding = charset::utf8)
    {
        Bunch sep(separator, separatorEncoding);
        return join(parts, sep);
    }

    int compare(const Bunch &other) const
    {
        std::size_t minSize = std::min(pointStorage.size(), other.pointStorage.size());
        for (std::size_t i = 0; i < minSize; ++i)
        {
            if (pointStorage[i].getValue() < other.pointStorage[i].getValue())
            {
                return -1;
            }
            if (pointStorage[i].getValue() > other.pointStorage[i].getValue())
            {
                return 1;
            }
        }
        if (pointStorage.size() < other.pointStorage.size())
        {
            return -1;
        }
        if (pointStorage.size() > other.pointStorage.size())
        {
            return 1;
        }
        return 0;
    }

    int compare(std::string_view str, charset strEncoding = charset::utf8) const
    {
        Bunch temp(str, strEncoding, storageEncoding);
        return compare(temp);
    }

    std::vector<unsigned char> getBytes(charset targetEncoding) const
    {
        std::vector<unsigned char> result;
        for (const auto &point : pointStorage)
        {
            auto encoded = encode(point.getValue(), targetEncoding);
            result.insert(result.end(), encoded.begin(), encoded.end());
        }
        return result;
    }

    std::vector<unsigned char> getBytes() const
    {
        return getBytes(storageEncoding);
    }

    std::vector<char> getCharacters() const
    {
        auto bytes = getBytes();
        return std::vector<char>(bytes.begin(), bytes.end());
    }

    const std::vector<Point> &getPoints() const
    {
        return pointStorage;
    }

    std::vector<Meta> getMetas() const
    {
        std::vector<Meta> metas;
        metas.reserve(pointStorage.size());
        for (const auto &point : pointStorage)
        {
            metas.emplace_back(point.getValue());
        }
        return metas;
    }

    Bunch operator+(const Bunch &other) const
    {
        Bunch result;
        result.storageEncoding = storageEncoding;
        result.pointStorage.reserve(pointStorage.size() + other.pointStorage.size());
        result.pointStorage.insert(result.pointStorage.end(), pointStorage.begin(), pointStorage.end());
        result.pointStorage.insert(result.pointStorage.end(), other.pointStorage.begin(), other.pointStorage.end());
        return result;
    }

    Bunch operator+(std::string_view str) const
    {
        Bunch temp(str, charset::utf8, storageEncoding);
        return *this + temp;
    }

    Bunch operator+(const char *str) const
    {
        return str == nullptr ? *this : *this + std::string_view(str);
    }

    Bunch &operator+=(const Bunch &other)
    {
        pointStorage.insert(pointStorage.end(), other.pointStorage.begin(), other.pointStorage.end());
        return *this;
    }

    Bunch &operator+=(std::string_view other)
    {
        Bunch temp(other, charset::utf8, storageEncoding);
        return *this += temp;
    }

    Bunch &operator+=(const char *other)
    {
        return other == nullptr ? *this : *this += std::string_view(other);
    }

    bool operator==(const Bunch &other) const { return compare(other) == 0; }
    bool operator!=(const Bunch &other) const { return compare(other) != 0; }
    bool operator<(const Bunch &other) const { return compare(other) < 0; }
    bool operator<=(const Bunch &other) const { return compare(other) <= 0; }
    bool operator>(const Bunch &other) const { return compare(other) > 0; }
    bool operator>=(const Bunch &other) const { return compare(other) >= 0; }

    bool operator==(std::string_view str) const
    {
        Bunch temp(str, charset::utf8, storageEncoding);
        return compare(temp) == 0;
    }
    bool operator!=(std::string_view str) const
    {
        Bunch temp(str, charset::utf8, storageEncoding);
        return compare(temp) != 0;
    }
    bool operator==(const char *str) const
    {
        return str != nullptr && compare(std::string_view(str)) == 0;
    }
    bool operator!=(const char *str) const
    {
        return str != nullptr && compare(std::string_view(str)) != 0;
    }

    Point operator[](std::size_t index) const { return at(index); }

    friend std::ostream &operator<<(std::ostream &stream, const Bunch &sequence)
    {
        auto bytes = sequence.getBytes();
        stream.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
        return stream;
    }

    friend std::istream &operator>>(std::istream &stream, Bunch &sequence)
    {
        std::string temp;
        std::getline(stream, temp);
        sequence = Bunch(std::string_view(temp), charset::utf8, sequence.storageEncoding);
        return stream;
    }

    friend Bunch operator+(std::string_view left, const Bunch &right)
    {
        Bunch temp(left, charset::utf8, right.storageEncoding);
        return temp + right;
    }

    PointIterator begin() const { return PointIterator(pointStorage.cbegin()); }
    PointIterator end() const { return PointIterator(pointStorage.cend()); }

  private:
    std::vector<Point> pointStorage;
    charset storageEncoding;

    static bool isWhitespace(unsigned int codepoint)
    {
        // Unicode White_Space property (UCD PropList.txt), not the ASCII-only
        // definition used by std::isspace.
        return (codepoint >= 0x0009 && codepoint <= 0x000D) ||
               codepoint == 0x0020 ||
               codepoint == 0x0085 ||
               codepoint == 0x00A0 ||
               codepoint == 0x1680 ||
               (codepoint >= 0x2000 && codepoint <= 0x200A) ||
               codepoint == 0x2028 ||
               codepoint == 0x2029 ||
               codepoint == 0x202F ||
               codepoint == 0x205F ||
               codepoint == 0x3000;
    }
};
} // namespace bunch

namespace std
{
template <>
struct hash<::bunch::Bunch>
{
    size_t operator()(const ::bunch::Bunch &sequence) const noexcept
    {
        const auto &points = sequence.getPoints();
        size_t seed = 0;
        for (const auto &point : points)
        {
            size_t value = hash<unsigned int>()(point.getValue());
            seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

} // namespace std
