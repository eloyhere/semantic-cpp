# 🚀 Semantic‑Cpp: A Future‑Ready C++ Intelligent Stream Processing Framework

Semantic‑Cpp is a ground‑up redesigned modern C++ stream processing library, adopting a **“multi‑header, zero external dependency”** modular architecture. Each header has a clear, single responsibility and can be tested independently, together forming a complete stream processing ecosystem. This library creatively blends the best of multiple programming paradigms:

- 🎯 **The elegance and fluency of Java Stream API**: chainable calls, declarative programming, making code read like poetry  
- ⚡ **The laziness and flexibility of JavaScript Generators**: deferred evaluation, on‑demand generation, memory‑friendly by design  
- 🗄️ **The efficiency and orderliness of database indexing**: intelligent sorting, index‑driven, ideal for time‑series data processing  

Unlike traditional data processing approaches (hand‑written loops, asynchronous callbacks), Semantic‑Cpp aims to provide a **type‑safe, expressive, and high‑performance** solution. Its core design philosophy is **precise dataflow control**: data flows only when required, with order and position finely governed by **“indices”**, ensuring optimal resource utilisation.

---

## 📐 Project Architecture: Seven‑Layer Modular Design

Semantic‑Cpp consists of **seven core headers**, layered progressively, each with a distinct and testable responsibility:

```
┌─────────────────────────────────────────────────┐
│                  semantic.h                     │
│  (stream intermediate ops, container specialisations, Collectable)
├─────────────────────────────────────────────────┤
│                 semantics.h                     │
│         (stream builders, factory methods, text processing)
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│    (collector factories: count, group, reduce, DFT, etc.)
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│     (collector framework: five‑stage model, concurrency support)
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│    (Unicode character sequences, encoding conversion, regex support)
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│   (thread pool: emergency shutdown, exception propagation, concurrent task submission)
├─────────────────────────────────────────────────┤
│                 function.h                      │
│     (type definitions: Generator, Supplier, Consumer, etc.)
└─────────────────────────────────────────────────┘
```

### Dependency Graph

```
function.h          ← no dependencies, type foundation
pool.h              ← depends on function.h
charsequence.h      ← independent module, Unicode handling
collector.h         ← depends on function.h, pool.h
collectors.h        ← depends on collector.h, charsequence.h
semantic.h          ← depends on collector.h, collectors.h, charsequence.h
semantics.h         ← depends on semantic.h
```

> Each header can be compiled and tested independently, or included on demand.  
> For example, if you only require collector functionality, simply include `collector.h` and `collectors.h`.

---

## 🏗️ Layer 1: `function.h` — Type Foundation

`function.h` defines the type system of the entire framework and serves as the common foundation for all modules:

```cpp
namespace function {
    using Timestamp = long long;           // index type
    using Module = unsigned long long;     // module / counter type
    using Generator = BiConsumer<
        BiConsumer<T, Timestamp>,
        BiPredicate<T, Timestamp>
    >;
}
```

`Generator` is the core abstraction of the entire stream system: it accepts two callbacks—`accept` (to receive data) and `interrupt` (to terminate the flow)—embodying the **“lazy pull”** model.

---

## ⚡ Layer 2: `pool.h` — Concurrency Foundation

`pool.h` provides the global thread pool `pool::pool`, which acts as the concurrency engine of the framework.

| Feature | Description |
|------|-------------|
| 🎯 Declarative parallelism | `parallel(n)` merely declares intent; execution starts at terminal operations |
| 🛡️ Emergency shutdown | Built‑in `emergencyShutdown()` and `std::set_terminate` handler |
| 🔄 Exception propagation | `submit()` returns `std::future`, supporting safe exception propagation |

---

## 🔤 Layer 3: `charsequence.h` — Unicode Character Sequences

`charsequence.h` is a fully featured Unicode handling module.

| Capability | Description |
|---------|-------------|
| 🌐 Multi‑encoding support | UTF‑8, UTF‑16, UTF‑32, Latin‑1, GBK, etc. |
| 🔍 Code point iterator | `PointIterator` supports bidirectional traversal of Unicode code points |
| 🏗️ Builder pattern | `Builder` class enables efficient string concatenation |
| 📐 Regular expressions | `Regex` class wraps `std::regex` |
| 🔑 Hashing & comparison | All core types are specialised for `std::hash` and `std::less` |

---

## 🔧 Layer 4: `collector.h` — Collector Framework

`collector.h` implements the collector pattern and serves as the core engine for terminal operations.

### Five‑Stage Model

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (optional short‑circuit)
```

### Type Aliases

| Type | Definition | Role |
|----|-----------|------|
| `Identity<A>` | `Supplier<A>` | Provides initial value |
| `Accumulator<A,E>` | `TriFunction<A,E,Timestamp,A>` | Accumulates elements |
| `Combiner<A>` | `BiFunction<A,A,A>` | Merges parallel results |
| `Finisher<A,R>` | `Function<A,R>` | Final transformation |
| `Interrupt<E,A>` | `TriPredicate<E,Timestamp,A>` | Short‑circuit condition |

### Concurrency Support

`Collector::collect()` automatically handles:

- 📦 Data partitioning (dispatched to threads via index modulo)
- 🔗 Result merging (local results combined via `Combiner`)
- ⚠️ Exception propagation (via `std::exception_ptr` and `std::atomic<bool>`)

---

## 🏭 Layer 5: `collectors.h` — Collector Factory

`collectors.h` supplies a rich set of ready‑made collector factory functions.

### 📊 Matching Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useAllMatch(predicate)` | All elements satisfy the predicate | `bool` |
| `useAnyMatch(predicate)` | Any element satisfies the predicate | `bool` |
| `useNoneMatch(predicate)` | No element satisfies the predicate | `bool` |

### 🔍 Finding Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useFindFirst()` | Find the first element | `std::optional<E>` |
| `useFindLast()` | Find the last element | `std::optional<E>` |
| `useFindAny()` | Randomly find an element | `std::optional<E>` |
| `useFindAt(index)` | Find element at specified index (negative indices supported) | `std::optional<E>` |
| `useFindMaximum()` | Find the maximum element | `std::optional<E>` |
| `useFindMinimum()` | Find the minimum element | `std::optional<E>` |

### 📈 Aggregation Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useCount()` | Count total number of elements | `Module` |
| `useSummate<E,D>()` | Summation | `D` |
| `useSummate<E,D>(mapper)` | Mapped summation | `D` |
| `useAverage<E,D>()` | Average | `D` |
| `useAverage<E,D>(mapper)` | Mapped average | `D` |
| `useRange<E,D>()` | Numeric range | `D` |
| `useRange<E,D>(mapper)` | Mapped numeric range | `D` |
| `useMinimum<E,D>()` | Minimum value | `std::optional<D>` |
| `useMaximum<E,D>()` | Maximum value | `std::optional<D>` |

### 📊 Statistical Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useVariance<E,D>()` | Population variance | `D` |
| `useStandardDeviation<E,D>()` | Population standard deviation | `D` |
| `useSkewness<E,D>()` | Skewness | `D` |
| `useKurtosis<E,D>()` | Kurtosis | `D` |
| `useMedian<E,D>()` | Median | `std::optional<D>` |
| `useMode<E>()` | Mode | `std::optional<E>` |
| `usePercentile<E,D>(p)` | p‑th percentile | `std::optional<D>` |
| `useFrequency<E>()` | Frequency domain features | `std::map<E, complex>` |
| `useDistribution<E>()` | Spatial distribution features | `std::map<E, complex>` |

### 🔗 Reduction Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useReduce(reducer)` | Reduction without identity | `std::optional<E>` |
| `useReduce(identity, reducer)` | Reduction with identity | `E` |
| `useReduce(id, red, comb, fin)` | Fully custom reduction | `R` |

### 📦 Collection Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useToVector()` | Collect into `vector` | `std::vector<E>` |
| `useToList()` | Collect into `list` | `std::list<E>` |
| `useToSet()` | Collect into `set` (deduplicated) | `std::set<E>` |
| `useToMap(keyExtractor)` | Collect into `map` | `std::map<K,E>` |
| `useGroup(keyExtractor)` | Group by key | `std::unordered_map<K,vector<E>>` |
| `usePartition(size)` | Partition by size | `std::vector<vector<E>>` |

### 🎨 Output Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useJoin()` | Join into string | `Charsequence` |
| `useOut()` | Output to stdout | `Charsequence` |
| `useError()` | Output to stderr | `Charsequence` |

### 🧮 Mathematical Utilities

| Method | Description | Return Type |
|------|-------------|-------------|
| `useDFT()` | Discrete Fourier Transform | `vector<complex<double>>` |
| `useIDFT()` | Inverse Discrete Fourier Transform | `vector<complex<double>>` |
| `useFFT()` | Fast Fourier Transform (Cooley‑Tukey) | `vector<complex<double>>` |
| `useIFFT()` | Inverse Fast Fourier Transform | `vector<complex<double>>` |
| `useGradient(gradFunc, lr, iter, th)` | Gradient descent (analytic gradient) | `vector<double>` |
| `useGradient(costFunc, lr, iter, th, h)` | Gradient descent (numeric gradient) | `vector<double>` |

---

## 🌊 Layer 6: `semantic.h` — Stream Intermediate Operations

`semantic.h` is the heart of the framework, comprising the `collectable` and `semantic` namespaces.

### `collectable` Namespace

Provides an inheritance hierarchy for collectable objects.

| Class | Description | Underlying Storage |
|-----|-------------|-------------------|
| `Collectable<E>` | Abstract base class with pure virtual `source()` | — |
| `OrderedCollectable<E>` | Ordered collection | `std::map<Timestamp, E>` |
| `UnorderedCollectable<E>` | Unordered collection | `std::unordered_map<Timestamp, E>` |
| `Statistics<E, D>` | Statistical collection (inherits `OrderedCollectable`) | 20+ statistical methods |
| `WindowCollectable<E>` | Windowed collection (inherits `OrderedCollectable`) | Supports sliding / tumbling windows |

#### `Statistics` Class Methods

| Method | Return Type | Description |
|------|-------------|-------------|
| `summate()` / `summate(mapper)` | `D` | Summation |
| `average()` / `average(mapper)` | `D` | Average |
| `minimum()` / `minimum(mapper)` | `std::optional<D>` | Minimum |
| `maximum()` / `maximum(mapper)` | `std::optional<D>` | Maximum |
| `range()` / `range(mapper)` | `D` | Range (max − min) |
| `variance()` / `variance(mapper)` | `D` | Population variance |
| `standardDeviation()` / `standardDeviation(mapper)` | `D` | Population standard deviation |
| `frequency()` / `frequency(mapper)` | `std::map<*, complex>` | Frequency domain features |
| `distribute()` / `distribute(mapper)` | `std::map<*, complex>` | Spatial distribution features |
| `median()` / `median(mapper)` | `std::optional<D>` | Median |
| `mode()` | `std::optional<E>` | Mode |
| `percentile(p)` / `percentile(p, mapper)` | `std::optional<D>` | p‑th percentile |
| `firstQuartile()` / `firstQuartile(mapper)` | `std::optional<D>` | First quartile (Q1) |
| `thirdQuartile()` / `thirdQuartile(mapper)` | `std::optional<D>` | Third quartile (Q3) |
| `interquartileRange()` / `interquartileRange(mapper)` | `std::optional<D>` | Interquartile range (Q3 − Q1) |
| `skewness()` / `skewness(mapper)` | `D` | Skewness |
| `kurtosis()` / `kurtosis(mapper)` | `D` | Kurtosis |
| `dft()` | `vector<complex<double>>` | Discrete Fourier Transform |
| `idft()` | `vector<complex<double>>` | Inverse Discrete Fourier Transform |
| `fft()` | `vector<complex<double>>` | Fast Fourier Transform |
| `ifft()` | `vector<complex<double>>` | Inverse Fast Fourier Transform |
| `gradient(gradFunc, lr, iter, th)` | `vector<double>` | Gradient descent (analytic gradient) |
| `gradient(costFunc, lr, iter, th, h)` | `vector<double>` | Gradient descent (numeric gradient) |

### `semantic` Namespace

Provides the `Semantic<E>` template class and its full specialisation system.

#### Primary Template Method Summary

| Category | Methods |
|--------|---------|
| 🎨 Element transformation | `map`, `flatMap`, `flat` |
| 🔍 Element filtering | `filter`, `takeWhile`, `dropWhile`, `distinct` |
| 📏 Cardinality control | `limit`, `skip`, `sub` |
| 📐 Index manipulation | `redirect`, `reverse`, `translate`, `sort` |
| 👀 Observation | `peek` |
| ⚡ Parallelism declaration | `parallel` |
| 🔗 Concatenation | `concatenate` |
| 📤 Terminal conversions | `toUnordered`, `toOrdered`, `toWindow`, `toStatistics` |

#### Fully Supported Container Specialisations

| Specialisation | Description |
|---------------|-------------|
| `Semantic<std::vector<E>>` | Vector stream |
| `Semantic<std::list<E>>` | List stream |
| `Semantic<std::set<E>>` | Ordered set stream |
| `Semantic<std::unordered_set<E>>` | Unordered set stream |
| `Semantic<std::deque<E>>` | Deque stream |
| `Semantic<std::queue<E>>` | Queue stream |
| `Semantic<std::stack<E>>` | Stack stream |
| `Semantic<std::map<K,V>>` | Map stream |
| `Semantic<std::unordered_map<K,V>>` | Unordered map stream |
| `Semantic<std::initializer_list<E>>` | Initialiser list stream |
| `Semantic<Semantic<E>>` | Nested stream flattening |

---

## 🏭 Layer 7: `semantics.h` — Stream Builders

`semantics.h` provides all stream construction factory functions.

### 📐 Numeric Ranges

| Method | Description |
|------|-------------|
| `useRange(start, end)` | Generate values in range `[start, end)` |

### 📦 Container Construction

| Method | Description |
|------|-------------|
| `useFrom(container)` | Create stream from any standard container |
| `useOf(args...)` | Create stream from variadic arguments |

### 📝 Text Processing

| Method | Description |
|------|-------------|
| `useBlob(text)` | Split string into `char` stream by bytes |
| `useText(text)` | Treat string as a whole text stream |
| `useText(text, delimiter)` | Split text by delimiter |
| `useText(istream)` | Read entire content from input stream |

### 🌐 Unicode Handling

| Method | Description |
|------|-------------|
| `useSequence(charsequence)` | Create code point stream from character sequence |
| `useSequence(text, encoding)` | Create code point stream from text with specified encoding |
| `useCharsequence(charsequence)` | Treat character sequence as a whole stream |
| `useCharsequence(charsequence, delimiter)` | Split character sequence by delimiter |

---

## 🧠 Core Concept: Index‑Driven Data World

Semantic‑Cpp abstracts data processing into operations on **“elements”** and their **“logical positions (indices)”**. Understanding this is the key to mastering the library.

### 1. 📐 Basic Index Transformations

| Method | Description |
|------|-------------|
| `redirect(fn)` | Core method: fully rewrite element indices via custom function |
| `reverse()` | Reverse all index logic (implemented internally via `redirect`) |
| `translate(offset)` | Fixed offset |
| `translate(translator)` | Dynamic offset function |

### 2. 📊 The “Authoritative” Rule of Sorting

⚠️ **`sort()` overrides everything**: once invoked, all prior index manipulations are discarded, and elements are reassigned natural order indices based on their values.

- `sort()` → eagerly materialised as `OrderedCollectable`
- `sort(comparator)` → sorted using a custom comparator

### 3. ⚡ Declarative Parallel Processing

- `parallel(n)` **only declares intent**, and does not immediately spawn threads
- Terminal operations (`toUnordered()`, `count()`, etc.) actually trigger parallelism
- Thread pool automatically manages task dispatch and result merging

### 4. 🎯 Choosing the Right Final Container

| Conversion | Underlying Structure | Performance Characteristics | Best Use Cases |
|----------|----------------------|------------------------------|----------------|
| `sort()` | `OrderedCollectable` | Materialised after sorting, preserves value order | Value‑based sorting, pagination, time series |
| `toOrdered()` | `OrderedCollectable` | Preserves current index order | Retaining custom index order |
| `toUnordered()` | `UnorderedCollectable` | Average O(1), highest throughput | Fast lookup, deduplication, aggregation |
| `toWindow()` | `WindowCollectable` | Based on ordered collections | Sliding / tumbling window analysis |
| `toStatistics()` | `Statistics` | 20+ statistical methods | Comprehensive statistical analysis |

---

## 🚀 Quick Start Guide

### Installation

Place all headers into your project directory and ensure your compiler supports **C++17 or later**:

```
include/
├── function.h
├── pool.h
├── charsequence.h
├── collector.h
├── collectors.h
├── semantic.h
└── semantics.h
```

```cpp
#include "semantics.h"  // automatically includes all dependencies
```

---

### 🎯 Basic Example: Experiencing Indices and Sorting

```cpp
auto result = semantic::useRange(0, 10)
    .map(int x -> int { return x * x; })
    .redirect(int value, auto index -> long long {
        return index * 2;
    })
    .reverse()
    .sort()                              // force value‑based sorting, overriding all index operations
    .toVector();

// Output: 0 1 4 9 16 25 36 49 64 81
```

### ⚡ Parallel Processing Example

```cpp
auto count = semantic::useRange(1, 1000)
    .parallel(4)
    .filter(int x { return x % 2 == 0; })
    .toUnordered()
    .count();

// Output: Number of even values: 500
```

### 📊 Statistical Analysis Example

```cpp
auto stats = semantic::useRange(1, 101)
    .toStatistics<int, double>();

auto avg = stats.average();               // mean
auto med = stats.median();                // median
auto std = stats.standardDeviation();     // standard deviation
auto q1  = stats.firstQuartile();          // first quartile
auto q3  = stats.thirdQuartile();          // third quartile
auto skew = stats.skewness();             // skewness
```

### 🔬 Frequency Domain Analysis Example

```cpp
auto freq = data.toUnordered().frequency();
for (const auto& [element, z] : freq) {
    auto magnitude = std::abs(z);  // concentration of distribution
    auto phase     = std::arg(z);  // central phase of distribution
}
```

### 🧮 FFT Example

```cpp
auto spectrum = semantic::useRange(0, 8)
    .map(int x -> std::complex<double> {
        return {x, 0};
    })
    .toUnordered()
    .collect(collector::useFFT<double>());
```

---

## ⚡ Performance Optimisation Recommendations

1. 🎯 **Choose the right container**
   - Equality lookups, unsorted aggregations → `toUnordered()`
   - Range queries, sorting, pagination → `toOrdered()` or `sort()`
   - Real‑time window analytics → `toWindow()`
2. ⚡ **Use parallelism wisely**: apply `parallel()` for large datasets or computationally intensive workloads; avoid blocking I/O
3. 📐 **Optimise operation order**: apply `filter` early, use `sort` judiciously
4. 🔄 **Leverage lazy evaluation**: intermediate operations do not execute immediately; `takeWhile` and `limit` may terminate early

---

## 📊 Comparison with C++ Standard Library and Alternatives

| Feature | Semantic‑Cpp | C++20/23 Ranges | Traditional Loops |
|-------|-------------|----------------|------------------|
| 🎯 Core paradigm | Declarative, index‑driven | View‑driven, functional composition | Imperative, procedural |
| ⚡ Parallelism support | Declarative, automatic thread pooling | Requires explicit parallel algorithms | Manual implementation |
| 📐 Sorting & indexing | Fine‑grained index control | Destructive sorting | Entirely manual |
| 📊 Statistical analysis | 20+ built‑in statistical methods | Not built‑in | Requires third‑party libraries |
| 🔬 Frequency domain analysis | Native DFT / FFT / frequency features | Not natively supported | Requires third‑party libraries |
| 🧮 Gradient descent | Analytic + numeric dual modes | Not built‑in | Requires third‑party libraries |
| 🌐 Unicode | Native multi‑encoding support | Not natively supported | Manual handling |
| 📦 Dependencies | Zero external dependencies, 7 headers | Standard library | None |

---

## 📜 Licence and Support

- 📄 **Licence**: MIT Licence  

---

**Semantic‑Cpp — Building efficient, elegant data processing pipelines with modern C++. 🚀**
