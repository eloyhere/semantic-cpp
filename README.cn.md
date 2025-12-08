# semantic-cpp —— 现代 C++ 语义流库（Semantic Stream）

**semantic-cpp** 是一个纯头文件、C++17 起、零外部依赖的高性能惰性流库。它融合了 JavaScript Generator、Java Stream、Kotlin Flow 和数据库索引访问的思想，首次在 C++ 中实现了：

- 真正的**无限流**支持  
- 每个元素自带**逻辑索引（Timestamp）**  
- **并行即默认**，顺序必须显式声明  
- **强制用户在编译期选择“要性能还是要顺序”**，杜绝隐式代价  

MIT 许可，商用开源完全免费。

## 核心设计哲学

```text
并行是默认的，顺序是需要付费的（最终一次归并排序）。
```

- 不写 `.ordered()` → 走最快无序路径（推荐 90% 场景）
- 写了 `.ordered()` → 结果严格按原始出现顺序（日志、协议、调试必备）

没有中间状态，没有“可能有序”的模糊地带，编译器强制你做出选择。

## 快速开始

```cpp
#include "semantic.h"
using namespace semantic;

int main() {
    // 1. 最快无序收集（推荐）
    auto fast = Semantic<int>::range(0, 1000000)
        .parallel(16)
        .map([](int x) { return x * x; })
        .toUnordered()          // 必须显式调用
        .toVector();

    // 2. 严格有序收集（必须显式声明）
    auto ordered = Semantic<int>::range(0, 1000000)
        .parallel(16)
        .redirect([](auto&, auto i) { return -i; })  // 彻底打乱索引
        .toOrdered()             // 必须显式调用
        .toVector();             // 结果仍然是 0,1,2,...,999999

    // 3. 有序去重（神器）
    auto unique = Semantic<int>::of(3,1,4,1,5,9,2,6,5)
        .parallel(8)
        .toOrdered()
        .toSet();                // {3,1,4,5,9,2,6} 按首次出现顺序

    return 0;
}
```

## 所有链式调用示例

```cpp
// 流创建方式
Semantic<int>::range(0, 100);                    // 有限范围
Semantic<long long>::range(0LL, 1LL<<60);        // 支持超大范围
Semantic<int>::iterate([](auto yield, auto) {    // 无限流
    for (int i = 0;; ++i) yield(i);
});
Semantic<int>::of(1, 2, 3, 4, 5);
Semantic<int>::from(std::vector{1,2,3,4});
Semantic<int>::fill(42, 1'000'000);              // 填充固定值
Semantic<int>::fill([] { return rand(); }, 1000);

// 中间操作（全部惰性）
stream.map([](int x) { return x * x; });
stream.filter([](int x) { return x % 2 == 0; });
stream.flatMap([](int x) { return Semantic<int>::range(0, x); });
stream.distinct();                               // 去重（基于值）
stream.skip(100);
stream.limit(50);
stream.takeWhile([](int x) { return x < 1000; });
stream.dropWhile([](int x) { return x < 100; });
stream.peek([](int x) { std::cout << x << ' '; });
stream.redirect([](auto&, auto i) { return -i; });     // 逆序
stream.redirect([](auto&, auto i) { return i % 10; }); // 循环
stream.parallel();                               // 使用全部核心
stream.parallel(8);                              // 指定线程数

// 必须显式选择收集方式
auto collector = stream.toOrdered();    // 我要顺序
auto fast = stream.toUnordered();       // 我要性能（默认路径）

// 有序收集（结果严格按原始顺序）
collector.toVector();
collector.toList();
collector.toSet();                      // 有序去重
collector.forEach([](int x) { std::cout << x << ' '; });
collector.cout();                       // 直接打印 [1, 2, 3, ...]

// 无序收集（最快路径）
fast.toVector();
fast.toList();
fast.toSet();                           // 无序去重
fast.forEach(...);
fast.cout();

// 统计（无序快速路径）
auto stats = stream.toUnordered().toStatistics();
std::cout << "均值 = " << stats.mean([](int x) { return x; });

// reduce 示例
int sum = stream.toUnordered()
    .reduce(0, [](int a, int b) { return a + b; });

std::optional<int> firstPrime = stream
    .filter(isPrime)
    .toOrdered()
    .findFirst();
```

## 关键方法一览表

| 方法                          | 说明                                      | 是否有序 |
|-------------------------------|-------------------------------------------|----------|
| `toOrdered()`                 | 进入有序收集模式（必须显式调用）          | 是       |
| `toUnordered()`               | 进入最快无序收集模式（默认）              | 否       |
| `toVector()` / `toList()`     | 收集为容器                                | 取决于前者 |
| `toSet()`                     | 收集为 set（去重）                        | 取决于前者 |
| `forEach(...)`                | 遍历每个元素                              | 取决于前者 |
| `cout()`                      | 直接打印 [1, 2, 3, ...]                   | 取决于前者 |
| `reduce(...)`                 | 归约                                      | 无序更快 |
| `collect(...)`                | 自定义收集器                              | 取决于前者 |
| `toStatistics()`             | 统计信息（均值、中位数、众数等）          | 无序更快 |

## 为什么必须显式选择 ordered / unordered？

因为并行后的顺序从来不是免费的。  
我们拒绝“默认有序但偷偷变慢”或“声称并行但其实乱序”的模糊设计。

你写什么，就得到什么。  
没有惊喜，只有性能和确定性。

## 编译要求

- C++17 或更高
- 仅需包含 `semantic.h`
- 零外部依赖
- 头文件单文件，直接 `#include "semantic.h"`

```cpp
// 推荐编译选项
g++ -std=c++17 -O3 -pthread semantic.cpp
```

## 许可证

MIT 许可证 —— 随意用于商用、开源、私有项目，无需保留版权声明。

---

**semantic-cpp**：  
**并行是默认的，顺序是选择的，索引是第一等公民。**

你写流，不再需要猜。  
你写流，只需要说清楚：**“我要快，还是我要顺序？”**
