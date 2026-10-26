#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <utility>
#include <vector>

#include "instant.h"
#include "week.h"

namespace temporal
{

class Year;

// A calendar month. relative is the month-of-year (February == 2); absolute is
// the month sequence since the calendar epoch (建元). leap marks a leap month.
// [start, end) is the instant range covered by the month; weeks holds its week
// cycles.
class Month
{
  public:
    Month() noexcept : absolute(0), relative(0), leap(false), start(), end(), year(nullptr) {}

    Month(Timestamp absoluteValue, Timestamp relativeValue, const Instant &startValue, const Instant &endValue,
          const Year *yearValue = nullptr)
        : absolute(absoluteValue), relative(relativeValue), leap(false), start(startValue), end(endValue),
          year(yearValue)
    {
    }

    Month(Timestamp absoluteValue, Timestamp relativeValue, bool leapValue, std::vector<Week> weeksValue,
          const Instant &startValue, const Instant &endValue, const Year *yearValue = nullptr)
        : absolute(absoluteValue), relative(relativeValue), leap(leapValue), weeks(std::move(weeksValue)),
          start(startValue), end(endValue), year(yearValue)
    {
    }

    Timestamp getAbsolute() const noexcept { return absolute; }
    Timestamp getRelative() const noexcept { return relative; }
    bool isLeap() const noexcept { return leap; }
    const Instant &getStart() const noexcept { return start; }
    const Instant &getEnd() const noexcept { return end; }
    const std::vector<Week> &getWeeks() const noexcept { return weeks; }
    const Year *getYear() const noexcept { return year; }
    void setYear(const Year *value) noexcept { year = value; }

    Module lengthOfDay() const
    {
        if (!weeks.empty())
        {
            Module total = 0;
            for (const Week &week : weeks)
            {
                total += week.lengthOfDay();
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
        if (!weeks.empty())
        {
            return static_cast<Module>(weeks.size());
        }
        return lengthOfDay() / 7;
    }

    Module lengthOfMonth() const noexcept { return 1; }
    Module lengthOfYear() const noexcept { return 0; }

    bool contains(const Instant &value) const noexcept { return start <= value && value < end; }

    Duration getDuration() const { return end - start; }

    friend bool operator==(const Month &left, const Month &right) noexcept
    {
        return left.absolute == right.absolute && left.relative == right.relative && left.leap == right.leap &&
               left.start == right.start && left.end == right.end;
    }

    friend bool operator!=(const Month &left, const Month &right) noexcept { return !(left == right); }

    friend bool operator<(const Month &left, const Month &right) noexcept
    {
        if (left.absolute != right.absolute)
        {
            return left.absolute < right.absolute;
        }
        return left.relative < right.relative;
    }

    friend bool operator<=(const Month &left, const Month &right) noexcept { return left < right || left == right; }

    friend bool operator>(const Month &left, const Month &right) noexcept { return right < left; }

    friend bool operator>=(const Month &left, const Month &right) noexcept { return right <= left; }

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
    std::vector<Week> weeks;
    Instant start;
    Instant end;
    const Year *year;
};

} // namespace temporal
