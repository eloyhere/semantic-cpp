#pragma once
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <memory>
#include <vector>
#include <sstream>
#include <regex>
#include <iterator>
#include <iostream>
#include <cassert>
#include <string>
#include <type_traits>

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
	latin1,
	gbk
};

class Meta
{
  public:
	Meta() : value(0) {}
	explicit Meta(unsigned int val) : value(val) {}
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
	using pointer = std::unique_ptr<Point>;
	using reference = Point;

	PointIterator(const Charsequence *sequence, std::size_t index)
		: charsequence(sequence), position(index) {}

	Point operator*() const;
	std::unique_ptr<Point> operator->() const;
	PointIterator &operator++();
	PointIterator operator++(int);
	PointIterator &operator--();
	PointIterator operator--(int);
	bool operator==(const PointIterator &other) const;
	bool operator!=(const PointIterator &other) const;

  private:
	const Charsequence *charsequence;
	std::size_t position;
};

class Charsequence
{
  public:
	Charsequence() : metaStorage(), pointStorage(), storageEncoding(charset::utf8) {}

	explicit Charsequence(charset encode) : metaStorage(), pointStorage(), storageEncoding(charset::utf8) {}

	explicit Charsequence(const std::string &source, charset sourceEncoding = charset::utf8)
		: metaStorage(), pointStorage(), storageEncoding(charset::utf8)
	{
		std::string temp(source);
		std::vector<unsigned char> bytes;
		if (sourceEncoding == charset::utf8)
		{
			bytes.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = convertEncoding(temp, sourceEncoding, charset::utf8);
			bytes.assign(converted.begin(), converted.end());
		}
		buildFromBytes(bytes);
	}

	explicit Charsequence(std::string &&source, charset sourceEncoding = charset::utf8)
		: metaStorage(), pointStorage(), storageEncoding(charset::utf8)
	{
		std::string temp(std::move(source));
		std::vector<unsigned char> bytes;
		if (sourceEncoding == charset::utf8)
		{
			bytes.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = convertEncoding(temp, sourceEncoding, charset::utf8);
			bytes.assign(converted.begin(), converted.end());
		}
		buildFromBytes(bytes);
	}

	explicit Charsequence(const char *source, charset sourceEncoding = charset::utf8)
		: metaStorage(), pointStorage(), storageEncoding(charset::utf8)
	{
		std::string temp(source ? source : "");
		std::vector<unsigned char> bytes;
		if (sourceEncoding == charset::utf8)
		{
			bytes.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = convertEncoding(temp, sourceEncoding, charset::utf8);
			bytes.assign(converted.begin(), converted.end());
		}
		buildFromBytes(bytes);
	}

	explicit Charsequence(std::istream &stream, std::size_t maxLength, charset sourceEncoding = charset::utf8)
		: metaStorage(), pointStorage(), storageEncoding(charset::utf8)
	{
		std::string temp;
		temp.resize(maxLength);
		stream.read(&temp[0], maxLength);
		std::size_t bytesRead = stream.gcount();
		temp.resize(bytesRead);
		std::vector<unsigned char> bytes;
		if (sourceEncoding == charset::utf8)
		{
			bytes.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = convertEncoding(temp, sourceEncoding, charset::utf8);
			bytes.assign(converted.begin(), converted.end());
		}
		buildFromBytes(bytes);
	}

	explicit Charsequence(const Meta *source, std::size_t length, charset sourceEncoding)
		: metaStorage(), pointStorage(), storageEncoding(charset::utf8)
	{
		std::string temp;
		temp.reserve(length);
		for (std::size_t i = 0; i < length; ++i)
		{
			temp.push_back(static_cast<char>(source[i].getValue() & 0xFF));
		}
		std::vector<unsigned char> bytes;
		if (sourceEncoding == charset::utf8)
		{
			bytes.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = convertEncoding(temp, sourceEncoding, charset::utf8);
			bytes.assign(converted.begin(), converted.end());
		}
		buildFromBytes(bytes);
	}

	explicit Charsequence(const Point *source, std::size_t length)
		: metaStorage(), pointStorage(), storageEncoding(charset::utf8)
	{
		for (std::size_t i = 0; i < length; ++i)
		{
			pointStorage.push_back(source[i]);
			unsigned int codepoint = source[i].getValue();
			encodePointToMeta(codepoint);
		}
	}

	Charsequence(const Charsequence &other)
		: metaStorage(other.metaStorage), pointStorage(other.pointStorage), storageEncoding(other.storageEncoding) {}

	Charsequence(Charsequence &&other) noexcept
		: metaStorage(std::move(other.metaStorage)), pointStorage(std::move(other.pointStorage)), storageEncoding(other.storageEncoding)
	{
		other.storageEncoding = charset::utf8;
	}

	Charsequence &operator=(const Charsequence &other)
	{
		if (this != &other)
		{
			metaStorage = other.metaStorage;
			pointStorage = other.pointStorage;
			storageEncoding = other.storageEncoding;
		}
		return *this;
	}

	Charsequence &operator=(Charsequence &&other) noexcept
	{
		if (this != &other)
		{
			metaStorage = std::move(other.metaStorage);
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

	std::unique_ptr<Charsequence> sub(std::size_t start, std::size_t length) const
	{
		std::size_t totalPoints = pointStorage.size();
		if (start > totalPoints)
			start = totalPoints;
		if (length > totalPoints - start)
			length = totalPoints - start;
		if (length == 0)
		{
			return std::make_unique<Charsequence>(std::string(), storageEncoding);
		}
		auto result = std::make_unique<Charsequence>();
		result->storageEncoding = storageEncoding;
		for (std::size_t i = start; i < start + length && i < totalPoints; ++i)
		{
			result->pointStorage.push_back(pointStorage[i]);
			result->encodePointToMeta(pointStorage[i].getValue());
		}
		return result;
	}

	std::unique_ptr<Charsequence> repeat(std::size_t count) const
	{
		if (count == 0)
		{
			return std::make_unique<Charsequence>(std::string(), storageEncoding);
		}
		auto result = std::make_unique<Charsequence>();
		result->storageEncoding = storageEncoding;
		for (std::size_t i = 0; i < count; ++i)
		{
			for (const auto &p : pointStorage)
			{
				result->pointStorage.push_back(p);
				result->encodePointToMeta(p.getValue());
			}
		}
		return result;
	}

	std::unique_ptr<Charsequence> concat(const Charsequence &other) const
	{
		auto result = std::make_unique<Charsequence>();
		result->storageEncoding = storageEncoding;
		for (const auto &p : pointStorage)
		{
			result->pointStorage.push_back(p);
			result->encodePointToMeta(p.getValue());
		}
		auto otherPoints = other.getPoints();
		for (const auto &p : otherPoints)
		{
			result->pointStorage.push_back(p);
			result->encodePointToMeta(p.getValue());
		}
		return result;
	}

	std::size_t indexOf(const Charsequence &other, std::size_t fromCodePoint = 0) const
	{
		if (fromCodePoint >= pointStorage.size())
		{
			return static_cast<std::size_t>(-1);
		}
		auto otherPoints = other.getPoints();
		if (otherPoints.empty())
		{
			return 0;
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
		auto otherPoints = other.getPoints();
		if (otherPoints.empty())
		{
			return pointStorage.size();
		}
		std::size_t startPos = pointStorage.size();
		if (fromCodePoint != static_cast<std::size_t>(-1))
		{
			if (fromCodePoint >= pointStorage.size())
			{
				return static_cast<std::size_t>(-1);
			}
			startPos = fromCodePoint + otherPoints.size();
			if (startPos > pointStorage.size())
				startPos = pointStorage.size();
		}
		for (std::size_t i = startPos; i >= otherPoints.size(); --i)
		{
			std::size_t idx = i - otherPoints.size();
			bool found = true;
			for (std::size_t j = 0; j < otherPoints.size(); ++j)
			{
				if (pointStorage[idx + j].getValue() != otherPoints[j].getValue())
				{
					found = false;
					break;
				}
			}
			if (found)
			{
				return idx;
			}
		}
		return static_cast<std::size_t>(-1);
	}

	std::unique_ptr<Charsequence> toString(charset targetEncoding) const
	{
		auto bytes = getBytes();
		auto converted = convertEncoding(std::string(bytes.begin(), bytes.end()), storageEncoding, targetEncoding);
		return std::make_unique<Charsequence>(converted, targetEncoding);
	}

	int compare(const Charsequence &other) const
	{
		std::size_t minSize = std::min(pointStorage.size(), other.pointStorage.size());
		for (std::size_t i = 0; i < minSize; ++i)
		{
			if (pointStorage[i].getValue() < other.pointStorage[i].getValue())
				return -1;
			if (pointStorage[i].getValue() > other.pointStorage[i].getValue())
				return 1;
		}
		if (pointStorage.size() < other.pointStorage.size())
			return -1;
		if (pointStorage.size() > other.pointStorage.size())
			return 1;
		return 0;
	}

	std::vector<unsigned char> getBytes(charset targetEncoding = charset::utf8) const
	{
		if (targetEncoding == storageEncoding && storageEncoding == charset::utf8)
		{
			return metaStorage;
		}
		std::string temp(metaStorage.begin(), metaStorage.end());
		auto converted = convertEncoding(temp, storageEncoding, targetEncoding);
		return std::vector<unsigned char>(converted.begin(), converted.end());
	}

	std::vector<char> getCharacters() const
	{
		return std::vector<char>(metaStorage.begin(), metaStorage.end());
	}

	std::vector<Point> getPoints() const
	{
		return pointStorage;
	}

	std::vector<Meta> getMetas() const
	{
		std::vector<Meta> metas;
		metas.reserve(metaStorage.size());
		for (unsigned char byte : metaStorage)
		{
			metas.emplace_back(byte);
		}
		return metas;
	}

	Charsequence operator+(const Charsequence &other) const
	{
		Charsequence result;
		result.storageEncoding = storageEncoding;
		for (const auto &p : pointStorage)
		{
			result.pointStorage.push_back(p);
			result.encodePointToMeta(p.getValue());
		}
		for (const auto &p : other.pointStorage)
		{
			result.pointStorage.push_back(p);
			result.encodePointToMeta(p.getValue());
		}
		return result;
	}

	Charsequence operator+(const std::string &str) const
	{
		Charsequence temp(str, storageEncoding);
		return *this + temp;
	}

	Charsequence operator+(const char *str) const
	{
		Charsequence temp(str, storageEncoding);
		return *this + temp;
	}

	Charsequence &operator+=(const Charsequence &other)
	{
		for (const auto &p : other.pointStorage)
		{
			pointStorage.push_back(p);
			encodePointToMeta(p.getValue());
		}
		return *this;
	}

	Charsequence &operator+=(const std::string &other)
	{
		Charsequence temp(other, storageEncoding);
		return *this += temp;
	}

	Charsequence &operator+=(const char *other)
	{
		Charsequence temp(other, storageEncoding);
		return *this += temp;
	}

	bool operator==(const Charsequence &other) const { return compare(other) == 0; }
	bool operator!=(const Charsequence &other) const { return compare(other) != 0; }
	bool operator<(const Charsequence &other) const { return compare(other) < 0; }
	bool operator<=(const Charsequence &other) const { return compare(other) <= 0; }
	bool operator>(const Charsequence &other) const { return compare(other) > 0; }
	bool operator>=(const Charsequence &other) const { return compare(other) >= 0; }

	Point operator[](std::size_t index) const { return at(index); }

	friend std::ostream &operator<<(std::ostream &os, const Charsequence &str)
	{
		os.write(reinterpret_cast<const char *>(str.metaStorage.data()), str.metaStorage.size());
		return os;
	}

	friend std::istream &operator>>(std::istream &is, Charsequence &str)
	{
		std::string temp;
		is >> temp;
		str = Charsequence(temp, str.storageEncoding);
		return is;
	}

	friend std::stringstream &operator<<(std::stringstream &ss, const Charsequence &str)
	{
		ss.write(reinterpret_cast<const char *>(str.metaStorage.data()), str.metaStorage.size());
		return ss;
	}

	friend std::stringstream &operator>>(std::stringstream &ss, Charsequence &str)
	{
		std::string temp;
		ss >> temp;
		str = Charsequence(temp, str.storageEncoding);
		return ss;
	}

	friend Charsequence operator+(const std::string &lhs, const Charsequence &rhs)
	{
		Charsequence temp(lhs, rhs.storageEncoding);
		return temp + rhs;
	}

	friend Charsequence operator+(const char *lhs, const Charsequence &rhs)
	{
		Charsequence temp(lhs, rhs.storageEncoding);
		return temp + rhs;
	}

	friend Charsequence operator+(std::unique_ptr<Charsequence> lhs, const Charsequence &rhs)
	{
		return *lhs + rhs;
	}

	friend Charsequence operator+(const Charsequence &lhs, std::unique_ptr<Charsequence> rhs)
	{
		return lhs + *rhs;
	}

	friend Charsequence operator+(std::unique_ptr<Charsequence> lhs, std::unique_ptr<Charsequence> rhs)
	{
		return *lhs + *rhs;
	}

	PointIterator begin() const { return PointIterator(this, 0); }
	PointIterator end() const { return PointIterator(this, pointStorage.size()); }

	static std::string convertEncoding(const std::string &input, charset from, charset to)
	{
		if (from == to)
			return input;
		if (from == charset::utf8 && to == charset::ascii)
		{
			std::string result;
			result.reserve(input.length());
			for (char c : input)
			{
				if (static_cast<unsigned char>(c) < 0x80)
					result.push_back(c);
				else
					result.push_back('?');
			}
			return result;
		}
		if (from == charset::utf8 && to == charset::utf16)
		{
			std::string result;
			std::size_t i = 0;
			while (i < input.length())
			{
				unsigned char byte = static_cast<unsigned char>(input[i]);
				std::size_t bytes = bytesInSequence(byte);
				unsigned int codepoint;
				if (bytes == 1)
					codepoint = byte;
				else if (bytes == 2)
				{
					codepoint = (byte & 0x1F) << 6;
					codepoint |= (static_cast<unsigned char>(input[i + 1]) & 0x3F);
				}
				else if (bytes == 3)
				{
					codepoint = (byte & 0x0F) << 12;
					codepoint |= (static_cast<unsigned char>(input[i + 1]) & 0x3F) << 6;
					codepoint |= (static_cast<unsigned char>(input[i + 2]) & 0x3F);
				}
				else
				{
					codepoint = (byte & 0x07) << 18;
					codepoint |= (static_cast<unsigned char>(input[i + 1]) & 0x3F) << 12;
					codepoint |= (static_cast<unsigned char>(input[i + 2]) & 0x3F) << 6;
					codepoint |= (static_cast<unsigned char>(input[i + 3]) & 0x3F);
				}
				if (codepoint <= 0xFFFF)
				{
					result.push_back(static_cast<char>(codepoint & 0xFF));
					result.push_back(static_cast<char>((codepoint >> 8) & 0xFF));
				}
				else
				{
					unsigned int high = 0xD800 + ((codepoint - 0x10000) >> 10);
					unsigned int low = 0xDC00 + ((codepoint - 0x10000) & 0x3FF);
					result.push_back(static_cast<char>(high & 0xFF));
					result.push_back(static_cast<char>((high >> 8) & 0xFF));
					result.push_back(static_cast<char>(low & 0xFF));
					result.push_back(static_cast<char>((low >> 8) & 0xFF));
				}
				i += bytes;
			}
			return result;
		}
		if (from == charset::utf8 && to == charset::utf32)
		{
			std::string result;
			std::size_t i = 0;
			while (i < input.length())
			{
				unsigned char byte = static_cast<unsigned char>(input[i]);
				std::size_t bytes = bytesInSequence(byte);
				unsigned int codepoint;
				if (bytes == 1)
					codepoint = byte;
				else if (bytes == 2)
				{
					codepoint = (byte & 0x1F) << 6;
					codepoint |= (static_cast<unsigned char>(input[i + 1]) & 0x3F);
				}
				else if (bytes == 3)
				{
					codepoint = (byte & 0x0F) << 12;
					codepoint |= (static_cast<unsigned char>(input[i + 1]) & 0x3F) << 6;
					codepoint |= (static_cast<unsigned char>(input[i + 2]) & 0x3F);
				}
				else
				{
					codepoint = (byte & 0x07) << 18;
					codepoint |= (static_cast<unsigned char>(input[i + 1]) & 0x3F) << 12;
					codepoint |= (static_cast<unsigned char>(input[i + 2]) & 0x3F) << 6;
					codepoint |= (static_cast<unsigned char>(input[i + 3]) & 0x3F);
				}
				result.push_back(static_cast<char>(codepoint & 0xFF));
				result.push_back(static_cast<char>((codepoint >> 8) & 0xFF));
				result.push_back(static_cast<char>((codepoint >> 16) & 0xFF));
				result.push_back(static_cast<char>((codepoint >> 24) & 0xFF));
				i += bytes;
			}
			return result;
		}
		return input;
	}

  private:
	std::vector<unsigned char> metaStorage;
	std::vector<Point> pointStorage;
	charset storageEncoding;

	void buildFromBytes(const std::vector<unsigned char> &bytes)
	{
		metaStorage = bytes;
		std::size_t index = 0;
		while (index < bytes.size())
		{
			pointStorage.push_back(decodePointFromBytes(bytes, index));
		}
		validateEncoding();
	}

	void encodePointToMeta(unsigned int codepoint)
	{
		if (codepoint <= 0x7F)
		{
			metaStorage.push_back(static_cast<unsigned char>(codepoint));
		}
		else if (codepoint <= 0x7FF)
		{
			metaStorage.push_back(static_cast<unsigned char>(0xC0 | (codepoint >> 6)));
			metaStorage.push_back(static_cast<unsigned char>(0x80 | (codepoint & 0x3F)));
		}
		else if (codepoint <= 0xFFFF)
		{
			if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
				throw std::invalid_argument("Surrogate code points not valid for UTF-8");
			metaStorage.push_back(static_cast<unsigned char>(0xE0 | (codepoint >> 12)));
			metaStorage.push_back(static_cast<unsigned char>(0x80 | ((codepoint >> 6) & 0x3F)));
			metaStorage.push_back(static_cast<unsigned char>(0x80 | (codepoint & 0x3F)));
		}
		else if (codepoint <= 0x10FFFF)
		{
			metaStorage.push_back(static_cast<unsigned char>(0xF0 | (codepoint >> 18)));
			metaStorage.push_back(static_cast<unsigned char>(0x80 | ((codepoint >> 12) & 0x3F)));
			metaStorage.push_back(static_cast<unsigned char>(0x80 | ((codepoint >> 6) & 0x3F)));
			metaStorage.push_back(static_cast<unsigned char>(0x80 | (codepoint & 0x3F)));
		}
		else
		{
			throw std::invalid_argument("Code point exceeds 0x10FFFF");
		}
	}

	Point decodePointFromBytes(const std::vector<unsigned char> &data, std::size_t &index) const
	{
		if (index >= data.size())
			return Point(0xFFFD);
		unsigned char byte = data[index];
		if (byte >= 0x80 && byte < 0xC2)
		{
			index++;
			return Point(0xFFFD);
		}
		unsigned int codepoint;
		std::size_t bytes = bytesInSequence(byte);
		if (index + bytes > data.size())
		{
			index = data.size();
			return Point(0xFFFD);
		}
		if (bytes == 1)
			codepoint = byte;
		else if (bytes == 2)
		{
			codepoint = (byte & 0x1F) << 6;
			codepoint |= (data[index + 1] & 0x3F);
		}
		else if (bytes == 3)
		{
			codepoint = (byte & 0x0F) << 12;
			codepoint |= (data[index + 1] & 0x3F) << 6;
			codepoint |= (data[index + 2] & 0x3F);
		}
		else if (bytes == 4)
		{
			codepoint = (byte & 0x07) << 18;
			codepoint |= (data[index + 1] & 0x3F) << 12;
			codepoint |= (data[index + 2] & 0x3F) << 6;
			codepoint |= (data[index + 3] & 0x3F);
		}
		else
		{
			codepoint = 0xFFFD;
			bytes = 1;
		}
		index += bytes;
		return Point(codepoint);
	}

	static std::size_t bytesInSequence(unsigned char byte)
	{
		if (byte < 0x80)
			return 1;
		if (byte < 0xC2)
			return 1;
		if (byte < 0xE0)
			return 2;
		if (byte < 0xF0)
			return 3;
		if (byte < 0xF8)
			return 4;
		return 1;
	}

	void validateEncoding()
	{
		if (storageEncoding == charset::utf8)
		{
			std::size_t i = 0;
			while (i < metaStorage.size())
			{
				unsigned char byte = metaStorage[i];
				if ((byte >= 0x80 && byte < 0xC2) || byte >= 0xF8)
				{
					throw std::invalid_argument("Invalid UTF-8 sequence");
				}
				std::size_t bytes = bytesInSequence(byte);
				if (bytes == 0 || i + bytes > metaStorage.size())
				{
					throw std::invalid_argument("Invalid UTF-8 sequence");
				}
				for (std::size_t j = 1; j < bytes; ++j)
				{
					if ((metaStorage[i + j] & 0xC0) != 0x80)
					{
						throw std::invalid_argument("Invalid UTF-8 continuation byte");
					}
				}
				unsigned int codepoint = 0;
				if (bytes == 1)
					codepoint = byte;
				else if (bytes == 2)
				{
					codepoint = (byte & 0x1F) << 6;
					codepoint |= (metaStorage[i + 1] & 0x3F);
					if (codepoint < 0x80)
						throw std::invalid_argument("Overlong UTF-8 sequence");
				}
				else if (bytes == 3)
				{
					codepoint = (byte & 0x0F) << 12;
					codepoint |= (metaStorage[i + 1] & 0x3F) << 6;
					codepoint |= (metaStorage[i + 2] & 0x3F);
					if (codepoint < 0x800)
						throw std::invalid_argument("Overlong UTF-8 sequence");
					if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
						throw std::invalid_argument("Invalid UTF-8 surrogate");
				}
				else if (bytes == 4)
				{
					codepoint = (byte & 0x07) << 18;
					codepoint |= (metaStorage[i + 1] & 0x3F) << 12;
					codepoint |= (metaStorage[i + 2] & 0x3F) << 6;
					codepoint |= (metaStorage[i + 3] & 0x3F);
					if (codepoint < 0x10000)
						throw std::invalid_argument("Overlong UTF-8 sequence");
					if (codepoint > 0x10FFFF)
						throw std::invalid_argument("Code point exceeds 0x10FFFF");
				}
				i += bytes;
			}
		}
	}
};

inline Point PointIterator::operator*() const { return charsequence->at(position); }
inline std::unique_ptr<Point> PointIterator::operator->() const { return std::make_unique<Point>(charsequence->at(position)); }
inline PointIterator &PointIterator::operator++()
{
	++position;
	return *this;
}
inline PointIterator PointIterator::operator++(int)
{
	PointIterator temp = *this;
	++position;
	return temp;
}
inline PointIterator &PointIterator::operator--()
{
	--position;
	return *this;
}
inline PointIterator PointIterator::operator--(int)
{
	PointIterator temp = *this;
	--position;
	return temp;
}
inline bool PointIterator::operator==(const PointIterator &other) const { return charsequence == other.charsequence && position == other.position; }
inline bool PointIterator::operator!=(const PointIterator &other) const { return !(*this == other); }

class Builder
{
  public:
	Builder() : storage(), storageEncoding(charset::utf8) {}
	explicit Builder(charset enc) : storage(), storageEncoding(charset::utf8) {}

	Builder(const Builder &other) : storage(other.storage), storageEncoding(other.storageEncoding) {}
	Builder(Builder &&other) noexcept : storage(std::move(other.storage)), storageEncoding(other.storageEncoding)
	{
		other.storageEncoding = charset::utf8;
	}

	explicit Builder(const std::string &source, charset sourceEncoding = charset::utf8)
		: storageEncoding(charset::utf8)
	{
		std::string temp(source);
		if (sourceEncoding == charset::utf8)
		{
			storage.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = Charsequence::convertEncoding(temp, sourceEncoding, charset::utf8);
			storage.assign(converted.begin(), converted.end());
		}
	}

	explicit Builder(std::string &&source, charset sourceEncoding = charset::utf8)
		: storageEncoding(charset::utf8)
	{
		std::string temp(std::move(source));
		if (sourceEncoding == charset::utf8)
		{
			storage.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = Charsequence::convertEncoding(temp, sourceEncoding, charset::utf8);
			storage.assign(converted.begin(), converted.end());
		}
	}

	explicit Builder(const char *source, charset sourceEncoding = charset::utf8)
		: storageEncoding(charset::utf8)
	{
		std::string temp(source ? source : "");
		if (sourceEncoding == charset::utf8)
		{
			storage.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = Charsequence::convertEncoding(temp, sourceEncoding, charset::utf8);
			storage.assign(converted.begin(), converted.end());
		}
	}

	Builder &insert(std::size_t pos, const Charsequence &source)
	{
		auto bytes = source.getBytes(storageEncoding);
		if (pos >= storage.size())
		{
			storage.insert(storage.end(), bytes.begin(), bytes.end());
		}
		else
		{
			storage.insert(storage.begin() + pos, bytes.begin(), bytes.end());
		}
		return *this;
	}

	Builder &insert(std::size_t pos, const std::string &source)
	{
		Charsequence temp(source, storageEncoding);
		return insert(pos, temp);
	}

	Builder &insert(std::size_t pos, const char *source)
	{
		Charsequence temp(source, storageEncoding);
		return insert(pos, temp);
	}

	Builder &insert(std::size_t pos, Point point)
	{
		Charsequence temp(&point, 1);
		return insert(pos, temp);
	}

	Builder &insert(std::size_t pos, unsigned char byte)
	{
		if (pos >= storage.size())
		{
			storage.push_back(byte);
		}
		else
		{
			storage.insert(storage.begin() + pos, byte);
		}
		return *this;
	}

	Builder &insert(std::size_t pos, char byte)
	{
		return insert(pos, static_cast<unsigned char>(byte));
	}

	Builder &insert(std::size_t pos, bool value)
	{
		std::string str = value ? "true" : "false";
		return insert(pos, str);
	}

	Builder &insert(std::size_t pos, short value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	Builder &insert(std::size_t pos, int value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	Builder &insert(std::size_t pos, long value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	Builder &insert(std::size_t pos, long long value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	Builder &insert(std::size_t pos, float value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	Builder &insert(std::size_t pos, double value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	Builder &insert(std::size_t pos, long double value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	Builder &insert(std::size_t pos, unsigned short value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	Builder &insert(std::size_t pos, unsigned int value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	Builder &insert(std::size_t pos, unsigned long value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	Builder &insert(std::size_t pos, unsigned long long value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	Builder &append(const Charsequence &source)
	{
		auto bytes = source.getBytes(storageEncoding);
		storage.insert(storage.end(), bytes.begin(), bytes.end());
		return *this;
	}

	Builder &append(const std::string &source)
	{
		Charsequence temp(source, storageEncoding);
		return append(temp);
	}

	Builder &append(const char *source)
	{
		Charsequence temp(source, storageEncoding);
		return append(temp);
	}

	Builder &append(Point point)
	{
		Charsequence temp(&point, 1);
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
		return append(str);
	}

	Builder &append(short value)
	{
		std::string str = std::to_string(value);
		return append(str);
	}

	Builder &append(int value)
	{
		std::string str = std::to_string(value);
		return append(str);
	}

	Builder &append(long value)
	{
		std::string str = std::to_string(value);
		return append(str);
	}

	Builder &append(long long value)
	{
		std::string str = std::to_string(value);
		return append(str);
	}

	Builder &append(float value)
	{
		std::string str = std::to_string(value);
		return append(str);
	}

	Builder &append(double value)
	{
		std::string str = std::to_string(value);
		return append(str);
	}

	Builder &append(long double value)
	{
		std::string str = std::to_string(value);
		return append(str);
	}

	Builder &append(unsigned short value)
	{
		std::string str = std::to_string(value);
		return append(str);
	}

	Builder &append(unsigned int value)
	{
		std::string str = std::to_string(value);
		return append(str);
	}

	Builder &append(unsigned long value)
	{
		std::string str = std::to_string(value);
		return append(str);
	}

	Builder &append(unsigned long long value)
	{
		std::string str = std::to_string(value);
		return append(str);
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

	Builder operator+(const std::string &str) const
	{
		Builder result(*this);
		result.append(str);
		return result;
	}

	Builder operator+(const char *str) const
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

	Builder &operator+=(const std::string &str)
	{
		append(str);
		return *this;
	}

	Builder &operator+=(const char *str)
	{
		append(str);
		return *this;
	}

	Charsequence toCharsequence() const
	{
		return Charsequence(std::string(storage.begin(), storage.end()), charset::utf8);
	}

	std::size_t size() const { return toCharsequence().size(); }
	bool empty() const { return storage.empty(); }
	charset encoding() const { return storageEncoding; }

	std::vector<unsigned char> getBytes() const { return storage; }

	friend std::ostream &operator<<(std::ostream &os, const Builder &builder)
	{
		os.write(reinterpret_cast<const char *>(builder.storage.data()), builder.storage.size());
		return os;
	}

	friend std::istream &operator>>(std::istream &is, Builder &builder)
	{
		std::string temp;
		is >> temp;
		builder = Builder(temp, builder.storageEncoding);
		return is;
	}

	friend std::stringstream &operator<<(std::stringstream &ss, const Builder &builder)
	{
		ss.write(reinterpret_cast<const char *>(builder.storage.data()), builder.storage.size());
		return ss;
	}

	friend std::stringstream &operator>>(std::stringstream &ss, Builder &builder)
	{
		std::string temp;
		ss >> temp;
		builder = Builder(temp, builder.storageEncoding);
		return ss;
	}

	friend Builder operator+(const std::string &lhs, const Builder &rhs)
	{
		Builder result(lhs, rhs.storageEncoding);
		result.append(rhs.toCharsequence());
		return result;
	}

	friend Builder operator+(const char *lhs, const Builder &rhs)
	{
		Builder result(lhs, rhs.storageEncoding);
		result.append(rhs.toCharsequence());
		return result;
	}

	friend Builder operator+(std::unique_ptr<Builder> lhs, const Builder &rhs)
	{
		return *lhs + rhs;
	}

	friend Builder operator+(const Builder &lhs, std::unique_ptr<Builder> rhs)
	{
		return lhs + *rhs;
	}

	friend Builder operator+(std::unique_ptr<Builder> lhs, std::unique_ptr<Builder> rhs)
	{
		return *lhs + *rhs;
	}

  private:
	std::vector<unsigned char> storage;
	charset storageEncoding;
};

class Buffer
{
  public:
	Buffer() : storage() {}
	explicit Buffer(std::size_t capacity) : storage() { storage.reserve(capacity); }

	std::size_t write(const char *input, std::size_t length)
	{
		if (!input || length == 0)
			return 0;
		storage.insert(storage.end(), input, input + length);
		return length;
	}

	std::size_t write(const std::string &input)
	{
		storage.insert(storage.end(), input.begin(), input.end());
		return input.size();
	}

	std::size_t write(const std::vector<unsigned char> &input)
	{
		storage.insert(storage.end(), input.begin(), input.end());
		return input.size();
	}

	std::size_t write(const Charsequence &input)
	{
		auto bytes = input.getBytes();
		storage.insert(storage.end(), bytes.begin(), bytes.end());
		return bytes.size();
	}

	std::size_t write(const Builder &input)
	{
		auto bytes = input.getBytes();
		storage.insert(storage.end(), bytes.begin(), bytes.end());
		return bytes.size();
	}

	std::size_t write(unsigned char byte)
	{
		storage.push_back(byte);
		return 1;
	}

	std::size_t write(char byte)
	{
		storage.push_back(static_cast<unsigned char>(byte));
		return 1;
	}

	std::size_t write(bool value)
	{
		std::string str = value ? "true" : "false";
		return write(str);
	}

	std::size_t write(short value)
	{
		std::string str = std::to_string(value);
		return write(str);
	}

	std::size_t write(int value)
	{
		std::string str = std::to_string(value);
		return write(str);
	}

	std::size_t write(long value)
	{
		std::string str = std::to_string(value);
		return write(str);
	}

	std::size_t write(long long value)
	{
		std::string str = std::to_string(value);
		return write(str);
	}

	std::size_t write(float value)
	{
		std::string str = std::to_string(value);
		return write(str);
	}

	std::size_t write(double value)
	{
		std::string str = std::to_string(value);
		return write(str);
	}

	std::size_t write(long double value)
	{
		std::string str = std::to_string(value);
		return write(str);
	}

	std::size_t write(unsigned short value)
	{
		std::string str = std::to_string(value);
		return write(str);
	}

	std::size_t write(unsigned int value)
	{
		std::string str = std::to_string(value);
		return write(str);
	}

	std::size_t write(unsigned long value)
	{
		std::string str = std::to_string(value);
		return write(str);
	}

	std::size_t write(unsigned long long value)
	{
		std::string str = std::to_string(value);
		return write(str);
	}

	std::size_t insert(std::size_t pos, const char *input, std::size_t length)
	{
		if (!input || length == 0)
			return 0;
		if (pos >= storage.size())
		{
			storage.insert(storage.end(), input, input + length);
		}
		else
		{
			storage.insert(storage.begin() + pos, input, input + length);
		}
		return length;
	}

	std::size_t insert(std::size_t pos, const std::string &input)
	{
		if (pos >= storage.size())
		{
			storage.insert(storage.end(), input.begin(), input.end());
		}
		else
		{
			storage.insert(storage.begin() + pos, input.begin(), input.end());
		}
		return input.size();
	}

	std::size_t insert(std::size_t pos, const std::vector<unsigned char> &input)
	{
		if (pos >= storage.size())
		{
			storage.insert(storage.end(), input.begin(), input.end());
		}
		else
		{
			storage.insert(storage.begin() + pos, input.begin(), input.end());
		}
		return input.size();
	}

	std::size_t insert(std::size_t pos, const Charsequence &input)
	{
		auto bytes = input.getBytes();
		return insert(pos, bytes);
	}

	std::size_t insert(std::size_t pos, const Builder &input)
	{
		auto bytes = input.getBytes();
		return insert(pos, bytes);
	}

	std::size_t insert(std::size_t pos, unsigned char byte)
	{
		if (pos >= storage.size())
		{
			storage.push_back(byte);
		}
		else
		{
			storage.insert(storage.begin() + pos, byte);
		}
		return 1;
	}

	std::size_t insert(std::size_t pos, char byte)
	{
		return insert(pos, static_cast<unsigned char>(byte));
	}

	std::size_t insert(std::size_t pos, bool value)
	{
		std::string str = value ? "true" : "false";
		return insert(pos, str);
	}

	std::size_t insert(std::size_t pos, short value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	std::size_t insert(std::size_t pos, int value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	std::size_t insert(std::size_t pos, long value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	std::size_t insert(std::size_t pos, long long value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	std::size_t insert(std::size_t pos, float value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	std::size_t insert(std::size_t pos, double value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	std::size_t insert(std::size_t pos, long double value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	std::size_t insert(std::size_t pos, unsigned short value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	std::size_t insert(std::size_t pos, unsigned int value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	std::size_t insert(std::size_t pos, unsigned long value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	std::size_t insert(std::size_t pos, unsigned long long value)
	{
		std::string str = std::to_string(value);
		return insert(pos, str);
	}

	std::size_t append(const char *input, std::size_t length)
	{
		return write(input, length);
	}

	std::size_t append(const std::string &input)
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

	std::size_t read(char *output, std::size_t maxLength) const
	{
		std::size_t len = std::min(maxLength, storage.size());
		if (len > 0)
			std::memcpy(output, storage.data(), len);
		return len;
	}

	const unsigned char *data() const { return storage.data(); }
	std::size_t size() const { return storage.size(); }
	void clear() { storage.clear(); }

	Buffer operator+(const Buffer &other) const
	{
		Buffer result;
		result.storage = this->storage;
		result.storage.insert(result.storage.end(), other.storage.begin(), other.storage.end());
		return result;
	}

	Buffer operator+(const std::string &str) const
	{
		Buffer result;
		result.storage = this->storage;
		result.storage.insert(result.storage.end(), str.begin(), str.end());
		return result;
	}

	Buffer operator+(const char *str) const
	{
		Buffer result;
		result.storage = this->storage;
		std::size_t len = std::strlen(str);
		result.storage.insert(result.storage.end(), str, str + len);
		return result;
	}

	Buffer &operator+=(const Buffer &other)
	{
		storage.insert(storage.end(), other.storage.begin(), other.storage.end());
		return *this;
	}

	Buffer &operator+=(const std::string &str)
	{
		storage.insert(storage.end(), str.begin(), str.end());
		return *this;
	}

	Buffer &operator+=(const char *str)
	{
		std::size_t len = std::strlen(str);
		storage.insert(storage.end(), str, str + len);
		return *this;
	}

	Buffer &operator<<(const std::string &input)
	{
		write(input);
		return *this;
	}

	Buffer &operator<<(const char *input)
	{
		if (input)
		{
			std::size_t len = std::strlen(input);
			write(input, len);
		}
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
		output.assign(storage.begin(), storage.end());
		clear();
		return *this;
	}

	friend std::ostream &operator<<(std::ostream &os, const Buffer &buffer)
	{
		os.write(reinterpret_cast<const char *>(buffer.storage.data()), buffer.storage.size());
		return os;
	}

	friend std::istream &operator>>(std::istream &is, Buffer &buffer)
	{
		std::string temp;
		is >> temp;
		buffer.write(temp);
		return is;
	}

	friend std::stringstream &operator<<(std::stringstream &ss, const Buffer &buffer)
	{
		ss.write(reinterpret_cast<const char *>(buffer.storage.data()), buffer.storage.size());
		return ss;
	}

	friend std::stringstream &operator>>(std::stringstream &ss, Buffer &buffer)
	{
		std::string temp;
		ss >> temp;
		buffer.write(temp);
		return ss;
	}

	friend Buffer operator+(const std::string &lhs, const Buffer &rhs)
	{
		Buffer result;
		result.storage.insert(result.storage.end(), lhs.begin(), lhs.end());
		result.storage.insert(result.storage.end(), rhs.storage.begin(), rhs.storage.end());
		return result;
	}

	friend Buffer operator+(const char *lhs, const Buffer &rhs)
	{
		Buffer result;
		std::size_t len = std::strlen(lhs);
		result.storage.insert(result.storage.end(), lhs, lhs + len);
		result.storage.insert(result.storage.end(), rhs.storage.begin(), rhs.storage.end());
		return result;
	}

	friend Buffer operator+(std::unique_ptr<Buffer> lhs, const Buffer &rhs)
	{
		return *lhs + rhs;
	}

	friend Buffer operator+(const Buffer &lhs, std::unique_ptr<Buffer> rhs)
	{
		return lhs + *rhs;
	}

	friend Buffer operator+(std::unique_ptr<Buffer> lhs, std::unique_ptr<Buffer> rhs)
	{
		return *lhs + *rhs;
	}

  private:
	std::vector<unsigned char> storage;
};

class Regex
{
  public:
	enum class option
	{
		none = 0,
		caseInsensitive = 1 << 0,
		multiline = 1 << 1,
		dotAll = 1 << 2,
		unicode = 1 << 3
	};

	explicit Regex(const Charsequence &pattern)
		: regexPattern(pattern.getBytes()), regexOptions(option::none)
	{
		compileRegex();
	}

	bool matches(const Charsequence &input) const
	{
		auto inputBytes = input.getBytes();
		std::string inputStr(inputBytes.begin(), inputBytes.end());
		return std::regex_match(inputStr, compiledRegex);
	}

	std::string getRegexPattern() const
	{
		return std::string(regexPattern.begin(), regexPattern.end());
	}

	option getRegexOptions() const { return regexOptions; }

  private:
	std::vector<unsigned char> regexPattern;
	option regexOptions;
	std::regex compiledRegex;

	void compileRegex()
	{
		try
		{
			std::regex::flag_type flags = std::regex::ECMAScript;
			compiledRegex = std::regex(getRegexPattern(), flags);
		}
		catch (const std::regex_error &)
		{
			compiledRegex = std::regex("");
		}
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
	size_t operator()(const charsequence::Charsequence &cs) const noexcept
	{
		auto bytes = cs.getBytes(charsequence::charset::utf8);
		size_t seed = bytes.size();
		for (auto byte : bytes)
		{
			seed ^= hash<unsigned char>()(byte) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
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
		size_t seed = buffer.size();
		for (size_t i = 0; i < buffer.size(); ++i)
		{
			seed ^= hash<unsigned char>()(buffer.data()[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

template <>
struct hash<charsequence::Regex>
{
	size_t operator()(const charsequence::Regex &regex) const noexcept
	{
		size_t h1 = hash<string>()(regex.getRegexPattern());
		size_t h2 = hash<int>()(static_cast<int>(regex.getRegexOptions()));
		return h1 ^ (h2 << 1);
	}
};

template <>
struct less<charsequence::Meta>
{
	bool operator()(const charsequence::Meta &lhs, const charsequence::Meta &rhs) const noexcept
	{
		return lhs.getValue() < rhs.getValue();
	}
};

template <>
struct less<charsequence::Point>
{
	bool operator()(const charsequence::Point &lhs, const charsequence::Point &rhs) const noexcept
	{
		return lhs.getValue() < rhs.getValue();
	}
};

template <>
struct less<charsequence::Charsequence>
{
	bool operator()(const charsequence::Charsequence &lhs, const charsequence::Charsequence &rhs) const noexcept
	{
		return lhs.compare(rhs) < 0;
	}
};

template <>
struct less<charsequence::Builder>
{
	bool operator()(const charsequence::Builder &lhs, const charsequence::Builder &rhs) const noexcept
	{
		return lhs.toCharsequence().compare(rhs.toCharsequence()) < 0;
	}
};

template <>
struct less<charsequence::Buffer>
{
	bool operator()(const charsequence::Buffer &lhs, const charsequence::Buffer &rhs) const noexcept
	{
		if (lhs.size() != rhs.size())
		{
			return lhs.size() < rhs.size();
		}
		for (size_t i = 0; i < lhs.size(); ++i)
		{
			if (lhs.data()[i] != rhs.data()[i])
			{
				return lhs.data()[i] < rhs.data()[i];
			}
		}
		return false;
	}
};

template <>
struct less<charsequence::Regex>
{
	bool operator()(const charsequence::Regex &lhs, const charsequence::Regex &rhs) const noexcept
	{
		int patternCmp = lhs.getRegexPattern().compare(rhs.getRegexPattern());
		if (patternCmp != 0)
		{
			return patternCmp < 0;
		}
		return static_cast<int>(lhs.getRegexOptions()) < static_cast<int>(rhs.getRegexOptions());
	}
};

} // namespace std
