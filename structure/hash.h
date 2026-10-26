#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <complex>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace std
{
#if __cplusplus == 201703L
template <typename T>
struct hash<std::vector<T>>
{
    std::size_t operator()(const std::vector<T> &container) const noexcept
    {
        std::size_t seed = container.size();
        for (const T &element : container)
        {
            seed ^= hash<T>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename T>
struct hash<std::list<T>>
{
    std::size_t operator()(const std::list<T> &container) const noexcept
    {
        std::size_t seed = container.size();
        for (const T &element : container)
        {
            seed ^= hash<T>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename T>
struct hash<std::deque<T>>
{
    std::size_t operator()(const std::deque<T> &container) const noexcept
    {
        std::size_t seed = container.size();
        for (const T &element : container)
        {
            seed ^= hash<T>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename T>
struct hash<std::forward_list<T>>
{
    std::size_t operator()(const std::forward_list<T> &container) const noexcept
    {
        std::size_t seed = 0;
        std::size_t count = 0;
        for (const T &element : container)
        {
            seed ^= hash<T>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            ++count;
        }
        seed ^= count + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

template <typename T>
struct hash<std::set<T>>
{
    std::size_t operator()(const std::set<T> &container) const noexcept
    {
        std::size_t seed = container.size();
        for (const T &element : container)
        {
            seed ^= hash<T>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename T>
struct hash<std::unordered_set<T>>
{
    std::size_t operator()(const std::unordered_set<T> &container) const noexcept
    {
        std::vector<T> sorted(container.begin(), container.end());
        std::sort(sorted.begin(), sorted.end());
        std::size_t seed = sorted.size();
        for (const T &element : sorted)
        {
            seed ^= hash<T>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename T>
struct hash<std::multiset<T>>
{
    std::size_t operator()(const std::multiset<T> &container) const noexcept
    {
        std::size_t seed = container.size();
        for (const T &element : container)
        {
            seed ^= hash<T>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename T>
struct hash<std::unordered_multiset<T>>
{
    std::size_t operator()(const std::unordered_multiset<T> &container) const noexcept
    {
        std::vector<T> sorted(container.begin(), container.end());
        std::sort(sorted.begin(), sorted.end());
        std::size_t seed = sorted.size();
        for (const T &element : sorted)
        {
            seed ^= hash<T>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename K, typename V>
struct hash<std::map<K, V>>
{
    std::size_t operator()(const std::map<K, V> &container) const noexcept
    {
        std::size_t seed = container.size();
        for (const auto &pair : container)
        {
            std::size_t elementHash = hash<K>{}(pair.first);
            elementHash ^= hash<V>{}(pair.second) + 0x9e3779b9 + (elementHash << 6) + (elementHash >> 2);
            seed ^= elementHash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename K, typename V>
struct hash<std::unordered_map<K, V>>
{
    std::size_t operator()(const std::unordered_map<K, V> &container) const noexcept
    {
        std::vector<std::pair<K, V>> sorted(container.begin(), container.end());
        std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) {
            if (a.first != b.first)
                return a.first < b.first;
            return a.second < b.second;
        });
        std::size_t seed = sorted.size();
        for (const auto &pair : sorted)
        {
            std::size_t elementHash = hash<K>{}(pair.first);
            elementHash ^= hash<V>{}(pair.second) + 0x9e3779b9 + (elementHash << 6) + (elementHash >> 2);
            seed ^= elementHash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename K, typename V>
struct hash<std::multimap<K, V>>
{
    std::size_t operator()(const std::multimap<K, V> &container) const noexcept
    {
        std::size_t seed = container.size();
        for (const auto &pair : container)
        {
            std::size_t elementHash = hash<K>{}(pair.first);
            elementHash ^= hash<V>{}(pair.second) + 0x9e3779b9 + (elementHash << 6) + (elementHash >> 2);
            seed ^= elementHash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename K, typename V>
struct hash<std::unordered_multimap<K, V>>
{
    std::size_t operator()(const std::unordered_multimap<K, V> &container) const noexcept
    {
        std::vector<std::pair<K, V>> sorted(container.begin(), container.end());
        std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) {
            if (a.first != b.first)
                return a.first < b.first;
            return a.second < b.second;
        });
        std::size_t seed = sorted.size();
        for (const auto &pair : sorted)
        {
            std::size_t elementHash = hash<K>{}(pair.first);
            elementHash ^= hash<V>{}(pair.second) + 0x9e3779b9 + (elementHash << 6) + (elementHash >> 2);
            seed ^= elementHash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename T>
struct hash<std::queue<T>>
{
    std::size_t operator()(const std::queue<T> &container) const noexcept
    {
        std::queue<T> copy = container;
        std::size_t seed = 0;
        std::size_t count = 0;
        while (!copy.empty())
        {
            seed ^= hash<T>{}(copy.front()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            copy.pop();
            ++count;
        }
        seed ^= count + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

template <typename T>
struct hash<std::stack<T>>
{
    std::size_t operator()(const std::stack<T> &container) const noexcept
    {
        std::stack<T> copy = container;
        std::vector<T> elements;
        while (!copy.empty())
        {
            elements.push_back(copy.top());
            copy.pop();
        }
        std::size_t seed = elements.size();
        for (const T &element : elements)
        {
            seed ^= hash<T>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename T>
struct hash<std::priority_queue<T>>
{
    std::size_t operator()(const std::priority_queue<T> &container) const noexcept
    {
        std::priority_queue<T> copy = container;
        std::vector<T> elements;
        while (!copy.empty())
        {
            elements.push_back(copy.top());
            copy.pop();
        }
        std::sort(elements.begin(), elements.end());
        std::size_t seed = elements.size();
        for (const T &element : elements)
        {
            seed ^= hash<T>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename T, std::size_t N>
struct hash<std::array<T, N>>
{
    std::size_t operator()(const std::array<T, N> &container) const noexcept
    {
        std::size_t seed = N;
        for (const T &element : container)
        {
            seed ^= hash<T>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

template <typename T1, typename T2>
struct hash<std::pair<T1, T2>>
{
    std::size_t operator()(const std::pair<T1, T2> &container) const noexcept
    {
        std::size_t seed = hash<T1>{}(container.first);
        seed ^= hash<T2>{}(container.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

template <typename... Ts>
struct hash<std::tuple<Ts...>>
{
    std::size_t operator()(const std::tuple<Ts...> &container) const noexcept
    {
        return hashImpl(container, std::index_sequence_for<Ts...>{});
    }

  private:
    template <std::size_t... Is>
    std::size_t hashImpl(const std::tuple<Ts...> &container, std::index_sequence<Is...>) const noexcept
    {
        std::size_t seed = sizeof...(Ts);
        auto hasher = [&seed](const auto &element) {
            using ElementType = std::decay_t<decltype(element)>;
            seed ^= hash<ElementType>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };
        (hasher(std::get<Is>(container)), ...);
        return seed;
    }
};

template <typename Rep, typename Period>
struct hash<std::chrono::duration<Rep, Period>>
{
    std::size_t operator()(const std::chrono::duration<Rep, Period> &container) const noexcept
    {
        return hash<Rep>{}(container.count());
    }
};

template <typename Clock, typename Duration>
struct hash<std::chrono::time_point<Clock, Duration>>
{
    std::size_t operator()(const std::chrono::time_point<Clock, Duration> &container) const noexcept
    {
        return hash<Duration>{}(container.time_since_epoch());
    }
};

template <typename T>
struct hash<std::complex<T>>
{
    std::size_t operator()(const std::complex<T> &container) const noexcept
    {
        std::size_t seed = hash<T>{}(container.real());
        seed ^= hash<T>{}(container.imag()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

template <typename T>
struct hash<std::unique_ptr<T>>
{
    std::size_t operator()(const std::unique_ptr<T> &container) const noexcept
    {
        if (!container)
        {
            return 0;
        }
        return hash<const T *>{}(container.get());
    }
};
#endif
} // namespace std