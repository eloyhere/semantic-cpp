#pragma once
#include "../structure/function.h"
#include "../structure/pool.h"
#include "../bunch/bunch.h"
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
#include <cmath>
#include <complex>
#include <tuple>
#include <limits>

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
        auto hasError = std::make_shared<std::atomic<bool>>(false);
        std::vector<std::future<A>> futures;
        futures.reserve(concurrent);

        for (function::Module thread = 0; thread < concurrent; ++thread)
        {
            futures.emplace_back(globalPool().submit<A>([this, &container, thread, concurrent, hasError]() -> A {
                A identityValue = (*identity)();
                function::Module index = 0;
                for (const E &element : container)
                {
                    if (hasError->load())
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
        auto hasError = std::make_shared<std::atomic<bool>>(false);
        std::vector<std::future<A>> futures;
        futures.reserve(concurrent);

        for (function::Module thread = 0; thread < concurrent; ++thread)
        {
            futures.emplace_back(globalPool().submit<A>([this, thread, &generator, concurrent, hasError]() -> A {
                A identityValue = (*identity)();
                generator(
                    [thread, &identityValue, concurrent, hasError, this](E element, function::Timestamp index) -> void {
                        if (!hasError->load() && index % concurrent == thread)
                        {
                            identityValue = (*accumulator)(identityValue, element, index);
                        }
                    },
                    [&identityValue, hasError, this](E element, function::Timestamp index) -> bool {
                        return hasError->load() || (*interrupt)(element, index, identityValue);
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
} // namespace collector
