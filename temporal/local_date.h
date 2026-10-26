#pragma once

#include "temporal/types.h"

#include "gregory/gregory.h"

namespace temporal
{

// The Gregorian implementation of the LocalDate contract. The contract itself
// lives in local_contract.h; this type stays a plain value type.
using LocalGregoryDate = gregory::Date;

} // namespace temporal
