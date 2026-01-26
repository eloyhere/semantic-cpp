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
namespace collector {

    template<typename A, typename E, typename R>
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
    using Generator = BiConsumer<BiConsumer<T, Timestamp>, BiPredicate<T, Module>>;

    template <typename D>
    D randomly(const D& start, const D& end) {
        static std::random_device random;
        static std::mt19937_64 generator(random());

        D maximum = std::max(start, end);
        D minimum = std::min(start, end);

        if constexpr (std::is_integral<D>::value) {
            std::uniform_int_distribution<D> distribution(minimum, maximum);
            return distribution(generator);
        } else {
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

namespace collector {

    template<typename T>
    using Generator = functional::Generator<T>;

    template <typename A, typename R>
    using Finisher = functional::Function<A, R>;

    using Timestamp = functional::Timestamp;
    using Module = functional::Module;

    template <typename A>
    using Identity = functional::Supplier<A>;

    template <typename A, typename E>
    using Interrupt = functional::TriPredicate<A, E, Timestamp>;

    template <typename A, typename E>
    using Accumulator = functional::TriFunction<A, E, Timestamp, A>;

    template <typename A>
    using Combiner = functional::BiFunction<A, A, A>;

    template<typename T, typename R>
    using Function = functional::Function<T, R>;

    template<typename T>
    using Consumer = functional::Consumer<T>;

    template<typename T, typename U>
    using Comparator = functional::Comparator<T, U>;

    template <typename A, typename E, typename R>
    class Collector {
    protected:
        const Identity<A> identity;
        const Interrupt<A, E> interrupt;
        const Accumulator<A, E> accumulator;
        const Combiner<A> combiner;
        const Finisher<A, R> finisher;
        Collector(const Identity<A>& identity, const Interrupt<A, E>& interrupt, const Accumulator<A, E>& accumulator, const Combiner<A>& combiner, const Finisher<A, R>& finisher) : identity(std::move(identity)), interrupt(std::move(interrupt)), accumulator(std::move(accumulator)), combiner(std::move(combiner)), finisher(std::move(finisher)) {
        }

    public:
        R collect(const std::set<E>& container, const Module& concurrent) const {
            if (container.empty()) {
                return this->finisher(this->identity());
            }
            if (concurrent < 2) {
                A accumulator = this->identity();
                Module index = 0;
                for (const E& element : container) {
                    if (this->interrupt(accumulator, element, index)) {
                        break;
                    }
                    accumulator = this->accumulator(accumulator, element, index);
                    index++;
                }
                return this->finisher(accumulator);
            }
            std::vector<E> elements(container.begin(), container.end());
            std::vector<std::future<A>> futures;
            futures.reserve(concurrent);
            const Module size = elements.size();
            for (Module thread = 0; thread < concurrent; thread++) {
                futures.push_back(std::async(std::launch::async, [this, &elements, concurrent, thread, size] () -> A {
                    try {
                        A accumulator = this->identity();
                        for (Module index = thread; index < size; index += concurrent) {
                            const E& element = elements[index];
                            if (this->interrupt(accumulator, element, index)) {
                                break;
                            }
                            accumulator = this->accumulator(accumulator, element, index);
                        }
                        return accumulator;
                    } catch (...) {
                        return this->identity();
                    } }));
            }
            A accumulator = this->identity();
            for (std::future<A>& future : futures) {
                A partial = future.get();
                accumulator = this->combiner(accumulator, partial);
            }
            return this->finisher(accumulator);
        }

        R collect(const std::unordered_set<E>& container, const Module& concurrent) const {
            if (container.empty()) {
                return this->finisher(this->identity());
            }
            if (concurrent < 2) {
                A accumulator = this->identity();
                Module index = 0;
                for (const E& element : container) {
                    if (this->interrupt(accumulator, element, index)) {
                        break;
                    }
                    accumulator = this->accumulator(accumulator, element, index);
                    index++;
                }
                return this->finisher(accumulator);
            }
            std::vector<E> elements(container.begin(), container.end());
            std::vector<std::future<A>> futures;
            futures.reserve(concurrent);
            const Module size = elements.size();
            for (Module thread = 0; thread < concurrent; thread++) {
                futures.push_back(std::async(std::launch::async, [this, &elements, concurrent, thread, size] () -> A {
                    try {
                        A accumulator = this->identity();
                        for (Module index = thread; index < size; index += concurrent) {
                            const E& element = elements[index];
                            if (this->interrupt(accumulator, element, index)) {
                                break;
                            }
                            accumulator = this->accumulator(accumulator, element, index);
                        }
                        return accumulator;
                    } catch (...) {
                        return this->identity();
                    } }));
            }
            A accumulator = this->identity();
            for (std::future<A>& future : futures) {
                A partial = future.get();
                accumulator = this->combiner(accumulator, partial);
            }
            return this->finisher(accumulator);
        }

        R collect(const std::deque<E>& container, const Module& concurrent) const {
            if (container.empty()) {
                return this->finisher(this->identity());
            }
            if (concurrent < 2) {
                A accumulator = this->identity();
                Module index = 0;
                for (const E& element : container) {
                    if (this->interrupt(accumulator, element, index)) {
                        break;
                    }
                    accumulator = this->accumulator(accumulator, element, index);
                    index++;
                }
                return this->finisher(accumulator);
            }
            std::vector<std::future<A>> futures;
            futures.reserve(concurrent);
            const Module size = container.size();
            for (Module thread = 0; thread < concurrent; thread++) {
                futures.push_back(std::async(std::launch::async, [this, &container, concurrent, thread, size] () -> A {
                    try {
                        A accumulator = this->identity();
                        for (Module index = thread; index < size; index += concurrent) {
                            const E& element = container[index];
                            if (this->interrupt(accumulator, element, index)) {
                                break;
                            }
                            accumulator = this->accumulator(accumulator, element, index);
                        }
                        return accumulator;
                    } catch (...) {
                        return this->identity();
                    } }));
            }
            A accumulator = this->identity();
            for (std::future<A>& future : futures) {
                A partial = future.get();
                accumulator = this->combiner(accumulator, partial);
            }
            return this->finisher(accumulator);
        }

        R collect(const std::list<E>& container, const Module& concurrent) const {
            if (container.empty()) {
                return this->finisher(this->identity());
            }
            if (concurrent < 2) {
                A accumulator = this->identity();
                Module index = 0;
                for (const E& element : container) {
                    if (this->interrupt(accumulator, element, index)) {
                        break;
                    }
                    accumulator = this->accumulator(accumulator, element, index);
                    index++;
                }
                return this->finisher(accumulator);
            }
            std::vector<E> elements(container.begin(), container.end());
            std::vector<std::future<A>> futures;
            futures.reserve(concurrent);
            const Module size = elements.size();
            for (Module thread = 0; thread < concurrent; thread++) {
                futures.push_back(std::async(std::launch::async, [this, &elements, concurrent, thread, size] () -> A {
                    try {
                        A accumulator = this->identity();
                        for (Module index = thread; index < size; index += concurrent) {
                            const E& element = elements[index];
                            if (this->interrupt(accumulator, element, index)) {
                                break;
                            }
                            accumulator = this->accumulator(accumulator, element, index);
                        }
                        return accumulator;
                    } catch (...) {
                        return this->identity();
                    } }));
            }
            A accumulator = this->identity();
            for (std::future<A>& future : futures) {
                A partial = future.get();
                accumulator = this->combiner(accumulator, partial);
            }
            return this->finisher(accumulator);
        }

        R collect(const std::initializer_list<E>& container, const Module& concurrent) const {
            if (concurrent < 2) {
                A accumulator = this->identity();
                Module index = 0;
                for (const E& element : container) {
                    if (this->interrupt(accumulator, element, index)) {
                        break;
                    }
                    accumulator = this->accumulator(accumulator, element, index);
                    index++;
                }
                return this->finisher(accumulator);
            }
            std::vector<E> elements(container.begin(), container.end());
            std::vector<std::future<A>> futures;
            futures.reserve(concurrent);
            const Module size = elements.size();
            for (Module thread = 0; thread < concurrent; thread++) {
                futures.push_back(std::async(std::launch::async, [this, &elements, concurrent, thread, size] () -> A {
                    try {
                        A accumulator = this->identity();
                        for (Module index = thread; index < size; index += concurrent) {
                            const E& element = elements[index];
                            if (this->interrupt(accumulator, element, index)) {
                                break;
                            }
                            accumulator = this->accumulator(accumulator, element, index);
                        }
                        return accumulator;
                    } catch (...) {
                        return this->identity();
                    } }));
            }
            A accumulator = this->identity();
            for (std::future<A>& future : futures) {
                A partial = future.get();
                accumulator = this->combiner(accumulator, partial);
            }
            return this->finisher(accumulator);
        }

        R collect(const Generator<E>& generator, const Module& concurrent) const {
            if (concurrent < 2) {
                A accumulator = this->identity();
                generator([this, &accumulator] (const E& element, const Module& index)-> void {
                    accumulator = this->accumulator(accumulator, element, index);
                }, [this, &accumulator] (const E& element, const Module& index)-> bool {
                    return this->interrupt(accumulator, element, index);
                    });
                return accumulator;
            }
            std::vector<std::future<A>> futures;
            futures.reserve(concurrent);
            for (Module thread = 0; thread < concurrent; thread++) {
                futures.push_back(std::async(std::launch::async, [this, thread, concurrent, generator] () -> A {
                    try {
                        A accumulator = this->identity();
                        generator([this, &accumulator, thread, concurrent] (const E& element, const Module& index)-> void {
                            if (thread == index % concurrent) {
                                accumulator = this->accumulator(accumulator, element, index);
                            }
                        }, [this, &accumulator, thread, concurrent] (const E& element, const Module& index)-> bool {
                            if (thread == index % concurrent) {
                                return this->interrupt(accumulator, element, index);
                            }
                            return false;
                            });
                        return accumulator;
                    } catch (...) {
                        return this->identity();
                    } }));
            }
            A accumulator = this->identity();
            for (std::future<A>& future : futures) {
                A partial = future.get();
                accumulator = this->combiner(accumulator, partial);
            }
            return this->finisher(accumulator);
        }

        R collect(const std::vector<E>& container, const Module& concurrent) const {
            if (container.empty()) {
                return this->finisher(this->identity());
            }
            if (concurrent < 2) {
                A accumulator = this->identity();
                Module index = 0;
                for (const E& element : container) {
                    if (this->interrupt(accumulator, element, index)) {
                        break;
                    }
                    accumulator = this->accumulator(accumulator, element, index);
                    index++;
                }
                return this->finisher(accumulator);
            }
            std::vector<std::future<A>> futures;
            futures.reserve(concurrent);
            const Module size = container.size();
            for (Module thread = 0; thread < concurrent; thread++) {
                futures.push_back(std::async(std::launch::async, [this, &container, concurrent, thread, size] () -> A {
                    try {
                        A accumulator = this->identity();
                        for (Module index = thread; index < size; index += concurrent) {
                            const E& element = container[index];
                            if (this->interrupt(accumulator, element, index)) {
                                break;
                            }
                            accumulator = this->accumulator(accumulator, element, index);
                        }
                        return accumulator;
                    } catch (...) {
                        return this->identity();
                    } }));
            }
            A accumulator = this->identity();
            for (std::future<A>& future : futures) {
                A partial = future.get();
                accumulator = this->combiner(accumulator, partial);
            }
            return this->finisher(accumulator);
        }

        static Collector<A, E, R> full(const Identity<A>& identity, const Accumulator<A, E>& accumulator, const Combiner<A>& combiner, const Finisher<A, R>& finisher) {
            const Interrupt<A, E>& interrupt = [] (const A& accumulator, const E& element, const Module& index) -> bool {
                return false;
                };
            return Collector(identity, interrupt, accumulator, combiner, finisher);
        }

        static Collector<A, E, R> shortable(const Identity<A>& identity, const Interrupt<A, E>& interrupt, const Accumulator<A, E>& accumulator, const Combiner<A>& combiner, const Finisher<A, R>& finisher) {
            return Collector(identity, interrupt, accumulator, combiner, finisher);
        }
    };

    template <typename E>
    Collector<bool, E, bool> useAnyMatch(const functional::Predicate<E>& predicate) {
        return Collector<bool, E, bool>::shortable(
            [] () -> bool {
                return false;
            },
            [] (bool accumulator, E element, Timestamp index) -> bool {
                return accumulator;
            },
            [predicate] (bool accumulator, E element, Timestamp index) -> bool {
                try {
                    return accumulator || predicate(element);
                } catch (...) {
                    return accumulator;
                }
            },
            [] (bool a, bool b) -> bool {
                return a || b;
            },
            [] (bool a) -> bool {
                return a;
            });
    }

    template <typename E>
    Collector<bool, E, bool> useAllMatch(const functional::Predicate<E>& predicate) {
        return Collector<bool, E, bool>::shortable(
            [] () -> bool {
                return true;
            },
            [] (bool accumulator, E element, Timestamp index) -> bool {
                return accumulator;
            },
            [predicate] (bool accumulator, E element, Timestamp index) -> bool {
                try {
                    return accumulator && predicate(element);
                } catch (...) {
                    return accumulator;
                }
            },
            [] (bool a, bool b) -> bool {
                return a && b;
            },
            [] (bool a) -> bool {
                return a;
            });
    }

    template <typename E>
    Collector<Module, E, Module> useCount() {
        return Collector<Module, E, Module>::full(
            [] () -> Module {
                return 0;
            },
            [] (Module accumulator, E element, Timestamp index) -> Module {
                return accumulator + 1;
            },
            [] (Module a, Module b) -> Module {
                return a + b;
            },
            [] (Module a) -> Module {
                return a;
            });
    }

    template <typename E>
    Collector<Module, E, Module> useForEach(const Consumer<E>& action) {
        return Collector<Module, E, Module>::full(
            [] () -> Module {
                return 0;
            },
            [action] (Module accumulator, E element, Timestamp index) -> Module {
                try {
                    action(element);
                } catch (...) {
                    return accumulator + 1;
                }
            },
            [] (Module a, Module b) -> Module {
                return a + b;
            },
            [] (Module a) -> Module {
                return a;
            });
    }

    template <typename E>
    Collector<bool, E, bool> useNoneMatch(const functional::Predicate<E>& predicate) {
        return Collector<bool, E, bool>::shortable(
            [] () -> bool {
                return true;
            },
            [] (bool accumulator, E element, Timestamp index) -> bool {
                return accumulator;
            },
            [predicate] (bool accumulator, E element, Timestamp index) -> bool {
                try {
                    return accumulator && !predicate(element);
                } catch (...) {
                    return accumulator;
                }
            },
            [] (bool a, bool b) -> bool {
                return a && b;
            },
            [] (bool a) -> bool {
                return a;
            });
    }

    template<typename E, Module length>
    Collector<std::array<E, length>, E, std::array<E, length>> useToArray() {
        return Collector<std::array<E, length>, E, std::array<E, length>>::full(
            [] () -> std::array<E, length> {
                return {};
            },
            [] (std::array<E, length> accumulator, E element, Timestamp index) -> std::array<E, length> {
                if (index < length) {
                    accumulator[index] = element;
                }
                return accumulator;
            },
            [] (std::array<E, length> a, std::array<E, length> b) -> std::array<E, length> {
                for (Module i = 0; i < length; i++) {
                    a[i] = b[i];
                }
                return a;
            },
            [] (std::array<E, length> a) -> std::array<E, length> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::list<E>, E, std::list<E>> useToList() {
        return Collector<std::list<E>, E, std::list<E>>::full(
            [] () -> std::list<E> {
                return {};
            },
            [] (std::list<E> accumulator, E element, Timestamp index) -> std::list<E> {
                accumulator.push_back(element);
                return accumulator;
            },
            [] (std::list<E> a, std::list<E> b) -> std::list<E> {
                a.splice(a.end(), b);
                return a;
            },
            [] (std::list<E> a) -> std::list<E> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::vector<E>, E, std::vector<E>> useToVector() {
        return Collector<std::vector<E>, E, std::vector<E>>::full(
            [] () -> std::vector<E> {
                return {};
            },
            [] (std::vector<E> accumulator, E element, Timestamp index) -> std::vector<E> {
                accumulator.push_back(element);
                return accumulator;
            },
            [] (std::vector<E> a, std::vector<E> b) -> std::vector<E> {
                a.insert(a.end(), b.begin(), b.end());
                return a;
            },
            [] (std::vector<E> a) -> std::vector<E> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::set<E>, E, std::set<E>> useToSet() {
        return Collector<std::set<E>, E, std::set<E>>::full(
            [] () -> std::set<E> {
                return {};
            },
            [] (std::set<E> accumulator, E element, Timestamp index) -> std::set<E> {
                accumulator.insert(element);
                return accumulator;
            },
            [] (std::set<E> a, std::set<E> b) -> std::set<E> {
                a.insert(b.begin(), b.end());
                return a;
            },
            [] (std::set<E> a) -> std::set<E> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::unordered_set<E>, E, std::unordered_set<E>> useToUnorderedSet() {
        return Collector<std::unordered_set<E>, E, std::unordered_set<E>>::full(
            [] () -> std::unordered_set<E> {
                return {};
            },
            [] (std::unordered_set<E> accumulator, E element, Timestamp index) -> std::unordered_set<E> {
                accumulator.insert(element);
                return accumulator;
            },
            [] (std::unordered_set<E> a, std::unordered_set<E> b) -> std::unordered_set<E> {
                a.insert(b.begin(), b.end());
                return a;
            },
            [] (std::unordered_set<E> a) -> std::unordered_set<E> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::deque<E>, E, std::deque<E>> useToDeque() {
        return Collector<std::deque<E>, E, std::deque<E>>::full(
            [] () -> std::deque<E> {
                return {};
            },
            [] (std::deque<E> accumulator, E element, Timestamp index) -> std::deque<E> {
                accumulator.push_back(element);
                return accumulator;
            },
            [] (std::deque<E> a, std::deque<E> b) -> std::deque<E> {
                a.insert(a.end(), b.begin(), b.end());
                return a;
            },
            [] (std::deque<E> a) -> std::deque<E> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::queue<E>, E, std::queue<E>> useToQueue() {
        return Collector<std::queue<E>, E, std::queue<E>>::full(
            [] () -> std::queue<E> {
                return {};
            },
            [] (std::queue<E> accumulator, E element, Timestamp index) -> std::queue<E> {
                accumulator.push(element);
                return accumulator;
            },
            [] (std::queue<E> a, std::queue<E> b) -> std::queue<E> {
                while (!b.empty()) {
                    a.push(b.front());
                    b.pop();
                }
                return a;
            },
            [] (std::queue<E> a) -> std::queue<E> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::stack<E>, E, std::stack<E>> useToStack() {
        return Collector<std::stack<E>, E, std::stack<E>>::full(
            [] () -> std::stack<E> {
                return {};
            },
            [] (std::stack<E> accumulator, E element, Timestamp index) -> std::stack<E> {
                accumulator.push(element);
                return accumulator;
            },
            [] (std::stack<E> a, std::stack<E> b) -> std::stack<E> {
                while (!b.empty()) {
                    a.push(b.top());
                    b.pop();
                }
                return a;
            },
            [] (std::stack<E> a) -> std::stack<E> {
                return a;
            }
        );
    }

    template<typename E, typename K, typename V>
    Collector<std::map<K, V>, E, std::map<K, V>> useToMap(const Function<E, K>& keyExtractor, const Function<E, V>& valueExtractor) {
        return Collector<std::map<K, V>, E, std::map<K, V>>::full(
            [] () -> std::map<K, V> {
                return {};
            },
            [keyExtractor, valueExtractor] (std::map<K, V> accumulator, E element, Timestamp index) -> std::map<K, V> {
                try {
                    accumulator[keyExtractor(element)] = valueExtractor(element);
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::map<K, V> a, std::map<K, V> b) -> std::map<K, V> {
                for (const std::pair<K, V>& pair : b) {
                    a[pair.first] = pair.second;
                }
                return a;
            },
            [] (std::map<K, V> a) -> std::map<K, V> {
                return a;
            }
        );
    }

    template<typename E, typename K, typename V>
    Collector<std::unordered_map<K, V>, E, std::unordered_map<K, V>> useToUnorderedMap(const Function<E, K>& keyExtractor, const Function<E, V>& valueExtractor) {
        return Collector<std::unordered_map<K, V>, E, std::unordered_map<K, V>>::full(
            [] () -> std::unordered_map<K, V> {
                return {};
            },
            [keyExtractor, valueExtractor] (std::unordered_map<K, V> accumulator, E element, Timestamp index) -> std::unordered_map<K, V> {
                try {
                    accumulator[keyExtractor(element)] = valueExtractor(element);
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::unordered_map<K, V> a, std::unordered_map<K, V> b) -> std::unordered_map<K, V> {
                for (const std::pair<K, V>& pair : b) {
                    a[pair.first] = pair.second;
                }
                return a;
            },
            [] (std::unordered_map<K, V> a) -> std::unordered_map<K, V> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::optional<E>, E, std::optional<E>> useReduce() {
        return Collector<std::optional<E>, E, std::optional<E>>::full(
            [] () -> std::optional<E> {
                return std::nullopt;
            },
            [] (std::optional<E> accumulator, E element, Timestamp index) -> std::optional<E> {
                if (!accumulator.has_value()) {
                    return element;
                }
                return accumulator;
            },
            [] (std::optional<E> a, std::optional<E> b) -> std::optional<E> {
                if (a.has_value()) {
                    return a;
                }
                return b;
            },
            [] (std::optional<E> a) -> std::optional<E> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<E, E, E> useReduce(const E& identity) {
        return Collector<E, E, E>::full(
            [&identity] () -> E {
                return identity;
            },
            [] (E accumulator, E element, Timestamp index) -> E {
                return element;
            },
            [] (E a, E b) -> E {
                return b;
            },
            [] (E a) -> E {
                return a;
            }
        );
    }

    template<typename E, typename A, typename R>
    Collector<A, E, R> useReduce(const E& identity, const Accumulator<A, E>& accumulator, const Combiner<A>& combiner, const Finisher<A, R>& finisher) {
        return Collector<A, E, R>::full(
            [&identity] () -> E {
                return identity;
            },
            accumulator,
            combiner,
            finisher
        );
    }

    template<typename E>
    Collector<std::string, E, std::string> useJoin() {
        return Collector<std::string, E, std::string>::full(
            [] () -> std::string {
                return "";
            },
            [] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return std::to_string(element) + ",";
                }
                return accumulator + std::to_string(element) + ",";
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [] (std::string a) -> std::string {
                return "[" + a.substr(0, a.size() - 1) + "]";
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::string> useJoin(const Accumulator<std::string, E>& serializer) {
        return Collector<std::string, E, std::string>::full(
            [] () -> std::string {
                return "";
            },
            [] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return serializer(accumulator, element, index) + ",";
                }
                return accumulator + serializer(element) + ",";
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [] (std::string a) -> std::string {
                return "[" + a.substr(0, a.size() - 1) + "]";
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::string> useJoin(const std::string& delimeter) {
        return Collector<std::string, E, std::string>::full(
            [] () -> std::string {
                return "";
            },
            [delimeter] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return std::to_string(element) + ",";
                }
                return accumulator + std::to_string(element) + delimeter;
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [] (std::string a) -> std::string {
                return "[" + a.substr(0, a.size() - 1) + "]";
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::string> useJoin(const std::string& prefix, const std::string& delimeter, const std::string& suffix) {
        return Collector<std::string, E, std::string>::full(
            [] () -> std::string {
                return "";
            },
            [delimeter] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return std::to_string(element) + delimeter;
                }
                return accumulator + std::to_string(element) + delimeter;
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [prefix, suffix] (std::string a) -> std::string {
                return prefix + a.substr(0, a.size() - 1) + suffix;
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::string> useJoin(const std::string& prefix, const Accumulator<std::string, E>& serializer, const std::string& suffix) {
        return Collector<std::string, E, std::string>::full(
            [] () -> std::string {
                return "";
            },
            [serializer] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return serializer(accumulator, element, index);
                }
                return accumulator + serializer(accumulator, element, index);
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [prefix, suffix] (std::string a) -> std::string {
                return prefix + a.substr(0, a.size() - 1) + suffix;
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::ostream&> useCout() {
        return Collector<std::string, E, std::ostream&>::full(
            [] () -> std::string {
                return "";
            },
            [] (std::string accumulator, E element, Timestamp index) -> std::string {
                std::cout << element;
                return accumulator;
            },
            [] (std::string a, std::string b) -> std::string {
                return a;
            },
            [] (std::string a) -> std::ostream& {
                return std::cout << a;
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::ostream&> useCout(const Accumulator<std::string, E>& serializer) {
        return Collector<std::string, E, std::ostream&>::full(
            [] () -> std::string {
                return "";
            },
            [] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return serializer(accumulator, element, index) + ",";
                }
                return accumulator + serializer(element) + ",";
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [] (std::string a) -> std::ostream& {
                return std::cout << ("[" + a.substr(0, a.size() - 1) + "]");
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::ostream&> useCout(const std::string& delimiter) {
        return Collector<std::string, E, std::ostream&>::full(
            [] () -> std::string {
                return "";
            },
            [delimiter] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return std::to_string(element) + delimiter;
                }
                return std::to_string(element) + delimiter;
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [] (std::string a) -> std::ostream& {
                return std::cout << ("[" + a.substr(0, a.size() - 1) + "]");
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::ostream&> useCout(const std::string& prefix, const std::string& delimiter, const std::string& suffix) {
        return Collector<std::string, E, std::ostream&>::full(
            [] () -> std::string {
                return "";
            },
            [delimiter] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return std::to_string(element) + delimiter;
                }
                return accumulator + std::to_string(element) + delimiter;
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [prefix, suffix] (std::string a) -> std::ostream& {
                return std::cout << (prefix + a.substr(0, a.size() - 1) + suffix);
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::ostream&> useCout(const std::string& prefix, const Accumulator<std::string, E>& serializer, const std::string& suffix) {
        return Collector<std::string, E, std::ostream&>::full(
            [] () -> std::string {
                return "";
            },
            [serializer] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return serializer(accumulator, element, index);
                }
                return accumulator + serializer(accumulator, element, index);
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [prefix, suffix] (std::string a) -> std::ostream& {
                return std::cout << (prefix + a.substr(0, a.size() - 1) + suffix);
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::ostream&> useCout(std::ostream& stream) {
        return Collector<std::string, E, std::ostream&>::full(
            [] () -> std::string {
                return "";
            },
            [] (std::string accumulator, E element, Timestamp index) -> std::string {
                std::cout << element;
                return accumulator;
            },
            [] (std::string a, std::string b) -> std::string {
                return a;
            },
            [&stream] (std::string a) -> std::ostream& {
                return stream << a;
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::ostream&> useCout(std::ostream& stream, const Accumulator<std::string, E>& serializer) {
        return Collector<std::string, E, std::ostream&>::full(
            [] () -> std::string {
                return "";
            },
            [] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return serializer(accumulator, element, index) + ",";
                }
                return accumulator + serializer(element) + ",";
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [&stream] (std::string a) -> std::ostream& {
                return stream << a;
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::ostream&> useCout(std::ostream& stream, const std::string& delimiter) {
        return Collector<std::string, E, std::ostream&>::full(
            [] () -> std::string {
                return "";
            },
            [delimiter] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return std::to_string(element) + delimiter;
                }
                return std::to_string(element) + delimiter;
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [&stream] (std::string a) -> std::ostream& {
                return stream << ("[" + a.substr(0, a.size() - 1) + "]");
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::ostream&> useCout(std::ostream& stream, const std::string& prefix, const std::string& delimiter, const std::string& suffix) {
        return Collector<std::string, E, std::ostream&>::full(
            [] () -> std::string {
                return "";
            },
            [delimiter] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return serializer(accumulator, element, index);
                }
                return accumulator + serializer(accumulator, element, index);
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [prefix, suffix, &stream] (std::string a) -> std::ostream& {
                return stream << (prefix + a.substr(0, a.size() - 1) + suffix);
            }
        );
    }

    template<typename E>
    Collector<std::string, E, std::ostream&> useCout(std::ostream& stream, const std::string& prefix, const Accumulator<std::string, E>& serializer, const std::string& suffix) {
        return Collector<std::string, E, std::ostream&>::full(
            [] () -> std::string {
                return "";
            },
            [serializer] (std::string accumulator, E element, Timestamp index) -> std::string {
                if (accumulator.size() == 0) {
                    return serializer(accumulator, element, index);
                }
                return accumulator + serializer(accumulator, element, index);
            },
            [] (std::string a, std::string b) -> std::string {
                return b;
            },
            [prefix, suffix, &stream] (std::string a) -> std::ostream& {
                return stream << (prefix + a.substr(0, a.size() - 1) + suffix);
            }
        );
    }

    template<typename E, typename K>
    Collector<std::unordered_map<K, std::vector<E>>, E, std::unordered_map<K, std::vector<E>>> useGroup(const Function<E, K>& classifier) {
        return Collector<std::unordered_map<K, std::vector<E>>, E, std::unordered_map<K, std::vector<E>>>::full(
            [] () -> std::unordered_map<K, std::vector<E>> {
                return std::unordered_map<K, std::vector<E>>();
            },
            [classifier] (std::unordered_map<K, std::vector<E>> accumulator, E element, Timestamp index) -> std::unordered_map<K, std::vector<E>> {
                try {
                    K key = classifier(element);
                    accumulator[key].push_back(element);
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::unordered_map<K, std::vector<E>> a, std::unordered_map<K, std::vector<E>> b) -> std::unordered_map<K, std::vector<E>> {
                for (auto& pair : b) {
                    const K& key = pair.first;
                    std::vector<E>& targetVector = a[key];
                    const std::vector<E>& sourceVector = pair.second;
                    targetVector.insert(targetVector.end(), sourceVector.begin(), sourceVector.end());
                }
                return a;
            },
            [] (std::unordered_map<K, std::vector<E>> a) -> std::unordered_map<K, std::vector<E>> {
                return a;
            }
        );
    }

    template<typename E, typename K, typename V>
    Collector<std::unordered_map<K, std::vector<V>>, E, std::unordered_map<K, std::vector<V>>> useGroupBy(const Function<E, K>& keyExtractor, const Function<E, V>& valueExtractor) {
        return Collector<std::unordered_map<K, std::vector<V>>, E, std::unordered_map<K, std::vector<V>>>::full(
            [] () -> std::unordered_map<K, std::vector<V>> {
                return std::unordered_map<K, std::vector<V>>();
            },
            [keyExtractor, valueExtractor] (std::unordered_map<K, std::vector<V>> accumulator, E element, Timestamp index) -> std::unordered_map<K, std::vector<V>> {
                try {
                    K key = keyExtractor(element);
                    V value = valueExtractor(element);
                    accumulator[key].push_back(value);
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::unordered_map<K, std::vector<V>> a, std::unordered_map<K, std::vector<V>> b) -> std::unordered_map<K, std::vector<V>> {
                for (auto& pair : b) {
                    const K& key = pair.first;
                    std::vector<V>& targetVector = a[key];
                    const std::vector<V>& sourceVector = pair.second;
                    targetVector.insert(targetVector.end(), sourceVector.begin(), sourceVector.end());
                }
                return a;
            },
            [] (std::unordered_map<K, std::vector<V>> a) -> std::unordered_map<K, std::vector<V>> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::vector<std::vector<E>>, E, std::vector<std::vector<E>>> usePartition(const Module& count) {
        return Collector<std::vector<std::vector<E>>, E, std::vector<std::vector<E>>>::full(
            [count] () -> std::vector<std::vector<E>> {
                return std::vector<std::vector<E>>(count);
            },
            [] (std::vector<std::vector<E>> accumulator, E element, Timestamp index) -> std::vector<std::vector<E>> {
                Module partitionIndex = index % count;
                accumulator[partitionIndex].push_back(element);
                return accumulator;
            },
            [] (std::vector<std::vector<E>> a, std::vector<std::vector<E>> b) -> std::vector<std::vector<E>> {
                for (Module i = 0; i < count; i++) {
                    a[i].insert(a[i].end(), b[i].begin(), b[i].end());
                }
                return a;
            },
            [] (std::vector<std::vector<E>> a) -> std::vector<std::vector<E>> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::vector<std::vector<E>>, E, std::vector<std::vector<E>>> usePartitionBy(const Function<E, Module>& delivery) {
        return Collector<std::vector<std::vector<E>>, E, std::vector<std::vector<E>>>::full(
            [] () -> std::vector<std::vector<E>> {
                return std::vector<std::vector<E>>();
            },
            [delivery] (std::vector<std::vector<E>> accumulator, E element, Timestamp index) -> std::vector<std::vector<E>> {
                Module partitionIndex = delivery(element);
                if (partitionIndex >= accumulator.size()) {
                    accumulator.resize(partitionIndex + 1);
                }
                accumulator[partitionIndex].push_back(element);
                return accumulator;
            },
            [] (std::vector<std::vector<E>> a, std::vector<std::vector<E>> b) -> std::vector<std::vector<E>> {
                if (b.size() > a.size()) {
                    a.resize(b.size());
                }
                for (Module i = 0; i < b.size(); i++) {
                    a[i].insert(a[i].end(), b[i].begin(), b[i].end());
                }
                return a;
            },
            [] (std::vector<std::vector<E>> a) -> std::vector<std::vector<E>> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::optional<E>, E, std::optional<E>> useMaximum(const Comparator<E, E>& comparator) {
        return Collector<std::optional<E>, E, std::optional<E>>::shortable(
            [] () -> std::optional<E> {
                return std::nullopt;
            },
            [] (std::optional<E> accumulator, E element, Timestamp index) -> std::optional<E> {
                return accumulator;
            },
            [comparator] (std::optional<E> accumulator, E element, Timestamp index) -> std::optional<E> {
                try {
                    if (!accumulator.has_value()) {
                        return element;
                    }
                    Timestamp result = comparator(element, accumulator.value());
                    if (result > 0) {
                        return element;
                    }
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::optional<E> a, std::optional<E> b) -> std::optional<E> {
                if (!a.has_value()) {
                    return b;
                }
                if (!b.has_value()) {
                    return a;
                }
                Timestamp result = comparator(a.value(), b.value());
                return result > 0 ? a : b;
            },
            [] (std::optional<E> a) -> std::optional<E> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::optional<E>, E, std::optional<E>> useMinimum(const Comparator<E, E>& comparator) {
        return Collector<std::optional<E>, E, std::optional<E>>::shortable(
            [] () -> std::optional<E> {
                return std::nullopt;
            },
            [] (std::optional<E> accumulator, E element, Timestamp index) -> std::optional<E> {
                return accumulator;
            },
            [comparator] (std::optional<E> accumulator, E element, Timestamp index) -> std::optional<E> {
                try {
                    if (!accumulator.has_value()) {
                        return element;
                    }
                    Timestamp result = comparator(element, accumulator.value());
                    if (result < 0) {
                        return element;
                    }
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::optional<E> a, std::optional<E> b) -> std::optional<E> {
                if (!a.has_value()) {
                    return b;
                }
                if (!b.has_value()) {
                    return a;
                }
                Timestamp result = comparator(a.value(), b.value());
                return result < 0 ? a : b;
            },
            [] (std::optional<E> a) -> std::optional<E> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::optional<E>, E, std::optional<E>> useFindFirst() {
        return Collector<std::optional<E>, E, std::optional<E>>::shortable(
            [] () -> std::optional<E> {
                return std::nullopt;
            },
            [] (std::optional<E> accumulator, E element, Timestamp index) -> std::optional<E> {
                return accumulator;
            },
            [] (std::optional<E> accumulator, E element, Timestamp index) -> std::optional<E> {
                if (!accumulator.has_value() && index == 0) {
                    return element;
                }
                return accumulator;
            },
            [] (std::optional<E> a, std::optional<E> b) -> std::optional<E> {
                if (!a.has_value() && b.has_value()) {
                    return b;
                }
                return a;
            },
            [] (std::optional<E> a) -> std::optional<E> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::optional<E>, E, std::optional<E>> useFindAny() {
        return Collector<std::optional<E>, E, std::optional<E>>::shortable(
            [] () -> std::optional<E> {
                return std::nullopt;
            },
            [] (std::optional<E> accumulator, E element, Timestamp index) -> std::optional<E> {
                return accumulator.has_value();
            },
            [] (std::optional<E> accumulator, E element, Timestamp index) -> std::optional<E> {
                if (accumulator.has_value() && functional::randomly()) {
                    return element;
                }
                return element;
            },
            [] (std::optional<E> a, std::optional<E> b) -> std::optional<E> {
                if (!a.has_value() && b.has_value()) {
                    return b;
                }
                if (a.has_value() && b.has_value() && functional::randomly()) {
                    return b;
                }
                return a;
            },
            [] (std::optional<E> a) -> std::optional<E> {
                return a;
            }
        );
    }

    template<typename E>
    Collector<std::optional<E>, E, std::optional<E>> useFindLast() {
        return Collector<std::optional<E>, E, std::optional<E>>::full(
            [] () -> std::optional<E> {
                return std::nullopt;
            },
            [] (std::optional<E> accumulator, E element, Timestamp index) -> std::optional<E> {
                return element;
            },
            [] (std::optional<E> a, std::optional<E> b) -> std::optional<E> {
                return b;
            },
            [] (std::optional<E> a) -> std::optional<E> {
                return a;
            }
        );
    }

    template<typename E, typename D>
    Collector<std::pair<D, D>, E, D> useRange(const Function<E, D>& mapper) {
        static_assert(std::is_arithmetic<D>::value, "D must be an arithmetic type");
        return Collector<std::pair<D, D>, E, D>::full(
            [] () -> std::pair<D, D> {
                return std::make_pair(std::numeric_limits<D>::max(), std::numeric_limits<D>::lowest());
            },
            [mapper] (std::pair<D, D> accumulator, E element, Timestamp index) -> std::pair<D, D> {
                try {
                    D value = static_cast<D>(mapper(element));
                    if (value < accumulator.first) {
                        accumulator.first = value;
                    }
                    if (value > accumulator.second) {
                        accumulator.second = value;
                    }
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::pair<D, D> a, std::pair<D, D> b) -> std::pair<D, D> {
                return std::make_pair(
                    std::min(a.first, b.first),
                    std::max(a.second, b.second)
                );
            },
            [] (std::pair<D, D> a) -> D {
                if (a.first > a.second) {
                    return static_cast<D>(0);
                }
                return a.second - a.first;
            }
        );
    }

    template<typename E, typename D>
    Collector<std::pair<D, D>, E, D> useAverage(const Function<E, D>& mapper) {
        static_assert(std::is_arithmetic<D>::value, "D must be an arithmetic type");
        return Collector<std::pair<D, D>, E, D>::full(
            [] () -> std::pair<D, D> {
                return std::make_pair(static_cast<D>(0), static_cast<D>(0));
            },
            [mapper] (std::pair<D, D> accumulator, E element, Timestamp index) -> std::pair<D, D> {
                try {
                    D value = static_cast<D>(mapper(element));
                    accumulator.first += value;
                    accumulator.second += static_cast<D>(1);
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }

            },
            [] (std::pair<D, D> a, std::pair<D, D> b) -> std::pair<D, D> {
                return std::make_pair(a.first + b.first, a.second + b.second);
            },
            [] (std::pair<D, D> a) -> D {
                if (a.second == static_cast<D>(0)) {
                    return static_cast<D>(0);
                }
                return a.first / a.second;
            }
        );
    }

    template<typename E, typename D>
    Collector<D, E, D> useSummate(const Function<E, D>& mapper) {
        static_assert(std::is_arithmetic<D>::value, "D must be an arithmetic type");
        return Collector<D, E, D>::full(
            [] () -> D {
                return static_cast<D>(0);
            },
            [mapper] (D accumulator, E element, Timestamp index) -> D {
                try {
                    D value = static_cast<D>(mapper(element));
                    accumulator += value;
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }
            },
            [] (D a, D b) -> D {
                return a + b;
            },
            [] (D a) -> D {
                return a;
            }
        );
    }

    template<typename E, typename D>
    Collector<std::pair<std::pair<D, D>, D>, E, D> useVariance(const Function<E, D>& mapper) {
        static_assert(std::is_arithmetic<D>::value, "D must be an arithmetic type");
        return Collector<std::pair<std::pair<D, D>, D>, E, D>::full(
            [] () -> std::pair<std::pair<D, D>, D> {
                return std::make_pair(std::make_pair(static_cast<D>(0), static_cast<D>(0)), static_cast<D>(0));
            },
            [mapper] (std::pair<std::pair<D, D>, D> accumulator, E element, Timestamp index) -> std::pair<std::pair<D, D>, D> {
                try {
                    D value = static_cast<D>(mapper(element));
                    D sum = accumulator.first.first + value;
                    D sumSquared = accumulator.first.second + value * value;
                    D count = accumulator.second + static_cast<D>(1);
                    return std::make_pair(std::make_pair(sum, sumSquared), count);
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::pair<std::pair<D, D>, D> a, std::pair<std::pair<D, D>, D> b) -> std::pair<std::pair<D, D>, D> {
                return std::make_pair(
                    std::make_pair(a.first.first + b.first.first, a.first.second + b.first.second),
                    a.second + b.second
                );
            },
            [] (std::pair<std::pair<D, D>, D> a) -> D {
                if (a.second <= static_cast<D>(1)) {
                    return static_cast<D>(0);
                }
                D mean = a.first.first / a.second;
                D variance = (a.first.second / a.second) - (mean * mean);
                return variance > static_cast<D>(0) ? variance : static_cast<D>(0);
            }
        );
    }

    template<typename E, typename D>
    Collector<std::pair<std::pair<D, D>, D>, E, D> useStandardDeviation(const Function<E, D>& mapper) {
        static_assert(std::is_arithmetic<D>::value, "D must be an arithmetic type");

        return Collector<std::pair<std::pair<D, D>, D>, E, D>::full(
            [] () -> std::pair<std::pair<D, D>, D> {
                return std::make_pair(std::make_pair(static_cast<D>(0), static_cast<D>(0)), static_cast<D>(0));
            },
            [mapper] (std::pair<std::pair<D, D>, D> accumulator, E element, Timestamp index) -> std::pair<std::pair<D, D>, D> {
                try {
                    D value = static_cast<D>(mapper(element));
                    D sum = accumulator.first.first + value;
                    D sumSquared = accumulator.first.second + value * value;
                    D count = accumulator.second + static_cast<D>(1);
                    return std::make_pair(std::make_pair(sum, sumSquared), count);
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::pair<std::pair<D, D>, D> a, std::pair<std::pair<D, D>, D> b) -> std::pair<std::pair<D, D>, D> {
                return std::make_pair(
                    std::make_pair(a.first.first + b.first.first, a.first.second + b.first.second),
                    a.second + b.second
                );
            },
            [] (std::pair<std::pair<D, D>, D> a) -> D {
                if (a.second <= static_cast<D>(1)) {
                    return static_cast<D>(0);
                }
                D mean = a.first.first / a.second;
                D variance = (a.first.second / a.second) - (mean * mean);
                return variance > static_cast<D>(0) ? std::sqrt(variance) : static_cast<D>(0);
            }
        );
    }

    template<typename E, typename D>
    Collector<D, E, D> useMean(const Function<E, D>& mapper) {
        static_assert(std::is_arithmetic<D>::value, "D must be an arithmetic type");
        return Collector<D, E, D>::full(
            [] () -> std::pair<D, D> {
                return std::make_pair(static_cast<D>(0), static_cast<D>(0));
            },
            [mapper] (std::pair<D, D> accumulator, E element, Timestamp index) -> std::pair<D, D> {
                try {
                    D value = static_cast<D>(mapper(element));
                    return std::make_pair(accumulator.first + value, accumulator.second + static_cast<D>(1));
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::pair<D, D> a, std::pair<D, D> b) -> std::pair<D, D> {
                return std::make_pair(a.first + b.first, a.second + b.second);
            },
            [] (std::pair<D, D> a) -> D {
                return a.second > static_cast<D>(0) ? a.first / a.second : static_cast<D>(0);
            }
        );
    }

    template<typename E, typename D>
    Collector<std::vector<D>, E, D> useMedian(const Function<E, D>& mapper) {
        return Collector<std::vector<D>, E, D>::full(
            [] () -> std::vector<D> {
                return std::vector<D>();
            },
            [mapper] (std::vector<D> accumulator, E element, Timestamp index) -> std::vector<D> {
                try {
                    accumulator.push_back(static_cast<D>(mapper(element)));
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::vector<D> a, std::vector<D> b) -> std::vector<D> {
                a.insert(a.end(), b.begin(), b.end());
                return a;
            },
            [] (std::vector<D> a) -> D {
                if (a.empty()) {
                    return static_cast<D>(0);
                }
                std::sort(a.begin(), a.end());
                size_t n = a.size();
                if (n % 2 == 0) {
                    return (a[n / 2 - 1] + a[n / 2]) / static_cast<D>(2);
                } else {
                    return a[n / 2];
                }
            }
        );
    }

    template<typename E, typename D>
    Collector<std::vector<D>, E, std::vector<D>> useMode(const Function<E, D>& mapper) {
        static_assert(std::is_arithmetic<D>::value, "D must be an arithmetic type");

        return Collector<std::vector<D>, E, std::vector<D>>::full(
            [] () -> std::map<D, Module> {
                return std::map<D, Module>();
            },
            [mapper] (std::map<D, Module> accumulator, E element, Timestamp index) -> std::map<D, Module> {
                try {
                    accumulator[static_cast<D>(element)]++;
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::map<D, Module> a, std::map<D, Module> b) -> std::map<D, Module> {
                for (const auto& pair : b) {
                    a[pair.first] += pair.second;
                }
                return a;
            },
            [] (std::map<D, Module> a) -> std::vector<D> {
                if (a.empty()) {
                    return std::vector<D>();
                }
                Module maxCount = 0;
                for (const auto& pair : a) {
                    if (pair.second > maxCount) {
                        maxCount = pair.second;
                    }
                }
                std::vector<D> modes;
                for (const auto& pair : a) {
                    if (pair.second == maxCount) {
                        modes.push_back(pair.first);
                    }
                }
                return modes;
            }
        );
    }

    template<typename E, typename D>
    Collector<std::vector<D>, E, std::tuple<D, D, D, D>> useQuartiles(const Function<E, D>& mapper) {
        static_assert(std::is_arithmetic<D>::value, "D must be an arithmetic type");

        return Collector<std::vector<D>, E, std::tuple<D, D, D, D>>::full(
            [] () -> std::vector<D> {
                return std::vector<D>();
            },
            [mapper] (std::vector<D> accumulator, E element, Timestamp index) -> std::vector<D> {
                try {
                    accumulator.push_back(static_cast<D>(element));
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }

            },
            [] (std::vector<D> a, std::vector<D> b) -> std::vector<D> {
                a.insert(a.end(), b.begin(), b.end());
                return a;
            },
            [] (std::vector<D> a) -> std::tuple<D, D, D, D> {
                if (a.size() < 4) {
                    D zero = static_cast<D>(0);
                    return std::make_tuple(zero, zero, zero, zero);
                }
                std::sort(a.begin(), a.end());
                size_t n = a.size();
                D q1 = a[n / 4];
                D median = a[n / 2];
                D q3 = a[(3 * n) / 4];
                return std::make_tuple(q1, median, q3, a.back() - a.front());
            }
        );
    }

    template<typename E, typename D>
    Collector<std::vector<D>, E, D> useInterquartileRange(const Function<E, D>& mapper) {
        static_assert(std::is_arithmetic<D>::value, "D must be an arithmetic type");

        return Collector<std::vector<D>, E, D>::full(
            [] () -> std::vector<D> {
                return std::vector<D>();
            },
            [mapper] (std::vector<D> accumulator, E element, Timestamp index) -> std::vector<D> {
                try {
                    accumulator.push_back(static_cast<D>(element));
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::vector<D> a, std::vector<D> b) -> std::vector<D> {
                a.insert(a.end(), b.begin(), b.end());
                return a;
            },
            [] (std::vector<D> a) -> D {
                if (a.size() < 4) {
                    return static_cast<D>(0);
                }
                std::sort(a.begin(), a.end());
                size_t n = a.size();
                D q1 = a[n / 4];
                D q3 = a[(3 * n) / 4];
                return q3 - q1;
            }
        );
    }

    template<typename E, typename D>
    Collector<std::vector<D>, E, D> useSkewness(const Function<E, D>& mapper) {
        static_assert(std::is_arithmetic<D>::value, "D must be an arithmetic type");

        return Collector<std::vector<D>, E, D>::full(
            [] () -> std::vector<D> {
                return std::vector<D>();
            },
            [mapper] (std::vector<D> accumulator, E element, Timestamp index) -> std::vector<D> {
                try {
                    D value = static_cast<D>(mapper(element));
                    accumulator.push_back(value);
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }

            },
            [] (std::vector<D> a, std::vector<D> b) -> std::vector<D> {
                a.insert(a.end(), b.begin(), b.end());
                return a;
            },
            [] (std::vector<D> a) -> D {
                if (a.size() < 3) {
                    return static_cast<D>(0);
                }
                D sum = std::accumulate(a.begin(), a.end(), static_cast<D>(0));
                D mean = sum / static_cast<D>(a.size());
                D m2 = static_cast<D>(0);
                D m3 = static_cast<D>(0);

                for (D value : a) {
                    D diff = value - mean;
                    D diff2 = diff * diff;
                    D diff3 = diff2 * diff;
                    m2 += diff2;
                    m3 += diff3;
                }

                m2 /= static_cast<D>(a.size() - 1);
                m3 /= static_cast<D>(a.size() - 1);
                if (m2 <= static_cast<D>(0)) {
                    return static_cast<D>(0);
                }

                D skewness = m3 / std::pow(m2, static_cast<D>(1.5));
                return skewness;
            }
        );
    }

    template<typename E, typename D>
    Collector<std::vector<D>, E, D> useKurtosis(const Function<E, D>& mapper) {
        static_assert(std::is_arithmetic<D>::value, "D must be an arithmetic type");

        return Collector<std::vector<D>, E, D>::full(
            [] () -> std::vector<D> {
                return std::vector<D>();
            },
            [mapper] (std::vector<D> accumulator, E element, Timestamp index) -> std::vector<D> {
                try {
                    D value = static_cast<D>(mapper(element));
                    accumulator.push_back(value);
                    return accumulator;
                } catch (...) {
                    return accumulator;
                }
            },
            [] (std::vector<D> a, std::vector<D> b) -> std::vector<D> {
                a.insert(a.end(), b.begin(), b.end());
                return a;
            },
            [] (std::vector<D> a) -> D {
                if (a.size() < 4) {
                    return static_cast<D>(0);
                }

                D sum = std::accumulate(a.begin(), a.end(), static_cast<D>(0));
                D mean = sum / static_cast<D>(a.size());

                D m2 = static_cast<D>(0);
                D m4 = static_cast<D>(0);

                for (D value : a) {
                    D diff = value - mean;
                    D diff2 = diff * diff;
                    D diff4 = diff2 * diff2;
                    m2 += diff2;
                    m4 += diff4;
                }

                m2 /= static_cast<D>(a.size() - 1);
                m4 /= static_cast<D>(a.size() - 1);
                if (m2 <= static_cast<D>(0)) {
                    return static_cast<D>(0);
                }

                D kurtosis = m4 / (m2 * m2);
                return kurtosis - static_cast<D>(3);
            }
        );
    }

    template<typename E>
    Collector<std::map<E, Module>, E, std::map<E, Module>> useFrequency() {
        return Collector<std::map<E, Module>, E, std::map<E, Module>>::full(
            [] () -> std::map<E, Module> {
                return std::map<E, Module>();
            },
            [] (std::map<E, Module> accumulator, E element, Timestamp index) -> std::map<E, Module> {
                accumulator[element]++;
                return accumulator;
            },
            [] (std::map<E, Module> a, std::map<E, Module> b) -> std::map<E, Module> {
                for (const auto& pair : b) {
                    a[pair.first] += pair.second;
                }
                return a;
            },
            [] (std::map<E, Module> a) -> std::map<E, Module> {
                return a;
            }
        );
    }
};

namespace collectable {
    using Timestamp = functional::Timestamp;
    using Module = functional::Module;

    template <typename T>
    using Generator = functional::Generator<T>;

    template<typename T>
    using Predicate = functional::Predicate<T>;

    template<typename T, typename R>
    using Function = functional::Function<T, R>;

    template<typename T, typename U, typename R>
    using BiFunction = functional::BiFunction<T, U, R>;

    template<typename T, typename U, typename V, typename R>
    using TriFunction = functional::TriFunction<T, U, V, R>;

    template <typename E>
    class Collectable {
    protected:
        const Module concurrent;
        const std::unique_ptr<Generator<E>> generator;
        virtual std::vector<E> source() = 0;
    public:
        Collectable(const Generator<E>& generator) : generator(std::make_unique<Generator<E>>(std::move(generator))), concurrent(1) { }
        Collectable(const Generator<E>& generator, const Module& concurrent) : generator(std::make_unique<Generator<E>>(std::move(generator))), concurrent(concurrent) { }
        Collectable(const Collectable<E>& other) : generator(std::move(other.generator)), concurrent(other.concurrent) { }
        Collectable(Collectable<E>&& other) noexcept : generator(std::move(other.generator)), concurrent(other.concurrent) { }
        Collectable<E>& operator=(const Collectable<E>& other) {
            if (*this != other) {
                this->generator = std::move(other.generator);
                this->concurrent = other.concurrent;
            }
            return *this;
        }
        Collectable<E>& operator=(Collectable<E>&& other) noexcept {
            if (*this != other) {
                this->generator = std::move(other.generator);
                this->concurrent = other.concurrent;
            }
            return *this;
        }

        bool anyMatch(const Predicate<E>& predicate)const {
            const collector::Collector<bool, E, bool> collector = collector::useAnyMatch(predicate);
            return collector.collect(*(this->source), this->concurrent);
        }

        bool allMatch(const Predicate<E>& predicate)const {
            const collector::Collector<bool, E, bool> collector = collector::useAllMatch(predicate);
            return collector.collect(*(this->source), this->concurrent);
        }

        bool noneMatch(const Predicate<E>& predicate)const {
            const collector::Collector<bool, E, bool> collector = collector::useNoneMatch(predicate);
            return collector.collect(*(this->source), this->concurrent);
        }

        Module count()const {
            const collector::Collector<Module, E, Module> collector = collector::useCount();
            return collector.collect(*(this->source), this->concurrent);
        }

        std::set<E> toSet() const {
            const collector::Collector<std::set<E>, E, std::set<E>> collector = collector::useToSet();
            return collector.collect(*(this->source), this->concurrent);
        }

        std::unordered_set<E> toUnorderedSet() const {
            const collector::Collector<std::unordered_set<E>, E, std::unordered_set<E>> collector = collector::useToUnorderedSet();
            return collector.collect(*(this->source), this->concurrent);
        }

        std::list<E> toList()const {
            const collector::Collector<std::list<E>, E, std::list<E>> collector = collector::useToList();
            return collector.collect(*(this->source), this->concurrent);
        }

        std::vector<E> toVector()const {
            const collector::Collector<std::vector<E>, E, std::vector<E>> collector = collector::useToVector();
            return collector.collect(*(this->source), this->concurrent);
        }

        std::deque<E> toDeque()const {
            const collector::Collector<std::deque<E>, E, std::deque<E>> collector = collector::useToDeque();
            return collector.collect(*(this->source), this->concurrent);
        }

        std::queue<E> toQueue()const {
            const collector::Collector<std::queue<E>, E, std::queue<E>> collector = collector::useToQueue();
            return collector.collect(*(this->source), this->concurrent);
        }

        std::stack<E> toStack()const {
            const collector::Collector<std::stack<E>, E, std::stack<E>> collector = collector::useToStack();
            return collector.collect(*(this->source), this->concurrent);
        }

        template<typename K, typename V>
        std::map<K, V> toMap(const Function<E, K>& keyExtractor, Function<E, V>& valueExtractor) const {
            const collector::Collector<std::map<K, V>, E, std::map<K, V>> collector = collector::useToMap(keyExtractor, valueExtractor);
            return collector.collect(*(this->source), this->concurrent);
        }

        template<typename K, typename V>
        std::unordered_map<K, V> toUnorderedMap(const Function<E, K>& keyExtractor, const Function<E, V>& valueExtractor)const {
            const collector::Collector<std::unordered_map<K, V>, E, std::unordered_map<K, V>> collector = collector::useToUnorderedMap(keyExtractor, valueExtractor);
            return collector.collect(*(this->source), this->concurrent);
        }

        template<Module length>
        std::array<E, length> toArray() const {
            const collector::Collector<std::array<E, length>, E, std::array<E, length>> collector = collector::useToArray<E, length>();
            return collector.collect(*(this->source), this->concurrent);
        }

        std::optional<E> reduce() const {
            const collector::Collector<std::optional<E>, E, std::optional<E>> collector = collector::useReduce<E>();
            return collector.collect(*(this->source), this->concurrent);
        }

        E reduce(const E& identity) const {
            const collector::Collector<E, E, E> collector = collector::useReduce(identity);
            return collector.collect(*(this->source), this->concurrent);
        }

        template<typename A, typename R>
        R reduce(const E& identity, const TriFunction<A, E, Timestamp, A>& accumulator, const BiFunction<A, A, A>& combiner, const Function<A, R>& finisher) const {
            const collector::Collector<E, E, E> collector = collector::useReduce(identity, accumulator, combiner, finisher);
            return collector.collect(*(this->source), this->concurrent);
        }

        void forEach()const {
            const collector::Collector<Module, E, Module> collector = collector::useForEach();
            return collector.collect(*(this->source), this->concurrent);
        }

        std::string join(const functional::Function<E, std::string>& serializer) const {
            const collector::Collector<std::string, E, std::string> collector = collector::useJoin(serializer);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::string join(const std::string& delimiter) const {
            const collector::Collector<std::string, E, std::string> collector = collector::useJoin<E>(delimiter);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::string join(const std::string& prefix, const std::string& delimiter, const std::string& suffix) const {
            const collector::Collector<std::string, E, std::string> collector = collector::useJoin(prefix, delimiter, suffix);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::string join(const std::string& prefix, const TriFunction<std::string, E, Timestamp, std::string>& serializer, const std::string& suffix) const {
            const collector::Collector<std::string, E, std::string> collector = collector::useJoin(prefix, serializer, suffix);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::ostream& cout() const {
            const collector::Collector<std::string, E, std::ostream&> collector = collector::useCout<E>();
            return collector.collect(*this->generator, this->concurrent);
        }

        std::ostream& cout(const functional::Function<E, std::string>& serializer) const {
            const collector::Collector<std::string, E, std::ostream&> collector = collector::useCout(serializer);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::ostream& cout(const functional::TriFunction<std::string, E, Timestamp, std::string>& serializer) const {
            const collector::Collector<std::string, E, std::ostream&> collector = collector::useCout<E>(serializer);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::ostream& cout(const std::string& delimiter) const {
            const collector::Collector<std::string, E, std::ostream&> collector = collector::useCout<E>(delimiter);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::ostream& cout(const std::string& prefix, const std::string& delimiter, const std::string& suffix) const {
            const collector::Collector<std::string, E, std::ostream&> collector = collector::useCout(prefix, delimiter, suffix);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::ostream& cout(const std::string& prefix, const functional::TriFunction<std::string, E, Timestamp, std::string>& serializer, const std::string& suffix) const {
            const collector::Collector<std::string, E, std::ostream&> collector = collector::useCout(prefix, serializer, suffix);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::ostream& cout(std::ostream& stream) const {
            const collector::Collector<std::string, E, std::ostream&> collector = collector::useCout(stream);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::ostream& cout(std::ostream& stream, const functional::Function<E, std::string>& serializer) const {
            const collector::Collector<std::string, E, std::ostream&> collector = collector::useCout(stream, serializer);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::ostream& cout(std::ostream& stream, const functional::TriFunction<std::string, E, Timestamp, std::string>& serializer) const {
            const collector::Collector<std::string, E, std::ostream&> collector = collector::useCout<E>(stream, serializer);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::ostream& cout(std::ostream& stream, const std::string& delimiter) const {
            const collector::Collector<std::string, E, std::ostream&> collector = collector::useCout(stream, delimiter);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::ostream& cout(std::ostream& stream, const std::string& prefix, const std::string& delimiter, const std::string& suffix) const {
            const collector::Collector<std::string, E, std::ostream&> collector = collector::useCout(stream, prefix, delimiter, suffix);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::ostream& cout(std::ostream& stream, const std::string& prefix, const functional::TriFunction<std::string, E, Timestamp, std::string>& serializer, const std::string& suffix) const {
            const collector::Collector<std::string, E, std::ostream&> collector = collector::useCout(stream, prefix, serializer, suffix);
            return collector.collect(*this->generator, this->concurrent);
        }

        template<typename K>
        std::unordered_map<K, std::vector<E>> group(const functional::Function<E, K>& classifier) const {
            const collector::Collector<std::unordered_map<K, std::vector<E>>, E, std::unordered_map<K, std::vector<E>>> collector = collector::useGroup(classifier);
            return collector.collect(*this->generator, this->concurrent);
        }

        template<typename K, typename V>
        std::unordered_map<K, std::vector<V>> groupBy(const functional::Function<E, K>& keyExtractor, const functional::Function<E, V>& valueExtractor) const {
            const collector::Collector<std::unordered_map<K, std::vector<V>>, E, std::unordered_map<K, std::vector<V>>> collector = collector::useGroupBy(keyExtractor, valueExtractor);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::vector<std::vector<E>> partition(const Module& count) const {
            const collector::Collector<std::vector<std::vector<E>>, E, std::vector<std::vector<E>>> collector = collector::usePartition(count);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::vector<std::vector<E>> partitionBy(const functional::Function<E, Module>& delivery) const {
            const collector::Collector<std::vector<std::vector<E>>, E, std::vector<std::vector<E>>> collector = collector::usePartitionBy(delivery);
            return collector.collect(*this->generator, this->concurrent);
        }

        template<typename E1, typename E2>
        std::optional<E> maximum(const functional::Comparator<E, E>& comparator) const {
            const collector::Collector<std::optional<E>, E, std::optional<E>> collector = collector::useMaximum(comparator);
            return collector.collect(*this->generator, this->concurrent);
        }

        template<typename E1, typename E2>
        std::optional<E> minimum(const functional::Comparator<E, E>& comparator) const {
            const collector::Collector<std::optional<E>, E, std::optional<E>> collector = collector::useMinimum(comparator);
            return collector.collect(*this->generator, this->concurrent);
        }

        std::optional<E> findFirst() const {
            const collector::Collector<std::optional<E>, E, std::optional<E>> collector = collector::useFindFirst();
            return collector.collect(*this->generator, this->concurrent);
        }

        std::optional<E> findAny() const {
            const collector::Collector<std::optional<E>, E, std::optional<E>> collector = collector::useFindAny();
            return collector.collect(*this->generator, this->concurrent);
        }

        std::optional<E> findLast() const {
            const collector::Collector<std::optional<E>, E, std::optional<E>> collector = collector::useFindLast();
            return collector.collect(*this->generator, this->concurrent);
        }
    };



    template <typename E>
    class OrderedCollectable : public Collectable<E> {
        using Buffer = std::set<std::pair<Timestamp, E>>;
    protected:
        virtual std::vector<E> source() override {
            const Generator<E> generator = *(this->generator);
            Buffer buffer;
            generator([&buffer] (const E& element, const Timestamp& index)-> void {
                buffer.insert(std::make_pair(index, element));
            }, [] (const E& element, const Timestamp& index)-> bool {
                return false;
                });
            std::vector<E> vector(buffer.size());
            for (const std::pair<E, Timestamp>& pair : buffer) {
                const Timestamp& index = ((pair.first % buffer.size()) + buffer.size()) % buffer.size();
                vector[index] = pair.second;
            }
            return vector;
        }
    public:
        OrderedCollectable(const Generator<E>& generator) : Collectable<E>(generator) { }
        OrderedCollectable(const Generator<E>& generator, const Module& concurrent) : Collectable<E>(generator, concurrent) { }
        OrderedCollectable(const OrderedCollectable<E>& other) : Collectable<E>(other.generator, other.concurrent) { }
        OrderedCollectable(OrderedCollectable<E>&& other) : Collectable<E>(other.generator, other.concurrent) { }

        OrderedCollectable<E>& operator=(const OrderedCollectable<E>& other) {
            if (*this != other) {
                this->generator = std::move(other.generator);
                this->concurrent = other.concurrent;
            }
            return *this;
        }

        OrderedCollectable<E>& operator=(OrderedCollectable<E>&& other) noexcept {
            if (*this != other) {
                this->generator = std::move(other.generator);
                this->concurrent = other.concurrent;
            }
            return *this;
        }
    };

    template <typename E, typename D>
    class Statistics : public OrderedCollectable<E> {
        static_assert(std::is_arithmetic<D>::value, "Statistical operations require arithmetic types.");

    public:
        Statistics(const Generator<E>& generator) : OrderedCollectable<E>(generator) { }
        Statistics(const Generator<E>& generator, const Module& concurrent) : OrderedCollectable<E>(generator, concurrent) { }
        Statistics(const Statistics<E, D>& other) : OrderedCollectable<E>(other.generator, other.concurrent) { }
        Statistics(Statistics<E, D>&& other) noexcept : OrderedCollectable<E>(other.generator, other.concurrent) { }

        Statistics<E, D>& operator=(const Statistics<E, D>& other) {
            if (*this != other) {
                this->generator = std::move(other.generator);
                this->concurrent = other.concurrent;
            }
            return *this;
        }
        Statistics<E, D>& operator=(Statistics<E, D>&& other) noexcept {
            if (*this != other) {
                this->generator = std::move(other.generator);
                this->concurrent = other.concurrent;
            }
            return *this;
        }

        D range(const Function<E, D>& mapper) const {
            const collector::Collector<D, E, D> collector = collector::useRange(mapper);
            return collector.collect(this->generator, this->concurrent);
        }
        D variance(const Function<E, D>& mapper) const {
            const collector::Collector<D, E, D> collector = collector::useVariance(mapper);
            return collector.collect(this->generator, this->concurrent);
        }
        D standardDeviation(const Function<E, D>& mapper) const {
            const collector::Collector<D, E, D> collector = collector::useStandardDeviation(mapper);
            return collector.collect(this->generator, this->concurrent);
        }
        D mean(const Function<E, D>& mapper) const {
            const collector::Collector<D, E, D> collector = collector::useMean(mapper);
            return collector.collect(this->generator, this->concurrent);
        }
        D median(const Function<E, D>& mapper) const {
            const collector::Collector<D, E, D> collector = collector::useMedian(mapper);
            return collector.collect(this->generator, this->concurrent);
        }
        D mode(const Function<E, D>& mapper) const {
            const collector::Collector<D, E, D> collector = collector::useMode(mapper);
            return collector.collect(this->generator, this->concurrent);
        }
        std::map<D, Module> frequency(const Function<E, D>& mapper) const {

        }
        D summate(const Function<E, D>& mapper) const {
            const collector::Collector<D, E, D> collector = collector::useSummate(mapper);
            return collector.collect(this->generator, this->concurrent);
        }
        std::vector<D> quartiles(const Function<E, D>& mapper) const {
            const collector::Collector<D, E, D> collector = collector::useQuartiles(mapper);
            return collector.collect(this->generator, this->concurrent);
        }
        D interquartileRange(const Function<E, D>& mapper) const {
            const collector::Collector<D, E, D> collector = collector::useInterquartileRange(mapper);
            return collector.collect(this->generator, this->concurrent);
        }
        D skewness(const Function<E, D>& mapper) const {
            const collector::Collector<D, E, D> collector = collector::useSkewness(mapper);
            return collector.collect(this->generator, this->concurrent);
        }
        D kurtosis(const Function<E, D>& mapper) const {
            const collector::Collector<D, E, D> collector = collector::useKurtosis(mapper);
            return collector.collect(this->generator, this->concurrent);
        }
    };


    template <typename E>
    class WindowCollectable : public OrderedCollectable<E> {
    protected:
        using Container = std::set<std::pair<Timestamp, E>>;
        using Window = std::vector<std::pair<Timestamp, E>>;
        using WindowGroup = std::vector<Window>;

    public:

        WindowCollectable(const Generator<E>& generator) : OrderedCollectable<E>(generator) { }

        WindowCollectable(const Generator<E>& generator, const Module& concurrent) : OrderedCollectable<E>(generator, concurrent) { }

        WindowCollectable(const WindowCollectable& other) : OrderedCollectable<E>(other.generator, other.concurrent) { }

        WindowCollectable(WindowCollectable&& other) noexcept : OrderedCollectable<E>(other.generator, other.concurrent) { }

        WindowCollectable<E>& operator=(const WindowCollectable<E>& other) {
            if (*this != other) {
                this->generator = std::move(other.generator);
                this->concurrent = other.concurrent;
            }
            return *this;
        }
        WindowCollectable<E>& operator=(WindowCollectable<E>&& other) noexcept {
            if (*this != other) {
                this->generator = std::move(other.generator);
                this->concurrent = other.concurrent;
            }
            return *this;
        }

        semantic::Semantic<semantic::Semantic<E>> slide(const Module& size, const Module& step) const {

        }

        semantic::Semantic<semantic::Semantic<E>> tumble(const Module& size) const {

        }
    };

    template <typename E>
    class UnorderedCollectable : public Collectable<E> { };
};

namespace semantic {
    using Timestamp = functional::Timestamp;
    using Module = functional::Module;

    template <typename T>
    using Generator = functional::Generator<T>;

    template<typename R>
    using Supplier = functional::Supplier<R>;

    template<typename T>
    using Consumer = functional::Consumer<T>;

    template<typename T, typename U>
    using BiConsumer = functional::BiConsumer<T, U>;

    template<typename T, typename U>
    using BiPredicate = functional::BiPredicate<T, U>;

    template<typename T>
    using Predicate = functional::Predicate<T>;

    template<typename T, typename U>
    using Comparator = functional::Comparator<T, U>;

    template<typename T, typename R>
    using Function = functional::Function<T, R>;

    template<typename T, typename U, typename R>
    using BiFunction = functional::BiFunction<T, U, R>;

    template <typename E>
    class Semantic {
    protected:
        const Module concurrent;
        const std::unique_ptr<Generator<E>> generator;
    public:
        Semantic(const Generator<E>& gernerator) : generator(std::make_unique<Generator<E>>(std::move(generator))), concurrent(1) { }

        Semantic(const Generator<E>& gernerator, const Module& concurrent) : generator(std::make_unique<Generator<E>>(std::move(generator))), concurrent(concurrent) { }

        template<typename E>
        Semantic<E> concat(const Semantic<E>& other) const {
            return Semantic<E>([this, other] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt) -> void {
                Module count = 0;
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index);
                    count++;
                    }, interrupt);

                (*other.generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index + count);
                    }, interrupt);
                }, std::max(this->concurrent, other.concurrent));
        }

        Semantic<E> concat(const Generator<E>& generator) const {
            return Semantic<E>([this, generator] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                Module count = 0;
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index);
                    count++;
                }, interrupt);
                generator([&] (const E& element, Timestamp index)->void {
                    accept(element, index + count);
                }, interrupt);
            }, this->concurrent);
        }

        Semantic<E> concat(const std::vector<E>& container) const {
            return Semantic<E>([this, container] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                Module count = 0;
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index);
                    count++;
                }, interrupt);
                for (const E& element : container) {
                    if (interrupt(element, count)) {
                        break;
                    }
                    (*this->generator)([&] (const E& element, Timestamp index)->void {
                        accept(element, count);
                        count++;
                    }, interrupt);
                }
            }, this->concurrent);
        }

        Semantic<E> concat(const std::set<E>& container) const {
            return Semantic<E>([this, container] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                Module count = 0;
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index);
                    count++;
                    }, interrupt);
                for (const E& element : container) {
                    if (interrupt(element, count)) {
                        break;
                    }
                    (*this->generator)([&] (const E& element, Timestamp index)->void {
                        accept(element, count);
                        count++;
                    }, interrupt);
                }
            }, this->concurrent);
        }

        Semantic<E> concat(const std::unordered_set<E>& container) const {
            return Semantic<E>([this, container] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                Module count = 0;
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index);
                    count++;
                    }, interrupt);
                for (const E& element : container) {
                    if (interrupt(element, count)) {
                        break;
                    }
                    (*this->generator)([&] (const E& element, Timestamp index)->void {
                        accept(element, count);
                        count++;
                    }, interrupt);
                }
            }, this->concurrent);
        }

        Semantic<E> concat(const std::list<E>& container) const {
            return Semantic<E>([this, container] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                Module count = 0;
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index);
                    count++;
                    }, interrupt);
                for (const E& element : container) {
                    if (interrupt(element, count)) {
                        break;
                    }
                    (*this->generator)([&] (const E& element, Timestamp index)->void {
                        accept(element, count);
                        count++;
                    }, interrupt);
                }
            }, this->concurrent);
        }

        Semantic<E> concat(const std::initializer_list<E>& container) const {
            return Semantic<E>([this, container] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                Module count = 0;
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index);
                    count++;
                    }, interrupt);
                for (const E& element : container) {
                    if (interrupt(element, count)) {
                        break;
                    }
                    (*this->generator)([&] (const E& element, Timestamp index)->void {
                        accept(element, count);
                        count++;
                    }, interrupt);
                }
            }, this->concurrent);
        }

        Semantic<E> concat(const std::deque<E>& container) const {
            return Semantic<E>([this, container] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                Module count = 0;
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index);
                    count++;
                    }, interrupt);
                for (const E& element : container) {
                    if (interrupt(element, count)) {
                        break;
                    }
                    (*this->generator)([&] (const E& element, Timestamp index)->void {
                        accept(element, count);
                        count++;
                    }, interrupt);
                }
            }, this->concurrent);
        }

        Semantic<E> distinct() const {
            return Semantic<E>([this] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                std::unordered_set<E> seen;
                (*this->generator)([&] (const E& element, const Timestamp& index)->void {
                    if (seen.insert(element).second) {
                        accept(element, index);
                    }
                }, interrupt);
            }, this->concurrent);
        }

        Semantic<E> distinct(const Comparator<E, E>& identifier) const {
            return Semantic<E>([this, identifier] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                std::set<Timestamp> seen;
                (*this->generator)([&] (const E& element, const Timestamp& index)->void {
                    Timestamp id = identifier(element, element);
                    if (seen.insert(id).second) {
                        accept(element, index);
                    }
                    }, interrupt);
                }, this->concurrent);
        }


        Semantic<E> dropWhile(const Predicate<E>& p) const {
            return Semantic<E>([this, p] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                bool dropping = true;
                (*this->generator)([&] (const E& element, const Timestamp& index)->void {
                    if (dropping) {
                        if (!p(element)) {
                            dropping = false;
                            accept(element, index);
                        }
                    } else {
                        accept(element, index);
                    }
                    }, interrupt);
                }, this->concurrent);
        }

        Semantic<E> filter(const Predicate<E>& p) const {
            return Semantic<E>([this, p] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    if (p(element)) {
                        accept(element, index);
                    }
                    }, interrupt);
                }, this->concurrent);
        }

        Semantic<E> flat(const Function<E, Semantic<E>>& mapper) const {
            return Semantic<E>([this, mapper] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                Module current = 0;
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    Semantic<E> inner = mapper(element);
                    (*inner.generator)([&] (const E& innerElement, Timestamp innerIndex)->void {
                        accept(innerElement, current);
                        current++;
                        }, interrupt);
                    }, interrupt);
                }, this->concurrent);
        }

        template <typename R>
        Semantic<R> flatMap(const Function<E, Semantic<R>>& mapper) const {
            return Semantic<R>([this, mapper] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                Module current = 0;
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    Semantic<R> inner = mapper(element);
                    (*inner.generator)([&] (const R& innerElement, Timestamp innerIndex)->void {
                        accept(innerElement, current);
                        current++;
                        }, interrupt);
                    }, interrupt);
                }, this->concurrent);
        }

        Semantic<E> limit(const Module& n) const {
            return Semantic<E>([this, n] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                Module count = 0;
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index);
                    count++;
                    }, [&] (const E& element)->bool {
                        return count >= n || interrupt(element);
                        });
                }, this->concurrent);
        }

        template <typename R>
        Semantic<R> map(const Function<E, R>& mapper) const {
            return Semantic<R>([this, mapper] (const BiConsumer<R, Timestamp>& accept, const BiPredicate<R, Timestamp>& interrupt)->void {
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(mapper(element), index);
                    }, interrupt);
                }, this->concurrent);
        }

        Semantic<E> parallel() const {
            return Semantic<E>(this->generator, std::thread::hardware_concurrency());
        }

        Semantic<E> parallel(const Module& threadCount) const {
            return Semantic<E>(this->generator, threadCount);
        }

        Semantic<E> peek(const Consumer<E>& c) const {
            return Semantic<E>([this, c] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    c(element);
                    accept(element, index);
                    }, interrupt);
                }, this->concurrent);
        }

        Semantic<E> redirect(const BiFunction<E, Timestamp, Timestamp>& redirector) const {
            return Semantic<E>([this, redirector] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, redirector(element, index));
                    }, interrupt);
                }, this->concurrent);
        }

        Semantic<E> reverse() const {
            return Semantic<E>([this] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                (*this->generator)([&] (const E& element, const Timestamp& index)->void {
                    accept(element, -index);
                    }, interrupt);
                }, this->concurrent);
        }

        Semantic<E> shuffle() const {
            return Semantic<E>([this] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, std::hash<E>{}(element) ^ std::hash<Timestamp>{}(index));
                    }, interrupt);
                }, this->concurrent);
        }

        Semantic<E> shuffle(const Function<E, Timestamp>& mapper) const {
            return Semantic<E>([this, &mapper] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, mapper(element));
                    }, interrupt);
                }, this->concurrent);
        }

        Semantic<E> skip(const Module& n) const {
            return Semantic<E>([this, n] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                Module skipped = 0;
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    if (skipped < n) {
                        skipped++;
                    } else {
                        accept(element, index - n);
                    }
                    }, interrupt);
                }, this->concurrent);
        }

        collectable::OrderedCollectable<E> sorted() const {
            return collectable::OrderedCollectable<E>([this] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                using Container = std::vector<std::pair<Timestamp, E>>;
                const Function<Container, Container>& arrange = [] (const Container& container) -> Container {
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
                (*this->generator)([&] (const E& element, Timestamp index)->void {
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

        collectable::OrderedCollectable<E> sorted(const Comparator<E, E>& indexer) const {
            return collectable::OrderedCollectable<E>([this, &indexer] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                using Container = std::vector<std::pair<Timestamp, E>>;
                const Function<Container, Container>& arrange = [] (const Container& container) -> Container {
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
                (*this->generator)([&elements] (const E& element, Timestamp index)->void {
                    elements.emplace_back(index, element);
                    }, interrupt);
                Container arranged = arrange(elements);
                std::sort(arranged.begin(), arranged.end(),
                    [&indexer] (const std::pair<Timestamp, E>& a, const std::pair<Timestamp, E>& b) {
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

        Semantic<E> sub(const Module& start, const Module& end) const {
            return Semantic<E>([this, start, end] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                Module index = 0;
                (*this->generator)([&] (const E& element, Timestamp)->void {
                    if (index >= start) {
                        accept(element, index - start);
                    }
                    index++;
                    }, [&] (const E& element)-> bool {
                        return index >= end || interrupt(element);
                        });
                }, this->concurrent);
        }

        Semantic<E> takeWhile(const Predicate<E>& p) const {
            return Semantic<E>([this, p] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                (*this->generator)([&] (const E& element, const Timestamp& index)->void {
                    if (p(element)) {
                        accept(element, index);
                    }
                    }, [&] (const E& element)->bool {
                        return !p(element) || interrupt(element);
                        });
                }, this->concurrent);
        }

        collectable::OrderedCollectable<E> toOrdered() const {
            return collectable::OrderedCollectable<E>([this] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index);
                    }, interrupt);
                }, this->concurrent);
        }

        collectable::Statistics<E, E> toStatistics() const {
            return collectable::Statistics<E, E>(this->generator, this->concurrent);
        }

        template <typename D>
        collectable::Statistics<E, D> toStatistics() const {
            return collectable::Statistics<E, D>(this->generator, this->concurrent);
        }

        collectable::UnorderedCollectable<E> toUnordered() const {
            return collectable::UnorderedCollectable<E>(this->generator, this->concurrent);
        }

        collectable::WindowCollectable<E> toWindow() const {
            return collectable::WindowCollectable<E>(this->generator, this->concurrent);
        }

        Semantic<E> translate(const Timestamp& offset) const {
            return Semantic<E>([this, &offset] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index + offset);
                    }, interrupt);
                }, this->concurrent);
        }

        Semantic<E> translate(const Function<E, Timestamp>& translator) const {
            return Semantic<E>([this, &translator] (const BiConsumer<E, Timestamp>& accept, const BiPredicate<E, Timestamp>& interrupt)->void {
                (*this->generator)([&] (const E& element, Timestamp index)->void {
                    accept(element, index + translator(element));
                    }, interrupt);
                }, this->concurrent);
        }
    };



    Semantic<char> bytes(std::istream& stream) {
        return Semantic<char>([&stream] (const BiConsumer<char, Timestamp>& accept, const BiPredicate<char, Timestamp>& interrupt)->void {
            char c;
            Timestamp index = 0;
            while (stream.get(c)) {
                if (interrupt(c, index)) {
                    break;
                }
                accept(c, index++);
            }
            });
    }

    Semantic<std::vector<char>> chunks(std::istream& stream, const Module& size) {
        return Semantic<std::vector<char>>([&stream, size] (const BiConsumer<std::vector<char>, Timestamp>& accept, const BiPredicate<std::vector<char>, Timestamp>& interrupt)->void {
            Timestamp index = 0;
            std::vector<char> buffer;
            buffer.reserve(static_cast<size_t>(size));

            char c;
            while (stream.get(c)) {
                buffer.push_back(c);
                if (buffer.size() == static_cast<size_t>(size)) {
                    if (interrupt(buffer, index)) {
                        break;
                    }
                    accept(buffer, index++);
                    buffer.clear();
                }
            }

            if (!buffer.empty()) {
                if (interrupt(buffer, index)) {
                    accept(buffer, index);
                }
            }
        });
    }

    template <typename E>
    Semantic<E> empty() {
        return Semantic<E>([] (const BiConsumer<E, Timestamp>& accept, const Predicate<E>& interrupt) -> void {

        });
    }

    template <typename E, typename... Args>
    Semantic<E> of(Args &&... args) {
        std::vector<E> elements = { std::forward<Args>(args)... };
        return Semantic<E>([elements = std::move(elements)] (const BiConsumer<char, Timestamp>& accept, const BiPredicate<char, Timestamp>& interrupt)->void {
            for (Timestamp index = 0; index < elements.size(); ++index) {
                if (predicate(elements[index], index)) {
                    break;
                }
                accept(elements[index], index);
            }
        });
    }

    template <typename E>
    Semantic<E> fill(const E& element, const Module& count) {
        return Semantic<E>([element, count] (const BiConsumer<char, Timestamp>& accept, const BiPredicate<char, Timestamp>& interrupt)->void {
            for (Timestamp index = 0; index < count; ++index) {
                if (predicate(element, index)) {
                    break;
                }
                accept(element, index);
            }
        });
    }

    template <typename E>
    Semantic<E> fill(const Supplier<E>& supplier, const Module& count) {
        return Semantic<E>([supplier, count] (const BiConsumer<char, Timestamp>& accept, const BiPredicate<char, Timestamp>& interrupt)->void {
            for (Timestamp index = 0; index < count; ++index) {
                E element = supplier();
                if (interrupt(element, index)) {
                    break;
                }
                accept(element, index);
            }
      });
    }

    template<typename E>
    Semantic<E> from(std::vector<E> container) {
        return Semantic<E>([container] (const BiConsumer<char, Timestamp>& accept, const BiPredicate<char, Timestamp>& interrupt)->void {
            Module index = 0;
            for (const E& element : container) {
                if (interrupt(element, index)) {
                    break;
                }
                accept(element, index);
            }
        });
    }

    template<typename E>
    Semantic<E> from(std::set<E> container) {
        return Semantic<E>([container] (const BiConsumer<char, Timestamp>& accept, const BiPredicate<char, Timestamp>& interrupt)->void {
            Module index = 0;
            for (const E& element : container) {
                if (interrupt(element, index)) {
                    break;
                }
                accept(element, index);
            }
        });
    }

    template<typename E>
    Semantic<E> from(std::unordered_set<E> container) {
        return Semantic<E>([container] (const BiConsumer<char, Timestamp>& accept, const BiPredicate<char, Timestamp>& interrupt)->void {
            Module index = 0;
            for (const E& element : container) {
                if (interrupt(element, index)) {
                    break;
                }
                accept(element, index);
            }
        });
    }

    template<typename E>
    Semantic<E> from(std::list<E> container) {
        return Semantic<E>([container] (const BiConsumer<char, Timestamp>& accept, const BiPredicate<char, Timestamp>& interrupt)->void {
            Module index = 0;
            for (const E& element : container) {
                if (interrupt(element, index)) {
                    break;
                }
                accept(element, index);
            }
        });
    }

    template<typename E>
    Semantic<E> from(std::initializer_list<E> container) {
        return Semantic<E>([container] (const BiConsumer<char, Timestamp>& accept, const BiPredicate<char, Timestamp>& interrupt)->void {
            Module index = 0;
            for (const E& element : container) {
                if (interrupt(element, index)) {
                    break;
                }
                accept(element, index);
            }
        });
    }

    template<typename E>
    Semantic<E> from(std::deque<E> container) {
        return Semantic<E>([container] (const BiConsumer<char, Timestamp>& accept, const BiPredicate<char, Timestamp>& interrupt)->void {
            Module index = 0;
            for (const E& element : container) {
                if (interrupt(element, index)) {
                    break;
                }
                accept(element, index);
            }
        });
    }

    template <typename E>
    Semantic<E> iterate(const Generator<E>& generator) {
        return Semantic<E>(generator);
    }

    Semantic<std::vector<char>> lines(std::istream& stream) {
        return Semantic<std::vector<char>>([&stream] (const BiConsumer<std::vector<char>, Timestamp>& accept, const BiPredicate<std::vector<char>, Timestamp>& interrupt)->void {
            Timestamp index = 0;
            std::string line;

            while (std::getline(stream, line)) {
                std::vector<char> chars(line.begin(), line.end());
                if (interrupt(chars, index)) {
                    break;
                }
                accept(chars, index);
                index++;
            }
        });
    }

    template <typename E>
    Semantic<E> range(const E& start, const E& end) {
        return Semantic<E>([start, end] (const BiConsumer<char, Timestamp>& accept, const BiPredicate<char, Timestamp>& interrupt)->void {
            E current = start;
            Timestamp index = 0;
            while (current < end) {
                if (interrupt(current, index)) {
                    break;
                }
                accept(current, index);
                index++;
                ++current;
            }
        });
    }

    template <typename E>
    Semantic<E> range(const E& start, const E& end, const E& step) {
        return Semantic<E>([start, end, step] (const BiConsumer<char, Timestamp>& accept, const BiPredicate<char, Timestamp>& interrupt)->void {
            E current = start;
            Timestamp index = 0;
            while (current < end) {
                if (interrupt(current)) {
                    break;
                }
                accept(current, index);
                index++;
                current += step;
            }
        });
    }

    Semantic<std::vector<char>> split(std::istream& stream, const char& delimeter) {
        return Semantic<std::vector<char>>([&stream, delimeter] (const BiConsumer<std::vector<char>, Timestamp>& accept, const BiPredicate<std::vector<char>, Timestamp>& interrupt)->void {
            Timestamp index = 0;
            std::vector<char> buffer;
            char c;

            while (stream.get(c)) {
                if (c == delimeter) {
                    if (!interrupt(buffer, index)) {
                        accept(buffer, index);
                        index++;
                    }
                    buffer.clear();
                } else {
                    buffer.push_back(c);
                }
            }

            if (!buffer.empty() && !interrupt(buffer, index)) {
                accept(buffer, index);
            }
        });
    }

    Semantic<std::string> text(std::istream& stream) {
        return Semantic<std::string>([&stream] (const BiConsumer<std::string, Timestamp>& accept, const BiPredicate<std::string, Timestamp>& interrupt)->void {
            Timestamp index = 0;
            std::string line;

            while (std::getline(stream, line)) {
                if (interrupt(line, index)) {
                    break;
                }
                accept(line, index++);
            }
        });
    }
};
