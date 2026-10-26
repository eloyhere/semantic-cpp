#include "structure/ast.h"

#include "test_framework.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
enum class Kind
{
    Program,
    Declaration,
    Identifier,
    Literal,
    Expression
};
} // namespace

TEST_CASE("ast source positions and ranges")
{
    const ast::SourcePosition position{4, 2, 3};
    CHECK_EQ(position.offset, std::size_t{4});
    CHECK_EQ(position.line, std::size_t{2});
    CHECK_EQ(position.column, std::size_t{3});

    const ast::SourceRange range{{4, 2, 3}, {9, 2, 8}};
    CHECK_FALSE(range.empty());
    CHECK_EQ(range.length(), std::size_t{5});
    CHECK(range.contains(ast::SourcePosition{4, 2, 3}));
    CHECK(range.contains(ast::SourcePosition{8, 2, 7}));
    CHECK_FALSE(range.contains(ast::SourcePosition{9, 2, 8}));

    const ast::SourceRange emptyRange;
    CHECK(emptyRange.empty());
    CHECK_EQ(emptyRange.length(), std::size_t{0});
    CHECK(emptyRange == ast::SourceRange{});
}

TEST_CASE("ast construction and navigation")
{
    const ast::SourceRange programRange{{0, 1, 1}, {20, 2, 1}};
    ast::Tree<Kind> tree(Kind::Program, std::string(), programRange);

    CHECK_FALSE(tree.empty());
    CHECK_EQ(tree.size(), std::size_t{1});
    CHECK_EQ(tree.height(), std::size_t{1});
    CHECK_EQ(tree.root()->kind(), Kind::Program);
    CHECK(tree.root()->isRoot());
    CHECK(tree.root()->isLeaf());
    CHECK_EQ(tree.root()->range(), programRange);

    ast::Node<Kind> &declaration = tree.root()->appendChild(Kind::Declaration, "let", {{0, 1, 1}, {12, 1, 13}});
    ast::Node<Kind> &identifier = declaration.appendChild(Kind::Identifier, "answer", {{4, 1, 5}, {10, 1, 11}});
    ast::Node<Kind> &literal = declaration.appendChild(Kind::Literal, "42", {{11, 1, 12}, {13, 1, 14}});

    CHECK_EQ(tree.size(), std::size_t{4});
    CHECK_EQ(tree.height(), std::size_t{3});
    CHECK_EQ(tree.root()->childCount(), std::size_t{1});
    CHECK_EQ(declaration.childCount(), std::size_t{2});
    CHECK_EQ(declaration.size(), std::size_t{3});
    CHECK_EQ(declaration.descendantCount(), std::size_t{2});
    CHECK_FALSE(tree.root()->isLeaf());
    CHECK(identifier.isLeaf());
    CHECK_EQ(identifier.parent(), &declaration);
    CHECK_EQ(declaration.parent(), tree.root());
    CHECK_EQ(identifier.root(), tree.root());
    CHECK_EQ(identifier.depth(), std::size_t{2});
    CHECK_EQ(literal.depth(), std::size_t{2});

    std::size_t childrenSeen = 0;
    for (const auto &child : declaration.children())
    {
        CHECK_EQ(child->parent(), &declaration);
        ++childrenSeen;
    }
    CHECK_EQ(childrenSeen, std::size_t{2});

    const std::vector<ast::Node<Kind> *> path = identifier.path();
    CHECK_EQ(path.size(), std::size_t{3});
    CHECK_EQ(path.front(), tree.root());
    CHECK_EQ(path.back(), &identifier);

    const std::vector<ast::Node<Kind> *> ancestors = identifier.ancestors();
    CHECK_EQ(ancestors.size(), std::size_t{2});
    CHECK_EQ(ancestors.front(), &declaration);
    CHECK_EQ(ancestors.back(), tree.root());

    const auto index = declaration.childIndex(identifier);
    CHECK(index.has_value());
    CHECK_EQ(*index, std::size_t{0});
    CHECK_EQ(declaration.child(0), &identifier);
    CHECK_EQ(declaration.child(1), &literal);
    CHECK_EQ(declaration.child(2), nullptr);
    CHECK_THROWS_AS(declaration.at(2), std::out_of_range);
}

TEST_CASE("ast traversals")
{
    ast::Tree<Kind> tree(Kind::Program);
    ast::Node<Kind> &declaration = tree.root()->appendChild(Kind::Declaration);
    declaration.appendChild(Kind::Identifier, "answer");
    declaration.appendChild(Kind::Literal, "42");
    ast::Node<Kind> &expression = tree.root()->appendChild(Kind::Expression);
    expression.appendChild(Kind::Literal, "7");

    std::vector<Kind> preorder;
    tree.preorder([&preorder](const ast::Node<Kind> &node) { preorder.push_back(node.kind()); });
    CHECK(preorder == std::vector<Kind>({Kind::Program, Kind::Declaration, Kind::Identifier, Kind::Literal,
                                         Kind::Expression, Kind::Literal}));

    std::vector<Kind> postorder;
    tree.postorder([&postorder](const ast::Node<Kind> &node) { postorder.push_back(node.kind()); });
    CHECK(postorder == std::vector<Kind>({Kind::Identifier, Kind::Literal, Kind::Declaration, Kind::Literal,
                                          Kind::Expression, Kind::Program}));

    std::vector<Kind> levelorder;
    tree.levelorder([&levelorder](const ast::Node<Kind> &node) { levelorder.push_back(node.kind()); });
    CHECK(levelorder == std::vector<Kind>({Kind::Program, Kind::Declaration, Kind::Expression, Kind::Identifier,
                                           Kind::Literal, Kind::Literal}));

    std::vector<std::string> enterLeave;
    tree.visit([&enterLeave](const ast::Node<Kind> &node) { enterLeave.push_back("enter:" + node.value()); },
               [&enterLeave](const ast::Node<Kind> &node) { enterLeave.push_back("leave:" + node.value()); });
    CHECK_EQ(enterLeave.size(), std::size_t{12});
    CHECK_EQ(enterLeave.front(), std::string("enter:"));
    CHECK_EQ(enterLeave.back(), std::string("leave:"));

    const ast::Tree<Kind> &constTree = tree;
    std::vector<Kind> constPreorder;
    constTree.preorder([&constPreorder](const ast::Node<Kind> &node) { constPreorder.push_back(node.kind()); });
    CHECK(constPreorder == preorder);

    std::vector<Kind> constPostorder;
    constTree.postorder([&constPostorder](const ast::Node<Kind> &node) { constPostorder.push_back(node.kind()); });
    CHECK(constPostorder == postorder);

    std::vector<Kind> constLevelorder;
    constTree.levelorder([&constLevelorder](const ast::Node<Kind> &node) {
        constLevelorder.push_back(node.kind());
    });
    CHECK(constLevelorder == levelorder);

    std::size_t constVisits = 0;
    constTree.visit([&constVisits](const ast::Node<Kind> &) { ++constVisits; },
                    [&constVisits](const ast::Node<Kind> &) { ++constVisits; });
    CHECK_EQ(constVisits, std::size_t{12});
}

TEST_CASE("ast queries")
{
    ast::Tree<Kind> tree(Kind::Program);
    ast::Node<Kind> &declaration = tree.root()->appendChild(Kind::Declaration);
    declaration.appendChild(Kind::Identifier, "first");
    declaration.appendChild(Kind::Literal, "42");
    ast::Node<Kind> &expression = tree.root()->appendChild(Kind::Expression);
    expression.appendChild(Kind::Identifier, "second");

    ast::Node<Kind> *identifier = tree.find([](const ast::Node<Kind> &node) {
        return node.kind() == Kind::Identifier;
    });
    CHECK(identifier != nullptr);
    CHECK_EQ(identifier->value(), std::string("first"));

    const ast::Tree<Kind> &constTree = tree;
    const ast::Node<Kind> *constIdentifier = constTree.find([](const ast::Node<Kind> &node) {
        return node.kind() == Kind::Identifier && node.value() == "second";
    });
    CHECK(constIdentifier != nullptr);
    CHECK_EQ(constIdentifier->value(), std::string("second"));

    const std::vector<ast::Node<Kind> *> literals = tree.findAll([](const ast::Node<Kind> &node) {
        return node.kind() == Kind::Literal;
    });
    CHECK_EQ(literals.size(), std::size_t{1});
    CHECK_EQ(literals.front()->value(), std::string("42"));

    const std::vector<const ast::Node<Kind> *> constLiterals = constTree.findAll([](const ast::Node<Kind> &node) {
        return node.kind() == Kind::Literal;
    });
    CHECK_EQ(constLiterals.size(), std::size_t{1});
    CHECK_EQ(constLiterals.front()->value(), std::string("42"));

    ast::Node<Kind> *missing = tree.find([](const ast::Node<Kind> &node) {
        return node.kind() == Kind::Literal && node.value() == "99";
    });
    CHECK(missing == nullptr);
}

TEST_CASE("ast mutation keeps parent links consistent")
{
    using NodeType = ast::Node<Kind>;

    ast::Tree<Kind> tree(Kind::Program);
    NodeType &declaration = tree.root()->appendChild(Kind::Declaration);
    NodeType &first = declaration.appendChild(Kind::Identifier, "a");
    declaration.appendChild(Kind::Identifier, "b");
    NodeType &inserted = declaration.insertChild(1, Kind::Identifier, "c");

    CHECK_EQ(declaration.childCount(), std::size_t{3});
    CHECK_EQ(declaration.child(0), &first);
    CHECK_EQ(declaration.child(1), &inserted);
    CHECK_EQ(inserted.parent(), &declaration);

    std::unique_ptr<NodeType> replaced =
        declaration.replaceChild(0, std::make_unique<NodeType>(Kind::Literal, "1"));
    CHECK_EQ(declaration.child(0)->kind(), Kind::Literal);
    CHECK(replaced != nullptr);
    CHECK_EQ(replaced->kind(), Kind::Identifier);
    CHECK_EQ(replaced->value(), std::string("a"));
    CHECK(replaced->parent() == nullptr);

    std::unique_ptr<NodeType> detached = declaration.detachChildAt(1);
    CHECK(detached != nullptr);
    CHECK_EQ(detached->value(), std::string("c"));
    CHECK(detached->parent() == nullptr);
    CHECK_EQ(declaration.childCount(), std::size_t{2});

    declaration.appendChild(std::move(detached));
    CHECK_EQ(declaration.child(2)->value(), std::string("c"));
    CHECK_EQ(declaration.child(2)->parent(), &declaration);

    CHECK(declaration.eraseChildAt(0));
    CHECK_EQ(declaration.childCount(), std::size_t{2});

    NodeType &removable = declaration.appendChild(Kind::Identifier, "d");
    CHECK(declaration.eraseChild(&removable));
    CHECK_EQ(declaration.childCount(), std::size_t{2});
    CHECK_FALSE(declaration.eraseChild(static_cast<NodeType *>(nullptr)));

    CHECK_THROWS_AS(declaration.at(2), std::out_of_range);
    CHECK_THROWS_AS(declaration.detachChildAt(static_cast<std::size_t>(10)), std::out_of_range);
    CHECK_THROWS_AS(declaration.appendChild(std::unique_ptr<NodeType>()), std::invalid_argument);
    CHECK_THROWS_AS(declaration.detachChild(static_cast<NodeType *>(nullptr)), std::invalid_argument);

    declaration.clearChildren();
    CHECK(declaration.isLeaf());
}

TEST_CASE("ast copy and move are deep")
{
    ast::Tree<Kind> original(Kind::Program);
    ast::Node<Kind> &declaration = original.root()->appendChild(Kind::Declaration);
    declaration.appendChild(Kind::Identifier, "x");
    declaration.appendChild(Kind::Literal, "1");

    ast::Tree<Kind> copy(original);
    CHECK(copy.root() != original.root());
    CHECK_EQ(copy.size(), original.size());
    CHECK_EQ(copy.root()->child(0)->parent(), copy.root());
    CHECK(copy.root()->child(0) != original.root()->child(0));

    copy.root()->child(0)->appendChild(Kind::Identifier, "y");
    CHECK_EQ(original.root()->child(0)->childCount(), std::size_t{2});
    CHECK_EQ(copy.root()->child(0)->childCount(), std::size_t{3});

    ast::Tree<Kind> assigned;
    assigned = original;
    CHECK_EQ(assigned.size(), original.size());
    CHECK(assigned.root() != original.root());
    CHECK_EQ(assigned.root()->child(0)->parent(), assigned.root());

    ast::Tree<Kind> moved(std::move(copy));
    CHECK_EQ(moved.size(), std::size_t{5});
    CHECK(copy.empty());
    CHECK_EQ(moved.root()->child(0)->parent(), moved.root());

    ast::Node<Kind> detached = *original.root()->child(0);
    CHECK(detached.parent() == nullptr);
    CHECK_EQ(detached.child(0)->parent(), &detached);
    CHECK_EQ(detached.size(), std::size_t{3});

    ast::Node<Kind> movedNode(std::move(detached));
    CHECK_EQ(movedNode.child(0)->parent(), &movedNode);
    CHECK_EQ(movedNode.size(), std::size_t{3});

    ast::Node<Kind> assignedNode(Kind::Expression);
    assignedNode = std::move(movedNode);
    CHECK_EQ(assignedNode.kind(), Kind::Declaration);
    CHECK_EQ(assignedNode.child(0)->parent(), &assignedNode);

    ast::Node<Kind> &attached = original.root()->appendChild(Kind::Expression);
    attached = *original.root()->child(0);
    CHECK_EQ(attached.parent(), original.root());
    CHECK_EQ(attached.child(0)->parent(), &attached);
    CHECK_EQ(attached.child(0)->value(), std::string("x"));
}

TEST_CASE("ast default string nodes and empty tree")
{
    ast::Tree<> empty;
    CHECK(empty.empty());
    CHECK_EQ(empty.size(), std::size_t{0});
    CHECK_EQ(empty.height(), std::size_t{0});
    CHECK(empty.root() == nullptr);

    std::vector<std::string> visited;
    empty.preorder([&visited](const ast::Node<> &node) { visited.push_back(node.value()); });
    CHECK(visited.empty());
    CHECK(empty.find([](const ast::Node<> &) { return true; }) == nullptr);
    CHECK_THROWS_AS(empty.atRoot(), std::logic_error);

    ast::Tree<> dynamic("program");
    ast::Node<> &identifier = dynamic.root()->appendChild("identifier", "x");
    CHECK_EQ(dynamic.root()->kind(), std::string("program"));
    CHECK_EQ(identifier.kind(), std::string("identifier"));
    CHECK_EQ(identifier.value(), std::string("x"));
    CHECK_EQ(dynamic.size(), std::size_t{2});

    std::unique_ptr<ast::Node<>> root = std::make_unique<ast::Node<>>("root");
    empty.setRoot(std::move(root));
    CHECK_FALSE(empty.empty());
    CHECK_EQ(empty.root()->kind(), std::string("root"));

    std::unique_ptr<ast::Node<>> released = empty.releaseRoot();
    CHECK(empty.empty());
    CHECK(released != nullptr);
    CHECK(released->parent() == nullptr);

    ast::Tree<> constructed(std::move(released));
    CHECK_EQ(constructed.size(), std::size_t{1});
    constructed.clear();
    CHECK(constructed.empty());
}
