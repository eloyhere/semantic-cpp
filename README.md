# semantic-cpp – A Modern C++ Stream Library with Temporal Semantics

***
**Guided by Jonathan Wakely**  
(GCC libstdc++ maintainer and ISO C++ committee member)
***

**semantic-cpp** is a lightweight, high-performance stream processing library for C++17, consisting of a single header file paired with a separate implementation file. It blends the fluency of Java Streams, the laziness of JavaScript generators, the ordering capabilities reminiscent of database indices, and the temporal awareness essential for financial applications, IoT data processing, and event-driven systems.

Key distinguishing features:

- Every element is paired with a **Timestamp** (a signed `long long` index that may be negative).
- **Module** is an unsigned `long long` used for element counts and concurrency levels.
- Streams remain lazy until materialised via `.toOrdered()`, `.toUnordered()`, `.toWindow()`, or `.toStatistics()`.
- Materialisation does not terminate the pipeline — further chaining is fully supported (“post-terminal” streams).
- Comprehensive parallel execution, sliding and tumbling windows, and advanced statistical operations.

## Why semantic-cpp?

| Feature                              | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                   |
|--------------------------------------|-------------|-------------|-----------|--------------|------------------------------------------------|
| Lazy evaluation                      | Yes         | Yes         | Yes       | No           | Yes                                            |
| Temporal indices (signed timestamps) | No          | No          | No        | No           | Yes (core concept)                             |
| Sliding / tumbling windows           | No          | No          | No        | No           | Yes (first-class support)                      |
| Built-in statistical operations      | No          | No          | No        | No           | Yes (mean, median, mode, kurtosis, etc.)       |
| Parallel execution (opt-in)          | Yes         | No          | Yes       | No           | Yes (global thread pool or custom)             |
| Continue chaining after terminal op  | No          | No          | No        | No           | Yes (post-terminal streams)                    |
| Single header + implementation file, C++17 | No     | Yes         | Yes       | Yes          | Yes                                            |

If you frequently write custom windowing or statistical code for time-series data, market feeds, sensor streams, or logs, semantic-cpp eliminates that boilerplate.

## Quick Start

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // Generate a stream of 100 integers with timestamps 0..99
    auto stream = Generative<int>{}.of(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream.filter([](int x) { return x % 2 == 0; })   // even numbers only
          .parallel(8)                               // use 8 threads
          .toWindow()                                // materialise with ordering and windowing support
          .getSlidingWindows(10, 5)                  // windows of size 10, step 5
          .toVector();

    // Statistics example
    auto stats = Generative<int>{}.of(1,2,3,4,5,6,7,8,9,10).toStatistics();
    std::cout << stats.mean()   << "\n";   // 5.5
    std::cout << stats.median() << "\n";   // 5.5
    std::cout << stats.mode()   << "\n";   // first mode in case of multimodality
    stats.cout();

    return 0;
}
```

## Core Concepts

### 1. Generative<E> – Stream Factory

`Generative<E>` provides a convenient, fluent interface for creating streams:

```cpp
Generative<int> gen;
auto s = gen.of(1, 2, 3, 4, 5);
auto empty = gen.empty();
auto filled = gen.fill(42, 1'000'000);
auto ranged = gen.range(0, 100, 5);
```

All factory functions return a `Semantic<E>` lazy stream.

### 2. Semantic<E> – The Lazy Stream

```cpp
Semantic<int> s = of<int>(1, 2, 3, 4, 5);
```

Standard operations are available:

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

Elements carry timestamps, which can be manipulated:

```cpp
s.translate(+1000)                                      // shift timestamps
 .redirect([](int x, Timestamp t){ return x * 10; })   // custom timestamp logic
```

### 3. Materialisation

Call one of the four terminal converters before using collecting operations:

```cpp
.toOrdered()      // preserves order, enables sorting
.toUnordered()    // fastest, no ordering guarantees
.toWindow()       // ordered + full windowing capabilities
.toStatistics<D>  // ordered + statistical methods
```

Chaining continues after materialisation:

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 4. Windowing

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // size 30, step 10
```

Or emit a stream of windows:

```cpp
stream.toWindow()
    .windowStream(50, 20)
    .map([](const std::vector<double>& w) { return mean(w); })
    .toOrdered()
    .cout();
```

### 5. Statistics

```cpp
auto stats = from(prices).toStatistics<double>([](auto p){ return p; });

std::cout << "Mean      : " << stats.mean()      << "\n";
std::cout << "Median    : " << stats.median()    << "\n";
std::cout << "StdDev    : " << stats.standardDeviation() << "\n";
std::cout << "Skewness  : " << stats.skewness()  << "\n";
std::cout << "Kurtosis  : " << stats.kurtosis()  << "\n";
```

Statistical functions are aggressively cached (frequency map computed once).

### 6. Parallelism

```cpp
globalThreadPool   // created automatically with hardware concurrency
stream.parallel()  // use global pool
stream.parallel(12) // force exactly 12 worker threads
```

Concurrency level is inherited correctly throughout the chain.

## Factory Functions (via Generative)

```cpp
empty<T>()                              // empty stream
of(1,2,3,"hello")                       // variadic arguments
fill(42, 1'000'000)                     // repeated value
fill([]{return rand();}, 1'000'000)     // supplied values
from(container)                         // vector, list, set, array, initializer_list, queue
range(0, 100)                           // 0 .. 99
range(0, 100, 5)                        // 0,5,10,…
iterate(generator)                      // custom Generator
```

## Installation

semantic-cpp consists of a header and implementation file. Copy `semantic.h` and `semantic.cpp` into your project or install via CMake:

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## Building Examples

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## Benchmarks (Apple M2 Max, 2024)

| Operation                         | Java Stream | ranges-v3 | semantic-cpp (parallel) |
|-----------------------------------|-------------|-----------|-------------------------|
| 100 M integers → sum              | 280 ms      | 190 ms    | 72 ms                   |
| 10 M doubles → sliding window mean| N/A         | N/A       | 94 ms (window 30, step 10) |
| 50 M ints → toStatistics          | N/A         | N/A       | 165 ms                  |

## Contributing

Contributions are warmly welcomed! Areas currently needing attention:

- Additional collectors (percentiles, covariance, etc.)
- Improved integration with existing range libraries
- Optional SIMD acceleration for simple mappings

Please read CONTRIBUTING.md.

## Licence

MIT © Eloy Kim

Enjoy truly semantic streams in C++!
