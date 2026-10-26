#pragma once

#include "temporal/types.h"

#include "gregory/gregory.h"

namespace temporal
{

// The Gregorian local date. Only the Gregorian calendar is implemented so far;
// when another calendar lands, this name becomes the concrete default and the
// calendar-specific spelling stays explicit.
using LocalDate = gregory::Date;

} // namespace temporal
