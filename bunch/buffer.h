#pragma once

#include "charset.h"
#include "builder.h"
#include "sequence.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace bunch
{

class Buffer
{
  public:
    static constexpr std::size_t defaultCapacity = 4096;

    Buffer() : storage(defaultCapacity), readPosition(0), writePosition(0), elementCount(0) {}
    explicit Buffer(std::size_t capacity) : storage(capacity > 0 ? capacity : defaultCapacity), readPosition(0), writePosition(0), elementCount(0) {}

    Buffer(std::string_view source) : storage(source.size() > 0 ? source.size() : defaultCapacity), readPosition(0), writePosition(0), elementCount(0)
    {
        write(source);
    }

    Buffer(const Buffer &other) : storage(), readPosition(0), writePosition(0), elementCount(0)
    {
        if (this == &other)
        {
            return;
        }
        std::lock_guard<std::mutex> lock(other.mutex);
        storage = other.storage;
        readPosition = other.readPosition;
        writePosition = other.writePosition;
        elementCount = other.elementCount;
    }

    Buffer(Buffer &&other) noexcept : storage(), readPosition(0), writePosition(0), elementCount(0)
    {
        if (this == &other)
        {
            return;
        }
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
        std::scoped_lock lock(mutex, other.mutex);
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
        std::scoped_lock lock(mutex, other.mutex);
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
        std::size_t availableSpace = storage.size() - elementCount;
        if (length > availableSpace)
        {
            ensureCapacity(elementCount + length);
        }
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

    std::size_t write(const char *input)
    {
        return input == nullptr ? 0 : write(std::string_view(input));
    }

    std::size_t write(const std::vector<unsigned char> &input)
    {
        return write(reinterpret_cast<const char *>(input.data()), input.size());
    }

    std::size_t write(const Bunch &input)
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
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return write(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    std::size_t write(int value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return write(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    std::size_t write(long value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return write(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    std::size_t write(long long value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return write(std::string_view(buf.data(), result.ptr - buf.data()));
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
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return write(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    std::size_t write(unsigned int value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return write(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    std::size_t write(unsigned long value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return write(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    std::size_t write(unsigned long long value)
    {
        std::array<char, 32> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return write(std::string_view(buf.data(), result.ptr - buf.data()));
    }

    std::size_t prepend(const char *input, std::size_t length)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (!input || length == 0)
        {
            return 0;
        }
        std::size_t availableSpace = storage.size() - elementCount;
        if (length > availableSpace)
        {
            ensureCapacity(elementCount + length);
        }
        std::size_t spaceBeforeRead = readPosition;
        if (spaceBeforeRead == 0)
        {
            spaceBeforeRead = storage.size();
        }
        std::size_t firstChunk = std::min(length, spaceBeforeRead);
        std::size_t secondChunk = length - firstChunk;
        if (firstChunk > 0)
        {
            readPosition = (readPosition + storage.size() - firstChunk) % storage.size();
            std::memcpy(&storage[readPosition], input + secondChunk, firstChunk);
        }
        if (secondChunk > 0)
        {
            readPosition = (readPosition + storage.size() - secondChunk) % storage.size();
            std::memcpy(&storage[readPosition], input, secondChunk);
        }
        elementCount += length;
        return length;
    }

    std::size_t prepend(std::string_view input)
    {
        return prepend(input.data(), input.size());
    }

    std::size_t prepend(const char *input)
    {
        return input == nullptr ? 0 : prepend(std::string_view(input));
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

    std::size_t append(const char *input)
    {
        return write(input);
    }

    std::size_t append(const std::vector<unsigned char> &input)
    {
        return write(input);
    }

    std::size_t append(const Bunch &input)
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
        return dataUnsafe();
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
            std::vector<unsigned char> d;
            {
                std::scoped_lock lock(mutex);
                d = dataUnsafe();
            }
            result.write(reinterpret_cast<const char *>(d.data()), d.size());
            result.write(reinterpret_cast<const char *>(d.data()), d.size());
        }
        else
        {
            std::scoped_lock lock(mutex, other.mutex);
            auto thisData = dataUnsafe();
            auto otherData = other.dataUnsafe();
            result.write(reinterpret_cast<const char *>(thisData.data()), thisData.size());
            result.write(reinterpret_cast<const char *>(otherData.data()), otherData.size());
        }
        return result;
    }

    Buffer operator+(std::string_view str) const
    {
        Buffer result;
        {
            std::lock_guard<std::mutex> lockThis(mutex);
            auto thisData = dataUnsafe();
            result.write(reinterpret_cast<const char *>(thisData.data()), thisData.size());
        }
        result.write(str.data(), str.size());
        return result;
    }

    Buffer &operator+=(const Buffer &other)
    {
        if (this == &other)
        {
            std::vector<unsigned char> d;
            {
                std::lock_guard<std::mutex> lock(mutex);
                d = dataUnsafe();
            }
            write(d);
        }
        else
        {
            std::scoped_lock lock(mutex, other.mutex);
            auto otherData = other.dataUnsafe();
            writeUnsafe(reinterpret_cast<const char *>(otherData.data()), otherData.size());
        }
        return *this;
    }

    Buffer &operator+=(std::string_view str)
    {
        write(str);
        return *this;
    }

    Buffer &operator+=(const char *str)
    {
        write(str);
        return *this;
    }

    Buffer &operator<<(std::string_view input)
    {
        write(input);
        return *this;
    }

    Buffer &operator<<(const char *input)
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
        if (elementCount > 0)
        {
            readDataUnsafe(output.data(), elementCount);
        }
        readPosition = 0;
        writePosition = 0;
        elementCount = 0;
        return *this;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Buffer &buffer)
    {
        auto d = buffer.data();
        stream.write(reinterpret_cast<const char *>(d.data()), d.size());
        return stream;
    }

    friend std::istream &operator>>(std::istream &stream, Buffer &buffer)
    {
        std::string temp;
        std::getline(stream, temp);
        buffer.write(std::string_view(temp));
        return stream;
    }

    friend Buffer operator+(std::string_view left, const Buffer &right)
    {
        Buffer result;
        {
            std::lock_guard<std::mutex> lockRight(right.mutex);
            auto rightData = right.dataUnsafe();
            result.write(left.data(), left.size());
            result.write(reinterpret_cast<const char *>(rightData.data()), rightData.size());
        }
        return result;
    }

  private:
    std::vector<unsigned char> storage;
    std::size_t readPosition;
    std::size_t writePosition;
    std::size_t elementCount;
    mutable std::mutex mutex;

    void writeUnsafe(const char *input, std::size_t length)
    {
        if (!input || length == 0)
            return;
        std::size_t availableSpace = storage.size() - elementCount;
        if (length > availableSpace)
        {
            ensureCapacity(elementCount + length);
        }
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

    void ensureCapacity(std::size_t required)
    {
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
        writePosition = elementCount % storage.size();
    }

    std::vector<unsigned char> dataUnsafe() const
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
} // namespace bunch

namespace std
{
template <>
struct hash<::bunch::Buffer>
{
    size_t operator()(const ::bunch::Buffer &buffer) const noexcept
    {
        auto d = buffer.data();
        size_t seed = 0;
        for (auto byte : d)
        {
            seed ^= hash<unsigned char>()(byte) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <>
struct less<::bunch::Buffer>
{
    bool operator()(const ::bunch::Buffer &left, const ::bunch::Buffer &right) const noexcept
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
