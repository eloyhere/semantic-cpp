#pragma once

#include "node.h"

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <queue>
#include <type_traits>
#include <utility>
#include <vector>

namespace tree
{

// An ordered set backed by a binary search tree. Duplicate values are ignored.
// The tree is not self-balancing; insertion order determines its shape.
template <typename T, typename Compare = std::less<T>>
class BinarySearchTree
{
  public:
    using value_type = T;
    using key_type = T;
    using size_type = std::size_t;
    using node_type = node::BinaryNode<T>;
    using compare_type = Compare;

    BinarySearchTree() = default;

    explicit BinarySearchTree(const Compare &compare) : compareValue(compare) {}

    template <typename InputIt, typename = std::enable_if_t<!std::is_integral<InputIt>::value>>
    BinarySearchTree(InputIt first, InputIt last, const Compare &compare = Compare()) : compareValue(compare)
    {
        insertRange(first, last);
    }

    BinarySearchTree(std::initializer_list<T> values, const Compare &compare = Compare()) : compareValue(compare)
    {
        insertRange(values.begin(), values.end());
    }

    BinarySearchTree(const BinarySearchTree &other)
        : rootValue(other.rootValue ? std::make_unique<node_type>(*other.rootValue) : nullptr),
          countValue(other.countValue),
          compareValue(other.compareValue)
    {
    }

    BinarySearchTree(BinarySearchTree &&other) = default;

    BinarySearchTree &operator=(const BinarySearchTree &other)
    {
        if (this != &other)
        {
            BinarySearchTree copy(other);
            swap(copy);
        }
        return *this;
    }

    BinarySearchTree &operator=(BinarySearchTree &&other) = default;
    ~BinarySearchTree() = default;

    // Inserts value unless an equivalent value is already present. Returns
    // true only when a new element was inserted.
    bool insert(const T &value) { return insertImpl(rootValue, value); }
    bool insert(T &&value) { return insertImpl(rootValue, std::move(value)); }

    template <typename... Args>
    bool emplace(Args &&... args)
    {
        return insert(T(std::forward<Args>(args)...));
    }

    bool erase(const T &value) { return eraseImpl(rootValue, value); }

    bool contains(const T &value) const { return find(value) != nullptr; }

    const T *find(const T &value) const
    {
        const node_type *current = rootValue.get();
        while (current)
        {
            if (compareValue(value, current->value))
                current = current->left.get();
            else if (compareValue(current->value, value))
                current = current->right.get();
            else
                return &current->value;
        }
        return nullptr;
    }

    T *find(const T &value)
    {
        node_type *current = rootValue.get();
        while (current)
        {
            if (compareValue(value, current->value))
                current = current->left.get();
            else if (compareValue(current->value, value))
                current = current->right.get();
            else
                return &current->value;
        }
        return nullptr;
    }

    const T *min() const
    {
        const node_type *current = root();
        if (!current)
            return nullptr;
        while (current->left)
            current = current->left.get();
        return &current->value;
    }

    T *min()
    {
        node_type *current = root();
        if (!current)
            return nullptr;
        while (current->left)
            current = current->left.get();
        return &current->value;
    }

    const T *max() const
    {
        const node_type *current = root();
        if (!current)
            return nullptr;
        while (current->right)
            current = current->right.get();
        return &current->value;
    }

    T *max()
    {
        node_type *current = root();
        if (!current)
            return nullptr;
        while (current->right)
            current = current->right.get();
        return &current->value;
    }

    // Visits values in ascending order according to Compare.
    template <typename Visitor>
    void inorder(Visitor &&visitor)
    {
        inorderVisit(root(), visitor);
    }

    template <typename Visitor>
    void inorder(Visitor &&visitor) const
    {
        inorderVisit(root(), visitor);
    }

    template <typename Visitor>
    void preorder(Visitor &&visitor)
    {
        preorderVisit(root(), visitor);
    }

    template <typename Visitor>
    void preorder(Visitor &&visitor) const
    {
        preorderVisit(root(), visitor);
    }

    template <typename Visitor>
    void postorder(Visitor &&visitor)
    {
        postorderVisit(root(), visitor);
    }

    template <typename Visitor>
    void postorder(Visitor &&visitor) const
    {
        postorderVisit(root(), visitor);
    }

    template <typename Visitor>
    void levelorder(Visitor &&visitor)
    {
        levelorderVisit(root(), visitor);
    }

    template <typename Visitor>
    void levelorder(Visitor &&visitor) const
    {
        levelorderVisit(root(), visitor);
    }

    std::vector<T> toVector() const
    {
        std::vector<T> values;
        values.reserve(countValue);
        inorder([&values](const T &value) { values.push_back(value); });
        return values;
    }

    void clear() noexcept
    {
        rootValue.reset();
        countValue = 0;
    }

    size_type size() const noexcept { return countValue; }
    bool empty() const noexcept { return countValue == 0; }
    size_type height() const noexcept { return rootValue ? rootValue->height() : 0; }

    node_type *root() noexcept { return rootValue.get(); }
    const node_type *root() const noexcept { return rootValue.get(); }

    const Compare &comparator() const noexcept { return compareValue; }

    void swap(BinarySearchTree &other)
    {
        using std::swap;
        swap(rootValue, other.rootValue);
        swap(countValue, other.countValue);
        swap(compareValue, other.compareValue);
    }

  private:
    template <typename U>
    bool insertImpl(std::unique_ptr<node_type> &link, U &&value)
    {
        if (!link)
        {
            link = std::make_unique<node_type>(std::forward<U>(value));
            ++countValue;
            return true;
        }

        if (compareValue(value, link->value))
            return insertImpl(link->left, std::forward<U>(value));
        if (compareValue(link->value, value))
            return insertImpl(link->right, std::forward<U>(value));
        return false;
    }

    bool eraseImpl(std::unique_ptr<node_type> &link, const T &value)
    {
        if (!link)
            return false;

        if (compareValue(value, link->value))
            return eraseImpl(link->left, value);
        if (compareValue(link->value, value))
            return eraseImpl(link->right, value);

        if (!link->left)
        {
            link = std::move(link->right);
        }
        else if (!link->right)
        {
            link = std::move(link->left);
        }
        else
        {
            std::unique_ptr<node_type> successor = detachMin(link->right);
            successor->left = std::move(link->left);
            successor->right = std::move(link->right);
            link = std::move(successor);
        }

        --countValue;
        return true;
    }

    // Removes and returns the left-most node of a non-empty subtree. The
    // removed node's right subtree is spliced back into its former position.
    static std::unique_ptr<node_type> detachMin(std::unique_ptr<node_type> &link)
    {
        if (!link->left)
        {
            std::unique_ptr<node_type> minimum = std::move(link);
            link = std::move(minimum->right);
            return minimum;
        }
        return detachMin(link->left);
    }

    template <typename InputIt>
    void insertRange(InputIt first, InputIt last)
    {
        for (; first != last; ++first)
        {
            insert(*first);
        }
    }

    template <typename NodePointer, typename Visitor>
    static void inorderVisit(NodePointer current, Visitor &visitor)
    {
        if (!current)
            return;
        inorderVisit(current->left.get(), visitor);
        visitor(current->value);
        inorderVisit(current->right.get(), visitor);
    }

    template <typename NodePointer, typename Visitor>
    static void preorderVisit(NodePointer current, Visitor &visitor)
    {
        if (!current)
            return;
        visitor(current->value);
        preorderVisit(current->left.get(), visitor);
        preorderVisit(current->right.get(), visitor);
    }

    template <typename NodePointer, typename Visitor>
    static void postorderVisit(NodePointer current, Visitor &visitor)
    {
        if (!current)
            return;
        postorderVisit(current->left.get(), visitor);
        postorderVisit(current->right.get(), visitor);
        visitor(current->value);
    }

    template <typename NodePointer, typename Visitor>
    static void levelorderVisit(NodePointer current, Visitor &visitor)
    {
        if (!current)
            return;

        std::queue<NodePointer> pending;
        pending.push(current);
        while (!pending.empty())
        {
            NodePointer node = pending.front();
            pending.pop();
            visitor(node->value);
            if (node->left)
                pending.push(node->left.get());
            if (node->right)
                pending.push(node->right.get());
        }
    }

    std::unique_ptr<node_type> rootValue{};
    size_type countValue{};
    Compare compareValue{};
};

} // namespace tree
