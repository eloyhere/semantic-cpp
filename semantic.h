#pragma once

#include "hash.h"
#include "less.h"
#include "function.h"
#include "charsequence.h"
#include "collector.h"
#include "collectors.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace semantic
{
template <typename E>
class Semantic;
}

namespace collectable
{
template <typename E>
class Collectable
{
  protected:
    function::Module concurrent;

  public:
    Collectable(const function::Module &concurrent) : concurrent(concurrent) {}

    virtual ~Collectable() = default;

    Collectable(const Collectable &) = default;
    Collectable(Collectable &&) = default;
    Collectable &operator=(const Collectable &) = default;
    Collectable &operator=(Collectable &&) = default;

    template <typename Predicate>
    auto anyMatch(Predicate &&predicate) const -> bool
    {
        collector::Collector<E, bool, bool> collectorValue = collector::useAnyMatch<E, Predicate>(std::forward<Predicate>(predicate));
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename Predicate>
    auto allMatch(Predicate &&predicate) const -> bool
    {
        collector::Collector<E, bool, bool> collectorValue = collector::useAllMatch<E, Predicate>(std::forward<Predicate>(predicate));
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename A, typename R>
    auto collect(const function::Supplier<R> &identity, const function::BiFunction<A, E, A> &accumulator, const function::BiFunction<A, A, A> &combiner, const function::Function<A, R> &finisher) const -> R
    {
        collector::Collector<E, A, R> collectorValue = collector::useCollect<E, A, R>(identity, accumulator, combiner, finisher);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename A, typename R>
    auto collect(const function::Supplier<R> &identity, const function::TriPredicate<E, function::Timestamp, A> &interrupt, const function::BiFunction<A, E, A> &accumulator, const function::BiFunction<A, A, A> &combiner, const function::Function<A, R> &finisher) const -> R
    {
        collector::Collector<E, A, R> collectorValue = collector::useCollect<E, A, R>(identity, interrupt, accumulator, combiner, finisher);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto count() const -> function::Module
    {
        collector::Collector<E, function::Module, function::Module> collectorValue = collector::useCount<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto empty() const -> bool
    {
        collector::Collector<E, function::Module, function::Module> collectorValue = collector::useCount<E>();
        return collectorValue.collect(this->source(), this->concurrent) == 0;
    }

    auto findAny() const -> std::optional<E>
    {
        collector::Collector<E, std::optional<E>, std::optional<E>> collectorValue = collector::useFindAny<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto findAt(const function::Timestamp &index) const -> std::optional<E>
    {
        if (index >= 0LL)
        {
            collector::Collector<E, std::optional<E>, std::optional<E>> collectorValue = collector::useFindAt<E>(index);
            return collectorValue.collect(this->source(), this->concurrent);
        }
        else
        {
            collector::Collector<E, std::pair<std::vector<E>, function::Module>, std::optional<E>> collectorValue = collector::useFindNegativeAt<E>(index);
            return collectorValue.collect(this->source(), this->concurrent);
        }
    }

    auto findFirst() const -> std::optional<E>
    {
        collector::Collector<E, std::optional<E>, std::optional<E>> collectorValue = collector::useFindFirst<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto findLast() const -> std::optional<E>
    {
        collector::Collector<E, std::vector<E>, std::optional<E>> collectorValue = collector::useFindLast<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto findMaximum() const -> std::optional<E>
    {
        collector::Collector<E, std::optional<E>, std::optional<E>> collectorValue = collector::useFindMaximum<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto findMaximum(const function::Comparator<E> &comparator) const -> std::optional<E>
    {
        collector::Collector<E, std::optional<E>, std::optional<E>> collectorValue = collector::useFindMaximum<E>(comparator);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto findMinimum() const -> std::optional<E>
    {
        collector::Collector<E, std::optional<E>, std::optional<E>> collectorValue = collector::useFindMinimum<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto findMinimum(const function::Comparator<E> &comparator) const -> std::optional<E>
    {
        collector::Collector<E, std::optional<E>, std::optional<E>> collectorValue = collector::useFindMinimum<E>(comparator);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename Consumer>
    auto forEach(Consumer &&consumer) const -> void
    {
        collector::Collector<E, function::Module, function::Module> collectorValue = collector::useForEach<E, Consumer>(std::forward<Consumer>(consumer));
        collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename KeyExtractor>
    auto group(KeyExtractor &&keyExtractor) const -> std::unordered_map<decltype(std::declval<KeyExtractor>()(std::declval<E>())), std::vector<E>>
    {
        using K = decltype(std::declval<KeyExtractor>()(std::declval<E>()));
        collector::Collector<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>> collectorValue = collector::useGroup<E, K, KeyExtractor>(std::forward<KeyExtractor>(keyExtractor));
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto join() const -> charsequence::Charsequence
    {
        collector::Collector<E, charsequence::Builder, charsequence::Charsequence> collectorValue = collector::useJoin<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto join(const charsequence::Charsequence &delimiter) const -> charsequence::Charsequence
    {
        collector::Collector<E, charsequence::Builder, charsequence::Charsequence> collectorValue = collector::useJoin<E>(delimiter);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto join(const charsequence::Charsequence &prefix, const charsequence::Charsequence &delimiter, const charsequence::Charsequence &suffix) const -> charsequence::Charsequence
    {
        collector::Collector<E, charsequence::Builder, charsequence::Charsequence> collectorValue = collector::useJoin<E>(prefix, delimiter, suffix);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename Converter>
    auto join(const charsequence::Charsequence &prefix, Converter &&converter, const charsequence::Charsequence &suffix) const -> charsequence::Charsequence
    {
        collector::Collector<E, charsequence::Builder, charsequence::Charsequence> collectorValue = collector::useJoin<E>(prefix, std::forward<Converter>(converter), suffix);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto out() const -> charsequence::Charsequence
    {
        collector::Collector<E, charsequence::Builder, charsequence::Charsequence> collectorValue = collector::useOut<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto out(const charsequence::Charsequence &delimiter) const -> charsequence::Charsequence
    {
        collector::Collector<E, charsequence::Builder, charsequence::Charsequence> collectorValue = collector::useOut<E>(delimiter);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto out(const charsequence::Charsequence &prefix, const charsequence::Charsequence &delimiter, const charsequence::Charsequence &suffix) const -> charsequence::Charsequence
    {
        collector::Collector<E, charsequence::Builder, charsequence::Charsequence> collectorValue = collector::useOut<E>(prefix, delimiter, suffix);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename Converter>
    auto out(const charsequence::Charsequence &prefix, Converter &&converter, const charsequence::Charsequence &suffix) const -> charsequence::Charsequence
    {
        collector::Collector<E, charsequence::Builder, charsequence::Charsequence> collectorValue = collector::useOut<E>(prefix, std::forward<Converter>(converter), suffix);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto error() const -> void
    {
        collector::Collector<E, charsequence::Builder, charsequence::Charsequence> collectorValue = collector::useError<E>();
        collectorValue.collect(this->source(), this->concurrent);
    }

    auto error(const charsequence::Charsequence &delimiter) const -> void
    {
        collector::Collector<E, charsequence::Builder, charsequence::Charsequence> collectorValue = collector::useError<E>(delimiter);
        collectorValue.collect(this->source(), this->concurrent);
    }

    auto error(const charsequence::Charsequence &prefix, const charsequence::Charsequence &delimiter, const charsequence::Charsequence &suffix) const -> void
    {
        collector::Collector<E, charsequence::Builder, charsequence::Charsequence> collectorValue = collector::useError<E>(prefix, delimiter, suffix);
        collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename Converter>
    auto error(const charsequence::Charsequence &prefix, Converter &&converter, const charsequence::Charsequence &suffix) const -> void
    {
        collector::Collector<E, charsequence::Builder, charsequence::Charsequence> collectorValue = collector::useError<E>(prefix, std::forward<Converter>(converter), suffix);
        collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename Predicate>
    auto noneMatch(Predicate &&predicate) const -> bool
    {
        collector::Collector<E, bool, bool> collectorValue = collector::useNoneMatch<E>(std::forward<Predicate>(predicate));
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto partition(const function::Module &size) const -> std::vector<std::vector<E>>
    {
        collector::Collector<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>> collectorValue = collector::usePartition<E>(size);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename KeyExtractor>
    auto partitionBy(KeyExtractor &&keyExtractor) const -> std::vector<std::vector<E>>
    {
        collector::Collector<E, std::map<function::Timestamp, std::vector<E>>, std::vector<std::vector<E>>> collectorValue = collector::usePartitionBy<E, KeyExtractor>(std::forward<KeyExtractor>(keyExtractor));
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto reduce(const function::BiFunction<E, E, E> &accumulator) const -> std::optional<E>
    {
        collector::Collector<E, std::optional<E>, std::optional<E>> collectorValue = collector::useReduce<E>(accumulator);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto reduce(const E &identity, const function::BiFunction<E, E, E> &accumulator) const -> E
    {
        collector::Collector<E, E, E> collectorValue = collector::useReduce<E>(identity, accumulator);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename R>
    auto reduce(const R &identity, const function::BiFunction<R, E, R> &accumulator, const function::BiFunction<R, R, R> &combiner) const -> R
    {
        collector::Collector<E, R, R> collectorValue = collector::useReduce<E, R>(identity, accumulator, combiner);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    virtual auto source() const -> function::Generator<E> = 0;

    template <std::size_t N>
    auto toArray() const -> std::array<E, N>
    {
        collector::Collector<E, std::array<E, N>, std::array<E, N>> collectorValue = collector::useToArray<E, N>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto toDeque() const -> std::deque<E>
    {
        collector::Collector<E, std::deque<E>, std::deque<E>> collectorValue = collector::useToDeque<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto toForwardList() const -> std::forward_list<E>
    {
        collector::Collector<E, std::forward_list<E>, std::forward_list<E>> collectorValue = collector::useToForwardList<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto toList() const -> std::list<E>
    {
        collector::Collector<E, std::list<E>, std::list<E>> collectorValue = collector::useToList<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename KeyExtractor>
    auto toMap(KeyExtractor &&keyExtractor) const -> std::map<decltype(std::declval<KeyExtractor>()(std::declval<E>())), E>
    {
        using K = decltype(std::declval<KeyExtractor>()(std::declval<E>()));
        collector::Collector<E, std::map<K, E>, std::map<K, E>> collectorValue = collector::useToMap<E, K, KeyExtractor>(std::forward<KeyExtractor>(keyExtractor));
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename KeyExtractor, typename ValueExtractor>
    auto toMap(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor) const -> std::map<decltype(std::declval<KeyExtractor>()(std::declval<E>())), decltype(std::declval<ValueExtractor>()(std::declval<E>()))>
    {
        using K = decltype(std::declval<KeyExtractor>()(std::declval<E>()));
        using V = decltype(std::declval<ValueExtractor>()(std::declval<E>()));
        collector::Collector<E, std::map<K, V>, std::map<K, V>> collectorValue = collector::useToMap<E, K, V, KeyExtractor, ValueExtractor>(std::forward<KeyExtractor>(keyExtractor), std::forward<ValueExtractor>(valueExtractor));
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename KeyExtractor>
    auto toMultimap(KeyExtractor &&keyExtractor) const -> std::multimap<decltype(std::declval<KeyExtractor>()(std::declval<E>())), E>
    {
        using K = decltype(std::declval<KeyExtractor>()(std::declval<E>()));
        collector::Collector<E, std::multimap<K, E>, std::multimap<K, E>> collectorValue = collector::useToMultimap<E, K, KeyExtractor>(std::forward<KeyExtractor>(keyExtractor));
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename KeyExtractor, typename ValueExtractor>
    auto toMultimap(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor) const -> std::multimap<decltype(std::declval<KeyExtractor>()(std::declval<E>())), decltype(std::declval<ValueExtractor>()(std::declval<E>()))>
    {
        using K = decltype(std::declval<KeyExtractor>()(std::declval<E>()));
        using V = decltype(std::declval<ValueExtractor>()(std::declval<E>()));
        collector::Collector<E, std::multimap<K, V>, std::multimap<K, V>> collectorValue = collector::useToMultimap<E, K, V, KeyExtractor, ValueExtractor>(std::forward<KeyExtractor>(keyExtractor), std::forward<ValueExtractor>(valueExtractor));
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto toMultiset() const -> std::multiset<E>
    {
        collector::Collector<E, std::multiset<E>, std::multiset<E>> collectorValue = collector::useToMultiset<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto toPriorityQueue() const -> std::priority_queue<E>
    {
        collector::Collector<E, std::priority_queue<E>, std::priority_queue<E>> collectorValue = collector::useToPriorityQueue<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto toQueue() const -> std::queue<E>
    {
        collector::Collector<E, std::queue<E>, std::queue<E>> collectorValue = collector::useToQueue<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto toSet() const -> std::set<E>
    {
        collector::Collector<E, std::set<E>, std::set<E>> collectorValue = collector::useToSet<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto toStack() const -> std::stack<E>
    {
        collector::Collector<E, std::stack<E>, std::stack<E>> collectorValue = collector::useToStack<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename K, typename V>
    auto toUnorderedMap(const function::BiFunction<E, function::Timestamp, K> &keyExtractor, const function::BiFunction<E, function::Timestamp, V> &valueExtractor) const -> std::unordered_map<K, V>
    {
        collector::Collector<E, std::unordered_map<K, V>, std::unordered_map<K, V>> collectorValue = collector::useToUnorderedMap<E, K, V>(keyExtractor, valueExtractor);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename KeyExtractor>
    auto toUnorderedMultimap(KeyExtractor &&keyExtractor) const -> std::unordered_multimap<decltype(std::declval<KeyExtractor>()(std::declval<E>())), E>
    {
        using K = decltype(std::declval<KeyExtractor>()(std::declval<E>()));
        collector::Collector<E, std::unordered_multimap<K, E>, std::unordered_multimap<K, E>> collectorValue = collector::useToUnorderedMultimap<E, K, KeyExtractor>(std::forward<KeyExtractor>(keyExtractor));
        return collectorValue.collect(this->source(), this->concurrent);
    }

    template <typename KeyExtractor, typename ValueExtractor>
    auto toUnorderedMultimap(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor) const -> std::unordered_multimap<decltype(std::declval<KeyExtractor>()(std::declval<E>())), decltype(std::declval<ValueExtractor>()(std::declval<E>()))>
    {
        using K = decltype(std::declval<KeyExtractor>()(std::declval<E>()));
        using V = decltype(std::declval<ValueExtractor>()(std::declval<E>()));
        collector::Collector<E, std::unordered_multimap<K, V>, std::unordered_multimap<K, V>> collectorValue = collector::useToUnorderedMultimap<E, K, V, KeyExtractor, ValueExtractor>(std::forward<KeyExtractor>(keyExtractor), std::forward<ValueExtractor>(valueExtractor));
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto toUnorderedMultiset() const -> std::unordered_multiset<E>
    {
        collector::Collector<E, std::unordered_multiset<E>, std::unordered_multiset<E>> collectorValue = collector::useToUnorderedMultiset<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto toUnorderedSet() const -> std::unordered_set<E>
    {
        collector::Collector<E, std::unordered_set<E>, std::unordered_set<E>> collectorValue = collector::useToUnorderedSet<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto toVector() const -> std::vector<E>
    {
        collector::Collector<E, std::vector<E>, std::vector<E>> collectorValue = collector::useToVector<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }
};

template <typename E>
class OrderedCollectable : public Collectable<E>
{
  protected:
    std::multimap<function::Timestamp, E> buffer;

    function::Comparator<std::pair<function::Timestamp, E>> build(const function::Comparator<E> &comparator) const
    {
        return [comparator](const std::pair<function::Timestamp, E> &a, const std::pair<function::Timestamp, E> &b) -> bool {
            if (comparator(a.second, b.second))
            {
                return true;
            }
            if (comparator(b.second, a.second))
            {
                return false;
            }
            return a.first < b.first;
        };
    }

    function::Module limit(const function::Timestamp &index, const function::Module period) const
    {
        if (period < 2ULL)
        {
            return 0ULL;
        }

        if (index < 0LL)
        {
            return (period - static_cast<function::Module>(std::abs(index) % period)) % period;
        }
        return index % period;
    }

  public:
    OrderedCollectable(const function::Generator<E> &generator) : Collectable<E>(1)
    {
        std::vector<std::pair<function::Timestamp, E>> tempBuffer;
        generator([&tempBuffer](E element, function::Timestamp index) -> void { tempBuffer.emplace_back(index, element); }, [](E element, function::Timestamp index) -> bool { return false; });
        function::Module period = static_cast<function::Module>(tempBuffer.size());
        for (const auto &pair : tempBuffer)
        {
            function::Timestamp index = limit(pair.first, period);
            this->buffer.insert(std::make_pair(index, pair.second));
        }
    }

    OrderedCollectable(const function::Generator<E> &generator, const function::Module &concurrent) : Collectable<E>(concurrent)
    {
        std::vector<std::pair<function::Timestamp, E>> tempBuffer;
        generator([&tempBuffer](E element, function::Timestamp index) -> void { tempBuffer.emplace_back(index, element); }, [](E element, function::Timestamp index) -> bool { return false; });
        function::Module period = static_cast<function::Module>(tempBuffer.size());
        for (const auto &pair : tempBuffer)
        {
            function::Timestamp index = limit(pair.first, period);
            this->buffer.insert(std::make_pair(index, pair.second));
        }
    }

    OrderedCollectable(const function::Generator<E> &generator, const function::Comparator<E> &comparator) : Collectable<E>(1)
    {
        auto comp = build(comparator);
        std::multiset<std::pair<function::Timestamp, E>, decltype(comp)> tempBuffer(comp);
        generator([&tempBuffer](E element, function::Timestamp index) -> void { tempBuffer.insert(std::make_pair(index, element)); }, [](E element, function::Timestamp index) -> bool { return false; });
        function::Module position = 0ULL;
        for (const auto &pair : tempBuffer)
        {
            this->buffer.insert(std::make_pair(position, pair.second));
            position = position + 1ULL;
        }
    }

    OrderedCollectable(const function::Generator<E> &generator, const function::Comparator<E> &comparator, const function::Module &concurrent) : Collectable<E>(concurrent)
    {
        auto comp = build(comparator);
        std::multiset<std::pair<function::Timestamp, E>, decltype(comp)> tempBuffer(comp);
        generator([&tempBuffer](E element, function::Timestamp index) -> void { tempBuffer.insert(std::make_pair(index, element)); }, [](E element, function::Timestamp index) -> bool { return false; });
        function::Module position = 0ULL;
        for (const auto &pair : tempBuffer)
        {
            this->buffer.insert(std::make_pair(position, pair.second));
            position = position + 1ULL;
        }
    }

    OrderedCollectable(const OrderedCollectable<E> &other) : Collectable<E>(other.concurrent), buffer(other.buffer)
    {
    }

    OrderedCollectable(OrderedCollectable<E> &&other) noexcept : Collectable<E>(other.concurrent), buffer(std::move(other.buffer))
    {
    }

    auto operator=(const OrderedCollectable<E> &other) -> OrderedCollectable<E> &
    {
        if (this != &other)
        {
            this->concurrent = other.concurrent;
            this->buffer = other.buffer;
        }
        return *this;
    }

    auto operator=(OrderedCollectable<E> &&other) noexcept -> OrderedCollectable<E> &
    {
        if (this != &other)
        {
            this->concurrent = other.concurrent;
            this->buffer = std::move(other.buffer);
        }
        return *this;
    }

    virtual auto source() const -> function::Generator<E> override
    {
        return [buffer = this->buffer](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            for (const auto &pair : buffer)
            {
                if (interrupt(pair.second, pair.first))
                {
                    break;
                }
                accept(pair.second, pair.first);
            }
        };
    }
};

template <typename E, typename D>
class Statistics : public OrderedCollectable<E>
{
  public:
    Statistics() : OrderedCollectable<E>(1) {}

    Statistics(const function::Module &concurrent) : OrderedCollectable<E>(concurrent) {}

    Statistics(const function::Generator<E> &generator) : OrderedCollectable<E>(generator) {}

    Statistics(const function::Generator<E> &generator, const function::Module &concurrent) : OrderedCollectable<E>(generator, concurrent) {}

    Statistics(const Statistics<E, D> &other) : OrderedCollectable<E>(other) {}

    Statistics(Statistics<E, D> &&other) noexcept : OrderedCollectable<E>(std::move(other)) {}

    auto operator=(const Statistics<E, D> &other) -> Statistics<E, D> &
    {
        if (this != &other)
        {
            OrderedCollectable<E>::operator=(other);
        }
        return *this;
    }

    auto operator=(Statistics<E, D> &&other) noexcept -> Statistics<E, D> &
    {
        if (this != &other)
        {
            OrderedCollectable<E>::operator=(std::move(other));
        }
        return *this;
    }

    auto summate() const -> D
    {
        collector::Collector<E, D, D> collectorValue = collector::useSummate<E, D>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto summate(const function::Function<E, D> &mapper) const -> D
    {
        collector::Collector<E, D, D> collectorValue = collector::useSummate<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto average() const -> D
    {
        collector::Collector<E, std::pair<D, function::Module>, D> collectorValue = collector::useAverage<E, D>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto average(const function::Function<E, D> &mapper) const -> D
    {
        collector::Collector<E, std::pair<D, function::Module>, D> collectorValue = collector::useAverage<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto minimum() const -> std::optional<D>
    {
        collector::Collector<E, std::optional<D>, std::optional<D>> collectorValue = collector::useMinimum<E, D>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto minimum(const function::Function<E, D> &mapper) const -> std::optional<D>
    {
        collector::Collector<E, std::optional<D>, std::optional<D>> collectorValue = collector::useMinimum<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto maximum() const -> std::optional<D>
    {
        collector::Collector<E, std::optional<D>, std::optional<D>> collectorValue = collector::useMaximum<E, D>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto maximum(const function::Function<E, D> &mapper) const -> std::optional<D>
    {
        collector::Collector<E, std::optional<D>, std::optional<D>> collectorValue = collector::useMaximum<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto range() const -> D
    {
        collector::Collector<E, std::pair<D, D>, D> collectorValue = collector::useRange<E, D>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto range(const function::Function<E, D> &mapper) const -> D
    {
        collector::Collector<E, std::pair<D, D>, D> collectorValue = collector::useRange<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto variance() const -> D
    {
        collector::Collector<E, std::tuple<D, D, D, function::Module>, D> collectorValue = collector::useVariance<E, D>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto variance(const function::Function<E, D> &mapper) const -> D
    {
        collector::Collector<E, std::tuple<D, D, D, function::Module>, D> collectorValue = collector::useVariance<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto standardDeviation() const -> D
    {
        collector::Collector<E, std::tuple<D, D, D, function::Module>, D> collectorValue = collector::useStandardDeviation<E, D>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto standardDeviation(const function::Function<E, D> &mapper) const -> D
    {
        collector::Collector<E, std::tuple<D, D, D, function::Module>, D> collectorValue = collector::useStandardDeviation<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto frequency() const -> std::map<E, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>
    {
        using InnerMap = std::unordered_map<E, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>;
        using AccumulatorType = std::pair<InnerMap, function::Timestamp>;
        using ResultMap = std::map<E, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>;
        collector::Collector<E, AccumulatorType, ResultMap> collectorValue = collector::useFrequency<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto frequency(const function::Function<E, D> &mapper) const -> std::map<D, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>
    {
        using InnerMap = std::unordered_map<D, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>;
        using AccumulatorType = std::pair<InnerMap, function::Timestamp>;
        using ResultMap = std::map<D, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>;
        collector::Collector<E, AccumulatorType, ResultMap> collectorValue = collector::useFrequency<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto distribute() const -> std::map<E, std::complex<double>>
    {
        collector::Collector<E, std::unordered_map<E, std::vector<function::Timestamp>>, std::map<E, std::complex<double>>> collectorValue = collector::useDistribution<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto distribute(const function::Function<E, D> &mapper) const -> std::map<D, std::complex<double>>
    {
        collector::Collector<E, std::unordered_map<D, std::vector<function::Timestamp>>, std::map<D, std::complex<double>>> collectorValue = collector::useDistribution<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto median() const -> std::optional<D>
    {
        collector::Collector<E, std::vector<D>, std::optional<D>> collectorValue = collector::useMedian<E, D>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto median(const function::Function<E, D> &mapper) const -> std::optional<D>
    {
        collector::Collector<E, std::vector<D>, std::optional<D>> collectorValue = collector::useMedian<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto mode() const -> std::optional<E>
    {
        collector::Collector<E, std::unordered_map<E, std::complex<double>>, std::optional<E>> collectorValue = collector::useMode<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto percentile(double p) const -> std::optional<D>
    {
        collector::Collector<E, std::vector<D>, std::optional<D>> collectorValue = collector::usePercentile<E, D>(p);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto percentile(double p, const function::Function<E, D> &mapper) const -> std::optional<D>
    {
        collector::Collector<E, std::vector<D>, std::optional<D>> collectorValue = collector::usePercentile<E, D>(p, mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto firstQuartile() const -> std::optional<D>
    {
        return percentile(25.0);
    }

    auto firstQuartile(const function::Function<E, D> &mapper) const -> std::optional<D>
    {
        return percentile(25.0, mapper);
    }

    auto thirdQuartile() const -> std::optional<D>
    {
        return percentile(75.0);
    }

    auto thirdQuartile(const function::Function<E, D> &mapper) const -> std::optional<D>
    {
        return percentile(75.0, mapper);
    }

    auto interquartileRange() const -> std::optional<D>
    {
        auto q1 = firstQuartile();
        auto q3 = thirdQuartile();
        if (q1.has_value() && q3.has_value())
            return std::optional<D>(q3.value() - q1.value());
        return std::nullopt;
    }

    auto interquartileRange(const function::Function<E, D> &mapper) const -> std::optional<D>
    {
        auto q1 = firstQuartile(mapper);
        auto q3 = thirdQuartile(mapper);
        if (q1.has_value() && q3.has_value())
            return std::optional<D>(q3.value() - q1.value());
        return std::nullopt;
    }

    auto skewness() const -> D
    {
        collector::Collector<E, std::vector<D>, D> collectorValue = collector::useSkewness<E, D>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto skewness(const function::Function<E, D> &mapper) const -> D
    {
        collector::Collector<E, std::vector<D>, D> collectorValue = collector::useSkewness<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto kurtosis() const -> D
    {
        collector::Collector<E, std::vector<D>, D> collectorValue = collector::useKurtosis<E, D>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto kurtosis(const function::Function<E, D> &mapper) const -> D
    {
        collector::Collector<E, std::vector<D>, D> collectorValue = collector::useKurtosis<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto dft() const -> std::vector<std::complex<double>>
    {
        collector::Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>> collectorValue = collector::useDFT<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto idft() const -> std::vector<std::complex<double>>
    {
        collector::Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>> collectorValue = collector::useIDFT<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto fft() const -> std::vector<std::complex<double>>
    {
        collector::Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>> collectorValue = collector::useFFT<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto ifft() const -> std::vector<std::complex<double>>
    {
        collector::Collector<E, std::vector<std::complex<double>>, std::vector<std::complex<double>>> collectorValue = collector::useIFFT<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto gradient(const std::function<std::vector<double>(const std::vector<E> &)> &gradientFunction,
                  double learningRate, std::size_t maxIterations, double convergenceThreshold) const -> std::vector<double>
    {
        collector::Collector<E, std::vector<double>, std::vector<double>> collectorValue =
            collector::useGradient<E>(gradientFunction, learningRate, maxIterations, convergenceThreshold);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto gradient(const std::function<double(const std::vector<E> &)> &costFunction,
                  double learningRate, std::size_t maxIterations, double convergenceThreshold, double numericalH) const -> std::vector<double>
    {
        collector::Collector<E, std::vector<double>, std::vector<double>> collectorValue =
            collector::useGradient<E>(costFunction, learningRate, maxIterations, convergenceThreshold, numericalH);
        return collectorValue.collect(this->source(), this->concurrent);
    }
};

template <typename E>
class WindowCollectable : public OrderedCollectable<E>
{
  public:
    WindowCollectable(const function::Module &concurrent) : OrderedCollectable<E>(concurrent) {}
    WindowCollectable(const function::Generator<E> &generator, const function::Module &concurrent) : OrderedCollectable<E>(generator, concurrent) {}
    WindowCollectable(const WindowCollectable<E> &other) : OrderedCollectable<E>(other) {}
    WindowCollectable(WindowCollectable<E> &&other) noexcept : OrderedCollectable<E>(std::move(other)) {}

    auto operator=(const WindowCollectable<E> &other) -> WindowCollectable<E> &
    {
        if (this != &other)
        {
            OrderedCollectable<E>::operator=(other);
        }
        return *this;
    }

    auto operator=(WindowCollectable<E> &&other) noexcept -> WindowCollectable<E> &
    {
        if (this != &other)
        {
            OrderedCollectable<E>::operator=(std::move(other));
        }
        return *this;
    }

    auto slide(const function::Module &size, const function::Timestamp &step) const -> semantic::Semantic<semantic::Semantic<E>>;

    auto tumble(const function::Module &size) const -> semantic::Semantic<semantic::Semantic<E>>
    {
        return this->slide(size, size);
    }
};

template <typename E>
class UnorderedCollectable : public Collectable<E>
{
  protected:
    std::unordered_multimap<function::Timestamp, E> buffer;

  public:
    UnorderedCollectable(const function::Generator<E> &generator) : Collectable<E>(1)
    {
        generator([this](E element, function::Timestamp index) -> void { this->buffer.insert(std::make_pair(index, element)); }, [](E element, function::Timestamp index) -> bool { return false; });
    }

    UnorderedCollectable(const function::Generator<E> &generator, const function::Module &concurrent) : Collectable<E>(concurrent)
    {
        generator([this](E element, function::Timestamp index) -> void { this->buffer.insert(std::make_pair(index, element)); }, [](E element, function::Timestamp index) -> bool { return false; });
    }

    UnorderedCollectable(const UnorderedCollectable &other) : Collectable<E>(other.concurrent), buffer(other.buffer)
    {
    }

    UnorderedCollectable(UnorderedCollectable &&other) noexcept : Collectable<E>(other.concurrent), buffer(std::move(other.buffer))
    {
    }

    UnorderedCollectable &operator=(const UnorderedCollectable &other)
    {
        if (this != &other)
        {
            Collectable<E>::operator=(other);
            buffer = other.buffer;
        }
        return *this;
    }

    UnorderedCollectable &operator=(UnorderedCollectable &&other) noexcept
    {
        if (this != &other)
        {
            Collectable<E>::operator=(std::move(other));
            buffer = std::move(other.buffer);
        }
        return *this;
    }

    virtual auto source() const -> function::Generator<E> override
    {
        return [buffer = this->buffer](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            for (const auto &pair : buffer)
            {
                if (interrupt(pair.second, pair.first))
                {
                    break;
                }
                accept(pair.second, pair.first);
            }
        };
    }
};

} // namespace collectable

namespace semantic
{
template <typename E>
class Semantic
{
  private:
    std::unique_ptr<function::Generator<E>> generator;
    function::Module concurrent;

  public:
    using Element = E;

    Semantic(Semantic<E> &&other) noexcept = default;

    Semantic(const function::Generator<E> &generator) : generator(std::make_unique<function::Generator<E>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<E> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<E>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<E> &other) : generator(std::make_unique<function::Generator<E>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic<E> &operator=(const Semantic<E> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<E>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<E> &operator=(Semantic<E> &&other) noexcept = default;

    auto source() const -> function::Generator<E>
    {
        return [generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module
    {
        return concurrent;
    }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, E, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, E, function::Timestamp>;
            static_assert(!std::is_same_v<R, void>, "Mapper must not return void");
            return Semantic<R>(
                [generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](
                    function::BiConsumer<R, function::Timestamp> accept,
                    function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                    bool stop = false;
                    generator(
                        [&accept, &mapper, &stop, &interrupt](E element, function::Timestamp index) -> void {
                            R mapped = std::invoke(mapper, element, index);
                            accept(mapped, index);
                            stop = stop || interrupt(mapped, index);
                        },
                        [&stop](E element, function::Timestamp index) -> bool {
                            return stop;
                        });
                },
                this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, E>)
        {
            using R = std::invoke_result_t<Mapper, E>;
            static_assert(!std::is_same_v<R, void>, "Mapper must not return void");
            return Semantic<R>(
                [generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](
                    function::BiConsumer<R, function::Timestamp> accept,
                    function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                    bool stop = false;
                    generator(
                        [&accept, &mapper, &stop, &interrupt](E element, function::Timestamp index) -> void {
                            R mapped = std::invoke(mapper, element);
                            accept(mapped, index);
                            stop = stop || interrupt(mapped, index);
                        },
                        [&stop](E element, function::Timestamp index) -> bool {
                            return stop;
                        });
                },
                this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, E, function::Timestamp> || std::is_invocable_v<Mapper, E>,
                          "Mapper must be callable as (E, Timestamp) -> R or (E) -> R, but neither signature matched");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<E>
    {
        static_assert(
            std::is_invocable_r_v<bool, Predicate, E, function::Timestamp> ||
                std::is_invocable_r_v<bool, Predicate, E>,
            "Predicate must be callable as (E, Timestamp) -> bool or (E) -> bool");

        return Semantic<E>(
            [generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](
                function::BiConsumer<E, function::Timestamp> accept,
                function::BiPredicate<E, function::Timestamp> interrupt) mutable -> void {
                function::Timestamp count = 0;
                generator(
                    [&accept, &count, &predicate](E element, function::Timestamp index) -> void {
                        bool keep;
                        if constexpr (std::is_invocable_r_v<bool, Predicate, E, function::Timestamp>)
                        {
                            keep = std::invoke(predicate, element, index);
                        }
                        else
                        {
                            keep = std::invoke(predicate, element);
                        }
                        if (keep)
                        {
                            accept(element, count);
                            count++;
                        }
                    },
                    [&interrupt, &count](E element, function::Timestamp index) -> bool {
                        return interrupt(element, count);
                    });
            },
            this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](E element, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, E, function::Timestamp>)
                {
                    matches = std::invoke(predicate, element, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, E>)
                {
                    matches = std::invoke(predicate, element);
                }
                if (matches)
                {
                    accept(element, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](E element, function::Timestamp index) -> bool { return interrupt(element, index) || stop; });
        },
                           this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](E element, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, E, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, element, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, E>)
                    {
                        matches = std::invoke(predicate, element);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(element, count);
                        count++;
                    }
                }
                else
                {
                    accept(element, count);
                    count++;
                } }, [&interrupt, &count](E element, function::Timestamp index) -> bool { return interrupt(element, count); });
        },
                           this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), n](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](E element, function::Timestamp index) -> void {
                accept(element, count);
                count++; }, [&interrupt, &count, &n](E element, function::Timestamp index) -> bool { return interrupt(element, count) || count >= n; });
        },
                           this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), n](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](E element, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(element, count);
                }
                count++; }, [&interrupt, &count](E element, function::Timestamp index) -> bool { return interrupt(element, count); });
        },
                           this->concurrent);
    }

    auto distinct() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            std::unordered_set<E> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](E element, function::Timestamp index) -> void {
                if (seen.find(element) == seen.end())
                {
                    seen.insert(element);
                    accept(element, count);
                    count++;
                } }, [&interrupt, &count](E element, function::Timestamp index) -> bool { return interrupt(element, count); });
        },
                           this->concurrent);
    }

    auto distinct(const function::Comparator<E> &comparator) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), comparator](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            std::set<E, function::Comparator<E>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](E element, function::Timestamp index) -> void {
                if (seen.find(element) == seen.end())
                {
                    seen.insert(element);
                    accept(element, count);
                    count++;
                } }, [&interrupt, &count](E element, function::Timestamp index) -> bool { return interrupt(element, count); });
        },
                           this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](E element, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, E, function::Timestamp>)
                {
                    std::invoke(consumer, element, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, E>)
                {
                    std::invoke(consumer, element);
                }
                accept(element, index);
            },
                      interrupt);
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten](E element, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, E, function::Timestamp>)
                {
                    Semantic<E> inner = std::invoke(flatten, element, index);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&stop](E innerElement, function::Timestamp innerIndex) -> bool {
                        return stop;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, E>)
                {
                    Semantic<E> inner = std::invoke(flatten, element);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&stop](E innerElement, function::Timestamp innerIndex) -> bool {
                        return stop;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, E, function::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, E>, "Flatten must be callable as either (E, Timestamp) -> Semantic<E> or (E) -> Semantic<E>");
                } }, [&interrupt, &stop](E element, function::Timestamp index) -> bool { return interrupt(element, index) || stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_r_v<Semantic, Flatten, E, function::Timestamp>)
        {
            using R = typename std::invoke_result_t<Flatten, E, function::Timestamp>::Element;
            static_assert(!std::is_same_v<R, void>, "Flatten must not return Semantic<void>");
            return Semantic<R>(
                [generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](
                    function::BiConsumer<R, function::Timestamp> accept,
                    function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                    function::Timestamp count = 0LL;
                    bool stop = false;
                    generator([&accept, &count, &stop, &flatten](E element, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, element, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&stop](R innerElement, function::Timestamp innerIndex) -> bool {
                        return stop;
                    }); }, [&interrupt, &stop](E element, function::Timestamp index) -> bool { return interrupt(element, index) || stop; });
                },
                this->concurrent);
        }
        else if constexpr (std::is_invocable_r_v<Semantic, Flatten, E>)
        {
            using R = typename std::invoke_result_t<Flatten, E>::Element;
            static_assert(!std::is_same_v<R, void>, "Flatten must not return Semantic<void>");
            return Semantic<R>(
                [generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](
                    function::BiConsumer<R, function::Timestamp> accept,
                    function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                    function::Timestamp count = 0LL;
                    bool stop = false;
                    generator([&accept, &count, &stop, &flatten](E element, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, element);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&stop](R innerElement, function::Timestamp innerIndex) -> bool {
                        return stop;
                    }); }, [&interrupt, &stop](E element, function::Timestamp index) -> bool { return interrupt(element, index) || stop; });
                },
                this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_r_v<Semantic, Flatten, E, function::Timestamp> ||
                              std::is_invocable_r_v<Semantic, Flatten, E>,
                          "Flatten must be callable as either (E, Timestamp) -> Semantic<R> or (E) -> Semantic<R>");
        }
    }

    auto parallel() const -> Semantic<E>
    {
        return Semantic<E>(this->source(), 1);
    }

    auto parallel(const function::Module &concurrent) const -> Semantic<E>
    {
        return Semantic<E>(this->source(), std::max(concurrent, 1ULL));
    }

    auto reverse() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            generator([&accept](E element, function::Timestamp index) -> void { accept(element, -index); }, [&interrupt](E element, function::Timestamp index) -> bool { return interrupt(element, -index); });
        },
                           this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), offset](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](E element, function::Timestamp index) -> void { accept(element, index + offset); }, [&interrupt, &offset](E element, function::Timestamp index) -> bool { return interrupt(element, index + offset); });
        },
                           this->concurrent);
    }

    auto translate(const function::BiFunction<E, function::Timestamp, function::Timestamp> &translator) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), translator](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](E element, function::Timestamp index) -> void { accept(element, translator(element, index)); }, [&interrupt, &translator](E element, function::Timestamp index) -> bool { return interrupt(element, translator(element, index)); });
        },
                           this->concurrent);
    }

    auto redirect(const function::BiFunction<E, function::Timestamp, E> &redirector) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), redirector](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](E element, function::Timestamp index) -> void { accept(redirector(element, index), index); }, [&interrupt, &redirector](E element, function::Timestamp index) -> bool { return interrupt(redirector(element, index), index); });
        },
                           this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), start, end](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](E element, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(element, count);
                }
                count++; }, [&interrupt, &count, &end](E element, function::Timestamp index) -> bool { return interrupt(element, count) || count >= end; });
        },
                           this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<E>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<E>>)
        {
            return Semantic<E>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](E element, function::Timestamp index) -> void {
                    accept(element, count);
                    count++; }, [&stop](E element, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](E element, function::Timestamp index) -> void {
                    accept(element, count);
                    count++; }, [&interrupt, &stop, &count](E element, function::Timestamp index) -> bool {
                    if (interrupt(element, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                               this->concurrent);
        }
        else
        {
            return Semantic<E>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](E element, function::Timestamp index) -> void {
                    accept(element, count);
                    count++; }, [&interrupt, &count](E element, function::Timestamp index) -> bool { return interrupt(element, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                               this->concurrent);
        }
    }

    auto sort() const -> collectable::OrderedCollectable<E>
    {
        if constexpr (std::is_invocable_v<std::less<E>, E, E>)
        {
            return collectable::OrderedCollectable<E>(
                this->source(),
                [](const E &a, const E &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<E>(
                this->source(),
                this->concurrent);
        }
    }

    auto sort(const function::Comparator<E> &comparator) const -> collectable::OrderedCollectable<E>
    {
        return collectable::OrderedCollectable<E>(this->source(), comparator, this->concurrent);
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<E>
    {
        return collectable::UnorderedCollectable<E>(this->source(), this->concurrent);
    }

    auto toOrdered() const -> collectable::OrderedCollectable<E>
    {
        return collectable::OrderedCollectable<E>(this->source(), this->concurrent);
    }

    auto toWindow() const -> collectable::WindowCollectable<E>
    {
        return collectable::WindowCollectable<E>(this->source(), this->concurrent);
    }

    template <typename D>
    auto toStatistics() const -> collectable::Statistics<E, D>
    {
        return collectable::Statistics<E, D>(this->source(), this->concurrent);
    }
};

template <typename E>
class Semantic<Semantic<E>>
{
  private:
    std::unique_ptr<function::Generator<Semantic<E>>> generator;
    function::Module concurrent;

  public:
    using Element = Semantic<E>;

    Semantic(Semantic<Semantic<E>> &&other) noexcept = default;

    Semantic(const function::Generator<Semantic<E>> &generator) : generator(std::make_unique<function::Generator<Semantic<E>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<Semantic<E>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<Semantic<E>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<Semantic<E>> &other) : generator(std::make_unique<function::Generator<Semantic<E>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic<Semantic<E>> &operator=(const Semantic<Semantic<E>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<Semantic<E>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<Semantic<E>> &operator=(Semantic<Semantic<E>> &&other) noexcept = default;

    auto source() const -> function::Generator<Semantic<E>>
    {
        return [generator = *(this->generator)](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module
    {
        return concurrent;
    }

    auto flat() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop](Semantic<E> inner, function::Timestamp index) -> void { inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++; }, [&stop](E innerElement, function::Timestamp innerIndex) -> bool { return stop; }); }, [&interrupt, &stop, &count](Semantic<E> inner, function::Timestamp index) -> bool {
                if (interrupt(inner, count))
                {
                    stop = true;
                    return true;
                }
                return false; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](Semantic<E> element, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, Semantic<E>, function::Timestamp>)
                {
                    Semantic<E> inner = std::invoke(flatten, element, index);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, Semantic<E>>)
                {
                    Semantic<E> inner = std::invoke(flatten, element);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, Semantic<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, Semantic<E>>, "Flatten must be callable as either (Semantic<E>, Timestamp) -> Semantic<E> or (Semantic<E>) -> Semantic<E>");
                } }, [&stop](Semantic<E> element, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, Semantic<E>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, Semantic<E>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](Semantic<E> element, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, element, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](Semantic<E> element, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, Semantic<E>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, Semantic<E>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](Semantic<E> element, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, element);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](Semantic<E> element, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, Semantic<E>, function::Timestamp> || std::is_invocable_v<Flatten, Semantic<E>>, "Flatten must be callable as either (Semantic<E>, Timestamp) -> Semantic<R> or (Semantic<E>) -> Semantic<R>");
        }
    }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, Semantic<E>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, Semantic<E>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](Semantic<E> element, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, element, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](Semantic<E> element, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, Semantic<E>>)
        {
            using R = std::invoke_result_t<Mapper, Semantic<E>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](Semantic<E> element, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, element);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](Semantic<E> element, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, Semantic<E>, function::Timestamp> || std::is_invocable_v<Mapper, Semantic<E>>, "Mapper must be callable as either (Semantic<E>, Timestamp) -> R or (Semantic<E>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](Semantic<E> element, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, Semantic<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, element, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, Semantic<E>>)
                {
                    matches = std::invoke(predicate, element);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, Semantic<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, Semantic<E>>, "Predicate must be callable as either (Semantic<E>, Timestamp) -> bool or (Semantic<E>) -> bool");
                }
                if (matches)
                {
                    accept(element, count);
                    count++;
                } }, [&interrupt, &count](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(element, count); });
        },
                                     this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](Semantic<E> element, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, Semantic<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, element, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, Semantic<E>>)
                {
                    matches = std::invoke(predicate, element);
                }
                if (matches)
                {
                    accept(element, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(element, index) || stop; });
        },
                                     this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](Semantic<E> element, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, Semantic<E>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, element, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, Semantic<E>>)
                    {
                        matches = std::invoke(predicate, element);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(element, count);
                        count++;
                    }
                }
                else
                {
                    accept(element, count);
                    count++;
                } }, [&interrupt, &count](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(element, count); });
        },
                                     this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator), n](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](Semantic<E> element, function::Timestamp index) -> void {
                accept(element, count);
                count++; }, [&interrupt, &count, &n](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(element, count) || count >= n; });
        },
                                     this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator), n](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](Semantic<E> element, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(element, count);
                }
                count++; }, [&interrupt, &count](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(element, count); });
        },
                                     this->concurrent);
    }

    auto distinct() const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator)](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            std::unordered_set<Semantic<E>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](Semantic<E> element, function::Timestamp index) -> void {
                if (seen.find(element) == seen.end())
                {
                    seen.insert(element);
                    accept(element, count);
                    count++;
                } }, [&interrupt, &count](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(element, count); });
        },
                                     this->concurrent);
    }

    auto distinct(const function::Comparator<Semantic<E>> &comparator) const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator), comparator](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            std::set<Semantic<E>, function::Comparator<Semantic<E>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](Semantic<E> element, function::Timestamp index) -> void {
                if (seen.find(element) == seen.end())
                {
                    seen.insert(element);
                    accept(element, count);
                    count++;
                } }, [&interrupt, &count](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(element, count); });
        },
                                     this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](Semantic<E> element, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, Semantic<E>, function::Timestamp>)
                {
                    std::invoke(consumer, element, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, Semantic<E>>)
                {
                    std::invoke(consumer, element);
                }
                accept(element, index);
            },
                      interrupt);
        },
                                     this->concurrent);
    }

    auto parallel() const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>(this->source(), 1);
    }

    auto parallel(const function::Module &concurrent) const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>(this->source(), std::max(concurrent, 1ULL));
    }

    auto reverse() const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator)](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            generator([&accept](Semantic<E> element, function::Timestamp index) -> void { accept(element, -index); }, [&interrupt](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(element, -index); });
        },
                                     this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator), offset](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](Semantic<E> element, function::Timestamp index) -> void { accept(element, index + offset); }, [&interrupt, &offset](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(element, index + offset); });
        },
                                     this->concurrent);
    }

    auto translate(const function::BiFunction<Semantic<E>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator), translator](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](Semantic<E> element, function::Timestamp index) -> void { accept(element, translator(element, index)); }, [&interrupt, &translator](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(element, translator(element, index)); });
        },
                                     this->concurrent);
    }

    auto redirect(const function::BiFunction<Semantic<E>, function::Timestamp, Semantic<E>> &redirector) const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator), redirector](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](Semantic<E> element, function::Timestamp index) -> void { accept(redirector(element, index), index); }, [&interrupt, &redirector](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(redirector(element, index), index); });
        },
                                     this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<Semantic<E>>
    {
        return Semantic<Semantic<E>>([generator = *(this->generator), start, end](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](Semantic<E> element, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(element, count);
                }
                count++; }, [&interrupt, &count, &end](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(element, count) || count >= end; });
        },
                                     this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<Semantic<E>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<Semantic<E>>>)
        {
            return Semantic<Semantic<E>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](Semantic<E> element, function::Timestamp index) -> void {
                    accept(element, count);
                    count++; }, [&stop](Semantic<E> element, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](Semantic<E> element, function::Timestamp index) -> void {
                    accept(element, count);
                    count++; }, [&interrupt, &stop, &count](Semantic<E> element, function::Timestamp index) -> bool {
                    if (interrupt(element, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                         this->concurrent);
        }
        else
        {
            return Semantic<Semantic<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<Semantic<E>, function::Timestamp> accept, function::BiPredicate<Semantic<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](Semantic<E> element, function::Timestamp index) -> void {
                    accept(element, count);
                    count++; }, [&interrupt, &count](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(element, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                         this->concurrent);
        }
    }

    auto sort() const -> collectable::OrderedCollectable<Semantic<E>>
    {
        if constexpr (std::is_invocable_v<std::less<Semantic<E>>, Semantic<E>, Semantic<E>>)
        {
            return collectable::OrderedCollectable<Semantic<E>>(
                this->source(),
                [](const Semantic<E> &a, const Semantic<E> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<Semantic<E>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<Semantic<E>> &comparator) const -> collectable::OrderedCollectable<Semantic<E>>
    {
        return collectable::OrderedCollectable<Semantic<E>>(this->source(), comparator, this->concurrent);
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<Semantic<E>>
    {
        return collectable::UnorderedCollectable<Semantic<E>>(this->source(), this->concurrent);
    }

    auto toOrdered() const -> collectable::OrderedCollectable<Semantic<E>>
    {
        return collectable::OrderedCollectable<Semantic<E>>(this->source(), this->concurrent);
    }

    auto toWindow() const -> collectable::WindowCollectable<Semantic<E>>
    {
        return collectable::WindowCollectable<Semantic<E>>(this->source(), this->concurrent);
    }

    template <typename D>
    auto toStatistics() const -> collectable::Statistics<Semantic<E>, D>
    {
        return collectable::Statistics<Semantic<E>, D>(this->source(), this->concurrent);
    }
};

template <typename E>
class Semantic<std::vector<E>>
{
  private:
    std::unique_ptr<function::Generator<std::vector<E>>> generator;
    function::Module concurrent;

  public:
    using Element = std::vector<E>;

    Semantic(std::vector<E> &&container) : generator(std::make_unique<function::Generator<std::vector<E>>>([elements = std::move(container)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
                                               accept(elements, 0LL);
                                           })),
                                           concurrent(1) {}

    Semantic(std::vector<E> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::vector<E>>>([elements = std::move(container)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
                                                                                   accept(elements, 0LL);
                                                                               })),
                                                                               concurrent(concurrent) {}

    Semantic(const function::Generator<std::vector<E>> &generator) : generator(std::make_unique<function::Generator<std::vector<E>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::vector<E>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::vector<E>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::vector<E>> &other) : generator(std::make_unique<function::Generator<std::vector<E>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::vector<E>> &&other) noexcept = default;

    Semantic<std::vector<E>> &operator=(const Semantic<std::vector<E>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::vector<E>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::vector<E>> &operator=(Semantic<std::vector<E>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::vector<E>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::vector<E>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::vector<E>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::vector<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::vector<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::vector<E>>)
        {
            using R = std::invoke_result_t<Mapper, std::vector<E>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::vector<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::vector<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::vector<E>, function::Timestamp> || std::is_invocable_v<Mapper, std::vector<E>>, "Mapper must be callable as either (vector<E>, Timestamp) -> R or (vector<E>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::vector<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::vector<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::vector<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::vector<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::vector<E>>, "Predicate must be callable as either (vector<E>, Timestamp) -> bool or (vector<E>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::vector<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::vector<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::vector<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                        this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::vector<E> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::vector<E>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::vector<E>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    auto distinct() const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::vector<E>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::vector<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    auto distinct(const function::Comparator<std::vector<E>> &comparator) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), comparator](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            std::set<std::vector<E>, function::Comparator<std::vector<E>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::vector<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::vector<E>>
    {
        if constexpr (std::is_invocable_v<std::less<std::vector<E>>, std::vector<E>, std::vector<E>>)
        {
            return collectable::OrderedCollectable<std::vector<E>>(
                this->source(),
                [](const std::vector<E> &a, const std::vector<E> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::vector<E>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::vector<E>> &comparator) const -> collectable::OrderedCollectable<std::vector<E>>
    {
        return collectable::OrderedCollectable<std::vector<E>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), n](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::vector<E> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                        this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), n](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::vector<E> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::vector<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::vector<E>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::vector<E>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                        this->concurrent);
    }

    auto flat() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::vector<E> container, function::Timestamp index) -> void {
                for (const auto &element : container)
                {
                    if (stop)
                    {
                        break;
                    }
                    if (interrupt(element, count))
                    {
                        stop = true;
                        break;
                    }
                    accept(element, count);
                    count++;
                } }, [&stop](std::vector<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::vector<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::vector<E>, function::Timestamp>)
                {
                    Semantic<E> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::vector<E>>)
                {
                    Semantic<E> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, std::vector<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, std::vector<E>>, "flat: Flatten must return Semantic<E>");
                } }, [&stop](std::vector<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::vector<E>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::vector<E>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::vector<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::vector<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::vector<E>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::vector<E>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::vector<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::vector<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::vector<E>, function::Timestamp> || std::is_invocable_v<Flatten, std::vector<E>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::vector<E>> { return Semantic<std::vector<E>>(this->source(), 1); }

    auto parallel(const function::Module &concurrent) const -> Semantic<std::vector<E>> { return Semantic<std::vector<E>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::vector<E> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                        this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), offset](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::vector<E> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                        this->concurrent);
    }

    auto translate(const function::BiFunction<std::vector<E>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), translator](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::vector<E> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                        this->concurrent);
    }

    auto redirect(const function::BiFunction<std::vector<E>, function::Timestamp, std::vector<E>> &redirector) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), redirector](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::vector<E> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                        this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), start, end](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::vector<E> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                        this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::vector<E>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::vector<E>>>)
        {
            return Semantic<std::vector<E>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::vector<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::vector<E> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::vector<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::vector<E> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                            this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::vector<E>>)
        {
            return Semantic<std::vector<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::vector<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                            this->concurrent);
        }
        else
        {
            return Semantic<std::vector<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::vector<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                            this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::vector<E>> { return collectable::UnorderedCollectable<std::vector<E>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::vector<E>> { return collectable::OrderedCollectable<std::vector<E>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::vector<E>> { return collectable::WindowCollectable<std::vector<E>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::vector<E>, D> { return collectable::Statistics<std::vector<E>, D>(this->source(), this->concurrent); }
};

template <typename E>
class Semantic<std::list<E>>
{
  private:
    std::unique_ptr<function::Generator<std::list<E>>> generator;
    function::Module concurrent;

  public:
    using Element = std::list<E>;

    Semantic(std::list<E> &&container) : generator(std::make_unique<function::Generator<std::list<E>>>([elements = std::move(container)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
                                             accept(elements, 0LL);
                                         })),
                                         concurrent(1) {}

    Semantic(std::list<E> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::list<E>>>([elements = std::move(container)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
                                                                                 accept(elements, 0LL);
                                                                             })),
                                                                             concurrent(concurrent) {}

    Semantic(const function::Generator<std::list<E>> &generator) : generator(std::make_unique<function::Generator<std::list<E>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::list<E>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::list<E>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::list<E>> &other) : generator(std::make_unique<function::Generator<std::list<E>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::list<E>> &&other) noexcept = default;

    Semantic<std::list<E>> &operator=(const Semantic<std::list<E>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::list<E>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::list<E>> &operator=(Semantic<std::list<E>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::list<E>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::list<E>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::list<E>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::list<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::list<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::list<E>>)
        {
            using R = std::invoke_result_t<Mapper, std::list<E>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::list<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::list<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::list<E>, function::Timestamp> || std::is_invocable_v<Mapper, std::list<E>>, "Mapper must be callable as either (list<E>, Timestamp) -> R or (list<E>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::list<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::list<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::list<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::list<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::list<E>>, "Predicate must be callable as either (list<E>, Timestamp) -> bool or (list<E>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                      this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::list<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::list<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::list<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                      this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::list<E> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::list<E>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::list<E>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                      this->concurrent);
    }

    auto distinct() const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::list<E>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::list<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                      this->concurrent);
    }

    auto distinct(const function::Comparator<std::list<E>> &comparator) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), comparator](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            std::set<std::list<E>, function::Comparator<std::list<E>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::list<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                      this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::list<E>>
    {
        if constexpr (std::is_invocable_v<std::less<std::list<E>>, std::list<E>, std::list<E>>)
        {
            return collectable::OrderedCollectable<std::list<E>>(
                this->source(),
                [](const std::list<E> &a, const std::list<E> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::list<E>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::list<E>> &comparator) const -> collectable::OrderedCollectable<std::list<E>>
    {
        return collectable::OrderedCollectable<std::list<E>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), n](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::list<E> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                      this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), n](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::list<E> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                      this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::list<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::list<E>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::list<E>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                      this->concurrent);
    }

    auto flat() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::list<E> container, function::Timestamp index) -> void {
                for (const auto &element : container)
                {
                    if (stop)
                    {
                        break;
                    }
                    if (interrupt(element, count))
                    {
                        stop = true;
                        break;
                    }
                    accept(element, count);
                    count++;
                } }, [&stop](std::list<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::list<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::list<E>, function::Timestamp>)
                {
                    Semantic<E> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::list<E>>)
                {
                    Semantic<E> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, std::list<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, std::list<E>>, "flat: Flatten must return Semantic<E>");
                } }, [&stop](std::list<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::list<E>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::list<E>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::list<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::list<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::list<E>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::list<E>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::list<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::list<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::list<E>, function::Timestamp> || std::is_invocable_v<Flatten, std::list<E>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::list<E>> { return Semantic<std::list<E>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::list<E>> { return Semantic<std::list<E>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::list<E> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                      this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), offset](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::list<E> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                      this->concurrent);
    }

    auto translate(const function::BiFunction<std::list<E>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), translator](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::list<E> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                      this->concurrent);
    }

    auto redirect(const function::BiFunction<std::list<E>, function::Timestamp, std::list<E>> &redirector) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), redirector](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::list<E> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::list<E> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                      this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), start, end](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::list<E> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                      this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::list<E>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::list<E>>>)
        {
            return Semantic<std::list<E>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::list<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::list<E> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::list<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::list<E> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                          this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::list<E>>)
        {
            return Semantic<std::list<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::list<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                          this->concurrent);
        }
        else
        {
            return Semantic<std::list<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::list<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::list<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                          this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::list<E>> { return collectable::UnorderedCollectable<std::list<E>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::list<E>> { return collectable::OrderedCollectable<std::list<E>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::list<E>> { return collectable::WindowCollectable<std::list<E>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::list<E>, D> { return collectable::Statistics<std::list<E>, D>(this->source(), this->concurrent); }
};

template <typename E, std::size_t N>
class Semantic<std::array<E, N>>
{
  private:
    std::unique_ptr<function::Generator<std::array<E, N>>> generator;
    function::Module concurrent;

  public:
    using Element = std::array<E, N>;

    Semantic(std::array<E, N> &&container) : generator(std::make_unique<function::Generator<std::array<E, N>>>([elements = std::move(container)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
                                                 accept(elements, 0LL);
                                             })),
                                             concurrent(1) {}

    Semantic(std::array<E, N> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::array<E, N>>>([elements = std::move(container)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
                                                                                     accept(elements, 0LL);
                                                                                 })),
                                                                                 concurrent(concurrent) {}

    Semantic(const function::Generator<std::array<E, N>> &generator) : generator(std::make_unique<function::Generator<std::array<E, N>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::array<E, N>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::array<E, N>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::array<E, N>> &other) : generator(std::make_unique<function::Generator<std::array<E, N>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::array<E, N>> &&other) noexcept = default;

    Semantic<std::array<E, N>> &operator=(const Semantic<std::array<E, N>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::array<E, N>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::array<E, N>> &operator=(Semantic<std::array<E, N>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::array<E, N>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::array<E, N>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::array<E, N>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::array<E, N> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::array<E, N> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::array<E, N>>)
        {
            using R = std::invoke_result_t<Mapper, std::array<E, N>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::array<E, N> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::array<E, N> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::array<E, N>, function::Timestamp> || std::is_invocable_v<Mapper, std::array<E, N>>, "Mapper must be callable as either (array<E,N>, Timestamp) -> R or (array<E,N>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::array<E, N> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::array<E, N>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::array<E, N>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::array<E, N>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::array<E, N>>, "Predicate must be callable as either (array<E,N>, Timestamp) -> bool or (array<E,N>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                          this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::array<E, N> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::array<E, N>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::array<E, N>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                          this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::array<E, N> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::array<E, N>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::array<E, N>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                          this->concurrent);
    }

    auto distinct() const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::array<E, N>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::array<E, N> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                          this->concurrent);
    }

    auto distinct(const function::Comparator<std::array<E, N>> &comparator) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), comparator](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            std::set<std::array<E, N>, function::Comparator<std::array<E, N>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::array<E, N> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                          this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::array<E, N>>
    {
        if constexpr (std::is_invocable_v<std::less<std::array<E, N>>, std::array<E, N>, std::array<E, N>>)
        {
            return collectable::OrderedCollectable<std::array<E, N>>(
                this->source(),
                [](const std::array<E, N> &a, const std::array<E, N> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::array<E, N>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::array<E, N>> &comparator) const -> collectable::OrderedCollectable<std::array<E, N>>
    {
        return collectable::OrderedCollectable<std::array<E, N>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), n](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::array<E, N> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                          this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), n](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::array<E, N> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                          this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::array<E, N> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::array<E, N>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::array<E, N>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                          this->concurrent);
    }

    auto flat() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::array<E, N> container, function::Timestamp index) -> void {
                for (const auto &element : container)
                {
                    if (stop)
                    {
                        break;
                    }
                    if (interrupt(element, count))
                    {
                        stop = true;
                        break;
                    }
                    accept(element, count);
                    count++;
                } }, [&stop](std::array<E, N> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::array<E, N> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::array<E, N>, function::Timestamp>)
                {
                    Semantic<E> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::array<E, N>>)
                {
                    Semantic<E> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, std::array<E, N>, function::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, std::array<E, N>>, "flat: Flatten must return Semantic<E>");
                } }, [&stop](std::array<E, N> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::array<E, N>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::array<E, N>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::array<E, N> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::array<E, N> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::array<E, N>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::array<E, N>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::array<E, N> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::array<E, N> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::array<E, N>, function::Timestamp> || std::is_invocable_v<Flatten, std::array<E, N>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::array<E, N>> { return Semantic<std::array<E, N>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::array<E, N>> { return Semantic<std::array<E, N>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::array<E, N> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                          this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), offset](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::array<E, N> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                          this->concurrent);
    }

    auto translate(const function::BiFunction<std::array<E, N>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), translator](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::array<E, N> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                          this->concurrent);
    }

    auto redirect(const function::BiFunction<std::array<E, N>, function::Timestamp, std::array<E, N>> &redirector) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), redirector](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::array<E, N> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                          this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), start, end](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::array<E, N> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                          this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::array<E, N>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::array<E, N>>>)
        {
            return Semantic<std::array<E, N>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::array<E, N> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::array<E, N> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::array<E, N> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::array<E, N> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                              this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::array<E, N>>)
        {
            return Semantic<std::array<E, N>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::array<E, N> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                              this->concurrent);
        }
        else
        {
            return Semantic<std::array<E, N>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::array<E, N> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                              this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::array<E, N>> { return collectable::UnorderedCollectable<std::array<E, N>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::array<E, N>> { return collectable::OrderedCollectable<std::array<E, N>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::array<E, N>> { return collectable::WindowCollectable<std::array<E, N>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::array<E, N>, D> { return collectable::Statistics<std::array<E, N>, D>(this->source(), this->concurrent); }
};

template <typename E>
class Semantic<std::deque<E>>
{
  private:
    std::unique_ptr<function::Generator<std::deque<E>>> generator;
    function::Module concurrent;

  public:
    using Element = std::deque<E>;

    Semantic(std::deque<E> &&container) : generator(std::make_unique<function::Generator<std::deque<E>>>([elements = std::move(container)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
                                              accept(elements, 0LL);
                                          })),
                                          concurrent(1) {}

    Semantic(std::deque<E> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::deque<E>>>([elements = std::move(container)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
                                                                                  accept(elements, 0LL);
                                                                              })),
                                                                              concurrent(concurrent) {}

    Semantic(const function::Generator<std::deque<E>> &generator) : generator(std::make_unique<function::Generator<std::deque<E>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::deque<E>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::deque<E>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::deque<E>> &other) : generator(std::make_unique<function::Generator<std::deque<E>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::deque<E>> &&other) noexcept = default;

    Semantic<std::deque<E>> &operator=(const Semantic<std::deque<E>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::deque<E>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::deque<E>> &operator=(Semantic<std::deque<E>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::deque<E>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::deque<E>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::deque<E>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::deque<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::deque<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::deque<E>>)
        {
            using R = std::invoke_result_t<Mapper, std::deque<E>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::deque<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::deque<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::deque<E>, function::Timestamp> || std::is_invocable_v<Mapper, std::deque<E>>, "Mapper must be callable as either (deque<E>, Timestamp) -> R or (deque<E>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::deque<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::deque<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::deque<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::deque<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::deque<E>>, "Predicate must be callable as either (deque<E>, Timestamp) -> bool or (deque<E>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::deque<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::deque<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::deque<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                       this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::deque<E> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::deque<E>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::deque<E>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    auto distinct() const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::deque<E>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::deque<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    auto distinct(const function::Comparator<std::deque<E>> &comparator) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), comparator](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            std::set<std::deque<E>, function::Comparator<std::deque<E>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::deque<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::deque<E>>
    {
        if constexpr (std::is_invocable_v<std::less<std::deque<E>>, std::deque<E>, std::deque<E>>)
        {
            return collectable::OrderedCollectable<std::deque<E>>(
                this->source(),
                [](const std::deque<E> &a, const std::deque<E> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::deque<E>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::deque<E>> &comparator) const -> collectable::OrderedCollectable<std::deque<E>>
    {
        return collectable::OrderedCollectable<std::deque<E>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), n](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::deque<E> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                       this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), n](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::deque<E> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::deque<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::deque<E>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::deque<E>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                       this->concurrent);
    }

    auto flat() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::deque<E> container, function::Timestamp index) -> void {
                for (const auto &element : container)
                {
                    if (stop)
                    {
                        break;
                    }
                    if (interrupt(element, count))
                    {
                        stop = true;
                        break;
                    }
                    accept(element, count);
                    count++;
                } }, [&stop](std::deque<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::deque<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::deque<E>, function::Timestamp>)
                {
                    Semantic<E> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::deque<E>>)
                {
                    Semantic<E> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, std::deque<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, std::deque<E>>, "flat: Flatten must return Semantic<E>");
                } }, [&stop](std::deque<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::deque<E>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::deque<E>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::deque<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::deque<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::deque<E>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::deque<E>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::deque<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::deque<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::deque<E>, function::Timestamp> || std::is_invocable_v<Flatten, std::deque<E>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::deque<E>> { return Semantic<std::deque<E>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::deque<E>> { return Semantic<std::deque<E>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::deque<E> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                       this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), offset](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::deque<E> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                       this->concurrent);
    }

    auto translate(const function::BiFunction<std::deque<E>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), translator](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::deque<E> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                       this->concurrent);
    }

    auto redirect(const function::BiFunction<std::deque<E>, function::Timestamp, std::deque<E>> &redirector) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), redirector](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::deque<E> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                       this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), start, end](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::deque<E> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                       this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::deque<E>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::deque<E>>>)
        {
            return Semantic<std::deque<E>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::deque<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::deque<E> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::deque<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::deque<E> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                           this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::deque<E>>)
        {
            return Semantic<std::deque<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::deque<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                           this->concurrent);
        }
        else
        {
            return Semantic<std::deque<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::deque<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                           this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::deque<E>> { return collectable::UnorderedCollectable<std::deque<E>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::deque<E>> { return collectable::OrderedCollectable<std::deque<E>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::deque<E>> { return collectable::WindowCollectable<std::deque<E>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::deque<E>, D> { return collectable::Statistics<std::deque<E>, D>(this->source(), this->concurrent); }
};

template <typename E>
class Semantic<std::queue<E>>
{
  private:
    std::unique_ptr<function::Generator<std::queue<E>>> generator;
    function::Module concurrent;

  public:
    using Element = std::queue<E>;

    Semantic(std::queue<E> &&container) : generator(std::make_unique<function::Generator<std::queue<E>>>([elements = std::move(container)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
                                              accept(elements, 0LL);
                                          })),
                                          concurrent(1) {}

    Semantic(std::queue<E> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::queue<E>>>([elements = std::move(container)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
                                                                                  accept(elements, 0LL);
                                                                              })),
                                                                              concurrent(concurrent) {}

    Semantic(const function::Generator<std::queue<E>> &generator) : generator(std::make_unique<function::Generator<std::queue<E>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::queue<E>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::queue<E>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::queue<E>> &other) : generator(std::make_unique<function::Generator<std::queue<E>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::queue<E>> &&other) noexcept = default;

    Semantic<std::queue<E>> &operator=(const Semantic<std::queue<E>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::queue<E>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::queue<E>> &operator=(Semantic<std::queue<E>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::queue<E>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::queue<E>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::queue<E>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::queue<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::queue<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::queue<E>>)
        {
            using R = std::invoke_result_t<Mapper, std::queue<E>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::queue<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::queue<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::queue<E>, function::Timestamp> || std::is_invocable_v<Mapper, std::queue<E>>, "Mapper must be callable as either (queue<E>, Timestamp) -> R or (queue<E>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::queue<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::queue<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::queue<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::queue<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::queue<E>>, "Predicate must be callable as either (queue<E>, Timestamp) -> bool or (queue<E>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::queue<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::queue<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::queue<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                       this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::queue<E> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::queue<E>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::queue<E>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    auto distinct() const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::queue<E>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::queue<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    auto distinct(const function::Comparator<std::queue<E>> &comparator) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), comparator](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            std::set<std::queue<E>, function::Comparator<std::queue<E>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::queue<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::queue<E>>
    {
        if constexpr (std::is_invocable_v<std::less<std::queue<E>>, std::queue<E>, std::queue<E>>)
        {
            return collectable::OrderedCollectable<std::queue<E>>(
                this->source(),
                [](const std::queue<E> &a, const std::queue<E> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::queue<E>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::queue<E>> &comparator) const -> collectable::OrderedCollectable<std::queue<E>>
    {
        return collectable::OrderedCollectable<std::queue<E>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), n](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::queue<E> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                       this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), n](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::queue<E> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::queue<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::queue<E>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::queue<E>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                       this->concurrent);
    }

    auto flat() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::queue<E> container, function::Timestamp index) -> void {
                std::queue<E> temp = container;
                while (!temp.empty())
                {
                    if (stop)
                    {
                        break;
                    }
                    if (interrupt(temp.front(), count))
                    {
                        stop = true;
                        break;
                    }
                    accept(temp.front(), count);
                    count++;
                    temp.pop();
                } }, [&stop](std::queue<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::queue<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::queue<E>, function::Timestamp>)
                {
                    Semantic<E> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::queue<E>>)
                {
                    Semantic<E> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, std::queue<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, std::queue<E>>, "flat: Flatten must return Semantic<E>");
                } }, [&stop](std::queue<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::queue<E>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::queue<E>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::queue<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::queue<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::queue<E>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::queue<E>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::queue<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::queue<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::queue<E>, function::Timestamp> || std::is_invocable_v<Flatten, std::queue<E>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::queue<E>> { return Semantic<std::queue<E>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::queue<E>> { return Semantic<std::queue<E>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::queue<E> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                       this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), offset](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::queue<E> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                       this->concurrent);
    }

    auto translate(const function::BiFunction<std::queue<E>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), translator](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::queue<E> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                       this->concurrent);
    }

    auto redirect(const function::BiFunction<std::queue<E>, function::Timestamp, std::queue<E>> &redirector) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), redirector](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::queue<E> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                       this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), start, end](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::queue<E> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                       this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::queue<E>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::queue<E>>>)
        {
            return Semantic<std::queue<E>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::queue<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::queue<E> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::queue<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::queue<E> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                           this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::queue<E>>)
        {
            return Semantic<std::queue<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::queue<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                           this->concurrent);
        }
        else
        {
            return Semantic<std::queue<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::queue<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                           this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::queue<E>> { return collectable::UnorderedCollectable<std::queue<E>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::queue<E>> { return collectable::OrderedCollectable<std::queue<E>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::queue<E>> { return collectable::WindowCollectable<std::queue<E>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::queue<E>, D> { return collectable::Statistics<std::queue<E>, D>(this->source(), this->concurrent); }
};

template <typename E>
class Semantic<std::stack<E>>
{
  private:
    std::unique_ptr<function::Generator<std::stack<E>>> generator;
    function::Module concurrent;

  public:
    using Element = std::stack<E>;

    Semantic(std::stack<E> &&container) : generator(std::make_unique<function::Generator<std::stack<E>>>([elements = std::move(container)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
                                              accept(elements, 0LL);
                                          })),
                                          concurrent(1) {}

    Semantic(std::stack<E> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::stack<E>>>([elements = std::move(container)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
                                                                                  accept(elements, 0LL);
                                                                              })),
                                                                              concurrent(concurrent) {}

    Semantic(const function::Generator<std::stack<E>> &generator) : generator(std::make_unique<function::Generator<std::stack<E>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::stack<E>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::stack<E>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::stack<E>> &other) : generator(std::make_unique<function::Generator<std::stack<E>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::stack<E>> &&other) noexcept = default;

    Semantic<std::stack<E>> &operator=(const Semantic<std::stack<E>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::stack<E>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::stack<E>> &operator=(Semantic<std::stack<E>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::stack<E>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::stack<E>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::stack<E>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::stack<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::stack<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::stack<E>>)
        {
            using R = std::invoke_result_t<Mapper, std::stack<E>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::stack<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::stack<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::stack<E>, function::Timestamp> || std::is_invocable_v<Mapper, std::stack<E>>, "Mapper must be callable as either (stack<E>, Timestamp) -> R or (stack<E>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::stack<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::stack<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::stack<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::stack<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::stack<E>>, "Predicate must be callable as either (stack<E>, Timestamp) -> bool or (stack<E>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::stack<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::stack<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::stack<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                       this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::stack<E> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::stack<E>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::stack<E>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    auto distinct() const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::stack<E>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::stack<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    auto distinct(const function::Comparator<std::stack<E>> &comparator) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), comparator](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            std::set<std::stack<E>, function::Comparator<std::stack<E>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::stack<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::stack<E>>
    {
        if constexpr (std::is_invocable_v<std::less<std::stack<E>>, std::stack<E>, std::stack<E>>)
        {
            return collectable::OrderedCollectable<std::stack<E>>(
                this->source(),
                [](const std::stack<E> &a, const std::stack<E> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::stack<E>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::stack<E>> &comparator) const -> collectable::OrderedCollectable<std::stack<E>>
    {
        return collectable::OrderedCollectable<std::stack<E>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), n](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::stack<E> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                       this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), n](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::stack<E> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                       this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::stack<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::stack<E>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::stack<E>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                       this->concurrent);
    }

    auto flat() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::stack<E> container, function::Timestamp index) -> void {
                std::stack<E> temp = container;
                std::vector<E> elements;
                while (!temp.empty())
                {
                    elements.push_back(temp.top());
                    temp.pop();
                }
                for (auto it = elements.rbegin(); it != elements.rend(); ++it)
                {
                    if (stop)
                    {
                        break;
                    }
                    if (interrupt(*it, count))
                    {
                        stop = true;
                        break;
                    }
                    accept(*it, count);
                    count++;
                } }, [&stop](std::stack<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::stack<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::stack<E>, function::Timestamp>)
                {
                    Semantic<E> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::stack<E>>)
                {
                    Semantic<E> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, std::stack<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, std::stack<E>>, "flat: Flatten must return Semantic<E>");
                } }, [&stop](std::stack<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::stack<E>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::stack<E>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::stack<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::stack<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::stack<E>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::stack<E>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::stack<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::stack<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::stack<E>, function::Timestamp> || std::is_invocable_v<Flatten, std::stack<E>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::stack<E>> { return Semantic<std::stack<E>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::stack<E>> { return Semantic<std::stack<E>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::stack<E> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                       this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), offset](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::stack<E> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                       this->concurrent);
    }

    auto translate(const function::BiFunction<std::stack<E>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), translator](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::stack<E> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                       this->concurrent);
    }

    auto redirect(const function::BiFunction<std::stack<E>, function::Timestamp, std::stack<E>> &redirector) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), redirector](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::stack<E> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                       this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), start, end](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::stack<E> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                       this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::stack<E>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::stack<E>>>)
        {
            return Semantic<std::stack<E>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::stack<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::stack<E> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::stack<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::stack<E> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                           this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::stack<E>>)
        {
            return Semantic<std::stack<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::stack<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                           this->concurrent);
        }
        else
        {
            return Semantic<std::stack<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::stack<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                           this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::stack<E>> { return collectable::UnorderedCollectable<std::stack<E>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::stack<E>> { return collectable::OrderedCollectable<std::stack<E>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::stack<E>> { return collectable::WindowCollectable<std::stack<E>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::stack<E>, D> { return collectable::Statistics<std::stack<E>, D>(this->source(), this->concurrent); }
};

template <typename E>
class Semantic<std::set<E>>
{
  private:
    std::unique_ptr<function::Generator<std::set<E>>> generator;
    function::Module concurrent;

  public:
    using Element = std::set<E>;

    Semantic(std::set<E> &&container) : generator(std::make_unique<function::Generator<std::set<E>>>([elements = std::move(container)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
                                            accept(elements, 0LL);
                                        })),
                                        concurrent(1) {}

    Semantic(std::set<E> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::set<E>>>([elements = std::move(container)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
                                                                                accept(elements, 0LL);
                                                                            })),
                                                                            concurrent(concurrent) {}

    Semantic(const function::Generator<std::set<E>> &generator) : generator(std::make_unique<function::Generator<std::set<E>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::set<E>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::set<E>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::set<E>> &other) : generator(std::make_unique<function::Generator<std::set<E>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::set<E>> &&other) noexcept = default;

    Semantic<std::set<E>> &operator=(const Semantic<std::set<E>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::set<E>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::set<E>> &operator=(Semantic<std::set<E>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::set<E>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::set<E>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::set<E>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::set<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::set<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::set<E>>)
        {
            using R = std::invoke_result_t<Mapper, std::set<E>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::set<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::set<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::set<E>, function::Timestamp> || std::is_invocable_v<Mapper, std::set<E>>, "Mapper must be callable as either (set<E>, Timestamp) -> R or (set<E>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::set<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::set<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::set<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::set<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::set<E>>, "Predicate must be callable as either (set<E>, Timestamp) -> bool or (set<E>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                     this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::set<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::set<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::set<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                     this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::set<E> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::set<E>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::set<E>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                     this->concurrent);
    }

    auto distinct() const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::set<E>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::set<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                     this->concurrent);
    }

    auto distinct(const function::Comparator<std::set<E>> &comparator) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), comparator](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            std::set<std::set<E>, function::Comparator<std::set<E>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::set<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                     this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::set<E>>
    {
        if constexpr (std::is_invocable_v<std::less<std::set<E>>, std::set<E>, std::set<E>>)
        {
            return collectable::OrderedCollectable<std::set<E>>(
                this->source(),
                [](const std::set<E> &a, const std::set<E> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::set<E>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::set<E>> &comparator) const -> collectable::OrderedCollectable<std::set<E>>
    {
        return collectable::OrderedCollectable<std::set<E>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), n](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::set<E> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                     this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), n](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::set<E> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                     this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::set<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::set<E>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::set<E>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                     this->concurrent);
    }

    auto flat() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::set<E> container, function::Timestamp index) -> void {
                for (const auto &element : container)
                {
                    if (stop)
                    {
                        break;
                    }
                    if (interrupt(element, count))
                    {
                        stop = true;
                        break;
                    }
                    accept(element, count);
                    count++;
                } }, [&stop](std::set<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::set<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::set<E>, function::Timestamp>)
                {
                    Semantic<E> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::set<E>>)
                {
                    Semantic<E> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, std::set<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, std::set<E>>, "flat: Flatten must return Semantic<E>");
                } }, [&stop](std::set<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::set<E>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::set<E>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::set<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::set<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::set<E>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::set<E>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::set<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::set<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::set<E>, function::Timestamp> || std::is_invocable_v<Flatten, std::set<E>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::set<E>> { return Semantic<std::set<E>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::set<E>> { return Semantic<std::set<E>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::set<E> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                     this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), offset](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::set<E> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                     this->concurrent);
    }

    auto translate(const function::BiFunction<std::set<E>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), translator](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::set<E> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                     this->concurrent);
    }

    auto redirect(const function::BiFunction<std::set<E>, function::Timestamp, std::set<E>> &redirector) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), redirector](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::set<E> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::set<E> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                     this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), start, end](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::set<E> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                     this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::set<E>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::set<E>>>)
        {
            return Semantic<std::set<E>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::set<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::set<E> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::set<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::set<E> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                         this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::set<E>>)
        {
            return Semantic<std::set<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::set<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                         this->concurrent);
        }
        else
        {
            return Semantic<std::set<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::set<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                         this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::set<E>> { return collectable::UnorderedCollectable<std::set<E>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::set<E>> { return collectable::OrderedCollectable<std::set<E>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::set<E>> { return collectable::WindowCollectable<std::set<E>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::set<E>, D> { return collectable::Statistics<std::set<E>, D>(this->source(), this->concurrent); }
};

template <typename E>
class Semantic<std::unordered_set<E>>
{
  private:
    std::unique_ptr<function::Generator<std::unordered_set<E>>> generator;
    function::Module concurrent;

  public:
    using Element = std::unordered_set<E>;

    Semantic(std::unordered_set<E> &&container) : generator(std::make_unique<function::Generator<std::unordered_set<E>>>([elements = std::move(container)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
                                                      accept(elements, 0LL);
                                                  })),
                                                  concurrent(1) {}

    Semantic(std::unordered_set<E> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::unordered_set<E>>>([elements = std::move(container)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
                                                                                          accept(elements, 0LL);
                                                                                      })),
                                                                                      concurrent(concurrent) {}

    Semantic(const function::Generator<std::unordered_set<E>> &generator) : generator(std::make_unique<function::Generator<std::unordered_set<E>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::unordered_set<E>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::unordered_set<E>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::unordered_set<E>> &other) : generator(std::make_unique<function::Generator<std::unordered_set<E>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::unordered_set<E>> &&other) noexcept = default;

    Semantic<std::unordered_set<E>> &operator=(const Semantic<std::unordered_set<E>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::unordered_set<E>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::unordered_set<E>> &operator=(Semantic<std::unordered_set<E>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::unordered_set<E>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::unordered_set<E>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::unordered_set<E>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::unordered_set<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::unordered_set<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::unordered_set<E>>)
        {
            using R = std::invoke_result_t<Mapper, std::unordered_set<E>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::unordered_set<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::unordered_set<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::unordered_set<E>, function::Timestamp> || std::is_invocable_v<Mapper, std::unordered_set<E>>, "Mapper must be callable as either (unordered_set<E>, Timestamp) -> R or (unordered_set<E>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::unordered_set<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>>, "Predicate must be callable as either (unordered_set<E>, Timestamp) -> bool or (unordered_set<E>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                               this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::unordered_set<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                               this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::unordered_set<E> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                               this->concurrent);
    }

    auto distinct() const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::unordered_set<E>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::unordered_set<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                               this->concurrent);
    }

    auto distinct(const function::Comparator<std::unordered_set<E>> &comparator) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), comparator](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            std::set<std::unordered_set<E>, function::Comparator<std::unordered_set<E>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::unordered_set<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                               this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::unordered_set<E>>
    {
        if constexpr (std::is_invocable_v<std::less<std::unordered_set<E>>, std::unordered_set<E>, std::unordered_set<E>>)
        {
            return collectable::OrderedCollectable<std::unordered_set<E>>(
                this->source(),
                [](const std::unordered_set<E> &a, const std::unordered_set<E> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::unordered_set<E>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::unordered_set<E>> &comparator) const -> collectable::OrderedCollectable<std::unordered_set<E>>
    {
        return collectable::OrderedCollectable<std::unordered_set<E>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), n](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::unordered_set<E> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                               this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), n](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::unordered_set<E> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                               this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::unordered_set<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::unordered_set<E>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::unordered_set<E>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                               this->concurrent);
    }

    auto flat() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::unordered_set<E> container, function::Timestamp index) -> void {
                for (const auto &element : container)
                {
                    if (stop)
                    {
                        break;
                    }
                    if (interrupt(element, count))
                    {
                        stop = true;
                        break;
                    }
                    accept(element, count);
                    count++;
                } }, [&stop](std::unordered_set<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_set<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::unordered_set<E>, function::Timestamp>)
                {
                    Semantic<E> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::unordered_set<E>>)
                {
                    Semantic<E> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, std::unordered_set<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, std::unordered_set<E>>, "flat: Flatten must return Semantic<E>");
                } }, [&stop](std::unordered_set<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::unordered_set<E>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::unordered_set<E>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_set<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::unordered_set<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::unordered_set<E>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::unordered_set<E>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_set<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::unordered_set<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::unordered_set<E>, function::Timestamp> || std::is_invocable_v<Flatten, std::unordered_set<E>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::unordered_set<E>> { return Semantic<std::unordered_set<E>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::unordered_set<E>> { return Semantic<std::unordered_set<E>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::unordered_set<E> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                               this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), offset](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::unordered_set<E> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                               this->concurrent);
    }

    auto translate(const function::BiFunction<std::unordered_set<E>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), translator](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::unordered_set<E> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                               this->concurrent);
    }

    auto redirect(const function::BiFunction<std::unordered_set<E>, function::Timestamp, std::unordered_set<E>> &redirector) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), redirector](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::unordered_set<E> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                               this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), start, end](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::unordered_set<E> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                               this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::unordered_set<E>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::unordered_set<E>>>)
        {
            return Semantic<std::unordered_set<E>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::unordered_set<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::unordered_set<E> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::unordered_set<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::unordered_set<E> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                                   this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::unordered_set<E>>)
        {
            return Semantic<std::unordered_set<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::unordered_set<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                                   this->concurrent);
        }
        else
        {
            return Semantic<std::unordered_set<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::unordered_set<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                                   this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::unordered_set<E>> { return collectable::UnorderedCollectable<std::unordered_set<E>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::unordered_set<E>> { return collectable::OrderedCollectable<std::unordered_set<E>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::unordered_set<E>> { return collectable::WindowCollectable<std::unordered_set<E>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::unordered_set<E>, D> { return collectable::Statistics<std::unordered_set<E>, D>(this->source(), this->concurrent); }
};

template <typename T1, typename T2>
class Semantic<std::pair<T1, T2>>
{
  private:
    std::unique_ptr<function::Generator<std::pair<T1, T2>>> generator;
    function::Module concurrent;

  public:
    using Element = std::pair<T1, T2>;

    Semantic(std::pair<T1, T2> &&container) : generator(std::make_unique<function::Generator<std::pair<T1, T2>>>([elements = std::move(container)](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
                                                  accept(elements, 0LL);
                                              })),
                                              concurrent(1) {}

    Semantic(std::pair<T1, T2> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::pair<T1, T2>>>([elements = std::move(container)](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
                                                                                      accept(elements, 0LL);
                                                                                  })),
                                                                                  concurrent(concurrent) {}

    Semantic(const function::Generator<std::pair<T1, T2>> &generator) : generator(std::make_unique<function::Generator<std::pair<T1, T2>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::pair<T1, T2>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::pair<T1, T2>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::pair<T1, T2>> &other) : generator(std::make_unique<function::Generator<std::pair<T1, T2>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::pair<T1, T2>> &&other) noexcept = default;

    Semantic<std::pair<T1, T2>> &operator=(const Semantic<std::pair<T1, T2>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::pair<T1, T2>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::pair<T1, T2>> &operator=(Semantic<std::pair<T1, T2>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::pair<T1, T2>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::pair<T1, T2>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::pair<T1, T2>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::pair<T1, T2> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::pair<T1, T2> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::pair<T1, T2>>)
        {
            using R = std::invoke_result_t<Mapper, std::pair<T1, T2>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::pair<T1, T2> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::pair<T1, T2> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::pair<T1, T2>, function::Timestamp> || std::is_invocable_v<Mapper, std::pair<T1, T2>>, "Mapper must be callable as either (pair<T1,T2>, Timestamp) -> R or (pair<T1,T2>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::pair<T1, T2> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::pair<T1, T2>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::pair<T1, T2>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::pair<T1, T2>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::pair<T1, T2>>, "Predicate must be callable as either (pair<T1,T2>, Timestamp) -> bool or (pair<T1,T2>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                           this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::pair<T1, T2> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::pair<T1, T2>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::pair<T1, T2>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                           this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::pair<T1, T2> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::pair<T1, T2>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::pair<T1, T2>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                           this->concurrent);
    }

    auto distinct() const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator)](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::pair<T1, T2>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::pair<T1, T2> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                           this->concurrent);
    }

    auto distinct(const function::Comparator<std::pair<T1, T2>> &comparator) const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator), comparator](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            std::set<std::pair<T1, T2>, function::Comparator<std::pair<T1, T2>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::pair<T1, T2> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                           this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::pair<T1, T2>>
    {
        if constexpr (std::is_invocable_v<std::less<std::pair<T1, T2>>, std::pair<T1, T2>, std::pair<T1, T2>>)
        {
            return collectable::OrderedCollectable<std::pair<T1, T2>>(
                this->source(),
                [](const std::pair<T1, T2> &a, const std::pair<T1, T2> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::pair<T1, T2>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::pair<T1, T2>> &comparator) const -> collectable::OrderedCollectable<std::pair<T1, T2>>
    {
        return collectable::OrderedCollectable<std::pair<T1, T2>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator), n](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::pair<T1, T2> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                           this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator), n](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::pair<T1, T2> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                           this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::pair<T1, T2> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::pair<T1, T2>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::pair<T1, T2>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                           this->concurrent);
    }

    auto parallel() const -> Semantic<std::pair<T1, T2>> { return Semantic<std::pair<T1, T2>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::pair<T1, T2>> { return Semantic<std::pair<T1, T2>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator)](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::pair<T1, T2> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                           this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator), offset](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::pair<T1, T2> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                           this->concurrent);
    }

    auto translate(const function::BiFunction<std::pair<T1, T2>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator), translator](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::pair<T1, T2> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                           this->concurrent);
    }

    auto redirect(const function::BiFunction<std::pair<T1, T2>, function::Timestamp, std::pair<T1, T2>> &redirector) const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator), redirector](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::pair<T1, T2> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                           this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::pair<T1, T2>>
    {
        return Semantic<std::pair<T1, T2>>([generator = *(this->generator), start, end](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::pair<T1, T2> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                           this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::pair<T1, T2>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::pair<T1, T2>>>)
        {
            return Semantic<std::pair<T1, T2>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::pair<T1, T2> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::pair<T1, T2> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::pair<T1, T2> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::pair<T1, T2> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                               this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::pair<T1, T2>>)
        {
            return Semantic<std::pair<T1, T2>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::pair<T1, T2> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                               this->concurrent);
        }
        else
        {
            return Semantic<std::pair<T1, T2>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::pair<T1, T2>, function::Timestamp> accept, function::BiPredicate<std::pair<T1, T2>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::pair<T1, T2> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::pair<T1, T2> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                               this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::pair<T1, T2>> { return collectable::UnorderedCollectable<std::pair<T1, T2>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::pair<T1, T2>> { return collectable::OrderedCollectable<std::pair<T1, T2>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::pair<T1, T2>> { return collectable::WindowCollectable<std::pair<T1, T2>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::pair<T1, T2>, D> { return collectable::Statistics<std::pair<T1, T2>, D>(this->source(), this->concurrent); }
};

template <typename K, typename V>
class Semantic<std::map<K, V>>
{
  private:
    std::unique_ptr<function::Generator<std::map<K, V>>> generator;
    function::Module concurrent;

  public:
    using Element = std::map<K, V>;

    Semantic(std::map<K, V> &&container) : generator(std::make_unique<function::Generator<std::map<K, V>>>([elements = std::move(container)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
                                               accept(elements, 0LL);
                                           })),
                                           concurrent(1) {}

    Semantic(std::map<K, V> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::map<K, V>>>([elements = std::move(container)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
                                                                                   accept(elements, 0LL);
                                                                               })),
                                                                               concurrent(concurrent) {}

    Semantic(const function::Generator<std::map<K, V>> &generator) : generator(std::make_unique<function::Generator<std::map<K, V>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::map<K, V>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::map<K, V>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::map<K, V>> &other) : generator(std::make_unique<function::Generator<std::map<K, V>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::map<K, V>> &&other) noexcept = default;

    Semantic<std::map<K, V>> &operator=(const Semantic<std::map<K, V>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::map<K, V>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::map<K, V>> &operator=(Semantic<std::map<K, V>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::map<K, V>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::map<K, V>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::map<K, V>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::map<K, V> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::map<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::map<K, V>>)
        {
            using R = std::invoke_result_t<Mapper, std::map<K, V>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::map<K, V> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::map<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::map<K, V>, function::Timestamp> || std::is_invocable_v<Mapper, std::map<K, V>>, "Mapper must be callable as either (map<K,V>, Timestamp) -> R or (map<K,V>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::map<K, V> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::map<K, V>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::map<K, V>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::map<K, V>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::map<K, V>>, "Predicate must be callable as either (map<K,V>, Timestamp) -> bool or (map<K,V>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::map<K, V> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::map<K, V>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::map<K, V>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                        this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::map<K, V> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::map<K, V>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::map<K, V>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    auto distinct() const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::map<K, V>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::map<K, V> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    auto distinct(const function::Comparator<std::map<K, V>> &comparator) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), comparator](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            std::set<std::map<K, V>, function::Comparator<std::map<K, V>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::map<K, V> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::map<K, V>>
    {
        if constexpr (std::is_invocable_v<std::less<std::map<K, V>>, std::map<K, V>, std::map<K, V>>)
        {
            return collectable::OrderedCollectable<std::map<K, V>>(
                this->source(),
                [](const std::map<K, V> &a, const std::map<K, V> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::map<K, V>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::map<K, V>> &comparator) const -> collectable::OrderedCollectable<std::map<K, V>>
    {
        return collectable::OrderedCollectable<std::map<K, V>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), n](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::map<K, V> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                        this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), n](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::map<K, V> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::map<K, V> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::map<K, V>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::map<K, V>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                        this->concurrent);
    }

    auto flat() const -> Semantic<std::pair<const K, V>>
    {
        return Semantic<std::pair<const K, V>>([generator = *(this->generator)](function::BiConsumer<std::pair<const K, V>, function::Timestamp> accept, function::BiPredicate<std::pair<const K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::map<K, V> container, function::Timestamp index) -> void {
            for (const auto &element : container)
            {
                if (stop)
                {
                    break;
                }
                if (interrupt(element, count))
                {
                    stop = true;
                    break;
                }
                accept(element, count);
                count++;
            } }, [&stop](std::map<K, V> container, function::Timestamp index) -> bool { return stop; });
        },
                                               this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::pair<const K, V>>
    {
        return Semantic<std::pair<const K, V>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::pair<const K, V>, function::Timestamp> accept, function::BiPredicate<std::pair<const K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::map<K, V> container, function::Timestamp index) -> void {
            if constexpr (std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::map<K, V>, function::Timestamp>)
            {
                Semantic<std::pair<const K, V>> inner = std::invoke(flatten, container, index);
                inner.source()([&accept, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++;
                }, [&interrupt, &stop, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerElement, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                });
            }
            else if constexpr (std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::map<K, V>>)
            {
                Semantic<std::pair<const K, V>> inner = std::invoke(flatten, container);
                inner.source()([&accept, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++;
                }, [&interrupt, &stop, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerElement, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                });
            }
            else
            {
                static_assert(std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::map<K, V>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::map<K, V>>, "flat: Flatten must return Semantic<pair<const K,V>>");
            } }, [&stop](std::map<K, V> container, function::Timestamp index) -> bool { return stop; });
        },
                                               this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::map<K, V>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::map<K, V>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::map<K, V> container, function::Timestamp index) -> void {
                Semantic<R> inner = std::invoke(flatten, container, index);
                inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++;
                }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerElement, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                }); }, [&stop](std::map<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::map<K, V>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::map<K, V>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::map<K, V> container, function::Timestamp index) -> void {
                Semantic<R> inner = std::invoke(flatten, container);
                inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++;
                }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerElement, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                }); }, [&stop](std::map<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::map<K, V>, function::Timestamp> || std::is_invocable_v<Flatten, std::map<K, V>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::map<K, V>> { return Semantic<std::map<K, V>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::map<K, V>> { return Semantic<std::map<K, V>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::map<K, V> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                        this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), offset](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::map<K, V> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                        this->concurrent);
    }

    auto translate(const function::BiFunction<std::map<K, V>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), translator](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::map<K, V> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                        this->concurrent);
    }

    auto redirect(const function::BiFunction<std::map<K, V>, function::Timestamp, std::map<K, V>> &redirector) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), redirector](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::map<K, V> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                        this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), start, end](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::map<K, V> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                        this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::map<K, V>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::map<K, V>>>)
        {
            return Semantic<std::map<K, V>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::map<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::map<K, V> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::map<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::map<K, V> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                            this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::map<K, V>>)
        {
            return Semantic<std::map<K, V>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::map<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                            this->concurrent);
        }
        else
        {
            return Semantic<std::map<K, V>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::map<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                            this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::map<K, V>> { return collectable::UnorderedCollectable<std::map<K, V>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::map<K, V>> { return collectable::OrderedCollectable<std::map<K, V>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::map<K, V>> { return collectable::WindowCollectable<std::map<K, V>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::map<K, V>, D> { return collectable::Statistics<std::map<K, V>, D>(this->source(), this->concurrent); }
};

template <typename K, typename V>
class Semantic<std::unordered_map<K, V>>
{
  private:
    std::unique_ptr<function::Generator<std::unordered_map<K, V>>> generator;
    function::Module concurrent;

  public:
    using Element = std::unordered_map<K, V>;

    Semantic(std::unordered_map<K, V> &&container) : generator(std::make_unique<function::Generator<std::unordered_map<K, V>>>([elements = std::move(container)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
                                                         accept(elements, 0LL);
                                                     })),
                                                     concurrent(1) {}

    Semantic(std::unordered_map<K, V> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::unordered_map<K, V>>>([elements = std::move(container)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
                                                                                             accept(elements, 0LL);
                                                                                         })),
                                                                                         concurrent(concurrent) {}

    Semantic(const function::Generator<std::unordered_map<K, V>> &generator) : generator(std::make_unique<function::Generator<std::unordered_map<K, V>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::unordered_map<K, V>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::unordered_map<K, V>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::unordered_map<K, V>> &other) : generator(std::make_unique<function::Generator<std::unordered_map<K, V>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::unordered_map<K, V>> &&other) noexcept = default;

    Semantic<std::unordered_map<K, V>> &operator=(const Semantic<std::unordered_map<K, V>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::unordered_map<K, V>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::unordered_map<K, V>> &operator=(Semantic<std::unordered_map<K, V>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::unordered_map<K, V>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::unordered_map<K, V>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::unordered_map<K, V>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::unordered_map<K, V>>)
        {
            using R = std::invoke_result_t<Mapper, std::unordered_map<K, V>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::unordered_map<K, V>, function::Timestamp> || std::is_invocable_v<Mapper, std::unordered_map<K, V>>, "Mapper must be callable as either (unordered_map<K,V>, Timestamp) -> R or (unordered_map<K,V>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>>, "Predicate must be callable as either (unordered_map<K,V>, Timestamp) -> bool or (unordered_map<K,V>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                  this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                                  this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                  this->concurrent);
    }

    auto distinct() const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::unordered_map<K, V>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                  this->concurrent);
    }

    auto distinct(const function::Comparator<std::unordered_map<K, V>> &comparator) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), comparator](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            std::set<std::unordered_map<K, V>, function::Comparator<std::unordered_map<K, V>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                  this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::unordered_map<K, V>>
    {
        if constexpr (std::is_invocable_v<std::less<std::unordered_map<K, V>>, std::unordered_map<K, V>, std::unordered_map<K, V>>)
        {
            return collectable::OrderedCollectable<std::unordered_map<K, V>>(
                this->source(),
                [](const std::unordered_map<K, V> &a, const std::unordered_map<K, V> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::unordered_map<K, V>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::unordered_map<K, V>> &comparator) const -> collectable::OrderedCollectable<std::unordered_map<K, V>>
    {
        return collectable::OrderedCollectable<std::unordered_map<K, V>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), n](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                                  this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), n](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                  this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::unordered_map<K, V>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::unordered_map<K, V>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                                  this->concurrent);
    }

    auto flat() const -> Semantic<std::pair<const K, V>>
    {
        return Semantic<std::pair<const K, V>>([generator = *(this->generator)](function::BiConsumer<std::pair<const K, V>, function::Timestamp> accept, function::BiPredicate<std::pair<const K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::unordered_map<K, V> container, function::Timestamp index) -> void {
            for (const auto &element : container)
            {
                if (stop)
                {
                    break;
                }
                if (interrupt(element, count))
                {
                    stop = true;
                    break;
                }
                accept(element, count);
                count++;
            } }, [&stop](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return stop; });
        },
                                               this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::pair<const K, V>>
    {
        return Semantic<std::pair<const K, V>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::pair<const K, V>, function::Timestamp> accept, function::BiPredicate<std::pair<const K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_map<K, V> container, function::Timestamp index) -> void {
            if constexpr (std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::unordered_map<K, V>, function::Timestamp>)
            {
                Semantic<std::pair<const K, V>> inner = std::invoke(flatten, container, index);
                inner.source()([&accept, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++;
                }, [&interrupt, &stop, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerElement, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                });
            }
            else if constexpr (std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::unordered_map<K, V>>)
            {
                Semantic<std::pair<const K, V>> inner = std::invoke(flatten, container);
                inner.source()([&accept, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++;
                }, [&interrupt, &stop, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerElement, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                });
            }
            else
            {
                static_assert(std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::unordered_map<K, V>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::unordered_map<K, V>>, "flat: Flatten must return Semantic<pair<const K,V>>");
            } }, [&stop](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return stop; });
        },
                                               this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::unordered_map<K, V>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::unordered_map<K, V>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                Semantic<R> inner = std::invoke(flatten, container, index);
                inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++;
                }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerElement, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                }); }, [&stop](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::unordered_map<K, V>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::unordered_map<K, V>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                Semantic<R> inner = std::invoke(flatten, container);
                inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++;
                }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerElement, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                }); }, [&stop](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::unordered_map<K, V>, function::Timestamp> || std::is_invocable_v<Flatten, std::unordered_map<K, V>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::unordered_map<K, V>> { return Semantic<std::unordered_map<K, V>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::unordered_map<K, V>> { return Semantic<std::unordered_map<K, V>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::unordered_map<K, V> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                                  this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), offset](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::unordered_map<K, V> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                                  this->concurrent);
    }

    auto translate(const function::BiFunction<std::unordered_map<K, V>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), translator](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::unordered_map<K, V> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                                  this->concurrent);
    }

    auto redirect(const function::BiFunction<std::unordered_map<K, V>, function::Timestamp, std::unordered_map<K, V>> &redirector) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), redirector](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::unordered_map<K, V> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                                  this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), start, end](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                                  this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::unordered_map<K, V>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::unordered_map<K, V>>>)
        {
            return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::unordered_map<K, V> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                                      this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::unordered_map<K, V>>)
        {
            return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                                      this->concurrent);
        }
        else
        {
            return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                                      this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::unordered_map<K, V>> { return collectable::UnorderedCollectable<std::unordered_map<K, V>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::unordered_map<K, V>> { return collectable::OrderedCollectable<std::unordered_map<K, V>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::unordered_map<K, V>> { return collectable::WindowCollectable<std::unordered_map<K, V>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::unordered_map<K, V>, D> { return collectable::Statistics<std::unordered_map<K, V>, D>(this->source(), this->concurrent); }
};

template <typename E>
class Semantic<std::multiset<E>>
{
  private:
    std::unique_ptr<function::Generator<std::multiset<E>>> generator;
    function::Module concurrent;

  public:
    using Element = std::multiset<E>;

    Semantic(std::multiset<E> &&container) : generator(std::make_unique<function::Generator<std::multiset<E>>>([elements = std::move(container)](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
                                                 accept(elements, 0LL);
                                             })),
                                             concurrent(1) {}

    Semantic(std::multiset<E> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::multiset<E>>>([elements = std::move(container)](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
                                                                                     accept(elements, 0LL);
                                                                                 })),
                                                                                 concurrent(concurrent) {}

    Semantic(const function::Generator<std::multiset<E>> &generator) : generator(std::make_unique<function::Generator<std::multiset<E>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::multiset<E>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::multiset<E>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::multiset<E>> &other) : generator(std::make_unique<function::Generator<std::multiset<E>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::multiset<E>> &&other) noexcept = default;

    Semantic<std::multiset<E>> &operator=(const Semantic<std::multiset<E>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::multiset<E>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::multiset<E>> &operator=(Semantic<std::multiset<E>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::multiset<E>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::multiset<E>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::multiset<E>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::multiset<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::multiset<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::multiset<E>>)
        {
            using R = std::invoke_result_t<Mapper, std::multiset<E>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::multiset<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::multiset<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::multiset<E>, function::Timestamp> || std::is_invocable_v<Mapper, std::multiset<E>>, "Mapper must be callable as either (multiset<E>, Timestamp) -> R or (multiset<E>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::multiset<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::multiset<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::multiset<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::multiset<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::multiset<E>>, "Predicate must be callable as either (multiset<E>, Timestamp) -> bool or (multiset<E>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                          this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::multiset<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::multiset<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::multiset<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                          this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::multiset<E> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::multiset<E>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::multiset<E>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                          this->concurrent);
    }

    auto distinct() const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator)](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::multiset<E>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::multiset<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                          this->concurrent);
    }

    auto distinct(const function::Comparator<std::multiset<E>> &comparator) const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator), comparator](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            std::set<std::multiset<E>, function::Comparator<std::multiset<E>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::multiset<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                          this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::multiset<E>>
    {
        if constexpr (std::is_invocable_v<std::less<std::multiset<E>>, std::multiset<E>, std::multiset<E>>)
        {
            return collectable::OrderedCollectable<std::multiset<E>>(
                this->source(),
                [](const std::multiset<E> &a, const std::multiset<E> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::multiset<E>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::multiset<E>> &comparator) const -> collectable::OrderedCollectable<std::multiset<E>>
    {
        return collectable::OrderedCollectable<std::multiset<E>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator), n](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::multiset<E> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                          this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator), n](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::multiset<E> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                          this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::multiset<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::multiset<E>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::multiset<E>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                          this->concurrent);
    }

    auto flat() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::multiset<E> container, function::Timestamp index) -> void {
                for (const auto &element : container)
                {
                    if (stop)
                    {
                        break;
                    }
                    if (interrupt(element, count))
                    {
                        stop = true;
                        break;
                    }
                    accept(element, count);
                    count++;
                } }, [&stop](std::multiset<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::multiset<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::multiset<E>, function::Timestamp>)
                {
                    Semantic<E> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::multiset<E>>)
                {
                    Semantic<E> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, std::multiset<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, std::multiset<E>>, "flat: Flatten must return Semantic<E>");
                } }, [&stop](std::multiset<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::multiset<E>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::multiset<E>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::multiset<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::multiset<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::multiset<E>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::multiset<E>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::multiset<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::multiset<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::multiset<E>, function::Timestamp> || std::is_invocable_v<Flatten, std::multiset<E>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::multiset<E>> { return Semantic<std::multiset<E>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::multiset<E>> { return Semantic<std::multiset<E>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator)](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::multiset<E> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                          this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator), offset](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::multiset<E> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                          this->concurrent);
    }

    auto translate(const function::BiFunction<std::multiset<E>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator), translator](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::multiset<E> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                          this->concurrent);
    }

    auto redirect(const function::BiFunction<std::multiset<E>, function::Timestamp, std::multiset<E>> &redirector) const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator), redirector](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::multiset<E> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                          this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::multiset<E>>
    {
        return Semantic<std::multiset<E>>([generator = *(this->generator), start, end](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::multiset<E> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                          this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::multiset<E>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::multiset<E>>>)
        {
            return Semantic<std::multiset<E>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::multiset<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::multiset<E> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::multiset<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::multiset<E> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                              this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::multiset<E>>)
        {
            return Semantic<std::multiset<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::multiset<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                              this->concurrent);
        }
        else
        {
            return Semantic<std::multiset<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::multiset<E>, function::Timestamp> accept, function::BiPredicate<std::multiset<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::multiset<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::multiset<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                              this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::multiset<E>> { return collectable::UnorderedCollectable<std::multiset<E>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::multiset<E>> { return collectable::OrderedCollectable<std::multiset<E>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::multiset<E>> { return collectable::WindowCollectable<std::multiset<E>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::multiset<E>, D> { return collectable::Statistics<std::multiset<E>, D>(this->source(), this->concurrent); }
};

template <typename K, typename V>
class Semantic<std::multimap<K, V>>
{
  private:
    std::unique_ptr<function::Generator<std::multimap<K, V>>> generator;
    function::Module concurrent;

  public:
    using Element = std::multimap<K, V>;

    Semantic(std::multimap<K, V> &&container) : generator(std::make_unique<function::Generator<std::multimap<K, V>>>([elements = std::move(container)](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
                                                    accept(elements, 0LL);
                                                })),
                                                concurrent(1) {}

    Semantic(std::multimap<K, V> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::multimap<K, V>>>([elements = std::move(container)](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
                                                                                        accept(elements, 0LL);
                                                                                    })),
                                                                                    concurrent(concurrent) {}

    Semantic(const function::Generator<std::multimap<K, V>> &generator) : generator(std::make_unique<function::Generator<std::multimap<K, V>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::multimap<K, V>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::multimap<K, V>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::multimap<K, V>> &other) : generator(std::make_unique<function::Generator<std::multimap<K, V>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::multimap<K, V>> &&other) noexcept = default;

    Semantic<std::multimap<K, V>> &operator=(const Semantic<std::multimap<K, V>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::multimap<K, V>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::multimap<K, V>> &operator=(Semantic<std::multimap<K, V>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::multimap<K, V>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::multimap<K, V>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::multimap<K, V>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::multimap<K, V> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::multimap<K, V>>)
        {
            using R = std::invoke_result_t<Mapper, std::multimap<K, V>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::multimap<K, V> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::multimap<K, V>, function::Timestamp> || std::is_invocable_v<Mapper, std::multimap<K, V>>, "Mapper must be callable as either (multimap<K,V>, Timestamp) -> R or (multimap<K,V>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::multimap<K, V> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::multimap<K, V>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::multimap<K, V>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::multimap<K, V>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::multimap<K, V>>, "Predicate must be callable as either (multimap<K,V>, Timestamp) -> bool or (multimap<K,V>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                             this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::multimap<K, V> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::multimap<K, V>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::multimap<K, V>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                             this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::multimap<K, V> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::multimap<K, V>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::multimap<K, V>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                             this->concurrent);
    }

    auto distinct() const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator)](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::multimap<K, V>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::multimap<K, V> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                             this->concurrent);
    }

    auto distinct(const function::Comparator<std::multimap<K, V>> &comparator) const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator), comparator](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            std::set<std::multimap<K, V>, function::Comparator<std::multimap<K, V>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::multimap<K, V> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                             this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::multimap<K, V>>
    {
        if constexpr (std::is_invocable_v<std::less<std::multimap<K, V>>, std::multimap<K, V>, std::multimap<K, V>>)
        {
            return collectable::OrderedCollectable<std::multimap<K, V>>(
                this->source(),
                [](const std::multimap<K, V> &a, const std::multimap<K, V> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::multimap<K, V>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::multimap<K, V>> &comparator) const -> collectable::OrderedCollectable<std::multimap<K, V>>
    {
        return collectable::OrderedCollectable<std::multimap<K, V>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator), n](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::multimap<K, V> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                             this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator), n](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::multimap<K, V> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                             this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::multimap<K, V> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::multimap<K, V>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::multimap<K, V>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                             this->concurrent);
    }

    auto flat() const -> Semantic<std::pair<const K, V>>
    {
        return Semantic<std::pair<const K, V>>([generator = *(this->generator)](function::BiConsumer<std::pair<const K, V>, function::Timestamp> accept, function::BiPredicate<std::pair<const K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::multimap<K, V> container, function::Timestamp index) -> void {
            for (const auto &element : container)
            {
                if (stop)
                {
                    break;
                }
                if (interrupt(element, count))
                {
                    stop = true;
                    break;
                }
                accept(element, count);
                count++;
            } }, [&stop](std::multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
        },
                                               this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::pair<const K, V>>
    {
        return Semantic<std::pair<const K, V>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::pair<const K, V>, function::Timestamp> accept, function::BiPredicate<std::pair<const K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::multimap<K, V> container, function::Timestamp index) -> void {
            if constexpr (std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::multimap<K, V>, function::Timestamp>)
            {
                Semantic<std::pair<const K, V>> inner = std::invoke(flatten, container, index);
                inner.source()([&accept, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++;
                }, [&interrupt, &stop, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerElement, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                });
            }
            else if constexpr (std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::multimap<K, V>>)
            {
                Semantic<std::pair<const K, V>> inner = std::invoke(flatten, container);
                inner.source()([&accept, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++;
                }, [&interrupt, &stop, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerElement, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                });
            }
            else
            {
                static_assert(std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::multimap<K, V>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::multimap<K, V>>, "flat: Flatten must return Semantic<pair<const K,V>>");
            } }, [&stop](std::multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
        },
                                               this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::multimap<K, V>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::multimap<K, V>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::multimap<K, V> container, function::Timestamp index) -> void {
                Semantic<R> inner = std::invoke(flatten, container, index);
                inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++;
                }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerElement, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                }); }, [&stop](std::multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::multimap<K, V>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::multimap<K, V>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::multimap<K, V> container, function::Timestamp index) -> void {
                Semantic<R> inner = std::invoke(flatten, container);
                inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++;
                }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerElement, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                }); }, [&stop](std::multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::multimap<K, V>, function::Timestamp> || std::is_invocable_v<Flatten, std::multimap<K, V>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::multimap<K, V>> { return Semantic<std::multimap<K, V>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::multimap<K, V>> { return Semantic<std::multimap<K, V>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator)](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::multimap<K, V> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                             this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator), offset](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::multimap<K, V> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                             this->concurrent);
    }

    auto translate(const function::BiFunction<std::multimap<K, V>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator), translator](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::multimap<K, V> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                             this->concurrent);
    }

    auto redirect(const function::BiFunction<std::multimap<K, V>, function::Timestamp, std::multimap<K, V>> &redirector) const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator), redirector](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::multimap<K, V> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                             this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::multimap<K, V>>
    {
        return Semantic<std::multimap<K, V>>([generator = *(this->generator), start, end](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::multimap<K, V> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                             this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::multimap<K, V>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::multimap<K, V>>>)
        {
            return Semantic<std::multimap<K, V>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::multimap<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::multimap<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::multimap<K, V> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                                 this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::multimap<K, V>>)
        {
            return Semantic<std::multimap<K, V>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::multimap<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                                 this->concurrent);
        }
        else
        {
            return Semantic<std::multimap<K, V>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::multimap<K, V>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::multimap<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                                 this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::multimap<K, V>> { return collectable::UnorderedCollectable<std::multimap<K, V>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::multimap<K, V>> { return collectable::OrderedCollectable<std::multimap<K, V>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::multimap<K, V>> { return collectable::WindowCollectable<std::multimap<K, V>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::multimap<K, V>, D> { return collectable::Statistics<std::multimap<K, V>, D>(this->source(), this->concurrent); }
};

template <typename E>
class Semantic<std::priority_queue<E>>
{
  private:
    std::unique_ptr<function::Generator<std::priority_queue<E>>> generator;
    function::Module concurrent;

  public:
    using Element = std::priority_queue<E>;

    Semantic(std::priority_queue<E> &&container) : generator(std::make_unique<function::Generator<std::priority_queue<E>>>([elements = std::move(container)](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
                                                       accept(elements, 0LL);
                                                   })),
                                                   concurrent(1) {}

    Semantic(std::priority_queue<E> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::priority_queue<E>>>([elements = std::move(container)](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
                                                                                           accept(elements, 0LL);
                                                                                       })),
                                                                                       concurrent(concurrent) {}

    Semantic(const function::Generator<std::priority_queue<E>> &generator) : generator(std::make_unique<function::Generator<std::priority_queue<E>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::priority_queue<E>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::priority_queue<E>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::priority_queue<E>> &other) : generator(std::make_unique<function::Generator<std::priority_queue<E>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::priority_queue<E>> &&other) noexcept = default;

    Semantic<std::priority_queue<E>> &operator=(const Semantic<std::priority_queue<E>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::priority_queue<E>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::priority_queue<E>> &operator=(Semantic<std::priority_queue<E>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::priority_queue<E>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::priority_queue<E>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::priority_queue<E>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::priority_queue<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::priority_queue<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::priority_queue<E>>)
        {
            using R = std::invoke_result_t<Mapper, std::priority_queue<E>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::priority_queue<E> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::priority_queue<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::priority_queue<E>, function::Timestamp> || std::is_invocable_v<Mapper, std::priority_queue<E>>, "Mapper must be callable as either (priority_queue<E>, Timestamp) -> R or (priority_queue<E>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::priority_queue<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::priority_queue<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::priority_queue<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::priority_queue<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::priority_queue<E>>, "Predicate must be callable as either (priority_queue<E>, Timestamp) -> bool or (priority_queue<E>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::priority_queue<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::priority_queue<E>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::priority_queue<E>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                                this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::priority_queue<E> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::priority_queue<E>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::priority_queue<E>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                this->concurrent);
    }

    auto distinct() const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator)](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            std::unordered_set<std::priority_queue<E>> seen;
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::priority_queue<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                this->concurrent);
    }

    auto distinct(const function::Comparator<std::priority_queue<E>> &comparator) const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator), comparator](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            std::set<std::priority_queue<E>, function::Comparator<std::priority_queue<E>>> seen(comparator);
            function::Timestamp count = 0LL;
            generator([&accept, &seen, &count](std::priority_queue<E> container, function::Timestamp index) -> void {
                if (seen.find(container) == seen.end())
                {
                    seen.insert(container);
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<std::priority_queue<E>>
    {
        if constexpr (std::is_invocable_v<std::less<std::priority_queue<E>>, std::priority_queue<E>, std::priority_queue<E>>)
        {
            return collectable::OrderedCollectable<std::priority_queue<E>>(
                this->source(),
                [](const std::priority_queue<E> &a, const std::priority_queue<E> &b) -> bool { return a < b; },
                this->concurrent);
        }
        else
        {
            return collectable::OrderedCollectable<std::priority_queue<E>>(this->source(), this->concurrent);
        }
    }

    auto sort(const function::Comparator<std::priority_queue<E>> &comparator) const -> collectable::OrderedCollectable<std::priority_queue<E>>
    {
        return collectable::OrderedCollectable<std::priority_queue<E>>(this->source(), comparator, this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator), n](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::priority_queue<E> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                                this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator), n](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::priority_queue<E> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::priority_queue<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::priority_queue<E>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::priority_queue<E>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                                this->concurrent);
    }

    auto flat() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::priority_queue<E> container, function::Timestamp index) -> void {
                std::priority_queue<E> temp = container;
                while (!temp.empty())
                {
                    if (stop)
                    {
                        break;
                    }
                    if (interrupt(temp.top(), count))
                    {
                        stop = true;
                        break;
                    }
                    accept(temp.top(), count);
                    count++;
                    temp.pop();
                } }, [&stop](std::priority_queue<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::priority_queue<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::priority_queue<E>, function::Timestamp>)
                {
                    Semantic<E> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, std::priority_queue<E>>)
                {
                    Semantic<E> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](E innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, std::priority_queue<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, std::priority_queue<E>>, "flat: Flatten must return Semantic<E>");
                } }, [&stop](std::priority_queue<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::priority_queue<E>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::priority_queue<E>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::priority_queue<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::priority_queue<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::priority_queue<E>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::priority_queue<E>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::priority_queue<E> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::priority_queue<E> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::priority_queue<E>, function::Timestamp> || std::is_invocable_v<Flatten, std::priority_queue<E>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::priority_queue<E>> { return Semantic<std::priority_queue<E>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::priority_queue<E>> { return Semantic<std::priority_queue<E>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator)](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::priority_queue<E> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                                this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator), offset](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::priority_queue<E> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                                this->concurrent);
    }

    auto translate(const function::BiFunction<std::priority_queue<E>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator), translator](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::priority_queue<E> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                                this->concurrent);
    }

    auto redirect(const function::BiFunction<std::priority_queue<E>, function::Timestamp, std::priority_queue<E>> &redirector) const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator), redirector](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::priority_queue<E> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                                this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::priority_queue<E>>
    {
        return Semantic<std::priority_queue<E>>([generator = *(this->generator), start, end](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::priority_queue<E> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                                this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::priority_queue<E>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::priority_queue<E>>>)
        {
            return Semantic<std::priority_queue<E>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::priority_queue<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::priority_queue<E> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::priority_queue<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::priority_queue<E> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                                    this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::priority_queue<E>>)
        {
            return Semantic<std::priority_queue<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::priority_queue<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                                    this->concurrent);
        }
        else
        {
            return Semantic<std::priority_queue<E>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::priority_queue<E>, function::Timestamp> accept, function::BiPredicate<std::priority_queue<E>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::priority_queue<E> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::priority_queue<E> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                                    this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::priority_queue<E>> { return collectable::UnorderedCollectable<std::priority_queue<E>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::priority_queue<E>> { return collectable::OrderedCollectable<std::priority_queue<E>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::priority_queue<E>> { return collectable::WindowCollectable<std::priority_queue<E>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::priority_queue<E>, D> { return collectable::Statistics<std::priority_queue<E>, D>(this->source(), this->concurrent); }
};

template <typename... Ts>
class Semantic<std::tuple<Ts...>>
{
  private:
    std::unique_ptr<function::Generator<std::tuple<Ts...>>> generator;
    function::Module concurrent;

  public:
    using Element = std::tuple<Ts...>;

    Semantic(std::tuple<Ts...> &&container) : generator(std::make_unique<function::Generator<std::tuple<Ts...>>>([elements = std::move(container)](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
                                                  accept(elements, 0LL);
                                              })),
                                              concurrent(1) {}

    Semantic(std::tuple<Ts...> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::tuple<Ts...>>>([elements = std::move(container)](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
                                                                                      accept(elements, 0LL);
                                                                                  })),
                                                                                  concurrent(concurrent) {}

    Semantic(const function::Generator<std::tuple<Ts...>> &generator) : generator(std::make_unique<function::Generator<std::tuple<Ts...>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::tuple<Ts...>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::tuple<Ts...>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::tuple<Ts...>> &other) : generator(std::make_unique<function::Generator<std::tuple<Ts...>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::tuple<Ts...>> &&other) noexcept = default;

    Semantic<std::tuple<Ts...>> &operator=(const Semantic<std::tuple<Ts...>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::tuple<Ts...>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::tuple<Ts...>> &operator=(Semantic<std::tuple<Ts...>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::tuple<Ts...>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::tuple<Ts...>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::tuple<Ts...>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::tuple<Ts...> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::tuple<Ts...> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::tuple<Ts...>>)
        {
            using R = std::invoke_result_t<Mapper, std::tuple<Ts...>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::tuple<Ts...> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::tuple<Ts...> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::tuple<Ts...>, function::Timestamp> || std::is_invocable_v<Mapper, std::tuple<Ts...>>, "Mapper must be callable as either (tuple<Ts...>, Timestamp) -> R or (tuple<Ts...>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::tuple<Ts...>>
    {
        return Semantic<std::tuple<Ts...>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::tuple<Ts...> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::tuple<Ts...>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::tuple<Ts...>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::tuple<Ts...>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::tuple<Ts...>>, "Predicate must be callable as either (tuple<Ts...>, Timestamp) -> bool or (tuple<Ts...>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::tuple<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                           this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::tuple<Ts...>>
    {
        return Semantic<std::tuple<Ts...>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::tuple<Ts...> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::tuple<Ts...>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::tuple<Ts...>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::tuple<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                           this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::tuple<Ts...>>
    {
        return Semantic<std::tuple<Ts...>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::tuple<Ts...> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::tuple<Ts...>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::tuple<Ts...>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::tuple<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                           this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::tuple<Ts...>>
    {
        return Semantic<std::tuple<Ts...>>([generator = *(this->generator), n](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::tuple<Ts...> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::tuple<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                           this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::tuple<Ts...>>
    {
        return Semantic<std::tuple<Ts...>>([generator = *(this->generator), n](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::tuple<Ts...> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::tuple<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                           this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::tuple<Ts...>>
    {
        return Semantic<std::tuple<Ts...>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::tuple<Ts...> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::tuple<Ts...>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::tuple<Ts...>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                           this->concurrent);
    }

    auto parallel() const -> Semantic<std::tuple<Ts...>> { return Semantic<std::tuple<Ts...>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::tuple<Ts...>> { return Semantic<std::tuple<Ts...>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::tuple<Ts...>>
    {
        return Semantic<std::tuple<Ts...>>([generator = *(this->generator)](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::tuple<Ts...> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::tuple<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                           this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::tuple<Ts...>>
    {
        return Semantic<std::tuple<Ts...>>([generator = *(this->generator), offset](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::tuple<Ts...> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::tuple<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                           this->concurrent);
    }

    auto translate(const function::BiFunction<std::tuple<Ts...>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::tuple<Ts...>>
    {
        return Semantic<std::tuple<Ts...>>([generator = *(this->generator), translator](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::tuple<Ts...> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::tuple<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                           this->concurrent);
    }

    auto redirect(const function::BiFunction<std::tuple<Ts...>, function::Timestamp, std::tuple<Ts...>> &redirector) const -> Semantic<std::tuple<Ts...>>
    {
        return Semantic<std::tuple<Ts...>>([generator = *(this->generator), redirector](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::tuple<Ts...> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::tuple<Ts...> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                           this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::tuple<Ts...>>
    {
        return Semantic<std::tuple<Ts...>>([generator = *(this->generator), start, end](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::tuple<Ts...> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::tuple<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                           this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::tuple<Ts...>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::tuple<Ts...>>>)
        {
            return Semantic<std::tuple<Ts...>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::tuple<Ts...> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::tuple<Ts...> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::tuple<Ts...> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::tuple<Ts...> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                               this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::tuple<Ts...>>)
        {
            return Semantic<std::tuple<Ts...>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::tuple<Ts...> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::tuple<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                               this->concurrent);
        }
        else
        {
            return Semantic<std::tuple<Ts...>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::tuple<Ts...>, function::Timestamp> accept, function::BiPredicate<std::tuple<Ts...>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::tuple<Ts...> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::tuple<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                               this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::tuple<Ts...>> { return collectable::UnorderedCollectable<std::tuple<Ts...>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::tuple<Ts...>> { return collectable::OrderedCollectable<std::tuple<Ts...>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::tuple<Ts...>> { return collectable::WindowCollectable<std::tuple<Ts...>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::tuple<Ts...>, D> { return collectable::Statistics<std::tuple<Ts...>, D>(this->source(), this->concurrent); }
};

template <typename... Ts>
class Semantic<std::variant<Ts...>>
{
  private:
    std::unique_ptr<function::Generator<std::variant<Ts...>>> generator;
    function::Module concurrent;

  public:
    using Element = std::variant<Ts...>;

    Semantic(std::variant<Ts...> &&container) : generator(std::make_unique<function::Generator<std::variant<Ts...>>>([elements = std::move(container)](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
                                                    accept(elements, 0LL);
                                                })),
                                                concurrent(1) {}

    Semantic(std::variant<Ts...> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::variant<Ts...>>>([elements = std::move(container)](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
                                                                                        accept(elements, 0LL);
                                                                                    })),
                                                                                    concurrent(concurrent) {}

    Semantic(const function::Generator<std::variant<Ts...>> &generator) : generator(std::make_unique<function::Generator<std::variant<Ts...>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::variant<Ts...>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::variant<Ts...>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::variant<Ts...>> &other) : generator(std::make_unique<function::Generator<std::variant<Ts...>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::variant<Ts...>> &&other) noexcept = default;

    Semantic<std::variant<Ts...>> &operator=(const Semantic<std::variant<Ts...>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::variant<Ts...>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::variant<Ts...>> &operator=(Semantic<std::variant<Ts...>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::variant<Ts...>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::variant<Ts...>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::variant<Ts...>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::variant<Ts...> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::variant<Ts...> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::variant<Ts...>>)
        {
            using R = std::invoke_result_t<Mapper, std::variant<Ts...>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::variant<Ts...> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::variant<Ts...> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::variant<Ts...>, function::Timestamp> || std::is_invocable_v<Mapper, std::variant<Ts...>>, "Mapper must be callable as either (variant<Ts...>, Timestamp) -> R or (variant<Ts...>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::variant<Ts...>>
    {
        return Semantic<std::variant<Ts...>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::variant<Ts...> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::variant<Ts...>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::variant<Ts...>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::variant<Ts...>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::variant<Ts...>>, "Predicate must be callable as either (variant<Ts...>, Timestamp) -> bool or (variant<Ts...>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::variant<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                             this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::variant<Ts...>>
    {
        return Semantic<std::variant<Ts...>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::variant<Ts...> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::variant<Ts...>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::variant<Ts...>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::variant<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                             this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::variant<Ts...>>
    {
        return Semantic<std::variant<Ts...>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::variant<Ts...> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::variant<Ts...>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::variant<Ts...>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::variant<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                             this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::variant<Ts...>>
    {
        return Semantic<std::variant<Ts...>>([generator = *(this->generator), n](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::variant<Ts...> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::variant<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                             this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::variant<Ts...>>
    {
        return Semantic<std::variant<Ts...>>([generator = *(this->generator), n](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::variant<Ts...> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::variant<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                             this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::variant<Ts...>>
    {
        return Semantic<std::variant<Ts...>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::variant<Ts...> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::variant<Ts...>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::variant<Ts...>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                             this->concurrent);
    }

    auto parallel() const -> Semantic<std::variant<Ts...>> { return Semantic<std::variant<Ts...>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::variant<Ts...>> { return Semantic<std::variant<Ts...>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::variant<Ts...>>
    {
        return Semantic<std::variant<Ts...>>([generator = *(this->generator)](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::variant<Ts...> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::variant<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                             this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::variant<Ts...>>
    {
        return Semantic<std::variant<Ts...>>([generator = *(this->generator), offset](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::variant<Ts...> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::variant<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                             this->concurrent);
    }

    auto translate(const function::BiFunction<std::variant<Ts...>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::variant<Ts...>>
    {
        return Semantic<std::variant<Ts...>>([generator = *(this->generator), translator](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::variant<Ts...> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::variant<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                             this->concurrent);
    }

    auto redirect(const function::BiFunction<std::variant<Ts...>, function::Timestamp, std::variant<Ts...>> &redirector) const -> Semantic<std::variant<Ts...>>
    {
        return Semantic<std::variant<Ts...>>([generator = *(this->generator), redirector](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::variant<Ts...> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::variant<Ts...> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                             this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::variant<Ts...>>
    {
        return Semantic<std::variant<Ts...>>([generator = *(this->generator), start, end](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::variant<Ts...> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::variant<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                             this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::variant<Ts...>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::variant<Ts...>>>)
        {
            return Semantic<std::variant<Ts...>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::variant<Ts...> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::variant<Ts...> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::variant<Ts...> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::variant<Ts...> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                                 this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::variant<Ts...>>)
        {
            return Semantic<std::variant<Ts...>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::variant<Ts...> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::variant<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                                 this->concurrent);
        }
        else
        {
            return Semantic<std::variant<Ts...>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::variant<Ts...>, function::Timestamp> accept, function::BiPredicate<std::variant<Ts...>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::variant<Ts...> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::variant<Ts...> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                                 this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::variant<Ts...>> { return collectable::UnorderedCollectable<std::variant<Ts...>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::variant<Ts...>> { return collectable::OrderedCollectable<std::variant<Ts...>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::variant<Ts...>> { return collectable::WindowCollectable<std::variant<Ts...>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::variant<Ts...>, D> { return collectable::Statistics<std::variant<Ts...>, D>(this->source(), this->concurrent); }
};

template <std::size_t N>
class Semantic<std::bitset<N>>
{
  private:
    std::unique_ptr<function::Generator<std::bitset<N>>> generator;
    function::Module concurrent;

  public:
    using Element = std::bitset<N>;

    Semantic(std::bitset<N> &&container) : generator(std::make_unique<function::Generator<std::bitset<N>>>([elements = std::move(container)](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
                                               accept(elements, 0LL);
                                           })),
                                           concurrent(1) {}

    Semantic(std::bitset<N> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::bitset<N>>>([elements = std::move(container)](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
                                                                                   accept(elements, 0LL);
                                                                               })),
                                                                               concurrent(concurrent) {}

    Semantic(const function::Generator<std::bitset<N>> &generator) : generator(std::make_unique<function::Generator<std::bitset<N>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::bitset<N>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::bitset<N>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::bitset<N>> &other) : generator(std::make_unique<function::Generator<std::bitset<N>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::bitset<N>> &&other) noexcept = default;

    Semantic<std::bitset<N>> &operator=(const Semantic<std::bitset<N>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::bitset<N>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::bitset<N>> &operator=(Semantic<std::bitset<N>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::bitset<N>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::bitset<N>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::bitset<N>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::bitset<N> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::bitset<N> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::bitset<N>>)
        {
            using R = std::invoke_result_t<Mapper, std::bitset<N>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::bitset<N> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::bitset<N> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::bitset<N>, function::Timestamp> || std::is_invocable_v<Mapper, std::bitset<N>>, "Mapper must be callable as either (bitset<N>, Timestamp) -> R or (bitset<N>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::bitset<N>>
    {
        return Semantic<std::bitset<N>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::bitset<N> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::bitset<N>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::bitset<N>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::bitset<N>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::bitset<N>>, "Predicate must be callable as either (bitset<N>, Timestamp) -> bool or (bitset<N>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::bitset<N> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::bitset<N>>
    {
        return Semantic<std::bitset<N>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::bitset<N> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::bitset<N>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::bitset<N>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::bitset<N> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                        this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::bitset<N>>
    {
        return Semantic<std::bitset<N>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::bitset<N> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::bitset<N>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::bitset<N>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::bitset<N> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::bitset<N>>
    {
        return Semantic<std::bitset<N>>([generator = *(this->generator), n](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::bitset<N> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::bitset<N> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                        this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::bitset<N>>
    {
        return Semantic<std::bitset<N>>([generator = *(this->generator), n](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::bitset<N> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::bitset<N> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                        this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::bitset<N>>
    {
        return Semantic<std::bitset<N>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::bitset<N> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::bitset<N>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::bitset<N>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                        this->concurrent);
    }

    auto parallel() const -> Semantic<std::bitset<N>> { return Semantic<std::bitset<N>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::bitset<N>> { return Semantic<std::bitset<N>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::bitset<N>>
    {
        return Semantic<std::bitset<N>>([generator = *(this->generator)](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::bitset<N> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::bitset<N> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                        this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::bitset<N>>
    {
        return Semantic<std::bitset<N>>([generator = *(this->generator), offset](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::bitset<N> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::bitset<N> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                        this->concurrent);
    }

    auto translate(const function::BiFunction<std::bitset<N>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::bitset<N>>
    {
        return Semantic<std::bitset<N>>([generator = *(this->generator), translator](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::bitset<N> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::bitset<N> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                        this->concurrent);
    }

    auto redirect(const function::BiFunction<std::bitset<N>, function::Timestamp, std::bitset<N>> &redirector) const -> Semantic<std::bitset<N>>
    {
        return Semantic<std::bitset<N>>([generator = *(this->generator), redirector](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::bitset<N> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::bitset<N> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                        this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::bitset<N>>
    {
        return Semantic<std::bitset<N>>([generator = *(this->generator), start, end](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::bitset<N> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::bitset<N> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                        this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::bitset<N>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::bitset<N>>>)
        {
            return Semantic<std::bitset<N>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::bitset<N> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::bitset<N> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::bitset<N> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::bitset<N> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                            this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::bitset<N>>)
        {
            return Semantic<std::bitset<N>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::bitset<N> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::bitset<N> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                            this->concurrent);
        }
        else
        {
            return Semantic<std::bitset<N>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::bitset<N>, function::Timestamp> accept, function::BiPredicate<std::bitset<N>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::bitset<N> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::bitset<N> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                            this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::bitset<N>> { return collectable::UnorderedCollectable<std::bitset<N>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::bitset<N>> { return collectable::OrderedCollectable<std::bitset<N>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::bitset<N>> { return collectable::WindowCollectable<std::bitset<N>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::bitset<N>, D> { return collectable::Statistics<std::bitset<N>, D>(this->source(), this->concurrent); }
};

template <typename K, typename V>
class Semantic<std::unordered_multimap<K, V>>
{
  private:
    std::unique_ptr<function::Generator<std::unordered_multimap<K, V>>> generator;
    function::Module concurrent;

  public:
    using Element = std::unordered_multimap<K, V>;

    Semantic(std::unordered_multimap<K, V> &&container) : generator(std::make_unique<function::Generator<std::unordered_multimap<K, V>>>([elements = std::move(container)](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
                                                              accept(elements, 0LL);
                                                          })),
                                                          concurrent(1) {}

    Semantic(std::unordered_multimap<K, V> &&container, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::unordered_multimap<K, V>>>([elements = std::move(container)](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
                                                                                                  accept(elements, 0LL);
                                                                                              })),
                                                                                              concurrent(concurrent) {}

    Semantic(const function::Generator<std::unordered_multimap<K, V>> &generator) : generator(std::make_unique<function::Generator<std::unordered_multimap<K, V>>>(generator)), concurrent(1) {}

    Semantic(const function::Generator<std::unordered_multimap<K, V>> &generator, const function::Module &concurrent) : generator(std::make_unique<function::Generator<std::unordered_multimap<K, V>>>(generator)), concurrent(concurrent) {}

    Semantic(const Semantic<std::unordered_multimap<K, V>> &other) : generator(std::make_unique<function::Generator<std::unordered_multimap<K, V>>>(*other.generator)), concurrent(other.concurrent) {}

    Semantic(Semantic<std::unordered_multimap<K, V>> &&other) noexcept = default;

    Semantic<std::unordered_multimap<K, V>> &operator=(const Semantic<std::unordered_multimap<K, V>> &other)
    {
        if (this != &other)
        {
            generator = std::make_unique<function::Generator<std::unordered_multimap<K, V>>>(*other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    Semantic<std::unordered_multimap<K, V>> &operator=(Semantic<std::unordered_multimap<K, V>> &&other) noexcept = default;

    auto source() const -> function::Generator<std::unordered_multimap<K, V>>
    {
        return [generator = *(this->generator)](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto getConcurrent() const -> function::Module { return concurrent; }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        if constexpr (std::is_invocable_v<Mapper, std::unordered_multimap<K, V>, function::Timestamp>)
        {
            using R = std::invoke_result_t<Mapper, std::unordered_multimap<K, V>, function::Timestamp>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container, index);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Mapper, std::unordered_multimap<K, V>>)
        {
            using R = std::invoke_result_t<Mapper, std::unordered_multimap<K, V>>;
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &mapper, &stop, &interrupt](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                    R mapped = std::invoke(mapper, container);
                    accept(mapped, index);
                    stop = stop || interrupt(mapped, index); }, [&stop](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Mapper, std::unordered_multimap<K, V>, function::Timestamp> || std::is_invocable_v<Mapper, std::unordered_multimap<K, V>>, "Mapper must be callable as either (unordered_multimap<K,V>, Timestamp) -> R or (unordered_multimap<K,V>) -> R");
        }
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::unordered_multimap<K, V>>
    {
        return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_multimap<K, V>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_multimap<K, V>>)
                {
                    matches = std::invoke(predicate, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::unordered_multimap<K, V>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::unordered_multimap<K, V>>, "Predicate must be callable as either (unordered_multimap<K,V>, Timestamp) -> bool or (unordered_multimap<K,V>) -> bool");
                }
                if (matches)
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                       this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<std::unordered_multimap<K, V>>
    {
        return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
            bool stop = false;
            generator([&accept, &stop, &predicate](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_multimap<K, V>, function::Timestamp>)
                {
                    matches = std::invoke(predicate, container, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_multimap<K, V>>)
                {
                    matches = std::invoke(predicate, container);
                }
                if (matches)
                {
                    accept(container, index);
                }
                else
                {
                    stop = true;
                } }, [&interrupt, &stop](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, index) || stop; });
        },
                                                       this->concurrent);
    }

    template <typename Predicate>
    auto dropWhile(Predicate &&predicate) const -> Semantic<std::unordered_multimap<K, V>>
    {
        return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
            bool dropping = true;
            function::Timestamp count = 0LL;
            generator([&accept, &dropping, &count, &predicate](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                if (dropping)
                {
                    bool matches = false;
                    if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_multimap<K, V>, function::Timestamp>)
                    {
                        matches = std::invoke(predicate, container, index);
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_multimap<K, V>>)
                    {
                        matches = std::invoke(predicate, container);
                    }
                    if (!matches)
                    {
                        dropping = false;
                        accept(container, count);
                        count++;
                    }
                }
                else
                {
                    accept(container, count);
                    count++;
                } }, [&interrupt, &count](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                       this->concurrent);
    }

    auto limit(const function::Module &n) const -> Semantic<std::unordered_multimap<K, V>>
    {
        return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), n](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                accept(container, count);
                count++; }, [&interrupt, &count, &n](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= n; });
        },
                                                       this->concurrent);
    }

    auto skip(const function::Module &n) const -> Semantic<std::unordered_multimap<K, V>>
    {
        return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), n](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &n](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                if (count >= n)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
        },
                                                       this->concurrent);
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<std::unordered_multimap<K, V>>
    {
        return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &consumer](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<void, Consumer, std::unordered_multimap<K, V>, function::Timestamp>)
                {
                    std::invoke(consumer, container, index);
                }
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::unordered_multimap<K, V>>)
                {
                    std::invoke(consumer, container);
                }
                accept(container, index);
            },
                      interrupt);
        },
                                                       this->concurrent);
    }

    auto flat() const -> Semantic<std::pair<const K, V>>
    {
        return Semantic<std::pair<const K, V>>([generator = *(this->generator)](function::BiConsumer<std::pair<const K, V>, function::Timestamp> accept, function::BiPredicate<std::pair<const K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &interrupt](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                for (const auto &element : container)
                {
                    if (stop)
                    {
                        break;
                    }
                    if (interrupt(element, count))
                    {
                        stop = true;
                        break;
                    }
                    accept(element, count);
                    count++;
                } }, [&stop](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
        },
                                               this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::pair<const K, V>>
    {
        return Semantic<std::pair<const K, V>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::pair<const K, V>, function::Timestamp> accept, function::BiPredicate<std::pair<const K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::unordered_multimap<K, V>, function::Timestamp>)
                {
                    Semantic<std::pair<const K, V>> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::unordered_multimap<K, V>>)
                {
                    Semantic<std::pair<const K, V>> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::pair<const K, V> innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::unordered_multimap<K, V>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::pair<const K, V>>, Flatten, std::unordered_multimap<K, V>>, "flat: Flatten must return Semantic<pair<const K,V>>");
                } }, [&stop](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
        },
                                               this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        if constexpr (std::is_invocable_v<Flatten, std::unordered_multimap<K, V>, function::Timestamp>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::unordered_multimap<K, V>, function::Timestamp>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else if constexpr (std::is_invocable_v<Flatten, std::unordered_multimap<K, V>>)
        {
            using InnerSemantic = std::invoke_result_t<Flatten, std::unordered_multimap<K, V>>;
            using R = typename InnerSemantic::Element;
            return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                    Semantic<R> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&interrupt, &stop, &count](R innerElement, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerElement, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    }); }, [&stop](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
            },
                               this->concurrent);
        }
        else
        {
            static_assert(std::is_invocable_v<Flatten, std::unordered_multimap<K, V>, function::Timestamp> || std::is_invocable_v<Flatten, std::unordered_multimap<K, V>>, "flatMap: Flatten must return a Semantic type");
        }
    }

    auto parallel() const -> Semantic<std::unordered_multimap<K, V>> { return Semantic<std::unordered_multimap<K, V>>(this->source(), 1); }
    auto parallel(const function::Module &concurrent) const -> Semantic<std::unordered_multimap<K, V>> { return Semantic<std::unordered_multimap<K, V>>(this->source(), std::max(concurrent, 1ULL)); }

    auto reverse() const -> Semantic<std::unordered_multimap<K, V>>
    {
        return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator)](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept](std::unordered_multimap<K, V> container, function::Timestamp index) -> void { accept(container, -index); }, [&interrupt](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, -index); });
        },
                                                       this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<std::unordered_multimap<K, V>>
    {
        return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), offset](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &offset](std::unordered_multimap<K, V> container, function::Timestamp index) -> void { accept(container, index + offset); }, [&interrupt, &offset](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, index + offset); });
        },
                                                       this->concurrent);
    }

    auto translate(const function::BiFunction<std::unordered_multimap<K, V>, function::Timestamp, function::Timestamp> &translator) const -> Semantic<std::unordered_multimap<K, V>>
    {
        return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), translator](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &translator](std::unordered_multimap<K, V> container, function::Timestamp index) -> void { accept(container, translator(container, index)); }, [&interrupt, &translator](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, translator(container, index)); });
        },
                                                       this->concurrent);
    }

    auto redirect(const function::BiFunction<std::unordered_multimap<K, V>, function::Timestamp, std::unordered_multimap<K, V>> &redirector) const -> Semantic<std::unordered_multimap<K, V>>
    {
        return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), redirector](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
            generator([&accept, &redirector](std::unordered_multimap<K, V> container, function::Timestamp index) -> void { accept(redirector(container, index), index); }, [&interrupt, &redirector](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(redirector(container, index), index); });
        },
                                                       this->concurrent);
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<std::unordered_multimap<K, V>>
    {
        return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), start, end](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
            function::Module count = 0;
            generator([&accept, &count, &start, &end](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                if (count >= start && count < end)
                {
                    accept(container, count);
                }
                count++; }, [&interrupt, &count, &end](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count) || count >= end; });
        },
                                                       this->concurrent);
    }

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<std::unordered_multimap<K, V>>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<std::unordered_multimap<K, V>>>)
        {
            return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &count](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&stop](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return stop; });
                other.source()([&accept, &count](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &stop, &count](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool {
                    if (interrupt(container, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false; });
            },
                                                           this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, std::unordered_multimap<K, V>>)
        {
            return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                if (!interrupt(elements, count))
                {
                    accept(elements, count);
                }
            },
                                                           this->concurrent);
        }
        else
        {
            return Semantic<std::unordered_multimap<K, V>>([generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<std::unordered_multimap<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_multimap<K, V>, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                generator([&accept, &count](std::unordered_multimap<K, V> container, function::Timestamp index) -> void {
                    accept(container, count);
                    count++; }, [&interrupt, &count](std::unordered_multimap<K, V> container, function::Timestamp index) -> bool { return interrupt(container, count); });
                for (const auto &element : elements)
                {
                    if (interrupt(element, count))
                    {
                        break;
                    }
                    accept(element, count);
                    count++;
                }
            },
                                                           this->concurrent);
        }
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<std::unordered_multimap<K, V>> { return collectable::UnorderedCollectable<std::unordered_multimap<K, V>>(this->source(), this->concurrent); }
    auto toOrdered() const -> collectable::OrderedCollectable<std::unordered_multimap<K, V>> { return collectable::OrderedCollectable<std::unordered_multimap<K, V>>(this->source(), this->concurrent); }
    auto toWindow() const -> collectable::WindowCollectable<std::unordered_multimap<K, V>> { return collectable::WindowCollectable<std::unordered_multimap<K, V>>(this->source(), this->concurrent); }
    template <typename D>
    auto toStatistics() const -> collectable::Statistics<std::unordered_multimap<K, V>, D> { return collectable::Statistics<std::unordered_multimap<K, V>, D>(this->source(), this->concurrent); }
};
} // namespace semantic

template <typename E>
auto collectable::WindowCollectable<E>::slide(const function::Module &size, const function::Timestamp &step) const -> semantic::Semantic<semantic::Semantic<E>>
{
    return semantic::Semantic<semantic::Semantic<E>>([buffer = this->buffer, size, step](auto accept, auto interrupt) -> void {
        function::Module total = buffer.size();
        function::Module outerIndex = 0LL;
        bool stop = false;
        for (function::Module start = 0; start < total && !stop; start += step)
        {
            function::Module end = std::min(start + size, total);
            if (start < end)
            {
                std::vector<E> window;
                for (function::Module i = start; i < end; i++)
                {
                    auto it = buffer.find(i);
                    if (it != buffer.end())
                    {
                        window.push_back(it->second);
                    }
                }
                auto inner = semantic::Semantic<E>([window = std::move(window)](function::BiConsumer<E, function::Timestamp> innerAccept, function::BiPredicate<E, function::Timestamp> innerInterrupt) -> void {
                    function::Timestamp innerIdx = 0LL;
                    bool innerStop = false;
                    for (const auto &element : window)
                    {
                        if (innerInterrupt(element, innerIdx))
                        {
                            innerStop = true;
                            break;
                        }
                        if (!innerStop)
                        {
                            innerAccept(element, innerIdx);
                            innerIdx++;
                        }
                    }
                },
                                                   1LL);
                if (interrupt(inner, outerIndex))
                {
                    break;
                }
                accept(inner, outerIndex);
                outerIndex++;
            }
        }
    },
                                                     this->concurrent);
}