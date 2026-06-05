#pragma once

#include "hash.h"
#include "less.h"
#include "function.h"
#include "charsequence.h"
#include "collector.h"

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
  protected:
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

    virtual ~Semantic() = default;

    template <typename Container>
    auto concatenate(Container &&container) const -> Semantic<E>
    {
        if constexpr (std::is_same_v<std::decay_t<Container>, Semantic<E>>)
        {
            return Semantic<E>(
                [generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                    function::Timestamp count = 0LL;
                    bool stop = false;
                    generator(
                        [&accept, &count](E element, function::Timestamp index) -> void {
                            accept(element, count);
                            count++;
                        },
                        [&stop](E element, function::Timestamp index) -> bool {
                            return stop;
                        });
                    other.source()(
                        [&accept, &count](E element, function::Timestamp index) -> void {
                            accept(element, count);
                            count++;
                        },
                        [&interrupt, &stop, &count](E element, function::Timestamp index) -> bool {
                            if (interrupt(element, count))
                            {
                                stop = true;
                                return true;
                            }
                            return false;
                        });
                },
                this->concurrent);
        }
        else if constexpr (std::is_same_v<std::decay_t<Container>, E>)
        {
            return Semantic<E>(
                [generator = *(this->generator), element = std::forward<Container>(container)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                    function::Timestamp count = 0LL;
                    generator(
                        [&accept, &count](E current, function::Timestamp index) -> void {
                            accept(current, count);
                            count++;
                        },
                        [&interrupt, &count](E current, function::Timestamp index) -> bool {
                            return interrupt(current, count);
                        });
                    if (!interrupt(element, count))
                    {
                        accept(element, count);
                    }
                },
                this->concurrent);
        }
        else if constexpr (std::is_invocable_v<std::decay_t<Container>, function::BiConsumer<E, function::Timestamp>, function::BiPredicate<E, function::Timestamp>>)
        {
            return Semantic<E>(
                [generator = *(this->generator), other = std::forward<Container>(container)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                    function::Timestamp count = 0LL;
                    bool stop = false;
                    generator(
                        [&accept, &count](E element, function::Timestamp index) -> void {
                            accept(element, count);
                            count++;
                        },
                        [&stop](E element, function::Timestamp index) -> bool {
                            return stop;
                        });
                    other(
                        [&accept, &count](E element, function::Timestamp index) -> void {
                            accept(element, count);
                            count++;
                        },
                        [&interrupt, &stop, &count](E element, function::Timestamp index) -> bool {
                            if (interrupt(element, count))
                            {
                                stop = true;
                                return true;
                            }
                            return false;
                        });
                },
                this->concurrent);
        }
        else
        {
            return Semantic<E>(
                [generator = *(this->generator), elements = std::forward<Container>(container)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                    function::Timestamp count = 0LL;
                    generator(
                        [&accept, &count](E element, function::Timestamp index) -> void {
                            accept(element, count);
                            count++;
                        },
                        [&interrupt, &count](E element, function::Timestamp index) -> bool {
                            return interrupt(element, count);
                        });
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

    auto distinct() const -> Semantic<E>
    {
        return Semantic<E>(
            [generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                std::unordered_set<E> seen;
                function::Timestamp count = 0LL;
                generator(
                    [&accept, &seen, &count](E element, function::Timestamp index) -> void {
                        if (seen.find(element) == seen.end())
                        {
                            seen.insert(element);
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

    auto distinct(const function::Comparator<E> &comparator) const -> Semantic<E>
    {
        return Semantic<E>(
            [generator = *(this->generator), comparator](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                std::set<E, function::Comparator<E>> seen(comparator);
                function::Timestamp count = 0LL;
                generator(
                    [&accept, &seen, &count](E element, function::Timestamp index) -> void {
                        if (seen.find(element) == seen.end())
                        {
                            seen.insert(element);
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
    auto dropWhile(Predicate &&predicate) const -> Semantic<E>
    {
        return Semantic<E>(
            [generator = *(this->generator), predicate = std::forward<Predicate>(predicate), this](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                bool dropping = true;
                function::Timestamp count = 0LL;
                generator(
                    [&accept, &dropping, &count, &predicate, this](E element, function::Timestamp index) -> void {
                        if (dropping)
                        {
                            if (!this->invoke(predicate, element, index))
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
                        }
                    },
                    [&interrupt, &count](E element, function::Timestamp index) -> bool {
                        return interrupt(element, count);
                    });
            },
            this->concurrent);
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<E>
    {
        return Semantic<E>(
            [generator = *(this->generator), predicate = std::forward<Predicate>(predicate), this](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) mutable -> void {
                function::Timestamp count = 0;
                generator(
                    [&accept, &count, &predicate, this](E element, function::Timestamp index) -> void {
                        if (this->invoke(predicate, element, index))
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

    template <typename T = E, typename = typename T::Element>
    auto flat() const -> Semantic<typename T::Element>
    {
        using InnerType = typename T::Element;
        return Semantic<InnerType>(
            [generator = *(this->generator)](function::BiConsumer<InnerType, function::Timestamp> accept, function::BiPredicate<InnerType, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator(
                    [&accept, &count, &stop](E inner, function::Timestamp index) -> void {
                        inner.source()(
                            [&accept, &count](InnerType innerElement, function::Timestamp innerIndex) -> void {
                                accept(innerElement, count);
                                count++;
                            },
                            [&stop](InnerType innerElement, function::Timestamp innerIndex) -> bool {
                                return stop;
                            });
                    },
                    [&stop](E, function::Timestamp) -> bool {
                        return stop;
                    });
            },
            this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const
    {
        using InnerSemantic = std::decay_t<decltype(this->invoke(std::forward<Flatten>(flatten), std::declval<E>(), std::declval<function::Timestamp>()))>;
        using InnerType = typename InnerSemantic::Element;
        return Semantic<InnerType>(
            [generator = *(this->generator), flatten = std::forward<Flatten>(flatten), this](function::BiConsumer<InnerType, function::Timestamp> accept, function::BiPredicate<InnerType, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator(
                    [&accept, &count, &stop, &flatten, &interrupt, this](E element, function::Timestamp index) -> void {
                        auto inner = this->invoke(flatten, element, index);
                        inner.source()(
                            [&accept, &count](InnerType innerElement, function::Timestamp innerIndex) -> void {
                                accept(innerElement, count);
                                count++;
                            },
                            [&interrupt, &stop, &count](InnerType innerElement, function::Timestamp innerIndex) -> bool {
                                if (interrupt(innerElement, count))
                                {
                                    stop = true;
                                    return true;
                                }
                                return false;
                            });
                    },
                    [&stop](E element, function::Timestamp index) -> bool {
                        return stop;
                    });
            },
            this->concurrent);
    }

    template <typename Flatten>
    auto flatMap(Flatten &&flatten) const
    {
        using InnerSemantic = std::decay_t<decltype(this->invoke(std::forward<Flatten>(flatten), std::declval<E>(), std::declval<function::Timestamp>()))>;
        using InnerType = typename InnerSemantic::Element;
        return Semantic<InnerType>(
            [generator = *(this->generator), flatten = std::forward<Flatten>(flatten), this](function::BiConsumer<InnerType, function::Timestamp> accept, function::BiPredicate<InnerType, function::Timestamp> interrupt) -> void {
                function::Timestamp count = 0LL;
                bool stop = false;
                generator(
                    [&accept, &count, &stop, &flatten, this](E element, function::Timestamp index) -> void {
                        auto inner = this->invoke(flatten, element, index);
                        inner.source()(
                            [&accept, &count](InnerType innerElement, function::Timestamp innerIndex) -> void {
                                accept(innerElement, count);
                                count++;
                            },
                            [&stop](InnerType innerElement, function::Timestamp innerIndex) -> bool {
                                return stop;
                            });
                    },
                    [&stop](E element, function::Timestamp index) -> bool {
                        return stop;
                    });
            },
            this->concurrent);
    }

    auto getConcurrent() const -> function::Module
    {
        return concurrent;
    }

    template <typename Function, typename Type>
    auto invoke(Function &&function, Type &&element, function::Timestamp index) const
    {
        if constexpr (std::is_invocable_v<Function, Type, function::Timestamp>)
        {
            return std::invoke(std::forward<Function>(function), std::forward<Type>(element), index);
        }
        else if constexpr (std::is_invocable_v<Function, Type>)
        {
            return std::invoke(std::forward<Function>(function), std::forward<Type>(element));
        }
        else
        {
            static_assert(sizeof(Type) == 0, "Function signature does not match");
        }
    }

    auto limit(const function::Module &limit) const -> Semantic<E>
    {
        return Semantic<E>(
            [generator = *(this->generator), limit](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                function::Module count = 0;
                generator(
                    [&accept, &count](E element, function::Timestamp index) -> void {
                        accept(element, count);
                        count++;
                    },
                    [&interrupt, &count, &limit](E element, function::Timestamp index) -> bool {
                        return interrupt(element, count) || count >= limit;
                    });
            },
            this->concurrent);
    }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
        using Result = std::decay_t<decltype(this->invoke(std::forward<Mapper>(mapper), std::declval<E>(), std::declval<function::Timestamp>()))>;
        static_assert(!std::is_same_v<Result, void>, "Mapper must not return void");
        return Semantic<Result>(
            [generator = *(this->generator), mapper = std::forward<Mapper>(mapper), this](function::BiConsumer<Result, function::Timestamp> accept, function::BiPredicate<Result, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator(
                    [&accept, &mapper, &stop, &interrupt, this](E element, function::Timestamp index) -> void {
                        Result mapped = this->invoke(mapper, element, index);
                        accept(mapped, index);
                        stop = stop || interrupt(mapped, index);
                    },
                    [&stop](E element, function::Timestamp index) -> bool {
                        return stop;
                    });
            },
            this->concurrent);
    }

    auto parallel() const -> Semantic<E>
    {
        return Semantic<E>(this->source(), 1);
    }

    auto parallel(const function::Module &concurrent) const -> Semantic<E>
    {
        return Semantic<E>(this->source(), std::max(concurrent, 1ULL));
    }

    template <typename Consumer>
    auto peek(Consumer &&consumer) const -> Semantic<E>
    {
        return Semantic<E>(
            [generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                generator(
                    [&accept, &consumer](E element, function::Timestamp index) -> void {
                        if constexpr (std::is_invocable_v<Consumer, E, function::Timestamp>)
                        {
                            std::invoke(consumer, element, index);
                        }
                        else if constexpr (std::is_invocable_v<Consumer, E>)
                        {
                            std::invoke(consumer, element);
                        }
                        else
                        {
                            static_assert(sizeof(E) == 0, "Consumer signature does not match");
                        }
                        accept(element, index);
                    },
                    interrupt);
            },
            this->concurrent);
    }

    auto redirect(const function::BiFunction<E, function::Timestamp, E> &redirector) const -> Semantic<E>
    {
        return Semantic<E>(
            [generator = *(this->generator), redirector](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                generator(
                    [&accept, &redirector](E element, function::Timestamp index) -> void {
                        accept(redirector(element, index), index);
                    },
                    [&interrupt, &redirector](E element, function::Timestamp index) -> bool {
                        return interrupt(redirector(element, index), index);
                    });
            },
            this->concurrent);
    }

    auto reverse() const -> Semantic<E>
    {
        return Semantic<E>(
            [generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                generator(
                    [&accept](E element, function::Timestamp index) -> void {
                        accept(element, -index);
                    },
                    [&interrupt](E element, function::Timestamp index) -> bool {
                        return interrupt(element, -index);
                    });
            },
            this->concurrent);
    }

    auto skip(const function::Module &skip) const -> Semantic<E>
    {
        return Semantic<E>(
            [generator = *(this->generator), skip](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                function::Module count = 0;
                generator(
                    [&accept, &count, &skip](E element, function::Timestamp index) -> void {
                        if (count >= skip)
                        {
                            accept(element, count);
                        }
                        count++;
                    },
                    [&interrupt, &count](E element, function::Timestamp index) -> bool {
                        return interrupt(element, count);
                    });
            },
            this->concurrent);
    }

    auto sort() const -> collectable::OrderedCollectable<E>
    {
        if constexpr (std::is_invocable_v<std::less<E>, E, E>)
        {
            return collectable::OrderedCollectable<E>(
                this->source(),
                [](const E &left, const E &right) -> bool { return left < right; },
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

    auto source() const -> function::Generator<E>
    {
        return [generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            generator(accept, interrupt);
        };
    }

    auto sub(const function::Module &start, const function::Module &end) const -> Semantic<E>
    {
        return Semantic<E>(
            [generator = *(this->generator), start, end](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                function::Module count = 0;
                generator(
                    [&accept, &count, &start, &end](E element, function::Timestamp index) -> void {
                        if (count >= start && count < end)
                        {
                            accept(element, count);
                        }
                        count++;
                    },
                    [&interrupt, &count, &end](E element, function::Timestamp index) -> bool {
                        return interrupt(element, count) || count >= end;
                    });
            },
            this->concurrent);
    }

    template <typename Predicate>
    auto takeWhile(Predicate &&predicate) const -> Semantic<E>
    {
        return Semantic<E>(
            [generator = *(this->generator), predicate = std::forward<Predicate>(predicate), this](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                bool stop = false;
                generator(
                    [&accept, &stop, &predicate, this](E element, function::Timestamp index) -> void {
                        if (this->invoke(predicate, element, index))
                        {
                            accept(element, index);
                        }
                        else
                        {
                            stop = true;
                        }
                    },
                    [&interrupt, &stop](E element, function::Timestamp index) -> bool {
                        return interrupt(element, index) || stop;
                    });
            },
            this->concurrent);
    }

    auto toOrdered() const -> collectable::OrderedCollectable<E>
    {
        return collectable::OrderedCollectable<E>(this->source(), this->concurrent);
    }

    template <typename Distribution>
    auto toStatistics() const -> collectable::Statistics<E, Distribution>
    {
        return collectable::Statistics<E, Distribution>(this->source(), this->concurrent);
    }

    auto toUnordered() const -> collectable::UnorderedCollectable<E>
    {
        return collectable::UnorderedCollectable<E>(this->source(), this->concurrent);
    }

    auto toWindow() const -> collectable::WindowCollectable<E>
    {
        return collectable::WindowCollectable<E>(this->source(), this->concurrent);
    }

    auto translate(const function::Timestamp &offset) const -> Semantic<E>
    {
        return Semantic<E>(
            [generator = *(this->generator), offset](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
                generator(
                    [&accept, &offset](E element, function::Timestamp index) -> void {
                        accept(element, index + offset);
                    },
                    [&interrupt, &offset](E element, function::Timestamp index) -> bool {
                        return interrupt(element, index + offset);
                    });
            },
            this->concurrent);
    }
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

auto useBlob(std::istream &stream, const char &delimiter) -> Semantic<std::string>
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
    std::string content;
    std::string chunk;
    constexpr std::size_t bufferSize = 4096;
    chunk.resize(bufferSize);
    while (stream.read(&chunk[0], bufferSize) || stream.gcount() > 0)
    {
        content.append(chunk, 0, stream.gcount());
    }
    charsequence::Charsequence sequence(content, charsequence::charset::utf8, charsequence::charset::utf8);
    return Semantic<charsequence::Charsequence>([sequence](function::BiConsumer<charsequence::Charsequence, function::Timestamp> accept, function::BiPredicate<charsequence::Charsequence, function::Timestamp> interrupt) -> void {
        if (!interrupt(sequence, 0LL))
        {
            accept(sequence, 0LL);
        }
    },
                                                1LL);
}

auto useText(std::istream &stream, const char &delimiter) -> Semantic<charsequence::Charsequence>
{
    std::string content;
    std::string chunk;
    constexpr std::size_t bufferSize = 4096;
    chunk.resize(bufferSize);
    while (stream.read(&chunk[0], bufferSize) || stream.gcount() > 0)
    {
        content.append(chunk, 0, stream.gcount());
    }
    charsequence::Charsequence delim(std::string(1, delimiter));
    charsequence::Charsequence sequence(content, charsequence::charset::utf8, charsequence::charset::utf8);
    auto parts = sequence.split(delim);
    return Semantic<charsequence::Charsequence>([parts = std::move(parts)](function::BiConsumer<charsequence::Charsequence, function::Timestamp> accept, function::BiPredicate<charsequence::Charsequence, function::Timestamp> interrupt) -> void {
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
    std::string content;
    std::string chunk;
    constexpr std::size_t bufferSize = 4096;
    chunk.resize(bufferSize);
    while (stream.read(&chunk[0], bufferSize) || stream.gcount() > 0)
    {
        content.append(chunk, 0, stream.gcount());
    }
    charsequence::Charsequence sequence(content, encoding, encoding);
    std::vector<charsequence::Point> points = sequence.getPoints();
    return Semantic<charsequence::Point>([points = std::move(points)](function::BiConsumer<charsequence::Point, function::Timestamp> accept, function::BiPredicate<charsequence::Point, function::Timestamp> interrupt) -> void {
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
    std::string content;
    std::string chunk;
    constexpr std::size_t bufferSize = 4096;
    chunk.resize(bufferSize);
    while (stream.read(&chunk[0], bufferSize) || stream.gcount() > 0)
    {
        content.append(chunk, 0, stream.gcount());
    }
    charsequence::Charsequence sequence(content, encoding, encoding);
    return Semantic<charsequence::Charsequence>([sequence](function::BiConsumer<charsequence::Charsequence, function::Timestamp> accept, function::BiPredicate<charsequence::Charsequence, function::Timestamp> interrupt) -> void {
        if (!interrupt(sequence, 0LL))
        {
            accept(sequence, 0LL);
        }
    },
                                                1LL);
}

auto useCharsequence(std::istream &stream, const charsequence::Charsequence &delimiter, charsequence::charset encoding = charsequence::charset::utf8) -> Semantic<charsequence::Charsequence>
{
    std::string content;
    std::string chunk;
    constexpr std::size_t bufferSize = 4096;
    chunk.resize(bufferSize);
    while (stream.read(&chunk[0], bufferSize) || stream.gcount() > 0)
    {
        content.append(chunk, 0, stream.gcount());
    }
    charsequence::Charsequence sequence(content, encoding, encoding);
    auto parts = sequence.split(delimiter);
    return Semantic<charsequence::Charsequence>([parts = std::move(parts)](function::BiConsumer<charsequence::Charsequence, function::Timestamp> accept, function::BiPredicate<charsequence::Charsequence, function::Timestamp> interrupt) -> void {
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