#pragma once

#include <cstdint>

namespace temporal
{

// Signed sequence/offset type used by day numbers, instants and calendar cycles.
using Timestamp = std::int64_t;

// Unsigned count/capacity type used by lengths and collection sizes.
using Module = std::uint64_t;

} // namespace temporal
