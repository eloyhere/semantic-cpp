#pragma once

#include "temporal/types.h"

namespace temporal::gregory
{

// The Gregorian era pair. It lives with the Gregorian calendar because era
// vocabularies are calendar data: a calendar may use regnal years, a
// sexagenary cycle or no era at all, and none of those fit this enumeration.
// Year 0 (astronomical numbering) is 1 BCE and year -1 is 2 BCE.
enum class Era : unsigned char
{
    Bce,
    Ce
};

} // namespace temporal::gregory
