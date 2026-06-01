🚀 Semantic-Cpp: A Modern C++ Intelligent Stream Processing Framework

Semantic-Cpp is a completely redesigned modern C++ stream processing library, built upon a "multi-header, zero external dependency" modular architecture. Each header file has a clear, single responsibility and can be tested independently, together forming a complete stream processing ecosystem. This library creatively fuses the best elements of multiple programming paradigms:

· 🎯 The elegance and fluidity of the Java Stream API: chainable calls and declarative programming
· ⚡ The lazy flexibility of JavaScript Generators: deferred computation and on-demand data generation
· 🗄️ The efficiency and ordering of database indexes: built-in intelligent sorting and index-driven mechanisms, ideal for time-series and event data

Unlike traditional data processing approaches such as hand-written loops or complex asynchronous callbacks, Semantic-Cpp aims to provide a type-safe, expressive, and high-performance solution. Its core design philosophy is precise data flow control: data only flows through the "processing pipeline" when explicitly needed, and the "order" and "position" of that flow can be finely regulated through "indices", thereby achieving optimal resource utilisation.

---

📐 Project Architecture: Seven-Layer Modular Design

Semantic-Cpp consists of seven core header files, each responsible for an independent concern, building layer upon layer:

```
┌─────────────────────────────────────────────────┐
│                  semantic.h                     │
│     (Intermediate ops, container specialisations,│
│                  Collectable)                   │
├─────────────────────────────────────────────────┤
│                 semantics.h                     │
│     (Stream builders, factory methods,          │
│                  text processing)               │
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│   (Collector factories: count, group, reduce,   │
│                  DFT, etc.)                     │
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│  (Collector framework: five-stage model,        │
│              concurrency support)               │
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│ (Unicode character sequences, encoding          │
│          conversion, regex support)             │
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│ (Thread pool: emergency shutdown, exception     │
│         propagation, task submission)           │
├─────────────────────────────────────────────────┤
│                 function.h                      │
│   (Type definitions: Generator, Supplier,       │
│               Consumer, etc.)                   │
└─────────────────────────────────────────────────┘
```

Dependency Graph

```
function.h          ← No dependencies, the type foundation
pool.h              ← Depends on function.h
charsequence.h      ← Independent module, Unicode processing
collector.h         ← Depends on function.h, pool.h
collectors.h        ← Depends on collector.h, charsequence.h
semantic.h          ← Depends on collector.h, collectors.h, charsequence.h
semantics.h         ← Depends on semantic.h
```

Each header file can be compiled and tested independently, and they also support on-demand inclusion. For example, if only the collector functionality is needed, simply include collector.h and collectors.h.

---

🏗️ Layer One: function.h — The Type Foundation

function.h defines the type system for the entire framework and serves as the common foundation for all modules:

```cpp
namespace function {
    using Timestamp = long long;           // Index type
    using Module = unsigned long long;     // Module/count type
    using Generator = BiConsumer<BiConsumer<T, Timestamp>, BiPredicate<T, Timestamp>>;
}
```

Generator is the core abstraction of the entire stream system: it is a function that accepts two callbacks—accept (to receive data) and interrupt (to halt the flow)—embodying the "lazy pull" model.

---

⚡ Layer Two: pool.h — The Concurrency Foundation

pool.h provides the global thread pool pool::pool, which is the concurrency engine of the entire framework:

Feature Description
🎯 Declarative Parallelism parallel(n) merely declares intent; the thread pool activates automatically at terminal operations
🛡️ Emergency Shutdown Built-in emergencyShutdown() and std::set_terminate handler for exception safety
🔄 Exception Propagation submit() returns std::future, enabling safe exception propagation

---

🔤 Layer Three: charsequence.h — Unicode Character Sequences

charsequence.h is a complete Unicode processing module:

Feature Description
🌐 Multi-Encoding Support UTF-8, UTF-16, UTF-32, Latin1, GBK, and more
🔍 Codepoint Iterator PointIterator supports bidirectional traversal of Unicode codepoints
🏗️ Builder Pattern Builder class for efficient string concatenation
📐 Regular Expressions Regex class wrapping std::regex
🔑 Hash and Comparison All core types have std::hash and std::less specialisations

---

🔧 Layer Four: collector.h — The Collector Framework

collector.h implements the Collector pattern and is the core engine for terminal operations:

The Five-Stage Model

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (optional short-circuit)
```

Type Aliases

Type Definition Role
Identity<A> Supplier<A> Provides initial value
Accumulator<A,E> TriFunction<A,E,Timestamp,A> Accumulates elements
Combiner<A> BiFunction<A,A,A> Merges parallel results
Finisher<A,R> Function<A,R> Final transformation
Interrupt<E,A> TriPredicate<E,Timestamp,A> Short-circuit predicate

Concurrency Support

Collector::collect() automatically handles:

· 📦 Data partitioning (distributed to threads via index modulo)
· 🔗 Result merging (partial results combined via Combiner)
· ⚠️ Exception propagation (via std::exception_ptr and std::atomic<bool>)

---

🏭 Layer Five: collectors.h — Collector Factories

collectors.h provides a rich set of pre-built collector factory functions, organised into the following categories:

📊 Matching Operations

Method Description Return Type
useAllMatch(predicate) All elements satisfy the predicate bool
useAnyMatch(predicate) Any element satisfies the predicate bool
useNoneMatch(predicate) No elements satisfy the predicate bool

🔍 Finding Operations

Method Description Return Type
useFindFirst() Find the first element std::optional<E>
useFindLast() Find the last element std::optional<E>
useFindAny() Find a random element std::optional<E>
useFindAt(index) Find element at index (supports negative) std::optional<E>
useFindMaximum() Find the maximum value std::optional<E>
useFindMinimum() Find the minimum value std::optional<E>

📈 Aggregation Operations

Method Description Return Type
useCount() Count total elements Module
useSummate<E,D>() Sum values D
useSummate<E,D>(mapper) Sum with mapper D
useAverage<E,D>() Average value D
useAverage<E,D>(mapper) Average with mapper D
useRange<E,D>() Numeric range D
useRange<E,D>(mapper) Range with mapper D
useMinimum<E,D>() Minimum value std::optional<D>
useMaximum<E,D>() Maximum value std::optional<D>

📊 Statistical Operations

Method Description Return Type
useVariance<E,D>() Population variance D
useStandardDeviation<E,D>() Population standard deviation D
useSkewness<E,D>() Skewness D
useKurtosis<E,D>() Kurtosis D
useMedian<E,D>() Median std::optional<D>
useMode<E>() Mode std::optional<E>
usePercentile<E,D>(p) p-th percentile std::optional<D>
useFrequency<E>() Frequency-domain features std::map<E, complex>
useDistribution<E>() Spatial distribution features std::map<E, complex>

🔗 Reduction Operations

Method Description Return Type
useReduce(reducer) Reduction without identity std::optional<E>
useReduce(identity, reducer) Reduction with identity E
useReduce(id, red, comb, fin) Fully custom reduction R

📦 Collection Operations

Method Description Return Type
useToVector() Collect as vector std::vector<E>
useToList() Collect as list std::list<E>
useToSet() Collect as set (deduplicates) std::set<E>
useToMap(keyExtractor) Collect as map std::map<K,E>
useGroup(keyExtractor) Group by key std::unordered_map<K,vector<E>>
usePartition(size) Partition by size std::vector<vector<E>>

🎨 Output Operations

Method Description Return Type
useJoin() Join as string Charsequence
useOut() Output to stdout Charsequence
useError() Output to stderr Charsequence

🧮 Mathematical Tools

Method Description Return Type
useDFT() Discrete Fourier Transform vector<complex<double>>
useIDFT() Inverse Discrete Fourier Transform vector<complex<double>>
useFFT() Fast Fourier Transform (Cooley-Tukey) vector<complex<double>>
useIFFT() Inverse Fast Fourier Transform vector<complex<double>>
useGradient(gradFunc, lr, iter, th) Gradient descent (analytic gradient) vector<double>
useGradient(costFunc, lr, iter, th, h) Gradient descent (numerical gradient) vector<double>

---

🌊 Layer Six: semantic.h — Intermediate Stream Operations

semantic.h is the core of the entire framework, containing both the collectable and semantic namespaces.

The collectable Namespace

Provides the inheritance hierarchy of collectable objects:

Class Description Underlying Storage
Collectable<E> Abstract base class, pure virtual source() —
OrderedCollectable<E> Ordered collection std::map<Timestamp, E>
UnorderedCollectable<E> Unordered collection std::unordered_map<Timestamp, E>
Statistics<E, D> Statistical collection (inherits OrderedCollectable) 20+ statistical methods
WindowCollectable<E> Windowed collection (inherits OrderedCollectable) Supports slide/tumble

Statistics Class Methods

Method Return Type Description
summate() / summate(mapper) D Sum
average() / average(mapper) D Mean
minimum() / minimum(mapper) std::optional<D> Minimum
maximum() / maximum(mapper) std::optional<D> Maximum
range() / range(mapper) D Range (max − min)
variance() / variance(mapper) D Population variance
standardDeviation() / standardDeviation(mapper) D Population standard deviation
frequency() / frequency(mapper) std::map<*, complex> Frequency-domain features
distribute() / distribute(mapper) std::map<*, complex> Spatial distribution features
median() / median(mapper) std::optional<D> Median
mode() std::optional<E> Mode
percentile(p) / percentile(p, mapper) std::optional<D> p-th percentile
firstQuartile() / firstQuartile(mapper) std::optional<D> First quartile (Q1)
thirdQuartile() / thirdQuartile(mapper) std::optional<D> Third quartile (Q3)
interquartileRange() / interquartileRange(mapper) std::optional<D> Interquartile range (Q3−Q1)
skewness() / skewness(mapper) D Skewness
kurtosis() / kurtosis(mapper) D Kurtosis
dft() vector<complex<double>> Discrete Fourier Transform
idft() vector<complex<double>> Inverse Discrete Fourier Transform
fft() vector<complex<double>> Fast Fourier Transform
ifft() vector<complex<double>> Inverse Fast Fourier Transform
gradient(gradFunc, lr, iter, th) vector<double> Gradient descent (analytic)
gradient(costFunc, lr, iter, th, h) vector<double> Gradient descent (numerical)

The semantic Namespace

Provides the Semantic<E> class template and its complete specialisation system:

Primary Template Methods:

Category Methods
🎨 Transformation map, flatMap, flat
🔍 Filtering filter, takeWhile, dropWhile, distinct
📏 Quantity Control limit, skip, sub
📐 Index Operations redirect, reverse, translate, sort
👀 Observation peek
⚡ Parallel Declaration parallel
🔗 Concatenation concatenate
📤 Terminal Conversion toUnordered, toOrdered, toWindow, toStatistics

Complete Container Specialisations:

Specialisation Description
Semantic<std::vector<E>> Vector container stream
Semantic<std::list<E>> List container stream
Semantic<std::set<E>> Sorted set container stream
Semantic<std::unordered_set<E>> Unordered set container stream
Semantic<std::deque<E>> Deque container stream
Semantic<std::queue<E>> Queue container stream
Semantic<std::stack<E>> Stack container stream
Semantic<std::map<K,V>> Map container stream
Semantic<std::unordered_map<K,V>> Unordered map container stream
Semantic<std::initializer_list<E>> Initialiser list stream
Semantic<Semantic<E>> Nested stream flattening

---

🏭 Layer Seven: semantics.h — Stream Builders

sematics.h provides all stream builder factory functions:

📐 Numeric Ranges

Method Description
useRange(start, end) Generate a numeric stream over [start, end)

📦 Container Construction

Method Description
useFrom(container) Create a stream from any standard container
useOf(args...) Create a stream from variadic arguments

📝 Text Processing

Method Description
useBlob(text) Split a string into a char stream by byte
useText(text) Treat the string as a whole text stream
useText(text, delimiter) Split text by delimiter
useText(istream) Read entire content from an input stream

🌐 Unicode Processing

Method Description
useSequence(charsequence) Create a codepoint stream from a character sequence
useSequence(text, encoding) Create a codepoint stream from text with specified encoding
useCharsequence(charsequence) Treat a character sequence as a whole stream
useCharsequence(charsequence, delimiter) Split a character sequence by delimiter

---

🧠 Core Concept: An Index-Driven Data World

Semantic-Cpp abstracts data processing as operations on "elements" and their "logical positions (indices)". Understanding this is key to mastering the library.

1. 📐 Basic Index Transformations

Method Description
redirect(fn) Core method: completely rewrite element indices via a custom function
reverse() Logically reverse all current indices (internally via redirect)
translate(offset) Fixed offset
translate(translator) Dynamic offset function

2. 📊 The "Overriding" Rule of Sorting

⚠️ sort() overrides everything: once called, all prior index operations are overwritten, and elements are reassigned natural-order indices based on their actual values.

· sort() → immediately materialises as OrderedCollectable
· sort(comparator) → sort with a custom comparator

3. ⚡ Declarative Parallel Processing

· parallel(n) merely declares intent; no threads are started immediately
· Terminal operations (toUnordered(), count(), etc.) are where parallelism is actually triggered
· The thread pool automatically handles task distribution and result merging

4. 🎯 How to Choose the Final Data Container?

Conversion Method Underlying Structure Performance Best Use Case
sort() OrderedCollectable Sorted then materialised Sort-by-value, pagination, time series
toOrdered() OrderedCollectable Preserves current index order Retaining custom index order
toUnordered() UnorderedCollectable Average O(1), best performance Fast lookup, dedup, aggregation
toWindow() WindowCollectable Based on ordered collection Sliding/tumbling window analysis
toStatistics() Statistics 20+ statistical methods Comprehensive statistical analysis

---

🚀 Quick Start Guide

Installation

Place all header files in your project directory and ensure your compiler supports C++17 or later:

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
#include "semantics.h"  // Automatically includes other dependencies
```

🎯 Basic Example: Experience Indices and Sorting

```cpp
auto result = semantic::useRange(0, 10)
    .map([](int x) -> int { return x * x; })
    .redirect([](int value, auto index) -> long long { return index * 2; })
    .reverse()
    .sort()                              // Forces sort by value, overriding all index ops
    .toVector();
// Output: 0 1 4 9 16 25 36 49 64 81
```

⚡ Parallel Processing Example

```cpp
auto count = semantic::useRange(1, 1000)
    .parallel(4)
    .filter([](int x) { return x % 2 == 0; })
    .toUnordered()
    .count();
// Output: Even count: 500
```

📊 Statistical Analysis Example

```cpp
auto stats = semantic::useRange(1, 101)
    .toStatistics<int, double>();

auto avg = stats.average();              // Mean
auto med = stats.median();               // Median
auto std = stats.standardDeviation();    // Standard deviation
auto q1 = stats.firstQuartile();         // First quartile
auto q3 = stats.thirdQuartile();         // Third quartile
auto skew = stats.skewness();            // Skewness
```

🔬 Frequency-Domain Analysis Example

```cpp
auto freq = data.toUnordered().frequency();
for (const auto& [element, z] : freq) {
    auto magnitude = std::abs(z);  // Distribution concentration
    auto phase = std::arg(z);      // Distribution centre phase
}
```

🧮 FFT Transform Example

```cpp
auto spectrum = semantic::useRange(0, 8)
    .map([](int x) -> std::complex<double> { return {x, 0}; })
    .toUnordered()
    .collect(collector::useFFT<double>());
```

---

⚡ Performance Optimisation Tips

1. 🎯 Choose the right container:
   · Equality lookups, unsorted aggregation → toUnordered()
   · Range queries, sorting, pagination → toOrdered() or sort()
   · Real-time window analysis → toWindow()
2. ⚡ Leverage parallelism: use parallel() when data volumes are large or processing is CPU-intensive; avoid blocking I/O in parallel streams
3. 📐 Optimise operation order: filter early, sort wisely
4. 🔄 Exploit lazy evaluation: intermediate operations are not executed immediately; takeWhile and limit can terminate early

---

📊 Comparison with the C++ Standard Library and Other Alternatives

Feature Semantic-Cpp C++20/23 ranges Hand-written loops
🎯 Core Paradigm Declarative, index-driven View-driven, functional composition Imperative, procedural
⚡ Parallel Support Declarative, automatic thread pool Requires combining parallel algorithms Manual implementation
📐 Sorting & Indices Fine-grained index control Destructive sort, no index abstraction Fully manual
📊 Statistical Analysis 20+ built-in statistical methods Not built in Requires third-party libraries
🔬 Frequency-Domain Native DFT/FFT/frequency features Not natively supported Requires third-party libraries
🧮 Gradient Descent Dual analytic + numerical modes Not built in Requires third-party libraries
🌐 Unicode Native multi-encoding support Not natively supported Manual handling
📦 Dependencies Zero external deps, 7 headers Standard library None

---

📜 Licence and Support

· 📄 Licence: Released under the MIT licence
· 🐛 Issues and Feedback: GitHub Issues
· 💬 Discussions: GitHub Discussions

---

Semantic-Cpp — Build efficient, clear data processing pipelines with modern C++. 🚀
