# semantic-cpp – Modern C++ Semantic Stream Library

**semantic-cpp** is a header-only, C++17+, zero-dependency, high-performance lazy stream library.  
Its core idea: **every element carries a logical index (Timestamp) from birth**. All ordering operations are nothing more than transformations of this index.

- True infinite streams  
- Index is a first-class citizen and can be freely remapped  
- Parallelism is the default  
- Whether index operations (`redirect`, etc.) take effect is decided explicitly only at collection time  

MIT licence – completely free for commercial and open-source projects.

## Design Philosophy

> **The index determines order – execution only determines speed.**

- `redirect`, `reverse`, `shuffle`, `cycle`, etc. modify only the logical index  
- Only `.toOrdered()` respects these transformations → result is sorted precisely by the final index  
- `.toUnordered()` ignores all index operations → maximum speed, no order  

```text
.toOrdered()   → all index operations take effect
.toUnordered() → all index operations are ignored (fastest path)
```

## Quick Start

```cpp
#include "semantic.h"
using namespace semantic;

int main() {
    // 1. Index operations take effect
    auto v1 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })   // reverse
        .toOrdered()                                  // required!
        .toVector();   // result: [99, 98, ..., 0]

    // 2. Index operations are ignored (maximum speed)
    auto v2 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })
        .toUnordered()                                // index ignored
        .toVector();   // result: unordered, but blazingly fast

    // 3. Infinite cyclic stream + ordered collection
    auto top10 = Semantic<int>::range(0, 1'000'000'000)
        .redirect([](auto&, auto i) { return i % 1000; }) // cycle 0–999
        .toOrdered()
        .limit(10)
        .toVector();   // [0,1,2,3,4,5,6,7,8,9]

    return 0;
}
```

## All Important Methods with Chaining Examples

```cpp
// Stream creation
Semantic<int>::range(0, 1000);
Semantic<long long>::range(0, 1LL<<60);
Semantic<int>::iterate([](auto yield, auto) {      // infinite stream
    for (int i = 0;; ++i) yield(i);
});
Semantic<int>::of(1, 2, 3, 4, 5);
Semantic<int>::from(std::vector{1,2,3,4});
Semantic<int>::fill(42, 1'000'000);
Semantic<int>::fill([] { return rand(); }, 1000);

// Intermediate operations (fully lazy)
stream.map([](int x) { return x * x; });
stream.filter([](int x) { return x % 2 == 0; });
stream.flatMap([](int x) { return Semantic<int>::range(0, x); });
stream.distinct();                                     // value-based deduplication
stream.skip(100);
stream.limit(50);
stream.takeWhile([](int x) { return x < 1000; });
stream.dropWhile([](int x) { return x < 100; });
stream.peek([](int x) { std::cout << x << ' '; });

// Index transformation operations (effective only with toOrdered())
stream.redirect([](auto&, auto i) { return -i; });           // reverse
stream.redirect([](auto&, auto i) { return i % 100; });      // cycle
stream.redirect([](auto e, auto i) { return std::hash<int>{}(e); }); // shuffle
stream.reverse();                                            // equivalent to redirect(-i)
stream.shuffle();

// Parallelism
stream.parallel();       // use all cores
stream.parallel(8);      // fixed thread count

// Required: explicitly choose whether to respect the index
auto ordered   = stream.toOrdered();     // all index operations take effect
auto unordered = stream.toUnordered();   // all index operations ignored (fastest)

// Ordered terminal operations (final index determines order)
ordered.toVector();
ordered.toList();
ordered.toSet();                 // deduplicate by final index
ordered.forEach([](int x) { std::cout << x << ' '; });
ordered.cout();                  // [99, 98, 97, …]

// Unordered terminal operations (maximum speed)
unordered.toVector();
unordered.toList();
unordered.toSet();
unordered.forEach(...);
unordered.cout();

// Statistics (always on the fast path)
auto stats = stream.toUnordered().toStatistics();

// reduce (recommended with unordered)
int sum = stream.toUnordered()
    .reduce(0, [](int a, int b) { return a + b; });
```

## Method Overview

| Method                           | Description                                       | Respects index operations? |
|----------------------------------|---------------------------------------------------|----------------------------|
| `toOrdered()`                    | Semantic mode — all index operations take effect  | Yes                        |
| `toUnordered()`                  | Performance mode — all index operations ignored   | No (fastest path)          |
| `toVector()` / `toList()`        | Collect into container                           | depends                    |
| `toSet()`                        | Collect into set (deduplication)                 | depends                    |
| `forEach` / `cout`               | Traverse or print                                | depends                    |
| `redirect` / `reverse` / `shuffle` | Logical index transformation               | only with toOrdered()      |
| `parallel`                       | Parallel execution                               | both modes                 |

## Why must you explicitly choose toOrdered / toUnordered?

Because **index transformation** and **parallel execution** are completely orthogonal dimensions:

- `redirect` etc. are semantic operations (where should the element logically appear?)  
- `parallel` is merely an execution strategy (how fast should it be computed?)

Only an explicit choice at collection time gives you both maximum speed and precise order control.

## Build Requirements

- C++17 or later  
- Only `#include "semantic.h"`  
- Zero external dependencies  
- Single-header library

```bash
g++ -std=c++17 -O3 -pthread semantic.cpp
```

## Licence

MIT Licence – may be used without restriction in commercial, open-source, and private projects.

---

**semantic-cpp**:  
**The index decides order – execution decides only speed.**  
Write `redirect` and it truly reverses. Write `toUnordered()` and it truly is the fastest.  
No compromise – only clear choices.

Write streams without guessing.  
Just say clearly: **“I want semantics – or I want pure speed.”**
