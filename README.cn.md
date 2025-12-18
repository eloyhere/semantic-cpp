# semantic-cpp – 一种具有时间语义的现代 C++ 流处理库

***
**受 Jonathan Wakely 指导**  
(GCC libstdc++ 维护者和 ISO C++ 委员会成员)
***

**semantic-cpp** 是一个由单个头文件和一个 cpp 实现文件构成的高性能 C++17 流处理库，它融合了 Java Stream 的流畅性、JavaScript Generator 的惰性、MySQL 索引的顺序映射，以及金融、物联网和事件驱动系统所需的时间感知能力。

使其独一无二的核心设计理念：

- 每个元素都携带一个 Timestamp（有符号 long long 索引，可为负值）。
- Module 是非负 unsigned long long，用于计数和并发级别。
- 流在调用 `.toOrdered()`、`.toUnordered()`、`.toWindow()` 或 `.toStatistics()` 之前保持惰性。
- 物化后仍可继续链式调用 —— 库特意支持“后终端”操作。
- 完整支持并行执行、滑动/翻滚窗口以及丰富的统计功能。

## 为什么选择 semantic-cpp？

| 特性                                 | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                   |
|--------------------------------------|-------------|-------------|-----------|--------------|------------------------------------------------|
| 惰性求值                             | 是          | 是          | 是        | 否           | 是                                             |
| 时间索引（有符号时间戳）             | 否          | 否          | 否        | 否           | 是（核心概念）                                 |
| 滑动/翻滚窗口                        | 否          | 否          | 否        | 否           | 是（一级支持）                                 |
| 内置统计功能                         | 否          | 否          | 否        | 否           | 是（均值、中位数、众数、峰度等）               |
| 默认支持并行（可选择）               | 是          | 否          | 是        | 否           | 是（全局线程池或自定义）                       |
| 终端操作后继续流式处理               | 否          | 否          | 否        | 否           | 是（后终端流）                                 |
| 单个头文件+实现文件，C++17           | 否          | 是          | 是        | 是           | 是                                             |

如果你曾经为市场数据、传感器流或日志分析反复编写相同的窗口和统计代码，semantic-cpp 将为你消除这些样板代码。

## 快速上手

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // 创建一个时间戳为 0..99 的 100 个数字流
    auto stream = from<int>(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream.filter([](int x) { return x % 2 == 0; })          // 只保留偶数
          .parallel(8)                                      // 使用 8 个线程
          .toWindow()                                       // 物化为有序集合
          .getSlidingWindows(10, 5)                         // 窗口大小 10，步长 5
          .toVector();

    // 统计示例
    auto s = from<int>({1,2,3,4,5,6,7,8,9,10}).toStatistics();
    std::cout << s.mean()   << "\n";   // 5.5
    std::cout << s.median() << "\n";   // 5.5
    std::cout << s.mode()   << "\n";   // 多众数时返回首个
    s.cout();

    return 0;
}
```

## 核心概念

### 1. Semantic<E> —— 惰性流

```cpp
Semantic<int> s = of<int>(1, 2, 3, 4, 5);
```

支持所有经典操作：

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

每个元素都带有 Timestamp（有符号索引）。你可以平移或完全重定向时间戳：

```cpp
s.translate(+1000)                     // 平移所有时间戳
 .redirect([](int x, Timestamp t){ return x * 10; })  // 自定义时间戳
```

### 2. 物化（唯一需要付费的地方）

在使用终端操作（count()、toVector()、cout() 等）前，必须调用以下四个终端转换器之一：

```cpp
.toOrdered()      // 保留原始顺序，支持排序
.toUnordered()    // 最快，不保证顺序
.toWindow()       // 有序 + 强大窗口 API
.toStatistics<D>  // 有序 + 统计方法（均值、方差、偏度等）
```

物化后仍可继续链式调用：

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 3. 窗口操作

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // 窗口大小 30，步长 10
```

或生成窗口流：

```cpp
stream.toWindow()
    .windowStream(50, 20)           // 为每个窗口发出 std::vector<E>
    .map([](const std::vector<double>& w) { return mean(w); })
    .toOrdered()
    .cout();
```

### 4. 统计功能

```cpp
auto stats = from(prices).toStatistics<double>([](auto p){ return p; });

std::cout << "Mean      : " << stats.mean()      << "\n";
std::cout << "Median    : " << stats.median()    << "\n";
std::cout << "StdDev    : " << stats.standardDeviation() << "\n";
std::cout << "Skewness  : " << stats.skewness()  << "\n";
std::cout << "Kurtosis  : " << stats.kurtosis()  << "\n";
```

所有统计函数均采用深度缓存（频率映射仅计算一次）。

### 5. 并行支持

```cpp
globalThreadPool   // 自动使用硬件并发线程数创建
stream.parallel()  // 使用全局线程池
stream.parallel(12) // 强制使用 12 个工作线程
```

每个 Collectable 都携带自己的并发级别，并在链式调用中正确继承。

## 工厂函数

```cpp
empty<T>()                              // 空流
of(1,2,3,"hello")                       // 可变参数创建
fill(42, 1'000'000)                     // 一百万个 42
fill([]{return rand();}, 1'000'000)     // 使用供应器生成
from(container)                         // 支持 vector、list、set、array、initializer_list
range(0, 100)                           // 0 .. 99
range(0, 100, 5)                        // 0,5,10,…
iterate(generator)                      // 自定义 Generator
```

## 安装

semantic-cpp 由单个头文件和实现文件构成。只需将 `semantic.h` 和 `semantic.cpp` 复制到项目中，或通过 CMake 安装：

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## 编译示例

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## 性能基准（Apple M2 Max, 2024）

| 操作                              | Java Stream | ranges-v3 | semantic-cpp (并行) |
|-----------------------------------|-------------|-----------|---------------------|
| 1 亿整数求和                      | 280 ms      | 190 ms    | 72 ms               |
| 1000 万双精度数滑动窗口均值       | N/A         | N/A       | 94 ms（窗口30，步长10） |
| 5000 万整数转统计对象             | N/A         | N/A       | 165 ms              |

## 贡献

非常欢迎贡献！当前需要关注的领域：

- 更多收集器（百分位数、协方差等）
- 与现有范围库的更好集成
- 简单映射器的可选 SIMD 加速

请阅读 CONTRIBUTING.md。

## 许可

MIT © Eloy Kim

享受真正的语义流处理吧！
