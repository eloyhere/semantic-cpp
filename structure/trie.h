#pragma once

#include "node.h"

#include <cstddef>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace trie
{

// A prefix tree (trie) over character sequences. Char must be a character
// type supported by std::basic_string (for example char, char16_t, char32_t).
// Value is the payload stored at terminal nodes; the default std::monostate
// turns the trie into a plain set of keys.
template <typename Char = char, typename Value = std::monostate>
class Trie
{
  public:
    using char_type = Char;
    using value_type = Value;
    using key_type = std::basic_string<Char>;
    using string_view_type = std::basic_string_view<Char>;
    using node_type = node::TrieNode<Char, Value>;
    using size_type = std::size_t;

    Trie() = default;

    Trie(std::initializer_list<key_type> keys)
    {
        for (const key_type &key : keys)
        {
            insert(key);
        }
    }

    template <typename InputIt, typename = std::enable_if_t<!std::is_integral<InputIt>::value>>
    Trie(InputIt first, InputIt last)
    {
        for (; first != last; ++first)
        {
            insert(*first);
        }
    }

    Trie(const Trie &) = default;
    Trie(Trie &&) = default;
    Trie &operator=(const Trie &) = default;
    Trie &operator=(Trie &&) = default;
    ~Trie() = default;

    // Inserts a key and its value unless the key is already present. Returns
    // true only when a new key was inserted.
    bool insert(string_view_type key, const Value &value) { return insertImpl(key, value, false); }
    bool insert(string_view_type key, Value &&value) { return insertImpl(key, std::move(value), false); }

    bool insert(string_view_type key)
    {
        static_assert(std::is_default_constructible<Value>::value,
                      "Trie::insert(key) requires a default-constructible Value");
        return insertImpl(key, Value{}, false);
    }

    // Inserts a key or replaces the value of an existing key. Returns true only
    // when a new key was inserted.
    bool assign(string_view_type key, const Value &value) { return insertImpl(key, value, true); }
    bool assign(string_view_type key, Value &&value) { return insertImpl(key, std::move(value), true); }

    bool assign(string_view_type key)
    {
        static_assert(std::is_default_constructible<Value>::value,
                      "Trie::assign(key) requires a default-constructible Value");
        return insertImpl(key, Value{}, true);
    }

    bool erase(string_view_type key) { return eraseImpl(rootValue, key, 0); }

    bool contains(string_view_type key) const { return find(key) != nullptr; }

    const Value *find(string_view_type key) const
    {
        const node_type *current = &rootValue;
        for (Char character : key)
        {
            current = current->child(character);
            if (!current)
                return nullptr;
        }
        return current->value ? &*current->value : nullptr;
    }

    Value *find(string_view_type key)
    {
        node_type *current = &rootValue;
        for (Char character : key)
        {
            current = current->child(character);
            if (!current)
                return nullptr;
        }
        return current->value ? &*current->value : nullptr;
    }

    // Returns true when at least one stored key starts with the given prefix.
    bool containsPrefix(string_view_type prefix) const
    {
        if (prefix.empty())
            return !empty();

        const node_type *current = &rootValue;
        for (Char character : prefix)
        {
            current = current->child(character);
            if (!current)
                return false;
        }
        return true;
    }

    // Returns every key in lexicographic order.
    std::vector<key_type> keys() const
    {
        std::vector<key_type> result;
        result.reserve(countValue);
        key_type prefix;
        collect(&rootValue, prefix, result);
        return result;
    }

    // Returns every key that starts with prefix, in lexicographic order.
    std::vector<key_type> keysWithPrefix(string_view_type prefix) const
    {
        const node_type *current = &rootValue;
        for (Char character : prefix)
        {
            current = current->child(character);
            if (!current)
                return {};
        }

        std::vector<key_type> result;
        key_type buffer(prefix.begin(), prefix.end());
        collect(current, buffer, result);
        return result;
    }

    // Returns the longest stored key that is a prefix of text, or std::nullopt
    // when no stored key matches.
    std::optional<key_type> longestPrefixOf(string_view_type text) const
    {
        const node_type *current = &rootValue;
        size_type best = rootValue.value ? 0 : npos;

        for (size_type index = 0; index < text.size(); ++index)
        {
            current = current->child(text[index]);
            if (!current)
                break;
            if (current->value)
                best = index + 1;
        }

        if (best == npos)
            return std::nullopt;
        return key_type(text.data(), best);
    }

    void clear() noexcept
    {
        rootValue.value.reset();
        rootValue.children.clear();
        countValue = 0;
    }

    size_type size() const noexcept { return countValue; }
    bool empty() const noexcept { return countValue == 0; }
    size_type nodeCount() const noexcept { return rootValue.nodeCount(); }

    const node_type &root() const noexcept { return rootValue; }
    node_type &root() noexcept { return rootValue; }

    void swap(Trie &other)
    {
        using std::swap;
        rootValue.swap(other.rootValue);
        swap(countValue, other.countValue);
    }

  private:
    template <typename V>
    bool insertImpl(string_view_type key, V &&value, bool replace)
    {
        node_type *current = &rootValue;
        for (Char character : key)
        {
            auto iterator = current->children.find(character);
            if (iterator == current->children.end())
            {
                iterator = current->children.emplace(character, std::make_unique<node_type>()).first;
            }
            current = iterator->second.get();
        }

        if (current->value)
        {
            if (replace)
                current->value = std::forward<V>(value);
            return false;
        }

        current->value.emplace(std::forward<V>(value));
        ++countValue;
        return true;
    }

    bool eraseImpl(node_type &current, string_view_type key, size_type depth)
    {
        if (depth == key.size())
        {
            if (!current.value)
                return false;
            current.value.reset();
            --countValue;
            return true;
        }

        auto iterator = current.children.find(key[depth]);
        if (iterator == current.children.end())
            return false;

        if (!eraseImpl(*iterator->second, key, depth + 1))
            return false;

        if (!iterator->second->value && iterator->second->children.empty())
            current.children.erase(iterator);
        return true;
    }

    static void collect(const node_type *current, key_type &prefix, std::vector<key_type> &result)
    {
        if (current->value)
            result.push_back(prefix);

        for (const auto &entry : current->children)
        {
            prefix.push_back(entry.first);
            collect(entry.second.get(), prefix, result);
            prefix.pop_back();
        }
    }

    static constexpr size_type npos = static_cast<size_type>(-1);

    node_type rootValue{};
    size_type countValue{};
};

} // namespace trie
