# Semantic Stream —— 现代 C++ 函数式流库

Semantic 是一个纯头文件、高性能、完全惰性求值的现代 C++17+ 函数式流库，设计灵感同时来源于 JavaScript Generator、Java Stream API、Java Function 包以及 MySQL 索引表的思想。

它将函数式编程的优雅表达能力与真正的惰性求值、并行执行以及基于时间戳的元素索引完美结合，可轻松处理有限序列与无限序列。

## 主要特性

- **完全惰性求值** —— 所有中间操作仅在触发终结操作时才执行。
- **原生支持无限流** —— 通过 Generator 可自然构造无限序列。
- **时间戳索引机制** —— 每个元素都带有隐式或显式的时间戳，支持类 MySQL 索引的高效跳跃与定位。
- **一键并行** —— 只需调用 `.parallel()` 或 `.parallel(n)` 即可开启多线程并行流水线。
- **丰富的函数式操作** —— `map`、`filter`、`flatMap`、`reduce`、`collect`、`group`、统计等一应俱全。
- **完整的 Java 风格 Collector** —— 熟悉的 supplier、accumulator、combiner、finisher 四阶段收集器。
- **强大的统计功能** —— 均值、中位数、众数、方差、标准差、分位数、偏度、峰度等全覆盖。
- **多种构造方式** —— 支持从容器、数组、范围、生成器、填充等方式快速创建。

## 设计理念

Semantic 把流视为一个带时间戳的、由 Generator 产生的元素序列，核心灵感来源：

- **JavaScript Generator** —— 拉取式的惰性值产生机制。
- **Java Stream** —— 流畅的链式调用与中间/终结操作分离。
- **Java Function 包** —— `Function`、`Consumer`、`Predicate` 等函数式接口。
- **MySQL 索引表** —— 通过逻辑时间戳实现高效的 `skip`、`limit`、`redirect`、`translate` 等操作。

## 核心概念

```cpp
using Generator<E> = BiConsumer<
    BiConsumer<E, Timestamp>,   // yield(元素, 时间戳)
    Predicate<E>                // 取消判断
>;
```

生成器接收一个 yield 函数和一个取消检查函数，这一底层抽象驱动所有流来源，包括无限流。

## 快速示例

```cpp
using namespace semantic;

// 无限随机整数流
auto s1 = Semantic<int>::iterate([](auto yield, auto cancel) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> d(1, 100);
    Timestamp ts = 0;
    while (!cancel(d(gen))) {
        yield(d(gen), ts++);
    }
});

// 惰性筛法生成素数（有限范围）
auto primes = Semantic<long long>::range(2LL, 1'000'000LL)
    .filter([](long long n) {
        return Semantic<long long>::range(2LL, n)
            .takeWhile([n](long long d){ return d * d <= n; })
            .noneMatch([n](long long d){ return n % d == 0; });
    });

// 并行单词计数
auto wordCount = Semantic<std::string>::from(lines)
    .flatMap([](const std::string& line) {
        return Semantic<std::string>::from(split(line));
    })
    .parallel()
    .group([](const std::string& w){ return w; })
    .map([](auto& p){ return std::make_pair(p.first, p.second.size()); });
```

## 编译要求

- C++17 或更高版本
- 纯头文件 —— 仅需 `#include "semantic.hpp"`
- 无任何外部依赖

库内置全局线程池 `semantic::globalThreadPool`，启动时自动使用 `std::thread::hardware_concurrency()` 个线程。

## 许可证

MIT License —— 可自由用于商业与开源项目。

## 作者

融合当代函数式编程精华，为现代 C++ 量身打造的高性能实现。
