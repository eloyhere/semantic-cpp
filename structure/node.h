#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <utility>

namespace node
{

// Owning node type for binary trees. Children are stored through
// std::unique_ptr, so copying a node recursively copies its whole subtree.
template <typename T>
struct BinaryNode
{
    using value_type = T;
    using node_type = BinaryNode<T>;

    T value{};
    std::unique_ptr<node_type> left{};
    std::unique_ptr<node_type> right{};

    BinaryNode() = default;
    explicit BinaryNode(const T &value) : value(value) {}
    explicit BinaryNode(T &&value) : value(std::move(value)) {}

    BinaryNode(const BinaryNode &other)
        : value(other.value),
          left(other.left ? std::make_unique<node_type>(*other.left) : nullptr),
          right(other.right ? std::make_unique<node_type>(*other.right) : nullptr)
    {
    }

    BinaryNode(BinaryNode &&other) = default;

    BinaryNode &operator=(const BinaryNode &other)
    {
        if (this != &other)
        {
            BinaryNode copy(other);
            swap(copy);
        }
        return *this;
    }

    BinaryNode &operator=(BinaryNode &&other) = default;
    ~BinaryNode() = default;

    void swap(BinaryNode &other)
    {
        using std::swap;
        swap(value, other.value);
        swap(left, other.left);
        swap(right, other.right);
    }

    bool isLeaf() const noexcept { return !left && !right; }

    std::size_t degree() const noexcept
    {
        return (left ? std::size_t{1} : std::size_t{0}) + (right ? std::size_t{1} : std::size_t{0});
    }

    // Number of values stored in this subtree, including this node.
    std::size_t size() const noexcept
    {
        std::size_t total = 1;
        if (left)
            total += left->size();
        if (right)
            total += right->size();
        return total;
    }

    // Number of nodes on the longest downward path. A leaf has height 1.
    std::size_t height() const noexcept
    {
        const std::size_t leftHeight = left ? left->height() : 0;
        const std::size_t rightHeight = right ? right->height() : 0;
        return 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
    }
};

// Node type for a trie. Every child is keyed by the character on the edge
// leading to it; a node is a terminal node exactly when its value is engaged.
template <typename Char, typename T>
struct TrieNode
{
    using char_type = Char;
    using value_type = T;
    using node_type = TrieNode<Char, T>;
    using children_type = std::map<Char, std::unique_ptr<node_type>>;

    std::optional<T> value{};
    children_type children{};

    TrieNode() = default;

    TrieNode(const TrieNode &other) : value(other.value)
    {
        for (const auto &entry : other.children)
        {
            children.emplace(entry.first, std::make_unique<node_type>(*entry.second));
        }
    }

    TrieNode(TrieNode &&other) = default;

    TrieNode &operator=(const TrieNode &other)
    {
        if (this != &other)
        {
            TrieNode copy(other);
            swap(copy);
        }
        return *this;
    }

    TrieNode &operator=(TrieNode &&other) = default;
    ~TrieNode() = default;

    void swap(TrieNode &other)
    {
        using std::swap;
        swap(value, other.value);
        swap(children, other.children);
    }

    bool terminal() const noexcept { return value.has_value(); }
    bool isLeaf() const noexcept { return children.empty(); }
    std::size_t degree() const noexcept { return children.size(); }

    // Number of terminal keys stored in this subtree.
    std::size_t size() const noexcept
    {
        std::size_t total = terminal() ? std::size_t{1} : std::size_t{0};
        for (const auto &entry : children)
        {
            total += entry.second->size();
        }
        return total;
    }

    // Number of nodes in this subtree, including this node.
    std::size_t nodeCount() const noexcept
    {
        std::size_t total = 1;
        for (const auto &entry : children)
        {
            total += entry.second->nodeCount();
        }
        return total;
    }

    // Number of nodes on the longest downward path. A leaf has height 1.
    std::size_t height() const noexcept
    {
        std::size_t maximum = 0;
        for (const auto &entry : children)
        {
            const std::size_t childHeight = entry.second->height();
            if (childHeight > maximum)
                maximum = childHeight;
        }
        return 1 + maximum;
    }

    node_type *child(Char character) noexcept
    {
        const auto iterator = children.find(character);
        return iterator == children.end() ? nullptr : iterator->second.get();
    }

    const node_type *child(Char character) const noexcept
    {
        const auto iterator = children.find(character);
        return iterator == children.end() ? nullptr : iterator->second.get();
    }

    node_type &ensureChild(Char character)
    {
        auto iterator = children.find(character);
        if (iterator == children.end())
        {
            iterator = children.emplace(character, std::make_unique<node_type>()).first;
        }
        return *iterator->second;
    }
};

// Node type for a ternary search tree. low and high hold smaller and larger
// characters at the same key position; equal advances to the next position.
template <typename Char, typename T>
struct TernaryNode
{
    using char_type = Char;
    using value_type = T;
    using node_type = TernaryNode<Char, T>;

    Char character{};
    std::optional<T> value{};
    std::unique_ptr<node_type> low{};
    std::unique_ptr<node_type> equal{};
    std::unique_ptr<node_type> high{};

    TernaryNode() = default;
    explicit TernaryNode(Char character) : character(character) {}

    TernaryNode(const TernaryNode &other)
        : character(other.character),
          value(other.value),
          low(other.low ? std::make_unique<node_type>(*other.low) : nullptr),
          equal(other.equal ? std::make_unique<node_type>(*other.equal) : nullptr),
          high(other.high ? std::make_unique<node_type>(*other.high) : nullptr)
    {
    }

    TernaryNode(TernaryNode &&other) = default;

    TernaryNode &operator=(const TernaryNode &other)
    {
        if (this != &other)
        {
            TernaryNode copy(other);
            swap(copy);
        }
        return *this;
    }

    TernaryNode &operator=(TernaryNode &&other) = default;
    ~TernaryNode() = default;

    void swap(TernaryNode &other)
    {
        using std::swap;
        swap(character, other.character);
        swap(value, other.value);
        swap(low, other.low);
        swap(equal, other.equal);
        swap(high, other.high);
    }

    bool terminal() const noexcept { return value.has_value(); }

    bool isLeaf() const noexcept { return !low && !equal && !high; }

    std::size_t degree() const noexcept
    {
        return (low ? std::size_t{1} : std::size_t{0}) + (equal ? std::size_t{1} : std::size_t{0}) +
               (high ? std::size_t{1} : std::size_t{0});
    }

    // Number of terminal keys stored in this subtree.
    std::size_t size() const noexcept
    {
        std::size_t total = terminal() ? std::size_t{1} : std::size_t{0};
        if (low)
            total += low->size();
        if (equal)
            total += equal->size();
        if (high)
            total += high->size();
        return total;
    }

    // Number of nodes in this subtree, including this node.
    std::size_t nodeCount() const noexcept
    {
        std::size_t total = 1;
        if (low)
            total += low->nodeCount();
        if (equal)
            total += equal->nodeCount();
        if (high)
            total += high->nodeCount();
        return total;
    }

    // Number of nodes on the longest downward path. A leaf has height 1.
    std::size_t height() const noexcept
    {
        const std::size_t lowHeight = low ? low->height() : 0;
        const std::size_t equalHeight = equal ? equal->height() : 0;
        const std::size_t highHeight = high ? high->height() : 0;
        std::size_t maximum = lowHeight > equalHeight ? lowHeight : equalHeight;
        if (highHeight > maximum)
            maximum = highHeight;
        return 1 + maximum;
    }
};

} // namespace node
