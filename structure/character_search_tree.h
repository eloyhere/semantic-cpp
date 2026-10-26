#pragma once

#include "node.h"

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace character
{

// A ternary search tree over byte strings. Each node stores one character:
// low and high hold smaller and larger characters at the same key position,
// while equal advances to the next position. Compared with a trie, the
// representation is more compact for sparse alphabets while retaining
// prefix-oriented searches.
//
// Value is the payload stored at terminal nodes; the default std::monostate
// turns the tree into a plain set of strings.
template <typename Value = std::monostate>
class CharacterSearchTree
{
  public:
    using char_type = char;
    using value_type = Value;
    using key_type = std::string;
    using string_view_type = std::string_view;
    using node_type = node::TernaryNode<char, Value>;
    using size_type = std::size_t;

    CharacterSearchTree() = default;

    CharacterSearchTree(std::initializer_list<key_type> keys)
    {
        for (const key_type &key : keys)
        {
            insert(key);
        }
    }

    template <typename InputIt, typename = std::enable_if_t<!std::is_integral<InputIt>::value>>
    CharacterSearchTree(InputIt first, InputIt last)
    {
        for (; first != last; ++first)
        {
            insert(*first);
        }
    }

    CharacterSearchTree(const CharacterSearchTree &other)
        : rootValue(other.rootValue ? std::make_unique<node_type>(*other.rootValue) : nullptr),
          emptyValue(other.emptyValue),
          countValue(other.countValue)
    {
    }

    CharacterSearchTree(CharacterSearchTree &&other) = default;

    CharacterSearchTree &operator=(const CharacterSearchTree &other)
    {
        if (this != &other)
        {
            CharacterSearchTree copy(other);
            swap(copy);
        }
        return *this;
    }

    CharacterSearchTree &operator=(CharacterSearchTree &&other) = default;
    ~CharacterSearchTree() = default;

    // Inserts a key and its value unless the key is already present. Returns
    // true only when a new key was inserted.
    bool insert(string_view_type key, const Value &value) { return insertImpl(key, value, false); }
    bool insert(string_view_type key, Value &&value) { return insertImpl(key, std::move(value), false); }

    bool insert(string_view_type key)
    {
        static_assert(std::is_default_constructible<Value>::value,
                      "CharacterSearchTree::insert(key) requires a default-constructible Value");
        return insertImpl(key, Value{}, false);
    }

    // Inserts a key or replaces the value of an existing key. Returns true only
    // when a new key was inserted.
    bool assign(string_view_type key, const Value &value) { return insertImpl(key, value, true); }
    bool assign(string_view_type key, Value &&value) { return insertImpl(key, std::move(value), true); }

    bool assign(string_view_type key)
    {
        static_assert(std::is_default_constructible<Value>::value,
                      "CharacterSearchTree::assign(key) requires a default-constructible Value");
        return insertImpl(key, Value{}, true);
    }

    bool erase(string_view_type key)
    {
        if (key.empty())
        {
            if (!emptyValue)
                return false;
            emptyValue.reset();
            --countValue;
            return true;
        }

        if (!eraseNode(rootValue, key, 0))
            return false;
        --countValue;
        return true;
    }

    bool contains(string_view_type key) const { return find(key) != nullptr; }

    const Value *find(string_view_type key) const
    {
        if (key.empty())
            return emptyValue ? &*emptyValue : nullptr;

        const node_type *current = rootValue.get();
        size_type depth = 0;
        while (current)
        {
            const char character = key[depth];
            if (character < current->character)
            {
                current = current->low.get();
            }
            else if (character > current->character)
            {
                current = current->high.get();
            }
            else if (depth + 1 == key.size())
            {
                return current->value ? &*current->value : nullptr;
            }
            else
            {
                current = current->equal.get();
                ++depth;
            }
        }
        return nullptr;
    }

    Value *find(string_view_type key)
    {
        if (key.empty())
            return emptyValue ? &*emptyValue : nullptr;

        node_type *current = rootValue.get();
        size_type depth = 0;
        while (current)
        {
            const char character = key[depth];
            if (character < current->character)
            {
                current = current->low.get();
            }
            else if (character > current->character)
            {
                current = current->high.get();
            }
            else if (depth + 1 == key.size())
            {
                return current->value ? &*current->value : nullptr;
            }
            else
            {
                current = current->equal.get();
                ++depth;
            }
        }
        return nullptr;
    }

    // Returns true when at least one stored key starts with the given prefix.
    bool containsPrefix(string_view_type prefix) const
    {
        if (prefix.empty())
            return !empty();

        const node_type *current = rootValue.get();
        size_type depth = 0;
        while (current)
        {
            const char character = prefix[depth];
            if (character < current->character)
            {
                current = current->low.get();
            }
            else if (character > current->character)
            {
                current = current->high.get();
            }
            else if (depth + 1 == prefix.size())
            {
                return current->value.has_value() || current->equal != nullptr;
            }
            else
            {
                current = current->equal.get();
                ++depth;
            }
        }
        return false;
    }

    // Returns every key in lexicographic order.
    std::vector<key_type> keys() const
    {
        std::vector<key_type> result;
        result.reserve(countValue);
        if (emptyValue)
            result.emplace_back();

        key_type prefix;
        collect(rootValue.get(), prefix, result);
        return result;
    }

    // Returns every key that starts with prefix, in lexicographic order.
    std::vector<key_type> keysWithPrefix(string_view_type prefix) const
    {
        if (prefix.empty())
            return keys();

        const node_type *current = rootValue.get();
        size_type depth = 0;
        while (current)
        {
            const char character = prefix[depth];
            if (character < current->character)
            {
                current = current->low.get();
            }
            else if (character > current->character)
            {
                current = current->high.get();
            }
            else if (depth + 1 == prefix.size())
            {
                std::vector<key_type> result;
                key_type buffer(prefix.begin(), prefix.end());
                if (current->value)
                    result.push_back(buffer);
                collect(current->equal.get(), buffer, result);
                return result;
            }
            else
            {
                current = current->equal.get();
                ++depth;
            }
        }
        return {};
    }

    // Returns the longest stored key that is a prefix of text, or std::nullopt
    // when no stored key matches.
    std::optional<key_type> longestPrefixOf(string_view_type text) const
    {
        size_type best = emptyValue ? 0 : npos;
        const node_type *current = rootValue.get();
        size_type depth = 0;

        while (current && depth < text.size())
        {
            const char character = text[depth];
            if (character < current->character)
            {
                current = current->low.get();
            }
            else if (character > current->character)
            {
                current = current->high.get();
            }
            else
            {
                ++depth;
                if (current->value)
                    best = depth;
                current = current->equal.get();
            }
        }

        if (best == npos)
            return std::nullopt;
        return key_type(text.data(), best);
    }

    // Returns stored keys matching a glob pattern. '?' matches exactly one
    // character and '*' matches any sequence, including an empty one. This
    // convenience operation scans the stored keys; for plain prefixes prefer
    // containsPrefix(), keysWithPrefix(), or longestPrefixOf().
    std::vector<key_type> match(string_view_type pattern) const
    {
        std::vector<key_type> result;
        for (const key_type &key : keys())
        {
            if (matchesPattern(key, pattern))
                result.push_back(key);
        }
        return result;
    }

    static bool matchesPattern(string_view_type text, string_view_type pattern)
    {
        size_type textIndex = 0;
        size_type patternIndex = 0;
        size_type starIndex = npos;
        size_type backtrack = 0;

        while (textIndex < text.size())
        {
            if (patternIndex < pattern.size() &&
                (pattern[patternIndex] == '?' || pattern[patternIndex] == text[textIndex]))
            {
                ++textIndex;
                ++patternIndex;
            }
            else if (patternIndex < pattern.size() && pattern[patternIndex] == '*')
            {
                starIndex = patternIndex++;
                backtrack = textIndex;
            }
            else if (starIndex != npos)
            {
                patternIndex = starIndex + 1;
                ++backtrack;
                textIndex = backtrack;
            }
            else
            {
                return false;
            }
        }

        while (patternIndex < pattern.size() && pattern[patternIndex] == '*')
            ++patternIndex;
        return patternIndex == pattern.size();
    }

    void clear() noexcept
    {
        rootValue.reset();
        emptyValue.reset();
        countValue = 0;
    }

    size_type size() const noexcept { return countValue; }
    bool empty() const noexcept { return countValue == 0; }
    size_type nodeCount() const noexcept { return rootValue ? rootValue->nodeCount() : 0; }

    // Structural height of the ternary tree, not the length of the longest key.
    size_type height() const noexcept { return rootValue ? rootValue->height() : 0; }

    const node_type *root() const noexcept { return rootValue.get(); }
    node_type *root() noexcept { return rootValue.get(); }

    void swap(CharacterSearchTree &other)
    {
        using std::swap;
        swap(rootValue, other.rootValue);
        swap(emptyValue, other.emptyValue);
        swap(countValue, other.countValue);
    }

  private:
    template <typename V>
    bool insertImpl(string_view_type key, V &&value, bool replace)
    {
        if (key.empty())
        {
            if (emptyValue)
            {
                if (replace)
                    emptyValue = std::forward<V>(value);
                return false;
            }
            emptyValue.emplace(std::forward<V>(value));
            ++countValue;
            return true;
        }

        bool inserted = false;
        insertNode(rootValue, key, 0, std::forward<V>(value), replace, inserted);
        if (inserted)
            ++countValue;
        return inserted;
    }

    template <typename V>
    static void insertNode(std::unique_ptr<node_type> &link, string_view_type key, size_type depth, V &&value,
                           bool replace, bool &inserted)
    {
        if (!link)
            link = std::make_unique<node_type>(key[depth]);

        node_type &current = *link;
        const char character = key[depth];

        if (character < current.character)
        {
            insertNode(current.low, key, depth, std::forward<V>(value), replace, inserted);
        }
        else if (character > current.character)
        {
            insertNode(current.high, key, depth, std::forward<V>(value), replace, inserted);
        }
        else if (depth + 1 == key.size())
        {
            if (current.value)
            {
                if (replace)
                    current.value = std::forward<V>(value);
                inserted = false;
            }
            else
            {
                current.value.emplace(std::forward<V>(value));
                inserted = true;
            }
        }
        else
        {
            insertNode(current.equal, key, depth + 1, std::forward<V>(value), replace, inserted);
        }
    }

    static bool eraseNode(std::unique_ptr<node_type> &link, string_view_type key, size_type depth)
    {
        if (!link)
            return false;

        node_type &current = *link;
        const char character = key[depth];
        bool removed = false;

        if (character < current.character)
        {
            removed = eraseNode(current.low, key, depth);
        }
        else if (character > current.character)
        {
            removed = eraseNode(current.high, key, depth);
        }
        else if (depth + 1 == key.size())
        {
            if (!current.value)
                return false;
            current.value.reset();
            removed = true;
        }
        else
        {
            removed = eraseNode(current.equal, key, depth + 1);
        }

        if (removed && !current.value && !current.low && !current.equal && !current.high)
            link.reset();
        return removed;
    }

    // Appends every terminal key below current. prefix holds the characters
    // that precede current->character, so the traversal emits keys in
    // lexicographic order.
    static void collect(const node_type *current, key_type &prefix, std::vector<key_type> &result)
    {
        if (!current)
            return;

        collect(current->low.get(), prefix, result);

        prefix.push_back(current->character);
        if (current->value)
            result.push_back(prefix);
        collect(current->equal.get(), prefix, result);
        prefix.pop_back();

        collect(current->high.get(), prefix, result);
    }

    static constexpr size_type npos = static_cast<size_type>(-1);

    std::unique_ptr<node_type> rootValue{};
    std::optional<Value> emptyValue{};
    size_type countValue{};
};

template <typename Value = std::monostate>
using TernarySearchTree = CharacterSearchTree<Value>;

} // namespace character
