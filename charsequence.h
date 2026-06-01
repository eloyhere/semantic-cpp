#pragma once
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstddef>
#include <limits>
#include <vector>
#include <deque>
#include <sstream>
#include <iterator>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <mutex>

namespace charsequence
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
		if (byte < 0xF8)
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
	else if (encoding == charset::utf16 || encoding == charset::utf16le)
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
	else if (encoding == charset::utf16be)
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
	else if (encoding == charset::utf32 || encoding == charset::utf32le)
	{
		result.push_back(static_cast<unsigned char>(codepoint & 0xFF));
		result.push_back(static_cast<unsigned char>((codepoint >> 8) & 0xFF));
		result.push_back(static_cast<unsigned char>((codepoint >> 16) & 0xFF));
		result.push_back(static_cast<unsigned char>((codepoint >> 24) & 0xFF));
	}
	else if (encoding == charset::utf32be)
	{
		result.push_back(static_cast<unsigned char>((codepoint >> 24) & 0xFF));
		result.push_back(static_cast<unsigned char>((codepoint >> 16) & 0xFF));
		result.push_back(static_cast<unsigned char>((codepoint >> 8) & 0xFF));
		result.push_back(static_cast<unsigned char>(codepoint & 0xFF));
	}
	return result;
}

inline unsigned int decode(const std::vector<unsigned char> &data, std::size_t &index, charset encoding)
{
	if (index >= data.size())
	{
		return 0xFFFD;
	}

	if (encoding == charset::latin1)
	{
		unsigned char byte = data[index];
		index++;
		return byte;
	}
	else if (encoding == charset::ascii)
	{
		unsigned char byte = data[index];
		index++;
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
		unsigned char byte = data[index];
		if (byte < 0x80)
		{
			index++;
			return byte;
		}
		if (byte < 0xC2)
		{
			index++;
			return 0xFFFD;
		}
		std::size_t seqLen;
		if (byte < 0xE0)
		{
			seqLen = 2;
		}
		else if (byte < 0xF0)
		{
			seqLen = 3;
		}
		else if (byte < 0xF8)
		{
			seqLen = 4;
		}
		else
		{
			index++;
			return 0xFFFD;
		}
		if (index + seqLen > data.size())
		{
			index = data.size();
			return 0xFFFD;
		}
		unsigned int codepoint;
		if (seqLen == 2)
		{
			if ((data[index + 1] & 0xC0) != 0x80)
			{
				index += 1;
				return 0xFFFD;
			}
			codepoint = (byte & 0x1F) << 6;
			codepoint |= (data[index + 1] & 0x3F);
			if (codepoint < 0x80)
			{
				index += 2;
				return 0xFFFD;
			}
		}
		else if (seqLen == 3)
		{
			if ((data[index + 1] & 0xC0) != 0x80 || (data[index + 2] & 0xC0) != 0x80)
			{
				index += 1;
				return 0xFFFD;
			}
			codepoint = (byte & 0x0F) << 12;
			codepoint |= (data[index + 1] & 0x3F) << 6;
			codepoint |= (data[index + 2] & 0x3F);
			if (codepoint < 0x800)
			{
				index += 3;
				return 0xFFFD;
			}
			if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
			{
				index += 3;
				return 0xFFFD;
			}
		}
		else
		{
			if ((data[index + 1] & 0xC0) != 0x80 || (data[index + 2] & 0xC0) != 0x80 || (data[index + 3] & 0xC0) != 0x80)
			{
				index += 1;
				return 0xFFFD;
			}
			codepoint = (byte & 0x07) << 18;
			codepoint |= (data[index + 1] & 0x3F) << 12;
			codepoint |= (data[index + 2] & 0x3F) << 6;
			codepoint |= (data[index + 3] & 0x3F);
			if (codepoint < 0x10000)
			{
				index += 4;
				return 0xFFFD;
			}
			if (codepoint > 0x10FFFF)
			{
				index += 4;
				return 0xFFFD;
			}
		}
		index += seqLen;
		return codepoint;
	}
	else if (encoding == charset::utf16 || encoding == charset::utf16le)
	{
		if (index + 1 >= data.size())
		{
			index = data.size();
			return 0xFFFD;
		}
		unsigned short unit = data[index] | (data[index + 1] << 8);
		if (unit >= 0xD800 && unit <= 0xDBFF)
		{
			if (index + 3 >= data.size())
			{
				index = data.size();
				return 0xFFFD;
			}
			unsigned short low = data[index + 2] | (data[index + 3] << 8);
			if (low < 0xDC00 || low > 0xDFFF)
			{
				index += 2;
				return 0xFFFD;
			}
			unsigned int codepoint = 0x10000 + ((unit - 0xD800) << 10) + (low - 0xDC00);
			index += 4;
			return codepoint;
		}
		else if (unit >= 0xDC00 && unit <= 0xDFFF)
		{
			index += 2;
			return 0xFFFD;
		}
		else
		{
			index += 2;
			return unit;
		}
	}
	else if (encoding == charset::utf16be)
	{
		if (index + 1 >= data.size())
		{
			index = data.size();
			return 0xFFFD;
		}
		unsigned short unit = (data[index] << 8) | data[index + 1];
		if (unit >= 0xD800 && unit <= 0xDBFF)
		{
			if (index + 3 >= data.size())
			{
				index = data.size();
				return 0xFFFD;
			}
			unsigned short low = (data[index + 2] << 8) | data[index + 3];
			if (low < 0xDC00 || low > 0xDFFF)
			{
				index += 2;
				return 0xFFFD;
			}
			unsigned int codepoint = 0x10000 + ((unit - 0xD800) << 10) + (low - 0xDC00);
			index += 4;
			return codepoint;
		}
		else if (unit >= 0xDC00 && unit <= 0xDFFF)
		{
			index += 2;
			return 0xFFFD;
		}
		else
		{
			index += 2;
			return unit;
		}
	}
	else if (encoding == charset::utf32 || encoding == charset::utf32le)
	{
		if (index + 3 >= data.size())
		{
			index = data.size();
			return 0xFFFD;
		}
		unsigned int codepoint = data[index] | (data[index + 1] << 8) | (data[index + 2] << 16) | (data[index + 3] << 24);
		index += 4;
		if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
		{
			return 0xFFFD;
		}
		return codepoint;
	}
	else if (encoding == charset::utf32be)
	{
		if (index + 3 >= data.size())
		{
			index = data.size();
			return 0xFFFD;
		}
		unsigned int codepoint = (data[index] << 24) | (data[index + 1] << 16) | (data[index + 2] << 8) | data[index + 3];
		index += 4;
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
	std::vector<unsigned char> bytes(input.begin(), input.end());
	std::size_t index = 0;
	while (index < bytes.size())
	{
		unsigned int codepoint = decode(bytes, index, from);
		auto encoded = encode(codepoint, to);
		result.insert(result.end(), encoded.begin(), encoded.end());
	}
	return result;
}

inline std::vector<unsigned char> convert(const std::vector<unsigned char> &input, charset from, charset to)
{
	if (from == to)
	{
		return input;
	}
	std::vector<unsigned char> result;
	std::size_t index = 0;
	while (index < input.size())
	{
		unsigned int codepoint = decode(input, index, from);
		auto encoded = encode(codepoint, to);
		result.insert(result.end(), encoded.begin(), encoded.end());
	}
	return result;
}

inline std::vector<char> convert(const std::vector<char> &input, charset from, charset to)
{
	std::vector<unsigned char> bytes(input.begin(), input.end());
	std::vector<unsigned char> converted = convert(bytes, from, to);
	return std::vector<char>(converted.begin(), converted.end());
}

inline void convertTo(std::string_view input, charset from, charset to, std::vector<unsigned char> &output)
{
	if (from == to)
	{
		output.insert(output.end(), input.begin(), input.end());
		return;
	}
	std::vector<unsigned char> bytes(input.begin(), input.end());
	std::size_t index = 0;
	while (index < bytes.size())
	{
		unsigned int codepoint = decode(bytes, index, from);
		auto encoded = encode(codepoint, to);
		output.insert(output.end(), encoded.begin(), encoded.end());
	}
}

inline void convertToDeque(std::string_view input, charset from, charset to, std::deque<unsigned char> &output)
{
	if (from == to)
	{
		output.insert(output.end(), input.begin(), input.end());
		return;
	}
	std::vector<unsigned char> bytes(input.begin(), input.end());
	std::size_t index = 0;
	while (index < bytes.size())
	{
		unsigned int codepoint = decode(bytes, index, from);
		auto encoded = encode(codepoint, to);
		output.insert(output.end(), encoded.begin(), encoded.end());
	}
}

class Meta
{
  public:
	Meta() : value(0) {}
	explicit Meta(unsigned int initial) : value(initial) {}
	Meta(const Meta &other) : value(other.value) {}
	Meta(Meta &&other) noexcept : value(other.value) { other.value = 0; }

	Meta &operator=(const Meta &other)
	{
		if (this != &other)
		{
			value = other.value;
		}
		return *this;
	}

	Meta &operator=(Meta &&other) noexcept
	{
		if (this != &other)
		{
			value = other.value;
			other.value = 0;
		}
		return *this;
	}

	bool operator==(const Meta &other) const { return value == other.value; }
	bool operator!=(const Meta &other) const { return value != other.value; }
	bool operator<(const Meta &other) const { return value < other.value; }
	unsigned int getValue() const { return value; }

  private:
	unsigned int value;
};

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
	bool isValidCodePoint() const { return code <= 0x10FFFF && !isSurrogate(); }

  private:
	unsigned int code;
};

class Charsequence;
class Builder;
class Buffer;

class PointIterator
{
  public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = Point;
	using difference_type = std::ptrdiff_t;
	using pointer = const Point *;
	using reference = const Point &;

	PointIterator(std::vector<Point>::const_iterator iterator) : baseIterator(iterator) {}

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

class Charsequence
{
  public:
	Charsequence() : pointStorage(), storageEncoding(charset::utf8) {}

	explicit Charsequence(charset encoding) : pointStorage(), storageEncoding(encoding) {}

	Charsequence(std::string_view source, charset sourceEncoding = charset::utf8, charset targetEncoding = charset::utf8)
		: pointStorage(), storageEncoding(targetEncoding)
	{
		initializeFromBytes(reinterpret_cast<const unsigned char *>(source.data()), source.size(), sourceEncoding, targetEncoding);
	}

	Charsequence(std::istream &stream, std::size_t maxLength, charset sourceEncoding = charset::utf8, charset targetEncoding = charset::utf8)
		: pointStorage(), storageEncoding(targetEncoding)
	{
		std::string temp;
		temp.resize(maxLength);
		stream.read(&temp[0], maxLength);
		std::size_t bytesRead = stream.gcount();
		temp.resize(bytesRead);
		initializeFromBytes(reinterpret_cast<const unsigned char *>(temp.data()), temp.size(), sourceEncoding, targetEncoding);
	}

	Charsequence(std::stringstream &stream, charset sourceEncoding = charset::utf8, charset targetEncoding = charset::utf8)
		: pointStorage(), storageEncoding(targetEncoding)
	{
		std::string temp = stream.str();
		initializeFromBytes(reinterpret_cast<const unsigned char *>(temp.data()), temp.size(), sourceEncoding, targetEncoding);
	}

	Charsequence(const Meta *source, std::size_t length, charset sourceEncoding, charset targetEncoding = charset::utf8)
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
		initializeFromBytes(reinterpret_cast<const unsigned char *>(temp.data()), temp.size(), sourceEncoding, targetEncoding);
	}

	Charsequence(const Point *source, std::size_t length, charset targetEncoding = charset::utf8)
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

	Charsequence(const Charsequence &other)
		: pointStorage(other.pointStorage), storageEncoding(other.storageEncoding) {}

	Charsequence(Charsequence &&other) noexcept
		: pointStorage(std::move(other.pointStorage)), storageEncoding(other.storageEncoding)
	{
		other.storageEncoding = charset::utf8;
	}

	Charsequence &operator=(const Charsequence &other)
	{
		if (this != &other)
		{
			pointStorage = other.pointStorage;
			storageEncoding = other.storageEncoding;
		}
		return *this;
	}

	Charsequence &operator=(Charsequence &&other) noexcept
	{
		if (this != &other)
		{
			pointStorage = std::move(other.pointStorage);
			storageEncoding = other.storageEncoding;
			other.storageEncoding = charset::utf8;
		}
		return *this;
	}

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

	Charsequence sub(std::size_t start, std::size_t length) const
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
			Charsequence result;
			result.storageEncoding = storageEncoding;
			return result;
		}
		Charsequence result;
		result.storageEncoding = storageEncoding;
		for (std::size_t i = start; i < start + length && i < totalPoints; ++i)
		{
			result.pointStorage.push_back(pointStorage[i]);
		}
		return result;
	}

	Charsequence repeat(std::size_t count) const
	{
		if (count == 0)
		{
			Charsequence result;
			result.storageEncoding = storageEncoding;
			return result;
		}
		if (count > 1 && pointStorage.size() > std::numeric_limits<std::size_t>::max() / count)
		{
			throw std::overflow_error("repeat size overflow");
		}
		Charsequence result;
		result.storageEncoding = storageEncoding;
		result.pointStorage.reserve(pointStorage.size() * count);
		for (std::size_t i = 0; i < count; ++i)
		{
			result.pointStorage.insert(result.pointStorage.end(), pointStorage.begin(), pointStorage.end());
		}
		return result;
	}

	Charsequence concat(const Charsequence &other) const
	{
		Charsequence result;
		result.storageEncoding = storageEncoding;
		result.pointStorage.reserve(pointStorage.size() + other.pointStorage.size());
		result.pointStorage.insert(result.pointStorage.end(), pointStorage.begin(), pointStorage.end());
		result.pointStorage.insert(result.pointStorage.end(), other.pointStorage.begin(), other.pointStorage.end());
		return result;
	}

	bool startsWith(const Charsequence &other) const
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

	bool startsWith(std::string_view str) const
	{
		Charsequence temp(str, storageEncoding, storageEncoding);
		return startsWith(temp);
	}

	bool endsWith(const Charsequence &other) const
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

	bool endsWith(std::string_view str) const
	{
		Charsequence temp(str, storageEncoding, storageEncoding);
		return endsWith(temp);
	}

	bool contains(const Charsequence &other) const
	{
		return indexOf(other) != static_cast<std::size_t>(-1);
	}

	bool contains(std::string_view str) const
	{
		Charsequence temp(str, storageEncoding, storageEncoding);
		return contains(temp);
	}

	std::size_t indexOf(const Charsequence &other, std::size_t fromCodePoint = 0) const
	{
		if (fromCodePoint >= pointStorage.size())
		{
			return static_cast<std::size_t>(-1);
		}
		const auto &otherPoints = other.pointStorage;
		if (otherPoints.empty())
		{
			return 0;
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

	std::size_t lastIndexOf(const Charsequence &other, std::size_t fromCodePoint = static_cast<std::size_t>(-1)) const
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

	Charsequence replace(const Charsequence &target, const Charsequence &replacement) const
	{
		if (target.pointStorage.empty())
		{
			return *this;
		}
		Charsequence result;
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

	Charsequence replace(std::string_view target, std::string_view replacement) const
	{
		Charsequence targetCs(target, storageEncoding, storageEncoding);
		Charsequence replacementCs(replacement, storageEncoding, storageEncoding);
		return replace(targetCs, replacementCs);
	}

	Charsequence replace(const Charsequence &target, const Charsequence &replacement, std::size_t maxCount) const
	{
		if (target.pointStorage.empty() || maxCount == 0)
		{
			return *this;
		}
		Charsequence result;
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

	std::size_t count(const Charsequence &target) const
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

	Charsequence trim() const
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

	Charsequence toUpperCase() const
	{
		Charsequence result;
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

	Charsequence toLowerCase() const
	{
		Charsequence result;
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

	Charsequence reverse() const
	{
		Charsequence result;
		result.storageEncoding = storageEncoding;
		result.pointStorage.reserve(pointStorage.size());
		for (auto it = pointStorage.rbegin(); it != pointStorage.rend(); ++it)
		{
			result.pointStorage.push_back(*it);
		}
		return result;
	}

	std::vector<Charsequence> split(const Charsequence &delimiter) const
	{
		std::vector<Charsequence> result;
		if (pointStorage.empty())
		{
			return result;
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

	std::vector<Charsequence> split(std::string_view delimiter) const
	{
		Charsequence delim(delimiter, storageEncoding, storageEncoding);
		return split(delim);
	}

	static Charsequence join(const std::vector<Charsequence> &parts, const Charsequence &separator)
	{
		if (parts.empty())
		{
			return Charsequence();
		}
		Charsequence result = parts[0];
		for (std::size_t i = 1; i < parts.size(); ++i)
		{
			result = result.concat(separator).concat(parts[i]);
		}
		return result;
	}

	Charsequence toString(charset targetEncoding) const
	{
		if (targetEncoding == storageEncoding)
		{
			return *this;
		}
		Charsequence result;
		result.storageEncoding = targetEncoding;
		for (const auto &point : pointStorage)
		{
			auto encoded = encode(point.getValue(), targetEncoding);
			std::size_t index = 0;
			while (index < encoded.size())
			{
				result.pointStorage.push_back(Point(decode(encoded, index, targetEncoding)));
			}
		}
		return result;
	}

	int compare(const Charsequence &other) const
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

	Charsequence operator+(const Charsequence &other) const
	{
		Charsequence result;
		result.storageEncoding = storageEncoding;
		result.pointStorage.reserve(pointStorage.size() + other.pointStorage.size());
		result.pointStorage.insert(result.pointStorage.end(), pointStorage.begin(), pointStorage.end());
		result.pointStorage.insert(result.pointStorage.end(), other.pointStorage.begin(), other.pointStorage.end());
		return result;
	}

	Charsequence operator+(std::string_view str) const
	{
		Charsequence temp(str, storageEncoding, storageEncoding);
		return *this + temp;
	}

	Charsequence &operator+=(const Charsequence &other)
	{
		pointStorage.insert(pointStorage.end(), other.pointStorage.begin(), other.pointStorage.end());
		return *this;
	}

	Charsequence &operator+=(std::string_view other)
	{
		Charsequence temp(other, storageEncoding, storageEncoding);
		return *this += temp;
	}

	bool operator==(const Charsequence &other) const { return compare(other) == 0; }
	bool operator!=(const Charsequence &other) const { return compare(other) != 0; }
	bool operator<(const Charsequence &other) const { return compare(other) < 0; }
	bool operator<=(const Charsequence &other) const { return compare(other) <= 0; }
	bool operator>(const Charsequence &other) const { return compare(other) > 0; }
	bool operator>=(const Charsequence &other) const { return compare(other) >= 0; }

	Point operator[](std::size_t index) const { return at(index); }

	friend std::ostream &operator<<(std::ostream &stream, const Charsequence &sequence)
	{
		auto bytes = sequence.getBytes();
		stream.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
		return stream;
	}

	friend std::istream &operator>>(std::istream &stream, Charsequence &sequence)
	{
		std::string temp;
		std::getline(stream, temp);
		sequence = Charsequence(std::string_view(temp), charset::utf8, sequence.storageEncoding);
		return stream;
	}

	friend std::stringstream &operator<<(std::stringstream &stream, const Charsequence &sequence)
	{
		auto bytes = sequence.getBytes();
		stream.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
		return stream;
	}

	friend std::stringstream &operator>>(std::stringstream &stream, Charsequence &sequence)
	{
		std::string temp = stream.str();
		stream.str("");
		stream.clear();
		sequence = Charsequence(std::string_view(temp), charset::utf8, sequence.storageEncoding);
		return stream;
	}

	friend Charsequence operator+(std::string_view left, const Charsequence &right)
	{
		Charsequence temp(left, right.storageEncoding, right.storageEncoding);
		return temp + right;
	}

	PointIterator begin() const { return PointIterator(pointStorage.cbegin()); }
	PointIterator end() const { return PointIterator(pointStorage.cend()); }

  private:
	std::vector<Point> pointStorage;
	charset storageEncoding;

	static bool isWhitespace(unsigned int codepoint)
	{
		return codepoint == ' ' || codepoint == '\t' || codepoint == '\n' || codepoint == '\r' || codepoint == '\v' || codepoint == '\f';
	}

	void initializeFromBytes(const unsigned char *data, std::size_t length, charset sourceEncoding, charset targetEncoding)
	{
		std::vector<unsigned char> bytes;
		if (sourceEncoding == targetEncoding)
		{
			bytes.assign(data, data + length);
		}
		else
		{
			convertTo(std::string_view(reinterpret_cast<const char *>(data), length), sourceEncoding, targetEncoding, bytes);
		}
		std::size_t index = 0;
		while (index < bytes.size())
		{
			pointStorage.push_back(Point(decode(bytes, index, targetEncoding)));
		}
	}
};

class Builder
{
  public:
	Builder() : storage(), storageEncoding(charset::utf8) {}
	explicit Builder(charset encoding) : storage(), storageEncoding(encoding) {}

	Builder(const Builder &other) : storage(other.storage), storageEncoding(other.storageEncoding) {}
	Builder(Builder &&other) noexcept : storage(std::move(other.storage)), storageEncoding(other.storageEncoding)
	{
		other.storageEncoding = charset::utf8;
	}

	Builder &operator=(const Builder &other)
	{
		if (this != &other)
		{
			storage = other.storage;
			storageEncoding = other.storageEncoding;
		}
		return *this;
	}

	Builder &operator=(Builder &&other) noexcept
	{
		if (this != &other)
		{
			storage = std::move(other.storage);
			storageEncoding = other.storageEncoding;
			other.storageEncoding = charset::utf8;
		}
		return *this;
	}

	Builder(std::string_view source, charset sourceEncoding = charset::utf8, charset targetEncoding = charset::utf8)
		: storage(), storageEncoding(targetEncoding)
	{
		if (sourceEncoding == targetEncoding)
		{
			storage.assign(source.begin(), source.end());
		}
		else
		{
			convertToDeque(source, sourceEncoding, targetEncoding, storage);
		}
	}

	Builder(std::istream &stream, std::size_t maxLength, charset sourceEncoding = charset::utf8, charset targetEncoding = charset::utf8)
		: storage(), storageEncoding(targetEncoding)
	{
		std::string chunk;
		std::deque<unsigned char> leftover;
		constexpr std::size_t chunkSize = 4096;
		std::size_t totalRead = 0;
		while (totalRead < maxLength && stream.good())
		{
			std::size_t toRead = std::min(chunkSize, maxLength - totalRead);
			chunk.resize(toRead);
			stream.read(&chunk[0], toRead);
			std::size_t bytesRead = stream.gcount();
			if (bytesRead == 0)
			{
				break;
			}
			chunk.resize(bytesRead);
			std::deque<unsigned char> combined;
			if (!leftover.empty())
			{
				combined = std::move(leftover);
				leftover.clear();
				combined.insert(combined.end(), chunk.begin(), chunk.end());
			}
			else
			{
				combined.assign(chunk.begin(), chunk.end());
			}
			std::size_t safeSize = combined.size();
			if (sourceEncoding == charset::utf8 && safeSize > 0)
			{
				std::size_t trailing = 0;
				auto it = combined.end();
				while (it != combined.begin())
				{
					--it;
					unsigned char b = *it;
					if (b < 0x80)
					{
						break;
					}
					if (b >= 0xC0 && b < 0xF8)
					{
						std::size_t expectedLen = sequenceLength(b, charset::utf8);
						std::size_t remaining = static_cast<std::size_t>(combined.end() - it);
						if (remaining < expectedLen)
						{
							trailing = remaining;
						}
						break;
					}
				}
				if (trailing > 0)
				{
					auto startIt = combined.end();
					std::advance(startIt, -static_cast<std::ptrdiff_t>(trailing));
					leftover.assign(startIt, combined.end());
					safeSize -= trailing;
				}
			}
			auto endIt = combined.begin();
			std::advance(endIt, safeSize);
			if (sourceEncoding == targetEncoding)
			{
				storage.insert(storage.end(), combined.begin(), endIt);
			}
			else
			{
				std::string temp(combined.begin(), endIt);
				std::string convertedStr = convert(std::string_view(temp), sourceEncoding, targetEncoding);
				storage.insert(storage.end(), convertedStr.begin(), convertedStr.end());
			}
			totalRead += bytesRead;
		}
		for (auto byte : leftover)
		{
			storage.push_back(byte);
		}
	}

	Builder(std::stringstream &stream, charset sourceEncoding = charset::utf8, charset targetEncoding = charset::utf8)
		: storage(), storageEncoding(targetEncoding)
	{
		std::string temp = stream.str();
		if (sourceEncoding == targetEncoding)
		{
			storage.assign(temp.begin(), temp.end());
		}
		else
		{
			convertToDeque(std::string_view(temp), sourceEncoding, targetEncoding, storage);
		}
	}

	Builder &prepend(const Charsequence &source)
	{
		auto bytes = source.getBytes(storageEncoding);
		storage.insert(storage.begin(), bytes.begin(), bytes.end());
		return *this;
	}

	Builder &prepend(std::string_view source)
	{
		Charsequence temp(source, storageEncoding, storageEncoding);
		return prepend(temp);
	}

	Builder &prepend(Point point)
	{
		Charsequence temp(&point, 1, storageEncoding);
		return prepend(temp);
	}

	Builder &prepend(unsigned char byte)
	{
		storage.push_front(byte);
		return *this;
	}

	Builder &prepend(char byte)
	{
		storage.push_front(static_cast<unsigned char>(byte));
		return *this;
	}

	Builder &insert(std::size_t position, const Charsequence &source)
	{
		auto bytes = source.getBytes(storageEncoding);
		if (position >= storage.size())
		{
			storage.insert(storage.end(), bytes.begin(), bytes.end());
		}
		else
		{
			auto it = storage.begin();
			std::advance(it, position);
			storage.insert(it, bytes.begin(), bytes.end());
		}
		return *this;
	}

	Builder &insert(std::size_t position, std::string_view source)
	{
		Charsequence temp(source, storageEncoding, storageEncoding);
		return insert(position, temp);
	}

	Builder &insert(std::size_t position, Point point)
	{
		Charsequence temp(&point, 1, storageEncoding);
		return insert(position, temp);
	}

	Builder &insert(std::size_t position, unsigned char byte)
	{
		if (position >= storage.size())
		{
			storage.push_back(byte);
		}
		else
		{
			auto it = storage.begin();
			std::advance(it, position);
			storage.insert(it, byte);
		}
		return *this;
	}

	Builder &insert(std::size_t position, char byte)
	{
		return insert(position, static_cast<unsigned char>(byte));
	}

	Builder &insert(std::size_t position, bool value)
	{
		std::string str = value ? "true" : "false";
		return insert(position, std::string_view(str));
	}

	Builder &insert(std::size_t position, short value)
	{
		std::string str = std::to_string(value);
		return insert(position, std::string_view(str));
	}

	Builder &insert(std::size_t position, int value)
	{
		std::string str = std::to_string(value);
		return insert(position, std::string_view(str));
	}

	Builder &insert(std::size_t position, long value)
	{
		std::string str = std::to_string(value);
		return insert(position, std::string_view(str));
	}

	Builder &insert(std::size_t position, long long value)
	{
		std::string str = std::to_string(value);
		return insert(position, std::string_view(str));
	}

	Builder &insert(std::size_t position, float value)
	{
		std::ostringstream stream;
		stream << value;
		std::string str = stream.str();
		return insert(position, std::string_view(str));
	}

	Builder &insert(std::size_t position, double value)
	{
		std::ostringstream stream;
		stream << value;
		std::string str = stream.str();
		return insert(position, std::string_view(str));
	}

	Builder &insert(std::size_t position, long double value)
	{
		std::ostringstream stream;
		stream << value;
		std::string str = stream.str();
		return insert(position, std::string_view(str));
	}

	Builder &insert(std::size_t position, unsigned short value)
	{
		std::string str = std::to_string(value);
		return insert(position, std::string_view(str));
	}

	Builder &insert(std::size_t position, unsigned int value)
	{
		std::string str = std::to_string(value);
		return insert(position, std::string_view(str));
	}

	Builder &insert(std::size_t position, unsigned long value)
	{
		std::string str = std::to_string(value);
		return insert(position, std::string_view(str));
	}

	Builder &insert(std::size_t position, unsigned long long value)
	{
		std::string str = std::to_string(value);
		return insert(position, std::string_view(str));
	}

	Builder &append(const Charsequence &source)
	{
		auto bytes = source.getBytes(storageEncoding);
		storage.insert(storage.end(), bytes.begin(), bytes.end());
		return *this;
	}

	Builder &append(std::string_view source)
	{
		Charsequence temp(source, storageEncoding, storageEncoding);
		return append(temp);
	}

	Builder &append(Point point)
	{
		Charsequence temp(&point, 1, storageEncoding);
		auto bytes = temp.getBytes(storageEncoding);
		storage.insert(storage.end(), bytes.begin(), bytes.end());
		return *this;
	}

	Builder &append(unsigned char byte)
	{
		storage.push_back(byte);
		return *this;
	}

	Builder &append(char byte)
	{
		storage.push_back(static_cast<unsigned char>(byte));
		return *this;
	}

	Builder &append(bool value)
	{
		std::string str = value ? "true" : "false";
		return append(std::string_view(str));
	}

	Builder &append(short value)
	{
		std::string str = std::to_string(value);
		return append(std::string_view(str));
	}

	Builder &append(int value)
	{
		std::string str = std::to_string(value);
		return append(std::string_view(str));
	}

	Builder &append(long value)
	{
		std::string str = std::to_string(value);
		return append(std::string_view(str));
	}

	Builder &append(long long value)
	{
		std::string str = std::to_string(value);
		return append(std::string_view(str));
	}

	Builder &append(float value)
	{
		std::ostringstream stream;
		stream << value;
		std::string str = stream.str();
		return append(std::string_view(str));
	}

	Builder &append(double value)
	{
		std::ostringstream stream;
		stream << value;
		std::string str = stream.str();
		return append(std::string_view(str));
	}

	Builder &append(long double value)
	{
		std::ostringstream stream;
		stream << value;
		std::string str = stream.str();
		return append(std::string_view(str));
	}

	Builder &append(unsigned short value)
	{
		std::string str = std::to_string(value);
		return append(std::string_view(str));
	}

	Builder &append(unsigned int value)
	{
		std::string str = std::to_string(value);
		return append(std::string_view(str));
	}

	Builder &append(unsigned long value)
	{
		std::string str = std::to_string(value);
		return append(std::string_view(str));
	}

	Builder &append(unsigned long long value)
	{
		std::string str = std::to_string(value);
		return append(std::string_view(str));
	}

	Builder operator+(const Builder &other) const
	{
		Builder result(*this);
		result.append(other.toCharsequence());
		return result;
	}

	Builder operator+(const Charsequence &other) const
	{
		Builder result(*this);
		result.append(other);
		return result;
	}

	Builder operator+(std::string_view str) const
	{
		Builder result(*this);
		result.append(str);
		return result;
	}

	Builder &operator+=(const Builder &other)
	{
		append(other.toCharsequence());
		return *this;
	}

	Builder &operator+=(const Charsequence &other)
	{
		append(other);
		return *this;
	}

	Builder &operator+=(std::string_view str)
	{
		append(str);
		return *this;
	}

	Charsequence toCharsequence() const
	{
		std::string temp(storage.begin(), storage.end());
		return Charsequence(std::string_view(temp), storageEncoding, storageEncoding);
	}

	std::size_t size() const { return storage.size(); }
	bool empty() const { return storage.empty(); }
	charset encoding() const { return storageEncoding; }

	std::vector<unsigned char> getBytes() const
	{
		return std::vector<unsigned char>(storage.begin(), storage.end());
	}

	friend std::ostream &operator<<(std::ostream &stream, const Builder &builder)
	{
		auto bytes = builder.getBytes();
		stream.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
		return stream;
	}

	friend std::istream &operator>>(std::istream &stream, Builder &builder)
	{
		std::string temp;
		std::getline(stream, temp);
		builder = Builder(std::string_view(temp), builder.storageEncoding, builder.storageEncoding);
		return stream;
	}

	friend std::stringstream &operator<<(std::stringstream &stream, const Builder &builder)
	{
		auto bytes = builder.getBytes();
		stream.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
		return stream;
	}

	friend std::stringstream &operator>>(std::stringstream &stream, Builder &builder)
	{
		std::string temp = stream.str();
		stream.str("");
		stream.clear();
		builder = Builder(std::string_view(temp), builder.storageEncoding, builder.storageEncoding);
		return stream;
	}

	friend Builder operator+(std::string_view left, const Builder &right)
	{
		Builder result(left, right.storageEncoding, right.storageEncoding);
		result.append(right.toCharsequence());
		return result;
	}

  private:
	std::deque<unsigned char> storage;
	charset storageEncoding;
};

class Buffer
{
  public:
	static constexpr std::size_t defaultCapacity = 4096;

	Buffer() : storage(defaultCapacity), readPosition(0), writePosition(0), elementCount(0), mutex() {}
	explicit Buffer(std::size_t capacity) : storage(capacity), readPosition(0), writePosition(0), elementCount(0), mutex() {}

	Buffer(std::string_view source) : storage(source.size() > 0 ? source.size() : defaultCapacity), readPosition(0), writePosition(0), elementCount(0), mutex()
	{
		write(source);
	}

	Buffer(std::istream &stream, std::size_t maxLength) : storage(maxLength > 0 ? maxLength : defaultCapacity), readPosition(0), writePosition(0), elementCount(0), mutex()
	{
		std::string temp;
		temp.resize(maxLength);
		stream.read(&temp[0], maxLength);
		std::size_t bytesRead = stream.gcount();
		temp.resize(bytesRead);
		write(std::string_view(temp));
	}

	Buffer(std::stringstream &stream) : storage(defaultCapacity), readPosition(0), writePosition(0), elementCount(0), mutex()
	{
		std::string temp = stream.str();
		if (temp.size() > 0)
		{
			storage.resize(temp.size());
		}
		write(std::string_view(temp));
	}

	Buffer(const Buffer &other) : storage(), readPosition(0), writePosition(0), elementCount(0), mutex()
	{
		std::lock_guard<std::mutex> lock(other.mutex);
		storage = other.storage;
		readPosition = other.readPosition;
		writePosition = other.writePosition;
		elementCount = other.elementCount;
	}

	Buffer(Buffer &&other) noexcept : storage(), readPosition(0), writePosition(0), elementCount(0), mutex()
	{
		std::lock_guard<std::mutex> lock(other.mutex);
		storage = std::move(other.storage);
		readPosition = other.readPosition;
		writePosition = other.writePosition;
		elementCount = other.elementCount;
		other.readPosition = 0;
		other.writePosition = 0;
		other.elementCount = 0;
	}

	Buffer &operator=(const Buffer &other)
	{
		if (this == &other)
		{
			return *this;
		}
		std::lock(mutex, other.mutex);
		std::lock_guard<std::mutex> lockThis(mutex, std::adopt_lock);
		std::lock_guard<std::mutex> lockOther(other.mutex, std::adopt_lock);
		storage = other.storage;
		readPosition = other.readPosition;
		writePosition = other.writePosition;
		elementCount = other.elementCount;
		return *this;
	}

	Buffer &operator=(Buffer &&other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}
		std::unique_lock<std::mutex> lockThis(mutex, std::defer_lock);
		std::unique_lock<std::mutex> lockOther(other.mutex, std::defer_lock);
		std::lock(lockThis, lockOther);
		storage = std::move(other.storage);
		readPosition = other.readPosition;
		writePosition = other.writePosition;
		elementCount = other.elementCount;
		other.readPosition = 0;
		other.writePosition = 0;
		other.elementCount = 0;
		return *this;
	}

	std::size_t write(const char *input, std::size_t length)
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (!input || length == 0)
		{
			return 0;
		}
		ensureCapacity(length);
		std::size_t written = 0;
		while (written < length)
		{
			std::size_t space = storage.size() - writePosition;
			std::size_t chunk = std::min(length - written, space);
			std::memcpy(&storage[writePosition], input + written, chunk);
			writePosition = (writePosition + chunk) % storage.size();
			written += chunk;
		}
		elementCount += written;
		return written;
	}

	std::size_t write(std::string_view input)
	{
		return write(input.data(), input.size());
	}

	std::size_t write(const std::vector<unsigned char> &input)
	{
		return write(reinterpret_cast<const char *>(input.data()), input.size());
	}

	std::size_t write(const Charsequence &input)
	{
		auto bytes = input.getBytes();
		return write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
	}

	std::size_t write(const Builder &input)
	{
		auto bytes = input.getBytes();
		return write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
	}

	std::size_t write(unsigned char byte)
	{
		return write(reinterpret_cast<const char *>(&byte), 1);
	}

	std::size_t write(char byte)
	{
		return write(&byte, 1);
	}

	std::size_t write(bool value)
	{
		std::string str = value ? "true" : "false";
		return write(std::string_view(str));
	}

	std::size_t write(short value)
	{
		std::string str = std::to_string(value);
		return write(std::string_view(str));
	}

	std::size_t write(int value)
	{
		std::string str = std::to_string(value);
		return write(std::string_view(str));
	}

	std::size_t write(long value)
	{
		std::string str = std::to_string(value);
		return write(std::string_view(str));
	}

	std::size_t write(long long value)
	{
		std::string str = std::to_string(value);
		return write(std::string_view(str));
	}

	std::size_t write(float value)
	{
		std::ostringstream stream;
		stream << value;
		std::string str = stream.str();
		return write(std::string_view(str));
	}

	std::size_t write(double value)
	{
		std::ostringstream stream;
		stream << value;
		std::string str = stream.str();
		return write(std::string_view(str));
	}

	std::size_t write(long double value)
	{
		std::ostringstream stream;
		stream << value;
		std::string str = stream.str();
		return write(std::string_view(str));
	}

	std::size_t write(unsigned short value)
	{
		std::string str = std::to_string(value);
		return write(std::string_view(str));
	}

	std::size_t write(unsigned int value)
	{
		std::string str = std::to_string(value);
		return write(std::string_view(str));
	}

	std::size_t write(unsigned long value)
	{
		std::string str = std::to_string(value);
		return write(std::string_view(str));
	}

	std::size_t write(unsigned long long value)
	{
		std::string str = std::to_string(value);
		return write(std::string_view(str));
	}

	std::size_t prepend(const char *input, std::size_t length)
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (!input || length == 0)
		{
			return 0;
		}
		ensureCapacity(length);
		for (std::size_t i = length; i > 0; --i)
		{
			readPosition = (readPosition + storage.size() - 1) % storage.size();
			storage[readPosition] = static_cast<unsigned char>(input[i - 1]);
		}
		elementCount += length;
		return length;
	}

	std::size_t prepend(std::string_view input)
	{
		return prepend(input.data(), input.size());
	}

	std::size_t prepend(unsigned char byte)
	{
		return prepend(reinterpret_cast<const char *>(&byte), 1);
	}

	std::size_t prepend(char byte)
	{
		return prepend(&byte, 1);
	}

	std::size_t append(const char *input, std::size_t length)
	{
		return write(input, length);
	}

	std::size_t append(std::string_view input)
	{
		return write(input);
	}

	std::size_t append(const std::vector<unsigned char> &input)
	{
		return write(input);
	}

	std::size_t append(const Charsequence &input)
	{
		return write(input);
	}

	std::size_t append(const Builder &input)
	{
		return write(input);
	}

	std::size_t append(unsigned char byte)
	{
		return write(byte);
	}

	std::size_t append(char byte)
	{
		return write(byte);
	}

	std::size_t append(bool value)
	{
		return write(value);
	}

	std::size_t append(short value)
	{
		return write(value);
	}

	std::size_t append(int value)
	{
		return write(value);
	}

	std::size_t append(long value)
	{
		return write(value);
	}

	std::size_t append(long long value)
	{
		return write(value);
	}

	std::size_t append(float value)
	{
		return write(value);
	}

	std::size_t append(double value)
	{
		return write(value);
	}

	std::size_t append(long double value)
	{
		return write(value);
	}

	std::size_t append(unsigned short value)
	{
		return write(value);
	}

	std::size_t append(unsigned int value)
	{
		return write(value);
	}

	std::size_t append(unsigned long value)
	{
		return write(value);
	}

	std::size_t append(unsigned long long value)
	{
		return write(value);
	}

	std::vector<unsigned char> read(std::size_t maxLength)
	{
		std::lock_guard<std::mutex> lock(mutex);
		std::size_t length = std::min(maxLength, elementCount);
		std::vector<unsigned char> result;
		result.reserve(length);
		std::size_t remaining = length;
		while (remaining > 0)
		{
			std::size_t available = storage.size() - readPosition;
			std::size_t chunk = std::min(remaining, available);
			result.insert(result.end(), storage.begin() + readPosition, storage.begin() + readPosition + chunk);
			readPosition = (readPosition + chunk) % storage.size();
			remaining -= chunk;
		}
		elementCount -= length;
		return result;
	}

	std::size_t read(char *output, std::size_t maxLength)
	{
		std::lock_guard<std::mutex> lock(mutex);
		std::size_t length = std::min(maxLength, elementCount);
		if (length == 0)
		{
			return 0;
		}
		std::size_t remaining = length;
		std::size_t offset = 0;
		while (remaining > 0)
		{
			std::size_t available = storage.size() - readPosition;
			std::size_t chunk = std::min(remaining, available);
			std::memcpy(output + offset, &storage[readPosition], chunk);
			readPosition = (readPosition + chunk) % storage.size();
			offset += chunk;
			remaining -= chunk;
		}
		elementCount -= length;
		return length;
	}

	std::vector<unsigned char> peek(std::size_t maxLength) const
	{
		std::lock_guard<std::mutex> lock(mutex);
		std::size_t length = std::min(maxLength, elementCount);
		std::vector<unsigned char> result;
		result.reserve(length);
		std::size_t remaining = length;
		std::size_t pos = readPosition;
		while (remaining > 0)
		{
			std::size_t available = storage.size() - pos;
			std::size_t chunk = std::min(remaining, available);
			result.insert(result.end(), storage.begin() + pos, storage.begin() + pos + chunk);
			pos = (pos + chunk) % storage.size();
			remaining -= chunk;
		}
		return result;
	}

	std::size_t peek(char *output, std::size_t maxLength) const
	{
		std::lock_guard<std::mutex> lock(mutex);
		std::size_t length = std::min(maxLength, elementCount);
		if (length == 0)
		{
			return 0;
		}
		std::size_t remaining = length;
		std::size_t offset = 0;
		std::size_t pos = readPosition;
		while (remaining > 0)
		{
			std::size_t available = storage.size() - pos;
			std::size_t chunk = std::min(remaining, available);
			std::memcpy(output + offset, &storage[pos], chunk);
			pos = (pos + chunk) % storage.size();
			offset += chunk;
			remaining -= chunk;
		}
		return length;
	}

	std::vector<unsigned char> data() const
	{
		std::lock_guard<std::mutex> lock(mutex);
		std::vector<unsigned char> result;
		result.reserve(elementCount);
		std::size_t remaining = elementCount;
		std::size_t pos = readPosition;
		while (remaining > 0)
		{
			std::size_t available = storage.size() - pos;
			std::size_t chunk = std::min(remaining, available);
			result.insert(result.end(), storage.begin() + pos, storage.begin() + pos + chunk);
			pos = (pos + chunk) % storage.size();
			remaining -= chunk;
		}
		return result;
	}

	const unsigned char *unsafeData() const noexcept
	{
		return storage.data() + readPosition;
	}

	std::size_t unsafeSize() const noexcept
	{
		return elementCount;
	}

	bool unsafeIsContiguous() const noexcept
	{
		return readPosition + elementCount <= storage.size();
	}

	std::size_t size() const
	{
		std::lock_guard<std::mutex> lock(mutex);
		return elementCount;
	}

	std::size_t capacity() const
	{
		std::lock_guard<std::mutex> lock(mutex);
		return storage.size();
	}

	void clear()
	{
		std::lock_guard<std::mutex> lock(mutex);
		readPosition = 0;
		writePosition = 0;
		elementCount = 0;
	}

	void shrinkToFit()
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (elementCount == 0)
		{
			storage.resize(defaultCapacity);
			readPosition = 0;
			writePosition = 0;
			return;
		}
		std::vector<unsigned char> newStorage(elementCount);
		std::size_t remaining = elementCount;
		std::size_t pos = readPosition;
		std::size_t offset = 0;
		while (remaining > 0)
		{
			std::size_t available = storage.size() - pos;
			std::size_t chunk = std::min(remaining, available);
			std::memcpy(&newStorage[offset], &storage[pos], chunk);
			pos = (pos + chunk) % storage.size();
			offset += chunk;
			remaining -= chunk;
		}
		storage = std::move(newStorage);
		readPosition = 0;
		writePosition = elementCount % storage.size();
	}

	template <typename Function>
	auto atomic(Function &&function) -> decltype(function(std::declval<std::vector<unsigned char> &>(), std::declval<std::size_t &>(), std::declval<std::size_t &>(), std::declval<std::size_t &>()))
	{
		std::lock_guard<std::mutex> lock(mutex);
		return function(storage, readPosition, writePosition, elementCount);
	}

	template <typename Function>
	auto atomic(Function &&function) const -> decltype(function(std::declval<const std::vector<unsigned char> &>(), std::declval<const std::size_t &>(), std::declval<const std::size_t &>(), std::declval<const std::size_t &>()))
	{
		std::lock_guard<std::mutex> lock(mutex);
		return function(storage, readPosition, writePosition, elementCount);
	}

	Buffer operator+(const Buffer &other) const
	{
		Buffer result;
		if (this == &other)
		{
			std::lock_guard<std::mutex> lock(mutex);
			auto data = peekAllUnsafe();
			result.writeDataUnsafe(reinterpret_cast<const char *>(data.data()), data.size());
			result.writeDataUnsafe(reinterpret_cast<const char *>(data.data()), data.size());
		}
		else
		{
			std::lock(mutex, other.mutex);
			std::lock_guard<std::mutex> lockThis(mutex, std::adopt_lock);
			std::lock_guard<std::mutex> lockOther(other.mutex, std::adopt_lock);
			auto thisData = peekAllUnsafe();
			auto otherData = other.peekAllUnsafe();
			result.writeDataUnsafe(reinterpret_cast<const char *>(thisData.data()), thisData.size());
			result.writeDataUnsafe(reinterpret_cast<const char *>(otherData.data()), otherData.size());
		}
		return result;
	}

	Buffer operator+(std::string_view str) const
	{
		Buffer result;
		std::lock_guard<std::mutex> lockThis(mutex);
		auto thisData = peekAllUnsafe();
		result.writeDataUnsafe(reinterpret_cast<const char *>(thisData.data()), thisData.size());
		result.writeDataUnsafe(str.data(), str.size());
		return result;
	}

	Buffer &operator+=(const Buffer &other)
	{
		if (this == &other)
		{
			std::lock_guard<std::mutex> lock(mutex);
			auto data = peekAllUnsafe();
			writeDataUnsafe(reinterpret_cast<const char *>(data.data()), data.size());
			writeDataUnsafe(reinterpret_cast<const char *>(data.data()), data.size());
		}
		else
		{
			std::lock(mutex, other.mutex);
			std::lock_guard<std::mutex> lockThis(mutex, std::adopt_lock);
			std::lock_guard<std::mutex> lockOther(other.mutex, std::adopt_lock);
			auto otherData = other.peekAllUnsafe();
			writeDataUnsafe(reinterpret_cast<const char *>(otherData.data()), otherData.size());
		}
		return *this;
	}

	Buffer &operator+=(std::string_view str)
	{
		write(str);
		return *this;
	}

	Buffer &operator<<(std::string_view input)
	{
		write(input);
		return *this;
	}

	Buffer &operator<<(unsigned char byte)
	{
		write(byte);
		return *this;
	}

	Buffer &operator<<(char byte)
	{
		write(byte);
		return *this;
	}

	Buffer &operator<<(bool value)
	{
		write(value);
		return *this;
	}

	Buffer &operator<<(short value)
	{
		write(value);
		return *this;
	}

	Buffer &operator<<(int value)
	{
		write(value);
		return *this;
	}

	Buffer &operator<<(long value)
	{
		write(value);
		return *this;
	}

	Buffer &operator<<(long long value)
	{
		write(value);
		return *this;
	}

	Buffer &operator<<(float value)
	{
		write(value);
		return *this;
	}

	Buffer &operator<<(double value)
	{
		write(value);
		return *this;
	}

	Buffer &operator<<(long double value)
	{
		write(value);
		return *this;
	}

	Buffer &operator>>(std::string &output)
	{
		std::lock_guard<std::mutex> lock(mutex);
		output.resize(elementCount);
		readDataUnsafe(output.data(), elementCount);
		readPosition = 0;
		writePosition = 0;
		elementCount = 0;
		return *this;
	}

	friend std::ostream &operator<<(std::ostream &stream, const Buffer &buffer)
	{
		auto data = buffer.data();
		stream.write(reinterpret_cast<const char *>(data.data()), data.size());
		return stream;
	}

	friend std::istream &operator>>(std::istream &stream, Buffer &buffer)
	{
		std::string temp;
		std::getline(stream, temp);
		buffer.write(std::string_view(temp));
		return stream;
	}

	friend std::stringstream &operator<<(std::stringstream &stream, const Buffer &buffer)
	{
		auto data = buffer.data();
		stream.write(reinterpret_cast<const char *>(data.data()), data.size());
		return stream;
	}

	friend std::stringstream &operator>>(std::stringstream &stream, Buffer &buffer)
	{
		std::string temp = stream.str();
		stream.str("");
		stream.clear();
		buffer.write(std::string_view(temp));
		return stream;
	}

	friend Buffer operator+(std::string_view left, const Buffer &right)
	{
		Buffer result;
		std::lock_guard<std::mutex> lockRight(right.mutex);
		auto rightData = right.peekAllUnsafe();
		result.writeDataUnsafe(left.data(), left.size());
		result.writeDataUnsafe(reinterpret_cast<const char *>(rightData.data()), rightData.size());
		return result;
	}

  private:
	std::vector<unsigned char> storage;
	std::size_t readPosition;
	std::size_t writePosition;
	std::size_t elementCount;
	mutable std::mutex mutex;

	void writeDataUnsafe(const char *input, std::size_t length)
	{
		if (!input || length == 0)
			return;
		ensureCapacity(length);
		std::size_t written = 0;
		while (written < length)
		{
			std::size_t space = storage.size() - writePosition;
			std::size_t chunk = std::min(length - written, space);
			std::memcpy(&storage[writePosition], input + written, chunk);
			writePosition = (writePosition + chunk) % storage.size();
			written += chunk;
		}
		elementCount += written;
	}

	void readDataUnsafe(char *output, std::size_t length)
	{
		if (!output || length == 0)
			return;
		std::size_t remaining = length;
		std::size_t offset = 0;
		while (remaining > 0)
		{
			std::size_t available = storage.size() - readPosition;
			std::size_t chunk = std::min(remaining, available);
			std::memcpy(output + offset, &storage[readPosition], chunk);
			readPosition = (readPosition + chunk) % storage.size();
			offset += chunk;
			remaining -= chunk;
		}
	}

	void ensureCapacity(std::size_t additional)
	{
		std::size_t required = elementCount + additional;
		if (required <= storage.size())
		{
			return;
		}
		if (storage.size() > std::numeric_limits<std::size_t>::max() / 2)
		{
			throw std::overflow_error("Buffer capacity overflow");
		}
		std::size_t newCapacity = storage.size() * 2;
		if (newCapacity < required)
		{
			newCapacity = required;
		}
		std::vector<unsigned char> newStorage(newCapacity);
		std::size_t remaining = elementCount;
		std::size_t pos = readPosition;
		std::size_t offset = 0;
		while (remaining > 0)
		{
			std::size_t available = storage.size() - pos;
			std::size_t chunk = std::min(remaining, available);
			std::memcpy(&newStorage[offset], &storage[pos], chunk);
			pos = (pos + chunk) % storage.size();
			offset += chunk;
			remaining -= chunk;
		}
		storage = std::move(newStorage);
		readPosition = 0;
		writePosition = elementCount;
	}

	std::vector<unsigned char> peekAllUnsafe() const
	{
		std::vector<unsigned char> result;
		result.reserve(elementCount);
		std::size_t remaining = elementCount;
		std::size_t pos = readPosition;
		while (remaining > 0)
		{
			std::size_t available = storage.size() - pos;
			std::size_t chunk = std::min(remaining, available);
			result.insert(result.end(), storage.begin() + pos, storage.begin() + pos + chunk);
			pos = (pos + chunk) % storage.size();
			remaining -= chunk;
		}
		return result;
	}
};

} // namespace charsequence

namespace std
{
template <>
struct hash<charsequence::Meta>
{
	size_t operator()(const charsequence::Meta &meta) const noexcept
	{
		return hash<unsigned int>()(meta.getValue());
	}
};

template <>
struct hash<charsequence::Point>
{
	size_t operator()(const charsequence::Point &point) const noexcept
	{
		return hash<unsigned int>()(point.getValue());
	}
};

template <>
struct hash<charsequence::Charsequence>
{
	size_t operator()(const charsequence::Charsequence &sequence) const noexcept
	{
		const auto &points = sequence.getPoints();
		size_t seed = points.size();
		for (const auto &point : points)
		{
			size_t value = hash<unsigned int>()(point.getValue());
			seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

template <>
struct hash<charsequence::Builder>
{
	size_t operator()(const charsequence::Builder &builder) const noexcept
	{
		auto bytes = builder.getBytes();
		size_t seed = bytes.size();
		for (auto byte : bytes)
		{
			seed ^= hash<unsigned char>()(byte) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

template <>
struct hash<charsequence::Buffer>
{
	size_t operator()(const charsequence::Buffer &buffer) const noexcept
	{
		auto data = buffer.data();
		size_t seed = data.size();
		for (auto byte : data)
		{
			seed ^= hash<unsigned char>()(byte) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

template <>
struct less<charsequence::Builder>
{
	bool operator()(const charsequence::Builder &left, const charsequence::Builder &right) const noexcept
	{
		return left.toCharsequence().compare(right.toCharsequence()) < 0;
	}
};

template <>
struct less<charsequence::Buffer>
{
	bool operator()(const charsequence::Buffer &left, const charsequence::Buffer &right) const noexcept
	{
		auto leftData = left.data();
		auto rightData = right.data();
		if (leftData.size() != rightData.size())
		{
			return leftData.size() < rightData.size();
		}
		for (size_t i = 0; i < leftData.size(); ++i)
		{
			if (leftData[i] != rightData[i])
			{
				return leftData[i] < rightData[i];
			}
		}
		return false;
	}
};

} // namespace std
