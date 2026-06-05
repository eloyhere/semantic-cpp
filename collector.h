#pragma once
#include "function.h"
#include "pool.h"
#include "charsequence.h"
#include <memory>
#include <vector>
#include <future>
#include <optional>
#include <string>
#include <set>
#include <list>
#include <deque>
#include <array>
#include <forward_list>
#include <stack>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <atomic>
#include <type_traits>

namespace collector
{
const double pi = std::acos(-1);

template <typename A>
using Identity = function::Supplier<A>;

template <typename E, typename A>
using Interrupt = function::TriPredicate<E, function::Timestamp, A>;

template <typename A, typename E>
using Accumulator = function::TriFunction<A, E, function::Timestamp, A>;

template <typename A>
using Combiner = function::BiFunction<A, A, A>;

template <typename A, typename R>
using Finisher = function::Function<A, R>;

inline pool::ThreadPool &globalPool()
{
    static pool::ThreadPool instance;
    return instance;
}

template <typename E, typename A, typename R>
class Collector
{
  private:
    std::unique_ptr<Identity<A>> identity;
    std::unique_ptr<Interrupt<E, A>> interrupt;
    std::unique_ptr<Accumulator<A, E>> accumulator;
    std::unique_ptr<Combiner<A>> combiner;
    std::unique_ptr<Finisher<A, R>> finisher;

    template <typename Container>
    auto group(const Container &container, const function::Module &concurrent) const -> std::vector<std::future<A>>
    {
        std::atomic<bool> hasError{false};
        std::vector<std::future<A>> futures;
        futures.reserve(concurrent);

        for (function::Module thread = 0; thread < concurrent; ++thread)
        {
            futures.emplace_back(globalPool().submit<A>([this, &container, thread, concurrent, &hasError]() -> A {
                A identityValue = (*identity)();
                function::Module index = 0;
                for (const E &element : container)
                {
                    if (hasError.load())
                    {
                        break;
                    }
                    if ((*interrupt)(element, index, identityValue))
                    {
                        break;
                    }
                    if (index % concurrent == thread)
                    {
                        identityValue = (*accumulator)(identityValue, element, index);
                    }
                    ++index;
                }
                return identityValue;
            }));
        }

        return futures;
    }

    auto group(const function::Generator<E> &generator, const function::Module &concurrent) const -> std::vector<std::future<A>>
    {
        std::atomic<bool> hasError{false};
        std::vector<std::future<A>> futures;
        futures.reserve(concurrent);

        for (function::Module thread = 0; thread < concurrent; ++thread)
        {
            futures.emplace_back(globalPool().submit<A>([this, thread, &generator, concurrent, &hasError]() -> A {
                A identityValue = (*identity)();
                generator(
                    [thread, &identityValue, concurrent, &hasError, this](E element, function::Timestamp index) -> void {
                        if (!hasError.load() && index % concurrent == thread)
                        {
                            identityValue = (*accumulator)(identityValue, element, index);
                        }
                    },
                    [&identityValue, &hasError, this](E element, function::Timestamp index) -> bool {
                        return hasError.load() || (*interrupt)(element, index, identityValue);
                    });
                return identityValue;
            }));
        }

        return futures;
    }

    auto concatenate(std::vector<std::future<A>> &futures) const -> A
    {
        std::exception_ptr firstException;
        std::mutex exceptionMutex;
        std::atomic<bool> hasError{false};

        A result = (*identity)();
        for (auto &future : futures)
        {
            try
            {
                result = (*combiner)(std::move(result), future.get());
            }
            catch (...)
            {
                hasError.store(true);
                std::lock_guard<std::mutex> lock(exceptionMutex);
                if (!firstException)
                {
                    firstException = std::current_exception();
                }
            }
        }

        if (firstException)
        {
            std::rethrow_exception(firstException);
        }

        return result;
    }

  public:
    Collector(const Identity<A> &identity, const Interrupt<E, A> &interrupt, const Accumulator<A, E> &accumulator, const Combiner<A> &combiner, const Finisher<A, R> &finisher)
        : identity(std::make_unique<Identity<A>>(identity)), interrupt(std::make_unique<Interrupt<E, A>>(interrupt)), accumulator(std::make_unique<Accumulator<A, E>>(accumulator)), combiner(std::make_unique<Combiner<A>>(combiner)), finisher(std::make_unique<Finisher<A, R>>(finisher))
    {
    }

    Collector(Identity<A> &&identity, Interrupt<E, A> &&interrupt, Accumulator<A, E> &&accumulator, Combiner<A> &&combiner, Finisher<A, R> &&finisher)
        : identity(std::make_unique<Identity<A>>(std::move(identity))), interrupt(std::make_unique<Interrupt<E, A>>(std::move(interrupt))), accumulator(std::make_unique<Accumulator<A, E>>(std::move(accumulator))), combiner(std::make_unique<Combiner<A>>(std::move(combiner))), finisher(std::make_unique<Finisher<A, R>>(std::move(finisher)))
    {
    }

    Collector(Collector<E, A, R> &&other) noexcept
        : identity(std::move(other.identity)), interrupt(std::move(other.interrupt)), accumulator(std::move(other.accumulator)), combiner(std::move(other.combiner)), finisher(std::move(other.finisher))
    {
    }

    Collector<E, A, R> &operator=(Collector<E, A, R> &&other) noexcept
    {
        if (this != &other)
        {
            identity = std::move(other.identity);
            interrupt = std::move(other.interrupt);
            accumulator = std::move(other.accumulator);
            combiner = std::move(other.combiner);
            finisher = std::move(other.finisher);
        }
        return *this;
    }

    ~Collector() = default;

    auto collect(const function::Generator<E> &generator, const function::Module &concurrent) const -> R
    {
        if (concurrent < 2)
        {
            A identityValue = (*identity)();
            generator(
                [&identityValue, this](E element, function::Timestamp index) -> void {
                    identityValue = (*accumulator)(identityValue, element, index);
                },
                [&identityValue, this](E element, function::Timestamp index) -> bool {
                    return (*interrupt)(element, index, identityValue);
                });
            return (*finisher)(identityValue);
        }

        auto futures = group(generator, concurrent);
        A result = concatenate(futures);
        return (*finisher)(result);
    }

    template <typename Container>
    auto collect(const Container &container, const function::Module &concurrent) const -> R
    {
        if (concurrent < 2)
        {
            A identityValue = (*identity)();
            function::Timestamp index = 0;
            for (const E &element : container)
            {
                if ((*interrupt)(element, index, identityValue))
                {
                    break;
                }
                identityValue = (*accumulator)(identityValue, element, index);
                ++index;
            }
            return (*finisher)(identityValue);
        }

        auto futures = group(container, concurrent);
        A result = concatenate(futures);
        return (*finisher)(result);
    }

    auto collect(const std::initializer_list<E> &container, const function::Module &concurrent) const -> R
    {
        if (concurrent < 2)
        {
            A identityValue = (*identity)();
            function::Timestamp index = 0;
            for (const E &element : container)
            {
                if ((*interrupt)(element, index, identityValue))
                {
                    break;
                }
                identityValue = (*accumulator)(identityValue, element, index);
                ++index;
            }
            return (*finisher)(identityValue);
        }

        auto futures = group(container, concurrent);
        A result = concatenate(futures);
        return (*finisher)(result);
    }

    template <typename T, std::size_t N>
    auto collect(const std::array<T, N> &container, const function::Module &concurrent) const -> R
    {
        if (concurrent < 2)
        {
            A identityValue = (*identity)();
            function::Timestamp index = 0;
            for (const auto &element : container)
            {
                if ((*interrupt)(element, index, identityValue))
                {
                    break;
                }
                identityValue = (*accumulator)(identityValue, element, index);
                ++index;
            }
            return (*finisher)(identityValue);
        }

        auto futures = group(container, concurrent);
        A result = concatenate(futures);
        return (*finisher)(result);
    }

    auto collect(const std::forward_list<E> &container, const function::Module &concurrent) const -> R
    {
        if (concurrent < 2)
        {
            A identityValue = (*identity)();
            function::Timestamp index = 0;
            for (const E &element : container)
            {
                if ((*interrupt)(element, index, identityValue))
                {
                    break;
                }
                identityValue = (*accumulator)(identityValue, element, index);
                ++index;
            }
            return (*finisher)(identityValue);
        }

        auto futures = group(container, concurrent);
        A result = concatenate(futures);
        return (*finisher)(result);
    }

    auto collect(const std::deque<E> &container, const function::Module &concurrent) const -> R
    {
        if (concurrent < 2)
        {
            A identityValue = (*identity)();
            function::Timestamp index = 0;
            for (const E &element : container)
            {
                if ((*interrupt)(element, index, identityValue))
                {
                    break;
                }
                identityValue = (*accumulator)(identityValue, element, index);
                ++index;
            }
            return (*finisher)(identityValue);
        }

        auto futures = group(container, concurrent);
        A result = concatenate(futures);
        return (*finisher)(result);
    }

    auto collect(std::stack<E> container, const function::Module &concurrent) const -> R
    {
        std::vector<E> temp;
        while (!container.empty())
        {
            temp.push_back(container.top());
            container.pop();
        }

        if (concurrent < 2)
        {
            A identityValue = (*identity)();
            function::Timestamp index = 0;
            for (const E &element : temp)
            {
                if ((*interrupt)(element, index, identityValue))
                {
                    break;
                }
                identityValue = (*accumulator)(identityValue, element, index);
                ++index;
            }
            return (*finisher)(identityValue);
        }

        auto futures = group(temp, concurrent);
        A result = concatenate(futures);
        return (*finisher)(result);
    }

    auto collect(std::queue<E> container, const function::Module &concurrent) const -> R
    {
        std::vector<E> temp;
        while (!container.empty())
        {
            temp.push_back(container.front());
            container.pop();
        }

        if (concurrent < 2)
        {
            A identityValue = (*identity)();
            function::Timestamp index = 0;
            for (const E &element : temp)
            {
                if ((*interrupt)(element, index, identityValue))
                {
                    break;
                }
                identityValue = (*accumulator)(identityValue, element, index);
                ++index;
            }
            return (*finisher)(identityValue);
        }

        auto futures = group(temp, concurrent);
        A result = concatenate(futures);
        return (*finisher)(result);
    }
};

template <typename E, typename A, typename R>
auto useFull(const Identity<A> &identity, const Accumulator<A, E> &accumulator, const Combiner<A> &combiner, const Finisher<A, R> &finisher) -> Collector<E, A, R>
{
    Interrupt<E, A> interrupt = [](const E element, const function::Timestamp &index, const A &accumulator) -> bool {
        return false;
    };
    return Collector<E, A, R>(identity, std::move(interrupt), accumulator, combiner, finisher);
}

template <typename E, typename A, typename R>
auto useShortable(const Identity<A> &identity, const Interrupt<E, A> &interrupt, const Accumulator<A, E> &accumulator, const Combiner<A> &combiner, const Finisher<A, R> &finisher) -> Collector<E, A, R>
{
    return Collector<E, A, R>(identity, interrupt, accumulator, combiner, finisher);
}

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
auto useVariance() -> Collector<E, std::tuple<D, D, D, function::Module>, D>
{
    return useFull<E, std::tuple<D, D, D, function::Module>, D>(
        []() -> std::tuple<D, D, D, function::Module> {
            return std::make_tuple(D{}, D{}, D{}, function::Module{0});
        },
        [](std::tuple<D, D, D, function::Module> acc, E element, function::Timestamp index) -> std::tuple<D, D, D, function::Module> {
            D value = static_cast<D>(element);
            function::Module count = std::get<3>(acc) + 1;
            D delta = value - std::get<0>(acc);
            D newMean = std::get<0>(acc) + delta / static_cast<D>(count);
            D delta2 = value - newMean;
            D newM2 = std::get<1>(acc) + delta * delta2;
            return std::make_tuple(newMean, newM2, std::get<2>(acc) + value, count);
        },
        [](std::tuple<D, D, D, function::Module> a, std::tuple<D, D, D, function::Module> b) -> std::tuple<D, D, D, function::Module> {
            function::Module countA = std::get<3>(a);
            function::Module countB = std::get<3>(b);
            if (countA == 0)
                return b;
            if (countB == 0)
                return a;
            D delta = std::get<0>(b) - std::get<0>(a);
            D totalCount = static_cast<D>(countA + countB);
            D newMean = std::get<0>(a) + delta * static_cast<D>(countB) / totalCount;
            D newM2 = std::get<1>(a) + std::get<1>(b) + delta * delta * static_cast<D>(countA) * static_cast<D>(countB) / totalCount;
            return std::make_tuple(newMean, newM2, std::get<2>(a) + std::get<2>(b), countA + countB);
        },
        [](std::tuple<D, D, D, function::Module> acc) -> D {
            function::Module count = std::get<3>(acc);
            if (count == 0)
                return D{};
            return std::get<1>(acc) / static_cast<D>(count);
        });
}

template <typename E, typename D>
auto useVariance(const function::Function<E, D> &mapper) -> Collector<E, std::tuple<D, D, D, function::Module>, D>
{
    return useFull<E, std::tuple<D, D, D, function::Module>, D>(
        []() -> std::tuple<D, D, D, function::Module> {
            return std::make_tuple(D{}, D{}, D{}, function::Module{0});
        },
        [mapper](std::tuple<D, D, D, function::Module> acc, E element, function::Timestamp index) -> std::tuple<D, D, D, function::Module> {
            D value = mapper(element);
            function::Module count = std::get<3>(acc) + 1;
            D delta = value - std::get<0>(acc);
            D newMean = std::get<0>(acc) + delta / static_cast<D>(count);
            D delta2 = value - newMean;
            D newM2 = std::get<1>(acc) + delta * delta2;
            return std::make_tuple(newMean, newM2, std::get<2>(acc) + value, count);
        },
        [](std::tuple<D, D, D, function::Module> a, std::tuple<D, D, D, function::Module> b) -> std::tuple<D, D, D, function::Module> {
            function::Module countA = std::get<3>(a);
            function::Module countB = std::get<3>(b);
            if (countA == 0)
                return b;
            if (countB == 0)
                return a;
            D delta = std::get<0>(b) - std::get<0>(a);
            D totalCount = static_cast<D>(countA + countB);
            D newMean = std::get<0>(a) + delta * static_cast<D>(countB) / totalCount;
            D newM2 = std::get<1>(a) + std::get<1>(b) + delta * delta * static_cast<D>(countA) * static_cast<D>(countB) / totalCount;
            return std::make_tuple(newMean, newM2, std::get<2>(a) + std::get<2>(b), countA + countB);
        },
        [](std::tuple<D, D, D, function::Module> acc) -> D {
            function::Module count = std::get<3>(acc);
            if (count == 0)
                return D{};
            return std::get<1>(acc) / static_cast<D>(count);
        });
}

template <typename E, typename D>
auto useStandardDeviation() -> Collector<E, std::tuple<D, D, D, function::Module>, D>
{
    return useFull<E, std::tuple<D, D, D, function::Module>, D>(
        []() -> std::tuple<D, D, D, function::Module> {
            return std::make_tuple(D{}, D{}, D{}, function::Module{0});
        },
        [](std::tuple<D, D, D, function::Module> acc, E element, function::Timestamp index) -> std::tuple<D, D, D, function::Module> {
            D value = static_cast<D>(element);
            function::Module count = std::get<3>(acc) + 1;
            D delta = value - std::get<0>(acc);
            D newMean = std::get<0>(acc) + delta / static_cast<D>(count);
            D delta2 = value - newMean;
            D newM2 = std::get<1>(acc) + delta * delta2;
            return std::make_tuple(newMean, newM2, std::get<2>(acc) + value, count);
        },
        [](std::tuple<D, D, D, function::Module> a, std::tuple<D, D, D, function::Module> b) -> std::tuple<D, D, D, function::Module> {
            function::Module countA = std::get<3>(a);
            function::Module countB = std::get<3>(b);
            if (countA == 0)
                return b;
            if (countB == 0)
                return a;
            D delta = std::get<0>(b) - std::get<0>(a);
            D totalCount = static_cast<D>(countA + countB);
            D newMean = std::get<0>(a) + delta * static_cast<D>(countB) / totalCount;
            D newM2 = std::get<1>(a) + std::get<1>(b) + delta * delta * static_cast<D>(countA) * static_cast<D>(countB) / totalCount;
            return std::make_tuple(newMean, newM2, std::get<2>(a) + std::get<2>(b), countA + countB);
        },
        [](std::tuple<D, D, D, function::Module> acc) -> D {
            function::Module count = std::get<3>(acc);
            if (count == 0)
                return D{};
            D variance = std::get<1>(acc) / static_cast<D>(count);
            return static_cast<D>(std::sqrt(static_cast<double>(variance)));
        });
}

template <typename E, typename D>
auto useStandardDeviation(const function::Function<E, D> &mapper) -> Collector<E, std::tuple<D, D, D, function::Module>, D>
{
    return useFull<E, std::tuple<D, D, D, function::Module>, D>(
        []() -> std::tuple<D, D, D, function::Module> {
            return std::make_tuple(D{}, D{}, D{}, function::Module{0});
        },
        [mapper](std::tuple<D, D, D, function::Module> acc, E element, function::Timestamp index) -> std::tuple<D, D, D, function::Module> {
            D value = mapper(element);
            function::Module count = std::get<3>(acc) + 1;
            D delta = value - std::get<0>(acc);
            D newMean = std::get<0>(acc) + delta / static_cast<D>(count);
            D delta2 = value - newMean;
            D newM2 = std::get<1>(acc) + delta * delta2;
            return std::make_tuple(newMean, newM2, std::get<2>(acc) + value, count);
        },
        [](std::tuple<D, D, D, function::Module> a, std::tuple<D, D, D, function::Module> b) -> std::tuple<D, D, D, function::Module> {
            function::Module countA = std::get<3>(a);
            function::Module countB = std::get<3>(b);
            if (countA == 0)
                return b;
            if (countB == 0)
                return a;
            D delta = std::get<0>(b) - std::get<0>(a);
            D totalCount = static_cast<D>(countA + countB);
            D newMean = std::get<0>(a) + delta * static_cast<D>(countB) / totalCount;
            D newM2 = std::get<1>(a) + std::get<1>(b) + delta * delta * static_cast<D>(countA) * static_cast<D>(countB) / totalCount;
            return std::make_tuple(newMean, newM2, std::get<2>(a) + std::get<2>(b), countA + countB);
        },
        [](std::tuple<D, D, D, function::Module> acc) -> D {
            function::Module count = std::get<3>(acc);
            if (count == 0)
                return D{};
            D variance = std::get<1>(acc) / static_cast<D>(count);
            return static_cast<D>(std::sqrt(static_cast<double>(variance)));
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
        throw std::invalid_argument("useFindAt: index must be non-negative, use useFindNegativeAt for negative index");
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
        throw std::invalid_argument("useFindNegativeAt: index must be negative, use useFindAt for non-negative index");
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

template <typename E, typename K, typename V, typename KeyExtractor, typename ValueExtractor>
auto useGroupBy(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor)
    -> Collector<E, std::unordered_map<K, std::vector<V>>, std::unordered_map<K, std::vector<V>>>
{
    return useFull<E, std::unordered_map<K, std::vector<V>>, std::unordered_map<K, std::vector<V>>>(
        []() -> std::unordered_map<K, std::vector<V>> { return std::unordered_map<K, std::vector<V>>(); },
        [keyExtractor, valueExtractor](std::unordered_map<K, std::vector<V>> accumulatorValue, E element, function::Timestamp index) -> std::unordered_map<K, std::vector<V>> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp>)
            {
                K key = std::invoke(keyExtractor, element, index);
                if constexpr (std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element, index));
                else if constexpr (std::is_invocable_r_v<V, ValueExtractor, E>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element));
            }
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E>)
            {
                K key = std::invoke(keyExtractor, element);
                if constexpr (std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element, index));
                else if constexpr (std::is_invocable_r_v<V, ValueExtractor, E>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element));
            }
            return accumulatorValue;
        },
        [](std::unordered_map<K, std::vector<V>> a, std::unordered_map<K, std::vector<V>> b) -> std::unordered_map<K, std::vector<V>> {
            for (auto &[key, vec] : b)
            {
                auto &target = a[key];
                target.reserve(target.size() + vec.size());
                target.insert(target.end(), vec.begin(), vec.end());
            }
            return a;
        },
        [](std::unordered_map<K, std::vector<V>> accumulatorValue) -> std::unordered_map<K, std::vector<V>> { return accumulatorValue; });
}

template <typename E>
auto useJoin() -> Collector<E, charsequence::Builder, charsequence::Charsequence>
{
    static const charsequence::Charsequence comma(",");
    static const charsequence::Charsequence leftBracket("[");
    static const charsequence::Charsequence rightBracket("]");
    return useFull<E, charsequence::Builder, charsequence::Charsequence>(
        []() -> charsequence::Builder { return charsequence::Builder(); },
        [](charsequence::Builder accumulatorValue, E element, function::Timestamp index) -> charsequence::Builder {
            if constexpr (std::is_same_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<charsequence::Charsequence>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [](charsequence::Builder a, charsequence::Builder b) -> charsequence::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(comma);
            a.append(b.toCharsequence());
            return a;
        },
        [](charsequence::Builder accumulatorValue) -> charsequence::Charsequence {
            charsequence::Builder result;
            result.append(leftBracket);
            result.append(accumulatorValue.toCharsequence());
            result.append(rightBracket);
            return result.toCharsequence();
        });
}

template <typename E>
auto useJoin(const charsequence::Charsequence &delimiter) -> Collector<E, charsequence::Builder, charsequence::Charsequence>
{
    static const charsequence::Charsequence leftBracket("[");
    static const charsequence::Charsequence rightBracket("]");
    return useFull<E, charsequence::Builder, charsequence::Charsequence>(
        []() -> charsequence::Builder { return charsequence::Builder(); },
        [delimiter](charsequence::Builder accumulatorValue, E element, function::Timestamp index) -> charsequence::Builder {
            if constexpr (std::is_same_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<charsequence::Charsequence>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [delimiter](charsequence::Builder a, charsequence::Builder b) -> charsequence::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(delimiter);
            a.append(b.toCharsequence());
            return a;
        },
        [](charsequence::Builder accumulatorValue) -> charsequence::Charsequence {
            charsequence::Builder result;
            result.append(leftBracket);
            result.append(accumulatorValue.toCharsequence());
            result.append(rightBracket);
            return result.toCharsequence();
        });
}

template <typename E>
auto useJoin(const charsequence::Charsequence &prefix, const charsequence::Charsequence &delimiter, const charsequence::Charsequence &suffix) -> Collector<E, charsequence::Builder, charsequence::Charsequence>
{
    return useFull<E, charsequence::Builder, charsequence::Charsequence>(
        []() -> charsequence::Builder { return charsequence::Builder(); },
        [delimiter](charsequence::Builder accumulatorValue, E element, function::Timestamp index) -> charsequence::Builder {
            if constexpr (std::is_same_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<charsequence::Charsequence>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [delimiter](charsequence::Builder a, charsequence::Builder b) -> charsequence::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(delimiter);
            a.append(b.toCharsequence());
            return a;
        },
        [prefix, suffix](charsequence::Builder accumulatorValue) -> charsequence::Charsequence {
            charsequence::Builder result;
            result.append(prefix);
            result.append(accumulatorValue.toCharsequence());
            result.append(suffix);
            return result.toCharsequence();
        });
}

template <typename E, typename Converter>
auto useJoin(const charsequence::Charsequence &prefix, Converter &&converter, const charsequence::Charsequence &suffix) -> Collector<E, charsequence::Builder, charsequence::Charsequence>
{
    static const charsequence::Charsequence comma(",");
    return useFull<E, charsequence::Builder, charsequence::Charsequence>(
        []() -> charsequence::Builder { return charsequence::Builder(); },
        [converter = std::forward<Converter>(converter)](charsequence::Builder accumulatorValue, E element, function::Timestamp index) -> charsequence::Builder {
            auto converted = converter(element);
            if constexpr (std::is_same_v<decltype(converted), charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            else if constexpr (std::is_same_v<decltype(converted), std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            else if constexpr (std::is_convertible_v<decltype(converted), charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<charsequence::Charsequence>(converted));
            }
            else if constexpr (std::is_convertible_v<decltype(converted), std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<std::string>(converted));
            }
            else if constexpr (std::is_arithmetic_v<decltype(converted)>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            return accumulatorValue;
        },
        [](charsequence::Builder a, charsequence::Builder b) -> charsequence::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(comma);
            a.append(b.toCharsequence());
            return a;
        },
        [prefix, suffix](charsequence::Builder accumulatorValue) -> charsequence::Charsequence {
            charsequence::Builder result;
            result.append(prefix);
            result.append(accumulatorValue.toCharsequence());
            result.append(suffix);
            return result.toCharsequence();
        });
}

template <typename E>
auto useOut() -> Collector<E, charsequence::Builder, charsequence::Charsequence>
{
    static const charsequence::Charsequence comma(",");
    static const charsequence::Charsequence leftBracket("[");
    static const charsequence::Charsequence rightBracket("]");
    return useFull<E, charsequence::Builder, charsequence::Charsequence>(
        []() -> charsequence::Builder { return charsequence::Builder(); },
        [](charsequence::Builder accumulatorValue, E element, function::Timestamp index) -> charsequence::Builder {
            if constexpr (std::is_same_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<charsequence::Charsequence>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [](charsequence::Builder a, charsequence::Builder b) -> charsequence::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(comma);
            a.append(b.toCharsequence());
            return a;
        },
        [](charsequence::Builder accumulatorValue) -> charsequence::Charsequence {
            charsequence::Builder result;
            result.append(leftBracket);
            result.append(accumulatorValue.toCharsequence());
            result.append(rightBracket);
            charsequence::Charsequence finalResult = result.toCharsequence();
            std::cout << finalResult << '\n';
            return finalResult;
        });
}

template <typename E>
auto useOut(const charsequence::Charsequence &delimiter) -> Collector<E, charsequence::Builder, charsequence::Charsequence>
{
    static const charsequence::Charsequence leftBracket("[");
    static const charsequence::Charsequence rightBracket("]");
    return useFull<E, charsequence::Builder, charsequence::Charsequence>(
        []() -> charsequence::Builder { return charsequence::Builder(); },
        [delimiter](charsequence::Builder accumulatorValue, E element, function::Timestamp index) -> charsequence::Builder {
            if constexpr (std::is_same_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<charsequence::Charsequence>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [delimiter](charsequence::Builder a, charsequence::Builder b) -> charsequence::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(delimiter);
            a.append(b.toCharsequence());
            return a;
        },
        [](charsequence::Builder accumulatorValue) -> charsequence::Charsequence {
            charsequence::Builder result;
            result.append(leftBracket);
            result.append(accumulatorValue.toCharsequence());
            result.append(rightBracket);
            charsequence::Charsequence finalResult = result.toCharsequence();
            std::cout << finalResult << '\n';
            return finalResult;
        });
}

template <typename E>
auto useOut(const charsequence::Charsequence &prefix, const charsequence::Charsequence &delimiter, const charsequence::Charsequence &suffix) -> Collector<E, charsequence::Builder, charsequence::Charsequence>
{
    return useFull<E, charsequence::Builder, charsequence::Charsequence>(
        []() -> charsequence::Builder { return charsequence::Builder(); },
        [delimiter](charsequence::Builder accumulatorValue, E element, function::Timestamp index) -> charsequence::Builder {
            if constexpr (std::is_same_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<charsequence::Charsequence>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [delimiter](charsequence::Builder a, charsequence::Builder b) -> charsequence::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(delimiter);
            a.append(b.toCharsequence());
            return a;
        },
        [prefix, suffix](charsequence::Builder accumulatorValue) -> charsequence::Charsequence {
            charsequence::Builder result;
            result.append(prefix);
            result.append(accumulatorValue.toCharsequence());
            result.append(suffix);
            charsequence::Charsequence finalResult = result.toCharsequence();
            std::cout << finalResult << '\n';
            return finalResult;
        });
}

template <typename E, typename Converter>
auto useOut(const charsequence::Charsequence &prefix, Converter &&converter, const charsequence::Charsequence &suffix) -> Collector<E, charsequence::Builder, charsequence::Charsequence>
{
    static const charsequence::Charsequence comma(",");
    return useFull<E, charsequence::Builder, charsequence::Charsequence>(
        []() -> charsequence::Builder { return charsequence::Builder(); },
        [converter = std::forward<Converter>(converter)](charsequence::Builder accumulatorValue, E element, function::Timestamp index) -> charsequence::Builder {
            auto converted = converter(element);
            if constexpr (std::is_same_v<decltype(converted), charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            else if constexpr (std::is_same_v<decltype(converted), std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            else if constexpr (std::is_convertible_v<decltype(converted), charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<charsequence::Charsequence>(converted));
            }
            else if constexpr (std::is_convertible_v<decltype(converted), std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<std::string>(converted));
            }
            else if constexpr (std::is_arithmetic_v<decltype(converted)>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            return accumulatorValue;
        },
        [](charsequence::Builder a, charsequence::Builder b) -> charsequence::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(comma);
            a.append(b.toCharsequence());
            return a;
        },
        [prefix, suffix](charsequence::Builder accumulatorValue) -> charsequence::Charsequence {
            charsequence::Builder result;
            result.append(prefix);
            result.append(accumulatorValue.toCharsequence());
            result.append(suffix);
            charsequence::Charsequence finalResult = result.toCharsequence();
            std::cout << finalResult << '\n';
            return finalResult;
        });
}

template <typename E>
auto useError() -> Collector<E, charsequence::Builder, charsequence::Charsequence>
{
    static const charsequence::Charsequence comma(",");
    static const charsequence::Charsequence leftBracket("[");
    static const charsequence::Charsequence rightBracket("]");
    return useFull<E, charsequence::Builder, charsequence::Charsequence>(
        []() -> charsequence::Builder { return charsequence::Builder(); },
        [](charsequence::Builder accumulatorValue, E element, function::Timestamp index) -> charsequence::Builder {
            if constexpr (std::is_same_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<charsequence::Charsequence>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [](charsequence::Builder a, charsequence::Builder b) -> charsequence::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(comma);
            a.append(b.toCharsequence());
            return a;
        },
        [](charsequence::Builder accumulatorValue) -> charsequence::Charsequence {
            charsequence::Builder result;
            result.append(leftBracket);
            result.append(accumulatorValue.toCharsequence());
            result.append(rightBracket);
            charsequence::Charsequence finalResult = result.toCharsequence();
            std::cerr << finalResult << '\n';
            return finalResult;
        });
}

template <typename E>
auto useError(const charsequence::Charsequence &delimiter) -> Collector<E, charsequence::Builder, charsequence::Charsequence>
{
    static const charsequence::Charsequence leftBracket("[");
    static const charsequence::Charsequence rightBracket("]");
    return useFull<E, charsequence::Builder, charsequence::Charsequence>(
        []() -> charsequence::Builder { return charsequence::Builder(); },
        [delimiter](charsequence::Builder accumulatorValue, E element, function::Timestamp index) -> charsequence::Builder {
            if constexpr (std::is_same_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<charsequence::Charsequence>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [delimiter](charsequence::Builder a, charsequence::Builder b) -> charsequence::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(delimiter);
            a.append(b.toCharsequence());
            return a;
        },
        [](charsequence::Builder accumulatorValue) -> charsequence::Charsequence {
            charsequence::Builder result;
            result.append(leftBracket);
            result.append(accumulatorValue.toCharsequence());
            result.append(rightBracket);
            charsequence::Charsequence finalResult = result.toCharsequence();
            std::cerr << finalResult << '\n';
            return finalResult;
        });
}

template <typename E>
auto useError(const charsequence::Charsequence &prefix, const charsequence::Charsequence &delimiter, const charsequence::Charsequence &suffix) -> Collector<E, charsequence::Builder, charsequence::Charsequence>
{
    return useFull<E, charsequence::Builder, charsequence::Charsequence>(
        []() -> charsequence::Builder { return charsequence::Builder(); },
        [delimiter](charsequence::Builder accumulatorValue, E element, function::Timestamp index) -> charsequence::Builder {
            if constexpr (std::is_same_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_same_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            else if constexpr (std::is_convertible_v<E, charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<charsequence::Charsequence>(element));
            }
            else if constexpr (std::is_convertible_v<E, std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(static_cast<std::string>(element));
            }
            else if constexpr (std::is_arithmetic_v<E>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(delimiter);
                accumulatorValue.append(element);
            }
            return accumulatorValue;
        },
        [delimiter](charsequence::Builder a, charsequence::Builder b) -> charsequence::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(delimiter);
            a.append(b.toCharsequence());
            return a;
        },
        [prefix, suffix](charsequence::Builder accumulatorValue) -> charsequence::Charsequence {
            charsequence::Builder result;
            result.append(prefix);
            result.append(accumulatorValue.toCharsequence());
            result.append(suffix);
            charsequence::Charsequence finalResult = result.toCharsequence();
            std::cerr << finalResult << '\n';
            return finalResult;
        });
}

template <typename E, typename Converter>
auto useError(const charsequence::Charsequence &prefix, Converter &&converter, const charsequence::Charsequence &suffix) -> Collector<E, charsequence::Builder, charsequence::Charsequence>
{
    static const charsequence::Charsequence comma(",");
    return useFull<E, charsequence::Builder, charsequence::Charsequence>(
        []() -> charsequence::Builder { return charsequence::Builder(); },
        [converter = std::forward<Converter>(converter)](charsequence::Builder accumulatorValue, E element, function::Timestamp index) -> charsequence::Builder {
            auto converted = converter(element);
            if constexpr (std::is_same_v<decltype(converted), charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            else if constexpr (std::is_same_v<decltype(converted), std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            else if constexpr (std::is_convertible_v<decltype(converted), charsequence::Charsequence>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<charsequence::Charsequence>(converted));
            }
            else if constexpr (std::is_convertible_v<decltype(converted), std::string>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(static_cast<std::string>(converted));
            }
            else if constexpr (std::is_arithmetic_v<decltype(converted)>)
            {
                if (accumulatorValue.size() > 0)
                    accumulatorValue.append(comma);
                accumulatorValue.append(converted);
            }
            return accumulatorValue;
        },
        [](charsequence::Builder a, charsequence::Builder b) -> charsequence::Builder {
            if (a.size() > 0 && b.size() > 0)
                a.append(comma);
            a.append(b.toCharsequence());
            return a;
        },
        [prefix, suffix](charsequence::Builder accumulatorValue) -> charsequence::Charsequence {
            charsequence::Builder result;
            result.append(prefix);
            result.append(accumulatorValue.toCharsequence());
            result.append(suffix);
            charsequence::Charsequence finalResult = result.toCharsequence();
            std::cerr << finalResult << '\n';
            return finalResult;
        });
}

template <typename E>
auto useFrequency() -> Collector<E, std::pair<std::unordered_map<E, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>, function::Timestamp>, std::map<E, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>>
{
    using InnerMap = std::unordered_map<E, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>;
    using AccumulatorType = std::pair<InnerMap, function::Timestamp>;
    using ResultMap = std::map<E, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>;
    return useFull<E, AccumulatorType, ResultMap>(
        []() -> AccumulatorType {
            return AccumulatorType(InnerMap(), 0LL);
        },
        [](AccumulatorType accumulatorValue, E element, function::Timestamp index) -> AccumulatorType {
            InnerMap &map = accumulatorValue.first;
            std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>> &pair = map[element];
            pair.first.push_back(std::complex<double>(static_cast<double>(index), 1.0));
            accumulatorValue.second = index;
            return accumulatorValue;
        },
        [](AccumulatorType a, AccumulatorType b) -> AccumulatorType {
            InnerMap &mapA = a.first;
            const InnerMap &mapB = b.first;
            for (auto &entry : mapB)
            {
                std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>> &target = mapA[entry.first];
                target.first.insert(target.first.end(), entry.second.first.begin(), entry.second.first.end());
            }
            if (b.second > a.second)
            {
                a.second = b.second;
            }
            return a;
        },
        [](AccumulatorType accumulatorValue) -> ResultMap {
            ResultMap result;
            InnerMap &map = accumulatorValue.first;
            function::Timestamp totalLength = accumulatorValue.second + 1;
            for (auto &entry : map)
            {
                std::size_t totalCount = entry.second.first.size();
                std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>> &target = result[entry.first];
                target.first = std::move(entry.second.first);
                target.second.reserve(totalCount);
                for (std::size_t i = 0; i < totalCount; i++)
                {
                    function::Timestamp position = static_cast<function::Timestamp>(target.first[i].real());
                    target.second.push_back(std::complex<double>(static_cast<double>(position), static_cast<double>(totalLength)));
                }
            }
            return result;
        });
}

template <typename E, typename D>
auto useFrequency(const function::Function<E, D> &mapper) -> Collector<E, std::pair<std::unordered_map<D, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>, function::Timestamp>, std::map<D, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>>
{
    using InnerMap = std::unordered_map<D, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>;
    using AccumulatorType = std::pair<InnerMap, function::Timestamp>;
    using ResultMap = std::map<D, std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>>>;
    return useFull<E, AccumulatorType, ResultMap>(
        []() -> AccumulatorType {
            return AccumulatorType(InnerMap(), 0LL);
        },
        [mapper](AccumulatorType accumulatorValue, E element, function::Timestamp index) -> AccumulatorType {
            InnerMap &map = accumulatorValue.first;
            D key = mapper(element);
            std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>> &pair = map[key];
            pair.first.push_back(std::complex<double>(static_cast<double>(index), 1.0));
            accumulatorValue.second = index;
            return accumulatorValue;
        },
        [](AccumulatorType a, AccumulatorType b) -> AccumulatorType {
            InnerMap &mapA = a.first;
            const InnerMap &mapB = b.first;
            for (auto &entry : mapB)
            {
                std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>> &target = mapA[entry.first];
                target.first.insert(target.first.end(), entry.second.first.begin(), entry.second.first.end());
            }
            if (b.second > a.second)
            {
                a.second = b.second;
            }
            return a;
        },
        [](AccumulatorType accumulatorValue) -> ResultMap {
            ResultMap result;
            InnerMap &map = accumulatorValue.first;
            function::Timestamp totalLength = accumulatorValue.second + 1;
            for (auto &entry : map)
            {
                std::size_t totalCount = entry.second.first.size();
                std::pair<std::vector<std::complex<double>>, std::vector<std::complex<double>>> &target = result[entry.first];
                target.first = std::move(entry.second.first);
                target.second.reserve(totalCount);
                for (std::size_t i = 0; i < totalCount; i++)
                {
                    function::Timestamp position = static_cast<function::Timestamp>(target.first[i].real());
                    target.second.push_back(std::complex<double>(static_cast<double>(position), static_cast<double>(totalLength)));
                }
            }
            return result;
        });
}

template <typename E>
auto useDistribution() -> Collector<E, std::unordered_map<E, std::vector<function::Timestamp>>, std::map<E, std::complex<double>>>
{
    using AccumulatorMap = std::unordered_map<E, std::vector<function::Timestamp>>;
    using ResultMap = std::map<E, std::complex<double>>;
    return useFull<E, AccumulatorMap, ResultMap>(
        []() -> AccumulatorMap {
            return AccumulatorMap();
        },
        [](AccumulatorMap accumulatorValue, E element, function::Timestamp index) -> AccumulatorMap {
            accumulatorValue[element].push_back(index);
            return accumulatorValue;
        },
        [](AccumulatorMap a, AccumulatorMap b) -> AccumulatorMap {
            for (auto &entry : b)
            {
                std::vector<function::Timestamp> &target = a[entry.first];
                target.insert(target.end(), entry.second.begin(), entry.second.end());
            }
            return a;
        },
        [](AccumulatorMap accumulatorValue) -> ResultMap {
            ResultMap result;
            if (accumulatorValue.empty())
                return result;
            std::vector<E> elements;
            std::vector<double> positionSums;
            std::vector<double> counts;
            for (auto &entry : accumulatorValue)
            {
                elements.push_back(entry.first);
                double positionSum = 0.0;
                double count = static_cast<double>(entry.second.size());
                for (function::Timestamp index : entry.second)
                {
                    positionSum += static_cast<double>(index);
                }
                positionSums.push_back(positionSum);
                counts.push_back(count);
            }
            double modePositionSum = 0.0;
            double modeCount = 0.0;
            {
                std::map<double, size_t> positionFreq;
                for (double val : positionSums)
                    positionFreq[val]++;
                size_t maxFreq = 0;
                for (auto &p : positionFreq)
                {
                    if (p.second > maxFreq)
                    {
                        maxFreq = p.second;
                        modePositionSum = p.first;
                    }
                }
            }
            {
                std::map<double, size_t> countFreq;
                for (double val : counts)
                    countFreq[val]++;
                size_t maxFreq = 0;
                for (auto &p : countFreq)
                {
                    if (p.second > maxFreq)
                    {
                        maxFreq = p.second;
                        modeCount = p.first;
                    }
                }
            }
            double positionStddev = 0.0;
            double countStddev = 0.0;
            for (size_t i = 0; i < elements.size(); i++)
            {
                double posDiff = positionSums[i] - modePositionSum;
                double cntDiff = counts[i] - modeCount;
                positionStddev += posDiff * posDiff;
                countStddev += cntDiff * cntDiff;
            }
            positionStddev = std::sqrt(positionStddev / static_cast<double>(elements.size()));
            countStddev = std::sqrt(countStddev / static_cast<double>(elements.size()));
            if (positionStddev < 0.001)
                positionStddev = 1.0;
            if (countStddev < 0.001)
                countStddev = 1.0;
            for (size_t i = 0; i < elements.size(); i++)
            {
                double posScore = (positionSums[i] - modePositionSum) / positionStddev;
                double cntScore = (counts[i] - modeCount) / countStddev;
                result[elements[i]] = std::complex<double>(posScore, cntScore);
            }
            return result;
        });
}

template <typename E, typename D>
auto useDistribution(const function::Function<E, D> &mapper) -> Collector<E, std::unordered_map<D, std::vector<function::Timestamp>>, std::map<D, std::complex<double>>>
{
    using AccumulatorMap = std::unordered_map<D, std::vector<function::Timestamp>>;
    using ResultMap = std::map<D, std::complex<double>>;
    return useFull<E, AccumulatorMap, ResultMap>(
        []() -> AccumulatorMap {
            return AccumulatorMap();
        },
        [mapper](AccumulatorMap accumulatorValue, E element, function::Timestamp index) -> AccumulatorMap {
            accumulatorValue[mapper(element)].push_back(index);
            return accumulatorValue;
        },
        [](AccumulatorMap a, AccumulatorMap b) -> AccumulatorMap {
            for (auto &entry : b)
            {
                std::vector<function::Timestamp> &target = a[entry.first];
                target.insert(target.end(), entry.second.begin(), entry.second.end());
            }
            return a;
        },
        [](AccumulatorMap accumulatorValue) -> ResultMap {
            ResultMap result;
            if (accumulatorValue.empty())
                return result;
            std::vector<D> elements;
            std::vector<double> positionSums;
            std::vector<double> counts;
            for (auto &entry : accumulatorValue)
            {
                elements.push_back(entry.first);
                double positionSum = 0.0;
                double count = static_cast<double>(entry.second.size());
                for (function::Timestamp index : entry.second)
                {
                    positionSum += static_cast<double>(index);
                }
                positionSums.push_back(positionSum);
                counts.push_back(count);
            }
            double modePositionSum = 0.0;
            double modeCount = 0.0;
            {
                std::map<double, size_t> positionFreq;
                for (double val : positionSums)
                    positionFreq[val]++;
                size_t maxFreq = 0;
                for (auto &p : positionFreq)
                {
                    if (p.second > maxFreq)
                    {
                        maxFreq = p.second;
                        modePositionSum = p.first;
                    }
                }
            }
            {
                std::map<double, size_t> countFreq;
                for (double val : counts)
                    countFreq[val]++;
                size_t maxFreq = 0;
                for (auto &p : countFreq)
                {
                    if (p.second > maxFreq)
                    {
                        maxFreq = p.second;
                        modeCount = p.first;
                    }
                }
            }
            double positionStddev = 0.0;
            double countStddev = 0.0;
            for (size_t i = 0; i < elements.size(); i++)
            {
                double posDiff = positionSums[i] - modePositionSum;
                double cntDiff = counts[i] - modeCount;
                positionStddev += posDiff * posDiff;
                countStddev += cntDiff * cntDiff;
            }
            positionStddev = std::sqrt(positionStddev / static_cast<double>(elements.size()));
            countStddev = std::sqrt(countStddev / static_cast<double>(elements.size()));
            if (positionStddev < 0.001)
                positionStddev = 1.0;
            if (countStddev < 0.001)
                countStddev = 1.0;
            for (size_t i = 0; i < elements.size(); i++)
            {
                double posScore = (positionSums[i] - modePositionSum) / positionStddev;
                double cntScore = (counts[i] - modeCount) / countStddev;
                result[elements[i]] = std::complex<double>(posScore, cntScore);
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

template <typename E, typename V, typename KeyExtractor, typename ValueExtractor>
auto usePartitionBy(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor)
    -> Collector<E, std::map<function::Timestamp, std::vector<V>>, std::vector<std::vector<V>>>
{
    return useFull<E, std::map<function::Timestamp, std::vector<V>>, std::vector<std::vector<V>>>(
        []() -> std::map<function::Timestamp, std::vector<V>> { return std::map<function::Timestamp, std::vector<V>>(); },
        [keyExtractor, valueExtractor](std::map<function::Timestamp, std::vector<V>> accumulatorValue, E element, function::Timestamp index) -> std::map<function::Timestamp, std::vector<V>> {
            if constexpr (std::is_invocable_r_v<function::Timestamp, KeyExtractor, E, function::Timestamp>)
            {
                function::Timestamp key = std::invoke(keyExtractor, element, index);
                if constexpr (std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element, index));
                else if constexpr (std::is_invocable_r_v<V, ValueExtractor, E>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element));
            }
            else if constexpr (std::is_invocable_r_v<function::Timestamp, KeyExtractor, E>)
            {
                function::Timestamp key = std::invoke(keyExtractor, element);
                if constexpr (std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element, index));
                else if constexpr (std::is_invocable_r_v<V, ValueExtractor, E>)
                    accumulatorValue[key].push_back(std::invoke(valueExtractor, element));
            }
            return accumulatorValue;
        },
        [](std::map<function::Timestamp, std::vector<V>> a, std::map<function::Timestamp, std::vector<V>> b) -> std::map<function::Timestamp, std::vector<V>> {
            for (auto &[key, vec] : b)
                for (V &element : vec)
                    a[key].push_back(std::move(element));
            return a;
        },
        [](std::map<function::Timestamp, std::vector<V>> accumulatorValue) -> std::vector<std::vector<V>> {
            std::vector<std::vector<V>> result;
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
            double angle = std::fmod(2.0 * pi * static_cast<double>(index), 2.0 * pi);
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
    if (p < 0.0 || p > 100.0)
        throw std::invalid_argument("usePercentile: p must be in range [0.0, 100.0]");
    return useFull<E, std::vector<D>, std::optional<D>>(
        []() -> std::vector<D> { return std::vector<D>(); },
        [](std::vector<D> acc, E element, function::Timestamp index) -> std::vector<D> {
            acc.push_back(static_cast<D>(element));
            return acc;
        },
        [](std::vector<D> a, std::vector<D> b) -> std::vector<D> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [p](std::vector<D> acc) -> std::optional<D> {
            if (acc.empty())
                return std::nullopt;
            std::sort(acc.begin(), acc.end());
            double rank = p / 100.0 * static_cast<double>(acc.size() - 1);
            std::size_t lo = static_cast<std::size_t>(std::floor(rank));
            std::size_t hi = static_cast<std::size_t>(std::ceil(rank));
            if (lo == hi)
                return std::optional<D>(acc[lo]);
            double frac = rank - static_cast<double>(lo);
            return std::optional<D>(acc[lo] + static_cast<D>(frac * static_cast<double>(acc[hi] - acc[lo])));
        });
}

template <typename E, typename D>
auto usePercentile(double p, const function::Function<E, D> &mapper) -> Collector<E, std::vector<D>, std::optional<D>>
{
    if (p < 0.0 || p > 100.0)
        throw std::invalid_argument("usePercentile: p must be in range [0.0, 100.0]");
    return useFull<E, std::vector<D>, std::optional<D>>(
        []() -> std::vector<D> { return std::vector<D>(); },
        [mapper](std::vector<D> acc, E element, function::Timestamp index) -> std::vector<D> {
            acc.push_back(mapper(element));
            return acc;
        },
        [](std::vector<D> a, std::vector<D> b) -> std::vector<D> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [p](std::vector<D> acc) -> std::optional<D> {
            if (acc.empty())
                return std::nullopt;
            std::sort(acc.begin(), acc.end());
            double rank = p / 100.0 * static_cast<double>(acc.size() - 1);
            std::size_t lo = static_cast<std::size_t>(std::floor(rank));
            std::size_t hi = static_cast<std::size_t>(std::ceil(rank));
            if (lo == hi)
                return std::optional<D>(acc[lo]);
            double frac = rank - static_cast<double>(lo);
            return std::optional<D>(acc[lo] + static_cast<D>(frac * static_cast<double>(acc[hi] - acc[lo])));
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
auto useToDeque() -> Collector<E, std::deque<E>, std::deque<E>>
{
    return useFull<E, std::deque<E>, std::deque<E>>(
        []() -> std::deque<E> { return std::deque<E>(); },
        [](std::deque<E> accumulatorValue, E element, function::Timestamp index) -> std::deque<E> {
            accumulatorValue.push_back(element);
            return accumulatorValue;
        },
        [](std::deque<E> a, std::deque<E> b) -> std::deque<E> {
            a.insert(a.end(), b.begin(), b.end());
            return a;
        },
        [](std::deque<E> accumulatorValue) -> std::deque<E> { return accumulatorValue; });
}

template <typename E>
auto useToForwardList() -> Collector<E, std::forward_list<E>, std::forward_list<E>>
{
    return useFull<E, std::forward_list<E>, std::forward_list<E>>(
        []() -> std::forward_list<E> { return std::forward_list<E>(); },
        [](std::forward_list<E> accumulatorValue, E element, function::Timestamp index) -> std::forward_list<E> {
            auto it = accumulatorValue.before_begin();
            while (std::next(it) != accumulatorValue.end())
            {
                ++it;
            }
            accumulatorValue.insert_after(it, element);
            return accumulatorValue;
        },
        [](std::forward_list<E> a, std::forward_list<E> b) -> std::forward_list<E> {
            auto it = a.before_begin();
            while (std::next(it) != a.end())
            {
                ++it;
            }
            a.splice_after(it, b);
            return a;
        },
        [](std::forward_list<E> accumulatorValue) -> std::forward_list<E> { return accumulatorValue; });
}

template <typename E, std::size_t N>
auto useToArray() -> Collector<E, std::array<E, N>, std::array<E, N>>
{
    return useFull<E, std::array<E, N>, std::array<E, N>>(
        []() -> std::array<E, N> { std::array<E, N> arr; return arr; },
        [](std::array<E, N> accumulatorValue, E element, function::Timestamp index) -> std::array<E, N> {
            if (index < N)
                accumulatorValue[index] = element;
            return accumulatorValue;
        },
        [](std::array<E, N> a, std::array<E, N> b) -> std::array<E, N> { return a; },
        [](std::array<E, N> accumulatorValue) -> std::array<E, N> { return accumulatorValue; });
}

template <typename E>
auto useToMultiset() -> Collector<E, std::multiset<E>, std::multiset<E>>
{
    return useFull<E, std::multiset<E>, std::multiset<E>>(
        []() -> std::multiset<E> { return std::multiset<E>(); },
        [](std::multiset<E> accumulatorValue, E element, function::Timestamp index) -> std::multiset<E> {
            accumulatorValue.insert(element);
            return accumulatorValue;
        },
        [](std::multiset<E> a, std::multiset<E> b) -> std::multiset<E> { a.insert(b.begin(), b.end()); return a; },
        [](std::multiset<E> accumulatorValue) -> std::multiset<E> { return accumulatorValue; });
}

template <typename E>
auto useToUnorderedMultiset() -> Collector<E, std::unordered_multiset<E>, std::unordered_multiset<E>>
{
    return useFull<E, std::unordered_multiset<E>, std::unordered_multiset<E>>(
        []() -> std::unordered_multiset<E> { return std::unordered_multiset<E>(); },
        [](std::unordered_multiset<E> accumulatorValue, E element, function::Timestamp index) -> std::unordered_multiset<E> {
            accumulatorValue.insert(element);
            return accumulatorValue;
        },
        [](std::unordered_multiset<E> a, std::unordered_multiset<E> b) -> std::unordered_multiset<E> { a.insert(b.begin(), b.end()); return a; },
        [](std::unordered_multiset<E> accumulatorValue) -> std::unordered_multiset<E> { return accumulatorValue; });
}

template <typename E>
auto useToMultimap(const function::Function<E, function::Timestamp> &keyExtractor) -> Collector<E, std::multimap<function::Timestamp, E>, std::multimap<function::Timestamp, E>>
{
    return useFull<E, std::multimap<function::Timestamp, E>, std::multimap<function::Timestamp, E>>(
        []() -> std::multimap<function::Timestamp, E> { return std::multimap<function::Timestamp, E>(); },
        [keyExtractor](std::multimap<function::Timestamp, E> accumulatorValue, E element, function::Timestamp index) -> std::multimap<function::Timestamp, E> {
            accumulatorValue.insert(std::make_pair(keyExtractor(element, index), element));
            return accumulatorValue;
        },
        [](std::multimap<function::Timestamp, E> a, std::multimap<function::Timestamp, E> b) -> std::multimap<function::Timestamp, E> {
            a.insert(b.begin(), b.end());
            return a;
        },
        [](std::multimap<function::Timestamp, E> accumulatorValue) -> std::multimap<function::Timestamp, E> { return accumulatorValue; });
}

template <typename E, typename K, typename KeyExtractor>
auto useToMultimap(KeyExtractor &&keyExtractor) -> Collector<E, std::multimap<K, E>, std::multimap<K, E>>
{
    return useFull<E, std::multimap<K, E>, std::multimap<K, E>>(
        []() -> std::multimap<K, E> { return std::multimap<K, E>(); },
        [keyExtractor](std::multimap<K, E> accumulatorValue, E element, function::Timestamp index) -> std::multimap<K, E> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element, index), element));
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element), element));
            return accumulatorValue;
        },
        [](std::multimap<K, E> a, std::multimap<K, E> b) -> std::multimap<K, E> {
            a.insert(b.begin(), b.end());
            return a;
        },
        [](std::multimap<K, E> accumulatorValue) -> std::multimap<K, E> { return accumulatorValue; });
}

template <typename E, typename K, typename V, typename KeyExtractor, typename ValueExtractor>
auto useToMultimap(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor) -> Collector<E, std::multimap<K, V>, std::multimap<K, V>>
{
    return useFull<E, std::multimap<K, V>, std::multimap<K, V>>(
        []() -> std::multimap<K, V> { return std::multimap<K, V>(); },
        [keyExtractor, valueExtractor](std::multimap<K, V> accumulatorValue, E element, function::Timestamp index) -> std::multimap<K, V> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp> && std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element, index), std::invoke(valueExtractor, element, index)));
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E> && std::is_invocable_r_v<V, ValueExtractor, E>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element), std::invoke(valueExtractor, element)));
            return accumulatorValue;
        },
        [](std::multimap<K, V> a, std::multimap<K, V> b) -> std::multimap<K, V> {
            a.insert(b.begin(), b.end());
            return a;
        },
        [](std::multimap<K, V> accumulatorValue) -> std::multimap<K, V> { return accumulatorValue; });
}

template <typename E>
auto useToUnorderedMultimap(const function::Function<E, function::Timestamp> &keyExtractor) -> Collector<E, std::unordered_multimap<function::Timestamp, E>, std::unordered_multimap<function::Timestamp, E>>
{
    return useFull<E, std::unordered_multimap<function::Timestamp, E>, std::unordered_multimap<function::Timestamp, E>>(
        []() -> std::unordered_multimap<function::Timestamp, E> { return std::unordered_multimap<function::Timestamp, E>(); },
        [keyExtractor](std::unordered_multimap<function::Timestamp, E> accumulatorValue, E element, function::Timestamp index) -> std::unordered_multimap<function::Timestamp, E> {
            accumulatorValue.insert(std::make_pair(keyExtractor(element, index), element));
            return accumulatorValue;
        },
        [](std::unordered_multimap<function::Timestamp, E> a, std::unordered_multimap<function::Timestamp, E> b) -> std::unordered_multimap<function::Timestamp, E> {
            a.insert(b.begin(), b.end());
            return a;
        },
        [](std::unordered_multimap<function::Timestamp, E> accumulatorValue) -> std::unordered_multimap<function::Timestamp, E> { return accumulatorValue; });
}

template <typename E, typename K, typename KeyExtractor>
auto useToUnorderedMultimap(KeyExtractor &&keyExtractor) -> Collector<E, std::unordered_multimap<K, E>, std::unordered_multimap<K, E>>
{
    return useFull<E, std::unordered_multimap<K, E>, std::unordered_multimap<K, E>>(
        []() -> std::unordered_multimap<K, E> { return std::unordered_multimap<K, E>(); },
        [keyExtractor](std::unordered_multimap<K, E> accumulatorValue, E element, function::Timestamp index) -> std::unordered_multimap<K, E> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element, index), element));
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element), element));
            return accumulatorValue;
        },
        [](std::unordered_multimap<K, E> a, std::unordered_multimap<K, E> b) -> std::unordered_multimap<K, E> {
            a.insert(b.begin(), b.end());
            return a;
        },
        [](std::unordered_multimap<K, E> accumulatorValue) -> std::unordered_multimap<K, E> { return accumulatorValue; });
}

template <typename E, typename K, typename V, typename KeyExtractor, typename ValueExtractor>
auto useToUnorderedMultimap(KeyExtractor &&keyExtractor, ValueExtractor &&valueExtractor) -> Collector<E, std::unordered_multimap<K, V>, std::unordered_multimap<K, V>>
{
    return useFull<E, std::unordered_multimap<K, V>, std::unordered_multimap<K, V>>(
        []() -> std::unordered_multimap<K, V> { return std::unordered_multimap<K, V>(); },
        [keyExtractor, valueExtractor](std::unordered_multimap<K, V> accumulatorValue, E element, function::Timestamp index) -> std::unordered_multimap<K, V> {
            if constexpr (std::is_invocable_r_v<K, KeyExtractor, E, function::Timestamp> && std::is_invocable_r_v<V, ValueExtractor, E, function::Timestamp>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element, index), std::invoke(valueExtractor, element, index)));
            else if constexpr (std::is_invocable_r_v<K, KeyExtractor, E> && std::is_invocable_r_v<V, ValueExtractor, E>)
                accumulatorValue.insert(std::make_pair(std::invoke(keyExtractor, element), std::invoke(valueExtractor, element)));
            return accumulatorValue;
        },
        [](std::unordered_multimap<K, V> a, std::unordered_multimap<K, V> b) -> std::unordered_multimap<K, V> {
            a.insert(b.begin(), b.end());
            return a;
        },
        [](std::unordered_multimap<K, V> accumulatorValue) -> std::unordered_multimap<K, V> { return accumulatorValue; });
}

template <typename E>
auto useToStack() -> Collector<E, std::stack<E>, std::stack<E>>
{
    return useFull<E, std::stack<E>, std::stack<E>>(
        []() -> std::stack<E> { return std::stack<E>(); },
        [](std::stack<E> accumulatorValue, E element, function::Timestamp index) -> std::stack<E> {
            accumulatorValue.push(element);
            return accumulatorValue;
        },
        [](std::stack<E> a, std::stack<E> b) -> std::stack<E> {
            std::vector<E> temp;
            while (!b.empty())
            {
                temp.push_back(b.top());
                b.pop();
            }
            for (auto it = temp.rbegin(); it != temp.rend(); ++it)
            {
                a.push(*it);
            }
            return a;
        },
        [](std::stack<E> accumulatorValue) -> std::stack<E> { return accumulatorValue; });
}

template <typename E>
auto useToQueue() -> Collector<E, std::queue<E>, std::queue<E>>
{
    return useFull<E, std::queue<E>, std::queue<E>>(
        []() -> std::queue<E> { return std::queue<E>(); },
        [](std::queue<E> accumulatorValue, E element, function::Timestamp index) -> std::queue<E> {
            accumulatorValue.push(element);
            return accumulatorValue;
        },
        [](std::queue<E> a, std::queue<E> b) -> std::queue<E> {
            while (!b.empty())
            {
                a.push(b.front());
                b.pop();
            }
            return a;
        },
        [](std::queue<E> accumulatorValue) -> std::queue<E> { return accumulatorValue; });
}

template <typename E>
auto useToPriorityQueue() -> Collector<E, std::priority_queue<E>, std::priority_queue<E>>
{
    return useFull<E, std::priority_queue<E>, std::priority_queue<E>>(
        []() -> std::priority_queue<E> { return std::priority_queue<E>(); },
        [](std::priority_queue<E> accumulatorValue, E element, function::Timestamp index) -> std::priority_queue<E> {
            accumulatorValue.push(element);
            return accumulatorValue;
        },
        [](std::priority_queue<E> a, std::priority_queue<E> b) -> std::priority_queue<E> {
            while (!b.empty())
            {
                a.push(b.top());
                b.pop();
            }
            return a;
        },
        [](std::priority_queue<E> accumulatorValue) -> std::priority_queue<E> { return accumulatorValue; });
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