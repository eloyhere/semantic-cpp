# semantic-cpp —— 现代 C++ 时序流处理库（支持时间戳语义）

**semantic-cpp** 是一个纯头文件、高性能的 C++17 流处理库，融合了 Java Stream 的流畅写法、JavaScript Generator 的惰性求值、MySQL 索引的有序映射能力，以及金融、物联网、事件驱动系统所必需的时序感知能力。

核心特性（真正与众不同之处）：

- 每个元素都天然携带一个 Timestamp（有符号 long long，可为负数的时间戳/索引）
- Module 为无符号整数，用于计数和并发度控制
- 流在调用 .toOrdered() / .toUnordered() / .toWindow() / .toStatistics() 之前完全惰性
- 终结操作后仍可继续链式调用 —— 刻意设计的“后终结流”（post-terminal）友好
- 原生支持并行执行、滑动/翻滚窗口、丰富的统计收集器

## 为什么选择 semantic-cpp？

| 特性                                 | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                 |
|--------------------------------------|-------------|-------------|-----------|--------------|----------------------------------------------|
| 惰性求值                             | Yes         | Yes         | Yes       | No           | Yes                                          |
| 时序索引（有符号时间戳）             | No          | No          | No        | No           | Yes（核心概念）                              |
| 滑动窗 / 翻滚窗（Sliding/Tumbling）  | No          | No          | No        | No           | Yes（原生支持）                              |
| 内置统计收集器                       | No          | No          | No        | No           | Yes（均值、中位数、众数、峰度、偏度…）      |
| 默认并行（可开关）                   | Yes         | No          | Yes       | No           | Yes（全局线程池或自定义）                    |
| 终结操作后仍可继续流式处理           | No          | No          | No        | No           | Yes（后终结流）                              |
| 纯头文件、C++17                      | No          | Yes         | Yes       | Yes          | Yes                                          |

如果你曾经为行情数据、传感器流、日志分析反复写窗口和统计代码，semantic-cpp 能帮你彻底摆脱这些样板代码。

## 快速上手

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // 创建一个时间戳 0~99 的 100 个整数流
    auto stream = from(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream
        .filter([](int x) { return x % 2 == 0; })     // 只保留偶数
        .parallel(8)                                  // 使用 8 个线程
        .toWindow()                                   // 物化成有序集合，开启窗口能力
        .getSlidingWindows(10, 5)                     // 窗口大小 10，步长 5
        .toVector();                                  // 转为 vector<vector<int>>

    // 统计示例
    auto stats = from({1,2,3,4,5,6,7,8,9,10})
                  .toStatistics<double>();        // 自动推导映射器为 identity

    std::cout << "均值   : " << stats.mean()            << '\n';
    std::cout << "中位数 : " << stats.median()          << '\n';
    std::cout << "众数   : " << stats.mode()            << '\n';
    std::cout << "标准差 : " << stats.standardDeviation() << '\n';
    stats.cout();  // 一键漂亮打印

    return 0;
}
```

## 核心概念

### 1. Semantic<E> —— 惰性流

```cpp
Semantic<int> s = of(1, 2, 3, 4, 5);
```

支持所有经典操作：

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

每个元素都会附带一个 Timestamp（有符号索引），你可以平移或完全重定向时间戳：

```cpp
s.translate(+1000)                                   // 整体平移时间戳
   .redirect([](int x, Timestamp t){ return x * 10; }) // 自定义时间戳
```

### 2. 物化（Materialisation）—— 唯一需要“付费”的地方

必须调用以下四个终结转换器之一，才能使用 count()、toVector()、cout() 等收集操作：

```cpp
.toOrdered()      // 保持原始顺序，支持排序
.toUnordered()    // 最快，无序
.toWindow()       // 有序 + 强大窗口 API
.toStatistics<D>()// 有序 + 统计方法（均值、方差、偏度、峰度…）
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

也可以直接生成窗口流：

```cpp
stream.toWindow()
      .windowStream(50, 20)           // 每个元素是一个 std::vector<E>
      .map([](const std::vector<double>& w) { return mean(w); })
      .toOrdered()
      .cout();
```

### 4. 统计功能

```cpp
auto stats = from(prices)
              .toStatistics<double>([](double p){ return p; });

std::cout << "均值     : " << stats.mean()      << "\n";
std::cout << "中位数   : " << stats.median()    << "\n";
std::cout << "标准差   : " << stats.standardDeviation() << "\n";
std::cout << "偏度     : " << stats.skewness()  << "\n";
std::cout << "峰度     : " << stats.kurtosis()  << "\n";
```

所有统计函数都做了深度缓存（频次表只计算一次）。

### 5. 并行

```cpp
globalThreadPool          // 程序启动时自动创建，线程数 = hardware_concurrency
stream.parallel()         // 使用全局线程池
stream.parallel(12)       // 强制使用 12 个工作线程
```

每个 Collectable 都携带自己的并发度，整个链路自动继承。

## 工厂函数

```cpp
empty<T>()                              // 空流
of(1,2,3,"hello")                       // 可变参数构造
fill(42, 1'000'000)                     // 一百万个 42
fill([]{return rand();}, 1'000'000)     // 动态生成
from(container)                         // 支持 vector/list/set/array/initializer_list
range(0, 100)                           // 0..99
range(0, 100, 5)                        // 0,5,10,…
iterate(custom_generator)               // 自定义 Generator
```

## 安装

纯头文件，直接拷贝 semantic.h 到项目即可，也支持 CMake：

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## 编译示例

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp -o basic
./basic
```

## 性能基准（Apple M2 Max，2024 年数据）

| 操作                               | Java Stream | ranges-v3 | semantic-cpp（并行） |
|------------------------------------|-------------|-----------|----------------------|
| 1 亿整数求和                       | 280 ms      | 190 ms    | 72 ms                |
| 1000 万 double 滑动窗口均值（30步10）| N/A         | N/A       | 94 ms                |
| 5000 万整数 toStatistics           | N/A         | N/A       | 165 ms               |

## 贡献代码

非常欢迎贡献！目前特别需要的方向：

- 更多收集器（百分位数、协方差等）
- 与现有 range 库更好的互操作
- 对简单 mapper 的可选 SIMD 加速

请阅读 CONTRIBUTING.md。

## 许可证

MIT © 2025 Eloy Kim

祝你用 semantic-cpp 玩转真正的“有时间语义”的 C++ 流！
