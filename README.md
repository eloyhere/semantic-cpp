# 🚀 Semantic-Cpp: A Future-Oriented Intelligent Stream Processing Framework for C++

Semantic-Cpp is a modern C++ stream processing library, completely redesigned from the ground up, featuring a **"multiple headers, zero external dependencies"** modular architecture. Each header file has a clear, singular responsibility and is independently testable, together forming a complete stream processing ecosystem. This library innovatively blends the essence of multiple programming paradigms:

*   **The Elegance and Fluidity of Java Stream API**: Chain calls, declarative programming, making code as graceful as poetry ✨
*   **The Laziness and Flexibility of JavaScript Generators**: Lazy evaluation, on-demand generation, memory-friendly 🌱
*   **The Efficiency and Order of Database Indexing**: Intelligent sorting, index-driven, a powerful tool for time-series data processing ⏱️
*   **The Batch Processing Philosophy of "Container-as-Element"**: Vectors, lists, maps... Any container can be a first-class citizen in the stream, flowing freely 📦

Are you tired of writing `for` loops to iterate over a `vector`, nesting an `if` for filtering, and manually `push_back` to another container? 😩
Have you ever debugged an off-by-one index bug late at night, just because you wanted the "third from the end" element while iterating backwards? 😵💫
Do you yearn to manipulate data like a database—pinpointing by index, analysing with sliding windows, completing the entire journey from data to statistics with a single chain of calls? 🤔

**Semantic-Cpp was born for this purpose. 🔧**

It abstracts data processing as operations on "elements" and their "logical positions (indices)"—much like "rows" and "primary keys" in a database. You can freely rearrange, offset, and reverse indices without touching the data itself; you can also pass any container (`vector`, `map`, `array`...) as an indivisible whole within the stream, and "unpack" it back to the element level at any time. This ability to freely switch between two granularities is absent in traditional stream frameworks. 🎯

---

## 🏗️ Project Architecture: Four Modules, Layered Design

Semantic-Cpp is organised into four module directories. Each header has a single responsibility and is independently testable. The modules are layered from encoding and bunch primitives up to the stream processing API:

```
semantic-cpp/
├── structure/          Foundation: type aliases, thread pool, data structures
│   ├── function.h
│   ├── pool.h
│   ├── hash.h
│   ├── less.h
│   ├── node.h          Tree node templates
│   ├── tree.h          Binary search tree
│   ├── ast.h           Abstract syntax tree
│   ├── trie.h          Prefix tree (trie)
│   ├── segment_tree.h  Segment tree
│   └── character_search_tree.h  Ternary character search tree
├── bunch/              Unicode characters, encodings and strings
│   ├── charset.h       Encoding selector
│   ├── encode.h        Codecs (UTF-8/16/32, ASCII, Latin1)
│   ├── meta.h          Meta
│   ├── point.h         Point / PointIterator
│   ├── character.h     Character
│   ├── unicode_data.h  Generated Unicode 15 tables
│   ├── unicode.h       Normalisation (NFC/NFD/NFKC/NFKD) and text styles
│   ├── sequence.h      Bunch
│   ├── builder.h       Builder
│   ├── buffer.h        Buffer
│   └── bunch.h         Umbrella header
├── localisation/       Message localisation
│   ├── localisation.h
│   └── unicode.md      Unicode concept notes and corrections
└── semantic/           Stream construction and collection
    ├── collector.h
    └── semantic.h
```

### 🧩 Dependency Graph
Each layer depends only on the layers beneath it. Encoding and value types are independent; bunch types build on them; the stream API builds on the bunch and foundation modules:

```
structure/function.h       ← No dependencies, the type foundation
structure/pool.h           ← structure/function.h
structure/hash.h, less.h   ← Standard library extensions, no internal dependencies
structure/node.h           ← Tree node templates, no internal dependencies
structure/tree.h           ← structure/node.h (binary search tree)
structure/ast.h            ← Standard library only (owning AST nodes)
structure/trie.h           ← structure/node.h (prefix tree)
structure/segment_tree.h   ← Standard library only (iterative segment tree)
structure/character_search_tree.h ← structure/node.h (ternary character search tree)
bunch/charset.h            ← Encoding primitives
bunch/encode.h             ← bunch/charset.h (codecs)
bunch/meta.h               ← Application metadata, no internal dependencies
bunch/point.h              ← Point / PointIterator, no internal dependencies
bunch/character.h          ← bunch/meta.h, bunch/point.h
bunch/unicode_data.h       ← Generated Unicode 15 tables, no internal dependencies
bunch/unicode.h            ← bunch/charset.h, bunch/encode.h, bunch/unicode_data.h
bunch/sequence.h           ← charset.h, encode.h, meta.h, point.h
bunch/builder.h            ← bunch/point.h, bunch/sequence.h
bunch/buffer.h             ← bunch/sequence.h, bunch/builder.h
bunch/bunch.h              ← All bunch headers
localisation/localisation.h ← bunch headers (message localisation, text styles)
semantic/collector.h       ← structure/function.h, structure/pool.h, bunch/bunch.h
semantic/semantic.h        ← All of the above
```

---

## 🌍 Namespace Overview

Semantic-Cpp meticulously designs several namespaces, each like an independent "department", with distinct responsibilities yet closely collaborating:

| Namespace     | Header File                              | Responsibility                                   | Core Types/Functions                                                          |
| :------------ | :--------------------------------------- | :----------------------------------------------- | :---------------------------------------------------------------------------- |
| function      | `structure/function.h`                   | Type system foundation                           | `Timestamp`, `Module`, `Generator<T>`, `Supplier<R>`, `Consumer<T>`, `Predicate<T>` etc. |
| pool          | `structure/pool.h`                       | Concurrent execution engine                      | `pool::ThreadPool`, `submit()`, `emergencyShutdown()`                         |
| node          | `structure/node.h`                       | Reusable tree node templates                     | `BinaryNode<T>`, `TrieNode<Char,T>`, `TernaryNode<Char,T>`                     |
| tree          | `structure/tree.h`                       | Ordered binary search tree                       | `tree::BinarySearchTree<T>`, `insert()`, `erase()`, `inorder()` etc.           |
| ast           | `structure/ast.h`                        | Abstract syntax tree                             | `ast::Tree<Kind,Value>`, `ast::Node<Kind,Value>`, `SourceRange`, traversals    |
| trie          | `structure/trie.h`                       | Prefix tree over character sequences             | `trie::Trie<Char,Value>`, `keysWithPrefix()`, `longestPrefixOf()`              |
| segment       | `structure/segment_tree.h`               | Range queries with point updates                 | `segment::SegmentTree<T,Operation>`, `query()`, `set()`, `update()`            |
| character     | `structure/character_search_tree.h`      | Ternary search tree over byte strings            | `character::CharacterSearchTree<Value>`, `keysWithPrefix()`, `match()`         |
| localisation  | `localisation/localisation.h`            | Message localisation and text styles             | `Locale`, `Catalog`, `Message`, `Arguments`, `TextFormat`, plural/number formatting |
| bunch         | `bunch/bunch.h`                          | Unicode characters, encodings and strings        | `charset`, `Character`, `Meta`, `Point`, `PointIterator`, `Bunch`, `Builder`, `Buffer`, `NormalizationForm`, `TextStyle` |
| collector     | `semantic/collector.h`                   | Terminal collection execution                    | `Collector<E,A,R>`, `Identity<A>`, `Accumulator<A,E>` etc.                    |
| collectable   | `semantic/semantic.h`                    | Materialised data containers                     | `Collectable<E>`, `OrderedCollectable<E>`, `UnorderedCollectable<E>` etc.     |
| semantic      | `semantic/semantic.h`                    | Stream construction & intermediate operations    | `Semantic<E>`, `useRange()`, `useFrom()` etc.                                 |

### 🔁 Namespace Collaboration Flow
Data flow between namespaces is like an assembly line in a factory—raw material enters from `semantic`, undergoes processing layer by layer, and is finally packaged and shipped from `collector`. Each step has a clear boundary of responsibility:

```cpp
semantic::useRange(0, 100)          // ← semantic namespace: create stream
    .map([](int x) { return x * 2; })   // ← semantic namespace: intermediate transform
    .filter([](int x) { return x > 50; }) // ← semantic namespace: intermediate filter
    .toUnordered()                  // ← Convert to collectable namespace
    .toVector();                    // ← Invoke collector from collector namespace
```

---

## 🛠️ Build, Test & Install

**Requirements:** CMake 3.21+ and a C++17 compiler (MSVC 2019+, GCC 9+, Clang 10+).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSEMANTIC_CPP_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Install the header-only package and its CMake package files:

```bash
cmake --install build --config Release --prefix <install-prefix>
```

Consume it from another CMake project:

```cmake
find_package(semantic-cpp CONFIG REQUIRED)
target_link_libraries(app PRIVATE semantic-cpp::semantic-cpp)
```

```cpp
#include <semantic-cpp/semantic/semantic.h>
#include <semantic-cpp/bunch/bunch.h>
```

When working inside this repository, include headers relative to the repository root (for example `#include "semantic/semantic.h"`). Every public header is compiled standalone by the test suite, so each one carries its own dependencies.

---

## 📦 Layer 1: structure/function.h — Type Foundation

`structure/function.h` defines the type system for the entire framework, the common foundation for all modules. 🔑

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

`Generator` is the core abstraction of the entire stream system. 🌀 It does not return data; instead, it accepts two callbacks—`accept` ("I'm ready, please accept this element") and `interrupt` ("should we stop?"). This inversion of control design means the data producer has no knowledge of the consumer; it simply "pushes" data at the appropriate moment. This is the essence of lazy evaluation: data only truly "flows" when `accept` is called; before that, everything is merely a description.

| Type Alias         | Full Definition                                     | Purpose                            |
| :----------------- | :-------------------------------------------------- | :--------------------------------- |
| Timestamp          | long long                                           | Logical position of an element in the stream |
| Module             | unsigned long long                                  | Counting, capacity, concurrency level |
| Runnable           | std::function<void()>                               | Parameterless, void-returning task |
| Supplier<R>        | std::function<R()>                                  | Supplier, creates from nothing     |
| Function<T,R>      | std::function<R(T)>                                 | Single-argument function           |
| BiFunction<T,U,R>  | std::function<R(T,U)>                               | Two-argument function              |
| TriFunction<T,U,V,R> | std::function<R(T,U,V)>                          | Three-argument function            |
| Unary<T>           | std::function<T(T)>                                 | Unary operation                    |
| Binary<T>          | std::function<T(T,T)>                               | Binary operation                   |
| Consumer<T>        | std::function<void(T)>                              | Consumer                           |
| BiConsumer<T,U>    | std::function<void(T,U)>                            | Two-argument consumer              |
| TriConsumer<T,U,V> | std::function<void(T,U,V)>                          | Three-argument consumer            |
| Predicate<T>       | std::function<bool(T)>                              | Predicate judgement                |
| BiPredicate<T,U>   | std::function<bool(T,U)>                            | Two-argument predicate             |
| TriPredicate<T,U,V>| std::function<bool(T,U,V)>                          | Three-argument predicate           |
| Comparator<T>      | std::function<int(const T&,const T&)>               | Comparator, returns negative/zero/positive |
| Generator<T>       | BiConsumer<BiConsumer<T,Timestamp>,<br>BiPredicate<T,Timestamp>> | Core abstraction for stream generation |

---

## 🧵 Layer 2: structure/pool.h — Concurrency Foundation

`structure/pool.h` provides the global thread pool `pool::ThreadPool`, the concurrency engine for the entire framework. 🚀 It employs a **declarative parallelism** design—when you write `.parallel(4)`, it does not immediately launch four threads to start processing. This line of code is merely a "declaration": telling the framework "I intend to use 4 threads for parallel processing". Actual parallel execution occurs when a terminal operation is invoked—that is, when you call collection methods like `toVector()`, `findFirst()`, `count()`, etc.

| Feature            | Description                                                                 |
| :----------------- | :------------------------------------------------------------------------- |
| Declarative Parallelism | `.parallel(4)` only declares "I want to use 4 threads", does not start immediately |
| Emergency Shutdown  | Built-in `emergencyShutdown()` and `std::set_terminate` handler           |
| Exception Propagation | `submit()` returns `std::future`, propagating exceptions safely to the main thread |

---

## 🔤 Layer 3: bunch/ + localisation/ — Unicode Characters and Strings

`bunch/charset.h` defines the encoding selector, `bunch/encode.h` implements the codecs, and `bunch/unicode.h` adds NFC/NFD/NFKC/NFKD normalisation, case mapping and text styles. The rest of the `bunch/` module builds code points, characters and immutable/mutable sequences on top of them. 🌍 Together they support UTF-8, UTF-16 (LE/BE), UTF-32 (LE/BE), ASCII, and Latin1. Invalid sequences are replaced with U+FFFD; see [Unicode concepts and corrections](localisation/unicode.md) for the precise terminology and the limits of the current implementation. `localisation/localisation.h` reuses these primitives for locale-aware messages.

| Type/Function    | Header                          | Description                                                                 |
| :--------------- | :------------------------------ | :-------------------------------------------------------------------------- |
| charset          | `bunch/charset.h`               | Encoding/encoding-scheme selector: ascii, utf8, utf16, utf16be, utf16le, utf32, utf32be, utf32le, latin1 |
| encode()         | `bunch/encode.h`                | Encode a single code point to code units of the specified encoding         |
| decode()         | `bunch/encode.h`                | Decode the next code point from code units, auto-advancing the input        |
| convert()        | `bunch/encode.h`                | Encoding conversion (supports string, vector, deque output)                 |
| NormalizationForm | `bunch/unicode.h`              | NFC/NFD/NFKC/NFKD form selector                                             |
| normalize()      | `bunch/unicode.h`               | Unicode normalisation: decompose, canonically order, recompose              |
| TextStyle        | `bunch/unicode.h`               | plain / lowercase / uppercase / capitalize / title                          |
| TextFormat       | `localisation/localisation.h`   | Locale-bound text style plus optional normalisation                         |
| Meta             | `bunch/meta.h`                  | Application metadata wrapper; not a Unicode property                        |
| Point            | `bunch/point.h`                 | Numeric Unicode code point, with scalar/noncharacter/surrogate predicates   |
| PointIterator    | `bunch/point.h`                 | Bidirectional iterator over code points                                     |
| Character        | `bunch/character.h`             | A code point plus application metadata                                      |
| Bunch            | `bunch/sequence.h`              | Immutable code-point sequence: split, replace, indexOf, lastIndexOf, sub, trim, toUpperCase, toLowerCase, reverse, startsWith, endsWith, contains, compare, getBytes, getPoints, getMetas, getCharacters, repeat, concat, count, join |
| Builder          | `bunch/builder.h`               | Mutable byte builder: prepend, insert, append (supports primitives, Point, Bunch, string_view), toBunch() |
| Buffer           | `bunch/buffer.h`                | Thread-safe byte ring buffer: write, read, peek, prepend, append, clear, shrinkToFit, data, size, capacity, atomic |

---

## ⚙️ Layer 4: semantic/collector.h — Collector Framework & Factories

`semantic/collector.h` is the core collector module of Semantic-Cpp, combining the collector framework with factory functions.

### 🧩 Five-Stage Model
```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (optional short-circuit)
```

| Type Alias       | Full Definition                                  | Role                           |
| :--------------- | :----------------------------------------------- | :----------------------------- |
| Identity<A>      | function::Supplier<A>                            | Provides initial value         |
| Accumulator<A,E> | function::TriFunction<A, E, Timestamp, A>        | Accumulates elements           |
| Combiner<A>      | function::BiFunction<A, A, A>                    | Combines parallel results      |
| Finisher<A,R>    | function::Function<A, R>                         | Final transformation           |
| Interrupt<E,A>   | function::TriPredicate<E, Timestamp, A>          | Short-circuit judgement        |

### Collector Factory Functions

#### ✅ Matching Operations
| Method                      | Description                | Return Type |
| :-------------------------- | :------------------------- | :---------- |
| `useAllMatch(predicate)`    | All elements match condition | `bool`      |
| `useAnyMatch(predicate)`    | Any element matches condition | `bool`      |
| `useNoneMatch(predicate)`   | No element matches condition | `bool`      |

#### 🔍 Finding Operations
| Method              | Description                      | Return Type       |
| :------------------ | :------------------------------- | :---------------- |
| `useFindFirst()`    | Find the first element           | `std::optional<E>` |
| `useFindLast()`     | Find the last element            | `std::optional<E>` |
| `useFindAny()`      | Find any (random) element        | `std::optional<E>` |
| `useFindAt(index)`  | Locate at specified index (supports negative) | `std::optional<E>` |
| `useFindMaximum()`  | Find maximum element             | `std::optional<E>` |
| `useFindMinimum()`  | Find minimum element             | `std::optional<E>` |

#### 🔢 Aggregation Operations
| Method             | Description  | Return Type |
| :----------------- | :----------- | :---------- |
| `useCount()`       | Total number of elements | `Module` |
| `useSummate<E,D>()`| Summation    | `D`         |
| `useAverage<E,D>()`| Average      | `D`         |
| `useRange<E,D>()`  | Numeric range (max - min) | `D` |

#### 📉 Statistical Operations
| Method                          | Description                | Return Type         |
| :------------------------------ | :------------------------- | :------------------ |
| `useVariance<E,D>()`            | Population variance        | `D`                 |
| `useStandardDeviation<E,D>()`   | Population standard deviation | `D`              |
| `useSkewness<E,D>()`            | Skewness                   | `D`                 |
| `useKurtosis<E,D>()`            | Kurtosis                   | `D`                 |
| `useMedian<E,D>()`              | Median                     | `std::optional<D>`  |
| `useMode<E>()`                  | Mode (frequency analysis)  | `std::optional<E>`  |
| `usePercentile<E,D>(p)`         | p-th percentile           | `std::optional<D>`  |
| `useFrequency<E>()`             | Frequency domain features  | `std::map<E, complex>` |
| `useDistribution<E>()`          | Spatial distribution features | `std::map<E, complex>` |

#### 🔀 Reduction Operations
| Method                              | Description          | Return Type       |
| :---------------------------------- | :------------------- | :---------------- |
| `useReduce(reducer)`                | Reduction without identity | `std::optional<E>` |
| `useReduce(identity, reducer)`     | Reduction with identity | `E`              |
| `useReduce(id, red, comb, fin)`    | Fully custom reduction | `R`              |

#### 🧺 Collection to Containers
| Method                                                              | Return Type |
| :------------------------------------------------------------------ | :---------- |
| `useToVector()`                                                     | `std::vector<E>` |
| `useToList()`                                                       | `std::list<E>` |
| `useToDeque()`                                                      | `std::deque<E>` |
| `useToForwardList()`                                                | `std::forward_list<E>` |
| `useToArray<N>()`                                                   | `std::array<E, N>` |
| `useToSet()`                                                        | `std::set<E>` |
| `useToMultiset()`                                                   | `std::multiset<E>` |
| `useToUnorderedSet()`                                               | `std::unordered_set<E>` |
| `useToUnorderedMultiset()`                                          | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)`                                            | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)`                            | `std::map<K, V>` |
| `useToMultimap(keyExtractor)`                                       | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)`                       | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)`                   | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)`                              | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)`              | `std::unordered_multimap<K, V>` |
| `useToStack()`                                                      | `std::stack<E>` |
| `useToQueue()`                                                      | `std::queue<E>` |
| `useToPriorityQueue()`                                              | `std::priority_queue<E>` |

#### 🧩 Grouping & Partitioning
| Method                                              | Return Type |
| :-------------------------------------------------- | :---------- |
| `useGroup(keyExtractor)`                            | `std::unordered_map<K, vector<E>>` |
| `useGroupBy(keyExtractor, valueExtractor)`         | `std::unordered_map<K, vector<V>>` |
| `usePartition(size)`                                | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)`                      | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor, valueExtractor)`      | `std::vector<vector<V>>` |

#### 📄 Bunch Output
| Method                               | Return Type |
| :----------------------------------- | :---------- |
| `useJoin()` / `useOut()` / `useError()` and their overloads | `bunch::Bunch` |

#### 📊 Mathematical Tools
| Method            | Return Type |
| :---------------- | :---------- |
| `useDFT()`        | `vector<complex<double>>` |
| `useIDFT()`       | `vector<complex<double>>` |
| `useFFT()`        | `vector<complex<double>>` |
| `useIFFT()`       | `vector<complex<double>>` |
| `useGradient()`   | `vector<double>` |

---

## 📦 Layer 5: semantic/semantic.h — Stream Intermediate Operations & Collection System

### 🧩 Core Design: Three-Stage Pipeline
```
Semantic<E> (Construction & Transformation)
    ↓ toUnordered() / toOrdered() / sort() / toWindow() / toStatistics()
Collectable<E> (Materialisation & Collection)
    ↓ toVector() / findFirst() / count() / summate() / ...
Final Result
```

**Key Rule**: A `Semantic<E>` must first be converted to a `Collectable<E>` via `toUnordered()`, `toOrdered()`, `toWindow()`, `toStatistics()`, or `sort()` before terminal methods can be called.

### 🧭 Five Materialisation Paths
| Conversion Method      | Target Type            | Underlying Data Structure | Performance Characteristic |
| :--------------------- | :--------------------- | :------------------------ | :------------------------- |
| `toUnordered()`        | `UnorderedCollectable` | `unordered_map`           | Average O(1) lookup        |
| `toOrdered()`          | `OrderedCollectable`   | `map`                     | O(log n) lookup            |
| `sort()`               | `OrderedCollectable`   | `map` (value-sorted)      | O(log n) lookup            |
| `toWindow()`           | `WindowCollectable`    | Inherits ordered collection | Supports slide/tumble      |
| `toStatistics<D>()`    | `Statistics<E,D>`      | Inherits ordered collection | 30+ statistical methods    |

### 📋 Collectable<E> — All Terminal Methods (Alphabetical Order)
| Method                                                | Return Type                     | Description                                     |
| :---------------------------------------------------- | :----------------------------- | :---------------------------------------------- |
| `allMatch(predicate)`                                 | `bool`                         | All elements match condition                    |
| `anyMatch(predicate)`                                 | `bool`                         | Any element matches condition                   |
| `average<D>()`                                        | `D`                            | Average                                         |
| `average<D>(mapper)`                                  | `D`                            | Average after mapping                           |
| `collect(identity, acc, comb, fin)`                  | `R`                            | Custom four-stage collection                    |
| `collect(identity, interrupt, acc, comb, fin)`        | `R`                            | Custom interruptible collection                 |
| `count()`                                             | `Module`                       | Total number of elements                        |
| `empty()`                                             | `bool`                         | Is the stream empty?                            |
| `error()`                                             | `void`                         | Output to stderr (supports delimiter/prefix/suffix/converter) |
| `findAny()`                                           | `std::optional<E>`             | Find any (random) element                       |
| `findAt(index)`                                       | `std::optional<E>`             | Find element at specified index (supports negative) |
| `findFirst()`                                         | `std::optional<E>`             | Find the first element                          |
| `findLast()`                                          | `std::optional<E>`             | Find the last element                           |
| `findMaximum()`                                       | `std::optional<E>`             | Find the maximum element                        |
| `findMaximum(comparator)`                             | `std::optional<E>`             | Find maximum with custom comparator             |
| `findMinimum()`                                       | `std::optional<E>`             | Find the minimum element                        |
| `findMinimum(comparator)`                             | `std::optional<E>`             | Find minimum with custom comparator             |
| `forEach(consumer)`                                   | `void`                         | Perform side-effect for each element            |
| `group(keyExtractor)`                                 | `unordered_map<K, vector<E>>`  | Group by key                                    |
| `groupBy(keyExtractor, valueExtractor)`              | `unordered_map<K, vector<V>>`  | Group by key and extract value                  |
| `join()`                                              | `Bunch`                  | Join with default format                        |
| `join(delimiter)`                                     | `Bunch`                  | Join with custom delimiter                      |
| `join(prefix, delimiter, suffix)`                    | `Bunch`                  | Join with fully custom format                   |
| `noneMatch(predicate)`                                | `bool`                         | No element matches condition                    |
| `out()`                                               | `Bunch`                  | Output to stdout (supports delimiter/prefix/suffix/converter) |
| `partition(size)`                                     | `vector<vector<E>>`             | Partition by fixed size                         |
| `partitionBy(keyExtractor)`                           | `vector<vector<E>>`             | Partition by index key                          |
| `partitionBy(keyExtractor, valueExtractor)`           | `vector<vector<V>>`             | Partition by index key and extract value        |
| `range<D>()`                                          | `D`                            | Numeric range (max - min)                       |
| `range<D>(mapper)`                                    | `D`                            | Numeric range after mapping                     |
| `reduce(accumulator)`                                 | `std::optional<E>`             | Reduction without identity                      |
| `reduce(identity, accumulator)`                       | `E`                            | Reduction with identity                         |
| `reduce(identity, acc, comb)`                         | `R`                            | Fully custom reduction                          |
| `summate<D>()`                                        | `D`                            | Summation                                       |
| `summate<D>(mapper)`                                  | `D`                            | Summation after mapping                         |
| `toArray<N>()`                                        | `std::array<E, N>`             | Collect into fixed-size array                   |
| `toDeque()`                                           | `std::deque<E>`                | Collect into deque                              |
| `toForwardList()`                                     | `std::forward_list<E>`         | Collect into forward_list                       |
| `toList()`                                            | `std::list<E>`                 | Collect into list                               |
| `toMap(keyExtractor)`                                 | `std::map<K, E>`               | Collect into map by key                         |
| `toMap(keyExtractor, valueExtractor)`                 | `std::map<K, V>`               | Collect into map with custom key & value        |
| `toMultimap(keyExtractor)`                            | `std::multimap<K, E>`          | Collect into multimap by key                    |
| `toMultimap(keyExtractor, valueExtractor)`            | `std::multimap<K, V>`          | Collect into multimap with custom key & value   |
| `toMultiset()`                                        | `std::multiset<E>`             | Collect into multiset                           |
| `toPriorityQueue()`                                   | `std::priority_queue<E>`       | Collect into priority_queue                     |
| `toQueue()`                                           | `std::queue<E>`                | Collect into queue                              |
| `toSet()`                                             | `std::set<E>`                  | Collect into set (unique & sorted)              |
| `toStack()`                                           | `std::stack<E>`                | Collect into stack                              |
| `toUnorderedMap(keyExtractor, valueExtractor)`        | `std::unordered_map<K, V>`     | Collect into unordered_map                      |
| `toUnorderedMultimap(keyExtractor)`                   | `std::unordered_multimap<K, E>`| Collect into unordered_multimap by key          |
| `toUnorderedMultimap(keyExtractor, valueExtractor)`   | `std::unordered_multimap<K, V>`| Collect into unordered_multimap with custom key & value |
| `toUnorderedMultiset()`                               | `std::unordered_multiset<E>`   | Collect into unordered_multiset                 |
| `toUnorderedSet()`                                    | `std::unordered_set<E>`        | Collect into unordered_set                      |
| `toVector()`                                          | `std::vector<E>`               | Collect into vector                             |

### 📈 Statistics<E,D> — Statistical Methods
| Method                 | Return Type            | Description                     |
| :--------------------- | :--------------------- | :------------------------------ |
| `summate()`            | `D`                    | Summation                       |
| `average()`            | `D`                    | Average                         |
| `minimum()`            | `std::optional<D>`    | Minimum value                   |
| `maximum()`            | `std::optional<D>`    | Maximum value                   |
| `range()`              | `D`                    | Range (max - min)               |
| `variance()`           | `D`                    | Population variance             |
| `standardDeviation()`  | `D`                    | Population standard deviation    |
| `median()`             | `std::optional<D>`    | Median                          |
| `mode()`               | `std::optional<E>`    | Mode                            |
| `percentile(p)`        | `std::optional<D>`    | p-th percentile                 |
| `firstQuartile()`      | `std::optional<D>`    | First quartile (Q1)             |
| `thirdQuartile()`      | `std::optional<D>`    | Third quartile (Q3)             |
| `interquartileRange()` | `std::optional<D>`    | Interquartile range (IQR)       |
| `skewness()`           | `D`                    | Skewness                        |
| `kurtosis()`           | `D`                    | Kurtosis                        |
| `frequency()`          | `map<E, complex>`     | Frequency domain features       |
| `distribute()`         | `map<E, complex>`     | Spatial distribution features   |
| `dft()`                | `vector<complex<double>>` | Discrete Fourier Transform    |
| `idft()`               | `vector<complex<double>>` | Inverse Discrete Fourier Transform |
| `fft()`                | `vector<complex<double>>` | Fast Fourier Transform       |
| `ifft()`               | `vector<complex<double>>` | Inverse Fast Fourier Transform |
| `gradient(...)`        | `vector<double>`      | Gradient descent               |

All the above methods also support an optional `mapper` parameter version.

### 🔧 Semantic<E> Intermediate Operation Methods
| Category      | Method        | Description                                   |
| :------------ | :------------ | :-------------------------------------------- |
| Element Transform | map         | One-to-one mapping transformation             |
|               | flatMap     | One-to-many mapping and flattening            |
|               | flat        | Flatten nested streams (supports Semantic and containers) |
| Element Filter | filter      | Conditional filtering                         |
|               | takeWhile   | Take while condition holds                    |
|               | dropWhile   | Drop while condition holds                    |
|               | distinct    | Remove duplicates (supports custom comparator) |
| Size Control  | limit       | Limit number of elements                      |
|               | skip        | Skip first n elements                         |
|               | sub         | Extract sub-range [start, end)                |
| Index Operations | redirect    | Remap indices                                 |
|               | reverse     | Reverse indices                               |
|               | translate   | Offset indices                                |
| Observation   | peek        | Observe each element (does not modify stream)  |
| Parallel Declaration | parallel(n) | Declare parallelism level                     |
| Concatenation | concatenate | Concatenate Semantic/elements/generators/containers |
| Terminal Conversion | toUnordered / toOrdered / toWindow / toStatistics / sort | Convert to Collectable |

---

## 🔧 Layer 6: semantic/semantic.h — Stream Construction Factories

### 🔢 Numeric Range Generation
| Method                          | Description                     |
| :------------------------------ | :------------------------------ |
| `useRange(start, end)`         | Generate range [start, end)     |
| `useRange(start, end, step)`   | Range with step (supports negative) |
| `useRangeClosed(start, end)`   | Generate closed range [start, end] |
| `useRangeClosed(start, end, step)` | Closed range with step      |

### ♾️ Infinite Stream Generation
| Method                        | Description                         |
| :---------------------------- | :--------------------------------- |
| `useInfinite(seed, generator)`| Infinite iteration from seed value  |
| `useGenerate(supplier)`       | Infinite calls to supplier         |
| `useGenerate(supplier, limit)`| Finite number of calls to supplier |
| `useIterate(seed, generator)` | Infinite iteration from seed value  |
| `useIterate(seed, generator, limit)` | Finite number of iterations  |
| `useRandom()`                 | Infinite stream of random integers |
| `useRandom(min, max)`         | Random number stream in specified range |
| `useRandom(min, max, count)` | Random number stream with specified range and count |

### 📦 Container & Element Construction
| Method                    | Description                     |
| :------------------------ | :----------------------------- |
| `useEmpty()`              | Create an empty stream         |
| `useOf(element)`          | Create stream from a single element |
| `useOf(e1, e2)`           | Create stream from two elements |
| `useOf(e1, e2, e3)`       | Create stream from three elements |
| `useOf({...})`            | Create stream from initialiser list |
| `useFrom(container)`      | Create stream from standard container |
| `useFrom({...})`          | Create stream from initialiser list |
| `useRepeat(element, count)` | Repeat element n times         |

### 📄 Text & Unicode Processing
| Method                        | Description                         |
| :---------------------------- | :--------------------------------- |
| `useBlob(text)`               | Split string into char stream by bytes |
| `useBlob(text, start, end)`   | Split specified range by bytes     |
| `useBlob(istream)`            | Read from input stream line by line |
| `useBlob(istream, delimiter)` | Read from input stream by delimiter |
| `useText(text)`               | Whole text stream (Bunch)   |
| `useText(text, delimiter)`    | Split text by delimiter            |
| `useText(istream)`            | Read entire content from input stream |
| `useSequence(string)`   | Create code point stream from character sequence |
| `useSequence(text, encoding)` | Create code point stream from text with specified encoding |
| `useBunch(bunch)` | Code-point sequence as a whole stream |
| `useBunch(bunch, delimiter)` | Split a code-point sequence by delimiter |

---

## 🔐 Layer 7: structure/hash.h / structure/less.h — The Universal Language of the Container World

Provides complete hash and comparison support for all standard library containers (including nested containers), `pair`, `tuple`, `optional`, `variant`, `chrono` time types, `complex` numbers, and more. Containers nested to any depth and in any combination can now be used as keys in `unordered_set` or elements in `set`. 🌉

---

## 🚀 Performance Optimisation Tips

1.  **Choose the Right Container**: Use `toUnordered()` if order doesn't matter, `toOrdered()` or `sort()` if sorting is needed.
2.  **Leverage Parallelism**: Use `parallel()` for large datasets.
3.  **Optimise Operation Order**: Filter early, sort wisely.
4.  **Utilise Lazy Evaluation**: `takeWhile` and `limit` can terminate early.

---

Semantic-Cpp — Building efficient, clear data processing pipelines with modern C++. 🚀🎯✨
