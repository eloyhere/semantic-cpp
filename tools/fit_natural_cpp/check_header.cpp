// Compile-time / value check for the generated natural-calendar data header.
#include <cstdint>
#include <cstdio>

#include "temporal/natural/data/waveforms.h"

int main()
{
    const double jds[] = {
        2451545.0,          // J2000
        736435.240150463,   // natural epoch new moon (local -2696-03-11 01:45:49)
        1721467.7226851853, // 0001 正月 new moon
        6651859.584664351,  // 13500 正月 new moon
    };
    for (double jd : jds)
    {
        std::printf("JD %.9f sun %.12f moonDiff %.12f moonLon %.12f\n", jd,
                    temporal::natural::data::sunLongitude(jd),
                    temporal::natural::data::moonSunDifference(jd),
                    temporal::natural::data::moonLongitude(jd));
    }
    const std::int64_t months[] = {-1, 1, 1000, 200328};
    for (std::int64_t n : months)
    {
        std::printf("month %lld cumDayResidual %.9f\n", static_cast<long long>(n),
                    temporal::natural::data::cumulativeDayResidual(n));
    }
    const std::int64_t years[] = {-2696, 1, 1000, 13500};
    for (std::int64_t y : years)
    {
        std::printf("year %lld synodic %.9f tropical %.9f\n", static_cast<long long>(y),
                    temporal::natural::data::synodicYearLength(y),
                    temporal::natural::data::tropicalYearLength(y));
    }
    return 0;
}
