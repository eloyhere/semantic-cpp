# semantic-cpp —— 现代 C++ 语义流库（Semantic Stream）

**semantic-cpp** 是纯头文件、C++17 起、零外部依赖的高性能惰性流库。  
它的核心思想是：**每一个元素都天然携带一个逻辑索引（Timestamp）**，所有对顺序的操作本质上都是对这个索引的重映射。

- 支持真正的无限流  
- 索引是第一等公民，可被任意重定向、打乱、循环、采样  
- 并行是默认行为  
- 是否尊重索引（即是否保持“语义顺序”）必须在收集时显式声明  

MIT 许可，可随意用于商用与开源项目。

## 核心设计哲学

> **索引决定顺序，执行不决定顺序。**

- `redirect`、`reverse`、`shuffle`、`cycle` 等操作会改变元素的逻辑索引  
- 只有调用 `.toOrdered()` 收集时，这些索引操作才会生效（结果严格按照最终索引排序）  
- 调用 `.toUnordered()` 收集时，所有索引操作都会被忽略（追求极致性能，无序）

```text
.toOrdered()  → 尊重所有索引操作（redirect 等有效）
.toUnordered() → 忽略所有索引操作（redirect 等无效，最快）
```

## 快速开始

```cpp
#include "semantic.h"
using namespace semantic;

int main() {
    // 1. 索引操作全部生效（推荐用于需要特定顺序的场景）
    auto v1 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })   // 逆序
        .toOrdered()                                 // 关键！索引生效
        .toVector();   // 结果：[99, 98, ..., 0]

    // 2. 索引操作全部被忽略（追求极致性能）
    auto v2 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })
        .toUnordered()                               // 索引被忽略
        .toVector();   // 结果：无序，但最快

    // 3. 循环流 + 有序收集 = 永久循环取前 10 个（经典用法）
    auto top10 = Semantic<int>::range(0, 1'000'000'000)
        .redirect([](auto&, auto i) { return i % 1000; })  // 0~999 循环
        .toOrdered()
        .limit(10)
        .toVector();   // [0,1,2,3,4,5,6,7,8,9]

    return 0;
}
```

## 完整链式调用示例

```cpp
// 流创建
Semantic<int>::range(0, 1000);
Semantic<long long>::range(0, 1LL<<60);
Semantic<int>::iterate([](auto yield, auto) {
    for (int i = 0;; ++i) yield(i);                  // 无限流
});
Semantic<int>::of(1, 2, 3, 4, 5);
Semantic<int>::from(std::vector{1,2,3,4});
Semantic<int>::fill(42, 1'000'000);
Semantic<int>::fill([] { return rand(); }, 1000);

// 中间操作（全部惰性）
stream.map([](int x) { return x * x; });
stream.filter([](int x) { return x % 2 == 0; });
stream.flatMap([](int x) { return Semantic<int>::range(0, x); });
stream.distinct();                                     // 值去重
stream.skip(100);
stream.limit(50);
stream.takeWhile([](int x) { return x < 1000; });
stream.dropWhile([](int x) { return x < 100; });
stream.peek([](int x) { std::cout << x << ' '; });

// 关键：索引重映射操作（只有 toOrdered 时才生效）
stream.redirect([](auto&, auto i) { return -i; });           // 逆序
stream.redirect([](auto&, auto i) { return i % 100; });      // 循环
stream.redirect([](auto e, auto i) { return std::hash<int>{}(e); }); // 打乱
stream.reverse();                                            // 等价于 redirect(-i)
stream.shuffle();

// 并行
stream.parallel();       // 使用所有核心
stream.parallel(8);      // 指定线程数

// 必须显式选择是否尊重索引
auto ordered   = stream.toOrdered();     // 所有 redirect 等操作生效
auto unordered = stream.toUnordered();   // 所有 redirect 等操作被忽略（最快）

// 有序收集（索引决定最终顺序）
ordered.toVector();
ordered.toList();
ordered.toSet();                 // 按最终索引去重
ordered.forEach([](int x) { std::cout << x << ' '; });
ordered.cout();                  // [99, 98, 97, ...]

// 无序收集（最快路径）
unordered.toVector();
unordered.toList();
unordered.toSet();
unordered.forEach(...);
unordered.cout();

// 统计（总是走最快路径）
auto stats = stream.toUnordered().toStatistics();

// reduce（建议使用无序路径）
int sum = stream.toUnordered()
    .reduce(0, [](int a, int b) { return a + b; });
```

## 方法总览

| 方法                     | 说明                                      | 是否尊重索引操作 |
|--------------------------|-------------------------------------------|------------------|
| `toOrdered()`            | 进入“语义收集”模式，所有索引操作生效     | 是               |
| `toUnordered()`          | 进入“性能优先”模式，所有索引操作被忽略   | 否（最快）       |
| `toVector()` / `toList()`| 收集为容器                               | 取决于前者       |
| `toSet()`                | 收集为 set（去重）                       | 取决于前者       |
| `forEach` / `cout`       | 遍历或打印                               | 取决于前者       |
| `redirect` / `reverse` / `shuffle` | 改变逻辑索引                   | 仅在 toOrdered 时生效 |
| `parallel`               | 并行执行                                 | 两者都支持       |

## 为什么必须显式选择 toOrdered / toUnordered？

因为“索引操作”和“并行执行”是完全正交的两个维度：

- `redirect` 等是**语义操作**（改变元素的逻辑位置）
- `parallel` 是**执行策略**（如何计算）

只有在收集时声明“是否尊重语义”，才能既提供极致性能，又提供精确的顺序控制。

## 编译要求

- C++17 或更高  
- 仅需 `#include "semantic.h"`  
- 零外部依赖  
- 单文件头库

```bash
g++ -std=c++17 -O3 -pthread semantic.cpp
```

## 许可证

MIT 许可证 —— 可随意用于商用、开源、私有项目。

---

**semantic-cpp**：  
**索引决定顺序，执行只管快慢。**  
你写 `redirect`，就真的能逆序；你写 `toUnordered()`，就真的最快。  
没有妥协，只有选择。

写流，不再猜。  
只说一句：**“我要语义，还是我要性能？”**
