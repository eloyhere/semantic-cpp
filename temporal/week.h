#pragma once

#include "temporal/types.h"

#include <cstdint>
#include <utility>
#include <vector>

#include "day.h"
#include "instant.h"

namespace temporal
{

class Month;

// A calendar week. relative is the week-of-year; absolute is the week sequence
// since the calendar epoch (建元). [start, end) is the instant range covered by
// the week. days holds the day cycles inside the week.
class Week
{
  public:
    Week() noexcept : absolute(0), relative(0), start(), end(), month(nullptr) {}

    Week(Timestamp absoluteValue, Timestamp relativeValue, const Instant &startValue, const Instant &endValue,
         const Month *monthValue = nullptr)
        : absolute(absoluteValue), relative(relativeValue), start(startValue), end(endValue), month(monthValue)
    {
    }

    Week(Timestamp absoluteValue, Timestamp relativeValue, std::vector<Day> daysValue, const Instant &startValue,
         const Instant &endValue, const Month *monthValue = nullptr)
        : absolute(absoluteValue), relative(relativeValue), days(std::move(daysValue)), start(startValue),
          end(endValue), month(monthValue)
    {
    }

    Timestamp getAbsolute() const noexcept { return absolute; }
    Timestamp getRelative() const noexcept { return relative; }
    const Instant &getStart() const noexcept { return start; }
    const Instant &getEnd() const noexcept { return end; }
    const std::vector<Day> &getDays() const noexcept { return days; }
    const Month *getMonth() const noexcept { return month; }
    void setMonth(const Month *value) noexcept { month = value; }

    Module lengthOfDay() const
    {
        if (!days.empty())
        {
            return static_cast<Module>(days.size());
        }
        return dayCount();
    }

    Module lengthOfWeek() const noexcept { return 1; }
    Module lengthOfMonth() const noexcept { return 0; }
    Module lengthOfYear() const noexcept { return 0; }

    bool contains(const Instant &value) const noexcept { return start <= value && value < end; }

    Duration getDuration() const { return end - start; }

    friend bool operator==(const Week &left, const Week &right) noexcept
    {
        return left.absolute == right.absolute && left.relative == right.relative && left.start == right.start &&
               left.end == right.end;
    }

    friend bool operator!=(const Week &left, const Week &right) noexcept { return !(left == right); }

    friend bool operator<(const Week &left, const Week &right) noexcept
    {
        if (left.absolute != right.absolute)
        {
            return left.absolute < right.absolute;
        }
        return left.relative < right.relative;
    }

    friend bool operator<=(const Week &left, const Week &right) noexcept { return left < right || left == right; }

    friend bool operator>(const Week &left, const Week &right) noexcept { return right < left; }

    friend bool operator>=(const Week &left, const Week &right) noexcept { return right <= left; }

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
    std::vector<Day> days;
    Instant start;
    Instant end;
    const Month *month;
};

} // namespace temporal
