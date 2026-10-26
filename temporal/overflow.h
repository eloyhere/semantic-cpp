#pragma once

#include "temporal/types.h"

namespace temporal
{

// Policy for calendar-field arithmetic that cannot land on the requested day
// (adding one month to January 31st, or one year to February 29th). This is
// Temporal's overflow option made explicit:
//
//   Constrain  clamp to the last day of the target month (java.time, Noda Time,
//              Temporal's default), so 2024-01-31 + 1 month is 2024-02-29
//   Reject     throw std::out_of_range instead of silently changing the day
//
// C++20's year_month_day has neither policy and can produce !ok() values; making
// the choice a parameter keeps the behaviour local to the call site.
enum class Overflow
{
    Constrain,
    Reject
};

} // namespace temporal
