# 🚀 Semantic-Cpp: A Future-Ready C++ Smart Stream Processing Framework

Semantic-Cpp is a modern, ground-up redesign of a C++ stream processing library, adopting a **"multi-header, zero external dependency"** modular architecture. Each header file has a clear, singular responsibility and is independently testable, together forming a complete stream processing ecosystem. This library creatively fuses the best aspects of multiple programming paradigms:

- 🎯 **Java Stream API** elegance and fluency: chainable calls and declarative programming for poetry-like code.
- ⚡ **JavaScript Generator** laziness and flexibility: deferred execution, on-demand generation, and memory efficiency.
- 🗄️ **Database indexing** efficiency and order: intelligent sorting, index-driven design, ideal for time-series data.

Unlike traditional data processing approaches (hand-written loops, asynchronous callbacks), Semantic-Cpp aims to provide a **type-safe, expressive, and high-performance** solution. Its core design philosophy is **precise dataflow control**: data flows only when needed, with order and position finely governed by **"indices"** to optimise resource utilisation.

---

## 📐 Project Architecture: Seven-Layer Modular Design

Semantic-Cpp consists of **seven core header files**, layered progressively, each with a single, independent responsibility:

```
┌─────────────────────────────────────────────────┐
│                 semantics.h                     │
│    (Stream factories: numeric ranges, containers,│
│     text, Unicode)                              │
├─────────────────────────────────────────────────┤
│                  semantic.h                     │
│   (Stream intermediate ops, Collectable system,  │
│    container specialisations)                    │
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│  (Collector factories: matching, finding,       │
│   aggregation, statistics, DFT/FFT, etc.)        │
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│     (Collector framework: five-stage model,      │
│      concurrency & parallelism support)          │
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│  (Unicode character sequences, multi-encoding    │
│   conversion, Builder, Buffer)                   │
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│   (Global thread pool: task submission,         │
│    emergency shutdown, exception propagation)    │
├─────────────────────────────────────────────────┤
│                 function.h                      │
│  (Type aliases: Generator, Supplier, Consumer,  │
│   etc.)                                         │
└─────────────────────────────────────────────────┘
```

### Dependency Graph

```
function.h          ← no dependencies, type cornerstone
pool.h              ← depends on function.h
charsequence.h      ← independent module, Unicode handling
collector.h         ← depends on function.h, pool.h
collectors.h        ← depends on collector.h, charsequence.h
semantic.h          ← depends on collector.h, collectors.h, charsequence.h
semantics.h         ← depends on semantic.h
```

Each header can be compiled and tested independently or included on demand.  
For example, if you only require collector functionality, simply include `collector.h` and `collectors.h`.

---

## 🏗️ Layer One: function.h — Type Foundation

`function.h` defines the type system underpinning the entire framework—the common foundation for all modules:

```cpp
namespace function {
    using Timestamp = long long;           // index type
    using Module = unsigned long long;     // module / counter type
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept
        std::function<bool(T, Timestamp)>       // interrupt
    )>;
}
```

`Generator` is the core abstraction of the entire stream system: it accepts two callbacks—`accept` (to receive data) and `interrupt` (to halt processing)—embodying the **"lazy pull"** model.

---

## ⚡ Layer Two: pool.h — Concurrency Cornerstone

`pool.h` provides the global thread pool `pool::pool`, serving as the concurrency engine for the framework:

| Feature | Description |
|-------|-------------|
| 🎯 Declarative parallelism | `parallel(n)` merely declares intent; activated automatically upon terminal operations |
| 🛡️ Emergency shutdown | Built-in `emergencyShutdown()` and `std::set_terminate` handler |
| 🔄 Exception propagation | `submit()` returns `std::future`, supporting safe exception propagation |

---

## 🔤 Layer Three: charsequence.h — Unicode Character Sequences

`charsequence.h` is a comprehensive Unicode handling module, offering creation, conversion, and manipulation of character sequences:

| Capability | Description |
|----------|-------------|
| 🌐 Multi-encoding support | UTF‑8, UTF‑16 (LE/BE), UTF‑32 (LE/BE), ASCII, Latin1 |
| 🔍 Code point iterator | `PointIterator` supports bidirectional traversal of Unicode code points |
| 🏗️ Builder pattern | `Builder` class enables efficient byte-level string concatenation |
| 📦 Buffer | `Buffer` class provides a thread-safe ring buffer |
| 🔑 Hashing & comparison | All core types have `std::hash` and `std::less` specialisations |

### Core Types

| Type | Description |
|-----|-------------|
| `Meta` | Metadata wrapper storing unsigned integer values |
| `Point` | Unicode code point with surrogate pair detection and validity checks |
| `Charsequence` | Immutable character sequence supporting splitting, replacement, searching, case conversion, etc. |
| `Builder` | Mutable byte builder supporting `prepend`, `insert`, `append`, and various data types |
| `Buffer` | Thread-safe ring buffer supporting read/write, prefetch, and capacity management |

---

## 🔧 Layer Four: collector.h — Collector Framework

`collector.h` implements the Collector pattern, forming the core engine behind terminal operations.

### Five-Stage Model

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (optional short-circuit)
```

### Type Aliases

| Type | Definition | Role |
|-----|-----------|------|
| `Identity<A>` | `Supplier<A>` | Provides initial value |
| `Accumulator<A,E>` | `TriFunction<A,E,Timestamp,A>` | Accumulates elements |
| `Combiner<A>` | `BiFunction<A,A,A>` | Merges parallel results |
| `Finisher<A,R>` | `Function<A,R>` | Final transformation |
| `Interrupt<E,A>` | `TriPredicate<E,Timestamp,A>` | Short-circuit evaluation |

### Concurrency Support

`Collector::collect()` automatically handles:

- 📦 Data partitioning (distributed across threads via index modulo)
- 🔗 Result merging (via `Combiner` aggregating partial results)
- ⚠️ Exception propagation (via `std::exception_ptr` and `std::atomic<bool>`)

---

## 🏭 Layer Five: collectors.h — Collector Factory

`collectors.h` supplies a rich set of ready-made collector factory functions.

### 📊 Matching Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useAllMatch(predicate)` | All elements satisfy condition | `bool` |
| `useAnyMatch(predicate)` | Any element satisfies condition | `bool` |
| `useNoneMatch(predicate)` | No element satisfies condition | `bool` |

### 🔍 Finding Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useFindFirst()` | Find first element | `std::optional<E>` |
| `useFindLast()` | Find last element | `std::optional<E>` |
| `useFindAny()` | Randomly find an element | `std::optional<E>` |
| `useFindAt(index)` | Find element at specified index (supports negative indices) | `std::optional<E>` |
| `useFindMaximum()` | Find maximum (custom comparator supported) | `std::optional<E>` |
| `useFindMinimum()` | Find minimum (custom comparator supported) | `std::optional<E>` |

### 📈 Aggregation Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useCount()` | Count total number of elements | `Module` |
| `useSummate<E,D>()` | Summation | `D` |
| `useSummate<E,D>(mapper)` | Mapped summation | `D` |
| `useAverage<E,D>()` | Average | `D` |
| `useAverage<E,D>(mapper)` | Mapped average | `D` |
| `useRange<E,D>()` | Numeric range (max − min) | `D` |
| `useRange<E,D>(mapper)` | Mapped range | `D` |
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
| `useMode<E>()` | Mode (frequency-domain based) | `std::optional<E>` |
| `usePercentile<E,D>(p)` | p-th percentile | `std::optional<D>` |
| `useFrequency<E>()` | Frequency-domain features (index-phase encoding) | `std::map<E, complex>` |
| `useDistribution<E>()` | Spatial distribution features (position encoding) | `std::map<E, complex>` |

### 🔗 Reduction Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useReduce(reducer)` | Reduction without initial value | `std::optional<E>` |
| `useReduce(identity, reducer)` | Reduction with initial value | `E` |
| `useReduce(id, red, comb, fin)` | Fully custom reduction | `R` |

### 📦 Container Collection Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useToVector()` | Collect into `std::vector` | `std::vector<E>` |
| `useToList()` | Collect into `std::list` | `std::list<E>` |
| `useToDeque()` | Collect into `std::deque` | `std::deque<E>` |
| `useToForwardList()` | Collect into `std::forward_list` | `std::forward_list<E>` |
| `useToArray<N>()` | Collect into fixed-size `std::array` | `std::array<E, N>` |
| `useToSet()` | Collect into `std::set` (dedup & sorted) | `std::set<E>` |
| `useToMultiset()` | Collect into `std::multiset` | `std::multiset<E>` |
| `useToUnorderedSet()` | Collect into `std::unordered_set` | `std::unordered_set<E>` |
| `useToUnorderedMultiset()` | Collect into `std::unordered_multiset` | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)` | Collect into `std::map` | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)` | Collect into `std::map` (custom value) | `std::map<K, V>` |
| `useToMultimap(keyExtractor)` | Collect into `std::multimap` | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)` | Collect into `std::multimap` (custom value) | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)` | Collect into `std::unordered_map` | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)` | Collect into `std::unordered_multimap` | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)` | Collect into `std::unordered_multimap` | `std::unordered_multimap<K, V>` |
| `useToStack()` | Collect into `std::stack` | `std::stack<E>` |
| `useToQueue()` | Collect into `std::queue` | `std::queue<E>` |
| `useToPriorityQueue()` | Collect into `std::priority_queue` | `std::priority_queue<E>` |

### 🔀 Grouping & Partitioning Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useGroup(keyExtractor)` | Group by key | `std::unordered_map<K, vector<E>>` |
| `usePartition(size)` | Partition into fixed-size chunks | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)` | Partition by custom key | `std::vector<vector<E>>` |

### 🎨 String Output Operations

| Method | Description | Return Type |
|------|-------------|-------------|
| `useJoin()` | Join into string (default comma-separated, bracketed) | `Charsequence` |
| `useJoin(delimiter)` | Join with custom delimiter | `Charsequence` |
| `useJoin(prefix, delimiter, suffix)` | Fully custom formatted join | `Charsequence` |
| `useOut()` | Formatted output to stdout | `Charsequence` |
| `useOut(delimiter)` | Custom-delimited output to stdout | `Charsequence` |
| `useOut(prefix, delimiter, suffix)` | Fully custom formatted output to stdout | `Charsequence` |
| `useError()` | Formatted output to stderr | `Charsequence` |
| `useError(delimiter)` | Custom-delimited output to stderr | `Charsequence` |
| `useError(prefix, delimiter, suffix)` | Fully custom formatted output to stderr | `Charsequence` |

### 🧮 Mathematical Utilities

| Method | Description | Return Type |
|------|-------------|-------------|
| `useDFT()` | Discrete Fourier Transform | `vector<complex<double>>` |
| `useIDFT()` | Inverse Discrete Fourier Transform | `vector<complex<double>>` |
| `useFFT()` | Fast Fourier Transform (Cooley–Tukey) | `vector<complex<double>>` |
| `useIFFT()` | Inverse Fast Fourier Transform | `vector<complex<double>>` |
| `useGradient(gradFunc, lr, iter, th)` | Gradient descent (analytic gradient) | `vector<double>` |
| `useGradient(costFunc, lr, iter, th, h)` | Gradient descent (numerical gradient) | `vector<double>` |

---

## 🌊 Layer Six: semantic.h — Stream Intermediate Operations & Collection System

`semantic.h` is the heart of the framework, containing the `collectable` and `semantic` namespaces.

### collectable Namespace

Provides the inheritance hierarchy for collectable objects:

| Class | Description | Underlying Storage |
|------|-------------|-------------------|
| `Collectable<E>` | Abstract base class with pure virtual `source()` | — |
| `OrderedCollectable<E>` | Ordered collection with custom sorting | `std::map<Timestamp, E>` |
| `UnorderedCollectable<E>` | Unordered collection, O(1) lookup | `std::unordered_map<Timestamp, E>` |
| `Statistics<E, D>` | Statistical collection (inherits `OrderedCollectable`) | 20+ statistical methods |
| `WindowCollectable<E>` | Windowed collection (inherits `OrderedCollectable`) | Supports `slide` / `tumble` |

#### Collectable Base Methods

Provides all `toXxx()` terminal collection methods (20+ container types), along with:

`count()`, `findFirst()`, `findAny()`, `anyMatch()`, `allMatch()`, `noneMatch()`, `reduce()`, `join()`, `out()`, `error()`, `group()`, `partition()`, `partitionBy()`, etc.

#### Statistics Class Methods

| Method | Return Type | Description |
|------|-------------|-------------|
| `summate()` / `summate(mapper)` | `D` | Summation |
| `average()` / `average(mapper)` | `D` | Average |
| `minimum()` / `minimum(mapper)` | `std::optional<D>` | Minimum |
| `maximum()` / `maximum(mapper)` | `std::optional<D>` | Maximum |
| `range()` / `range(mapper)` | `D` | Range (max − min) |
| `variance()` / `variance(mapper)` | `D` | Population variance |
| `standardDeviation()` / `standardDeviation(mapper)` | `D` | Population standard deviation |
| `frequency()` / `frequency(mapper)` | `std::map<*, complex>` | Frequency-domain features (index-phase encoding) |
| `distribute()` / `distribute(mapper)` | `std::map<*, complex>` | Spatial distribution features (position encoding) |
| `median()` / `median(mapper)` | `std::optional<D>` | Median |
| `mode()` | `std::optional<E>` | Mode |
| `percentile(p)` / `percentile(p, mapper)` | `std::optional<D>` | p-th percentile |
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
| `gradient(costFunc, lr, iter, th, h)` | `vector<double>` | Gradient descent (numerical gradient) |

### semantic Namespace

Provides the `Semantic<E>` template class and its full specialisation system.

#### Main Template Method Summary

| Category | Method | Description |
|--------|--------|-------------|
| 🎨 Element Transformation | `map` | One-to-one mapping |
| | `flatMap` | One-to-many mapping with flattening |
| | `flat` | Flatten nested streams |
| 🔍 Element Filtering | `filter` | Conditional filtering |
| | `takeWhile` | Take while condition holds |
| | `dropWhile` | Drop while condition holds |
| | `distinct` | Deduplication (custom comparator supported) |
| 📏 Quantity Control | `limit` | Limit number of elements |
| | `skip` | Skip first n elements |
| | `sub` | Sub-range extraction |
| 📐 Index Operations | `redirect` | Remap indices |
| | `reverse` | Reverse indices |
| | `translate` | Offset indices (fixed or dynamic) |
| 👀 Observation | `peek` | Observe elements without modification |
| ⚡ Parallelism Declaration | `parallel` | Declare parallelism level |
| 🔗 Concatenation | `concatenate` | Concatenate another stream or container |
| 📤 Terminal Conversion | `toUnordered` | Convert to unordered collector |
| | `toOrdered` | Convert to ordered collector |
| | `toWindow` | Convert to windowed collector |
| | `toStatistics` | Convert to statistical collector |

#### Full Container Specialisation Support

| Specialisation | Description |
|---------------|-------------|
| `Semantic<std::vector<E>>` | Vector stream with sorting, deduplication, etc. |
| `Semantic<std::list<E>>` | List stream with sorting, deduplication, etc. |

---

## 🏭 Layer Seven: semantics.h — Stream Construction Factory

`semantics.h` provides all stream construction factory functions.

### 📐 Numeric Range Generation

| Method | Description |
|------|-------------|
| `useRange(start, end)` | Generate numeric stream over `[start, end)` |
| `useRange(start, end, step)` | Generate numeric stream with step size |
| `useRangeClosed(start, end)` | Generate numeric stream over `[start, end]` |
| `useRangeClosed(start, end, step)` | Generate numeric stream with step size over closed interval |

### ♾️ Infinite Stream Generation

| Method | Description |
|------|-------------|
| `useInfinite(seed, generator)` | Infinite iteration from seed value |
| `useGenerate(supplier)` | Infinite supplier invocation |
| `useGenerate(supplier, limit)` | Finite supplier invocation |
| `useIterate(seed, generator)` | Infinite iteration from seed |
| `useIterate(seed, generator, limit)` | Finite iteration |
| `useRandom()` | Infinite random integer stream |
| `useRandom(min, max)` | Infinite random integers within range |
| `useRandom(min, max, count)` | Fixed-count random integers within range |

### 📦 Container & Element Construction

| Method | Description |
|------|-------------|
| `useEmpty()` | Create empty stream |
| `useOf(element)` | Single-element stream |
| `useOf(e1, e2)` | Two-element stream |
| `useOf(e1, e2, e3)` | Three-element stream |
| `useOf({...})` | Stream from initialiser list |
| `useFrom(container)` | Stream from any standard container |
| `useFrom({...})` | Stream from initialiser list |
| `useRepeat(element, count)` | Repeat element n times |

### 📝 Text Processing

| Method | Description |
|------|-------------|
| `useBlob(text)` | Split string into `char` stream by bytes |
| `useBlob(text, start, end)` | Split substring into `char` stream by bytes |
| `useBlob(istream)` | Read lines from input stream |
| `useBlob(istream, delimiter)` | Read from input stream using delimiter |
| `useText(text)` | Treat string as whole text stream |
| `useText(text, delimiter)` | Split text by delimiter |
| `useText(istream)` | Read entire input stream content |
| `useText(istream, delimiter)` | Read input stream using delimiter |

### 🌐 Unicode Handling

| Method | Description |
|------|-------------|
| `useSequence(charsequence)` | Create code point stream from character sequence |
| `useSequence(charsequence, start, end)` | Create code point stream from subsequence |
| `useSequence(text, encoding)` | Create code point stream from text with encoding |
| `useSequence(istream, encoding)` | Create code point stream from input stream with encoding |
| `useCharsequence(charsequence)` | Treat character sequence as whole stream |
| `useCharsequence(charsequence, delimiter)` | Split character sequence by delimiter |
| `useCharsequence(istream, encoding)` | Read entire character sequence from input stream |
| `useCharsequence(istream, delimiter, encoding)` | Read character sequence from input stream using delimiter |

---

## 🧠 Core Concept: An Index-Driven Data World

Semantic-Cpp abstracts data processing as operations on **"elements"** and their **"logical positions (indices)"**. Grasping this is key to mastering the library.

### 1. 📐 Basic Index Transformations

| Method | Description |
|------|-------------|
| `redirect(fn)` | Core method: fully rewrite element indices via custom function |
| `reverse()` | Reverse all index logic (implemented internally via `redirect`) |
| `translate(offset)` | Fixed offset |
| `translate(translator)` | Dynamic offset function computed per element and index |

### 2. 📊 The “Authoritative” Rule of Sorting

> ⚠️ **`sort()` overrides everything**: once called, all prior index operations are discarded; elements are reassigned natural order indices based on value.

- `sort()` → Immediately materialised into `OrderedCollectable`, naturally sorted by element value
- `sort(comparator)` → Sorted using custom comparator

### 3. ⚡ Declarative Parallel Processing

- `parallel(n)` only declares intent; threads are not launched immediately
- Terminal operations (`toUnordered()`, `count()`, etc.) trigger parallelism
- Thread pool automatically handles task distribution and result merging

### 4. 🎯 Choosing the Right Final Container

| Conversion Method | Underlying Structure | Performance Characteristics | Best Use Case |
|------------------|----------------------|----------------------------|---------------|
| `sort()` | `OrderedCollectable` | Materialised and sorted by value | Value-based sorting, pagination, time series |
| `toOrdered()` | `OrderedCollectable` | Preserves current index order | Retaining custom index order |
| `toUnordered()` | `UnorderedCollectable` | Average O(1), highest throughput | Fast lookups, deduplication, aggregation |
| `toWindow()` | `WindowCollectable` | Based on ordered collections | Sliding / tumbling window analysis |
| `toStatistics()` | `Statistics` | 20+ statistical methods | Comprehensive statistical analysis |

---

## 🚀 Quick Start Guide

### Installation

Place all header files in your project directory and ensure your compiler supports **C++17 or later**:

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

## 🎯 Basic Example: Experiencing Indices & Sorting

```cpp
auto result = semantic::useRange(0, 10)
    .map(int x { return x * x; })
    .redirect(int value, auto index -> long long {
        return index * 2;
    })
    .reverse()
    .sort()                              // force value-based sorting, overriding all index ops
    .toVector();

// Output: 0 1 4 9 16 25 36 49 64 81
```

## ⚡ Parallel Processing Example

```cpp
auto count = semantic::useRange(1, 1000)
    .parallel(4)
    .filter(int x { return x % 2 == 0; })
    .toUnordered()
    .count();

// Output: even number count: 500
```

## 📊 Statistical Analysis Example

```cpp
auto stats = semantic::useRange(1, 101)
    .toStatistics<int, double>();

auto avg = stats.average();               // mean
auto med = stats.median();                // median
auto std = stats.standardDeviation();     // standard deviation
auto q1  = stats.firstQuartile();          // first quartile
auto q3  = stats.thirdQuartile();          // third quartile
auto skew = stats.skewness();              // skewness
```

## 🔬 Frequency-Domain Analysis Example

```cpp
auto freq = data.toUnordered().frequency();
for (const auto& [element, z] : freq) {
    auto magnitude = std::abs(z);  // distribution concentration
    auto phase     = std::arg(z);  // distribution centre phase
}
```

## 🧮 FFT Example

```cpp
auto spectrum = semantic::useRange(0, 8)
    .map(int x -> std::complex<double> {
        return {static_cast<double>(x), 0.0};
    })
    .toUnordered()
    .collect(collector::useFFT<double>());
```

---

## ⚡ Performance Optimisation Recommendations

1. 🎯 **Choose the right container**
   - Equality lookups, unsorted aggregation → `toUnordered()`
   - Range queries, sorting, pagination → `toOrdered()` or `sort()`
   - Real-time window analytics → `toWindow()`
2. ⚡ **Use parallelism wisely**: apply `parallel()` for large datasets or compute-heavy logic; avoid blocking I/O.
3. 📐 **Optimise operation ordering**: filter early, sort judiciously.
4. 🔄 **Leverage lazy evaluation**: intermediate operations execute only upon termination; `takeWhile` and `limit` enable early exit.

---

## 📊 Comparison with C++ Standard Library & Alternatives

| Feature | Semantic-Cpp | C++20/23 Ranges | Traditional Loops |
|-------|-------------|-----------------|------------------|
| 🎯 Core Paradigm | Declarative, index-driven | View-driven, functional composition | Imperative, procedural |
| ⚡ Parallelism | Declarative, automatic thread pool | Requires explicit parallel algorithms | Manual implementation |
| 📐 Sorting & Indexing | Fine-grained index control, negative indices supported | Destructive sorting | Entirely manual |
| 📊 Statistical Analysis | 20+ built-in statistical methods | Not built-in | Requires third-party libraries |
| 🔬 Frequency-Domain Analysis | Native DFT / FFT / frequency-domain features | Not natively supported | Requires third-party libraries |
| 🧮 Gradient Descent | Analytic + numerical dual modes | Not built-in | Requires third-party libraries |
| 🌐 Unicode | Native multi-encoding support (UTF‑8/16/32, etc.) | Not natively supported | Manual handling |
| 📦 Container Collection | 20+ standard containers fully covered | Partial support | Manual implementation |
| 📦 Dependencies | Zero external dependencies, 7 headers | Standard library | None |

---

## 📜 Licence

- 📄 **Licence**: MIT

---

**Semantic-Cpp — Building efficient, lucid data processing pipelines with modern C++. 🚀**
