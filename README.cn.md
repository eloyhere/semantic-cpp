# 🚀 Semantic-Cpp：面向未来的 C++ 智能流处理框架

Semantic-Cpp 是一个完全重构设计的现代化 C++ 流处理库，采用**“多头文件、零外部依赖”**的模块化架构。每个头文件职责清晰、独立可测，共同构成了一个完整的流处理生态系统。本库创造性地融合了多种编程范式的精华：

- **Java Stream API 的优雅与流畅**：链式调用，声明式编程，让代码如诗般优雅 ✨
- **JavaScript Generator 的惰性与灵活**：延迟计算，按需生成，内存友好 🌱
- **数据库索引的高效与有序**：智能排序，索引驱动，时序数据处理利器 ⏱️
- **容器即元素的批处理哲学**：向量、链表、映射……任何容器都能作为流中一等公民，自由流动 📦

你是否厌倦了手写 for 循环遍历 vector，再嵌套一个 if 过滤，再手动 push_back 到另一个容器？😩
你是否在深夜调试过索引偏移一格的 Bug，只因为想在反向遍历时取“倒数第三个”元素？😵💫
你是否渴望像操作数据库一样——按索引精确定位、按窗口滑动分析、用一行链式调用完成从数据到统计的完整旅程？🤔

**Semantic-Cpp 正是为此而生。🔧**

它将数据处理抽象为对“元素”及其“逻辑位置（索引）”的操作——就像数据库中的“行”和“主键”，你可以在不触碰数据本身的情况下，随意重排、偏移、反转索引；也可以把任何一个容器（vector、map、array……）当作一个不可分割的整体在流中传递，然后随时“拆包”回元素级别。这种双粒度自由切换的能力，是传统流式框架所不具备的。🎯

---

## 🏗️ 项目架构：七层模块化设计

Semantic-Cpp 由七个核心头文件组成，层层递进，每个文件职责单一、独立可测。五个命名空间各司其职，共同构成了从数据源头到最终结果的完整流水线：

```
┌─────────────────────────────────────────────────┐
│               🌊 semantics.h                     │
│   命名空间: semantic                             │
│   流构建工厂：数值范围、容器、文本、Unicode         │
├─────────────────────────────────────────────────┤
│                 📦 semantic.h                     │
│   命名空间: semantic / collectable               │
│   流式中介操作、Collectable体系、容器展开支持      │
├─────────────────────────────────────────────────┤
│                ⚙️ collector.h                     │
│   命名空间: collector                            │
│   收集器框架 + 工厂：匹配、查找、聚合、统计、DFT/FFT│
├─────────────────────────────────────────────────┤
│                🔤 charsequence.h                   │
│   命名空间: charsequence                         │
│   Unicode字符序列、多编码转换、Builder、Buffer     │
├─────────────────────────────────────────────────┤
│                🧵 pool.h                          │
│   命名空间: pool                                 │
│   全局线程池：任务提交、紧急关闭、异常传播            │
├─────────────────────────────────────────────────┤
│                📄 function.h                      │
│   命名空间: function                             │
│   类型定义：Generator、Supplier、Consumer等别名     │
├─────────────────────────────────────────────────┤
│               🔐 hash.h / less.h                  │
│   命名空间: std (扩展)                            │
│   标准库容器哈希与比较特化，支持任意嵌套             │
└─────────────────────────────────────────────────┘
```

### 🧩 依赖关系
依赖链路清晰明了，如同精心设计的电路图：电流从最底层的类型定义开始，逐级向上，每一层都只依赖于它脚下的层级。最终，所有线路汇聚到 `semantic.h` 和 `semantics.h`，形成完整的流处理能力。

```
function.h          ← 无依赖，类型基石
pool.h              ← 依赖 function.h
charsequence.h      ← 独立模块，Unicode 处理
collector.h         ← 依赖 function.h、pool.h
hash.h / less.h     ← 独立模块，标准库扩展
semantic.h          ← 依赖以上全部
semantics.h         ← 依赖 semantic.h
```

---

## 🌍 命名空间全景

Semantic-Cpp 精心设计了五个命名空间，每个都像一个独立的“部门”，各司其职又紧密协作：

| 命名空间    | 所在文件         | 职责                                   | 核心类型/函数                                                                  |
| :---------- | :--------------- | :------------------------------------- | :----------------------------------------------------------------------------- |
| function    | function.h       | 类型系统基石                           | `Timestamp`、`Module`、`Generator<T>`、`Supplier<R>`、`Consumer<T>`、`Predicate<T>` 等 |
| pool        | pool.h           | 并发执行引擎                           | `pool::pool`（全局线程池）、`submit()`、`emergencyShutdown()`                    |
| charsequence| charsequence.h   | Unicode 字符串处理                     | `charset`、`Meta`、`Point`、`Charsequence`、`Builder`、`Buffer` 等                |
| collector   | collector.h      | 终端收集执行                           | `Collector<E,A,R>`、`Identity<A>`、`Accumulator<A,E>` 等                        |
| collectable | semantic.h       | 物化数据容器                           | `Collectable<E>`、`OrderedCollectable<E>`、`UnorderedCollectable<E>` 等         |
| semantic    | semantic.h<br>semantics.h | 流构建与中间操作                 | `Semantic<E>`、`useRange()`、`useFrom()` 等                                      |

### 🔁 命名空间协作流程
数据在命名空间之间的流转，就像工厂里的流水线——原料从 `semantic` 进入，经过层层加工，最终在 `collector` 包装出厂。每一步都有明确的职责边界：

```cpp
semantic::useRange(0, 100)          // ← semantic 命名空间：创建流
    .map(int x { return x * 2; })   // ← semantic 命名空间：中间变换
    .filter(int x { return x > 50; }) // ← semantic 命名空间：中间过滤
    .toUnordered()                  // ← 转换为 collectable 命名空间
    .toVector();                    // ← 调用 collector 命名空间的收集器
```

---

## 📦 第一层：function.h — 类型基础

`function.h` 定义了整个框架的类型系统，是所有模块的共同基石。🔑

```cpp
namespace function {
    using Timestamp = long long;           // 索引类型，数据在流中的“时间戳”
    using Module = unsigned long long;     // 模块/计数类型
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept — 接收一个元素
        std::function<bool(T, Timestamp)>       // interrupt — 是否中断？
    )>;
}
```

`Generator` 是整个流系统的核心抽象。🌀 它不返回数据，而是接受两个回调——`accept`（“我准备好了，请接收这个元素”）和 `interrupt`（“需要停下来吗？”）。这种控制反转的设计让数据生产者完全不知道消费者是谁，只需在合适的时机“推送”数据。这正是惰性求值的精髓：数据只在 `accept` 被调用时才真正“流动”，在此之前一切都只是描述。

| 类型别名          | 完整定义                                     | 用途                         |
| :---------------- | :------------------------------------------- | :--------------------------- |
| Timestamp         | long long                                    | 元素在流中的逻辑位置           |
| Module            | unsigned long long                           | 计数、容量、并发度             |
| Runnable          | std::function<void()>                        | 无参无返回值任务               |
| Supplier<R>       | std::function<R()>                           | 供应者，无中生有               |
| Function<T,R>     | std::function<R(T)>                          | 单参数函数                     |
| BiFunction<T,U,R> | std::function<R(T,U)>                        | 双参数函数                     |
| TriFunction<T,U,V,R> | std::function<R(T,U,V)>                   | 三参数函数                     |
| Unary<T>          | std::function<T(T)>                          | 一元运算                       |
| Binary<T>         | std::function<T(T,T)>                        | 二元运算                       |
| Consumer<T>       | std::function<void(T)>                       | 消费者                         |
| BiConsumer<T,U>   | std::function<void(T,U)>                     | 双参数消费者                   |
| TriConsumer<T,U,V>| std::function<void(T,U,V)>                   | 三参数消费者                   |
| Predicate<T>      | std::function<bool(T)>                       | 谓词判断                       |
| BiPredicate<T,U>  | std::function<bool(T,U)>                     | 双参数谓词                     |
| TriPredicate<T,U,V> | std::function<bool(T,U,V)>                 | 三参数谓词                     |
| Comparator<T>     | std::function<int(const T&,const T&)>        | 比较器，返回负/零/正           |
| Generator<T>      | BiConsumer<BiConsumer<T,Timestamp>,<br>BiPredicate<T,Timestamp>> | 流生成器核心抽象      |

---

## 🧵 第二层：pool.h — 并发基石

`pool.h` 提供全局线程池 `pool::pool`，是整个框架的并发引擎。🚀 它采用**声明式并行**设计——当你写下 `.parallel(4)` 时，并没有立即启动 4 个线程开始处理。这行代码仅仅是一个“声明”：告诉框架“我打算用 4 个线程来并行处理”。真正的并行执行发生在终端操作被调用时——也就是当你调用 `toVector()`、`findFirst()`、`count()` 等收集方法的那一刻。

| 特性         | 说明                                                   |
| :----------- | :----------------------------------------------------- |
| 声明式并行   | `.parallel(4)` 只声明“我想用 4 个线程”，不立即启动     |
| 紧急关闭     | 内置 `emergencyShutdown()` 和 `std::set_terminate` 处理器 |
| 异常传播     | `submit()` 返回 `std::future`，异常安全地传播到主线程  |

---

## 🔤 第三层：charsequence.h — Unicode 字符序列

`charsequence.h` 是一个完整的 Unicode 处理模块，提供字符序列的创建、转换和操作功能。🌍 它支持 UTF-8、UTF-16（LE/BE）、UTF-32（LE/BE）、ASCII 和 Latin1 等多种编码，能够正确检测和处理代理对，对无效码点返回标准的 U+FFFD 替代字符。

| 类型/函数       | 描述                                                                     |
| :-------------- | :----------------------------------------------------------------------- |
| charset         | 枚举 ascii、utf8、utf16、utf16be、utf16le、utf32、utf32be、utf32le、latin1 |
| Meta            | 元数据包装器，存储无符号整数值                                           |
| Point           | Unicode 码点，支持代理对检测和有效性验证                                 |
| Charsequence    | 不可变字符序列：split、replace、indexOf、lastIndexOf、sub、trim、toUpperCase、toLowerCase、reverse、startsWith、endsWith、contains、compare、getBytes、getPoints、getMetas、getCharacters、repeat、concat、count、join |
| Builder         | 可变字节构建器：prepend、insert、append（支持基本类型、Point、Charsequence、string_view） |
| Buffer          | 线程安全环形缓冲区：write、read、peek、prepend、append、clear、shrinkToFit、data、size、capacity、atomic |
| PointIterator   | 双向迭代器，遍历 Unicode 码点                                            |
| encode()        | 将单个码点编码为指定编码的字节序列                                       |
| decode()        | 从字节序列解码下一个码点，自动推进指针                                   |
| convert()       | 编码转换（支持 string、vector、deque 输出）                               |

---

## ⚙️ 第四层：collector.h — 收集器框架与工厂

`collector.h` 是 Semantic-Cpp 的收集器核心模块，将收集器框架与工厂函数合二为一。

### 🧩 五阶段模型
```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (可选短路)
```

| 类型别名         | 完整定义                                     | 角色                       |
| :--------------- | :------------------------------------------- | :------------------------- |
| Identity<A>      | function::Supplier<A>                        | 提供初始值                 |
| Accumulator<A,E> | function::TriFunction<A, E, Timestamp, A>    | 累加元素                   |
| Combiner<A>      | function::BiFunction<A, A, A>                | 合并并行结果               |
| Finisher<A,R>    | function::Function<A, R>                     | 最终转换                   |
| Interrupt<E,A>   | function::TriPredicate<E, Timestamp, A>      | 短路判断                   |

### 收集器工厂函数

#### ✅ 匹配操作
| 方法                     | 描述               | 返回类型        |
| :----------------------- | :----------------- | :-------------- |
| `useAllMatch(predicate)` | 所有元素满足条件   | `bool`          |
| `useAnyMatch(predicate)` | 任意元素满足条件   | `bool`          |
| `useNoneMatch(predicate)`| 没有元素满足条件   | `bool`          |

#### 🔍 查找操作
| 方法               | 描述                     | 返回类型         |
| :----------------- | :----------------------- | :--------------- |
| `useFindFirst()`   | 查找第一个元素           | `std::optional<E>` |
| `useFindLast()`    | 查找最后一个元素         | `std::optional<E>` |
| `useFindAny()`     | 随机查找元素             | `std::optional<E>` |
| `useFindAt(index)` | 精确定位（支持负索引）   | `std::optional<E>` |
| `useFindMaximum()` | 查找最大值               | `std::optional<E>` |
| `useFindMinimum()` | 查找最小值               | `std::optional<E>` |

#### 🔢 聚合操作
| 方法            | 描述     | 返回类型 |
| :-------------- | :------- | :------- |
| `useCount()`    | 元素总数 | `Module` |
| `useSummate<E,D>()` | 求和     | `D`      |
| `useAverage<E,D>()` | 平均值   | `D`      |
| `useRange<E,D>()`   | 数值范围 | `D`      |

#### 📉 统计操作
| 方法                          | 描述             | 返回类型         |
| :---------------------------- | :--------------- | :--------------- |
| `useVariance<E,D>()`          | 总体方差         | `D`              |
| `useStandardDeviation<E,D>()` | 总体标准差       | `D`              |
| `useSkewness<E,D>()`          | 偏度             | `D`              |
| `useKurtosis<E,D>()`          | 峰度             | `D`              |
| `useMedian<E,D>()`            | 中位数           | `std::optional<D>` |
| `useMode<E>()`                | 众数（频域分析） | `std::optional<E>` |
| `usePercentile<E,D>(p)`       | 第 p 百分位数    | `std::optional<D>` |
| `useFrequency<E>()`           | 频域特征         | `std::map<E, complex>` |
| `useDistribution<E>()`       | 空间分布特征     | `std::map<E, complex>` |

#### 🔀 归约操作
| 方法                                      | 描述             | 返回类型         |
| :---------------------------------------- | :--------------- | :--------------- |
| `useReduce(reducer)`                      | 无初始值归约     | `std::optional<E>` |
| `useReduce(identity, reducer)`            | 带初始值归约     | `E`              |
| `useReduce(id, red, comb, fin)`           | 完全自定义       | `R`              |

#### 🧺 收集到容器
| 方法                                                             | 返回类型 |
| :--------------------------------------------------------------- | :------- |
| `useToVector()`                                                  | `std::vector<E>` |
| `useToList()`                                                    | `std::list<E>` |
| `useToDeque()`                                                   | `std::deque<E>` |
| `useToForwardList()`                                             | `std::forward_list<E>` |
| `useToArray<N>()`                                                | `std::array<E, N>` |
| `useToSet()`                                                     | `std::set<E>` |
| `useToMultiset()`                                                | `std::multiset<E>` |
| `useToUnorderedSet()`                                            | `std::unordered_set<E>` |
| `useToUnorderedMultiset()`                                       | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)`                                         | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)`                         | `std::map<K, V>` |
| `useToMultimap(keyExtractor)`                                    | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)`                   | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)`                | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)`                           | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)`            | `std::unordered_multimap<K, V>` |
| `useToStack()`                                                   | `std::stack<E>` |
| `useToQueue()`                                                   | `std::queue<E>` |
| `useToPriorityQueue()`                                           | `std::priority_queue<E>` |

#### 🧩 分组与分区
| 方法                                              | 返回类型 |
| :------------------------------------------------ | :------- |
| `useGroup(keyExtractor)`                          | `std::unordered_map<K, vector<E>>` |
| `useGroupBy(keyExtractor, valueExtractor)`       | `std::unordered_map<K, vector<V>>` |
| `usePartition(size)`                              | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)`                    | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor, valueExtractor)`    | `std::vector<vector<V>>` |

#### 📄 字符串输出
| 方法                               | 返回类型 |
| :--------------------------------- | :------- |
| `useJoin()` / `useOut()` / `useError()` 及其各种重载 | `charsequence::Charsequence` |

#### 📊 数学工具
| 方法             | 返回类型 |
| :--------------- | :------- |
| `useDFT()`       | `vector<complex<double>>` |
| `useIDFT()`      | `vector<complex<double>>` |
| `useFFT()`       | `vector<complex<double>>` |
| `useIFFT()`      | `vector<complex<double>>` |
| `useGradient()`  | `vector<double>` |

---

## 📦 第五层：semantic.h — 流式中间操作与收集体系

### 🧩 核心设计：三阶段流水线
```
Semantic<E> (构建与变换)
    ↓ toUnordered() / toOrdered() / sort() / toWindow() / toStatistics()
Collectable<E> (物化与收集)
    ↓ toVector() / findFirst() / count() / summate() / ...
最终结果
```

**关键规则**：必须先通过 `toUnordered()`、`toOrdered()`、`toWindow()`、`toStatistics()` 或 `sort()` 将 `Semantic<E>` 转换为 `Collectable<E>`，然后才能调用终端方法。

### 🧭 五种物化路径
| 转换方法           | 目标类型              | 底层数据结构     | 性能特征             |
| :----------------- | :-------------------- | :--------------- | :------------------- |
| `toUnordered()`    | `UnorderedCollectable` | `unordered_map`  | 平均 O(1) 查找      |
| `toOrdered()`      | `OrderedCollectable`  | `map`            | O(log n) 查找       |
| `sort()`           | `OrderedCollectable`  | `map`（按值排序）| O(log n) 查找       |
| `toWindow()`       | `WindowCollectable`   | 继承有序集合     | 支持 slide/tumble   |
| `toStatistics<D>()`| `Statistics<E,D>`     | 继承有序集合     | 30+ 统计方法        |

### 📋 Collectable<E> — 全部终端方法（按字母排序）
| 方法                                                | 返回类型                     | 描述                                     |
| :-------------------------------------------------- | :--------------------------- | :--------------------------------------- |
| `allMatch(predicate)`                               | `bool`                       | 所有元素满足条件                         |
| `anyMatch(predicate)`                               | `bool`                       | 任意元素满足条件                         |
| `average<D>()`                                      | `D`                          | 平均值                                   |
| `average<D>(mapper)`                                | `D`                          | 映射后求平均值                           |
| `collect(identity, acc, comb, fin)`                | `R`                          | 自定义四阶段收集                         |
| `collect(identity, interrupt, acc, comb, fin)`     | `R`                          | 自定义可中断收集                         |
| `count()`                                           | `Module`                     | 元素总数                                 |
| `empty()`                                           | `bool`                       | 流是否为空                               |
| `error()`                                           | `void`                       | 输出到 stderr（支持 delimiter/prefix/suffix/converter） |
| `findAny()`                                         | `std::optional<E>`           | 随机查找元素                             |
| `findAt(index)`                                     | `std::optional<E>`           | 指定索引查找（支持负索引）               |
| `findFirst()`                                       | `std::optional<E>`           | 查找第一个元素                           |
| `findLast()`                                        | `std::optional<E>`           | 查找最后一个元素                         |
| `findMaximum()`                                     | `std::optional<E>`           | 查找最大值                               |
| `findMaximum(comparator)`                           | `std::optional<E>`           | 自定义比较查找最大值                     |
| `findMinimum()`                                     | `std::optional<E>`           | 查找最小值                               |
| `findMinimum(comparator)`                           | `std::optional<E>`           | 自定义比较查找最小值                     |
| `forEach(consumer)`                                 | `void`                      | 遍历每个元素执行副作用                   |
| `group(keyExtractor)`                               | `unordered_map<K, vector<E>>`| 按键分组                                 |
| `groupBy(keyExtractor, valueExtractor)`            | `unordered_map<K, vector<V>>`| 按键分组并提取值                         |
| `join()`                                            | `Charsequence`               | 默认格式连接                             |
| `join(delimiter)`                                   | `Charsequence`               | 自定义分隔符连接                         |
| `join(prefix, delimiter, suffix)`                  | `Charsequence`               | 完全自定义格式连接                       |
| `noneMatch(predicate)`                              | `bool`                       | 没有元素满足条件                         |
| `out()`                                             | `Charsequence`               | 输出到 stdout（支持 delimiter/prefix/suffix/converter） |
| `partition(size)`                                   | `vector<vector<E>>`          | 按固定大小分区                           |
| `partitionBy(keyExtractor)`                        | `vector<vector<E>>`          | 按索引键分区                             |
| `partitionBy(keyExtractor, valueExtractor)`        | `vector<vector<V>>`          | 按索引键分区并提取值                     |
| `range<D>()`                                        | `D`                          | 数值范围（最大值减最小值）               |
| `range<D>(mapper)`                                  | `D`                          | 映射后求数值范围                         |
| `reduce(accumulator)`                               | `std::optional<E>`           | 无初始值归约                             |
| `reduce(identity, accumulator)`                     | `E`                          | 带初始值归约                             |
| `reduce(identity, acc, comb)`                       | `R`                          | 完全自定义归约                           |
| `summate<D>()`                                      | `D`                          | 求和                                     |
| `summate<D>(mapper)`                                | `D`                          | 映射后求和                               |
| `toArray<N>()`                                      | `std::array<E, N>`           | 收集为固定大小 array                     |
| `toDeque()`                                         | `std::deque<E>`              | 收集为 deque                             |
| `toForwardList()`                                   | `std::forward_list<E>`       | 收集为 forward_list                      |
| `toList()`                                          | `std::list<E>`               | 收集为 list                              |
| `toMap(keyExtractor)`                               | `std::map<K, E>`             | 按键收集为 map                           |
| `toMap(keyExtractor, valueExtractor)`               | `std::map<K, V>`             | 自定义键值收集为 map                     |
| `toMultimap(keyExtractor)`                          | `std::multimap<K, E>`        | 按键收集为 multimap                      |
| `toMultimap(keyExtractor, valueExtractor)`           | `std::multimap<K, V>`        | 自定义键值收集为 multimap                |
| `toMultiset()`                                      | `std::multiset<E>`           | 收集为 multiset                          |
| `toPriorityQueue()`                                 | `std::priority_queue<E>`     | 收集为 priority_queue                    |
| `toQueue()`                                         | `std::queue<E>`              | 收集为 queue                             |
| `toSet()`                                           | `std::set<E>`                | 收集为 set（去重排序）                   |
| `toStack()`                                         | `std::stack<E>`              | 收集为 stack                             |
| `toUnorderedMap(keyExtractor, valueExtractor)`      | `std::unordered_map<K, V>`   | 收集为 unordered_map                     |
| `toUnorderedMultimap(keyExtractor)`                  | `std::unordered_multimap<K, E>` | 按键收集为 unordered_multimap         |
| `toUnorderedMultimap(keyExtractor, valueExtractor)`  | `std::unordered_multimap<K, V>` | 自定义键值收集为 unordered_multimap   |
| `toUnorderedMultiset()`                             | `std::unordered_multiset<E>` | 收集为 unordered_multiset                |
| `toUnorderedSet()`                                  | `std::unordered_set<E>`      | 收集为 unordered_set                     |
| `toVector()`                                        | `std::vector<E>`             | 收集为 vector                            |

### 📈 Statistics<E,D> — 统计方法
| 方法                 | 返回类型             | 描述                       |
| :------------------- | :------------------- | :------------------------- |
| `summate()`          | `D`                  | 求和                       |
| `average()`          | `D`                  | 平均值                     |
| `minimum()`          | `std::optional<D>`   | 最小值                     |
| `maximum()`          | `std::optional<D>`   | 最大值                     |
| `range()`            | `D`                  | 范围                       |
| `variance()`         | `D`                  | 总体方差                   |
| `standardDeviation()`| `D`                  | 总体标准差                 |
| `median()`           | `std::optional<D>`   | 中位数                     |
| `mode()`             | `std::optional<E>`   | 众数                       |
| `percentile(p)`      | `std::optional<D>`   | 第 p 百分位数              |
| `firstQuartile()`    | `std::optional<D>`   | 第一四分位数 (Q1)          |
| `thirdQuartile()`    | `std::optional<D>`   | 第三四分位数 (Q3)          |
| `interquartileRange()`| `std::optional<D>`  | 四分位距 (IQR)            |
| `skewness()`         | `D`                  | 偏度                       |
| `kurtosis()`         | `D`                  | 峰度                       |
| `frequency()`        | `map<E, complex>`    | 频域特征                   |
| `distribute()`       | `map<E, complex>`    | 空间分布特征               |
| `dft()`              | `vector<complex<double>>` | 离散傅里叶变换       |
| `idft()`             | `vector<complex<double>>` | 逆离散傅里叶变换     |
| `fft()`              | `vector<complex<double>>` | 快速傅里叶变换       |
| `ifft()`             | `vector<complex<double>>` | 逆快速傅里叶变换     |
| `gradient(...)`      | `vector<double>`     | 梯度下降                   |

以上方法均支持可选的 mapper 参数版本。

### 🔧 Semantic<E> 中间操作方法
| 类别       | 方法        | 描述                                   |
| :--------- | :---------- | :------------------------------------- |
| 元素变换   | map         | 一对一映射转换                         |
|            | flatMap     | 一对多映射并展平                       |
|            | flat        | 展平嵌套流（支持Semantic和容器）       |
| 元素过滤   | filter      | 条件过滤                               |
|            | takeWhile   | 条件满足时持续获取                     |
|            | dropWhile   | 条件满足时持续丢弃                     |
|            | distinct    | 去重（支持自定义比较器）               |
| 数量控制   | limit       | 限制元素数量                           |
|            | skip        | 跳过前 n 个元素                        |
|            | sub         | 截取子范围 [start, end)                |
| 索引操作   | redirect    | 重新映射索引                           |
|            | reverse     | 反转索引                               |
|            | translate   | 偏移索引                               |
| 观察操作   | peek        | 观察每个元素（不修改流）               |
| 并行声明   | parallel(n) | 声明并行度                             |
| 连接操作   | concatenate | 连接 Semantic/元素/生成器/容器         |
| 终端转换   | toUnordered / toOrdered / toWindow / toStatistics / sort | 转换为 Collectable |

---

## 🔧 第六层：semantics.h — 流构建工厂

### 🔢 数值范围生成
| 方法                          | 描述                     |
| :---------------------------- | :----------------------- |
| `useRange(start, end)`        | 生成 [start, end) 范围   |
| `useRange(start, end, step)`  | 带步长范围，支持负步长   |
| `useRangeClosed(start, end)`  | 生成 [start, end] 闭区间 |
| `useRangeClosed(start, end, step)` | 带步长闭区间         |

### ♾️ 无限流生成
| 方法                        | 描述                           |
| :-------------------------- | :----------------------------- |
| `useInfinite(seed, generator)` | 从种子值无限迭代             |
| `useGenerate(supplier)`     | 无限调用供应者                 |
| `useGenerate(supplier, limit)` | 有限次数调用供应者           |
| `useIterate(seed, generator)` | 从种子值无限迭代             |
| `useIterate(seed, generator, limit)` | 有限次数迭代             |
| `useRandom()`               | 无限随机整数流                 |
| `useRandom(min, max)`       | 指定范围随机数流               |
| `useRandom(min, max, count)`| 指定范围和数量随机数流         |

### 📦 容器与元素构建
| 方法                    | 描述                     |
| :---------------------- | :----------------------- |
| `useEmpty()`            | 创建空流                 |
| `useOf(element)`        | 从单个元素创建流         |
| `useOf(e1, e2)`         | 从两个元素创建流         |
| `useOf(e1, e2, e3)`     | 从三个元素创建流         |
| `useOf({...})`          | 从初始化列表创建流       |
| `useFrom(container)`    | 从标准容器创建流         |
| `useFrom({...})`        | 从初始化列表创建流       |
| `useRepeat(element, count)` | 重复元素 n 次        |

### 📄 文本与 Unicode 处理
| 方法                        | 描述                         |
| :-------------------------- | :--------------------------- |
| `useBlob(text)`             | 字符串按字节拆分为 char 流   |
| `useBlob(text, start, end)` | 指定范围按字节拆分           |
| `useBlob(istream)`          | 从输入流按行读取             |
| `useBlob(istream, delimiter)` | 从输入流按分隔符读取       |
| `useText(text)`             | 整体文本流（Charsequence）   |
| `useText(text, delimiter)`  | 按分隔符拆分文本             |
| `useText(istream)`          | 从输入流读取整个内容         |
| `useSequence(charsequence)` | 从字符序列创建码点流         |
| `useSequence(text, encoding)` | 从文本创建指定编码码点流   |
| `useCharsequence(charsequence)` | 字符序列作为整体流       |
| `useCharsequence(charsequence, delimiter)` | 按分隔符拆分字符序列 |

---

## 🔐 第七层：hash.h / less.h — 容器世界的通用语言

为所有标准库容器（包括嵌套容器）、pair、tuple、optional、variant、chrono 时间类型、complex 复数等提供完整的哈希和比较支持。任意深度、任意组合的嵌套容器现在都可以作为 unordered_set 的键或 set 的元素。🌉

---

## 🚀 性能优化建议

1.  **选择对的容器**：不关心顺序用 `toUnordered()`，需要排序用 `toOrdered()` 或 `sort()`
2.  **善用并行**：数据量大时使用 `parallel()`
3.  **优化操作顺序**：尽早 filter，明智 sort
4.  **利用惰性求值**：`takeWhile` 和 `limit` 可提前终止

---

Semantic-Cpp — 用现代 C++ 构建高效、清晰的数据处理管道。🚀🎯✨
