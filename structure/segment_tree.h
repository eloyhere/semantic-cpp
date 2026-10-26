#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace segment
{

// A segment tree over a fixed-size sequence. Values are combined with an
// associative Operation; identity is the neutral element of that operation
// (for example 0 for addition or the maximum value for minimum).
//
// The tree supports point assignment and range queries on the half-open
// interval [left, right). It is stored in the usual iterative layout, where
// leaves occupy [size, 2 * size) and each internal node combines its children.
template <typename T, typename Operation = std::plus<T>>
class SegmentTree
{
  public:
    using value_type = T;
    using operation_type = Operation;
    using size_type = std::size_t;

    SegmentTree() = default;

    explicit SegmentTree(size_type count, T identity = T{}, Operation operation = Operation{})
        : operationValue(std::move(operation)),
          identityValue(std::move(identity)),
          treeValue(2 * count, identityValue),
          countValue(count)
    {
        build();
    }

    template <typename InputIt, typename = std::enable_if_t<!std::is_integral<InputIt>::value>>
    SegmentTree(InputIt first, InputIt last, T identity = T{}, Operation operation = Operation{})
        : operationValue(std::move(operation)), identityValue(std::move(identity)), treeValue(), countValue(0)
    {
        std::vector<T> leaves(first, last);
        countValue = leaves.size();
        treeValue.assign(2 * countValue, identityValue);
        std::move(leaves.begin(), leaves.end(),
                  treeValue.begin() + static_cast<std::ptrdiff_t>(countValue));
        build();
    }

    SegmentTree(std::initializer_list<T> values, T identity = T{}, Operation operation = Operation{})
        : SegmentTree(values.begin(), values.end(), std::move(identity), std::move(operation))
    {
    }

    SegmentTree(const SegmentTree &) = default;
    SegmentTree(SegmentTree &&) = default;
    SegmentTree &operator=(const SegmentTree &) = default;
    SegmentTree &operator=(SegmentTree &&) = default;
    ~SegmentTree() = default;

    // Assigns value to the element at index.
    void set(size_type index, const T &value) { setImpl(index, value); }
    void set(size_type index, T &&value) { setImpl(index, std::move(value)); }

    // Replaces the element at index with function(currentValue).
    template <typename Function>
    void update(size_type index, Function &&function)
    {
        checkIndex(index);
        set(index, function(treeValue[countValue + index]));
    }

    const T &at(size_type index) const
    {
        checkIndex(index);
        return treeValue[countValue + index];
    }

    // Folds the half-open interval [left, right). An empty interval yields
    // identity.
    T query(size_type left, size_type right) const
    {
        if (left > right || right > countValue)
        {
            throw std::out_of_range("segment::SegmentTree::query: range out of bounds");
        }

        T leftResult = identityValue;
        T rightResult = identityValue;

        left += countValue;
        right += countValue;
        while (left < right)
        {
            if (left & 1U)
                leftResult = operationValue(leftResult, treeValue[left++]);
            if (right & 1U)
                rightResult = operationValue(treeValue[--right], rightResult);
            left /= 2;
            right /= 2;
        }

        return operationValue(leftResult, rightResult);
    }

    // Folds the whole sequence. An empty tree yields identity.
    T query() const { return countValue == 0 ? identityValue : treeValue[1]; }

    void clear() noexcept
    {
        treeValue.clear();
        countValue = 0;
    }

    size_type size() const noexcept { return countValue; }
    bool empty() const noexcept { return countValue == 0; }

    const T &identity() const noexcept { return identityValue; }
    const Operation &operation() const noexcept { return operationValue; }

    void swap(SegmentTree &other)
    {
        using std::swap;
        swap(operationValue, other.operationValue);
        swap(identityValue, other.identityValue);
        swap(treeValue, other.treeValue);
        swap(countValue, other.countValue);
    }

  private:
    void build()
    {
        if (countValue < 2)
            return;

        for (size_type index = countValue - 1; index > 0; --index)
        {
            treeValue[index] = operationValue(treeValue[index * 2], treeValue[index * 2 + 1]);
        }
    }

    template <typename U>
    void setImpl(size_type index, U &&value)
    {
        checkIndex(index);

        size_type position = countValue + index;
        treeValue[position] = std::forward<U>(value);
        for (position /= 2; position > 0; position /= 2)
        {
            treeValue[position] = operationValue(treeValue[position * 2], treeValue[position * 2 + 1]);
        }
    }

    void checkIndex(size_type index) const
    {
        if (index >= countValue)
        {
            throw std::out_of_range("segment::SegmentTree: index out of bounds");
        }
    }

    Operation operationValue{};
    T identityValue{};
    std::vector<T> treeValue{};
    size_type countValue{};
};

} // namespace segment
