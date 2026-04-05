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
#include <forward_list>
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

    template <typename D>
    auto useRange(const D& start, const D& end) -> Semantic<D>;

    template <typename Container>
    auto useFrom(Container container) -> Semantic<typename Container::value_type>;
};
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

    template <typename T>
    using Comparator = std::function<Timestamp(T)>;

    template <typename T>
    using Generator = BiConsumer<BiConsumer<T, Timestamp>, BiPredicate<T, Timestamp>>;

    template <typename D>
    D randomly(const D& start, const D& end) {
        try {
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
        }catch (const std::exception& exception) {
            std::cerr << exception.what() << '\n';
            return 0;
        }
    }

    bool randomly() {
        try {
            static std::random_device device;
            static std::mt19937 generator(device());
            std::bernoulli_distribution distribution(0.5);
            return distribution(generator);
        }catch (const std::exception& exception) {
            std::cerr << exception.what() << '\n';
        }
        return 0;
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
                        try {
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
                        }catch (const std::exception &exception) {
                            throw exception;
                        }
                    }
                    try {
                        task();
                    }catch (const std::exception& exception) {
                        throw exception;
                    }
                }
            });
        }

        void decrease() {
            if (!this->workers.empty()) {
                {
                    try {
                        std::lock_guard<std::mutex> lock(this->mutex);
                        this->tasks.emplace([] {});
                    }catch (const std::exception& exception) {
                        std::cerr << exception.what() << '\n';
                    }
                }
                try {
                    this->condition.notify_one();
                    if (this->workers.back().joinable()) {
                        this->workers.back().join();
                    }
                    this->workers.pop_back();
                }catch (const std::exception& exception) {
                    std::cerr << exception.what() << '\n';
                }
            }
        }

        void boot() {
            if (!this->active.exchange(true)) {
                try {
                    this->condition.notify_all();
                }catch (const std::exception& exception) {
                    std::cerr << exception.what() << '\n';
                }
            }
        }

        void shutdown() {
            if (this->active.exchange(false)) {
                {
                    try {
                        std::unique_lock<std::mutex> lock(this->mutex);
                        this->condition.wait(lock, [this] {
                            return this->tasks.empty();
                        });
                    }catch (const std::exception& exception) {
                        std::cerr << exception.what() << '\n';
                    }
                }
                try {
                    this->condition.notify_all();
                    for (std::thread& worker : this->workers) {
                        if (worker.joinable()) {
                            worker.join();
                        }
                    }
                }catch (const std::exception& exception) {
                    std::cerr << exception.what() << '\n';
                }
            }
        }

        auto submit(const Runnable& task) -> std::future<void> {
            if (!this->active.load()) {
                throw std::runtime_error("ThreadPool is not active.");
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
        auto submit(const Supplier<R>& task) -> std::future<R> {
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

    template<typename A>
    using Identity = functional::Supplier<A>;

    template<typename E, typename A>
    using Interrupt = functional::TriPredicate<E, functional::Timestamp, A>;

    template <typename A, typename E>
    using Accumulator = functional::TriFunction<A, E, functional::Timestamp, A>;

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

        auto collect(const functional::Generator<E>& generator, const functional::Module& concurrent) const -> R {
            if (concurrent < 2) {
                try {
                    A identity = (*(this->identity))();
                    generator([&identity, this](E element, functional::Timestamp index)-> void {
                        identity = (*(this->accumulator))(identity, element, index);
                    }, [&identity, this](E element, functional::Timestamp index)-> bool {
                        return (*(this->interrupt))(element, index, identity);
                    });
                    return (*(this->finisher))(identity);
                }catch (const std::exception& exception) {
                    throw exception;
                }
            }
            std::vector<std::future<A>> futures;
            futures.reserve(concurrent);
            for (functional::Module thread = 0; thread < concurrent; thread++) {
                futures.emplace_back(pool::pool.submit<A>([this, thread, &generator, concurrent]()-> A {
                    try {
                        A identity = (*(this->identity))();
                        generator([thread, &identity, concurrent, this](E element, functional::Timestamp index)-> void {
                            if (index % concurrent == thread) {
                                identity = (*(this->accumulator))(identity, element, index);
                            }
                            }, [&identity, this](E element, functional::Timestamp index)-> bool {
                                return (*(this->interrupt))(element, index, identity);
                                });
                            return identity;
                    }catch (const std::exception& exception) {
                        throw exception;
                    }
                }));
            }
            A identity = (*(this->identity))();
            for (std::future<A>& future : futures) {
                identity = (*(this->combiner))(identity, future.get());
            }
            return (*(this->finisher))(identity);
        }

        auto collect(const std::vector<E>& container, const functional::Module& concurrent) const -> R {
            if (concurrent < 2) {
                try {
                    A identity = (*(this->identity))();
                    functional::Timestamp index = 0;
                    for (const E& element : container) {
                        if ((*(this->interrupt))(element, index, identity)) {
                            break;
                        }
                        identity = (*(this->accumulator))(identity, element, index);
                        index++;
                    }
                    return (*(this->finisher))(identity);
                }catch (const std::exception& exception) {
                    std::cerr << exception.what() << '\n';
                }
            }
            std::vector<std::future<A>> futures;
            futures.reserve(concurrent);
            for (functional::Module thread = 0; thread < concurrent; thread++) {
                futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]()-> A {
                    A identity = (*(this->identity))();
                    functional::Module index = 0;
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

        auto collect(const std::set<E>& container, const functional::Module& concurrent) const -> R {
            if (concurrent < 2) {
                try {
                    A identity = (*(this->identity))();
                    functional::Timestamp index = 0;
                    for (const E& element : container) {
                        if ((*(this->interrupt))(element, index, identity)) {
                            break;
                        }
                        identity = (*(this->accumulator))(identity, element, index);
                        index++;
                    }
                    return (*(this->finisher))(identity);
                }catch (const std::exception& exception) {
                    throw exception;
                }
            }
            std::vector<std::future<A>> futures;
            futures.reserve(concurrent);
            for (functional::Module thread = 0; thread < concurrent; thread++) {
                futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]()-> A {
                    try {
                        A identity = (*(this->identity))();
                        functional::Module index = 0;
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
                    }catch (const std::exception& exception) {
                        throw exception;
                    }
                }));
            }
            A identity = (*(this->identity))();
            for (std::future<A>& future : futures) {
                identity = (*(this->combiner))(identity, future.get());
            }
            return (*(this->finisher))(identity);
        }

        auto collect(const std::unordered_set<E>& container, const functional::Module& concurrent) const -> R{
            if (concurrent < 2) {
                try {
                    A identity = (*(this->identity))();
                    functional::Timestamp index = 0;
                    for (const E& element : container) {
                        if ((*(this->interrupt))(element, index, identity)) {
                            break;
                        }
                        identity = (*(this->accumulator))(identity, element, index);
                        index++;
                    }
                    return (*(this->finisher))(identity);
                }catch (const std::exception& exception) {
                    throw exception;
                }
            }
            std::vector<std::future<A>> futures;
            futures.reserve(concurrent);
            for (functional::Module thread = 0; thread < concurrent; thread++) {
                futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]()-> A {
                    try {
                        A identity = (*(this->identity))();
                        functional::Module index = 0;
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
                    }catch (const std::exception& exception) {
                        throw exception;
                    }
                }));
            }
            A identity = (*(this->identity))();
            for (std::future<A>& future : futures) {
                identity = (*(this->combiner))(identity, future.get());
            }
            return (*(this->finisher))(identity);
        }

        auto collect(const std::list<E>& container, const functional::Module& concurrent) const -> R{
            if (concurrent < 2) {
                try {
                    A identity = (*(this->identity))();
                    functional::Timestamp index = 0;
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
            for (functional::Module thread = 0; thread < concurrent; thread++) {
                futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]()-> A {
                    A identity = (*(this->identity))();
                    functional::Module index = 0;
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

        auto collect(const std::initializer_list<E>& container, const functional::Module& concurrent) const -> R{
            if (concurrent < 2) {
                try {
                    A identity = (*(this->identity))();
                    functional::Timestamp index = 0;
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
            for (functional::Module thread = 0; thread < concurrent; thread++) {
                futures.emplace_back(pool::pool.submit<A>([this, &container, thread, concurrent]()-> A {
                    A identity = (*(this->identity))();
                    functional::Module index = 0;
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
    auto useFull(const Identity<A>& identity, const Accumulator<A, E>& accumulator, const Combiner<A>& combiner, const Finisher<A, R>& finisher) -> Collector<E, A, R> {
        const Interrupt<E, A>& interrupt = [](const E element, const functional::Timestamp& index, const A& accumulator)-> bool {
            return false;
        };
        return  Collector<E, A, R>(identity, interrupt, accumulator, combiner, finisher);
    }

    template<typename E, typename A, typename R>
    auto useShortable(const Identity<A>& identity, const Interrupt<E, A>& interrupt, const Accumulator<A, E>& accumulator, const Combiner<A>& combiner, const Finisher<A, R>& finisher) -> Collector<E, A, R> {
        return  Collector<E, A, R>(identity, interrupt, accumulator, combiner, finisher);
    }

    template<typename E, typename Predicate>
    auto useAllMatch(Predicate &&predicate) -> Collector<E, bool, bool> {
        return useShortable<E, bool, bool>(
            []()->bool {
                return true;
            },
            [predicate](E element, functional::Timestamp index, bool accumulator)-> bool {
                return !accumulator;
            },
            [predicate](bool accumulator, E element, functional::Timestamp index)-> bool {
                if constexpr (std::is_invocable_r_v<bool, Predicate, E, functional::Timestamp>) {
                    return accumulator && std::invoke(predicate, element, index);
                }
                else if (std::is_invocable_r_v<bool, Predicate, E>) {
                    return accumulator && std::invoke(predicate, element);
                }
                return false;
            },
            [](bool a, bool b)-> bool {
                return a && b;
            },
            [](bool accumulator)-> bool {
                return accumulator;
            }
        );
    }

    template<typename E, typename Predicate>
    auto useAnyMatch(Predicate&& predicate) -> Collector<E, bool, bool> {
        return useShortable<E, bool, bool>(
            []()->bool {
                return false;
            },
            [predicate](E element, functional::Timestamp index, bool accumulator)-> bool {
                return accumulator;
            },
            [predicate](bool accumulator, E element, functional::Timestamp index)-> bool {
                if constexpr (std::is_invocable_r_v<bool, Predicate, E, functional::Timestamp>) {
                    return accumulator || std::invoke(predicate, element, index);
                }
                else if (std::is_invocable_r_v<bool, Predicate, E>) {
                    return accumulator || std::invoke(predicate, element);
                }
                return false;
            },
            [](bool a, bool b)-> bool {
                return a || b;
            },
            [](bool accumulator)-> bool {
                return accumulator;
            }
        );
    }

    template<typename E, typename Predicate>
    auto useNoneMatch(Predicate&& predicate) -> Collector<E, bool, bool> {
        return useShortable<E, bool, bool>(
            []()->bool {
                return true;
            },
            [predicate](E element, functional::Timestamp index, bool accumulator)-> bool {
                return !accumulator;
            },
            [predicate](bool accumulator, E element, functional::Timestamp index)-> bool {
                if constexpr (std::is_invocable_r_v<bool, Predicate, E, functional::Timestamp>) {
                    return accumulator && !std::invoke(predicate, element, index);
                }
                else if (std::is_invocable_r_v<bool, Predicate, E>) {
                    return accumulator && !std::invoke(predicate, element);
                }
                return false;
            },
            [](bool a, bool b)-> bool {
                return a && b;
            },
            [](bool accumulator)-> bool {
                return accumulator;
            }
        );
    }

    template<typename E, typename Consumer>
    auto useForEach(Consumer&& consumer) -> Collector<E, functional::Module, functional::Module> {
        return useFull<E, functional::Module, functional::Module>(
            []()-> functional::Module {
                return 0;
            },
            [consumer](const functional::Module& accumulator, const E& element, const functional::Timestamp& index)-> functional::Module {
                if constexpr (std::is_invocable_r_v<void, Consumer, E, functional::Timestamp>) {
                    std::invoke(consumer, element, index);
                    return accumulator + 1;
                }
                else if (std::is_invocable_r_v<void, Consumer, E>) {
                    std::invoke(consumer, element);
                    return accumulator + 1;
                }
                return accumulator;
            },
            [](const functional::Module& a, const functional::Module& b)-> functional::Module {
                return a + b;
            },
            [](const functional::Module& identity)-> functional::Module {
                return identity;
            }
        );
    }

    template<typename E, typename A, typename R>
    auto useCollect(const Identity<R>& identity, const Accumulator<A, E>& accumulator, const  Combiner<A>& combiner, const Finisher<A, R>& finisher) -> Collector<E, A, R> {
        return useFull<E, A, R>(identity, accumulator, combiner, finisher);
    }

    template<typename E, typename A, typename R>
    Collector<E, A, R> useCollect(const Identity<R>& identity, const Interrupt<E, A>& interrupt, const Accumulator<A, E>& accumulator, const  Combiner<A>& combiner, const Finisher<A, R>& finisher) {
        return useShortable<E, A, R>(identity, interrupt, accumulator, combiner, finisher);
    }

    template<typename E>
    auto useCount() -> Collector<E, functional::Module, functional::Module> {
        Collector< E, functional::Module, functional::Module> collector = useFull<E, functional::Module, functional::Module>(
            []()-> functional::Module {
                return 0LL;
            },
            [](functional::Module accumulator, E element, functional::Timestamp index)-> functional::Module {
                return accumulator + 1;
            },
            [](functional::Module a, functional::Module b)-> functional::Module {
                return a + b;
            },
            [](functional::Module accumulator)-> functional::Module {
                return accumulator;
            }
        );
        return collector;
    }

    template<typename E>
    auto useError() -> Collector<E, std::string, std::string> {
        return useFull<E, std::string, std::string>(
            []()->std::string {
                return "";
            },
            [](std::string accumulator, E element, functional::Timestamp index)-> std::string {
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
    auto useError(const std::string& prefix, const std::string& delimiter, const std::string suffix) -> Collector<E, std::string, std::string> {
        return useFull<E, std::string, std::string>(
            []()->std::string {
                return "";
            },
            [delimiter](std::string accumulator, E element, functional::Timestamp index)-> std::string {
                if (accumulator.size() > prefix.size()) {
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

    template<typename E, typename Concatenate>
    auto useError(const std::string& prefix, Concatenate && concatenate, const std::string suffix, const functional::BiFunction<std::string, std::string, std::string>& combiner) -> Collector<E, std::string, std::string> {
        return useFull<E, std::string, std::string>(
            []()->std::string {
                return "";
            },
            [concatenate](std::string accumulator, E element, functional::Timestamp index)-> std::string {
                if constexpr (std::is_invocable_r_v<std::string, Concatenate, std::string, E, functional::Timestamp>) {
                    if (accumulator.size() > 0) {
                        return accumulator + std::invoke(accumulator, concatenate, element, index);
                    }
                    return std::invoke(accumulator, concatenate, element, index);
                }
                else if (std::is_invocable_r_v<std::string, Concatenate, E, functional::Timestamp>) {
                    if (accumulator.size() > 0) {
                        return accumulator + std::invoke(concatenate, element, index);
                    }
                    return std::invoke(concatenate, element, index);
                }
                else if (std::is_invocable_r_v<std::string, Concatenate, E>) {
                    if (accumulator.size() > 0) {
                        return accumulator + std::invoke(concatenate, element);
                    }
                    return std::invoke(concatenate, element);
                }
                return accumulator;
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
    auto useFindAny() -> Collector<E, std::optional<E>, std::optional<E>> {
        return useShortable<E, std::optional<E>, std::optional<E >>(
            []()-> std::optional<E> {
                return std::nullopt;
            },
            [](E element, functional::Timestamp index, std::optional<E> accumulator)-> bool {
                return accumulator.has_value();
            },
            [](std::optional<E> accumulator, E element, functional::Timestamp index)-> std::optional<E> {
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
    auto useFindAt(const functional::Timestamp& index) -> Collector<E, std::vector<E>, std::optional<E>> {
        if (index < 0) {
            return useFull<E, std::vector<E>, std::optional<E>>(
                []()->std::vector<E> {
                    return std::vector<E>();
                },
                [](std::vector<E> accumulator, E element, functional::Timestamp index)-> std::vector<E> {
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
                    functional::Timestamp target = accumulator.size() - (std::abs(index) % accumulator.size());
                    return std::optional<E>(accumulator[target]);
                }
            );
        }
        return useShortable<E, std::vector<E>, std::optional<E>>(
            []()->std::vector<E> {
                return std::vector<E>();
            },
            [](E element, functional::Timestamp index, std::vector<E> accumulator)-> bool {
                return accumulator.size() - 1 == index;
            },
            [](std::vector<E> accumulator, E element, functional::Timestamp index)-> std::vector<E> {
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
                functional::Timestamp target = ((index % accumulator.size()) + accumulator.size()) % accumulator.size();
                return std::optional<E>(accumulator[target]);
            }
        );
    }

    template<typename E>
    auto useFindFirst() -> Collector<E, std::optional<E>, std::optional<E>> {
        return useShortable<E, std::optional<E>, std::optional<E >>(
            []()-> std::optional<E> {
                return std::nullopt;
            },
            [](E element, functional::Timestamp index, std::optional<E> accumulator)-> bool {
                return accumulator.has_value();
            },
            [](std::optional<E> accumulator, E element, functional::Timestamp index)-> std::optional<E> {
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
    auto useFindLast() -> Collector<E, std::vector<E>, std::optional<E>> {
        return useFull<E, std::vector<E>, std::optional<E >>(
            []()-> std::vector<E> {
                return std::vector<E>();
            },
            [](std::vector<E> accumulator, E element, functional::Timestamp index)-> std::vector<E> {
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
    auto useFindMaximum() -> Collector<E, std::optional<E>, std::optional<E>> {
        return useFull<E, std::optional<E>, std::optional<E>>(
            []()-> std::optional<E> {
                return std::nullopt;
            },
            [](std::optional<E> accumulator, E element, functional::Timestamp index)-> std::optional<E> {
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
    auto useFindMaximum(const functional::Comparator<E>& comparator) -> Collector<E, std::optional<E>, std::optional<E>> {
        return useFull<E, std::optional<E>, std::optional<E>>(
            []()-> std::optional<E> {
                return std::nullopt;
            },
            [comparator](std::optional<E> accumulator, E element, functional::Timestamp index)-> std::optional<E> {
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
    auto useFindMinimum() -> Collector<E, std::optional<E>, std::optional<E>> {
        return useFull<E, std::optional<E>, std::optional<E>>(
            []()-> std::optional<E> {
                return std::nullopt;
            },
            [](std::optional<E> accumulator, E element, functional::Timestamp index)-> std::optional<E> {
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
    auto useFindMinimum(const functional::Comparator<E>& comparator) -> Collector<E, std::optional<E>, std::optional<E>> {
        return useFull<E, std::optional<E>, std::optional<E>>(
            []()-> std::optional<E> {
                return std::nullopt;
            },
            [comparator](std::optional<E> accumulator, E element, functional::Timestamp index)-> std::optional<E> {
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
    auto useGroup(const functional::BiFunction<E, functional::Timestamp, K>& keyExtractor) -> Collector<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>> {
        return useFull<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>>(
            []()->std::unordered_map<K, std::vector<E>> {
                return std::unordered_map<K, std::vector<E>>();
            },
            [keyExtractor](std::unordered_map<K, std::vector<E>> accumulator, E element, functional::Timestamp index)-> std::unordered_map<K, std::vector<E>> {
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
    auto useGroupBy(const functional::BiFunction<E, functional::Timestamp, K>& keyExtractor, const functional::BiFunction<E, functional::Timestamp, V>& valueExtractor) -> Collector<E, std::unordered_map<K, std::vector<V>>, std::unordered_map<K, std::vector<V>>> {
        return useFull<E, std::unordered_map<K, E>, std::unordered_map<K, std::vector<E>>>(
            []()->std::unordered_map<K, E> {
                return std::unordered_map<K, E>();
            },
            [keyExtractor, valueExtractor](std::unordered_map<K, std::vector<V>> accumulator, E element, functional::Timestamp index)-> std::unordered_map<K, std::vector<V>> {
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
    auto useJoin() -> Collector<E, std::string, std::string> {
        return useFull<E, std::string, std::string>(
            []()-> std::string {
                return "";
            },
            [](std::string accumulator, E element, functional::Timestamp index)-> std::string {
                if constexpr (std::is_same_v<E, std::string>) {
                    if (accumulator.length() > 1) {
                        return accumulator + "," + element;
                    }
                    return element;
                }
                else {
                    if (accumulator.length() > 1) {
                        return accumulator + "," + std::to_string(element);
                    }
                    return std::to_string(element);
                }
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
    auto useJoin(const std::string& prefix, const std::string& delimiter, const std::string suffix) -> Collector<E, std::string, std::string> {
        return useFull<E, std::string, std::string>(
            []()-> std::string {
                return "";
            },
            [delimiter](std::string accumulator, E element, functional::Timestamp index)-> std::string {
                if constexpr (std::is_same_v<E, std::string>) {
                    if (accumulator.length() > 1) {
                        return accumulator + delimiter + element;
                    }
                    return element;
                }
                else {
                    if (accumulator.length() > 1) {
                        return accumulator + delimiter + std::to_string(element);
                    }
                    return std::to_string(element);
                }
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
    auto useJoin(const std::string& prefix, const Accumulator<std::string, E>& serializer, const std::string suffix, const functional::BiFunction<std::string, std::string, std::string>& combiner) -> Collector<E, std::string, std::string> {
        return useFull<E, std::string, std::string>(
            []()->std::string {
                return "";
            },
            [serializer](std::string accumulator, E element, functional::Timestamp index)-> std::string {
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
    auto useOut() -> Collector<E, std::string, std::string> {
        return useFull<E, std::string, std::string>(
            []()->std::string {
                return "";
            },
            [](std::string accumulator, E element, functional::Timestamp index)-> std::string {
                if constexpr (std::is_same_v<E, std::string>) {
                    if (accumulator.length() > 1) {
                        return accumulator + "," + element;
                    }
                    return element;
                }
                else {
                    if (accumulator.length() > 1) {
                        return accumulator + "," + std::to_string(element);
                    }
                    return std::to_string(element);
                }
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
    auto useOut(const std::string& prefix, const std::string& delimiter, const std::string suffix) -> Collector<E, std::string, std::string> {
        return useFull<E, std::string, std::string>(
            []()->std::string {
                return "";
            },
            [delimiter](std::string accumulator, E element, functional::Timestamp index)-> std::string {
                if constexpr (std::is_same_v<E, std::string>) {
                    if (accumulator.length() > 1) {
                        return accumulator + delimiter + element;
                    }
                    return element;
                }
                else {
                    if (accumulator.length() > 1) {
                        return accumulator + delimiter + std::to_string(element);
                    }
                    return std::to_string(element);
                }
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
    auto useOut(const std::string& prefix, const Accumulator<std::string, E>& serializer, const std::string suffix, const functional::BiFunction<std::string, std::string, std::string>& combiner) -> Collector<E, std::string, std::string> {
        return useFull<E, std::string, std::string>(
            []()->std::string {
                return "";
            },
            [serializer](std::string accumulator, E element, functional::Timestamp index)-> std::string {
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
    auto usePartition(const functional::Module& size) -> Collector<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>> {
        return useFull<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>>(
            [size]()-> std::vector<std::vector<E>> {
                return std::vector<std::vector<E>>();
            },
            [size](std::vector<std::vector<E>> accumulator, E element, functional::Timestamp index)-> std::vector<std::vector<E>> {
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
                    functional::Module can_take = first_b.capacity() - last_a.size();
                    functional::Module take_count = std::min(can_take, first_b.size());
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
    auto usePartitionBy(const functional::BiFunction<E, functional::Timestamp, functional::Timestamp>& keyExtractor) {
        return useFull<E, std::map<functional::Timestamp, std::vector<E>>, std::vector<std::vector<E>>> -> Collector<E, std::map<functional::Timestamp, std::vector<E>>, std::vector<std::vector<E>>>(
            []()-> std::map<functional::Timestamp, std::vector<E>> {
                return std::map<functional::Timestamp, std::vector<E>>();
            },
            [keyExtractor](std::map<functional::Timestamp, std::vector<E>> accumulator, E element, functional::Timestamp index)-> std::map<functional::Timestamp, std::vector<E>> {
                functional::Timestamp key = keyExtractor(element, index);
                accumulator[key].push_back(element);
                return accumulator;
            },
            [](std::map<functional::Timestamp, std::vector<E>> a, std::map<functional::Timestamp, std::vector<E>> b)-> std::map<functional::Timestamp, std::vector<E>> {
                for (auto& [key, vec] : b) {
                    for (E element : vec) {
                        a[key].push_back(std::move(element));
                    }
                }
                return a;
            },
            [](std::map<functional::Timestamp, std::vector<E>> accumulator)-> std::vector<std::vector<E>> {
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
    auto useReduce(const functional::BiFunction<E, E, E>& reducer) {
        return useFull<E, std::optional<E>, std::optional<E>> -> Collector<E, std::optional<E>, std::optional<E>>(
            []()-> std::optional<E> {
                return std::nullopt;
            },
            [reducer](std::optional<E> accumulator, E element, functional::Timestamp index)-> std::optional<E> {
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
    auto useReduce(const E& identity, const functional::BiFunction<E, E, E>& reducer) -> Collector<E, E, E> {
        return useFull<E, E, E>(
            [identity]()-> E {
                return identity;
            },
            [reducer](E accumulator, E element, functional::Timestamp index)-> E {
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
    auto useReduce(const R& identity, const functional::BiFunction<R, E, R>& reducer, const functional::BiFunction<R, R, R>& combiner, const functional::Function<R, R>& finisher) -> Collector<E, R, R> {
        return useFull<E, R, R>(
            [identity]()-> R {
                return identity;
            },
            [reducer](R accumulator, E element, functional::Timestamp index)-> R {
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
    auto useToMap(const functional::BiFunction<E, functional::Timestamp, K>& keyExtractor) -> Collector<E, std::map<K, E>, std::map<K, E>> {
        return useFull<E, std::map<K, E>, std::map<K, E>>(
            []()-> std::map<K, E> {
                return std::map<K, E>();
            },
            [keyExtractor](std::map<K, E> accumulator, E element, functional::Timestamp index)-> std::map<K, E> {
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
    auto useToMap(const functional::BiFunction<E, functional::Timestamp, K>& keyExtractor, const functional::BiFunction<E, functional::Timestamp, V>& valueExtractor) -> Collector<E, std::map<K, V>, std::map<K, V>> {
        return useFull<E, std::map<K, V>, std::map<K, V>>(
            []()-> std::map<K, V> {
                return std::map<K, V>();
            },
            [keyExtractor, valueExtractor](std::map<K, V> accumulator, E element, functional::Timestamp index)-> std::map<K, V> {
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
    auto useToUnorderedMap(const functional::BiFunction<E, functional::Timestamp, K>& keyExtractor, const functional::BiFunction<E, functional::Timestamp, V>& valueExtractor) -> Collector<E, std::unordered_map<K, V>, std::unordered_map<K, V>> {
        return useFull<E, std::unordered_map<K, V>, std::unordered_map<K, V>>(
            []()-> std::unordered_map<K, V> {
                return std::unordered_map<K, V>();
            },
            [keyExtractor, valueExtractor](std::unordered_map<K, V> accumulator, E element, functional::Timestamp index)-> std::unordered_map<K, V> {
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
    auto useToSet() -> Collector<E, std::set<E>, std::set<E>> {
        return useFull<E, std::set<E>, std::set<E>>(
            []()-> std::set<E> {
                return std::set<E>();
            },
            [](std::set<E> accumulator, E element, functional::Timestamp index)-> std::set<E> {
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
    auto useToUnorderedSet() -> Collector<E, std::unordered_set<E>, std::unordered_set<E>> {
        return useFull<E, std::unordered_set<E>, std::unordered_set<E>>(
            []()-> std::unordered_set<E> {
                return std::unordered_set<E>();
            },
            [](std::unordered_set<E> accumulator, E element, functional::Timestamp index)-> std::unordered_set<E> {
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
    auto useToVector() -> Collector<E, std::vector<E>, std::vector<E>> {
        return useFull<E, std::vector<E>, std::vector<E>>(
            []()-> std::vector<E> {
                return std::vector<E>();
            },
            [](std::vector<E> accumulator, E element, functional::Timestamp index)-> std::vector<E> {
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
    auto useToList() -> Collector<E, std::list<E>, std::list<E>> {
        return useFull<E, std::list<E>, std::list<E>>(
            []()-> std::list<E> {
                return std::list<E>();
            },
            [](std::list<E> accumulator, E element, functional::Timestamp index)-> std::list<E> {
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
    auto useSummate(const functional::Function<E, D>& mapper) -> Collector<E, D, D> {
        return useFull<E, D, D>(
            []()-> D {
                return D{};
            },
            [mapper](D accumulator, E element, functional::Timestamp index)-> D {
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
    auto useAverage() -> Collector<E, std::pair<D, functional::Module>, D> {
        return useFull<E, std::pair<D, functional::Module>, D>(
            []()-> std::pair<D, functional::Module> {
                return std::make_pair(D{}, 0);
            },
            [](std::pair<D, functional::Module> accumulator, E element, functional::Timestamp index)-> std::pair<D, functional::Module> {
                D value = static_cast<D>(element);
                return std::make_pair(accumulator.first + value, accumulator.second + 1);
            },
            [](std::pair<D, functional::Module> a, std::pair<D, functional::Module> b)-> std::pair<D, functional::Module> {
                return std::make_pair(a.first + b.first, a.second + b.second);
            },
            [](std::pair<D, functional::Module> accumulator)-> D {
                if (accumulator.second == 0) {
                    return D{};
                }
                return accumulator.first / static_cast<D>(accumulator.second);
            }
        );
    }

    template<typename E, typename D>
    auto useAverage(const functional::Function<E, D>& mapper) -> Collector<E, std::pair<D, functional::Module>, D> {
        return useFull<E, std::pair<D, functional::Module>, D>(
            []()-> std::pair<D, functional::Module> {
                return std::make_pair(D{}, 0);
            },
            [mapper](std::pair<D, functional::Module> accumulator, E element, functional::Timestamp index)-> std::pair<D, functional::Module> {
                D value = mapper(element);
                return std::make_pair(accumulator.first + value, accumulator.second + 1);
            },
            [](std::pair<D, functional::Module> a, std::pair<D, functional::Module> b)-> std::pair<D, functional::Module> {
                return std::make_pair(a.first + b.first, a.second + b.second);
            },
            [](std::pair<D, functional::Module> accumulator)-> D {
                if (accumulator.second == 0) {
                    return D{};
                }
                return accumulator.first / static_cast<D>(accumulator.second);
            }
        );
    }

    template<typename E, typename D>
    auto useRange() -> Collector<E, std::pair<D, D>, D> {
        return useFull<E, std::pair<D, D>, D>(
            []()-> std::pair<D, D> {
                return std::make_pair<D, D>(D{}, D{});
            },
            [](std::pair<D, D> accumulator, E element, functional::Timestamp index)-> std::pair<D, functional::Module> {
                D mapped = static_cast<D>(element);
                if (accumulator.first == D{}) {
                    return std::make_pair<D, D>(mapped, accumulator.second);
                }
                if (accumulator.second == D{}) {
                    return std::make_pair<D, D>(accumulator.first, mapped);
                }
                if (mapped < accumulator.first) {
                    return std::make_pair<D, D>(mapped, accumulator.second);
                }
                if (mapped > accumulator.second) {
                    return std::make_pair<D, D>(accumulator.first, mapped);
                }
            },
            [](std::pair<D, D> a, std::pair<D, D> b)-> std::pair<D, D> {
                return std::make_pair(a.first + b.first, a.second + b.second);
            },
            [](std::pair<D, D> accumulator)-> D {
                if (accumulator.second == 0) {
                    return D{};
                }
                return std::abs(accumulator.first - accumulator.second);
            }
        );
    }

    template<typename E, typename D>
    auto useRange(const functional::Function<E, D>& mapper) -> Collector<E, std::pair<D, functional::Module>, D> {
        return useFull<E, std::pair<D, D>, D>(
            []()-> std::pair<D, D> {
                return std::make_pair<D, D>(D{}, D{});
            },
            [mapper](std::pair<D, D> accumulator, E element, functional::Timestamp index)-> std::pair<D, functional::Module> {
                D mapped = mapper(element);
                if (accumulator.first == D{}) {
                    return std::make_pair<D, D>(mapped, accumulator.second);
                }
                if (accumulator.second == D{}) {
                    return std::make_pair<D, D>(accumulator.first, mapped);
                }
                if (mapped < accumulator.first) {
                    return std::make_pair<D, D>(mapped, accumulator.second);
                }
                if (mapped > accumulator.second) {
                    return std::make_pair<D, D>(accumulator.first, mapped);
                }
            },
            [](std::pair<D, D> a, std::pair<D, D> b)-> std::pair<D, D> {
                return std::make_pair(a.first + b.first, a.second + b.second);
            },
            [](std::pair<D, D> accumulator)-> D {
                if (accumulator.second == 0) {
                    return D{};
                }
                return std::abs(accumulator.first - accumulator.second);
            }
        );
    }

    template<typename E>
    auto useFrequency() -> Collector<E, std::map<E, functional::Module>, std::map<E, functional::Module>> {
        return useFull<E, std::map<E, functional::Module>, std::map<E, functional::Module>>(
            []()-> std::map<E, functional::Module> {
                return std::map<E, functional::Module>();
            },
            [](std::map<E, functional::Module> accumulator, E element, functional::Timestamp index)-> std::map<E, functional::Module> {
                accumulator[element]++;
                return accumulator;
            },
            [](std::map<E, functional::Module> a, std::map<E, functional::Module> b)-> std::map<E, functional::Module> {
                for (const auto& [key, count] : b) {
                    a[key] += count;
                }
                return a;
            },
            [](std::map<E, functional::Module> accumulator)-> std::map<E, functional::Module> {
                return accumulator;
            }
        );
    }

    template<typename E, typename D>
    auto useFrequency(const functional::Function<E, D>& mapper) {
        return useFull<E, std::map<D, functional::Module>, std::map<D, functional::Module>> -> Collector<E, std::map<D, functional::Module>, std::map<D, functional::Module>>(
            []()-> std::map<D, functional::Module> {
                return std::map<D, functional::Module>();
            },
            [mapper](std::map<D, functional::Module> accumulator, E element, functional::Timestamp index)-> std::map<D, functional::Module> {
                D key = mapper(element);
                accumulator[key]++;
                return accumulator;
            },
            [](std::map<D, functional::Module> a, std::map<D, functional::Module> b)-> std::map<D, functional::Module> {
                for (const auto& [key, count] : b) {
                    a[key] += count;
                }
                return a;
            },
            [](std::map<D, functional::Module> accumulator)-> std::map<D, functional::Module> {
                return accumulator;
            }
        );
    }
};

namespace collectable {

    template<typename E>
    class Collectable {
    protected:
        functional::Module concurrent;
    public:
        Collectable(const functional::Module& concurrent) : concurrent(concurrent) {}

        auto anyMatch(const functional::BiPredicate<E, functional::Timestamp>& predicate) const -> bool {
            collector::Collector<E, bool, bool> collector = collector::useAnyMatch<E>(predicate);
            return collector.collect(this->source(), this->concurrent);
        }

        auto allMatch(const functional::BiPredicate<E, functional::Timestamp>& predicate) const -> bool{
            collector::Collector<E, bool, bool> collector = collector::useAllMatch<E>(predicate);
            return collector.collect(this->source(), this->concurrent);
        }

        template<typename A, typename R>
        auto collect(const functional::Supplier<R>& identity, const functional::BiFunction<A, E, A>& accumulator, const functional::BiFunction<A, A, A>& combiner, const functional::Function<A, R>& finisher) const -> R {
            collector::Collector<E, A, R> collector = collector::useCollect<E, A, R>(identity, accumulator, combiner, finisher);
            return collector.collect(this->source(), this->concurrent);
        }

        template<typename A, typename R>
        auto collect(const functional::Supplier<R>& identity, const functional::TriPredicate<E, functional::Timestamp, A>& interrupt, const functional::BiFunction<A, E, A>& accumulator, const functional::BiFunction<A, A, A>& combiner, const functional::Function<A, R>& finisher) const -> R {
            collector::Collector<E, A, R> collector = collector::useCollect<E, A, R>(identity, interrupt, accumulator, combiner, finisher);
            return collector.collect(this->source(), this->concurrent);
        }

        auto count() const -> functional::Module {
            collector::Collector<E, functional::Module, functional::Module> collector = collector::useCount<E>();
            return collector.collect(this->source(), this->concurrent);
        }

        auto error() const -> void {
            collector::Collector<E, std::string, std::string> collector = collector::useError<E>();
            return collector.collect(this->source(), this->concurrent);
        }

        auto error(const std::string& prefix, const std::string& delimiter, const std::string& suffix) const -> void {
            collector::Collector<E, std::string, std::string> collector = collector::useError(prefix, delimiter, suffix);
            return collector.collect(this->source(), this->concurrent);
        }

        auto error(const std::string& prefix, const functional::BiFunction<std::string, E, std::string>& serializer, const std::string& suffix, const functional::BiFunction<std::string, std::string, std::string>& combiner) const -> void {
            collector::Collector<E, std::string, std::string> collector = collector::useError(prefix, serializer, suffix, combiner);
            return collector.collect(this->source(), this->concurrent);
        }

        auto empty() const  -> bool {
            collector::Collector<E, functional::Module, functional::Module> collector = collector::useCount();
            return collector.collect(this->source(), this->concurrent) == 0;
        }

        auto findAny() const -> std::optional<E> {
            collector::Collector<E, std::optional<E>, std::optional<E>> collector = collector::useFindAny<E>();
            return collector.collect(this->source(), this->concurrent);
        }

        auto findAt(const functional::Timestamp& index) const -> std::optional<E> {
            collector::Collector<E, std::optional<E>, std::optional<E>> collector = collector::useFindAt(index);
            return collector.collect(this->source(), this->concurrent);
        }

        auto findFirst() const -> std::optional<E> {
            collector::Collector<E, std::optional<E>, std::optional<E>> collector = collector::useFindFirst<E>();
            return collector.collect(this->source(), this->concurrent);
        }

        auto findLast() const -> std::optional<E> {
            collector::Collector<E, std::vector<E>, std::optional<E>> collector = collector::useFindLast<E>();
            return collector.collect(this->source(), this->concurrent);
        }

        auto findMaximum() const -> std::optional<E> {
            collector::Collector<E, std::optional<E>, std::optional<E>> collector = collector::useFindMaximum<E>();
            return collector.collect(this->source(), this->concurrent);
        }

        auto findMaximum(const functional::Comparator<E> &comparator) const -> std::optional<E> {
            collector::Collector<E, std::optional<E>, std::optional<E>> collector = collector::useFindMaximum<E>(comparator);
            return collector.collect(this->source(), this->concurrent);
        }

        auto findMinimum() const -> std::optional<E> {
            collector::Collector<E, std::optional<E>, std::optional<E>> collector = collector::useFindMinimum();
            return collector.collect(this->source(), this->concurrent);
        }

        auto findMinimum(const functional::Comparator<E> &comparator) const -> std::optional<E> {
            collector::Collector<E, std::optional<E>, std::optional<E>> collector = collector::useFindMinimum(comparator);
            return collector.collect(this->source(), this->concurrent);
        }

        auto forEach(const functional::BiConsumer<E, functional::Timestamp>& action) const -> void {
            collector::Collector<E, functional::Module, functional::Module> collector = collector::useForEach(action);
            return collector.collect(this->source(), this->concurrent);
        }

        template<typename K>
        auto group(const functional::BiFunction<E, functional::Timestamp, K>& keyExtractor) const -> std::unordered_map<K, std::vector<E>> {
            collector::Collector<E, std::unordered_map<K, std::vector<E>>, std::unordered_map<K, std::vector<E>>> collector = collector::useGroup(keyExtractor);
            return collector.collect(this->source(), this->concurrent);
        }

        template<typename K, typename V>
        auto groupBy(const functional::BiFunction<E, functional::Timestamp, K>& keyExtractor, const functional::BiFunction<E, functional::Timestamp, V>& valueExtractor) const -> std::unordered_map<K, std::vector<V>> {
            collector::Collector<E, std::unordered_map<K, std::vector<V>>, std::unordered_map<K, std::vector<V>>> collector = collector::useGroupBy(keyExtractor, valueExtractor);
            return collector.collect(this->source(), this->concurrent);
        }

        auto join() const -> std::string {
            collector::Collector<E, std::string, std::string> collector = collector::useJoin<E>();
            return collector.collect(this->source(), this->concurrent);
        }

        auto join(const std::string& prefix, const std::string& delimiter, const std::string& suffix) const -> std::string {
            collector::Collector<E, std::string, std::string> collector = collector::useJoin(prefix, delimiter, suffix);
            return collector.collect(this->source(), this->concurrent);
        }

        auto join(const std::string& prefix, const functional::BiFunction<std::string, E, std::string>& serializer, const std::string suffix, const functional::BiFunction<std::string, std::string, std::string>& combiner) const -> std::string {
            collector::Collector<E, std::string, std::string> collector = collector::useJoin(prefix, serializer, suffix, combiner);
            return collector.collect(this->source(), this->concurrent);
        }

        auto noneMatch(const functional::BiPredicate<E, functional::Timestamp>& predicate) const -> bool{
            collector::Collector<E, bool, bool> collector = collector::useNoneMatch(predicate);
            return collector.collect(this->source(), this->concurrent);
        }

        auto out() const -> std::string {
            collector::Collector<E, std::string, std::string> collector = collector::useOut<E>();
            return collector.collect(this->source(), this->concurrent);
        }

        auto out(const std::string& prefix, const std::string& delimiter, const std::string& suffix) const -> std::string {
            collector::Collector<E, std::string, std::string> collector = collector::useOut(prefix, delimiter, suffix);
            return collector.collect(this->source(), this->concurrent);
        }

        auto out(const std::string& prefix, const functional::BiFunction<std::string, E, std::string>& serializer, const std::string suffix, const functional::BiFunction<std::string, std::string, std::string>& combiner) const -> std::string {
            collector::Collector<E, std::string, std::string> collector = collector::useOut(prefix, serializer, suffix, combiner);
            return collector.collect(this->source(), this->concurrent);
        }

        auto partition(const functional::Module& size) const -> std::vector<std::vector<E>> {
            collector::Collector<E, std::vector<std::vector<E>>, std::vector<std::vector<E>>> collector = collector::usePartition(size);
            return collector.collect(this->source(), this->concurrent);
        }

        auto partitionBy(const functional::Module& size) const -> std::vector<std::vector<E>> {
            collector::Collector<E, std::map<functional::Timestamp, std::vector<E>>, std::vector<std::vector<E>>> collector = collector::usePartitionBy<E>(size);
            return collector.collect(this->source(), this->concurrent);
        }

        auto reduce(const functional::BiFunction<E, E, E>& accumulator) const -> std::optional<E> {
            collector::Collector<E, std::optional<E>, std::optional<E>> collector = collector::useReduce(accumulator);
            return collector.collect(this->source(), this->concurrent);
        }

        auto reduce(const E& identity, const functional::BiFunction<E, E, E>& accumulator) const -> E {
            collector::Collector < E, E, E> collector = collector::useReduce(identity, accumulator);
            return collector.collect(this->source(), this->concurrent);
        }

        template<typename R>
        auto reduce(const R& identity, const functional::BiFunction<R, E, R>& accumulator, const functional::BiFunction<R, R, R>& combiner) const -> R {
            collector::Collector<E, R, R> collector = collector::useReduce(identity, accumulator, combiner);
            return collector.collect(this->source(), this->concurrent);
        }

        virtual auto source() const -> functional::Generator <E> = 0;

        auto toList() const -> std::list<E> {
            collector::Collector<E, std::list<E>, std::list<E>> collector = collector::useToList();
            return collector.collect(this->source(), this->concurrent);
        }

        template<typename K>
        auto toMap(const functional::BiFunction<E, functional::Timestamp, K>& keyExtractor) const -> std::map<K, E> {
            collector::Collector<E, std::map<K, E>, std::map<K, E>> collector = collector::useToMap(keyExtractor);
            return collector.collect(this->source(), this->concurrent);
        }

        template<typename K, typename V>
        auto toMap(const functional::BiFunction<E, functional::Timestamp, K>& keyExtractor, const functional::BiFunction<E, functional::Timestamp, V>& valueExtractor) const -> std::map<K, V> {
            collector::Collector<E, std::map<K, E>, std::map<K, E>> collector = collector::useToMap(keyExtractor, valueExtractor);
            return collector.collect(this->source(), this->concurrent);
        }

        auto toSet() const -> std::set<E> {
            collector::Collector<E, std::set<E>, std::set<E>> collector = collector::useToSet();
            return collector.collect(this->source(), this->concurrent);
        }

        auto toVector() const -> std::vector<E> {
            collector::Collector<E, std::vector<E>, std::vector<E>> collector = collector::useToVector();
            return collector.collect(this->source(), this->concurrent);
        }
    };

    template<typename E>
    class OrderedCollectable :public Collectable<E> {
    protected:
        std::map<functional::Timestamp, E> buffer;
    public:

        OrderedCollectable(const functional::Generator<E>& generator) : Collectable<E>(1) {
            std::set<std::pair<functional::Timestamp, E>> buffer;
            generator([&buffer](E element, functional::Timestamp index)->void {
                buffer.insert(std::make_pair(index, element));
            }, [](E element, functional::Timestamp index)-> bool {
                return false;
            });
            for (std::pair<functional::Timestamp, E> pair : buffer) {
                functional::Timestamp index = pair.first < 0 ? (buffer.size() - (std::abs(pair.first) % buffer.size())) : (pair.first % buffer.size());
                this->buffer.insert(std::make_pair(index, pair.second));
            }
        }

        OrderedCollectable(const functional::Generator<E>& generator, const functional::Module& concurrent) : Collectable<E>(concurrent) {
            std::set<std::pair<functional::Timestamp, E>> buffer;
                generator([&buffer](E element, functional::Timestamp index)->void {
                buffer.insert(std::make_pair(index, element));
            }, [](E element, functional::Timestamp index)-> bool {
                    return false;
            });
            for (std::pair<functional::Timestamp, E> pair : buffer) {
                functional::Timestamp index = pair.first < 0 ? (buffer.size() - (std::abs(pair.first) % buffer.size())) : (pair.first % buffer.size());
                this->buffer.insert(std::make_pair(index, pair.second));
            }
        }

        OrderedCollectable(const functional::Generator<E>& generator, const functional::Comparator<E>& comparator) : Collectable<E>(1) {
            std::set<std::pair<functional::Timestamp, E>> buffer(comparator);
            generator([&buffer](E element, functional::Timestamp index)->void {
                buffer.insert(std::make_pair(index, element));
            }, [](E element, functional::Timestamp index)-> bool {
                return false;
            });
            for (std::pair<functional::Timestamp, E> pair : buffer) {
                functional::Timestamp index = pair.first < 0 ? (buffer.size() - (std::abs(pair.first) % buffer.size())) : (pair.first % buffer.size());
                this->buffer.insert(std::make_pair(index, pair.second));
            }
        }

        OrderedCollectable(const functional::Generator<E>& generator, const functional::Comparator<E> &comparator, const functional::Module& concurrent) : Collectable<E>(concurrent) {
            std::set<std::pair<functional::Timestamp, E>> buffer(comparator);
                generator([&buffer](E element, functional::Timestamp index)->void {
                buffer.insert(std::make_pair(index, element));
            }, [](E element, functional::Timestamp index)-> bool {
                return false;
            });
            for (std::pair<functional::Timestamp, E> pair : buffer) {
                functional::Timestamp index = pair.first < 0 ? (buffer.size() - (std::abs(pair.first) % buffer.size())) : (pair.first % buffer.size());
                this->buffer.insert(std::make_pair(index, pair.second));
            }
        }

        virtual auto source() const -> functional::Generator<E> override {
            return [buffer = this-> buffer](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt)-> void {
                for (std::pair<functional::Timestamp, E> pair : buffer) {
                    if (interrupt(pair.second, pair.first)) {
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
        Statistics(const functional::Module& concurrent) : OrderedCollectable<E>(concurrent) {}
        Statistics(const functional::Generator<E>& generator, const functional::Module& concurrent) : OrderedCollectable<E>(generator, concurrent) {}
        Statistics(const Statistics<E, D>& other) : OrderedCollectable<E>(other) {}
        Statistics(Statistics<E, D>&& other) noexcept : OrderedCollectable<E>(std::move(other)) {}

        auto operator=(const Statistics<E, D>& other) -> Statistics<E, D>& {
            if (this != &other) {
                OrderedCollectable<E>::operator=(other);
            }
            return *this;
        }
        auto operator=(Statistics<E, D>&& other) noexcept -> Statistics<E, D>& {
            if (this != &other) {
                OrderedCollectable<E>::operator=(std::move(other));
            }
            return *this;
        }

        auto average() const -> D {
            collector::Collector<E, std::pair<D, functional::Module>, D> collector = collector::useAverage<E, D>();
            return collector.collect(this->source(), this->concurrent);
        }

        auto average(const functional::Function<E, D> &mapper) const -> D {
            collector::Collector<E, std::pair<D, functional::Module>, D> collector = collector::useAverage<E, D>(mapper);
            return collector.collect(this->source(), this->concurrent);
        }

        auto range() const -> D {
            collector::Collector<E, std::pair<D, D>, D> collector = collector::useRange<E, D>();
            return collector.collect(this->source(), this->concurrent);
        }

        auto range(const functional::Function<E, D>& mapper) const -> D {
            collector::Collector<E, std::pair<D, D>, D> collector = collector::useRange<E, D>(mapper);
            return collector.collect(this->source(), this->concurrent);
        }
    };

    template<typename E>
    class WindowCollectable : public OrderedCollectable<E> {
    public:
        WindowCollectable(const functional::Module& concurrent) : OrderedCollectable<E>(concurrent) {}
        WindowCollectable(const functional::Generator<E>& generator, const functional::Module& concurrent) : OrderedCollectable<E>(generator, concurrent) {}
        WindowCollectable(const WindowCollectable<E>& other) : OrderedCollectable<E>(other) {}
        WindowCollectable(WindowCollectable<E>&& other) noexcept : OrderedCollectable<E>(std::move(other)) {}

        auto operator=(const WindowCollectable<E>& other) -> WindowCollectable<E>& {
            if (this != &other) {
                OrderedCollectable<E>::operator=(other);
            }
            return *this;
        }
        auto operator=(WindowCollectable<E>&& other) noexcept -> WindowCollectable<E>& {
            if (this != &other) {
                OrderedCollectable<E>::operator=(std::move(other));
            }
            return *this;
        }

        auto slide(const functional::Module& size, const functional::Timestamp& step) -> semantic::Semantic<semantic::Semantic<E>> const {
            return semantic::Semantic<semantic::Semantic<E>>([buffer = this->buffer, size, step](auto accept, auto interrupt) -> void {
                    functional::Module total = static_cast<functional::Timestamp>(buffer.size());
                    functional::Module index = 0LL;
                    bool stop = false;
                    for (functional::Module start = 0; start < total && !stop; start += step) {
                        functional::Module end = std::min(start + size, total);
                        if (start < end) { 
                            std::vector<E> window;
                            for (functional::Module i = start; i < end; i++) {
                                window.push_back(buffer.at(i));
                            }
                            auto semantic = semantic::useFrom(window);
                            if (interrupt(semantic, index)) {
                                break;
                            }
                            accept(semantic, index++);
                        }
                    }
                }, this->concurrent);
        }

        auto tumble(const functional::Module& size) -> semantic::Semantic<semantic::Semantic<E>> const {
            return this->slide(size, size);
        }
    };

    template<typename E>
    class UnorderedCollectable : public Collectable<E> {
    protected:
        std::unordered_map<functional::Timestamp, E> buffer;
    public:
        UnorderedCollectable(const functional::Generator<E>& generator) : Collectable<E>(1) {
            generator([this](E element, functional::Timestamp index)->void {
                this->buffer.insert(std::make_pair(index, element));
            }, [](E element, functional::Timestamp index)-> bool {
                return false;
            });
        }

        UnorderedCollectable(const functional::Generator<E>& generator, const functional::Module& concurrent) : Collectable<E>(concurrent) {
            generator([this](E element, functional::Timestamp index)->void {
                this->buffer.insert(std::make_pair(index, element));
            }, [](E element, functional::Timestamp index)-> bool {
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

        virtual auto source() const -> functional::Generator<E> override {
            return [buffer = this-> buffer](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt)-> void {
                for (std::pair<functional::Timestamp, E> pair : buffer) {
                    if (interrupt(pair.second, pair.first)) {
                        break;
                    }
                    accept(pair.second, pair.first);
                }
            };
        }
    };
};

namespace semantic {

    template<typename E>
    class Semantic {
    private:
        std::unique_ptr<functional::Generator<E>> generator;
        const functional::Module concurrent;
    public:
        using Element = E;
        Semantic(Semantic<E>&& other) noexcept = default;
        Semantic(const functional::Generator<E>& generator) : generator(std::make_unique<functional::Generator<E>>(generator)), concurrent(1) {}
        Semantic(const Semantic& other): generator(std::make_unique<functional::Generator<E>>(*other.generator)),concurrent(other.concurrent) {}
        Semantic(const functional::Generator<E>& generator, const functional::Module& concurrent) : generator(std::make_unique<functional::Generator<E>>(generator)), concurrent(concurrent) {}
        Semantic& operator=(const Semantic& other) {
            if (this != &other) {
                generator = std::make_unique<decltype(*other.generator)>(*other.generator);
                concurrent = other.concurrent;
            }
            return *this;
        }

        auto concatenate(Semantic<E> other) const -> Semantic<E> {
            return Semantic<E>([other](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                functional::Module count = 0LL;
                generator([&accept, &count](E element, functional::Timestamp index) -> void {
                    accept(element, index);
                    count++;
                   }, [&count, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count);
                });
                other.source()([&accept, &count](E element, functional::Timestamp index) -> void {
                    accept(element, index);
                    count++;
                }, [&count, &interrupt](E element, functional::Timestamp index) -> bool {
                    return interrupt(element, count);
                });
                
            }, this->concurrent);
        }

        auto concatenate(std::vector<E> vector) const -> Semantic<E> {
            return Semantic<E>([vector = std::move(vector), generator = *(this-> generator)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                functional::Module count = 0LL;
                generator([&accept, &count](E element, functional::Timestamp index) -> void {
                    accept(element, index);
                    count++;
                    }, [&count, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count);
                        });
                    for (E element : vector) {
                        if (interrupt(element, count)) {
                            break;
                        }
                        accept(element, count);
                        count++;
                    }
                }, this->concurrent);
        }

        auto concatenate(std::list<E> list) const -> Semantic<E> {
            return Semantic<E>([list = std::move(list), generator = *(this-> generator)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                functional::Module count = 0LL;
                generator([&accept, &count](E element, functional::Timestamp index) -> void {
                    accept(element, index);
                    count++;
                    }, [&count, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count);
                        });
                    for (E element : list) {
                        if (interrupt(element, count)) {
                            break;
                        }
                        accept(element, count);
                        count++;
                    }
                }, this->concurrent);
        }

        auto concatenate(std::set<E> set) const -> Semantic<E> {
            return Semantic<E>([set = std::move(set), generator = *(this-> generator)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                functional::Module count = 0LL;
                generator([&accept, &count](E element, functional::Timestamp index) -> void {
                    accept(element, index);
                    count++;
                    }, [&count, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count);
                        });
                    for (E element : set) {
                        if (interrupt(element, count)) {
                            break;
                        }
                        accept(element, count);
                        count++;
                    }
                }, this->concurrent);
        }

        auto concatenate(std::unordered_set<E> unordered_set) const -> Semantic<E> {
            return Semantic<E>([unordered_set = std::move(unordered_set), generator = *(this-> generator)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                functional::Module count = 0LL;
                generator([&accept, &count](E element, functional::Timestamp index) -> void {
                    accept(element, index);
                    count++;
                    }, [&count, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count);
                        });
                    for (E element : unordered_set) {
                        if (interrupt(element, count)) {
                            break;
                        }
                        accept(element, count);
                        count++;
                    }
                }, this->concurrent);
        }

        auto concatenate(std::initializer_list<E> initializer_list) const -> Semantic<E> {
            return Semantic<E>([initializer_list = std::move(initializer_list), generator = *(this-> generator)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                functional::Module count = 0LL;
                generator([&accept, &count](E element, functional::Timestamp index) -> void {
                    accept(element, index);
                    count++;
                    }, [&count, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count);
                        });
                    for (E element : initializer_list) {
                        if (interrupt(element, count)) {
                            break;
                        }
                        accept(element, count);
                        count++;
                    }
                }, this->concurrent);
        }

        auto concatenate(std::forward_list<E> forward_list) const -> Semantic<E> {
            return Semantic<E>([forward_list = std::move(forward_list), generator = *(this-> generator)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                functional::Module count = 0LL;
                generator([&accept, &count](E element, functional::Timestamp index) -> void {
                    accept(element, index);
                    count++;
                    }, [&count, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count);
                        });
                    for (E element : forward_list) {
                        if (interrupt(element, count)) {
                            break;
                        }
                        accept(element, count);
                        count++;
                    }
                }, this->concurrent);
        }

        auto concatenate(std::deque<E> deque) const -> Semantic<E> {
            return Semantic<E>([deque = std::move(deque), generator = *(this-> generator)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                functional::Module count = 0LL;
                generator([&accept, &count](E element, functional::Timestamp index) -> void {
                    accept(element, index);
                    count++;
                    }, [&count, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count);
                        });
                    for (E element : deque) {
                        if (interrupt(element, count)) {
                            break;
                        }
                        accept(element, count);
                        count++;
                    }
                }, this->concurrent);
        }

        template<functional::Module Length>
        auto concatenate(std::array<E, Length> array) const -> Semantic<E> {
            return Semantic<E>([array = std::move(array), generator = *(this-> generator)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                functional::Module count = 0LL;
                generator([&accept, &count](E element, functional::Timestamp index) -> void {
                    accept(element, index);
                    count++;
                    }, [&count, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count);
                        });
                    for (E element : array) {
                        if (interrupt(element, count)) {
                            break;
                        }
                        accept(element, count);
                        count++;
                    }
                }, this->concurrent);
        }

        auto distinct() const -> Semantic<E> {
            return Semantic<E>([generator = *(this-> generator)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt)-> void {
                std::unordered_set<E> seen = {};
                generator([&accept, &seen](E element, functional::Timestamp index) -> void {
                    if (seen.find(element) == seen.end()) {
                        accept(element, seen.size());
                        seen.insert(element);
                    }
                    }, [&seen, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, seen.size());
                        });
                }, this->concurrent);
        }

        auto distinct(const functional::Comparator<E>& comparator) const -> Semantic<E> {
            return Semantic<E>([generator = *(this-> generator), comparator](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt)-> void {
                std::set<E, functional::Comparator<E>> seen(comparator);
                generator([&accept, &seen](E element, functional::Timestamp index) -> void {
                    if (seen.find(element) == seen.end()) {
                        accept(element, seen.size());
                        seen.insert(element);
                    }
                    }, [&seen, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, seen.size());
                        });
                }, this->concurrent);
        }

        template<typename Predicate>
        auto dropWhile(Predicate&& predicate) const -> Semantic<E> {
            return Semantic<E>([generator = *(this-> generator), predicate = std::forward<Predicate>(predicate)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt)-> void {
                functional::Timestamp count = -1LL;
                generator([&accept, &count, predicate](E element, functional::Timestamp index) -> void {
                    if (count == -1LL) {
                        if constexpr (std::is_invocable_r_v<bool, Predicate, E, functional::Timestamp>) {
                            if (!std::invoke(predicate, element, index)) {
                                count++;
                                accept(element, count);
                            }
                        }
                        else if constexpr (std::is_invocable_r_v<bool, Predicate, E>) {
                            if (!std::invoke(predicate, element)) {
                                count++;
                            }
                        }
                        else {
                            static_assert(std::is_invocable_r_v<bool, Predicate, E, functional::Timestamp> || std::is_invocable_r_v<bool, Predicate, E>, "Predicate must be callable as either (E, functional::Timestamp) -> bool or (E) -> bool");
                        }
                    }
                    else {
                        accept(element, count);
                        count++;
                    }
                    }, [&interrupt, count](E element, functional::Timestamp index) -> bool {
                        if (count == -1) {
                            return false;
                        }
                        return interrupt(element, count);
                        });
                }, this->concurrent);
        }

        template<typename Predicate>
        auto filter(Predicate&& predicate) const -> Semantic<E> {
            return Semantic<E>([generator = *(this-> generator), predicate = std::forward<Predicate>(predicate)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt)-> void {
                functional::Timestamp count = 0LL;
                generator([&accept, &count, predicate](E element, functional::Timestamp index) -> void {
                    if constexpr (std::is_invocable_r_v<bool, Predicate, E, functional::Timestamp>) {
                        if (std::invoke(predicate, element, index)) {
                            accept(element, count);
                            count++;
                        }
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, E>) {
                        if (std::invoke(predicate, element)) {
                            accept(element, count);
                            count++;
                        }
                    }
                    else {
                        static_assert(std::is_invocable_r_v<bool, Predicate, E, functional::Timestamp> || std::is_invocable_r_v<bool, Predicate, E>, "Predicate must be callable as either (E, functional::Timestamp) -> bool or (E) -> bool");
                    }
                }, [&interrupt, count](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count);
                });
            }, this->concurrent);
        }

        template<typename Flatten>
        auto flat(Flatten&& flatten) const -> Semantic<E> {
            return Semantic<E>([generator = *(this-> generator), flatten = std::forward<Flatten>(flatten)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt)-> void {
                functional::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &interrupt, &stop, &count, flatten](E element, functional::Timestamp index) -> void {
                    if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, E, functional::Timestamp>) {
                        Semantic<E> inner = std::invoke(flatten, element, index);
                        inner.source()([&accept, &count](E element, functional::Timestamp index) -> void {
                            accept(element, count);
                            count++;
                            }, [&interrupt, &stop, &count](E element, functional::Timestamp index) -> bool {
                                stop = stop || interrupt(element, count);
                                return stop;
                                });
                    }
                    else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, E>) {
                        Semantic<E> inner = std::invoke(flatten, element);
                        inner.source()([&accept, &count](E element, functional::Timestamp index) -> void {
                            accept(element, count);
                            count++;
                            }, [&interrupt, &stop, &count](E innerElement, functional::Timestamp index) -> bool {
                                stop = stop || interrupt(element, count);
                                return stop;
                                });
                    }
                    else {
                        static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, E, functional::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, E>, "Flatten must be callable as either (E, functional::Timestamp) -> Semantic<E> or (E) -> Semantic<E>");
                    }
                    }, [&interrupt, &stop, &count](E element, functional::Timestamp index) -> bool {
                        return stop;
                        });
                }, this->concurrent);
        }

        template<typename R, typename Flatten>
        auto flatMap(Flatten&& flatten) const -> Semantic<R> {
            return Semantic<R>([generator = *(this-> generator), flatten = std::forward<Flatten>(flatten)](functional::BiConsumer<R, functional::Timestamp> accept, functional::BiPredicate<R, functional::Timestamp> interrupt)-> void {
                functional::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &interrupt, &stop, &count, flatten](E element, functional::Timestamp index) -> void {
                    if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, E, functional::Timestamp>) {
                        Semantic<R> inner = std::invoke(flatten, element, index);
                        inner.source()([&accept, &count](R innerElement, functional::Timestamp innerIndex) -> void {
                            accept(innerElement, count);
                            count++;
                            }, [&interrupt, &stop, &count](R innerElement, functional::Timestamp innerIndex) -> bool {
                                stop = stop || interrupt(innerElement, count);
                                return stop;
                                });
                    }
                    else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, E>) {
                        Semantic<R> inner = std::invoke(flatten, element);
                        inner.source()([&accept, &count](R innerElement, functional::Timestamp innerIndex) -> void {
                            accept(innerElement, count);
                            count++;
                            }, [&interrupt, &stop, &count](R innerElement, functional::Timestamp innerIndex) -> bool {
                                stop = stop || interrupt(innerElement, count);
                                return stop;
                                });
                    }
                    else {
                        static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, E, functional::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, E>, "Flatten must be callable as either (E, functional::Timestamp) -> Semantic<R> or (E) -> Semantic<R>");
                    }
                    }, [&interrupt, &stop, &count](E element, functional::Timestamp index) -> bool {
                        return stop;
                        });
                }, this->concurrent);
        }

        auto limit(const functional::Module& n) const -> Semantic<E> {
            return Semantic<E>([generator = *(this-> generator), n](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt)-> void {
                functional::Module count = 0;
                generator([&accept, &count, n](E element, functional::Timestamp index) -> void {
                    if (count < n) {
                        accept(element, count);
                        count++;
                    }
                    }, [&count, n, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count) || count >= n;
                        });
                }, this->concurrent);
        }

        template<typename Mapper>
        auto map(Mapper&& mapper) const -> Semantic<decltype(std::declval<Mapper>()(std::declval<E>()))> {
            using R = decltype(std::declval<Mapper>()(std::declval<E>()));
            return Semantic<R>([generator = *(this->generator), mapper = std::forward<Mapper>(mapper)](functional::BiConsumer<R, functional::Timestamp> accept, functional::BiPredicate<R, functional::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &interrupt, &stop, &mapper](E element, functional::Timestamp index) -> void {
                    if constexpr (std::is_invocable_r_v<R, Mapper, E, functional::Timestamp>) {
                        R mapped = std::invoke(mapper, element, index);
                        accept(mapped, index);
                        stop = stop || interrupt(mapped, index);
                    }
                    else if constexpr (std::is_invocable_r_v<R, Mapper, E>) {
                        R mapped = std::invoke(mapper, element);
                        accept(mapped, index);
                        stop = stop || interrupt(mapped, index);
                    }
                    else {
                        static_assert(std::is_invocable_r_v<R, Mapper, E, functional::Timestamp> || std::is_invocable_r_v<R, Mapper, E>, "Mapper must be callable as either (E, functional::Timestamp) -> R or (E) -> R");
                    }
                    }, [&stop, &interrupt](E element, functional::Timestamp index) -> bool {
                        return stop;
                        });
                }, this->concurrent);
        }

        auto parallel() const -> Semantic<E> {
            return Semantic<E>(this->source(), this->concurrent + 1);
        }

        auto parallel(const functional::Module& concurrent) const -> Semantic<E> {
            return Semantic<E>(this->source(), std::max(concurrent, 1ULL));
        }

        template<typename Consumer>
        auto peek(Consumer&& consumer) const -> Semantic<E> {
            return Semantic<E>([generator = *(this->generator), consumer = std::forward<Consumer>(consumer)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &interrupt, &stop, &consumer](E element, functional::Timestamp index) -> void {
                    if constexpr (std::is_invocable_r_v<void, Consumer, E, functional::Timestamp>) {
                        std::invoke(consumer, element, index);
                        accept(element, index);
                    }
                    else if constexpr (std::is_invocable_r_v<void, Consumer, E>) {
                        std::invoke(consumer, element);
                        accept(element, index);
                    }
                    else {
                        static_assert(std::is_invocable_r_v<void, Consumer, E, functional::Timestamp> || std::is_invocable_r_v<void, Consumer, E>, "Consumer must be callable as either (E, functional::Timestamp) -> void or (E) -> void");
                    }
                    }, [&stop, &interrupt](E element, functional::Timestamp index) -> bool {
                        return stop;
                        });
                }, this->concurrent);
        }

        auto redirect(const functional::BiFunction<E, functional::Timestamp, E>& redirector) const -> Semantic<E> {
            return Semantic<E>([generator = *(this->generator), redirector](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &interrupt, &stop, &redirector](E element, functional::Timestamp index) -> void {
                    functional::Timestamp redirected = redirector(element, index);
                    stop = stop || interrupt(element, redirected);
                    accept(element, redirected);
                    }, [&stop](E element, functional::Timestamp index) -> bool {
                        return stop;
                        });
                }, this->concurrent);
        }

        auto reverse() const -> Semantic<E> {
            return Semantic<E>([generator = *(this->generator)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &interrupt, &stop](E element, functional::Timestamp index) -> void {
                    functional::Timestamp redirected = -index;
                    stop = stop || interrupt(element, redirected);
                    accept(element, redirected);
                    }, [&stop](E element, functional::Timestamp index) -> bool {
                        return stop;
                        });
                }, this->concurrent);
        }

        auto skip(const functional::Module& n) const -> Semantic<E> {
            return Semantic<E>([generator = *(this-> generator), n](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt)-> void {
                functional::Module count = 0;
                generator([&accept, &count, n](E element, functional::Timestamp index) -> void {
                    if (count >= n) {
                        accept(element, count);
                    }
                    count++;
                    }, [&count, n, &interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count);
                        });
                }, this->concurrent);
        }

        auto source() const -> functional::Generator<E> {
            return [generator = *(this-> generator)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                generator([&accept](E element, functional::Timestamp index) -> void {
                    accept(element, index);
                    }, [&interrupt](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, index);
                        });
                };
        }

        auto sub(const functional::Module& start, const functional::Module& end) const -> Semantic<E> {
            return Semantic<E>([generator = *(this-> generator), start, end](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt)-> void {
                functional::Module count = 0;
                generator([&accept, &count, start, end](E element, functional::Timestamp index) -> void {
                    if (count >= start && count < end) {
                        accept(element, count);
                    }
                    count++;
                    }, [&count, &interrupt, end](E element, functional::Timestamp index) -> bool {
                        return interrupt(element, count) || count >= end;
                        });
                }, this->concurrent);
        }

        template<typename Predicate>
        auto takeWhile(Predicate&& predicate) const -> Semantic<E> {
            return Semantic<E>([generator = *(this-> generator), predicate = std::forward<Predicate>(predicate)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt)-> void {
                bool stop = false;
                generator([&accept, &stop](E element, functional::Timestamp index) -> void {
                    if constexpr (std::is_invocable_r_v<bool, Predicate, E, functional::Timestamp>) {
                        if (std::invoke(predicate, element, index) && (!stop)) {
                            accept(element, index);
                        }
                        else {
                            stop = true;
                        }
                    }
                    else if constexpr (std::is_invocable_r_v<bool, Predicate, E>) {
                        if (std::invoke(predicate, element) && (!stop)) {
                            accept(element, index);
                        }
                        else {
                            stop = true;
                        }
                    }
                    else {
                        static_assert(std::is_invocable_r_v<bool, Predicate, E, functional::Timestamp> || std::is_invocable_r_v<bool, Predicate, E>, "Predicate must be callable as either (E, functional::Timestamp) -> bool or (E) -> bool");
                    }
                    }, [&interrupt, &stop](E element, functional::Timestamp index) -> bool {
                        return stop || interrupt(element, index);
                        });
                }, this->concurrent);
        }

        auto toUnordered() const -> collectable::UnorderedCollectable<E> {
            return collectable::UnorderedCollectable<E>(this->source(), this->concurrent);
        }

        auto toOrdered() const -> collectable::OrderedCollectable<E> {
            return collectable::OrderedCollectable<E>(this->source(), this->concurrent);
        }

        auto toWindow() const -> collectable::WindowCollectable<E> {
            return collectable::WindowCollectable<E>(this->source(), this->concurrent);
        }

        template<typename D>
        auto toStatistics() const -> collectable::Statistics<E, D> {
            return collectable::Statistics<E, D>(this->source(), this->concurrent);
        }

        auto translate(const functional::Timestamp& offset) const -> Semantic<E> {
            return Semantic<E>([generator = *(this->generator), offset](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                bool stop = false;
                generator([&accept, &interrupt, &stop, offset](E element, functional::Timestamp index) -> void {
                    functional::Timestamp redirected = index + offset;
                    stop = stop || interrupt(element, redirected);
                    accept(element, redirected);
                    }, [&stop](E element, functional::Timestamp index) -> bool {
                        return stop;
                        });
                }, this->concurrent);
        }
    };

    template<typename E>
    class Semantic<Semantic<E>> {
    private:
        std::unique_ptr<functional::Generator<Semantic<E>>> generator;
        const functional::Module concurrent;
    public:
        using Element = E;
        Semantic(const functional::Generator<Semantic<E>>& generator) : generator(std::make_unique<functional::Generator<Semantic<E>>>(generator)), concurrent(1) {}
        Semantic(const functional::Generator<Semantic<E>>& generator, const functional::Module& concurrent) : generator(std::make_unique<functional::Generator<Semantic<E>>>(generator)), concurrent(concurrent) {}
        Semantic(const Semantic<Semantic<E>>& other) = default;
        Semantic(Semantic<Semantic<E>>&& other) noexcept = default;

        auto flat() const -> Semantic<E> {
            return Semantic<E>([generator = this-> source()](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                functional::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &interrupt, &stop, &count](Semantic<E> semantic, functional::Timestamp index) -> void {
                    semantic.source()([&accept, &count](E element, functional::Timestamp index) -> void {
                        accept(element, count);
                        count++;
                    }, [&interrupt, &stop, &count](E element, functional::Timestamp index) -> bool {
                        if (!stop && interrupt(element, count)) {
                            stop = true;
                        }
                        return stop;
                    });
                }, [&interrupt, &stop, &count](Semantic<E> element, functional::Timestamp index) -> bool {
                    return stop;
                });
            }, this->concurrent);
        }

        template <typename Flatten>
        auto flat(Flatten &&flatten) const -> Semantic<E> {
            return Semantic<E>([generator = this-> source(), flatten = std::forward<Flatten>(flatten)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
                functional::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &interrupt, &stop, &count, flatten](Semantic<E> semantic, functional::Timestamp index) -> void {
                    if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, Semantic<E>, functional::Timestamp>) {
                        Semantic<E> inner = std::invoke(flatten, semantic, index);
                        inner.source()([&accept, &count](E element, functional::Timestamp index) -> void {
                            accept(element, count);
                            count++;
                        }, [&interrupt, &stop, &count](E element, functional::Timestamp index) -> bool {
                            stop = stop || interrupt(element, count);
                            return stop;
                        });
                    }
                    else if constexpr (std::is_invocable_r_v<Semantic<E>, Flatten, Semantic<E>>) {
                        Semantic<E> inner = std::invoke(flatten, semantic);
                        inner.source()([&accept, &count, flatten](E element, functional::Timestamp index) -> void {
                            accept(element, count);
                            count++;
                        }, [&interrupt, &stop, &count](E element, functional::Timestamp index) -> bool {
                            stop = stop || interrupt(element, count);
                            return stop;
                        });
                    }
                    else {
                        static_assert(std::is_invocable_r_v<Semantic<E>, Flatten, Semantic<E>, functional::Timestamp> || std::is_invocable_r_v<Semantic<E>, Flatten, Semantic<E>>, "Flatten must be callable as either (E, functional::Timestamp) -> Semantic<E> or (E) -> Semantic<E>");
                    }
                }, [&stop](Semantic<E> semantic, functional::Timestamp index)-> bool {
                    return stop;
                });
            }, this->concurrent);
        }

        template <typename R, typename Flatten>
        auto flatMap(Flatten&& flatten) const -> Semantic<R> {
            return Semantic<R>([generator = this-> source(), flatten = std::forward<Flatten>(flatten)](functional::BiConsumer<R, functional::Timestamp> accept, functional::BiPredicate<R, functional::Timestamp> interrupt) -> void {
                functional::Timestamp count = 0LL;
                bool stop = false;
                generator([&accept, &interrupt, &stop, &count, flatten](Semantic<E> semantic, functional::Timestamp index) -> void {
                    if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, Semantic<E>, functional::Timestamp>) {
                        Semantic<R> inner = std::invoke(flatten, semantic, index);
                        inner.source()([&accept, &count](R element, functional::Timestamp index) -> void {
                            accept(element, count);
                            count++;
                        }, [&interrupt, &stop, &count](R element, functional::Timestamp index) -> bool {
                            stop = stop || interrupt(element, count);
                            return stop;
                        });
                    }
                    else if constexpr (std::is_invocable_r_v<Semantic<R>, Flatten, Semantic<E>>) {
                        Semantic<R> inner = std::invoke(flatten, semantic);
                        inner.source()([&accept, &count, flatten](R element, functional::Timestamp index) -> void {
                            accept(element, count);
                            count++;
                        }, [&interrupt, &stop, &count](R element, functional::Timestamp index) -> bool {
                            stop = stop || interrupt(element, count);
                            return stop;
                        });
                    }
                    else {
                        static_assert(std::is_invocable_r_v<Semantic<R>, Flatten, Semantic<E>, functional::Timestamp> || std::is_invocable_r_v<Semantic<R>, Flatten, Semantic<E>>, "Flatten must be callable as either (E, functional::Timestamp) -> Semantic<E> or (E) -> Semantic<E>");
                    }
                }, [&stop](Semantic<E> semantic, functional::Timestamp index)-> bool {
                    return stop;
                });
            }, this->concurrent);
        }

        auto source() const -> functional::Generator<Semantic<E>> {
            return [generator = *(this-> generator)](functional::BiConsumer<Semantic<E>, functional::Timestamp> accept, functional::BiPredicate<Semantic<E>, functional::Timestamp> interrupt) -> void {
                generator([&accept](Semantic<E> element, functional::Timestamp index) -> void {
                    accept(element, index);
                }, [&interrupt](Semantic<E> element, functional::Timestamp index) -> bool {
                        return interrupt(element, index);
                });
            };
        }
    };

    template <typename D>
    auto useRange(const D& start, const D& end) -> Semantic<D> {
        return Semantic<D>([start = std::min(start, end), end = std::max(start, end)](functional::BiConsumer<D, functional::Timestamp> accept, functional::BiPredicate<D, functional::Timestamp> interrupt)->void {
            for (D index = start; index < end; index++) {
                if (interrupt(index, index)) {
                    break;
                }
                accept(index, index);
            }
        });
    }

    template <typename Container>
    auto useFrom(Container container) -> Semantic<typename Container::value_type> {
        using E = typename Container::value_type;
        return Semantic<E>([elements = std::forward<Container>(container)](functional::BiConsumer<E, functional::Timestamp> accept, functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {
            functional::Timestamp index = 0LL;
            for (const E& element : elements) {
                if (interrupt(element, index)) {
                    break;
                }
                accept(element, index);
                index++;
            }
        }, 1LL);
    }

    template<typename E>
    auto useOf(std::initializer_list<E> elements) -> Semantic<E> {
        return Semantic<E>([elements = std::move(elements)](functional::BiConsumer<E, functional::Timestamp> accept,functional::BiPredicate<E, functional::Timestamp> interrupt) -> void {functional::Timestamp index = 0LL;
            for (const E& element : elements) {
                if (interrupt(element, index)) {
                    break;
                }
                accept(element, index);
                index++;
            }
        }, 1LL);
    }

    auto useBlob(const std::string& text) -> Semantic<char> {
        return Semantic<char>([text](functional::BiConsumer<char, functional::Timestamp> accept, functional::BiPredicate<char, functional::Timestamp> interrupt) -> void {
            functional::Timestamp index = 0LL;
            for (const auto& byte : text) {
                if (interrupt(byte, index)) {
                    break;
                }
                accept(byte, index);
                index++;
            }
        }, 1LL);
    }

    auto useBlob(const std::string& text, functional::Module start, const functional::Module end) -> Semantic<char> {
        return Semantic<char>([text, start, end](functional::BiConsumer<char, functional::Timestamp> accept,functional::BiPredicate<char, functional::Timestamp> interrupt) -> void {
                functional::Module limitedStart = std::max(start, static_cast<functional::Module>(0LL));
                functional::Module limitedEnd = std::min(end, static_cast<functional::Module>(text.size()));
                if (start < end) {
                    functional::Timestamp index = 0LL;
                    for (const auto& byte : text) {
                        if (interrupt(byte, index)) {
                            break;
                        }
                        accept(byte, index);
                        index++;
                    }
                }
        }, 1LL);
    }

    auto useBlob(std::istream& stream) -> Semantic<std::string> {
        return Semantic<std::string>([&stream](functional::BiConsumer<std::string, functional::Timestamp> accept, functional::BiPredicate<std::string, functional::Timestamp> interrupt) -> void {
            std::string line;
            functional::Timestamp index = 0LL;
            while (std::getline(stream, line)) {
                if (interrupt(line, index)) {
                    break;
                }
                accept(line, index);
                index++;
            }
        }, 1LL);
    }

    auto useText(const std::string& text) -> Semantic<std::string> {
        return Semantic<std::string>([text](functional::BiConsumer<std::string, functional::Timestamp> accept, functional::BiPredicate<std::string, functional::Timestamp> interrupt) -> void {
            if (!interrupt(text, 0LL)) {
                accept(text, 0LL);
            }
        }, 1LL);
    }

    auto useText(const std::string& text, const char& delimiter) -> Semantic<std::string> {
        return Semantic<std::string>([text, delimiter](functional::BiConsumer<std::string, functional::Timestamp> accept, functional::BiPredicate<std::string, functional::Timestamp> interrupt) -> void {
            std::vector<std::string> pool;
            for (functional::Module a = 0; a < text.size(); a++) {
                std::string target = "";
                for (functional::Module b = a; b < text.size(); b++) {
                    if (text[b] == delimiter) {
                        a = b;
                        break;
                    }
                    target += text[b];
                }
                pool.push_back(target);
                functional::Module index = std::min(0ULL, pool.size() - 1);
                if (interrupt(target, index)) {
                    break;
                }
                accept(target, index);
            }
        }, 1LL);
    }
};
