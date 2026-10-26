#pragma once

#include "charset.h"
#include "point.h"
#include "sequence.h"

#include <array>
#include <charconv>
#include <cstddef>
#include <deque>
#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace bunch
{

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
            convert(source, sourceEncoding, targetEncoding, storage);
        }
    }

    Builder &prepend(const Bunch &source)
    {
        auto bytes = source.getBytes(storageEncoding);
        storage.insert(storage.begin(), bytes.begin(), bytes.end());
        return *this;
    }

    Builder &prepend(std::string_view source, charset sourceEncoding = charset::utf8)
    {
        Bunch temp(source, sourceEncoding, storageEncoding);
        return prepend(temp);
    }

    Builder &prepend(const char *source, charset sourceEncoding = charset::utf8)
    {
        return source == nullptr ? *this : prepend(std::string_view(source), sourceEncoding);
    }

    Builder &prepend(Point point)
    {
        Bunch temp(&point, 1, storageEncoding);
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

    Builder &insert(std::size_t position, const Bunch &source)
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

    Builder &insert(std::size_t position, std::string_view source, charset sourceEncoding = charset::utf8)
    {
        Bunch temp(source, sourceEncoding, storageEncoding);
        return insert(position, temp);
    }

    Builder &insert(std::size_t position, const char *source, charset sourceEncoding = charset::utf8)
    {
        return source == nullptr ? *this : insert(position, std::string_view(source), sourceEncoding);
    }

    Builder &insert(std::size_t position, Point point)
    {
        Bunch temp(&point, 1, storageEncoding);
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
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return insert(position, std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &insert(std::size_t position, int value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return insert(position, std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &insert(std::size_t position, long value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return insert(position, std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &insert(std::size_t position, long long value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return insert(position, std::string_view(buf.data(), result.ptr - buf.data()));
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
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return insert(position, std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &insert(std::size_t position, unsigned int value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return insert(position, std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &insert(std::size_t position, unsigned long value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return insert(position, std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &insert(std::size_t position, unsigned long long value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return insert(position, std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &append(const Bunch &source)
    {
        auto bytes = source.getBytes(storageEncoding);
        storage.insert(storage.end(), bytes.begin(), bytes.end());
        return *this;
    }

    Builder &append(std::string_view source, charset sourceEncoding = charset::utf8)
    {
        Bunch temp(source, sourceEncoding, storageEncoding);
        return append(temp);
    }

    Builder &append(const char *source, charset sourceEncoding = charset::utf8)
    {
        return source == nullptr ? *this : append(std::string_view(source), sourceEncoding);
    }

    Builder &append(Point point)
    {
        Bunch temp(&point, 1, storageEncoding);
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
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return append(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &append(int value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return append(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &append(long value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return append(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &append(long long value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return append(std::string_view(buf.data(), result.ptr - buf.data()));
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
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return append(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &append(unsigned int value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return append(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &append(unsigned long value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return append(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder &append(unsigned long long value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return append(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    Builder operator+(const Builder &other) const
    {
        Builder result(*this);
        result.append(other.toBunch());
        return result;
    }

    Builder operator+(const Bunch &other) const
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

    Builder operator+(const char *str) const
    {
        Builder result(*this);
        result.append(str);
        return result;
    }

    Builder &operator+=(const Builder &other)
    {
        append(other.toBunch());
        return *this;
    }

    Builder &operator+=(const Bunch &other)
    {
        append(other);
        return *this;
    }

    Builder &operator+=(std::string_view str)
    {
        append(str);
        return *this;
    }

    Builder &operator+=(const char *str)
    {
        append(str);
        return *this;
    }

    Bunch toBunch() const
    {
        std::string temp(storage.begin(), storage.end());
        return Bunch(std::string_view(temp), storageEncoding, storageEncoding);
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

    friend Builder operator+(std::string_view left, const Builder &right)
    {
        Builder result(left, right.storageEncoding, right.storageEncoding);
        result.append(right.toBunch());
        return result;
    }

    friend Builder operator+(const char *left, const Builder &right)
    {
        Builder result(left == nullptr ? std::string_view() : std::string_view(left), right.storageEncoding, right.storageEncoding);
        result.append(right.toBunch());
        return result;
    }

  private:
    std::deque<unsigned char> storage;
    charset storageEncoding;
};
} // namespace bunch

namespace std
{
template <>
struct hash<::bunch::Builder>
{
    size_t operator()(const ::bunch::Builder &builder) const noexcept
    {
        auto bytes = builder.getBytes();
        size_t seed = 0;
        for (auto byte : bytes)
        {
            seed ^= hash<unsigned char>()(byte) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <>
struct less<::bunch::Builder>
{
    bool operator()(const ::bunch::Builder &left, const ::bunch::Builder &right) const noexcept
    {
        return left.toBunch().compare(right.toBunch()) < 0;
    }
};

} // namespace std
