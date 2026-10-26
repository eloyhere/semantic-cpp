#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ast
{

// Offset is zero-based; line and column are one-based. A source range is
// half-open: [begin, end).
struct SourcePosition
{
    std::size_t offset{};
    std::size_t line{1};
    std::size_t column{1};

    friend bool operator==(const SourcePosition &left, const SourcePosition &right)
    {
        return left.offset == right.offset && left.line == right.line && left.column == right.column;
    }

    friend bool operator!=(const SourcePosition &left, const SourcePosition &right) { return !(left == right); }
};

struct SourceRange
{
    SourcePosition begin{};
    SourcePosition end{};

    bool empty() const noexcept { return begin.offset == end.offset; }

    std::size_t length() const noexcept
    {
        return end.offset >= begin.offset ? end.offset - begin.offset : std::size_t{0};
    }

    bool contains(const SourcePosition &position) const noexcept
    {
        return position.offset >= begin.offset && position.offset < end.offset;
    }

    friend bool operator==(const SourceRange &left, const SourceRange &right)
    {
        return left.begin == right.begin && left.end == right.end;
    }

    friend bool operator!=(const SourceRange &left, const SourceRange &right) { return !(left == right); }
};

template <typename Kind, typename Value>
class Tree;

// An owning node of an abstract syntax tree. Kind identifies the grammar
// production (an enum, a string tag, and so on), while Value carries node
// specific data such as the spelling of an identifier or literal. The source
// range is stored on every node so diagnostics remain available after the
// tree has been rearranged.
//
// A node owns its children. Parent pointers are maintained automatically by
// the mutation functions and are never owned.
template <typename Kind = std::string, typename Value = std::string>
class Node
{
  public:
    using kind_type = Kind;
    using value_type = Value;
    using node_type = Node<Kind, Value>;
    using children_type = std::vector<std::unique_ptr<node_type>>;
    using size_type = std::size_t;

    explicit Node(Kind kind, Value value = Value{}, SourceRange range = {})
        : kindValue(std::move(kind)), valueValue(std::move(value)), rangeValue(std::move(range))
    {
    }

    Node(const Node &other)
        : kindValue(other.kindValue), valueValue(other.valueValue), rangeValue(other.rangeValue)
    {
        childrenValue.reserve(other.childrenValue.size());
        for (const auto &child : other.childrenValue)
        {
            std::unique_ptr<node_type> copied = std::make_unique<node_type>(*child);
            copied->parentValue = this;
            childrenValue.push_back(std::move(copied));
        }
    }

    Node(Node &&other)
        : kindValue(std::move(other.kindValue)),
          valueValue(std::move(other.valueValue)),
          rangeValue(other.rangeValue),
          childrenValue(std::move(other.childrenValue))
    {
        other.childrenValue.clear();
        for (auto &child : childrenValue)
        {
            child->parentValue = this;
        }
    }

    Node &operator=(const Node &other)
    {
        if (this != &other)
        {
            Node copy(other);
            swapContent(copy);
        }
        return *this;
    }

    Node &operator=(Node &&other)
    {
        if (this != &other)
        {
            childrenValue.clear();
            kindValue = std::move(other.kindValue);
            valueValue = std::move(other.valueValue);
            rangeValue = other.rangeValue;
            childrenValue = std::move(other.childrenValue);
            other.childrenValue.clear();
            for (auto &child : childrenValue)
            {
                child->parentValue = this;
            }
        }
        return *this;
    }

    ~Node() = default;

    const Kind &kind() const noexcept { return kindValue; }
    Kind &kind() noexcept { return kindValue; }

    const Value &value() const noexcept { return valueValue; }
    Value &value() noexcept { return valueValue; }

    const SourceRange &range() const noexcept { return rangeValue; }
    SourceRange &range() noexcept { return rangeValue; }

    node_type *parent() noexcept { return parentValue; }
    const node_type *parent() const noexcept { return parentValue; }

    bool isRoot() const noexcept { return parentValue == nullptr; }
    bool isLeaf() const noexcept { return childrenValue.empty(); }

    size_type childCount() const noexcept { return childrenValue.size(); }

    const children_type &children() const noexcept { return childrenValue; }

    node_type *child(size_type index) noexcept
    {
        return index < childrenValue.size() ? childrenValue[index].get() : nullptr;
    }

    const node_type *child(size_type index) const noexcept
    {
        return index < childrenValue.size() ? childrenValue[index].get() : nullptr;
    }

    node_type &at(size_type index)
    {
        checkChildIndex(index);
        return *childrenValue[index];
    }

    const node_type &at(size_type index) const
    {
        checkChildIndex(index);
        return *childrenValue[index];
    }

    std::optional<size_type> childIndex(const node_type &child) const noexcept
    {
        for (size_type index = 0; index < childrenValue.size(); ++index)
        {
            if (childrenValue[index].get() == &child)
                return index;
        }
        return std::nullopt;
    }

    // Appends a child and returns a reference to it. Throws if child is null.
    node_type &appendChild(std::unique_ptr<node_type> child)
    {
        if (!child)
            throw std::invalid_argument("ast::Node::appendChild: null child");

        child->parentValue = this;
        childrenValue.push_back(std::move(child));
        return *childrenValue.back();
    }

    node_type &appendChild(Kind kind, Value value = Value{}, SourceRange range = {})
    {
        return appendChild(std::make_unique<node_type>(std::move(kind), std::move(value), std::move(range)));
    }

    // Inserts a child at index; index may equal childCount() to append.
    node_type &insertChild(size_type index, std::unique_ptr<node_type> child)
    {
        if (!child)
            throw std::invalid_argument("ast::Node::insertChild: null child");
        if (index > childrenValue.size())
            throw std::out_of_range("ast::Node::insertChild: index out of bounds");

        child->parentValue = this;
        auto iterator = childrenValue.insert(childrenValue.begin() + static_cast<std::ptrdiff_t>(index),
                                             std::move(child));
        return **iterator;
    }

    node_type &insertChild(size_type index, Kind kind, Value value = Value{}, SourceRange range = {})
    {
        return insertChild(index,
                           std::make_unique<node_type>(std::move(kind), std::move(value), std::move(range)));
    }

    // Replaces a child, detaches the previous child, and returns it.
    std::unique_ptr<node_type> replaceChild(size_type index, std::unique_ptr<node_type> child)
    {
        if (!child)
            throw std::invalid_argument("ast::Node::replaceChild: null child");
        checkChildIndex(index);

        child->parentValue = this;
        std::unique_ptr<node_type> previous = std::move(childrenValue[index]);
        childrenValue[index] = std::move(child);
        previous->parentValue = nullptr;
        return previous;
    }

    // Removes a child by index without destroying it.
    std::unique_ptr<node_type> detachChildAt(size_type index)
    {
        checkChildIndex(index);

        std::unique_ptr<node_type> child = std::move(childrenValue[index]);
        childrenValue.erase(childrenValue.begin() + static_cast<std::ptrdiff_t>(index));
        child->parentValue = nullptr;
        return child;
    }

    std::unique_ptr<node_type> detachChild(node_type *child)
    {
        if (!child)
            throw std::invalid_argument("ast::Node::detachChild: null child");

        const std::optional<size_type> index = childIndex(*child);
        if (!index)
            throw std::invalid_argument("ast::Node::detachChild: node is not a child");
        return detachChildAt(*index);
    }

    bool eraseChildAt(size_type index)
    {
        checkChildIndex(index);
        childrenValue.erase(childrenValue.begin() + static_cast<std::ptrdiff_t>(index));
        return true;
    }

    bool eraseChild(node_type *child)
    {
        if (!child)
            return false;

        const std::optional<size_type> index = childIndex(*child);
        if (!index)
            return false;
        return eraseChildAt(*index);
    }

    void clearChildren() noexcept { childrenValue.clear(); }

    node_type *root() noexcept
    {
        node_type *current = this;
        while (current->parentValue)
            current = current->parentValue;
        return current;
    }

    const node_type *root() const noexcept
    {
        const node_type *current = this;
        while (current->parentValue)
            current = current->parentValue;
        return current;
    }

    // Number of edges between this node and the root; the root has depth 0.
    size_type depth() const noexcept
    {
        size_type result = 0;
        for (const node_type *current = parentValue; current; current = current->parentValue)
            ++result;
        return result;
    }

    // Number of nodes in this subtree, including this node.
    size_type size() const noexcept
    {
        size_type total = 1;
        for (const auto &child : childrenValue)
            total += child->size();
        return total;
    }

    size_type descendantCount() const noexcept { return size() - 1; }

    // Number of nodes on the longest downward path. A leaf has height 1.
    size_type height() const noexcept
    {
        size_type maximum = 0;
        for (const auto &child : childrenValue)
            maximum = std::max(maximum, child->height());
        return 1 + maximum;
    }

    // Ancestors from the parent up to the root.
    std::vector<node_type *> ancestors()
    {
        std::vector<node_type *> result;
        for (node_type *current = parentValue; current; current = current->parentValue)
            result.push_back(current);
        return result;
    }

    std::vector<const node_type *> ancestors() const
    {
        std::vector<const node_type *> result;
        for (const node_type *current = parentValue; current; current = current->parentValue)
            result.push_back(current);
        return result;
    }

    // Path from the root down to this node.
    std::vector<node_type *> path()
    {
        std::vector<node_type *> result;
        for (node_type *current = this; current; current = current->parentValue)
            result.push_back(current);
        std::reverse(result.begin(), result.end());
        return result;
    }

    std::vector<const node_type *> path() const
    {
        std::vector<const node_type *> result;
        for (const node_type *current = this; current; current = current->parentValue)
            result.push_back(current);
        std::reverse(result.begin(), result.end());
        return result;
    }

    // Visitors must not add or remove children while a traversal is running;
    // updating a node's kind, value, or range is safe.
    template <typename Visitor>
    void preorder(Visitor &&visitor)
    {
        preorderVisit(this, visitor);
    }

    template <typename Visitor>
    void preorder(Visitor &&visitor) const
    {
        preorderVisit(this, visitor);
    }

    template <typename Visitor>
    void postorder(Visitor &&visitor)
    {
        postorderVisit(this, visitor);
    }

    template <typename Visitor>
    void postorder(Visitor &&visitor) const
    {
        postorderVisit(this, visitor);
    }

    template <typename Visitor>
    void levelorder(Visitor &&visitor)
    {
        levelorderVisit(this, visitor);
    }

    template <typename Visitor>
    void levelorder(Visitor &&visitor) const
    {
        levelorderVisit(this, visitor);
    }

    // Calls enter on the way down and leave on the way up.
    template <typename Enter, typename Leave>
    void visit(Enter &&enter, Leave &&leave)
    {
        visitImpl(this, enter, leave);
    }

    template <typename Enter, typename Leave>
    void visit(Enter &&enter, Leave &&leave) const
    {
        visitImpl(this, enter, leave);
    }

    template <typename Predicate>
    node_type *find(Predicate &&predicate)
    {
        if (predicate(*this))
            return this;
        for (auto &child : childrenValue)
        {
            node_type *found = child->find(predicate);
            if (found)
                return found;
        }
        return nullptr;
    }

    template <typename Predicate>
    const node_type *find(Predicate &&predicate) const
    {
        if (predicate(*this))
            return this;
        for (const auto &child : childrenValue)
        {
            const node_type *childNode = child.get();
            const node_type *found = childNode->find(predicate);
            if (found)
                return found;
        }
        return nullptr;
    }

    template <typename Predicate>
    std::vector<node_type *> findAll(Predicate &&predicate)
    {
        std::vector<node_type *> result;
        collectMatches(this, predicate, result);
        return result;
    }

    template <typename Predicate>
    std::vector<const node_type *> findAll(Predicate &&predicate) const
    {
        std::vector<const node_type *> result;
        collectMatches(this, predicate, result);
        return result;
    }

  private:
    template <typename K, typename V>
    friend class Tree;

    void checkChildIndex(size_type index) const
    {
        if (index >= childrenValue.size())
            throw std::out_of_range("ast::Node: child index out of bounds");
    }

    void swapContent(Node &other)
    {
        using std::swap;
        swap(kindValue, other.kindValue);
        swap(valueValue, other.valueValue);
        swap(rangeValue, other.rangeValue);
        swap(childrenValue, other.childrenValue);

        for (auto &child : childrenValue)
            child->parentValue = this;
        for (auto &child : other.childrenValue)
            child->parentValue = &other;
    }

    template <typename NodePointer, typename Visitor>
    static void preorderVisit(NodePointer current, Visitor &visitor)
    {
        if (!current)
            return;
        visitor(*current);
        for (const auto &child : current->childrenValue)
            preorderVisit(child.get(), visitor);
    }

    template <typename NodePointer, typename Visitor>
    static void postorderVisit(NodePointer current, Visitor &visitor)
    {
        if (!current)
            return;
        for (const auto &child : current->childrenValue)
            postorderVisit(child.get(), visitor);
        visitor(*current);
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
            visitor(*node);
            for (const auto &child : node->childrenValue)
                pending.push(child.get());
        }
    }

    template <typename NodePointer, typename Enter, typename Leave>
    static void visitImpl(NodePointer current, Enter &enter, Leave &leave)
    {
        if (!current)
            return;
        enter(*current);
        for (const auto &child : current->childrenValue)
            visitImpl(child.get(), enter, leave);
        leave(*current);
    }

    template <typename Predicate>
    static void collectMatches(node_type *current, Predicate &predicate, std::vector<node_type *> &result)
    {
        if (predicate(*current))
            result.push_back(current);
        for (auto &child : current->childrenValue)
            collectMatches(child.get(), predicate, result);
    }

    template <typename Predicate>
    static void collectMatches(const node_type *current, Predicate &predicate,
                               std::vector<const node_type *> &result)
    {
        if (predicate(*current))
            result.push_back(current);
        for (const auto &child : current->childrenValue)
            collectMatches(child.get(), predicate, result);
    }

    Kind kindValue;
    Value valueValue;
    SourceRange rangeValue{};
    node_type *parentValue = nullptr;
    children_type childrenValue{};
};

// Owns the root of an abstract syntax tree.
template <typename Kind = std::string, typename Value = std::string>
class Tree
{
  public:
    using kind_type = Kind;
    using value_type = Value;
    using node_type = Node<Kind, Value>;
    using size_type = std::size_t;

    Tree() = default;

    explicit Tree(Kind rootKind, Value rootValue = Value{}, SourceRange range = {})
        : rootValue(std::make_unique<node_type>(std::move(rootKind), std::move(rootValue), std::move(range)))
    {
    }

    explicit Tree(std::unique_ptr<node_type> root) : rootValue(std::move(root))
    {
        if (rootValue)
            rootValue->parentValue = nullptr;
    }

    Tree(const Tree &other)
        : rootValue(other.rootValue ? std::make_unique<node_type>(*other.rootValue) : nullptr)
    {
    }

    Tree(Tree &&other) = default;

    Tree &operator=(const Tree &other)
    {
        if (this != &other)
        {
            Tree copy(other);
            swap(copy);
        }
        return *this;
    }

    Tree &operator=(Tree &&other) = default;
    ~Tree() = default;

    node_type *root() noexcept { return rootValue.get(); }
    const node_type *root() const noexcept { return rootValue.get(); }

    node_type &atRoot()
    {
        if (!rootValue)
            throw std::logic_error("ast::Tree::atRoot: tree is empty");
        return *rootValue;
    }

    const node_type &atRoot() const
    {
        if (!rootValue)
            throw std::logic_error("ast::Tree::atRoot: tree is empty");
        return *rootValue;
    }

    bool empty() const noexcept { return !rootValue; }

    size_type size() const noexcept { return rootValue ? rootValue->size() : 0; }
    size_type height() const noexcept { return rootValue ? rootValue->height() : 0; }

    void setRoot(std::unique_ptr<node_type> root)
    {
        if (root)
            root->parentValue = nullptr;
        rootValue = std::move(root);
    }

    std::unique_ptr<node_type> releaseRoot() noexcept { return std::move(rootValue); }

    void clear() noexcept { rootValue.reset(); }

    void swap(Tree &other) noexcept { rootValue.swap(other.rootValue); }

    template <typename Visitor>
    void preorder(Visitor &&visitor)
    {
        if (rootValue)
            rootValue->preorder(std::forward<Visitor>(visitor));
    }

    template <typename Visitor>
    void preorder(Visitor &&visitor) const
    {
        const node_type *rootNode = root();
        if (rootNode)
            rootNode->preorder(std::forward<Visitor>(visitor));
    }

    template <typename Visitor>
    void postorder(Visitor &&visitor)
    {
        if (rootValue)
            rootValue->postorder(std::forward<Visitor>(visitor));
    }

    template <typename Visitor>
    void postorder(Visitor &&visitor) const
    {
        const node_type *rootNode = root();
        if (rootNode)
            rootNode->postorder(std::forward<Visitor>(visitor));
    }

    template <typename Visitor>
    void levelorder(Visitor &&visitor)
    {
        if (rootValue)
            rootValue->levelorder(std::forward<Visitor>(visitor));
    }

    template <typename Visitor>
    void levelorder(Visitor &&visitor) const
    {
        const node_type *rootNode = root();
        if (rootNode)
            rootNode->levelorder(std::forward<Visitor>(visitor));
    }

    template <typename Enter, typename Leave>
    void visit(Enter &&enter, Leave &&leave)
    {
        if (rootValue)
            rootValue->visit(std::forward<Enter>(enter), std::forward<Leave>(leave));
    }

    template <typename Enter, typename Leave>
    void visit(Enter &&enter, Leave &&leave) const
    {
        const node_type *rootNode = root();
        if (rootNode)
            rootNode->visit(std::forward<Enter>(enter), std::forward<Leave>(leave));
    }

    template <typename Predicate>
    node_type *find(Predicate &&predicate)
    {
        return rootValue ? rootValue->find(std::forward<Predicate>(predicate)) : nullptr;
    }

    template <typename Predicate>
    const node_type *find(Predicate &&predicate) const
    {
        const node_type *rootNode = root();
        return rootNode ? rootNode->find(std::forward<Predicate>(predicate)) : nullptr;
    }

    template <typename Predicate>
    std::vector<node_type *> findAll(Predicate &&predicate)
    {
        return rootValue ? rootValue->findAll(std::forward<Predicate>(predicate)) : std::vector<node_type *>{};
    }

    template <typename Predicate>
    std::vector<const node_type *> findAll(Predicate &&predicate) const
    {
        const node_type *rootNode = root();
        return rootNode ? rootNode->findAll(std::forward<Predicate>(predicate))
                        : std::vector<const node_type *>{};
    }

  private:
    std::unique_ptr<node_type> rootValue{};
};

} // namespace ast
