#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <utility>
#include <vector>

#include "instant.h"
#include "month.h"

namespace temporal
{

class Era;

// A calendar year. relative is the year inside its era; absolute is the year
// sequence since the calendar epoch (建元). leap marks a leap year. [start, end)
// is the instant range covered by the year; months holds its month cycles.
class Year
{
  public:
    Year() noexcept : absolute(0), relative(0), leap(false), start(), end(), era(nullptr) {}

    Year(Timestamp absoluteValue, Timestamp relativeValue, const Instant &startValue, const Instant &endValue,
         const Era *eraValue = nullptr)
        : absolute(absoluteValue), relative(relativeValue), leap(false), start(startValue), end(endValue),
          era(eraValue)
    {
    }

    Year(Timestamp absoluteValue, Timestamp relativeValue, bool leapValue, std::vector<Month> monthsValue,
         const Instant &startValue, const Instant &endValue, const Era *eraValue = nullptr)
        : absolute(absoluteValue),
          relative(relativeValue),
          leap(leapValue),
          months(std::move(monthsValue)),
          start(startValue),
          end(endValue),
          era(eraValue)
    {
    }

    Timestamp getAbsolute() const noexcept { return absolute; }
    Timestamp getRelative() const noexcept { return relative; }
    bool isLeap() const noexcept { return leap; }
    const Instant &getStart() const noexcept { return start; }
    const Instant &getEnd() const noexcept { return end; }
    const std::vector<Month> &getMonths() const noexcept { return months; }
    const Era *getEra() const noexcept { return era; }
    void setEra(const Era *value) noexcept { era = value; }

    Module lengthOfDay() const
    {
        if (!months.empty())
        {
            Module total = 0;
            for (const Month &month : months)
            {
                total += month.lengthOfDay();
            }
            if (total != 0)
            {
                return total;
            }
        }
        return dayCount();
    }

    Module lengthOfWeek() const
    {
        if (!months.empty())
        {
            Module total = 0;
            for (const Month &month : months)
            {
                total += month.lengthOfWeek();
            }
            if (total != 0)
            {
                return total;
            }
        }
        return lengthOfDay() / 7;
    }

    Module lengthOfMonth() const
    {
        if (!months.empty())
        {
            return static_cast<Module>(months.size());
        }
        return 0;
    }

    Module lengthOfYear() const noexcept { return 1; }

    bool contains(const Instant &value) const noexcept { return start <= value && value < end; }

    Duration getDuration() const { return end - start; }

    friend bool operator==(const Year &left, const Year &right) noexcept
    {
        return left.absolute == right.absolute && left.relative == right.relative && left.leap == right.leap &&
               left.start == right.start && left.end == right.end;
    }

    friend bool operator!=(const Year &left, const Year &right) noexcept { return !(left == right); }

    friend bool operator<(const Year &left, const Year &right) noexcept
    {
        if (left.absolute != right.absolute)
        {
            return left.absolute < right.absolute;
        }
        return left.relative < right.relative;
    }

    friend bool operator<=(const Year &left, const Year &right) noexcept { return left < right || left == right; }

    friend bool operator>(const Year &left, const Year &right) noexcept { return right < left; }

    friend bool operator>=(const Year &left, const Year &right) noexcept { return right <= left; }

  private:
    Module dayCount() const
    {
        const Timestamp seconds = getDuration().getSeconds();
        if (seconds <= 0)
        {
            return 0;
        }
        return static_cast<Module>(seconds / SecondsPerDay);
    }

    Timestamp absolute;
    Timestamp relative;
    bool leap;
    std::vector<Month> months;
    Instant start;
    Instant end;
    const Era *era;
};

} // namespace temporal
