#include "semantic.h"
namespace semantic
{
template <typename D>
D randomly(const D &start, const D &end)
{
	static_assert(std::is_arithmetic<D>::value, "D must be an arithmetic type");
	std::random_device rd;
	std::mt19937 gen(rd());
	if constexpr (std::is_integral<D>::value)
	{
		std::uniform_int_distribution<D> dis(start, end);
		return dis(gen);
	}
	else
	{
		std::uniform_real_distribution<D> dis(start, end);
		return dis(gen);
	}
}

ThreadPool::ThreadPool(Module threads) : threads(threads), stop(false)
{
	try
	{
		for (Module i = 0; i < threads; ++i)
		{
			workers.emplace_back([this] {
				for (;;)
				{
					Runnable task;
					{
						std::unique_lock<std::mutex> lock(m_queue_mutex);
						condition.wait(lock, [this] { return stop || !tasks.empty(); });
						if (stop && tasks.empty())
						{
							return;
						}
						task = std::move(tasks.front());
						tasks.pop();
					}
					task();
				}
			});
		}
	}
	catch (...)
	{
		stop = true;
		condition.notify_all();
		for (auto &worker : workers)
		{
			if (worker.joinable())
			{
				worker.join();
			}
		}
		throw;
	}
}

ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(m_queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (auto &worker : workers)
	{
		if (worker.joinable())
		{
			worker.join();
		}
	}
}

template <class F>
auto ThreadPool::submit(F &&f) -> std::future<typename std::result_of<F()>::type>
{
	using return_type = typename std::result_of<F()>::type;
	auto task = std::make_shared<std::packaged_task<return_type()>>(
		std::forward<F>(f));
	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(m_queue_mutex);
		if (stop)
		{
			throw std::runtime_error("submit on stopped ThreadPool");
		}
		tasks.emplace([task]() {
			(*task)();
		});
	}
	condition.notify_one();
	return res;
}

template std::future<void> ThreadPool::submit(std::function<void()> &&f);

Module ThreadPool::getThreadCount()
{
	return threads;
}

Module ThreadPool::getTaskQueueSize()
{
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	return tasks.size();
}

template <typename E, typename D>
Module Statistics<E, D>::count() const
{
	return elements.size();
}

template <typename E, typename D>
E Statistics<E, D>::maximum(const Comparator<E, E> &comparator) const
{
	std::string key = "maximum";
	auto it = cache.find(key);
	if (it != cache.end())
	{
		return static_cast<E>(it->second);
	}
	E result = *std::max_element(elements.begin(), elements.end(),
								 [&comparator](const E &a, const E &b) { return comparator(a, b) < 0; });
	cache[key] = static_cast<D>(result);
	return result;
}

template <typename E, typename D>
E Statistics<E, D>::minimum(const Comparator<E, E> &comparator) const
{
	std::string key = "minimum";
	auto it = cache.find(key);
	if (it != cache.end())
	{
		return static_cast<E>(it->second);
	}
	E result = *std::min_element(elements.begin(), elements.end(),
								 [&comparator](const E &a, const E &b) { return comparator(a, b) < 0; });
	cache[key] = static_cast<D>(result);
	return result;
}

template <typename E, typename D>
D Statistics<E, D>::range(const Function<E, D> &mapper) const
{
	std::string key = "range";
	auto it = cache.find(key);
	if (it != cache.end())
	{
		return it->second;
	}
	std::vector<D> values;
	std::transform(elements.begin(), elements.end(), std::back_inserter(values), mapper);
	D result = *std::max_element(values.begin(), values.end()) - *std::min_element(values.begin(), values.end());
	cache[key] = result;
	return result;
}

template <typename E, typename D>
D Statistics<E, D>::variance(const Function<E, D> &mapper) const
{
	std::string key = "variance";
	auto it = cache.find(key);
	if (it != cache.end())
	{
		return it->second;
	}
	std::vector<D> values;
	std::transform(elements.begin(), elements.end(), std::back_inserter(values), mapper);
	D mean_val = std::accumulate(values.begin(), values.end(), D{}) / values.size();
	D sum_sq = D{};
	for (const auto &v : values)
	{
		sum_sq += (v - mean_val) * (v - mean_val);
	}
	D result = values.size() > 1 ? sum_sq / (values.size() - 1) : D{};
	cache[key] = result;
	return result;
}

template <typename E, typename D>
D Statistics<E, D>::standardDeviation(const Function<E, D> &mapper) const
{
	std::string key = "standardDeviation";
	auto it = cache.find(key);
	if (it != cache.end())
	{
		return it->second;
	}
	D result = std::sqrt(variance(mapper));
	cache[key] = result;
	return result;
}

template <typename E, typename D>
D Statistics<E, D>::mean(const Function<E, D> &mapper) const
{
	std::string key = "mean";
	auto iterator = cache.find(key);
	if (iterator != cache.end())
	{
		return iterator->second;
	}
	D sum = std::accumulate(elements.begin(), elements.end(), D{},
							[&mapper](const D &accumulator, const E &element) {
								return accumulator + mapper(element);
							});
	D result = elements.size() > 0 ? sum / static_cast<D>(elements.size()) : D{};
	cache[key] = result;
	return result;
}

template <typename E, typename D>
D Statistics<E, D>::median(const Function<E, D> &mapper) const
{
	std::string key = "median";
	auto it = cache.find(key);
	if (it != cache.end())
	{
		return it->second;
	}
	std::vector<D> values;
	std::transform(elements.begin(), elements.end(), std::back_inserter(values), mapper);
	std::sort(values.begin(), values.end());
	D result;
	size_t n = values.size();
	if (n == 0)
	{
		result = D{};
	}
	else if (n % 2 == 0)
	{
		result = (values[n / 2 - 1] + values[n / 2]) / 2;
	}
	else
	{
		result = values[n / 2];
	}
	cache[key] = result;
	return result;
}

template <typename E, typename D>
D Statistics<E, D>::mode(const Function<E, D> &mapper) const
{
	std::string key = "mode";
	auto it = cache.find(key);
	if (it != cache.end())
	{
		return it->second;
	}
	std::map<D, Module> freq_map;
	for (const auto &element : elements)
	{
		D value = mapper(element);
		freq_map[value]++;
	}
	D result = D{};
	Module max_count = 0;
	for (const auto &pair : freq_map)
	{
		if (pair.second > max_count)
		{
			max_count = pair.second;
			result = pair.first;
		}
	}
	cache[key] = result;
	return result;
}

template <typename E, typename D>
std::map<D, Module> Statistics<E, D>::frequency(const Function<E, D> &mapper) const
{
	std::string key = "frequency";
	auto it = frequencyCache.find(key);
	if (it != frequencyCache.end())
	{
		return it->second;
	}
	std::map<D, Module> result;
	for (const auto &element : elements)
	{
		D value = mapper(element);
		result[value]++;
	}
	frequencyCache[key] = result;
	return result;
}

template <typename E, typename D>
D Statistics<E, D>::sum(const Function<E, D> &mapper) const
{
	std::string key = "sum";
	auto iterator = cache.find(key);
	if (iterator != cache.end())
	{
		return iterator->second;
	}
	D result = std::accumulate(elements.begin(), elements.end(), D{},
							   [&mapper](const D &accumulator, const E &element) {
								   return accumulator + mapper(element);
							   });
	cache[key] = result;
	return result;
}

template <typename E, typename D>
std::vector<D> Statistics<E, D>::quartiles(const Function<E, D> &mapper) const
{
	std::string key = "quartiles";
	auto it = cache.find(key);
	if (it != cache.end())
	{
		return std::vector<D>{it->second}; // Simplified for caching
	}
	std::vector<D> values;
	std::transform(elements.begin(), elements.end(), std::back_inserter(values), mapper);
	std::sort(values.begin(), values.end());
	std::vector<D> result(3);
	size_t n = values.size();
	result = n > 0 ? values[n / 4] : D{};
	result = n > 0 ? values[n / 2] : D{};
	result = n > 0 ? values[3 * n / 4] : D{};
	cache[key] = result; // Simplified caching
	return result;
}

template <typename E, typename D>
D Statistics<E, D>::interquartileRange(const Function<E, D> &mapper) const
{
	std::string key = "interquartileRange";
	auto it = cache.find(key);
	if (it != cache.end())
	{
		return it->second;
	}
	std::vector<D> q = quartiles(mapper);
	D result = q - q;
	cache[key] = result;
	return result;
}

template <typename E, typename D>
D Statistics<E, D>::skewness(const Function<E, D> &mapper) const
{
	std::string key = "skewness";
	auto it = cache.find(key);
	if (it != cache.end())
	{
		return it->second;
	}
	std::vector<D> values;
	std::transform(elements.begin(), elements.end(), std::back_inserter(values), mapper);
	D mean_val = mean(mapper);
	D stddev_val = standardDeviation(mapper);
	D result = D{};
	if (stddev_val != D{})
	{
		D sum_cubed = D{};
		for (const auto &v : values)
		{
			sum_cubed += std::pow((v - mean_val) / stddev_val, 3);
		}
		result = values.size() > 2 ? sum_cubed / values.size() : D{};
	}
	cache[key] = result;
	return result;
}

template <typename E, typename D>
D Statistics<E, D>::kurtosis(const Function<E, D> &mapper) const
{
	std::string key = "kurtosis";
	auto it = cache.find(key);
	if (it != cache.end())
	{
		return it->second;
	}
	std::vector<D> values;
	std::transform(elements.begin(), elements.end(), std::back_inserter(values), mapper);
	D mean_val = mean(mapper);
	D stddev_val = standardDeviation(mapper);
	D result = D{};
	if (stddev_val != D{})
	{
		D sum_quad = D{};
		for (const auto &v : values)
		{
			sum_quad += std::pow((v - mean_val) / stddev_val, 4);
		}
		result = values.size() > 3 ? (sum_quad / values.size()) - 3 : D{};
	}
	cache[key] = result;
	return result;
}

template <typename E, typename D>
bool Statistics<E, D>::isEmpty() const
{
	return elements.empty();
}

template <typename E, typename D>
void Statistics<E, D>::clear()
{
	elements.clear();
	cache.clear();
	frequencyCache.clear();
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(const std::list<E> &l)
{
	elements.assign(l.begin(), l.end());
	cache.clear();
	frequencyCache.clear();
	return *this;
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(const std::vector<E> &v)
{
	elements = v;
	cache.clear();
	frequencyCache.clear();
	return *this;
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(std::initializer_list<E> l)
{
	elements.assign(l.begin(), l.end());
	cache.clear();
	frequencyCache.clear();
	return *this;
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(const Statistics &other)
{
	if (this != &other)
	{
		elements = other.elements;
		cache = other.cache;
		frequencyCache = other.frequencyCache;
	}
	return *this;
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(Statistics &&other) noexcept
{
	if (this != &other)
	{
		elements = std::move(other.elements);
		cache = std::move(other.cache);
		frequencyCache = std::move(other.frequencyCache);
	}
	return *this;
}

template <typename E>
bool Semantic<E>::anyMatch(const Predicate<E> &predicate) const
{
	if (this->concurrent < 2)
	{
		bool match = false;
		(*generator)([&](const E &element, Timestamp) -> void { 
		if(predicate(element)){
		 match = true;
		} }, [&](const E &) -> bool { return match; });
	}
	return collect([]() -> bool { return false; }, [&predicate](bool matched, const E &element) -> bool {
                if (matched){
                    return true;
                }
                return predicate(element); }, [](bool a, bool b) -> bool { return a || b; }, [](bool result) -> bool { return result; });
}

template <typename E>
bool Semantic<E>::allMatch(const Predicate<E> &predicate) const
{
	if (this->concurrent < 2)
	{
		bool mismatch = false;
		(*generator)([&](const E &element, Timestamp) -> void { 
		if(predicate(element)){
		 mismatch = true;
		} }, [&](const E &) -> bool { return mismatch; });
	}
	return collect([]() -> bool { return true; }, [&predicate](bool allMatched, const E &element) -> bool {
                if (!allMatched){
                    return false;
                }
                return predicate(element); }, [](bool a, bool b) -> bool { return a && b; }, [](bool result) -> bool { return result; });
}

template <typename E>
template <typename A, typename R>
R Semantic<E>::collect(const Supplier<A> &supplier, const BiFunction<A, E, A> &accumulator, const BiFunction<A, A, A> &combiner, const Function<A, R> &finisher) const
{
	if (concurrent < 2)
	{
		A result = supplier();
		(*generator)([&](const E &element, Timestamp) -> void { accumulator(result, element); }, [](const E &) -> bool { return false; });
		return finisher(result);
	}
	std::vector<std::future<A>> futures;
	std::atomic<Module> nextThread{0};
	A localResult = supplier();

	(*generator)([&](const E &element, Timestamp) -> void {
            Module threadId = nextThread++ % concurrent;
            if (threadId == 0) {
                accumulator(localResult, element);
            }else {
                futures.push_back(globalThreadPool.submit([=, &element]() {
                    A partial = supplier();
                    accumulator(partial, element);
                    return partial;
                }));
            } }, [](const E &) -> bool { return false; });
	A finalResult = localResult;
	for (auto &future : futures)
	{
		A partial = future.get();
		finalResult = combiner(finalResult, partial);
	}

	return finisher(finalResult);
}

template <typename E>
template <typename A, typename R>
R Semantic<E>::collect(const Collector<E, A, R> &c) const
{
	return this->collect(c.identity, c.accumulator, c.combiner, c.finisher);
}

template <typename E>
Semantic<E> Semantic<E>::concat(const Semantic<E> &other) const
{
	return Semantic<E>([this, &other](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) -> void {
		(*this->generator)([&](const E &element, Timestamp ts) -> void { accept(element, ts); }, interrupt);
		(*other.generator)([&](const E &element, Timestamp ts) -> void { accept(element, ts); }, interrupt);
	},
					   this->concurrent);
}

template <typename E>
void Semantic<E>::cout() const
{
	std::cout << '[';
	bool splitable = false;
	this->cout(std::cout, [&splitable](const E &element, std::ostream &stream) -> void {
		if(splitable){
			std::cout << ',';
		}else{
			splitable = true;
		}
		stream << element;
	});
	std::cout << ']';
}

template <typename E>
void Semantic<E>::cout(const BiFunction<E, std::ostream &, std::ostream &> &accumulator) const
{
	std::cout << '[';
	this->cout(std::cout, accumulator);
	std::cout << ']';
}

template <typename E>
void Semantic<E>::cout(std::ostream &stream) const
{
	this->cout(stream, [](const E &element, std::ostream &os) -> std::ostream & {
		return os << element << ',';
	});
}

template <typename E>
void Semantic<E>::cout(std::ostream &stream, const BiConsumer<E, std::ostream &> &accumulator) const
{
	if (concurrent < 2)
	{
		(*generator)([&](const E &element, Timestamp) -> void { accumulator(element, stream); }, [&](const E &) -> bool { return false; });
	}
	else
	{
		std::mutex streamMutex;
		std::vector<std::future<void>> futures;
		std::atomic<Module> nextThread{0};

		(*generator)([&](const E &element, Timestamp) {
            Module threadId = nextThread.fetch_add(1, std::memory_order_relaxed) % concurrent;
            if (threadId == 0) {
                std::lock_guard<std::mutex> lock(streamMutex);
                accumulator(element, stream);
            } else {
                futures.push_back(std::async(std::launch::async, [&]() {
                    std::lock_guard<std::mutex> lock(streamMutex);
                    accumulator(element, stream);
                }));
            } }, [](const E &) -> bool { return false; });

		for (auto &future : futures)
		{
			future.get();
		}
	}
}

template <typename E>
Module Semantic<E>::count() const
{
	return this->toSet().size();
}

template <typename E>
Semantic<E> Semantic<E>::distinct() const
{
	return this->distinct([](const E &a) -> Timestamp {
		return static_cast<Timestamp>(std::hash<E>{}(a));
	});
}

template <typename E>
Semantic<E> Semantic<E>::distinct(const Function<E, Timestamp> &identifier) const
{
	return Semantic<E>([this, identifier](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		std::unordered_set<Timestamp> seen;
		std::mutex seenMutex;
		(*generator)([&](const E &element, Timestamp index) {
			Timestamp id = identifier(element);
			std::lock_guard<std::mutex> lock(seenMutex);
			if (seen.find(id) == seen.end())
			{
				seen.insert(id);
				accept(element, index);
			}
		},
					 interrupt);
	},
					   concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::dropWhile(const Predicate<E> &p) const
{
	return Semantic<E>([this, p](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) -> void {
		bool dropping = true;
		(*generator)([&](const E &element, Timestamp index) {
            if (dropping && p(element)) {
                return;
            }
            dropping = false;
            accept(element, index); }, interrupt, [](const E &, Timestamp index) { return index; });
	},
					   this->concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::filter(const Predicate<E> &p) const
{
	return Semantic<E>([*this, &p](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) -> void {
		(*generator)([&](const E &element, const Timestamp &index) -> void {
            if (p(element))
            {
                accept(element, index);
            } }, interrupt);
	});
}

template <typename E>
void Semantic<E>::forEach(const Consumer<E> &c) const
{
	if (concurrent < 2)
	{
		(*generator)([&](const E &element, const Timestamp &) -> void { c(element); }, [&](const E &) -> bool { return false; });
	}
	std::mutex cMutex;
	std::vector<std::future<void>> futures;
	std::atomic<Module> nextThread{0};
	(*generator)([&](const E &element, const Timestamp &) {
            Module threadId = nextThread.fetch_add(1, std::memory_order_relaxed) % concurrent;
            if (threadId == 0)
            {
                c(element);
            }
            else
            {
                futures.push_back(std::async(std::launch::async, [element, &c]() { c(element); }));
            } }, [&](const E &) -> bool { return false; });

	for (auto &future : futures)
	{
		future.get();
	}
}

template <typename E>
std::optional<E> Semantic<E>::findFirst() const
{
	if (concurrent < 2)
	{
		std::optional<E> result;
		(*generator)([&](const E &element, Timestamp) {
            if (!result.has_value()) {
                result = element;
            } }, [&](const E &) -> bool { return result.has_value(); });
		return result;
	}
	std::optional<E> result;
	std::mutex resultMutex;
	std::atomic<bool> found{false};
	std::vector<std::future<void>> futures;

	for (Module t = 0; t < concurrent; ++t)
	{
		futures.push_back(globalThreadPool.submit([this, &result, &resultMutex, &found]() {
			(*generator)([&](const E &element, Timestamp) {
                    if (!found.exchange(true)) {
                        std::lock_guard<std::mutex> lock(resultMutex);
                        if (!result.has_value()) {
                            result = element;
                        }
                    } }, [&](const E &) -> bool { return found.load(); });
		}));
	}

	for (auto &future : futures)
	{
		future.wait();
	}
	return result;
}

template <typename E>
std::optional<E> Semantic<E>::findAny() const
{
	if (concurrent < 2)
	{
		std::optional<E> result;
		(*generator)([&](const E &element, Timestamp) {
            if (!result.has_value()) {
                result = element;
            } }, [&](const E &) -> bool { return result.has_value(); });
		return result;
	}

	std::optional<E> result;
	std::mutex resultMutex;
	std::atomic<bool> found{false};
	std::vector<std::future<void>> futures;

	for (Module t = 0; t < concurrent; ++t)
	{
		futures.push_back(globalThreadPool.submit([this, &result, &resultMutex, &found]() {
			(*generator)([&](const E &element, Timestamp) {
                    if (!found.exchange(true)) {
                        std::lock_guard<std::mutex> lock(resultMutex);
                        if (!result.has_value()) {
                            result = element;
                        }
                    } }, [&](const E &) -> bool { return found.load(); });
		}));
	}
	for (auto &future : futures)
	{
		future.wait();
	}
	return result;
}

template <typename E>
Semantic<E> Semantic<E>::flat(const Function<E, Semantic<E>> &mapper) const
{
	return Semantic<E>([this, mapper](const BiConsumer<E, Timestamp> accept, const Predicate<E> interrupt) {
		(*generator)([&](const E &element, Timestamp timestamp) {
			Semantic<E> innerStream = mapper(element);
			(*innerStream.generator)([&](const E &innerElement, Timestamp innerTimestamp) {
				accept(innerElement, innerTimestamp);
			},
									 interrupt);
		},
					 interrupt);
	});
}

template <typename E>
template <typename R>
Semantic<R> Semantic<E>::flatMap(const Function<E, Semantic<R>> &mapper) const
{
	return Semantic<R>([this, mapper](const BiConsumer<R, Timestamp> &accept, const Predicate<R> &interrupt) {
		(*generator)([&](const E &element, Timestamp timestamp) {
			Semantic<R> innerStream = mapper(element);
			(*innerStream.generator)([&](const R &innerElement, Timestamp innerTimestamp) {
				accept(innerElement, innerTimestamp);
			},
									 interrupt);
		},
					 interrupt);
	});
}

template <typename E>
template <typename K>
std::map<K, std::vector<E>> Semantic<E>::group(const Function<E, K> &classifier) const
{
	return this->collect<std::map<K, std::vector<E>>, std::map<K, std::vector<E>>>([]() { return std::map<K, std::vector<E>>{}; }, [classifier](std::map<K, std::vector<E>> &accumulator, const E &element) { accumulator[classifier(element)].push_back(element); }, [](std::map<K, std::vector<E>> left, const std::map<K, std::vector<E>> &right) {
            for (const auto &pair : right)
            {
                auto &vector = left[pair.first];
                vector.insert(vector.end(), pair.second.begin(), pair.second.end());
            }
            return left; }, [](std::map<K, std::vector<E>> result) { return result; });
}

template <typename E>
template <typename K, typename V>
std::map<K, std::vector<V>> Semantic<E>::groupBy(
	const Function<E, K> &keyExtractor,
	const Function<E, V> &valueExtractor) const
{
	return this->collect<std::map<K, std::vector<V>>, std::map<K, std::vector<V>>>(
		[]() {
			return std::map<K, std::vector<V>>{};
		},
		[keyExtractor, valueExtractor](std::map<K, std::vector<V>> &accumulator, const E &element) {
			accumulator[keyExtractor(element)].push_back(valueExtractor(element));
		},
		[](std::map<K, std::vector<V>> left, const std::map<K, std::vector<V>> &right) {
			for (const auto &pair : right)
			{
				auto &vector = left[pair.first];
				vector.insert(vector.end(), pair.second.begin(), pair.second.end());
			}
			return left;
		},
		[](std::map<K, std::vector<V>> result) {
			return result;
		});
}

template <typename E>
Semantic<E> Semantic<E>::limit(const Module &n) const
{
	return Semantic<E>([this, n](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		Module count = 0;
		(*generator)([&](const E &element, Timestamp timestamp) {
            if (count < n) {
                accept(element, timestamp);
                ++count;
            } }, [&](const E &element) -> bool { return count >= n || interrupt(element); });
	});
}

template <typename E>
template <typename R>
Semantic<R> Semantic<E>::map(const Function<E, R> &mapper) const
{
	return Semantic<R>([this, mapper](const BiConsumer<R, Timestamp> accept, const Predicate<R> &interrupt) {
		(*generator)([&](const E &element, Timestamp timestamp) {
			accept(mapper(element), timestamp);
		},
					 interrupt);
	});
}

template <typename E>
bool Semantic<E>::noneMatch(const Predicate<E> &p) const
{
	if (concurrent < 2)
	{
		bool found = false;
		(*generator)([&](const E &element, Timestamp) {
            if (p(element)) {
                found = true;
            } }, [&](const E &) -> bool { return found; });
		return !found;
	}
	std::atomic<bool> anyMatch{false};
	std::vector<std::future<void>> futures;

	for (Module t = 0; t < concurrent; ++t)
	{
		futures.push_back(globalThreadPool.submit([this, &anyMatch, &p]() {
			(*generator)([&](const E &element, Timestamp) {
                    if (p(element)) {
                        anyMatch.store(true);
                    } }, [&]() -> bool { return anyMatch.load(); });
		}));
	}
	for (auto &f : futures)
	{
		f.wait();
	}
	return !anyMatch.load();
}

template <typename E>
Semantic<E> Semantic<E>::parallel() const
{
	return Semantic<E>(generator, std::max<Module>(2, std::thread::hardware_concurrency()));
}

template <typename E>
Semantic<E> Semantic<E>::parallel(const Module &threadCount) const
{
	return Semantic<E>(generator, threadCount);
}

template <typename E>
std::vector<std::vector<E>> Semantic<E>::partition(const Module &count) const
{
	return this->collect<std::vector<std::vector<E>>, std::vector<std::vector<E>>>(
		[count]() {
			return std::vector<std::vector<E>>(count);
		},
		[count](std::vector<std::vector<E>> &accumulator, const E &element) {
			static std::atomic<Module> counter{0};
			Module index = counter.fetch_add(1) % count;
			accumulator[index].push_back(element);
		},
		[](std::vector<std::vector<E>> left, const std::vector<std::vector<E>> &right) {
			for (Module i = 0; i < left.size(); ++i)
			{
				left[i].insert(left[i].end(), right[i].begin(), right[i].end());
			}
			return left;
		},
		[](std::vector<std::vector<E>> result) {
			return result;
		});
}

template <typename E>
std::vector<std::vector<E>> Semantic<E>::partitionBy(const Function<E, Module> &classifier) const
{
	return this->collect<std::vector<std::vector<E>>, std::vector<std::vector<E>>>(
		[]() {
			return std::vector<std::vector<E>>{};
		},
		[classifier](std::vector<std::vector<E>> &accumulator, const E &element) {
			Module key = classifier(element);
			if (key >= accumulator.size())
			{
				accumulator.resize(key + 1);
			}
			accumulator[key].push_back(element);
		},
		[](std::vector<std::vector<E>> left, const std::vector<std::vector<E>> &right) {
			if (right.size() > left.size())
			{
				left.resize(right.size());
			}
			for (Module i = 0; i < right.size(); ++i)
			{
				left[i].insert(left[i].end(), right[i].begin(), right[i].end());
			}
			return left;
		},
		[](std::vector<std::vector<E>> result) {
			return result;
		});
}

template <typename E>
Semantic<E> Semantic<E>::peek(const Consumer<E> &c) const
{
	return Semantic<E>(
		[this, c](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
			(*generator)([&](const E &element, Timestamp index) {
				c(element);
				accept(element, index);
			},
						 interrupt);
		},
		concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::redirect(const BiFunction<E, Timestamp, Timestamp> &redirector) const
{
	return Semantic<E>(
		[this, redirector](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
			(*this->generator)(
				[&](const E &element, Timestamp index) {
					Timestamp redirectedIndex = redirector(element, index);
					accept(element, redirectedIndex);
				},
				interrupt);
		},
		this->concurrent);
}

template <typename E>
std::optional<E> Semantic<E>::reduce(const BiFunction<E, E, E> &accumulator) const
{
	return this->collect<std::optional<E>, std::optional<E>>(
		[]() -> std::optional<E> {
			return std::nullopt;
		},
		[&accumulator](std::optional<E> &currentResult, const E &element) {
			if (!currentResult.has_value())
			{
				currentResult = element;
			}
			else
			{
				currentResult = accumulator(currentResult.value(), element);
			}
		},
		[&accumulator](const std::optional<E> &left, const std::optional<E> &right) -> std::optional<E> {
			if (!left.has_value())
			{
				return right;
			}
			if (!right.has_value())
			{
				return left;
			}
			return std::optional<E>(accumulator(left.value(), right.value()));
		},
		[](std::optional<E> finalResult) -> std::optional<E> {
			return finalResult;
		});
}

template <typename E>
E Semantic<E>::reduce(const E &identity, const BiFunction<E, E, E> &accumulator) const
{
	return this->collect<E, E>(
		[&identity]() {
			return identity;
		},
		[&accumulator](E &currentValue, const E &element) {
			currentValue = accumulator(currentValue, element);
		},
		[&accumulator](const E &left, const E &right) {
			return accumulator(left, right);
		},
		[](E result) {
			return result;
		});
}

template <typename E>
template <typename R>
R Semantic<E>::reduce(const R &identity, const BiFunction<R, E, R> &accumulator, const BiFunction<R, R, R> &combiner) const
{
	return this->collect<R, R>(
		[&identity]() {
			return identity;
		},
		[&accumulator](R &currentResult, const E &element) {
			currentResult = accumulator(currentResult, element);
		},
		[&combiner](const R &left, const R &right) {
			return combiner(left, right);
		},
		[](R result) {
			return result;
		});
}

template <typename E>
Semantic<E> Semantic<E>::reverse() const
{
	return this->redirect([](const E &, Timestamp index) { return -index; });
}

template <typename E>
Semantic<E> Semantic<E>::shuffle() const
{
	return this->redirect([](const E &element, const Timestamp &index) {
		Timestamp h1 = std::hash<E>{}(element);
		Timestamp h2 = std::hash<Timestamp>{}(index);
		return h1 ^ (h2 << 1);
	});
}

template <typename E>
Semantic<E> Semantic<E>::shuffle(const Function<E, Timestamp> &mapper) const
{
	return this->redirect([&](const E &element, const Timestamp &index) {
		Timestamp base = mapper(element);
		Timestamp h1 = std::hash<E>{}(element);
		Timestamp h2 = std::hash<Timestamp>{}(index);
		Timestamp h3 = std::hash<Timestamp>{}(base);
		Timestamp combined = h1 ^ (h2 << 1) ^ (h3 << 2);
		return combined;
	});
}

template <typename E>
Semantic<E> Semantic<E>::skip(const Module &n) const
{
	return Semantic<E>(
		[this, n](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
			Module skippedCount = 0;
			(*this->generator)(
				[&](const E &element, Timestamp timestamp) {
					if (skippedCount >= n)
					{
						if (!interrupt(element))
						{
							accept(element, timestamp);
						}
					}
					else
					{
						++skippedCount;
					}
				},
				[&](const E &element) -> bool {
					return interrupt(element);
				});
		},
		this->concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::sorted() const
{
	static_assert(std::is_arithmetic<E>::value, "E must be a type cast into an unsigned long long.");
	return this->sorted([](const E &element) -> Timestamp {
		return static_cast<Timestamp>(element);
	});
}

template <typename E>
Semantic<E> Semantic<E>::sorted(const Function<E, Timestamp> &indexer) const
{
	return this->redirect([&indexer](const E &element, const Timestamp &index) -> Timestamp {
		return indexer(element);
	});
}

template <typename E>
Semantic<E> Semantic<E>::sub(const Module &start, const Module &end) const
{
	return Semantic<E>([this, start, end](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		Module index = 0;
		(*this->generator)(
			[&](const E &element, Timestamp ts) {
				if (index >= start && index < end)
				{
					if (interrupt(element))
						return;
					accept(element, ts);
				}
				++index;
			},
			[&](const E &element) -> bool {
				return interrupt(element);
			});
	},
					   this->concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::takeWhile(const Predicate<E> &p) const
{
	return Semantic<E>([this, p](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		bool shouldContinue = true;
		(*this->generator)(
			[&](const E &element, Timestamp ts) {
				if (!shouldContinue || interrupt(element))
				{
					return;
				}
				if (p(element))
				{
					accept(element, ts);
				}
				else
				{
					shouldContinue = false;
				}
			},
			[&](const E &element) -> bool {
				return !shouldContinue || interrupt(element);
			});
	},
					   this->concurrent);
}

template <typename E>
std::set<std::pair<Timestamp, E>> Semantic<E>::toIndexedSet() const
{
	if (concurrent < 2)
	{
		std::set<std::pair<Timestamp, E>> result;
		(*generator)(
			[&](const E &element, const Timestamp &timestamp) {
				result.emplace(timestamp, element);
			},
			[](const E &) -> bool {
				return false;
			});
		return result;
	}

	std::vector<std::vector<std::pair<Timestamp, E>>> threadLocalBuffers(concurrent);
	std::atomic<Module> sequenceCounter{0};

	(*generator)(
		[&](const E &element, const Timestamp &timestamp) {
			Module sequence = sequenceCounter.fetch_add(1, std::memory_order_relaxed);
			Module targetThread = sequence % concurrent;
			threadLocalBuffers[targetThread].emplace_back(timestamp, element);
		},
		[](const E &) -> bool {
			return false;
		});

	std::set<std::pair<Timestamp, E>> finalResult;
	std::mutex resultMutex;
	std::vector<std::thread> workerThreads;

	for (Module threadIndex = 0; threadIndex < concurrent; ++threadIndex)
	{
		workerThreads.emplace_back([&, threadIndex]() {
			for (const auto &item : threadLocalBuffers[threadIndex])
			{
				std::lock_guard<std::mutex> lock(resultMutex);
				finalResult.insert(item);
			}
		});
	}

	for (auto &thread : workerThreads)
	{
		thread.join();
	}

	return finalResult;
}

template <typename E>
std::list<E> Semantic<E>::toList() const
{
	return collect(
		[]() {
			return std::list<E>{};
		},
		[](std::list<E> &accumulator, const E &element) {
			accumulator.push_back(element);
		},
		[](std::list<E> left, const std::list<E> &right) {
			left.insert(left.end(), right.begin(), right.end());
			return left;
		},
		[](std::list<E> result) {
			return result;
		});
}

template <typename E>
template <typename K, typename V>
std::map<K, V> Semantic<E>::toMap(
	const Function<E, K> &keyExtractor,
	const Function<E, V> &valueExtractor) const
{
	return collect(
		[]() {
			return std::map<K, V>{};
		},
		[keyExtractor, valueExtractor](std::map<K, V> &accumulator, const E &element) {
			accumulator[keyExtractor(element)] = valueExtractor(element);
		},
		[](std::map<K, V> left, const std::map<K, V> &right) {
			for (const auto &pair : right)
			{
				left[pair.first] = pair.second;
			}
			return left;
		},
		[](std::map<K, V> result) {
			return result;
		});
}

template <typename E>
std::set<E> Semantic<E>::toSet() const
{
	return collect(
		[]() {
			return std::set<E>{};
		},
		[](std::set<E> &accumulator, const E &element) {
			accumulator.insert(element);
		},
		[](std::set<E> left, const std::set<E> &right) {
			for (const auto &element : right)
			{
				left.insert(element);
			}
			return left;
		},
		[](std::set<E> result) {
			return result;
		});
}

template <typename E>
std::vector<E> Semantic<E>::toVector() const
{
	return collect(
		[]() {
			return std::vector<E>{};
		},
		[](std::vector<E> &accumulator, const E &element) {
			accumulator.push_back(element);
		},
		[](std::vector<E> left, const std::vector<E> &right) {
			left.insert(left.end(), right.begin(), right.end());
			return left;
		},
		[](std::vector<E> result) {
			return result;
		});
}

template <typename E>
Semantic<E> Semantic<E>::translate(const Timestamp &offset) const
{
	return this->redirect([offset](const E &, const Timestamp &index) {
		return index + offset;
	});
}

template <typename E>
Semantic<E> empty()
{
	return Semantic<E>();
}

template <typename E, typename... Args>
Semantic<E> of(Args &&... args)
{
	std::vector<E> values = {std::forward<Args>(args)...};
	return from(values);
}

template <typename E>
Semantic<E> fill(const E &element, const Module &count)
{
	return Semantic<E>([element, count](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		for (Module i = 0; i < count; ++i)
		{
			if (interrupt(element))
			{
				break;
			}
			accept(element, i);
		}
	},
					   1);
}

template <typename E>
Semantic<E> fill(const Supplier<E> &supplier, const Module &count)
{
	return Semantic<E>([supplier, count](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		for (Module i = 0; i < count; ++i)
		{
			E value = supplier();
			if (interrupt(value))
			{
				break;
			}
			accept(value, i);
		}
	},
					   1);
}

template <typename E>
Semantic<E> from(const E *array, const Module &length)
{
	return Semantic<E>([array, length](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		for (Module i = 0; i < length; ++i)
		{
			const E &element = array[i];
			if (interrupt(element))
			{
				break;
			}
			accept(element, i);
		}
	},
					   1);
}

template <typename E, Module length>
Semantic<E> from(const std::array<E, length> &array)
{
	return Semantic<E>([array](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		for (Module i = 0; i < length; ++i)
		{
			const E &element = array[i];
			if (interrupt(element))
			{
				break;
			}
			accept(element, i);
		}
	},
					   1);
}

template <typename E>
Semantic<E> from(const std::list<E> &l)
{
	return Semantic<E>([l](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		Module i = 0;
		for (const auto &element : l)
		{
			if (interrupt(element))
			{
				break;
			}
			accept(element, i);
			i++;
		}
	},
					   1);
}

template <typename E>
Semantic<E> from(const std::vector<E> &v)
{
	return Semantic<E>([v](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		Module i = 0;
		for (const auto &element : v)
		{
			if (interrupt(element))
			{
				break;
			}
			accept(element, i);
			i++;
		}
	},
					   1);
}

template <typename E>
Semantic<E> from(const std::initializer_list<E> &l)
{
	return Semantic<E>([l](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		Module i = 0;
		for (const auto &element : l)
		{
			if (interrupt(element))
			{
				break;
			}
			accept(element, i);
			i++;
		}
	},
					   1);
}

template <typename E>
Semantic<E> from(const std::set<E> &s)
{
	return Semantic<E>([s](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		Module i = 0;
		for (const auto &element : s)
		{
			if (interrupt(element))
			{
				break;
			}
			accept(element, i);
			i++;
		}
	},
					   1);
}

template <typename E>
Semantic<E> from(const std::unordered_set<E> &s)
{
	return Semantic<E>([s](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		Module i = 0;
		for (const auto &element : s)
		{
			if (interrupt(element))
			{
				break;
			}
			accept(element, i);
			i++;
		}
	},
					   1);
}

template <typename E>
Semantic<E> iterate(const Generator<E> &generator)
{
	return Semantic<E>([generator](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
			   generator(accept, interrupt);
		   }),
		   1;
}

template <typename E>
Semantic<E> range(const E &start, const E &end)
{
	static_assert(std::is_arithmetic<E>::value, "E must be an arithmetic type.");
	return Semantic<E>([start, end](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		E current = start;
		Module index = 0;
		while (current < end)
		{
			accept(current, index);
			index++;
			++current;
		}
	},
					   1);
}

template <typename E>
Semantic<E> range(const E &start, const E &end, const E &step)
{
	static_assert(std::is_arithmetic<E>::value, "E must be an arithmetic type.");
	return Semantic<E>([start, end, step](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
		E current = start;
		Module index = 0;
		while (current < end)
		{
			accept(current, index);
			index++;
			current += step;
		}
	},
					   1);
}
}; // namespace semantic
int main()
{
	semantic::from({1, 2, 3, 4}).cout();
	return 0;
}
