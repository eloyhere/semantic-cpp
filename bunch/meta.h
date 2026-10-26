#pragma once

#include <cstddef>
#include <functional>

namespace bunch
{

class Meta
{
  public:
    // Application metadata attached to a character; Meta is not a Unicode
    // concept and does not affect any Unicode property.
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

} // namespace bunch

namespace std
{
template <>
struct hash<::bunch::Meta>
{
    size_t operator()(const ::bunch::Meta &meta) const noexcept
    {
        return hash<unsigned int>()(meta.getValue());
    }
};
} // namespace std
