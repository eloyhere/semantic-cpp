# Semantic-Cpp In-Depth: A Future-Oriented C++ Intelligent Stream Processing Framework

# Introduction
Semantic-Cpp is a comprehensively redesigned modern C++ stream processing framework, architected for simplicity and efficiency as a "single-header, zero-dependency" library. Its entire functionality is encapsulated within the core  semantic.h  header file. The framework is a creative synthesis of the most effective paradigms from across the programming landscape. It draws its fluid, chainable API and declarative programming model from the elegance of the Java Stream API, enabling developers to express complex data transformations with clear, readable code. Simultaneously, it incorporates the laziness and on-demand generation capabilities intrinsic to JavaScript Generator functions, ensuring that data flows through the processing pipeline only when explicitly required, thereby optimising resource utilisation. Furthermore, the library introduces a novel, index-centric data model, inspired by the efficiency and ordering principles of database indexing. This model provides built-in intelligent sorting and index-driven mechanisms, making the framework particularly adept at handling time-series data, event streams, and any scenario where the logical position of data is as crucial as its value.

Diverging from conventional data-processing approaches—such as manual loop writing or intricate asynchronous callback structures—Semantic-Cpp is engineered to deliver a type-safe, highly expressive, and high-performance solution. Its foundational philosophy centres on precise control of data flow: every element within a stream is coupled with a logical index, and the movement, ordering, and transformation of this data can be meticulously managed through index operations. This design ensures optimal performance and clarity, allowing developers to focus on the "what" of data processing rather than the "how."

---

## The Core Soul: An Index-Driven Data World

Semantic-Cpp abstracts data processing as operations on “elements” and their “logical positions (indices)”. Understanding this is the key to mastering the library.

### 1. Basic Index Transformations
Indices determine the logical order of elements within the processing chain and can be manipulated flexibly:
-   **`redirect(redirection function)`**: the core method. You can completely rewrite an element’s index using a custom function—for example, doubling the index or generating a new one based on the element’s value.
-   **`reverse()`**: a convenient method, implemented internally via `redirect`, that logically inverts the current indices (e.g., positive indices become negative).
-   **`translate(offset)`**: adds a fixed offset to all indices.

### 2. The “Overriding” Rules of Sorting
The sorting operation (`sorted`) has the highest priority in the library and its behaviour is deterministic:
-   **`sorted()` overrides everything**: no matter how complex the previous index transformations via `redirect` or `reverse` may have been, calling `sorted()` will **override** all prior index operations. The system will re-assign natural-order indices starting from 0 based on the **actual values** of the elements.
-   **Immediate materialisation into an ordered collection**: to avoid unnecessary repeated sorting later, the `sorted()` method **immediately** returns an object of type `OrderedCollectable`. This means the data has already been collected and sorted at this point.

### 3. Declarative Parallel Processing
Parallel processing becomes remarkably simple and intuitive:
-   **`parallel(thread count)` is merely a declaration**: calling this method simply expresses the intention “I would like subsequent operations to execute in parallel” and specifies the desired number of threads; **it does not launch any threads or submit tasks immediately**.
-   **Terminal operations trigger parallelism**: real parallel computation is triggered only when a **terminal operation** such as `toUnordered()`, `toOrdered()`, `count()`, etc. is called. At that point, the library’s built-in thread pool automatically shards the data and submits tasks according to the declared thread count.
-   **No manual management required**: you do not need to concern yourself with thread creation, task distribution, or result merging—the library handles everything automatically.

### 4. How to Choose the Final Data Container?
Depending on your performance requirements and operation type, you may select different terminal conversion methods:

| Conversion Method | Underlying Data Structure | Performance Characteristics | Best Use Cases |
| :--- | :--- | :--- | :--- |
| **`sorted()`** | `std::map<Index, Element>` | O(log n) access, strictly preserves element order. | Pagination, range queries, time-series analysis, rolling statistics. |
| **`sorted(comparator)`** | `std::map<Index, Element>` | O(log n) access, sorted by custom rules. | Pagination or range queries with custom sorting. |
| **`toOrdered()`** | `std::map<Index, Element>` | O(log n) access, preserves the **current index** order. | When you wish to retain the index order defined by operations such as `redirect` and perform ordered operations. |
| **`toUnordered()`** | `std::unordered_map<Index, Element>` | Average O(1) access, **highest performance**, but order is not guaranteed. | Fast lookups, deduplication statistics, aggregation calculations—scenarios where order does not matter. |
| **`toWindow()`** | Map-based window view | O(log n), supports sliding or rolling windows on ordered datasets. | Real-time stream analysis, sliding-window aggregation, event session partitioning. |

> **Important Note**: `WindowCollectable` (returned by `toWindow()`) internally relies on an ordered collection (implemented via `toOrdered()`) to ensure that window sliding and rolling operations execute correctly based on a deterministic order.

---

## Quick Start Guide

### Installation
Simply place the `semantic.h` header file in your project and ensure your compiler supports C++17 or higher.
```cpp
#include "semantic.h"
// Optional: use the semantic namespace
using namespace semantic;
```

### Basic Example: Experiencing Indices and Sorting
```cpp
#include <iostream>
#include "semantic.h"

int main() {
    auto result = semantic::useRange(0, 10)   // 1. Create an integer stream from 0 to 9
        .map([](int x) -> int { return x * x; })    // 2. Square each element (0,1,4,9...81)
        .redirect([](int value, auto index) -> long long {    // 3. Index redirection: double the index
            return index * 2;                  // Indices are now 0,2,4,6...
        })
        .reverse()                           // 4. Logically reverse indices (...,6,4,2,0)
        .sorted()                            // 5. ⚠️ Force re-sort by element value (1,4,9...)!
                                             //    All previous index operations are overridden; indices become 0,1,2...
        .toList();                           // 6. Collect into std::vector

    // Output: 0 1 4 9 16 25 36 49 64 81 (sorted)
    for (auto& item : result) {
        std::cout << item << " ";
    }
    return 0;
}
```

### Parallel Processing Example
```cpp
#include <iostream>
#include "semantic.h"

int main() {
    // 1. Build a stream-processing pipeline and declare the desire to use 4 threads for parallel execution.
    auto dataStream = semantic::useRange(1, 1000)
        .parallel(4)                         // Declare parallelism; not yet executed
        .filter([](int x) -> bool {
            return x % 2 == 0;               // Filter even numbers
        })
        .filter([](int x, auto index) -> bool {
            return index < 5LL;              // Further filter elements whose logical index is less than 5
        });

    // 2. The terminal operation `count()` triggers genuine parallel computation
    //    The thread pool starts, data is sharded, four threads count concurrently, and results are automatically merged.
    auto result = dataStream
        .toUnordered()                       // Convert to unordered collection for parallel processing
        .count();                            // Count the final number of elements

    std::cout << "Number of elements after filtering: " << result << std::endl;
    return 0;
}
```

### Time-Series and Window Analysis Example
```cpp
#include <iostream>
#include "semantic.h"

int main() {
    // Simulate a time-series dataset (e.g. stock prices)
    auto timeSeries = semantic::useFrom(std::vector<double>{1.1, 2.2, 3.3, 4.4, 5.5});

    // 1. Convert to window view
    auto windowStats = timeSeries
        .toWindow()                     // Convert to WindowCollectable
        .slide(3, 1)                    // Define a sliding window of size 3 with step 1
                                        // Window 1: {1.1, 2.2, 3.3}
                                        // Window 2: {2.2, 3.3, 4.4}
                                        // Window 3: {3.3, 4.4, 5.5}
        .sub(1, 4)                      // Take windows with indices 1 to 3 (i.e. windows 2 and 3)
        .map([](auto&& window) -> double { // Process each window
            // Calculate the average of each window
            return window
                .toStatistics<double, double>() // Convert window to Statistics for mathematical operations
                .average();
        })
        .toStatistics<double, double>() // Perform ordered statistics on the sequence of averages
        .summate();                     // Sum the averages of all selected windows

    std::cout << "Sum of averages of the selected sliding windows: " << windowStats << std::endl;
    // Output: result of ( (2.2+3.3+4.4)/3 + (3.3+4.4+5.5)/3 )
    return 0;
}
```

---

## Core API Quick Reference

### Stream Builders (Stream Sources)
| Method | Description | Example |
| :--- | :--- | :--- |
| `useRange(start, end)` | Generate an integer stream within a numeric range. | `useRange(0, 10)` |
| `useFrom(container)` | Create a stream from a standard container (e.g. vector, list). | `useFrom(std::vector{1,2,3})` |
| `useOf(args...)` | Create a stream from a variadic argument list. | `useOf(1, 2, 3, 4, 5)` |
| `useBlob(text)` | Split a string into a stream by character. | `useBlob("Hello")` |
| `useBlob(text, start, end)` | Split a string into a stream by character within a range. | `useBlob("Hello", 0, 3)` |
| `useBlob(istream)` | Split an input stream into a stream by character. | `useBlob(istream)` |
| `useBlob(istream, start, end)` | Split an input stream into a stream by character within a range. | `useBlob(istream, 0, 3)` |
| `useText(text)` | Treat the entire text as a single-element stream. | `useText("Hello")` |
| `useText(text, delimiter)` | Split text into a stream by delimiter. | `useText("Hello", 'e')` |

### Intermediate Operations
| Method | Description | Notes |
| :--- | :--- | :--- |
| `map(transform function)` | Transform elements into another form. | The function may receive `(element)` or `(element, index)`. |
| `filter(predicate function)` | Filter elements that satisfy the condition. | Predicate may be based on `(element)` or `(element, index)`. |
| `distinct()` | Remove duplicate elements. | A custom comparator may be supplied. |
| `limit(n)` | Limit the stream to the first `n` elements. | |
| `skip(n)` | Skip the first `n` elements of the stream. | |
| `sub(start, end)` | Obtain a sub-stream of elements whose indices are in the range `[start, end)`. | Similar to a string’s `substr`. |

### Index Operations
| Method | Description | Key Features |
| :--- | :--- | :--- |
| `redirect(redirection function)` | Core method allowing full control over each element’s index. | Function signature: `(element, old index) -> new index`. |
| `reverse()` | Logically invert the indices of all current elements. | Implemented internally via `redirect`. |
| `translate(offset)` | Add a fixed offset to all element indices. | |
| **`sorted()`** | **Force sorting**. Sort elements in ascending order by value, **overriding all existing indices**. | Immediately returns `OrderedCollectable`. |
| **`sorted(comparator)`** | Force sorting with a custom comparator. | Immediately returns `OrderedCollectable`. |

### Parallel Declaration
| Method | Description | Execution Timing |
| :--- | :--- | :--- |
| `parallel()` | Declare the default parallel strategy (usually number of CPU cores). | Triggered by subsequent **terminal operations**. |
| `parallel(n)` | Declare the desire to use `n` threads for parallel processing. | Triggered by subsequent **terminal operations**. |

### Terminal Conversions (Trigger Computation)
| Method | Description | Internal State |
| :--- | :--- | :--- |
| `toOrdered()` | Convert to an ordered collectable, preserving the **current index order**. | Materialised as `std::map<Index, Value>`. |
| `toUnordered()` | Convert to an unordered collectable for maximum performance. | Materialised as `std::unordered_map<Index, Value>`. |
| `toWindow()` | Convert to a window collectable for sliding/rolling analysis. | Internally based on `toOrdered()`. |

### Terminal Actions (Produce Final Result)
| Method | Description | Return Type |
| :--- | :--- | :--- |
| `anyMatch(predicate)` | Check whether any element satisfies the condition; exits immediately if found. | Boolean |
| `allMatch(predicate)` | Check whether all elements satisfy the condition; exits as soon as one fails. | Boolean |
| `noneMatch(predicate)` | Check whether no elements satisfy the condition; exits as soon as one succeeds. | Boolean |
| `forEach(consumer)` | Iterate over all elements in the stream. | Void |
| `count()` | Count the total number of elements in the stream. | `Module` (`unsigned long long`) |
| `average()` | Compute the average of numeric elements. | Average of the element type (e.g. `double`).  Only for Statistics|
| `findAny()` | Find any element at random. | A random element from the stream. |
| `findFirst()` | Find the first element. | The first element in the stream. |
| `findLast()` | Find the last element. | The last element in the stream. |
| `findAt(none negative index)` | Find the nth element; | The element at the specified index. |
| `findNegativeAt(negative index)` | Find the the (size + index)th element. | The element at the specified index. |
| `findMinimum()` / `findMaximum()` | Find the minimum/maximum value in the stream. | `std::optional<ElementType>` |
| `reduce(accumulator)` | Reduce the stream to a single value (e.g. sum). | Type of the accumulator result. |
| `reduce(identity, accumulator)` | Reduce the stream to a single value (e.g. sum). | Type of the accumulator result. |
| `collect(collector)` | Perform complex aggregation with a custom collector. | Type defined by the collector. |
| `toList()` / `toVector()` | Collect all elements into a list/vector. | `std::vector<E>` |
| `toSet()` | Collect all elements into a set (deduplicated). | `std::set<ElementType>` |
| `group(keyExtractor)` | Group into a Map (deduplicated). | `std::map<K, std::vector<E>>` |
| `toMap(keyExtractor)` | Collect into a Map (deduplicated). | `std::map<K, E>` |

---

## Advanced Topics and Best Practices

### Architectural Essence: Lazy Evaluation and Precise Callback Control
Behind every stream operation is a “generator” that accepts two callback functions:
-   **`accept(element, index)`**: called by downstream operations when they are ready to receive data, “pulling” an element on demand.
-   **`interrupt(element, index)`**: called before processing each element; if it returns `true`, the entire processing chain **terminates immediately**.
This mechanism ensures data is “pulled on demand” and can be terminated early at any time, avoiding unnecessary computation.

### Performance Optimisation Suggestions
1.  **Choose the right container**:
    -   For equality lookups, deduplication, or unsorted aggregation → prefer `toUnordered()`.
    -   For range queries, sorting, or pagination → use `toOrdered()` or `sorted()`.
    -   For real-time window analysis → use `toWindow()`.
2.  **Make good use of parallelism**:
    -   Parallelism via `parallel()` usually yields benefits when the dataset is large (e.g. >1,000 items) or when processing logic (`map`, `filter`) is computationally expensive.
    -   Avoid blocking I/O operations inside parallel streams.
3.  **Optimise operation order**:
    -   **Filter early (`filter`)**: reduce the data volume with `filter` before applying expensive `map` transformations.
    -   **Be strategic with sorting**: sorting is costly. If subsequent operations (e.g. `distinct`) do not depend on order, perform them before sorting.

### Custom Collectors
When the built-in terminal operations are insufficient, you can create custom collectors to implement complex reduction logic.
```cpp
// Create a collector that joins numbers into a specially formatted string
auto myCollector = semantic::collector::useFull<int, std::string, std::string>(
     []() -> std::string { return ""; }, // Supplier: initial accumulator value
    [](std::string acc, int val, auto idx) -> std::string { // Accumulator
        if (!acc.empty()) acc += "|";
        return acc + "Num(" + std::to_string(val) + ")";
    },
    [](std::string a, std::string b) -> std::string { // Combiner (for parallelism)
        if (a.empty()) return b;
        if (b.empty()) return a;
        return a + "|" + b;
    },
    [](std::string acc) -> std::string { // Finisher: final processing of the result
        return "[" + acc + "]";
    }
);

auto result = semantic::useRange(1, 5)
    .toOrdered() // Trigger computation
    .collect(myCollector); // Use the custom collector

std::cout << result << std::endl; // Output: [Num(1)|Num(2)|Num(3)|Num(4)]
```

### Text Processing Example
```cpp
auto text = semantic::useText("Hello 世界！")
    .map([](const std::string& text) -> std::string {
        return "<" + text + ">";
    })
    .toOrdered()
    .join(" "); // Join all characters with spaces

std::cout << text << std::endl;
// Output: <H><e><l><l><o>< ><世><界><！>
```

---

### Comparison with the C++ Standard Library and Other Competitors

To help you better understand Semantic-Cpp’s design positioning and use cases, the following table compares it with several mainstream data-processing solutions in the C++ community.

| Feature / Library | **Semantic-Cpp** | **C++20/23 `std::ranges` + `std::views`** | **Range-v3 Library** | **Traditional Hand-Written Loops** |
| :--- | :--- | :--- | :--- | :--- |
| **Core Paradigm** | **Declarative, index-driven** stream processing. Data is abstracted as an “element + logical index” pipeline, emphasising **order control**. | **Declarative, view-driven** functional composition. Provides adapters (`views::transform`, `views::filter`) for lazy computation. | **Declarative, range-driven** functional composition. The blueprint and predecessor of `std::ranges`, with richer functionality. | **Imperative, procedural** programming. Direct manipulation of iterators and containers. |
| **Core Design Philosophy** | **Precise control** of data’s logical position and flow order within the pipeline via **indices**, achieving optimal resource utilisation. | Provides composable, lazily evaluated **view adapters** to build efficient generic algorithms. | Provides a complete, composable set of **range algorithms and views**, forming the cornerstone of modern C++ functional programming. | Full control of computation flow and state rests entirely with the developer. |
| **Parallel Support** | **Declarative parallelism**. `.parallel(n)` declares intent; terminal operations automatically trigger the thread pool. No manual management needed. | C++17/20 provides parallel algorithms (`std::for_each(std::execution::par, ...)`), but they are **not declarative** and must be combined with views. | Does not provide parallel algorithms directly, but can be combined with external libraries such as TBB or HPX. | Requires manual implementation (e.g. `std::thread`, `std::async`, or parallel algorithms); high complexity. |
| **Sorting & Indexing** | **`sorted()` has the highest priority** and overrides all prior index transformations. Provides fine-grained operations such as `redirect` and `reverse`; a core feature. | Provides sorting algorithms (`std::ranges::sort`), but they are **in-place and destructive**, breaking the view chain. No custom “index” concept. | Similar to `std::ranges`; sorting is destructive. No “index” abstraction. | Developer must implement sorting logic and manage data relationships before and after sorting. |
| **Window / Sliding Analysis** | **Native support**. `.toWindow()` together with `.slide()`, `.tumble()`, etc., directly builds sliding/rolling windows; first-class citizens for advanced analytics. | Not natively supported. Requires combining multiple views (e.g. `views::slide` in C++23 or `views::adjacent`) and manual handling; code is relatively complex. | Provides components such as `ranges::views::slide` (pre-C++20), but advanced window aggregation still requires manual composition. | Requires hand-written nested loops and state management; code is verbose and error-prone. |
| **Data Structures** | Clearly mapped to `std::map` (ordered), `std::unordered_map` (unordered), `std::vector`, etc. Terminal operations determine the final structure. | Algorithms operate on ranges without forcing a final container. Use `std::ranges::to` (C++23) or hand-written code to store results. | Similar to `std::ranges`; results are stored via `ranges::to<Container>`. | Entirely chosen and managed by the developer. |
| **Ease of Use & Expressiveness** | **High**. Fluent chainable style, API modelled on Java Stream; low learning curve. Focuses on “what” rather than “how”. | **Medium**. View composition is powerful, but syntax (pipe operator `\|`, projections `std::identity`) has a learning curve for beginners; compile errors can be complex. | **Medium-High**. Richest set of views and algorithms, but steepest learning curve. | **Low**. Complex logic results in large code volume, unclear intent, and easy introduction of bugs. |
| **Performance Characteristics** | Optimised by predefined data structures in index-control and window-computation scenarios. Declarative parallelism simplifies concurrent programming. | **Ultimate performance**. Lazy view composition and compile-time optimisations can produce code comparable to or better than hand-written loops (e.g. elimination of intermediate temporaries). | Same as `std::ranges`; performance is one of its core goals. | **Theoretical peak is high**. Experienced developers can achieve extreme micro-optimisation, but implementation and maintenance costs are very high. |
| **Typical Use Cases** | 1. **Time-series / event stream processing** (logs, sensor data).<br>2. Data transformations requiring **complex order control**.<br>3. **Declarative parallel computation**.<br>4. **Real-time sliding-window analysis**. | 1. **General, high-performance container data transformation and filtering**.<br>2. Building reusable **generic components**.<br>3. Tight integration with the existing STL algorithm and container ecosystem. | 1. Modern range-library functionality on projects unable to use C++20.<br>2. Research and experimentation with the most cutting-edge range proposals. | 1. Performance-critical scenarios with extremely simple logic.<br>2. Special low-level needs that no library can satisfy. |
| **Dependencies & Integration** | **Zero dependencies, single header**. Extremely simple integration. | Part of the C++20/23 standard library; no additional dependencies. | Requires integration as a third-party library; most feature-rich but adds project dependencies. | None. |

**Summary Recommendations:**
- **Choose Semantic-Cpp** if your project **strongly depends on fine-grained control of data order/indices**, requires **complex sliding-window analysis**, or you want **declarative parallelism with the smallest cognitive load**. Semantic-Cpp offers a highly abstracted, purpose-built solution.
- **Choose `std::ranges`** if your project already uses C++20/23 and your needs are **general, high-performance data transformation and querying**, you want seamless integration with the STL ecosystem, and you are comfortable with a moderate learning curve. `std::ranges` is the most standard and future-compatible choice.
- **Choose Range-v3** if compiler constraints prevent C++20 usage but you still need functionality similar to `std::ranges`.
- **Choose hand-written loops** only when logic is extremely simple, you require nanosecond-level performance, and library abstractions truly become a bottleneck.

---

## Licence and Support
- **Licence**: This project is released under the MIT licence.
- **Issues and Feedback**: If you encounter any bugs or have feature suggestions, please submit them on the https://github.com/eloyhere/semantic-cpp/issues page.
- **Discussion and Exchange**: You may also start a discussion at https://github.com/eloyhere/semantic-cpp/discussions.

**Semantic-Cpp** — Building efficient, clear data-processing pipelines with modern C++. 🚀