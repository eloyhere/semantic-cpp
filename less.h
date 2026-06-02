#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <complex>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

namespace std
{
template <typename T>
bool operator<(const std::unordered_set<T> &left, const std::unordered_set<T> &right)
{
    std::vector<T> leftSorted(left.begin(), left.end());
    std::vector<T> rightSorted(right.begin(), right.end());
    std::sort(leftSorted.begin(), leftSorted.end());
    std::sort(rightSorted.begin(), rightSorted.end());
    return std::lexicographical_compare(
        leftSorted.begin(), leftSorted.end(),
        rightSorted.begin(), rightSorted.end());
}

template <typename T>
bool operator<(const std::unordered_multiset<T> &left, const std::unordered_multiset<T> &right)
{
    std::vector<T> leftSorted(left.begin(), left.end());
    std::vector<T> rightSorted(right.begin(), right.end());
    std::sort(leftSorted.begin(), leftSorted.end());
    std::sort(rightSorted.begin(), rightSorted.end());
    return std::lexicographical_compare(
        leftSorted.begin(), leftSorted.end(),
        rightSorted.begin(), rightSorted.end());
}

template <typename K, typename V>
bool operator<(const std::unordered_map<K, V> &left, const std::unordered_map<K, V> &right)
{
    std::vector<std::pair<K, V>> leftSorted(left.begin(), left.end());
    std::vector<std::pair<K, V>> rightSorted(right.begin(), right.end());
    auto pairLess = [](const auto &a, const auto &b) {
        if (a.first != b.first)
            return a.first < b.first;
        return a.second < b.second;
    };
    std::sort(leftSorted.begin(), leftSorted.end(), pairLess);
    std::sort(rightSorted.begin(), rightSorted.end(), pairLess);
    return std::lexicographical_compare(
        leftSorted.begin(), leftSorted.end(),
        rightSorted.begin(), rightSorted.end(), pairLess);
}

template <typename K, typename V>
bool operator<(const std::unordered_multimap<K, V> &left, const std::unordered_multimap<K, V> &right)
{
    std::vector<std::pair<K, V>> leftSorted(left.begin(), left.end());
    std::vector<std::pair<K, V>> rightSorted(right.begin(), right.end());
    auto pairLess = [](const auto &a, const auto &b) {
        if (a.first != b.first)
            return a.first < b.first;
        return a.second < b.second;
    };
    std::sort(leftSorted.begin(), leftSorted.end(), pairLess);
    std::sort(rightSorted.begin(), rightSorted.end(), pairLess);
    return std::lexicographical_compare(
        leftSorted.begin(), leftSorted.end(),
        rightSorted.begin(), rightSorted.end(), pairLess);
}

template <typename T>
struct less<std::forward_list<T>>
{
    bool operator()(const std::forward_list<T> &left, const std::forward_list<T> &right) const noexcept
    {
        return std::lexicographical_compare(
            left.begin(), left.end(),
            right.begin(), right.end());
    }
};

template <typename T>
struct less<std::queue<T>>
{
    bool operator()(const std::queue<T> &left, const std::queue<T> &right) const noexcept
    {
        std::queue<T> leftCopy = left;
        std::queue<T> rightCopy = right;
        while (!leftCopy.empty() && !rightCopy.empty())
        {
            if (leftCopy.front() < rightCopy.front())
                return true;
            if (rightCopy.front() < leftCopy.front())
                return false;
            leftCopy.pop();
            rightCopy.pop();
        }
        return leftCopy.empty() && !rightCopy.empty();
    }
};

template <typename T>
struct less<std::stack<T>>
{
    bool operator()(const std::stack<T> &left, const std::stack<T> &right) const noexcept
    {
        std::stack<T> leftCopy = left;
        std::stack<T> rightCopy = right;
        std::vector<T> leftElements;
        std::vector<T> rightElements;
        while (!leftCopy.empty())
        {
            leftElements.push_back(leftCopy.top());
            leftCopy.pop();
        }
        while (!rightCopy.empty())
        {
            rightElements.push_back(rightCopy.top());
            rightCopy.pop();
        }
        return std::lexicographical_compare(
            leftElements.begin(), leftElements.end(),
            rightElements.begin(), rightElements.end());
    }
};

template <typename T>
struct less<std::unordered_set<T>>
{
    bool operator()(const std::unordered_set<T> &left, const std::unordered_set<T> &right) const noexcept
    {
        return left < right;
    }
};

template <typename T>
struct less<std::unordered_multiset<T>>
{
    bool operator()(const std::unordered_multiset<T> &left, const std::unordered_multiset<T> &right) const noexcept
    {
        return left < right;
    }
};

template <typename K, typename V>
struct less<std::unordered_map<K, V>>
{
    bool operator()(const std::unordered_map<K, V> &left, const std::unordered_map<K, V> &right) const noexcept
    {
        return left < right;
    }
};

template <typename K, typename V>
struct less<std::unordered_multimap<K, V>>
{
    bool operator()(const std::unordered_multimap<K, V> &left, const std::unordered_multimap<K, V> &right) const noexcept
    {
        return left < right;
    }
};

template <typename T>
struct less<std::priority_queue<T>>
{
    bool operator()(const std::priority_queue<T> &left, const std::priority_queue<T> &right) const noexcept
    {
        std::priority_queue<T> leftCopy = left;
        std::priority_queue<T> rightCopy = right;
        std::vector<T> leftElements;
        std::vector<T> rightElements;
        while (!leftCopy.empty())
        {
            leftElements.push_back(leftCopy.top());
            leftCopy.pop();
        }
        while (!rightCopy.empty())
        {
            rightElements.push_back(rightCopy.top());
            rightCopy.pop();
        }
        std::sort(leftElements.begin(), leftElements.end());
        std::sort(rightElements.begin(), rightElements.end());
        return std::lexicographical_compare(
            leftElements.begin(), leftElements.end(),
            rightElements.begin(), rightElements.end());
    }
};

template <typename T>
struct less<std::optional<T>>
{
    bool operator()(const std::optional<T> &left, const std::optional<T> &right) const noexcept
    {
        if (!left.has_value() && !right.has_value())
            return false;
        if (!left.has_value())
            return true;
        if (!right.has_value())
            return false;
        return left.value() < right.value();
    }
};

template <typename... Ts>
struct less<std::variant<Ts...>>
{
    bool operator()(const std::variant<Ts...> &left, const std::variant<Ts...> &right) const noexcept
    {
        if (left.index() != right.index())
            return left.index() < right.index();
        return std::visit([](const auto &leftValue, const auto &rightValue) -> bool {
            using ValueType = std::decay_t<decltype(leftValue)>;
            return std::less<ValueType>{}(leftValue, rightValue);
        },
                          left, right);
    }
};

template <std::size_t N>
struct less<std::bitset<N>>
{
    bool operator()(const std::bitset<N> &left, const std::bitset<N> &right) const noexcept
    {
        for (std::size_t i = N; i > 0; --i)
        {
            std::size_t index = i - 1;
            if (left[index] != right[index])
            {
                return right[index];
            }
        }
        return false;
    }
};

template <typename Rep, typename Period>
struct less<std::chrono::duration<Rep, Period>>
{
    bool operator()(const std::chrono::duration<Rep, Period> &left, const std::chrono::duration<Rep, Period> &right) const noexcept
    {
        return left.count() < right.count();
    }
};

template <typename Clock, typename Duration>
struct less<std::chrono::time_point<Clock, Duration>>
{
    bool operator()(const std::chrono::time_point<Clock, Duration> &left, const std::chrono::time_point<Clock, Duration> &right) const noexcept
    {
        return left.time_since_epoch() < right.time_since_epoch();
    }
};

template <typename T>
struct less<std::complex<T>>
{
    bool operator()(const std::complex<T> &left, const std::complex<T> &right) const noexcept
    {
        if (left.real() != right.real())
            return left.real() < right.real();
        return left.imag() < right.imag();
    }
};

template <typename T>
struct less<std::unique_ptr<T>>
{
    bool operator()(const std::unique_ptr<T> &left, const std::unique_ptr<T> &right) const noexcept
    {
        if (!left && !right)
            return false;
        if (!left)
            return true;
        if (!right)
            return false;
        return less<const T *>{}(left.get(), right.get());
    }
};

template <>
struct less<std::error_code>
{
    bool operator()(const std::error_code &left, const std::error_code &right) const noexcept
    {
        if (left.value() != right.value())
            return left.value() < right.value();
        return &left.category() < &right.category();
    }
};

template <>
struct less<std::error_condition>
{
    bool operator()(const std::error_condition &left, const std::error_condition &right) const noexcept
    {
        if (left.value() != right.value())
            return left.value() < right.value();
        return &left.category() < &right.category();
    }
};

} // namespace std