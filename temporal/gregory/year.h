#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <stdexcept>

#include "era.h"

namespace temporal::gregory
{

// The Gregorian year is an astronomical year number with an era view on top:
// year 0 is 1 BCE, year -1 is 2 BCE, and getYearInEra() reports the number used
// by inscriptions and by java.time's YEAR_OF_ERA. The supported range is the
// ISO 8601 expanded four-digit range [-9999, 9999]; it is a property of this
// calendar policy, not of a global Year type, so another calendar can number
// its years from a different epoch, with a different range and with its own era
// vocabulary.
class Year
{
  public:
    constexpr Year() noexcept : value(0) {}
    explicit constexpr Year(std::int32_t value) noexcept : value(value) {}

    static constexpr std::int32_t minimum() noexcept { return -9999; }
    static constexpr std::int32_t maximum() noexcept { return 9999; }

    constexpr std::int32_t getValue() const noexcept { return value; }
    constexpr bool isOk() const noexcept { return value >= minimum() && value <= maximum(); }

    constexpr Era getEra() const noexcept { return value > 0 ? Era::Ce : Era::Bce; }

    constexpr Timestamp getYearInEra() const noexcept
    {
        return value > 0 ? static_cast<Timestamp>(value) : 1 - static_cast<Timestamp>(value);
    }

    static constexpr Year fromEra(Era era, Timestamp yearInEra)
    {
        if (yearInEra < 1)
        {
            throw std::invalid_argument("temporal: era year must be positive");
        }
        const Timestamp value = era == Era::Ce ? yearInEra : 1 - yearInEra;
        if (value < minimum() || value > maximum())
        {
            throw std::invalid_argument("temporal: year outside the supported range");
        }
        return Year(static_cast<std::int32_t>(value));
    }

    constexpr bool operator==(Year other) const noexcept { return value == other.value; }
    constexpr bool operator!=(Year other) const noexcept { return value != other.value; }
    constexpr bool operator<(Year other) const noexcept { return value < other.value; }
    constexpr bool operator<=(Year other) const noexcept { return value <= other.value; }
    constexpr bool operator>(Year other) const noexcept { return value > other.value; }
    constexpr bool operator>=(Year other) const noexcept { return value >= other.value; }

  private:
    std::int32_t value;
};

} // namespace temporal::gregory
