
# 🚀 Semantic‑Cpp：面向未来的 C++ 智能流处理框架

Semantic‑Cpp 是一个完全重构设计的现代化 C++ 流处理库，采用 **“多头文件、零外部依赖”** 的模块化架构。每个头文件职责清晰、独立可测，共同构成了一个完整的流处理生态系统。本库创造性地融合了多种编程范式的精华：

- 🎯 **Java Stream API 的优雅与流畅**：链式调用，声明式编程，让代码如诗般优雅  
- ⚡ **JavaScript Generator 的惰性与灵活**：延迟计算，按需生成，内存友好  
- 🗄️ **数据库索引的高效与有序**：智能排序，索引驱动，时序数据处理利器  

与传统的数据处理方式（手写循环、异步回调）不同，Semantic‑Cpp 旨在提供一种 **类型安全、表达力强且高性能** 的解决方案。其核心设计哲学是 **精准的数据流控制**：数据仅在需要时流动，顺序和位置通过 **“索引”** 精细调控，实现资源最优利用。

---

## 📐 项目架构：七层模块化设计

Semantic‑Cpp 由 **七个核心头文件** 组成，层层递进，每个文件职责单一、独立可测：

```
┌─────────────────────────────────────────────────┐
│                  semantic.h                     │
│        (流式中间操作、容器特化、Collectable)       │
├─────────────────────────────────────────────────┤
│                 semantics.h                     │
│         (流构建器、工厂方法、文本处理)            │
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│    (收集器工厂：count、group、reduce、DFT等)     │
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│     (收集器框架：五阶段模型、并发支持)            │
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│    (Unicode字符序列、编码转换、正则支持)          │
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│   (线程池：紧急关闭、异常传播、并发任务提交)       │
├─────────────────────────────────────────────────┤
│                 function.h                      │
│     (类型定义：Generator、Supplier、Consumer等)   │
└─────────────────────────────────────────────────┘
```

### 依赖关系

```
function.h          ← 无依赖，类型基石
pool.h              ← 依赖 function.h
charsequence.h      ← 独立模块，Unicode处理
collector.h         ← 依赖 function.h、pool.h
collectors.h        ← 依赖 collector.h、charsequence.h
semantic.h          ← 依赖 collector.h、collectors.h、charsequence.h
semantics.h         ← 依赖 semantic.h
```

> 每个头文件均可独立编译测试，也支持按需引入。  
> 例如，只需收集器功能时，仅引入 `collector.h` 和 `collectors.h` 即可。

---

## 🏗️ 第一层：`function.h` — 类型基础

`function.h` 定义了整个框架的类型系统，是所有模块的共同基石：

```cpp
namespace function {
    using Timestamp = long long;           // 索引类型
    using Module = unsigned long long;     // 模块/计数类型
    using Generator = BiConsumer<
        BiConsumer<T, Timestamp>,
        BiPredicate<T, Timestamp>
    >;
}
```

`Generator` 是整个流系统的核心抽象：接受 `accept`（接收数据）和 `interrupt`（中断流程）两个回调，体现了 **“惰性拉取”** 模型。

---

## ⚡ 第二层：`pool.h` — 并发基石

`pool.h` 提供全局线程池 `pool::pool`，是整个框架的并发引擎：

| 特性 | 说明 |
|----|----|
| 🎯 声明式并行 | `parallel(n)` 仅声明意图，终端操作时自动激活 |
| 🛡️ 紧急关闭 | 内置 `emergencyShutdown()` 和 `std::set_terminate` 处理器 |
| 🔄 异常传播 | `submit()` 返回 `std::future`，支持异常安全传播 |

---

## 🔤 第三层：`charsequence.h` — Unicode 字符序列

`charsequence.h` 是一个完整的 Unicode 处理模块：

| 功能 | 描述 |
|----|----|
| 🌐 多编码支持 | UTF‑8、UTF‑16、UTF‑32、Latin1、GBK 等 |
| 🔍 码点迭代器 | `PointIterator` 支持双向遍历 Unicode 码点 |
| 🏗️ 构建器模式 | `Builder` 类支持高效字符串拼接 |
| 📐 正则表达式 | `Regex` 类封装 `std::regex` |
| 🔑 哈希与比较 | 所有核心类型均有 `std::hash` 和 `std::less` 特化 |

---

## 🔧 第四层：`collector.h` — 收集器框架

`collector.h` 实现了收集器模式，是终端操作的核心引擎。

### 五阶段模型

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (可选短路)
```

### 类型别名

| 类型 | 定义 | 角色 |
|----|----|----|
| `Identity<A>` | `Supplier<A>` | 提供初始值 |
| `Accumulator<A,E>` | `TriFunction<A,E,Timestamp,A>` | 累加元素 |
| `Combiner<A>` | `BiFunction<A,A,A>` | 合并并行结果 |
| `Finisher<A,R>` | `Function<A,R>` | 最终转换 |
| `Interrupt<E,A>` | `TriPredicate<E,Timestamp,A>` | 短路判断 |

### 并发支持

`Collector::collect()` 自动处理：

- 📦 数据分片（按索引模运算分发到各线程）
- 🔗 结果合并（通过 `Combiner` 归并局部结果）
- ⚠️ 异常传播（通过 `std::exception_ptr` 和 `std::atomic<bool>`）

---

## 🏭 第五层：`collectors.h` — 收集器工厂

`collectors.h` 提供了丰富的预置收集器工厂函数。

### 📊 匹配操作

| 方法 | 描述 | 返回类型 |
|----|----|----|
| `useAllMatch(predicate)` | 所有元素满足条件 | `bool` |
| `useAnyMatch(predicate)` | 任意元素满足条件 | `bool` |
| `useNoneMatch(predicate)` | 没有元素满足条件 | `bool` |

### 🔍 查找操作

| 方法 | 描述 | 返回类型 |
|----|----|----|
| `useFindFirst()` | 查找第一个元素 | `std::optional<E>` |
| `useFindLast()` | 查找最后一个元素 | `std::optional<E>` |
| `useFindAny()` | 随机查找元素 | `std::optional<E>` |
| `useFindAt(index)` | 查找指定索引元素（支持负索引） | `std::optional<E>` |
| `useFindMaximum()` | 查找最大值 | `std::optional<E>` |
| `useFindMinimum()` | 查找最小值 | `std::optional<E>` |

### 📈 聚合操作

| 方法 | 描述 | 返回类型 |
|----|----|----|
| `useCount()` | 计算元素总数 | `Module` |
| `useSummate<E,D>()` | 求和 | `D` |
| `useSummate<E,D>(mapper)` | 带映射的求和 | `D` |
| `useAverage<E,D>()` | 平均值 | `D` |
| `useAverage<E,D>(mapper)` | 带映射的平均值 | `D` |
| `useRange<E,D>()` | 数值范围 | `D` |
| `useRange<E,D>(mapper)` | 带映射的范围 | `D` |
| `useMinimum<E,D>()` | 最小值 | `std::optional<D>` |
| `useMaximum<E,D>()` | 最大值 | `std::optional<D>` |

### 📊 统计操作

| 方法 | 描述 | 返回类型 |
|----|----|----|
| `useVariance<E,D>()` | 总体方差 | `D` |
| `useStandardDeviation<E,D>()` | 总体标准差 | `D` |
| `useSkewness<E,D>()` | 偏度 | `D` |
| `useKurtosis<E,D>()` | 峰度 | `D` |
| `useMedian<E,D>()` | 中位数 | `std::optional<D>` |
| `useMode<E>()` | 众数 | `std::optional<E>` |
| `usePercentile<E,D>(p)` | 第 p 百分位数 | `std::optional<D>` |
| `useFrequency<E>()` | 频域特征 | `std::map<E, complex>` |
| `useDistribution<E>()` | 空间分布特征 | `std::map<E, complex>` |

### 🔗 归约操作

| 方法 | 描述 | 返回类型 |
|----|----|----|
| `useReduce(reducer)` | 无初始值归约 | `std::optional<E>` |
| `useReduce(identity, reducer)` | 带初始值归约 | `E` |
| `useReduce(id, red, comb, fin)` | 完全自定义归约 | `R` |

### 📦 收集操作

| 方法 | 描述 | 返回类型 |
|----|----|----|
| `useToVector()` | 收集为 `vector` | `std::vector<E>` |
| `useToList()` | 收集为 `list` | `std::list<E>` |
| `useToSet()` | 收集为 `set`（去重） | `std::set<E>` |
| `useToMap(keyExtractor)` | 收集为 `map` | `std::map<K,E>` |
| `useGroup(keyExtractor)` | 按键分组 | `std::unordered_map<K,vector<E>>` |
| `usePartition(size)` | 按大小分区 | `std::vector<vector<E>>` |

### 🎨 输出操作

| 方法 | 描述 | 返回类型 |
|----|----|----|
| `useJoin()` | 连接为字符串 | `Charsequence` |
| `useOut()` | 输出到 stdout | `Charsequence` |
| `useError()` | 输出到 stderr | `Charsequence` |

### 🧮 数学工具

| 方法 | 描述 | 返回类型 |
|----|----|----|
| `useDFT()` | 离散傅里叶变换 | `vector<complex<double>>` |
| `useIDFT()` | 逆离散傅里叶变换 | `vector<complex<double>>` |
| `useFFT()` | 快速傅里叶变换 (Cooley‑Tukey) | `vector<complex<double>>` |
| `useIFFT()` | 逆快速傅里叶变换 | `vector<complex<double>>` |
| `useGradient(gradFunc, lr, iter, th)` | 梯度下降（解析梯度） | `vector<double>` |
| `useGradient(costFunc, lr, iter, th, h)` | 梯度下降（数值梯度） | `vector<double>` |

---

## 🌊 第六层：`semantic.h` — 流式中间操作

`semantic.h` 是整个框架的核心，包含 `collectable` 和 `semantic` 两个命名空间。

### `collectable` 命名空间

提供可收集对象的继承体系：

| 类 | 描述 | 底层存储 |
|----|----|----|
| `Collectable<E>` | 抽象基类，纯虚函数 `source()` | — |
| `OrderedCollectable<E>` | 有序收集 | `std::map<Timestamp, E>` |
| `UnorderedCollectable<E>` | 无序收集 | `std::unordered_map<Timestamp, E>` |
| `Statistics<E, D>` | 统计收集（继承 `OrderedCollectable`） | 提供 20+ 统计方法 |
| `WindowCollectable<E>` | 窗口收集（继承 `OrderedCollectable`） | 支持 slide / tumble |

#### `Statistics` 类方法

| 方法 | 返回类型 | 描述 |
|----|----|----|
| `summate()` / `summate(mapper)` | `D` | 求和 |
| `average()` / `average(mapper)` | `D` | 平均值 |
| `minimum()` / `minimum(mapper)` | `std::optional<D>` | 最小值 |
| `maximum()` / `maximum(mapper)` | `std::optional<D>` | 最大值 |
| `range()` / `range(mapper)` | `D` | 范围（最大 − 最小） |
| `variance()` / `variance(mapper)` | `D` | 总体方差 |
| `standardDeviation()` / `standardDeviation(mapper)` | `D` | 总体标准差 |
| `frequency()` / `frequency(mapper)` | `std::map<*, complex>` | 频域特征 |
| `distribute()` / `distribute(mapper)` | `std::map<*, complex>` | 空间分布特征 |
| `median()` / `median(mapper)` | `std::optional<D>` | 中位数 |
| `mode()` | `std::optional<E>` | 众数 |
| `percentile(p)` / `percentile(p, mapper)` | `std::optional<D>` | 第 p 百分位数 |
| `firstQuartile()` / `firstQuartile(mapper)` | `std::optional<D>` | 第一四分位数 (Q1) |
| `thirdQuartile()` / `thirdQuartile(mapper)` | `std::optional<D>` | 第三四分位数 (Q3) |
| `interquartileRange()` / `interquartileRange(mapper)` | `std::optional<D>` | 四分位距 (Q3 − Q1) |
| `skewness()` / `skewness(mapper)` | `D` | 偏度 |
| `kurtosis()` / `kurtosis(mapper)` | `D` | 峰度 |
| `dft()` | `vector<complex<double>>` | 离散傅里叶变换 |
| `idft()` | `vector<complex<double>>` | 逆离散傅里叶变换 |
| `fft()` | `vector<complex<double>>` | 快速傅里叶变换 |
| `ifft()` | `vector<complex<double>>` | 逆快速傅里叶变换 |
| `gradient(gradFunc, lr, iter, th)` | `vector<double>` | 梯度下降（解析梯度） |
| `gradient(costFunc, lr, iter, th, h)` | `vector<double>` | 梯度下降（数值梯度） |

### `semantic` 命名空间

提供 `Semantic<E>` 模板类及其完整的特化体系。

#### 主模板方法列表

| 类别 | 方法 |
|----|----|
| 🎨 元素变换 | `map`、`flatMap`、`flat` |
| 🔍 元素过滤 | `filter`、`takeWhile`、`dropWhile`、`distinct` |
| 📏 数量控制 | `limit`、`skip`、`sub` |
| 📐 索引操作 | `redirect`、`reverse`、`translate`、`sort` |
| 👀 观察操作 | `peek` |
| ⚡ 并行声明 | `parallel` |
| 🔗 连接操作 | `concatenate` |
| 📤 终端转换 | `toUnordered`、`toOrdered`、`toWindow`、`toStatistics` |

#### 容器特化完整支持

| 特化类型 | 描述 |
|----|----|
| `Semantic<std::vector<E>>` | 向量容器流 |
| `Semantic<std::list<E>>` | 链表容器流 |
| `Semantic<std::set<E>>` | 有序集合容器流 |
| `Semantic<std::unordered_set<E>>` | 无序集合容器流 |
| `Semantic<std::deque<E>>` | 双端队列容器流 |
| `Semantic<std::queue<E>>` | 队列容器流 |
| `Semantic<std::stack<E>>` | 栈容器流 |
| `Semantic<std::map<K,V>>` | 映射容器流 |
| `Semantic<std::unordered_map<K,V>>` | 无序映射容器流 |
| `Semantic<std::initializer_list<E>>` | 初始化列表流 |
| `Semantic<Semantic<E>>` | 嵌套流展平 |

---

## 🏭 第七层：`semantics.h` — 流构建器

`semantics.h` 提供所有流构建工厂函数。

### 📐 数值范围

| 方法 | 描述 |
|----|----|
| `useRange(start, end)` | 生成 `[start, end)` 范围内的数值流 |

### 📦 容器构建

| 方法 | 描述 |
|----|----|
| `useFrom(container)` | 从任何标准容器创建流 |
| `useOf(args...)` | 从可变参数列表创建流 |

### 📝 文本处理

| 方法 | 描述 |
|----|----|
| `useBlob(text)` | 将字符串按字节拆分为 `char` 流 |
| `useText(text)` | 将字符串作为整体文本流 |
| `useText(text, delimiter)` | 按分隔符拆分文本 |
| `useText(istream)` | 从输入流读取整个内容 |

### 🌐 Unicode 处理

| 方法 | 描述 |
|----|----|
| `useSequence(charsequence)` | 从字符序列创建码点流 |
| `useSequence(text, encoding)` | 从文本创建指定编码的码点流 |
| `useCharsequence(charsequence)` | 将字符序列作为整体流 |
| `useCharsequence(charsequence, delimiter)` | 按分隔符拆分字符序列 |

---

## 🧠 核心概念：索引驱动的数据世界

Semantic‑Cpp 将数据处理抽象为对 **“元素”** 及其 **“逻辑位置（索引）”** 的操作。理解这一点是掌握本库的关键。

### 1. 📐 基础索引变换

| 方法 | 描述 |
|----|----|
| `redirect(fn)` | 核心方法：自定义函数完全重写元素索引 |
| `reverse()` | 将所有索引逻辑反转（内部通过 `redirect` 实现） |
| `translate(offset)` | 固定偏移 |
| `translate(translator)` | 动态偏移函数 |

### 2. 📊 排序的“霸道”规则

⚠️ **`sort()` 会覆盖一切**：调用后所有之前的索引操作都将被覆盖，元素按值重新分配自然顺序索引。

- `sort()` → 立即物化为 `OrderedCollectable`
- `sort(comparator)` → 自定义比较器排序

### 3. ⚡ 声明式并行处理

- `parallel(n)` **仅声明意图**，不立即启动线程
- 终端操作（`toUnordered()`、`count()` 等）才真正触发并行
- 线程池自动处理任务分发和结果合并

### 4. 🎯 如何选择最终的数据容器？

| 转换方法 | 底层数据结构 | 性能特征 | 最佳适用场景 |
|----|----|----|----|
| `sort()` | `OrderedCollectable` | 排序后物化，保持值顺序 | 按值排序、分页、时间序列 |
| `toOrdered()` | `OrderedCollectable` | 保持当前索引顺序 | 保留自定义索引顺序 |
| `toUnordered()` | `UnorderedCollectable` | 平均 O(1)，最高性能 | 快速查找、去重、聚合 |
| `toWindow()` | `WindowCollectable` | 基于有序集合 | 滑动 / 滚动窗口分析 |
| `toStatistics()` | `Statistics` | 20+ 统计方法 | 全面统计分析 |

---

## 🚀 快速上手指南

### 安装

将所有头文件放入项目目录，确保编译器支持 **C++17 或更高标准**：

```
include/
├── function.h
├── pool.h
├── charsequence.h
├── collector.h
├── collectors.h
├── semantic.h
└── semantics.h
```

```cpp
#include "semantics.h"  // 自动包含其他依赖
```

---

### 🎯 基础示例：体验索引与排序

```cpp
auto result = semantic::useRange(0, 10)
    .map(int x -> int { return x * x; })
    .redirect(int value, auto index -> long long {
        return index * 2;
    })
    .reverse()
    .sort()                              // 强制按值排序，覆盖所有索引操作
    .toVector();

// 输出: 0 1 4 9 16 25 36 49 64 81
```

### ⚡ 并行处理示例

```cpp
auto count = semantic::useRange(1, 1000)
    .parallel(4)
    .filter(int x { return x % 2 == 0; })
    .toUnordered()
    .count();

// 输出: 偶数数量: 500
```

### 📊 统计分析示例

```cpp
auto stats = semantic::useRange(1, 101)
    .toStatistics<int, double>();

auto avg = stats.average();               // 平均值
auto med = stats.median();                // 中位数
auto std = stats.standardDeviation();     // 标准差
auto q1  = stats.firstQuartile();         // 第一四分位数
auto q3  = stats.thirdQuartile();         // 第三四分位数
auto skew = stats.skewness();             // 偏度
```

### 🔬 频域分析示例

```cpp
auto freq = data.toUnordered().frequency();
for (const auto& [element, z] : freq) {
    auto magnitude = std::abs(z);  // 分布集中度
    auto phase     = std::arg(z);  // 分布中心相位
}
```

### 🧮 FFT 变换示例

```cpp
auto spectrum = semantic::useRange(0, 8)
    .map(int x -> std::complex<double> {
        return {x, 0};
    })
    .toUnordered()
    .collect(collector::useFFT<double>());
```

---

## ⚡ 性能优化建议

1. 🎯 **选择对的容器**
   - 等值查找、不排序的聚合 → `toUnordered()`
   - 范围查询、排序、分页 → `toOrdered()` 或 `sort()`
   - 实时窗口分析 → `toWindow()`
2. ⚡ **善用并行**：数据量大或处理逻辑耗时时使用 `parallel()`，避免阻塞式 I/O
3. 📐 **优化操作顺序**：尽早 `filter`，明智 `sort`
4. 🔄 **利用惰性求值**：中间操作不立即执行，`takeWhile` 和 `limit` 可提前终止

---

## 📊 与 C++ 标准库及竞品对比

| 特性 | Semantic‑Cpp | C++20/23 ranges | 传统循环 |
|----|----|----|----|
| 🎯 核心范式 | 声明式、索引驱动 | 视图驱动、函数式组合 | 命令式、过程化 |
| ⚡ 并行支持 | 声明式，自动线程池 | 需组合并行算法 | 手动实现 |
| 📐 排序与索引 | 索引精细控制 | 破坏性排序 | 完全手动 |
| 📊 统计分析 | 20+ 内置统计方法 | 不内置 | 需第三方库 |
| 🔬 频域分析 | 原生 DFT / FFT / 频域特征 | 不原生支持 | 需第三方库 |
| 🧮 梯度下降 | 解析 + 数值双模式 | 不内置 | 需第三方库 |
| 🌐 Unicode | 原生多编码支持 | 不原生支持 | 手动处理 |
| 📦 依赖 | 零外部依赖，7 头文件 | 标准库 | 无 |

---

## 📜 许可

- 📄 **许可证**：基于 MIT 开源  

---

**Semantic‑Cpp — 用现代 C++ 构建高效、清晰的数据处理管道。🚀**

---
