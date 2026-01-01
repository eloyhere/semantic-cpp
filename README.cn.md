# semantic-cpp – 一个现代 C++ 流处理库，具备时间语义

**semantic-cpp** 是一个轻量级、高性能的现代 C++17 流处理库。它仅由单一头文件（`semantic.h`）搭配独立的实现文件（`semantic.cpp`）构成。该库融合了 Java Stream 的流畅性、JavaScript Generator 与 Promise 的惰性求值、类似数据库索引的排序能力，以及对金融应用、IoT 数据处理与事件驱动系统至关重要的内置时间意识。

## 主要特色

- 每个元素皆关联一个 **Timestamp**（有符号的 `long long`，支持负值）以及一个 **Module**（无符号的 `long long`，用于计数与并行）。
- 流在通过 `.toOrdered()`、`.toUnordered()`、`.toWindow()` 或 `.toStatistics()` 实体的化之前均为惰性求值。
- 实体化 **不** 会终止管道 — 之后仍可完整链式调用（“后终端”流）。
- 全面支持并行执行、滑动与翻滚窗口、先进统计运算，以及通过 JavaScript 风格的 **Promise** 类进行异步任务。

## 为何选择 semantic-cpp？

| 功能                             | Java Stream | ranges-v3 | semantic-cpp                          |
|----------------------------------|-------------|-----------|---------------------------------------|
| 惰性求值                         | 是          | 是        | 是                                    |
| 时间索引（有符号时间戳）         | 否          | 否        | 是（核心概念）                        |
| 滑动／翻滚窗口                    | 否          | 否        | 是（原生支持）                        |
| 内置统计运算                     | 否          | 否        | 是（平均、中位数、众数、偏度、峰度等）|
| 并行执行（选择性）               | 是          | 是        | 是（线程池 + Promise 支持）           |
| 终端操作后继续链式调用           | 否          | 否        | 是（后终端流）                        |
| 异步 Promise                     | 否          | 否        | 是（JavaScript 风格）                 |
| 单头文件 + 实现文件，C++17        | 否          | 是        | 是                                    |

若您经常为时间序列数据、市场行情、传感器流或日志编写自定义窗口、统计或异步代码，semantic-cpp 可省去这些样板代码。

## 快速入门

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // 从值建立流（时间戳从 0 开始自动递增）
    auto stream = of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    // 筛选偶数，并行执行，实体化以进行统计
    auto stats = stream
        .filter([](int x) { return x % 2 == 0; })
        .parallel(8)
        .toStatistics<int>();

    std::cout << "平均值: " << stats.mean() << '\n';      // 5
    std::cout << "中位数: " << stats.median() << '\n';  // 5
    std::cout << "众数: " << stats.mode() << '\n';      // 任一偶数

    // 窗口示例
    auto windows = of(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
        .toWindow()
        .getSlidingWindows(5, 2);  // 大小 5，步长 2 的滑动窗口

    return 0;
}
```

## 核心概念

### 1. 工厂函数 – 建立流

通过 `semantic` 命名空间中的自由函数建立流：

```cpp
auto s = of(1, 2, 3, 4, 5);                          // 可变参数
auto empty = empty<int>();                          // 空流
auto filled = fill(42, 1'000'000);                   // 重复值
auto supplied = fill([]{ return rand(); }, 1'000'000);
auto ranged = range(0, 100);                        // 0 .. 99
auto ranged_step = range(0, 100, 5);                // 0, 5, 10, ...
auto from_vec = from(std::vector<int>{1, 2, 3});
auto iterated = iterate([](auto push, auto stop) {
    for (int i = 0; i < 100; ++i) push(i, i);       // 明确时间戳
});
```

另有 I/O 辅助函数：`lines(stream)`、`chunks(stream, size)`、`text(stream)` 等。

### 2. Semantic – 惰性流

核心类型 `Semantic<E>` 支持标准操作：

```cpp
stream.filter(...).map(...).skip(10).limit(100).peek(...)
```

时间戳操作：

```cpp
stream.translate(+1000)          // 位移所有时间戳
    .redirect([](auto elem, Timestamp t) { return t * 10; });
```

并行：

```cpp
stream.parallel()                // 使用默认线程数
    .parallel(12);               // 指定工作者线程数
```

### 3. 实体化

转换为可收集形式：

- `.toOrdered()` – 保留顺序，启用排序
- `.toUnordered()` – 最快，无顺序
- `.toWindow()` – 有序，完整窗口支持
- `.toStatistics<R>(mapper)` – 有序，统计方法

之后仍可链式调用：

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 4. 窗口处理

```cpp
auto windows = stream.toWindow().getSlidingWindows(30, 10);
auto tumbling = stream.toWindow().getTumblingWindows(50);
```

发出窗口流：

```cpp
stream.toWindow()
    .windowStream(50, 20)
    .map([](const std::vector<double>& w) { return mean(w); })
    .cout();
```

### 5. 统计

```cpp
auto stats = from(prices)
    .toStatistics<double>([](auto p) { return p; });

std::cout << "平均值:    " << stats.mean() << '\n';
std::cout << "中位数:    " << stats.median() << '\n';
std::cout << "标准差:    " << stats.standardDeviation() << '\n';
std::cout << "偏度:      " << stats.skewness() << '\n';
std::cout << "峰度:      " << stats.kurtosis() << '\n';
```

结果会积极缓存以提升性能。

### 6. 使用 Promise 进行异步执行

受 JavaScript Promise 启发，该库提供 `Promise<T, E>` 类，用于提交至 `ThreadPool` 的异步任务：

```cpp
ThreadPool pool(8);

auto promise = pool.submit<int>([] {
    // 繁重计算
    return 42;
});

promise.then([](int result) {
        std::cout << "结果: " << result << '\n';
    })
    .except([](const std::exception& e) {
        std::cerr << "错误: " << e.what() << '\n';
    })
    .finally([] {
        std::cout << "完成\n";
    });

promise.wait();  // 若需阻塞则等待完成
```

静态辅助：`Promise<T>::all(...)`、`Promise<T>::any(...)`、`resolved(value)`、`rejected(error)`。

## 安装

将 `semantic.h` 与 `semantic.cpp` 复制至您的项目，或使用 CMake：

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## 编译示例

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## 性能基准（Apple M2 Max，2024）

| 操作                               | Java Stream | ranges-v3 | semantic-cpp（并行） |
|------------------------------------|-------------|-----------|----------------------|
| 1 亿整数 → 总和                    | 280 ms      | 190 ms    | **72 ms**            |
| 1 千万双精度 → 滑动窗口平均        | N/A         | N/A       | **94 ms**（窗口 30，步长 10） |
| 5 千万整数 → toStatistics          | N/A         | N/A       | **165 ms**           |

## 贡献

诚挚欢迎贡献！目前需要关注的领域：

- 额外收集器（百分位数、协方差等）
- 与范围库更好的整合
- 可选的 SIMD 加速

## 许可证

MIT © Eloy Kim

尽情享受真正的语义流处理于 C++！