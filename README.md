# Semantic - C++ Stream Processing Library

## Overview

Semantic is a C++ stream processing library inspired by JavaScript generators, index from MySQL and Java Stream API, providing functional programming-style lazy evaluation data stream operations.

## Core Features

- Lazy Evaluation: All operations are lazy, executing only when terminal operations are called
- Functional Programming: Supports higher-order functions, lambda expressions, and function composition
- Type Safety: Template-based strong type system
- Cache Optimisation: Automatic caching of statistical computation results
- Multiple Data Sources: Supports arrays, containers, generators, and various data sources

## Quick Start

- Basic Usage
  ```c++
  #include "semantic.h"

  int main() {
    // Create stream from array
    int data[] = {1, 2, 3, 4, 5};
    auto stream = semantic::from(data, 5);
    
    // Chain operations
    stream.filter([](int x) { return x % 2 == 0; })
          .map([](int x) { return x * 2; })
          .cout();  // Output: 4 8
    
    return 0;
  }
  ```
## Core Components

- Semantic Stream Class.  
  The main stream processing class providing rich intermediate and terminal operations.
  - Creating Streams
    ```c++
    // Empty stream
    auto emptyStream = semantic::empty<int>();

    auto unorderedStream = semantic::fromUnordered<int>({1,2,3,4,5})// Creates an unindexed semantic, whose redirect, distinct, sorted, reverse, translate would never cause any effect before calling "reindex" method.
    .redirect([](const int& element, const auto& index)-> auto{
        return -index;
    }) // No effect.
    .distinct() // No effect.
    .cout(); // [1,2,3,4,5];

    auto orderedStream = semantic::fromOrdered<int>({1,2,3,4,5}) // Creates an indexed semantic, which could redirect, distinct, sorted, reverse, translate. Only ordered and reindexed semantic could cause the effect on calling methods above.
    .redirect([](const int& element, const auto& index)-> auto{
        return -index; // Reverses the semantic.
    }).redirect([](const int& element, const auto& index)-> auto{
        return index + 3; // Translates all elements to next 3 points, the positive number moves the tail elements to the head, while the positive number move head elements to the tail, zero causes no effect.
    }).cout(); //[3,2,1,5,4]
    

    // From values
    auto single = semantic::of(42);
    auto multiple = semantic::of(1, 2, 3, 4, 5);

    // From containers
    std::vector<int> vec = {1, 2, 3};
    auto fromVec = semantic::from(vec);
    auto fromList = semantic::from(std::list{1, 2, 3});

    // From arrays
    int arr[] = {1, 2, 3};
    auto fromArray = semantic::from(arr, 3);

    // Numeric ranges
    auto rangeStream = semantic::range(1, 10);      // 1 to 9
    auto stepStream = semantic::range(1, 10, 2);    // 1,3,5,7,9

    // Generated streams
    auto generated = semantic::fill(42, 5);          // Five 42s
    auto randomStream = semantic::fill([]{ return rand() % 100; }, 10);
    ```
## Intermediate Operations

- Filtering Operations
  ```c++
  .filter(predicate)          // Filter elements
  .distinct()                 // Remove duplicates
  .distinct(comparator)       // Custom duplicate removal
  .limit(n)                   // Limit quantity
  .skip(n)                    // Skip first n elements
  .takeWhile(predicate)       // Take consecutive elements satisfying condition
  .dropWhile(predicate)       // Drop consecutive elements satisfying condition
  ```
- Transformation Operations
  ```c++
  .map(mapper)                // Element transformation
  .flatMap(mapper)            // Flattening map
  .sorted()                   // Natural sorting
  .sorted(comparator)        // Custom sorting
  .reindex(indexer)          // Reindex elements
  .reverse()                 // Reverse order
  .shuffle()                 // Random shuffle
  ```
- Debugging Operations
  ```c++
  .peek(consumer)            // Inspect elements without modifying stream
  ```
## Terminal Operations

- Matching Checks
  ```c++
  .anyMatch(predicate)       // Any element matches
  .allMatch(predicate)       // All elements match  
  .noneMatch(predicate)      // No elements match
  ```
- Search Operations
  ```c++
  .findFirst()               // Find first element
  .findAny()                 // Find any element
  ```
- Reduction Operations
  ```c++
  .reduce(accumulator)       // Reduction operation
  .reduce(identity, accumulator) // Reduction with initial value
  ```
- Collection Operations
  ```c++
  .toVector()               // Convert to vector
  .toList()                 // Convert to list
  .toSet()                  // Convert to set
  .toMap(keyMapper, valueMapper) // Convert to map
  .collect(collector)       // Custom collection
  ```
- Grouping and Partitioning
  ```c++
  .group(classifier)        // Group by classifier
  .partition(n)            // Partition by size
  ```
- Output Operations
  ```c++
  .cout()                  // Output to standard output
  .forEach(consumer)       // Execute operation for each element
  ```
## Statistics Class

Provides comprehensive statistical computation functionality with cache optimisation.
```c++
std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
auto stats = semantic::Statistics<double, double>(data);

// Basic statistics
auto count = stats.count();           // Count
auto sum = stats.sum();               // Sum
auto mean = stats.mean();             // Mean
auto min = stats.minimum();           // Minimum
auto max = stats.maximum();           // Maximum

// Dispersion statistics
auto variance = stats.variance();     // Variance
auto stdDev = stats.standardDeviation(); // Standard deviation
auto range = stats.range();           // Range

// Advanced statistics
auto median = stats.median();         // Median
auto mode = stats.mode();             // Mode
auto quartiles = stats.quartiles();   // Quartiles
auto skewness = stats.skewness();     // Skewness
auto kurtosis = stats.kurtosis();     // Kurtosis

// Frequency analysis
auto frequency = stats.frequency();   // Frequency distribution
```
## Collector

Supports custom collection strategies.
```c++
// String concatenation collector
auto concatenator = semantic::Collector<std::string, std::string>(
    []() { return std::string(""); },
    [](std::string& acc, int value) { acc += std::to_string(value); },
    [](std::string a, std::string b) { return a + b; },
    [](std::string result) { return result; }
);

auto result = stream.collect(concatenator);
```
## Advanced Features

Lazy Evaluation Example
```c++
auto stream = semantic::range(1, 1000)
    .filter([](int x) { 
        std::cout << "Filtering: " << x << std::endl;
        return x % 2 == 0; 
    })
    .map([](int x) {
        std::cout << "Mapping: " << x << std::endl;
        return x * 2;
    })
    .limit(3);  // Only process first 3 elements

// Nothing executed yet, only executes when terminal operation is called
auto result = stream.toVector();  // Execution starts
```
## Custom Generators
```c++
auto fibGenerator = [](const auto& consumer, const auto& interrupt, const auto& redirect) {
    int a = 0, b = 1;
    for (int i = 0; i < 10; ++i) {
        if (interrupt && interrupt(b)) break;
        if (consumer) consumer(b);
        int next = a + b;
        a = b;
        b = next;
    }
};

auto fibStream = semantic::iterate(fibGenerator);
```
## Performance Characteristics

- Lazy Evaluation: Avoids unnecessary computations
- Cache Optimisation: Automatic caching of statistical results
- Zero-copy: Uses references where possible to avoid copying
- Memory Safety: Smart pointer resource management

Compilation Requirements

- C++11 or higher
- Standard Template Library support

API Reference

Key Type Definitions
```c++
namespace semantic {
    typedef long long Timestamp;
    typedef unsigned long long Module;
    
    using Runnable = std::function<void()>;
    template <typename R> using Supplier = std::function<R()>;
    template <typename T, typename R> using Function = std::function<R(T)>;
    template <typename T> using Consumer = std::function<void(T)>;
    template <typename T> using Predicate = std::function<bool(T)>;
    // ... and more
}
```
## Factory Functions
```c++
// Creation functions
template<typename E> Semantic<E> empty();
template<typename E, typename... Args> Semantic<E> of(Args &&... args);
template<typename E> Semantic<E> from(const E* array, const Module &length);
template<typename E> Semantic<E> range(const E& start, const E& end);
template<typename E> Semantic<E> iterate(const Generator<E>& generator);
```
## Examples

- Data Processing Pipeline
```c++
// Process user data
auto processedUsers = semantic::from(users)
    .reindex()
    .filter([](const User& u) { return u.isActive(); })
    .map([](const User& u) { return u.getName().toUpperCase(); })
    .distinct()
    .sorted()
    .toList();
```
- Statistical Analysis
```c++
// Analyse sales data
auto salesStats = semantic::from(salesRecords)
    .map([](const Sale& s) { return s.amount(); })
    .toStatistics();

std::cout << "Average sale: " << salesStats.mean() << std::endl;
std::cout << "Sales variance: " << salesStats.variance() << std::endl;
```
## Why semantic-cpp? (The Indexable Revolution)

- **redirect()**: Declares index and element mapping.
- **reindex()**: Build indexes to enable redirect, distinct,sorted, reverse, translate.
- Small data (<32768 elems): Instant indexing. Big data: Pure laziness.

```cpp
fromUnordered(huge_data)  // No order assumed
    .reindex() // Build ondices now
    .redirect([](auto e, auto i){ return e.key; })  // Now redirect/sorted/distinct/reverse/translate could cause effect.
    .filter(...)
    .sorted()   // O(1)!
    .toVector();
```
License

MIT License







