#include "structure/hash.h"
#include "structure/less.h"

#include "test_framework.h"

#include <string>
#include <vector>

TEST_CASE("container hash smoke test")
{
    const std::vector<int> left{1, 2, 3};
    const std::vector<int> right{1, 2, 3};
    CHECK_EQ(std::hash<std::vector<int>>{}(left), std::hash<std::vector<int>>{}(right));
}

TEST_CASE("container less smoke test")
{
    const std::vector<int> left{1, 2, 3};
    const std::vector<int> right{1, 2, 4};
    CHECK(std::less<std::vector<int>>{}(left, right));
}
