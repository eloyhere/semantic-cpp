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
	Charsequence() : storage(), storageEncoding(charset::utf8) {}

	explicit Charsequence(charset encode) : storage(), storageEncoding(charset::utf8) {}

	template <typename T,
			  typename = std::enable_if_t<
				  std::is_constructible_v<std::string, T> &&
				  !std::is_base_of_v<std::istream, std::decay_t<T>> &&
				  !std::is_same_v<std::decay_t<T>, Charsequence> &&
				  !std::is_same_v<std::decay_t<T>, Builder> &&
				  !std::is_same_v<std::decay_t<T>, Buffer>>>
	explicit Charsequence(T &&source, charset sourceEncoding = charset::utf8)
		: storageEncoding(charset::utf8)
	{
		std::string temp(std::forward<T>(source));
		if (sourceEncoding == charset::utf8)
		{
			storage.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = convertEncoding(temp, sourceEncoding, charset::utf8);
			storage.assign(converted.begin(), converted.end());
		}
		validateEncoding();
	}

	template <typename T,
			  typename = std::enable_if_t<std::is_base_of_v<std::istream, std::decay_t<T>>>>
	explicit Charsequence(T &&stream, std::size_t maxLength, charset sourceEncoding = charset::utf8)
		: storageEncoding(charset::utf8)
	{
		std::string temp;
		temp.resize(maxLength);
		stream.read(&temp[0], maxLength);
		std::size_t bytesRead = stream.gcount();
		temp.resize(bytesRead);
		if (sourceEncoding == charset::utf8)
		{
			storage.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = convertEncoding(temp, sourceEncoding, charset::utf8);
			storage.assign(converted.begin(), converted.end());
		}
		validateEncoding();
	}

	explicit Charsequence(std::istream &stream, std::size_t maxLength, charset sourceEncoding = charset::utf8)
		: storageEncoding(charset::utf8)
	{
		std::string temp;
		temp.resize(maxLength);
		stream.read(&temp[0], maxLength);
		std::size_t bytesRead = stream.gcount();
		temp.resize(bytesRead);
		if (sourceEncoding == charset::utf8)
		{
			storage.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = convertEncoding(temp, sourceEncoding, charset::utf8);
			storage.assign(converted.begin(), converted.end());
		}
		validateEncoding();
	}

	Charsequence(const char *source, charset sourceEncoding = charset::utf8)
		: storageEncoding(charset::utf8)
	{
		std::string temp(source ? source : "");
		if (sourceEncoding == charset::utf8)
		{
			storage.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = convertEncoding(temp, sourceEncoding, charset::utf8);
			storage.assign(converted.begin(), converted.end());
		}
		validateEncoding();
	}

	explicit Charsequence(const Meta *source, std::size_t length, charset sourceEncoding)
		: storageEncoding(charset::utf8)
	{
		std::string temp;
		temp.reserve(length);
		for (std::size_t i = 0; i < length; ++i)
		{
			temp.push_back(static_cast<char>(source[i].getValue() & 0xFF));
		}
		if (sourceEncoding == charset::utf8)
		{
			storage.assign(temp.begin(), temp.end());
		}
		else
		{
			auto converted = convertEncoding(temp, sourceEncoding, charset::utf8);
			storage.assign(converted.begin(), converted.end());
		}
		validateEncoding();
	}

	explicit Charsequence(const Point *source, std::size_t length)
		: storageEncoding(charset::utf8)
	{
		for (std::size_t i = 0; i < length; ++i)
		{
			appendPoint(source[i]);
		}
	}

	Charsequence(const Charsequence &other)
		: storage(other.storage), storageEncoding(other.storageEncoding) {}

	Charsequence(Charsequence &&other) noexcept
		: storage(std::move(other.storage)), storageEncoding(other.storageEncoding)
	{
		other.storageEncoding = charset::utf8;
	}

	Charsequence &operator=(const Charsequence &other)
	{
		if (this != &other)
		{
			storage = other.storage;
			storageEncoding = other.storageEncoding;
		}
		return *this;
	}

	Charsequence &operator=(Charsequence &&other) noexcept
	{
		if (this != &other)
		{
			storage = std::move(other.storage);
			storageEncoding = other.storageEncoding;
			other.storageEncoding = charset::utf8;
		}
		return *this;
	}

	std::size_t size() const { return countPoints(); }
	bool empty() const { return storage.empty(); }

	Point at(std::size_t index) const
	{
		if (index >= size())
		{
			throw std::out_of_range("Index out of range");
		}
		std::size_t pointCount = 0;
		std::size_t byteIndex = 0;
		while (byteIndex < storage.size() && pointCount < index)
		{
			byteIndex += bytesInSequence(storage[byteIndex]);
			pointCount++;
		}
		std::size_t tempIndex = byteIndex;
		return decodePoint(storage, tempIndex);
	}

	charset encoding() const { return storageEncoding; }

	std::unique_ptr<Charsequence> sub(std::size_t start, std::size_t length) const
	{
		std::size_t totalPoints = size();
		if (start > totalPoints)
			start = totalPoints;
		if (length > totalPoints - start)
			length = totalPoints - start;
		if (length == 0)
		{
			return std::make_unique<Charsequence>(std::string(), storageEncoding);
		}
		std::size_t pointCount = 0;
		std::size_t byteIndex = 0;
		while (byteIndex < storage.size() && pointCount < start)
		{
			byteIndex += bytesInSequence(storage[byteIndex]);
			pointCount++;
		}
		std::size_t startByte = byteIndex;
		for (std::size_t i = 0; i < length && byteIndex < storage.size(); ++i)
		{
			byteIndex += bytesInSequence(storage[byteIndex]);
		}
		std::vector<unsigned char> result(storage.begin() + startByte, storage.begin() + byteIndex);
		return std::make_unique<Charsequence>(vectorToString(result), storageEncoding);
	}

	std::unique_ptr<Charsequence> repeat(std::size_t count) const
	{
		if (count == 0)
		{
			return std::make_unique<Charsequence>(std::string(), storageEncoding);
		}
		std::vector<unsigned char> result;
		result.reserve(storage.size() * count);
		for (std::size_t i = 0; i < count; ++i)
		{
			result.insert(result.end(), storage.begin(), storage.end());
		}
		return std::make_unique<Charsequence>(vectorToString(result), storageEncoding);
	}

	std::unique_ptr<Charsequence> concat(const Charsequence &other) const
	{
		std::vector<unsigned char> result = storage;
		auto otherStr = other.getBytes(storageEncoding);
		result.insert(result.end(), otherStr.begin(), otherStr.end());
		return std::make_unique<Charsequence>(vectorToString(result), storageEncoding);
	}

	std::size_t indexOf(const Charsequence &other, std::size_t fromCodePoint = 0) const
	{
		if (fromCodePoint >= size())
		{
			return static_cast<std::size_t>(-1);
		}
		std::size_t byteFrom = codePointToByte(fromCodePoint);
		auto searchStr = other.getBytes(storageEncoding);
		std::string storageStr(storage.begin(), storage.end());
		std::string searchStrStr(searchStr.begin(), searchStr.end());
		std::size_t bytePos = storageStr.find(searchStrStr, byteFrom);
		if (bytePos == std::string::npos)
		{
			return static_cast<std::size_t>(-1);
		}
		return byteToCodePoint(bytePos);
	}

	std::size_t lastIndexOf(const Charsequence &other, std::size_t fromCodePoint = static_cast<std::size_t>(-1)) const
	{
		auto searchStr = other.getBytes(storageEncoding);
		std::string storageStr(storage.begin(), storage.end());
		std::string searchStrStr(searchStr.begin(), searchStr.end());
		std::size_t byteFrom = std::string::npos;
		if (fromCodePoint != static_cast<std::size_t>(-1))
		{
			if (fromCodePoint >= size())
			{
				return static_cast<std::size_t>(-1);
			}
			byteFrom = codePointToByte(fromCodePoint);
		}
		std::size_t bytePos = storageStr.rfind(searchStrStr, byteFrom);
		if (bytePos == std::string::npos)
		{
			return static_cast<std::size_t>(-1);
		}
		return byteToCodePoint(bytePos);
	}

	std::unique_ptr<Charsequence> toString(charset targetEncoding) const
	{
		auto converted = convertEncoding(vectorToString(storage), storageEncoding, targetEncoding);
		return std::make_unique<Charsequence>(converted, targetEncoding);
	}

	int compare(const Charsequence &other) const
	{
		auto otherData = other.getBytes(storageEncoding);
		std::string thisStr(storage.begin(), storage.end());
		std::string otherStr(otherData.begin(), otherData.end());
		return thisStr.compare(otherStr);
	}

	std::vector<unsigned char> getBytes(charset targetEncoding = charset::utf8) const
	{
		if (targetEncoding == storageEncoding)
		{
			return storage;
		}
		auto converted = convertEncoding(vectorToString(storage), storageEncoding, targetEncoding);
		return std::vector<unsigned char>(converted.begin(), converted.end());
	}

	std::vector<char> getCharacters() const
	{
		return std::vector<char>(storage.begin(), storage.end());
	}

	std::vector<Point> getPoints() const
	{
		std::vector<Point> points;
		std::size_t byteIndex = 0;
		while (byteIndex < storage.size())
		{
			points.push_back(decodePoint(storage, byteIndex));
		}
		return points;
	}

	std::vector<Meta> getMetas() const
	{
		std::vector<Meta> metas;
		metas.reserve(storage.size());
		for (unsigned char byte : storage)
		{
			metas.emplace_back(byte);
		}
		return metas;
	}

	Charsequence operator+(const Charsequence &other) const
	{
		std::vector<unsigned char> result = storage;
		auto otherBytes = other.getBytes(storageEncoding);
		result.insert(result.end(), otherBytes.begin(), otherBytes.end());
		return Charsequence(vectorToString(result), storageEncoding);
	}

	Charsequence &operator+=(const Charsequence &other)
	{
		auto otherBytes = other.getBytes(storageEncoding);
		storage.insert(storage.end(), otherBytes.begin(), otherBytes.end());
		return *this;
	}

	Charsequence &operator+=(const std::string &other)
	{
		storage.insert(storage.end(), other.begin(), other.end());
		return *this;
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
		os.write(reinterpret_cast<const char *>(str.storage.data()), str.storage.size());
		return os;
	}

	friend std::istream &operator>>(std::istream &is, Charsequence &str)
	{
		std::string temp;
		is >> temp;
		str = Charsequence(temp, str.storageEncoding);
		return is;
	}

	PointIterator begin() const { return PointIterator(this, 0); }
	PointIterator end() const { return PointIterator(this, size()); }

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
	std::vector<unsigned char> storage;
	charset storageEncoding;

	void validateEncoding()
	{
		if (storageEncoding == charset::utf8)
		{
			std::size_t i = 0;
			while (i < storage.size())
			{
				unsigned char byte = storage[i];
				if ((byte >= 0x80 && byte < 0xC2) || byte >= 0xF8)
				{
					throw std::invalid_argument("Invalid UTF-8 sequence");
				}
				std::size_t bytes = bytesInSequence(byte);
				if (bytes == 0 || i + bytes > storage.size())
				{
					throw std::invalid_argument("Invalid UTF-8 sequence");
				}
				for (std::size_t j = 1; j < bytes; ++j)
				{
					if ((storage[i + j] & 0xC0) != 0x80)
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
					codepoint |= (storage[i + 1] & 0x3F);
					if (codepoint < 0x80)
						throw std::invalid_argument("Overlong UTF-8 sequence");
				}
				else if (bytes == 3)
				{
					codepoint = (byte & 0x0F) << 12;
					codepoint |= (storage[i + 1] & 0x3F) << 6;
					codepoint |= (storage[i + 2] & 0x3F);
					if (codepoint < 0x800)
						throw std::invalid_argument("Overlong UTF-8 sequence");
					if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
						throw std::invalid_argument("Invalid UTF-8 surrogate");
				}
				else if (bytes == 4)
				{
					codepoint = (byte & 0x07) << 18;
					codepoint |= (storage[i + 1] & 0x3F) << 12;
					codepoint |= (storage[i + 2] & 0x3F) << 6;
					codepoint |= (storage[i + 3] & 0x3F);
					if (codepoint < 0x10000)
						throw std::invalid_argument("Overlong UTF-8 sequence");
					if (codepoint > 0x10FFFF)
						throw std::invalid_argument("Code point exceeds 0x10FFFF");
				}
				i += bytes;
			}
		}
	}

	std::size_t countPoints() const
	{
		if (storageEncoding != charset::utf8)
			return storage.size();
		std::size_t count = 0;
		std::size_t i = 0;
		while (i < storage.size())
		{
			i += bytesInSequence(storage[i]);
			count++;
		}
		return count;
	}

	std::size_t codePointToByte(std::size_t codePointIndex) const
	{
		if (storageEncoding != charset::utf8)
			return codePointIndex;
		std::size_t byteIndex = 0;
		for (std::size_t i = 0; i < codePointIndex && byteIndex < storage.size(); ++i)
		{
			byteIndex += bytesInSequence(storage[byteIndex]);
		}
		return byteIndex;
	}

	std::size_t byteToCodePoint(std::size_t byteIndex) const
	{
		if (storageEncoding != charset::utf8)
			return byteIndex;
		std::size_t codePointIndex = 0;
		std::size_t currentByte = 0;
		while (currentByte < byteIndex && currentByte < storage.size())
		{
			currentByte += bytesInSequence(storage[currentByte]);
			codePointIndex++;
		}
		return codePointIndex;
	}

	void appendPoint(Point point)
	{
		unsigned int codepoint = point.getValue();
		if (codepoint <= 0x7F)
		{
			storage.push_back(static_cast<unsigned char>(codepoint));
		}
		else if (codepoint <= 0x7FF)
		{
			storage.push_back(static_cast<unsigned char>(0xC0 | (codepoint >> 6)));
			storage.push_back(static_cast<unsigned char>(0x80 | (codepoint & 0x3F)));
		}
		else if (codepoint <= 0xFFFF)
		{
			if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
				throw std::invalid_argument("Surrogate code points not valid for UTF-8");
			storage.push_back(static_cast<unsigned char>(0xE0 | (codepoint >> 12)));
			storage.push_back(static_cast<unsigned char>(0x80 | ((codepoint >> 6) & 0x3F)));
			storage.push_back(static_cast<unsigned char>(0x80 | (codepoint & 0x3F)));
		}
		else if (codepoint <= 0x10FFFF)
		{
			storage.push_back(static_cast<unsigned char>(0xF0 | (codepoint >> 18)));
			storage.push_back(static_cast<unsigned char>(0x80 | ((codepoint >> 12) & 0x3F)));
			storage.push_back(static_cast<unsigned char>(0x80 | ((codepoint >> 6) & 0x3F)));
			storage.push_back(static_cast<unsigned char>(0x80 | (codepoint & 0x3F)));
		}
		else
		{
			throw std::invalid_argument("Code point exceeds 0x10FFFF");
		}
	}

	Point decodePoint(const std::vector<unsigned char> &data, std::size_t &index) const
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

	static std::string vectorToString(const std::vector<unsigned char> &vec)
	{
		return std::string(vec.begin(), vec.end());
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

	template <typename T,
			  typename = std::enable_if_t<
				  std::is_constructible_v<std::string, T> &&
				  !std::is_base_of_v<std::istream, std::decay_t<T>> &&
				  !std::is_same_v<std::decay_t<T>, Charsequence> &&
				  !std::is_same_v<std::decay_t<T>, Builder> &&
				  !std::is_same_v<std::decay_t<T>, Buffer>>>
	explicit Builder(T &&source, charset sourceEncoding = charset::utf8)
		: storageEncoding(charset::utf8)
	{
		std::string temp(std::forward<T>(source));
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

	Builder &append(const Charsequence &source)
	{
		auto bytes = source.getBytes(storageEncoding);
		storage.insert(storage.end(), bytes.begin(), bytes.end());
		return *this;
	}

	Builder &append(Point point)
	{
		Charsequence temp(&point, 1);
		auto bytes = temp.getBytes(storageEncoding);
		storage.insert(storage.end(), bytes.begin(), bytes.end());
		return *this;
	}

	Charsequence toCharsequence() const
	{
		return Charsequence(std::string(storage.begin(), storage.end()), storageEncoding);
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