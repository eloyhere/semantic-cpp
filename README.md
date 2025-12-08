# Semantic Stream — A Modern C++ Functional Stream Library

Semantic is a header-only, high-performance, lazy functional stream library for C++17 and later, inspired by JavaScript Generators, Java Stream API, Java functional interfaces, and MySQL indexed access patterns.

It combines the expressive power of functional programming with true lazy evaluation, parallel execution capabilities, and independent element indexing, enabling elegant processing of both finite and infinite sequences.

## Key Features

- **Fully lazy evaluation** – Operations are only executed when a terminal operation is invoked.
- **Infinite streams** – Naturally supports infinite sequences via generator functions.
- **Independent indexing** – Every element carries an implicit or explicit index, enabling MySQL-like indexed access and efficient skipping/seeking.
- **Parallel processing** – Simple `.parallel()` or `.parallel(n)` turns the stream into a multi-threaded pipeline using a global or custom thread pool.
- **Rich functional operations** – `map`, `filter`, `flatMap`, `reduce`, `collect`, `group`, statistics, and many more.
- **Java-style Collector API** – Familiar collecting patterns with supplier, accumulator, combiner, and finisher.
- **Comprehensive statistics** – Mean, median, mode, variance, standard deviation, quartiles, skewness, kurtosis, etc.
- **Multiple construction methods** – From containers, arrays, ranges, generators, or manual filling.

## Design Philosophy

Semantic treats a stream as a timed sequence of elements produced by a Generator. This design draws from:

- **JavaScript Generator** – Pull-based lazy production of values.
- **Java Stream** – Fluent chainable API with intermediate and terminal operations.
- **Java Function package** – Type aliases for `Function`, `Consumer`, `Predicate`, etc.
- **MySQL indexed tables** – Elements can be addressed by logical index, enabling efficient `skip`, `limit`, `redirect`, and `translate`.

## Core Concepts

```cpp
using Generator<E> = BiConsumer<
    BiConsumer<E, Timestamp>,      // yield(element, timestamp)
    Predicate<E>                   // cancel predicate
>;
```

A generator receives a yielder and a cancellation check. This low-level abstraction powers all stream sources, including infinite ones.

## Quick Examples

```cpp
using namespace semantic;

// Infinite stream of random integers
auto stream = Semantic<int>::iterate([](auto accept, auto interrupt) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);

    Timestamp index = 0;
    while (!interrupt(dist(gen))) {
        accept(dist(gen), index);
    }
});

// Finite example: odds using Sieve-like lazy filter
auto odds = Semantic<long long>::range(2LL, 1'000'000LL)
  .filter([](long long n) {
    return n % 2 == 1;
  }).sub(10, 25)
  .redirect([](const auto &element, const auto &index)-> auto{
    return -index;//reverse.
  })
  .cout();// [23, 21, 19, 17, 15, 13, 11]

// Parallel word count
auto wordCount = Semantic<std::string>::from(fileLines)
  .flatMap([](const std::string& line) {
    return Semantic<std::string>::from(split(line));
  })
  .parallel()
  .group([](const std::string& w) { return w});
```

## Building & Requirements

- C++17 or later
- Header-only – simply `#include "semantic.hpp"`
- No external dependencies

A global thread pool (`semantic::globalThreadPool`) is automatically initialised with `std::thread::hardware_concurrency()` threads.

## Licence

MIT Licence – feel free to use in commercial and open-source projects.

## Author

Written with inspiration from the best ideas in modern functional programming, adapted for idiomatic, performant C++.



