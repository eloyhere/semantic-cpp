#pragma once

#include "temporal/types.h"
#include "trunk.h"
#include "branch.h"

namespace temporal::natural::tree
{

// One position in the sexagenary cycle. Absolute 1 is Jia-Zi; the value always
// consists of a matching trunk/branch pair.
class Tree
{
  public:
    constexpr Tree() noexcept : trunk(Trunk::Jia), branch(Branch::Zi) {}

    explicit constexpr Tree(Timestamp absolute) noexcept
        : trunk(trunkOf(absolute)), branch(branchOf(absolute))
    {
    }

    // Build from a 1-based cycle position. Unlike Tree(absolute), this does not
    // apply the "no zero" signed-sequence rule; callers that need an offset use
    // this entry point with a value in [1, 60].
    static constexpr Tree fromIndex(Timestamp index) noexcept
    {
        return Tree(static_cast<Trunk>(positiveModulo(index - 1, 10) + 1),
                    static_cast<Branch>(positiveModulo(index - 1, 12) + 1));
    }

    constexpr Trunk getTrunk() const noexcept { return trunk; }
    constexpr Branch getBranch() const noexcept { return branch; }

    friend constexpr bool operator==(const Tree &left, const Tree &right) noexcept
    {
        return left.trunk == right.trunk && left.branch == right.branch;
    }

    friend constexpr bool operator!=(const Tree &left, const Tree &right) noexcept { return !(left == right); }

  private:
    constexpr Tree(Trunk trunkValue, Branch branchValue) noexcept : trunk(trunkValue), branch(branchValue) {}

    Trunk trunk;
    Branch branch;
};

constexpr Tree treeOf(Timestamp absolute) noexcept { return Tree(absolute); }

} // namespace temporal::natural::tree
