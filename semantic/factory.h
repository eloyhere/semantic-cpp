// Factory functions for the collector and semantic layers.
//
// Every factory used to live in collector.h (the collector factories) and in
// semantic.h (the stream factories). They now share this header and their
// `use` prefix has been dropped from every factory name, so `count()`,
// `toVector()` or `range()` are used instead of the old prefixed spellings.
//
// Dependency note: the collector factories only need `collector::Collector`,
// but the semantic factories need the complete `semantic::Semantic` type. The
// collectable classes in semantic.h in turn call the collector factories. To
// keep every entry point working (semantic.h first, factory.h first, or any
// other order) this header is split into two guarded sections:
//
//   * SEMANTIC_CPP_FACTORY_COLLECTOR - only needs collector.h.
//   * SEMANTIC_CPP_FACTORY_SEMANTIC  - only compiled once semantic.h has
//                                      finished its class definitions; it
//                                      sets SEMANTIC_CPP_SEMANTIC_COMPLETE.
//
// semantic.h includes this header twice: once before its class definitions and
// once after them. Both inclusions are idempotent, therefore this header
// intentionally has no `#pragma once`.
#ifndef SEMANTIC_CPP_FACTORY_COLLECTOR
#define SEMANTIC_CPP_FACTORY_COLLECTOR

#include "collector.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <complex>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <functional>
#include <future>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <stack>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace collector
{
template <typename E, typename A, typename R>
auto full(const Identity<A> &identity, const Accumulator<A, E> &accumulator, const Combiner<A> &combiner, const Finisher<A, R> &finisher) -> Collector<E, A, R>
{
    Interrupt<E, A> interrupt = [](const E element, const function::Timestamp &index, const A &accumulator) -> bool {
        return false;
    };
    return Collector<E, A, R>(identity, std::move(interrupt), accumulator, combiner, finisher);
}

template <typename E, typename A, typename R>
auto shortable(const Identity<A> &identity, const Interrupt<E, A> &interrupt, const Accumulator<A, E> &accumulator, const Combiner<A> &combiner, const Finisher<A, R> &finisher) -> Collector<E, A, R>
{
    return Collector<E, A, R>(identity, interrupt, accumulator, combiner, finisher);
}

template <typename E, typename Predicate>
auto allMatch(Predicate &&predicate) -> Collector<E, bool, bool>
{
    return shortable<E, bool, bool>(
        []() -> bool { return true; },
        [](E element, function::Timestamp index, bool accumulator) -> bool { return !accumulator; },
        [predicate](bool accumulator, E element, function::Timestamp index) -> bool {
            if constexpr (std::is_invocable_r_v<bool, Predicate, E, function::Timestamp>)
                return accumulator && std::invoke(predicate, element, index);
            else if constexpr (std::is_invocable_r_v<bool, Predicate, E>)
                return accumulator && std::invoke(predicate, element);
            return false;
        },
        [](bool a, bool b) -> bool { return a && b; },
        [](bool accumulator) -> bool { return accumulator; });
}

template <typename E, typename Predicate>
auto anyMatch(Predicate &&predicate) -> Collector<E, bool, bool>
{
    return shortable<E, bool, bool>(
        []() -> bool { return false; },
        [](E element, function::Timestamp index, bool accumulator) -> bool { return accumulator; },
        [predicate](bool accumulator, E element, function::Timestamp index) -> bool {
            if constexpr (std::is_invocable_r_v<bool, Predicate, E, function::Timestamp>)
                return accumulator || std::invoke(predicate, element, index);
            else if constexpr (std::is_invocable_r_v<bool, Predicate, E>)
                return accumulator || std::invoke(predicate, element);
            return false;
        },
        [](bool a, bool b) -> bool { return a || b; },
        [](bool accumulator) -> bool { return accumulator; });
}

template <typename E, typename Predicate>
auto noneMatch(Predicate &&predicate) -> Collector<E, bool, bool>
{
    return shortable<E, bool, bool>(
        []() -> bool { return true; },
        [](E element, function::Timestamp index, bool accumulator) -> bool { return !accumulator; },
        [predicate](bool accumulator, E element, function::Timestamp index) -> bool {
            if constexpr (std::is_invocable_r_v<bool, Predicate, E, function::Timestamp>)
                return accumulator && !std::invoke(predicate, element, index);
            else if constexpr (std::is_invocable_r_v<bool, Predicate, E>)
                return accumulator && !std::invoke(predicate, element);
            return false;
        },
        [](bool a, bool b) -> bool { return a && b; },
        [](bool accumulator) -> bool { return accumulator; });
}

template <typename E, typename Consumer>
auto forEach(Consumer &&consumer) -> Collector<E, function::Module, function::Module>
{
    return full<E, function::Module, function::Module>(
        []() -> function::Module { return 0; },
        [consumer](const function::Module &accumulator, const E &element, const function::Timestamp &index) -> function::Module {
            if constexpr (std::is_invocable_r_v<void, Consumer, E, function::Timestamp>)
                std::invoke(consumer, element, index);
            else if constexpr (std::is_invocable_r_v<void, Consumer, E>)
                std::invoke(consumer, element);
            return accumulator + 1;
        },
        [](const function::Module &a, const function::Module &b) -> function::Module { return a + b; },
        [](const function::Module &identityValue) -> function::Module { return identityValue; });
}

template <typename E>
auto count() -> Collector<E, function::Module, function::Module>
{
    return full<E, function::Module, function::Module>(
        []() -> function::Module { return 0LL; },
        [](function::Module accumulatorValue, E element, function::Timestamp index) -> function::Module { return accumulatorValue + 1; },
        [](function::Module a, function::Module b) -> function::Module { return a + b; },
        [](function::Module accumulatorValue) -> function::Module { return accumulatorValue; });
}

template <typename E, typename A, typename R>
auto collect(const Identity<R> &identity, const Accumulator<A, E> &accumulator, const Combiner<A> &combiner, const Finisher<A, R> &finisher) -> Collector<E, A, R>
{
    return full<E, A, R>(identity, accumulator, combiner, finisher);
}

template <typename E, typename A, typename R>
Collector<E, A, R> collect(const Identity<R> &identity, const Interrupt<E, A> &interrupt, const Accumulator<A, E> &accumulator, const Combiner<A> &combiner, const Finisher<A, R> &finisher)
{
    return shortable<E, A, R>(identity, interrupt, accumulator, combiner, finisher);
}

template <typename E, typename D>
auto summate() -> Collector<E, D, D>
{
    return full<E, D, D>(
        []() -> D { return D{}; },
        [](D accumulatorValue, E element, function::Timestamp index) -> D { return accumulatorValue + static_cast<D>(element); },
        [](D a, D b) -> D { return a + b; },
        [](D accumulatorValue) -> D { return accumulatorValue; });
}

template <typename E, typename D>
auto summate(const function::Function<E, D> &mapper) -> Collector<E, D, D>
{
    return full<E, D, D>(
        []() -> D { return D{}; },
        [mapper](D accumulatorValue, E element, function::Timestamp index) -> D { return accumulatorValue + mapper(element); },
        [](D a, D b) -> D { return a + b; },
        [](D accumulatorValue) -> D { return accumulatorValue; });
}

template <typename E, typename D>
auto average() -> Collector<E, std::pair<D, function::Module>, D>
{
    return full<E, std::pair<D, function::Module>, D>(
        []() -> std::pair<D, function::Module> { return std::make_pair(D{}, 0); },
        [](std::pair<D, function::Module> accumulatorValue, E element, function::Timestamp index) -> std::pair<D, function::Module> {
            D value = static_cast<D>(element);
            return std::make_pair(accumulatorValue.first + value, accumulatorValue.second + 1);
        },
        [](std::pair<D, function::Module> a, std::pair<D, function::Module> b) -> std::pair<D, function::Module> {
            return std::make_pair(a.first + b.first, a.second + b.second);
        },
        [](std::pair<D, function::Module> accumulatorValue) -> D {
            if (accumulatorValue.second == 0)
                return D{};
            return accumulatorValue.first / static_cast<D>(accumulatorValue.second);
        });
}

template <typename E, typename D>
auto average(const function::Function<E, D> &mapper) -> Collector<E, std::pair<D, function::Module>, D>
{
    return full<E, std::pair<D, function::Module>, D>(
        []() -> std::pair<D, function::Module> { return std::make_pair(D{}, 0); },
        [mapper](std::pair<D, function::Module> accumulatorValue, E element, function::Timestamp index) -> std::pair<D, function::Module> {
            D value = mapper(element);
            return std::make_pair(accumulatorValue.first + value, accumulatorValue.second + 1);
        },
        [](std::pair<D, function::Module> a, std::pair<D, function::Module> b) -> std::pair<D, function::Module> {
            return std::make_pair(a.first + b.first, a.second + b.second);
        },
        [](std::pair<D, function::Module> accumulatorValue) -> D {
            if (accumulatorValue.second == 0)
                return D{};
            return accumulatorValue.first / static_cast<D>(accumulatorValue.second);
        });
}

template <typename E, typename D>
auto range() -> Collector<E, std::pair<D, D>, D>
{
    return full<E, std::pair<D, D>, D>(
        []() -> std::pair<D, D> { return std::pair<D, D>(D{}, D{}); },
        [](std::pair<D, D> accumulatorValue, E element, function::Timestamp index) -> std::pair<D, D> {
            D mapped = static_cast<D>(element);
            if (accumulatorValue.first == D{} && accumulatorValue.second == D{})
                return std::pair<D, D>(mapped, mapped);
            if (mapped < accumulatorValue.first)
                return std::pair<D, D>(mapped, accumulatorValue.second);
            if (mapped > accumulatorValue.second)
                return std::pair<D, D>(accumulatorValue.first, mapped);
            return accumulatorValue;
        },
        [](std::pair<D, D> a, std::pair<D, D> b) -> std::pair<D, D> {
            if (a.first == D{} && a.second == D{})
                return b;
            if (b.first == D{} && b.second == D{})
                return a;
            return std::pair<D, D>(a.first < b.first ? a.first : b.first, a.second > b.second ? a.second : b.second);
        },
        [](std::pair<D, D> accumulatorValue) -> D {
            if (accumulatorValue.first == D{} && accumulatorValue.second == D{})
                return D{};
            return accumulatorValue.second - accumulatorValue.first;
        });
}

template <typename E, typename D>
auto range(const function::Function<E, D> &mapper) -> Collector<E, std::pair<D, D>, D>
{
    return full<E, std::pair<D, D>, D>(
        []() -> std::pair<D, D> { return std::pair<D, D>(D{}, D{}); },
        [mapper](std::pair<D, D> accumulatorValue, E element, function::Timestamp index) -> std::pair<D, D> {
            D mapped = mapper(element);
            if (accumulatorValue.first == D{} && accumulatorValue.second == D{})
                return std::pair<D, D>(mapped, mapped);
            if (mapped < accumulatorValue.first)
                return std::pair<D, D>(mapped, accumulatorValue.second);
            if (mapped > accumulatorValue.second)
                return std::pair<D, D>(accumulatorValue.first, mapped);
            return accumulatorValue;
        },
        [](std::pair<D, D> a, std::pair<D, D> b) -> std::pair<D, D> {
            if (a.first == D{} && a.second == D{})
                return b;
            if (b.first == D{} && b.second == D{})
                return a;
            return std::pair<D, D>(a.first < b.first ? a.first : b.first, a.second > b.second ? a.second : b.second);
        },
        [](std::pair<D, D> accumulatorValue) -> D {
            if (accumulatorValue.first == D{} && accumulatorValue.second == D{})
                return D{};
            return accumulatorValue.second - accumulatorValue.first;
        });
}

template <typename E, typename D>
auto minimum() -> Collector<E, std::optional<D>, std::optional<D>>
{
    return full<E, std::optional<D>, std::optional<D>>(
        []() -> std::optional<D> { return std::nullopt; },
        [](std::optional<D> accumulatorValue, E element, function::Timestamp index) -> std::optional<D> {
            D value = static_cast<D>(element);
            if (!accumulatorValue.has_value() || value < accumulatorValue.value())
                return std::optional<D>(value);
            return accumulatorValue;
        },
        [](std::optional<D> a, std::optional<D> b) -> std::optional<D> {
            if (!a.has_value())
                return b;
            if (!b.has_value())
                return a;
            return a.value() < b.value() ? a : b;
        },
        [](std::optional<D> accumulatorValue) -> std::optional<D> { return accumulatorValue; });
}

template <typename E, typename D>
auto minimum(const function::Function<E, D> &mapper) -> Collector<E, std::optional<D>, std::optional<D>>
{
    return full<E, std::optional<D>, std::optional<D>>(
        []() -> std::optional<D> { return std::nullopt; },
        [mapper](std::optional<D> accumulatorValue, E element, function::Timestamp index) -> std::optional<D> {
            D value = mapper(element);
            if (!accumulatorValue.has_value() || value < accumulatorValue.value())
                return std::optional<D>(value);
            return accumulatorValue;
        },
        [](std::optional<D> a, std::optional<D> b) -> std::optional<D> {
            if (!a.has_value())
                return b;
            if (!b.has_value())
                return a;
            return a.value() < b.value() ? a : b;
        },
        [](std::optional<D> accumulatorValue) -> std::optional<D> { return accumulatorValue; });
}

template <typename E, typename D>
auto maximum() -> Collector<E, std::optional<D>, std::optional<D>>
{
    return full<E, std::optional<D>, std::optional<D>>(
        []() -> std::optional<D> { return std::nullopt; },
        [](std::optional<D> accumulatorValue, E element, function::Timestamp index) -> std::optional<D> {
            D value = static_cast<D>(element);
            if (!accumulatorValue.has_value() || value > accumulatorValue.value())
                return std::optional<D>(value);
            return accumulatorValue;
        },
        [](std::optional<D> a, std::optional<D> b) -> std::optional<D> {
            if (!a.has_value())
                return b;
            if (!b.has_value())
                return a;
            return a.value() > b.value() ? a : b;
        },
        [](std::optional<D> accumulatorValue) -> std::optional<D> { return accumulatorValue; });
}

template <typename E, typename D>
auto maximum(const function::Function<E, D> &mapper) -> Collector<E, std::optional<D>, std::optional<D>>
{
    return full<E, std::optional<D>, std::optional<D>>(
        []() -> std::optional<D> { return std::nullopt; },
        [mapper](std::optional<D> accumulatorValue, E element, function::Timestamp index) -> std::optional<D> {
            D value = mapper(element);
            if (!accumulatorValue.has_value() || value > accumulatorValue.value())
                return std::optional<D>(value);
            return accumulatorValue;
        },
        [](std::optional<D> a, std::optional<D> b) -> std::optional<D> {
            if (!a.has_value())
                return b;
            if (!b.has_value())
                return a;
            return a.value() > b.value() ? a : b;
        },
        [](std::optional<D> accumulatorValue) -> std::optional<D> { return accumulatorValue; });
}

template <typename E, typename D>
auto variance() -> Collector<E, std::tuple<D, D, D, function::Module>, D>
{
    return full<E, std::tuple<D, D, D, function::Module>, D>(
        []() -> std::tuple<D, D, D, function::Module> {
            return std::make_tuple(D{}, D{}, D{}, function::Module{0});
        },
        [](std::tuple<D, D, D, function::Module> acc, E element, function::Timestamp index) -> std::tuple<D, D, D, function::Module> {
            D value = static_cast<D>(element);
            function::Module count = std::get<3>(acc) + 1;
            D delta = value - std::get<0>(acc);
            D newMean = std::get<0>(acc) + delta / static_cast<D>(count);
            D delta2 = value - newMean;
            D newM2 = std::get<1>(acc) + delta * delta2;
            return std::make_tuple(newMean, newM2, std::get<2>(acc) + value, count);
        },
        [](std::tuple<D, D, D, function::Module> a, std::tuple<D, D, D, function::Module> b) -> std::tuple<D, D, D, function::Module> {
            function::Module countA = std::get<3>(a);
            function::Module countB = std::get<3>(b);
            if (countA == 0)
                return b;
            if (countB == 0)
                return a;
            D delta = std::get<0>(b) - std::get<0>(a);
            D totalCount = static_cast<D>(countA + countB);
            D newMean = std::get<0>(a) + delta * static_cast<D>(countB) / totalCount;
            D newM2 = std::get<1>(a) + std::get<1>(b) + delta * delta * static_cast<D>(countA) * static_cast<D>(countB) / totalCount;
            return std::make_tuple(newMean, newM2, std::get<2>(a) + std::get<2>(b), countA + countB);
        },
        [](std::tuple<D, D, D, function::Module> acc) -> D {
            function::Module count = std::get<3>(acc);
            if (count == 0)
                return D{};
            return std::get<1>(acc) / static_cast<D>(count);
        });
}

template <typename E, typename D>
auto variance(const function::Function<E, D> &mapper) -> Collector<E, std::tuple<D, D, D, function::Module>, D>
{
    return full<E, std::tuple<D, D, D, function::Module>, D>(
        []() -> std::tuple<D, D, D, function::Module> {
            return std::make_tuple(D{}, D{}, D{}, function::Module{0});
        },
        [mapper](std::tuple<D, D, D, function::Module> acc, E element, function::Timestamp index) -> std::tuple<D, D, D, function::Module> {
            D value = mapper(element);
            function::Module count = std::get<3>(acc) + 1;
            D delta = value - std::get<0>(acc);
            D newMean = std::get<0>(acc) + delta / static_cast<D>(count);
            D delta2 = value - newMean;
            D newM2 = std::get<1>(acc) + delta * delta2;
            return std::make_tuple(newMean, newM2, std::get<2>(acc) + value, count);
        },
        [](std::tuple<D, D, D, function::Module> a, std::tuple<D, D, D, function::Module> b) -> std::tuple<D, D, D, function::Module> {
            function::Module countA = std::get<3>(a);
            function::Module countB = std::get<3>(b);
            if (countA == 0)
                return b;
            if (countB == 0)
                return a;
            D delta = std::get<0>(b) - std::get<0>(a);
            D totalCount = static_cast<D>(countA + countB);
            D newMean = std::get<0>(a) + delta * static_cast<D>(countB) / totalCount;
            D newM2 = std::get<1>(a) + std::get<1>(b) + delta * delta * static_cast<D>(countA) * static_cast<D>(countB) / totalCount;
            return std::make_tuple(newMean, newM2, std::get<2>(a) + std::get<2>(b), countA + countB);
        },
        [](std::tuple<D, D, D, function::Module> acc) -> D {
            function::Module count = std::get<3>(acc);
            if (count == 0)
                return D{};
            return std::get<1>(acc) / static_cast<D>(count);
        });
}

template <typename E, typename D>
auto standardDeviation() -> Collector<E, std::tuple<D, D, D, function::Module>, D>
{
    return full<E, std::tuple<D, D, D, function::Module>, D>(
        []() -> std::tuple<D, D, D, function::Module> {
            return std::make_tuple(D{}, D{}, D{}, function::Module{0});
        },
        [](std::tuple<D, D, D, function::Module> acc, E element, function::Timestamp index) -> std::tuple<D, D, D, function::Module> {
            D value = static_cast<D>(element);
            function::Module count = std::get<3>(acc) + 1;
            D delta = value - std::get<0>(acc);
            D newMean = std::get<0>(acc) + delta / static_cast<D>(count);
            D delta2 = value - newMean;
            D newM2 = std::get<1>(acc) + delta * delta2;
            return std::make_tuple(newMean, newM2, std::get<2>(acc) + value, count);
        },
        [](std::tuple<D, D, D, function::Module> a, std::tuple<D, D, D, function::Module> b) -> std::tuple<D, D, D, function::Module> {
            function::Module countA = std::get<3>(a);
            function::Module countB = std::get<3>(b);
            if (countA == 0)
                return b;
            if (countB == 0)
                return a;
            D delta = std::get<0>(b) - std::get<0>(a);
            D totalCount = static_cast<D>(countA + countB);
            D newMean = std::get<0>(a) + delta * static_cast<D>(countB) / totalCount;
            D newM2 = std::get<1>(a) + std::get<1>(b) + delta * delta * static_cast<D>(countA) * static_cast<D>(countB) / totalCount;
            return std::make_tuple(newMean, newM2, std::get<2>(a) + std::get<2>(b), countA + countB);
        },
        [](std::tuple<D, D, D, function::Module> acc) -> D {
            function::Module count = std::get<3>(acc);
            if (count == 0)
                return D{};
            D variance = std::get<1>(acc) / static_cast<D>(count);
            return static_cast<D>(std::sqrt(static_cast<double>(variance)));
        });
}

template <typename E, typename D>
auto standardDeviation(const function::Function<E, D> &mapper) -> Collector<E, std::tuple<D, D, D, function::Module>, D>
{
    return full<E, std::tuple<D, D, D, function::Module>, D>(
        []() -> std::tuple<D, D, D, function::Module> {
            return std::make_tuple(D{}, D{}, D{}, function::Module{0});
        },
        [mapper](std::tuple<D, D, D, function::Module> acc, E element, function::Timestamp index) -> std::tuple<D, D, D, function::Module> {
            D value = mapper(element);
            function::Module count = std::get<3>(acc) + 1;
            D delta = value - std::get<0>(acc);
            D newMean = std::get<0>(acc) + delta / static_cast<D>(count);
            D delta2 = value - newMean;
            D newM2 = std::get<1>(acc) + delta * delta2;
            return std::make_tuple(newMean, newM2, std::get<2>(acc) + value, count);
        },
        [](std::tuple<D, D, D, function::Module> a, std::tuple<D, D, D, function::Module> b) -> std::tuple<D, D, D, function::Module> {
            function::Module countA = std::get<3>(a);
            function::Module countB = std::get<3>(b);
            if (countA == 0)
                return b;
            if (countB == 0)
                return a;
            D delta = std::get<0>(b) - std::get<0>(a);
            D totalCount = static_cast<D>(countA + countB);
            D newMean = std::get<0>(a) + delta * static_cast<D>(countB) / totalCount;
            D newM2 = std::get<1>(a) + std::get<1>(b) + delta * delta * static_cast<D>(countA) * static_cast<D>(countB) / totalCount;
            return std::make_tuple(newMean, newM2, std::get<2>(a) + std::get<2>(b), countA + countB);
        },
        [](std::tuple<D, D, D, function::Module> acc) -> D {
            function::Module count = std::get<3>(acc);
            if (count == 0)
                return D{};
            D variance = std::get<1>(acc) / static_cast<D>(count);
            return static_cast<D>(std::sqrt(static_cast<double>(variance)));
        });
}

template <typename E>
auto findAny() -> Collector<E, std::optional<E>, std::optional<E>>
{
    return shortable<E, std::optional<E>, std::optional<E>>(
        []() -> std::optional<E> { return std::nullopt; },
        [](E element, function::Timestamp index, std::optional<E> accumulatorValue) -> bool { return accumulatorValue.has_value(); },
        [](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
            if (function::randomly())
                return std::optional<E>(element);
            return std::nullopt;
        },
        [](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
            if (a.has_value())
                return a;
            if (b.has_value())
                return b;
            return std::nullopt;
        },
        [](std::optional<E> accumulatorValue) -> std::optional<E> { return accumulatorValue; });
}

template <typename E>
auto findAt(const function::Timestamp &index) -> Collector<E, std::optional<E>, std::optional<E>>
{
    if (index < 0LL)
        throw std::invalid_argument("findAt: index must be non-negative, use findNegativeAt for negative index");
    const function::Timestamp target = index;
    return shortable<E, std::optional<E>, std::optional<E>>(
        []() -> std::optional<E> { return std::nullopt; },
        [](E element, function::Timestamp index, std::optional<E> accumulatorValue) -> bool { return accumulatorValue.has_value(); },
        [target](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
            if (target == index)
                return std::optional<E>(element);
            return accumulatorValue;
        },
        [](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
            if (a.has_value())
                return a;
            if (b.has_value())
                return b;
            return std::nullopt;
        },
        [](std::optional<E> accumulatorValue) -> std::optional<E> { return accumulatorValue; });
}

template <typename E>
auto findNegativeAt(const function::Timestamp &index) -> Collector<E, std::pair<std::vector<E>, function::Module>, std::optional<E>>
{
    if (index > -1LL)
        throw std::invalid_argument("findNegativeAt: index must be negative, use findAt for non-negative index");
    return full<E, std::pair<std::vector<E>, function::Module>, std::optional<E>>(
        []() -> std::pair<std::vector<E>, function::Module> { return std::make_pair(std::vector<E>(), 0); },
        [](std::pair<std::vector<E>, function::Module> accumulatorValue, E element, function::Timestamp index) -> std::pair<std::vector<E>, function::Module> {
            accumulatorValue.first.push_back(element);
            accumulatorValue.second++;
            return accumulatorValue;
        },
        [](std::pair<std::vector<E>, function::Module> a, std::pair<std::vector<E>, function::Module> b) -> std::pair<std::vector<E>, function::Module> {
            a.first.insert(a.first.end(), b.first.begin(), b.first.end());
            a.second += b.second;
            return a;
        },
        [index](std::pair<std::vector<E>, function::Module> accumulatorValue) -> std::optional<E> {
            if (accumulatorValue.second == 0)
                return std::nullopt;
            function::Module size = accumulatorValue.second;
            function::Module absIndex = static_cast<function::Module>(std::abs(index));
            function::Module target = (size - (absIndex % size)) % size;
            if (target < accumulatorValue.first.size())
                return std::optional<E>(accumulatorValue.first[target]);
            return std::nullopt;
        });
}

template <typename E>
auto findFirst() -> Collector<E, std::optional<E>, std::optional<E>>
{
    return shortable<E, std::optional<E>, std::optional<E>>(
        []() -> std::optional<E> { return std::nullopt; },
        [](E element, function::Timestamp index, std::optional<E> accumulatorValue) -> bool { return accumulatorValue.has_value(); },
        [](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
            if (accumulatorValue.has_value())
                return accumulatorValue;
            return std::optional<E>(element);
        },
        [](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
            if (a.has_value())
                return a;
            if (b.has_value())
                return b;
            return std::nullopt;
        },
        [](std::optional<E> accumulatorValue) -> std::optional<E> { return accumulatorValue; });
}

template <typename E>
auto findLast() -> Collector<E, std::vector<E>, std::optional<E>>
{
    return full<E, std::vector<E>, std::optional<E>>(
        []() -> std::vector<E> { return std::vector<E>(); },
        [](std::vector<E> accumulatorValue, E element, function::Timestamp index) -> std::vector<E> {
            accumulatorValue.push_back(element);
            return accumulatorValue;
        },
        [](std::vector<E> a, std::vector<E> b) -> std::vector<E> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<E> accumulatorValue) -> std::optional<E> {
            if (accumulatorValue.empty())
                return std::nullopt;
            return std::optional<E>(accumulatorValue[accumulatorValue.size() - 1]);
        });
}

template <typename E>
auto findMaximum() -> Collector<E, std::optional<E>, std::optional<E>>
{
    return full<E, std::optional<E>, std::optional<E>>(
        []() -> std::optional<E> { return std::nullopt; },
        [](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
            if (accumulatorValue.has_value())
            {
                if (element > accumulatorValue.value())
                    return std::optional<E>(element);
                return accumulatorValue;
            }
            return std::optional<E>(element);
        },
        [](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
            if (a.has_value())
            {
                if (b.has_value())
                    return a.value() > b.value() ? a : b;
                return a;
            }
            if (b.has_value())
                return b;
            return std::nullopt;
        },
        [](std::optional<E> accumulatorValue) -> std::optional<E> { return accumulatorValue; });
}

template <typename E>
auto findMaximum(const function::Comparator<E> &comparator) -> Collector<E, std::optional<E>, std::optional<E>>
{
    return full<E, std::optional<E>, std::optional<E>>(
        []() -> std::optional<E> { return std::nullopt; },
        [comparator](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
            if (accumulatorValue.has_value())
            {
                if (comparator(element, accumulatorValue.value()) > 0)
                    return std::optional<E>(element);
                return accumulatorValue;
            }
            return std::optional<E>(element);
        },
        [comparator](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
            if (a.has_value())
            {
                if (b.has_value())
                    return comparator(a.value(), b.value()) > 0 ? a : b;
                return a;
            }
            if (b.has_value())
                return b;
            return std::nullopt;
        },
        [](std::optional<E> accumulatorValue) -> std::optional<E> { return accumulatorValue; });
}

template <typename E>
auto findMinimum() -> Collector<E, std::optional<E>, std::optional<E>>
{
    return full<E, std::optional<E>, std::optional<E>>(
        []() -> std::optional<E> { return std::nullopt; },
        [](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
            if (accumulatorValue.has_value())
            {
                if (element < accumulatorValue.value())
                    return std::optional<E>(element);
                return accumulatorValue;
            }
            return std::optional<E>(element);
        },
        [](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
            if (a.has_value())
            {
                if (b.has_value())
                    return a.value() < b.value() ? a : b;
                return a;
            }
            if (b.has_value())
                return b;
            return std::nullopt;
        },
        [](std::optional<E> accumulatorValue) -> std::optional<E> { return accumulatorValue; });
}

template <typename E>
auto findMinimum(const function::Comparator<E> &comparator) -> Collector<E, std::optional<E>, std::optional<E>>
{
    return full<E, std::optional<E>, std::optional<E>>(
        []() -> std::optional<E> { return std::nullopt; },
        [comparator](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
            if (accumulatorValue.has_value())
            {
                if (comparator(element, accumulatorValue.value()) < 0)
                    return std::optional<E>(element);
                return accumulatorValue;
            }
            return std::optional<E>(element);
        },
        [comparator](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
            if (a.has_value())
            {
                if (b.has_value())
                    return comparator(a.value(), b.value()) < 0 ? a : b;
                return a;
            }
            if (b.has_value())
                return b;
            return std::nullopt;
        },
        [](std::optional<E> accumulatorValue) -> std::optional<E> { return accumulatorValue; });
}

template <typename E, typename K, typename KeyExtractor>
auto group(KeyExtractor &&keyExtractor) -> Collector<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>>
{
    return full<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>>(
        []() -> std::unordered_map<K, std::vector<E>> { return std::unordered_map<K, std::vector<E>>(); },
        [keyExtractor](std::unordered_map<K, std::vector<E>> accumulatorValue, E element, function::Timestamp index) -> std::unordered_map<K, std::vector<E>> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp>)
            {
                K key = std::invoke(keyExtractor, element, index);
                accumulatorValue[key].push_back(element);
            }
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E>)
            {
                K key = std::invoke(keyExtractor, element);
                accumulatorValue[key].push_back(element);
            }
            return accumulatorValue;
        },
        [](std::unordered_map<K, std::vector<E>> a, std::unordered_map<K, std::vector<E>> b) -> std::unordered_map<K, std::vector<E>> {
            for (auto &[key, vec] : b)
            {
                auto &target = a[key];
                target.reserve(target.size() + vec.size());
                target.insert(target.end(), vec.begin(), vec.end());
            }
            return a;
        },
        [](std::unordered_map<K, std::vector<E>> accumulatorValue) -> std::unordered_map<K, std::vector<E>> { return accumulatorValue; });
}

template <typename E, typename K, typename V, typename KeyExtractor, typename ValueExtractor>
auto groupBy(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor)
    -> Collector<E, std::unordered_map<K, std::vector<V>>, std::unordered_map<K, std::vector<V>>>
{
    return full<E, std::unordered_map<K, std::vector<V>>, std::unordered_map<K, std::vector<V>>>(
        []() -> std::unordered_map<K, std::vector<V>> { return std::unordered_map<K, std::vector<V>>(); },
        [keyExtractor, valueExtractor](std::unordered_map<K, std::vector<V>> accumulatorValue, E element, function::Timestamp index) -> std::unordered_map<K, std::vector<V>> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp>)
            {
                K key = std::invoke(keyExtractor, element, index);
                if constexpr (std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element, index));
                else if constexpr (std::is_invocable_r_v<V, ValueExtractor, E>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element));
            }
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E>)
            {
                K key = std::invoke(keyExtractor, element);
                if constexpr (std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element, index));
                else if constexpr (std::is_invocable_r_v<V, ValueExtractor, E>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element));
            }
            return accumulatorValue;
        },
        [](std::unordered_map<K, std::vector<V>> a, std::unordered_map<K, std::vector<V>> b) -> std::unordered_map<K, std::vector<V>> {
            for (auto &[key, vec] : b)
            {
                auto &target = a[key];
                target.reserve(target.size() + vec.size());
                target.insert(target.end(), vec.begin(), vec.end());
            }
            return a;
        },
        [](std::unordered_map<K, std::vector<V>> accumulatorValue) -> std::unordered_map<K, std::vector<V>> { return accumulatorValue; });
}

template <typename E>
auto join() -> Collector<E, ::bunch::Builder, ::bunch::Bunch>
{
    static const ::bunch::Bunch comma(",");
    static const ::bunch::Bunch leftBracket("[");
    static const ::bunch::Bunch rightBracket("]");
    return full<E, ::bunch::Builder, ::bunch::Bunch>(
        []() -> ::bunch::Builder { return ::bunch::Builder(); },
        [](::bunch::Builder accumulatorValue, E element, function::Timestamp index) -> ::bunch::Builder {
            if constexpr (std::is_same_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<::bunch::Bunch>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [](::bunch::Builder a, ::bunch::Builder b) -> ::bunch::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(comma);
            a.append(b.toBunch());
            return a;
        },
        [](::bunch::Builder accumulatorValue) -> ::bunch::Bunch {
            ::bunch::Builder result;
            result.append(leftBracket);
            result.append(accumulatorValue.toBunch());
            result.append(rightBracket);
            return result.toBunch();
        });
}

template <typename E>
auto join(const ::bunch::Bunch &delimiter) -> Collector<E, ::bunch::Builder, ::bunch::Bunch>
{
    static const ::bunch::Bunch leftBracket("[");
    static const ::bunch::Bunch rightBracket("]");
    return full<E, ::bunch::Builder, ::bunch::Bunch>(
        []() -> ::bunch::Builder { return ::bunch::Builder(); },
        [delimiter](::bunch::Builder accumulatorValue, E element, function::Timestamp index) -> ::bunch::Builder {
            if constexpr (std::is_same_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<::bunch::Bunch>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [delimiter](::bunch::Builder a, ::bunch::Builder b) -> ::bunch::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(delimiter);
            a.append(b.toBunch());
            return a;
        },
        [](::bunch::Builder accumulatorValue) -> ::bunch::Bunch {
            ::bunch::Builder result;
            result.append(leftBracket);
            result.append(accumulatorValue.toBunch());
            result.append(rightBracket);
            return result.toBunch();
        });
}

template <typename E>
auto join(const ::bunch::Bunch &prefix, const ::bunch::Bunch &delimiter, const ::bunch::Bunch &suffix) -> Collector<E, ::bunch::Builder, ::bunch::Bunch>
{
    return full<E, ::bunch::Builder, ::bunch::Bunch>(
        []() -> ::bunch::Builder { return ::bunch::Builder(); },
        [delimiter](::bunch::Builder accumulatorValue, E element, function::Timestamp index) -> ::bunch::Builder {
            if constexpr (std::is_same_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<::bunch::Bunch>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [delimiter](::bunch::Builder a, ::bunch::Builder b) -> ::bunch::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(delimiter);
            a.append(b.toBunch());
            return a;
        },
        [prefix, suffix](::bunch::Builder accumulatorValue) -> ::bunch::Bunch {
            ::bunch::Builder result;
            result.append(prefix);
            result.append(accumulatorValue.toBunch());
            result.append(suffix);
            return result.toBunch();
        });
}

template <typename E, typename Converter>
auto join(const ::bunch::Bunch &prefix, Converter &&converter, const ::bunch::Bunch &suffix) -> Collector<E, ::bunch::Builder, ::bunch::Bunch>
{
    static const ::bunch::Bunch comma(",");
    return full<E, ::bunch::Builder, ::bunch::Bunch>(
        []() -> ::bunch::Builder { return ::bunch::Builder(); },
        [converter = std::forward<Converter>(converter)](::bunch::Builder accumulatorValue, E element, function::Timestamp index) -> ::bunch::Builder {
            auto converted = converter(element);
            if constexpr (std::is_same_v<decltype(converted), ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            else if constexpr (std::is_same_v<decltype(converted), std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            else if constexpr (std::is_convertible_v<decltype(converted), ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<::bunch::Bunch>(converted));
            }
            else if constexpr (std::is_convertible_v<decltype(converted), std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<std::string>(converted));
            }
            else if constexpr (std::is_arithmetic_v<decltype(converted)>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            return accumulatorValue;
        },
        [](::bunch::Builder a, ::bunch::Builder b) -> ::bunch::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(comma);
            a.append(b.toBunch());
            return a;
        },
        [prefix, suffix](::bunch::Builder accumulatorValue) -> ::bunch::Bunch {
            ::bunch::Builder result;
            result.append(prefix);
            result.append(accumulatorValue.toBunch());
            result.append(suffix);
            return result.toBunch();
        });
}

template <typename E>
auto out() -> Collector<E, ::bunch::Builder, ::bunch::Bunch>
{
    static const ::bunch::Bunch comma(",");
    static const ::bunch::Bunch leftBracket("[");
    static const ::bunch::Bunch rightBracket("]");
    return full<E, ::bunch::Builder, ::bunch::Bunch>(
        []() -> ::bunch::Builder { return ::bunch::Builder(); },
        [](::bunch::Builder accumulatorValue, E element, function::Timestamp index) -> ::bunch::Builder {
            if constexpr (std::is_same_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<::bunch::Bunch>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [](::bunch::Builder a, ::bunch::Builder b) -> ::bunch::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(comma);
            a.append(b.toBunch());
            return a;
        },
        [](::bunch::Builder accumulatorValue) -> ::bunch::Bunch {
            ::bunch::Builder result;
            result.append(leftBracket);
            result.append(accumulatorValue.toBunch());
            result.append(rightBracket);
            ::bunch::Bunch finalResult = result.toBunch();
            std::cout << finalResult << '\n';
            return finalResult;
        });
}

template <typename E>
auto out(const ::bunch::Bunch &delimiter) -> Collector<E, ::bunch::Builder, ::bunch::Bunch>
{
    static const ::bunch::Bunch leftBracket("[");
    static const ::bunch::Bunch rightBracket("]");
    return full<E, ::bunch::Builder, ::bunch::Bunch>(
        []() -> ::bunch::Builder { return ::bunch::Builder(); },
        [delimiter](::bunch::Builder accumulatorValue, E element, function::Timestamp index) -> ::bunch::Builder {
            if constexpr (std::is_same_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<::bunch::Bunch>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [delimiter](::bunch::Builder a, ::bunch::Builder b) -> ::bunch::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(delimiter);
            a.append(b.toBunch());
            return a;
        },
        [](::bunch::Builder accumulatorValue) -> ::bunch::Bunch {
            ::bunch::Builder result;
            result.append(leftBracket);
            result.append(accumulatorValue.toBunch());
            result.append(rightBracket);
            ::bunch::Bunch finalResult = result.toBunch();
            std::cout << finalResult << '\n';
            return finalResult;
        });
}

template <typename E>
auto out(const ::bunch::Bunch &prefix, const ::bunch::Bunch &delimiter, const ::bunch::Bunch &suffix) -> Collector<E, ::bunch::Builder, ::bunch::Bunch>
{
    return full<E, ::bunch::Builder, ::bunch::Bunch>(
        []() -> ::bunch::Builder { return ::bunch::Builder(); },
        [delimiter](::bunch::Builder accumulatorValue, E element, function::Timestamp index) -> ::bunch::Builder {
            if constexpr (std::is_same_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<::bunch::Bunch>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [delimiter](::bunch::Builder a, ::bunch::Builder b) -> ::bunch::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(delimiter);
            a.append(b.toBunch());
            return a;
        },
        [prefix, suffix](::bunch::Builder accumulatorValue) -> ::bunch::Bunch {
            ::bunch::Builder result;
            result.append(prefix);
            result.append(accumulatorValue.toBunch());
            result.append(suffix);
            ::bunch::Bunch finalResult = result.toBunch();
            std::cout << finalResult << '\n';
            return finalResult;
        });
}

template <typename E, typename Converter>
auto out(const ::bunch::Bunch &prefix, Converter &&converter, const ::bunch::Bunch &suffix) -> Collector<E, ::bunch::Builder, ::bunch::Bunch>
{
    static const ::bunch::Bunch comma(",");
    return full<E, ::bunch::Builder, ::bunch::Bunch>(
        []() -> ::bunch::Builder { return ::bunch::Builder(); },
        [converter = std::forward<Converter>(converter)](::bunch::Builder accumulatorValue, E element, function::Timestamp index) -> ::bunch::Builder {
            auto converted = converter(element);
            if constexpr (std::is_same_v<decltype(converted), ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            else if constexpr (std::is_same_v<decltype(converted), std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            else if constexpr (std::is_convertible_v<decltype(converted), ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<::bunch::Bunch>(converted));
            }
            else if constexpr (std::is_convertible_v<decltype(converted), std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<std::string>(converted));
            }
            else if constexpr (std::is_arithmetic_v<decltype(converted)>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            return accumulatorValue;
        },
        [](::bunch::Builder a, ::bunch::Builder b) -> ::bunch::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(comma);
            a.append(b.toBunch());
            return a;
        },
        [prefix, suffix](::bunch::Builder accumulatorValue) -> ::bunch::Bunch {
            ::bunch::Builder result;
            result.append(prefix);
            result.append(accumulatorValue.toBunch());
            result.append(suffix);
            ::bunch::Bunch finalResult = result.toBunch();
            std::cout << finalResult << '\n';
            return finalResult;
        });
}

template <typename E>
auto error() -> Collector<E, ::bunch::Builder, ::bunch::Bunch>
{
    static const ::bunch::Bunch comma(",");
    static const ::bunch::Bunch leftBracket("[");
    static const ::bunch::Bunch rightBracket("]");
    return full<E, ::bunch::Builder, ::bunch::Bunch>(
        []() -> ::bunch::Builder { return ::bunch::Builder(); },
        [](::bunch::Builder accumulatorValue, E element, function::Timestamp index) -> ::bunch::Builder {
            if constexpr (std::is_same_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<::bunch::Bunch>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [](::bunch::Builder a, ::bunch::Builder b) -> ::bunch::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(comma);
            a.append(b.toBunch());
            return a;
        },
        [](::bunch::Builder accumulatorValue) -> ::bunch::Bunch {
            ::bunch::Builder result;
            result.append(leftBracket);
            result.append(accumulatorValue.toBunch());
            result.append(rightBracket);
            ::bunch::Bunch finalResult = result.toBunch();
            std::cerr << finalResult << '\n';
            return finalResult;
        });
}

template <typename E>
auto error(const ::bunch::Bunch &delimiter) -> Collector<E, ::bunch::Builder, ::bunch::Bunch>
{
    static const ::bunch::Bunch leftBracket("[");
    static const ::bunch::Bunch rightBracket("]");
    return full<E, ::bunch::Builder, ::bunch::Bunch>(
        []() -> ::bunch::Builder { return ::bunch::Builder(); },
        [delimiter](::bunch::Builder accumulatorValue, E element, function::Timestamp index) -> ::bunch::Builder {
            if constexpr (std::is_same_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<::bunch::Bunch>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [delimiter](::bunch::Builder a, ::bunch::Builder b) -> ::bunch::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(delimiter);
            a.append(b.toBunch());
            return a;
        },
        [](::bunch::Builder accumulatorValue) -> ::bunch::Bunch {
            ::bunch::Builder result;
            result.append(leftBracket);
            result.append(accumulatorValue.toBunch());
            result.append(rightBracket);
            ::bunch::Bunch finalResult = result.toBunch();
            std::cerr << finalResult << '\n';
            return finalResult;
        });
}

template <typename E>
auto error(const ::bunch::Bunch &prefix, const ::bunch::Bunch &delimiter, const ::bunch::Bunch &suffix) -> Collector<E, ::bunch::Builder, ::bunch::Bunch>
{
    return full<E, ::bunch::Builder, ::bunch::Bunch>(
        []() -> ::bunch::Builder { return ::bunch::Builder(); },
        [delimiter](::bunch::Builder accumulatorValue, E element, function::Timestamp index) -> ::bunch::Builder {
            if constexpr (std::is_same_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<::bunch::Bunch>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [delimiter](::bunch::Builder a, ::bunch::Builder b) -> ::bunch::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(delimiter);
            a.append(b.toBunch());
            return a;
        },
        [prefix, suffix](::bunch::Builder accumulatorValue) -> ::bunch::Bunch {
            ::bunch::Builder result;
            result.append(prefix);
            result.append(accumulatorValue.toBunch());
            result.append(suffix);
            ::bunch::Bunch finalResult = result.toBunch();
            std::cerr << finalResult << '\n';
            return finalResult;
        });
}

template <typename E, typename Converter>
auto error(const ::bunch::Bunch &prefix, Converter &&converter, const ::bunch::Bunch &suffix) -> Collector<E, ::bunch::Builder, ::bunch::Bunch>
{
    static const ::bunch::Bunch comma(",");
    return full<E, ::bunch::Builder, ::bunch::Bunch>(
        []() -> ::bunch::Builder { return ::bunch::Builder(); },
        [converter = std::forward<Converter>(converter)](::bunch::Builder accumulatorValue, E element, function::Timestamp index) -> ::bunch::Builder {
            auto converted = converter(element);
            if constexpr (std::is_same_v<decltype(converted), ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            else if constexpr (std::is_same_v<decltype(converted), std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            else if constexpr (std::is_convertible_v<decltype(converted), ::bunch::Bunch>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<::bunch::Bunch>(converted));
            }
            else if constexpr (std::is_convertible_v<decltype(converted), std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<std::string>(converted));
            }
            else if constexpr (std::is_arithmetic_v<decltype(converted)>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            return accumulatorValue;
        },
        [](::bunch::Builder a, ::bunch::Builder b) -> ::bunch::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(comma);
            a.append(b.toBunch());
            return a;
        },
        [prefix, suffix](::bunch::Builder accumulatorValue) -> ::bunch::Bunch {
            ::bunch::Builder result;
            result.append(prefix);
            result.append(accumulatorValue.toBunch());
            result.append(suffix);
            ::bunch::Bunch finalResult = result.toBunch();
            std::cerr << finalResult << '\n';
            return finalResult;
        });
}

template <typename E>
auto frequency() -> Collector<E, std::pair<std::unordered_map<E, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>, function::Timestamp>, std::map<E, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>>
{
    using InnerMap = std::unordered_map<E, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>;
    using AccumulatorType = std::pair<InnerMap, function::Timestamp>;
    using ResultMap = std::map<E, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>;
    return full<E, AccumulatorType, ResultMap>(
        []() -> AccumulatorType {
            return AccumulatorType(InnerMap(), 0LL);
        },
        [](AccumulatorType accumulatorValue, E element, function::Timestamp index) -> AccumulatorType {
            InnerMap &map = accumulatorValue.first;
            std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>> &pair = map[element];
            pair.first.push_back(std::complex<double>(static_cast<double>(index), 1.0));
            accumulatorValue.second = index;
            return accumulatorValue;
        },
        [](AccumulatorType a, AccumulatorType b) -> AccumulatorType {
            InnerMap &mapA = a.first;
            const InnerMap &mapB = b.first;
            for (auto &entry : mapB)
            {
                std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>> &target = mapA[entry.first];
                target.first.insert(target.first.end(), entry.second.first.begin(), entry.second.first.end());
            }
            if (b.second > a.second)
            {
                a.second = b.second;
            }
            return a;
        },
        [](AccumulatorType accumulatorValue) -> ResultMap {
            ResultMap result;
            InnerMap &map = accumulatorValue.first;
            function::Timestamp totalLength = accumulatorValue.second + 1;
            for (auto &entry : map)
            {
                std::size_t totalCount = entry.second.first.size();
                std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>> &target = result[entry.first];
                target.first = std::move(entry.second.first);
                target.second.reserve(totalCount);
                for (std::size_t i = 0; i < totalCount; i++)
                {
                    function::Timestamp position = static_cast<function::Timestamp>(target.first[i].real());
                    target.second.push_back(std::complex<double>(static_cast<double>(position), static_cast<double>(totalLength)));
                }
            }
            return result;
        });
}

template <typename E, typename D>
auto frequency(const function::Function<E, D> &mapper) -> Collector<E, std::pair<std::unordered_map<D, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>, function::Timestamp>, std::map<D, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>>
{
    using InnerMap = std::unordered_map<D, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>;
    using AccumulatorType = std::pair<InnerMap, function::Timestamp>;
    using ResultMap = std::map<D, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>;
    return full<E, AccumulatorType, ResultMap>(
        []() -> AccumulatorType {
            return AccumulatorType(InnerMap(), 0LL);
        },
        [mapper](AccumulatorType accumulatorValue, E element, function::Timestamp index) -> AccumulatorType {
            InnerMap &map = accumulatorValue.first;
            D key = mapper(element);
            std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>> &pair = map[key];
            pair.first.push_back(std::complex<double>(static_cast<double>(index), 1.0));
            accumulatorValue.second = index;
            return accumulatorValue;
        },
        [](AccumulatorType a, AccumulatorType b) -> AccumulatorType {
            InnerMap &mapA = a.first;
            const InnerMap &mapB = b.first;
            for (auto &entry : mapB)
            {
                std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>> &target = mapA[entry.first];
                target.first.insert(target.first.end(), entry.second.first.begin(), entry.second.first.end());
            }
            if (b.second > a.second)
            {
                a.second = b.second;
            }
            return a;
        },
        [](AccumulatorType accumulatorValue) -> ResultMap {
            ResultMap result;
            InnerMap &map = accumulatorValue.first;
            function::Timestamp totalLength = accumulatorValue.second + 1;
            for (auto &entry : map)
            {
                std::size_t totalCount = entry.second.first.size();
                std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>> &target = result[entry.first];
                target.first = std::move(entry.second.first);
                target.second.reserve(totalCount);
                for (std::size_t i = 0; i < totalCount; i++)
                {
                    function::Timestamp position = static_cast<function::Timestamp>(target.first[i].real());
                    target.second.push_back(std::complex<double>(static_cast<double>(position), static_cast<double>(totalLength)));
                }
            }
            return result;
        });
}

template <typename E>
auto distribution() -> Collector<E, std::unordered_map<E, std::vector<function::Timestamp>>, std::map<E, std::complex<double>>>
{
    using AccumulatorMap = std::unordered_map<E, std::vector<function::Timestamp>>;
    using ResultMap = std::map<E, std::complex<double>>;
    return full<E, AccumulatorMap, ResultMap>(
        []() -> AccumulatorMap {
            return AccumulatorMap();
        },
        [](AccumulatorMap accumulatorValue, E element, function::Timestamp index) -> AccumulatorMap {
            accumulatorValue[element].push_back(index);
            return accumulatorValue;
        },
        [](AccumulatorMap a, AccumulatorMap b) -> AccumulatorMap {
            for (auto &entry : b)
            {
                std::vector<function::Timestamp> &target = a[entry.first];
                target.insert(target.end(), entry.second.begin(), entry.second.end());
            }
            return a;
        },
        [](AccumulatorMap accumulatorValue) -> ResultMap {
            ResultMap result;
            if (accumulatorValue.empty())
                return result;
            std::vector<E> elements;
            std::vector<double> positionSums;
            std::vector<double> counts;
            for (auto &entry : accumulatorValue)
            {
                elements.push_back(entry.first);
                double positionSum = 0.0;
                double count = static_cast<double>(entry.second.size());
                for (function::Timestamp index : entry.second)
                {
                    positionSum += static_cast<double>(index);
                }
                positionSums.push_back(positionSum);
                counts.push_back(count);
            }
            double modePositionSum = 0.0;
            double modeCount = 0.0;
            {
                std::map<double, size_t> positionFreq;
                for (double val : positionSums)
                    positionFreq[val]++;
                size_t maxFreq = 0;
                for (auto &p : positionFreq)
                {
                    if (p.second > maxFreq)
                    {
                        maxFreq = p.second;
                        modePositionSum = p.first;
                    }
                }
            }
            {
                std::map<double, size_t> countFreq;
                for (double val : counts)
                    countFreq[val]++;
                size_t maxFreq = 0;
                for (auto &p : countFreq)
                {
                    if (p.second > maxFreq)
                    {
                        maxFreq = p.second;
                        modeCount = p.first;
                    }
                }
            }
            double positionStddev = 0.0;
            double countStddev = 0.0;
            for (size_t i = 0; i < elements.size(); i++)
            {
                double posDiff = positionSums[i] - modePositionSum;
                double cntDiff = counts[i] - modeCount;
                positionStddev += posDiff * posDiff;
                countStddev += cntDiff * cntDiff;
            }
            positionStddev = std::sqrt(positionStddev / static_cast<double>(elements.size()));
            countStddev = std::sqrt(countStddev / static_cast<double>(elements.size()));
            if (positionStddev < 0.001)
                positionStddev = 1.0;
            if (countStddev < 0.001)
                countStddev = 1.0;
            for (size_t i = 0; i < elements.size(); i++)
            {
                double posScore = (positionSums[i] - modePositionSum) / positionStddev;
                double cntScore = (counts[i] - modeCount) / countStddev;
                result[elements[i]] = std::complex<double>(posScore, cntScore);
            }
            return result;
        });
}

template <typename E, typename D>
auto distribution(const function::Function<E, D> &mapper) -> Collector<E, std::unordered_map<D, std::vector<function::Timestamp>>, std::map<D, std::complex<double>>>
{
    using AccumulatorMap = std::unordered_map<D, std::vector<function::Timestamp>>;
    using ResultMap = std::map<D, std::complex<double>>;
    return full<E, AccumulatorMap, ResultMap>(
        []() -> AccumulatorMap {
            return AccumulatorMap();
        },
        [mapper](AccumulatorMap accumulatorValue, E element, function::Timestamp index) -> AccumulatorMap {
            accumulatorValue[mapper(element)].push_back(index);
            return accumulatorValue;
        },
        [](AccumulatorMap a, AccumulatorMap b) -> AccumulatorMap {
            for (auto &entry : b)
            {
                std::vector<function::Timestamp> &target = a[entry.first];
                target.insert(target.end(), entry.second.begin(), entry.second.end());
            }
            return a;
        },
        [](AccumulatorMap accumulatorValue) -> ResultMap {
            ResultMap result;
            if (accumulatorValue.empty())
                return result;
            std::vector<D> elements;
            std::vector<double> positionSums;
            std::vector<double> counts;
            for (auto &entry : accumulatorValue)
            {
                elements.push_back(entry.first);
                double positionSum = 0.0;
                double count = static_cast<double>(entry.second.size());
                for (function::Timestamp index : entry.second)
                {
                    positionSum += static_cast<double>(index);
                }
                positionSums.push_back(positionSum);
                counts.push_back(count);
            }
            double modePositionSum = 0.0;
            double modeCount = 0.0;
            {
                std::map<double, size_t> positionFreq;
                for (double val : positionSums)
                    positionFreq[val]++;
                size_t maxFreq = 0;
                for (auto &p : positionFreq)
                {
                    if (p.second > maxFreq)
                    {
                        maxFreq = p.second;
                        modePositionSum = p.first;
                    }
                }
            }
            {
                std::map<double, size_t> countFreq;
                for (double val : counts)
                    countFreq[val]++;
                size_t maxFreq = 0;
                for (auto &p : countFreq)
                {
                    if (p.second > maxFreq)
                    {
                        maxFreq = p.second;
                        modeCount = p.first;
                    }
                }
            }
            double positionStddev = 0.0;
            double countStddev = 0.0;
            for (size_t i = 0; i < elements.size(); i++)
            {
                double posDiff = positionSums[i] - modePositionSum;
                double cntDiff = counts[i] - modeCount;
                positionStddev += posDiff * posDiff;
                countStddev += cntDiff * cntDiff;
            }
            positionStddev = std::sqrt(positionStddev / static_cast<double>(elements.size()));
            countStddev = std::sqrt(countStddev / static_cast<double>(elements.size()));
            if (positionStddev < 0.001)
                positionStddev = 1.0;
            if (countStddev < 0.001)
                countStddev = 1.0;
            for (size_t i = 0; i < elements.size(); i++)
            {
                double posScore = (positionSums[i] - modePositionSum) / positionStddev;
                double cntScore = (counts[i] - modeCount) / countStddev;
                result[elements[i]] = std::complex<double>(posScore, cntScore);
            }
            return result;
        });
}

template <typename E>
auto partition(const function::Module &size) -> Collector<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>>
{
    return full<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>>(
        []() -> std::vector<std::vector<E>> { return std::vector<std::vector<E>>(); },
        [size](std::vector<std::vector<E>> accumulatorValue, E element, function::Timestamp index) -> std::vector<std::vector<E>> {
            if (size <= 1)
            {
                accumulatorValue.push_back({element});
                return accumulatorValue;
            }
            if (accumulatorValue.empty() || accumulatorValue.back().size() >= size)
                accumulatorValue.push_back({element});
            else
                accumulatorValue.back().push_back(element);
            return accumulatorValue;
        },
        [size](std::vector<std::vector<E>> a, std::vector<std::vector<E>> b) -> std::vector<std::vector<E>> {
            if (a.empty())
                return b;
            if (b.empty())
                return a;
            std::vector<E> &lastA = a.back();
            std::vector<E> &firstB = b.front();
            if (lastA.size() < size)
            {
                function::Module canTake = size - lastA.size();
                function::Module takeCount = std::min(canTake, static_cast<function::Module>(firstB.size()));
                lastA.insert(lastA.end(), firstB.begin(), firstB.begin() + takeCount);
                if (takeCount == firstB.size())
                    b.erase(b.begin());
                else
                    firstB.erase(firstB.begin(), firstB.begin() + takeCount);
            }
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<std::vector<E>> accumulatorValue) -> std::vector<std::vector<E>> { return accumulatorValue; });
}

template <typename E, typename KeyExtractor>
auto partitionBy(KeyExtractor &&keyExtractor) -> Collector<E, std::map<function::Timestamp, std::vector<E>>, std::vector<std::vector<E>>>
{
    return full<E, std::map<function::Timestamp, std::vector<E>>, std::vector<std::vector<E>>>(
        []() -> std::map<function::Timestamp, std::vector<E>> { return std::map<function::Timestamp, std::vector<E>>(); },
        [keyExtractor](std::map<function::Timestamp, std::vector<E>> accumulatorValue, E element, function::Timestamp index) -> std::map<function::Timestamp, std::vector<E>> {
            if constexpr (std::is_invocable_r_v<function::Timestamp, KeyExtractor, E, function::Timestamp>)
                accumulatorValue[std::invoke(keyExtractor, element, index)].push_back(element);
            else if constexpr (std::is_invocable_r_v<function::Timestamp, KeyExtractor, E>)
                accumulatorValue[std::invoke(keyExtractor, element)].push_back(element);
            return accumulatorValue;
        },
        [](std::map<function::Timestamp, std::vector<E>> a, std::map<function::Timestamp, std::vector<E>> b) -> std::map<function::Timestamp, std::vector<E>> {
            for (auto &[key, vec] : b)
                for (E &element : vec)
                    a[key].push_back(std::move(element));
            return a;
        },
        [](std::map<function::Timestamp, std::vector<E>> accumulatorValue) -> std::vector<std::vector<E>> {
            std::vector<std::vector<E>> result;
            result.reserve(accumulatorValue.size());
            for (auto &[key, vec] : accumulatorValue)
                result.push_back(std::move(vec));
            return result;
        });
}

template <typename E, typename V, typename KeyExtractor, typename ValueExtractor>
auto partitionBy(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor)
    -> Collector<E, std::map<function::Timestamp, std::vector<V>>, std::vector<std::vector<V>>>
{
    return full<E, std::map<function::Timestamp, std::vector<V>>, std::vector<std::vector<V>>>(
        []() -> std::map<function::Timestamp, std::vector<V>> { return std::map<function::Timestamp, std::vector<V>>(); },
        [keyExtractor, valueExtractor](std::map<function::Timestamp, std::vector<V>> accumulatorValue, E element, function::Timestamp index) -> std::map<function::Timestamp, std::vector<V>> {
            if constexpr (std::is_invocable_r_v<function::Timestamp, KeyExtractor, E, function::Timestamp>)
            {
                function::Timestamp key = std::invoke(keyExtractor, element, index);
                if constexpr (std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element, index));
                else if constexpr (std::is_invocable_r_v<V, ValueExtractor, E>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element));
            }
            else if constexpr (std::is_invocable_r_v<function::Timestamp, KeyExtractor, E>)
            {
                function::Timestamp key = std::invoke(keyExtractor, element);
                if constexpr (std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element, index));
                else if constexpr (std::is_invocable_r_v<V, ValueExtractor, E>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element));
            }
            return accumulatorValue;
        },
        [](std::map<function::Timestamp, std::vector<V>> a, std::map<function::Timestamp, std::vector<V>> b) -> std::map<function::Timestamp, std::vector<V>> {
            for (auto &[key, vec] : b)
                for (V &element : vec)
                    a[key].push_back(std::move(element));
            return a;
        },
        [](std::map<function::Timestamp, std::vector<V>> accumulatorValue) -> std::vector<std::vector<V>> {
            std::vector<std::vector<V>> result;
            result.reserve(accumulatorValue.size());
            for (auto &[key, vec] : accumulatorValue)
                result.push_back(std::move(vec));
            return result;
        });
}

template <typename E, typename D>
auto median() -> Collector<E, std::vector<D>, std::optional<D>>
{
    return full<E, std::vector<D>, std::optional<D>>(
        []() -> std::vector<D> { return std::vector<D>(); },
        [](std::vector<D> accumulatorValue, E element, function::Timestamp index) -> std::vector<D> {
            accumulatorValue.push_back(static_cast<D>(element));
            return accumulatorValue;
        },
        [](std::vector<D> a, std::vector<D> b) -> std::vector<D> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<D> accumulatorValue) -> std::optional<D> {
            if (accumulatorValue.empty())
                return std::nullopt;
            std::sort(accumulatorValue.begin(), accumulatorValue.end());
            if (accumulatorValue.size() % 2 == 1)
                return std::optional<D>(accumulatorValue[accumulatorValue.size() / 2]);
            return std::optional<D>((accumulatorValue[accumulatorValue.size() / 2 - 1] + accumulatorValue[accumulatorValue.size() / 2]) / static_cast<D>(2));
        });
}

template <typename E, typename D>
auto median(const function::Function<E, D> &mapper) -> Collector<E, std::vector<D>, std::optional<D>>
{
    return full<E, std::vector<D>, std::optional<D>>(
        []() -> std::vector<D> { return std::vector<D>(); },
        [mapper](std::vector<D> accumulatorValue, E element, function::Timestamp index) -> std::vector<D> {
            accumulatorValue.push_back(mapper(element));
            return accumulatorValue;
        },
        [](std::vector<D> a, std::vector<D> b) -> std::vector<D> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<D> accumulatorValue) -> std::optional<D> {
            if (accumulatorValue.empty())
                return std::nullopt;
            std::sort(accumulatorValue.begin(), accumulatorValue.end());
            if (accumulatorValue.size() % 2 == 1)
                return std::optional<D>(accumulatorValue[accumulatorValue.size() / 2]);
            return std::optional<D>((accumulatorValue[accumulatorValue.size() / 2 - 1] + accumulatorValue[accumulatorValue.size() / 2]) / static_cast<D>(2));
        });
}

template <typename E>
auto mode() -> Collector<E, std::unordered_map<E, std::complex<double>>, std::optional<E>>
{
    return full<E, std::unordered_map<E, std::complex<double>>, std::optional<E>>(
        []() -> std::unordered_map<E, std::complex<double>> { return std::unordered_map<E, std::complex<double>>(); },
        [](std::unordered_map<E, std::complex<double>> accumulatorValue, E element, function::Timestamp index) -> std::unordered_map<E, std::complex<double>> {
            double angle = std::fmod(2.0 * pi * static_cast<double>(index), 2.0 * pi);
            accumulatorValue[element] += std::complex<double>(std::cos(angle), std::sin(angle));
            return accumulatorValue;
        },
        [](std::unordered_map<E, std::complex<double>> a, std::unordered_map<E, std::complex<double>> b) -> std::unordered_map<E, std::complex<double>> {
            for (auto &[key, value] : b)
                a[key] += value;
            return a;
        },
        [](std::unordered_map<E, std::complex<double>> accumulatorValue) -> std::optional<E> {
            if (accumulatorValue.empty())
                return std::nullopt;
            auto modeIter = std::max_element(accumulatorValue.begin(), accumulatorValue.end(),
                                             [](const auto &a, const auto &b) {
                                                 return std::abs(a.second) < std::abs(b.second);
                                             });
            if (std::abs(modeIter->second) == 0.0)
                return std::nullopt;
            return std::optional<E>(modeIter->first);
        });
}

template <typename E, typename D>
auto percentile(double p) -> Collector<E, std::vector<D>, std::optional<D>>
{
    if (p < 0.0 || p > 100.0)
        throw std::invalid_argument("percentile: p must be in range [0.0, 100.0]");
    return full<E, std::vector<D>, std::optional<D>>(
        []() -> std::vector<D> { return std::vector<D>(); },
        [](std::vector<D> acc, E element, function::Timestamp index) -> std::vector<D> {
            acc.push_back(static_cast<D>(element));
            return acc;
        },
        [](std::vector<D> a, std::vector<D> b) -> std::vector<D> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [p](std::vector<D> acc) -> std::optional<D> {
            if (acc.empty())
                return std::nullopt;
            std::sort(acc.begin(), acc.end());
            double rank = p / 100.0 * static_cast<double>(acc.size() - 1);
            std::size_t lo = static_cast<std::size_t>(std::floor(rank));
            std::size_t hi = static_cast<std::size_t>(std::ceil(rank));
            if (lo == hi)
                return std::optional<D>(acc[lo]);
            double frac = rank - static_cast<double>(lo);
            return std::optional<D>(acc[lo] + static_cast<D>(frac * static_cast<double>(acc[hi] - acc[lo])));
        });
}

template <typename E, typename D>
auto percentile(double p, const function::Function<E, D> &mapper) -> Collector<E, std::vector<D>, std::optional<D>>
{
    if (p < 0.0 || p > 100.0)
        throw std::invalid_argument("percentile: p must be in range [0.0, 100.0]");
    return full<E, std::vector<D>, std::optional<D>>(
        []() -> std::vector<D> { return std::vector<D>(); },
        [mapper](std::vector<D> acc, E element, function::Timestamp index) -> std::vector<D> {
            acc.push_back(mapper(element));
            return acc;
        },
        [](std::vector<D> a, std::vector<D> b) -> std::vector<D> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [p](std::vector<D> acc) -> std::optional<D> {
            if (acc.empty())
                return std::nullopt;
            std::sort(acc.begin(), acc.end());
            double rank = p / 100.0 * static_cast<double>(acc.size() - 1);
            std::size_t lo = static_cast<std::size_t>(std::floor(rank));
            std::size_t hi = static_cast<std::size_t>(std::ceil(rank));
            if (lo == hi)
                return std::optional<D>(acc[lo]);
            double frac = rank - static_cast<double>(lo);
            return std::optional<D>(acc[lo] + static_cast<D>(frac * static_cast<double>(acc[hi] - acc[lo])));
        });
}

template <typename E>
auto reduce(const function::BiFunction<E, E, E> &reducer) -> Collector<E, std::optional<E>, std::optional<E>>
{
    return full<E, std::optional<E>, std::optional<E>>(
        []() -> std::optional<E> { return std::nullopt; },
        [reducer](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
            if (!accumulatorValue.has_value())
                return std::optional<E>(element);
            return std::optional<E>(reducer(accumulatorValue.value(), element));
        },
        [reducer](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
            if (!a.has_value())
                return b;
            if (!b.has_value())
                return a;
            return std::optional<E>(reducer(a.value(), b.value()));
        },
        [](std::optional<E> accumulatorValue) -> std::optional<E> { return accumulatorValue; });
}

template <typename E>
auto reduce(const E &identity, const function::BiFunction<E, E, E> &reducer) -> Collector<E, E, E>
{
    return full<E, E, E>(
        [identity]() -> E { return identity; },
        [reducer](E accumulatorValue, E element, function::Timestamp index) -> E { return reducer(accumulatorValue, element); },
        [reducer](E a, E b) -> E { return reducer(a, b); },
        [](E accumulatorValue) -> E { return accumulatorValue; });
}

template <typename E, typename R>
auto reduce(const R &identity, const function::BiFunction<R, E, R> &reducer, const function::BiFunction<R, R, R> &combiner, const function::Function<R, R> &finisher) -> Collector<E, R, R>
{
    return full<E, R, R>(
        [identity]() -> R { return identity; },
        [reducer](R accumulatorValue, E element, function::Timestamp index) -> R { return reducer(accumulatorValue, element); },
        [combiner](R a, R b) -> R { return combiner(a, b); },
        [finisher](R accumulatorValue) -> R { return finisher(accumulatorValue); });
}

template <typename E, typename K, typename KeyExtractor>
auto toMap(KeyExtractor &&keyExtractor) -> Collector<E, std::map<K, E>, std::map<K, E>>
{
    return full<E, std::map<K, E>, std::map<K, E>>(
        []() -> std::map<K, E> { return std::map<K, E>(); },
        [keyExtractor](std::map<K, E> accumulatorValue, E element, function::Timestamp index) -> std::map<K, E> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp>)
                accumulatorValue[std::invoke(keyExtractor, element, index)] = element;
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E>)
                accumulatorValue[std::invoke(keyExtractor, element)] = element;
            return accumulatorValue;
        },
        [](std::map<K, E> a, std::map<K, E> b) -> std::map<K, E> {
            for (const auto &[key, value] : b)
                a[key] = value;
            return a;
        },
        [](std::map<K, E> accumulatorValue) -> std::map<K, E> { return accumulatorValue; });
}

template <typename E, typename K, typename V, typename KeyExtractor, typename ValueExtractor>
auto toMap(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor) -> Collector<E, std::map<K, V>, std::map<K, V>>
{
    return full<E, std::map<K, V>, std::map<K, V>>(
        []() -> std::map<K, V> { return std::map<K, V>(); },
        [keyExtractor, valueExtractor](std::map<K, V> accumulatorValue, E element, function::Timestamp index) -> std::map<K, V> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp> && std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                accumulatorValue[std::invoke(keyExtractor, element, index)] = std::invoke(valueExtractor, element, index);
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E> && std::is_invocable_r_v<V, ValueExtractor, E>)
                accumulatorValue[std::invoke(keyExtractor, element)] = std::invoke(valueExtractor, element);
            return accumulatorValue;
        },
        [](std::map<K, V> a, std::map<K, V> b) -> std::map<K, V> {
            for (const auto &[key, value] : b)
                a[key] = value;
            return a;
        },
        [](std::map<K, V> accumulatorValue) -> std::map<K, V> { return accumulatorValue; });
}

template <typename E, typename K, typename V>
auto toUnorderedMap(const function::BiFunction<E, function::Timestamp, K> &keyExtractor, const function::BiFunction<E, function::Timestamp, V> &valueExtractor) -> Collector<E, std::unordered_map<K, V>, std::unordered_map<K, V>>
{
    return full<E, std::unordered_map<K, V>, std::unordered_map<K, V>>(
        []() -> std::unordered_map<K, V> { return std::unordered_map<K, V>(); },
        [keyExtractor, valueExtractor](std::unordered_map<K, V> accumulatorValue, E element, function::Timestamp index) -> std::unordered_map<K, V> {
            accumulatorValue[keyExtractor(element, index)] = valueExtractor(element, index);
            return accumulatorValue;
        },
        [](std::unordered_map<K, V> a, std::unordered_map<K, V> b) -> std::unordered_map<K, V> {
            for (const auto &[key, value] : b)
                a[key] = value;
            return a;
        },
        [](std::unordered_map<K, V> accumulatorValue) -> std::unordered_map<K, V> { return accumulatorValue; });
}

template <typename E>
auto toVector() -> Collector<E, std::vector<E>, std::vector<E>>
{
    return full<E, std::vector<E>, std::vector<E>>(
        []() -> std::vector<E> { return std::vector<E>(); },
        [](std::vector<E> accumulatorValue, E element, function::Timestamp index) -> std::vector<E> {
            accumulatorValue.push_back(element);
            return accumulatorValue;
        },
        [](std::vector<E> a, std::vector<E> b) -> std::vector<E> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<E> accumulatorValue) -> std::vector<E> { return accumulatorValue; });
}

template <typename E>
auto toList() -> Collector<E, std::list<E>, std::list<E>>
{
    return full<E, std::list<E>, std::list<E>>(
        []() -> std::list<E> { return std::list<E>(); },
        [](std::list<E> accumulatorValue, E element, function::Timestamp index) -> std::list<E> {
            accumulatorValue.push_back(element);
            return accumulatorValue;
        },
        [](std::list<E> a, std::list<E> b) -> std::list<E> { a.splice(a.end(), b); return a; },
        [](std::list<E> accumulatorValue) -> std::list<E> { return accumulatorValue; });
}

template <typename E>
auto toSet() -> Collector<E, std::set<E>, std::set<E>>
{
    return full<E, std::set<E>, std::set<E>>(
        []() -> std::set<E> { return std::set<E>(); },
        [](std::set<E> accumulatorValue, E element, function::Timestamp index) -> std::set<E> {
            accumulatorValue.insert(element);
            return accumulatorValue;
        },
        [](std::set<E> a, std::set<E> b) -> std::set<E> { a.insert(b.begin(), b.end()); return a; },
        [](std::set<E> accumulatorValue) -> std::set<E> { return accumulatorValue; });
}

template <typename E>
auto toUnorderedSet() -> Collector<E, std::unordered_set<E>, std::unordered_set<E>>
{
    return full<E, std::unordered_set<E>, std::unordered_set<E>>(
        []() -> std::unordered_set<E> { return std::unordered_set<E>(); },
        [](std::unordered_set<E> accumulatorValue, E element, function::Timestamp index) -> std::unordered_set<E> {
            accumulatorValue.insert(element);
            return accumulatorValue;
        },
        [](std::unordered_set<E> a, std::unordered_set<E> b) -> std::unordered_set<E> { a.insert(b.begin(), b.end()); return a; },
        [](std::unordered_set<E> accumulatorValue) -> std::unordered_set<E> { return accumulatorValue; });
}

template <typename E>
auto toDeque() -> Collector<E, std::deque<E>, std::deque<E>>
{
    return full<E, std::deque<E>, std::deque<E>>(
        []() -> std::deque<E> { return std::deque<E>(); },
        [](std::deque<E> accumulatorValue, E element, function::Timestamp index) -> std::deque<E> {
            accumulatorValue.push_back(element);
            return accumulatorValue;
        },
        [](std::deque<E> a, std::deque<E> b) -> std::deque<E> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::deque<E> accumulatorValue) -> std::deque<E> { return accumulatorValue; });
}

template <typename E>
auto toForwardList() -> Collector<E, std::forward_list<E>, std::forward_list<E>>
{
    return full<E, std::forward_list<E>, std::forward_list<E>>(
        []() -> std::forward_list<E> { return std::forward_list<E>(); },
        [](std::forward_list<E> accumulatorValue, E element, function::Timestamp index) -> std::forward_list<E> {
            auto it = accumulatorValue.before_begin();
            while (std::next(it) != accumulatorValue.end())
            {
                ++it;
            }
            accumulatorValue.insert_after(it, element);
            return accumulatorValue;
        },
        [](std::forward_list<E> a, std::forward_list<E> b) -> std::forward_list<E> {
            auto it = a.before_begin();
            while (std::next(it) != a.end())
            {
                ++it;
            }
            a.splice_after(it, b);
            return a;
        },
        [](std::forward_list<E> accumulatorValue) -> std::forward_list<E> { return accumulatorValue; });
}

template <typename E, std::size_t N>
auto toArray() -> Collector<E, std::array<E, N>, std::array<E, N>>
{
    return full<E, std::array<E, N>, std::array<E, N>>(
        []() -> std::array<E, N> { std::array<E, N> arr; return arr; },
        [](std::array<E, N> accumulatorValue, E element, function::Timestamp index) -> std::array<E, N> {
            if (index < N)
                accumulatorValue[index] = element;
            return accumulatorValue;
        },
        [](std::array<E, N> a, std::array<E, N> b) -> std::array<E, N> { return a; },
        [](std::array<E, N> accumulatorValue) -> std::array<E, N> { return accumulatorValue; });
}

template <typename E>
auto toMultiset() -> Collector<E, std::multiset<E>, std::multiset<E>>
{
    return full<E, std::multiset<E>, std::multiset<E>>(
        []() -> std::multiset<E> { return std::multiset<E>(); },
        [](std::multiset<E> accumulatorValue, E element, function::Timestamp index) -> std::multiset<E> {
            accumulatorValue.insert(element);
            return accumulatorValue;
        },
        [](std::multiset<E> a, std::multiset<E> b) -> std::multiset<E> { a.insert(b.begin(), b.end()); return a; },
        [](std::multiset<E> accumulatorValue) -> std::multiset<E> { return accumulatorValue; });
}

template <typename E>
auto toUnorderedMultiset() -> Collector<E, std::unordered_multiset<E>, std::unordered_multiset<E>>
{
    return full<E, std::unordered_multiset<E>, std::unordered_multiset<E>>(
        []() -> std::unordered_multiset<E> { return std::unordered_multiset<E>(); },
        [](std::unordered_multiset<E> accumulatorValue, E element, function::Timestamp index) -> std::unordered_multiset<E> {
            accumulatorValue.insert(element);
            return accumulatorValue;
        },
        [](std::unordered_multiset<E> a, std::unordered_multiset<E> b) -> std::unordered_multiset<E> { a.insert(b.begin(), b.end()); return a; },
        [](std::unordered_multiset<E> accumulatorValue) -> std::unordered_multiset<E> { return accumulatorValue; });
}

template <typename E>
auto toMultimap(const function::Function<E, function::Timestamp> &keyExtractor) -> Collector<E, std::multimap<function::Timestamp, E>, std::multimap<function::Timestamp, E>>
{
    return full<E, std::multimap<function::Timestamp, E>, std::multimap<function::Timestamp, E>>(
        []() -> std::multimap<function::Timestamp, E> { return std::multimap<function::Timestamp, E>(); },
        [keyExtractor](std::multimap<function::Timestamp, E> accumulatorValue, E element, function::Timestamp index) -> std::multimap<function::Timestamp, E> {
            accumulatorValue.insert(std::make_pair(keyExtractor(element, index), element));
            return accumulatorValue;
        },
        [](std::multimap<function::Timestamp, E> a, std::multimap<function::Timestamp, E> b) -> std::multimap<function::Timestamp, E> {
            a.insert(b.begin(), b.end());
            return a;
        },
        [](std::multimap<function::Timestamp, E> accumulatorValue) -> std::multimap<function::Timestamp, E> { return accumulatorValue; });
}

template <typename E, typename K, typename KeyExtractor>
auto toMultimap(KeyExtractor &&keyExtractor) -> Collector<E, std::multimap<K, E>, std::multimap<K, E>>
{
    return full<E, std::multimap<K, E>, std::multimap<K, E>>(
        []() -> std::multimap<K, E> { return std::multimap<K, E>(); },
        [keyExtractor](std::multimap<K, E> accumulatorValue, E element, function::Timestamp index) -> std::multimap<K, E> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element, index), element));
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element), element));
            return accumulatorValue;
        },
        [](std::multimap<K, E> a, std::multimap<K, E> b) -> std::multimap<K, E> {
            a.insert(b.begin(), b.end());
            return a;
        },
        [](std::multimap<K, E> accumulatorValue) -> std::multimap<K, E> { return accumulatorValue; });
}

template <typename E, typename K, typename V, typename KeyExtractor, typename ValueExtractor>
auto toMultimap(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor) -> Collector<E, std::multimap<K, V>, std::multimap<K, V>>
{
    return full<E, std::multimap<K, V>, std::multimap<K, V>>(
        []() -> std::multimap<K, V> { return std::multimap<K, V>(); },
        [keyExtractor, valueExtractor](std::multimap<K, V> accumulatorValue, E element, function::Timestamp index) -> std::multimap<K, V> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp> && std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element, index), std::invoke(valueExtractor, element, index)));
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E> && std::is_invocable_r_v<V, ValueExtractor, E>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element), std::invoke(valueExtractor, element)));
            return accumulatorValue;
        },
        [](std::multimap<K, V> a, std::multimap<K, V> b) -> std::multimap<K, V> {
            a.insert(b.begin(), b.end());
            return a;
        },
        [](std::multimap<K, V> accumulatorValue) -> std::multimap<K, V> { return accumulatorValue; });
}

template <typename E>
auto toUnorderedMultimap(const function::Function<E, function::Timestamp> &keyExtractor) -> Collector<E, std::unordered_multimap<function::Timestamp, E>, std::unordered_multimap<function::Timestamp, E>>
{
    return full<E, std::unordered_multimap<function::Timestamp, E>, std::unordered_multimap<function::Timestamp, E>>(
        []() -> std::unordered_multimap<function::Timestamp, E> { return std::unordered_multimap<function::Timestamp, E>(); },
        [keyExtractor](std::unordered_multimap<function::Timestamp, E> accumulatorValue, E element, function::Timestamp index) -> std::unordered_multimap<function::Timestamp, E> {
            accumulatorValue.insert(std::make_pair(keyExtractor(element, index), element));
            return accumulatorValue;
        },
        [](std::unordered_multimap<function::Timestamp, E> a, std::unordered_multimap<function::Timestamp, E> b) -> std::unordered_multimap<function::Timestamp, E> {
            a.insert(b.begin(), b.end());
            return a;
        },
        [](std::unordered_multimap<function::Timestamp, E> accumulatorValue) -> std::unordered_multimap<function::Timestamp, E> { return accumulatorValue; });
}

template <typename E, typename K, typename KeyExtractor>
auto toUnorderedMultimap(KeyExtractor &&keyExtractor) -> Collector<E, std::unordered_multimap<K, E>, std::unordered_multimap<K, E>>
{
    return full<E, std::unordered_multimap<K, E>, std::unordered_multimap<K, E>>(
        []() -> std::unordered_multimap<K, E> { return std::unordered_multimap<K, E>(); },
        [keyExtractor](std::unordered_multimap<K, E> accumulatorValue, E element, function::Timestamp index) -> std::unordered_multimap<K, E> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element, index), element));
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element), element));
            return accumulatorValue;
        },
        [](std::unordered_multimap<K, E> a, std::unordered_multimap<K, E> b) -> std::unordered_multimap<K, E> {
            a.insert(b.begin(), b.end());
            return a;
        },
        [](std::unordered_multimap<K, E> accumulatorValue) -> std::unordered_multimap<K, E> { return accumulatorValue; });
}

template <typename E, typename K, typename V, typename KeyExtractor, typename ValueExtractor>
auto toUnorderedMultimap(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor) -> Collector<E, std::unordered_multimap<K, V>, std::unordered_multimap<K, V>>
{
    return full<E, std::unordered_multimap<K, V>, std::unordered_multimap<K, V>>(
        []() -> std::unordered_multimap<K, V> { return std::unordered_multimap<K, V>(); },
        [keyExtractor, valueExtractor](std::unordered_multimap<K, V> accumulatorValue, E element, function::Timestamp index) -> std::unordered_multimap<K, V> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp> && std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element, index), std::invoke(valueExtractor, element, index)));
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E> && std::is_invocable_r_v<V, ValueExtractor, E>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element), std::invoke(valueExtractor, element)));
            return accumulatorValue;
        },
        [](std::unordered_multimap<K, V> a, std::unordered_multimap<K, V> b) -> std::unordered_multimap<K, V> {
            a.insert(b.begin(), b.end());
            return a;
        },
        [](std::unordered_multimap<K, V> accumulatorValue) -> std::unordered_multimap<K, V> { return accumulatorValue; });
}

template <typename E>
auto toStack() -> Collector<E, std::stack<E>, std::stack<E>>
{
    return full<E, std::stack<E>, std::stack<E>>(
        []() -> std::stack<E> { return std::stack<E>(); },
        [](std::stack<E> accumulatorValue, E element, function::Timestamp index) -> std::stack<E> {
            accumulatorValue.push(element);
            return accumulatorValue;
        },
        [](std::stack<E> a, std::stack<E> b) -> std::stack<E> {
            std::vector<E> temp;
            while (!b.empty())
            {
                temp.push_back(b.top());
                b.pop();
            }
            for (auto it = temp.rbegin(); it != temp.rend(); ++it)
            {
                a.push(*it);
            }
            return a;
        },
        [](std::stack<E> accumulatorValue) -> std::stack<E> { return accumulatorValue; });
}

template <typename E>
auto toQueue() -> Collector<E, std::queue<E>, std::queue<E>>
{
    return full<E, std::queue<E>, std::queue<E>>(
        []() -> std::queue<E> { return std::queue<E>(); },
        [](std::queue<E> accumulatorValue, E element, function::Timestamp index) -> std::queue<E> {
            accumulatorValue.push(element);
            return accumulatorValue;
        },
        [](std::queue<E> a, std::queue<E> b) -> std::queue<E> {
            while (!b.empty())
            {
                a.push(b.front());
                b.pop();
            }
            return a;
        },
        [](std::queue<E> accumulatorValue) -> std::queue<E> { return accumulatorValue; });
}

template <typename E>
auto toPriorityQueue() -> Collector<E, std::priority_queue<E>, std::priority_queue<E>>
{
    return full<E, std::priority_queue<E>, std::priority_queue<E>>(
        []() -> std::priority_queue<E> { return std::priority_queue<E>(); },
        [](std::priority_queue<E> accumulatorValue, E element, function::Timestamp index) -> std::priority_queue<E> {
            accumulatorValue.push(element);
            return accumulatorValue;
        },
        [](std::priority_queue<E> a, std::priority_queue<E> b) -> std::priority_queue<E> {
            while (!b.empty())
            {
                a.push(b.top());
                b.pop();
            }
            return a;
        },
        [](std::priority_queue<E> accumulatorValue) -> std::priority_queue<E> { return accumulatorValue; });
}

template <typename E>
auto dft() -> Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>
{
    return full<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>(
        []() -> std::vector<std::complex<double>> { return std::vector<std::complex<double>>(); },
        [](std::vector<std::complex<double>> accumulatorValue, E element, function::Timestamp index) -> std::vector<std::complex<double>> {
            if constexpr (std::is_arithmetic_v<E>)
                accumulatorValue.push_back(std::complex<double>(static_cast<double>(element), 0.0));
            else if constexpr (std::is_same_v<E, std::complex<double>>)
                accumulatorValue.push_back(element);
            else
                accumulatorValue.push_back(std::complex<double>(0.0, 0.0));
            return accumulatorValue;
        },
        [](std::vector<std::complex<double>> a, std::vector<std::complex<double>> b) -> std::vector<std::complex<double>> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<std::complex<double>> accumulatorValue) -> std::vector<std::complex<double>> {
            std::size_t N = accumulatorValue.size();
            if (N == 0)
                return accumulatorValue;
            std::vector<std::complex<double>> result(N);
            for (std::size_t k = 0; k < N; ++k)
            {
                std::complex<double> sum(0.0, 0.0);
                for (std::size_t n = 0; n < N; ++n)
                {
                    double angle = -2.0 * pi * static_cast<double>(k * n) / static_cast<double>(N);
                    sum += accumulatorValue[n] * std::complex<double>(std::cos(angle), std::sin(angle));
                }
                result[k] = sum;
            }
            return result;
        });
}

template <typename E>
auto idft() -> Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>
{
    return full<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>(
        []() -> std::vector<std::complex<double>> { return std::vector<std::complex<double>>(); },
        [](std::vector<std::complex<double>> accumulatorValue, E element, function::Timestamp index) -> std::vector<std::complex<double>> {
            if constexpr (std::is_arithmetic_v<E>)
                accumulatorValue.push_back(std::complex<double>(static_cast<double>(element), 0.0));
            else if constexpr (std::is_same_v<E, std::complex<double>>)
                accumulatorValue.push_back(element);
            else
                accumulatorValue.push_back(std::complex<double>(0.0, 0.0));
            return accumulatorValue;
        },
        [](std::vector<std::complex<double>> a, std::vector<std::complex<double>> b) -> std::vector<std::complex<double>> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<std::complex<double>> accumulatorValue) -> std::vector<std::complex<double>> {
            std::size_t N = accumulatorValue.size();
            if (N == 0)
                return accumulatorValue;
            std::vector<std::complex<double>> result(N);
            for (std::size_t k = 0; k < N; ++k)
            {
                std::complex<double> sum(0.0, 0.0);
                for (std::size_t n = 0; n < N; ++n)
                {
                    double angle = 2.0 * pi * static_cast<double>(k * n) / static_cast<double>(N);
                    sum += accumulatorValue[n] * std::complex<double>(std::cos(angle), std::sin(angle));
                }
                result[k] = sum / static_cast<double>(N);
            }
            return result;
        });
}

template <typename E>
auto fft() -> Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>
{
    return full<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>(
        []() -> std::vector<std::complex<double>> { return std::vector<std::complex<double>>(); },
        [](std::vector<std::complex<double>> accumulatorValue, E element, function::Timestamp index) -> std::vector<std::complex<double>> {
            if constexpr (std::is_arithmetic_v<E>)
                accumulatorValue.push_back(std::complex<double>(static_cast<double>(element), 0.0));
            else if constexpr (std::is_same_v<E, std::complex<double>>)
                accumulatorValue.push_back(element);
            else
                accumulatorValue.push_back(std::complex<double>(0.0, 0.0));
            return accumulatorValue;
        },
        [](std::vector<std::complex<double>> a, std::vector<std::complex<double>> b) -> std::vector<std::complex<double>> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<std::complex<double>> accumulatorValue) -> std::vector<std::complex<double>> {
            std::size_t N = accumulatorValue.size();
            if (N == 0)
                return accumulatorValue;
            std::size_t paddedSize = 1;
            while (paddedSize < N)
                paddedSize <<= 1;
            std::vector<std::complex<double>> data = accumulatorValue;
            data.resize(paddedSize, std::complex<double>(0.0, 0.0));
            std::size_t bits = 0;
            for (std::size_t temp = paddedSize; temp > 1; temp >>= 1)
                bits++;
            for (std::size_t i = 0; i < paddedSize; ++i)
            {
                std::size_t reversed = 0;
                for (std::size_t bit = 0; bit < bits; ++bit)
                    if (i & (1 << bit))
                        reversed |= (1 << (bits - 1 - bit));
                if (i < reversed)
                    std::swap(data[i], data[reversed]);
            }
            for (std::size_t length = 2; length <= paddedSize; length <<= 1)
            {
                double angle = -2.0 * pi / static_cast<double>(length);
                std::complex<double> omega(std::cos(angle), std::sin(angle));
                for (std::size_t start = 0; start < paddedSize; start += length)
                {
                    std::complex<double> factor(1.0, 0.0);
                    std::size_t halfLength = length >> 1;
                    for (std::size_t j = 0; j < halfLength; ++j)
                    {
                        std::complex<double> even = data[start + j];
                        std::complex<double> odd = data[start + j + halfLength] * factor;
                        data[start + j] = even + odd;
                        data[start + j + halfLength] = even - odd;
                        factor *= omega;
                    }
                }
            }
            data.resize(N);
            return data;
        });
}

template <typename E>
auto ifft() -> Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>
{
    return full<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>(
        []() -> std::vector<std::complex<double>> { return std::vector<std::complex<double>>(); },
        [](std::vector<std::complex<double>> accumulatorValue, E element, function::Timestamp index) -> std::vector<std::complex<double>> {
            if constexpr (std::is_arithmetic_v<E>)
                accumulatorValue.push_back(std::complex<double>(static_cast<double>(element), 0.0));
            else if constexpr (std::is_same_v<E, std::complex<double>>)
                accumulatorValue.push_back(element);
            else
                accumulatorValue.push_back(std::complex<double>(0.0, 0.0));
            return accumulatorValue;
        },
        [](std::vector<std::complex<double>> a, std::vector<std::complex<double>> b) -> std::vector<std::complex<double>> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<std::complex<double>> accumulatorValue) -> std::vector<std::complex<double>> {
            std::size_t N = accumulatorValue.size();
            if (N == 0)
                return accumulatorValue;
            std::size_t paddedSize = 1;
            while (paddedSize < N)
                paddedSize <<= 1;
            std::vector<std::complex<double>> data = accumulatorValue;
            data.resize(paddedSize, std::complex<double>(0.0, 0.0));
            std::size_t bits = 0;
            for (std::size_t temp = paddedSize; temp > 1; temp >>= 1)
                bits++;
            for (std::size_t i = 0; i < paddedSize; ++i)
            {
                std::size_t reversed = 0;
                for (std::size_t bit = 0; bit < bits; ++bit)
                    if (i & (1 << bit))
                        reversed |= (1 << (bits - 1 - bit));
                if (i < reversed)
                    std::swap(data[i], data[reversed]);
            }
            for (std::size_t length = 2; length <= paddedSize; length <<= 1)
            {
                double angle = 2.0 * pi / static_cast<double>(length);
                std::complex<double> omega(std::cos(angle), std::sin(angle));
                for (std::size_t start = 0; start < paddedSize; start += length)
                {
                    std::complex<double> factor(1.0, 0.0);
                    std::size_t halfLength = length >> 1;
                    for (std::size_t j = 0; j < halfLength; ++j)
                    {
                        std::complex<double> even = data[start + j];
                        std::complex<double> odd = data[start + j + halfLength] * factor;
                        data[start + j] = even + odd;
                        data[start + j + halfLength] = even - odd;
                        factor *= omega;
                    }
                }
            }
            data.resize(N);
            for (std::size_t i = 0; i < N; ++i)
                data[i] /= static_cast<double>(N);
            return data;
        });
}

template <typename E>
auto gradient(const std::function<std::vector<double>(const std::vector<E> &)> &gradientFunction,
                 double learningRate, std::size_t maxIterations, double convergenceThreshold)
    -> Collector<E, std::vector<double>, std::vector<double>>
{
    return full<E, std::vector<double>, std::vector<double>>(
        []() -> std::vector<double> { return std::vector<double>(); },
        [](std::vector<double> accumulatorValue, E element, function::Timestamp index) -> std::vector<double> {
            if constexpr (std::is_arithmetic_v<E>)
                accumulatorValue.push_back(static_cast<double>(element));
            else
                accumulatorValue.push_back(0.0);
            return accumulatorValue;
        },
        [](std::vector<double> a, std::vector<double> b) -> std::vector<double> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [gradientFunction, learningRate, maxIterations, convergenceThreshold](std::vector<double> accumulatorValue) -> std::vector<double> {
            if (accumulatorValue.empty())
                return accumulatorValue;
            std::vector<double> parameters = accumulatorValue;
            std::size_t dimensions = parameters.size();
            for (std::size_t iteration = 0; iteration < maxIterations; ++iteration)
            {
                std::vector<E> currentParameters;
                currentParameters.reserve(dimensions);
                for (double value : parameters)
                {
                    if constexpr (std::is_arithmetic_v<E>)
                        currentParameters.push_back(static_cast<E>(value));
                    else
                        currentParameters.push_back(E{});
                }
                std::vector<double> gradients = gradientFunction(currentParameters);
                if (gradients.size() != dimensions)
                    throw std::invalid_argument("Gradient function returned incorrect number of dimensions");
                double maxGradientMagnitude = 0.0;
                for (std::size_t i = 0; i < dimensions; ++i)
                {
                    parameters[i] -= learningRate * gradients[i];
                    double gradMag = std::abs(gradients[i]);
                    if (gradMag > maxGradientMagnitude)
                        maxGradientMagnitude = gradMag;
                }
                if (maxGradientMagnitude < convergenceThreshold)
                    break;
            }
            return parameters;
        });
}

template <typename E>
auto gradient(const std::function<double(const std::vector<E> &)> &costFunction,
                 double learningRate, std::size_t maxIterations, double convergenceThreshold, double numericalH)
    -> Collector<E, std::vector<double>, std::vector<double>>
{
    return full<E, std::vector<double>, std::vector<double>>(
        []() -> std::vector<double> { return std::vector<double>(); },
        [](std::vector<double> accumulatorValue, E element, function::Timestamp index) -> std::vector<double> {
            if constexpr (std::is_arithmetic_v<E>)
                accumulatorValue.push_back(static_cast<double>(element));
            else
                accumulatorValue.push_back(0.0);
            return accumulatorValue;
        },
        [](std::vector<double> a, std::vector<double> b) -> std::vector<double> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [costFunction, learningRate, maxIterations, convergenceThreshold, numericalH](std::vector<double> accumulatorValue) -> std::vector<double> {
            if (accumulatorValue.empty())
                return accumulatorValue;
            std::vector<double> parameters = accumulatorValue;
            std::size_t dimensions = parameters.size();
            for (std::size_t iteration = 0; iteration < maxIterations; ++iteration)
            {
                std::vector<double> gradients(dimensions);
                for (std::size_t i = 0; i < dimensions; ++i)
                {
                    double originalValue = parameters[i];
                    parameters[i] = originalValue + numericalH;
                    std::vector<E> forwardParameters;
                    forwardParameters.reserve(dimensions);
                    for (double value : parameters)
                    {
                        if constexpr (std::is_arithmetic_v<E>)
                            forwardParameters.push_back(static_cast<E>(value));
                        else
                            forwardParameters.push_back(E{});
                    }
                    double costForward = costFunction(forwardParameters);
                    parameters[i] = originalValue - numericalH;
                    std::vector<E> backwardParameters;
                    backwardParameters.reserve(dimensions);
                    for (double value : parameters)
                    {
                        if constexpr (std::is_arithmetic_v<E>)
                            backwardParameters.push_back(static_cast<E>(value));
                        else
                            backwardParameters.push_back(E{});
                    }
                    double costBackward = costFunction(backwardParameters);
                    gradients[i] = (costForward - costBackward) / (2.0 * numericalH);
                    parameters[i] = originalValue;
                }
                double maxGradientMagnitude = 0.0;
                for (std::size_t i = 0; i < dimensions; ++i)
                {
                    parameters[i] -= learningRate * gradients[i];
                    double gradMag = std::abs(gradients[i]);
                    if (gradMag > maxGradientMagnitude)
                        maxGradientMagnitude = gradMag;
                }
                if (maxGradientMagnitude < convergenceThreshold)
                    break;
            }
            return parameters;
        });
}

template <typename E, typename D>
auto skewness() -> Collector<E, std::vector<D>, D>
{
    return full<E, std::vector<D>, D>(
        []() -> std::vector<D> { return std::vector<D>(); },
        [](std::vector<D> accumulatorValue, E element, function::Timestamp index) -> std::vector<D> {
            accumulatorValue.push_back(static_cast<D>(element));
            return accumulatorValue;
        },
        [](std::vector<D> a, std::vector<D> b) -> std::vector<D> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<D> accumulatorValue) -> D {
            if (accumulatorValue.size() < 3)
                return D{};
            D n = static_cast<D>(accumulatorValue.size());
            D mean = D{};
            for (const D &val : accumulatorValue)
                mean += val;
            mean /= n;
            D variance = D{};
            for (const D &val : accumulatorValue)
            {
                D diff = val - mean;
                variance += diff * diff;
            }
            variance /= n;
            if (variance == D{})
                return D{};
            D stdDev = static_cast<D>(std::sqrt(static_cast<double>(variance)));
            D sumOfCubes = D{};
            for (const D &val : accumulatorValue)
            {
                D diff = val - mean;
                sumOfCubes += diff * diff * diff;
            }
            return (n / ((n - 1) * (n - 2))) * (sumOfCubes / (stdDev * stdDev * stdDev));
        });
}

template <typename E, typename D>
auto skewness(const function::Function<E, D> &mapper) -> Collector<E, std::vector<D>, D>
{
    return full<E, std::vector<D>, D>(
        []() -> std::vector<D> { return std::vector<D>(); },
        [mapper](std::vector<D> accumulatorValue, E element, function::Timestamp index) -> std::vector<D> {
            accumulatorValue.push_back(mapper(element));
            return accumulatorValue;
        },
        [](std::vector<D> a, std::vector<D> b) -> std::vector<D> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<D> accumulatorValue) -> D {
            if (accumulatorValue.size() < 3)
                return D{};
            D n = static_cast<D>(accumulatorValue.size());
            D mean = D{};
            for (const D &val : accumulatorValue)
                mean += val;
            mean /= n;
            D variance = D{};
            for (const D &val : accumulatorValue)
            {
                D diff = val - mean;
                variance += diff * diff;
            }
            variance /= n;
            if (variance == D{})
                return D{};
            D stdDev = static_cast<D>(std::sqrt(static_cast<double>(variance)));
            D sumOfCubes = D{};
            for (const D &val : accumulatorValue)
            {
                D diff = val - mean;
                sumOfCubes += diff * diff * diff;
            }
            return (n / ((n - 1) * (n - 2))) * (sumOfCubes / (stdDev * stdDev * stdDev));
        });
}

template <typename E, typename D>
auto kurtosis() -> Collector<E, std::vector<D>, D>
{
    return full<E, std::vector<D>, D>(
        []() -> std::vector<D> { return std::vector<D>(); },
        [](std::vector<D> accumulatorValue, E element, function::Timestamp index) -> std::vector<D> {
            accumulatorValue.push_back(static_cast<D>(element));
            return accumulatorValue;
        },
        [](std::vector<D> a, std::vector<D> b) -> std::vector<D> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<D> accumulatorValue) -> D {
            if (accumulatorValue.size() < 4)
                return D{};
            D n = static_cast<D>(accumulatorValue.size());
            D mean = D{};
            for (const D &val : accumulatorValue)
                mean += val;
            mean /= n;
            D variance = D{};
            for (const D &val : accumulatorValue)
            {
                D diff = val - mean;
                variance += diff * diff;
            }
            variance /= n;
            if (variance == D{})
                return D{};
            D sumOfQuads = D{};
            for (const D &val : accumulatorValue)
            {
                D diff = val - mean;
                sumOfQuads += diff * diff * diff * diff;
            }
            D s4 = variance * variance;
            D num = n * (n + 1) * (n - 1) * sumOfQuads;
            D denom = (n - 2) * (n - 3) * s4 * n * n;
            if (denom == D{})
                return D{};
            D kurt = num / denom;
            D adjustment = 3.0 * (n - 1) * (n - 1) / ((n - 2) * (n - 3));
            return kurt - adjustment;
        });
}

template <typename E, typename D>
auto kurtosis(const function::Function<E, D> &mapper) -> Collector<E, std::vector<D>, D>
{
    return full<E, std::vector<D>, D>(
        []() -> std::vector<D> { return std::vector<D>(); },
        [mapper](std::vector<D> accumulatorValue, E element, function::Timestamp index) -> std::vector<D> {
            accumulatorValue.push_back(mapper(element));
            return accumulatorValue;
        },
        [](std::vector<D> a, std::vector<D> b) -> std::vector<D> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::vector<D> accumulatorValue) -> D {
            if (accumulatorValue.size() < 4)
                return D{};
            D n = static_cast<D>(accumulatorValue.size());
            D mean = D{};
            for (const D &val : accumulatorValue)
                mean += val;
            mean /= n;
            D variance = D{};
            for (const D &val : accumulatorValue)
            {
                D diff = val - mean;
                variance += diff * diff;
            }
            variance /= n;
            if (variance == D{})
                return D{};
            D sumOfQuads = D{};
            for (const D &val : accumulatorValue)
            {
                D diff = val - mean;
                sumOfQuads += diff * diff * diff * diff;
            }
            D s4 = variance * variance;
            D num = n * (n + 1) * (n - 1) * sumOfQuads;
            D denom = (n - 2) * (n - 3) * s4 * n * n;
            if (denom == D{})
                return D{};
            D kurt = num / denom;
            D adjustment = 3.0 * (n - 1) * (n - 1) / ((n - 2) * (n - 3));
            return kurt - adjustment;
        });
}

} // namespace collector

#endif // SEMANTIC_CPP_FACTORY_COLLECTOR

#if defined(SEMANTIC_CPP_SEMANTIC_COMPLETE) && !defined(SEMANTIC_CPP_FACTORY_SEMANTIC)
#define SEMANTIC_CPP_FACTORY_SEMANTIC

namespace semantic
{
template <typename D>
auto range(const D &start, const D &end) -> Semantic<D>
{
    return Semantic<D>([startValue = std::min(start, end), endValue = std::max(start, end)](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        function::Timestamp index = 0LL;
        for (D value = startValue; value < endValue; value++)
        {
            if (interrupt(value, index))
            {
                break;
            }
            accept(value, index);
            index++;
        }
    },
                       1ULL);
}

template <typename D>
auto range(const D &start, const D &end, const D &step) -> Semantic<D>
{
    return Semantic<D>([startValue = start, endValue = end, stepValue = step](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        if (stepValue == D{})
        {
            return;
        }
        function::Timestamp index = 0LL;
        if (stepValue > D{})
        {
            for (D value = startValue; value < endValue; value += stepValue)
            {
                if (interrupt(value, index))
                {
                    break;
                }
                accept(value, index);
                index++;
            }
        }
        else
        {
            for (D value = startValue; value > endValue; value += stepValue)
            {
                if (interrupt(value, index))
                {
                    break;
                }
                accept(value, index);
                index++;
            }
        }
    },
                       1ULL);
}

template <typename D>
auto rangeClosed(const D &start, const D &end) -> Semantic<D>
{
    return Semantic<D>([startValue = std::min(start, end), endValue = std::max(start, end)](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        function::Timestamp index = 0LL;
        for (D value = startValue; value <= endValue; value++)
        {
            if (interrupt(value, index))
            {
                break;
            }
            accept(value, index);
            index++;
        }
    },
                       1ULL);
}

template <typename D>
auto rangeClosed(const D &start, const D &end, const D &step) -> Semantic<D>
{
    return Semantic<D>([startValue = start, endValue = end, stepValue = step](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        if (stepValue == D{})
        {
            return;
        }
        function::Timestamp index = 0LL;
        if (stepValue > D{})
        {
            for (D value = startValue; value <= endValue; value += stepValue)
            {
                if (interrupt(value, index))
                {
                    break;
                }
                accept(value, index);
                index++;
            }
        }
        else
        {
            for (D value = startValue; value >= endValue; value += stepValue)
            {
                if (interrupt(value, index))
                {
                    break;
                }
                accept(value, index);
                index++;
            }
        }
    },
                       1ULL);
}

template <typename D, typename UnaryFunc,
          typename = std::enable_if_t<std::is_invocable_r_v<D, UnaryFunc, const D &>>>
auto infinite(const D &seed, UnaryFunc &&generator) -> Semantic<D>
{
    static_assert(std::is_invocable_r_v<D, UnaryFunc, const D &>, "infinite: generator must be callable as D(const D&)");
    return Semantic<D>([seed, generator = std::forward<UnaryFunc>(generator)](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        D current = seed;
        function::Timestamp index = 0LL;
        while (true)
        {
            if (interrupt(current, index))
            {
                break;
            }
            accept(current, index);
            current = generator(current);
            index++;
        }
    },
                       1ULL);
}

template <typename SupplierFunc,
          typename D = std::invoke_result_t<SupplierFunc>,
          typename = std::enable_if_t<std::is_invocable_r_v<D, SupplierFunc>>>
auto generate(SupplierFunc &&supplier) -> Semantic<D>
{
    static_assert(std::is_invocable_r_v<D, SupplierFunc>, "generate: supplier must be callable as D()");
    return Semantic<D>([supplier = std::forward<SupplierFunc>(supplier)](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        function::Timestamp index = 0LL;
        while (true)
        {
            D value = supplier();
            if (interrupt(value, index))
            {
                break;
            }
            accept(value, index);
            index++;
        }
    },
                       1ULL);
}

template <typename SupplierFunc,
          typename D = std::invoke_result_t<SupplierFunc>,
          typename = std::enable_if_t<std::is_invocable_r_v<D, SupplierFunc>>>
auto generate(SupplierFunc &&supplier, const function::Module &limit) -> Semantic<D>
{
    static_assert(std::is_invocable_r_v<D, SupplierFunc>, "generate: supplier must be callable as D()");
    return Semantic<D>([supplier = std::forward<SupplierFunc>(supplier), limit](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        function::Timestamp index = 0LL;
        while (index < limit)
        {
            D value = supplier();
            if (interrupt(value, index))
            {
                break;
            }
            accept(value, index);
            index++;
        }
    },
                       1ULL);
}

template <typename D, typename UnaryFunc,
          typename = std::enable_if_t<std::is_invocable_r_v<D, UnaryFunc, const D &>>>
auto iterate(const D &seed, UnaryFunc &&generator) -> Semantic<D>
{
    static_assert(std::is_invocable_r_v<D, UnaryFunc, const D &>, "iterate: generator must be callable as D(const D&)");
    return Semantic<D>([seed, generator = std::forward<UnaryFunc>(generator)](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        D current = seed;
        function::Timestamp index = 0LL;
        while (true)
        {
            if (interrupt(current, index))
            {
                break;
            }
            accept(current, index);
            current = generator(current);
            index++;
        }
    },
                       1ULL);
}

template <typename D, typename UnaryFunc,
          typename = std::enable_if_t<std::is_invocable_r_v<D, UnaryFunc, const D &>>>
auto iterate(const D &seed, UnaryFunc &&generator, const function::Module &limit) -> Semantic<D>
{
    static_assert(std::is_invocable_r_v<D, UnaryFunc, const D &>, "iterate: generator must be callable as D(const D&)");
    return Semantic<D>([seed, generator = std::forward<UnaryFunc>(generator), limit](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        D current = seed;
        function::Timestamp index = 0LL;
        while (index < limit)
        {
            if (interrupt(current, index))
            {
                break;
            }
            accept(current, index);
            current = generator(current);
            index++;
        }
    },
                       1ULL);
}

template <typename D>
auto random() -> Semantic<D>
{
    return Semantic<D>([](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        std::random_device device;
        std::mt19937 generator(device());
        std::uniform_int_distribution<D> distribution;
        function::Timestamp index = 0LL;
        while (true)
        {
            D value = distribution(generator);
            if (interrupt(value, index))
            {
                break;
            }
            accept(value, index);
            index++;
        }
    },
                       1ULL);
}

template <typename D>
auto random(const D &min, const D &max) -> Semantic<D>
{
    return Semantic<D>([minValue = std::min(min, max), maxValue = std::max(min, max)](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        std::random_device device;
        std::mt19937 generator(device());
        if constexpr (std::is_integral_v<D>)
        {
            std::uniform_int_distribution<D> distribution(minValue, maxValue);
            function::Timestamp index = 0LL;
            while (true)
            {
                D value = distribution(generator);
                if (interrupt(value, index))
                {
                    break;
                }
                accept(value, index);
                index++;
            }
        }
        else
        {
            std::uniform_real_distribution<D> distribution(minValue, maxValue);
            function::Timestamp index = 0LL;
            while (true)
            {
                D value = distribution(generator);
                if (interrupt(value, index))
                {
                    break;
                }
                accept(value, index);
                index++;
            }
        }
    },
                       1ULL);
}

template <typename D>
auto random(const D &min, const D &max, const function::Module &count) -> Semantic<D>
{
    return Semantic<D>([minValue = std::min(min, max), maxValue = std::max(min, max), count](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        std::random_device device;
        std::mt19937 generator(device());
        if constexpr (std::is_integral_v<D>)
        {
            std::uniform_int_distribution<D> distribution(minValue, maxValue);
            function::Timestamp index = 0LL;
            while (index < count)
            {
                D value = distribution(generator);
                if (interrupt(value, index))
                {
                    break;
                }
                accept(value, index);
                index++;
            }
        }
        else
        {
            std::uniform_real_distribution<D> distribution(minValue, maxValue);
            function::Timestamp index = 0LL;
            while (index < count)
            {
                D value = distribution(generator);
                if (interrupt(value, index))
                {
                    break;
                }
                accept(value, index);
                index++;
            }
        }
    },
                       1ULL);
}

template <typename D>
auto empty() -> Semantic<D>
{
    return Semantic<D>([](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        return;
    });
}

template <typename D>
auto of(D element) -> Semantic<D>
{
    return Semantic<D>([element](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
        if (!interrupt(element, 0LL))
        {
            accept(element, 0LL);
        }
    },
                       1LL);
}

template <typename E>
auto of(E element1, E element2) -> Semantic<E>
{
    return Semantic<E>([element1, element2](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
        if (!interrupt(element1, 0LL))
        {
            accept(element1, 0LL);
        }
        if (!interrupt(element2, 1LL))
        {
            accept(element2, 1LL);
        }
    },
                       1LL);
}

template <typename E>
auto of(E element1, E element2, E element3) -> Semantic<E>
{
    return Semantic<E>([element1, element2, element3](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
        if (!interrupt(element1, 0LL))
        {
            accept(element1, 0LL);
        }
        if (!interrupt(element2, 1LL))
        {
            accept(element2, 1LL);
        }
        if (!interrupt(element3, 2LL))
        {
            accept(element3, 2LL);
        }
    },
                       1LL);
}

template <typename Container>
auto from(Container container) -> Semantic<typename Container::value_type>
{
    using E = typename Container::value_type;
    return Semantic<E>([elements = std::forward<Container>(container)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
        function::Timestamp index = 0LL;
        for (const E &element : elements)
        {
            if (interrupt(element, index))
            {
                break;
            }
            accept(element, index);
            index++;
        }
    },
                       1LL);
}

template <typename E>
auto from(std::initializer_list<E> list) -> Semantic<E>
{
    return Semantic<E>([elements = std::vector<E>(list)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
        function::Timestamp index = 0LL;
        for (const E &element : elements)
        {
            if (interrupt(element, index))
            {
                break;
            }
            accept(element, index);
            index++;
        }
    },
                       1LL);
}

template <typename E>
auto of(std::initializer_list<E> elements) -> Semantic<E>
{
    return Semantic<E>([elements = std::vector<E>(elements)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
        function::Timestamp index = 0LL;
        for (const E &element : elements)
        {
            if (interrupt(element, index))
            {
                break;
            }
            accept(element, index);
            index++;
        }
    },
                       1LL);
}

template <typename E>
auto repeat(const E &element, const function::Module &count) -> Semantic<E>
{
    return Semantic<E>([element, count](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
        for (function::Timestamp index = 0LL; index < count; index++)
        {
            if (interrupt(element, index))
            {
                break;
            }
            accept(element, index);
        }
    });
}

auto blob(const std::string &text) -> Semantic<char>
{
    return Semantic<char>([text](function::BiConsumer<char, function::Timestamp> accept, function::BiPredicate<char, function::Timestamp> interrupt) -> void {
        function::Timestamp index = 0LL;
        for (const auto &byte : text)
        {
            if (interrupt(byte, index))
            {
                break;
            }
            accept(byte, index);
            index++;
        }
    },
                          1LL);
}

auto blob(const std::string &text, function::Module start, const function::Module end) -> Semantic<char>
{
    return Semantic<char>([text, start, end](function::BiConsumer<char, function::Timestamp> accept, function::BiPredicate<char, function::Timestamp> interrupt) -> void {
        function::Module limitedStart = std::max(start, static_cast<function::Module>(0LL));
        function::Module limitedEnd = std::min(end, static_cast<function::Module>(text.size()));
        if (limitedStart < limitedEnd)
        {
            function::Timestamp index = 0LL;
            for (function::Module i = limitedStart; i < limitedEnd; i++)
            {
                if (interrupt(text[i], index))
                {
                    break;
                }
                accept(text[i], index);
                index++;
            }
        }
    },
                          1LL);
}

auto blob(std::istream &stream) -> Semantic<std::string>
{
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(stream, line))
    {
        lines.push_back(line);
    }
    return Semantic<std::string>([lines = std::move(lines)](function::BiConsumer<std::string, function::Timestamp> accept, function::BiPredicate<std::string, function::Timestamp> interrupt) -> void {
        function::Timestamp index = 0LL;
        for (const auto &l : lines)
        {
            if (interrupt(l, index))
            {
                break;
            }
            accept(l, index);
            index++;
        }
    },
                                 1LL);
}

auto blob(std::istream &stream, const char &delimiter) -> Semantic<std::string>
{
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(stream, line, delimiter))
    {
        lines.push_back(line);
    }
    return Semantic<std::string>([lines = std::move(lines)](function::BiConsumer<std::string, function::Timestamp> accept, function::BiPredicate<std::string, function::Timestamp> interrupt) -> void {
        function::Timestamp index = 0LL;
        for (const auto &l : lines)
        {
            if (interrupt(l, index))
            {
                break;
            }
            accept(l, index);
            index++;
        }
    },
                                 1LL);
}

auto text(const std::string &text) -> Semantic<::bunch::Bunch>
{
    return Semantic<::bunch::Bunch>([sequence = ::bunch::Bunch(text)](function::BiConsumer<::bunch::Bunch, function::Timestamp> accept, function::BiPredicate<::bunch::Bunch, function::Timestamp> interrupt) -> void {
        if (!interrupt(sequence, 0LL))
        {
            accept(sequence, 0LL);
        }
    },
                                                1LL);
}

auto text(const std::string &text, const char &delimiter) -> Semantic<::bunch::Bunch>
{
    return Semantic<::bunch::Bunch>([sequence = ::bunch::Bunch(text), delimiter](function::BiConsumer<::bunch::Bunch, function::Timestamp> accept, function::BiPredicate<::bunch::Bunch, function::Timestamp> interrupt) -> void {
        ::bunch::Bunch delim(std::string(1, delimiter));
        auto parts = sequence.split(delim);
        for (function::Module i = 0; i < parts.size(); i++)
        {
            if (interrupt(parts[i], i))
            {
                break;
            }
            accept(parts[i], i);
        }
    },
                                                1LL);
}

auto text(std::istream &stream) -> Semantic<::bunch::Bunch>
{
    std::string content;
    std::string chunk;
    constexpr std::size_t bufferSize = 4096;
    chunk.resize(bufferSize);
    while (stream.read(&chunk[0], bufferSize) || stream.gcount() > 0)
    {
        content.append(chunk, 0, stream.gcount());
    }
    ::bunch::Bunch sequence(content, ::bunch::charset::utf8, ::bunch::charset::utf8);
    return Semantic<::bunch::Bunch>([sequence](function::BiConsumer<::bunch::Bunch, function::Timestamp> accept, function::BiPredicate<::bunch::Bunch, function::Timestamp> interrupt) -> void {
        if (!interrupt(sequence, 0LL))
        {
            accept(sequence, 0LL);
        }
    },
                                                1LL);
}

auto text(std::istream &stream, const char &delimiter) -> Semantic<::bunch::Bunch>
{
    std::string content;
    std::string chunk;
    constexpr std::size_t bufferSize = 4096;
    chunk.resize(bufferSize);
    while (stream.read(&chunk[0], bufferSize) || stream.gcount() > 0)
    {
        content.append(chunk, 0, stream.gcount());
    }
    ::bunch::Bunch delim(std::string(1, delimiter));
    ::bunch::Bunch sequence(content, ::bunch::charset::utf8, ::bunch::charset::utf8);
    auto parts = sequence.split(delim);
    return Semantic<::bunch::Bunch>([parts = std::move(parts)](function::BiConsumer<::bunch::Bunch, function::Timestamp> accept, function::BiPredicate<::bunch::Bunch, function::Timestamp> interrupt) -> void {
        for (function::Module i = 0; i < parts.size(); i++)
        {
            if (interrupt(parts[i], i))
            {
                break;
            }
            accept(parts[i], i);
        }
    },
                                                1LL);
}

auto sequence(const ::bunch::Bunch &sequence) -> Semantic<::bunch::Point>
{
    return Semantic<::bunch::Point>([sequence](function::BiConsumer<::bunch::Point, function::Timestamp> accept, function::BiPredicate<::bunch::Point, function::Timestamp> interrupt) -> void {
        std::vector<::bunch::Point> points = sequence.getPoints();
        function::Timestamp index = 0LL;
        for (const ::bunch::Point &point : points)
        {
            if (interrupt(point, index))
            {
                break;
            }
            accept(point, index);
            index++;
        }
    },
                                         1LL);
}

auto sequence(const ::bunch::Bunch &sequence, function::Module start, const function::Module end) -> Semantic<::bunch::Point>
{
    return Semantic<::bunch::Point>([sequence, start, end](function::BiConsumer<::bunch::Point, function::Timestamp> accept, function::BiPredicate<::bunch::Point, function::Timestamp> interrupt) -> void {
        std::vector<::bunch::Point> points = sequence.getPoints();
        function::Module limitedStart = std::max(start, static_cast<function::Module>(0LL));
        function::Module limitedEnd = std::min(end, static_cast<function::Module>(points.size()));
        if (limitedStart < limitedEnd)
        {
            function::Timestamp index = 0LL;
            for (function::Module i = limitedStart; i < limitedEnd; i++)
            {
                if (interrupt(points[i], index))
                {
                    break;
                }
                accept(points[i], index);
                index++;
            }
        }
    },
                                         1LL);
}

auto sequence(std::istream &stream, ::bunch::charset encoding = ::bunch::charset::utf8) -> Semantic<::bunch::Point>
{
    std::string content;
    std::string chunk;
    constexpr std::size_t bufferSize = 4096;
    chunk.resize(bufferSize);
    while (stream.read(&chunk[0], bufferSize) || stream.gcount() > 0)
    {
        content.append(chunk, 0, stream.gcount());
    }
    ::bunch::Bunch sequence(content, encoding, encoding);
    std::vector<::bunch::Point> points = sequence.getPoints();
    return Semantic<::bunch::Point>([points = std::move(points)](function::BiConsumer<::bunch::Point, function::Timestamp> accept, function::BiPredicate<::bunch::Point, function::Timestamp> interrupt) -> void {
        function::Timestamp index = 0LL;
        for (const ::bunch::Point &point : points)
        {
            if (interrupt(point, index))
            {
                break;
            }
            accept(point, index);
            index++;
        }
    },
                                         1LL);
}

auto sequence(const std::string &text, ::bunch::charset encoding = ::bunch::charset::utf8) -> Semantic<::bunch::Point>
{
    return Semantic<::bunch::Point>([text, encoding](function::BiConsumer<::bunch::Point, function::Timestamp> accept, function::BiPredicate<::bunch::Point, function::Timestamp> interrupt) -> void {
        ::bunch::Bunch sequence(text, encoding);
        std::vector<::bunch::Point> points = sequence.getPoints();
        function::Timestamp index = 0LL;
        for (const ::bunch::Point &point : points)
        {
            if (interrupt(point, index))
            {
                break;
            }
            accept(point, index);
            index++;
        }
    },
                                         1LL);
}

auto bunch(const ::bunch::Bunch &sequence) -> Semantic<::bunch::Bunch>
{
    return Semantic<::bunch::Bunch>([sequence](function::BiConsumer<::bunch::Bunch, function::Timestamp> accept, function::BiPredicate<::bunch::Bunch, function::Timestamp> interrupt) -> void {
        if (!interrupt(sequence, 0LL))
        {
            accept(sequence, 0LL);
        }
    },
                                                1LL);
}

auto bunch(const ::bunch::Bunch &sequence, const ::bunch::Bunch &delimiter) -> Semantic<::bunch::Bunch>
{
    return Semantic<::bunch::Bunch>([sequence, delimiter](function::BiConsumer<::bunch::Bunch, function::Timestamp> accept, function::BiPredicate<::bunch::Bunch, function::Timestamp> interrupt) -> void {
        auto parts = sequence.split(delimiter);
        for (function::Module i = 0; i < parts.size(); i++)
        {
            if (interrupt(parts[i], i))
            {
                break;
            }
            accept(parts[i], i);
        }
    },
                                                1LL);
}

auto bunch(std::istream &stream, ::bunch::charset encoding = ::bunch::charset::utf8) -> Semantic<::bunch::Bunch>
{
    std::string content;
    std::string chunk;
    constexpr std::size_t bufferSize = 4096;
    chunk.resize(bufferSize);
    while (stream.read(&chunk[0], bufferSize) || stream.gcount() > 0)
    {
        content.append(chunk, 0, stream.gcount());
    }
    ::bunch::Bunch sequence(content, encoding, encoding);
    return Semantic<::bunch::Bunch>([sequence](function::BiConsumer<::bunch::Bunch, function::Timestamp> accept, function::BiPredicate<::bunch::Bunch, function::Timestamp> interrupt) -> void {
        if (!interrupt(sequence, 0LL))
        {
            accept(sequence, 0LL);
        }
    },
                                                1LL);
}

auto bunch(std::istream &stream, const ::bunch::Bunch &delimiter, ::bunch::charset encoding = ::bunch::charset::utf8) -> Semantic<::bunch::Bunch>
{
    std::string content;
    std::string chunk;
    constexpr std::size_t bufferSize = 4096;
    chunk.resize(bufferSize);
    while (stream.read(&chunk[0], bufferSize) || stream.gcount() > 0)
    {
        content.append(chunk, 0, stream.gcount());
    }
    ::bunch::Bunch sequence(content, encoding, encoding);
    auto parts = sequence.split(delimiter);
    return Semantic<::bunch::Bunch>([parts = std::move(parts)](function::BiConsumer<::bunch::Bunch, function::Timestamp> accept, function::BiPredicate<::bunch::Bunch, function::Timestamp> interrupt) -> void {
        for (function::Module i = 0; i < parts.size(); i++)
        {
            if (interrupt(parts[i], i))
            {
                break;
            }
            accept(parts[i], i);
        }
    },
                                                1LL);
}

} // namespace semantic

#endif // SEMANTIC_CPP_FACTORY_SEMANTIC

#if !defined(SEMANTIC_CPP_SEMANTIC_COMPLETE) && !defined(SEMANTIC_CPP_FACTORY_SEMANTIC)
#include "semantic.h"
#endif
