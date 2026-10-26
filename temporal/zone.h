#pragma once

#include "temporal/types.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "offset.h"

namespace temporal
{

// How to resolve a local time that is missing (DST gap) or repeated (DST
// overlap). The names follow Temporal's disambiguation option; Compatible is
// the behaviour of java.time and Temporal's default: shift forward through a
// gap and choose the earlier instant in an overlap.
enum class Disambiguation
{
    Compatible,
    Earlier,
    Later,
    Reject
};

enum class LocalResultKind
{
    Single,
    Gap,
    Overlap
};

// The result of mapping a local wall-clock time into a zone. Ambiguity is data,
// not an exception: the caller can see whether the time is unique, missing or
// repeated before choosing an offset.
//
//   Single   first == second == the only offset
//   Gap      first == offset before the transition, second == offset after
//   Overlap  first == offset of the earlier instant, second == offset of the
//            later instant (so first is normally the larger offset)
struct ZoneLocalResult
{
    LocalResultKind kind;
    Offset first;
    Offset second;
};

struct ZoneTransition
{
    Timestamp unixSeconds;
    Offset offset;

    constexpr bool operator==(const ZoneTransition &other) const noexcept
    {
        return unixSeconds == other.unixSeconds && offset == other.offset;
    }
};

// A time zone as a value: an identifier, an initial offset, and a sorted list of
// transitions. Each transition says "from this instant onward the offset is
// this". This is the minimal model that can express DST gaps and overlaps
// without a time-zone database, and the shape an IANA-backed provider would
// fill in later.
//
// A Zone is data, not an Offset: two zones can share an offset at one instant
// and differ at another. Keep the types separate (Haskell's ZonedTime, which
// stores only an offset, cannot answer DST questions).
class Zone
{
  public:
    Zone() = delete;

    static Zone utc() { return fixed(Offset(), "UTC"); }

    static Zone fixed(const Offset &offset, std::string identifier = std::string())
    {
        if (identifier.empty())
        {
            identifier = offset.isZero() ? "UTC" : offset.to_string();
        }
        return Zone(std::move(identifier), offset, {});
    }

    // Transitions are sorted by instant; duplicate instants keep the last
    // supplied transition. The identifier is a name, not a rule source: no
    // database is consulted.
    static Zone of(std::string identifier, const Offset &initial, std::vector<ZoneTransition> transitions)
    {
        std::sort(transitions.begin(), transitions.end(), [](const ZoneTransition &left, const ZoneTransition &right)
                  { return left.unixSeconds < right.unixSeconds; });
        std::vector<ZoneTransition> normalized;
        normalized.reserve(transitions.size());
        for (const ZoneTransition &transition : transitions)
        {
            if (!normalized.empty() && normalized.back().unixSeconds == transition.unixSeconds)
            {
                normalized.back() = transition;
            }
            else
            {
                normalized.push_back(transition);
            }
        }
        return Zone(std::move(identifier), initial, std::move(normalized));
    }

    const std::string &getIdentifier() const noexcept { return identifier; }
    const Offset &getInitialOffset() const noexcept { return initial; }
    const std::vector<ZoneTransition> &getTransitions() const noexcept { return transitions; }
    bool isFixed() const noexcept { return transitions.empty(); }

    Offset getOffsetAt(Timestamp unixSeconds) const noexcept
    {
        if (transitions.empty())
        {
            return initial;
        }
        auto position = std::upper_bound(transitions.begin(), transitions.end(), unixSeconds,
                                         [](Timestamp value, const ZoneTransition &transition)
                                         { return value < transition.unixSeconds; });
        if (position == transitions.begin())
        {
            return initial;
        }
        --position;
        return position->offset;
    }

    // Maps a local wall-clock instant (encoded as if it were UTC) to its
    // possible offsets. Offsets are tested for round-trip validity, which is
    // O(transitions) and exact; a production tzdb provider would index them.
    ZoneLocalResult getLocalResult(Timestamp localUnixSeconds) const
    {
        std::vector<Offset> candidates;
        candidates.push_back(initial);
        for (const ZoneTransition &transition : transitions)
        {
            if (std::find(candidates.begin(), candidates.end(), transition.offset) == candidates.end())
            {
                candidates.push_back(transition.offset);
            }
        }

        std::vector<Offset> valid;
        for (const Offset &candidate : candidates)
        {
            if (getOffsetAt(localUnixSeconds - candidate.getSeconds()) == candidate)
            {
                valid.push_back(candidate);
            }
        }

        if (valid.size() == 1)
        {
            return ZoneLocalResult{LocalResultKind::Single, valid[0], valid[0]};
        }
        if (valid.empty())
        {
            Offset before = initial;
            for (const ZoneTransition &transition : transitions)
            {
                if (transition.offset.getSeconds() > before.getSeconds())
                {
                    const Timestamp gapStart = transition.unixSeconds + before.getSeconds();
                    const Timestamp gapEnd = transition.unixSeconds + transition.offset.getSeconds();
                    if (localUnixSeconds >= gapStart && localUnixSeconds < gapEnd)
                    {
                        return ZoneLocalResult{LocalResultKind::Gap, before, transition.offset};
                    }
                }
                before = transition.offset;
            }
            return ZoneLocalResult{LocalResultKind::Single, getOffsetAt(localUnixSeconds),
                                   getOffsetAt(localUnixSeconds)};
        }

        Offset earlier = valid[0];
        Offset later = valid[0];
        for (const Offset &candidate : valid)
        {
            if (candidate.getSeconds() > earlier.getSeconds())
            {
                earlier = candidate;
            }
            if (candidate.getSeconds() < later.getSeconds())
            {
                later = candidate;
            }
        }
        return ZoneLocalResult{LocalResultKind::Overlap, earlier, later};
    }

    bool operator==(const Zone &other) const noexcept
    {
        return identifier == other.identifier && initial == other.initial && transitions == other.transitions;
    }

    bool operator!=(const Zone &other) const noexcept { return !(*this == other); }

  private:
    Zone(std::string identifier, const Offset &initial, std::vector<ZoneTransition> transitions)
        : identifier(std::move(identifier)), initial(initial), transitions(std::move(transitions))
    {
    }

    std::string identifier;
    Offset initial;
    std::vector<ZoneTransition> transitions;
};

} // namespace temporal

namespace std
{
template <> struct hash<temporal::Zone>
{
    size_t operator()(const temporal::Zone &zone) const noexcept
    {
        size_t result = hash<std::string>{}(zone.getIdentifier());
        result ^=
            hash<int>{}(zone.getInitialOffset().getSeconds()) + 0x9e3779b97f4a7c15ULL + (result << 6) + (result >> 2);
        for (const temporal::ZoneTransition &transition : zone.getTransitions())
        {
            result ^= hash<temporal::Timestamp>{}(transition.unixSeconds) + 0x9e3779b97f4a7c15ULL + (result << 6) +
                      (result >> 2);
            result ^=
                hash<int>{}(transition.offset.getSeconds()) + 0x9e3779b97f4a7c15ULL + (result << 6) + (result >> 2);
        }
        return result;
    }
};
} // namespace std
