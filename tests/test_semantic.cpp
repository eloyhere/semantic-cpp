#include "semantic/semantic.h"

#include "test_framework.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace
{
bool same(const ::bunch::Bunch &sequence, std::string_view text)
{
    return sequence == text;
}

// toUnordered() gives no iteration order guarantee, so assertions compare the
// sorted multiset instead of relying on the standard library's layout.
template <typename T>
std::vector<T> sorted(std::vector<T> values)
{
    std::sort(values.begin(), values.end());
    return values;
}
} // namespace

TEST_CASE("semantic range and factory operations")
{
    const std::vector<int> values = semantic::range(0, 5).map([](int value) { return value * 2; }).filter([](int value) { return value > 3; }).toUnordered().toVector();
    CHECK_EQ(sorted(values), (std::vector<int>{4, 6, 8}));

    const std::vector<int> closed = semantic::rangeClosed(1, 3).toUnordered().toVector();
    CHECK_EQ(sorted(closed), (std::vector<int>{1, 2, 3}));

    const std::vector<int> stepped = semantic::range(0, 10, 3).toUnordered().toVector();
    CHECK_EQ(sorted(stepped), (std::vector<int>{0, 3, 6, 9}));

    const std::vector<int> repeated = semantic::repeat(7, 3).toUnordered().toVector();
    CHECK_EQ(sorted(repeated), (std::vector<int>{7, 7, 7}));

    const std::vector<int> from = semantic::from(std::vector<int>{3, 1, 2}).toUnordered().toVector();
    CHECK_EQ(sorted(from), (std::vector<int>{1, 2, 3}));

    const std::vector<int> generated = semantic::generate([] { return 5; }, 2).toUnordered().toVector();
    CHECK_EQ(sorted(generated), (std::vector<int>{5, 5}));

    const std::vector<int> iterated = semantic::iterate(1, [](int value) { return value * 2; }, 4).toUnordered().toVector();
    CHECK_EQ(sorted(iterated), (std::vector<int>{1, 2, 4, 8}));
}

TEST_CASE("semantic collectable terminal operations")
{
    auto collectable = semantic::range(1, 5).toUnordered();

    CHECK_EQ(collectable.count(), 4u);
    CHECK_FALSE(collectable.empty());
    CHECK_EQ(collectable.summate<int>(), 10);
    CHECK_EQ(collectable.average<double>(), 2.5);
    CHECK_EQ(collectable.range<int>(), 3);
    CHECK(collectable.allMatch([](int value) { return value < 5; }));
    CHECK(collectable.anyMatch([](int value) { return value == 3; }));
    CHECK(collectable.noneMatch([](int value) { return value > 10; }));

    // An unordered collectable has no "first" element, but it must still return
    // one of the collected values. Order-dependent semantics use toOrdered().
    const auto anyFirst = collectable.findFirst();
    CHECK(anyFirst.has_value());
    CHECK(*anyFirst >= 1 && *anyFirst <= 4);

    const auto ordered = semantic::range(1, 5).toOrdered();
    const auto first = ordered.findFirst();
    CHECK(first.has_value());
    CHECK_EQ(*first, 1);

    const auto last = ordered.findLast();
    CHECK(last.has_value());
    CHECK_EQ(*last, 4);

    const auto minimum = collectable.findMinimum();
    CHECK(minimum.has_value());
    CHECK_EQ(*minimum, 1);

    const auto maximum = collectable.findMaximum();
    CHECK(maximum.has_value());
    CHECK_EQ(*maximum, 4);

    const std::vector<int> sortedValues = semantic::range(0, 5).sort().toVector();
    CHECK_EQ(sortedValues, (std::vector<int>{0, 1, 2, 3, 4}));

    const std::vector<int> orderedValues = semantic::range(0, 5).toOrdered().toVector();
    CHECK_EQ(orderedValues, (std::vector<int>{0, 1, 2, 3, 4}));
}

TEST_CASE("semantic text and character sequence factories")
{
    const auto parts = semantic::text(std::string("a,b,c"), ',').toUnordered().toVector();
    CHECK_EQ(parts.size(), static_cast<std::size_t>(3));
    CHECK_EQ(std::count_if(parts.begin(), parts.end(), [](const ::bunch::Bunch &part) { return same(part, "a"); }), 1);
    CHECK_EQ(std::count_if(parts.begin(), parts.end(), [](const ::bunch::Bunch &part) { return same(part, "b"); }), 1);
    CHECK_EQ(std::count_if(parts.begin(), parts.end(), [](const ::bunch::Bunch &part) { return same(part, "c"); }), 1);

    // join() concatenates in encounter order, so it uses the ordered collectable.
    const ::bunch::Bunch bracketed = semantic::text(std::string("a,b,c"), ',').toOrdered().join(::bunch::Bunch("-"));
    CHECK(same(bracketed, "[a-b-c]"));

    const std::vector<::bunch::Point> points = semantic::sequence(std::string("A\xE2\x82\xAC")).toUnordered().toVector();
    const std::vector<::bunch::Point> pointsByValue = sorted(points);
    CHECK_EQ(pointsByValue.size(), static_cast<std::size_t>(2));
    CHECK_EQ(pointsByValue[0].getValue(), 0x41u);
    CHECK_EQ(pointsByValue[1].getValue(), 0x20ACu);

    const std::vector<char> bytes = semantic::blob(std::string("abc")).toUnordered().toVector();
    CHECK_EQ(sorted(bytes), (std::vector<char>{'a', 'b', 'c'}));

    const std::vector<::bunch::Bunch> whole = semantic::bunch(::bunch::Bunch("hello")).toUnordered().toVector();
    CHECK_EQ(whole.size(), static_cast<std::size_t>(1));
    CHECK(same(whole[0], "hello"));
}

TEST_CASE("semantic parallel pipeline")
{
    const std::vector<int> values = semantic::range(0, 100).parallel(4).map([](int value) { return value + 1; }).toUnordered().toVector();
    CHECK_EQ(values.size(), static_cast<std::size_t>(100));

    int sum = 0;
    for (int value : values)
    {
        sum += value;
    }
    CHECK_EQ(sum, 5050);
}
