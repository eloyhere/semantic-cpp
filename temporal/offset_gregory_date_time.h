#pragma once

#include "temporal/types.h"

#include "gregory/gregory.h"

namespace temporal
{

// The Gregorian local date-time plus a fixed UTC offset.
using OffsetGregoryDateTime = gregory::OffsetDateTime;

} // namespace temporal
