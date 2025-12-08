#pragma once
#include <set>
#include <list>
#include <map>
#include <array>
#include <string>
#include <vector>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
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
#include <condition_variable>
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
using Comparator = std::function<Timestamp(T, U)>;

template <typename T>
using Generator = BiConsumer<BiConsumer<T, Timestamp>, Predicate<T>>;

template <typename D>
D randomly(const D &start, const D &end);

class ThreadPool
{
  public:
	explicit ThreadPool(Module threads = std::thread::hardware_concurrency());
	~ThreadPool();

	ThreadPool(const ThreadPool &) = delete;
	ThreadPool &operator=(const ThreadPool &) = delete;

	template <class F>
	auto submit(F &&f) -> std::future<typename std::result_of<F()>::type>;

	Module getThreadCount();

	Module getTaskQueueSize();

  protected:
	Module threads;
	std::vector<std::thread> workers;
	std::queue<Runnable> tasks;
	std::mutex m_queue_mutex;
	std::condition_variable condition;
	bool stop;
};

extern ThreadPool globalThreadPool;

template <typename E, typename A, typename R>
class Collector
{
  public:
	using Identity = Supplier<A>;
	using Accumulator = BiFunction<A, E, A>;
	using Combiner = BiFunction<A, A, A>;
	using Finisher = Function<A, R>;

	const Identity identity;

	const Accumulator accumulator;

	const Combiner combiner;

	const Finisher finisher;

	Collector(const Identity &identity, const Accumulator &accumulator, const Combiner &combiner, const Finisher &finisher) : identity(identity), accumulator(accumulator), combiner(combiner), finisher(finisher) {}
};

template <typename E>
class Collectable{
	protected:
	
	const std::shared_ptr<Generator<E>> generator;
	
	Module concurrent;
	
	public:
	
		Collectable() : generator(std::make_shared<Generator<E>>([](const Consumer<E> &, const Predicate<E> &) {})), concurrent(1) {}

	Collectable(const Generator<E> &generator) : generator(std::make_shared<Generator<E>>(generator)), concurrent(1) {}

	Collectable(const Generator<E> &generator, const Module &concurrent) : generator(std::make_shared<Generator<E>>(generator)), concurrent(concurrent) {}

	Collectable(std::shared_ptr<Generator<E>> generator) : generator(generator), concurrent(1) {}

    Collectable(std::shared_ptr<Generator<E>> generator, const Module &concurrent) : generator(generator), concurrent(concurrent) {}

	Collectable(const Semantic &other) : generator(other.generator), concurrent(other.concurrent) {}

	Collectable(Semantic &&other) noexcept : generator(std::move(other.generator)), concurrent(other.concurrent) {}

	Collectable<E> &operator=(const Collectable<E> &other);

	Collectable<E> &operator=(Collectable<E> &&other) noexcept;
	
	bool anyMatch(const Predicate<E> &p) const;
    bool allMatch(const Predicate<E> &p) const;

    template <typename A, typename R>
    R collect(const Supplier<A> &identity, const BiFunction<A, E, A> &accumulator, const BiFunction<A, A, A> &combiner, const Function<A, R> &finisher) const;

    template <typename A, typename R>
    R collect(const Collector<E, A, R> &c) const;

    void cout() const;
    void cout(const BiFunction<E, std::ostream &, std::ostream &> &accumulator) const;
    void cout(std::ostream &stream) const;
    void cout(std::ostream &stream, const BiConsumer<E, std::ostream &> &accumulator) const;

    Module count() const;

    std::optional<E> findFirst() const;
    std::optional<E> findAny() const;

    void forEach(const Consumer<E> &c) const;

    template <typename K>
    std::map<K, std::vector<E>> group(const Function<E, K> &classifier) const;

    template <typename K, typename V>
    std::map<K, std::vector<V>> groupBy(const Function<E, K> &keyExtractor, const Function<E, V> &valueExtractor) const;

    bool noneMatch(const Predicate<E> &p) const;

    std::vector<std::vector<E>> partition(const Module &count) const;
    std::vector<std::vector<E>> partitionBy(const Function<E, Module> &classifier) const;

    std::optional<E> reduce(const BiFunction<E, E, E> &accumulator) const;
    E reduce(const E &identity, const BiFunction<E, E, E> &accumulator) const;

    template <typename R>
    R reduce(const R &identity, const BiFunction<R, E, R> &accumulator, const BiFunction<R, R, R> &combiner) const;

    std::list<E> toList() const;

    template <typename K, typename V>
    std::map<K, V> toMap(const Function<E, K> &keyExtractor, const Function<E, V> &valueExtractor) const;

    std::set<E> toSet() const;
    std::unordered_set<E> toUnorderedSet() const;

    std::vector<E> toVector() const;
};

template <typename E, typename D>
class Statistics : public Collectable<E> {
    static_assert(std::is_arithmetic<D>::value, "Statistical operations require arithmetic types.");

protected:

    mutable Module total = 0;
    mutable std::map<D, Module> frequencyCache;

public:
    Statistics() : Collectable<E>(), frequencyCache() {}
    Statistics(const Generator<E> &generator) : Collectable<E>(generator), frequencyCache() {}
    Statistics(const Generator<E> &generator, const Module &concurrent) : Collectable<E>(generator, concurrent), frequencyCache() {}
    Statistics(std::shared_ptr<Generator<E>> generator) : Collectable<E>(generator), frequencyCache() {}
    Statistics(std::shared_ptr<Generator<E>> generator, const Module &concurrent) : Collectable<E>(generator, concurrent), frequencyCache() {}
    Statistics(const Statistics<E, D> &other) : Collectable<E>(other), frequencyCache(other.frequencyCache) {}
    Statistics(Statistics<E, D> &&other) noexcept : Collectable<E>(std::move(other)), frequencyCache(std::move(other.frequencyCache)) {}

    Statistics<E, D> &operator=(const Collectable<E> &other);
    Statistics<E, D> &operator=(Collectable<E> &&other) noexcept;

    Module count() const;
    E maximum(const Comparator<E, E> &comparator) const;
    E minimum(const Comparator<E, E> &comparator) const;
    D range(const Function<E, D> &mapper) const;
    D variance(const Function<E, D> &mapper) const;
    D standardDeviation(const Function<E, D> &mapper) const;
    D mean(const Function<E, D> &mapper) const;
    D median(const Function<E, D> &mapper) const;
    D mode(const Function<E, D> &mapper) const;
    std::map<D, Module> frequency(const Function<E, D> &mapper) const;
    D sum(const Function<E, D> &mapper) const;
    std::vector<D> quartiles(const Function<E, D> &mapper) const;
    D interquartileRange(const Function<E, D> &mapper) const;
    D skewness(const Function<E, D> &mapper) const;
    D kurtosis(const Function<E, D> &mapper) const;
    bool isEmpty() const;
    void clear();

    Statistics<E, D> &operator=(const std::list<E> &l);
    Statistics<E, D> &operator=(const std::vector<E> &v);
    Statistics<E, D> &operator=(std::initializer_list<E> l);
    Statistics<E, D> &operator=(const Statistics<E, D> &other);
    Statistics<E, D> &operator=(Statistics<E, D> &&other) noexcept;
};

template <typename E>
class OrderedCollectable : public Collectable<E> {
protected:
    using Container = std::set<std::pair<Timestamp, E>>;

    mutable Container container;

    Container toIndexedSet() const;

public:
    OrderedCollectable() : Collectable<E>(), container() {}
        
        OrderedCollectable(const Container& container) : Collectable<E>(), container(container) {}

    OrderedCollectable(const Generator<E> &generator)
        : Collectable<E>(generator), container(toIndexedSet()) {}

    OrderedCollectable(const Generator<E> &generator, const Module &concurrent)
        : Collectable<E>(generator, concurrent), container(toIndexedSet()) {}

    OrderedCollectable(std::shared_ptr<Generator<E>> generator)
        : Collectable<E>(generator), container(toIndexedSet()) {}

    OrderedCollectable(std::shared_ptr<Generator<E>> generator, const Module &concurrent)
        : Collectable<E>(generator, concurrent), container(toIndexedSet()) {}

    OrderedCollectable(const OrderedCollectable &other)
        : Collectable<E>(other), container(other.container) {}

    OrderedCollectable(OrderedCollectable &&other) noexcept
        : Collectable<E>(std::move(other)), container(std::move(other.container)) {}

    OrderedCollectable<E> &operator=(const Collectable<E> &other);
    OrderedCollectable<E> &operator=(Collectable<E> &&other) noexcept;
};


template <typename E>
class UnorderedCollectable : public Collectable<E> {
public:
    UnorderedCollectable() : Collectable<E>() {}
    UnorderedCollectable(const Generator<E> &generator) : Collectable<E>(generator) {}
    UnorderedCollectable(const Generator<E> &generator, const Module &concurrent) : Collectable<E>(generator, concurrent) {}
    UnorderedCollectable(std::shared_ptr<Generator<E>> generator) : Collectable<E>(generator) {}
    UnorderedCollectable(std::shared_ptr<Generator<E>> generator, const Module &concurrent) : Collectable<E>(generator, concurrent) {}
    UnorderedCollectable(const UnorderedCollectable &other) : Collectable<E>(other) {}
    UnorderedCollectable(UnorderedCollectable &&other) noexcept : Collectable<E>(std::move(other)) {}

    UnorderedCollectable<E> &operator=(const Collectable<E> &other);
    UnorderedCollectable<E> &operator=(Collectable<E> &&other) noexcept;
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

template <typename E, Module length>
Semantic<E> from(const std::array<E, length> &array);

template <typename E>
Semantic<E> from(const std::list<E> &l);

template <typename E>
Semantic<E> from(const std::vector<E> &v);

template <typename E>
Semantic<E> from(const std::initializer_list<E> &l);

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
	Semantic() : generator(std::make_shared<Generator<E>>([](const Consumer<E> &, const Predicate<E> &) {})), concurrent(1) {}

	Semantic(const Generator<E> &generator) : generator(std::make_shared<Generator<E>>(generator)), concurrent(1) {}

	Semantic(const Generator<E> &generator, const Module &concurrent) : generator(std::make_shared<Generator<E>>(generator)), concurrent(concurrent) {}

	Semantic(std::shared_ptr<Generator<E>> generator) : generator(generator), concurrent(1) {}

	Semantic(std::shared_ptr<Generator<E>> generator, const Module &concurrent) : generator(generator), concurrent(concurrent) {}

	Semantic(const Semantic &other) : generator(other.generator), concurrent(other.concurrent) {}

	Semantic(Semantic &&other) noexcept : generator(std::move(other.generator)), concurrent(other.concurrent) {}

	Semantic<E> &operator=(const Semantic<E> &other);

	Semantic<E> &operator=(Semantic<E> &&other) noexcept;

	Semantic<E> concat(const Semantic<E> &other) const;

	Semantic<E> distinct() const;

	Semantic<E> distinct(const Comparator<E, E> &identifier) const;

	Semantic<E> dropWhile(const Predicate<E> &p) const;

	Semantic<E> filter(const Predicate<E> &p) const;

	Semantic<E> flat(const Function<E, Semantic<E>> &mapper) const;

	template <typename R>
	Semantic<R> flatMap(const Function<E, Semantic<R>> &mapper) const;

	Semantic<E> limit(const Module &n) const;

	template <typename R>
	Semantic<R> map(const Function<E, R> &mapper) const;

	Semantic<E> parallel() const;

	Semantic<E> parallel(const Module &threadCount) const;

	Semantic<E> peek(const Consumer<E> &c) const;

	Semantic<E> redirect(const BiFunction<E, Timestamp, Timestamp> &redirector) const;

	Semantic<E> reverse() const;

	Semantic<E> shuffle() const;

	Semantic<E> shuffle(const Function<E, Timestamp> &mapper) const;

	Semantic<E> skip(const Module &n) const;

	OrderedCollectable<E> sorted() const;

	OrderedCollectable<E> sorted(const Comparator<E, E> &indexer) const;

	Semantic<E> sub(const Module &start, const Module &end) const;

	Semantic<E> takeWhile(const Predicate<E> &p) const;
	
	OrderedCollectable<E> toOrdered() const;
	
	Statistics<E, E> toStatistics() const;
    template <typename R>
    Statistics<E, R> toStatistics(const Function<E, R> &mapper) const;
	
	UnorderedCollectable<E> toUnordered() const;

	Semantic<E> translate(const Timestamp &offset) const;
	
};
}; // namespace semantic
