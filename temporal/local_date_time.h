#pragma once

#include "temporal/types.h"

#include "gregory/gregory.h"

namespace temporal
{

// The Gregorian local date-time: LocalDate plus LocalTime, without an offset or
// a time zone.
using LocalDateTime = gregory::DateTime;

} // namespace temporal
