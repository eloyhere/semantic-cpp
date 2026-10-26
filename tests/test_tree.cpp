#include "structure/character_search_tree.h"
#include "structure/node.h"
#include "structure/segment_tree.h"
#include "structure/tree.h"
#include "structure/trie.h"

#include "test_framework.h"

#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

TEST_CASE("node binary node deep copy")
{
    node::BinaryNode<int> root(1);
    root.left = std::make_unique<node::BinaryNode<int>>(2);
    root.right = std::make_unique<node::BinaryNode<int>>(3);
    root.left->left = std::make_unique<node::BinaryNode<int>>(4);

    CHECK_EQ(root.size(), std::size_t{4});
    CHECK_EQ(root.height(), std::size_t{3});
    CHECK_EQ(root.degree(), std::size_t{2});
    CHECK_FALSE(root.isLeaf());

    node::BinaryNode<int> copy(root);
    CHECK_EQ(copy.size(), std::size_t{4});
    CHECK(copy.left.get() != root.left.get());
    copy.left->value = 20;
    CHECK_EQ(root.left->value, 2);
    CHECK_EQ(copy.left->value, 20);

    node::BinaryNode<int> assigned;
    assigned = root;
    CHECK_EQ(assigned.size(), std::size_t{4});
}

TEST_CASE("node trie node structure")
{
    node::TrieNode<char, int> root;
    CHECK(root.isLeaf());

    root.value.emplace(7);
    node::TrieNode<char, int> &a = root.ensureChild('a');
    a.value.emplace(1);
    node::TrieNode<char, int> &b = root.ensureChild('b');
    b.value.emplace(2);

    CHECK_EQ(root.size(), std::size_t{3});
    CHECK_EQ(root.nodeCount(), std::size_t{3});
    CHECK_EQ(root.degree(), std::size_t{2});
    CHECK_EQ(root.height(), std::size_t{2});

    node::TrieNode<char, int> copy(root);
    CHECK_EQ(copy.size(), std::size_t{3});
    CHECK(copy.child('a') != root.child('a'));
}

TEST_CASE("binary search tree insert and traversal")
{
    tree::BinarySearchTree<int> tree;
    for (int value : {5, 3, 8, 1, 4, 7, 9})
    {
        CHECK(tree.insert(value));
    }
    CHECK_FALSE(tree.insert(3));

    CHECK_EQ(tree.size(), std::size_t{7});
    CHECK_EQ(tree.height(), std::size_t{3});
    CHECK_EQ(*tree.min(), 1);
    CHECK_EQ(*tree.max(), 9);
    CHECK(tree.contains(7));
    CHECK_FALSE(tree.contains(6));
    CHECK_EQ(*tree.find(8), 8);
    CHECK(tree.find(42) == nullptr);

    const std::vector<int> expected{1, 3, 4, 5, 7, 8, 9};
    CHECK(tree.toVector() == expected);

    std::vector<int> ordered;
    tree.inorder([&ordered](int value) { ordered.push_back(value); });
    CHECK(ordered == expected);

    std::vector<int> preorder;
    tree.preorder([&preorder](int value) { preorder.push_back(value); });
    CHECK(preorder == std::vector<int>({5, 3, 1, 4, 8, 7, 9}));

    std::vector<int> postorder;
    tree.postorder([&postorder](int value) { postorder.push_back(value); });
    CHECK(postorder == std::vector<int>({1, 4, 3, 7, 9, 8, 5}));

    std::vector<int> levelorder;
    tree.levelorder([&levelorder](int value) { levelorder.push_back(value); });
    CHECK(levelorder == std::vector<int>({5, 3, 8, 1, 4, 7, 9}));
}

TEST_CASE("binary search tree erase variants")
{
    tree::BinarySearchTree<int> tree{5, 3, 8, 1, 4, 7, 9};

    CHECK(tree.erase(1));
    CHECK(tree.erase(3));
    CHECK(tree.erase(5));
    CHECK_FALSE(tree.erase(42));

    CHECK_EQ(tree.size(), std::size_t{4});
    CHECK_EQ(tree.root()->value, 7);
    CHECK(tree.toVector() == std::vector<int>({4, 7, 8, 9}));
}

TEST_CASE("binary search tree copy and move")
{
    tree::BinarySearchTree<int> original{2, 1, 3};

    tree::BinarySearchTree<int> copy(original);
    CHECK(copy.insert(4));
    CHECK_FALSE(original.contains(4));

    tree::BinarySearchTree<int> assigned;
    assigned = original;
    CHECK(assigned.insert(0));
    CHECK_FALSE(original.contains(0));

    tree::BinarySearchTree<int> moved(std::move(copy));
    CHECK(moved.contains(4));

    const std::vector<int> input{6, 4, 8};
    tree::BinarySearchTree<int> fromRange(input.begin(), input.end());
    CHECK_EQ(fromRange.size(), std::size_t{3});
    CHECK(fromRange.emplace(5));
    CHECK(fromRange.contains(5));
}

TEST_CASE("binary search tree custom comparator")
{
    tree::BinarySearchTree<int, std::greater<int>> tree{5, 3, 8, 1};

    CHECK(tree.toVector() == std::vector<int>({8, 5, 3, 1}));
    CHECK_EQ(*tree.min(), 8);
    CHECK_EQ(*tree.max(), 1);
}

TEST_CASE("trie insert find and prefix")
{
    trie::Trie<char, int> tree;
    CHECK(tree.insert("car", 1));
    CHECK(tree.insert("card", 2));
    CHECK(tree.insert("cart", 3));
    CHECK(tree.insert("cat", 4));
    CHECK(tree.insert("dog", 5));
    CHECK_FALSE(tree.insert("car", 99));

    CHECK_EQ(*tree.find("car"), 1);
    CHECK_EQ(tree.size(), std::size_t{5});
    CHECK(tree.contains("dog"));
    CHECK_FALSE(tree.contains("ca"));
    CHECK(tree.containsPrefix("ca"));
    CHECK_FALSE(tree.containsPrefix("cz"));
    CHECK(tree.containsPrefix(""));

    CHECK_EQ(tree.longestPrefixOf("cartoon").value(), std::string("cart"));
    CHECK_EQ(tree.longestPrefixOf("careful").value(), std::string("car"));
    CHECK_FALSE(tree.longestPrefixOf("zebra").has_value());

    const std::vector<std::string> expected{"car", "card", "cart", "cat", "dog"};
    CHECK(tree.keys() == expected);

    const std::vector<std::string> carKeys{"car", "card", "cart"};
    CHECK(tree.keysWithPrefix("car") == carKeys);

    const std::vector<std::string> caKeys{"car", "card", "cart", "cat"};
    CHECK(tree.keysWithPrefix("ca") == caKeys);
    CHECK(tree.keysWithPrefix("z").empty());

    CHECK_FALSE(tree.assign("car", 10));
    CHECK_EQ(*tree.find("car"), 10);
    CHECK(tree.assign("cow", 6));
    CHECK_EQ(tree.size(), std::size_t{6});

    CHECK(tree.erase("cart"));
    CHECK_FALSE(tree.contains("cart"));
    CHECK_FALSE(tree.erase("cart"));
    CHECK(tree.erase("car"));
    CHECK_FALSE(tree.contains("car"));
    CHECK(tree.contains("card"));
    CHECK_EQ(tree.size(), std::size_t{4});

    const std::vector<std::string> remainingCarKeys{"card"};
    CHECK(tree.keysWithPrefix("car") == remainingCarKeys);

    trie::Trie<char, int> copy(tree);
    CHECK(copy.assign("new", 7));
    CHECK_FALSE(tree.contains("new"));
}

TEST_CASE("trie empty key")
{
    trie::Trie<char, int> tree;
    CHECK(tree.insert("", 1));
    CHECK_FALSE(tree.insert("", 2));

    CHECK_EQ(*tree.find(""), 1);
    CHECK_EQ(tree.size(), std::size_t{1});
    CHECK(tree.containsPrefix(""));
    CHECK_EQ(tree.longestPrefixOf("anything").value(), std::string());

    const std::vector<std::string> keys = tree.keys();
    CHECK_EQ(keys.size(), std::size_t{1});
    CHECK(keys.front().empty());

    CHECK(tree.erase(""));
    CHECK(tree.empty());
    CHECK_FALSE(tree.containsPrefix(""));
}

TEST_CASE("trie default value type")
{
    trie::Trie<> tree;
    CHECK(tree.insert("alpha"));
    CHECK(tree.insert("alphabet"));
    CHECK(tree.contains("alpha"));
    CHECK_FALSE(tree.contains("alph"));
    CHECK(tree.erase("alpha"));
    CHECK(tree.contains("alphabet"));
}

TEST_CASE("character search tree default value type")
{
    character::CharacterSearchTree<> tree;
    CHECK(tree.insert("alpha"));
    CHECK(tree.insert("alphabet"));
    CHECK(tree.contains("alpha"));
    CHECK_FALSE(tree.contains("alph"));
    CHECK(tree.erase("alpha"));
    CHECK(tree.contains("alphabet"));
}

TEST_CASE("trie unicode keys")
{
    trie::Trie<char32_t, int> tree;
    CHECK(tree.insert(U"\u4F60\u597D", 1));
    CHECK(tree.insert(U"\u4F60\u597D\u4E16\u754C", 2));

    CHECK_EQ(*tree.find(U"\u4F60\u597D"), 1);
    CHECK_EQ(tree.size(), std::size_t{2});

    const std::vector<std::u32string> keys = tree.keys();
    CHECK_EQ(keys.size(), std::size_t{2});
    CHECK(keys[0] == std::u32string(U"\u4F60\u597D"));
    CHECK(keys[1] == std::u32string(U"\u4F60\u597D\u4E16\u754C"));

    const std::vector<std::u32string> prefixed = tree.keysWithPrefix(U"\u4F60\u597D");
    CHECK_EQ(prefixed.size(), std::size_t{2});
}

TEST_CASE("segment tree sum and point updates")
{
    const std::vector<int> values{1, 2, 3, 4, 5};
    segment::SegmentTree<int> tree(values.begin(), values.end(), 0);

    CHECK_EQ(tree.size(), std::size_t{5});
    CHECK_EQ(tree.query(0, 5), 15);
    CHECK_EQ(tree.query(1, 4), 9);
    CHECK_EQ(tree.query(2, 2), 0);
    CHECK_EQ(tree.query(0, 0), 0);
    CHECK_EQ(tree.query(), 15);
    CHECK_EQ(tree.identity(), 0);
    CHECK_EQ(tree.operation()(2, 3), 5);

    tree.set(2, 10);
    CHECK_EQ(tree.at(2), 10);
    CHECK_EQ(tree.query(1, 4), 16);
    CHECK_EQ(tree.query(), 22);

    tree.update(0, [](int value) { return value + 1; });
    CHECK_EQ(tree.at(0), 2);
    CHECK_EQ(tree.query(), 23);

    CHECK_THROWS_AS(tree.at(5), std::out_of_range);
    CHECK_THROWS_AS(tree.set(5, 0), std::out_of_range);
    CHECK_THROWS_AS(tree.query(3, 2), std::out_of_range);
    CHECK_THROWS_AS(tree.query(0, 6), std::out_of_range);
}

TEST_CASE("segment tree custom operation")
{
    const std::vector<int> values{4, 2, 7, 1, 5};

    struct Minimum
    {
        int operator()(int left, int right) const { return left < right ? left : right; }
    };

    segment::SegmentTree<int, Minimum> minimum(values.begin(), values.end(),
                                               std::numeric_limits<int>::max());
    CHECK_EQ(minimum.query(1, 4), 1);
    CHECK_EQ(minimum.query(), 1);
    CHECK_EQ(minimum.query(3, 3), std::numeric_limits<int>::max());

    struct Concatenate
    {
        std::string operator()(const std::string &left, const std::string &right) const
        {
            return left + right;
        }
    };

    const std::vector<std::string> pieces{"a", "b", "c", "d"};
    segment::SegmentTree<std::string, Concatenate> text(pieces.begin(), pieces.end(), std::string{});
    CHECK_EQ(text.query(1, 3), std::string("bc"));
    CHECK_EQ(text.query(), std::string("abcd"));

    text.set(2, "X");
    CHECK_EQ(text.query(1, 4), std::string("bXd"));
}

TEST_CASE("segment tree constructors and empty state")
{
    segment::SegmentTree<int> listed({1, 2, 3});
    CHECK_EQ(listed.query(), 6);

    segment::SegmentTree<int> zeros(3, 0);
    CHECK_EQ(zeros.query(), 0);
    zeros.set(1, 5);
    CHECK_EQ(zeros.query(), 5);

    segment::SegmentTree<int> empty;
    CHECK(empty.empty());
    CHECK_EQ(empty.size(), std::size_t{0});
    CHECK_EQ(empty.query(), 0);
    CHECK_THROWS_AS(empty.at(0), std::out_of_range);
}

TEST_CASE("character search tree exact and prefix")
{
    character::CharacterSearchTree<int> tree;
    CHECK(tree.insert("car", 1));
    CHECK(tree.insert("card", 2));
    CHECK(tree.insert("care", 3));
    CHECK(tree.insert("cat", 4));
    CHECK(tree.insert("dog", 5));
    CHECK_FALSE(tree.insert("car", 99));

    CHECK_EQ(*tree.find("car"), 1);
    CHECK_EQ(tree.size(), std::size_t{5});
    CHECK(tree.contains("care"));
    CHECK_FALSE(tree.contains("ca"));
    CHECK(tree.containsPrefix("ca"));
    CHECK_FALSE(tree.containsPrefix("cz"));

    CHECK_EQ(tree.longestPrefixOf("careful").value(), std::string("care"));
    CHECK_FALSE(tree.longestPrefixOf("zebra").has_value());

    const std::vector<std::string> expected{"car", "card", "care", "cat", "dog"};
    CHECK(tree.keys() == expected);

    const std::vector<std::string> carKeys{"car", "card", "care"};
    CHECK(tree.keysWithPrefix("car") == carKeys);

    const std::vector<std::string> catKeys{"cat"};
    CHECK(tree.keysWithPrefix("cat") == catKeys);
    CHECK(tree.keysWithPrefix("z").empty());

    CHECK_FALSE(tree.assign("car", 10));
    CHECK_EQ(*tree.find("car"), 10);
    CHECK(tree.assign("cow", 6));

    CHECK(tree.erase("care"));
    CHECK_FALSE(tree.contains("care"));
    CHECK_FALSE(tree.erase("care"));
    CHECK_EQ(tree.size(), std::size_t{5});

    const std::vector<std::string> remaining{"car", "card", "cat", "cow", "dog"};
    CHECK(tree.keys() == remaining);

    const std::vector<std::string> remainingCarKeys{"car", "card"};
    CHECK(tree.keysWithPrefix("car") == remainingCarKeys);

    character::CharacterSearchTree<int> copy(tree);
    CHECK(copy.assign("zebra", 7));
    CHECK_FALSE(tree.contains("zebra"));
}

TEST_CASE("character search tree wildcard match")
{
    character::CharacterSearchTree<int> tree{"car", "card", "care", "cat", "dog"};

    CHECK(tree.match("ca?") == std::vector<std::string>({"car", "cat"}));
    CHECK(tree.match("ca*") == std::vector<std::string>({"car", "card", "care", "cat"}));
    CHECK(tree.match("c?r") == std::vector<std::string>({"car"}));
    CHECK(tree.match("d*g") == std::vector<std::string>({"dog"}));
    CHECK(tree.match("*") == std::vector<std::string>({"car", "card", "care", "cat", "dog"}));
    CHECK(tree.match("x*").empty());

    CHECK(character::CharacterSearchTree<int>::matchesPattern("abc", "a?c"));
    CHECK(character::CharacterSearchTree<int>::matchesPattern("ac", "a*c"));
    CHECK(character::CharacterSearchTree<int>::matchesPattern("abc", "*"));
    CHECK_FALSE(character::CharacterSearchTree<int>::matchesPattern("abc", "a*d"));
}

TEST_CASE("character search tree empty key")
{
    character::CharacterSearchTree<int> tree;
    CHECK(tree.insert("", 0));
    CHECK(tree.insert("a", 1));
    CHECK_FALSE(tree.insert("", 2));

    CHECK_EQ(*tree.find(""), 0);
    CHECK_EQ(tree.size(), std::size_t{2});
    CHECK_EQ(tree.longestPrefixOf("anything").value(), std::string("a"));
    CHECK_EQ(tree.longestPrefixOf("xyz").value(), std::string());
    CHECK(tree.containsPrefix(""));

    const std::vector<std::string> keys = tree.keys();
    CHECK_EQ(keys.size(), std::size_t{2});
    CHECK(keys.front().empty());
    CHECK(keys.back() == std::string("a"));

    CHECK(tree.erase(""));
    CHECK_FALSE(tree.contains(""));
    CHECK_EQ(tree.size(), std::size_t{1});
}

TEST_CASE("character search tree copy and move")
{
    character::CharacterSearchTree<int> original;
    original.insert("alpha", 1);
    original.insert("beta", 2);

    character::CharacterSearchTree<int> copy(original);
    CHECK(copy.assign("gamma", 3));
    CHECK_FALSE(original.contains("gamma"));

    character::CharacterSearchTree<int> assigned;
    assigned = original;
    CHECK(assigned.insert("delta", 4));
    CHECK_FALSE(original.contains("delta"));

    character::CharacterSearchTree<int> moved(std::move(copy));
    CHECK(moved.contains("gamma"));

    CHECK(original.erase("alpha"));
    CHECK_FALSE(original.containsPrefix("alph"));
}
