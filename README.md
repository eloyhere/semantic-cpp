# 🚀 Semantic-Cpp: A Future-Oriented C++ Intelligent Stream Processing Framework

Semantic-Cpp is a completely redesigned, modern C++ stream processing library, built upon a **"multi-header, zero external dependencies"** modular architecture. Each header file has clear responsibilities, is independently testable, and collectively they form a complete stream processing ecosystem. This library creatively integrates the essence of multiple programming paradigms:

*   **Elegance and Fluency of Java Stream API:** Chain calls, declarative programming, making code as elegant as poetry ✨
*   **Laziness and Flexibility of JavaScript Generators:** Lazy evaluation, on-demand generation, memory-friendly 🌱
*   **Efficiency and Order of Database Indexes:** Intelligent sorting, index-driven, a powerful tool for time-series data processing ⏱️
*   **Batch Processing Philosophy of Container-as-Element:** Vectors, lists, maps... Any container can be a first-class citizen in the stream, flowing freely 📦

Are you tired of manually writing `for` loops to traverse a `vector`, nesting an `if` to filter, and then manually `push_back` to another container? 😩
Have you debugged an off-by-one index error late at night, just because you wanted the "third from last" element while iterating backwards? 😵💫
Do you long to operate like a database—pinpointing by index, analysing with sliding windows, completing the full journey from data to statistics with a single chain of calls? 🤔

Semantic-Cpp was born for this purpose. 🔧
It abstracts data processing as operations on **"elements"** and their **"logical positions (indices)"**—much like "rows" and "primary keys" in a database. You can freely rearrange, offset, and reverse indices without touching the data itself; you can also treat any container (`vector`, `map`, `array`...) as an indivisible whole flowing through the stream, and "unpack" it back to the element level at any time. This ability to switch granularity is something traditional streaming frameworks lack. 🎯

---

## 🏗️ Project Architecture: Eight-Layer Modular Design

Semantic-Cpp consists of **eight core header files**, layered progressively, with each file having a single, clearly defined responsibility and being independently testable. Five namespaces each have their own role, working together to form a complete pipeline from data source to final result:

```
┌─────────────────────────────────────────────────┐
│               🔧 semantics.h                     │
│   Namespace: semantic                           │
│   Stream factory: numeric ranges, containers, text, Unicode │
├─────────────────────────────────────────────────┤
│                 📦 semantic.h                    │
│   Namespace: semantic / collectable             │
│   Stream intermediate operations, Collectable system, 10 container specialisations │
├─────────────────────────────────────────────────┤
│                📊 collectors.h                   │
│   Namespace: collector                          │
│   Collector factory: matching, finding, aggregation, statistics, DFT/FFT, etc. │
├─────────────────────────────────────────────────┤
│                ⚙️ collector.h                    │
│   Namespace: collector                          │
│   Collector framework: five-stage model, concurrency and parallelism support │
├─────────────────────────────────────────────────┤
│                🔤 charsequence.h                 │
│   Namespace: charsequence                       │
│   Unicode character sequences, multi-encoding conversion, Builder, Buffer │
├─────────────────────────────────────────────────┤
│                🧵 pool.h                         │
│   Namespace: pool                               │
│   Global thread pool: task submission, emergency shutdown, exception propagation │
├─────────────────────────────────────────────────┤
│                📄 function.h                     │
│   Namespace: function                           │
│   Type definitions: Generator, Supplier, Consumer, and other aliases │
├─────────────────────────────────────────────────┤
│               🔐 hash.h / less.h                 │
│   Namespace: std (extension)                    │
│   Standard library container hash and comparison specialisations, supporting arbitrary nesting │
└─────────────────────────────────────────────────┘
```

### 🧩 Dependencies

```
function.h          ← No dependencies, type foundation
pool.h              ← Depends on function.h
charsequence.h      ← Independent module, Unicode handling
collector.h         ← Depends on function.h, pool.h
collectors.h        ← Depends on collector.h, charsequence.h
hash.h / less.h     ← Independent modules, standard library extensions
semantic.h          ← Depends on all of the above
semantics.h         ← Depends on semantic.h
```

---

## 🌍 Namespace Overview

Semantic-Cpp meticulously designs five namespaces, each like an independent "department", performing its own duties while collaborating closely:

| Namespace     | File              | Responsibility                     | Core Types/Functions                                                                 |
|---------------|-------------------|------------------------------------|--------------------------------------------------------------------------------------|
| `function`    | function.h        | Type system foundation             | Timestamp, Module, Generator<T>, Supplier<R>, Consumer<T>, Predicate<T>, etc.       |
| `pool`        | pool.h            | Concurrent execution engine        | pool::pool (global thread pool), submit(), emergencyShutdown()                       |
| `charsequence`| charsequence.h    | Unicode string handling            | charset, Meta, Point, Charsequence, Builder, Buffer, etc.                           |
| `collector`   | collector.h + collectors.h | Terminal collection execution | Collector<E,A,R>, Identity<A>, Accumulator<A,E>, etc.                               |
| `collectable` | semantic.h        | Materialised data container        | Collectable<E>, OrderedCollectable<E>, UnorderedCollectable<E>, etc.                |
| `semantic`    | semantic.h + semantics.h | Stream construction and intermediate operations | Semantic<E>, useRange(), useFrom(), etc.                                             |

### 🔁 Namespace Collaboration Flow

```cpp
semantic::useRange(0, 100)          // ← semantic namespace: Create stream
    .map(int x { ... })         // ← semantic namespace: Intermediate transformation
    .filter(int x { ... })      // ← semantic namespace: Intermediate filtering
    .toUnordered()                  // ← Convert to collectable namespace
    .toVector();                    // ← Invoke collector from collector namespace
```

---

## 📦 Layer 1: function.h — Type Foundation

`function.h` defines the type system for the entire framework, the common foundation for all modules. 🔑

```cpp
namespace function {
    using Timestamp = long long;           // Index type, the "timestamp" of data in the stream
    using Module = unsigned long long;     // Module/count type
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept — receive an element
        std::function<bool(T, Timestamp)>       // interrupt — should we stop?
    )>;
}
```

`Generator` is the core abstraction of the entire streaming system. 🌀 It doesn't return data; instead, it accepts two callbacks—`accept` ("I'm ready, please receive this element") and `interrupt` ("Need to stop?"). This inversion of control design means the data producer has no idea who the consumer is; it only needs to "push" data at the appropriate moment. This is the essence of lazy evaluation: data only truly "flows" when `accept` is called; before that, everything is just a description.

| Type Alias       | Full Definition                                    | Purpose                             |
|------------------|----------------------------------------------------|-------------------------------------|
| Timestamp        | long long                                          | Logical position of an element in the stream |
| Module           | unsigned long long                                 | Count, capacity, concurrency degree |
| Runnable         | std::function<void()>                              | Parameterless, void-returning task |
| Supplier<R>      | std::function<R()>                                 | Supplier, creates from nothing      |
| Function<T,R>    | std::function<R(T)>                                | Single-parameter function           |
| BiFunction<T,U,R>| std::function<R(T,U)>                              | Two-parameter function              |
| TriFunction<T,U,V,R>| std::function<R(T,U,V)>                        | Three-parameter function            |
| Unary<T>         | std::function<T(T)>                                | Unary operation                     |
| Binary<T>        | std::function<T(T,T)>                              | Binary operation                    |
| Consumer<T>      | std::function<void(T)>                             | Consumer                            |
| BiConsumer<T,U>  | std::function<void(T,U)>                           | Two-parameter consumer              |
| TriConsumer<T,U,V>| std::function<void(T,U,V)>                         | Three-parameter consumer            |
| Predicate<T>     | std::function<bool(T)>                             | Predicate judgement                 |
| BiPredicate<T,U> | std::function<bool(T,U)>                           | Two-parameter predicate             |
| TriPredicate<T,U,V>| std::function<bool(T,U,V)>                     | Three-parameter predicate           |
| Comparator<T>    | std::function<int(const T&,const T&)>              | Comparator, returns negative/zero/positive |
| Generator<T>     | BiConsumer<BiConsumer<T,Timestamp>, BiPredicate<T,Timestamp>> | Core abstraction of stream generator |

---

## 🧵 Layer 2: pool.h — Concurrency Foundation

`pool.h` provides the global thread pool `pool::pool`, the concurrency engine for the entire framework. 🚀 It employs a declarative parallelism design:

| Feature            | Description                                                                         |
|--------------------|-------------------------------------------------------------------------------------|
| Declarative Parallelism | `.parallel(4)` only declares "I want to use 4 threads", doesn't start immediately |
| Emergency Shutdown   | Built-in `emergencyShutdown()` and `std::set_terminate` handler                    |
| Exception Propagation | `submit()` returns `std::future`, safely propagating exceptions to the main thread |
| Member                       | Type                | Description                             |
|------------------------------|---------------------|-----------------------------------------|
| `pool::pool`                 | Global thread pool instance | Program-level singleton thread pool, auto-initialised |
| `pool::pool.submit<A>(task)` | Method              | Submits a task, returns `std::future<A>` |
| `pool::pool.emergencyShutdown()` | Method       | Emergency shutdown of all threads       |

---

## 🔤 Layer 3: charsequence.h — Unicode Character Sequences

`charsequence.h` is a complete Unicode processing module, providing functionality for creating, converting, and manipulating character sequences. 🌍 It supports multiple encodings like UTF-8, UTF-16 (LE/BE), UTF-32 (LE/BE), ASCII, and Latin1, correctly detects and handles surrogate pairs, and returns the standard U+FFFD replacement character for invalid code points.

| Type/Function     | Description                                                                 |
|-------------------|-----------------------------------------------------------------------------|
| charset           | Enumeration: ascii, utf8, utf16, utf16be, utf16le, utf32, utf32be, utf32le, latin1 |
| Meta              | Metadata wrapper, storing unsigned integer values                          |
| Point             | Unicode code point, supports surrogate pair detection (`isSurrogate()`) and validity verification (`isValidCodePoint()`) |
| Charsequence      | Immutable character sequence: split, replace, indexOf, lastIndexOf, sub, trim, toUpperCase, toLowerCase, reverse, startsWith, endsWith, contains, compare, getBytes, getPoints, getMetas, getCharacters, repeat, concat, count, join |
| Builder           | Mutable byte builder: prepend, insert, append (supports bool, short, int, long, long long, float, double, long double and corresponding unsigned types, char, unsigned char, Point, Charsequence, string_view) |
| Buffer            | Thread-safe ring buffer: write, read, peek, prepend, append, clear, shrinkToFit, data, size, capacity, atomic (locked batch operations) |
| PointIterator     | Bidirectional iterator, traverses Unicode code points                      |
| sequenceLength()  | Determines sequence length for multi-byte encodings like UTF-8 based on the first byte |
| encode()          | Encodes a single code point into a byte sequence of the specified encoding |
| decode()          | Decodes the next code point from a byte sequence, automatically advancing the pointer |
| convert()         | Encoding conversion (supports string, vector<unsigned char>, deque<unsigned char> output) |

---

## ⚙️ Layer 4: collector.h — Collector Framework

`collector.h` implements the collector pattern, the core engine for terminal operations. 🔧

### 🧩 Five-Stage Model

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (optional short-circuit)
```

These five stages are like a precise assembly line: first, prepare the initial state (Identity), then receive elements one by one and update intermediate results (Accumulator), next, merge partial results from each thread in parallel scenarios (Combiner), and finally transform the intermediate result into the user's desired final form (Finisher). Interrupt acts like an alert overseer, ready to halt the entire process at any time when conditions are met. 🚨

| Type Alias       | Full Definition                                 | Role                         |
|------------------|-------------------------------------------------|------------------------------|
| Identity<A>      | function::Supplier<A>                           | Provides initial value       |
| Accumulator<A,E> | function::TriFunction<A, E, Timestamp, A>       | Accumulates elements         |
| Combiner<A>      | function::BiFunction<A, A, A>                   | Merges parallel results      |
| Finisher<A,R>    | function::Function<A, R>                        | Final transformation         |
| Interrupt<E,A>   | function::TriPredicate<E, Timestamp, A>         | Short-circuit judgement      |

### 🔧 Framework Functions

| Function                                                               | Description                         |
|------------------------------------------------------------------------|-------------------------------------|
| `useFull(identity, accumulator, combiner, finisher)`                   | Creates a full collector (no short-circuit) |
| `useShortable(identity, interrupt, accumulator, combiner, finisher)`   | Creates a short-circuitable collector |

### 🧱 `Collector<E,A,R>` Class

| Method                                 | Description                               |
|----------------------------------------|-------------------------------------------|
| `collect(generator, concurrent)`       | Collects from a Generator, supports parallelism |
| `collect(container, concurrent)`       | Collects from a standard container, supports parallelism |
| `collect(initializer_list, concurrent)`| Collects from an initializer list         |
| `collect(array, concurrent)`           | Collects from a std::array               |
| `collect(deque, concurrent)`           | Collects from a std::deque              |
| `collect(forward_list, concurrent)`    | Collects from a std::forward_list       |
| `collect(stack, concurrent)`           | Collects from a std::stack             |
| `collect(queue, concurrent)`           | Collects from a std::queue            |

### 🔀 Concurrency Support

`Collector::collect()` automatically handles: data partitioning (distributing to threads via index modulo), result merging (combining partial results via Combiner), exception propagation (via `std::exception_ptr` and `std::atomic<bool>`). 🔀

---

## 📊 Layer 5: collectors.h — Collector Factory

`collectors.h` provides a rich set of pre-defined collector factory functions. This is not simply "a few utility functions"—it's a complete arsenal of data processing tools, covering the full spectrum from basic matching to frequency domain analysis. 🔥

### ✅ Matching Operations

Want to know if all elements in the stream satisfy a condition? Or if there exists any "rebel"? These three instantly provide the answer, and all support short-circuit evaluation—stopping traversal as soon as the answer is determined. ⚡

| Method                  | Description                                | Return Type |
|-------------------------|--------------------------------------------|-------------|
| `useAllMatch(predicate)` | All elements satisfy the condition (universal quantifier) | `bool` |
| `useAnyMatch(predicate)` | Any element satisfies the condition (existential quantifier) | `bool` |
| `useNoneMatch(predicate)`| No element satisfies the condition         | `bool` |

### 🔍 Finding Operations

Find the desired "it" from the vast sea of data. Negative indices are Semantic-Cpp's unique feature—`findAt(-1)` directly gets the last element. 🎯

| Method               | Description                                   | Return Type         |
|----------------------|-----------------------------------------------|---------------------|
| `useFindFirst()`     | Finds the first element                       | `std::optional<E>`  |
| `useFindLast()`      | Finds the last element                        | `std::optional<E>`  |
| `useFindAny()`       | Finds any element randomly                    | `std::optional<E>`  |
| `useFindAt(index)`   | Precise positioning: supports positive and negative indices | `std::optional<E>` |
| `useFindMaximum()`   | Finds the maximum value (supports custom comparator) | `std::optional<E>` |
| `useFindMinimum()`   | Finds the minimum value (supports custom comparator) | `std::optional<E>` |

### 🔢 Aggregation Operations

Let the data speak, summarising everything with numbers. Each aggregation method supports an optional mapper parameter—applying a transformation to each element before aggregation. 📈

| Method               | Description                     | Return Type         |
|----------------------|---------------------------------|---------------------|
| `useCount()`         | Total number of elements        | `Module`            |
| `useSummate<E,D>()`  | Summation                       | `D`                 |
| `useAverage<E,D>()`  | Average                         | `D`                 |
| `useRange<E,D>()`    | Numeric range (max - min)       | `D`                 |
| `useMinimum<E,D>()`  | Minimum value                   | `std::optional<D>`  |
| `useMaximum<E,D>()`  | Maximum value                   | `std::optional<D>`  |

### 📉 Statistical Operations

From descriptive statistics to frequency domain analysis, a statistician's Swiss Army knife. Particularly noteworthy are `useMode()` and `useFrequency()`—they use index phase encoding technology, encoding the position information of each element's occurrence as an angle on the complex plane, utilising Euler's formula to capture periodic patterns in the data. 🎼

| Method                          | Description                               | Return Type              |
|---------------------------------|-------------------------------------------|--------------------------|
| `useVariance<E,D>()`           | Population variance                       | `D`                      |
| `useStandardDeviation<E,D>()`  | Population standard deviation             | `D`                      |
| `useSkewness<E,D>()`           | Skewness (symmetry of distribution)       | `D`                      |
| `useKurtosis<E,D>()`           | Kurtosis (tail thickness of distribution) | `D`                      |
| `useMedian<E,D>()`             | Median                                    | `std::optional<D>`       |
| `useMode<E>()`                 | Mode (based on frequency domain analysis) | `std::optional<E>`       |
| `usePercentile<E,D>(p)`        | p-th percentile                           | `std::optional<D>`       |
| `useFrequency<E>()`            | Frequency domain features (index phase encoding) | `std::map<E, complex>` |
| `useDistribution<E>()`         | Spatial distribution features (position encoding) | `std::map<E, complex>` |

### 🔀 Reduction Operations

Reduction (Reduce) is one of the most powerful concepts in functional programming—it can "fold" a stream into a single value. 🎁

| Method                          | Description                                 | Return Type         |
|---------------------------------|---------------------------------------------|---------------------|
| `useReduce(reducer)`            | Reduction without initial value (returns nullopt if stream is empty) | `std::optional<E>` |
| `useReduce(identity, reducer)`  | Reduction with initial value                | `E`                 |
| `useReduce(id, red, comb, fin)` | Fully custom: custom accumulation, combination, finishing | `R`    |

### 🧺 Collecting to Container Operations

Data that has wandered through the stream its entire life finally needs a "home". Here are 20+ standard library containers for you to choose from: 🏡

| Method                           | Description                         | Return Type               |
|----------------------------------|-------------------------------------|---------------------------|
| `useToVector()`                  | Collects into vector, preserving order | `std::vector<E>`         |
| `useToList()`                    | Collects into list                 | `std::list<E>`           |
| `useToDeque()`                   | Collects into deque                | `std::deque<E>`          |
| `useToForwardList()`             | Collects into forward_list         | `std::forward_list<E>`   |
| `useToArray<N>()`                | Collects into fixed-size array     | `std::array<E, N>`       |
| `useToSet()`                     | Collects into set (deduplicated, sorted) | `std::set<E>`        |
| `useToMultiset()`                | Collects into multiset             | `std::multiset<E>`       |
| `useToUnorderedSet()`            | Collects into unordered_set        | `std::unordered_set<E>`  |
| `useToUnorderedMultiset()`       | Collects into unordered_multiset   | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)`         | Collects into map by key           | `std::map<K, E>`         |
| `useToMap(keyExtractor, valueExtractor)` | Collects into map with custom key-value | `std::map<K, V>` |
| `useToMultimap(keyExtractor)`    | Collects into multimap by key      | `std::multimap<K, E>`    |
| `useToMultimap(keyExtractor, valueExtractor)` | Collects into multimap with custom key-value | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)` | Collects into unordered_map | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)` | Collects into unordered_multimap by key | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)` | Collects into unordered_multimap with custom key-value | `std::unordered_multimap<K, V>` |
| `useToStack()`                   | Collects into stack                | `std::stack<E>`          |
| `useToQueue()`                   | Collects into queue                | `std::queue<E>`          |
| `useToPriorityQueue()`           | Collects into priority_queue       | `std::priority_queue<E>` |

### 🧩 Grouping and Partitioning Operations

The equivalent of SQL's `GROUP BY` in C++. The two-parameter versions of `groupBy` and `partitionBy` allow you to specify both a key extractor and a value extractor simultaneously—for example, grouping employees by department but only taking employee names rather than the entire object. 👥

| Method                                     | Description                       | Return Type                          |
|--------------------------------------------|-----------------------------------|---------------------------------------|
| `useGroup(keyExtractor)`                   | Groups by key, retaining complete elements | `std::unordered_map<K, vector<E>>` |
| `useGroupBy(keyExtractor, valueExtractor)` | Groups by key, custom value extraction | `std::unordered_map<K, vector<V>>` |
| `usePartition(size)`                       | Partitions by fixed size         | `std::vector<vector<E>>`             |
| `usePartitionBy(keyExtractor)`             | Partitions by index key, retaining complete elements | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor, valueExtractor)` | Partitions by index key, custom value extraction | `std::vector<vector<V>>` |

```cpp
// Group by department, only keep names
auto deptNames = people.toUnordered().groupBy(
    Person p { return p.department; },   // Key: department
    Person p { return p.name; }          // Value: name
);
// Returns: unordered_map<string, vector<string>>
```

### 📃 String Output Operations

The `useJoin()`, `useOut()`, `useError()` series lets your data be presented in a human-readable format: 📃

| Method                               | Description                 | Return Type               |
|--------------------------------------|-----------------------------|---------------------------|
| `useJoin()`                          | Default format join: `[elem1,elem2,...]` | `Charsequence`     |
| `useJoin(delimiter)`                 | Custom delimiter           | `Charsequence`            |
| `useJoin(prefix, delimiter, suffix)` | Fully custom format       | `Charsequence`            |
| `useOut()`                           | Outputs to stdout         | `Charsequence`            |
| `useOut(delimiter)`                  | Outputs to stdout with delimiter | `Charsequence`        |
| `useOut(prefix, delimiter, suffix)`  | Fully custom output to stdout | `Charsequence`        |
| `useError()`                         | Outputs to stderr         | `Charsequence`            |
| `useError(delimiter)`                | Outputs to stderr with delimiter | `Charsequence`        |
| `useError(prefix, delimiter, suffix)`| Fully custom output to stderr | `Charsequence`        |

### 📊 Mathematical Tools

| Method                                     | Description                         | Return Type                    |
|--------------------------------------------|-------------------------------------|--------------------------------|
| `useDFT()`                                 | Discrete Fourier Transform         | `vector<complex<double>>`      |
| `useIDFT()`                                | Inverse Discrete Fourier Transform | `vector<complex<double>>`      |
| `useFFT()`                                 | Fast Fourier Transform (Cooley-Tukey) | `vector<complex<double>>`  |
| `useIFFT()`                                | Inverse Fast Fourier Transform     | `vector<complex<double>>`      |
| `useGradient(gradFunc, lr, iter, th)`      | Gradient Descent (analytical gradient) | `vector<double>`         |
| `useGradient(costFunc, lr, iter, th, h)`   | Gradient Descent (numerical gradient) | `vector<double>`         |

---

## 📦 Layer 6: semantic.h — Stream Intermediate Operations and Collection System

---

### 🧩 Core Design: Three-Stage Pipeline — `Semantic` → `Collectable` → `Collector`

Many streaming frameworks mix "intermediate operations" and "terminal operations" in the same type, making it unclear when data is actually processed. Semantic-Cpp adopts a fundamentally different three-stage separation of responsibilities design, where each stage has a clear duty and distinct boundary:

```
┌──────────────────────────────────────────────────────────┐
│                 🌱 Stage 1: Construction & Transformation │
│               Semantic<E>  (Semantic Stream)             │
│                 Namespace: semantic                       │
│                                                          │
│  · Lazy Construction: useRange, useFrom, useGenerate... │
│  · Intermediate Transformations: map, filter, takeWhile, distinct... |
│  · Index Operations: reverse, translate, redirect...    │
│  · Parallel Declaration: parallel(n)                     │
│                                                          │
│  At this point, data is still "lying" in the pipeline, motionless, nothing has happened. │
│  Like you've written a detailed travel itinerary, but haven't left home yet. │
└──────────────────┬───────────────────────────────────────┘
                   │  toUnordered() / toOrdered() /
                   │  toWindow() / toStatistics() / sort()
                   ▼
┌──────────────────────────────────────────────────────────┐
│                📦 Stage 2: Materialisation & Collection  │
│           Collectable<E>  (Collectable Object)           │
│             Namespace: collectable                       │
│                                                          │
│  · Triggers Data Flow: The Generator is now actually invoked │
│  · Selects Data Structure: ordered map / unordered unordered_map │
│  · Materialises all elements into memory                │
│                                                          │
│  The data finally "comes alive"! It pours from the pipeline, organised into suitable containers. │
│  Like you've finally set off, luggage packed, ready to go. │
└──────────────────┬───────────────────────────────────────┘
                   │  toVector() / findFirst() / count() /
                   │  summate() / average() / join() / ...
                   ▼
┌──────────────────────────────────────────────────────────┐
│                ⚙️ Stage 3: Terminal Computation           │
│              Collector<E,A,R>                            │
│               Namespace: collector                       │
│                                                          │
│  · Five-Stage Execution: Identity → Accumulate → Combine → Finish │
│  · Parallel Support: Multi-threaded partitioned accumulation, automatic merging │
│  · Returns Final Result: vector, optional, double, bool... │
│                                                          │
│  The final result is out! Like you've reached your destination and taken the perfect photo. │
└──────────────────────────────────────────────────────────┘
```

**Key Rule:** You must first convert `Semantic<E>` to `Collectable<E>` via `toUnordered()`, `toOrdered()`, `toWindow()`, `toStatistics()`, or `sort()`, before you can call terminal methods like `toVector()`, `findFirst()`, `count()`, etc. These methods are not on `Semantic`—they belong to `Collectable`. This is not an oversight, but a deliberate design decision. ✅

---

### 🧭 Five Materialisation Paths, Five Different Destinies

When you stand at the crossroads of `Semantic<E>`, there are five directions to choose. Each determines how the data will be organised, queried, and used. Understanding the differences between them is key to mastering Semantic-Cpp. 🧠

| Conversion Method  | Target Type            | Underlying Data Structure  | Performance Characteristics | Typical Terminal Methods       |
|--------------------|------------------------|----------------------------|----------------------------|--------------------------------|
| `toUnordered()`    | UnorderedCollectable<E>| unordered_map<Timestamp,E> | Avg O(1) lookup, highest throughput | toVector, findFirst, count, group... |
| `toOrdered()`      | OrderedCollectable<E>  | map<Timestamp,E>           | O(log n) lookup, sorted by index | toVector, findAt, join, toMap... |
| `sort()`           | OrderedCollectable<E>  | map<Timestamp,E>           | Materialised after sorting by value | Same as above (but index overwritten to value order) |
| `toWindow()`       | WindowCollectable<E>   | Inherits ordered collection| Supports slide / tumble    | slide, tumble, and all parent methods |
| `toStatistics<D>()`| Statistics<E,D>        | Inherits ordered collection| 30+ built-in statistical methods | summate, average, median, fft... |

### 🔄 Detailed Comparison of the Five Paths

| Comparison Dimension | toUnordered()         | toOrdered()           | sort()                | toWindow()            | toStatistics<D>()    |
|----------------------|-----------------------|-----------------------|-----------------------|-----------------------|----------------------|
| Preserves Index Order? | No (unordered)      | Yes (by original index) | No (re-sorted by value) | Yes (inherits ordered) | Yes (inherits ordered) |
| Lookup Performance   | Fastest O(1)         | Faster O(log n)       | Faster O(log n)       | Faster O(log n)       | Faster O(log n)      |
| Memory Usage         | Lower                 | Lower                 | Lower                 | Lower                 | Depends on statistical method |
| Can Continue Chain?  | Yes (terminal methods) | Yes (terminal methods) | Yes (terminal methods) | Yes (slide/tumble returns Semantic) | Yes (terminal methods) |
| Use Case             | Fast aggregation where order doesn't matter | Time series, preserving generation order | Sorting by value, ranking, pagination | Sliding window analysis | Mathematical statistical modelling |

`sort()` is the only intermediate operation that skips `toXxx()` and goes directly to `Collectable`. After calling it, all previous index operations (reverse, translate, redirect) will be overridden, and elements are re-assigned natural sequential indices based on their sorted values. 🔀

**Which path to choose? Ask yourself:** 🤔

* Do I care about order? If yes, use `toOrdered()`; if no, use `toUnordered()`.
* Do I need window analysis? `toWindow()` then `slide()` or `tumble()`.
* Do I need statistics? `toStatistics<double>()` for a one-stop solution.
* Do I just want sorting? `sort()` in one step.

---

### 📋 collectable Namespace — All Terminal Methods

#### ✅ Matching Operations

| Method               | Return Type | Description                 |
|----------------------|-------------|-----------------------------|
| `anyMatch(predicate)`  | bool        | Any element satisfies condition |
| `allMatch(predicate)`  | bool        | All elements satisfy condition |
| `noneMatch(predicate)` | bool        | No element satisfies condition |

#### 🔍 Finding Operations

| Method                     | Return Type         | Description                     |
|----------------------------|---------------------|---------------------------------|
| `findFirst()`              | std::optional<E>    | Finds the first element         |
| `findLast()`               | std::optional<E>    | Finds the last element          |
| `findAny()`                | std::optional<E>    | Finds any element randomly      |
| `findAt(index)`            | std::optional<E>    | Finds element at specified index (supports negative) |
| `findMaximum()`            | std::optional<E>    | Finds the maximum value         |
| `findMaximum(comparator)`  | std::optional<E>    | Finds maximum with custom comparator |
| `findMinimum()`            | std::optional<E>    | Finds the minimum value         |
| `findMinimum(comparator)`  | std::optional<E>    | Finds minimum with custom comparator |

#### 🔢 Aggregation Operations

| Method      | Return Type        | Description         |
|-------------|-------------------|---------------------|
| `count()`   | function::Module  | Counts total elements |
| `empty()`   | bool              | Whether stream is empty |

#### 🔀 Reduction Operations

| Method                      | Return Type         | Description             |
|-----------------------------|---------------------|-------------------------|
| `reduce(accumulator)`       | std::optional<E>    | Reduction without initial value |
| `reduce(identity, accumulator)` | E           | Reduction with initial value |
| `reduce(identity, acc, combiner)` | R | Fully custom reduction |

#### 🧺 Collecting to Sequence Containers

| Method             | Return Type               | Description                 |
|--------------------|---------------------------|-----------------------------|
| `toVector()`       | std::vector<E>            | Collects into vector, preserving order |
| `toList()`         | std::list<E>              | Collects into list          |
| `toDeque()`        | std::deque<E>             | Collects into deque         |
| `toForwardList()`  | std::forward_list<E>      | Collects into forward_list  |
| `toArray<N>()`     | std::array<E, N>          | Collects into fixed-size array |

#### 🔐 Collecting to Associative Containers

| Method                                     | Return Type                        | Description                 |
|--------------------------------------------|------------------------------------|-----------------------------|
| `toSet()`                                  | std::set<E>                        | Collects into set (deduplicated, sorted) |
| `toMultiset()`                             | std::multiset<E>                   | Collects into multiset      |
| `toUnorderedSet()`                         | std::unordered_set<E>              | Collects into unordered_set |
| `toUnorderedMultiset()`                    | std::unordered_multiset<E>         | Collects into unordered_multiset |
| `toMap(keyExtractor)`                      | std::map<K, E>                     | Collects into map by key    |
| `toMap(keyExtractor, valueExtractor)`      | std::map<K, V>                     | Collects into map with custom key-value |
| `toMultimap(keyExtractor)`                 | std::multimap<K, E>                | Collects into multimap by key |
| `toMultimap(keyExtractor, valueExtractor)` | std::multimap<K, V>                | Collects into multimap with custom key-value |
| `toUnorderedMap(keyExtractor, valueExtractor)` | std::unordered_map<K, V> | Collects into unordered_map |
| `toUnorderedMultimap(keyExtractor)`        | std::unordered_multimap<K, E>      | Collects into unordered_multimap by key |
| `toUnorderedMultimap(keyExtractor, valueExtractor)` | std::unordered_multimap<K, V> | Collects into unordered_multimap with custom key-value |

#### 🧮 Collecting to Adapter Containers

| Method                | Return Type                  | Description         |
|-----------------------|-----------------------------|---------------------|
| `toStack()`           | std::stack<E>               | Collects into stack |
| `toQueue()`           | std::queue<E>               | Collects into queue |
| `toPriorityQueue()`   | std::priority_queue<E>      | Collects into priority_queue |

#### 👥 Grouping and Partitioning

| Method                                     | Return Type                          | Description                 |
|--------------------------------------------|--------------------------------------|-----------------------------|
| `group(keyExtractor)`                      | std::unordered_map<K, std::vector<E>> | Groups by key, retaining complete elements |
| `groupBy(keyExtractor, valueExtractor)`    | std::unordered_map<K, std::vector<V>> | Groups by key, custom value extraction |
| `partition(size)`                          | std::vector<std::vector<E>>         | Partitions by fixed size   |
| `partitionBy(keyExtractor)`                | std::vector<std::vector<E>>         | Partitions by index key, retaining complete elements |
| `partitionBy(keyExtractor, valueExtractor)` | std::vector<std::vector<V>>       | Partitions by index key, custom value extraction |

#### 📃 String Output Operations

| Method                               | Return Type               | Description                 |
|--------------------------------------|---------------------------|-----------------------------|
| `join()`                             | charsequence::Charsequence | Default format join: `[elem1,elem2,...]` |
| `join(delimiter)`                    | charsequence::Charsequence | Custom delimiter           |
| `join(prefix, delimiter, suffix)`    | charsequence::Charsequence | Fully custom format       |
| `out()`                              | charsequence::Charsequence | Outputs to stdout         |
| `out(delimiter)`                     | charsequence::Charsequence | Outputs to stdout with delimiter |
| `out(prefix, delimiter, suffix)`     | charsequence::Charsequence | Fully custom output to stdout |
| `error()`                            | charsequence::Charsequence | Outputs to stderr         |
| `error(delimiter)`                   | charsequence::Charsequence | Outputs to stderr with delimiter |
| `error(prefix, delimiter, suffix)`   | charsequence::Charsequence | Fully custom output to stderr |

#### 🔧 Custom Collection and Traversal

| Method                                 | Return Type | Description                 |
|----------------------------------------|-------------|-----------------------------|
| `collect(identity, acc, comb, fin)`    | R           | Custom four-stage collection |
| `collect(identity, interrupt, acc, comb, fin)` | R | Custom interruptible collection |
| `forEach(consumer)`                    | void        | Performs side effect for each element |

---

### 📈 `Statistics<E,D>` — Statistical Methods

| Method                     | Return Type              | Description                               |
|----------------------------|--------------------------|-------------------------------------------|
| `summate()`                | D                        | Summation                                 |
| `average()`                | D                        | Average                                   |
| `minimum()`                | std::optional<D>         | Minimum value                             |
| `maximum()`                | std::optional<D>         | Maximum value                             |
| `range()`                  | D                        | Range (max - min)                         |
| `variance()`               | D                        | Population variance                       |
| `standardDeviation()`      | D                        | Population standard deviation             |
| `median()`                 | std::optional<D>         | Median                                    |
| `mode()`                   | std::optional<E>         | Mode (based on frequency domain analysis) |
| `percentile(p)`            | std::optional<D>         | p-th percentile                           |
| `firstQuartile()`          | std::optional<D>         | First quartile (Q1)                       |
| `thirdQuartile()`          | std::optional<D>         | Third quartile (Q3)                       |
| `interquartileRange()`     | std::optional<D>         | Interquartile range (IQR = Q3 - Q1)      |
| `skewness()`               | D                        | Skewness                                  |
| `kurtosis()`               | D                        | Kurtosis                                  |
| `frequency()`              | std::map<E, std::complex<double>> | Frequency domain features (index phase encoding) |
| `distribute()`             | std::map<E, std::complex<double>> | Spatial distribution features (position encoding) |
| `dft()`                    | std::vector<std::complex<double>> | Discrete Fourier Transform             |
| `idft()`                   | std::vector<std::complex<double>> | Inverse Discrete Fourier Transform   |
| `fft()`                    | std::vector<std::complex<double>> | Fast Fourier Transform                 |
| `ifft()`                   | std::vector<std::complex<double>> | Inverse Fast Fourier Transform       |
| `gradient(gradFunc, lr, iter, th)` | std::vector<double> | Gradient Descent (analytical gradient)   |
| `gradient(costFunc, lr, iter, th, h)` | std::vector<double> | Gradient Descent (numerical gradient) |

All the above methods also support an optional mapper parameter version (e.g., `average(mapper)`), which transforms elements first before performing statistics.

---

### 🧰 New Container Specialisations: Containers as Elements

This is one of Semantic-Cpp's most unique features. In traditional streaming frameworks, each element in the stream is typically a scalar value—an int, a string. But in the real world, data often exists in batches:

* A batch of sensor readings = `std::vector<double>`
* A set of user orders = `std::list<Order>`
* A key-value configuration = `std::map<string, string>`
* A fixed-size matrix row = `std::array<float, 4>`

Semantic-Cpp provides `Semantic` template specialisations for **10 standard library containers**:

| Specialised Type                     | Description          | Supported Operations               |
|--------------------------------------|----------------------|------------------------------------|
| `Semantic<std::vector<E>>`           | Vector container stream | All intermediate + all terminal ops |
| `Semantic<std::list<E>>`             | List container stream   | Same as above                     |
| `Semantic<std::deque<E>>`            | Deque container stream  | Same as above                     |
| `Semantic<std::set<E>>`              | Ordered set container stream | Same as above              |
| `Semantic<std::unordered_set<E>>`    | Unordered set container stream | Same as above           |
| `Semantic<std::map<K,V>>`            | Ordered map container stream | Same as above              |
| `Semantic<std::unordered_map<K,V>>`  | Unordered map container stream | Same as above           |
| `Semantic<std::queue<E>>`            | Queue container stream   | Same as above                     |
| `Semantic<std::stack<E>>`            | Stack container stream   | Same as above                     |
| `Semantic<std::array<E,N>>`          | Fixed array container stream | Same as above                |

All container specialisations fully support: `map`, `filter`, `takeWhile`, `dropWhile`, `distinct`, `sort`, `limit`, `skip`, `reverse`, `translate`, `redirect`, `sub`, `concatenate`, `peek`, `flatMap`, `flat`, `parallel`, and all terminal conversion methods. 🔧

```cpp
// Container stream example: Flatten multiple vectors into an element stream
auto flattened = semantic::useOf({
    std::vector<int>{1, 2},
    std::vector<int>{3, 4, 5}
}).flatMap(std::vector<int> v {
    return semantic::useFrom(v).map(int x { return x * 10; });
}).toOrdered().toVector();
// Output: 10 20 30 40 50
```

---

### 🔧 `Semantic<E>` Intermediate Operation Methods

| Category     | Method       | Description                               |
|--------------|--------------|-------------------------------------------|
| Element Transformation | `map`      | One-to-one mapping transformation         |
|              | `flatMap`  | One-to-many mapping and flattening (R deduced from return type) |
|              | `flat`     | Flattens nested streams (element type unchanged) |
| Element Filtering | `filter`   | Conditional filtering                     |
|              | `takeWhile`| Takes while condition is true, stops immediately when false |
|              | `dropWhile`| Drops while condition is true, until first false |
|              | `distinct` | Deduplication (supports custom comparator) |
| Quantity Control | `limit`    | Limits number of elements                 |
|              | `skip`     | Skips first n elements                    |
|              | `sub`      | Takes sub-range [start, end)              |
| Index Operations | `redirect` | Re-maps indices                           |
|              | `reverse`  | Reverses indices (uses negative indices)  |
|              | `translate`| Offsets indices (fixed value or dynamic function) |
| Observation  | `peek`     | Observes each element (doesn't modify stream) |
| Parallel Declaration | `parallel(n)` | Declares parallelism degree        |
| Concatenation | `concatenate` | Concatenates another Semantic stream or standard container |
| Terminal Conversion | `toUnordered` | Converts to unordered collector   |
|              | `toOrdered`  | Converts to ordered collector     |
|              | `toWindow`   | Converts to window collector      |
|              | `toStatistics<D>` | Converts to statistical collector |

---

## 🔧 Layer 7: semantics.h — Stream Factory Functions

`semantics.h` provides all stream factory functions. If `semantic.h` is the "brain" of the stream (responsible for transformation and scheduling), then `semantics.h` is the "heart"—every data journey starts here. ❤️

### 🔢 Numeric Range Generation

| Method                          | Description                         |
|---------------------------------|-------------------------------------|
| `useRange(start, end)`          | Generates numeric stream in range [start, end) |
| `useRange(start, end, step)`    | Range generation with step, supports negative step |
| `useRangeClosed(start, end)`    | Generates numeric stream in closed range [start, end] |
| `useRangeClosed(start, end, step)` | Closed range generation with step, supports negative step |

### ♾️ Infinite Stream Generation

| Method                          | Description                     |
|---------------------------------|---------------------------------|
| `useInfinite(seed, generator)`  | Infinite iteration from seed value |
| `useGenerate(supplier)`         | Infinite calls to supplier      |
| `useGenerate(supplier, limit)`  | Finite number of calls to supplier |
| `useIterate(seed, generator)`   | Infinite iteration from seed    |
| `useIterate(seed, generator, limit)` | Finite iteration from seed |
| `useRandom()`                   | Infinite random integer stream  |
| `useRandom(min, max)`           | Infinite random number stream in specified range (auto-detects integer/float) |
| `useRandom(min, max, count)`    | Random number stream with specified range and count |

### 📦 Container and Element Construction

| Method                | Description                             |
|-----------------------|-----------------------------------------|
| `useEmpty()`          | Creates an empty stream                 |
| `useOf(element)`      | Creates a stream from a single element  |
| `useOf(e1, e2)`       | Creates a stream from two elements      |
| `useOf(e1, e2, e3)`   | Creates a stream from three elements    |
| `useOf({...})`        | Creates a stream from an initializer list |
| `useFrom(container)`  | Creates a stream from any standard container (supports move semantics) |
| `useFrom({...})`      | Creates a stream from an initializer list |
| `useRepeat(element, count)` | Repeats the specified element n times |

### 📄 Text Processing

| Method                    | Description                         |
|---------------------------|-------------------------------------|
| `useBlob(text)`           | Splits string into char stream by byte |
| `useBlob(text, start, end)` | Splits specified range of string by byte |
| `useBlob(istream)`        | Reads from input stream by line     |
| `useBlob(istream, delimiter)` | Reads from input stream by delimiter |
| `useText(text)`           | Treats string as whole text stream (Charsequence) |
| `useText(text, delimiter)` | Splits text by delimiter           |
| `useText(istream)`        | Reads entire content from input stream |
| `useText(istream, delimiter)` | Reads from input stream by delimiter |

### 🌍 Unicode Processing

| Method                          | Description                                   |
|---------------------------------|-----------------------------------------------|
| `useSequence(charsequence)`     | Creates code point stream from character sequence |
| `useSequence(charsequence, start, end)` | Creates code point stream from specified range of character sequence |
| `useSequence(text, encoding)`   | Creates code point stream from text with specified encoding |
| `useSequence(istream, encoding)` | Creates code point stream from input stream with specified encoding |
| `useCharsequence(charsequence)` | Treats character sequence as whole stream     |
| `useCharsequence(charsequence, delimiter)` | Splits character sequence by delimiter |
| `useCharsequence(istream, encoding)` | Reads entire character sequence from input stream |
| `useCharsequence(istream, delimiter, encoding)` | Reads character sequence from input stream by delimiter |

---

## 🔐 Layer 8: hash.h / less.h — The Universal Language of the Container World

When you call `distinct()` on `Semantic<std::vector<E>>`, it internally needs `std::unordered_set<std::vector<E>>`. The standard library doesn't provide hash specialisations for `std::vector`—this is a "blank spot" left by the C++ standards committee. Semantic-Cpp's `hash.h` and `less.h` fill this gap entirely: providing complete hash and comparison support for all standard library containers (including nested containers), pairs, tuples, optionals, variants, chrono time types, complex numbers, etc. Containers of arbitrary depth and arbitrary combinations can now be keys in unordered_sets or elements in sets. 🌉

---

## 🚀 Performance Optimisation Recommendations

1.  **Choose the Right Container:** Use `toUnordered()` if order doesn't matter, `toOrdered()` or `sort()` if sorting is needed.
2.  **Make Good Use of Parallelism:** Use `parallel()` for large datasets or time-consuming processing logic, avoid blocking I/O.
3.  **Optimise Operation Order:** `filter` early, `sort` wisely.
4.  **Leverage Lazy Evaluation:** Intermediate operations don't execute immediately; `takeWhile` and `limit` can terminate early.

---

Semantic-Cpp — Building efficient, clear data processing pipelines with modern C++. 🚀🎯✨
