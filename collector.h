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
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <initializer_list>
#include <iostream>

namespace collector
{
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

template <typename E, typename A, typename R>
class Collector
{
  private:
	std::unique_ptr<Identity<A>> identity;
	std::unique_ptr<Interrupt<E, A>> interrupt;
	std::unique_ptr<Accumulator<A, E>> accumulator;
	std::unique_ptr<Combiner<A>> combiner;
	std::unique_ptr<Finisher<A, R>> finisher;

  public:
	Collector(const Identity<A> &identity, const Interrupt<E, A> &interrupt, const Accumulator<A, E> &accumulator, const Combiner<A> &combiner, const Finisher<A, R> &finisher) : identity(std::make_unique<Identity<A>>(identity)), interrupt(std::make_unique<Interrupt<E, A>>(interrupt)), accumulator(std::make_unique<Accumulator<A, E>>(accumulator)), combiner(std::make_unique<Combiner<A>>(combiner)), finisher(std::make_unique<Finisher<A, R>>(finisher)) {}

	Collector(Identity<A> &&identity, Interrupt<E, A> &&interrupt, Accumulator<A, E> &&accumulator, Combiner<A> &&combiner, Finisher<A, R> &&finisher) : identity(std::make_unique<Identity<A>>(std::move(identity))), interrupt(std::make_unique<Interrupt<E, A>>(std::move(interrupt))), accumulator(std::make_unique<Accumulator<A, E>>(std::move(accumulator))), combiner(std::make_unique<Combiner<A>>(std::move(combiner))), finisher(std::make_unique<Finisher<A, R>>(std::move(finisher))) {}

	Collector(Collector<E, A, R> &&other) noexcept : identity(std::move(other.identity)), interrupt(std::move(other.interrupt)), accumulator(std::move(other.accumulator)), combiner(std::move(other.combiner)), finisher(std::move(other.finisher)) {}

	Collector<E, A, R> &operator=(Collector<E, A, R> &&other) noexcept
	{
		if (this != &other)
		{
			this->identity = std::move(other.identity);
			this->interrupt = std::move(other.interrupt);
			this->accumulator = std::move(other.accumulator);
			this->combiner = std::move(other.combiner);
			this->finisher = std::move(other.finisher);
		}
		return *this;
	}

	~Collector() = default;

	auto collect(const function::Generator<E> &generator, const function::Module &concurrent) const -> R
	{
		if (concurrent < 2)
		{
			try
			{
				A identityValue = (*(this->identity))();
				generator([&identityValue, this](E element, function::Timestamp index) -> void { identityValue = (*(this->accumulator))(identityValue, element, index); }, [&identityValue, this](E element, function::Timestamp index) -> bool { return (*(this->interrupt))(element, index, identityValue); });
				return (*(this->finisher))(identityValue);
			}
			catch (const std::exception &exception)
			{
				throw exception;
			}
		}
		std::vector<std::future<A>> futures;
		futures.reserve(concurrent);
		for (function::Module thread = 0; thread < concurrent; thread++)
		{
			futures.emplace_back(pool::pool.submit<A>([this, thread, &generator, concurrent]() -> A {
				try
				{
					A identityValue = (*(this->identity))();
					generator([thread, &identityValue, concurrent, this](E element, function::Timestamp index) -> void {
                            if (index % concurrent == thread) {
                                identityValue = (*(this->accumulator))(identityValue, element, index);
                            } }, [&identityValue, this](E element, function::Timestamp index) -> bool { return (*(this->interrupt))(element, index, identityValue); });
					return identityValue;
				}
				catch (const std::exception &exception)
				{
					throw exception;
				}
			}));
		}
		A identityValue = (*(this->identity))();
		for (std::future<A> &future : futures)
		{
			identityValue = (*(this->combiner))(identityValue, future.get());
		}
		return (*(this->finisher))(identityValue);
	}

	auto collect(const std::vector<E> &container, const function::Module &concurrent) const -> R
	{
		if (concurrent < 2)
		{
			try
			{
				A identityValue = (*(this->identity))();
				function::Timestamp index = 0;
				for (const E &element : container)
				{
					if ((*(this->interrupt))(element, index, identityValue))
					{
						break;
					}
					identityValue = (*(this->accumulator))(identityValue, element, index);
					index++;
				}
				return (*(this->finisher))(identityValue);
			}
			catch (const std::exception &exception)
			{
				std::cerr << exception.what() << '\n';
			}
		}
		std::vector<std::future<A>> futures;
		futures.reserve(concurrent);
		for (function::Module thread = 0; thread < concurrent; thread++)
		{
			futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]() -> A {
				A identityValue = (*(this->identity))();
				function::Module index = 0;
				for (const E &element : container)
				{
					if ((*(this->interrupt))(element, index, identityValue))
					{
						break;
					}
					if (index % concurrent == thread)
					{
						identityValue = (*(this->accumulator))(identityValue, element, index);
					}
					index++;
				}
				return identityValue;
			}));
		}
		A identityValue = (*(this->identity))();
		for (std::future<A> &future : futures)
		{
			identityValue = (*(this->combiner))(identityValue, future.get());
		}
		return (*(this->finisher))(identityValue);
	}

	auto collect(const std::set<E> &container, const function::Module &concurrent) const -> R
	{
		if (concurrent < 2)
		{
			try
			{
				A identityValue = (*(this->identity))();
				function::Timestamp index = 0;
				for (const E &element : container)
				{
					if ((*(this->interrupt))(element, index, identityValue))
					{
						break;
					}
					identityValue = (*(this->accumulator))(identityValue, element, index);
					index++;
				}
				return (*(this->finisher))(identityValue);
			}
			catch (const std::exception &exception)
			{
				throw exception;
			}
		}
		std::vector<std::future<A>> futures;
		futures.reserve(concurrent);
		for (function::Module thread = 0; thread < concurrent; thread++)
		{
			futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]() -> A {
				try
				{
					A identityValue = (*(this->identity))();
					function::Module index = 0;
					for (const E &element : container)
					{
						if ((*(this->interrupt))(element, index, identityValue))
						{
							break;
						}
						if (index % concurrent == thread)
						{
							identityValue = (*(this->accumulator))(identityValue, element, index);
						}
						index++;
					}
					return identityValue;
				}
				catch (const std::exception &exception)
				{
					throw exception;
				}
			}));
		}
		A identityValue = (*(this->identity))();
		for (std::future<A> &future : futures)
		{
			identityValue = (*(this->combiner))(identityValue, future.get());
		}
		return (*(this->finisher))(identityValue);
	}

	auto collect(const std::unordered_set<E> &container, const function::Module &concurrent) const -> R
	{
		if (concurrent < 2)
		{
			try
			{
				A identityValue = (*(this->identity))();
				function::Timestamp index = 0;
				for (const E &element : container)
				{
					if ((*(this->interrupt))(element, index, identityValue))
					{
						break;
					}
					identityValue = (*(this->accumulator))(identityValue, element, index);
					index++;
				}
				return (*(this->finisher))(identityValue);
			}
			catch (const std::exception &exception)
			{
				throw exception;
			}
		}
		std::vector<std::future<A>> futures;
		futures.reserve(concurrent);
		for (function::Module thread = 0; thread < concurrent; thread++)
		{
			futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]() -> A {
				try
				{
					A identityValue = (*(this->identity))();
					function::Module index = 0;
					for (const E &element : container)
					{
						if ((*(this->interrupt))(element, index, identityValue))
						{
							break;
						}
						if (index % concurrent == thread)
						{
							identityValue = (*(this->accumulator))(identityValue, element, index);
						}
						index++;
					}
					return identityValue;
				}
				catch (const std::exception &exception)
				{
					throw exception;
				}
			}));
		}
		A identityValue = (*(this->identity))();
		for (std::future<A> &future : futures)
		{
			identityValue = (*(this->combiner))(identityValue, future.get());
		}
		return (*(this->finisher))(identityValue);
	}

	auto collect(const std::list<E> &container, const function::Module &concurrent) const -> R
	{
		if (concurrent < 2)
		{
			try
			{
				A identityValue = (*(this->identity))();
				function::Timestamp index = 0;
				for (const E &element : container)
				{
					if ((*(this->interrupt))(element, index, identityValue))
					{
						break;
					}
					identityValue = (*(this->accumulator))(identityValue, element, index);
					index++;
				}
				return (*(this->finisher))(identityValue);
			}
			catch (const std::exception &exception)
			{
				throw exception;
			}
		}
		std::vector<std::future<A>> futures;
		futures.reserve(concurrent);
		for (function::Module thread = 0; thread < concurrent; thread++)
		{
			futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]() -> A {
				A identityValue = (*(this->identity))();
				function::Module index = 0;
				for (const E &element : container)
				{
					if ((*(this->interrupt))(element, index, identityValue))
					{
						break;
					}
					if (index % concurrent == thread)
					{
						identityValue = (*(this->accumulator))(identityValue, element, index);
					}
					index++;
				}
				return identityValue;
			}));
		}
		A identityValue = (*(this->identity))();
		for (std::future<A> &future : futures)
		{
			identityValue = (*(this->combiner))(identityValue, future.get());
		}
		return (*(this->finisher))(identityValue);
	}

	auto collect(const std::initializer_list<E> &container, const function::Module &concurrent) const -> R
	{
		if (concurrent < 2)
		{
			try
			{
				A identityValue = (*(this->identity))();
				function::Timestamp index = 0;
				for (const E &element : container)
				{
					if ((*(this->interrupt))(element, index, identityValue))
					{
						break;
					}
					identityValue = (*(this->accumulator))(identityValue, element, index);
					index++;
				}
				return (*(this->finisher))(identityValue);
			}
			catch (const std::exception &exception)
			{
				throw exception;
			}
		}
		std::vector<std::future<A>> futures;
		futures.reserve(concurrent);
		for (function::Module thread = 0; thread < concurrent; thread++)
		{
			futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]() -> A {
				A identityValue = (*(this->identity))();
				function::Module index = 0;
				for (const E &element : container)
				{
					if ((*(this->interrupt))(element, index, identityValue))
					{
						break;
					}
					if (index % concurrent == thread)
					{
						identityValue = (*(this->accumulator))(identityValue, element, index);
					}
					index++;
				}
				return identityValue;
			}));
		}
		A identityValue = (*(this->identity))();
		for (std::future<A> &future : futures)
		{
			identityValue = (*(this->combiner))(identityValue, future.get());
		}
		return (*(this->finisher))(identityValue);
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

} // namespace collector
