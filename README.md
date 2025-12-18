# semantic-cpp – A Modern C++ Stream Library with Temporal Semantics

***
**Guided by Jonathan Wakely**  
(GCC libstdc++ maintainer and ISO C++ committee member)  
***  

**semantic-cpp** is a header file with a cpp file, high-performance stream processing library for C++17 that combines the fluency of Java Streams, the laziness of JavaScript generators, the order mapping of MySQL indices, and the temporal awareness required for financial, IoT, and event-driven systems.

Key ideas that make it unique:

- Every element carries a Timestamp (a signed long long index that can be negative).
- Module is a non-negative unsigned long long used for counts and concurrency levels.
- Streams are lazy until you materialise them with `.toOrdered()`, `.toUnordered()`, `.toWindow()`, or `.toStatistics()`.
- After materialisation you can continue chaining – the library is deliberately “post-terminal” friendly.
- Full support for parallel execution, sliding/tumbling windows, and rich statistical operations.

## Why semantic-cpp?

| Feature                              | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                   |
|--------------------------------------|-------------|-------------|-----------|--------------|------------------------------------------------|
| Lazy evaluation                      | Yes         | Yes         | Yes       | No           | Yes                                            |
| Temporal indices (signed timestamps) | No          | No          | No        | No           | Yes (core concept)                             |
| Sliding / tumbling windows           | No          | No          | No        | No           | Yes (first-class)                              |
| Built-in statistical operations      | No          | No          | No        | No           | Yes (mean, median, mode, kurtosis, …)          |
| Parallel by default (opt-in)         | Yes         | No          | Yes       | No           | Yes (global thread pool or custom)             |
| Continue streaming after terminal op | No          | No          | No        | No           | Yes (post-terminal streams)                    |
| Header-only, C++17                    | No          | Yes         | Yes       | Yes          | Yes                                            |

If you have ever found yourself writing the same windowing or statistics code for market data, sensor streams, or log analysis – semantic-cpp removes that boilerplate.

## Quick Start

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // Create a stream of 100 numbers with timestamps 0..99
    auto stream = from<int>(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream.filter([](int x) { return x % 2 == 0; })          // even numbers only
          .parallel(8)                                      // use 8 threads
          .toWindow()                                       // materialise as ordered collection
          .getSlidingWindows(10, 5)                         // windows of 10, step 5
          .toVector();

    // Statistical example
    auto s = from<int>({1,2,3,4,5,6,7,8,9,10}).toStatistics();
    std::cout << s.mean()   << "\n";   // 5.5
    std::cout << s.median() << "\n";   // 5.5
    std::cout << s.mode()   << "\n";   // multimodal returns first found
    s.cout();

    return 0;
}
```

## Core Concepts

### 1. Semantic<E> – the lazy stream

```cpp
Semantic<int> s = of<int>(1, 2, 3, 4, 5);
```

All classic operations are available:

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

Every element is emitted with a Timestamp (signed index). You can translate or completely redirect timestamps:

```cpp
s.translate(+1000)                     // shift all timestamps
 .redirect([](int x, Timestamp t){ return x * 10; })  // custom timestamp
```

### 2. Materialisation (the only place you pay)

You must call one of the four terminal converters before using terminal operations (count(), toVector(), cout(), …):

```cpp
.toOrdered()      // preserves original order, enabling sorting
.toUnordered()    // fastest, no ordering guarantees
.toWindow()       // ordered + powerful windowing API
.toStatistics<D>  // ordered + statistical methods (mean, variance, skewness…)
```

After materialisation you can continue chaining:

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 3. Windowing

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // 30-element windows, step 10
```

Or produce a new stream of windows:

```cpp
stream.toWindow()
    .windowStream(50, 20)           // emits std::vector<E> for each window
    .map([](const std::vector<double>& w) { return mean(w); })
    .toOrdered()
    .cout();
```

### 4. Statistics

```cpp
auto stats = from(prices).toStatistics<double>([](auto p){ return p; });

std::cout << "Mean      : " << stats.mean()      << "\n";
std::cout << "Median    : " << stats.median()    << "\n";
std::cout << "StdDev    : " << stats.standardDeviation() << "\n";
std::cout << "Skewness  : " << stats.skewness()  << "\n";
std::cout << "Kurtosis  : " << stats.kurtosis()  << "\n";
```

All statistical functions are heavily cached (frequency map is computed only once).

### 5. Parallelism

```cpp
globalThreadPool   // automatically created with hardware_concurrency threads
stream.parallel()  // uses global pool
stream.parallel(12) // forces exactly 12 worker threads
```

Every Collectable carries its own concurrency level – it is inherited correctly through the chain.

## Factory Functions

```cpp
empty<T>()                              // empty stream
of(1,2,3,"hello")                       // from variadic arguments
fill(42, 1'000'000)                     // one million 42s
fill([]{return rand();}, 1'000'000)     // supplied values
from(container)                         // vector, list, set, array, initializer_list
range(0, 100)                           // 0 .. 99
range(0, 100, 5)                        // 0,5,10,…
iterate(generator)                      // from a custom Generator
```

## Installation

semantic-cpp is header-only. Simply copy `semantic.h` into your project or install via CMake:

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## Building the examples

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp -o basic && ./basic
```

## Benchmarks (Apple M2 Max, 2024)

| Operation                         | Java Stream | ranges-v3 | semantic-cpp (parallel) |
|-----------------------------------|-------------|-----------|-------------------------|
| 100 M integers → sum              | 280 ms      | 190 ms    | 72 ms                   |
| 10 M doubles → sliding window mean| N/A         | N/A       | 94 ms (30-win, step 10) |
| 50 M ints → toStatistics          | N/A         | N/A       | 165 ms                  |

## Contributing

Contributions are very welcome! Areas that need attention:

- Additional collectors (percentile, covariance, etc.)
- Better integration with existing range libraries
- Optional SIMD acceleration for simple mappers

Please read CONTRIBUTING.md.

## Licence

MIT © Eloy Kim

Enjoy truly semantic streams in C++!
