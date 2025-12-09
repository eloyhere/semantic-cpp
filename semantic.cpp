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

//Thread pool
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

template <typename E>
Collector<E, std::string, std::string> joining() {
    return Collector<E, std::string, std::string>(
        []() -> std::string {
            return std::string{};
        },
        [](std::string& acc, const E& elem) -> std::string& {
            if (!acc.empty()) {
                acc += ',';
            }
            acc += toString(elem);
            return acc;
        },
        [](std::string& a, const std::string& b) -> std::string& {
            if (!a.empty() && !b.empty()) {
                a += ',';
            }
            a += b;
            return a;
        },
        [](const std::string& s) -> std::string {
            return s;
        }
    );
}

template <typename E>
Collector<E, std::string, std::string> joining(const std::string& delimiter) {
    return Collector<E, std::string, std::string>(
        []() -> std::string {
            return std::string{};
        },
        [delimiter](std::string& acc, const E& elem) -> std::string& {
            if (!acc.empty()) {
                acc += delimiter;
            }
            acc += toString(elem);
            return acc;
        },
        [delimiter](std::string& a, const std::string& b) -> std::string& {
            if (!a.empty() && !b.empty()) {
                a += delimiter;
            }
            a += b;
            return a;
        },
        [](const std::string& s) -> std::string {
            return s;
        }
    );
}

template <typename E>
Collector<E, std::string, std::string> joining(const std::string& delimiter, const std::string& prefix, const std::string& suffix) {
    return Collector<E, std::string, std::string>(
        []() -> std::string {
            return std::string{};
        },
        [delimiter](std::string& acc, const E& elem) -> std::string& {
            if (!acc.empty()) {
                acc += delimiter;
            }
            acc += toString(elem);
            return acc;
        },
        [delimiter](std::string& a, const std::string& b) -> std::string& {
            if (!a.empty() && !b.empty()) {
                a += delimiter;
            }
            a += b;
            return a;
        },
        [prefix, suffix](const std::string& s) -> std::string {
            return prefix + s + suffix;
        }
    );
}

template <typename E>
Collector<E, std::string, std::string> quotedJoining(const std::string& delimiter, char quote, char escape) {
    return Collector<E, std::string, std::string>(
        []() { return std::string{}; },
        [delimiter, quote, escape](std::string& acc, const E& elem) {
            if (!acc.empty()) acc += delimiter;
            std::string s = toString(elem);
            std::string quoted;
            quoted += quote;
            for (char c : s) {
                if (c == quote || c == escape) quoted += escape;
                quoted += c;
            }
            quoted += quote;
            acc += quoted;
            return acc;
        },
        [delimiter, quote, escape](std::string& a, const std::string& b) {
            if (!a.empty() && !b.empty()) a += delimiter;
            a += b;
            return a;
        },
        [](const std::string& s) { return s; }
    );
}

template <typename E, typename D>
Collector<E, Statistics<E, D>, Statistics<E, D>> toStatistics() {
    return Collector<E, Statistics<E, D>, Statistics<E, D>>(
        []() { return Statistics<E, D>{}; },
        [](Statistics<E, D>& stats, const E& elem) {
            stats.accept(elem);
            return stats;
        },
        [](Statistics<E, D>& a, const Statistics<E, D>& b) {
            a.combine(b);
            return a;
        },
        [](const Statistics<E, D>& s) { return s; }
    );
}


//Collectable
template <typename E>
Collectable<E> &Collectable<E>::operator=(const Collectable<E> &other) {
    if (this != &other) {
        generator = other.generator;
        concurrent = other.concurrent;
    }
    return *this;
}

template <typename E>
Collectable<E> &Collectable<E>::operator=(Collectable<E> &&other) noexcept {
    if (this != &other) {
        generator = std::move(other.generator);
        concurrent = other.concurrent;
    }
    return *this;
}

template <typename E>
bool Collectable<E>::anyMatch(const Predicate<E> &predicate) const {
    if (concurrent < 2) {
        bool result = false;
        (*generator)(
            [&](const E &element, Timestamp index) {
                if (predicate(element)) {
                    result = true;
                }
            },
            [&](const E &element) -> bool {
                return result;
            }
        );
        return result;
    } else {
        std::atomic<bool> found{false};
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (found.load()) return;
                        if (index % threadCount == threadIndex) {
                            if (predicate(element)) {
                                found.store(true);
                            }
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }

        return found.load();
    }
}

template <typename E>
bool Collectable<E>::allMatch(const Predicate<E> &predicate) const {
    if (concurrent < 2) {
        bool result = true;
        (*generator)(
            [&](const E &element, Timestamp index) {
                if (!predicate(element)) {
                    result = false;
                }
            },
            [&](const E &element) -> bool {
                return !result;
            }
        );
        return result;
    } else {
        std::atomic<bool> allTrue{true};
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (!allTrue.load()) return;
                        if (index % threadCount == threadIndex) {
                            if (!predicate(element)) {
                                allTrue.store(false);
                            }
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }

        return allTrue.load();
    }
}

template <typename E>
template <typename A, typename R>
R Collectable<E>::collect(const Supplier<A> &identity, const BiFunction<A, E, A> &accumulator, const BiFunction<A, A, A> &combiner, const Function<A, R> &finisher) const {
    if (concurrent < 2) {
        A result = identity();
        (*generator)(
            [&](const E &element, Timestamp index) {
                result = accumulator(result, element);
            },
            [](const E &element) -> bool { return false; }
        );
        return finisher(result);
    } else {
        std::vector<A> localResults(concurrent);
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            localResults[threadIndex] = identity();
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (index % threadCount == threadIndex) {
                            localResults[threadIndex] = accumulator(localResults[threadIndex], element);
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }

        A merged = identity();
        for (const auto &local : localResults) {
            merged = combiner(merged, local);
        }
        return finisher(merged);
    }
}

template <typename E>
template <typename A, typename R>
R Collectable<E>::collect(const Collector<E, A, R> &collector) const {
    return collect(collector.identity, collector.accumulator, collector.combiner, collector.finisher);
}

template <typename E>
void Collectable<E>::cout() const {
    if (concurrent < 2) {
        std::cout << "[";
        bool first = true;
        (*generator)([&](const E& elem, Timestamp) {
            if (!first) std::cout << ",";
            std::cout << elem;
            first = false;
        }, [](const E&) { return false; });
        std::cout << "]" << std::endl;
    } else {
        std::mutex mtx;
        std::vector<std::future<void>> futures;
        std::vector<std::string> buffers(concurrent);
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                std::ostringstream buffer;
                buffer << "[";
                bool first = true;
                (*generator)([&](const E& elem, Timestamp index) {
                    if (index % threadCount == threadIndex) {
                        if (!first) buffer << ",";
                        buffer << elem;
                        first = false;
                    }
                }, [](const E&) { return false; });
                buffer << "]";
                buffers[threadIndex] = buffer.str();
            }));
        }

        for (auto& future : futures) future.wait();

        std::cout << "[";
        for (size_t i = 0; i < buffers.size(); ++i) {
            if (i > 0) std::cout << ",";
            std::cout << buffers[i].substr(1, buffers[i].size() - 2);
        }
        std::cout << "]" << std::endl;
    }
}

template <typename E>
void Collectable<E>::cout(const BiFunction<E, std::ostream&, std::ostream&>& accumulator) const {
    if (concurrent < 2) {
        std::cout << "[";
        (*generator)([&](const E& elem, Timestamp) {
            accumulator(elem, std::cout);
        }, [](const E&) { return false; });
        std::cout << "]" << std::endl;
    } else {
        std::mutex mtx;
        std::vector<std::future<void>> futures;
        std::vector<std::string> buffers(concurrent);
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                std::ostringstream buffer;
                buffer << "[";
                (*generator)([&](const E& elem, Timestamp index) {
                    if (index % threadCount == threadIndex) {
                        accumulator(elem, buffer);
                        first = false;
                    }
                }, [](const E&) { return false; });
                buffer << "]";
                buffers[threadIndex] = buffer.str();
            }));
        }

        for (auto& future : futures) future.wait();

        std::cout << "[";
        for (Module i = 0; i < buffers.size(); ++i) {
            std::cout << buffers[i].substr(1, buffers[i].size() - 2);
        }
        std::cout << "]" << std::endl;
    }
}

template <typename E>
void Collectable<E>::cout(std::ostream& stream) const {
    if (concurrent < 2) {
        bool first = true;
        (*generator)([&](const E& elem, Timestamp) {
            if (!first) stream << ",";
            stream << elem;
            first = false;
        }, [](const E&) { return false; });
    } else {
        std::mutex mtx;
        std::vector<std::future<void>> futures;
        std::vector<std::string> buffers(concurrent);
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                std::ostringstream buffer;
                bool first = true;
                (*generator)([&](const E& elem, Timestamp index) {
                    if (index % threadCount == threadIndex) {
                        if(!first){
                        	buffer << ',';
                        }
                        buffer << elem;
                    }
                }, [](const E&) { return false; });
                buffers[threadIndex] = buffer.str();
            }));
        }

        for (auto& future : futures) future.wait();

        std::lock_guard<std::mutex> lock(mtx);
        for (Module i = 0; i < buffers.size(); ++i) {
            if (i > 0) stream << ",";
            stream << buffers[i].substr(1, buffers[i].size() - 2);
        }
    }
}

template <typename E>
void Collectable<E>::cout(std::ostream& stream, const BiConsumer<E, std::ostream&>& accumulator) const {
    if (concurrent < 2) {
        (*generator)([&](const E& elem, Timestamp) {
            accumulator(elem, stream);
        }, [](const E&) { return false; });
    } else {
        std::mutex mtx;
        std::vector<std::future<void>> futures;
        std::vector<std::string> buffers(concurrent);
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                std::ostringstream buffer;
                (*generator)([&](const E& elem, Timestamp index) {
                    if (index % threadCount == threadIndex) {
                        accumulator(elem, buffer);
                    }
                }, [](const E&) { return false; });
                buffers[threadIndex] = buffer.str();
            }));
        }

        for (auto& future : futures) future.wait();

        std::lock_guard<std::mutex> lock(mtx);
        for (Module i = 0; i < buffers.size(); ++i) {
            stream << buffers[i].substr(1, buffers[i].size() - 2);
        }
    }
}

template <typename E>
Module Collectable<E>::count() const {
    return collect(
        []() -> Module { return 0; },
        [](Module current, const E &element) -> Module { return current + 1; },
        [](Module a, Module b) -> Module { return a + b; },
        [](Module result) -> Module { return result; }
    );
}

template <typename E>
std::optional<E> Collectable<E>::findFirst() const {
    if (concurrent < 2) {
        std::optional<E> result;
        (*generator)(
            [&](const E &element, Timestamp index) {
                if (!result.has_value()) {
                    result = element;
                }
            },
            [](const E &element) -> bool { return false; }
        );
        return result;
    } else {
        std::mutex resultMutex;
        std::optional<E> firstElement;
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (index % threadCount == threadIndex) {
                            std::lock_guard<std::mutex> lock(resultMutex);
                            if (!firstElement.has_value()) {
                                firstElement = element;
                            }
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }

        return firstElement;
    }
}

template <typename E>
std::optional<E> Collectable<E>::findAny() const {
    if (concurrent < 2) {
        std::optional<E> result;
        (*generator)(
            [&](const E &element, Timestamp index) {
                if (!result.has_value()) {
                    result = element;
                }
            },
            [](const E &element) -> bool { return false; }
        );
        return result;
    } else {
        std::mutex resultMutex;
        std::optional<E> anyElement;
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (index % threadCount == threadIndex) {
                            std::lock_guard<std::mutex> lock(resultMutex);
                            if (!anyElement.has_value()) {
                                anyElement = element;
                            }
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }

        return anyElement;
    }
}

template <typename E>
void Collectable<E>::forEach(const Consumer<E> &consumer) const {
    if (concurrent < 2) {
        (*generator)(
            [&](const E &element, Timestamp index) {
                consumer(element);
            },
            [](const E &element) -> bool { return false; }
        );
    } else {
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (index % threadCount == threadIndex) {
                            consumer(element);
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }
    }
}

template <typename E>
template <typename K>
std::map<K, std::vector<E>> Collectable<E>::group(const Function<E, K> &classifier) const {
    if (concurrent < 2) {
        std::map<K, std::vector<E>> result;
        (*generator)(
            [&](const E &element, Timestamp index) {
                K key = classifier(element);
                result[key].push_back(element);
            },
            [](const E &element) -> bool { return false; }
        );
        return result;
    } else {
        std::vector<std::map<K, std::vector<E>>> localMaps(concurrent);
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (index % threadCount == threadIndex) {
                            K key = classifier(element);
                            localMaps[threadIndex][key].push_back(element);
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }

        std::map<K, std::vector<E>> merged;
        for (const auto &localMap : localMaps) {
            for (const auto &pair : localMap) {
                merged[pair.first].insert(merged[pair.first].end(), pair.second.begin(), pair.second.end());
            }
        }
        return merged;
    }
}

template <typename E>
template <typename K, typename V>
std::map<K, std::vector<V>> Collectable<E>::groupBy(const Function<E, K> &keyExtractor, const Function<E, V> &valueExtractor) const {
    if (concurrent < 2) {
        std::map<K, std::vector<V>> result;
        (*generator)(
            [&](const E &element, Timestamp index) {
                K key = keyExtractor(element);
                V value = valueExtractor(element);
                result[key].push_back(value);
            },
            [](const E &element) -> bool { return false; }
        );
        return result;
    } else {
        std::vector<std::map<K, std::vector<V>>> localMaps(concurrent);
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (index % threadCount == threadIndex) {
                            K key = keyExtractor(element);
                            V value = valueExtractor(element);
                            localMaps[threadIndex][key].push_back(value);
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }

        std::map<K, std::vector<V>> merged;
        for (const auto &localMap : localMaps) {
            for (const auto &pair : localMap) {
                merged[pair.first].insert(merged[pair.first].end(), pair.second.begin(), pair.second.end());
            }
        }
        return merged;
    }
}

template <typename E>
bool Collectable<E>::noneMatch(const Predicate<E> &predicate) const {
    return !anyMatch(predicate);
}

template <typename E>
std::vector<std::vector<E>> Collectable<E>::partition(const Module &count) const {
    if (count == 0) {
        return {};
    }
    std::vector<std::vector<E>> result(count);
    Module index = 0;
    forEach([&](const E &element) {
        result[(index++) % count].push_back(element);
    });
    return result;
}

template <typename E>
std::vector<std::vector<E>> Collectable<E>::partitionBy(const Function<E, Module> &classifier) const {
    auto groups = group(classifier);
    std::vector<std::vector<E>> result;
    for (auto &pair : groups) {
        result.push_back(std::move(pair.second));
    }
    return result;
}

template <typename E>
std::optional<E> Collectable<E>::reduce(const BiFunction<E, E, E> &accumulator) const {
    std::optional<E> result;
    forEach([&](const E &element) {
        if (!result.has_value()) {
            result = element;
        } else {
            result = accumulator(result.value(), element);
        }
    });
    return result;
}

template <typename E>
E Collectable<E>::reduce(const E &identity, const BiFunction<E, E, E> &accumulator) const {
    E result = identity;
    forEach([&](const E &element) {
        result = accumulator(result, element);
    });
    return result;
}

template <typename E>
template <typename R>
R Collectable<E>::reduce(const R &identity, const BiFunction<R, E, R> &accumulator, const BiFunction<R, R, R> &combiner) const {
    return collect(
        [&identity]() -> R { return identity; },
        [&accumulator](R current, const E &element) -> R { return accumulator(current, element); },
        combiner,
        [](R result) -> R { return result; }
    );
}

template <typename E>
std::list<E> Collectable<E>::toList() const {
    if (concurrent < 2) {
        std::list<E> result;
        (*generator)(
            [&](const E &element, Timestamp index) {
                result.push_back(element);
            },
            [](const E &element) -> bool { return false; }
        );
        return result;
    } else {
        std::vector<std::list<E>> localLists(concurrent);
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (index % threadCount == threadIndex) {
                            localLists[threadIndex].push_back(element);
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }

        std::list<E> merged;
        for (auto &localList : localLists) {
            merged.splice(merged.end(), localList);
        }
        return merged;
    }
}

template <typename E>
template <typename K, typename V>
std::map<K, V> Collectable<E>::toMap(const Function<E, K> &keyExtractor, const Function<E, V> &valueExtractor) const {
    if (concurrent < 2) {
        std::map<K, V> result;
        (*generator)(
            [&](const E &element, Timestamp index) {
                K key = keyExtractor(element);
                V value = valueExtractor(element);
                result[key] = value;
            },
            [](const E &element) -> bool { return false; }
        );
        return result;
    } else {
        std::vector<std::map<K, V>> localMaps(concurrent);
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (index % threadCount == threadIndex) {
                            K key = keyExtractor(element);
                            V value = valueExtractor(element);
                            localMaps[threadIndex][key] = value;
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }

        std::map<K, V> merged;
        for (const auto &localMap : localMaps) {
            for (const auto &pair : localMap) {
                merged[pair.first] = pair.second;
            }
        }
        return merged;
    }
}

template <typename E>
std::set<E> Collectable<E>::toSet() const {
    if (concurrent < 2) {
        std::set<E> result;
        (*generator)(
            [&](const E &element, Timestamp index) {
                result.insert(element);
            },
            [](const E &element) -> bool { return false; }
        );
        return result;
    } else {
        std::vector<std::set<E>> localSets(concurrent);
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (index % threadCount == threadIndex) {
                            localSets[threadIndex].insert(element);
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }

        std::set<E> merged;
        for (const auto &localSet : localSets) {
            merged.insert(localSet.begin(), localSet.end());
        }
        return merged;
    }
}

template <typename E>
std::unordered_set<E> Collectable<E>::toUnorderedSet() const {
    if (concurrent < 2) {
        std::unordered_set<E> result;
        (*generator)(
            [&](const E &element, Timestamp index) {
                result.insert(element);
            },
            [](const E &element) -> bool { return false; }
        );
        return result;
    } else {
        std::vector<std::unordered_set<E>> localSets(concurrent);
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (index % threadCount == threadIndex) {
                            localSets[threadIndex].insert(element);
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }

        std::unordered_set<E> merged;
        for (const auto &localSet : localSets) {
            merged.insert(localSet.begin(), localSet.end());
        }
        return merged;
    }
}

template <typename E>
std::vector<E> Collectable<E>::toVector() const {
    if (concurrent < 2) {
        std::vector<E> result;
        (*generator)(
            [&](const E &element, Timestamp index) {
                result.push_back(element);
            },
            [](const E &element) -> bool { return false; }
        );
        return result;
    } else {
        std::vector<std::vector<E>> localVectors(concurrent);
        std::vector<std::future<void>> futures;
        Module threadCount = concurrent;

        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                (*generator)(
                    [&](const E &element, Timestamp index) {
                        if (index % threadCount == threadIndex) {
                            localVectors[threadIndex].push_back(element);
                        }
                    },
                    [](const E &element) -> bool { return false; }
                );
            }));
        }

        for (auto &future : futures) {
            future.wait();
        }

        std::vector<E> merged;
        for (auto &localVector : localVectors) {
            merged.insert(merged.end(), localVector.begin(), localVector.end());
        }
        return merged;
    }
}

//Statistics
template <typename E, typename D>
Module Statistics<E, D>::count() const {
    if (!total) {
        total = Collectable<E>::count();
    }
    return total;
}

template <typename E, typename D>
E Statistics<E, D>::maximum(const Comparator<E, E> &comparator) const {
    std::optional<E> result;
    forEach([&](const E &element) {
        if (!result.has_value() || comparator(result.value(), element) < 0) {
            result = element;
        }
    });
    return result.value();
}

template <typename E, typename D>
E Statistics<E, D>::minimum(const Comparator<E, E> &comparator) const {
    std::optional<E> result;
    forEach([&](const E &element) {
        if (!result.has_value() || comparator(result.value(), element) > 0) {
            result = element;
        }
    });
    return result.value();
}

template <typename E, typename D>
D Statistics<E, D>::range(const Function<E, D> &mapper) const {
    std::vector<D> values = toVector(mapper);
    return *std::max_element(values.begin(), values.end()) - *std::min_element(values.begin(), values.end());
}

template <typename E, typename D>
D Statistics<E, D>::variance(const Function<E, D> &mapper) const {
    std::vector<D> values = toVector(mapper);
    D mean_val = mean(mapper);
    D sum_sq = std::accumulate(values.begin(), values.end(), D{}, 
        [&](D acc, D val) { return acc + (val - mean_val) * (val - mean_val); });
    return values.size() > 1 ? sum_sq / (values.size() - 1) : D{};
}

template <typename E, typename D>
D Statistics<E, D>::standardDeviation(const Function<E, D> &mapper) const {
    return std::sqrt(variance(mapper));
}

template <typename E, typename D>
D Statistics<E, D>::mean(const Function<E, D> &mapper) const {
    return sum(mapper) / count();
}

template <typename E, typename D>
D Statistics<E, D>::median(const Function<E, D> &mapper) const {
    std::vector<D> values = toVector(mapper);
    std::sort(values.begin(), values.end());
    size_t n = values.size();
    if (n == 0) return D{};
    if (n % 2 == 0) {
        return (values[n/2 - 1] + values[n/2]) / 2;
    } else {
        return values[n/2];
    }
}

template <typename E, typename D>
D Statistics<E, D>::mode(const Function<E, D> &mapper) const {
    std::map<D, Module> freq_map = frequency(mapper);
    return std::max_element(freq_map.begin(), freq_map.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; })->first;
}

template <typename E, typename D>
std::map<D, Module> Statistics<E, D>::frequency(const Function<E, D> &mapper) const {
    std::map<D, Module> freq_map;
    forEach([&](const E &element) {
        freq_map[mapper(element)]++;
    });
    return freq_map;
}

template <typename E, typename D>
D Statistics<E, D>::sum(const Function<E, D> &mapper) const {
    D result = std::accumulate(toVector(mapper).begin(), toVector(mapper).end(), D{});
    return result;
}

template <typename E, typename D>
std::vector<D> Statistics<E, D>::quartiles(const Function<E, D> &mapper) const {
    std::vector<D> values = toVector(mapper);
    std::sort(values.begin(), values.end());
    std::vector<D> quartiles(3);
    size_t n = values.size();
    if (n == 0) return quartiles;
    quartiles = values[n/4];
    quartiles = values[n/2];
    quartiles = values[3*n/4];
    return quartiles;
}

template <typename E, typename D>
D Statistics<E, D>::interquartileRange(const Function<E, D> &mapper) const {
    std::vector<D> q = quartiles(mapper);
    return q - q;
}

template <typename E, typename D>
D Statistics<E, D>::skewness(const Function<E, D> &mapper) const {
    std::vector<D> values = toVector(mapper);
    D mean_val = mean(mapper);
    D stddev_val = standardDeviation(mapper);
    if (stddev_val == D{}) return D{};
    D sum_cubed = std::accumulate(values.begin(), values.end(), D{},
        [&](D acc, D val) { return acc + std::pow((val - mean_val) / stddev_val, 3); });
    return sum_cubed / values.size();
}

template <typename E, typename D>
D Statistics<E, D>::kurtosis(const Function<E, D> &mapper) const {
    std::vector<D> values = toVector(mapper);
    D mean_val = mean(mapper);
    D stddev_val = standardDeviation(mapper);
    if (stddev_val == D{}) return D{};
    D sum_quad = std::accumulate(values.begin(), values.end(), D{},
        [&](D acc, D val) { return acc + std::pow((val - mean_val) / stddev_val, 4); });
    return (sum_quad / values.size()) - 3;
}

template <typename E, typename D>
bool Statistics<E, D>::isEmpty() const {
    return count() == 0;
}

template <typename E, typename D>
void Statistics<E, D>::clear() {
    total = 0;
    frequencyCache.clear();
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(const std::list<E> &l) {
    generator = std::make_shared<Generator<E>>([l](const Consumer<E> &consumer, const Predicate<E> &interrupt) {
        for (const auto &element : l) {
            if (interrupt(element)) return;
            consumer(element, 0);
        }
    });
    total = 0;
    frequencyCache.clear();
    return *this;
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(const std::vector<E> &v) {
    generator = std::make_shared<Generator<E>>([v](const Consumer<E> &consumer, const Predicate<E> &interrupt) {
        for (const auto &element : v) {
            if (interrupt(element)) return;
            consumer(element, 0);
        }
    });
    total = 0;
    frequencyCache.clear();
    return *this;
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(std::initializer_list<E> l) {
    generator = std::make_shared<Generator<E>>([l](const Consumer<E> &consumer, const Predicate<E> &interrupt) {
        for (const auto &element : l) {
            if (interrupt(element)) return;
            consumer(element, 0);
        }
    });
    total = 0;
    frequencyCache.clear();
    return *this;
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(const Statistics<E, D> &other) {
    if (this != &other) {
        generator = other.generator;
        concurrent = other.concurrent;
        total = other.total;
        frequencyCache = other.frequencyCache;
    }
    return *this;
}

template <typename E, typename D>
Statistics<E, D> &Statistics<E, D>::operator=(Statistics<E, D> &&other) noexcept {
    if (this != &other) {
        generator = std::move(other.generator);
        concurrent = other.concurrent;
        total = other.total;
        frequencyCache = std::move(other.frequencyCache);
    }
    return *this;
}

//OrderedCollectable
template <typename E>
OrderedCollectable<E>::OrderedCollectable()
    : Collectable<E>(), container() {}

template <typename E>
OrderedCollectable<E>::OrderedCollectable(const Generator<E>& generator)
    : Collectable<E>(generator), container(toIndexedSet()) {}

template <typename E>
OrderedCollectable<E>::OrderedCollectable(const Generator<E>& generator, const Module& concurrent)
    : Collectable<E>(generator, concurrent), container(toIndexedSet()) {}

template <typename E>
OrderedCollectable<E>::OrderedCollectable(std::shared_ptr<Generator<E>> generator)
    : Collectable<E>(generator), container(toIndexedSet()) {}

template <typename E>
OrderedCollectable<E>::OrderedCollectable(std::shared_ptr<Generator<E>> generator, const Module& concurrent)
    : Collectable<E>(generator, concurrent), container(toIndexedSet()) {}

template <typename E>
OrderedCollectable<E>::OrderedCollectable(const OrderedCollectable<E>& other)
    : Collectable<E>(other), container(other.container) {}

template <typename E>
OrderedCollectable<E>::OrderedCollectable(OrderedCollectable<E>&& other) noexcept
    : Collectable<E>(std::move(other)), container(std::move(other.container)) {}

template <typename E>
OrderedCollectable<E>& OrderedCollectable<E>::operator=(const Collectable<E>& other) {
    Collectable<E>::operator=(other);
    return *this;
}

template <typename E>
OrderedCollectable<E>& OrderedCollectable<E>::operator=(Collectable<E>&& other) noexcept {
    Collectable<E>::operator=(std::move(other));
    return *this;
}

template <typename E>
typename OrderedCollectable<E>::Container OrderedCollectable<E>::toIndexedSet() const {
    Container result;
    if (this->concurrent < 2) {
        (*this->generator)([&](const E& elem, Timestamp ts) {
            result.emplace(ts, elem);
        }, [](const E&) { return false; });
    } else {
        std::vector<std::pair<Timestamp, E>> buffer;
        std::mutex mtx;
        (*this->generator)([&](const E& elem, Timestamp ts) {
            std::lock_guard<std::mutex> lock(mtx);
            buffer.emplace_back(ts, elem);
        }, [](const E&) { return false; });
        for (auto&& p : buffer) {
            result.insert(std::move(p));
        }
    }
    return result;
}


template <typename E>
bool OrderedCollectable<E>::allMatch(const Predicate<E>& p) const {
    if (this->concurrent < 2) {
        for (const auto& pair : container) {
            if (!p(pair.second)) return false;
        }
        return true;
    } else {
        std::vector<bool> results;
        std::mutex mtx;
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it]() {
                bool local_result = true;
                for (auto iter = start; iter != end; ++iter) {
                    if (!p(iter->second)) {
                        local_result = false;
                        break;
                    }
                }
                std::lock_guard<std::mutex> lock(mtx);
                results.push_back(local_result);
            });
        }
        for (auto& t : threads) t.join();
        return std::all_of(results.begin(), results.end(), [](bool b) { return b; });
    }
}

template <typename E>
bool OrderedCollectable<E>::anyMatch(const Predicate<E>& p) const {
    if (this->concurrent < 2) {
        for (const auto& pair : container) {
            if (p(pair.second)) return true;
        }
        return false;
    } else {
        std::atomic<bool> found(false);
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it]() {
                if (found.load()) return;
                for (auto iter = start; iter != end; ++iter) {
                    if (p(iter->second)) {
                        found.store(true);
                        return;
                    }
                }
            });
        }
        for (auto& t : threads) t.join();
        return found.load();
    }
}

template <typename E>
template <typename A, typename R>
R OrderedCollectable<E>::collect(const Supplier<A>& identity, const BiFunction<A, E, A>& accumulator, const BiFunction<A, A, A>& combiner, const Function<A, R>& finisher) const {
    if (this->concurrent < 2) {
        A result = identity();
        for (const auto& pair : container) {
            result = accumulator(result, pair.second);
        }
        return finisher(result);
    } else {
        std::vector<A> partials(this->concurrent, identity());
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        Module thread_idx = 0;
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i, ++thread_idx) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it, idx = thread_idx]() {
                A local = identity();
                for (auto iter = start; iter != end; ++iter) {
                    local = accumulator(local, iter->second);
                }
                partials[idx] = local;
            });
        }
        for (auto& t : threads) t.join();
        A result = partials;
        for (Module i = 1; i < thread_idx; ++i) {
            result = combiner(result, partials[i]);
        }
        return finisher(result);
    }
}

template <typename E>
template <typename A, typename R>
R OrderedCollectable<E>::collect(const Collector<E, A, R>& c) const {
    if (this->concurrent < 2) {
        A container = c.supplier()();
        for (const auto& pair : this->container) {
            c.accumulator()(container, pair.second);
        }
        return c.finisher()(container);
    } else {
        std::vector<A> partials(this->concurrent);
        for (Module i = 0; i < this->concurrent; ++i) {
            partials[i] = c.supplier()();
        }
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(this->container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = this->container.begin();
        Module thread_idx = 0;
        for (Module i = 0; i < this->concurrent && it != this->container.end(); ++i, ++thread_idx) {
            auto start = it;
            Module count = 0;
            while (it != this->container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it, idx = thread_idx]() {
                for (auto iter = start; iter != end; ++iter) {
                    c.accumulator()(partials[idx], iter->second);
                }
            });
        }
        for (auto& t : threads) t.join();
        A result = partials;
        for (Module i = 1; i < thread_idx; ++i) {
            result = c.combiner()(result, partials[i]);
        }
        return c.finisher()(result);
    }
}

template <typename E>
void OrderedCollectable<E>::cout() const {
    if (this->concurrent < 2) {
        for (const auto& pair : container) {
            std::cout << pair.second << std::endl;
        }
    } else {
        std::mutex mtx;
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it]() {
                std::ostringstream oss;
                for (auto iter = start; iter != end; ++iter) {
                    oss << iter->second << std::endl;
                }
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << oss.str();
            });
        }
        for (auto& t : threads) t.join();
    }
}

template <typename E>
void OrderedCollectable<E>::cout() const {
    if (concurrent < 2) {
        std::cout << "[";
        bool first = true;
        for (const auto& pair : container) {
            if (!first) {
                std::cout << ",";
            }
            std::cout << pair.second;
            first = false;
        }
        std::cout << "]" << std::endl;
    } else {
        std::mutex mtx;
        std::vector<std::future<void>> futures;
        std::vector<std::string> buffers(concurrent);
        Module threadCount = concurrent;
        Module chunkSize = (container.size() + threadCount - 1) / threadCount;
        auto it = container.begin();
        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                std::ostringstream buffer;
                buffer << "[";
                bool first = true;
                Module count = 0;
                while (it != container.end() && count < chunkSize) {
                    if (!first) {
                        buffer << ",";
                    }
                    buffer << it->second;
                    first = false;
                    ++it;
                    ++count;
                }
                buffer << "]";
                buffers[threadIndex] = buffer.str();
            }));
        }
        for (auto& future : futures) {
            future.wait();
        }
        std::cout << "[";
        for (Module i = 0; i < buffers.size(); ++i) {
            if (i > 0) {
                std::cout << ",";
            }
            std::cout << buffers[i].substr(1, buffers[i].size() - 2);
        }
        std::cout << "]" << std::endl;
    }
}

template <typename E>
void OrderedCollectable<E>::cout(const BiFunction<E, std::ostream&, std::ostream&>& accumulator) const {
    if (concurrent < 2) {
        std::cout << "[";
        for (const auto& pair : container) {
            accumulator(pair.second, std::cout);
        }
        std::cout << "]" << std::endl;
    } else {
        std::mutex mtx;
        std::vector<std::future<void>> futures;
        std::vector<std::string> buffers(concurrent);
        Module threadCount = concurrent;
        Module chunkSize = (container.size() + threadCount - 1) / threadCount;
        auto it = container.begin();
        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                std::ostringstream buffer;
                buffer << "[";
                Module count = 0;
                while (it != container.end() && count < chunkSize) {
                    accumulator(it->second, buffer);
                    ++it;
                    ++count;
                }
                buffer << "]";
                buffers[threadIndex] = buffer.str();
            }));
        }
        for (auto& future : futures) {
            future.wait();
        }
        std::cout << "[";
        for (Module i = 0; i < buffers.size(); ++i) {
            if (i > 0) {
                std::cout << ",";
            }
            std::cout << buffers[i].substr(1, buffers[i].size() - 2);
        }
        std::cout << "]" << std::endl;
    }
}

template <typename E>
void OrderedCollectable<E>::cout(std::ostream& stream) const {
    if (concurrent < 2) {
        bool first = true;
        for (const auto& pair : container) {
            if (!first) {
                stream << ",";
            }
            stream << pair.second;
            first = false;
        }
        stream << std::endl;
    } else {
        std::mutex mtx;
        std::vector<std::future<void>> futures;
        std::vector<std::string> buffers(concurrent);
        Module threadCount = concurrent;
        Module chunkSize = (container.size() + threadCount - 1) / threadCount;
        auto it = container.begin();
        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                std::ostringstream buffer;
                bool first = true;
                Module count = 0;
                while (it != container.end() && count < chunkSize) {
                    if (!first) {
                        buffer << ",";
                    }
                    buffer << it->second;
                    first = false;
                    ++it;
                    ++count;
                }
                buffers[threadIndex] = buffer.str();
            }));
        }
        for (auto& future : futures) {
            future.wait();
        }
        std::lock_guard<std::mutex> lock(mtx);
        for (Module i = 0; i < buffers.size(); ++i) {
            if (i > 0) {
                stream << ",";
            }
            stream << buffers[i].substr(1, buffers[i].size() - 2);
        }
        stream << std::endl;
    }
}

template <typename E>
void OrderedCollectable<E>::cout(std::ostream& stream, const BiConsumer<E, std::ostream&>& accumulator) const {
    if (concurrent < 2) {
        for (const auto& pair : container) {
            accumulator(pair.second, stream);
        }
        stream << std::endl;
    } else {
        std::mutex mtx;
        std::vector<std::future<void>> futures;
        std::vector<std::string> buffers(concurrent);
        Module threadCount = concurrent;
        Module chunkSize = (container.size() + threadCount - 1) / threadCount;
        auto it = container.begin();
        for (Module threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
            futures.push_back(globalThreadPool.submit([&, threadIndex]() {
                std::ostringstream buffer;
                Module count = 0;
                while (it != container.end() && count < chunkSize) {
                    accumulator(it->second, buffer);
                    first = false;
                    ++it;
                    ++count;
                }
                buffers[threadIndex] = buffer.str();
            }));
        }
        for (auto& future : futures) {
            future.wait();
        }
        std::lock_guard<std::mutex> lock(mtx);
        for (Module i = 0; i < buffers.size(); ++i) {
            stream << buffers[i].substr(1, buffers[i].size() - 2);
        }
        stream << std::endl;
    }
}

template <typename E>
Module OrderedCollectable<E>::count() const {
    return static_cast<Module>(container.size());
}

template <typename E>
std::optional<E> OrderedCollectable<E>::findFirst() const {
    if (container.empty()) return std::nullopt;
    return container.begin()->second;
}

template <typename E>
std::optional<E> OrderedCollectable<E>::findAny() const {
    return findFirst();
}

template <typename E>
void OrderedCollectable<E>::forEach(const Consumer<E>& c) const {
    if (this->concurrent < 2) {
        for (const auto& pair : container) {
            c(pair.second);
        }
    } else {
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it]() {
                for (auto iter = start; iter != end; ++iter) {
                    c(iter->second);
                }
            });
        }
        for (auto& t : threads) t.join();
    }
}

template <typename E>
template <typename K>
std::map<K, std::vector<E>> OrderedCollectable<E>::group(const Function<E, K>& classifier) const {
    if (this->concurrent < 2) {
        std::map<K, std::vector<E>> result;
        for (const auto& pair : container) {
            K key = classifier(pair.second);
            result[key].push_back(pair.second);
        }
        return result;
    } else {
        std::vector<std::map<K, std::vector<E>>> partials(this->concurrent);
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        Module thread_idx = 0;
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i, ++thread_idx) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it, idx = thread_idx]() {
                for (auto iter = start; iter != end; ++iter) {
                    K key = classifier(iter->second);
                    partials[idx][key].push_back(iter->second);
                }
            });
        }
        for (auto& t : threads) t.join();
        std::map<K, std::vector<E>> result;
        for (Module i = 0; i < thread_idx; ++i) {
            for (auto& kv : partials[i]) {
                result[kv.first].insert(result[kv.first].end(), kv.second.begin(), kv.second.end());
            }
        }
        return result;
    }
}

template <typename E>
template <typename K, typename V>
std::map<K, std::vector<V>> OrderedCollectable<E>::groupBy(const Function<E, K>& keyExtractor, const Function<E, V>& valueExtractor) const {
    if (this->concurrent < 2) {
        std::map<K, std::vector<V>> result;
        for (const auto& pair : container) {
            K key = keyExtractor(pair.second);
            V value = valueExtractor(pair.second);
            result[key].push_back(value);
        }
        return result;
    } else {
        std::vector<std::map<K, std::vector<V>>> partials(this->concurrent);
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        Module thread_idx = 0;
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i, ++thread_idx) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it, idx = thread_idx]() {
                for (auto iter = start; iter != end; ++iter) {
                    K key = keyExtractor(iter->second);
                    V value = valueExtractor(iter->second);
                    partials[idx][key].push_back(value);
                }
            });
        }
        for (auto& t : threads) t.join();
        std::map<K, std::vector<V>> result;
        for (Module i = 0; i < thread_idx; ++i) {
            for (auto& kv : partials[i]) {
                result[kv.first].insert(result[kv.first].end(), kv.second.begin(), kv.second.end());
            }
        }
        return result;
    }
}

template <typename E>
bool OrderedCollectable<E>::noneMatch(const Predicate<E>& p) const {
    return !anyMatch(p);
}

template <typename E>
std::vector<std::vector<E>> OrderedCollectable<E>::partition(const Module& count) const {
    if (count == 0) return {};
    std::vector<std::vector<E>> result;
    result.resize((container.size() + count - 1) / count);
    Module i = 0;
    for (const auto& pair : container) {
        result[i / count].push_back(pair.second);
        ++i;
    }
    return result;
}

template <typename E>
std::vector<std::vector<E>> OrderedCollectable<E>::partitionBy(const Function<E, Module>& classifier) const {
    if (this->concurrent < 2) {
        std::map<Module, std::vector<E>> groups;
        for (const auto& pair : container) {
            Module key = classifier(pair.second);
            groups[key].push_back(pair.second);
        }
        std::vector<std::vector<E>> result;
        for (auto& kv : groups) {
            result.push_back(std::move(kv.second));
        }
        return result;
    } else {
        std::vector<std::map<Module, std::vector<E>>> partials(this->concurrent);
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        Module thread_idx = 0;
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i, ++thread_idx) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it, idx = thread_idx]() {
                for (auto iter = start; iter != end; ++iter) {
                    Module key = classifier(iter->second);
                    partials[idx][key].push_back(iter->second);
                }
            });
        }
        for (auto& t : threads) t.join();
        std::map<Module, std::vector<E>> merged;
        for (Module i = 0; i < thread_idx; ++i) {
            for (auto& kv : partials[i]) {
                merged[kv.first].insert(merged[kv.first].end(), kv.second.begin(), kv.second.end());
            }
        }
        std::vector<std::vector<E>> result;
        for (auto& kv : merged) {
            result.push_back(std::move(kv.second));
        }
        return result;
    }
}

template <typename E>
std::optional<E> OrderedCollectable<E>::reduce(const BiFunction<E, E, E>& accumulator) const {
    if (container.empty()) return std::nullopt;
    if (this->concurrent < 2) {
        auto it = container.begin();
        E result = it->second;
        ++it;
        for (; it != container.end(); ++it) {
            result = accumulator(result, it->second);
        }
        return result;
    } else {
        std::vector<E> partials;
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&partials, start, end = it, &accumulator]() {
                auto iter = start;
                E local = iter->second;
                ++iter;
                for (; iter != end; ++iter) {
                    local = accumulator(local, iter->second);
                }
                partials.push_back(local);
            });
        }
        for (auto& t : threads) t.join();
        E result = partials;
        for (size_t i = 1; i < partials.size(); ++i) {
            result = accumulator(result, partials[i]);
        }
        return result;
    }
}

template <typename E>
E OrderedCollectable<E>::reduce(const E& identity, const BiFunction<E, E, E>& accumulator) const {
    if (this->concurrent < 2) {
        E result = identity;
        for (const auto& pair : container) {
            result = accumulator(result, pair.second);
        }
        return result;
    } else {
        std::vector<E> partials(this->concurrent, identity);
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        Module thread_idx = 0;
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i, ++thread_idx) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it, idx = thread_idx]() {
                E local = identity;
                for (auto iter = start; iter != end; ++iter) {
                    local = accumulator(local, iter->second);
                }
                partials[idx] = local;
            });
        }
        for (auto& t : threads) t.join();
        E result = identity;
        for (Module i = 0; i < thread_idx; ++i) {
            result = accumulator(result, partials[i]);
        }
        return result;
    }
}

template <typename E>
template <typename R>
R OrderedCollectable<E>::reduce(const R& identity, const BiFunction<R, E, R>& accumulator, const BiFunction<R, R, R>& combiner) const {
    if (this->concurrent < 2) {
        R result = identity;
        for (const auto& pair : container) {
            result = accumulator(result, pair.second);
        }
        return result;
    } else {
        std::vector<R> partials(this->concurrent, identity);
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        Module thread_idx = 0;
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i, ++thread_idx) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it, idx = thread_idx]() {
                R local = identity;
                for (auto iter = start; iter != end; ++iter) {
                    local = accumulator(local, iter->second);
                }
                partials[idx] = local;
            });
        }
        for (auto& t : threads) t.join();
        R result = identity;
        for (Module i = 0; i < thread_idx; ++i) {
            result = combiner(result, partials[i]);
        }
        return result;
    }
}

template <typename E>
std::list<E> OrderedCollectable<E>::toList() const {
    if (this->concurrent < 2) {
        std::list<E> result;
        for (const auto& pair : container) {
            result.push_back(pair.second);
        }
        return result;
    } else {
        std::vector<std::list<E>> partials(this->concurrent);
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        Module thread_idx = 0;
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i, ++thread_idx) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it, idx = thread_idx]() {
                for (auto iter = start; iter != end; ++iter) {
                    partials[idx].push_back(iter->second);
                }
            });
        }
        for (auto& t : threads) t.join();
        std::list<E> result;
        for (Module i = 0; i < thread_idx; ++i) {
            result.splice(result.end(), partials[i]);
        }
        return result;
    }
}

template <typename E>
template <typename K, typename V>
std::map<K, V> OrderedCollectable<E>::toMap(const Function<E, K>& keyExtractor, const Function<E, V>& valueExtractor) const {
    if (this->concurrent < 2) {
        std::map<K, V> result;
        for (const auto& pair : container) {
            K key = keyExtractor(pair.second);
            V value = valueExtractor(pair.second);
            result.emplace(key, value);
        }
        return result;
    } else {
        std::vector<std::map<K, V>> partials(this->concurrent);
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        Module thread_idx = 0;
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i, ++thread_idx) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it, idx = thread_idx]() {
                for (auto iter = start; iter != end; ++iter) {
                    K key = keyExtractor(iter->second);
                    V value = valueExtractor(iter->second);
                    partials[idx].emplace(key, value);
                }
            });
        }
        for (auto& t : threads) t.join();
        std::map<K, V> result;
        for (Module i = 0; i < thread_idx; ++i) {
            result.insert(partials[i].begin(), partials[i].end());
        }
        return result;
    }
}

template <typename E>
std::set<E> OrderedCollectable<E>::toSet() const {
    if (this->concurrent < 2) {
        std::set<E> result;
        for (const auto& pair : container) {
            result.insert(pair.second);
        }
        return result;
    } else {
        std::vector<std::set<E>> partials(this->concurrent);
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        Module thread_idx = 0;
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i, ++thread_idx) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it, idx = thread_idx]() {
                for (auto iter = start; iter != end; ++iter) {
                    partials[idx].insert(iter->second);
                }
            });
        }
        for (auto& t : threads) t.join();
        std::set<E> result;
        for (Module i = 0; i < thread_idx; ++i) {
            result.insert(partials[i].begin(), partials[i].end());
        }
        return result;
    }
}

template <typename E>
std::unordered_set<E> OrderedCollectable<E>::toUnorderedSet() const {
    if (this->concurrent < 2) {
        std::unordered_set<E> result;
        result.reserve(container.size());
        for (const auto& pair : container) {
            result.insert(pair.second);
        }
        return result;
    } else {
        std::vector<std::unordered_set<E>> partials(this->concurrent);
        for (Module i = 0; i < this->concurrent; ++i) {
            partials[i].reserve(container.size() / this->concurrent + 1);
        }
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        Module thread_idx = 0;
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i, ++thread_idx) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it, idx = thread_idx]() {
                for (auto iter = start; iter != end; ++iter) {
                    partials[idx].insert(iter->second);
                }
            });
        }
        for (auto& t : threads) t.join();
        std::unordered_set<E> result;
        result.reserve(container.size());
        for (Module i = 0; i < thread_idx; ++i) {
            result.insert(partials[i].begin(), partials[i].end());
        }
        return result;
    }
}

template <typename E>
std::vector<E> OrderedCollectable<E>::toVector() const {
    if (this->concurrent < 2) {
        std::vector<E> result;
        result.reserve(container.size());
        for (const auto& pair : container) {
            result.push_back(pair.second);
        }
        return result;
    } else {
        std::vector<std::vector<E>> partials(this->concurrent);
        for (Module i = 0; i < this->concurrent; ++i) {
            partials[i].reserve(container.size() / this->concurrent + 1);
        }
        Module chunk_size = std::max(static_cast<Module>(1), static_cast<Module>(container.size()) / this->concurrent);
        std::vector<std::thread> threads;
        auto it = container.begin();
        Module thread_idx = 0;
        for (Module i = 0; i < this->concurrent && it != container.end(); ++i, ++thread_idx) {
            auto start = it;
            Module count = 0;
            while (it != container.end() && count < chunk_size) {
                ++it;
                ++count;
            }
            threads.emplace_back([&, start, end = it, idx = thread_idx]() {
                for (auto iter = start; iter != end; ++iter) {
                    partials[idx].push_back(iter->second);
                }
            });
        }
        for (auto& t : threads) t.join();
        std::vector<E> result;
        result.reserve(container.size());
        for (Module i = 0; i < thread_idx; ++i) {
            result.insert(result.end(), partials[i].begin(), partials[i].end());
        }
        return result;
    }
}

//Semantic
template <typename E>
Semantic<E> &Semantic<E>::operator=(const Semantic<E> &other) {
    if (this != &other) {
        generator = other.generator;
        concurrent = other.concurrent;
    }
    return *this;
}

template <typename E>
Semantic<E> &Semantic<E>::operator=(Semantic<E> &&other) noexcept {
    if (this != &other) {
        generator = std::move(other.generator);
        concurrent = other.concurrent;
    }
    return *this;
}

template <typename E>
Semantic<E> Semantic<E>::concat(const Semantic<E> &other) const {
    return Semantic<E>([this, &other](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        (*generator)(accept, interrupt);
        (*other.generator)(accept, interrupt);
    }, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::distinct() const {
    return Semantic<E>([this](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        std::set<E> seen;
        (*generator)([&](const E &element, Timestamp index) {
            if (seen.insert(element).second && !interrupt(element)) {
                accept(element, index);
            }
        }, interrupt);
    }, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::distinct(const Comparator<E, E> &identifier) const {
    return Semantic<E>([this, &identifier](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        std::set<Timestamp> seen;
        (*generator)([&](const E &element, Timestamp index) {
            Timestamp id = identifier(element, element);
            if (seen.insert(id).second && !interrupt(element)) {
                accept(element, index);
            }
        }, interrupt);
    }, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::dropWhile(const Predicate<E> &p) const {
    return Semantic<E>([this, &p](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        bool dropping = true;
        (*generator)([&](const E &element, Timestamp index) {
            if (dropping && p(element)) {
                return;
            }
            dropping = false;
            accept(element, index);
        }, [](const E &element) {
            return false;
        });
    }, concurrent);
}


template <typename E>
Semantic<E> Semantic<E>::filter(const Predicate<E> &p) const {
    return Semantic<E>([this, &p](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        (*generator)([&](const E &element, Timestamp index) {
            if (p(element) && !interrupt(element)) {
                accept(element, index);
            }
        }, interrupt);
    }, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::flat(const Function<E, Semantic<E>> &mapper) const {
    return Semantic<E>([this, &mapper](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        (*generator)([&](const E &element, Timestamp index) {
            if (!interrupt(element)) {
                Semantic<E> nested = mapper(element);
                (*nested.generator)(accept, interrupt);
            }
        }, interrupt);
    }, concurrent);
}

template <typename E>
template <typename R>
Semantic<R> Semantic<E>::flatMap(const Function<E, Semantic<R>> &mapper) const {
    return Semantic<R>([this, &mapper](const BiConsumer<R, Timestamp> &accept, const Predicate<R> &interrupt) {
        (*generator)([&](const E &element, Timestamp index) {
            if (!interrupt(element)) {
                Semantic<R> nested = mapper(element);
                (*nested.generator)([&](const R &r_element, Timestamp r_index) {
                    accept(r_element, r_index);
                }, [&](const R &r_element) {
                    return interrupt(r_element);
                });
            }
        }, [&](const E &element) {
            return false;
        });
    }, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::limit(const Module &n) const {
    return Semantic<E>([this, n](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        Module count = 0;
        (*generator)([&](const E &element, Timestamp index) {
            accept(element, index);
            count++;
        }, [&n](const E& element)->bool{
        	return count < n;
        });
    }, concurrent);
}

template <typename E>
template <typename R>
Semantic<R> Semantic<E>::map(const Function<E, R> &mapper) const {
    return Semantic<R>([this, &mapper](const BiConsumer<R, Timestamp> &accept, const Predicate<R> &interrupt) {
        (*generator)([&](const E &element, Timestamp index) {
            R mapped = mapper(element);
            if (!interrupt(mapped)) {
                accept(mapped, index);
            }
        }, [&](const E &element) {
            return false;
        });
    }, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::parallel() const {
    return Semantic<E>(generator, std::thread::hardware_concurrency());
}

template <typename E>
Semantic<E> Semantic<E>::parallel(const Module &threadCount) const {
    return Semantic<E>(generator, threadCount);
}

template <typename E>
Semantic<E> Semantic<E>::peek(const Consumer<E> &c) const {
    return Semantic<E>([this, &c](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        (*generator)([&](const E &element, Timestamp index) {
            c(element);
            if (!interrupt(element)) {
                accept(element, index);
            }
        }, interrupt);
    }, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::redirect(const BiFunction<E, Timestamp, Timestamp> &redirector) const {
    return Semantic<E>([*this, &redirector](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        (*this->generator)([&](const E &element, const Timestamp &index) {
            accept(element, redirector(element, index));
        }, [](const E &element) {
            return false;
        });
    }, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::reverse() const {
    return Semantic<E>([*this](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        (*this->generator)([&](const E &element, const Timestamp &index) {
            accept(element, -index);
        }, [](const E &element) {
            return false;
        });
    }, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::shuffle() const {
        (*this->generator)([&](const E &element, const Timestamp &index) {
            accept(element, std::hash<E>{}(element) ^ std::hash<E>{}(index));
        }, [](const E &element) {
            return false;
        });
    }, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::shuffle(const Function<E, Timestamp> &mapper) const {
    return Semantic<E>([this, &mapper](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        (*generator)([&](const E &element, Timestamp index) {
            accept(element, mapper(element));
        }, [](const E &element) {
            return false;
        });
    }, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::skip(const Module &n) const {
    return Semantic<E>([this, n](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        Module count = 0;
        (*generator)([&](const E &element, Timestamp index) {
            if (count < n) {
                count++;
                return;
            }
            accept(element, index);
        }, [](const E &element) {
            return false;
        });
    }, concurrent);
}

template <typename E>
OrderedCollectable<E> Semantic<E>::sorted() const {
    return sorted(std::less<E>{});
}

template <typename E>
template <typename Comparator>
OrderedCollectable<E> Semantic<E>::sorted(Comparator comp) const {
    using Pair = std::pair<Timestamp, E>;
    auto pairComp = [&comp](const Pair& a, const Pair& b) {
        if (comp(a.second, b.second)) return true;
        if (comp(b.second, a.second)) return false;
        return a.first < b.first;
    };

    std::set<Pair, decltype(pairComp)> sortedSet(pairComp);

    if (this->concurrent < 2) {
        (*this->generator)([&sortedSet](const E& elem, Timestamp ts) {
            sortedSet.emplace(ts, elem);
        }, [](const E&) { return false; });
    } else {
        std::vector<Pair> buffer;
        std::mutex mtx;
        (*this->generator)([&buffer, &mtx](const E& elem, Timestamp ts) {
            std::lock_guard<std::mutex> lock(mtx);
            buffer.emplace_back(ts, elem);
        }, [](const E&) { return false; });
        for (auto&& item : buffer) {
            sortedSet.insert(std::move(item));
        }
    }

    typename OrderedCollectable<E>::Container finalContainer;
    Timestamp newTs = 0;
    for (const auto& item : sortedSet) {
        finalContainer.emplace(newTs++, item.second);
    }

    auto gen = std::make_shared<Generator<E>>([finalContainer](const Consumer<E>& c, const Predicate<E>&) {
        for (const auto& p : finalContainer) {
            c(p.second, p.first);
        }
    });

    return OrderedCollectable<E>(gen, this->concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::sub(const Module &start, const Module &end) const {
    return Semantic<E>([this, start, end](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        Module index = 0;
        (*generator)([&](const E &element, Timestamp timestamp) {
            if (index >= start && index < end) {
                accept(element, timestamp);
            }
            index++;
        }, [](const E &element) {
            return false;
        });
    }, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::takeWhile(const Predicate<E> &p) const {
    return Semantic<E>([this, &p](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        bool taking = true;
        (*generator)([&](const E &element, Timestamp index) {
            if (!taking) {
                return;
            }
            if (p(element)) {
                accept(element, index);
            } else {
                taking = false;
            }
        }, [](const E &element) {
            return false;
        });
    }, concurrent);
}

template <typename E>
OrderedCollectable<E> Semantic<E>::toOrdered() const {
    return OrderedCollectable<E>(generator, concurrent);
}

template <typename E>
Statistics<E, E> Semantic<E>::toStatistics() const {
    return Statistics<E, E>(generator, concurrent);
}

template <typename E>
template <typename R>
Statistics<E, R> Semantic<E>::toStatistics(const Function<E, R> &mapper) const {
    return Statistics<E, R>([this, &mapper](const BiConsumer<R, Timestamp> &accept, const Predicate<R> &interrupt) {
        (*generator)([&](const E &element, Timestamp index) {
            R mapped = mapper(element);
            if (!interrupt(mapped)) {
                accept(mapped, index);
            }
        }, [&](const E &element) {
            return false;
        });
    }, concurrent);
}

template <typename E>
UnorderedCollectable<E> Semantic<E>::toUnordered() const {
    return UnorderedCollectable<E>(generator, concurrent);
}

template <typename E>
Semantic<E> Semantic<E>::translate(const Timestamp &offset) const {
    return Semantic<E>([this, offset](const BiConsumer<E, Timestamp> &accept, const Predicate<E> &interrupt) {
        (*generator)([&](const E &element, Timestamp index) {
            if (!interrupt(element)) {
                accept(element, index + offset);
            }
        }, interrupt);
    }, concurrent);
}


//Semantic factory function
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
