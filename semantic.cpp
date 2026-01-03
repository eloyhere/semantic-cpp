#include "semantic.h"
namespace semantic {

    //Global functions
    template <typename D>
    D randomly(const D& start, const D& end)
    {
        static std::random_device random;
        static std::mt19937_64 generator(random());

        D maximum = std::max(start, end);
        D minimum = std::min(start, end);

        if constexpr (std::is_integral<D>::value)
        {
            std::uniform_int_distribution<D> distribution(minimum, maximum);
            return distribution(generator);
        }
        else
        {
            std::uniform_real_distribution<D> distribution(minimum, maximum);
            return distribution(generator);
        }
    }

    //ThreadPool
    ThreadPool::ThreadPool(Module workerCount) : workerCount(workerCount)
    {
        for (Module i = 0; i < workerCount; ++i)
        {
            workers.emplace_back(
                std::thread([this]
                    {
                        while (true)
                        {
                            Runnable task;
                            {
                                std::unique_lock<std::mutex> lock(this->queueMutex);
                                this->condition.wait(lock, [this]
                                    {
                                        if (this->stop)
                                        {
                                            return true;
                                        }
                                        return !this->taskQueue.empty();
                                    });
                                if (this->stop && this->taskQueue.empty())
                                {
                                    return;
                                }
                                task = std::move(this->taskQueue.front());
                                this->taskQueue.pop();
                            }
                            task();
                        }
                    }),
                std::atomic<bool>(false)
            );
        }
    }

    ThreadPool::~ThreadPool()
    {
        shutdown();
    }

    void ThreadPool::shutdown()
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop)
            {
                return;
            }
            stop = true;
        }
        condition.notify_all();
        for (auto& worker : workers)
        {
            if (worker.first.joinable())
            {
                worker.first.join();
            }
        }
        workers.clear();
    }

    template <typename T, typename E>
    Promise<T, E> ThreadPool::submit(Supplier<T> task)
    {
        Promise<T, E> promise(task);
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop)
            {
                throw std::runtime_error("submit on stopped ThreadPool");
            }
            taskQueue.push([promise, task]()
                {
                    try
                    {
                        T result = task();
                        promise.resolveCallback(result);
                    }
                    catch (...)
                    {
                        promise.rejectCallback(std::current_exception());
                    }
                });
        }
        condition.notify_one();
        return promise;
    }

    template <typename T, typename E>
    Promise<T, E> ThreadPool::submit(Supplier<T> task, Timestamp timeoutMilliseconds)
    {
        Promise<T, E> promise(task);
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop)
            {
                throw std::runtime_error("submit on stopped ThreadPool");
            }
            taskQueue.push([promise, task]()
                {
                    try
                    {
                        T result = task();
                        promise.resolveCallback(result);
                    }
                    catch (...)
                    {
                        promise.rejectCallback(std::current_exception());
                    }
                });
        }
        condition.notify_one();
        return promise;
    }

    Module ThreadPool::getWorkerCount() const
    {
        return workerCount;
    }

    Module ThreadPool::getQueuedTaskCount() const
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        return taskQueue.size();
    }

    ThreadPool globalThreadPool(std::thread::hardware_concurrency());

    //Promise
    template <typename T, typename E>
    Promise<T, E>::Promise()
    {
        completionAction = [this]()
            {
                completed = true;
            };
    }

    template <typename T, typename E>
    Promise<T, E>::Promise(Supplier<T> task)
    {
        completionAction = [this]()
            {
                completed = true;
            };
        try
        {
            T value = task();
            result = value;
            state = PromiseState::fullfilled;
            if (resolveCallback)
            {
                resolveCallback(result);
            }
        }
        catch (...)
        {
            exception = std::current_exception();
            state = PromiseState::rejected;
            if (rejectCallback)
            {
                rejectCallback(exception);
            }
        }
        if (finallyCallback)
        {
            finallyCallback();
        }
        completionAction();
    }

    template <typename T, typename E>
    Promise<T, E> Promise<T, E>::then(Consumer<T> action)
    {
        std::unique_lock<std::mutex> lock(callbackMutex);
        if (state == PromiseState::fullfilled)
        {
            action(result);
        }
        else
        {
            auto previousCallback = resolveCallback;
            resolveCallback = [previousCallback, action](T value)
                {
                    if (previousCallback)
                    {
                        previousCallback(value);
                    }
                    action(value);
                };
        }
        return *this;
    }

    template <typename T, typename E>
    Promise<T, E> Promise<T, E>::then(Consumer<T> action, Consumer<E> handler)
    {
        std::unique_lock<std::mutex> lock(callbackMutex);
        if (state == PromiseState::fullfilled)
        {
            action(result);
        }
        else if (state == PromiseState::rejected)
        {
            handler(exception);
        }
        else
        {
            auto previousResolve = resolveCallback;
            auto previousReject = rejectCallback;
            resolveCallback = [previousResolve, action](T value)
                {
                    if (previousResolve)
                    {
                        previousResolve(value);
                    }
                    action(value);
                };
            rejectCallback = [previousReject, handler](E error)
                {
                    if (previousReject)
                    {
                        previousReject(error);
                    }
                    handler(error);
                };
        }
        return *this;
    }

    template <typename T, typename E>
    Promise<T, E> Promise<T, E>::except(Consumer<E> handler)
    {
        std::unique_lock<std::mutex> lock(callbackMutex);
        if (state == PromiseState::rejected)
        {
            handler(exception);
        }
        else
        {
            auto previousCallback = rejectCallback;
            rejectCallback = [previousCallback, handler](E error)
                {
                    if (previousCallback)
                    {
                        previousCallback(error);
                    }
                    handler(error);
                };
        }
        return *this;
    }

    template <typename T, typename E>
    Promise<T, E> Promise<T, E>::finally(Runnable finalAction)
    {
        std::unique_lock<std::mutex> lock(callbackMutex);
        if (completed)
        {
            finalAction();
        }
        else
        {
            auto previousCallback = finallyCallback;
            finallyCallback = [previousCallback, finalAction]()
                {
                    if (previousCallback)
                    {
                        previousCallback();
                    }
                    finalAction();
                };
        }
        return *this;
    }

    template <typename T, typename E>
    Promise<T, E> Promise<T, E>::resolved(const T& value)
    {
        Promise<T, E> promise;
        promise.result = value;
        promise.state = PromiseState::fullfilled;
        promise.completed = true;
        return promise;
    }

    template <typename T, typename E>
    Promise<T, E> Promise<T, E>::rejected(const E& error)
    {
        Promise<T, E> promise;
        promise.exception = error;
        promise.state = PromiseState::rejected;
        promise.completed = true;
        return promise;
    }

    template <typename T, typename E>
    Promise<std::vector<T>, E> Promise<T, E>::all(const std::vector<Promise<T, E>>& promises)
    {
        Promise<std::vector<T>, E> resultPromise;
        std::vector<T> results(promises.size());
        Module remaining = promises.size();
        std::mutex mutex;

        for (Module i = 0; i < promises.size(); ++i)
        {
            promises[i].then([i, &results, &remaining, &mutex, &resultPromise](const T& value)
                {
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        results[i] = value;
                        --remaining;
                    }
                    if (remaining == 0)
                    {
                        resultPromise.result = results;
                        resultPromise.state = PromiseState::fullfilled;
                        resultPromise.completionAction();
                    }
                }).except([&resultPromise](const E& error)
                    {
                        resultPromise.exception = error;
                        resultPromise.state = PromiseState::rejected;
                        resultPromise.completionAction();
                    });
        }
        return resultPromise;
    }

    template <typename T, typename E>
    Promise<std::vector<T>, E> Promise<T, E>::any(const std::vector<Promise<T, E>>& promises)
    {
        Promise<std::vector<T>, E> resultPromise;
        std::mutex mutex;
        Module remaining = promises.size();

        for (Module i = 0; i < promises.size(); ++i)
        {
            promises[i].then([&resultPromise, &mutex, &remaining](const T& value)
                {
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        if (resultPromise.state != PromiseState::fullfilled)
                        {
                            resultPromise.result = std::vector<T>{ value };
                            resultPromise.state = PromiseState::fullfilled;
                            resultPromise.completionAction();
                        }
                    }
                }).except([&resultPromise, &mutex, &remaining](const E& error)
                    {
                        {
                            std::unique_lock<std::mutex> lock(mutex);
                            --remaining;
                            if (remaining == 0 && resultPromise.state != PromiseState::fullfilled)
                            {
                                resultPromise.state = PromiseState::rejected;
                                resultPromise.completionAction();
                            }
                        }
                    });
        }
        return resultPromise;
    }

    template <typename T, typename E>
    PromiseState Promise<T, E>::getState() const
    {
        return state.load();
    }

    template <typename T, typename E>
    bool Promise<T, E>::isCompleted() const
    {
        return completed.load();
    }

    template <typename T, typename E>
    T Promise<T, E>::getResult() const
    {
        while (!completed)
        {
            std::this_thread::yield();
        }
        if (state == PromiseState::rejected)
        {
            std::rethrow_exception(exception);
        }
        return result;
    }

    template <typename T, typename E>
    E Promise<T, E>::getException() const
    {
        while (!completed)
        {
            std::this_thread::yield();
        }
        return exception;
    }

    template <typename T, typename E>
    void Promise<T, E>::wait() const
    {
        while (!completed)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    //Collectable
    template <typename E>
    Collectable<E>::Collectable()
        : generator(std::make_shared<Generator<E>>([](const Consumer<E>&, const Predicate<E>&) {})), concurrent(1) {
    }

    template <typename E>
    Collectable<E>::Collectable(const Generator<E>& generator)
        : generator(std::make_shared<Generator<E>>(generator)), concurrent(1) {
    }

    template <typename E>
    Collectable<E>::Collectable(const Generator<E>& generator, const Module& concurrent)
        : generator(std::make_shared<Generator<E>>(generator)), concurrent(concurrent) {
    }

    template <typename E>
    Collectable<E>::Collectable(std::shared_ptr<Generator<E>> generator)
        : generator(generator), concurrent(1) {
    }

    template <typename E>
    Collectable<E>::Collectable(std::shared_ptr<Generator<E>> generator, const Module& concurrent)
        : generator(generator), concurrent(concurrent) {
    }

    template <typename E>
    Collectable<E>::Collectable(const Collectable<E>& other)
        : generator(other.generator), concurrent(other.concurrent) {
    }

    template <typename E>
    Collectable<E>::Collectable(Collectable<E>&& other) noexcept
        : generator(std::move(other.generator)), concurrent(other.concurrent) {
    }

    template <typename E>
    Collectable<E>& Collectable<E>::operator=(const Collectable<E>& other)
    {
        if (this != &other)
        {
            generator = other.generator;
            concurrent = other.concurrent;
        }
        return *this;
    }

    template <typename E>
    Collectable<E>& Collectable<E>::operator=(Collectable<E>&& other) noexcept
    {
        if (this != &other)
        {
            generator = std::move(other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    template<typename E>
    bool Collectable<E>::anyMatch(const Predicate<E>& predicate) const {
        return collect<bool, bool>(
            []()->bool { return false; },
            [&](const E& element)->bool { return predicate(element); },
            [&](bool accumulated, const E& element)->bool { return accumulated || predicate(element); },
            [](bool accumulated, bool current)->bool { return accumulated || current; },
            [](bool result)->bool { return result; }
        );
    }

    template<typename E>
    bool Collectable<E>::allMatch(const Predicate<E>& predicate) const {
        return collect<bool, bool>(
            []()->bool { return true; },
            [&](const E& element)->bool { return !predicate(element); },
            [&](bool accumulated, const E& element)->bool { return accumulated && predicate(element); },
            [](bool accumulated, bool current)->bool { return accumulated && current; },
            [](bool result)->bool { return result; }
        );
    }

    template<typename E>
    template<typename A, typename R>
    R Collectable<E>::collect(const Supplier<A>& identity, const BiFunction<A, E, A>& accumulator, const BiFunction<A, A, A>& combiner, const Function<A, R>& finisher) const {
        if (this->concurrent < 2) {
            A result = identity();
            (*generator)([&](const E& element, Timestamp index)->void {
                result = accumulator(result, element);
                }, [](const E& element)->bool { return false; });
            return finisher(result);
        }
        else {
            return globalThreadPool.submit([this, identity, accumulator, combiner, finisher]() -> R {
                A result = identity();
                std::vector<Promise<void>> promises;
                for (Module i = 0; i < this->concurrent; ++i) {
                    promises.emplace_back(globalThreadPool.submit([&, i]() {
                        A localResult = identity();
                        (*generator)([&](const E& element, Timestamp index)->void {
                            localResult = accumulator(localResult, element);
                            }, [](const E& element)->bool { return false; });
                        return;
                        }));
                }
                for (auto& promise : promises) {
                    promise.wait();
                }
                return finisher(result);
                }).getResult();
        }
    }

    template<typename E>
    template<typename A, typename R>
    R Collectable<E>::collect(const Supplier<A>& identity, const Predicate<E>& interrupter, const BiFunction<A, E, A>& accumulator, const BiFunction<A, A, A>& combiner, const Function<A, R>& finisher) const {
        if (this->concurrent < 2) {
            A result = identity();
            bool shouldInterrupt = false;
            (*generator)([&](const E& element, Timestamp index)->void {
                if (!shouldInterrupt) {
                    result = accumulator(result, element);
                    shouldInterrupt = interrupter(element);
                }
                }, [&](const E& element)->bool { return shouldInterrupt; });
            return finisher(result);
        }
        else {
            return globalThreadPool.submit([this, identity, interrupter, accumulator, combiner, finisher]() -> R {
                A result = identity();
                std::atomic<bool> shouldInterrupt(false);
                std::vector<Promise<void>> promises;
                for (Module i = 0; i < this->concurrent; ++i) {
                    promises.emplace_back(globalThreadPool.submit([&, i]() {
                        A localResult = identity();
                        (*generator)([&](const E& element, Timestamp index)->void {
                            if (!shouldInterrupt.load()) {
                                localResult = accumulator(localResult, element);
                                if (interrupter(element)) {
                                    shouldInterrupt.store(true);
                                }
                            }
                            }, [&](const E& element)->bool { return shouldInterrupt.load(); });
                        return;
                        }));
                }
                for (auto& promise : promises) {
                    promise.wait();
                }
                return finisher(result);
                }).getResult();
        }
    }

    template<typename E>
    template<typename A, typename R>
    R Collectable<E>::collect(const Collector<E, A, R>& collector) const {
        return collect(collector.identity, collector.interrupter, collector.accumulator, collector.combiner, collector.finisher);
    }

    template<typename E>
    void Collectable<E>::cout() const {
        cout(std::cout, [](const E& element, std::ostream& os) -> void {
            os << element;
            });
    }

    template<typename E>
    void Collectable<E>::cout(std::ostream& stream) const {
        cout(stream, [](const E& element, std::ostream& os) -> void {
            os << element;
            });
    }

    template<typename E>
    void Collectable<E>::cout(const BiFunction<E, std::ostream&, std::ostream&>& accumulator) const {
        auto formatter = [&accumulator](const E& element, std::ostream& os) -> void {
            accumulator(element, os, os);
            };
        cout(std::cout, formatter);
    }

    template<typename E>
    void Collectable<E>::cout(std::ostream& stream, const BiConsumer<E, std::ostream&>& formatter) const {
        std::string delimiter = ", ";
        std::string result = collect<std::string, std::string>(
            []()->std::string { return ""; },
            [](const E&)->bool { return false; },
            [&](std::string result, const E& element)->std::string {
                std::stringstream localStream;
                if (!result.empty()) {
                    result += delimiter;
                }
                formatter(element, localStream);
                return result + localStream.str();
            },
            [&](std::string result1, std::string result2)->std::string {
                if (result1.empty()) return result2;
                if (result2.empty()) return result1;
                return result1 + delimiter + result2;
            },
            [](std::string result)->std::string { return result; }
        );
        stream << "[" << result << "]";
    }

    template<typename E>
    void Collectable<E>::cout(std::ostream& stream, const std::string& prefix, const BiConsumer<E, std::ostream&>& formatter, const std::string& suffix) const {
        std::string result = collect<std::string, std::string>(
            []()->std::string { return ""; },
            [](const E&)->bool { return false; },
            [&](std::string result, const E& element)->std::string {
                std::stringstream localStream;
                if (!result.empty()) {
                    result += ", ";
                }
                formatter(element, localStream);
                return result + localStream.str();
            },
            [&](std::string result1, std::string result2)->std::string {
                if (result1.empty()) return result2;
                if (result2.empty()) return result1;
                return result1 + ", " + result2;
            },
            [](std::string result)->std::string { return result; }
        );
        stream << prefix << result << suffix;
    }

    template<typename E>
    Module Collectable<E>::count() const {
        return collect<Module, Module>(
            []()->Module { return 0; },
            [](const E& element)->bool { return false; },
            [](Module count, const E& element)->Module { return count + 1; },
            [](Module total, Module partial)->Module { return total + partial; },
            [](Module result)->Module { return result; }
        );
    }

    template<typename E>
    std::optional<E> Collectable<E>::findFirst() const {
        std::atomic<bool> found(false);
        return this->collect<std::optional<E>, std::optional<E>>(
            []()->std::optional<E> { return std::nullopt; },
            [&found](const E& element)->bool { return found.load(); },
            [&found](std::optional<E> result, const E& element)->std::optional<E> {
                if (!result.has_value()) {
                    found.store(true);
                    return element;
                }
                return result;
            },
            [](std::optional<E> a, std::optional<E> b)->std::optional<E> {
                if (a.has_value()) return a;
                if (b.has_value()) return b;
                return std::nullopt;
            },
            [](std::optional<E> result)->std::optional<E> { return result; }
        );
    }

    template<typename E>
    std::optional<E> Collectable<E>::findAny() const {
        std::atomic<bool> found(false);
        return this->collect<std::optional<E>, std::optional<E>>(
            []()->std::optional<E> { return std::nullopt; },
            [&found](const E& element)->bool { return found.load(); },
            [&found](std::optional<E> result, const E& element)->std::optional<E> {
                if (!result.has_value() && randomly<short>(0, 100) > 49) {
                    found.store(true);
                    return element;
                }
                return result;
            },
            [](std::optional<E> a, std::optional<E> b)->std::optional<E> {
                if (a.has_value()) return a;
                if (b.has_value()) return b;
                return std::nullopt;
            },
            [](std::optional<E> result)->std::optional<E> { return result; }
        );
    }

    template<typename E>
    void Collectable<E>::forEach(const Consumer<E>& consumer) const {
        this->collect<Module, Module>(
            []()-> Module { return 0; },
            [](const E& element)->bool { return false; },
            [&](const Module& module, const E& element)->Module {
                consumer(element);
                return module;
            },
            [](const Module& a, const Module& b)-> Module { return a; },
            [](const Module& module)-> Module { return module; }
        );
    }

    template<typename E>
    template<typename K>
    std::map<K, std::vector<E>> Collectable<E>::group(const Function<E, K>& classifier) const {
        return collect<std::map<K, std::vector<E>>, std::map<K, std::vector<E>>>(
            []()->std::map<K, std::vector<E>> { return {}; },
            [](const E& element)->bool { return false; },
            [&](std::map<K, std::vector<E>> groups, const E& element)->std::map<K, std::vector<E>> {
                groups[classifier(element)].push_back(element);
                return groups;
            },
            [](std::map<K, std::vector<E>> groups1, std::map<K, std::vector<E>> groups2)->std::map<K, std::vector<E>> {
                for (const auto& pair : groups2) {
                    groups1[pair.first].insert(groups1[pair.first].end(), pair.second.begin(), pair.second.end());
                }
                return groups1;
            },
            [](std::map<K, std::vector<E>> result)->std::map<K, std::vector<E>> { return result; }
        );
    }

    template<typename E>
    template<typename K, typename V>
    std::map<K, std::vector<V>> Collectable<E>::groupBy(const Function<E, K>& keyExtractor, const Function<E, V>& valueExtractor) const {
        return collect<std::map<K, std::vector<V>>, std::map<K, std::vector<V>>>(
            []()->std::map<K, std::vector<V>> { return {}; },
            [](const E& element)->bool { return false; },
            [&](std::map<K, std::vector<V>> groups, const E& element)->std::map<K, std::vector<V>> {
                groups[keyExtractor(element)].push_back(valueExtractor(element));
                return groups;
            },
            [](std::map<K, std::vector<V>> groups1, std::map<K, std::vector<V>> groups2)->std::map<K, std::vector<V>> {
                for (const auto& pair : groups2) {
                    groups1[pair.first].insert(groups1[pair.first].end(), pair.second.begin(), pair.second.end());
                }
                return groups1;
            },
            [](std::map<K, std::vector<V>> result)->std::map<K, std::vector<V>> { return result; }
        );
    }

    template<typename E>
    std::string Collectable<E>::join() const {
        return join(",", "[", "]");
    }

    template<typename E>
    std::string Collectable<E>::join(const std::string& delimiter) const {
        return join(delimiter, "[", "]");
    }

    template<typename E>
    std::string Collectable<E>::join(const std::string& delimiter, const std::string& prefix, const std::string& suffix) const {
        if (this->concurrent < 2) {
            std::stringstream stream;
            bool first = true;

            (*generator)([&](const E& element, Timestamp index)->void {
                if (!first) {
                    stream << delimiter;
                }
                stream << element;
                first = false;
                }, [](const E& element)->bool { return false; });

            return prefix + stream.str() + suffix;
        }
        else {
            std::vector<std::future<std::string>> futures;

            for (Module i = 0; i < this->concurrent; ++i) {
                futures.push_back(std::async(std::launch::async, [&]()->std::string {
                    std::stringstream localStream;
                    bool localFirst = true;

                    (*generator)([&](const E& element, Timestamp index)->void {
                        if (!localFirst) {
                            localStream << delimiter;
                        }
                        localStream << element;
                        localFirst = false;
                        }, [](const E& element)->bool { return false; });

                    return localStream.str();
                    }));
            }

            std::stringstream finalStream;
            bool globalFirst = true;

            for (auto& future : futures) {
                std::string partial = future.get();
                if (!partial.empty()) {
                    if (!globalFirst) {
                        finalStream << delimiter;
                    }
                    finalStream << partial;
                    globalFirst = false;
                }
            }

            return prefix + finalStream.str() + suffix;
        }
    }

    template<typename E>
    bool Collectable<E>::noneMatch(const Predicate<E>& predicate) const {
        return collect<bool, bool>(
            []()->bool { return true; },
            [&](const E& element)->bool { return predicate(element); },
            [&](bool accumulated, const E& element)->bool { return accumulated && !predicate(element); },
            [](bool accumulated, bool current)->bool { return accumulated && current; },
            [](bool result)->bool { return result; }
        );
    }

    template<typename E>
    std::vector<std::vector<E>> Collectable<E>::partition(const Module& count) const {
        return collect<std::vector<std::vector<E>>, std::vector<std::vector<E>>>(
            [count]()->std::vector<std::vector<E>> { return std::vector<std::vector<E>>(count); },
            [](const E& element)->bool { return false; },
            [&, index = 0](std::vector<std::vector<E>> partitions, const E& element) mutable -> std::vector<std::vector<E>> {
                partitions[index % partitions.size()].push_back(element);
                ++index;
                return partitions;
            },
            [](std::vector<std::vector<E>> partitions1, std::vector<std::vector<E>> partitions2)->std::vector<std::vector<E>> {
                for (size_t i = 0; i < partitions1.size(); ++i) {
                    partitions1[i].insert(partitions1[i].end(), partitions2[i].begin(), partitions2[i].end());
                }
                return partitions1;
            },
            [](std::vector<std::vector<E>> result)->std::vector<std::vector<E>> { return result; }
        );
    }

    template<typename E>
    std::vector<std::vector<E>> Collectable<E>::partitionBy(const Function<E, Module>& classifier) const {
        return collect<std::vector<std::vector<E>>, std::vector<std::vector<E>>>(
            []()->std::vector<std::vector<E>> { return {}; },
            [](const E& element)->bool { return false; },
            [&](std::vector<std::vector<E>> partitions, const E& element)->std::vector<std::vector<E>> {
                Module partitionIndex = classifier(element);
                if (partitionIndex >= partitions.size()) {
                    partitions.resize(partitionIndex + 1);
                }
                partitions[partitionIndex].push_back(element);
                return partitions;
            },
            [](std::vector<std::vector<E>> partitions1, std::vector<std::vector<E>> partitions2)->std::vector<std::vector<E>> {
                for (size_t i = 0; i < partitions2.size(); ++i) {
                    if (i >= partitions1.size()) {
                        partitions1.push_back(partitions2[i]);
                    }
                    else {
                        partitions1[i].insert(partitions1[i].end(), partitions2[i].begin(), partitions2[i].end());
                    }
                }
                return partitions1;
            },
            [](std::vector<std::vector<E>> result)->std::vector<std::vector<E>> { return result; }
        );
    }

    template<typename E>
    std::optional<E> Collectable<E>::reduce(const BiFunction<E, E, E>& accumulator) const {
        return collect<std::optional<E>, std::optional<E>>(
            []()->std::optional<E> { return std::nullopt; },
            [](const E& element)->bool { return false; },
            [&](std::optional<E> result, const E& element)->std::optional<E> {
                if (!result.has_value()) {
                    return element;
                }
                return accumulator(*result, element);
            },
            [&](std::optional<E> a, std::optional<E> b)->std::optional<E> {
                if (!a.has_value()) return b;
                if (!b.has_value()) return a;
                return accumulator(*a, *b);
            },
            [](std::optional<E> result)->std::optional<E> { return result; }
        );
    }

    template<typename E>
    E Collectable<E>::reduce(const E& identity, const BiFunction<E, E, E>& accumulator) const {
        return collect<E, E>(
            [&]()->E { return identity; },
            [](const E& element)->bool { return false; },
            accumulator,
            accumulator,
            [](E result)->E { return result; }
        );
    }

    template<typename E>
    template <typename R>
    R Collectable<E>::reduce(const R& identity, const BiFunction<R, E, R>& accumulator, const BiFunction<R, R, R>& combiner) const {
        return collect<R, R>(
            [&]()->R { return identity; },
            [](const E& element)->bool { return false; },
            accumulator,
            combiner,
            [](R result)->R { return result; }
        );
    }

    template<typename E>
    Semantic<E> Collectable<E>::semantic() const {
        return iterate(this->generator);
    }

    template<typename E>
    std::list<E> Collectable<E>::toList() const {
        return collect<std::list<E>, std::list<E>>(
            []()->std::list<E> { return {}; },
            [](const E& element)->bool { return false; },
            [](std::list<E> list, const E& element)->std::list<E> {
                list.push_back(element);
                return list;
            },
            [](std::list<E> list1, std::list<E> list2)->std::list<E> {
                list1.splice(list1.end(), list2);
                return list1;
            },
            [](std::list<E> result)->std::list<E> { return result; }
        );
    }

    template<typename E>
    template <typename K, typename V>
    std::map<K, V> Collectable<E>::toMap(const Function<E, K>& keyExtractor, const Function<E, V>& valueExtractor) const {
        return collect<std::map<K, V>, std::map<K, V>>(
            []()->std::map<K, V> { return {}; },
            [](const E& element)->bool { return false; },
            [&](std::map<K, V> map, const E& element)->std::map<K, V> {
                map[keyExtractor(element)] = valueExtractor(element);
                return map;
            },
            [](std::map<K, V> map1, std::map<K, V> map2)->std::map<K, V> {
                map1.insert(map2.begin(), map2.end());
                return map1;
            },
            [](std::map<K, V> result)->std::map<K, V> { return result; }
        );
    }

    template<typename E>
    std::set<E> Collectable<E>::toSet() const {
        return collect<std::set<E>, std::set<E>>(
            []()->std::set<E> { return {}; },
            [](const E& element)->bool { return false; },
            [](std::set<E> set, const E& element)->std::set<E> {
                set.insert(element);
                return set;
            },
            [](std::set<E> set1, std::set<E> set2)->std::set<E> {
                set1.insert(set2.begin(), set2.end());
                return set1;
            },
            [](std::set<E> result)->std::set<E> { return result; }
        );
    }

    template<typename E>
    std::unordered_set<E> Collectable<E>::toUnorderedSet() const {
        return collect<std::unordered_set<E>, std::unordered_set<E>>(
            []()->std::unordered_set<E> { return {}; },
            [](const E& element)->bool { return false; },
            [](std::unordered_set<E> set, const E& element)->std::unordered_set<E> {
                set.insert(element);
                return set;
            },
            [](std::unordered_set<E> set1, std::unordered_set<E> set2)->std::unordered_set<E> {
                set1.insert(set2.begin(), set2.end());
                return set1;
            },
            [](std::unordered_set<E> result)->std::unordered_set<E> { return result; }
        );
    }

    template<typename E>
    std::vector<E> Collectable<E>::toVector() const {
        return collect<std::vector<E>, std::vector<E>>(
            []()->std::vector<E> { return {}; },
            [](const E& element)->bool { return false; },
            [](std::vector<E> vector, const E& element)->std::vector<E> {
                vector.push_back(element);
                return vector;
            },
            [](std::vector<E> vector1, std::vector<E> vector2)->std::vector<E> {
                vector1.insert(vector1.end(), vector2.begin(), vector2.end());
                return vector1;
            },
            [](std::vector<E> result)->std::vector<E> { return result; }
        );
    }

    //OrderedCollectable
    template <typename E>
    OrderedCollectable<E>::OrderedCollectable()
        : Collectable<E>(), container({}) {}

    template <typename E>
    OrderedCollectable<E>::OrderedCollectable(const Container& container)
        : Collectable<E>(), container(container) {}

    template <typename E>
    OrderedCollectable<E>::OrderedCollectable(const Generator<E>& generator)
        : Collectable<E>(generator)
    {
        container = toIndexedSet();
    }

    template <typename E>
    OrderedCollectable<E>::OrderedCollectable(const Generator<E>& generator, const Module& concurrent)
        : Collectable<E>(generator, concurrent)
    {
        container = toIndexedSet();
    }

    template <typename E>
    OrderedCollectable<E>::OrderedCollectable(std::shared_ptr<Generator<E>> generator)
        : Collectable<E>(generator)
    {
        container = toIndexedSet();
    }

    template <typename E>
    OrderedCollectable<E>::OrderedCollectable(std::shared_ptr<Generator<E>> generator, const Module& concurrent)
        : Collectable<E>(generator, concurrent)
    {
        container = toIndexedSet();
    }

    template <typename E>
    OrderedCollectable<E>::OrderedCollectable(const OrderedCollectable& other)
        : Collectable<E>(other), container(other.container) {}

    template <typename E>
    OrderedCollectable<E>::OrderedCollectable(OrderedCollectable<E>&& other) noexcept
        : Collectable<E>(std::move(other)), container(std::move(other.container)) {}

    template <typename E>
    OrderedCollectable<E>& OrderedCollectable<E>::operator=(const Collectable<E>& other)
    {
        if (this != &other)
        {
            Collectable<E>::operator=(other);
            container.clear();
        }
        return *this;
    }

    template <typename E>
    OrderedCollectable<E>& OrderedCollectable<E>::operator=(Collectable<E>&& other) noexcept
    {
        if (this != &other)
        {
            Collectable<E>::operator=(std::move(other));
            container.clear();
        }
        return *this;
    }

    template<typename E>
    typename OrderedCollectable<E>::Container OrderedCollectable<E>::toIndexedSet() const {
        const Function<Container, Container>& arrange = [](const Container& container) -> Container {
            Module size = static_cast<Module>(container.size());
            Container result;
            for (std::pair pair : container) {
                Timestamp first = pair.first > 0 ? (pair.first % size) : (size - (std::abs(pair.first) % size));
                Timestamp second = pair.second;
                result.insert(std::make_pair(first, second));
            }
            return result;
            };
        if (this->concurrent < 2) {
            Container container;
            (*this->generator)([&](const E& element, const Timestamp index)->void {
                container.insert(std::make_pair(index, element));
                }, [](const E& element)->bool { return false; });
            return arrange(container);
        }
        else {
            std::vector<std::future<Container>> futures;
            std::mutex containerMutex;

            for (Module i = 0; i < this->concurrent; ++i) {
                futures.push_back(std::async(std::launch::async, [&]()->Container {
                    Container localContainer;
                    (*this->generator)([&](const E& element, Timestamp index)->void {
                        std::lock_guard<std::mutex> lock(containerMutex);
                        localContainer.emplace(index, element);
                        }, [](const E& element)->bool { return false; });
                    return localContainer;
                    }));
            }

            Container container;
            for (auto& future : futures) {
                Container partial = future.get();
                for (const auto& pair : partial) {
                    container.insert(pair);
                }
            }
            return arrange(container);
        }
    }

    template<typename E>
    bool OrderedCollectable<E>::anyMatch(const Predicate<E>& predicate) const {
        return collect<bool, bool>(
            []()->bool { return false; },
            [&](const E& element)->bool { return predicate(element); },
            [&](bool accumulated, const E& element)->bool { return accumulated || predicate(element); },
            [](bool accumulated, bool current)->bool { return accumulated || current; },
            [](bool result)->bool { return result; }
        );
    }

    template<typename E>
    bool OrderedCollectable<E>::allMatch(const Predicate<E>& predicate) const {
        return collect<bool, bool>(
            []()->bool { return true; },
            [&](const E& element)->bool { return !predicate(element); },
            [&](bool accumulated, const E& element)->bool { return accumulated && predicate(element); },
            [](bool accumulated, bool current)->bool { return accumulated && current; },
            [](bool result)->bool { return result; }
        );
    }

    template<typename E>
    template<typename A, typename R>
    R OrderedCollectable<E>::collect(const Supplier<A>& identity, const BiFunction<A, E, A>& accumulator, const BiFunction<A, A, A>& combiner, const Function<A, R>& finisher) const {
        if (this->concurrent < 2) {
            A result = identity();
            for (const auto& pair : container) {
                result = accumulator(result, pair.second);
            }
            return finisher(result);
        }
        else {
            std::vector<Promise<A>> promises;
            A finalResult = identity();

            for (Module i = 0; i < this->concurrent; ++i) {
                promises.emplace_back(globalThreadPool.submit([&, i]() -> A {
                    A localResult = identity();
                    auto localBegin = container.begin();
                    std::advance(localBegin, i * (container.size() / this->concurrent));
                    auto localEnd = (i == this->concurrent - 1) ? container.end() : container.begin() + (i + 1) * (container.size() / this->concurrent);
                    for (auto it = localBegin; it != localEnd; ++it) {
                        localResult = accumulator(localResult, it->second);
                    }
                    return localResult;
                    }));
            }

            for (auto& promise : promises) {
                finalResult = combiner(finalResult, promise.getResult());
            }

            return finisher(finalResult);
        }
    }

    template<typename E>
    template<typename A, typename R>
    R OrderedCollectable<E>::collect(const Supplier<A>& identity, const Predicate<E>& interrupter, const BiFunction<A, E, A>& accumulator, const BiFunction<A, A, A>& combiner, const Function<A, R>& finisher) const {
        if (this->concurrent < 2) {
            A result = identity();
            bool shouldInterrupt = false;
            for (const auto& pair : container) {
                if (shouldInterrupt) break;
                result = accumulator(result, pair.second);
                shouldInterrupt = interrupter(pair.second);
            }
            return finisher(result);
        }
        else {
            std::atomic<bool> shouldInterruptFlag(false);
            std::vector<Promise<A>> promises;
            A finalResult = identity();

            for (Module i = 0; i < this->concurrent; ++i) {
                promises.emplace_back(globalThreadPool.submit([&, i]() -> A {
                    A localResult = identity();
                    auto localBegin = container.begin();
                    std::advance(localBegin, i * (container.size() / this->concurrent));
                    auto localEnd = (i == this->concurrent - 1) ? container.end() : container.begin() + (i + 1) * (container.size() / this->concurrent);
                    for (auto it = localBegin; it != localEnd; ++it) {
                        localResult = accumulator(localResult, it->second);
                        if (interrupter(it->second)) {
                            shouldInterruptFlag.store(true);
                            break;
                        }
                    }
                    return localResult;
                    }));
            }

            for (auto& promise : promises) {
                finalResult = combiner(finalResult, promise.getResult());
                if (shouldInterruptFlag.load()) break;
            }

            return finisher(finalResult);
        }
    }

    template<typename E>
    template<typename A, typename R>
    R OrderedCollectable<E>::collect(const Collector<E, A, R>& collector) const {
        return collect(collector.identity, collector.interrupter, collector.accumulator, collector.combiner, collector.finisher);
    }

    template<typename E>
    void OrderedCollectable<E>::cout() const {
        cout(std::cout, [](const E& element, std::ostream& os) -> std::ostream& {
            return os << element;
            });
    }

    template<typename E>
    void OrderedCollectable<E>::cout(const BiFunction<E, std::ostream&, std::ostream&>& accumulator) const {
        auto formatter = [&accumulator](const E& element, std::ostream& os) -> void {
            accumulator(element, os, os);
            };
        cout(std::cout, "", formatter, "");
    }

    template<typename E>
    void OrderedCollectable<E>::cout(std::ostream& stream) const {
        cout(stream, [](const E& element, std::ostream& os) -> std::ostream& {
            return os << element;
            });
    }

    template<typename E>
    void OrderedCollectable<E>::cout(std::ostream& stream, const BiConsumer<E, std::ostream&>& formatter) const {
        cout(stream, "", formatter, "");
    }

    template<typename E>
    void OrderedCollectable<E>::cout(std::ostream& stream, const std::string& prefix, const BiConsumer<E, std::ostream&>& formatter, const std::string& suffix) const {
        std::string result = collect<std::string, std::string>(
            []()->std::string { return ""; },
            [](const E&)->bool { return false; },
            [&](std::string result, const E& element)->std::string {
                std::stringstream localStream;
                if (!result.empty()) {
                    result += ", ";
                }
                formatter(element, localStream);
                return result + localStream.str();
            },
            [&](std::string result1, std::string result2)->std::string {
                if (result1.empty()) return result2;
                if (result2.empty()) return result1;
                return result1 + ", " + result2;
            },
            [](std::string result)->std::string { return result; }
        );
        stream << prefix << result << suffix;
    }

    template<typename E>
    Module OrderedCollectable<E>::count() const {
        return static_cast<Module>(container.size());
    }

    template<typename E>
    std::optional<E> OrderedCollectable<E>::findFirst() const {
        std::atomic<bool> found(false);
        return this->collect<std::optional<E>, std::optional<E>>(
            []()->std::optional<E> { return std::nullopt; },
            [&found](const E& element)->bool { return found.load(); },
            [&found](std::optional<E> result, const E& element)->std::optional<E> {
                if (!result.has_value()) {
                    found.store(true);
                    return element;
                }
                return result;
            },
            [](std::optional<E> a, std::optional<E> b)->std::optional<E> {
                if (a.has_value()) return a;
                if (b.has_value()) return b;
                return std::nullopt;
            },
            [](std::optional<E> result)->std::optional<E> { return result; }
        );
    }

    template<typename E>
    std::optional<E> OrderedCollectable<E>::findAny() const {
        std::atomic<bool> found(false);
        return this->collect<std::optional<E>, std::optional<E>>(
            []()->std::optional<E> { return std::nullopt; },
            [&found](const E& element)->bool { return found.load(); },
            [&found](std::optional<E> result, const E& element)->std::optional<E> {
                if (!result.has_value() && randomly<short>(0, 100) > 49) {
                    found.store(true);
                    return element;
                }
                return result;
            },
            [](std::optional<E> a, std::optional<E> b)->std::optional<E> {
                if (a.has_value()) return a;
                if (b.has_value()) return b;
                return std::nullopt;
            },
            [](std::optional<E> result)->std::optional<E> { return result; }
        );
    }

    template<typename E>
    void OrderedCollectable<E>::forEach(const Consumer<E>& consumer) const {
        this->collect<Module, Module>(
            []()-> Module { return 0; },
            [](const E& element)->bool { return false; },
            [&](const Module& module, const E& element)->Module {
                consumer(element);
                return module;
            },
            [](const Module& a, const Module& b)-> Module { return a; },
            [](const Module& module)-> Module { return module; }
        );
    }

    template<typename E>
    template<typename K>
    std::map<K, std::vector<E>> OrderedCollectable<E>::group(const Function<E, K>& classifier) const {
        return collect<std::map<K, std::vector<E>>, std::map<K, std::vector<E>>>(
            []()->std::map<K, std::vector<E>> { return {}; },
            [](const E& element)->bool { return false; },
            [&](std::map<K, std::vector<E>> groups, const E& element)->std::map<K, std::vector<E>> {
                groups[classifier(element)].push_back(element);
                return groups;
            },
            [](std::map<K, std::vector<E>> groups1, std::map<K, std::vector<E>> groups2)->std::map<K, std::vector<E>> {
                for (const auto& pair : groups2) {
                    groups1[pair.first].insert(groups1[pair.first].end(), pair.second.begin(), pair.second.end());
                }
                return groups1;
            },
            [](std::map<K, std::vector<E>> result)->std::map<K, std::vector<E>> { return result; }
        );
    }

    template<typename E>
    template<typename K, typename V>
    std::map<K, std::vector<V>> OrderedCollectable<E>::groupBy(const Function<E, K>& keyExtractor, const Function<E, V>& valueExtractor) const {
        return collect<std::map<K, std::vector<V>>, std::map<K, std::vector<V>>>(
            []()->std::map<K, std::vector<V>> { return {}; },
            [](const E& element)->bool { return false; },
            [&](std::map<K, std::vector<V>> groups, const E& element)->std::map<K, std::vector<V>> {
                groups[keyExtractor(element)].push_back(valueExtractor(element));
                return groups;
            },
            [](std::map<K, std::vector<V>> groups1, std::map<K, std::vector<V>> groups2)->std::map<K, std::vector<V>> {
                for (const auto& pair : groups2) {
                    groups1[pair.first].insert(groups1[pair.first].end(), pair.second.begin(), pair.second.end());
                }
                return groups1;
            },
            [](std::map<K, std::vector<V>> result)->std::map<K, std::vector<V>> { return result; }
        );
    }

    template<typename E>
    std::string OrderedCollectable<E>::join() const {
        return join(",", "[", "]");
    }

    template<typename E>
    std::string OrderedCollectable<E>::join(const std::string& delimiter) const {
        return join(delimiter, "[", "]");
    }

    template<typename E>
    std::string OrderedCollectable<E>::join(const std::string& delimiter, const std::string& prefix, const std::string& suffix) const {
        return collect<std::string, std::string>(
            []()->std::string { return ""; },
            [](const E& element)->bool { return false; },
            [&](std::string result, const E& element)->std::string {
                if (!result.empty()) {
                    result += delimiter;
                }
                std::stringstream ss;
                ss << element;
                return result + ss.str();
            },
            [&](std::string result1, std::string result2)->std::string {
                if (result1.empty()) return result2;
                if (result2.empty()) return result1;
                return result1 + delimiter + result2;
            },
            [&](std::string result)->std::string {
                return prefix + result + suffix;
            }
        );
    }

    template<typename E>
    bool OrderedCollectable<E>::noneMatch(const Predicate<E>& predicate) const {
        return collect<bool, bool>(
            []()->bool { return true; },
            [&](const E& element)->bool { return predicate(element); },
            [&](bool accumulated, const E& element)->bool { return accumulated && !predicate(element); },
            [](bool accumulated, bool current)->bool { return accumulated && current; },
            [](bool result)->bool { return result; }
        );
    }

    template<typename E>
    std::vector<std::vector<E>> OrderedCollectable<E>::partition(const Module& count) const {
        return collect<std::vector<std::vector<E>>, std::vector<std::vector<E>>>(
            [count]()->std::vector<std::vector<E>> { return std::vector<std::vector<E>>(count); },
            [](const E& element)->bool { return false; },
            [&, index = 0](std::vector<std::vector<E>> partitions, const E& element) mutable -> std::vector<std::vector<E>> {
                partitions[index % partitions.size()].push_back(element);
                ++index;
                return partitions;
            },
            [](std::vector<std::vector<E>> partitions1, std::vector<std::vector<E>> partitions2)->std::vector<std::vector<E>> {
                for (size_t i = 0; i < partitions1.size(); ++i) {
                    partitions1[i].insert(partitions1[i].end(), partitions2[i].begin(), partitions2[i].end());
                }
                return partitions1;
            },
            [](std::vector<std::vector<E>> result)->std::vector<std::vector<E>> { return result; }
        );
    }

    template<typename E>
    std::vector<std::vector<E>> OrderedCollectable<E>::partitionBy(const Function<E, Module>& classifier) const {
        return collect<std::vector<std::vector<E>>, std::vector<std::vector<E>>>(
            []()->std::vector<std::vector<E>> { return {}; },
            [](const E& element)->bool { return false; },
            [&](std::vector<std::vector<E>> partitions, const E& element)->std::vector<std::vector<E>> {
                Module partitionIndex = classifier(element);
                if (partitionIndex >= partitions.size()) {
                    partitions.resize(partitionIndex + 1);
                }
                partitions[partitionIndex].push_back(element);
                return partitions;
            },
            [](std::vector<std::vector<E>> partitions1, std::vector<std::vector<E>> partitions2)->std::vector<std::vector<E>> {
                for (size_t i = 0; i < partitions2.size(); ++i) {
                    if (i >= partitions1.size()) {
                        partitions1.push_back(partitions2[i]);
                    }
                    else {
                        partitions1[i].insert(partitions1[i].end(), partitions2[i].begin(), partitions2[i].end());
                    }
                }
                return partitions1;
            },
            [](std::vector<std::vector<E>> result)->std::vector<std::vector<E>> { return result; }
        );
    }

    template<typename E>
    std::optional<E> OrderedCollectable<E>::reduce(const BiFunction<E, E, E>& accumulator) const {
        return collect<std::optional<E>, std::optional<E>>(
            []()->std::optional<E> { return std::nullopt; },
            [](const E& element)->bool { return false; },
            [&](std::optional<E> result, const E& element)->std::optional<E> {
                if (!result.has_value()) {
                    return element;
                }
                return accumulator(*result, element);
            },
            [&](std::optional<E> a, std::optional<E> b)->std::optional<E> {
                if (!a.has_value()) return b;
                if (!b.has_value()) return a;
                return accumulator(*a, *b);
            },
            [](std::optional<E> result)->std::optional<E> { return result; }
        );
    }

    template<typename E>
    E OrderedCollectable<E>::reduce(const E& identity, const BiFunction<E, E, E>& accumulator) const {
        return collect<E, E>(
            [&]()->E { return identity; },
            [](const E& element)->bool { return false; },
            accumulator,
            accumulator,
            [](E result)->E { return result; }
        );
    }

    template<typename E>
    template <typename R>
    R OrderedCollectable<E>::reduce(const R& identity, const BiFunction<R, E, R>& accumulator, const BiFunction<R, R, R>& combiner) const {
        return collect<R, R>(
            [&]()->R { return identity; },
            [](const E& element)->bool { return false; },
            accumulator,
            combiner,
            [](R result)->R { return result; }
        );
    }

    template<typename E>
    Semantic<E> OrderedCollectable<E>::semantic() const {
        return Semantic<E>([container = this->container](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            for (const auto& pair : container) {
                if (predicate(pair.second)) {
                    break;
                }
                accept(pair.second, pair.first);
            }
            });
    }

    template<typename E>
    std::list<E> OrderedCollectable<E>::toList() const {
        return collect<std::list<E>, std::list<E>>(
            []()->std::list<E> { return {}; },
            [](const E& element)->bool { return false; },
            [](std::list<E> list, const E& element)->std::list<E> {
                list.push_back(element);
                return list;
            },
            [](std::list<E> list1, std::list<E> list2)->std::list<E> {
                list1.splice(list1.end(), list2);
                return list1;
            },
            [](std::list<E> result)->std::list<E> { return result; }
        );
    }

    template<typename E>
    template <typename K, typename V>
    std::map<K, V> OrderedCollectable<E>::toMap(const Function<E, K>& keyExtractor, const Function<E, V>& valueExtractor) const {
        return collect<std::map<K, V>, std::map<K, V>>(
            []()->std::map<K, V> { return {}; },
            [](const E& element)->bool { return false; },
            [&](std::map<K, V> map, const E& element)->std::map<K, V> {
                map[keyExtractor(element)] = valueExtractor(element);
                return map;
            },
            [](std::map<K, V> map1, std::map<K, V> map2)->std::map<K, V> {
                map1.insert(map2.begin(), map2.end());
                return map1;
            },
            [](std::map<K, V> result)->std::map<K, V> { return result; }
        );
    }

    template<typename E>
    std::set<E> OrderedCollectable<E>::toSet() const {
        return collect<std::set<E>, std::set<E>>(
            []()->std::set<E> { return {}; },
            [](const E& element)->bool { return false; },
            [](std::set<E> set, const E& element)->std::set<E> {
                set.insert(element);
                return set;
            },
            [](std::set<E> set1, std::set<E> set2)->std::set<E> {
                set1.insert(set2.begin(), set2.end());
                return set1;
            },
            [](std::set<E> result)->std::set<E> { return result; }
        );
    }

    template<typename E>
    std::unordered_set<E> OrderedCollectable<E>::toUnorderedSet() const {
        return collect<std::unordered_set<E>, std::unordered_set<E>>(
            []()->std::unordered_set<E> { return {}; },
            [](const E& element)->bool { return false; },
            [](std::unordered_set<E> set, const E& element)->std::unordered_set<E> {
                set.insert(element);
                return set;
            },
            [](std::unordered_set<E> set1, std::unordered_set<E> set2)->std::unordered_set<E> {
                set1.insert(set2.begin(), set2.end());
                return set1;
            },
            [](std::unordered_set<E> result)->std::unordered_set<E> { return result; }
        );
    }

    template<typename E>
    std::vector<E> OrderedCollectable<E>::toVector() const {
        return collect<std::vector<E>, std::vector<E>>(
            []()->std::vector<E> { return {}; },
            [](const E& element)->bool { return false; },
            [](std::vector<E> vector, const E& element)->std::vector<E> {
                vector.push_back(element);
                return vector;
            },
            [](std::vector<E> vector1, std::vector<E> vector2)->std::vector<E> {
                vector1.insert(vector1.end(), vector2.begin(), vector2.end());
                return vector1;
            },
            [](std::vector<E> result)->std::vector<E> { return result; }
        );
    }

    //Satistics
    template <typename E, typename D>
    Statistics<E, D>::Statistics()
        : OrderedCollectable<E>() {}

    template <typename E, typename D>
    Statistics<E, D>::Statistics(const Generator<E>& generator)
        : OrderedCollectable<E>(generator) {}

    template <typename E, typename D>
    Statistics<E, D>::Statistics(const Generator<E>& generator, const Module& concurrent)
        : OrderedCollectable<E>(generator, concurrent) {}

    template <typename E, typename D>
    Statistics<E, D>::Statistics(std::shared_ptr<Generator<E>> generator)
        : OrderedCollectable<E>(generator) {}

    template <typename E, typename D>
    Statistics<E, D>::Statistics(std::shared_ptr<Generator<E>> generator, const Module& concurrent)
        : OrderedCollectable<E>(generator, concurrent) {}

    template <typename E, typename D>
    Statistics<E, D>::Statistics(const Statistics<E, D>& other)
        : OrderedCollectable<E>(other), frequencyCache(other.frequencyCache) {}

    template <typename E, typename D>
    Statistics<E, D>::Statistics(Statistics<E, D>&& other) noexcept
        : OrderedCollectable<E>(std::move(other)), frequencyCache(std::move(other.frequencyCache)) {}

    template <typename E, typename D>
    Statistics<E, D>& Statistics<E, D>::operator=(const Collectable<E>& other)
    {
        if (this != &other)
        {
            OrderedCollectable<E>::operator=(other);
            frequencyCache.clear();
        }
        return *this;
    }

    template <typename E, typename D>
    Statistics<E, D>& Statistics<E, D>::operator=(Collectable<E>&& other) noexcept
    {
        if (this != &other)
        {
            OrderedCollectable<E>::operator=(std::move(other));
            frequencyCache.clear();
        }
        return *this;
    }

    template <typename E, typename D>
    Statistics<E, D>& Statistics<E, D>::operator=(const std::list<E>& l)
    {
        this->container.clear();
        frequencyCache.clear();
        Timestamp index = 0;
        for (const auto& element : l)
        {
            this->container.insert(std::make_pair(element, index));
            index++;
        }
        return *this;
    }

    template <typename E, typename D>
    Statistics<E, D>& Statistics<E, D>::operator=(const std::vector<E>& v)
    {
        this->container.clear();
        frequencyCache.clear();
        Timestamp index = 0;
        for (const E& element : v)
        {
            this->container.insert(std::make_pair(element, index));
            index++;
        }
        return *this;
    }

    template <typename E, typename D>
    Statistics<E, D>& Statistics<E, D>::operator=(const std::set<E>& s)
    {
        this->container.clear();
        frequencyCache.clear();
        Timestamp index = 0;
        for (const E& element : s)
        {
            this->container.insert(std::make_pair(element, index));
            index++;
        }
        return *this;
    }

    template <typename E, typename D>
    Statistics<E, D>& Statistics<E, D>::operator=(const std::unordered_set<E>& s)
    {
        this->container.clear();
        frequencyCache.clear();
        Timestamp index = 0;
        for (const E& element : s)
        {
            this->container.insert(std::make_pair(element, index));
            index++;
        }
        return *this;
    }

    template <typename E, typename D>
    Statistics<E, D>& Statistics<E, D>::operator=(const std::initializer_list<E>& l)
    {
        this->container.clear();
        frequencyCache.clear();
        Timestamp index = 0;
        for (const auto& element : l)
        {
            this->container.insert(std::make_pair(element, index));
            index++;
        }
        return *this;
    }

    template <typename E, typename D>
    Statistics<E, D>& Statistics<E, D>::operator=(const Statistics<E, D>& other)
    {
        if (this != &other)
        {
            OrderedCollectable<E>::operator=(other);
            frequencyCache = other.frequencyCache;
        }
        return *this;
    }

    template <typename E, typename D>
    Statistics<E, D>& Statistics<E, D>::operator=(Statistics<E, D>&& other) noexcept
    {
        if (this != &other)
        {
            OrderedCollectable<E>::operator=(std::move(other));
            frequencyCache = std::move(other.frequencyCache);
        }
        return *this;
    }

    template<typename E, typename D>
    Module Statistics<E, D>::count() const {
        return OrderedCollectable<E>::count();
    }

    template<typename E, typename D>
    std::optional<E> Statistics<E, D>::maximum(const Comparator<E, E>& comparator) const {
        return OrderedCollectable<E>::template collect<std::optional<E>, std::optional<E>>(
            []()->std::optional<E> { return std::nullopt; },
            [](const E& element)->bool { return false; },
            [&](std::optional<E> currentMax, const E& element)->std::optional<E> {
                if (!currentMax.has_value()) {
                    return element;
                }
                return comparator(*currentMax, element) >= 0 ? *currentMax : element;
            },
            [&](std::optional<E> a, std::optional<E> b)->std::optional<E> {
                if (!a.has_value()) return b;
                if (!b.has_value()) return a;
                return comparator(*a, *b) >= 0 ? *a : *b;
            },
            [](std::optional<E> result)->std::optional<E> { return result; }
        );
    }

    template<typename E, typename D>
    std::optional<E> Statistics<E, D>::minimum(const Comparator<E, E>& comparator) const {
        return OrderedCollectable<E>::template collect<std::optional<E>, std::optional<E>>(
            []()->std::optional<E> { return std::nullopt; },
            [](const E& element)->bool { return false; },
            [&](std::optional<E> currentMin, const E& element)->std::optional<E> {
                if (!currentMin.has_value()) {
                    return element;
                }
                return comparator(*currentMin, element) <= 0 ? *currentMin : element;
            },
            [&](std::optional<E> a, std::optional<E> b)->std::optional<E> {
                if (!a.has_value()) return b;
                if (!b.has_value()) return a;
                return comparator(*a, *b) <= 0 ? *a : *b;
            },
            [](std::optional<E> result)->std::optional<E> { return result; }
        );
    }

    template<typename E, typename D>
    D Statistics<E, D>::range(const Function<E, D>& mapper) const {
        D minimumValue = this->template collect<D, D>(
            []()->D { return std::numeric_limits<D>::max(); },
            [](const E& element)->bool { return false; },
            [&](D currentMin, const E& element)->D {
                D mappedValue = mapper(element);
                return mappedValue < currentMin ? mappedValue : currentMin;
            },
            [](D a, D b)->D { return a < b ? a : b; },
            [](D result)->D { return result; }
        );

        D maximumValue = this->template collect<D, D>(
            []()->D { return std::numeric_limits<D>::lowest(); },
            [](const E& element)->bool { return false; },
            [&](D currentMax, const E& element)->D {
                D mappedValue = mapper(element);
                return mappedValue > currentMax ? mappedValue : currentMax;
            },
            [](D a, D b)->D { return a > b ? a : b; },
            [](D result)->D { return result; }
        );

        return maximumValue - minimumValue;
    }

    template<typename E, typename D>
    D Statistics<E, D>::variance(const Function<E, D>& mapper) const {
        Module elementCount = count();
        if (elementCount < 2) return 0;

        D meanValue = mean(mapper);

        D sumOfSquares = this->template collect<D, D>(
            []()->D { return 0; },
            [](const E& element)->bool { return false; },
            [&](D sum, const E& element)->D {
                D diff = mapper(element) - meanValue;
                return sum + diff * diff;
            },
            [](D a, D b)->D { return a + b; },
            [](D result)->D { return result; }
        );

        return sumOfSquares / (elementCount - 1);
    }

    template<typename E, typename D>
    D Statistics<E, D>::standardDeviation(const Function<E, D>& mapper) const {
        return std::sqrt(variance(mapper));
    }

    template<typename E, typename D>
    D Statistics<E, D>::mean(const Function<E, D>& mapper) const {
        D sumValue = sum(mapper);
        Module elementCount = count();
        return elementCount == 0 ? 0 : sumValue / elementCount;
    }

    template<typename E, typename D>
    D Statistics<E, D>::median(const Function<E, D>& mapper) const {
        std::vector<D> sortedValues = this->template collect<std::vector<D>, std::vector<D>>(
            []()->std::vector<D> { return {}; },
            [](const E& element)->bool { return false; },
            [&](std::vector<D> vector, const E& element)->std::vector<D> {
                vector.push_back(mapper(element));
                return vector;
            },
            [](std::vector<D> vector1, std::vector<D> vector2)->std::vector<D> {
                vector1.insert(vector1.end(), vector2.begin(), vector2.end());
                return vector1;
            },
            [](std::vector<D> result)->std::vector<D> {
                std::sort(result.begin(), result.end());
                return result;
            }
        );

        Module elementCount = sortedValues.size();
        if (elementCount == 0) return 0;

        if (elementCount % 2 == 0) {
            return (sortedValues[elementCount / 2 - 1] + sortedValues[elementCount / 2]) / 2;
        }
        else {
            return sortedValues[elementCount / 2];
        }
    }

    template<typename E, typename D>
    D Statistics<E, D>::mode(const Function<E, D>& mapper) const {
        std::map<D, Module> frequencies = frequency(mapper);

        D modeValue = 0;
        Module maxFrequency = 0;
        bool foundMode = false;

        for (const auto& pair : frequencies) {
            if (pair.second > maxFrequency) {
                modeValue = pair.first;
                maxFrequency = pair.second;
                foundMode = true;
            }
        }

        return foundMode ? modeValue : 0;
    }

    template<typename E, typename D>
    std::map<D, Module> Statistics<E, D>::frequency(const Function<E, D>& mapper) const {
        if (frequencyCache.empty()) {
            frequencyCache = this->template collect<std::map<D, Module>, std::map<D, Module>>(
                []()->std::map<D, Module> { return {}; },
                [](const E& element)->bool { return false; },
                [&](std::map<D, Module> frequencies, const E& element)->std::map<D, Module> {
                    D mappedValue = mapper(element);
                    frequencies[mappedValue]++;
                    return frequencies;
                },
                [](std::map<D, Module> frequencies1, std::map<D, Module> frequencies2)->std::map<D, Module> {
                    for (const auto& pair : frequencies2) {
                        frequencies1[pair.first] += pair.second;
                    }
                    return frequencies1;
                },
                [](std::map<D, Module> result)->std::map<D, Module> { return result; }
            );
        }
        return frequencyCache;
    }

    template<typename E, typename D>
    D Statistics<E, D>::sum(const Function<E, D>& mapper) const {
        return this->template collect<D, D>(
            []()->D { return 0; },
            [](const E& element)->bool { return false; },
            [&](D sum, const E& element)->D { return sum + mapper(element); },
            [](D a, D b)->D { return a + b; },
            [](D result)->D { return result; }
        );
    }

    template<typename E, typename D>
    std::vector<D> Statistics<E, D>::quartiles(const Function<E, D>& mapper) const {
        std::vector<D> sortedValues = this->template collect<std::vector<D>, std::vector<D>>(
            []()->std::vector<D> { return {}; },
            [](const E& element)->bool { return false; },
            [&](std::vector<D> vector, const E& element)->std::vector<D> {
                vector.push_back(mapper(element));
                return vector;
            },
            [](std::vector<D> vector1, std::vector<D> vector2)->std::vector<D> {
                vector1.insert(vector1.end(), vector2.begin(), vector2.end());
                return vector1;
            },
            [](std::vector<D> result)->std::vector<D> {
                std::sort(result.begin(), result.end());
                return result;
            }
        );

        Module elementCount = sortedValues.size();
        std::vector<D> quartiles;

        if (elementCount > 0) {
            D q1 = sortedValues[elementCount / 4];
            D q2 = median(mapper);
            D q3 = sortedValues[(3 * elementCount) / 4];
            quartiles = { q1, q2, q3 };
        }

        return quartiles;
    }

    template<typename E, typename D>
    D Statistics<E, D>::interquartileRange(const Function<E, D>& mapper) const {
        std::vector<D> quartilesResult = quartiles(mapper);
        if (quartilesResult.size() >= 2) {
            return quartilesResult[2] - quartilesResult[0];
        }
        return 0;
    }

    template<typename E, typename D>
    D Statistics<E, D>::skewness(const Function<E, D>& mapper) const {
        Module elementCount = count();
        if (elementCount < 3) return 0;

        D meanValue = mean(mapper);
        D standardDeviationValue = standardDeviation(mapper);
        if (standardDeviationValue == 0) return 0;

        D sumOfCubes = this->template collect<D, D>(
            []()->D { return 0; },
            [](const E& element)->bool { return false; },
            [&](D sum, const E& element)->D {
                D diff = mapper(element) - meanValue;
                return sum + diff * diff * diff;
            },
            [](D a, D b)->D { return a + b; },
            [](D result)->D { return result; }
        );

        D numerator = sumOfCubes / elementCount;
        D denominator = standardDeviationValue * standardDeviationValue * standardDeviationValue;

        return numerator / denominator;
    }

    template<typename E, typename D>
    D Statistics<E, D>::kurtosis(const Function<E, D>& mapper) const {
        Module elementCount = count();
        if (elementCount < 4) return 0;

        D meanValue = mean(mapper);
        D standardDeviationValue = standardDeviation(mapper);
        if (standardDeviationValue == 0) return 0;

        D sumOfQuartics = this->template collect<D, D>(
            []()->D { return 0; },
            [](const E& element)->bool { return false; },
            [&](D sum, const E& element)->D {
                D diff = mapper(element) - meanValue;
                return sum + diff * diff * diff * diff;
            },
            [](D a, D b)->D { return a + b; },
            [](D result)->D { return result; }
        );

        D numerator = sumOfQuartics / elementCount;
        D denominator = standardDeviationValue * standardDeviationValue * standardDeviationValue * standardDeviationValue;

        return numerator / denominator - 3;
    }

    template<typename E, typename D>
    bool Statistics<E, D>::isEmpty() const {
        return count() == 0;
    }

    template<typename E, typename D>
    void Statistics<E, D>::clear() {
        frequencyCache.clear();
        this->generator = std::make_shared<Generator<E>>([](const Consumer<E>&, const Predicate<E>&) {});
        this->container.clear();
    }

    //WindowCollectable
    template <typename E>
    WindowCollectable<E>::WindowCollectable()
        : OrderedCollectable<E>() {}

    template <typename E>
    WindowCollectable<E>::WindowCollectable(const Container& container)
        : OrderedCollectable<E>(container) {}

    template <typename E>
    WindowCollectable<E>::WindowCollectable(const Generator<E>& generator)
        : OrderedCollectable<E>(generator) {}

    template <typename E>
    WindowCollectable<E>::WindowCollectable(const Generator<E>& generator, const Module& concurrent)
        : OrderedCollectable<E>(generator, concurrent) {}

    template <typename E>
    WindowCollectable<E>::WindowCollectable(std::shared_ptr<Generator<E>> generator)
        : OrderedCollectable<E>(generator) {}

    template <typename E>
    WindowCollectable<E>::WindowCollectable(std::shared_ptr<Generator<E>> generator, const Module& concurrent)
        : OrderedCollectable<E>(generator, concurrent) {}

    template <typename E>
    WindowCollectable<E>::WindowCollectable(const WindowCollectable& other)
        : OrderedCollectable<E>(other) {}

    template <typename E>
    WindowCollectable<E>::WindowCollectable(WindowCollectable&& other) noexcept
        : OrderedCollectable<E>(std::move(other)) {}

    template <typename E>
    WindowCollectable<E>& WindowCollectable<E>::operator=(const OrderedCollectable<E>& other)
    {
        if (this != &other)
        {
            OrderedCollectable<E>::operator=(other);
        }
        return *this;
    }

    template <typename E>
    WindowCollectable<E>& WindowCollectable<E>::operator=(OrderedCollectable<E>&& other) noexcept
    {
        if (this != &other)
        {
            OrderedCollectable<E>::operator=(std::move(other));
        }
        return *this;
    }

    template <typename E>
    WindowCollectable<E>& WindowCollectable<E>::operator=(const WindowCollectable<E>& other)
    {
        if (this != &other)
        {
            OrderedCollectable<E>::operator=(other);
        }
        return *this;
    }

    template <typename E>
    WindowCollectable<E>& WindowCollectable<E>::operator=(WindowCollectable<E>&& other) noexcept
    {
        if (this != &other)
        {
            OrderedCollectable<E>::operator=(std::move(other));
        }
        return *this;
    }

    template<typename E>
    typename WindowCollectable<E>::Container WindowCollectable<E>::convertToContainer() const {
        return this->container;
    }

    template<typename E>
    Semantic<Semantic<E>> WindowCollectable<E>::slide(const Module& size, const Module& step) const {
        if(size > 0 && step > 0){
            std::vector<std::vector<E>> windows;
            Module windowStartIndex = 0;
            Module sourceSize = static_cast<Module>(source.size());
            while (windowStartIndex < sourceSize) {
                Module windowEnd = windowStartIndex + windowSize;
                std::vector<E> window;
                window.reserve(static_cast<size_t>(std::min(static_cast<Module>(windowSize), sourceSize - windowStartIndex)));
                for (Module i = windowStartIndex; i < windowEnd && i < sourceSize; ++i) {
                    window.push_back(source[static_cast<size_t>(i)]);
                }
                windows.push_back(std::move(window));
                windowStartIndex += step;
            }
        }
        return empty<Semantic<E>>();
    }

    template<typename E>
    Semantic<Semantic<E>> WindowCollectable<E>::tumble(const Module& size) const {
        return this-> side(size, size);
    }

    //UnorderedCollectable
    template <typename E>
    UnorderedCollectable<E>::UnorderedCollectable()
        : Collectable<E>() {}

    template <typename E>
    UnorderedCollectable<E>::UnorderedCollectable(const Generator<E>& generator)
        : Collectable<E>(generator) {}

    template <typename E>
    UnorderedCollectable<E>::UnorderedCollectable(const Generator<E>& generator, const Module& concurrent)
        : Collectable<E>(generator, concurrent) {}

    template <typename E>
    UnorderedCollectable<E>::UnorderedCollectable(std::shared_ptr<Generator<E>> generator)
        : Collectable<E>(generator) {}

    template <typename E>
    UnorderedCollectable<E>::UnorderedCollectable(std::shared_ptr<Generator<E>> generator, const Module& concurrent)
        : Collectable<E>(generator, concurrent) {}

    template <typename E>
    UnorderedCollectable<E>::UnorderedCollectable(const UnorderedCollectable& other)
        : Collectable<E>(other) {}

    template <typename E>
    UnorderedCollectable<E>::UnorderedCollectable(UnorderedCollectable&& other) noexcept
        : Collectable<E>(std::move(other)) {}

    template <typename E>
    UnorderedCollectable<E>& UnorderedCollectable<E>::operator=(const Collectable<E>& other)
    {
        if (this != &other)
        {
            Collectable<E>::operator=(other);
        }
        return *this;
    }

    template <typename E>
    UnorderedCollectable<E>& UnorderedCollectable<E>::operator=(Collectable<E>&& other) noexcept
    {
        if (this != &other)
        {
            Collectable<E>::operator=(std::move(other));
        }
        return *this;
    }

    template <typename E>
    UnorderedCollectable<E>& UnorderedCollectable<E>::operator=(const UnorderedCollectable<E>& other)
    {
        if (this != &other)
        {
            Collectable<E>::operator=(other);
        }
        return *this;
    }

    template <typename E>
    UnorderedCollectable<E>& UnorderedCollectable<E>::operator=(UnorderedCollectable<E>&& other) noexcept
    {
        if (this != &other)
        {
            Collectable<E>::operator=(std::move(other));
        }
        return *this;
    }

    //Semantic factory function
    Semantic<char> bytes(std::istream& stream)
    {
        return Semantic<char>([&stream](const BiConsumer<char, Timestamp>& accept, const Predicate<char>& predicate) -> void {
            char c;
            Timestamp index = 0;
            while (stream.get(c)) {
                if (predicate(c)) {
                    break;
                }
                accept(c, index++);
            }
            });
    }

    Semantic<std::vector<char>> chunks(std::istream& stream, const Module& size)
    {
        return Semantic<std::vector<char>>([&stream, size](const BiConsumer<std::vector<char>, Timestamp>& accept, const Predicate<std::vector<char>>& predicate) -> void {
            Timestamp index = 0;
            std::vector<char> buffer;
            buffer.reserve(static_cast<size_t>(size));

            char c;
            while (stream.get(c)) {
                buffer.push_back(c);
                if (buffer.size() == static_cast<size_t>(size)) {
                    if (predicate(buffer)) {
                        break;
                    }
                    accept(buffer, index++);
                    buffer.clear();
                }
            }

            if (!buffer.empty()) {
                if (!predicate(buffer)) {
                    accept(buffer, index);
                }
            }
            });
    }

    template<typename E>
    Semantic<E> empty() {
        return Semantic<E>([](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {});
    }

    template<typename E, typename... Args>
    Semantic<E> of(Args&&... args) {
        std::vector<E> elements = { std::forward<Args>(args)... };
        return Semantic<E>([elements = std::move(elements)](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            for (Timestamp index = 0; index < elements.size(); ++index) {
                if (predicate(elements[index])) {
                    break;
                }
                accept(elements[index], index);
            }
            });
    }

    template<typename E>
    Semantic<E> fill(const E& element, const Module& count) {
        return Semantic<E>([element, count](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            for (Timestamp index = 0; index < count; ++index) {
                if (predicate(element)) {
                    break;
                }
                accept(element, index);
            }
            });
    }

    template<typename E>
    Semantic<E> fill(const Supplier<E>& supplier, const Module& count) {
        return Semantic<E>([supplier, count](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            for (Timestamp index = 0; index < count; ++index) {
                E element = supplier();
                if (predicate(element)) {
                    break;
                }
                accept(element, index);
            }
            });
    }

    template<typename E>
    Semantic<E> from(const E* array, const Module& length) {
        return Semantic<E>([array, length](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            for (Timestamp index = 0; index < length; ++index) {
                if (predicate(array[index])) {
                    break;
                }
                accept(array[index], index);
            }
            });
    }

    template<typename E, Module length>
    Semantic<E> from(const std::array<E, length>& array) {
        return Semantic<E>([array](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            for (Timestamp index = 0; index < length; ++index) {
                if (predicate(array[index])) {
                    break;
                }
                accept(array[index], index);
            }
            });
    }

    template<typename E>
    Semantic<E> from(const std::list<E>& list) {
        return Semantic<E>([list](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            Timestamp index = 0;
            for (const auto& element : list) {
                if (predicate(element)) {
                    break;
                }
                accept(element, index++);
            }
            });
    }

    template<typename E>
    Semantic<E> from(const std::vector<E>& vector) {
        return Semantic<E>([vector](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            for (Timestamp index = 0; index < vector.size(); ++index) {
                if (predicate(vector[index])) {
                    break;
                }
                accept(vector[index], index);
            }
            });
    }

    template<typename E>
    Semantic<E> from(const std::initializer_list<E>& initializerList) {
        return Semantic<E>([initializerList](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            Timestamp index = 0;
            for (const auto& element : initializerList) {
                if (predicate(element)) {
                    break;
                }
                accept(element, index++);
            }
            });
    }

    template<typename E>
    Semantic<E> from(const std::set<E>& set) {
        return Semantic<E>([set](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            Timestamp index = 0;
            for (const auto& element : set) {
                if (predicate(element)) {
                    break;
                }
                accept(element, index++);
            }
            });
    }

    template<typename E>
    Semantic<E> from(const std::unordered_set<E>& unorderedSet) {
        return Semantic<E>([unorderedSet](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            Timestamp index = 0;
            for (const auto& element : unorderedSet) {
                if (predicate(element)) {
                    break;
                }
                accept(element, index++);
            }
            });
    }

    template<typename E>
    Semantic<E> iterate(const Generator<E>& generator) {
        return Semantic<E>(generator);
    }

    Semantic<std::vector<char>> lines(std::istream& stream)
    {
        return Semantic<std::vector<char>>([&stream](const BiConsumer<std::vector<char>, Timestamp>& accept, const Predicate<std::vector<char>>& predicate) -> void {
            Timestamp index = 0;
            std::string line;

            while (std::getline(stream, line)) {
                std::vector<char> chars(line.begin(), line.end());
                if (predicate(chars)) {
                    break;
                }
                accept(chars, index++);
            }
            });
    }

    template<typename E>
    Semantic<E> range(const E& start, const E& end) {
        return Semantic<E>([start, end](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            E current = start;
            Timestamp index = 0;
            while (current < end) {
                if (predicate(current)) {
                    break;
                }
                accept(current, index++);
                ++current;
            }
            });
    }

    template<typename E>
    Semantic<E> range(const E& start, const E& end, const E& step) {
        return Semantic<E>([start, end, step](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            E current = start;
            Timestamp index = 0;
            while (current < end) {
                if (predicate(current)) {
                    break;
                }
                accept(current, index++);
                current += step;
            }
            });
    }

    Semantic<std::vector<char>> split(std::istream& stream, const char& delimiter)
    {
        return Semantic<std::vector<char>>([&stream, delimiter](const BiConsumer<std::vector<char>, Timestamp>& accept, const Predicate<std::vector<char>>& predicate) -> void {
            Timestamp index = 0;
            std::vector<char> buffer;
            char c;

            while (stream.get(c)) {
                if (c == delimiter) {
                    if (!predicate(buffer)) {
                        accept(buffer, index++);
                    }
                    buffer.clear();
                }
                else {
                    buffer.push_back(c);
                }
            }

            if (!buffer.empty() && !predicate(buffer)) {
                accept(buffer, index);
            }
            });
    }

    Semantic<std::string> text(std::istream& stream)
    {
        return Semantic<std::string>([&stream](const BiConsumer<std::string, Timestamp>& accept, const Predicate<std::string>& predicate) -> void {
            Timestamp index = 0;
            std::string line;

            while (std::getline(stream, line)) {
                if (predicate(line)) {
                    break;
                }
                accept(line, index++);
            }
            });
    }

    //Semantic
    template <typename E>
    Semantic<E>::Semantic()
        : generator(std::make_shared<Generator<E>>([](const Consumer<E>&, const Predicate<E>&) {})), concurrent(1) {
    }

    template <typename E>
    Semantic<E>::Semantic(const Generator<E>& generator)
        : generator(std::make_shared<Generator<E>>(generator)), concurrent(1) {
    }

    template <typename E>
    Semantic<E>::Semantic(const Generator<E>& generator, const Module& concurrent)
        : generator(std::make_shared<Generator<E>>(generator)), concurrent(concurrent) {
    }

    template <typename E>
    Semantic<E>::Semantic(std::shared_ptr<Generator<E>> generator)
        : generator(generator), concurrent(1) {
    }

    template <typename E>
    Semantic<E>::Semantic(std::shared_ptr<Generator<E>> generator, const Module& concurrent)
        : generator(generator), concurrent(concurrent) {
    }

    template <typename E>
    Semantic<E>::Semantic(const Semantic& other)
        : generator(other.generator), concurrent(other.concurrent) {
    }

    template <typename E>
    Semantic<E>::Semantic(Semantic&& other) noexcept
        : generator(std::move(other.generator)), concurrent(other.concurrent) {
    }

    template <typename E>
    Semantic<E>& Semantic<E>::operator=(const Semantic<E>& other)
    {
        if (this != &other)
        {
            generator = other.generator;
            concurrent = other.concurrent;
        }
        return *this;
    }

    template <typename E>
    Semantic<E>& Semantic<E>::operator=(Semantic<E>&& other) noexcept
    {
        if (this != &other)
        {
            generator = std::move(other.generator);
            concurrent = other.concurrent;
        }
        return *this;
    }

    template<typename E>
    Semantic<E> Semantic<E>::concat(const Semantic<E>& other) const {
        return Semantic<E>([this, other](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            Module count = 0;
            (*this->generator)([&](const E& element, Timestamp index)->void {
                accept(element, index);
                count++;
                }, interrupt);

            (*other.generator)([&](const E& element, Timestamp index)->void {
                accept(element, index + count);
                }, interrupt);
            }, std::max(this->concurrent, other.concurrent));
    }

    template<typename E>
    Semantic<E> Semantic<E>::distinct() const {
        return Semantic<E>([this](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            std::unordered_set<E> seen;
            (*this->generator)([&](const E& element, const Timestamp& index)->void {
                if (seen.insert(element).second) {
                    accept(element, index);
                }
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::distinct(const Comparator<E, E>& identifier) const {
        return Semantic<E>([this, identifier](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            std::set<Timestamp> seen;
            (*this->generator)([&](const E& element, const Timestamp& index)->void {
                Timestamp id = identifier(element, element);
                if (seen.insert(id).second) {
                    accept(element, index);
                }
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::dropWhile(const Predicate<E>& p) const {
        return Semantic<E>([this, p](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            bool dropping = true;
            (*this->generator)([&](const E& element, const Timestamp& index)->void {
                if (dropping) {
                    if (!p(element)) {
                        dropping = false;
                        accept(element, index);
                    }
                }
                else {
                    accept(element, index);
                }
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::filter(const Predicate<E>& p) const {
        return Semantic<E>([this, p](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            (*this->generator)([&](const E& element, Timestamp index)->void {
                if (p(element)) {
                    accept(element, index);
                }
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::flat(const Function<E, Semantic<E>>& mapper) const {
        return Semantic<E>([this, mapper](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            Module current = 0;
            (*this->generator)([&](const E& element, Timestamp index)->void {
                Semantic<E> inner = mapper(element);
                (*inner.generator)([&](const E& innerElement, Timestamp innerIndex)->void {
                    accept(innerElement, current);
                    current++;
                    }, interrupt);
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    template<typename R>
    Semantic<R> Semantic<E>::flatMap(const Function<E, Semantic<R>>& mapper) const {
        return Semantic<R>([this, mapper](const BiConsumer<R, Timestamp>& accept, const Predicate<R>& interrupt)->void {
            Module current = 0;
            (*this->generator)([&](const E& element, Timestamp index)->void {
                Semantic<R> inner = mapper(element);
                (*inner.generator)([&](const R& innerElement, Timestamp innerIndex)->void {
                    accept(innerElement, current);
                    current++;
                    }, interrupt);
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::limit(const Module& n) const {
        return Semantic<E>([this, n](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            Module count = 0;
            (*this->generator)([&](const E& element, Timestamp index)->void {
                accept(element, index);
                count++;
                }, [&](const E& element)->bool {
                    return count >= n || interrupt(element);
                    });
            }, this->concurrent);
    }

    template<typename E>
    template<typename R>
    Semantic<R> Semantic<E>::map(const Function<E, R>& mapper) const {
        return Semantic<R>([this, mapper](const BiConsumer<R, Timestamp>& accept, const Predicate<R>& interrupt)->void {
            (*this->generator)([&](const E& element, Timestamp index)->void {
                accept(mapper(element), index);
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::parallel() const {
        return Semantic<E>(this->generator, std::thread::hardware_concurrency());
    }

    template<typename E>
    Semantic<E> Semantic<E>::parallel(const Module& threadCount) const {
        return Semantic<E>(this->generator, threadCount);
    }

    template<typename E>
    Semantic<E> Semantic<E>::peek(const Consumer<E>& c) const {
        return Semantic<E>([this, c](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            (*this->generator)([&](const E& element, Timestamp index)->void {
                c(element);
                accept(element, index);
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::redirect(const BiFunction<E, Timestamp, Timestamp>& redirector) const {
        return Semantic<E>([this, redirector](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            (*this->generator)([&](const E& element, Timestamp index)->void {
                accept(element, redirector(element, index));
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::reverse() const {
        return Semantic<E>([this](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            (*this->generator)([&](const E& element, const Timestamp& index)->void {
                accept(element, -index);
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::shuffle() const {
        return Semantic<E>([this](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            (*this->generator)([&](const E& element, Timestamp index)->void {
                accept(element, std::hash<E>{}(element) ^ std::hash<Timestamp>{}(index));
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::shuffle(const Function<E, Timestamp>& mapper) const {
        return Semantic<E>([this, &mapper](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            (*this->generator)([&](const E& element, Timestamp index)->void {
                accept(element, mapper(element));
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::skip(const Module& n) const {
        return Semantic<E>([this, n](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            Module skipped = 0;
            (*this->generator)([&](const E& element, Timestamp index)->void {
                if (skipped < n) {
                    skipped++;
                }
                else {
                    accept(element, index - n);
                }
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    OrderedCollectable<E> Semantic<E>::sorted() const {
        return OrderedCollectable<E>([this](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            using Container = std::vector<std::pair<Timestamp, E>>;
            const Function<Container, Container>& arrange = [](const Container& container) -> Container {
                Module size = static_cast<Module>(container.size());
                Container result;
                for (std::pair pair : container) {
                    Timestamp first = pair.first > 0 ? (pair.first % size) : (size - (std::abs(pair.first) % size));
                    Timestamp second = pair.second;
                    result.insert(std::make_pair(first, second));
                }
                return result;
                };
            Container elements;
            (*this->generator)([&](const E& element, Timestamp index)->void {
                if (interrupt(element)) return;
                elements.emplace_back(index, element);
                }, interrupt);
            Container arranged = arrange(elements);
            std::sort(arranged.begin(), arranged.end());

            for (const auto& pair : arranged) {
                if (interrupt(pair.second)) return;
                accept(pair.second, pair.first);
            }
            }, this->concurrent);
    }

    template<typename E>
    OrderedCollectable<E> Semantic<E>::sorted(const Comparator<E, E>& indexer) const {
        return OrderedCollectable<E>([this, &indexer](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            using Container = std::vector<std::pair<Timestamp, E>>;
            const Function<Container, Container>& arrange = [](const Container& container) -> Container {
                Module size = static_cast<Module>(container.size());
                Container result;
                for (std::pair pair : container) {
                    Timestamp first = pair.first > 0 ? (pair.first % size) : (size - (std::abs(pair.first) % size));
                    Timestamp second = pair.second;
                    result.insert(std::make_pair(first, second));
                }
                return result;
                };
            Container elements;
            (*this->generator)([&elements](const E& element, Timestamp index)->void {
                elements.emplace_back(index, element);
                }, interrupt);
            Container arranged = arrange(elements);
            std::sort(arranged.begin(), arranged.end(),
                [&indexer](const std::pair<Timestamp, E>& a, const std::pair<Timestamp, E>& b) {
                    return indexer(a.second, b.second) < 0;
                });

            for (const auto& pair : arranged) {
                if (interrupt(pair.second)) {
                    break;
                }
                accept(pair.second, pair.first);
            }
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::sub(const Module& start, const Module& end) const {
        return Semantic<E>([this, start, end](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            Module index = 0;
            (*this->generator)([&](const E& element, Timestamp)->void {
                if (index >= start) {
                    accept(element, index - start);
                }
                index++;
                }, [&](const E& element)-> bool {
                    return index >= end || interrupt(element);
                    });
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::takeWhile(const Predicate<E>& p) const {
        return Semantic<E>([this, p](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            (*this->generator)([&](const E& element, const Timestamp& index)->void {
                if (p(element)) {
                    accept(element, index);
                }
                }, [&](const E& element)->bool {
                    return !p(element) || interrupt(element);
                    });
            }, this->concurrent);
    }

    template<typename E>
    OrderedCollectable<E> Semantic<E>::toOrdered() const {
        return OrderedCollectable<E>([this](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            (*this->generator)([&](const E& element, Timestamp index)->void {
                accept(element, index);
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Statistics<E, E> Semantic<E>::toStatistics() const {
        return Statistics<E, E>(this->generator, this->concurrent);
    }

    template<typename E>
    template<typename R>
    Statistics<E, R> Semantic<E>::toStatistics(const Function<E, R>& mapper) const {
        return Statistics<E, R>([this, mapper](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& predicate)->void {
            (*this->generator)([&](const E& element, Timestamp index)->void {
                accept(element, index);
                }, predicate);
            }, this->concurrent);
    }

    template<typename E>
    UnorderedCollectable<E> Semantic<E>::toUnordered() const {
        return UnorderedCollectable<E>(this->generator, this->concurrent);
    }

    template<typename E>
    WindowCollectable<E> Semantic<E>::toWindow() const {
        return WindowCollectable<E>(this->generator, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::translate(const Timestamp& offset) const {
        return Semantic<E>([this, &offset](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            (*this->generator)([&](const E& element, Timestamp index)->void {
                accept(element, index + offset);
                }, interrupt);
            }, this->concurrent);
    }

    template<typename E>
    Semantic<E> Semantic<E>::translate(const Function<E, Timestamp>& translator) const {
        return Semantic<E>([this, &translator](const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt)->void {
            (*this->generator)([&](const E& element, Timestamp index)->void {
                accept(element, index + translator(element));
                }, interrupt);
            }, this->concurrent);
    }

};

