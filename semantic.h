#pragma once
#include <set>
#include <map>
#include <list>
#include <cmath>
#include <array>
#include <deque>
#include <stack>
#include <queue>
#include <mutex>
#include <string>
#include <vector>
#include <future>
#include <thread>
#include <memory>
#include <random>
#include <numeric>
#include <iostream>
#include <optional>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <initializer_list>
namespace pool {
	class ThreadPool;
};
namespace collector {

	template<typename E, typename A, typename R>
	class Collector;

};
namespace collectable {

	template<typename E>
	class Collectable;

	template<typename E>
	class OrderedCollectable;

	template<typename E, typename D>
	class Statistics;

	template<typename E>
	class WindowCollectable;

	template<typename E>
	class UnorderedCollectable;
};
namespace semantic {
	template<typename E>
	class Semantic;
}
namespace functional {
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
	using Generator = BiConsumer<BiConsumer<T, Timestamp>, BiPredicate<T, Timestamp>>;

	template <typename D>
	D randomly(const D& start, const D& end) {
		static std::random_device random;
		static std::mt19937_64 generator(random());

		D maximum = std::max(start, end);
		D minimum = std::min(start, end);

		if constexpr (std::is_integral<D>::value) {
			std::uniform_int_distribution<D> distribution(minimum, maximum);
			return distribution(generator);
		}
		else {
			std::uniform_real_distribution<D> distribution(minimum, maximum);
			return distribution(generator);
		}
	}

	bool randomly() {
		static std::random_device device;
		static std::mt19937 generator(device());
		std::bernoulli_distribution distribution(0.5);
		return distribution(generator);
	}
};

namespace pool {

	using Timestamp = functional::Timestamp;
	using Module = functional::Module;

	using Runnable = functional::Runnable;

	template<typename R>
	using Supplier = functional::Supplier<R>;

	class ThreadPool {
	private:
		std::vector<std::thread> workers;
		std::queue<Runnable> tasks;
		std::mutex mutex;
		std::atomic<bool> active{ true };
		std::condition_variable condition;
	public:
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool& operator=(ThreadPool&&) = delete;
		~ThreadPool() {
			if (active.load()) {
				shutdown();
			}
		}
		explicit ThreadPool(const Module& size = std::thread::hardware_concurrency()) {
			for (Module i = 0; i < size; ++i) {
				this->increase();
			}
		}

		void increase() {
			this->workers.emplace_back([this] {
				while (this->active.load() || this->tasks.size() > 0) {
					Runnable task;
					{
						std::unique_lock<std::mutex> lock(this->mutex);
						this->condition.wait(lock, [this] {
							return !this->tasks.empty() || !this->active.load();
							});
						if (!active.load() && this->tasks.empty()) {
							break;
						}
						if (!this->tasks.empty()) {
							task = std::move(this->tasks.front());
							this->tasks.pop();
						}
					}
					task();
				}
				});
		}

		void decrease() {
			if (!this->workers.empty()) {
				{
					std::lock_guard<std::mutex> lock(this->mutex);
					this->tasks.emplace([] {});
				}
				this->condition.notify_one();
				if (this->workers.back().joinable()) {
					this->workers.back().join();
				}
				this->workers.pop_back();
			}
		}

		void boot() {
			if (!this->active.exchange(true)) {
				this->condition.notify_all();
			}
		}

		void shutdown() {
			if (this->active.exchange(false)) {
				{
					std::unique_lock<std::mutex> lock(this->mutex);
					this->condition.wait(lock, [this] {
						return this->tasks.empty();
						});
				}
				this->condition.notify_all();
				for (std::thread& worker : this->workers) {
					if (worker.joinable()) {
						worker.join();
					}
				}
			}
		}

		std::future<void> submit(const Runnable& task) {
			if (!this->active.load()) {
				throw std::runtime_error("ThreadPool is not active");
			}
			std::shared_ptr<std::packaged_task<void()>> packaged_task = std::make_shared<std::packaged_task<void()>>(task);
			std::future<void> result = packaged_task->get_future();
			{
				std::lock_guard<std::mutex> lock(this->mutex);
				this->tasks.emplace([packaged_task]() {
					(*packaged_task)();
					});
			}
			this->condition.notify_one();
			return result;
		}

		template<typename R>
		std::future<R> submit(const Supplier<R>& task) {
			if (!this->active.load()) {
				throw std::runtime_error("ThreadPool is not active");
			}
			std::shared_ptr<std::packaged_task<R()>> packaged_task = std::make_shared<std::packaged_task<R()>>(task);
			std::future<R> result = packaged_task->get_future();
			{
				std::lock_guard<std::mutex> lock(this->mutex);
				this->tasks.emplace([packaged_task]() {
					(*packaged_task)();
					});
			}
			this->condition.notify_one();
			return result;
		}
	};

	inline ThreadPool pool;
};

namespace collector {

	using Timestamp = functional::Timestamp;
	using Module = functional::Module;

	using Runnable = functional::Runnable;

	template<typename R>
	using Supplier = functional::Supplier<R>;

	template<typename T, typename R>
	using Function = functional::Function<T, R>;

	template<typename T, typename U, typename R>
	using BiFunction = functional::BiFunction<T, U, R>;

	template<typename T, typename U, typename V, typename R>
	using TriFunction = functional::TriFunction<T, U, V, R>;

	template<typename T>
	using Consumer = functional::Consumer<T>;

	template<typename T, typename U>
	using BiConsumer = functional::BiConsumer<T, U>;

	template <typename T, typename U, typename V>
	using TriConsumer = functional::TriConsumer<T, U, V>;

	template<typename T>
	using Predicate = functional::Predicate<T>;

	template<typename T, typename U>
	using BiPredicate = functional::BiPredicate<T, U>;

	template<typename T, typename U, typename V>
	using TriPredicate = functional::TriPredicate<T, U, V>;

	template<typename T, typename U>
	using Comparator = functional::Comparator<T, U>;

	template<typename T>
	using Generator = functional::Generator<T>;

	template<typename A>
	using Identity = Supplier<A>;

	template<typename E, typename A>
	using Interrupt = functional::TriPredicate<E, Timestamp, A>;

	template <typename A, typename E>
	using Accumulator = functional::TriFunction<A, E, Timestamp, A>;

	template<typename A>
	using Combiner = functional::BiFunction<A, A, A>;

	template<typename A, typename R>
	using Finisher = functional::Function<A, R>;

	template<typename E, typename A, typename R>
	class Collector {
	private:
		std::unique_ptr<Identity<A>> identity;
		std::unique_ptr<Interrupt<E, A>> interrupt;
		std::unique_ptr<Accumulator<A, E>> accumulator;
		std::unique_ptr<Combiner<A>> combiner;
		std::unique_ptr<Finisher<A, R>> finisher;
	public:
		Collector(const Identity<A>& identity, const Interrupt<E, A>& interrupt, const Accumulator<A, E>& accumulator, const Combiner<A>& combiner, const Finisher<A, R>& finisher) : identity(std::make_unique<Identity<A>>(identity)), interrupt(std::make_unique<Interrupt<E, A>>(interrupt)), accumulator(std::make_unique<Accumulator<A, E>>(accumulator)), combiner(std::make_unique<Combiner<A>>(combiner)), finisher(std::make_unique<Finisher<A, R>>(finisher)) {}
		Collector(Identity<A>&& identity, Interrupt<E, A>&& interrupt, Accumulator<A, E>&& accumulator, Combiner<A>&& combiner, Finisher<A, R>&& finisher) : identity(std::make_unique<Identity<A>>(std::move(identity))), interrupt(std::make_unique<Interrupt<E, A>>(std::move(interrupt))), accumulator(std::make_unique<Accumulator<A, E>>(std::move(accumulator))), combiner(std::make_unique<Combiner<A>>(std::move(combiner))), finisher(std::make_unique<Finisher<A, R>>(std::move(finisher))) {}
		Collector(Collector<E, A, R>&& other) noexcept : identity(std::move(other.identity)), interrupt(std::move(other.interrupt)), accumulator(std::move(other.accumulator)), combiner(std::move(other.combiner)), finisher(std::move(other.finisher)) {}
		Collector<E, A, R>& operator=(Collector<E, A, R>&& other) noexcept {
			if (this != &other) {
				this->identity = std::move(other.identity);
				this->interrupt = std::move(other.interrupt);
				this->accumulator = std::move(other.accumulator);
				this->combiner = std::move(other.combiner);
				this->finisher = std::move(other.finisher);
			}
			return *this;
		}
		~Collector() = default;

		R collect(const Generator<E>& generator, const Module& concurrent) const {
			if (concurrent < 2) {
				try {
					A identity = (*(this->identity))();
					generator([&identity, this](E element, Timestamp index)-> void {
						identity = (*(this->accumulator))(identity, element, index);
					}, [&identity, this](E element, Timestamp index)-> bool {
						return (*(this->interrupt))(element, index, identity);
					});
					return (*(this->finisher))(identity);
				}
				catch (const std::exception& exception) {
					throw exception;
				}
			}
			std::vector<std::future<A>> futures;
			futures.reserve(concurrent);
			for (Module thread = 0; thread < concurrent; thread++) {
				futures.emplace_back(pool::pool.submit<A>([this, thread, &generator, concurrent]()-> A {
					A identity = (*(this->identity))();
					generator([thread, &identity, concurrent, this](E element, Timestamp index)-> void {
						if (index % concurrent == thread) {
							identity = (*(this->accumulator))(identity, element, index);
						}
					}, [&identity, this](E element, Timestamp index)-> bool {
						return (*(this->interrupt))(element, index, identity);
					});
					return identity;
				}));
			}
			A identity = (*(this->identity))();
			for (std::future<A>& future : futures) {
				identity = (*(this->combiner))(identity, future.get());
			}
			return (*(this->finisher))(identity);
		}

		R collect(const std::vector<E>& container, const Module& concurrent) const {
			if (concurrent < 2) {
				try {
					A identity = (*(this->identity))();
					Timestamp index = 0;
					for (const E& element : container) {
						if ((*(this->interrupt))(element, index, identity)) {
							break;
						}
						identity = (*(this->accumulator))(identity, element, index);
						index++;
					}
					return (*(this->finisher))(identity);
				}
				catch (const std::exception& exception) {
					throw exception;
				}
			}
			std::vector<std::future<A>> futures;
			futures.reserve(concurrent);
			for (Module thread = 0; thread < concurrent; thread++) {
				futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]()-> A {
					A identity = (*(this->identity))();
					Module index = 0;
					for (const E& element : container) {
						if ((*(this->interrupt))(element, index, identity)) {
							break;
						}
						if (index % concurrent == thread) {
							identity = (*(this->accumulator))(identity, element, index);
						}
						index++;
					}
					return identity;
					}));
			}
			A identity = (*(this->identity))();
			for (std::future<A>& future : futures) {
				identity = (*(this->combiner))(identity, future.get());
			}
			return (*(this->finisher))(identity);
		}

		R collect(const std::set<E>& container, const Module& concurrent) const {
			if (concurrent < 2) {
				try {
					A identity = (*(this->identity))();
					Timestamp index = 0;
					for (const E& element : container) {
						if ((*(this->interrupt))(element, index, identity)) {
							break;
						}
						identity = (*(this->accumulator))(identity, element, index);
						index++;
					}
					return (*(this->finisher))(identity);
				}
				catch (const std::exception& exception) {
					throw exception;
				}
			}
			std::vector<std::future<A>> futures;
			futures.reserve(concurrent);
			for (Module thread = 0; thread < concurrent; thread++) {
				futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]()-> A {
					A identity = (*(this->identity))();
					Module index = 0;
					for (const E& element : container) {
						if ((*(this->interrupt))(element, index, identity)) {
							break;
						}
						if (index % concurrent == thread) {
							identity = (*(this->accumulator))(identity, element, index);
						}
						index++;
					}
					return identity;
					}));
			}
			A identity = (*(this->identity))();
			for (std::future<A>& future : futures) {
				identity = (*(this->combiner))(identity, future.get());
			}
			return (*(this->finisher))(identity);
		}

		R collect(const std::unordered_set<E>& container, const Module& concurrent) const {
			if (concurrent < 2) {
				try {
					A identity = (*(this->identity))();
					Timestamp index = 0;
					for (const E& element : container) {
						if ((*(this->interrupt))(element, index, identity)) {
							break;
						}
						identity = (*(this->accumulator))(identity, element, index);
						index++;
					}
					return (*(this->finisher))(identity);
				}
				catch (const std::exception& exception) {
					throw exception;
				}
			}
			std::vector<std::future<A>> futures;
			futures.reserve(concurrent);
			for (Module thread = 0; thread < concurrent; thread++) {
				futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]()-> A {
					A identity = (*(this->identity))();
					Module index = 0;
					for (const E& element : container) {
						if ((*(this->interrupt))(element, index, identity)) {
							break;
						}
						if (index % concurrent == thread) {
							identity = (*(this->accumulator))(identity, element, index);
						}
						index++;
					}
					return identity;
					}));
			}
			A identity = (*(this->identity))();
			for (std::future<A>& future : futures) {
				identity = (*(this->combiner))(identity, future.get());
			}
			return (*(this->finisher))(identity);
		}

		R collect(const std::list<E>& container, const Module& concurrent) const {
			if (concurrent < 2) {
				try {
					A identity = (*(this->identity))();
					Timestamp index = 0;
					for (const E& element : container) {
						if ((*(this->interrupt))(element, index, identity)) {
							break;
						}
						identity = (*(this->accumulator))(identity, element, index);
						index++;
					}
					return (*(this->finisher))(identity);
				}
				catch (const std::exception& exception) {
					throw exception;
				}
			}
			std::vector<std::future<A>> futures;
			futures.reserve(concurrent);
			for (Module thread = 0; thread < concurrent; thread++) {
				futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]()-> A {
					A identity = (*(this->identity))();
					Module index = 0;
					for (const E& element : container) {
						if ((*(this->interrupt))(element, index, identity)) {
							break;
						}
						if (index % concurrent == thread) {
							identity = (*(this->accumulator))(identity, element, index);
						}
						index++;
					}
					return identity;
					}));
			}
			A identity = (*(this->identity))();
			for (std::future<A>& future : futures) {
				identity = (*(this->combiner))(identity, future.get());
			}
			return (*(this->finisher))(identity);
		}

		R collect(const std::initializer_list<E>& container, const Module& concurrent) const {
			if (concurrent < 2) {
				try {
					A identity = (*(this->identity))();
					Timestamp index = 0;
					for (const E& element : container) {
						if ((*(this->interrupt))(element, index, identity)) {
							break;
						}
						identity = (*(this->accumulator))(identity, element, index);
						index++;
					}
					return (*(this->finisher))(identity);
				}
				catch (const std::exception& exception) {
					throw exception;
				}
			}
			std::vector<std::future<A>> futures;
			futures.reserve(concurrent);
			for (Module thread = 0; thread < concurrent; thread++) {
				futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]()-> A {
					A identity = (*(this->identity))();
					Module index = 0;
					for (const E& element : container) {
						if ((*(this->interrupt))(element, index, identity)) {
							break;
						}
						if (index % concurrent == thread) {
							identity = (*(this->accumulator))(identity, element, index);
						}
						index++;
					}
					return identity;
				}));
			}
			A identity = (*(this->identity))();
			for (std::future<A>& future : futures) {
				identity = (*(this->combiner))(identity, future.get());
			}
			return (*(this->finisher))(identity);
		}
	};


	template<typename E, typename A, typename R>
	Collector<E, A, R> useFull(const Identity<A>& identity, const Accumulator<A, E>& accumulator, const Combiner<A>& combiner, const Finisher<A, R>& finisher) {
		const Interrupt<E, A>& interrupt = [](const E element, const Timestamp& index, const A& accumulator)-> bool {
			return false;
			};
		return  Collector<E, A, R>(identity, interrupt, accumulator, combiner, finisher);
	}

	template<typename E, typename A, typename R>
	Collector<E, A, R> useShortable(const Identity<A>& identity, const Interrupt<E, A>& interrupt, const Accumulator<A, E>& accumulator, const Combiner<A>& combiner, const Finisher<A, R>& finisher) {
		return  Collector<E, A, R>(identity, interrupt, accumulator, combiner, finisher);
	}

	template<typename E>
	Collector<E, bool, bool> useAllMatch(const BiPredicate<E, Module>& predicate) {
		return useShortable<E, bool, bool>(
			[]()->bool {
				return true;
			},
			[predicate](E element, Timestamp index, bool accumulator)-> bool {
				return !accumulator;
			},
			[predicate](bool accumulator, E element, Timestamp index)-> bool {
				return accumulator && predicate(element, index);
			},
			[](bool a, bool b)-> bool {
				return a && b;
			},
			[](bool accumulator)-> bool {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, bool, bool> useAnyMatch(const BiPredicate<E, Module>& predicate) {
		return useShortable<E, bool, bool>(
			[]()->bool {
				return false;
			},
			[predicate](E element, Timestamp index, bool accumulator)-> bool {
				return accumulator;
			},
			[predicate](bool accumulator, E element, Timestamp index)-> bool {
				return accumulator || predicate(element, index);
			},
			[](bool a, bool b)-> bool {
				return a || b;
			},
			[](bool accumulator)-> bool {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, bool, bool> useNoneMatch(const BiPredicate<E, Timestamp>& predicate) {
		return useShortable<E, bool, bool>(
			[]()->bool {
				return true;
			},
			[predicate](E element, Timestamp index, bool accumulator)-> bool {
				return !accumulator;
			},
			[predicate](bool accumulator, E element, Timestamp index)-> bool {
				return accumulator && !predicate(element, index);
			},
			[](bool a, bool b)-> bool {
				return a && b;
			},
			[](bool accumulator)-> bool {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, Module, Module> useForEach(const BiConsumer<E, Timestamp>& action) {
		return useFull<E, Module, Module>(
			[]()-> Module {
				return 0;
			},
			[action](const Module& accumulator, const E& element, const Timestamp& index)-> Module {
				action(element, index);
				return accumulator + 1;
			},
			[](const Module& a, const Module& b)-> Module {
				return a + b;
			},
			[](const Module& identity)-> Module {
				return identity;
			}
		);
	}

	template<typename E, typename A, typename R>
	Collector<E, A, R> useCollect(const Identity<R>& identity, const Accumulator<A, E>& accumulator, const  Combiner<A>& combiner, const Finisher<A, R>& finisher) {
		return useFull<E, A, R>(identity, accumulator, combiner, finisher);
	}

	template<typename E, typename A, typename R>
	Collector<E, A, R> useCollect(const Identity<R>& identity, const Interrupt<E, A>& interrupt, const Accumulator<A, E>& accumulator, const  Combiner<A>& combiner, const Finisher<A, R>& finisher) {
		return useShortable<E, A, R>(identity, interrupt, accumulator, combiner, finisher);
	}

	template<typename E>
	Collector<E, Module, Module> useCount() {
		return useFull<E, Module, Module>(
			[]()-> Module {
				return 0;
			},
			[](const Module& accumulator, const E& element, const Timestamp& index)-> Module {
				return accumulator + 1;
			},
			[](const Module& a, const Module& b)-> Module {
				return a + b;
			},
			[](const Module& accumulator)-> Module {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, std::string, std::string> useError() {
		return useFull<E, std::string, std::string>(
			[]()->std::string {
				return "";
			},
			[](std::string accumulator, E element, Timestamp index)-> std::string {
				if (accumulator.length() > 1) {
					return accumulator + "," + std::to_string(element);
				}
				return std::to_string(element);
			},
			[](std::string a, std::string b)-> std::string {
				return a + "," + b;
			},
			[](std::string accumulator)-> std::string {
				std::string result = "[" + accumulator + "]";
				std::cerr << result << std::endl;
				return result;
			}
		);
	}

	template<typename E>
	Collector<E, std::string, std::string> useError(const std::string& prefix, const std::string& delimiter, const std::string suffix) {
		return useFull<E, std::string, std::string>(
			[]()->std::string {
				return "";
			},
			[delimiter](std::string accumulator, E element, Timestamp index)-> std::string {
				if (accumulator.length() > prefix.length()) {
					return accumulator + delimiter + std::to_string(element);
				}
				return std::to_string(element);
			},
			[delimiter](std::string a, std::string b)-> std::string {
				return a + delimiter + b;
			},
			[prefix, suffix](std::string accumulator)-> std::string {
				std::string result = accumulator + suffix;
				std::cerr << result << std::endl;
				return result;
			}
		);
	}

	template<typename E>
	Collector<E, std::string, std::string> useError(const std::string& prefix, const Accumulator<std::string, E>& serializer, const std::string suffix, const BiFunction<std::string, std::string, std::string> &combiner) {
		return useFull<E, std::string, std::string>(
			[]()->std::string {
				return "";
			},
			[serializer](std::string accumulator, E element, Timestamp index)-> std::string {
				if (accumulator.length() > 0) {
					return accumulator + serializer(accumulator, element, index);
				}
				return serializer(accumulator, element, index);
			},
			[combiner](std::string a, std::string b)-> std::string {
				return combiner(a, b);
			},
			[prefix, suffix](std::string accumulator)-> std::string {
				std::string result = accumulator + suffix;
				std::cerr << result << std::endl;
				return result;
			}
		);
	}

	template<typename E>
	Collector<E, std::optional<E>, std::optional<E>> useFindAny() {
		return useShortable<E, std::optional<E>, std::optional<E >> (
			[]()-> std::optional<E> {
				return std::nullopt;
			},
			[](E element, Timestamp index, std::optional<E> accumulator)-> bool {
				return accumulator.has_value();
			},
			[](std::optional<E> accumulator, E element, Timestamp index)-> std::optional<E> {
				if (functional::randomly()) {
					return std::optional<E>(element);
				}
				return std::nullopt;
			},
			[](std::optional<E> a, std::optional<E> b)-> std::optional<E> {
				if (a.has_value()) {
					return a;
				}
				if (b.has_value()) {
					return b;
				}
				return std::nullopt;
			},
			[](std::optional<E> accumulator)-> std::optional<E> {
				return accumulator;
			}
			);
	}

	template<typename E>
	Collector<E, std::vector<E>, std::optional<E>> useFindAt(const Timestamp& index) {
		if (index < 0) {
			return useFull<E, std::vector<E>, std::optional<E>>(
				[]()->std::vector<E> {
					return std::vector<E>();
				},
				[](std::vector<E> accumulator, E element, Timestamp index)-> std::vector<E> {
					accumulator.push_back(element);
					return accumulator;
				},
				[](std::vector<E> a, std::vector<E> b)->std::vector<E> {
					for (const E& element : b) {
						a.push_back(element);
					}
					return a;
				},
				[index](std::vector<E> accumulator)-> std::optional<E> {
					if (accumulator.empty()) {
						return std::nullopt;
					}
					Timestamp target = accumulator.size() - (std::abs(index) % accumulator.size());
					return std::optional<E>(accumulator[target]);
				}
			);
		}
		return useShortable<E, std::vector<E>, std::optional<E>>(
			[]()->std::vector<E> {
				return std::vector<E>();
			},
			[](E element, Timestamp index, std::vector<E> accumulator)-> bool {
				return accumulator.size() - 1 == index;
			},
			[](std::vector<E> accumulator, E element, Timestamp index)-> std::vector<E> {
				accumulator.push_back(element);
				return accumulator;
			},
			[](std::vector<E> a, std::vector<E> b)->std::vector<E> {
				for (const E& element : b) {
					a.push_back(element);
				}
				return a;
			},
			[index](std::vector<E> accumulator)-> std::optional<E> {
				if (accumulator.empty()) {
					return std::nullopt;
				}
				Timestamp target = ((index % accumulator.size()) + accumulator.size()) % accumulator.size();
				return std::optional<E>(accumulator[target]);
			}
		);
	}

	template<typename E>
	Collector<E, std::optional<E>, std::optional<E>> useFindFirst() {
		return useShortable<E, std::optional<E>, std::optional<E >> (
			[]()-> std::optional<E> {
				return std::nullopt;
			},
			[](E element, Timestamp index, std::optional<E> accumulator)-> bool {
				return accumulator.has_value();
			},
			[](std::optional<E> accumulator, E element, Timestamp index)-> std::optional<E> {
				if (accumulator.has_value()) {
					return accumulator;
				}
				return std::optional<E>(element);
			},
			[](std::optional<E> a, std::optional<E> b)-> std::optional<E> {
				if (a.has_value()) {
					return a;
				}
				if (b.has_value()) {
					return b;
				}
				return std::nullopt;
			},
			[](std::optional<E> accumulator)-> std::optional<E> {
				return accumulator;
			});
	}

	template<typename E>
	Collector<E, std::vector<E>, std::optional<E>> useFindLast() {
		return useFull<E, std::vector<E>, std::optional<E >> (
			[]()-> std::vector<E> {
				return std::vector<E>();
			},
			[](std::vector<E> accumulator, E element, Timestamp index)-> std::vector<E> {
				accumulator.push_back(element);
				return accumulator;
			},
			[](std::vector<E> a, std::vector<E> b)-> std::vector<E> {
				for (const E& element : b) {
					a.push_back(element);
				}
				return a;
			},
			[](std::vector<E> accumulator)-> std::optional<E> {
				if (accumulator.empty()) {
					return std::nullopt;
				}
				return std::optional<E>(accumulator[accumulator.size() - 1]);
			});
	}

	template<typename E>
	Collector<E, std::optional<E>, std::optional<E>> useFindMaximum() {
		return useFull<E, std::optional<E>, std::optional<E>>(
			[]()-> std::optional<E> {
				return std::nullopt;
			},
			[](std::optional<E> accumulator, E element, Timestamp index)-> std::optional<E> {
				try {
					if (accumulator.has_value()) {
						if (element > accumulator.value()) {
						    return std::optional<E>(element);
						}
						return accumulator;
					}
					return std::optional<E>(element);
				}
				catch (...) {
					return std::nullopt;
				}
			},
			[](std::optional<E> a, std::optional<E> b)-> std::optional<E> {
				try {
					if (a.has_value()) {
						if (b.has_value()) {
							return a.value() > b.value() ? a : b;
						}
						return a;
					}
					if (b.has_value()) {
						return b;
					}
					return std::nullopt;
				}
				catch (...) {
					return std::nullopt;
				}
			},
			[](std::optional<E> accumulator)-> std::optional<E> {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, std::optional<E>, std::optional<E>> useFindMaximum(const Comparator<E, E> &comparator) {
		return useFull<E, std::optional<E>, std::optional<E>>(
			[]()-> std::optional<E> {
				return std::nullopt;
			},
			[comparator](std::optional<E> accumulator, E element, Timestamp index)-> std::optional<E> {
				if (accumulator.has_value()) {
					if (comparator(element, accumulator.value()) > 0) {
						return std::optional<E>(element);
					}
					return accumulator;
				}
				return std::optional<E>(element);
			},
			[comparator](std::optional<E> a, std::optional<E> b)-> std::optional<E> {
				if (a.has_value()) {
					if (b.has_value()) {
						return comparator(a.value(), b.value()) > 0 ? a : b;
					}
					return a;
				}
				if (b.has_value()) {
					return b;
				}
				return std::nullopt;
			},
			[](std::optional<E> accumulator)-> std::optional<E> {
				return accumulator;
			});
	}

	template<typename E>
	Collector<E, std::optional<E>, std::optional<E>> useFindMinimum() {
		return useFull<E, std::optional<E>, std::optional<E>>(
			[]()-> std::optional<E> {
				return std::nullopt;
			},
			[](std::optional<E> accumulator, E element, Timestamp index)-> std::optional<E> {
				try {
					if (accumulator.has_value()) {
						if (element < accumulator.value()) {
							return std::optional<E>(element);
						}
						return accumulator;
					}
					return std::optional<E>(element);
				}
				catch (...) {
					return std::nullopt;
				}
			},
			[](std::optional<E> a, std::optional<E> b)-> std::optional<E> {
				try {
					if (a.has_value()) {
						if (b.has_value()) {
							return a.value() < b.value() ? a : b;
						}
						return a;
					}
					if (b.has_value()) {
						return b;
					}
					return std::nullopt;
				}
				catch (...) {
					return std::nullopt;
				}
			},
			[](std::optional<E> accumulator)-> std::optional<E> {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, std::optional<E>, std::optional<E>> useFindMinimum(const Comparator<E, E>& comparator) {
		return useFull<E, std::optional<E>, std::optional<E>>(
			[]()-> std::optional<E> {
				return std::nullopt;
			},
			[comparator](std::optional<E> accumulator, E element, Timestamp index)-> std::optional<E> {
				if (accumulator.has_value()) {
					if (comparator(element, accumulator.value()) < 0) {
						return std::optional<E>(element);
					}
					return accumulator;
				}
				return std::optional<E>(element);
			},
			[comparator](std::optional<E> a, std::optional<E> b)-> std::optional<E> {
				if (a.has_value()) {
					if (b.has_value()) {
						return comparator(a.value(), b.value()) < 0 ? a : b;
					}
					return a;
				}
				if (b.has_value()) {
					return b;
				}
				return std::nullopt;
			},
			[](std::optional<E> accumulator)-> std::optional<E> {
				return accumulator;
			});
	}

	template<typename E, typename K>
	Collector<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>> useGroup(const BiFunction<E, Timestamp, K> &keyExtractor) {
		return useFull<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>>(
			[]()->std::unordered_map<K, std::vector<E>> {
				return std::unordered_map<K, std::vector<E>>();
			},
			[keyExtractor](std::unordered_map<K, std::vector<E>> accumulator, E element, Timestamp index)-> std::unordered_map<K, std::vector<E>> {
			    K key = keyExtractor(element, index);
				if (accumulator.contains(key)) {
					accumulator[key].push_back(element);
					return accumulator;
				}
				std::vector<E> group;
				group.push_back(element);
				accumulator.insert(std::pair<K, std::vector<E>>(key, group));
				return accumulator;
			},
			[](std::unordered_map<K, std::vector<E>> a, std::unordered_map<K, std::vector<E>> b)-> std::unordered_map<K, std::vector<E>> {
				for (std::pair<K, std::vector<E>> pair : b) {
					if (a.contains(pair.first)) {
						std::vector<E> groupA = a[pair.first];
						std::vector<E> groupB = pair.second;
						groupA.reserve(groupA.size() + groupB.size());
						groupA.insert(groupA.end(), groupB.begin(), groupB.end());
					}
					else {
						a[pair.first] = std::move(pair.second);
					}
				}
				return a;
			},
			[](std::unordered_map<K, std::vector<E>> accumulator)-> std::unordered_map<K, std::vector<E>> {
				return accumulator;
			}
		);
	}

	template<typename E, typename K, typename V>
	Collector<E, std::unordered_map<K, std::vector<V>>, std::unordered_map<K, std::vector<V>>> useGroupBy(const BiFunction<E, Timestamp, K>& keyExtractor, const BiFunction<E, Timestamp, V>& valueExtractor) {
		return useFull<E, std::unordered_map<K, E>, std::unordered_map<K, std::vector<E>>>(
			[]()->std::unordered_map<K, E> {
				return std::unordered_map<K, E>();
			},
			[keyExtractor, valueExtractor](std::unordered_map<K, std::vector<V>> accumulator, E element, Timestamp index)-> std::unordered_map<K, std::vector<V>> {
				K key = keyExtractor(element, index);
				V value = valueExtractor(element, index);
				if (accumulator.contains(key)) {
					accumulator[key].push_back(value);
					return accumulator;
				}
				std::vector<E> group();
				group.push_back(value);
				accumulator[key] = group;
				return accumulator;
			},
			[](std::unordered_map<K, std::vector<V>> a, std::unordered_map<K, std::vector<V>> b)-> std::unordered_map<K, std::vector<V>> {
				for (std::pair<K, std::vector<E>> pair : b) {
					if (a.contains(pair.first)) {
						std::vector<E> groupA = a[pair.first];
						std::vector<E> groupB = pair.second;
						groupA.reserve(groupA.size() + groupB.size());
						groupA.insert(groupA.end(), groupB.begin(), groupB.end());
					}
					else {
						a[pair.first] = std::move(pair.second);
					}
				}
				return a;
			},
			[](std::unordered_map<K, std::vector<V>> accumulator)-> std::unordered_map<K, std::vector<V>> {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, std::string, std::string> useJoin() {
		return useFull<E, std::string, std::string>(
			[]()-> std::string {
				return "";
			},
			[](std::string accumulator, E element, Timestamp index)-> std::string {
				if (accumulator.length() > 1) {
					return accumulator + "," + std::to_string(element);
				}
				return std::to_string(element);
			},
			[](std::string a, std::string b)-> std::string {
				return a + "," + b;
			},
			[](std::string accumulator)-> std::string {
				return "[" + accumulator + "]";
			}
		);
	}

	template<typename E>
	Collector<E, std::string, std::string> useJoin(const std::string& prefix, const std::string& delimiter, const std::string suffix) {
		return useFull<E, std::string, std::string>(
			[]()-> std::string {
				return "";
			},
			[delimiter](std::string accumulator, E element, Timestamp index)-> std::string {
				if (accumulator.length() > 0) {
					return accumulator + delimiter + std::to_string(element);
				}
				return std::to_string(element);
			},
			[delimiter](std::string a, std::string b)-> std::string {
				return a + delimiter + b.substr(1);
			},
			[prefix, suffix](std::string accumulator)-> std::string {
				return "[" + accumulator + "]";
			}
		);
	}

	template<typename E>
	Collector<E, std::string, std::string> useJoin(const std::string& prefix, const Accumulator<std::string, E>& serializer, const std::string suffix, const BiFunction<std::string, std::string, std::string>& combiner) {
		return useFull<E, std::string, std::string>(
			[]()->std::string {
				return "";
			},
			[serializer](std::string accumulator, E element, Timestamp index)-> std::string {
				if (accumulator.length() > 0) {
					return accumulator + serializer(accumulator, element, index);
				}
				return serializer(accumulator, element, index);
			},
			[combiner](std::string a, std::string b)-> std::string {
				return combiner(a, b);
			},
			[prefix, suffix](std::string accumulator)-> std::string {
				std::string result = accumulator + suffix;
				std::cerr << result << std::endl;
				return result;
			}
		);
	}

	template<typename E>
	Collector<E, std::string, std::string> useOut() {
		return useFull<E, std::string, std::string>(
			[]()->std::string {
				return "";
			},
			[](std::string accumulator, E element, Timestamp index)-> std::string {
				if (accumulator.length() > 1) {
					return accumulator + "," + std::to_string(element);
				}
				return std::to_string(element);
			},
			[](std::string a, std::string b)-> std::string {
				return a + "," + b;
			},
			[](std::string accumulator)-> std::string {
				std::string result = "[" + accumulator + "]";
				std::cerr << result << std::endl;
				return result;
			}
		);
	}

	template<typename E>
	Collector<E, std::string, std::string> useOut(const std::string& prefix, const std::string& delimiter, const std::string suffix) {
		return useFull<E, std::string, std::string>(
			[]()->std::string {
				return "";
			},
			[delimiter](std::string accumulator, E element, Timestamp index)-> std::string {
				if (accumulator.length() > prefix.length()) {
					return accumulator + delimiter + std::to_string(element);
				}
				return std::to_string(element);
			},
			[delimiter](std::string a, std::string b)-> std::string {
				return a + delimiter + b;
			},
			[prefix, suffix](std::string accumulator)-> std::string {
				std::string result = accumulator + suffix;
				std::cerr << result << std::endl;
				return result;
			}
		);
	}

	template<typename E>
	Collector<E, std::string, std::string> useOut(const std::string& prefix, const Accumulator<std::string, E>& serializer, const std::string suffix, const BiFunction<std::string, std::string, std::string>& combiner) {
		return useFull<E, std::string, std::string>(
			[]()->std::string {
				return "";
			},
			[serializer](std::string accumulator, E element, Timestamp index)-> std::string {
				if (accumulator.length() > 0) {
					return accumulator + serializer(accumulator, element, index);
				}
				return serializer(accumulator, element, index);
			},
			[combiner](std::string a, std::string b)-> std::string {
				return combiner(a, b);
			},
			[prefix, suffix](std::string accumulator)-> std::string {
				std::string result = accumulator + suffix;
				std::cerr << result << std::endl;
				return result;
			}
		);
	}

	template<typename E>
	Collector<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>> usePartition(const Module& size) {
		return useFull<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>>(
			[size]()-> std::vector<std::vector<E>> {
				return std::vector<std::vector<E>>();
			},
			[size](std::vector<std::vector<E>> accumulator, E element, Timestamp index)-> std::vector<std::vector<E>> {
				if (size == 0 || size == 1) {
					std::vector<E> single = { element };
					accumulator.push_back(single);
					return accumulator;
				}
				if (accumulator.empty()) {
					std::vector<E> partition = { element };
					accumulator.push_back(partition);
					return accumulator;
				}
				std::vector<E>& last = accumulator.back();
				if (last.size() < size) {
					last.push_back(element);
				}
				else {
					std::vector<E> new_partition = { element };
					accumulator.push_back(new_partition);
				}
				return accumulator;
			},
			[](std::vector<std::vector<E>> a, std::vector<std::vector<E>> b)-> std::vector<std::vector<E>> {
				if (a.empty()) return b;
				if (b.empty()) return a;
				std::vector<E>& last_a = a.back();
				std::vector<E>& first_b = b.front();
				if (last_a.size() < first_b.capacity()) {
					Module can_take = first_b.capacity() - last_a.size();
					Module take_count = std::min(can_take, first_b.size());
					for (size_t i = 0; i < take_count; i++) {
						last_a.push_back(first_b[i]);
					}
					if (take_count == first_b.size()) {
						b.erase(b.begin());
					}
					else {
						first_b.erase(first_b.begin(), first_b.begin() + take_count);
					}
				}
				a.insert(a.end(), b.begin(), b.end());
				return a;
			},
			[](std::vector<std::vector<E>> accumulator)-> std::vector<std::vector<E>> {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, std::map<Timestamp, std::vector<E>>, std::vector<std::vector<E>>> usePartitionBy(const BiFunction<E, Timestamp, Timestamp>& keyExtractor) {
		return useFull<E, std::map<Timestamp, std::vector<E>>, std::vector<std::vector<E>>>(
			[]()-> std::map<Timestamp, std::vector<E>> {
				return std::map<Timestamp, std::vector<E>>();
			},
			[keyExtractor](std::map<Timestamp, std::vector<E>> accumulator, E element, Timestamp index)-> std::map<Timestamp, std::vector<E>> {
				Timestamp key = keyExtractor(element, index);
				accumulator[key].push_back(element);
				return accumulator;
			},
			[](std::map<Timestamp, std::vector<E>> a, std::map<Timestamp, std::vector<E>> b)-> std::map<Timestamp, std::vector<E>> {
				for (auto& [key, vec] : b) {
					for (E element : vec) {
						a[key].push_back(std::move(element));
					}
				}
				return a;
			},
			[](std::map<Timestamp, std::vector<E>> accumulator)-> std::vector<std::vector<E>> {
				std::vector<std::vector<E>> result;
				result.reserve(accumulator.size());
				for (auto& [key, vec] : accumulator) {
					result.push_back(std::move(vec));
				}
				return result;
			}
		);
	}

	template<typename E>
	Collector<E, std::optional<E>, std::optional<E>> useReduce(const BiFunction<E, E, E>& reducer) {
		return useFull<E, std::optional<E>, std::optional<E>>(
			[]()-> std::optional<E> {
				return std::nullopt;
			},
			[reducer](std::optional<E> accumulator, E element, Timestamp index)-> std::optional<E> {
				if (!accumulator.has_value()) {
					return std::optional<E>(element);
				}
				return std::optional<E>(reducer(accumulator.value(), element));
			},
			[reducer](std::optional<E> a, std::optional<E> b)-> std::optional<E> {
				if (!a.has_value()) return b;
				if (!b.has_value()) return a;
				return std::optional<E>(reducer(a.value(), b.value()));
			},
			[](std::optional<E> accumulator)-> std::optional<E> {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, E, E> useReduce(const E& identity, const BiFunction<E, E, E>& reducer) {
		return useFull<E, E, E>(
			[identity]()-> E {
				return identity;
			},
			[reducer](E accumulator, E element, Timestamp index)-> E {
				return reducer(accumulator, element);
			},
			[reducer](E a, E b)-> E {
				return reducer(a, b);
			},
			[](E accumulator)-> E {
				return accumulator;
			}
		);
	}

	template<typename E, typename R>
	Collector<E, R, R> useReduce(const R& identity, const BiFunction<R, E, R>& reducer, const BiFunction<R, R, R>& combiner, const Function<R, R>& finisher) {
		return useFull<E, R, R>(
			[identity]()-> R {
				return identity;
			},
			[reducer](R accumulator, E element, Timestamp index)-> R {
				return reducer(accumulator, element);
			},
			[combiner](R a, R b)-> R {
				return combiner(a, b);
			},
			[finisher](R accumulator)-> R {
				return finisher(accumulator);
			}
		);
	}

	template<typename E, typename K>
	Collector<E, std::map<K, E>, std::map<K, E>> useToMap(const BiFunction<E, Timestamp, K>& keyExtractor) {
		return useFull<E, std::map<K, E>, std::map<K, E>>(
			[]()-> std::map<K, E> {
				return std::map<K, E>();
			},
			[keyExtractor](std::map<K, E> accumulator, E element, Timestamp index)-> std::map<K, E> {
				K key = keyExtractor(element, index);
				accumulator[key] = element;
				return accumulator;
			},
			[](std::map<K, E> a, std::map<K, E> b)-> std::map<K, E> {
				for (const auto& [key, value] : b) {
					a[key] = value;
				}
				return a;
			},
			[](std::map<K, E> accumulator)-> std::map<K, E> {
				return accumulator;
			}
		);
	}

	template<typename E, typename K, typename V>
	Collector<E, std::map<K, V>, std::map<K, V>> useToMap(const BiFunction<E, Timestamp, K>& keyExtractor, const BiFunction<E, Timestamp, V>& valueExtractor) {
		return useFull<E, std::map<K, V>, std::map<K, V>>(
			[]()-> std::map<K, V> {
				return std::map<K, V>();
			},
			[keyExtractor, valueExtractor](std::map<K, V> accumulator, E element, Timestamp index)-> std::map<K, V> {
				K key = keyExtractor(element, index);
				V value = valueExtractor(element, index);
				accumulator[key] = value;
				return accumulator;
			},
			[](std::map<K, V> a, std::map<K, V> b)-> std::map<K, V> {
				for (const auto& [key, value] : b) {
					a[key] = value;
				}
				return a;
			},
			[](std::map<K, V> accumulator)-> std::map<K, V> {
				return accumulator;
			}
		);
	}

	template<typename E, typename K, typename V>
	Collector<E, std::unordered_map<K, V>, std::unordered_map<K, V>> useToUnorderedMap(const BiFunction<E, Timestamp, K>& keyExtractor, const BiFunction<E, Timestamp, V>& valueExtractor) {
		return useFull<E, std::unordered_map<K, V>, std::unordered_map<K, V>>(
			[]()-> std::unordered_map<K, V> {
				return std::unordered_map<K, V>();
			},
			[keyExtractor, valueExtractor](std::unordered_map<K, V> accumulator, E element, Timestamp index)-> std::unordered_map<K, V> {
				K key = keyExtractor(element, index);
				V value = valueExtractor(element, index);
				accumulator[key] = value;
				return accumulator;
			},
			[](std::unordered_map<K, V> a, std::unordered_map<K, V> b)-> std::unordered_map<K, V> {
				for (const auto& [key, value] : b) {
					a[key] = value;
				}
				return a;
			},
			[](std::unordered_map<K, V> accumulator)-> std::unordered_map<K, V> {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, std::set<E>, std::set<E>> useToSet() {
		return useFull<E, std::set<E>, std::set<E>>(
			[]()-> std::set<E> {
				return std::set<E>();
			},
			[](std::set<E> accumulator, E element, Timestamp index)-> std::set<E> {
				accumulator.insert(element);
				return accumulator;
			},
			[](std::set<E> a, std::set<E> b)-> std::set<E> {
				a.insert(b.begin(), b.end());
				return a;
			},
			[](std::set<E> accumulator)-> std::set<E> {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, std::unordered_set<E>, std::unordered_set<E>> useToUnorderedSet() {
		return useFull<E, std::unordered_set<E>, std::unordered_set<E>>(
			[]()-> std::unordered_set<E> {
				return std::unordered_set<E>();
			},
			[](std::unordered_set<E> accumulator, E element, Timestamp index)-> std::unordered_set<E> {
				accumulator.insert(element);
				return accumulator;
			},
			[](std::unordered_set<E> a, std::unordered_set<E> b)-> std::unordered_set<E> {
				a.insert(b.begin(), b.end());
				return a;
			},
			[](std::unordered_set<E> accumulator)-> std::unordered_set<E> {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, std::vector<E>, std::vector<E>> useToVector() {
		return useFull<E, std::vector<E>, std::vector<E>>(
			[]()-> std::vector<E> {
				return std::vector<E>();
			},
			[](std::vector<E> accumulator, E element, Timestamp index)-> std::vector<E> {
				accumulator.push_back(element);
				return accumulator;
			},
			[](std::vector<E> a, std::vector<E> b)-> std::vector<E> {
				a.insert(a.end(), b.begin(), b.end());
				return a;
			},
			[](std::vector<E> accumulator)-> std::vector<E> {
				return accumulator;
			}
		);
	}

	template<typename E>
	Collector<E, std::list<E>, std::list<E>> useToList() {
		return useFull<E, std::list<E>, std::list<E>>(
			[]()-> std::list<E> {
				return std::list<E>();
			},
			[](std::list<E> accumulator, E element, Timestamp index)-> std::list<E> {
				accumulator.push_back(element);
				return accumulator;
			},
			[](std::list<E> a, std::list<E> b)-> std::list<E> {
				a.splice(a.end(), b);
				return a;
			},
			[](std::list<E> accumulator)-> std::list<E> {
				return accumulator;
			}
		);
	}

	template<typename E, typename D>
	Collector<E, D, D> useSummate(const Function<E, D>& mapper) {
		return useFull<E, D, D>(
			[]()-> D {
				return D{};
			},
			[mapper](D accumulator, E element, Timestamp index)-> D {
				return accumulator + mapper(element);
			},
			[](D a, D b)-> D {
				return a + b;
			},
			[](D accumulator)-> D {
				return accumulator;
			}
		);
	}

	template<typename E, typename D>
	Collector<E, std::pair<D, Module>, D> useAverage(const Function<E, D>& mapper) {
		return useFull<E, std::pair<D, Module>, D>(
			[]()-> std::pair<D, Module> {
				return std::make_pair(D{}, 0);
			},
			[mapper](std::pair<D, Module> accumulator, E element, Timestamp index)-> std::pair<D, Module> {
				D value = mapper(element);
				return std::make_pair(accumulator.first + value, accumulator.second + 1);
			},
			[](std::pair<D, Module> a, std::pair<D, Module> b)-> std::pair<D, Module> {
				return std::make_pair(a.first + b.first, a.second + b.second);
			},
			[](std::pair<D, Module> accumulator)-> D {
				if (accumulator.second == 0) {
					return D{};
				}
				return accumulator.first / static_cast<D>(accumulator.second);
			}
		);
	}

	template<typename E>
	Collector<E, std::map<E, Module>, std::map<E, Module>> useFrequency() {
		return useFull<E, std::map<E, Module>, std::map<E, Module>>(
			[]()-> std::map<E, Module> {
				return std::map<E, Module>();
			},
			[](std::map<E, Module> accumulator, E element, Timestamp index)-> std::map<E, Module> {
				accumulator[element]++;
				return accumulator;
			},
			[](std::map<E, Module> a, std::map<E, Module> b)-> std::map<E, Module> {
				for (const auto& [key, count] : b) {
					a[key] += count;
				}
				return a;
			},
			[](std::map<E, Module> accumulator)-> std::map<E, Module> {
				return accumulator;
			}
		);
	}

	template<typename E, typename D>
	Collector<E, std::map<D, Module>, std::map<D, Module>> useFrequency(const Function<E, D>& mapper) {
		return useFull<E, std::map<D, Module>, std::map<D, Module>>(
			[]()-> std::map<D, Module> {
				return std::map<D, Module>();
			},
			[mapper](std::map<D, Module> accumulator, E element, Timestamp index)-> std::map<D, Module> {
				D key = mapper(element);
				accumulator[key]++;
				return accumulator;
			},
			[](std::map<D, Module> a, std::map<D, Module> b)-> std::map<D, Module> {
				for (const auto& [key, count] : b) {
					a[key] += count;
				}
				return a;
			},
			[](std::map<D, Module> accumulator)-> std::map<D, Module> {
				return accumulator;
			}
		);
	}
};

namespace collectable {
	using Timestamp = functional::Timestamp;
	using Module = functional::Module;

	using Runnable = functional::Runnable;

	template<typename R>
	using Supplier = functional::Supplier<R>;

	template<typename T, typename R>
	using Function = functional::Function<T, R>;

	template<typename T, typename U, typename R>
	using BiFunction = functional::BiFunction<T, U, R>;

	template<typename T, typename U, typename V, typename R>
	using TriFunction = functional::TriFunction<T, U, V, R>;

	template<typename T>
	using Consumer = functional::Consumer<T>;

	template<typename T, typename U>
	using BiConsumer = functional::BiConsumer<T, U>;

	template <typename T, typename U, typename V>
	using TriConsumer = functional::TriConsumer<T, U, V>;

	template<typename T>
	using Predicate = functional::Predicate<T>;

	template<typename T, typename U>
	using BiPredicate = functional::BiPredicate<T, U>;

	template<typename T, typename U, typename V>
	using TriPredicate = functional::TriPredicate<T, U, V>;

	template<typename T, typename U>
	using Comparator = functional::Comparator<T, U>;

	template<typename T>
	using Generator = functional::Generator<T>;

	template<typename E, typename A, typename R>
	using Collector = collector::Collector<E, A, R>;

	template<typename E>
	class Collectable {
	protected:
		Module concurrent;
	public:
		Collectable(const Module& concurrent): concurrent(concurrent) {}

		bool anyMatch(const BiPredicate<E, Timestamp>& predicate) const {
			Collector<E, bool, bool> collector = collector::useAnyMatch(predicate);
			return collector.collect(this->source(), this->concurrent);
		}

		bool allMatch(const BiPredicate<E, Timestamp>& predicate) const {
			Collector<E, bool, bool> collector = collector::useAllMatch(predicate);
			return collector.collect(this->source(), this->concurrent);
		}

		template<typename A, typename R>
		R collect(const Supplier<R>& identity, const BiFunction<A, E, A>& accumulator, const  BiFunction<A, A, A>& combiner, const Function<A, R>& finisher) const {
			Collector<E, A, R> collector = collector::useCollect(identity, accumulator, combiner, finisher);
			return collector.collect(this->source(), this->concurrent);
		}

		template<typename A, typename R>
		R collect(const Supplier<R>& identity, const TriPredicate<E, Timestamp, A>& interrupt, const BiFunction<A, E, A>& accumulator, const  BiFunction<A, A, A>& combiner, const Function<A, R>& finisher) const {
			Collector<E, A, R> collector = collector::useCollect(identity, interrupt, accumulator, combiner, finisher);
			return collector.collect(this->source(), this->concurrent);
		}

		Module count() const {
			Collector<E, Module, Module> collector = collector::useCount();
			return collector.collect(this->source(), this->concurrent);
		}

		void error() const {
			Collector<E, std::string, std::string> collector = collector::useError();
			return collector.collect(this->source(), this->concurrent);
		}

		void error(const std::string& prefix, const std::string& delimiter, const std::string &suffix) const {
			Collector<E, std::string, std::string> collector = collector::useError(prefix, delimiter, suffix);
			return collector.collect(this->source(), this->concurrent);
		}

		void error(const std::string& prefix, const BiFunction<std::string, E, std::string>& serializer, const std::string& suffix, const BiFunction<std::string, std::string, std::string>& combiner) const {
			Collector<E, std::string, std::string> collector = collector::useError(prefix, serializer, suffix, combiner);
			return collector.collect(this->source(), this->concurrent);
		}

		bool empty() const {
			Collector<E, Module, Module> collector = collector::useCount();
			return collector.collect(this->source(), this->concurrent) == 0;
		}

		std::optional<E> findAny() const {
			Collector<E, std::optional<E>, std::optional<E>> collector = collector::useFindAny();
			return collector.collect(this->source(), this->concurrent);
		}

		std::optional<E> findAt(const Timestamp& index) const {
			Collector<E, std::optional<E>, std::optional<E>> collector = collector::useFindAt(index);
			return collector.collect(this->source(), this->concurrent);
		}

		std::optional<E> findFirst() const {
			Collector<E, std::optional<E>, std::optional<E>> collector = collector::useFindFirst();
			return collector.collect(this->source(), this->concurrent);
		}

		std::optional<E> findLast() const {
			Collector<E, std::vector<E>, std::optional<E>> collector = collector::useFindLast();
			return collector.collect(this->source(), this->concurrent);
		}

		std::optional<E> findMaximum() const {
			Collector<E, std::optional<E>, std::optional<E>> collector = collector::useFindMaximum();
			return collector.collect(this->source(), this->concurrent);
		}

		std::optional<E> findMinimum() const {
			Collector<E, std::optional<E>, std::optional<E>> collector = collector::useFindMinimum();
			return collector.collect(this->source(), this->concurrent);
		}

		void forEach(const BiConsumer<E, Timestamp>& action) const {
			Collector<E, Module, Module> collector = collector::useForEach(action);
			return collector.collect(this->source(), this->concurrent);
		}

		template<typename K>
		auto group(const BiFunction<E, Timestamp, K>& keyExtractor) const -> std::unordered_map<K, std::vector<E>> {
			Collector<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>> collector = collector::useGroup(keyExtractor);
			return collector.collect(this->source(), this->concurrent);
		}

		template<typename K, typename V>
		auto groupBy(const BiFunction<E, Timestamp, K>& keyExtractor, const BiFunction<E, Timestamp, V>& valueExtractor) const -> std::unordered_map<K, std::vector<V>> {
			Collector<E, std::unordered_map<K, std::vector<V>>, std::unordered_map<K, std::vector<V>>> collector = collector::useGroupBy(keyExtractor, valueExtractor);
			return collector.collect(this->source(), this->concurrent);
		}

		std::string join() const {
			Collector<E, std::string, std::string> collector = collector::useJoin();
			return collector.collect(this->source(), this->concurrent);
		}

		std::string join(const std::string& prefix, const std::string& delimiter, const std::string& suffix) const {
			Collector<E, std::string, std::string> collector = collector::useJoin(prefix, delimiter, suffix);
			return collector.collect(this->source(), this->concurrent);
		}

		std::string join(const std::string& prefix, const BiFunction<std::string, E, std::string>& serializer, const std::string suffix, const BiFunction<std::string, std::string, std::string>& combiner) const {
			Collector<E, std::string, std::string> collector = collector::useJoin(prefix, serializer, suffix, combiner);
			return collector.collect(this->source(), this->concurrent);
		}

		bool noneMatch(const BiPredicate<E, Timestamp>& predicate) const {
			Collector<E, bool, bool> collector = collector::useNoneMatch(predicate);
			return collector.collect(this->source(), this->concurrent);
		}

		std::vector<std::vector<E>> partition(const Module& size) const {
			Collector<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>> collector = collector::usePartition(size);
			return collector.collect(this->source(), this->concurrent);
		}

		std::vector<std::vector<E>> partitionBy(const Module& size) const {
			Collector<E, std::map<Timestamp, std::vector<E>>, std::vector<std::vector<E>>> collector = collector::usePartitionBy(size);
			return collector.collect(this->source(), this->concurrent);
		}

		std::optional<E> reduce(const BiFunction<E, E, E>& accumulator) const {
			Collector<E, std::optional<E>, std::optional<E>> collector = collector::useReduce(accumulator);
			return collector.collect(this->source(), this->concurrent);
		}

		E reduce(const E& identity, const BiFunction<E, E, E>& accumulator) const {
			Collector < E, E, E> collector = collector::useReduce(identity, accumulator);
			return collector.collect(this->source(), this->concurrent);
		}

		template<typename R>
		auto reduce(const R& identity, const BiFunction<R, E, R>& accumulator, const BiFunction<R, R, R>& combiner) const -> R {
			Collector<E, R, R> collector = collector::useReduce(identity, accumulator, combiner);
			return collector.collect(this->source(), this->concurrent);
		}

		virtual Generator<E> source() = 0;

		std::list<E> toList() const {
			Collector<E, std::list<E>, std::list<E>> collector = collector::useToList();
			return collector.collect(this->source(), this->concurrent);
		}

		template<typename K>
		auto toMap(const BiFunction<E, Timestamp, K> &keyExtractor) const -> std::map<K, E> {
			Collector<E, std::map<K, E>, std::map<K, E>> collector = collector::useToMap(keyExtractor);
			return collector.collect(this->source(), this->concurrent);
		}

		template<typename K, typename V>
		auto toMap(const BiFunction<E, Timestamp, K>& keyExtractor, const BiFunction<E, Timestamp, V>& valueExtractor) const -> std::map<K, V> {
			Collector<E, std::map<K, E>, std::map<K, E>> collector = collector::useToMap(keyExtractor, valueExtractor);
			return collector.collect(this->source(), this->concurrent);
		}

		 std::set<E> toSet() const {
			Collector<E, std::set<E>, std::set<E>> collector = collector::useToSet();
			return collector.collect(this->source(), this->concurrent);
		}

		 std::vector<E> toVector() const {
			 Collector<E, std::vector<E>, std::vector<E>> collector = collector::useToVector();
			 return collector.collect(this->source(), this->concurrent);
		 }

		void out() const {
			Collector<E, std::string, std::string> collector = collector::useOut();
			collector.collect(this->source(), this->concurrent);
		}

		void out(const std::string& prefix, const std::string& delimiter, const std::string& suffix) const {
			Collector<E, std::string, std::string> collector = collector::useOut(prefix, delimiter, suffix);
			collector.collect(this->source(), this->concurrent);
		}

		void out(const std::string& prefix, const BiFunction<std::string, E, std::string>& serializer, const std::string suffix, const BiFunction<std::string, std::string, std::string>& combiner) const {
			Collector<E, std::string, std::string> collector = collector::useOut(prefix, serializer, suffix, combiner);
			collector.collect(this->source(), this->concurrent);
		}
	};

	template<typename E>
	class OrderedCollectable :public Collectable<E> {
	protected:
		std::map<Timestamp, E> buffer;
	public:

		OrderedCollectable(const Generator<E> &generator): Collectable<E>(1) {
		    std::set<std::pair<Timestamp, E>> buffer;
			generator([&buffer](E element, Timestamp index)->void {
				buffer.insert(std::make_pair(index, element));
			}, [](E element, Timestamp index)-> bool {
				return false;
			});
			for (std::pair<Timestamp, E> pair : buffer) {
				Timestamp index = pair.first < 0 ? (buffer.size() - (std::abs(pair.first) % buffer.size())) : (pair.first % buffer.size());
				this->buffer.insert(std::make_pair(index, pair.second));
			}
		}

		OrderedCollectable(const Generator<E>& generator, const Module& concurrent) : Collectable<E>(concurrent) {
			std::set<std::pair<Timestamp, E>> buffer;
			generator([&buffer](E element, Timestamp index)->void {
				buffer.insert(std::make_pair(index, element));
				}, [](E element, Timestamp index)-> bool {
					return false;
					});
				for (std::pair<Timestamp, E> pair : buffer) {
					Timestamp index = pair.first < 0 ? (buffer.size() - (std::abs(pair.first) % buffer.size())) : (pair.first % buffer.size());
					this->buffer.insert(std::make_pair(index, pair.second));
				}
		}

		OrderedCollectable(const Generator<E>& generator, const Comparator<E, E>& comparator) : Collectable<E>(1) {
			std::set<std::pair<Timestamp, E>> buffer([comparator](std::pair<Timestamp, E> a, std::pair<Timestamp, E> b)-> Timestamp {
				return comparator(a, b);
			});
			generator([&buffer](E element, Timestamp index)->void {
				buffer.insert(std::make_pair(index, element));
			}, [](E element, Timestamp index)-> bool {
				return false;
			});
			for (std::pair<Timestamp, E> pair : buffer) {
				Timestamp index = pair.first < 0 ? (buffer.size() - (std::abs(pair.first) % buffer.size())) : (pair.first % buffer.size());
				this->buffer.insert(std::make_pair(index, pair.second));
			}
		}

		OrderedCollectable(const Generator<E>& generator, const Comparator<E, E>& comparator, const Module& concurrent) : Collectable<E>(concurrent) {
			std::set<std::pair<Timestamp, E>> buffer([comparator](std::pair<Timestamp, E> a, std::pair<Timestamp, E> b)-> Timestamp {
				return comparator(a, b);
			});
			generator([&buffer](E element, Timestamp index)->void {
				buffer.insert(std::make_pair(index, element));
				}, [](E element, Timestamp index)-> bool {
					return false;
				});
				for (std::pair<Timestamp, E> pair : buffer) {
					Timestamp index = pair.first < 0 ? (buffer.size() - (std::abs(pair.first) % buffer.size())) : (pair.first % buffer.size());
					this->buffer.insert(std::make_pair(index, pair.second));
				}
		}

		virtual Generator<E> source() override {
			return [](BiConsumer<E, Timestamp> accept, BiPredicate<E, Timestamp> interrupt)-> void {
				for (std::pair<Timestamp, E> pair : buffer) {
					if (interrupt(pair.first, pair.second)) {
						break;
					}
					accept(pair.second, pair.first);
				}
			};
		}
	};

	template<typename E, typename D>
	class Statistics : public OrderedCollectable<E> {
	public:
		Statistics(const Module& concurrent) : OrderedCollectable<E>(concurrent) {}
		Statistics(const Generator<E> &generator, const Module& concurrent) : OrderedCollectable<E>(generator, concurrent) {}
	};

	template<typename E>
	class WindowCollectable : public OrderedCollectable<E> {
	public:
		WindowCollectable(const Module& concurrent): OrderedCollectable<E>(concurrent) {}
		WindowCollectable(const Generator<E>& generator, const Module& concurrent) : OrderedCollectable<E>(generator, concurrent) {}
		semantic::Semantic<Semantic<E>> slide(const Module& size, const Timestamp& step) const {
		
		}
		semantic::Semantic<Semantic<E>> tumble(const Module& size) const {
		
		}
	};

	template<typename E>
	class UnorderedCollectable : public OrderedCollectable<E> {
	protected:
		std::unordered_map<Timestamp, E> buffer;
	public:
		UnorderedCollectable(const Generator<E>& generator) : Collectable<E>(1) {
			generator([this](E element, Timestamp index)->void {
				this-> buffer.insert(std::make_pair(index, element));
			}, [](E element, Timestamp index)-> bool {
				return false;
			});
		}

		UnorderedCollectable(const Generator<E>& generator, const Module& concurrent) : Collectable<E>(concurrent) {
			generator([this](E element, Timestamp index)->void {
				this->buffer.insert(std::make_pair(index, element));
				}, [](E element, Timestamp index)-> bool {
					return false;
				});
		}

		OrderedCollectable<E>& operator=(const OrderedCollectable<E>& other) {
			if (this != &other) {
				Collectable<E>::operator=(other);
				buffer = other.buffer;
			}
			return *this;
		}

		OrderedCollectable<E>& operator=(OrderedCollectable<E>&& other) noexcept {
			if (this != &other) {
				Collectable<E>::operator=(std::move(other));
				buffer = std::move(other.buffer);
			}
			return *this;
		}

		virtual Generator<E> source() override {
			return [](BiConsumer<E, Timestamp> accept, BiPredicate<E, Timestamp> interrupt)-> void {
				for (std::pair<Timestamp, E> pair : buffer) {
					if (interrupt(pair.first, pair.second)) {
						break;
					}
					accept(pair.second, pair.first);
				}
				};
		}
	};
};

namespace semantic {

	using Timestamp = functional::Timestamp;
	using Module = functional::Module;

	using Runnable = functional::Runnable;

	template<typename R>
	using Supplier = functional::Supplier<R>;

	template<typename T, typename R>
	using Function = functional::Function<T, R>;

	template<typename T, typename U, typename R>
	using BiFunction = functional::BiFunction<T, U, R>;

	template<typename T, typename U, typename V, typename R>
	using TriFunction = functional::TriFunction<T, U, V, R>;

	template<typename T>
	using Consumer = functional::Consumer<T>;

	template<typename T, typename U>
	using BiConsumer = functional::BiConsumer<T, U>;

	template <typename T, typename U, typename V>
	using TriConsumer = functional::TriConsumer<T, U, V>;

	template<typename T>
	using Predicate = functional::Predicate<T>;

	template<typename T, typename U>
	using BiPredicate = functional::BiPredicate<T, U>;

	template<typename T, typename U, typename V>
	using TriPredicate = functional::TriPredicate<T, U, V>;

	template<typename T>
	using Generator = functional::Generator<T>;

	template<typename E>
	class Semantic {
	private:
		std::unique_ptr<Generator<E>> generator;
		const Module concurrent;
	public:
		Semantic(const Generator<E>& generator) : generator(std::make_unique<Generator<E>>(generator)), concurrent(1) {}
		Semantic(const Generator<E>& generator, const Module& concurrent) : generator(std::make_unique<Generator<E>>(generator)), concurrent(concurrent) {}


	};


};
