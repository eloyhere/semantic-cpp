# semantic-cpp – A Modern C++ Stream Library with Temporal Semantics

**semantic-cpp** is a lightweight, high-performance stream processing library for modern C++17. It consists of a single header file (`semantic.h`) paired with a separate implementation file (`semantic.cpp`). The library blends the fluency of Java Streams, the laziness of JavaScript generators and Promises, the ordering capabilities reminiscent of database indices, and built-in temporal awareness essential for financial applications, IoT data processing, and event-driven systems.

## Key Distinguishing Features

- Every element is associated with a **Timestamp** (a signed `long long` that supports negative values) and a **Module** (an unsigned `long long` used for counts and concurrency).
- Streams are lazy until materialised via `.toOrdered()`, `.toUnordered()`, `.toWindow()`, or `.toStatistics()`.
- Materialisation does **not** terminate the pipeline — further chaining is fully supported (“post-terminal” streams).
- Comprehensive support for parallel execution, sliding and tumbling windows, advanced statistical operations, and asynchronous tasks via a JavaScript-inspired **Promise** class.

## Why semantic-cpp?

| Feature                          | Java Stream | ranges-v3 | semantic-cpp                          |
|----------------------------------|-------------|-----------|---------------------------------------|
| Lazy evaluation                  | Yes         | Yes       | Yes                                   |
| Temporal indices (signed timestamps) | No          | No        | Yes (core concept)                    |
| Sliding / tumbling windows       | No          | No        | Yes (first-class support)             |
| Built-in statistical operations  | No          | No        | Yes (mean, median, mode, kurtosis, etc.) |
| Parallel execution (opt-in)      | Yes         | Yes       | Yes (thread pool with Promise support)|
| Continue chaining after terminal operation | No          | No        | Yes (post-terminal streams)           |
| Asynchronous Promises            | No          | No        | Yes (JavaScript-style)                |
| Single header + implementation file, C++17 | No          | Yes       | Yes                                   |

If you frequently write custom windowing, statistical, or asynchronous code for time-series data, market feeds, sensor streams, or logs, semantic-cpp eliminates that boilerplate.

## Quick Start

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // Create a stream from values (timestamps auto-assigned sequentially from 0)
    auto stream = of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    // Filter even numbers, run in parallel, materialise for statistics
    auto stats = stream
        .filter([](int x) { return x % 2 == 0; })
        .parallel(8)
        .toStatistics<int>();

    std::cout << "Mean: " << stats.mean() << '\n';      // 5
    std::cout << "Median: " << stats.median() << '\n';  // 5
    std::cout << "Mode: " << stats.mode() << '\n';      // e.g., any even number

    // Windowing example
    auto windows = of(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
        .toWindow()
        .getSlidingWindows(5, 2);  // windows of size 5, step 2

    return 0;
}
```

## Core Concepts

### 1. Factory Functions – Creating Streams

Streams are created via free functions in the `semantic` namespace:

```cpp
auto s = of(1, 2, 3, 4, 5);                          // variadic arguments
auto empty = empty<int>();                          // empty stream
auto filled = fill(42, 1'000'000);                   // repeated value
auto supplied = fill([]{ return rand(); }, 1'000'000);
auto ranged = range(0, 100);                        // 0 .. 99
auto ranged_step = range(0, 100, 5);                // 0, 5, 10, ...
auto from_vec = from(std::vector<int>{1, 2, 3});
auto iterated = iterate([](auto push, auto stop) {
    for (int i = 0; i < 100; ++i) push(i, i);       // explicit timestamps
});
```

Additional I/O helpers: `lines(stream)`, `chunks(stream, size)`, `text(stream)`, etc.

### 2. Semantic – The Lazy Stream

The core type `Semantic<E>` supports standard operations:

```cpp
stream.filter(...).map(...).skip(10).limit(100).peek(...)
```

Timestamp manipulation:

```cpp
stream.translate(+1000)          // shift all timestamps
    .redirect([](auto elem, Timestamp t) { return t * 10; });
```

Parallelism:

```cpp
stream.parallel()                // use default thread count
    .parallel(12);               // specify worker threads
```

### 3. Materialisation

Convert to a collectable form:

- `.toOrdered()` – preserves order, enables sorting
- `.toUnordered()` – fastest, no ordering
- `.toWindow()` – ordered with full windowing support
- `.toStatistics<R>(mapper)` – ordered with statistical methods

Chaining continues afterwards:

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
auto windows = stream.toWindow().getSlidingWindows(30, 10);
auto tumbling = stream.toWindow().getTumblingWindows(50);
```

Emit streams of windows:

```cpp
stream.toWindow()
    .windowStream(50, 20)
    .map([](const std::vector<double>& w) { return mean(w); })
    .cout();
```

### 5. Statistics

```cpp
auto stats = from(prices)
    .toStatistics<double>([](auto p) { return p; });

std::cout << "Mean:      " << stats.mean() << '\n';
std::cout << "Median:    " << stats.median() << '\n';
std::cout << "StdDev:    " << stats.standardDeviation() << '\n';
std::cout << "Skewness:  " << stats.skewness() << '\n';
std::cout << "Kurtosis:  " << stats.kurtosis() << '\n';
```

Results are aggressively cached for performance.

### 6. Asynchronous Execution with Promises

Inspired by JavaScript Promises, the library provides a `Promise<T, E>` class for asynchronous tasks submitted to a `ThreadPool`:

```cpp
ThreadPool pool(8);

auto promise = pool.submit<int>([] {
    // heavy computation
    return 42;
});

promise.then([](int result) {
        std::cout << "Result: " << result << '\n';
    })
    .except([](const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    })
    .finally([] {
        std::cout << "Done\n";
    });

promise.wait();  // block until completion if needed
```

Static helpers: `Promise<T>::all(...)`, `Promise<T>::any(...)`, `resolved(value)`, `rejected(error)`.

## Installation

Copy `semantic.h` and `semantic.cpp` into your project or use CMake:

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## Building Examples

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## Benchmarks (Apple M2 Max, 2024)

| Operation                          | Java Stream | ranges-v3 | semantic-cpp (parallel) |
|------------------------------------|-------------|-----------|--------------------------|
| 100 M integers → sum               | 280 ms      | 190 ms    | **72 ms**                |
| 10 M doubles → sliding window mean | N/A         | N/A       | **94 ms** (window 30, step 10) |
| 50 M ints → toStatistics           | N/A         | N/A       | **165 ms**               |

## Contributing

Contributions are warmly welcomed! Areas needing attention:

- Additional collectors (percentiles, covariance, etc.)
- Better integration with range libraries
- Optional SIMD acceleration

## Licence

MIT © Eloy Kim

Enjoy truly semantic streams in C++!