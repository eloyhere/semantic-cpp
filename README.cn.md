# 🚀 Semantic-Cpp：面向未来的 C++ 智能流处理框架  

Semantic-Cpp 是一个完全重构设计的现代化 C++ 流处理库，采用 **“多头文件、零外部依赖”** 的模块化架构。每个头文件职责清晰、独立可测，共同构成了一个完整的流处理生态系统。本库创造性地融合了多种编程范式的精华：  

- 🎯 **Java Stream API 的优雅与流畅**：链式调用，声明式编程，让代码如诗般优雅  
- ⚡ **JavaScript Generator 的惰性与灵活**：延迟计算，按需生成，内存友好  
- 🗄️ **数据库索引的高效与有序**：智能排序，索引驱动，时序数据处理利器  
- 📦 **容器即元素的批处理哲学**：向量、链表、映射……任何容器都能作为流中一等公民，自由流动  

你是否厌倦了手写 for 循环遍历 vector，再嵌套一个 if 过滤，再手动 push_back 到另一个容器？  
你是否在深夜调试过索引偏移一格的 Bug，只因为想在反向遍历时取“倒数第三个”元素？  
你是否渴望像操作数据库一样——按索引精确定位、按窗口滑动分析、用一行链式调用完成从数据到统计的完整旅程？  

**Semantic-Cpp 正是为此而生。** 它将数据处理抽象为对 **“元素”** 及其 **“逻辑位置（索引）”** 的操作——就像数据库中的“行”和“主键”，你可以在不触碰数据本身的情况下，随意重排、偏移、反转索引；也可以把任何一个容器（vector、map、array……）当作一个不可分割的整体在流中传递，然后随时“拆包”回元素级别。这种双粒度自由切换的能力，是传统流式框架所不具备的。  

---

## 📐 项目架构：八层模块化设计  

Semantic-Cpp 由 **八个核心头文件** 组成，层层递进，每个文件职责单一、独立可测。五个命名空间各司其职，共同构成了从数据源头到最终结果的完整流水线：  

```
┌─────────────────────────────────────────────────┐
│                 semantics.h                     │
│   命名空间: semantic                             │
│   流构建工厂：数值范围、容器、文本、Unicode         │
├─────────────────────────────────────────────────┤
│                  semantic.h                     │
│   命名空间: semantic / collectable               │
│   流式中介操作、Collectable体系、10种容器特化      │
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│   命名空间: collector                            │
│   收集器工厂：匹配、查找、聚合、统计、DFT/FFT等     │
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│   命名空间: collector                            │
│   收集器框架：五阶段模型、并发并行支持              │
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│   命名空间: charsequence                         │
│   Unicode字符序列、多编码转换、Builder、Buffer     │
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│   命名空间: pool                                 │
│   全局线程池：任务提交、紧急关闭、异常传播            │
├─────────────────────────────────────────────────┤
│                 function.h                      │
│   命名空间: function                             │
│   类型定义：Generator、Supplier、Consumer等别名     │
├─────────────────────────────────────────────────┤
│               hash.h / less.h                   │
│   命名空间: std (扩展)                            │
│   标准库容器哈希与比较特化，支持任意嵌套             │
└─────────────────────────────────────────────────┘
```

---

## 📦 命名空间全景  

Semantic-Cpp 精心设计了五个命名空间，每个都像一个独立的“部门”，各司其职又紧密协作：  

| 命名空间 | 所在文件 | 职责 | 核心类型/函数 |
|----------|----------|------|---------------|
| `function` | function.h | 类型系统基石 | `Timestamp`, `Module`, `Generator<T>`, `Supplier<R>`, `Consumer<T>`, `Predicate<T>`, `Comparator<T>`, `Unary<T>`, `Binary<T>`, `BiFunction<T,U,R>`, `TriFunction<T,U,V,R>`, `BiConsumer<T,U>`, `BiPredicate<T,U>`, `TriPredicate<T,U,V>`, `Runnable`, `randomly()` |
| `pool` | pool.h | 并发执行引擎 | `pool::pool`（全局线程池）、`submit()`、`emergencyShutdown()` |
| `charsequence` | charsequence.h | Unicode 字符串处理 | `charset`（枚举：utf8/utf16/utf32/ascii/latin1 等）、`Meta`, `Point`, `Charsequence`, `Builder`, `Buffer`, `PointIterator` |
| `collector` | collector.h + collectors.h | 终端收集执行 | `Collector<E,A,R>`（五阶段模型）、`Identity<A>`, `Accumulator<A,E>`, `Combiner<A>`, `Finisher<A,R>`, `Interrupt<E,A>`, `useFull()`, `useShortable()`, 以及 100+ `useXxx()` 工厂函数 |
| `collectable` | semantic.h | 物化数据容器 | `Collectable<E>`（抽象基类）、`OrderedCollectable<E>`, `UnorderedCollectable<E>`, `WindowCollectable<E>`, `Statistics<E,D>` |
| `semantic` | semantic.h + semantics.h | 流构建与中间操作 | `Semantic<E>`（主模板）、`Semantic<Semantic<E>>`（嵌套特化）、10 种容器特化、`useRange()`, `useFrom()`, `useText()` 等 30+ 工厂函数 |

---

## 🔗 命名空间协作流程  

```
semantic::useRange(0, 100)          ← semantic 命名空间：创建流
    .map(int x { ... })         ← semantic 命名空间：中间变换
    .filter(int x { ... })      ← semantic 命名空间：中间过滤
    .toUnordered()                  ← 转换为 collectable 命名空间
    .toVector()                     ← 调用 collector 命名空间的收集器
```

---

## 🏗️ 第一层：function.h — 类型基础  

function.h 定义了整个框架的类型系统，是所有模块的共同基石。  

```cpp
namespace function {
    using Timestamp = long long;           // 索引类型
    using Module = unsigned long long;     // 模块/计数类型
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept
        std::function<bool(T, Timestamp)>       // interrupt
    )>;
}
```

Generator 是整个流系统的核心抽象，体现了 **“惰性拉取”** 模型——数据只在 `accept` 被调用时才真正流动。  

| 类型别名 | 完整定义 | 用途 |
|----------|----------|------|
| `Timestamp` | `long long` | 元素在流中的逻辑位置 |
| `Module` | `unsigned long long` | 计数、容量、并发度 |
| `Runnable` | `std::function<void()>` | 无参无返回值任务 |
| `Supplier<R>` | `std::function<R()>` | 供应者，无中生有 |
| `Function<T,R>` | `std::function<R(T)>` | 单参数函数 |
| `BiFunction<T,U,R>` | `std::function<R(T,U)>` | 双参数函数 |
| `TriFunction<T,U,V,R>` | `std::function<R(T,U,V)>` | 三参数函数 |
| `Unary<T>` | `std::function<T(T)>` | 一元运算 |
| `Binary<T>` | `std::function<T(T,T)>` | 二元运算 |
| `Consumer<T>` | `std::function<void(T)>` | 消费者 |
| `BiConsumer<T,U>` | `std::function<void(T,U)>` | 双参数消费者 |
| `TriConsumer<T,U,V>` | `std::function<void(T,U,V)>` | 三参数消费者 |
| `Predicate<T>` | `std::function<bool(T)>` | 谓词判断 |
| `BiPredicate<T,U>` | `std::function<bool(T,U)>` | 双参数谓词 |
| `TriPredicate<T,U,V>` | `std::function<bool(T,U,V)>` | 三参数谓词 |
| `Comparator<T>` | `std::function<int(const T&,const T&)>` | 比较器，返回负/零/正 |
| `Generator<T>` | `BiConsumer<BiConsumer<T,Timestamp>, BiPredicate<T,Timestamp>>` | 流生成器核心抽象 |

---

## ⚡ 第二层：pool.h — 并发基石  

pool.h 提供全局线程池 `pool::pool`，是整个框架的并发引擎。  

| 成员 | 类型 | 描述 |
|------|------|------|
| `pool::pool` | 全局线程池实例 | 程序级别的单例线程池，自动初始化 |
| `pool::pool.submit<A>(task)` | 方法 | 提交任务，返回 `std::future<A>` |
| `pool::pool.emergencyShutdown()` | 方法 | 紧急关闭所有线程 |

---

## 🔤 第三层：charsequence.h — Unicode 字符序列  

charsequence.h 是一个完整的 Unicode 处理模块，提供字符序列的创建、转换和操作功能。  

| 类型/函数 | 描述 |
|-----------|------|
| `charset` | 枚举 ascii、utf8、utf16、utf16be、utf16le、utf32、utf32be、utf32le、latin1 |
| `Meta` | 元数据包装器，存储无符号整数值 |
| `Point` | Unicode 码点，支持代理对检测和有效性验证 |
| `Charsequence` | 不可变字符序列：split、replace、indexOf、sub、trim、toUpperCase、toLowerCase、reverse、startsWith、endsWith、contains、compare、getBytes、getPoints、join |
| `Builder` | 可变字节构建器：prepend、insert、append（支持 bool、整数、浮点、字符串等） |
| `Buffer` | 线程安全环形缓冲区：write、read、peek、prepend、append、clear、shrinkToFit、atomic |
| `PointIterator` | 双向迭代器，遍历 Unicode 码点 |
| `sequenceLength()` | 根据首字节判断 UTF‑8 序列长度 |
| `encode()` / `decode()` | 编码/解码单个码点 |
| `convert()` | 编码转换 |

---

## 🔧 第四层：collector.h — 收集器框架  

### 五阶段模型  

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (可选短路)
```

| 类型别名 | 完整定义 | 角色 |
|----------|----------|------|
| `Identity<A>` | `function::Supplier<A>` | 提供初始值 |
| `Accumulator<A,E>` | `function::TriFunction<A, E, Timestamp, A>` | 累加元素 |
| `Combiner<A>` | `function::BiFunction<A, A, A>` | 合并并行结果 |
| `Finisher<A,R>` | `function::Function<A, R>` | 最终转换 |
| `Interrupt<E,A>` | `function::TriPredicate<E, Timestamp, A>` | 短路判断 |
| 框架函数 | 描述 |
|----------|------|
| `useFull(identity, accumulator, combiner, finisher)` | 创建完整收集器（无短路） |
| `useShortable(identity, interrupt, accumulator, combiner, finisher)` | 创建可短路收集器 |

---

## 🏭 第五层：collectors.h — 收集器工厂  

### 📊 匹配操作  

| 方法 | 描述 | 返回类型 |
|------|------|----------|
| `useAllMatch(predicate)` | 所有元素满足条件 | `bool` |
| `useAnyMatch(predicate)` | 任意元素满足条件 | `bool` |
| `useNoneMatch(predicate)` | 没有元素满足条件 | `bool` |

### 🔍 查找操作  

| 方法 | 描述 | 返回类型 |
|------|------|----------|
| `useFindFirst()` | 查找第一个元素 | `std::optional<E>` |
| `useFindLast()` | 查找最后一个元素 | `std::optional<E>` |
| `useFindAny()` | 随机查找元素 | `std::optional<E>` |
| `useFindAt(index)` | 查找指定索引元素（支持负索引） | `std::optional<E>` |
| `useFindMaximum()` | 查找最大值（支持自定义比较器） | `std::optional<E>` |
| `useFindMinimum()` | 查找最小值（支持自定义比较器） | `std::optional<E>` |

### 📈 聚合操作  

| 方法 | 描述 | 返回类型 |
|------|------|----------|
| `useCount()` | 计算元素总数 | `Module` |
| `useSummate<E,D>()` | 求和 | `D` |
| `useAverage<E,D>()` | 平均值 | `D` |
| `useRange<E,D>()` | 数值范围（最大减最小） | `D` |
| `useMinimum<E,D>()` | 最小值 | `std::optional<D>` |
| `useMaximum<E,D>()` | 最大值 | `std::optional<D>` |

### 📊 统计操作  

| 方法 | 描述 | 返回类型 |
|------|------|----------|
| `useVariance<E,D>()` | 总体方差 | `D` |
| `useStandardDeviation<E,D>()` | 总体标准差 | `D` |
| `useSkewness<E,D>()` | 偏度 | `D` |
| `useKurtosis<E,D>()` | 峰度 | `D` |
| `useMedian<E,D>()` | 中位数 | `std::optional<D>` |
| `useMode<E>()` | 众数（基于频域分析） | `std::optional<E>` |
| `usePercentile<E,D>(p)` | 第 p 百分位数 | `std::optional<D>` |
| `useFrequency<E>()` | 频域特征（索引相位编码） | `std::map<E, complex>` |
| `useDistribution<E>()` | 空间分布特征（位置编码） | `std::map<E, complex>` |

### 🔗 归约操作  

| 方法 | 描述 | 返回类型 |
|------|------|----------|
| `useReduce(reducer)` | 无初始值归约 | `std::optional<E>` |
| `useReduce(identity, reducer)` | 带初始值归约 | `E` |
| `useReduce(id, red, comb, fin)` | 完全自定义归约 | `R` |

### 📦 收集到容器操作  

| 方法 | 描述 | 返回类型 |
|------|------|----------|
| `useToVector()` | 收集为 vector | `std::vector<E>` |
| `useToList()` | 收集为 list | `std::list<E>` |
| `useToDeque()` | 收集为 deque | `std::deque<E>` |
| `useToForwardList()` | 收集为 forward_list | `std::forward_list<E>` |
| `useToArray<N>()` | 收集为固定大小 array | `std::array<E, N>` |
| `useToSet()` | 收集为 set（去重排序） | `std::set<E>` |
| `useToMultiset()` | 收集为 multiset | `std::multiset<E>` |
| `useToUnorderedSet()` | 收集为 unordered_set | `std::unordered_set<E>` |
| `useToUnorderedMultiset()` | 收集为 unordered_multiset | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)` | 收集为 map | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)` | 收集为 map（自定义值） | `std::map<K, V>` |
| `useToMultimap(keyExtractor)` | 收集为 multimap | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)` | 收集为 multimap（自定义值） | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)` | 收集为 unordered_map | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)` | 收集为 unordered_multimap | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)` | 收集为 unordered_multimap | `std::unordered_multimap<K, V>` |
| `useToStack()` | 收集为 stack | `std::stack<E>` |
| `useToQueue()` | 收集为 queue | `std::queue<E>` |
| `useToPriorityQueue()` | 收集为 priority_queue | `std::priority_queue<E>` |

### 🔀 分组与分区操作  

| 方法 | 描述 | 返回类型 |
|------|------|----------|
| `useGroup(keyExtractor)` | 按键分组 | `std::unordered_map<K, vector<E>>` |
| `usePartition(size)` | 按固定大小分区 | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)` | 按自定义键分区 | `std::vector<vector<E>>` |

### 🎨 字符串输出操作  

| 方法 | 描述 | 返回类型 |
|------|------|----------|
| `useJoin()` | 连接为字符串（默认逗号分隔，方括号包围） | `Charsequence` |
| `useJoin(delimiter)` | 自定义分隔符连接 | `Charsequence` |
| `useJoin(prefix, delimiter, suffix)` | 完全自定义格式化连接 | `Charsequence` |
| `useOut()` | 格式化输出到 stdout | `Charsequence` |
| `useOut(delimiter)` | 自定义分隔符输出到 stdout | `Charsequence` |
| `useOut(prefix, delimiter, suffix)` | 完全自定义格式化输出到 stdout | `Charsequence` |
| `useError()` | 格式化输出到 stderr | `Charsequence` |
| `useError(delimiter)` | 自定义分隔符输出到 stderr | `Charsequence` |
| `useError(prefix, delimiter, suffix)` | 完全自定义格式化输出到 stderr | `Charsequence` |

### 🧮 数学工具  

| 方法 | 描述 | 返回类型 |
|------|------|----------|
| `useDFT()` | 离散傅里叶变换 | `vector<complex<double>>` |
| `useIDFT()` | 逆离散傅里叶变换 | `vector<complex<double>>` |
| `useFFT()` | 快速傅里叶变换 (Cooley-Tukey) | `vector<complex<double>>` |
| `useIFFT()` | 逆快速傅里叶变换 | `vector<complex<double>>` |
| `useGradient(gradFunc, lr, iter, th)` | 梯度下降（解析梯度） | `vector<double>` |
| `useGradient(costFunc, lr, iter, th, h)` | 梯度下降（数值梯度） | `vector<double>` |

---

## 🌊 第六层：semantic.h — 流式中间操作与收集体系  

---

### 🔑 核心设计：三阶段流水线 — Semantic → Collectable → Collector  

```
┌──────────────────────────────────────────────────────────┐
│                    第一阶段：构建与变换                      │
│                   Semantic<E>  (语义流)                    │
│                   命名空间: semantic                       │
│                                                          │
│  · 惰性构建：useRange、useFrom、useGenerate...             │
│  · 中间变换：map、filter、takeWhile、distinct...           │
│  · 索引操作：reverse、translate、redirect...               │
│  · 并行声明：parallel(n)                                   │
│                                                          │
│  此时数据还“躺”在管道里，一动不动，什么都没发生。              │
└──────────────────┬───────────────────────────────────────┘
                   │  toUnordered() / toOrdered() /
                   │  toWindow() / toStatistics() / sort()
                   ▼
┌──────────────────────────────────────────────────────────┐
│                   第二阶段：物化与收集                       │
│               Collectable<E>  (可收集对象)                  │
│                命名空间: collectable                       │
│                                                          │
│  · 触发数据流动：此时 Generator 被真正调用                   │
│  · 选择数据结构：有序 map / 无序 unordered_map               │
│  · 物化所有元素到内存                                       │
│                                                          │
│  数据终于“活”了！它们从管道中涌出，被整理到合适的容器中。        │
└──────────────────┬───────────────────────────────────────┘
                   │  toVector() / findFirst() / count() /
                   │  summate() / average() / join() / ...
                   ▼
┌──────────────────────────────────────────────────────────┐
│                   第三阶段：终端计算                         │
│                  Collector<E,A,R>                         │
│                  命名空间: collector                       │
│                                                          │
│  · 五阶段执行：Identity → Accumulate → Combine → Finish    │
│  · 支持并行：多线程分片累加，自动合并                         │
│  · 返回最终结果：vector、optional、double、bool...          │
└──────────────────────────────────────────────────────────┘
```

**关键规则** 👉 你必须先通过 `toUnordered()`、`toOrdered()`、`toWindow()`、`toStatistics()` 或 `sort()` 将 `Semantic<E>` 转换为 `Collectable<E>`，然后才能调用终端方法。

---

### 🎯 五种物化路径  

| 转换方法 | 目标类型 | 底层数据结构 | 性能特征 | 最佳适用场景 |
|----------|----------|--------------|----------|--------------|
| `toUnordered()` | `UnorderedCollectable<E>` | `unordered_map<Timestamp,E>` | 平均 O(1) 查找 | 快速查找、去重、聚合 |
| `toOrdered()` | `OrderedCollectable<E>` | `map<Timestamp,E>` | O(log n) 查找 | 保留索引顺序、时间序列 |
| `toWindow()` | `WindowCollectable<E>` | 继承有序集合 | 支持 slide/tumble | 滑动/滚动窗口分析 |
| `toStatistics()` | `Statistics<E,D>` | 继承有序集合 30+ 统计方法 | 全面统计分析 |
| `sort()` | `OrderedCollectable<E>` | `map<Timestamp,E>` 按值排序后物化 | 排序、分页、排名 |

---

### 📦 collectable 命名空间 — 全部终端方法  

#### 匹配操作  

| 方法 | 返回类型 | 描述 |
|------|----------|------|
| `anyMatch(predicate)` | `bool` | 任意元素满足条件 |
| `allMatch(predicate)` | `bool` | 所有元素满足条件 |
| `noneMatch(predicate)` | `bool` | 没有元素满足条件 |

#### 查找操作  

| 方法 | 返回类型 | 描述 |
|------|----------|------|
| `findFirst()` | `std::optional<E>` | 查找第一个元素 |
| `findLast()` | `std::optional<E>` | 查找最后一个元素 |
| `findAny()` | `std::optional<E>` | 随机查找元素 |
| `findAt(index)` | `std::optional<E>` | 指定索引查找（支持负索引） |
| `findMaximum()` | `std::optional<E>` | 查找最大值 |
| `findMaximum(comparator)` | `std::optional<E>` | 自定义比较查找最大值 |
| `findMinimum()` | `std::optional<E>` | 查找最小值 |
| `findMinimum(comparator)` | `std::optional<E>` | 自定义比较查找最小值 |

#### 聚合操作  

| 方法 | 返回类型 | 描述 |
|------|----------|------|
| `count()` | `function::Module` | 计算元素总数 |
| `empty()` | `bool` | 流是否为空 |

#### 归约操作  

| 方法 | 返回类型 | 描述 |
|------|----------|------|
| `reduce(accumulator)` | `std::optional<E>` | 无初始值归约 |
| `reduce(identity, accumulator)` | `E` | 带初始值归约 |
| `reduce(identity, acc, combiner)` | `R` | 完全自定义归约 |

#### 收集到序列容器  

| 方法 | 返回类型 | 描述 |
|------|----------|------|
| `toVector()` | `std::vector<E>` | 收集为 vector，保持顺序 |
| `toList()` | `std::list<E>` | 收集为 list |
| `toDeque()` | `std::deque<E>` | 收集为 deque |
| `toForwardList()` | `std::forward_list<E>` | 收集为 forward_list |
| `toArray<N>()` | `std::array<E, N>` | 收集为固定大小 array |

#### 收集到关联容器  

| 方法 | 返回类型 | 描述 |
|------|----------|------|
| `toSet()` | `std::set<E>` | 收集为 set（去重排序） |
| `toMultiset()` | `std::multiset<E>` | 收集为 multiset |
| `toUnorderedSet()` | `std::unordered_set<E>` | 收集为 unordered_set |
| `toUnorderedMultiset()` | `std::unordered_multiset<E>` | 收集为 unordered_multiset |
| `toMap(keyExtractor)` | `std::map<K, E>` | 按键收集为 map |
| `toMap(keyExtractor, valueExtractor)` | `std::map<K, V>` | 自定义键值收集为 map |
| `toMultimap(keyExtractor)` | `std::multimap<K, E>` | 按键收集为 multimap |
| `toMultimap(keyExtractor, valueExtractor)` | `std::multimap<K, V>` | 自定义键值收集为 multimap |
| `toUnorderedMap(keyExtractor, valueExtractor)` | `std::unordered_map<K, V>` | 收集为 unordered_map |
| `toUnorderedMultimap(keyExtractor)` | `std::unordered_multimap<K, E>` | 按键收集为 unordered_multimap |
| `toUnorderedMultimap(keyExtractor, valueExtractor)` | `std::unordered_multimap<K, V>` | 自定义键值收集为 unordered_multimap |

#### 收集到适配器容器  

| 方法 | 返回类型 | 描述 |
|------|----------|------|
| `toStack()` | `std::stack<E>` | 收集为 stack |
| `toQueue()` | `std::queue<E>` | 收集为 queue |
| `toPriorityQueue()` | `std::priority_queue<E>` | 收集为 priority_queue |

#### 分组与分区  

| 方法 | 返回类型 | 描述 |
|------|----------|------|
| `group(keyExtractor)` | `std::unordered_map<K, std::vector<E>>` | 按键分组 |
| `partition(size)` | `std::vector<std::vector<E>>` | 按固定大小分区 |
| `partitionBy(keyExtractor)` | `std::vector<std::vector<E>>` | 按自定义键分区 |

#### 字符串输出操作  

| 方法 | 返回类型 | 描述 |
|------|----------|------|
| `join()` | `charsequence::Charsequence` | 默认格式连接：`[elem1,elem2,...]` |
| `join(delimiter)` | `charsequence::Charsequence` | 自定义分隔符 |
| `join(prefix, delimiter, suffix)` | `charsequence::Charsequence` | 完全自定义格式 |
| `out()` | `charsequence::Charsequence` | 输出到 stdout |
| `out(delimiter)` | `charsequence::Charsequence` | 带分隔符输出到 stdout |
| `out(prefix, delimiter, suffix)` | `charsequence::Charsequence` | 完全自定义输出到 stdout |
| `error()` | `charsequence::Charsequence` | 输出到 stderr |
| `error(delimiter)` | `charsequence::Charsequence` | 带分隔符输出到 stderr |
| `error(prefix, delimiter, suffix)` | `charsequence::Charsequence` | 完全自定义输出到 stderr |

#### 自定义收集  

| 方法 | 返回类型 | 描述 |
|------|----------|------|
| `collect(identity, acc, comb, fin)` | `R` | 自定义四阶段收集 |
| `collect(identity, interrupt, acc, comb, fin)` | `R` | 自定义可中断收集 |
| `forEach(consumer)` | `void` | 遍历每个元素执行副作用 |

---

### 📦 Statistics<E,D> — 统计方法  

| 方法 | 返回类型 | 描述 |
|------|----------|------|
| `summate()` | `D` | 求和 |
| `average()` | `D` | 平均值 |
| `minimum()` | `std::optional<D>` | 最小值 |
| `maximum()` | `std::optional<D>` | 最大值 |
| `range()` | `D` | 范围（最大值 − 最小值） |
| `variance()` | `D` | 总体方差 |
| `standardDeviation()` | `D` | 总体标准差 |
| `median()` | `std::optional<D>` | 中位数 |
| `mode()` | `std::optional<E>` | 众数（基于频域分析） |
| `percentile(p)` | `std::optional<D>` | 第 p 百分位数 |
| `firstQuartile()` | `std::optional<D>` | 第一四分位数 (Q1) |
| `thirdQuartile()` | `std::optional<D>` | 第三四分位数 (Q3) |
| `interquartileRange()` | `std::optional<D>` | 四分位距 (Q3 − Q1) |
| `skewness()` | `D` | 偏度 |
| `kurtosis()` | `D` | 峰度 |
| `frequency()` | `std::map<E, std::complex<double>>` | 频域特征（索引相位编码） |
| `distribute()` | `std::map<E, std::complex<double>>` | 空间分布特征（位置编码） |
| `dft()` | `std::vector<std::complex<double>>` | 离散傅里叶变换 |
| `idft()` | `std::vector<std::complex<double>>` | 逆离散傅里叶变换 |
| `fft()` | `std::vector<std::complex<double>>` | 快速傅里叶变换 |
| `ifft()` | `std::vector<std::complex<double>>` | 逆快速傅里叶变换 |
| `gradient(gradFunc, lr, iter, th)` | `std::vector<double>` | 梯度下降（解析梯度） |
| `gradient(costFunc, lr, iter, th, h)` | `std::vector<double>` | 梯度下降（数值梯度） |

> 📝 以上方法均支持可选的 `mapper` 参数版本（如 `average(mapper)`），先对元素做变换再统计。

---

### 📦 全新的容器特化：容器也是元素  

Semantic-Cpp 为 **10 种标准库容器** 提供了 `Semantic` 模板特化：  

| 特化类型 | 说明 |
|----------|------|
| `Semantic<std::vector<E>>` | 向量容器流 |
| `Semantic<std::list<E>>` | 链表容器流 |
| `Semantic<std::deque<E>>` | 双端队列容器流 |
| `Semantic<std::set<E>>` | 有序集合容器流 |
| `Semantic<std::unordered_set<E>>` | 无序集合容器流 |
| `Semantic<std::map<K,V>>` | 有序映射容器流 |
| `Semantic<std::unordered_map<K,V>>` | 无序映射容器流 |
| `Semantic<std::queue<E>>` | 队列容器流 |
| `Semantic<std::stack<E>>` | 栈容器流 |
| `Semantic<std::array<E,N>>` | 固定数组容器流 |

所有容器特化完整支持 `map`、`filter`、`takeWhile`、`dropWhile`、`distinct`、`sort`、`limit`、`skip`、`reverse`、`translate`、`redirect`、`sub`、`concatenate`、`peek`、`flatMap`、`flat`、`parallel` 及全部终端转换方法。  

```cpp
// 容器流示例：展开多个向量为元素流
auto flattened = semantic::useOf({
    std::vector<int>{1, 2},
    std::vector<int>{3, 4, 5}
}).flatMap(std::vector<int> v {
    return semantic::useFrom(v).map(int x { return x * 10; });
}).toOrdered().toVector();
// 输出: 10 20 30 40 50
```

---

## 🏭 第七层：semantics.h — 流构建工厂  

### 📐 数值范围生成  

| 方法 | 描述 |
|------|------|
| `useRange(start, end)` | 生成 [start, end) 范围 |
| `useRange(start, end, step)` | 带步长，支持负步长 |
| `useRangeClosed(start, end)` | 生成 [start, end] 闭区间 |
| `useRangeClosed(start, end, step)` | 带步长的闭区间 |

### ♾️ 无限流生成  

| 方法 | 描述 |
|------|------|
| `useInfinite(seed, generator)` | 从种子值无限迭代 |
| `useGenerate(supplier)` | 无限调用供应者 |
| `useGenerate(supplier, limit)` | 有限次数调用供应者 |
| `useIterate(seed, generator)` | 从种子值无限迭代 |
| `useIterate(seed, generator, limit)` | 有限次数迭代 |
| `useRandom()` | 无限随机整数流 |
| `useRandom(min, max)` | 指定范围无限随机数 |
| `useRandom(min, max, count)` | 指定范围和数量随机数 |

### 📦 容器与元素构建  

| 方法 | 描述 |
|------|------|
| `useEmpty()` | 创建空流 |
| `useOf(element)` | 从单个元素创建流 |
| `useOf(e1, e2)` | 从两个元素创建流 |
| `useOf(e1, e2, e3)` | 从三个元素创建流 |
| `useOf({...})` | 从初始化列表创建流 |
| `useFrom(container)` | 从任何标准容器创建流 |
| `useFrom({...})` | 从初始化列表创建流 |
| `useRepeat(element, count)` | 重复指定元素 n 次 |

### 📝 文本处理  

| 方法 | 描述 |
|------|------|
| `useBlob(text)` | 按字节拆分为 char 流 |
| `useBlob(text, start, end)` | 指定范围按字节拆分 |
| `useBlob(istream)` | 从输入流按行读取 |
| `useBlob(istream, delimiter)` | 从输入流按分隔符读取 |
| `useText(text)` | 整体文本流（Charsequence） |
| `useText(text, delimiter)` | 按分隔符拆分文本 |
| `useText(istream)` | 从输入流读取整个内容 |
| `useText(istream, delimiter)` | 从输入流按分隔符读取 |

### 🌐 Unicode 处理  

| 方法 | 描述 |
|------|------|
| `useSequence(charsequence)` | 从字符序列创建码点流 |
| `useSequence(charsequence, start, end)` | 指定范围创建码点流 |
| `useSequence(text, encoding)` | 从文本创建指定编码的码点流 |
| `useSequence(istream, encoding)` | 从输入流创建码点流 |
| `useCharsequence(charsequence)` | 整体字符序列流 |
| `useCharsequence(charsequence, delimiter)` | 按分隔符拆分字符序列 |
| `useCharsequence(istream, encoding)` | 从输入流读取字符序列 |
| `useCharsequence(istream, delimiter, encoding)` | 从输入流按分隔符读取 |

---

## 📦 第八层：hash.h / less.h — 容器世界的通用语言  

`hash.h` 和 `less.h` 为所有标准库容器、pair、tuple、optional、variant、chrono 时间类型、complex 复数等提供了完整的 `std::hash` 和 `std::less` 特化，以及 `operator<` 重载。任意深度、任意组合的嵌套容器都可以作为 `unordered_set` 的键或 `set` 的元素。  

---

**Semantic-Cpp** — 用现代 C++ 构建高效、清晰的数据处理管道。🚀
