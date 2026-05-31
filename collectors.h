#pragma once
#include "collector.h"
#include "charsequence.h"
#include <cmath>
#include <complex>
#include <numbers>
#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <functional>

namespace collector
{
template <typename E, typename Predicate>
auto useAllMatch(Predicate &&predicate) -> Collector<E, bool, bool>
{
	return useShortable<E, bool, bool>(
		[]() -> bool {
			return true;
		},
		[predicate](E element, function::Timestamp index, bool accumulator) -> bool {
			return !accumulator;
		},
		[predicate](bool accumulator, E element, function::Timestamp index) -> bool {
			if constexpr (std::is_invocable_r_v<bool, Predicate, E, function::Timestamp>)
			{
				return accumulator && std::invoke(predicate, element, index);
			}
			else if (std::is_invocable_r_v<bool, Predicate, E>)
			{
				return accumulator && std::invoke(predicate, element);
			}
			return false;
		},
		[](bool a, bool b) -> bool {
			return a && b;
		},
		[](bool accumulator) -> bool {
			return accumulator;
		});
}

template <typename E, typename Predicate>
auto useAnyMatch(Predicate &&predicate) -> Collector<E, bool, bool>
{
	return useShortable<E, bool, bool>(
		[]() -> bool {
			return false;
		},
		[predicate](E element, function::Timestamp index, bool accumulator) -> bool {
			return accumulator;
		},
		[predicate](bool accumulator, E element, function::Timestamp index) -> bool {
			if constexpr (std::is_invocable_r_v<bool, Predicate, E, function::Timestamp>)
			{
				return accumulator || std::invoke(predicate, element, index);
			}
			else if constexpr (std::is_invocable_r_v<bool, Predicate, E>)
			{
				return accumulator || std::invoke(predicate, element);
			}
			return false;
		},
		[](bool a, bool b) -> bool {
			return a || b;
		},
		[](bool accumulator) -> bool {
			return accumulator;
		});
}

template <typename E, typename Predicate>
auto useNoneMatch(Predicate &&predicate) -> Collector<E, bool, bool>
{
	return useShortable<E, bool, bool>(
		[]() -> bool {
			return true;
		},
		[predicate](E element, function::Timestamp index, bool accumulator) -> bool {
			return !accumulator;
		},
		[predicate](bool accumulator, E element, function::Timestamp index) -> bool {
			if constexpr (std::is_invocable_r_v<bool, Predicate, E, function::Timestamp>)
			{
				return accumulator && !std::invoke(predicate, element, index);
			}
			else if constexpr (std::is_invocable_r_v<bool, Predicate, E>)
			{
				return accumulator && !std::invoke(predicate, element);
			}
			return false;
		},
		[](bool a, bool b) -> bool {
			return a && b;
		},
		[](bool accumulator) -> bool {
			return accumulator;
		});
}

template <typename E, typename Consumer>
auto useForEach(Consumer &&consumer) -> Collector<E, function::Module, function::Module>
{
	return useFull<E, function::Module, function::Module>(
		[]() -> function::Module {
			return 0;
		},
		[consumer](const function::Module &accumulator, const E &element, const function::Timestamp &index) -> function::Module {
			if constexpr (std::is_invocable_r_v<void, Consumer, E, function::Timestamp>)
			{
				std::invoke(consumer, element, index);
				return accumulator + 1;
			}
			else if constexpr (std::is_invocable_r_v<void, Consumer, E>)
			{
				std::invoke(consumer, element);
				return accumulator + 1;
			}
			return accumulator;
		},
		[](const function::Module &a, const function::Module &b) -> function::Module {
			return a + b;
		},
		[](const function::Module &identityValue) -> function::Module {
			return identityValue;
		});
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

template <typename E>
auto useCount() -> Collector<E, function::Module, function::Module>
{
	return useFull<E, function::Module, function::Module>(
		[]() -> function::Module {
			return 0LL;
		},
		[](function::Module accumulatorValue, E element, function::Timestamp index) -> function::Module {
			return accumulatorValue + 1;
		},
		[](function::Module a, function::Module b) -> function::Module {
			return a + b;
		},
		[](function::Module accumulatorValue) -> function::Module {
			return accumulatorValue;
		});
}

template <typename E>
auto useError() -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence {
			return charsequence::Charsequence();
		},
		[](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + charsequence::Charsequence(",") + element;
				}
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + charsequence::Charsequence(",") + elementStr;
				}
				return elementStr;
			}
			else
			{
				return accumulatorValue;
			}
		},
		[](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence {
			return a + charsequence::Charsequence(",") + b;
		},
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
		[]() -> charsequence::Charsequence {
			return charsequence::Charsequence();
		},
		[delimiter](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + delimiter + element;
				}
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + delimiter + elementStr;
				}
				return elementStr;
			}
			else
			{
				return accumulatorValue;
			}
		},
		[](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence {
			return a + charsequence::Charsequence(",") + b;
		},
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
		[]() -> charsequence::Charsequence {
			return charsequence::Charsequence();
		},
		[delimiter](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 1)
				{
					return accumulatorValue + delimiter + element;
				}
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 1)
				{
					return accumulatorValue + delimiter + elementStr;
				}
				return elementStr;
			}
			else
			{
				return accumulatorValue;
			}
		},
		[delimiter](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence {
			return a + delimiter + b;
		},
		[prefix, suffix](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence {
			charsequence::Charsequence result = accumulatorValue + suffix;
			std::cerr << result << '\n';
			return result;
		});
}

template <typename E>
auto useFindAny() -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useShortable<E, std::optional<E>, std::optional<E>>(
		[]() -> std::optional<E> {
			return std::nullopt;
		},
		[](E element, function::Timestamp index, std::optional<E> accumulatorValue) -> bool {
			return accumulatorValue.has_value();
		},
		[](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
			if (function::randomly())
			{
				return std::optional<E>(element);
			}
			return std::nullopt;
		},
		[](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
			if (a.has_value())
			{
				return a;
			}
			if (b.has_value())
			{
				return b;
			}
			return std::nullopt;
		},
		[](std::optional<E> accumulatorValue) -> std::optional<E> {
			return accumulatorValue;
		});
}

template <typename E>
auto useFindAt(const function::Timestamp &index) -> Collector<E, std::optional<E>, std::optional<E>>
{
	if (index < 0LL)
	{
		throw std::invalid_argument("use useFindNegativeAt for negative index.");
	}
	const function::Timestamp target = index;
	return useShortable<E, std::optional<E>, std::optional<E>>(
		[]() -> std::optional<E> {
			return std::nullopt;
		},
		[](E element, function::Timestamp index, std::optional<E> accumulatorValue) -> bool {
			return accumulatorValue.has_value();
		},
		[target](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
			if (target == index)
			{
				return std::optional<E>(element);
			}
			return accumulatorValue;
		},
		[](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
			if (a.has_value())
			{
				return a;
			}
			if (b.has_value())
			{
				return b;
			}
			return std::nullopt;
		},
		[](std::optional<E> accumulatorValue) -> std::optional<E> {
			return accumulatorValue;
		});
}

template <typename E>
auto useFindFirst() -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useShortable<E, std::optional<E>, std::optional<E>>(
		[]() -> std::optional<E> {
			return std::nullopt;
		},
		[](E element, function::Timestamp index, std::optional<E> accumulatorValue) -> bool {
			return accumulatorValue.has_value();
		},
		[](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
			if (accumulatorValue.has_value())
			{
				return accumulatorValue;
			}
			return std::optional<E>(element);
		},
		[](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
			if (a.has_value())
			{
				return a;
			}
			if (b.has_value())
			{
				return b;
			}
			return std::nullopt;
		},
		[](std::optional<E> accumulatorValue) -> std::optional<E> {
			return accumulatorValue;
		});
}

template <typename E>
auto useFindLast() -> Collector<E, std::vector<E>, std::optional<E>>
{
	return useFull<E, std::vector<E>, std::optional<E>>(
		[]() -> std::vector<E> {
			return std::vector<E>();
		},
		[](std::vector<E> accumulatorValue, E element, function::Timestamp index) -> std::vector<E> {
			accumulatorValue.push_back(element);
			return accumulatorValue;
		},
		[](std::vector<E> a, std::vector<E> b) -> std::vector<E> {
			for (const E &element : b)
			{
				a.push_back(element);
			}
			return a;
		},
		[](std::vector<E> accumulatorValue) -> std::optional<E> {
			if (accumulatorValue.empty())
			{
				return std::nullopt;
			}
			return std::optional<E>(accumulatorValue[accumulatorValue.size() - 1]);
		});
}

template <typename E>
auto useFindMaximum() -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useFull<E, std::optional<E>, std::optional<E>>(
		[]() -> std::optional<E> {
			return std::nullopt;
		},
		[](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
			if (accumulatorValue.has_value())
			{
				if (element > accumulatorValue.value())
				{
					return std::optional<E>(element);
				}
				return accumulatorValue;
			}
			return std::optional<E>(element);
		},
		[](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
			if (a.has_value())
			{
				if (b.has_value())
				{
					return a.value() > b.value() ? a : b;
				}
				return a;
			}
			if (b.has_value())
			{
				return b;
			}
			return std::nullopt;
		},
		[](std::optional<E> accumulatorValue) -> std::optional<E> {
			return accumulatorValue;
		});
}

template <typename E>
auto useFindMaximum(const function::Comparator<E> &comparator) -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useFull<E, std::optional<E>, std::optional<E>>(
		[]() -> std::optional<E> {
			return std::nullopt;
		},
		[comparator](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
			if (accumulatorValue.has_value())
			{
				if (comparator(element, accumulatorValue.value()) > 0)
				{
					return std::optional<E>(element);
				}
				return accumulatorValue;
			}
			return std::optional<E>(element);
		},
		[comparator](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
			if (a.has_value())
			{
				if (b.has_value())
				{
					return comparator(a.value(), b.value()) > 0 ? a : b;
				}
				return a;
			}
			if (b.has_value())
			{
				return b;
			}
			return std::nullopt;
		},
		[](std::optional<E> accumulatorValue) -> std::optional<E> {
			return accumulatorValue;
		});
}

template <typename E>
auto useFindMinimum() -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useFull<E, std::optional<E>, std::optional<E>>(
		[]() -> std::optional<E> {
			return std::nullopt;
		},
		[](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
			if (accumulatorValue.has_value())
			{
				if (element < accumulatorValue.value())
				{
					return std::optional<E>(element);
				}
				return accumulatorValue;
			}
			return std::optional<E>(element);
		},
		[](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
			if (a.has_value())
			{
				if (b.has_value())
				{
					return a.value() < b.value() ? a : b;
				}
				return a;
			}
			if (b.has_value())
			{
				return b;
			}
			return std::nullopt;
		},
		[](std::optional<E> accumulatorValue) -> std::optional<E> {
			return accumulatorValue;
		});
}

template <typename E>
auto useFindMinimum(const function::Comparator<E> &comparator) -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useFull<E, std::optional<E>, std::optional<E>>(
		[]() -> std::optional<E> {
			return std::nullopt;
		},
		[comparator](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
			if (accumulatorValue.has_value())
			{
				if (comparator(element, accumulatorValue.value()) < 0)
				{
					return std::optional<E>(element);
				}
				return accumulatorValue;
			}
			return std::optional<E>(element);
		},
		[comparator](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
			if (a.has_value())
			{
				if (b.has_value())
				{
					return comparator(a.value(), b.value()) < 0 ? a : b;
				}
				return a;
			}
			if (b.has_value())
			{
				return b;
			}
			return std::nullopt;
		},
		[](std::optional<E> accumulatorValue) -> std::optional<E> {
			return accumulatorValue;
		});
}

template <typename E, typename K, typename KeyExtractor>
auto useGroup(KeyExtractor &&keyExtractor) -> Collector<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>>
{
	return useFull<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>>(
		[]() -> std::unordered_map<K, std::vector<E>> {
			return std::unordered_map<K, std::vector<E>>();
		},
		[keyExtractor](std::unordered_map<K, std::vector<E>> accumulatorValue, E element, function::Timestamp index) -> std::unordered_map<K, std::vector<E>> {
			if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp>)
			{
				K key = std::invoke(keyExtractor, element, index);
				if (accumulatorValue.contains(key))
				{
					accumulatorValue[key].push_back(element);
					return accumulatorValue;
				}
				std::vector<E> group;
				group.push_back(element);
				accumulatorValue.insert(std::pair<K, std::vector<E>>(key, group));
				return accumulatorValue;
			}
			else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E>)
			{
				K key = std::invoke(keyExtractor, element);
				if (accumulatorValue.contains(key))
				{
					accumulatorValue[key].push_back(element);
					return accumulatorValue;
				}
				std::vector<E> group;
				group.push_back(element);
				accumulatorValue.insert(std::pair<K, std::vector<E>>(key, group));
				return accumulatorValue;
			}
			return accumulatorValue;
		},
		[](std::unordered_map<K, std::vector<E>> a, std::unordered_map<K, std::vector<E>> b) -> std::unordered_map<K, std::vector<E>> {
			for (std::pair<K, std::vector<E>> pair : b)
			{
				if (a.contains(pair.first))
				{
					std::vector<E> &groupA = a[pair.first];
					std::vector<E> &groupB = pair.second;
					groupA.reserve(groupA.size() + groupB.size());
					groupA.insert(groupA.end(), groupB.begin(), groupB.end());
				}
				else
				{
					a[pair.first] = std::move(pair.second);
				}
			}
			return a;
		},
		[](std::unordered_map<K, std::vector<E>> accumulatorValue) -> std::unordered_map<K, std::vector<E>> {
			return accumulatorValue;
		});
}

template <typename E>
auto useJoin() -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence {
			return charsequence::Charsequence();
		},
		[](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + charsequence::Charsequence(",") + element;
				}
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + charsequence::Charsequence(",") + elementStr;
				}
				return elementStr;
			}
			else
			{
				return accumulatorValue;
			}
		},
		[](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence {
			return a + charsequence::Charsequence(",") + b;
		},
		[](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence {
			return charsequence::Charsequence("[") + accumulatorValue + charsequence::Charsequence("]");
		});
}

template <typename E>
auto useJoin(const charsequence::Charsequence &delimiter) -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence {
			return charsequence::Charsequence();
		},
		[delimiter](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + delimiter + element;
				}
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + delimiter + elementStr;
				}
				return elementStr;
			}
			else
			{
				return accumulatorValue;
			}
		},
		[](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence {
			return a + charsequence::Charsequence(",") + b;
		},
		[](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence {
			return charsequence::Charsequence("[") + accumulatorValue + charsequence::Charsequence("]");
		});
}

template <typename E>
auto useJoin(const charsequence::Charsequence &prefix, const charsequence::Charsequence &delimiter, const charsequence::Charsequence &suffix) -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence {
			return charsequence::Charsequence();
		},
		[delimiter](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + delimiter + element;
				}
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + delimiter + elementStr;
				}
				return elementStr;
			}
			else
			{
				return accumulatorValue;
			}
		},
		[delimiter](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence {
			return a + delimiter + b;
		},
		[prefix, suffix](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence {
			return prefix + accumulatorValue + suffix;
		});
}

template <typename E>
auto useOut() -> Collector<E, charsequence::Charsequence, charsequence::Charsequence>
{
	return useFull<E, charsequence::Charsequence, charsequence::Charsequence>(
		[]() -> charsequence::Charsequence {
			return charsequence::Charsequence();
		},
		[](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + charsequence::Charsequence(",") + element;
				}
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + charsequence::Charsequence(",") + elementStr;
				}
				return elementStr;
			}
			else
			{
				return accumulatorValue;
			}
		},
		[](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence {
			return a + charsequence::Charsequence(",") + b;
		},
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
		[]() -> charsequence::Charsequence {
			return charsequence::Charsequence();
		},
		[delimiter](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + delimiter + element;
				}
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 0)
				{
					return accumulatorValue + delimiter + elementStr;
				}
				return elementStr;
			}
			else
			{
				return accumulatorValue;
			}
		},
		[](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence {
			return a + charsequence::Charsequence(",") + b;
		},
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
		[]() -> charsequence::Charsequence {
			return charsequence::Charsequence();
		},
		[delimiter](charsequence::Charsequence accumulatorValue, E element, function::Timestamp index) -> charsequence::Charsequence {
			if constexpr (std::is_same_v<E, charsequence::Charsequence>)
			{
				if (accumulatorValue.size() > 1)
				{
					return accumulatorValue + delimiter + element;
				}
				return element;
			}
			else if constexpr (std::is_arithmetic_v<E>)
			{
				charsequence::Charsequence elementStr(std::to_string(element));
				if (accumulatorValue.size() > 1)
				{
					return accumulatorValue + delimiter + elementStr;
				}
				return elementStr;
			}
			else
			{
				return accumulatorValue;
			}
		},
		[delimiter](charsequence::Charsequence a, charsequence::Charsequence b) -> charsequence::Charsequence {
			return a + delimiter + b;
		},
		[prefix, suffix](charsequence::Charsequence accumulatorValue) -> charsequence::Charsequence {
			charsequence::Charsequence result = prefix + accumulatorValue + suffix;
			std::cout << result << '\n';
			return result;
		});
}

template <typename E>
auto usePartition(const function::Module &size) -> Collector<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>>
{
	return useFull<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>>(
		[]() -> std::vector<std::vector<E>> {
			return std::vector<std::vector<E>>();
		},
		[size](std::vector<std::vector<E>> accumulatorValue, E element, function::Timestamp index) -> std::vector<std::vector<E>> {
			if (size == 0 || size == 1)
			{
				std::vector<E> single = {element};
				accumulatorValue.push_back(single);
				return accumulatorValue;
			}
			if (accumulatorValue.empty())
			{
				std::vector<E> partition = {element};
				accumulatorValue.push_back(partition);
				return accumulatorValue;
			}
			std::vector<E> &last = accumulatorValue.back();
			if (last.size() < size)
			{
				last.push_back(element);
			}
			else
			{
				std::vector<E> newPartition = {element};
				accumulatorValue.push_back(newPartition);
			}
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
				function::Module takeCount = std::min(canTake, firstB.size());
				for (size_t i = 0; i < takeCount; i++)
				{
					lastA.push_back(firstB[i]);
				}
				if (takeCount == firstB.size())
				{
					b.erase(b.begin());
				}
				else
				{
					firstB.erase(firstB.begin(), firstB.begin() + takeCount);
				}
			}
			a.insert(a.end(), b.begin(), b.end());
			return a;
		},
		[](std::vector<std::vector<E>> accumulatorValue) -> std::vector<std::vector<E>> {
			return accumulatorValue;
		});
}

template <typename E, typename KeyExtractor>
auto usePartitionBy(KeyExtractor &&keyExtractor) -> Collector<E, std::map<function::Timestamp, std::vector<E>>, std::vector<std::vector<E>>>
{
	return useFull<E, std::map<function::Timestamp, std::vector<E>>, std::vector<std::vector<E>>>(
		[]() -> std::map<function::Timestamp, std::vector<E>> {
			return std::map<function::Timestamp, std::vector<E>>();
		},
		[keyExtractor](std::map<function::Timestamp, std::vector<E>> accumulatorValue, E element, function::Timestamp index) -> std::map<function::Timestamp, std::vector<E>> {
			if constexpr (std::is_invocable_r_v<function::Timestamp, KeyExtractor, E, function::Timestamp>)
			{
				function::Timestamp key = std::invoke(keyExtractor, element, index);
				accumulatorValue[key].push_back(element);
				return accumulatorValue;
			}
			else if constexpr (std::is_invocable_r_v<function::Timestamp, KeyExtractor, E>)
			{
				function::Timestamp key = std::invoke(keyExtractor, element);
				accumulatorValue[key].push_back(element);
				return accumulatorValue;
			}
			return accumulatorValue;
		},
		[](std::map<function::Timestamp, std::vector<E>> a, std::map<function::Timestamp, std::vector<E>> b) -> std::map<function::Timestamp, std::vector<E>> {
			for (auto &[key, vec] : b)
			{
				for (E element : vec)
				{
					a[key].push_back(std::move(element));
				}
			}
			return a;
		},
		[](std::map<function::Timestamp, std::vector<E>> accumulatorValue) -> std::vector<std::vector<E>> {
			std::vector<std::vector<E>> result;
			result.reserve(accumulatorValue.size());
			for (auto &[key, vec] : accumulatorValue)
			{
				result.push_back(std::move(vec));
			}
			return result;
		});
}

template <typename E>
auto useReduce(const function::BiFunction<E, E, E> &reducer) -> Collector<E, std::optional<E>, std::optional<E>>
{
	return useFull<E, std::optional<E>, std::optional<E>>(
		[]() -> std::optional<E> {
			return std::nullopt;
		},
		[reducer](std::optional<E> accumulatorValue, E element, function::Timestamp index) -> std::optional<E> {
			if (!accumulatorValue.has_value())
			{
				return std::optional<E>(element);
			}
			return std::optional<E>(reducer(accumulatorValue.value(), element));
		},
		[reducer](std::optional<E> a, std::optional<E> b) -> std::optional<E> {
			if (!a.has_value())
				return b;
			if (!b.has_value())
				return a;
			return std::optional<E>(reducer(a.value(), b.value()));
		},
		[](std::optional<E> accumulatorValue) -> std::optional<E> {
			return accumulatorValue;
		});
}

template <typename E>
auto useReduce(const E &identity, const function::BiFunction<E, E, E> &reducer) -> Collector<E, E, E>
{
	return useFull<E, E, E>(
		[identity]() -> E {
			return identity;
		},
		[reducer](E accumulatorValue, E element, function::Timestamp index) -> E {
			return reducer(accumulatorValue, element);
		},
		[reducer](E a, E b) -> E {
			return reducer(a, b);
		},
		[](E accumulatorValue) -> E {
			return accumulatorValue;
		});
}

template <typename E, typename R>
auto useReduce(const R &identity, const function::BiFunction<R, E, R> &reducer, const function::BiFunction<R, R, R> &combiner, const function::Function<R, R> &finisher) -> Collector<E, R, R>
{
	return useFull<E, R, R>(
		[identity]() -> R {
			return identity;
		},
		[reducer](R accumulatorValue, E element, function::Timestamp index) -> R {
			return reducer(accumulatorValue, element);
		},
		[combiner](R a, R b) -> R {
			return combiner(a, b);
		},
		[finisher](R accumulatorValue) -> R {
			return finisher(accumulatorValue);
		});
}

template <typename E, typename K, typename KeyExtractor>
auto useToMap(KeyExtractor &&keyExtractor) -> Collector<E, std::map<K, E>, std::map<K, E>>
{
	return useFull<E, std::map<K, E>, std::map<K, E>>(
		[]() -> std::map<K, E> {
			return std::map<K, E>();
		},
		[keyExtractor](std::map<K, E> accumulatorValue, E element, function::Timestamp index) -> std::map<K, E> {
			if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp>)
			{
				K key = std::invoke(keyExtractor, element, index);
				accumulatorValue[key] = element;
				return accumulatorValue;
			}
			else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E>)
			{
				K key = std::invoke(keyExtractor, element);
				accumulatorValue[key] = element;
				return accumulatorValue;
			}
			return accumulatorValue;
		},
		[](std::map<K, E> a, std::map<K, E> b) -> std::map<K, E> {
			for (const auto &[key, value] : b)
			{
				a[key] = value;
			}
			return a;
		},
		[](std::map<K, E> accumulatorValue) -> std::map<K, E> {
			return accumulatorValue;
		});
}

template <typename E, typename K, typename V, typename KeyExtractor, typename ValueExtractor>
auto useToMap(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor) -> Collector<E, std::map<K, V>, std::map<K, V>>
{
	return useFull<E, std::map<K, V>, std::map<K, V>>(
		[]() -> std::map<K, V> {
			return std::map<K, V>();
		},
		[keyExtractor, valueExtractor](std::map<K, V> accumulatorValue, E element, function::Timestamp index) -> std::map<K, V> {
			if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp> && std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
			{
				K key = std::invoke(keyExtractor, element, index);
				V value = std::invoke(valueExtractor, element, index);
				accumulatorValue[key] = value;
				return accumulatorValue;
			}
			else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E> && std::is_invocable_r_v<V, ValueExtractor, E>)
			{
				K key = std::invoke(keyExtractor, element);
				V value = std::invoke(valueExtractor, element);
				accumulatorValue[key] = value;
				return accumulatorValue;
			}
			return accumulatorValue;
		},
		[](std::map<K, V> a, std::map<K, V> b) -> std::map<K, V> {
			for (const auto &[key, value] : b)
			{
				a[key] = value;
			}
			return a;
		},
		[](std::map<K, V> accumulatorValue) -> std::map<K, V> {
			return accumulatorValue;
		});
}

template <typename E, typename K, typename V>
auto useToUnorderedMap(const function::BiFunction<E, function::Timestamp, K> &keyExtractor, const function::BiFunction<E, function::Timestamp, V> &valueExtractor) -> Collector<E, std::unordered_map<K, V>, std::unordered_map<K, V>>
{
	return useFull<E, std::unordered_map<K, V>, std::unordered_map<K, V>>(
		[]() -> std::unordered_map<K, V> {
			return std::unordered_map<K, V>();
		},
		[keyExtractor, valueExtractor](std::unordered_map<K, V> accumulatorValue, E element, function::Timestamp index) -> std::unordered_map<K, V> {
			K key = keyExtractor(element, index);
			V value = valueExtractor(element, index);
			accumulatorValue[key] = value;
			return accumulatorValue;
		},
		[](std::unordered_map<K, V> a, std::unordered_map<K, V> b) -> std::unordered_map<K, V> {
			for (const auto &[key, value] : b)
			{
				a[key] = value;
			}
			return a;
		},
		[](std::unordered_map<K, V> accumulatorValue) -> std::unordered_map<K, V> {
			return accumulatorValue;
		});
}

template <typename E>
auto useToSet() -> Collector<E, std::set<E>, std::set<E>>
{
	return useFull<E, std::set<E>, std::set<E>>(
		[]() -> std::set<E> {
			return std::set<E>();
		},
		[](std::set<E> accumulatorValue, E element, function::Timestamp index) -> std::set<E> {
			accumulatorValue.insert(element);
			return accumulatorValue;
		},
		[](std::set<E> a, std::set<E> b) -> std::set<E> {
			a.insert(b.begin(), b.end());
			return a;
		},
		[](std::set<E> accumulatorValue) -> std::set<E> {
			return accumulatorValue;
		});
}

template <typename E>
auto useToUnorderedSet() -> Collector<E, std::unordered_set<E>, std::unordered_set<E>>
{
	return useFull<E, std::unordered_set<E>, std::unordered_set<E>>(
		[]() -> std::unordered_set<E> {
			return std::unordered_set<E>();
		},
		[](std::unordered_set<E> accumulatorValue, E element, function::Timestamp index) -> std::unordered_set<E> {
			accumulatorValue.insert(element);
			return accumulatorValue;
		},
		[](std::unordered_set<E> a, std::unordered_set<E> b) -> std::unordered_set<E> {
			a.insert(b.begin(), b.end());
			return a;
		},
		[](std::unordered_set<E> accumulatorValue) -> std::unordered_set<E> {
			return accumulatorValue;
		});
}

template <typename E>
auto useToVector() -> Collector<E, std::vector<E>, std::vector<E>>
{
	return useFull<E, std::vector<E>, std::vector<E>>(
		[]() -> std::vector<E> {
			return std::vector<E>();
		},
		[](std::vector<E> accumulatorValue, E element, function::Timestamp index) -> std::vector<E> {
			accumulatorValue.push_back(element);
			return accumulatorValue;
		},
		[](std::vector<E> a, std::vector<E> b) -> std::vector<E> {
			a.insert(a.end(), b.begin(), b.end());
			return a;
		},
		[](std::vector<E> accumulatorValue) -> std::vector<E> {
			return accumulatorValue;
		});
}

template <typename E>
auto useToList() -> Collector<E, std::list<E>, std::list<E>>
{
	return useFull<E, std::list<E>, std::list<E>>(
		[]() -> std::list<E> {
			return std::list<E>();
		},
		[](std::list<E> accumulatorValue, E element, function::Timestamp index) -> std::list<E> {
			accumulatorValue.push_back(element);
			return accumulatorValue;
		},
		[](std::list<E> a, std::list<E> b) -> std::list<E> {
			a.splice(a.end(), b);
			return a;
		},
		[](std::list<E> accumulatorValue) -> std::list<E> {
			return accumulatorValue;
		});
}

template <typename E, typename D>
auto useSummate(const function::Function<E, D> &mapper) -> Collector<E, D, D>
{
	return useFull<E, D, D>(
		[]() -> D {
			return D{};
		},
		[mapper](D accumulatorValue, E element, function::Timestamp index) -> D {
			return accumulatorValue + mapper(element);
		},
		[](D a, D b) -> D {
			return a + b;
		},
		[](D accumulatorValue) -> D {
			return accumulatorValue;
		});
}

template <typename E, typename D>
auto useAverage() -> Collector<E, std::pair<D, function::Module>, D>
{
	return useFull<E, std::pair<D, function::Module>, D>(
		[]() -> std::pair<D, function::Module> {
			return std::make_pair(D{}, 0);
		},
		[](std::pair<D, function::Module> accumulatorValue, E element, function::Timestamp index) -> std::pair<D, function::Module> {
			D value = static_cast<D>(element);
			return std::make_pair(accumulatorValue.first + value, accumulatorValue.second + 1);
		},
		[](std::pair<D, function::Module> a, std::pair<D, function::Module> b) -> std::pair<D, function::Module> {
			return std::make_pair(a.first + b.first, a.second + b.second);
		},
		[](std::pair<D, function::Module> accumulatorValue) -> D {
			if (accumulatorValue.second == 0)
			{
				return D{};
			}
			return accumulatorValue.first / static_cast<D>(accumulatorValue.second);
		});
}

template <typename E, typename D>
auto useAverage(const function::Function<E, D> &mapper) -> Collector<E, std::pair<D, function::Module>, D>
{
	return useFull<E, std::pair<D, function::Module>, D>(
		[]() -> std::pair<D, function::Module> {
			return std::make_pair(D{}, 0);
		},
		[mapper](std::pair<D, function::Module> accumulatorValue, E element, function::Timestamp index) -> std::pair<D, function::Module> {
			D value = mapper(element);
			return std::make_pair(accumulatorValue.first + value, accumulatorValue.second + 1);
		},
		[](std::pair<D, function::Module> a, std::pair<D, function::Module> b) -> std::pair<D, function::Module> {
			return std::make_pair(a.first + b.first, a.second + b.second);
		},
		[](std::pair<D, function::Module> accumulatorValue) -> D {
			if (accumulatorValue.second == 0)
			{
				return D{};
			}
			return accumulatorValue.first / static_cast<D>(accumulatorValue.second);
		});
}

template <typename E, typename D>
auto useRange() -> Collector<E, std::pair<D, D>, D>
{
	return useFull<E, std::pair<D, D>, D>(
		[]() -> std::pair<D, D> {
			return std::make_pair<D, D>(D{}, D{});
		},
		[](std::pair<D, D> accumulatorValue, E element, function::Timestamp index) -> std::pair<D, D> {
			D mapped = static_cast<D>(element);
			if (accumulatorValue.first == D{})
			{
				return std::make_pair<D, D>(mapped, accumulatorValue.second);
			}
			if (accumulatorValue.second == D{})
			{
				return std::make_pair<D, D>(accumulatorValue.first, mapped);
			}
			if (mapped < accumulatorValue.first)
			{
				return std::make_pair<D, D>(mapped, accumulatorValue.second);
			}
			if (mapped > accumulatorValue.second)
			{
				return std::make_pair<D, D>(accumulatorValue.first, mapped);
			}
			return accumulatorValue;
		},
		[](std::pair<D, D> a, std::pair<D, D> b) -> std::pair<D, D> {
			D first = a.first;
			D second = a.second;
			if (first == D{} || (b.first != D{} && b.first < first))
				first = b.first;
			if (second == D{} || (b.second != D{} && b.second > second))
				second = b.second;
			return std::make_pair(first, second);
		},
		[](std::pair<D, D> accumulatorValue) -> D {
			if (accumulatorValue.first == D{} && accumulatorValue.second == D{})
			{
				return D{};
			}
			return std::abs(accumulatorValue.second - accumulatorValue.first);
		});
}

template <typename E, typename D>
auto useRange(const function::Function<E, D> &mapper) -> Collector<E, std::pair<D, D>, D>
{
	return useFull<E, std::pair<D, D>, D>(
		[]() -> std::pair<D, D> {
			return std::make_pair<D, D>(D{}, D{});
		},
		[mapper](std::pair<D, D> accumulatorValue, E element, function::Timestamp index) -> std::pair<D, D> {
			D mapped = mapper(element);
			if (accumulatorValue.first == D{})
			{
				return std::make_pair<D, D>(mapped, accumulatorValue.second);
			}
			if (accumulatorValue.second == D{})
			{
				return std::make_pair<D, D>(accumulatorValue.first, mapped);
			}
			if (mapped < accumulatorValue.first)
			{
				return std::make_pair<D, D>(mapped, accumulatorValue.second);
			}
			if (mapped > accumulatorValue.second)
			{
				return std::make_pair<D, D>(accumulatorValue.first, mapped);
			}
			return accumulatorValue;
		},
		[](std::pair<D, D> a, std::pair<D, D> b) -> std::pair<D, D> {
			D first = a.first;
			D second = a.second;
			if (first == D{} || (b.first != D{} && b.first < first))
				first = b.first;
			if (second == D{} || (b.second != D{} && b.second > second))
				second = b.second;
			return std::make_pair(first, second);
		},
		[](std::pair<D, D> accumulatorValue) -> D {
			if (accumulatorValue.first == D{} && accumulatorValue.second == D{})
			{
				return D{};
			}
			return std::abs(accumulatorValue.second - accumulatorValue.first);
		});
}

template <typename E>
auto useFrequency() -> Collector<E, std::map<E, function::Module>, std::map<E, function::Module>>
{
	return useFull<E, std::map<E, function::Module>, std::map<E, function::Module>>(
		[]() -> std::map<E, function::Module> {
			return std::map<E, function::Module>();
		},
		[](std::map<E, function::Module> accumulatorValue, E element, function::Timestamp index) -> std::map<E, function::Module> {
			accumulatorValue[element]++;
			return accumulatorValue;
		},
		[](std::map<E, function::Module> a, std::map<E, function::Module> b) -> std::map<E, function::Module> {
			for (const auto &[key, count] : b)
			{
				a[key] += count;
			}
			return a;
		},
		[](std::map<E, function::Module> accumulatorValue) -> std::map<E, function::Module> {
			return accumulatorValue;
		});
}

template <typename E, typename D>
auto useFrequency(const function::Function<E, D> &mapper) -> Collector<E, std::map<D, function::Module>, std::map<D, function::Module>>
{
	return useFull<E, std::map<D, function::Module>, std::map<D, function::Module>>(
		[]() -> std::map<D, function::Module> {
			return std::map<D, function::Module>();
		},
		[mapper](std::map<D, function::Module> accumulatorValue, E element, function::Timestamp index) -> std::map<D, function::Module> {
			D key = mapper(element);
			accumulatorValue[key]++;
			return accumulatorValue;
		},
		[](std::map<D, function::Module> a, std::map<D, function::Module> b) -> std::map<D, function::Module> {
			for (const auto &[key, count] : b)
			{
				a[key] += count;
			}
			return a;
		},
		[](std::map<D, function::Module> accumulatorValue) -> std::map<D, function::Module> {
			return accumulatorValue;
		});
}

template <typename E>
auto useDFT() -> Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>
{
	return useFull<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>>(
		[]() -> std::vector<std::complex<double>> {
			return std::vector<std::complex<double>>();
		},
		[](std::vector<std::complex<double>> accumulatorValue, E element, function::Timestamp index) -> std::vector<std::complex<double>> {
			if constexpr (std::is_arithmetic_v<E>)
			{
				accumulatorValue.push_back(std::complex<double>(static_cast<double>(element), 0.0));
			}
			else if constexpr (std::is_same_v<E, std::complex<double>>)
			{
				accumulatorValue.push_back(element);
			}
			else
			{
				accumulatorValue.push_back(std::complex<double>(0.0, 0.0));
			}
			return accumulatorValue;
		},
		[](std::vector<std::complex<double>> a, std::vector<std::complex<double>> b) -> std::vector<std::complex<double>> {
			a.insert(a.end(), b.begin(), b.end());
			return a;
		},
		[](std::vector<std::complex<double>> accumulatorValue) -> std::vector<std::complex<double>> {
			std::size_t N = accumulatorValue.size();
			if (N == 0)
			{
				return accumulatorValue;
			}
			std::vector<std::complex<double>> result(N);
			for (std::size_t k = 0; k < N; ++k)
			{
				std::complex<double> sum(0.0, 0.0);
				for (std::size_t n = 0; n < N; ++n)
				{
					double angle = -2.0 * std::numbers::pi * static_cast<double>(k * n) / static_cast<double>(N);
					std::complex<double> twiddle(std::cos(angle), std::sin(angle));
					sum += accumulatorValue[n] * twiddle;
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
		[]() -> std::vector<std::complex<double>> {
			return std::vector<std::complex<double>>();
		},
		[](std::vector<std::complex<double>> accumulatorValue, E element, function::Timestamp index) -> std::vector<std::complex<double>> {
			if constexpr (std::is_arithmetic_v<E>)
			{
				accumulatorValue.push_back(std::complex<double>(static_cast<double>(element), 0.0));
			}
			else if constexpr (std::is_same_v<E, std::complex<double>>)
			{
				accumulatorValue.push_back(element);
			}
			else
			{
				accumulatorValue.push_back(std::complex<double>(0.0, 0.0));
			}
			return accumulatorValue;
		},
		[](std::vector<std::complex<double>> a, std::vector<std::complex<double>> b) -> std::vector<std::complex<double>> {
			a.insert(a.end(), b.begin(), b.end());
			return a;
		},
		[](std::vector<std::complex<double>> accumulatorValue) -> std::vector<std::complex<double>> {
			std::size_t N = accumulatorValue.size();
			if (N == 0)
			{
				return accumulatorValue;
			}
			std::vector<std::complex<double>> result(N);
			for (std::size_t k = 0; k < N; ++k)
			{
				std::complex<double> sum(0.0, 0.0);
				for (std::size_t n = 0; n < N; ++n)
				{
					double angle = 2.0 * std::numbers::pi * static_cast<double>(k * n) / static_cast<double>(N);
					std::complex<double> twiddle(std::cos(angle), std::sin(angle));
					sum += accumulatorValue[n] * twiddle;
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
		[]() -> std::vector<std::complex<double>> {
			return std::vector<std::complex<double>>();
		},
		[](std::vector<std::complex<double>> accumulatorValue, E element, function::Timestamp index) -> std::vector<std::complex<double>> {
			if constexpr (std::is_arithmetic_v<E>)
			{
				accumulatorValue.push_back(std::complex<double>(static_cast<double>(element), 0.0));
			}
			else if constexpr (std::is_same_v<E, std::complex<double>>)
			{
				accumulatorValue.push_back(element);
			}
			else
			{
				accumulatorValue.push_back(std::complex<double>(0.0, 0.0));
			}
			return accumulatorValue;
		},
		[](std::vector<std::complex<double>> a, std::vector<std::complex<double>> b) -> std::vector<std::complex<double>> {
			a.insert(a.end(), b.begin(), b.end());
			return a;
		},
		[](std::vector<std::complex<double>> accumulatorValue) -> std::vector<std::complex<double>> {
			std::size_t N = accumulatorValue.size();
			if (N == 0)
			{
				return accumulatorValue;
			}
			std::size_t paddedSize = 1;
			while (paddedSize < N)
			{
				paddedSize <<= 1;
			}
			std::vector<std::complex<double>> data = accumulatorValue;
			data.resize(paddedSize, std::complex<double>(0.0, 0.0));
			std::size_t bits = 0;
			std::size_t temp = paddedSize;
			while (temp > 1)
			{
				temp >>= 1;
				bits++;
			}
			for (std::size_t i = 0; i < paddedSize; ++i)
			{
				std::size_t reversed = 0;
				for (std::size_t bit = 0; bit < bits; ++bit)
				{
					if (i & (1 << bit))
					{
						reversed |= (1 << (bits - 1 - bit));
					}
				}
				if (i < reversed)
				{
					std::swap(data[i], data[reversed]);
				}
			}
			for (std::size_t length = 2; length <= paddedSize; length <<= 1)
			{
				double angle = -2.0 * std::numbers::pi / static_cast<double>(length);
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
		[]() -> std::vector<std::complex<double>> {
			return std::vector<std::complex<double>>();
		},
		[](std::vector<std::complex<double>> accumulatorValue, E element, function::Timestamp index) -> std::vector<std::complex<double>> {
			if constexpr (std::is_arithmetic_v<E>)
			{
				accumulatorValue.push_back(std::complex<double>(static_cast<double>(element), 0.0));
			}
			else if constexpr (std::is_same_v<E, std::complex<double>>)
			{
				accumulatorValue.push_back(element);
			}
			else
			{
				accumulatorValue.push_back(std::complex<double>(0.0, 0.0));
			}
			return accumulatorValue;
		},
		[](std::vector<std::complex<double>> a, std::vector<std::complex<double>> b) -> std::vector<std::complex<double>> {
			a.insert(a.end(), b.begin(), b.end());
			return a;
		},
		[](std::vector<std::complex<double>> accumulatorValue) -> std::vector<std::complex<double>> {
			std::size_t N = accumulatorValue.size();
			if (N == 0)
			{
				return accumulatorValue;
			}
			std::size_t paddedSize = 1;
			while (paddedSize < N)
			{
				paddedSize <<= 1;
			}
			std::vector<std::complex<double>> data = accumulatorValue;
			data.resize(paddedSize, std::complex<double>(0.0, 0.0));
			std::size_t bits = 0;
			std::size_t temp = paddedSize;
			while (temp > 1)
			{
				temp >>= 1;
				bits++;
			}
			for (std::size_t i = 0; i < paddedSize; ++i)
			{
				std::size_t reversed = 0;
				for (std::size_t bit = 0; bit < bits; ++bit)
				{
					if (i & (1 << bit))
					{
						reversed |= (1 << (bits - 1 - bit));
					}
				}
				if (i < reversed)
				{
					std::swap(data[i], data[reversed]);
				}
			}
			for (std::size_t length = 2; length <= paddedSize; length <<= 1)
			{
				double angle = 2.0 * std::numbers::pi / static_cast<double>(length);
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
			{
				data[i] /= static_cast<double>(N);
			}
			return data;
		});
}

template <typename E>
auto useGradient(const std::function<std::vector<double>(const std::vector<E> &)> &gradientFunction,
				 double learningRate,
				 std::size_t maxIterations,
				 double convergenceThreshold) -> Collector<E, std::vector<double>, std::vector<double>>
{
	return useFull<E, std::vector<double>, std::vector<double>>(
		[]() -> std::vector<double> {
			return std::vector<double>();
		},
		[](std::vector<double> accumulatorValue, E element, function::Timestamp index) -> std::vector<double> {
			if constexpr (std::is_arithmetic_v<E>)
			{
				accumulatorValue.push_back(static_cast<double>(element));
			}
			else
			{
				accumulatorValue.push_back(0.0);
			}
			return accumulatorValue;
		},
		[](std::vector<double> a, std::vector<double> b) -> std::vector<double> {
			a.insert(a.end(), b.begin(), b.end());
			return a;
		},
		[gradientFunction, learningRate, maxIterations, convergenceThreshold](std::vector<double> accumulatorValue) -> std::vector<double> {
			if (accumulatorValue.empty())
			{
				return accumulatorValue;
			}
			std::vector<double> parameters = accumulatorValue;
			std::size_t dimensions = parameters.size();
			for (std::size_t iteration = 0; iteration < maxIterations; ++iteration)
			{
				std::vector<E> currentParameters;
				currentParameters.reserve(dimensions);
				for (double value : parameters)
				{
					if constexpr (std::is_arithmetic_v<E>)
					{
						currentParameters.push_back(static_cast<E>(value));
					}
					else
					{
						currentParameters.push_back(E{});
					}
				}
				std::vector<double> gradients = gradientFunction(currentParameters);
				if (gradients.size() != dimensions)
				{
					throw std::invalid_argument("Gradient function returned incorrect number of dimensions");
				}
				double maxGradientMagnitude = 0.0;
				for (std::size_t i = 0; i < dimensions; ++i)
				{
					parameters[i] -= learningRate * gradients[i];
					double gradientMagnitude = std::abs(gradients[i]);
					if (gradientMagnitude > maxGradientMagnitude)
					{
						maxGradientMagnitude = gradientMagnitude;
					}
				}
				if (maxGradientMagnitude < convergenceThreshold)
				{
					break;
				}
			}
			return parameters;
		});
}

template <typename E>
auto useGradient(const std::function<double(const std::vector<E> &)> &costFunction,
				 double learningRate,
				 std::size_t maxIterations,
				 double convergenceThreshold,
				 double numericalH) -> Collector<E, std::vector<double>, std::vector<double>>
{
	return useFull<E, std::vector<double>, std::vector<double>>(
		[]() -> std::vector<double> {
			return std::vector<double>();
		},
		[](std::vector<double> accumulatorValue, E element, function::Timestamp index) -> std::vector<double> {
			if constexpr (std::is_arithmetic_v<E>)
			{
				accumulatorValue.push_back(static_cast<double>(element));
			}
			else
			{
				accumulatorValue.push_back(0.0);
			}
			return accumulatorValue;
		},
		[](std::vector<double> a, std::vector<double> b) -> std::vector<double> {
			a.insert(a.end(), b.begin(), b.end());
			return a;
		},
		[costFunction, learningRate, maxIterations, convergenceThreshold, numericalH](std::vector<double> accumulatorValue) -> std::vector<double> {
			if (accumulatorValue.empty())
			{
				return accumulatorValue;
			}
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
						{
							forwardParameters.push_back(static_cast<E>(value));
						}
						else
						{
							forwardParameters.push_back(E{});
						}
					}
					double costForward = costFunction(forwardParameters);
					parameters[i] = originalValue - numericalH;
					std::vector<E> backwardParameters;
					backwardParameters.reserve(dimensions);
					for (double value : parameters)
					{
						if constexpr (std::is_arithmetic_v<E>)
						{
							backwardParameters.push_back(static_cast<E>(value));
						}
						else
						{
							backwardParameters.push_back(E{});
						}
					}
					double costBackward = costFunction(backwardParameters);
					gradients[i] = (costForward - costBackward) / (2.0 * numericalH);
					parameters[i] = originalValue;
				}
				double maxGradientMagnitude = 0.0;
				for (std::size_t i = 0; i < dimensions; ++i)
				{
					parameters[i] -= learningRate * gradients[i];
					double gradientMagnitude = std::abs(gradients[i]);
					if (gradientMagnitude > maxGradientMagnitude)
					{
						maxGradientMagnitude = gradientMagnitude;
					}
				}
				if (maxGradientMagnitude < convergenceThreshold)
				{
					break;
				}
			}
			return parameters;
		});
}

} // namespace collector
