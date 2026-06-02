#include "semantics.h"
#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <queue>
#include <stack>
#include <array>
#include <string>

int main()
{
    std::cout << "=== Test 1: Semantic<vector<int>> ===" << std::endl;
    auto vecStream = semantic::useOf({std::vector<int>{1, 2, 3, 4, 5}})
                         .map([](std::vector<int> v) { return static_cast<int>(v.size()); })
                         .toOrdered()
                         .toVector();
    std::cout << "Vector sizes: ";
    for (auto v : vecStream)
        std::cout << v << " ";
    std::cout << std::endl;

    auto vecFiltered = semantic::useOf({std::vector<int>{1, 2, 3},
                                        std::vector<int>{4, 5},
                                        std::vector<int>{6, 7, 8, 9}})
                           .filter([](std::vector<int> v) { return v.size() > 2; })
                           .toOrdered()
                           .toVector();
    std::cout << "Filtered vectors (size>2): " << vecFiltered.size() << std::endl;

    auto vecSorted = semantic::useOf({std::vector<int>{3, 1, 2},
                                      std::vector<int>{1, 2},
                                      std::vector<int>{5, 4}})
                         .sort()
                         .toVector();
    std::cout << "Sorted vectors count: " << vecSorted.size() << std::endl;

    auto vecDistinct = semantic::useOf({std::vector<int>{1, 2},
                                        std::vector<int>{1, 2},
                                        std::vector<int>{3, 4}})
                           .distinct()
                           .toOrdered()
                           .toVector();
    std::cout << "Distinct vectors count: " << vecDistinct.size() << std::endl;

    auto vecTakeWhile = semantic::useOf({std::vector<int>{1},
                                         std::vector<int>{1, 2},
                                         std::vector<int>{1, 2, 3},
                                         std::vector<int>{1},
                                         std::vector<int>{1, 2}})
                            .takeWhile([](std::vector<int> v) { return v.size() < 3; })
                            .toOrdered()
                            .toVector();
    std::cout << "takeWhile size<3 count: " << vecTakeWhile.size() << std::endl;

    auto vecDropWhile = semantic::useOf({std::vector<int>{1},
                                         std::vector<int>{1, 2},
                                         std::vector<int>{1, 2, 3},
                                         std::vector<int>{4, 5, 6, 7}})
                            .dropWhile([](std::vector<int> v) { return v.size() < 3; })
                            .toOrdered()
                            .toVector();
    std::cout << "dropWhile size<3 count: " << vecDropWhile.size() << std::endl;

    auto vecReversed = semantic::useOf({std::vector<int>{1},
                                        std::vector<int>{2, 3},
                                        std::vector<int>{4, 5, 6}})
                           .reverse()
                           .toOrdered()
                           .toVector();
    std::cout << "Reversed first size: " << vecReversed[0].size() << std::endl;

    auto vecLimited = semantic::useOf({std::vector<int>{1},
                                       std::vector<int>{2},
                                       std::vector<int>{3},
                                       std::vector<int>{4},
                                       std::vector<int>{5}})
                          .limit(3)
                          .toOrdered()
                          .toVector();
    std::cout << "Limited to 3 count: " << vecLimited.size() << std::endl;

    auto vecSkip = semantic::useOf({std::vector<int>{1},
                                    std::vector<int>{2},
                                    std::vector<int>{3},
                                    std::vector<int>{4}})
                       .skip(2)
                       .toOrdered()
                       .toVector();
    std::cout << "Skip 2 count: " << vecSkip.size() << std::endl;

    auto vecSub = semantic::useOf({std::vector<int>{0},
                                   std::vector<int>{1},
                                   std::vector<int>{2},
                                   std::vector<int>{3},
                                   std::vector<int>{4}})
                      .sub(1, 4)
                      .toOrdered()
                      .toVector();
    std::cout << "Sub 1..4 count: " << vecSub.size() << std::endl;

    auto vecConcat = semantic::useOf({std::vector<int>{1, 2}})
                         .concatenate(semantic::useOf({std::vector<int>{3, 4}}))
                         .toOrdered()
                         .toVector();
    std::cout << "Concatenated count: " << vecConcat.size() << std::endl;

    auto vecConcatContainer = semantic::useOf({std::vector<int>{1}})
                                  .concatenate(std::vector<std::vector<int>>{std::vector<int>{2}, std::vector<int>{3}})
                                  .toOrdered()
                                  .toVector();
    std::cout << "Concat with container count: " << vecConcatContainer.size() << std::endl;

    auto vecUnordered = semantic::useOf({std::vector<int>{1, 2, 3}}).toUnordered().count();
    std::cout << "Unordered count: " << vecUnordered << std::endl;

    auto vecWindow = semantic::useOf({std::vector<int>{1},
                                      std::vector<int>{2},
                                      std::vector<int>{3},
                                      std::vector<int>{4},
                                      std::vector<int>{5}})
                         .toWindow()
                         .tumble(2)
                         .toUnordered()
                         .count();
    std::cout << "Window tumble 2 count: " << vecWindow << std::endl;

    auto vecStats = semantic::useOf({std::vector<int>{1, 2},
                                     std::vector<int>{3, 4, 5},
                                     std::vector<int>{6}})
                        .toStatistics<double>()
                        .average([](std::vector<int> v) -> double { return static_cast<double>(v.size()); });
    std::cout << "Average vector size: " << vecStats << std::endl;

    std::cout << "=== vector passed ===" << std::endl
              << std::endl;

    std::cout << "=== Test 2: Semantic<list<int>> ===" << std::endl;
    auto listStream = semantic::useOf({std::list<int>{10, 20, 30}})
                          .map([](std::list<int> l) { return static_cast<int>(l.size()); })
                          .toOrdered()
                          .toVector();
    std::cout << "List sizes: ";
    for (auto v : listStream)
        std::cout << v << " ";
    std::cout << std::endl;

    auto listFiltered = semantic::useOf({std::list<int>{1, 2, 3},
                                         std::list<int>{4},
                                         std::list<int>{5, 6}})
                            .filter([](std::list<int> l) { return l.size() > 1; })
                            .toOrdered()
                            .toVector();
    std::cout << "Filtered lists count: " << listFiltered.size() << std::endl;

    auto listDistinct = semantic::useOf({std::list<int>{1, 2},
                                         std::list<int>{1, 2},
                                         std::list<int>{3}})
                            .distinct()
                            .toOrdered()
                            .toVector();
    std::cout << "Distinct lists count: " << listDistinct.size() << std::endl;

    std::cout << "=== list passed ===" << std::endl
              << std::endl;

    std::cout << "=== Test 3: Semantic<deque<int>> ===" << std::endl;
    auto dequeStream = semantic::useOf({std::deque<int>{1, 2, 3},
                                        std::deque<int>{4, 5}})
                           .map([](std::deque<int> d) { return static_cast<int>(d.size()); })
                           .toOrdered()
                           .toVector();
    std::cout << "Deque sizes: ";
    for (auto v : dequeStream)
        std::cout << v << " ";
    std::cout << std::endl;

    auto dequeFiltered = semantic::useOf({std::deque<int>{1},
                                          std::deque<int>{2, 3},
                                          std::deque<int>{4, 5, 6}})
                             .filter([](std::deque<int> d) { return d.size() >= 2; })
                             .toOrdered()
                             .toVector();
    std::cout << "Filtered deques count: " << dequeFiltered.size() << std::endl;

    std::cout << "=== deque passed ===" << std::endl
              << std::endl;

    std::cout << "=== Test 4: Semantic<set<int>> ===" << std::endl;
    auto setStream = semantic::useOf({std::set<int>{1, 2, 3},
                                      std::set<int>{4, 5}})
                         .map([](std::set<int> s) { return static_cast<int>(s.size()); })
                         .toOrdered()
                         .toVector();
    std::cout << "Set sizes: ";
    for (auto v : setStream)
        std::cout << v << " ";
    std::cout << std::endl;

    auto setSorted = semantic::useOf({std::set<int>{3, 1, 2},
                                      std::set<int>{1},
                                      std::set<int>{5, 4}})
                         .sort()
                         .toVector();
    std::cout << "Sorted sets count: " << setSorted.size() << std::endl;

    auto setDistinct = semantic::useOf({std::set<int>{1, 2},
                                        std::set<int>{1, 2},
                                        std::set<int>{3}})
                           .distinct()
                           .toOrdered()
                           .toVector();
    std::cout << "Distinct sets count: " << setDistinct.size() << std::endl;

    std::cout << "=== set passed ===" << std::endl
              << std::endl;

    std::cout << "=== Test 5: Semantic<unordered_set<int>> ===" << std::endl;
    auto usetStream = semantic::useOf({std::unordered_set<int>{1, 2, 3},
                                       std::unordered_set<int>{4, 5}})
                          .map([](std::unordered_set<int> s) { return static_cast<int>(s.size()); })
                          .toOrdered()
                          .toVector();
    std::cout << "UnorderedSet sizes: ";
    for (auto v : usetStream)
        std::cout << v << " ";
    std::cout << std::endl;

    std::cout << "=== unordered_set passed ===" << std::endl
              << std::endl;

    std::cout << "=== Test 6: Semantic<map<int,string>> ===" << std::endl;
    auto mapStream = semantic::useOf({std::map<int, std::string>{{1, "one"}, {2, "two"}},
                                      std::map<int, std::string>{{3, "three"}},
                                      std::map<int, std::string>{{4, "four"}, {5, "five"}, {6, "six"}}})
                         .map([](std::map<int, std::string> m) { return static_cast<int>(m.size()); })
                         .toOrdered()
                         .toVector();
    std::cout << "Map sizes: ";
    for (auto v : mapStream)
        std::cout << v << " ";
    std::cout << std::endl;

    auto mapDistinct = semantic::useOf({std::map<int, std::string>{{1, "one"}},
                                        std::map<int, std::string>{{1, "one"}},
                                        std::map<int, std::string>{{2, "two"}}})
                           .distinct()
                           .toOrdered()
                           .toVector();
    std::cout << "Distinct maps count: " << mapDistinct.size() << std::endl;

    std::cout << "=== map passed ===" << std::endl
              << std::endl;

    std::cout << "=== Test 7: Semantic<unordered_map<int,string>> ===" << std::endl;
    auto umapStream = semantic::useOf({std::unordered_map<int, std::string>{{1, "one"}, {2, "two"}},
                                       std::unordered_map<int, std::string>{{3, "three"}}})
                          .map([](std::unordered_map<int, std::string> m) { return static_cast<int>(m.size()); })
                          .toOrdered()
                          .toVector();
    std::cout << "UnorderedMap sizes: ";
    for (auto v : umapStream)
        std::cout << v << " ";
    std::cout << std::endl;

    std::cout << "=== unordered_map passed ===" << std::endl
              << std::endl;

    std::cout << "=== Test 8: Semantic<queue<int>> ===" << std::endl;
    std::queue<int> q1;
    q1.push(1);
    q1.push(2);
    q1.push(3);
    std::queue<int> q2;
    q2.push(4);
    q2.push(5);

    auto queueStream = semantic::useOf(std::move(q1), std::move(q2))
                           .map([](std::queue<int> q) { return static_cast<int>(q.size()); })
                           .toOrdered()
                           .toVector();
    std::cout << "Queue sizes: ";
    for (auto v : queueStream)
        std::cout << v << " ";
    std::cout << std::endl;

    std::cout << "=== queue passed ===" << std::endl
              << std::endl;

    std::cout << "=== Test 9: Semantic<stack<int>> ===" << std::endl;
    std::stack<int> s1;
    s1.push(1);
    s1.push(2);
    s1.push(3);
    std::stack<int> s2;
    s2.push(4);

    auto stackStream = semantic::useOf(std::move(s1), std::move(s2))
                           .map([](std::stack<int> s) { return static_cast<int>(s.size()); })
                           .toOrdered()
                           .toVector();
    std::cout << "Stack sizes: ";
    for (auto v : stackStream)
        std::cout << v << " ";
    std::cout << std::endl;

    std::cout << "=== stack passed ===" << std::endl
              << std::endl;

    std::cout << "=== Test 10: Semantic<array<int,3>> ===" << std::endl;
    auto arrStream = semantic::useOf({std::array<int, 3>{1, 2, 3},
                                      std::array<int, 3>{4, 5, 6},
                                      std::array<int, 3>{7, 8, 9}})
                         .map([](std::array<int, 3> a) { return a[0]; })
                         .toOrdered()
                         .toVector();
    std::cout << "Array first elements: ";
    for (auto v : arrStream)
        std::cout << v << " ";
    std::cout << std::endl;

    auto arrFiltered = semantic::useOf({std::array<int, 3>{1, 2, 3},
                                        std::array<int, 3>{0, 0, 0},
                                        std::array<int, 3>{4, 5, 6}})
                           .filter([](std::array<int, 3> a) { return a[0] > 0; })
                           .toOrdered()
                           .toVector();
    std::cout << "Filtered arrays count: " << arrFiltered.size() << std::endl;

    auto arrDistinct = semantic::useOf({std::array<int, 2>{1, 2},
                                        std::array<int, 2>{1, 2},
                                        std::array<int, 2>{3, 4}})
                           .distinct()
                           .toOrdered()
                           .toVector();
    std::cout << "Distinct arrays count: " << arrDistinct.size() << std::endl;

    auto arrTakeWhile = semantic::useOf({std::array<int, 2>{1, 0},
                                         std::array<int, 2>{2, 0},
                                         std::array<int, 2>{0, 1},
                                         std::array<int, 2>{3, 0}})
                            .takeWhile([](std::array<int, 2> a) { return a[0] > 0; })
                            .toOrdered()
                            .toVector();
    std::cout << "takeWhile a[0]>0 count: " << arrTakeWhile.size() << std::endl;

    auto arrDropWhile = semantic::useOf({std::array<int, 2>{1, 0},
                                         std::array<int, 2>{2, 0},
                                         std::array<int, 2>{0, 1},
                                         std::array<int, 2>{3, 0}})
                            .dropWhile([](std::array<int, 2> a) { return a[0] > 0; })
                            .toOrdered()
                            .toVector();
    std::cout << "dropWhile a[0]>0 count: " << arrDropWhile.size() << std::endl;

    auto arrReversed = semantic::useOf({std::array<int, 2>{1, 2},
                                        std::array<int, 2>{3, 4},
                                        std::array<int, 2>{5, 6}})
                           .reverse()
                           .toOrdered()
                           .toVector();
    std::cout << "Reversed first[0]: " << arrReversed[0][0] << std::endl;

    auto arrLimited = semantic::useOf({std::array<int, 2>{1, 2},
                                       std::array<int, 2>{3, 4},
                                       std::array<int, 2>{5, 6},
                                       std::array<int, 2>{7, 8}})
                          .limit(2)
                          .toOrdered()
                          .toVector();
    std::cout << "Limited to 2 count: " << arrLimited.size() << std::endl;

    auto arrSkip = semantic::useOf({std::array<int, 2>{1, 2},
                                    std::array<int, 2>{3, 4},
                                    std::array<int, 2>{5, 6}})
                       .skip(1)
                       .toOrdered()
                       .toVector();
    std::cout << "Skip 1 count: " << arrSkip.size() << std::endl;

    auto arrSub = semantic::useOf({std::array<int, 2>{0, 0},
                                   std::array<int, 2>{1, 1},
                                   std::array<int, 2>{2, 2},
                                   std::array<int, 2>{3, 3},
                                   std::array<int, 2>{4, 4}})
                      .sub(1, 4)
                      .toOrdered()
                      .toVector();
    std::cout << "Sub 1..4 count: " << arrSub.size() << std::endl;

    auto arrConcat = semantic::useOf({std::array<int, 2>{1, 2}})
                         .concatenate(semantic::useOf({std::array<int, 2>{3, 4}}))
                         .toOrdered()
                         .toVector();
    std::cout << "Concatenated count: " << arrConcat.size() << std::endl;

    auto arrUnordered = semantic::useOf({std::array<int, 2>{1, 2}}).toUnordered().count();
    std::cout << "Unordered count: " << arrUnordered << std::endl;

    auto arrWindow = semantic::useOf({std::array<int, 2>{1, 2},
                                      std::array<int, 2>{3, 4},
                                      std::array<int, 2>{5, 6}})
                         .toWindow()
                         .tumble(2)
                         .toUnordered()
                         .count();
    std::cout << "Window tumble 2 count: " << arrWindow << std::endl;

    auto arrStats = semantic::useOf({std::array<int, 2>{1, 2},
                                     std::array<int, 2>{3, 4},
                                     std::array<int, 2>{5, 6}})
                        .toStatistics<double>()
                        .average([](std::array<int, 2> a) -> double { return static_cast<double>(a[0]); });
    std::cout << "Average first element: " << arrStats << std::endl;

    std::cout << "=== array passed ===" << std::endl
              << std::endl;

    std::cout << "=== Test 11: flatMap on container specializations ===" << std::endl;
    auto vecFlatMap = semantic::useOf({std::vector<int>{1, 2},
                                       std::vector<int>{3, 4, 5}})
                          .flatMap([](std::vector<int> v) {
                              return semantic::useFrom(v).map([](int x) { return x * 10; });
                          })
                          .toOrdered()
                          .toVector();
    std::cout << "vector flatMap: ";
    for (auto v : vecFlatMap)
        std::cout << v << " ";
    std::cout << std::endl;

    auto listFlatMap = semantic::useOf({std::list<int>{1, 2},
                                        std::list<int>{3, 4}})
                           .flatMap([](std::list<int> l) {
                               return semantic::useFrom(l).map([](int x) { return x + 100; });
                           })
                           .toOrdered()
                           .toVector();
    std::cout << "list flatMap: ";
    for (auto v : listFlatMap)
        std::cout << v << " ";
    std::cout << std::endl;

    std::cout << "=== flatMap tests passed ===" << std::endl
              << std::endl;

    std::cout << "All 11 specialization tests passed." << std::endl;
    return 0;
}
