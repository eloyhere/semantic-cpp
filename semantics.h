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

namespace semantic
{
template <typename D>
auto useRange(const D &start, const D &end) -> Semantic<D>
{
	return Semantic<D>([startValue = std::min(start, end), endValue = std::max(start, end)](function::BiConsumer<D, function::Timestamp> accept, function::BiPredicate<D, function::Timestamp> interrupt) -> void {
		for (D index = startValue; index < endValue; index++)
		{
			if (interrupt(index, index))
			{
				break;
			}
			accept(index, index);
		}
	});
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
	return Semantic<E>([elements = std::move(list)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
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
	return Semantic<E>([elements = std::move(elements)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
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

auto useText(const std::string &text) -> Semantic<std::string>
{
	return Semantic<std::string>([text](function::BiConsumer<std::string, function::Timestamp> accept, function::BiPredicate<std::string, function::Timestamp> interrupt) -> void {
		if (!interrupt(text, 0LL))
		{
			accept(text, 0LL);
		}
	},
								 1LL);
}

auto useText(const std::string &text, const char &delimiter) -> Semantic<std::string>
{
	return Semantic<std::string>([text, delimiter](function::BiConsumer<std::string, function::Timestamp> accept, function::BiPredicate<std::string, function::Timestamp> interrupt) -> void {
		std::vector<std::string> pool;
		for (function::Module a = 0; a < text.size(); a++)
		{
			std::string target;
			for (function::Module b = a; b < text.size(); b++)
			{
				if (text[b] == delimiter)
				{
					a = b;
					break;
				}
				target += text[b];
			}
			pool.push_back(target);
			function::Module index = pool.size() - 1;
			if (interrupt(target, index))
			{
				break;
			}
			accept(target, index);
		}
	},
								 1LL);
}

auto useText(std::istream &stream) -> Semantic<std::string>
{
	return Semantic<std::string>([&stream](function::BiConsumer<std::string, function::Timestamp> accept, function::BiPredicate<std::string, function::Timestamp> interrupt) -> void {
		std::string content;
		std::string line;
		function::Timestamp index = 0LL;
		while (std::getline(stream, line))
		{
			content += line;
			if (!stream.eof())
			{
				content += '\n';
			}
		}
		if (!interrupt(content, 0LL))
		{
			accept(content, 0LL);
		}
	},
								 1LL);
}

auto useText(std::istream &stream, const char &delimiter) -> Semantic<std::string>
{
	return Semantic<std::string>([&stream, delimiter](function::BiConsumer<std::string, function::Timestamp> accept, function::BiPredicate<std::string, function::Timestamp> interrupt) -> void {
		std::string token;
		function::Timestamp index = 0LL;
		while (std::getline(stream, token, delimiter))
		{
			if (interrupt(token, index))
			{
				break;
			}
			accept(token, index);
			index++;
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
		std::string delimiterStr(delimiter.getBytes().begin(), delimiter.getBytes().end());
		std::string sequenceStr(sequence.getBytes().begin(), sequence.getBytes().end());
		std::vector<std::string> pool;
		for (function::Module a = 0; a < sequenceStr.size(); a++)
		{
			std::string target;
			bool matched = false;
			for (function::Module b = a; b < sequenceStr.size(); b++)
			{
				if (b + delimiterStr.size() <= sequenceStr.size() && sequenceStr.substr(b, delimiterStr.size()) == delimiterStr)
				{
					a = b + delimiterStr.size() - 1;
					matched = true;
					break;
				}
				target += sequenceStr[b];
			}
			if (!target.empty() || matched)
			{
				pool.push_back(target);
				function::Module index = pool.size() - 1;
				charsequence::Charsequence targetSequence(target);
				if (interrupt(targetSequence, index))
				{
					break;
				}
				accept(targetSequence, index);
			}
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
		std::string delimiterStr(delimiter.getBytes().begin(), delimiter.getBytes().end());
		std::string sequenceStr(sequence.getBytes().begin(), sequence.getBytes().end());
		std::vector<std::string> pool;
		for (function::Module a = 0; a < sequenceStr.size(); a++)
		{
			std::string target;
			bool matched = false;
			for (function::Module b = a; b < sequenceStr.size(); b++)
			{
				if (b + delimiterStr.size() <= sequenceStr.size() && sequenceStr.substr(b, delimiterStr.size()) == delimiterStr)
				{
					a = b + delimiterStr.size() - 1;
					matched = true;
					break;
				}
				target += sequenceStr[b];
			}
			if (!target.empty() || matched)
			{
				pool.push_back(target);
				function::Module index = pool.size() - 1;
				charsequence::Charsequence targetSequence(target);
				if (interrupt(targetSequence, index))
				{
					break;
				}
				accept(targetSequence, index);
			}
		}
	},
												1LL);
}

} // namespace semantics
