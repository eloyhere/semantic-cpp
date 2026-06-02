#pragma once
#include <cstdint>
#include <random>
#include <functional>
#include <iostream>
#include <algorithm>
#include <type_traits>

namespace function
{
using Timestamp = long long;

using Module = unsigned long long;

using Runnable = std::function<void()>;

template <typename R>
using Supplier = std::function<R()>;

template <typename T, typename R>
using Function = std::function<R(T)>;

template <typename T, typename U, typename R>
using BiFunction = std::function<R(T, U)>;

template <typename T, typename U, typename V, typename R>
using TriFunction = std::function<R(T, U, V)>;

template <typename T>
using Unary = std::function<T(T)>;

template <typename T>
using Binary = std::function<T(T, T)>;

template <typename T>
using Consumer = std::function<void(T)>;

template <typename T, typename U>
using BiConsumer = std::function<void(T, U)>;

template <typename T, typename U, typename V>
using TriConsumer = std::function<void(T, U, V)>;

template <typename T>
using Predicate = std::function<bool(T)>;

template <typename T, typename U>
using BiPredicate = std::function<bool(T, U)>;

template <typename T, typename U, typename V>
using TriPredicate = std::function<bool(T, U, V)>;

template <typename T>
using Comparator = std::function<int(const T &, const T &)>;

template <typename T>
using Generator = BiConsumer<BiConsumer<T, Timestamp>, BiPredicate<T, Timestamp>>;

template <typename D>
D randomly(const D &start, const D &end)
{
	try
	{
		static std::random_device random;
		static std::mt19937_64 generator(random());

		D maximum = std::max(start, end);
		D minimum = std::min(start, end);

		if constexpr (std::is_integral<D>::value)
		{
			std::uniform_int_distribution<D> distribution(minimum, maximum);
			return distribution(generator);
		}
		else
		{
			std::uniform_real_distribution<D> distribution(minimum, maximum);
			return distribution(generator);
		}
	}
	catch (const std::exception &exception)
	{
		std::cerr << exception.what() << '\n';
		return 0;
	}
}

bool randomly()
{
	try
	{
		static std::random_device device;
		static std::mt19937 generator(device());
		std::bernoulli_distribution distribution(0.5);
		return distribution(generator);
	}
	catch (const std::exception &exception)
	{
		std::cerr << exception.what() << '\n';
	}
	return 0;
}

} // namespace function