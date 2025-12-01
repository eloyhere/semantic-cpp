#pragma once
#ifndef SEMANTIC_H
#define SEMANTIC_H
#include <set>
#include <list>
#include <map>
#include <array>
#include <string>
#include <vector>
#include <cmath>
#include <random>
#include <sstream>
#include <optional>
#include <memory>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <initializer_list>
#include <unordered_set>
namespace semantic
{
typedef long long Timestamp;

typedef unsigned long long Module;

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

template <typename T, typename U>
using Comparator = std::function<int(T, U)>;

template <typename T>
using Generator = TriConsumer<Consumer<T>, Predicate<T>, BiFunction<T, Timestamp, Timestamp>>;

template <typename D>
D randomly(const D &start, const D &end);

template <typename E, typename D>
class Statistics
{
  protected:
	const E *elements;

	Module size;

	mutable std::map<std::string, D> cache;

	mutable std::map<D, Module> frequencyCache;

  public:
	Statistics(const E *elements, const Module &size) : elements(elements), size(size)
	{
		static_assert(std::is_arithmetic<D>::value, "Statistical operations require arithmetic types.");
	}

	Statistics(const std::vector<E> &elements) : elements(elements.data()), size(elements.size())
	{
		static_assert(std::is_arithmetic<D>::value, "Statistical operations require arithmetic types.");
	}

	Statistics(const std::list<E> &elements) : elements(nullptr), size(elements.size())
	{
		static_assert(std::is_arithmetic<D>::value, "Statistical operations require arithmetic types.");
		std::vector<E> temp(elements.begin(), elements.end());
		this->elements = new E[temp.size()];
		std::copy(temp.begin(), temp.end(), const_cast<E *>(this->elements));
	}

	Statistics(std::initializer_list<E> initList) : elements(initList.begin()), size(initList.size())
	{
		static_assert(std::is_arithmetic<D>::value, "Statistical operations require arithmetic types.");
	}

	Statistics(const Statistics &other) : elements(other.elements), size(other.size), cache(other.cache)
	{
		static_assert(std::is_arithmetic<D>::value, "Statistical operations require arithmetic types.");
	}

	Statistics(Statistics &&other) noexcept : elements(other.elements), size(other.size), cache(std::move(other.cache))
	{
		static_assert(std::is_arithmetic<D>::value, "Statistical operations require arithmetic types.");
		other.elements = nullptr;
		other.size = 0;
	}

	~Statistics()
	{
		if (elements && size > 0)
		{
			delete[] elements;
		}
	}

	Module count() const;

	D maximum() const;

	D minimum() const;

	D range() const;

	D variance() const;

	D standardDeviation() const;

	D mean() const;

	D median() const;

	D mode() const;

	std::map<D, Module> frequency() const;

	D sum() const;

	std::vector<D> quartiles() const;

	D interquartileRange() const;

	D skewness() const;

	D kurtosis() const;

	bool isEmpty() const;

	void clear();

	Statistics &operator=(const std::list<E> &l);

	Statistics &operator=(const std::vector<E> &v);

	Statistics &operator=(std::initializer_list<E> l);

	Statistics &operator=(const Statistics &other);

	Statistics &operator=(Statistics &&other) noexcept;
};

template <typename E, typename A, typename R>
class Collector
{
  public:
	const Supplier<A> supplier;

	const BiConsumer<A, E> accumulator;

	const BiFunction<A, A, A> combiner;

	const Function<A, R> finisher;

	Collector(const Supplier<A> &supplier, const BiConsumer<A, E> &accumulator, const BiFunction<A, A, A> &combiner, const Function<A, R> &finisher) : supplier(supplier), accumulator(accumulator), combiner(combiner), finisher(finisher) {}
};

template <typename E>
class Semantic;

template <typename E>
Semantic<E> empty();

template <typename E, typename... Args>
Semantic<E> of(Args &&... args);

template <typename E>
Semantic<E> fill(const E &element, const Module &count);

template <typename E>
Semantic<E> fill(const Supplier<E> &supplier, const Module &count);

template <typename E>
Semantic<E> from(const E *array, const Module &length);

template <typename E>
Semantic<E> fromUnordered(const E *array, const Module &length);

template <typename E>
Semantic<E> fromOrdered(const E *array, const Module &length);

template <typename E, Module length>
Semantic<E> from(const std::array<E, length> &array);

template <typename E, Module length>
Semantic<E> fromUnordered(const std::array<E, length> &array);

template <typename E, Module length>
Semantic<E> fromOrdered(const std::array<E, length> &array);

template <typename E>
Semantic<E> from(const std::list<E> &l);

template <typename E>
Semantic<E> fromUnordered(const std::list<E> &l);

template <typename E>
Semantic<E> fromOrdered(const std::list<E> &l);

template <typename E>
Semantic<E> from(const std::vector<E> &v);

template <typename E>
Semantic<E> fromUnordered(const std::vector<E> &v);

template <typename E>
Semantic<E> fromOrdered(const std::vector<E> &v);

template <typename E>
Semantic<E> from(const std::set<E> &s);

template <typename E>
Semantic<E> from(const std::unordered_set<E> &s);

template <typename E>
Semantic<E> iterate(const Generator<E> &generator);

template <typename E>
Semantic<E> range(const E &start, const E &end);

template <typename E>
Semantic<E> range(const E &start, const E &end, const E &step);

template <typename E>
class Semantic
{
  protected:
	std::shared_ptr<Generator<E>> generator;

	Module concurrent;

  public:
	Semantic() : generator(std::make_shared<Generator<E>>([](const Consumer<E> &, const Predicate<E> &, const BiFunction<E, Timestamp, Timestamp> &) {})), concurrent(1) {}

	Semantic(const Module &concurrent) : generator(std::make_shared<Generator<E>>([](const Consumer<E> &, const Predicate<E> &, const BiFunction<E, Timestamp, Timestamp> &) {})), concurrent(concurrent) {}

	Semantic(const Generator<E> &generator) : generator(std::make_shared<Generator<E>>(generator)) {}

	Semantic(const Generator<E> &generator, const Module &concurrent) : generator(std::make_shared<Generator<E>>(generator)), concurrent(concurrent) {}

	Semantic(std::shared_ptr<Generator<E>> generator) : generator(generator) {}

	Semantic(std::shared_ptr<Generator<E>> generator, const Module &concurrent) : generator(generator), concurrent(concurrent) {}

	Semantic(const Semantic &other) : generator(other.generator), concurrent(other.concurrent) {}

	Semantic(Semantic &&other) noexcept : generator(std::move(other.generator)), concurrent(other.concurrent) {}

	Semantic &operator=(const Semantic &other);

	Semantic &operator=(Semantic &&other) noexcept;

	bool anyMatch(const Predicate<E> &p) const;

	bool allMatch(const Predicate<E> &p) const;

	template <typename A, typename R>
	R collect(const Supplier<A> &supplier, const BiConsumer<A, E> &accumulator, const BiFunction<A, A, A> &combiner, const Function<A, R> &finisher) const;

	template <typename A, typename R>
	R collect(const Collector<E, A, R> &c) const;

	Semantic<E> concat(const Semantic<E> &other) const;

	void cout() const;

	void cout(const BiFunction<E, std::ostream &, std::ostream &> &accumulator) const;

	void cout(const std::ostream &stream) const;

	void cout(const std::ostream &stream, const BiFunction<E, std::ostream &, std::ostream &> &accumulator) const;

	Module count() const;

	Semantic<E> distinct() const;

	Semantic<E> distinct(const BiPredicate<E, E> &comparator) const;

	Semantic<E> dropWhile(const Predicate<E> &p) const;

	void forEach(const Consumer<E> &c) const;

	Semantic<E> filter(const Predicate<E> &p) const;

	std::optional<E> findFirst() const;

	std::optional<E> findAny() const;

	Semantic<E> flat(const Function<E, Semantic<E>> &mapper) const;

	template <typename R>
	Semantic<R> flatMap(const Function<E, Semantic<R>> &mapper) const;

	bool noneMatch(const Predicate<E> &p) const;

	template <typename K>
	std::map<K, std::vector<E>> group(const Function<E, K> &classifier) const;

	template <typename K, typename V>
	std::map<K, std::vector<V>> groupBy(const Function<E, K> &keyExtractor, const Function<E, V> &valueExtractor) const;

	Semantic<E> limit(const Module &n) const;

	template <typename R>
	Semantic<R> map(const Function<E, R> &mapper) const;

	Semantic<E> offset(const Function<E, Timestamp> &n) const;

	Semantic<E> parallel() const;

	Semantic<E> parallel(const Module &threadCount) const;

	std::vector<std::vector<E>> partition(const Module &count) const;

	std::vector<std::vector<E>> partitionBy(const Function<E, Module> &classifier) const;

	Semantic<E> peek(const Consumer<E> &c) const;

	Semantic<E> redirect(const BiFunction<E, Timestamp, Timestamp> &redirector) const;

	std::optional<E> reduce(const BiFunction<E, E, E> &accumulator) const;

	E reduce(const E &identity, const BiFunction<E, E, E> &accumulator) const;

	template <typename R>
	R reduce(const R &identity, const BiFunction<R, E, R> &accumulator) const;

	Semantic<E> reindex() const;

	Semantic<E> reverse() const;

	Semantic<E> shuffle() const;

	Semantic<E> shuffle(const Function<E, E> &mapper) const;

	Semantic<E> skip(const Module &n) const;

	Semantic<E> sorted() const;

	Semantic<E> sorted(const Comparator<E, E> &comparator) const;

	Semantic<E> sub(const Module &start, const Module &end) const;

	Semantic<E> takeWhile(const Predicate<E> &p) const;

	std::list<E> toList() const;

	template <typename K, typename V>
	std::map<K, V> toMap(const Function<E, K> &keyExtractor, const Function<E, V> &valueExtractor) const;

	std::set<E> toSet() const;

	std::unordered_set<E> toUnorderedSet() const;

	Statistics<E, E> toStatistics() const;

	template <typename R>
	Statistics<E, R> toStatistics(const Function<E, R> &mapper) const;

	std::vector<E> toVector() const;

	Semantic<E> translate(const Timestamp &offset) const;
};
}; // namespace semantic
#endif
