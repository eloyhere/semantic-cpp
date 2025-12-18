# semantic-cpp – 一种具有时间语义的现代 C++ 流处理库

***
**受 Jonathan Wakely 指导**  
(GCC libstdc++ 维护者和 ISO C++ 委员会成员)
***

**semantic-cpp** 是一个轻量级、高性能的 C++17 流处理库，由单个头文件和对应的实现文件组成。它融合了 Java Stream 的流畅性、JavaScript Generator 的惰性、数据库索引的顺序映射，以及金融、物联网和事件驱动系统所需的时间感知能力。

使其独一无二的核心特性：

- 每个元素都携带一个 **Timestamp**（有符号 `long long` 索引，可为负值）。
- **Module** 是无符号 `long long`，用于计数和并发级别控制。
- 流在调用 `.toOrdered()`、`.toUnordered()`、`.toWindow()` 或 `.toStatistics()` 之前保持惰性。
- 物化后仍可继续链式调用 —— 库特意支持“后终端”流。
- 完整支持并行执行、滑动窗口与翻滚窗口，以及丰富的统计功能。

## 为什么选择 semantic-cpp？

| 特性                                 | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                   |
|--------------------------------------|-------------|-------------|-----------|--------------|------------------------------------------------|
| 惰性求值                             | 是          | 是          | 是        | 否           | 是                                             |
| 时间索引（有符号时间戳）             | 否          | 否          | 否        | 否           | 是（核心概念）                                 |
| 滑动/翻滚窗口                        | 否          | 否          | 否        | 否           | 是（一级支持）                                 |
| 内置统计功能                         | 否          | 否          | 否        | 否           | 是（均值、中位数、众数、峰度等）               |
| 支持并行（可选择）                   | 是          | 否          | 是        | 否           | 是（全局线程池或自定义）                       |
| 终端操作后继续链式调用               | 否          | 否          | 否        | 否           | 是（后终端流）                                 |
| 单个头文件+实现文件，C++17           | 否          | 是          | 是        | 是           | 是                                             |

如果你经常为时间序列、市场数据、传感器流或日志分析编写重复的窗口和统计代码，semantic-cpp 将彻底消除这些样板代码。

## 快速上手

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // 生成时间戳为 0..99 的 100 个整数流
    auto stream = Generative<int>{}.of(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream.filter([](int x) { return x % 2 == 0; })   // 只保留偶数
          .parallel(8)                               // 使用 8 个线程
          .toWindow()                                // 物化为支持窗口操作的有序集合
          .getSlidingWindows(10, 5)                  // 窗口大小 10，步长 5
          .toVector();

    // 统计示例
    auto stats = Generative<int>{}.of(1,2,3,4,5,6,7,8,9,10).toStatistics();
    std::cout << stats.mean()   << "\n";   // 5.5
    std::cout << stats.median() << "\n";   // 5.5
    std::cout << stats.mode()   << "\n";   // 多众数时返回首个
    stats.cout();

    return 0;
}
```

## 核心概念

### 1. Generative<E> —— 流工厂

`Generative<E>` 提供便捷的流创建接口，所有方法均返回惰性 `Semantic<E>`：

```cpp
Generative<int> gen;
auto s1 = gen.of(1, 2, 3, 4, 5);
auto s2 = gen.empty();
auto s3 = gen.fill(42, 1'000'000);
auto s4 = gen.range(0, 100, 5);
auto s5 = gen.from(std::vector<int>{10, 20, 30});
```

### 2. Semantic<E> —— 惰性流

```cpp
Semantic<int> s = of<int>(1, 2, 3, 4, 5);
```

支持经典操作：

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

元素自带时间戳，可进行平移或重定向：

```cpp
s.translate(+1000)                                      // 平移时间戳
 .redirect([](int x, Timestamp t){ return x * 10; })   // 自定义时间戳
```

### 3. 物化（唯一产生开销的地方）

在使用收集操作（count()、toVector()、cout() 等）前，必须调用以下四个终端转换器之一：

```cpp
.toOrdered()      // 保留顺序，支持排序
.toUnordered()    // 最快，不保证顺序
.toWindow()       // 有序 + 完整窗口功能
.toStatistics<D>  // 有序 + 统计功能（均值、方差、偏度等）
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

### 4. 窗口操作

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

### 5. 统计功能

```cpp
auto stats = from(prices).toStatistics<double>([](auto p){ return p; });

std::cout << "均值      : " << stats.mean()      << "\n";
std::cout << "中位数    : " << stats.median()    << "\n";
std::cout << "标准差    : " << stats.standardDeviation() << "\n";
std::cout << "偏度      : " << stats.skewness()  << "\n";
std::cout << "峰度      : " << stats.kurtosis()  << "\n";
```

所有统计函数均采用深度缓存（频率映射仅计算一次）。

### 6. 并行支持

```cpp
globalThreadPool   // 自动使用硬件并发线程数创建
stream.parallel()  // 使用全局线程池
stream.parallel(12) // 强制使用 12 个工作线程
```

并发级别在整个链式调用中正确继承。

## 工厂函数（通过 Generative）

```cpp
empty<T>()                              // 空流
of(1,2,3,"hello")                       // 可变参数创建
fill(42, 1'000'000)                     // 重复值
fill([]{return rand();}, 1'000'000)     // 使用供应器生成
from(container)                         // 支持 vector、list、set、array、initializer_list、queue
range(0, 100)                           // 0 .. 99
range(0, 100, 5)                        // 0,5,10,…
iterate(generator)                      // 自定义 Generator
```

## 安装

semantic-cpp 由单个头文件和实现文件组成。只需将 `semantic.h` 和 `semantic.cpp` 复制到项目中，或通过 CMake 安装：

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

非常欢迎贡献！当前重点关注的领域：

- 更多收集器（百分位数、协方差等）
- 与现有范围库的更好集成
- 简单映射器的可选 SIMD 加速

请阅读 CONTRIBUTING.md。

## 许可

MIT © Eloy Kim

尽情享受真正的语义流处理吧！
