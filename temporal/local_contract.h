#pragma once

#include <type_traits>
#include <utility>

#include "duration.h"
#include "period.h"

namespace temporal
{

// Compile-time contracts for the calendar-independent local value families.
// They are intentionally not base classes: concrete implementations such as
// LocalGregoryDate/LocalGregoryTime/LocalGregoryDateTime stay plain value
// types and only have to provide the required expressions. A typo or a missing
// operation fails at the static_assert in the implementation's test, not with a
// virtual call at runtime.

template <typename T, typename = void> struct LocalDate : std::false_type
{
};

template <typename T>
struct LocalDate<T, std::void_t<decltype(std::declval<const T &>().getYear()),
                                decltype(std::declval<const T &>().getMonth()),
                                decltype(std::declval<const T &>().getDay()),
                                decltype(std::declval<const T &>().getDayNumber()),
                                decltype(std::declval<const T &>().to_string()),
                                decltype(std::declval<const T &>() == std::declval<const T &>())>> : std::true_type
{
};

template <typename T, typename = void> struct LocalTime : std::false_type
{
};

template <typename T>
struct LocalTime<T, std::void_t<decltype(std::declval<const T &>().getHour()),
                                decltype(std::declval<const T &>().getMinute()),
                                decltype(std::declval<const T &>().getSecond()),
                                decltype(std::declval<const T &>().getNanosecond()),
                                decltype(std::declval<const T &>().getNanoseconds()),
                                decltype(std::declval<const T &>().plus(std::declval<Duration>())),
                                decltype(std::declval<const T &>().until(std::declval<const T &>())),
                                decltype(std::declval<const T &>().to_string())>> : std::true_type
{
};

template <typename T, typename = void> struct LocalDateTime : std::false_type
{
};

template <typename T>
struct LocalDateTime<T, std::void_t<decltype(std::declval<const T &>().getDate()),
                                    decltype(std::declval<const T &>().getTime()),
                                    decltype(std::declval<const T &>().getYear()),
                                    decltype(std::declval<const T &>().getMonth()),
                                    decltype(std::declval<const T &>().getDay()),
                                    decltype(std::declval<const T &>().getHour()),
                                    decltype(std::declval<const T &>().getMinute()),
                                    decltype(std::declval<const T &>().getSecond()),
                                    decltype(std::declval<const T &>().getNanosecond()),
                                    decltype(std::declval<const T &>().plus(std::declval<Duration>())),
                                    decltype(std::declval<const T &>().plus(std::declval<Period>())),
                                    decltype(std::declval<const T &>().durationUntil(std::declval<const T &>())),
                                    decltype(std::declval<const T &>().to_string())>> : std::true_type
{
};

} // namespace temporal
