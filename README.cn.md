# Semantic-Cpp 详解：面向未来的C++智能流处理框架

Semantic-Cpp 是一个完全重构设计的现代化C++流处理库，采用“单头文件、零依赖”架构，其核心文件 `semantic.h` 集成了整个库的功能。本库创造性地融合了多种编程范式的精华：

-   **Java Stream API 的优雅与流畅**：提供链式调用和声明式编程体验。
-   **JavaScript Generator 的惰性与灵活**：支持延迟计算和按需生成数据。
-   **数据库索引的高效与有序**：内置智能排序和索引驱动机制，特别适合处理时序和事件数据。

与传统的数据处理方式（如手写循环或复杂的异步回调）不同，Semantic-Cpp 旨在提供一种**类型安全、表达力强且高性能**的解决方案。其最核心的设计哲学是**精准的数据流控制**：数据仅在明确需要时才会在“处理管道”中流动，并且流动的“顺序”和“位置”可以通过“索引”进行精细调控，从而实现资源的最优利用。

---

## 核心灵魂：索引驱动的数据世界

Semantic-Cpp 将数据处理抽象为对“元素”及其“逻辑位置（索引）”的操作。理解这一点是掌握本库的关键。

### 1. 基础索引变换
索引决定了元素在处理链中的逻辑顺序，您可以对其进行灵活操作：
-   **`redirect(重定向函数)`**：核心方法。您可以通过一个自定义函数，完全重写元素的索引。例如，可以将索引翻倍、基于元素值生成新索引等。
-   **`reverse()`**：一个便捷方法，内部通过 `redirect` 实现，将当前所有索引逻辑反转（例如，正索引变负）。
-   **`translate(偏移量)`**：为所有索引增加一个固定的偏移量。

### 2. 排序的“霸道”规则
排序操作 (`sorted`) 在库中拥有最高优先级，其行为是确定性的：
-   **`sorted()` 会覆盖一切**：无论您之前通过 `redirect`、`reverse` 进行了多么复杂的索引变换，一旦调用 `sorted()`，所有之前的索引操作都将被**覆盖**。系统会根据元素的**实际值**，为其重新分配从0开始的自然顺序索引。
-   **立即“物化”为有序集合**：为了避免后续操作中不必要的重复排序开销，`sorted()` 方法会**立即**返回一个 `OrderedCollectable` 类型的对象。这意味着数据在此刻已经被收集并排序完成。

### 3. 声明式并行处理
并行处理变得非常简单和直观：
-   **`parallel(线程数)` 只是声明**：调用此方法仅是表达了“我希望后续操作能并行执行”的意图，并指定了期望的线程数，**并不会立即启动任何线程或提交任务**。
-   **终端操作触发并行**：真正的并行计算是在调用如 `toUnordered()`、`toOrdered()`、`count()` 等**终端操作方法**时才被触发。此时，库的内置线程池会根据声明的线程数，自动将数据分片并提交任务。
-   **无需手动管理**：您无需关心线程创建、任务分发和结果合并的细节，库会为您自动处理。

### 4. 如何选择最终的数据容器？
根据您的性能需求和操作类型，可以选择不同的终端转换方法：

| 转换方法 | 底层数据结构 | 性能特征 | 最佳适用场景 |
| :--- | :--- | :--- | :--- |
| **`sorted()`** | `std::map<索引, 元素>` | O(log n) 访问，严格保持元素顺序。 | 需要分页、按范围查询、时间序列分析、滚动统计。 |
| **`sorted(comparator)`** | `std::map<索引, 元素>` | O(log n) 访问，按自定义规则排序。 | 自定义排序规则的分页或范围查询。 |
| **`toOrdered()`** | `std::map<索引, 元素>` | O(log n) 访问，保持**当前索引**的顺序。 | 当您想保留 `redirect` 等操作定义的索引顺序，并进行有序操作时。 |
| **`toUnordered()`** | `std::unordered_map<索引, 元素>` | 平均 O(1) 访问，**最高性能**，但不保证顺序。 | 快速查找、去重统计、聚合计算等不关心顺序的场景。 |
| **`toWindow()`** | 基于 `std::map` 的窗口视图 | O(log n)，支持在有序数据集上定义滑动或滚动窗口。 | 实时流数据分析、滑动窗口聚合、事件会话划分。 |

> **重要提示**：`WindowCollectable`（由 `toWindow()` 返回）内部依赖于一个有序的集合（通过 `toOrdered()` 实现），以确保窗口滑动和翻滚操作能基于确定的顺序正确执行。

---

## 快速上手指南

### 安装
只需将 `semantic.h` 头文件放入您的项目，并确保编译器支持 C++17 或更高标准。
```cpp
#include "semantic.h"
// 可选：使用语义命名空间
using namespace semantic;
```

### 基础示例：体验索引与排序
```cpp
#include <iostream>
#include "semantic.h"

int main() {
    auto result = semantic::useRange(0, 10)   // 1. 创建 0 到 9 的整数流
        .map(int x { return x * x; })    // 2. 将每个元素平方 (0,1,4,9...81)
        .redirect(int val, auto idx {    // 3. 索引重定向：将索引翻倍
            return idx * 2;                  // 现在索引是 0,2,4,6...
        })
        .reverse()                           // 4. 逻辑反转索引 (...,6,4,2,0)
        .sorted()                            // 5. ⚠️ 强制按元素值(1,4,9...)重新排序！
                                             //    之前的所有索引操作被覆盖，索引变为0,1,2...
        .toList();                           // 6. 收集到 std::vector

    // 输出: 0 1 4 9 16 25 36 49 64 81 （已排序）
    for (auto& item : result) {
        std::cout << item << " ";
    }
    return 0;
}
```

### 并行处理示例
```cpp
#include <iostream>
#include "semantic.h"

int main() {
    // 1. 构建一个流处理管道，并声明希望使用4个线程并行执行。
    auto dataStream = semantic::useRange(1, 1000)
        .parallel(4)                         // 声明并行，尚未执行
        .filter(int x -> bool {
            return x % 2 == 0;               // 过滤出偶数
        })
        .filter(int x, auto index -> bool {
            return index < 5LL;              // 再过滤出逻辑索引小于5的元素
        });

    // 2. 终端操作 `count()` 触发真正的并行计算
    //    线程池启动，数据被分片，四个线程并发计数，结果自动合并。
    auto result = dataStream
        .toUnordered()                       // 转换为无序集合并行处理
        .count();                            // 统计最终元素数量

    std::cout << "过滤后元素的数量: " << result << std::endl;
    return 0;
}
```

### 时间序列与窗口分析示例
```cpp
#include <iostream>
#include "semantic.h"

int main() {
    // 模拟一组时间序列数据（例如股价）
    auto timeSeries = semantic::useFrom(std::vector<double>{1.1, 2.2, 3.3, 4.4, 5.5});

    // 1. 转换为窗口视图
    auto windowStats = timeSeries
        .toWindow()                     // 转换为 WindowCollectable
        .slide(3, 1)                    // 定义大小为3的滑动窗口，步长为1
                                        // 窗口1: {1.1, 2.2, 3.3}
                                        // 窗口2: {2.2, 3.3, 4.4}
                                        // 窗口3: {3.3, 4.4, 5.5}
        .sub(1, 4)                      // 取索引1到3的窗口（即窗口2和窗口3）
        .map(auto&& window -> double { // 对每个窗口进行处理
            // 计算每个窗口的平均值
            return window
                .toStatistics<double, double>() // 将窗口转为 Statistics 以进行数学计算
                .average();
        })
        .toStatistics<double, double>() // 对计算结果（平均值序列）进行有序统计
        .summate();                     // 对所有窗口平均值求和

    std::cout << "选定滑动窗口的平均值总和: " << windowStats << std::endl;
    // 输出: ( (2.2+3.3+4.4)/3 + (3.3+4.4+5.5)/3 ) 的结果
    return 0;
}
```

---

## 核心API速查手册

### 流构建器 (Stream Sources)
| 方法 | 描述 | 示例 |
| :--- | :--- | :--- |
| `useRange(start, end)` | 生成一个数值范围内的整数流。 | `useRange(0, 10)` |
| `useFrom(container)` | 从标准容器（如 vector, list）创建流。 | `useFrom(std::vector{1,2,3})` |
| `useOf(args...)` | 从可变参数列表创建流。 | `useOf(1, 2, 3, 4, 5)` |
| `useText(text, encoding)` | 将字符串按字符或编码点拆分为流。 | `useText("Hello", charset::UTF8)` |

### 中间操作 (Intermediate Operations)
| 方法 | 描述 | 注意事项 |
| :--- | :--- | :--- |
| `map(转换函数)` | 将元素转换为另一种形式。 | 函数可接收`(元素)`或`(元素, 索引)`。 |
| `filter(断言函数)` | 过滤满足条件的元素。 | 断言可基于`(元素)`或`(元素, 索引)`。 |
| `distinct()` | 去除重复的元素。 | 可传入自定义`比较器`。 |
| `limit(n)` | 限制流中元素的数量为前`n`个。 | |
| `skip(n)` | 跳过流中的前`n`个元素。 | |
| `sub(start, end)` | 获取索引在`[start, end)`范围内的子流。 | 类似字符串的`substr`。 |

### 索引操作 (Index Operations)
| 方法 | 描述 | 关键特性 |
| :--- | :--- | :--- |
| `redirect(重定向函数)` | 核心方法，允许完全控制每个元素的索引。 | 函数签名为`(元素, 旧索引) -> 新索引`。 |
| `reverse()` | 将当前所有元素的索引逻辑反转。 | 内部通过`redirect`实现。 |
| `translate(offset)` | 将所有元素的索引增加一个固定偏移量。 | |
| **`sorted()`** | **强制排序**。按元素值升序排列，**覆盖所有已有索引**。 | 立即返回`OrderedCollectable`。 |
| **`sorted(比较器)`** | 使用自定义比较器强制排序。 | 立即返回`OrderedCollectable`。 |

### 并行声明 (Parallel Declaration)
| 方法 | 描述 | 执行时机 |
| :--- | :--- | :--- |
| `parallel()` | 声明使用默认并行策略（通常为CPU核心数）。 | 在后续的**终端操作**中触发。 |
| `parallel(n)` | 声明希望使用`n`个线程进行并行处理。 | 在后续的**终端操作**中触发。 |

### 终端转换 (Terminal Conversions - 触发计算)
| 方法 | 描述 | 内部状态 |
| :--- | :--- | :--- |
| `toOrdered()` | 转换为有序集合，保留**当前的索引顺序**。 | 物化为`std::map<Index, Value>`。 |
| `toUnordered()` | 转换为无序集合，以获得最高性能。 | 物化为`std::unordered_map<Index, Value>`。 |
| `toWindow()` | 转换为窗口集合，用于滑动/滚动分析。 | 内部基于`toOrdered()`。 |

### 终端操作 (Terminal Actions - 产生最终结果)
| 方法 | 描述 | 返回类型 |
| :--- | :--- | :--- |
| `count()` | 计算流中元素的总数。 | `Module` (`unsigned long long`) |
| `average()` | 计算数值元素的平均值。 | 元素类型的平均值（如`double`）。 |
| `min()` / `max()` | 查找流中的最小/最大值。 | `std::optional<元素类型>` |
| `reduce(初始值, 累加器)` | 将流归约为单个值（如求和）。 | 累加器结果的类型。 |
| `collect(收集器)` | 使用自定义收集器进行复杂聚合。 | 收集器定义的返回类型。 |
| `toList()` / `toVector()` | 收集所有元素到列表/向量。 | `std::vector<元素类型>` |
| `toSet()` | 收集所有元素到集合（去重）。 | `std::set<元素类型>` |

---

## 高级主题与最佳实践

### 架构精髓：惰性求值与精准回调控
每个流操作背后都是一个接受两个回调函数的“生成器”：
-   **`accept(元素, 索引)`**：当下游操作准备好处理数据时，会调用此回调来“请求”一个元素。
-   **`interrupt(元素, 索引)`**：在处理每个元素前调用，如果返回`true`，则整个处理链会**立即终止**。
这种机制确保了数据是“按需拉取”的，并且可以随时提前结束，避免了不必要的计算。

### 性能优化建议
1.  **选择对的容器**：
    -   做等值查找、去重、不排序的聚合 → 优先用 `toUnordered()`。
    -   需要范围查询、排序、分页 → 用 `toOrdered()` 或 `sorted()`。
    -   做实时窗口分析 → 用 `toWindow()`。
2.  **善用并行**：
    -   数据量较大（例如 >1000 条）或处理逻辑（`map`、`filter`）较耗时时，使用 `parallel()` 通常能获得收益。
    -   避免在并行流中进行阻塞式的I/O操作。
3.  **优化操作顺序**：
    -   **尽早过滤(`filter`)**：在应用昂贵的`map`转换之前，先通过`filter`减少数据量。
    -   **明智排序**：排序开销大。如果后续操作（如`distinct`）不依赖顺序，可先进行这些操作再排序。

### 自定义收集器
当内置的终端操作不满足需求时，您可以构建自定义收集器，实现复杂的归约逻辑。
```cpp
// 创建一个将数字连接成特定格式字符串的收集器
auto myCollector = semantic::collector::useFull<int, std::string, std::string>(
     -> std::string { return ""; }, // 供应器：提供初始累加值
    std::string acc, int val, auto idx -> std::string { // 累加器
        if (!acc.empty()) acc += "|";
        return acc + "Num(" + std::to_string(val) + ")";
    },
    std::string a, std::string b -> std::string { // 合并器（用于并行）
        if (a.empty()) return b;
        if (b.empty()) return a;
        return a + "|" + b;
    },
    std::string acc -> std::string { // 完成器：对最终结果做最后处理
        return "[" + acc + "]";
    }
);

auto result = semantic::useRange(1, 5)
    .toOrdered() // 触发计算
    .collect(myCollector); // 使用自定义收集器

std::cout << result << std::endl; // 输出: [Num(1)|Num(2)|Num(3)|Num(4)]
```

### 文本处理示例
```cpp
auto text = semantic::useText("Hello 世界！")
    .map([](const std::string& text) -> std::string {
        return "<" + text + ">";
    })
    .toOrdered()
    .join(" "); // 用空格连接所有字符

std::cout << text << std::endl;
// 输出: <H><e><l><l><o>< ><世><界><！>
```

---

## 许可证与支持

-   **许可证**：本项目基于 MIT 开源。
-   **问题与反馈**：如果您遇到任何错误或有新功能建议，欢迎在 https://github.com/eloyhere/semantic-cpp/issues 页面提交。
-   **讨论与交流**：您也可以在 https://github.com/eloyhere/semantic-cpp/discussions 发起讨论。

**Semantic-Cpp** —— 用现代C++构建高效、清晰的数据处理管道。 🚀
