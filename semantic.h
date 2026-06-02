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

    template <typename Predicate>
    auto noneMatch(Predicate &&predicate) const -> bool
    {
        collector::Collector<E, bool, bool> collectorValue = collector::useNoneMatch<E>(std::forward<Predicate>(predicate));
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
    std::map<function::Timestamp, E> buffer;

  public:
    OrderedCollectable(const function::Generator<E> &generator) : Collectable<E>(1)
    {
        std::set<std::pair<function::Timestamp, E>> tempBuffer;
        generator([&tempBuffer](E element, function::Timestamp index) -> void { tempBuffer.insert(std::make_pair(index, element)); }, [](E element, function::Timestamp index) -> bool { return false; });
        for (const auto &pair : tempBuffer)
        {
            function::Timestamp index = pair.first < 0 ? (tempBuffer.size() - (static_cast<function::Timestamp>(std::abs(pair.first)) - 1) % tempBuffer.size() - 1) : (pair.first % tempBuffer.size());
            this->buffer.insert(std::make_pair(index, pair.second));
        }
    }

    OrderedCollectable(const function::Generator<E> &generator, const function::Module &concurrent) : Collectable<E>(concurrent)
    {
        std::set<std::pair<function::Timestamp, E>> tempBuffer;
        generator([&tempBuffer](E element, function::Timestamp index) -> void { tempBuffer.insert(std::make_pair(index, element)); }, [](E element, function::Timestamp index) -> bool { return false; });
        for (const auto &pair : tempBuffer)
        {
            function::Timestamp index = pair.first < 0 ? (tempBuffer.size() - (static_cast<function::Timestamp>(std::abs(pair.first)) - 1) % tempBuffer.size() - 1) : (pair.first % tempBuffer.size());
            this->buffer.insert(std::make_pair(index, pair.second));
        }
    }

    OrderedCollectable(const function::Generator<E> &generator, const function::Comparator<E> &comparator) : Collectable<E>(1)
    {
        auto comp = [comparator](const std::pair<function::Timestamp, E> &a, const std::pair<function::Timestamp, E> &b) -> bool {
            return comparator(a.second, b.second) < 0;
        };
        std::set<std::pair<function::Timestamp, E>, decltype(comp)> tempBuffer(comp);
        generator([&tempBuffer](E element, function::Timestamp index) -> void { tempBuffer.insert(std::make_pair(index, element)); }, [](E element, function::Timestamp index) -> bool { return false; });
        for (const auto &pair : tempBuffer)
        {
            function::Timestamp index = pair.first < 0 ? (tempBuffer.size() - (static_cast<function::Timestamp>(std::abs(pair.first)) - 1) % tempBuffer.size() - 1) : (pair.first % tempBuffer.size());
            this->buffer.insert(std::make_pair(index, pair.second));
        }
    }

    OrderedCollectable(const function::Generator<E> &generator, const function::Comparator<E> &comparator, const function::Module &concurrent) : Collectable<E>(concurrent)
    {
        auto comp = [comparator](const std::pair<function::Timestamp, E> &a, const std::pair<function::Timestamp, E> &b) -> bool {
            return comparator(a.second, b.second) < 0;
        };
        std::set<std::pair<function::Timestamp, E>, decltype(comp)> tempBuffer(comp);
        generator([&tempBuffer](E element, function::Timestamp index) -> void { tempBuffer.insert(std::make_pair(index, element)); }, [](E element, function::Timestamp index) -> bool { return false; });
        for (const auto &pair : tempBuffer)
        {
            function::Timestamp index = pair.first < 0 ? (tempBuffer.size() - (static_cast<function::Timestamp>(std::abs(pair.first)) - 1) % tempBuffer.size() - 1) : (pair.first % tempBuffer.size());
            this->buffer.insert(std::make_pair(index, pair.second));
        }
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
    Statistics(const function::Module &concurrent) : OrderedCollectable<E>(concurrent) {}
    Statistics(const function::Generator<E> &generator, const function::Module &concurrent) : OrderedCollectable<E>(generator, concurrent) {}
    Statistics(const Statistics<E, D> &other) : OrderedCollectable<E>(other) {}
    Statistics(Statistics<E, D> &&other) noexcept : OrderedCollectable<E>(std::move(other)) {}

    auto operator=(const Statistics<E, D> &other) -> Statistics<E, D> &
    {
        if (this != &other)
            OrderedCollectable<E>::operator=(other);
        return *this;
    }

    auto operator=(Statistics<E, D> &&other) noexcept -> Statistics<E, D> &
    {
        if (this != &other)
            OrderedCollectable<E>::operator=(std::move(other));
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
        collector::Collector<E, std::pair<D, std::vector<D>>, D> collectorValue = collector::useVariance<E, D>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto variance(const function::Function<E, D> &mapper) const -> D
    {
        collector::Collector<E, std::pair<D, std::vector<D>>, D> collectorValue = collector::useVariance<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto standardDeviation() const -> D
    {
        collector::Collector<E, std::pair<D, std::vector<D>>, D> collectorValue = collector::useStandardDeviation<E, D>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto standardDeviation(const function::Function<E, D> &mapper) const -> D
    {
        collector::Collector<E, std::pair<D, std::vector<D>>, D> collectorValue = collector::useStandardDeviation<E, D>(mapper);
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto frequency() const -> std::map<E, std::complex<double>>
    {
        collector::Collector<E, std::unordered_map<E, std::complex<double>>, std::map<E, std::complex<double>>> collectorValue = collector::useFrequency<E>();
        return collectorValue.collect(this->source(), this->concurrent);
    }

    auto frequency(const function::Function<E, D> &mapper) const -> std::map<D, std::complex<double>>
    {
        collector::Collector<E, std::unordered_map<D, std::complex<double>>, std::map<D, std::complex<double>>> collectorValue = collector::useFrequency<E, D>(mapper);
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
    std::unordered_map<function::Timestamp, E> buffer;

  public:
    UnorderedCollectable(const function::Generator<E> &generator) : Collectable<E>(1)
    {
        generator([this](E element, function::Timestamp index) -> void { this->buffer.insert(std::make_pair(index, element)); }, [](E element, function::Timestamp index) -> bool { return false; });
    }

    UnorderedCollectable(const function::Generator<E> &generator, const function::Module &concurrent) : Collectable<E>(concurrent)
    {
        generator([this](E element, function::Timestamp index) -> void { this->buffer.insert(std::make_pair(index, element)); }, [](E element, function::Timestamp index) -> bool { return false; });
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
    auto map(Mapper &&mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<E>()))>
    {
        using R = decltype(std::declval<Mapper>()(std::declval<E>()));
        return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            R mapped;
            generator([&accept, &mapper, &mapped](E element, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<R, Mapper, E, function::Timestamp>)
                {
                    mapped = std::invoke(mapper, element, index);
                }
                else if constexpr (std::is_invocable_r_v<R, Mapper, E>)
                {
                    mapped = std::invoke(mapper, element);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<R, Mapper, E, function::Timestamp> || std::is_invocable_r_v<R, Mapper, E>, "Mapper must be callable as either (E, Timestamp) -> R or (E) -> R");
                }
                accept(mapped, index); }, [&interrupt, &mapped](E element, function::Timestamp index) -> bool { return interrupt(mapped, index); });
        },
                           this->concurrent);
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](E element, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, E, function::Timestamp>)
                {
                    matches = std::invoke(predicate, element, index);
                }
                else if constexpr (std::is_invocable_r_v<bool, Predicate, E>)
                {
                    matches = std::invoke(predicate, element);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<bool, Predicate, E, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, E>, "Predicate must be callable as either (E, Timestamp) -> bool or (E) -> bool");
                }
                if (matches)
                {
                    accept(element, count);
                    count++;
                } }, [&interrupt, &count](E element, function::Timestamp index) -> bool { return interrupt(element, count); });
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

    template <typename R, typename Flatten>
    auto flatMap(Flatten &&flatten) const -> Semantic<R>
    {
        return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten](E element, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, E, function::Timestamp>)
                {
                    Semantic<R> inner = std::invoke(flatten, element, index);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&stop](R innerElement, function::Timestamp innerIndex) -> bool {
                        return stop;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, E>)
                {
                    Semantic<R> inner = std::invoke(flatten, element);
                    inner.source()([&accept, &count](R innerElement, function::Timestamp innerIndex) -> void {
                        accept(innerElement, count);
                        count++;
                    }, [&stop](R innerElement, function::Timestamp innerIndex) -> bool {
                        return stop;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, E, function::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, E>, "Flatten must be callable as either (E, Timestamp) -> Semantic<R> or (E) -> Semantic<R>");
                } }, [&interrupt, &stop](E element, function::Timestamp index) -> bool { return interrupt(element, index) || stop; });
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
                    accept(element, count);
                    count++;
                }
            },
                               this->concurrent);
        }
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

    auto flatten() const -> Semantic<E>
    {
        return Semantic<E>([generator = *(this->generator)](function::BiConsumer<E, function::Timestamp> accept, function::BiPredicate<E, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop](Semantic<E> inner, function::Timestamp index) -> void { inner.source()([&accept, &count](E innerElement, function::Timestamp innerIndex) -> void {
                    accept(innerElement, count);
                    count++; }, [&stop](E innerElement, function::Timestamp innerIndex) -> bool { return stop; }); }, [&interrupt, &stop](Semantic<E> inner, function::Timestamp index) -> bool { return interrupt(inner, index) || stop; });
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

    template <typename Flatten,
              typename InnerSemantic = std::invoke_result_t<Flatten, Semantic<E>>,
              typename R = typename InnerSemantic::Element>
    auto flatMap(Flatten &&flatten) const -> Semantic<R>
    {
        return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](Semantic<E> element, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, Semantic<E>, function::Timestamp>)
                {
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
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, Semantic<E>>)
                {
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
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, Semantic<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, Semantic<E>>, "Flatten must be callable as either (Semantic<E>, Timestamp) -> Semantic<R> or (Semantic<E>) -> Semantic<R>");
                } }, [&stop](Semantic<E> element, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
    }

    template <typename Mapper>
    auto map(Mapper &&mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<Semantic<E>>()))>
    {
        using R = decltype(std::declval<Mapper>()(std::declval<Semantic<E>>()));
        return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            R mapped;
            generator([&accept, &mapper, &mapped](Semantic<E> element, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<R, Mapper, Semantic<E>, function::Timestamp>)
                {
                    mapped = std::invoke(mapper, element, index);
                }
                else if constexpr (std::is_invocable_r_v<R, Mapper, Semantic<E>>)
                {
                    mapped = std::invoke(mapper, element);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<R, Mapper, Semantic<E>, function::Timestamp> || std::is_invocable_r_v<R, Mapper, Semantic<E>>, "Mapper must be callable as either (Semantic<E>, Timestamp) -> R or (Semantic<E>) -> R");
                }
                accept(mapped, index); }, [&interrupt, &mapped](Semantic<E> element, function::Timestamp index) -> bool { return interrupt(mapped, index); });
        },
                           this->concurrent);
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
                    accept(element, count);
                    count++;
                }
            },
                                         this->concurrent);
        }
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
    auto map(Mapper &&mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<std::vector<E>>()))>
    {
        using R = decltype(std::declval<Mapper>()(std::declval<std::vector<E>>()));
        return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            R mapped;
            generator([&accept, &mapper, &mapped](std::vector<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<R, Mapper, std::vector<E>, function::Timestamp>)
                {
                    mapped = std::invoke(mapper, container, index);
                }
                else if constexpr (std::is_invocable_r_v<R, Mapper, std::vector<E>>)
                {
                    mapped = std::invoke(mapper, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<R, Mapper, std::vector<E>, function::Timestamp> || std::is_invocable_r_v<R, Mapper, std::vector<E>>, "Mapper must be callable as either (vector<E>, Timestamp) -> R or (vector<E>) -> R");
                }
                accept(mapped, index); }, [&interrupt, &mapped](std::vector<E> container, function::Timestamp index) -> bool { return interrupt(mapped, index); });
        },
                           this->concurrent);
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::vector<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::vector<E>, function::Timestamp>)
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::vector<E>>)
                    matches = std::invoke(predicate, container);
                else
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::vector<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::vector<E>>, "Predicate must be callable as either (vector<E>, Timestamp) -> bool or (vector<E>) -> bool");
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
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::vector<E>>)
                    matches = std::invoke(predicate, container);
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
                        matches = std::invoke(predicate, container, index);
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::vector<E>>)
                        matches = std::invoke(predicate, container);
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
        return collectable::OrderedCollectable<std::vector<E>>(this->source(), this->concurrent);
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
                    std::invoke(consumer, container, index);
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::vector<E>>)
                    std::invoke(consumer, container);
                accept(container, index);
            },
                      interrupt);
        },
                                        this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::vector<E>>
    {
        return Semantic<std::vector<E>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::vector<E>, function::Timestamp> accept, function::BiPredicate<std::vector<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::vector<E> container, function::Timestamp index) -> void {
            if constexpr (std::is_invocable_r_v<Semantic<std::vector<E>>, Flatten, std::vector<E>, function::Timestamp>)
            {
                Semantic<std::vector<E>> inner = std::invoke(flatten, container, index);
                inner.source()([&accept, &count](std::vector<E> innerContainer, function::Timestamp innerIndex) -> void {
                    accept(innerContainer, count);
                    count++;
                }, [&interrupt, &stop, &count](std::vector<E> innerContainer, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerContainer, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                });
            }
            else if constexpr (std::is_invocable_r_v<Semantic<std::vector<E>>, Flatten, std::vector<E>>)
            {
                Semantic<std::vector<E>> inner = std::invoke(flatten, container);
                inner.source()([&accept, &count](std::vector<E> innerContainer, function::Timestamp innerIndex) -> void {
                    accept(innerContainer, count);
                    count++;
                }, [&interrupt, &stop, &count](std::vector<E> innerContainer, function::Timestamp innerIndex) -> bool {
                    if (interrupt(innerContainer, count))
                    {
                        stop = true;
                        return true;
                    }
                    return false;
                });
            }
            else
            {
                static_assert(std::is_invocable_r_v<Semantic<std::vector<E>>, Flatten, std::vector<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::vector<E>>, Flatten, std::vector<E>>, "flat: Flatten must return Semantic<vector<E>>");
            } }, [&stop](std::vector<E> container, function::Timestamp index) -> bool { return stop; });
        },
                                        this->concurrent);
    }

    template <typename Flatten,
              typename InnerSemantic = std::invoke_result_t<Flatten, std::vector<E>>,
              typename R = typename InnerSemantic::Element>
    auto flatMap(Flatten &&flatten) const -> Semantic<R>
    {
        return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::vector<E> container, function::Timestamp index) -> void {
            if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::vector<E>, function::Timestamp>)
            {
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
                });
            }
            else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::vector<E>>)
            {
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
                });
            }
            else
            {
                static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, std::vector<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, std::vector<E>>, "flatMap: Flatten must return a Semantic type");
            } }, [&stop](std::vector<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
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
                accept(elements, count);
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
    auto map(Mapper &&mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<std::list<E>>()))>
    {
        using R = decltype(std::declval<Mapper>()(std::declval<std::list<E>>()));
        return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            R mapped;
            generator([&accept, &mapper, &mapped](std::list<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<R, Mapper, std::list<E>, function::Timestamp>)
                {
                    mapped = std::invoke(mapper, container, index);
                }
                else if constexpr (std::is_invocable_r_v<R, Mapper, std::list<E>>)
                {
                    mapped = std::invoke(mapper, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<R, Mapper, std::list<E>, function::Timestamp> || std::is_invocable_r_v<R, Mapper, std::list<E>>, "Mapper must be callable as either (list<E>, Timestamp) -> R or (list<E>) -> R");
                }
                accept(mapped, index); }, [&interrupt, &mapped](std::list<E> container, function::Timestamp index) -> bool { return interrupt(mapped, index); });
        },
                           this->concurrent);
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::list<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::list<E>, function::Timestamp>)
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::list<E>>)
                    matches = std::invoke(predicate, container);
                else
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::list<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::list<E>>, "Predicate must be callable as either (list<E>, Timestamp) -> bool or (list<E>) -> bool");
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
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::list<E>>)
                    matches = std::invoke(predicate, container);
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
                        matches = std::invoke(predicate, container, index);
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::list<E>>)
                        matches = std::invoke(predicate, container);
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
        return collectable::OrderedCollectable<std::list<E>>(this->source(), this->concurrent);
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
                    std::invoke(consumer, container, index);
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::list<E>>)
                    std::invoke(consumer, container);
                accept(container, index);
            },
                      interrupt);
        },
                                      this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::list<E>>
    {
        return Semantic<std::list<E>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::list<E>, function::Timestamp> accept, function::BiPredicate<std::list<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::list<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<std::list<E>>, Flatten, std::list<E>, function::Timestamp>)
                {
                    Semantic<std::list<E>> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](std::list<E> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::list<E> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<std::list<E>>, Flatten, std::list<E>>)
                {
                    Semantic<std::list<E>> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](std::list<E> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::list<E> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<std::list<E>>, Flatten, std::list<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::list<E>>, Flatten, std::list<E>>, "flat: Flatten must return Semantic<list<E>>");
                } }, [&stop](std::list<E> container, function::Timestamp index) -> bool { return stop; });
        },
                                      this->concurrent);
    }

    template <typename Flatten,
              typename InnerSemantic = std::invoke_result_t<Flatten, std::list<E>>,
              typename R = typename InnerSemantic::Element>
    auto flatMap(Flatten &&flatten) const -> Semantic<R>
    {
        return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::list<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::list<E>, function::Timestamp>)
                {
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
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::list<E>>)
                {
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
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, std::list<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, std::list<E>>, "flatMap: Flatten must return a Semantic type");
                } }, [&stop](std::list<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
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
                accept(elements, count);
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
    auto map(Mapper &&mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<std::array<E, N>>()))>
    {
        using R = decltype(std::declval<Mapper>()(std::declval<std::array<E, N>>()));
        return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            R mapped;
            generator([&accept, &mapper, &mapped](std::array<E, N> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<R, Mapper, std::array<E, N>, function::Timestamp>)
                {
                    mapped = std::invoke(mapper, container, index);
                }
                else if constexpr (std::is_invocable_r_v<R, Mapper, std::array<E, N>>)
                {
                    mapped = std::invoke(mapper, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<R, Mapper, std::array<E, N>, function::Timestamp> || std::is_invocable_r_v<R, Mapper, std::array<E, N>>, "Mapper must be callable as either (array<E,N>, Timestamp) -> R or (array<E,N>) -> R");
                }
                accept(mapped, index); }, [&interrupt, &mapped](std::array<E, N> container, function::Timestamp index) -> bool { return interrupt(mapped, index); });
        },
                           this->concurrent);
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::array<E, N> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::array<E, N>, function::Timestamp>)
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::array<E, N>>)
                    matches = std::invoke(predicate, container);
                else
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::array<E, N>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::array<E, N>>, "Predicate must be callable as either (array<E,N>, Timestamp) -> bool or (array<E,N>) -> bool");
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
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::array<E, N>>)
                    matches = std::invoke(predicate, container);
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
                        matches = std::invoke(predicate, container, index);
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::array<E, N>>)
                        matches = std::invoke(predicate, container);
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
        return collectable::OrderedCollectable<std::array<E, N>>(this->source(), this->concurrent);
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
                    std::invoke(consumer, container, index);
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::array<E, N>>)
                    std::invoke(consumer, container);
                accept(container, index);
            },
                      interrupt);
        },
                                          this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::array<E, N>>
    {
        return Semantic<std::array<E, N>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::array<E, N>, function::Timestamp> accept, function::BiPredicate<std::array<E, N>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::array<E, N> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<std::array<E, N>>, Flatten, std::array<E, N>, function::Timestamp>)
                {
                    Semantic<std::array<E, N>> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](std::array<E, N> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::array<E, N> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<std::array<E, N>>, Flatten, std::array<E, N>>)
                {
                    Semantic<std::array<E, N>> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](std::array<E, N> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::array<E, N> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<std::array<E, N>>, Flatten, std::array<E, N>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::array<E, N>>, Flatten, std::array<E, N>>, "flat: Flatten must return Semantic<array<E,N>>");
                } }, [&stop](std::array<E, N> container, function::Timestamp index) -> bool { return stop; });
        },
                                          this->concurrent);
    }

    template <typename Flatten,
              typename InnerSemantic = std::invoke_result_t<Flatten, std::array<E, N>>,
              typename R = typename InnerSemantic::Element>
    auto flatMap(Flatten &&flatten) const -> Semantic<R>
    {
        return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::array<E, N> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::array<E, N>, function::Timestamp>)
                {
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
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::array<E, N>>)
                {
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
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, std::array<E, N>, function::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, std::array<E, N>>, "flatMap: Flatten must return a Semantic type");
                } }, [&stop](std::array<E, N> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
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
                accept(elements, count);
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
    auto map(Mapper &&mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<std::deque<E>>()))>
    {
        using R = decltype(std::declval<Mapper>()(std::declval<std::deque<E>>()));
        return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            R mapped;
            generator([&accept, &mapper, &mapped](std::deque<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<R, Mapper, std::deque<E>, function::Timestamp>)
                {
                    mapped = std::invoke(mapper, container, index);
                }
                else if constexpr (std::is_invocable_r_v<R, Mapper, std::deque<E>>)
                {
                    mapped = std::invoke(mapper, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<R, Mapper, std::deque<E>, function::Timestamp> || std::is_invocable_r_v<R, Mapper, std::deque<E>>, "Mapper must be callable as either (deque<E>, Timestamp) -> R or (deque<E>) -> R");
                }
                accept(mapped, index); }, [&interrupt, &mapped](std::deque<E> container, function::Timestamp index) -> bool { return interrupt(mapped, index); });
        },
                           this->concurrent);
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::deque<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::deque<E>, function::Timestamp>)
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::deque<E>>)
                    matches = std::invoke(predicate, container);
                else
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::deque<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::deque<E>>, "Predicate must be callable as either (deque<E>, Timestamp) -> bool or (deque<E>) -> bool");
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
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::deque<E>>)
                    matches = std::invoke(predicate, container);
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
                        matches = std::invoke(predicate, container, index);
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::deque<E>>)
                        matches = std::invoke(predicate, container);
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
        return collectable::OrderedCollectable<std::deque<E>>(this->source(), this->concurrent);
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
                    std::invoke(consumer, container, index);
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::deque<E>>)
                    std::invoke(consumer, container);
                accept(container, index);
            },
                      interrupt);
        },
                                       this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::deque<E>>
    {
        return Semantic<std::deque<E>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::deque<E>, function::Timestamp> accept, function::BiPredicate<std::deque<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::deque<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<std::deque<E>>, Flatten, std::deque<E>, function::Timestamp>)
                {
                    Semantic<std::deque<E>> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](std::deque<E> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::deque<E> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<std::deque<E>>, Flatten, std::deque<E>>)
                {
                    Semantic<std::deque<E>> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](std::deque<E> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::deque<E> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<std::deque<E>>, Flatten, std::deque<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::deque<E>>, Flatten, std::deque<E>>, "flat: Flatten must return Semantic<deque<E>>");
                } }, [&stop](std::deque<E> container, function::Timestamp index) -> bool { return stop; });
        },
                                       this->concurrent);
    }

    template <typename Flatten,
              typename InnerSemantic = std::invoke_result_t<Flatten, std::deque<E>>,
              typename R = typename InnerSemantic::Element>
    auto flatMap(Flatten &&flatten) const -> Semantic<R>
    {
        return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::deque<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::deque<E>, function::Timestamp>)
                {
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
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::deque<E>>)
                {
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
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, std::deque<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, std::deque<E>>, "flatMap: Flatten must return a Semantic type");
                } }, [&stop](std::deque<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
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
                accept(elements, count);
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
    auto map(Mapper &&mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<std::queue<E>>()))>
    {
        using R = decltype(std::declval<Mapper>()(std::declval<std::queue<E>>()));
        return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            R mapped;
            generator([&accept, &mapper, &mapped](std::queue<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<R, Mapper, std::queue<E>, function::Timestamp>)
                {
                    mapped = std::invoke(mapper, container, index);
                }
                else if constexpr (std::is_invocable_r_v<R, Mapper, std::queue<E>>)
                {
                    mapped = std::invoke(mapper, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<R, Mapper, std::queue<E>, function::Timestamp> || std::is_invocable_r_v<R, Mapper, std::queue<E>>, "Mapper must be callable as either (queue<E>, Timestamp) -> R or (queue<E>) -> R");
                }
                accept(mapped, index); }, [&interrupt, &mapped](std::queue<E> container, function::Timestamp index) -> bool { return interrupt(mapped, index); });
        },
                           this->concurrent);
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::queue<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::queue<E>, function::Timestamp>)
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::queue<E>>)
                    matches = std::invoke(predicate, container);
                else
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::queue<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::queue<E>>, "Predicate must be callable as either (queue<E>, Timestamp) -> bool or (queue<E>) -> bool");
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
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::queue<E>>)
                    matches = std::invoke(predicate, container);
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
                        matches = std::invoke(predicate, container, index);
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::queue<E>>)
                        matches = std::invoke(predicate, container);
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
        return collectable::OrderedCollectable<std::queue<E>>(this->source(), this->concurrent);
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
                    std::invoke(consumer, container, index);
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::queue<E>>)
                    std::invoke(consumer, container);
                accept(container, index);
            },
                      interrupt);
        },
                                       this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::queue<E>>
    {
        return Semantic<std::queue<E>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::queue<E>, function::Timestamp> accept, function::BiPredicate<std::queue<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::queue<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<std::queue<E>>, Flatten, std::queue<E>, function::Timestamp>)
                {
                    Semantic<std::queue<E>> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](std::queue<E> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::queue<E> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<std::queue<E>>, Flatten, std::queue<E>>)
                {
                    Semantic<std::queue<E>> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](std::queue<E> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::queue<E> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<std::queue<E>>, Flatten, std::queue<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::queue<E>>, Flatten, std::queue<E>>, "flat: Flatten must return Semantic<queue<E>>");
                } }, [&stop](std::queue<E> container, function::Timestamp index) -> bool { return stop; });
        },
                                       this->concurrent);
    }

    template <typename Flatten,
              typename InnerSemantic = std::invoke_result_t<Flatten, std::queue<E>>,
              typename R = typename InnerSemantic::Element>
    auto flatMap(Flatten &&flatten) const -> Semantic<R>
    {
        return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::queue<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::queue<E>, function::Timestamp>)
                {
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
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::queue<E>>)
                {
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
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, std::queue<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, std::queue<E>>, "flatMap: Flatten must return a Semantic type");
                } }, [&stop](std::queue<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
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
                accept(elements, count);
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
    auto map(Mapper &&mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<std::stack<E>>()))>
    {
        using R = decltype(std::declval<Mapper>()(std::declval<std::stack<E>>()));
        return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            R mapped;
            generator([&accept, &mapper, &mapped](std::stack<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<R, Mapper, std::stack<E>, function::Timestamp>)
                {
                    mapped = std::invoke(mapper, container, index);
                }
                else if constexpr (std::is_invocable_r_v<R, Mapper, std::stack<E>>)
                {
                    mapped = std::invoke(mapper, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<R, Mapper, std::stack<E>, function::Timestamp> || std::is_invocable_r_v<R, Mapper, std::stack<E>>, "Mapper must be callable as either (stack<E>, Timestamp) -> R or (stack<E>) -> R");
                }
                accept(mapped, index); }, [&interrupt, &mapped](std::stack<E> container, function::Timestamp index) -> bool { return interrupt(mapped, index); });
        },
                           this->concurrent);
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::stack<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::stack<E>, function::Timestamp>)
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::stack<E>>)
                    matches = std::invoke(predicate, container);
                else
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::stack<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::stack<E>>, "Predicate must be callable as either (stack<E>, Timestamp) -> bool or (stack<E>) -> bool");
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
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::stack<E>>)
                    matches = std::invoke(predicate, container);
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
                        matches = std::invoke(predicate, container, index);
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::stack<E>>)
                        matches = std::invoke(predicate, container);
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
        return collectable::OrderedCollectable<std::stack<E>>(this->source(), this->concurrent);
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
                    std::invoke(consumer, container, index);
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::stack<E>>)
                    std::invoke(consumer, container);
                accept(container, index);
            },
                      interrupt);
        },
                                       this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::stack<E>>
    {
        return Semantic<std::stack<E>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::stack<E>, function::Timestamp> accept, function::BiPredicate<std::stack<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::stack<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<std::stack<E>>, Flatten, std::stack<E>, function::Timestamp>)
                {
                    Semantic<std::stack<E>> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](std::stack<E> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::stack<E> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<std::stack<E>>, Flatten, std::stack<E>>)
                {
                    Semantic<std::stack<E>> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](std::stack<E> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::stack<E> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<std::stack<E>>, Flatten, std::stack<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::stack<E>>, Flatten, std::stack<E>>, "flat: Flatten must return Semantic<stack<E>>");
                } }, [&stop](std::stack<E> container, function::Timestamp index) -> bool { return stop; });
        },
                                       this->concurrent);
    }

    template <typename Flatten,
              typename InnerSemantic = std::invoke_result_t<Flatten, std::stack<E>>,
              typename R = typename InnerSemantic::Element>
    auto flatMap(Flatten &&flatten) const -> Semantic<R>
    {
        return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::stack<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::stack<E>, function::Timestamp>)
                {
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
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::stack<E>>)
                {
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
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, std::stack<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, std::stack<E>>, "flatMap: Flatten must return a Semantic type");
                } }, [&stop](std::stack<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
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
                accept(elements, count);
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
    auto map(Mapper &&mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<std::set<E>>()))>
    {
        using R = decltype(std::declval<Mapper>()(std::declval<std::set<E>>()));
        return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            R mapped;
            generator([&accept, &mapper, &mapped](std::set<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<R, Mapper, std::set<E>, function::Timestamp>)
                {
                    mapped = std::invoke(mapper, container, index);
                }
                else if constexpr (std::is_invocable_r_v<R, Mapper, std::set<E>>)
                {
                    mapped = std::invoke(mapper, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<R, Mapper, std::set<E>, function::Timestamp> || std::is_invocable_r_v<R, Mapper, std::set<E>>, "Mapper must be callable as either (set<E>, Timestamp) -> R or (set<E>) -> R");
                }
                accept(mapped, index); }, [&interrupt, &mapped](std::set<E> container, function::Timestamp index) -> bool { return interrupt(mapped, index); });
        },
                           this->concurrent);
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::set<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::set<E>, function::Timestamp>)
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::set<E>>)
                    matches = std::invoke(predicate, container);
                else
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::set<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::set<E>>, "Predicate must be callable as either (set<E>, Timestamp) -> bool or (set<E>) -> bool");
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
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::set<E>>)
                    matches = std::invoke(predicate, container);
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
                        matches = std::invoke(predicate, container, index);
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::set<E>>)
                        matches = std::invoke(predicate, container);
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
        return collectable::OrderedCollectable<std::set<E>>(this->source(), this->concurrent);
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
                    std::invoke(consumer, container, index);
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::set<E>>)
                    std::invoke(consumer, container);
                accept(container, index);
            },
                      interrupt);
        },
                                     this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::set<E>>
    {
        return Semantic<std::set<E>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::set<E>, function::Timestamp> accept, function::BiPredicate<std::set<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::set<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<std::set<E>>, Flatten, std::set<E>, function::Timestamp>)
                {
                    Semantic<std::set<E>> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](std::set<E> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::set<E> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<std::set<E>>, Flatten, std::set<E>>)
                {
                    Semantic<std::set<E>> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](std::set<E> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::set<E> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<std::set<E>>, Flatten, std::set<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::set<E>>, Flatten, std::set<E>>, "flat: Flatten must return Semantic<set<E>>");
                } }, [&stop](std::set<E> container, function::Timestamp index) -> bool { return stop; });
        },
                                     this->concurrent);
    }

    template <typename Flatten,
              typename InnerSemantic = std::invoke_result_t<Flatten, std::set<E>>,
              typename R = typename InnerSemantic::Element>
    auto flatMap(Flatten &&flatten) const -> Semantic<R>
    {
        return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::set<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::set<E>, function::Timestamp>)
                {
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
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::set<E>>)
                {
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
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, std::set<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, std::set<E>>, "flatMap: Flatten must return a Semantic type");
                } }, [&stop](std::set<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
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
                accept(elements, count);
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
    auto map(Mapper &&mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<std::unordered_set<E>>()))>
    {
        using R = decltype(std::declval<Mapper>()(std::declval<std::unordered_set<E>>()));
        return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            R mapped;
            generator([&accept, &mapper, &mapped](std::unordered_set<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<R, Mapper, std::unordered_set<E>, function::Timestamp>)
                {
                    mapped = std::invoke(mapper, container, index);
                }
                else if constexpr (std::is_invocable_r_v<R, Mapper, std::unordered_set<E>>)
                {
                    mapped = std::invoke(mapper, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<R, Mapper, std::unordered_set<E>, function::Timestamp> || std::is_invocable_r_v<R, Mapper, std::unordered_set<E>>, "Mapper must be callable as either (unordered_set<E>, Timestamp) -> R or (unordered_set<E>) -> R");
                }
                accept(mapped, index); }, [&interrupt, &mapped](std::unordered_set<E> container, function::Timestamp index) -> bool { return interrupt(mapped, index); });
        },
                           this->concurrent);
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::unordered_set<E> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>, function::Timestamp>)
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>>)
                    matches = std::invoke(predicate, container);
                else
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>>, "Predicate must be callable as either (unordered_set<E>, Timestamp) -> bool or (unordered_set<E>) -> bool");
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
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>>)
                    matches = std::invoke(predicate, container);
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
                        matches = std::invoke(predicate, container, index);
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_set<E>>)
                        matches = std::invoke(predicate, container);
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
        return collectable::OrderedCollectable<std::unordered_set<E>>(this->source(), this->concurrent);
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
                    std::invoke(consumer, container, index);
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::unordered_set<E>>)
                    std::invoke(consumer, container);
                accept(container, index);
            },
                      interrupt);
        },
                                               this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::unordered_set<E>>
    {
        return Semantic<std::unordered_set<E>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::unordered_set<E>, function::Timestamp> accept, function::BiPredicate<std::unordered_set<E>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_set<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<std::unordered_set<E>>, Flatten, std::unordered_set<E>, function::Timestamp>)
                {
                    Semantic<std::unordered_set<E>> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](std::unordered_set<E> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::unordered_set<E> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<std::unordered_set<E>>, Flatten, std::unordered_set<E>>)
                {
                    Semantic<std::unordered_set<E>> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](std::unordered_set<E> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::unordered_set<E> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<std::unordered_set<E>>, Flatten, std::unordered_set<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::unordered_set<E>>, Flatten, std::unordered_set<E>>, "flat: Flatten must return Semantic<unordered_set<E>>");
                } }, [&stop](std::unordered_set<E> container, function::Timestamp index) -> bool { return stop; });
        },
                                               this->concurrent);
    }

    template <typename Flatten,
              typename InnerSemantic = std::invoke_result_t<Flatten, std::unordered_set<E>>,
              typename R = typename InnerSemantic::Element>
    auto flatMap(Flatten &&flatten) const -> Semantic<R>
    {
        return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_set<E> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::unordered_set<E>, function::Timestamp>)
                {
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
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::unordered_set<E>>)
                {
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
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, std::unordered_set<E>, function::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, std::unordered_set<E>>, "flatMap: Flatten must return a Semantic type");
                } }, [&stop](std::unordered_set<E> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
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
                accept(elements, count);
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
    auto map(Mapper &&mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<std::map<K, V>>()))>
    {
        using R = decltype(std::declval<Mapper>()(std::declval<std::map<K, V>>()));
        return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            R mapped;
            generator([&accept, &mapper, &mapped](std::map<K, V> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<R, Mapper, std::map<K, V>, function::Timestamp>)
                {
                    mapped = std::invoke(mapper, container, index);
                }
                else if constexpr (std::is_invocable_r_v<R, Mapper, std::map<K, V>>)
                {
                    mapped = std::invoke(mapper, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<R, Mapper, std::map<K, V>, function::Timestamp> || std::is_invocable_r_v<R, Mapper, std::map<K, V>>, "Mapper must be callable as either (map<K,V>, Timestamp) -> R or (map<K,V>) -> R");
                }
                accept(mapped, index); }, [&interrupt, &mapped](std::map<K, V> container, function::Timestamp index) -> bool { return interrupt(mapped, index); });
        },
                           this->concurrent);
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::map<K, V> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::map<K, V>, function::Timestamp>)
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::map<K, V>>)
                    matches = std::invoke(predicate, container);
                else
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::map<K, V>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::map<K, V>>, "Predicate must be callable as either (map<K,V>, Timestamp) -> bool or (map<K,V>) -> bool");
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
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::map<K, V>>)
                    matches = std::invoke(predicate, container);
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
                        matches = std::invoke(predicate, container, index);
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::map<K, V>>)
                        matches = std::invoke(predicate, container);
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
        return collectable::OrderedCollectable<std::map<K, V>>(this->source(), this->concurrent);
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
                    std::invoke(consumer, container, index);
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::map<K, V>>)
                    std::invoke(consumer, container);
                accept(container, index);
            },
                      interrupt);
        },
                                        this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::map<K, V>>
    {
        return Semantic<std::map<K, V>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::map<K, V>, function::Timestamp> accept, function::BiPredicate<std::map<K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::map<K, V> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<std::map<K, V>>, Flatten, std::map<K, V>, function::Timestamp>)
                {
                    Semantic<std::map<K, V>> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](std::map<K, V> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::map<K, V> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<std::map<K, V>>, Flatten, std::map<K, V>>)
                {
                    Semantic<std::map<K, V>> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](std::map<K, V> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::map<K, V> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<std::map<K, V>>, Flatten, std::map<K, V>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::map<K, V>>, Flatten, std::map<K, V>>, "flat: Flatten must return Semantic<map<K,V>>");
                } }, [&stop](std::map<K, V> container, function::Timestamp index) -> bool { return stop; });
        },
                                        this->concurrent);
    }

    template <typename Flatten,
              typename InnerSemantic = std::invoke_result_t<Flatten, std::map<K, V>>,
              typename R = typename InnerSemantic::Element>
    auto flatMap(Flatten &&flatten) const -> Semantic<R>
    {
        return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::map<K, V> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::map<K, V>, function::Timestamp>)
                {
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
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::map<K, V>>)
                {
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
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, std::map<K, V>, function::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, std::map<K, V>>, "flatMap: Flatten must return a Semantic type");
                } }, [&stop](std::map<K, V> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
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
                accept(elements, count);
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
    auto map(Mapper &&mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<std::unordered_map<K, V>>()))>
    {
        using R = decltype(std::declval<Mapper>()(std::declval<std::unordered_map<K, V>>()));
        return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            R mapped;
            generator([&accept, &mapper, &mapped](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<R, Mapper, std::unordered_map<K, V>, function::Timestamp>)
                {
                    mapped = std::invoke(mapper, container, index);
                }
                else if constexpr (std::is_invocable_r_v<R, Mapper, std::unordered_map<K, V>>)
                {
                    mapped = std::invoke(mapper, container);
                }
                else
                {
                    static_assert(std::is_invocable_r_v<R, Mapper, std::unordered_map<K, V>, function::Timestamp> || std::is_invocable_r_v<R, Mapper, std::unordered_map<K, V>>, "Mapper must be callable as either (unordered_map<K,V>, Timestamp) -> R or (unordered_map<K,V>) -> R");
                }
                accept(mapped, index); }, [&interrupt, &mapped](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return interrupt(mapped, index); });
        },
                           this->concurrent);
    }

    template <typename Predicate>
    auto filter(Predicate &&predicate) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), predicate = std::forward<Predicate>(predicate)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            generator([&accept, &count, &predicate](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                bool matches = false;
                if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>, function::Timestamp>)
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>>)
                    matches = std::invoke(predicate, container);
                else
                    static_assert(std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>, function::Timestamp> || std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>>, "Predicate must be callable as either (unordered_map<K,V>, Timestamp) -> bool or (unordered_map<K,V>) -> bool");
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
                    matches = std::invoke(predicate, container, index);
                else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>>)
                    matches = std::invoke(predicate, container);
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
                        matches = std::invoke(predicate, container, index);
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, std::unordered_map<K, V>>)
                        matches = std::invoke(predicate, container);
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
        return collectable::OrderedCollectable<std::unordered_map<K, V>>(this->source(), this->concurrent);
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
                    std::invoke(consumer, container, index);
                else if constexpr (std::is_invocable_r_v<void, Consumer, std::unordered_map<K, V>>)
                    std::invoke(consumer, container);
                accept(container, index);
            },
                      interrupt);
        },
                                                  this->concurrent);
    }

    template <typename Flatten>
    auto flat(Flatten &&flatten) const -> Semantic<std::unordered_map<K, V>>
    {
        return Semantic<std::unordered_map<K, V>>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<std::unordered_map<K, V>, function::Timestamp> accept, function::BiPredicate<std::unordered_map<K, V>, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<std::unordered_map<K, V>>, Flatten, std::unordered_map<K, V>, function::Timestamp>)
                {
                    Semantic<std::unordered_map<K, V>> inner = std::invoke(flatten, container, index);
                    inner.source()([&accept, &count](std::unordered_map<K, V> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::unordered_map<K, V> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<std::unordered_map<K, V>>, Flatten, std::unordered_map<K, V>>)
                {
                    Semantic<std::unordered_map<K, V>> inner = std::invoke(flatten, container);
                    inner.source()([&accept, &count](std::unordered_map<K, V> innerContainer, function::Timestamp innerIndex) -> void {
                        accept(innerContainer, count);
                        count++;
                    }, [&interrupt, &stop, &count](std::unordered_map<K, V> innerContainer, function::Timestamp innerIndex) -> bool {
                        if (interrupt(innerContainer, count))
                        {
                            stop = true;
                            return true;
                        }
                        return false;
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<std::unordered_map<K, V>>, Flatten, std::unordered_map<K, V>, function::Timestamp> || std::is_invocable_r_v<Semantic<std::unordered_map<K, V>>, Flatten, std::unordered_map<K, V>>, "flat: Flatten must return Semantic<unordered_map<K,V>>");
                } }, [&stop](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return stop; });
        },
                                                  this->concurrent);
    }

    template <typename Flatten,
              typename InnerSemantic = std::invoke_result_t<Flatten, std::unordered_map<K, V>>,
              typename R = typename InnerSemantic::Element>
    auto flatMap(Flatten &&flatten) const -> Semantic<R>
    {
        return Semantic<R>([generator = *(this->generator), flatten = std::forward<Flatten>(flatten)](function::BiConsumer<R, function::Timestamp> accept, function::BiPredicate<R, function::Timestamp> interrupt) -> void {
            function::Timestamp count = 0LL;
            bool stop = false;
            generator([&accept, &count, &stop, &flatten, &interrupt](std::unordered_map<K, V> container, function::Timestamp index) -> void {
                if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::unordered_map<K, V>, function::Timestamp>)
                {
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
                    });
                }
                else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, std::unordered_map<K, V>>)
                {
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
                    });
                }
                else
                {
                    static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, std::unordered_map<K, V>, function::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, std::unordered_map<K, V>>, "flatMap: Flatten must return a Semantic type");
                } }, [&stop](std::unordered_map<K, V> container, function::Timestamp index) -> bool { return stop; });
        },
                           this->concurrent);
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
                accept(elements, count);
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
                    window.push_back(buffer.at(i));
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