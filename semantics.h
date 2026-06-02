#pragma once
#include "function.h"
#include "semantic.h"
#include "charsequence.h"
#include <string>
#include <vector>
#include <algorithm>
#include <istream>
#include <sstream>
#include <fstream>
#include <random>
#include <limits>
#include <cmath>
#include <unordered_set>
#include <type_traits>

namespace semantic
{
template <typename D>
auto useRange(const D &start, const D &end) -> Semantic<D>
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
	});
}

template <typename D>
auto useRange(const D &start, const D &end, const D &step) -> Semantic<D>
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
	});
}

template <typename D>
auto useRangeClosed(const D &start, const D &end) -> Semantic<D>
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
	});
}

template <typename D>
auto useRangeClosed(const D &start, const D &end, const D &step) -> Semantic<D>
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
	});
}

template <typename D, typename UnaryFunc,
		  typename = std::enable_if_t<std::is_invocable_r_v<D, UnaryFunc, const D &>>>
auto useInfinite(const D &seed, UnaryFunc &&generator) -> Semantic<D>
{
	static_assert(std::is_invocable_r_v<D, UnaryFunc, const D &>, "useInfinite: generator must be callable as D(const D&)");
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
	});
}

template <typename SupplierFunc,
		  typename D = std::invoke_result_t<SupplierFunc>,
		  typename = std::enable_if_t<std::is_invocable_r_v<D, SupplierFunc>>>
auto useGenerate(SupplierFunc &&supplier) -> Semantic<D>
{
	static_assert(std::is_invocable_r_v<D, SupplierFunc>, "useGenerate: supplier must be callable as D()");
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
	});
}

template <typename SupplierFunc,
		  typename D = std::invoke_result_t<SupplierFunc>,
		  typename = std::enable_if_t<std::is_invocable_r_v<D, SupplierFunc>>>
auto useGenerate(SupplierFunc &&supplier, const function::Module &limit) -> Semantic<D>
{
	static_assert(std::is_invocable_r_v<D, SupplierFunc>, "useGenerate: supplier must be callable as D()");
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
	});
}

template <typename D, typename UnaryFunc,
		  typename = std::enable_if_t<std::is_invocable_r_v<D, UnaryFunc, const D &>>>
auto useIterate(const D &seed, UnaryFunc &&generator) -> Semantic<D>
{
	static_assert(std::is_invocable_r_v<D, UnaryFunc, const D &>, "useIterate: generator must be callable as D(const D&)");
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
	});
}

template <typename D, typename UnaryFunc,
		  typename = std::enable_if_t<std::is_invocable_r_v<D, UnaryFunc, const D &>>>
auto useIterate(const D &seed, UnaryFunc &&generator, const function::Module &limit) -> Semantic<D>
{
	static_assert(std::is_invocable_r_v<D, UnaryFunc, const D &>, "useIterate: generator must be callable as D(const D&)");
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
	});
}

template <typename D>
auto useRandom() -> Semantic<D>
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
	});
}

template <typename D>
auto useRandom(const D &min, const D &max) -> Semantic<D>
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
	});
}

template <typename D>
auto useRandom(const D &min, const D &max, const function::Module &count) -> Semantic<D>
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
	});
}

template <typename D>
auto useEmpty() -> Semantic<D>
{
	return Semantic<D>([](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
		return;
	});
}

template <typename D>
auto useOf(D element) -> Semantic<D>
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
auto useOf(E element1, E element2) -> Semantic<E>
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
					   2LL);
}

template <typename E>
auto useOf(E element1, E element2, E element3) -> Semantic<E>
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
					   3LL);
}

template <typename Container>
auto useFrom(Container container) -> Semantic<typename Container::value_type>
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
auto useFrom(std::initializer_list<E> list) -> Semantic<E>
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
auto useOf(std::initializer_list<E> elements) -> Semantic<E>
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
auto useRepeat(const E &element, const function::Module &count) -> Semantic<E>
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

auto useBlob(const std::string &text) -> Semantic<char>
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

auto useBlob(const std::string &text, function::Module start, const function::Module end) -> Semantic<char>
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

auto useBlob(std::istream &stream) -> Semantic<std::string>
{
	return Semantic<std::string>([&stream](function::BiConsumer<std::string, function::Timestamp> accept, function::BiPredicate<std::string, function::Timestamp> interrupt) -> void {
		std::string line;
		function::Timestamp index = 0LL;
		while (std::getline(stream, line))
		{
			if (interrupt(line, index))
			{
				break;
			}
			accept(line, index);
			index++;
		}
	},
								 1LL);
}

auto useBlob(std::istream &stream, const char &delimiter) -> Semantic<std::string>
{
	return Semantic<std::string>([&stream, delimiter](function::BiConsumer<std::string, function::Timestamp> accept, function::BiPredicate<std::string, function::Timestamp> interrupt) -> void {
		std::string line;
		function::Timestamp index = 0LL;
		while (std::getline(stream, line, delimiter))
		{
			if (interrupt(line, index))
			{
				break;
			}
			accept(line, index);
			index++;
		}
	},
								 1LL);
}

auto useText(const std::string &text) -> Semantic<charsequence::Charsequence>
{
	return Semantic<charsequence::Charsequence>([sequence = charsequence::Charsequence(text)](function::BiConsumer<charsequence::Charsequence, function::Timestamp> accept, function::BiPredicate<charsequence::Charsequence, function::Timestamp> interrupt) -> void {
		if (!interrupt(sequence, 0LL))
		{
			accept(sequence, 0LL);
		}
	},
												1LL);
}

auto useText(const std::string &text, const char &delimiter) -> Semantic<charsequence::Charsequence>
{
	return Semantic<charsequence::Charsequence>([sequence = charsequence::Charsequence(text), delimiter](function::BiConsumer<charsequence::Charsequence, function::Timestamp> accept, function::BiPredicate<charsequence::Charsequence, function::Timestamp> interrupt) -> void {
		charsequence::Charsequence delim(std::string(1, delimiter));
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

auto useText(std::istream &stream) -> Semantic<charsequence::Charsequence>
{
	return Semantic<charsequence::Charsequence>([&stream](function::BiConsumer<charsequence::Charsequence, function::Timestamp> accept, function::BiPredicate<charsequence::Charsequence, function::Timestamp> interrupt) -> void {
		charsequence::Charsequence sequence(stream, std::numeric_limits<std::size_t>::max(), charsequence::charset::utf8);
		if (!interrupt(sequence, 0LL))
		{
			accept(sequence, 0LL);
		}
	},
												1LL);
}

auto useText(std::istream &stream, const char &delimiter) -> Semantic<charsequence::Charsequence>
{
	return Semantic<charsequence::Charsequence>([&stream, delimiter](function::BiConsumer<charsequence::Charsequence, function::Timestamp> accept, function::BiPredicate<charsequence::Charsequence, function::Timestamp> interrupt) -> void {
		charsequence::Charsequence delim(std::string(1, delimiter));
		charsequence::Charsequence sequence(stream, std::numeric_limits<std::size_t>::max(), charsequence::charset::utf8);
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

auto useSequence(const charsequence::Charsequence &sequence) -> Semantic<charsequence::Point>
{
	return Semantic<charsequence::Point>([sequence](function::BiConsumer<charsequence::Point, function::Timestamp> accept, function::BiPredicate<charsequence::Point, function::Timestamp> interrupt) -> void {
		std::vector<charsequence::Point> points = sequence.getPoints();
		function::Timestamp index = 0LL;
		for (const charsequence::Point &point : points)
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

auto useSequence(const charsequence::Charsequence &sequence, function::Module start, const function::Module end) -> Semantic<charsequence::Point>
{
	return Semantic<charsequence::Point>([sequence, start, end](function::BiConsumer<charsequence::Point, function::Timestamp> accept, function::BiPredicate<charsequence::Point, function::Timestamp> interrupt) -> void {
		std::vector<charsequence::Point> points = sequence.getPoints();
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

auto useSequence(std::istream &stream, charsequence::charset encoding = charsequence::charset::utf8) -> Semantic<charsequence::Point>
{
	return Semantic<charsequence::Point>([&stream, encoding](function::BiConsumer<charsequence::Point, function::Timestamp> accept, function::BiPredicate<charsequence::Point, function::Timestamp> interrupt) -> void {
		charsequence::Charsequence sequence(stream, std::numeric_limits<std::size_t>::max(), encoding);
		std::vector<charsequence::Point> points = sequence.getPoints();
		function::Timestamp index = 0LL;
		for (const charsequence::Point &point : points)
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

auto useSequence(const std::string &text, charsequence::charset encoding = charsequence::charset::utf8) -> Semantic<charsequence::Point>
{
	return Semantic<charsequence::Point>([text, encoding](function::BiConsumer<charsequence::Point, function::Timestamp> accept, function::BiPredicate<charsequence::Point, function::Timestamp> interrupt) -> void {
		charsequence::Charsequence sequence(text, encoding);
		std::vector<charsequence::Point> points = sequence.getPoints();
		function::Timestamp index = 0LL;
		for (const charsequence::Point &point : points)
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

auto useCharsequence(const charsequence::Charsequence &sequence) -> Semantic<charsequence::Charsequence>
{
	return Semantic<charsequence::Charsequence>([sequence](function::BiConsumer<charsequence::Charsequence, function::Timestamp> accept, function::BiPredicate<charsequence::Charsequence, function::Timestamp> interrupt) -> void {
		if (!interrupt(sequence, 0LL))
		{
			accept(sequence, 0LL);
		}
	},
												1LL);
}

auto useCharsequence(const charsequence::Charsequence &sequence, const charsequence::Charsequence &delimiter) -> Semantic<charsequence::Charsequence>
{
	return Semantic<charsequence::Charsequence>([sequence, delimiter](function::BiConsumer<charsequence::Charsequence, function::Timestamp> accept, function::BiPredicate<charsequence::Charsequence, function::Timestamp> interrupt) -> void {
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

auto useCharsequence(std::istream &stream, charsequence::charset encoding = charsequence::charset::utf8) -> Semantic<charsequence::Charsequence>
{
	return Semantic<charsequence::Charsequence>([&stream, encoding](function::BiConsumer<charsequence::Charsequence, function::Timestamp> accept, function::BiPredicate<charsequence::Charsequence, function::Timestamp> interrupt) -> void {
		charsequence::Charsequence sequence(stream, std::numeric_limits<std::size_t>::max(), encoding);
		if (!interrupt(sequence, 0LL))
		{
			accept(sequence, 0LL);
		}
	},
												1LL);
}

auto useCharsequence(std::istream &stream, const charsequence::Charsequence &delimiter, charsequence::charset encoding = charsequence::charset::utf8) -> Semantic<charsequence::Charsequence>
{
	return Semantic<charsequence::Charsequence>([&stream, delimiter, encoding](function::BiConsumer<charsequence::Charsequence, function::Timestamp> accept, function::BiPredicate<charsequence::Charsequence, function::Timestamp> interrupt) -> void {
		charsequence::Charsequence sequence(stream, std::numeric_limits<std::size_t>::max(), encoding);
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

} // namespace semantic
