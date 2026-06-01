#pragma once
#include "collector.h"
#include "charsequence.h"
#include <cmath>
#include <complex>
#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <numeric>

namespace collector
{
constexpr double PI() noexcept
{
	double sum = 0.0;
	double sign = 1.0;
	for (int i = 0; i < 20; ++i)
	{
		sum += sign / (2.0 * static_cast<double>(i) + 1.0);
		sign = -sign;
	}
	return 4.0 * sum;
}

constexpr double pi = PI();

template <typename E, typename Predicate>
auto useAllMatch(Predicate &&predicate) -> Collector<E, bool, bool>
{
	return useShortable<E, bool, bool>(
		[]() -> bool { return true; },
		[](E element, function::Timestamp index, bool accumulator) -> bool { return !accumulator; },
		[predicate](bool accumulator, E element, function::Timestamp index) -> bool {
			if constexpr (std::is_invocable_r_v<bool, Predicate, E, function::Timestamp>)
				return accumulator && std::invoke(predicate, element, index);
			else if (std::is_invocable_r_v<bool, Predicate, E>)
				return accumulator && std::invoke(predicate, element);
			return false;
		},
		[](bool a, bool b) -> bool { return a && b; },
		[](bool accumulator) -> bool { return accumulator; });
}

template <typename E, typename Predicate>
auto useAnyMatch(Predicate &&predicate) -> Collector<E, bool, bool>
{
	return useShortable<E, bool, bool>(
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
auto useNoneMatch(Predicate &&predicate) -> Collector<E, bool, bool>
{
	return useShortable<E, bool, bool>(
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
auto useForEach(Consumer &&consumer) -> Collector<E, function::Module, function::Module>
{
	return useFull<E, function::Module, function::Module>(
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
auto useCount() -> Collector<E, function::Module, function::Module>
{
	return useFull<E, function::Module, function::Module>(
		[]() -> function::Module { return 0LL; },
		[](function::Module accumulatorValue, E element, function::Timestamp index) -> function::Module { return accumulatorValue + 1; },
		[](function::Module a, function::Module b) -> function::Module { return a + b; },
		[](function::Module accumulatorValue) -> function::Module { return accumulatorValue; });
}

template <typename E, typename A, typename R>
auto useCollect(const Identity<R> &identity, const Accumulator<A, E> &accumulator, const Combiner<A> &combiner, const Finisher<A, R> &finisher) -> Collector<E, A, R>
{
	return useFull<E, A, R>(identity, accumulator, combiner, finisher);
}

template <typename E, typename A, typename R>
Collector<E, A, R> useCollect(const Identity<R> &identity, const Interrupt<E, A> &interrupt, const Accumulator<A, E> &accumulator, const Combiner<A> &combiner, const Finisher<A, R> &finisher)
{
	return useShortable<E, A, R>(identity, interrupt, accumulator, combiner, finisher);
}

template <typename E, typename D>
auto useSummate() -> Collector<E, D, D>
{
	return useFull<E, D, D>(
		[]() -> D { return D{}; },
		[](D accumulatorValue, E element, function::Timestamp index) -> D { return accumulatorValue + static_cast<D>(element); },
		[](D a, D b) -> D { return a + b; },
		[](D accumulatorValue) -> D { return accumulatorValue; });
}

template <typename E, typename D>
auto useSummate(const function::Function<E, D> &mapper) -> Collector<E, D, D>
{
	return useFull<E, D, D>(
		[]() -> D { return D{}; },
		[mapper](D accumulatorValue, E element, function::Timestamp index) -> D { return accumulatorValue + mapper(element); },
		[](D a, D b) -> D { return a + b; },
		[](D accumulatorValue) -> D { return accumulatorValue; });
}

template <typename E, typename D>
auto useAverage() -> Collector<E, std::pair<D, function::Module>, D>
{
	return useFull<E, std::pair<D, function::Module>, D>(
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
auto useAverage(const function::Function<E, D> &mapper) -> Collector<E, std::pair<D, function::Module>, D>
{
	return useFull<E, std::pair<D, function::Module>, D>(
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
auto useRange() -> Collector<E, std::pair<D, D>, D>
{
	return useFull<E, std::pair<D, D>, D>(
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
auto useRange(const function::Function<E, D> &mapper) -> Collector<E, std::pair<D, D>, D>
{
	return useFull<E, std::pair<D, D>, D>(
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
auto useMinimum() -> Collector<E, std::optional<D>, std::optional<D>>
{
	return useFull<E, std::optional<D>, std::optional<D>>(
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
auto useMinimum(const function::Function<E, D> &mapper) -> Collector<E, std::optional<D>, std::optional<D>>
{
	return useFull<E, std::optional<D>, std::optional<D>>(
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
auto useMaximum() -> Collector<E, std::optional<D>, std::optional<D>>
{
	return useFull<E, std::optional<D>, std::optional<D>>(
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
auto useMaximum(const function::Function<E, D> &mapper) -> Collector<E, std::optional<D>, std::optional<D>>
{
	return useFull<E, std::optional<D>, std::optional<D>>(
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
auto useVariance() -> Collector<E, std::pair<D, std::vector<D>>, D>
{
	return useFull<E, std::pair<D, std::vector<D>>, D>(
		[]() -> std::pair<D, std::vector<D>> { return std::make_pair(D{}, std::vector<D>()); },
		[](std::pair<D, std::vector<D>> accumulatorValue, E element, function::Timestamp index) -> std::pair<D, std::vector<D>> {
			D value = static_cast<D>(element);
			accumulatorValue.first += value;
			accumulatorValue.second.push_back(value);
			return accumulatorValue;
		},
		[](std::pair<D, std::vector<D>> a, std::pair<D, std::vector<D>> b) -> std::pair<D, std::vector<D>> {
			a.first += b.first;
			a.second.insert(a.second.end(), b.second.begin(), b.second.end());
			return a;
		},
		[](std::pair<D, std::vector<D>> accumulatorValue) -> D {
			if (accumulatorValue.second.empty())
				return D{};
			D mean = accumulatorValue.first / static_cast<D>(accumulatorValue.second.size());
			D sumOfSquares = D{};
			for (const D &value : accumulatorValue.second)
			{
				D diff = value - mean;
				sumOfSquares += diff * diff;
			}
			return sumOfSquares / static_cast<D>(accumulatorValue.second.size());
		});
}

template <typename E, typename D>
auto useVariance(const function::Function<E, D> &mapper) -> Collector<E, std::pair<D, std::vector<D>>, D>
{
	return useFull<E, std::pair<D, std::vector<D>>, D>(
		[]() -> std::pair<D, std::vector<D>> { return std::make_pair(D{}, std::vector<D>()); },
		[mapper](std::pair<D, std::vector<D>> accumulatorValue, E element, function::Timestamp index) -> std::pair<D, std::vector<D>> {
			D value = mapper(element);
			accumulatorValue.first += value;
			accumulatorValue.second.push_back(value);
			return accumulatorValue;
		},
		[](std::pair<D, std::vector<D>> a, std::pair<D, std::vector<D>> b) -> std::pair<D, std::vector<D>> {
			a.first += b.first;
			a.second.insert(a.second.end(), b.second.begin(), b.second.end());
			return a;
		},
		[](std::pair<D, std::vector<D>> accumulatorValue) -> D {
			if (accumulatorValue.second.empty())
				return D{};
			D mean = accumulatorValue.first / static_cast<D>(accumulatorValue.second.size());
			D sumOfSquares = D{};
			for (const D &value : accumulatorValue.second)
			{
				D diff = value - mean;
				sumOfSquares += diff * diff;
			}
			return sumOfSquares / static_cast<D>(accumulatorValue.second.size());
		});
}

template <typename E>
auto useFindAny() -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useShortable<E, std::optional<E>, std::optional<E>>(
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
auto useFindAt(const function::Timestamp &index) -> Collector<E, std::optional<E>, std::optional<E>>
{
	if (index < 0LL)
		throw std::invalid_argument("use useFindNegativeAt for negative index.");
	const function::Timestamp target = index;
	return useShortable<E, std::optional<E>, std::optional<E>>(
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
auto useFindNegativeAt(const function::Timestamp &index) -> Collector<E, std::pair<std::vector<E>, function::Module>, std::optional<E>>
{
	if (index > -1LL)
		throw std::invalid_argument("use useFindAt for non-negative index.");
	return useFull<E, std::pair<std::vector<E>, function::Module>, std::optional<E>>(
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
auto useFindFirst() -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useShortable<E, std::optional<E>, std::optional<E>>(
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
auto useFindLast() -> Collector<E, std::vector<E>, std::optional<E>>
{
	return useFull<E, std::vector<E>, std::optional<E>>(
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
auto useFindMaximum() -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useFull<E, std::optional<E>, std::optional<E>>(
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
auto useFindMaximum(const function::Comparator<E> &comparator) -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useFull<E, std::optional<E>, std::optional<E>>(
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
auto useFindMinimum() -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useFull<E, std::optional<E>, std::optional<E>>(
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
auto useFindMinimum(const function::Comparator<E> &comparator) -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useFull<E, std::optional<E>, std::optional<E>>(
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
auto useGroup(KeyExtractor &&keyExtractor) -> Collector<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>>
{
	return useFull<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>>(
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

template <typename E>
auto useJoin() -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence { return charsequence::Charsequence(); },
		[](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
					return accumulatorValue + charsequence::Charsequence(",") + element;
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
					return accumulatorValue + charsequence::Charsequence(",") + elementStr;
				return elementStr;
			}
			return accumulatorValue;
		},
		[](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence { return a + charsequence::Charsequence(",") + b; },
		[](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence { return charsequence::Charsequence("[") + accumulatorValue + charsequence::Charsequence("]"); });
}

template <typename E>
auto useJoin(const charsequence::Charsequence &delimiter) -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence { return charsequence::Charsequence(); },
		[delimiter](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
					return accumulatorValue + delimiter + element;
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
					return accumulatorValue + delimiter + elementStr;
				return elementStr;
			}
			return accumulatorValue;
		},
		[](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence { return a + charsequence::Charsequence(",") + b; },
		[](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence { return charsequence::Charsequence("[") + accumulatorValue + charsequence::Charsequence("]"); });
}

template <typename E>
auto useJoin(const charsequence::Charsequence &prefix, const charsequence::Charsequence &delimiter, const charsequence::Charsequence &suffix) -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence { return charsequence::Charsequence(); },
		[delimiter](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
					return accumulatorValue + delimiter + element;
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
					return accumulatorValue + delimiter + elementStr;
				return elementStr;
			}
			return accumulatorValue;
		},
		[delimiter](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence { return a + delimiter + b; },
		[prefix, suffix](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence { return prefix + accumulatorValue + suffix; });
}

template <typename E>
auto useOut() -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence { return charsequence::Charsequence(); },
		[](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
					return accumulatorValue + charsequence::Charsequence(",") + element;
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
					return accumulatorValue + charsequence::Charsequence(",") + elementStr;
				return elementStr;
			}
			return accumulatorValue;
		},
		[](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence { return a + charsequence::Charsequence(",") + b; },
		[](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence {
			charsequence::Charsequence result = charsequence::Charsequence("[") + accumulatorValue + charsequence::Charsequence("]");
			std::cout << result << '\n';
			return result;
		});
}

template <typename E>
auto useOut(const charsequence::Charsequence &delimiter) -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence { return charsequence::Charsequence(); },
		[delimiter](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
					return accumulatorValue + delimiter + element;
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
					return accumulatorValue + delimiter + elementStr;
				return elementStr;
			}
			return accumulatorValue;
		},
		[](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence { return a + charsequence::Charsequence(",") + b; },
		[](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence {
			charsequence::Charsequence result = charsequence::Charsequence("[") + accumulatorValue + charsequence::Charsequence("]");
			std::cout << result << '\n';
			return result;
		});
}

template <typename E>
auto useOut(const charsequence::Charsequence &prefix, const charsequence::Charsequence &delimiter, const charsequence::Charsequence &suffix) -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence { return charsequence::Charsequence(); },
		[delimiter](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 1)
					return accumulatorValue + delimiter + element;
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 1)
					return accumulatorValue + delimiter + elementStr;
				return elementStr;
			}
			return accumulatorValue;
		},
		[delimiter](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence { return a + delimiter + b; },
		[prefix, suffix](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence {
			charsequence::Charsequence result = prefix + accumulatorValue + suffix;
			std::cout << result << '\n';
			return result;
		});
}

template <typename E>
auto useError() -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence { return charsequence::Charsequence(); },
		[](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
					return accumulatorValue + charsequence::Charsequence(",") + element;
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
					return accumulatorValue + charsequence::Charsequence(",") + elementStr;
				return elementStr;
			}
			return accumulatorValue;
		},
		[](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence { return a + charsequence::Charsequence(",") + b; },
		[](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence {
			charsequence::Charsequence result = charsequence::Charsequence("[") + accumulatorValue + charsequence::Charsequence("]");
			std::cerr << result << '\n';
			return result;
		});
}

template <typename E>
auto useError(const charsequence::Charsequence &delimiter) -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence { return charsequence::Charsequence(); },
		[delimiter](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
					return accumulatorValue + delimiter + element;
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
					return accumulatorValue + delimiter + elementStr;
				return elementStr;
			}
			return accumulatorValue;
		},
		[](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence { return a + charsequence::Charsequence(",") + b; },
		[](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence {
			charsequence::Charsequence result = charsequence::Charsequence("[") + accumulatorValue + charsequence::Charsequence("]");
			std::cerr << result << '\n';
			return result;
		});
}

template <typename E>
auto useError(const charsequence::Charsequence &prefix, const charsequence::Charsequence &delimiter, const charsequence::Charsequence &suffix) -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence { return charsequence::Charsequence(); },
		[delimiter](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 1)
					return accumulatorValue + delimiter + element;
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 1)
					return accumulatorValue + delimiter + elementStr;
				return elementStr;
			}
			return accumulatorValue;
		},
		[delimiter](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence { return a + delimiter + b; },
		[prefix, suffix](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence {
			charsequence::Charsequence result = accumulatorValue + suffix;
			std::cerr << result << '\n';
			return result;
		});
}

template <typename E>
auto useFrequency() -> Collector<E, std::unordered_map<E, std::complex<double>>, std::map<E, std::complex<double>>>
{
	return useFull<E, std::unordered_map<E, std::complex<double>>, std::map<E, std::complex<double>>>(
		[]() -> std::unordered_map<E, std::complex<double>> {
			return std::unordered_map<E, std::complex<double>>();
		},
		[](std::unordered_map<E, std::complex<double>> accumulatorValue, E element, function::Timestamp index) -> std::unordered_map<E, std::complex<double>> {
			double angle = 2.0 * pi * static_cast<double>(index);
			accumulatorValue[element] += std::complex<double>(std::cos(angle), std::sin(angle));
			return accumulatorValue;
		},
		[](std::unordered_map<E, std::complex<double>> a, std::unordered_map<E, std::complex<double>> b) -> std::unordered_map<E, std::complex<double>> {
			for (auto &[key, value] : b)
				a[key] += value;
			return a;
		},
		[](std::unordered_map<E, std::complex<double>> accumulatorValue) -> std::map<E, std::complex<double>> {
			return std::map<E, std::complex<double>>(accumulatorValue.begin(), accumulatorValue.end());
		});
}

template <typename E, typename D>
auto useFrequency(const function::Function<E, D> &mapper) -> Collector<E, std::unordered_map<D, std::complex<double>>, std::map<D, std::complex<double>>>
{
	return useFull<E, std::unordered_map<D, std::complex<double>>, std::map<D, std::complex<double>>>(
		[]() -> std::unordered_map<D, std::complex<double>> {
			return std::unordered_map<D, std::complex<double>>();
		},
		[mapper](std::unordered_map<D, std::complex<double>> accumulatorValue, E element, function::Timestamp index) -> std::unordered_map<D, std::complex<double>> {
			D key = mapper(element);
			double angle = 2.0 * pi * static_cast<double>(index);
			accumulatorValue[key] += std::complex<double>(std::cos(angle), std::sin(angle));
			return accumulatorValue;
		},
		[](std::unordered_map<D, std::complex<double>> a, std::unordered_map<D, std::complex<double>> b) -> std::unordered_map<D, std::complex<double>> {
			for (auto &[key, value] : b)
				a[key] += value;
			return a;
		},
		[](std::unordered_map<D, std::complex<double>> accumulatorValue) -> std::map<D, std::complex<double>> {
			return std::map<D, std::complex<double>>(accumulatorValue.begin(), accumulatorValue.end());
		});
}

template <typename E>
auto useDistribution() -> Collector<E, std::unordered_map<E, std::vector<function::Timestamp>>, std::map<E, std::complex<double>>>
{
	return useFull<E, std::unordered_map<E, std::vector<function::Timestamp>>, std::map<E, std::complex<double>>>(
		[]() -> std::unordered_map<E, std::vector<function::Timestamp>> {
			return std::unordered_map<E, std::vector<function::Timestamp>>();
		},
		[](std::unordered_map<E, std::vector<function::Timestamp>> accumulatorValue, E element, function::Timestamp index) -> std::unordered_map<E, std::vector<function::Timestamp>> {
			accumulatorValue[element].push_back(index);
			return accumulatorValue;
		},
		[](std::unordered_map<E, std::vector<function::Timestamp>> a, std::unordered_map<E, std::vector<function::Timestamp>> b) -> std::unordered_map<E, std::vector<function::Timestamp>> {
			for (auto &[key, indices] : b)
			{
				auto &target = a[key];
				target.insert(target.end(), indices.begin(), indices.end());
			}
			return a;
		},
		[](std::unordered_map<E, std::vector<function::Timestamp>> accumulatorValue) -> std::map<E, std::complex<double>> {
			std::map<E, std::complex<double>> result;
			function::Module totalSize = 0;
			for (const auto &[key, indices] : accumulatorValue)
				totalSize += indices.size();
			if (totalSize == 0)
				return result;
			for (auto &[key, indices] : accumulatorValue)
			{
				std::complex<double> sum(0.0, 0.0);
				for (function::Timestamp index : indices)
				{
					double distanceToHead = static_cast<double>(index);
					double distanceToTail = static_cast<double>(totalSize - 1 - index);
					double angle = 2.0 * pi * (distanceToHead - distanceToTail) / static_cast<double>(totalSize);
					sum += std::complex<double>(std::cos(angle), std::sin(angle));
				}
				result[key] = sum;
			}
			return result;
		});
}

template <typename E, typename D>
auto useDistribution(const function::Function<E, D> &mapper) -> Collector<E, std::unordered_map<D, std::vector<function::Timestamp>>, std::map<D, std::complex<double>>>
{
	return useFull<E, std::unordered_map<D, std::vector<function::Timestamp>>, std::map<D, std::complex<double>>>(
		[]() -> std::unordered_map<D, std::vector<function::Timestamp>> {
			return std::unordered_map<D, std::vector<function::Timestamp>>();
		},
		[mapper](std::unordered_map<D, std::vector<function::Timestamp>> accumulatorValue, E element, function::Timestamp index) -> std::unordered_map<D, std::vector<function::Timestamp>> {
			accumulatorValue[mapper(element)].push_back(index);
			return accumulatorValue;
		},
		[](std::unordered_map<D, std::vector<function::Timestamp>> a, std::unordered_map<D, std::vector<function::Timestamp>> b) -> std::unordered_map<D, std::vector<function::Timestamp>> {
			for (auto &[key, indices] : b)
			{
				auto &target = a[key];
				target.insert(target.end(), indices.begin(), indices.end());
			}
			return a;
		},
		[](std::unordered_map<D, std::vector<function::Timestamp>> accumulatorValue) -> std::map<D, std::complex<double>> {
			std::map<D, std::complex<double>> result;
			function::Module totalSize = 0;
			for (const auto &[key, indices] : accumulatorValue)
				totalSize += indices.size();
			if (totalSize == 0)
				return result;
			for (auto &[key, indices] : accumulatorValue)
			{
				std::complex<double> sum(0.0, 0.0);
				for (function::Timestamp index : indices)
				{
					double distanceToHead = static_cast<double>(index);
					double distanceToTail = static_cast<double>(totalSize - 1 - index);
					double angle = 2.0 * pi * (distanceToHead - distanceToTail) / static_cast<double>(totalSize);
					sum += std::complex<double>(std::cos(angle), std::sin(angle));
				}
				result[key] = sum;
			}
			return result;
		});
}

template <typename E>
auto usePartition(const function::Module &size) -> Collector<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>>
{
	return useFull<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>>(
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
auto usePartitionBy(KeyExtractor &&keyExtractor) -> Collector<E, std::map<function::Timestamp, std::vector<E>>, std::vector<std::vector<E>>>
{
	return useFull<E, std::map<function::Timestamp, std::vector<E>>, std::vector<std::vector<E>>>(
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

template <typename E, typename D>
auto useMedian() -> Collector<E, std::vector<D>, std::optional<D>>
{
	return useFull<E, std::vector<D>, std::optional<D>>(
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
auto useMedian(const function::Function<E, D> &mapper) -> Collector<E, std::vector<D>, std::optional<D>>
{
	return useFull<E, std::vector<D>, std::optional<D>>(
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
auto useMode() -> Collector<E, std::unordered_map<E, std::complex<double>>, std::optional<E>>
{
	return useFull<E, std::unordered_map<E, std::complex<double>>, std::optional<E>>(
		[]() -> std::unordered_map<E, std::complex<double>> { return std::unordered_map<E, std::complex<double>>(); },
		[](std::unordered_map<E, std::complex<double>> accumulatorValue, E element, function::Timestamp index) -> std::unordered_map<E, std::complex<double>> {
			double angle = 2.0 * pi * static_cast<double>(index);
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
auto usePercentile(double p) -> Collector<E, std::vector<D>, std::optional<D>>
{
	return useFull<E, std::vector<D>, std::optional<D>>(
		[]() -> std::vector<D> { return std::vector<D>(); },
		[](std::vector<D> accumulatorValue, E element, function::Timestamp index) -> std::vector<D> {
			accumulatorValue.push_back(static_cast<D>(element));
			return accumulatorValue;
		},
		[](std::vector<D> a, std::vector<D> b) -> std::vector<D> {
			a.insert(a.end(), b.begin(), b.end());
			return a;
		},
		[p](std::vector<D> accumulatorValue) -> std::optional<D> {
			if (accumulatorValue.empty() || p < 0.0 || p > 100.0)
				return std::nullopt;
			std::sort(accumulatorValue.begin(), accumulatorValue.end());
			double rank = p / 100.0 * static_cast<double>(accumulatorValue.size() - 1);
			std::size_t lowerIndex = static_cast<std::size_t>(std::floor(rank));
			std::size_t upperIndex = static_cast<std::size_t>(std::ceil(rank));
			if (lowerIndex == upperIndex)
				return std::optional<D>(accumulatorValue[lowerIndex]);
			double fraction = rank - static_cast<double>(lowerIndex);
			return std::optional<D>(accumulatorValue[lowerIndex] + fraction * (accumulatorValue[upperIndex] - accumulatorValue[lowerIndex]));
		});
}

template <typename E, typename D>
auto usePercentile(double p, const function::Function<E, D> &mapper) -> Collector<E, std::vector<D>, std::optional<D>>
{
	return useFull<E, std::vector<D>, std::optional<D>>(
		[]() -> std::vector<D> { return std::vector<D>(); },
		[mapper](std::vector<D> accumulatorValue, E element, function::Timestamp index) -> std::vector<D> {
			accumulatorValue.push_back(mapper(element));
			return accumulatorValue;
		},
		[](std::vector<D> a, std::vector<D> b) -> std::vector<D> {
			a.insert(a.end(), b.begin(), b.end());
			return a;
		},
		[p](std::vector<D> accumulatorValue) -> std::optional<D> {
			if (accumulatorValue.empty() || p < 0.0 || p > 100.0)
				return std::nullopt;
			std::sort(accumulatorValue.begin(), accumulatorValue.end());
			double rank = p / 100.0 * static_cast<double>(accumulatorValue.size() - 1);
			std::size_t lowerIndex = static_cast<std::size_t>(std::floor(rank));
			std::size_t upperIndex = static_cast<std::size_t>(std::ceil(rank));
			if (lowerIndex == upperIndex)
				return std::optional<D>(accumulatorValue[lowerIndex]);
			double fraction = rank - static_cast<double>(lowerIndex);
			return std::optional<D>(accumulatorValue[lowerIndex] + fraction * (accumulatorValue[upperIndex] - accumulatorValue[lowerIndex]));
		});
}

template <typename E>
auto useReduce(const function::BiFunction<E, E, E> &reducer) -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useFull<E, std::optional<E>, std::optional<E>>(
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
auto useReduce(const E &identity, const function::BiFunction<E, E, E> &reducer) -> Collector<E, E, E>
{
	return useFull<E, E, E>(
		[identity]() -> E { return identity; },
		[reducer](E accumulatorValue, E element, function::Timestamp index) -> E { return reducer(accumulatorValue, element); },
		[reducer](E a, E b) -> E { return reducer(a, b); },
		[](E accumulatorValue) -> E { return accumulatorValue; });
}

template <typename E, typename R>
auto useReduce(const R &identity, const function::BiFunction<R, E, R> &reducer, const function::BiFunction<R, R, R> &combiner, const function::Function<R, R> &finisher) -> Collector<E, R, R>
{
	return useFull<E, R, R>(
		[identity]() -> R { return identity; },
		[reducer](R accumulatorValue, E element, function::Timestamp index) -> R { return reducer(accumulatorValue, element); },
		[combiner](R a, R b) -> R { return combiner(a, b); },
		[finisher](R accumulatorValue) -> R { return finisher(accumulatorValue); });
}

template <typename E, typename K, typename KeyExtractor>
auto useToMap(KeyExtractor &&keyExtractor) -> Collector<E, std::map<K, E>, std::map<K, E>>
{
	return useFull<E, std::map<K, E>, std::map<K, E>>(
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
auto useToMap(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor) -> Collector<E, std::map<K, V>, std::map<K, V>>
{
	return useFull<E, std::map<K, V>, std::map<K, V>>(
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
auto useToUnorderedMap(const function::BiFunction<E, function::Timestamp, K> &keyExtractor, const function::BiFunction<E, function::Timestamp, V> &valueExtractor) -> Collector<E, std::unordered_map<K, V>, std::unordered_map<K, V>>
{
	return useFull<E, std::unordered_map<K, V>, std::unordered_map<K, V>>(
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
auto useToVector() -> Collector<E, std::vector<E>, std::vector<E>>
{
	return useFull<E, std::vector<E>, std::vector<E>>(
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
auto useToList() -> Collector<E, std::list<E>, std::list<E>>
{
	return useFull<E, std::list<E>, std::list<E>>(
		[]() -> std::list<E> { return std::list<E>(); },
		[](std::list<E> accumulatorValue, E element, function::Timestamp index) -> std::list<E> {
			accumulatorValue.push_back(element);
			return accumulatorValue;
		},
		[](std::list<E> a, std::list<E> b) -> std::list<E> { a.splice(a.end(), b); return a; },
		[](std::list<E> accumulatorValue) -> std::list<E> { return accumulatorValue; });
}

template <typename E>
auto useToSet() -> Collector<E, std::set<E>, std::set<E>>
{
	return useFull<E, std::set<E>, std::set<E>>(
		[]() -> std::set<E> { return std::set<E>(); },
		[](std::set<E> accumulatorValue, E element, function::Timestamp index) -> std::set<E> {
			accumulatorValue.insert(element);
			return accumulatorValue;
		},
		[](std::set<E> a, std::set<E> b) -> std::set<E> { a.insert(b.begin(), b.end()); return a; },
		[](std::set<E> accumulatorValue) -> std::set<E> { return accumulatorValue; });
}

template <typename E>
auto useToUnorderedSet() -> Collector<E, std::unordered_set<E>, std::unordered_set<E>>
{
	return useFull<E, std::unordered_set<E>, std::unordered_set<E>>(
		[]() -> std::unordered_set<E> { return std::unordered_set<E>(); },
		[](std::unordered_set<E> accumulatorValue, E element, function::Timestamp index) -> std::unordered_set<E> {
			accumulatorValue.insert(element);
			return accumulatorValue;
		},
		[](std::unordered_set<E> a, std::unordered_set<E> b) -> std::unordered_set<E> { a.insert(b.begin(), b.end()); return a; },
		[](std::unordered_set<E> accumulatorValue) -> std::unordered_set<E> { return accumulatorValue; });
}

template <typename E>
auto useDFT() -> Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>
{
	return useFull<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>(
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
auto useIDFT() -> Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>
{
	return useFull<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>(
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
auto useFFT() -> Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>
{
	return useFull<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>(
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
auto useIFFT() -> Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>
{
	return useFull<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>(
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
auto useGradient(const std::function<std::vector<double>(const std::vector<E> &)> &gradientFunction,
				 double learningRate, std::size_t maxIterations, double convergenceThreshold)
	-> Collector<E, std::vector<double>, std::vector<double>>
{
	return useFull<E, std::vector<double>, std::vector<double>>(
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
auto useGradient(const std::function<double(const std::vector<E> &)> &costFunction,
				 double learningRate, std::size_t maxIterations, double convergenceThreshold, double numericalH)
	-> Collector<E, std::vector<double>, std::vector<double>>
{
	return useFull<E, std::vector<double>, std::vector<double>>(
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
auto useStandardDeviation() -> Collector<E, std::pair<D, std::vector<D>>, D>
{
	return useFull<E, std::pair<D, std::vector<D>>, D>(
		[]() -> std::pair<D, std::vector<D>> { return std::make_pair(D{}, std::vector<D>()); },
		[](std::pair<D, std::vector<D>> accumulatorValue, E element, function::Timestamp index) -> std::pair<D, std::vector<D>> {
			D value = static_cast<D>(element);
			accumulatorValue.first += value;
			accumulatorValue.second.push_back(value);
			return accumulatorValue;
		},
		[](std::pair<D, std::vector<D>> a, std::pair<D, std::vector<D>> b) -> std::pair<D, std::vector<D>> {
			a.first += b.first;
			a.second.insert(a.second.end(), b.second.begin(), b.second.end());
			return a;
		},
		[](std::pair<D, std::vector<D>> accumulatorValue) -> D {
			if (accumulatorValue.second.empty())
				return D{};
			D mean = accumulatorValue.first / static_cast<D>(accumulatorValue.second.size());
			D sumOfSquares = D{};
			for (const D &value : accumulatorValue.second)
			{
				D diff = value - mean;
				sumOfSquares += diff * diff;
			}
			D variance = sumOfSquares / static_cast<D>(accumulatorValue.second.size());
			return static_cast<D>(std::sqrt(static_cast<double>(variance)));
		});
}

template <typename E, typename D>
auto useStandardDeviation(const function::Function<E, D> &mapper) -> Collector<E, std::pair<D, std::vector<D>>, D>
{
	return useFull<E, std::pair<D, std::vector<D>>, D>(
		[]() -> std::pair<D, std::vector<D>> { return std::make_pair(D{}, std::vector<D>()); },
		[mapper](std::pair<D, std::vector<D>> accumulatorValue, E element, function::Timestamp index) -> std::pair<D, std::vector<D>> {
			D value = mapper(element);
			accumulatorValue.first += value;
			accumulatorValue.second.push_back(value);
			return accumulatorValue;
		},
		[](std::pair<D, std::vector<D>> a, std::pair<D, std::vector<D>> b) -> std::pair<D, std::vector<D>> {
			a.first += b.first;
			a.second.insert(a.second.end(), b.second.begin(), b.second.end());
			return a;
		},
		[](std::pair<D, std::vector<D>> accumulatorValue) -> D {
			if (accumulatorValue.second.empty())
				return D{};
			D mean = accumulatorValue.first / static_cast<D>(accumulatorValue.second.size());
			D sumOfSquares = D{};
			for (const D &value : accumulatorValue.second)
			{
				D diff = value - mean;
				sumOfSquares += diff * diff;
			}
			D variance = sumOfSquares / static_cast<D>(accumulatorValue.second.size());
			return static_cast<D>(std::sqrt(static_cast<double>(variance)));
		});
}

template <typename E, typename D>
auto useSkewness() -> Collector<E, std::vector<D>, D>
{
	return useFull<E, std::vector<D>, D>(
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
auto useSkewness(const function::Function<E, D> &mapper) -> Collector<E, std::vector<D>, D>
{
	return useFull<E, std::vector<D>, D>(
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
auto useKurtosis() -> Collector<E, std::vector<D>, D>
{
	return useFull<E, std::vector<D>, D>(
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
auto useKurtosis(const function::Function<E, D> &mapper) -> Collector<E, std::vector<D>, D>
{
	return useFull<E, std::vector<D>, D>(
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
