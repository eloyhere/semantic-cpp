#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <utility>
#include <vector>

#include "instant.h"
#include "year.h"

namespace temporal
{

// A calendar era. absolute is the era sequence since the calendar epoch (建元);
// relative is the position of this era inside a larger cycle, or 0 when the
// calendar has no such cycle. [start, end) is the instant range of the era;
// years holds its year cycles.
class Era
{
  public:
    Era() noexcept : absolute(0), relative(0), start(), end() {}

    Era(Timestamp absoluteValue, Timestamp relativeValue, const Instant &startValue, const Instant &endValue)
        : absolute(absoluteValue), relative(relativeValue), start(startValue), end(endValue)
    {
    }

    Era(Timestamp absoluteValue, Timestamp relativeValue, std::vector<Year> yearsValue, const Instant &startValue,
        const Instant &endValue)
        : absolute(absoluteValue), relative(relativeValue), years(std::move(yearsValue)), start(startValue),
          end(endValue)
    {
    }

    Timestamp getAbsolute() const noexcept { return absolute; }
    Timestamp getRelative() const noexcept { return relative; }
    const Instant &getStart() const noexcept { return start; }
    const Instant &getEnd() const noexcept { return end; }
    const std::vector<Year> &getYears() const noexcept { return years; }

    Module lengthOfDay() const
    {
        if (!years.empty())
        {
            Module total = 0;
            for (const Year &year : years)
            {
                total += year.lengthOfDay();
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
        if (!years.empty())
        {
            Module total = 0;
            for (const Year &year : years)
            {
                total += year.lengthOfWeek();
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
        if (!years.empty())
        {
            Module total = 0;
            for (const Year &year : years)
            {
                total += year.lengthOfMonth();
            }
            return total;
        }
        return 0;
    }

    Module lengthOfYear() const
    {
        if (!years.empty())
        {
            return static_cast<Module>(years.size());
        }
        return 0;
    }

    bool contains(const Instant &value) const noexcept { return start <= value && value < end; }

    Duration getDuration() const { return end - start; }

    friend bool operator==(const Era &left, const Era &right) noexcept
    {
        return left.absolute == right.absolute && left.relative == right.relative && left.start == right.start &&
               left.end == right.end;
    }

    friend bool operator!=(const Era &left, const Era &right) noexcept { return !(left == right); }

    friend bool operator<(const Era &left, const Era &right) noexcept
    {
        if (left.absolute != right.absolute)
        {
            return left.absolute < right.absolute;
        }
        return left.relative < right.relative;
    }

    friend bool operator<=(const Era &left, const Era &right) noexcept { return left < right || left == right; }

    friend bool operator>(const Era &left, const Era &right) noexcept { return right < left; }

    friend bool operator>=(const Era &left, const Era &right) noexcept { return right <= left; }

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
    std::vector<Year> years;
    Instant start;
    Instant end;
};

} // namespace temporal
