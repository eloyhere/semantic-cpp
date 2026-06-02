# 🚀 Semantic-Cpp：面向未来的 C++ 智能流处理框架

Semantic-Cpp 是一个完全重构设计的现代化 C++ 流处理库，采用 **“多头文件、零外部依赖”** 的模块化架构。每个头文件职责清晰、独立可测，共同构成了一个完整的流处理生态系统。本库创造性地融合了多种编程范式的精华：

- **Java Stream API 的优雅与流畅**：链式调用，声明式编程，让代码如诗般优雅 ✨
- **JavaScript Generator 的惰性与灵活**：延迟计算，按需生成，内存友好 🌱
- **数据库索引的高效与有序**：智能排序，索引驱动，时序数据处理利器 ⏱️
- **容器即元素的批处理哲学**：向量、链表、映射……任何容器都能作为流中一等公民，自由流动 📦

你是否厌倦了手写 for 循环遍历 vector，再嵌套一个 if 过滤，再手动 push_back 到另一个容器？😩  
你是否在深夜调试过索引偏移一格的 Bug，只因为想在反向遍历时取“倒数第三个”元素？😵💫  
你是否渴望像操作数据库一样——按索引精确定位、按窗口滑动分析、用一行链式调用完成从数据到统计的完整旅程？🤔

Semantic-Cpp 正是为此而生。🔧  
它将数据处理抽象为对 **“元素”** 及其 **“逻辑位置（索引）”** 的操作——就像数据库中的“行”和“主键”，你可以在不触碰数据本身的情况下，随意重排、偏移、反转索引；也可以把任何一个容器（vector、map、array……）当作一个不可分割的整体在流中传递，然后随时“拆包”回元素级别。这种双粒度自由切换的能力，是传统流式框架所不具备的。🎯

---

## 🏗️ 项目架构：八层模块化设计

Semantic-Cpp 由 **八个核心头文件** 组成，层层递进，每个文件职责单一、独立可测。五个命名空间各司其职，共同构成了从数据源头到最终结果的完整流水线：

```
┌─────────────────────────────────────────────────┐
│               🔧 semantics.h                     │
│   命名空间: semantic                             │
│   流构建工厂：数值范围、容器、文本、Unicode         │
├─────────────────────────────────────────────────┤
│                 📦 semantic.h                     │
│   命名空间: semantic / collectable               │
│   流式中介操作、Collectable体系、10种容器特化      │
├─────────────────────────────────────────────────┤
│                📊 collectors.h                    │
│   命名空间: collector                            │
│   收集器工厂：匹配、查找、聚合、统计、DFT/FFT等     │
├─────────────────────────────────────────────────┤
│                ⚙️ collector.h                     │
│   命名空间: collector                            │
│   收集器框架：五阶段模型、并发并行支持              │
├─────────────────────────────────────────────────┤
│                🔤 charsequence.h                   │
│   命名空间: charsequence                         │
│   Unicode字符序列、多编码转换、Builder、Buffer     │
├─────────────────────────────────────────────────┤
│                🧵 pool.h                        │
│   命名空间: pool                                 │
│   全局线程池：任务提交、紧急关闭、异常传播            │
├─────────────────────────────────────────────────┤
│                📄 function.h                      │
│   命名空间: function                             │
│   类型定义：Generator、Supplier、Consumer等别名     │
├─────────────────────────────────────────────────┤
│               🔐 hash.h / less.h                   │
│   命名空间: std (扩展)                            │
│   标准库容器哈希与比较特化，支持任意嵌套             │
└─────────────────────────────────────────────────┘
```

### 🧩 依赖关系

```
function.h          ← 无依赖，类型基石
pool.h              ← 依赖 function.h
charsequence.h      ← 独立模块，Unicode 处理
collector.h         ← 依赖 function.h、pool.h
collectors.h        ← 依赖 collector.h、charsequence.h
hash.h / less.h     ← 独立模块，标准库扩展
semantic.h          ← 依赖以上全部
semantics.h         ← 依赖 semantic.h
```

---

## 🌍 命名空间全景

Semantic-Cpp 精心设计了五个命名空间，每个都像一个独立的“部门”，各司其职又紧密协作：

| 命名空间     | 所在文件         | 职责                     | 核心类型/函数                                                             |
|--------------|------------------|--------------------------|--------------------------------------------------------------------------|
| `function`   | function.h       | 类型系统基石             | Timestamp、Module、Generator<T>、Supplier<R>、Consumer<T>、Predicate<T> 等 |
| `pool`       | pool.h           | 并发执行引擎             | pool::pool（全局线程池）、submit()、emergencyShutdown()                  |
| `charsequence` | charsequence.h | Unicode 字符串处理       | charset、Meta、Point、Charsequence、Builder、Buffer 等                   |
| `collector`  | collector.h + collectors.h | 终端收集执行      | Collector<E,A,R>、Identity<A>、Accumulator<A,E> 等                       |
| `collectable` | semantic.h       | 物化数据容器             | Collectable<E>、OrderedCollectable<E>、UnorderedCollectable<E> 等         |
| `semantic`   | semantic.h + semantics.h | 流构建与中间操作  | Semantic<E>、useRange()、useFrom() 等                                     |

### 🔁 命名空间协作流程

```cpp
semantic::useRange(0, 100)          // ← semantic 命名空间：创建流
    .map(int x { ... })         // ← semantic 命名空间：中间变换
    .filter(int x { ... })      // ← semantic 命名空间：中间过滤
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

Generator 是整个流系统的核心抽象。🌀 它不返回数据，而是接受两个回调——accept（“我准备好了，请接收这个元素”）和 interrupt（“需要停下来吗？”）。这种控制反转的设计让数据生产者完全不知道消费者是谁，只需在合适的时机“推送”数据。这正是惰性求值的精髓：数据只在 accept 被调用时才真正“流动”，在此之前一切都只是描述。

| 类型别名         | 完整定义                               | 用途                     |
|------------------|----------------------------------------|--------------------------|
| Timestamp        | long long                              | 元素在流中的逻辑位置     |
| Module           | unsigned long long                     | 计数、容量、并发度       |
| Runnable         | std::function<void()>                  | 无参无返回值任务         |
| Supplier<R>      | std::function<R()>                     | 供应者，无中生有         |
| Function<T,R>    | std::function<R(T)>                    | 单参数函数               |
| BiFunction<T,U,R>| std::function<R(T,U)>                  | 双参数函数               |
| TriFunction<T,U,V,R>| std::function<R(T,U,V)>            | 三参数函数               |
| Unary<T>         | std::function<T(T)>                    | 一元运算                 |
| Binary<T>        | std::function<T(T,T)>                  | 二元运算                 |
| Consumer<T>      | std::function<void(T)>                 | 消费者                   |
| BiConsumer<T,U>  | std::function<void(T,U)>               | 双参数消费者             |
| TriConsumer<T,U,V>| std::function<void(T,U,V)>             | 三参数消费者             |
| Predicate<T>     | std::function<bool(T)>                 | 谓词判断                 |
| BiPredicate<T,U> | std::function<bool(T,U)>               | 双参数谓词               |
| TriPredicate<T,U,V>| std::function<bool(T,U,V)>           | 三参数谓词               |
| Comparator<T>    | std::function<int(const T&,const T&)>  | 比较器，返回负/零/正     |
| Generator<T>     | BiConsumer<BiConsumer<T,Timestamp>, BiPredicate<T,Timestamp>> | 流生成器核心抽象 |

---

## 🧵 第二层：pool.h — 并发基石

`pool.h` 提供全局线程池 `pool::pool`，是整个框架的并发引擎。🚀 它采用声明式并行设计：

| 特性         | 说明                                                          |
|--------------|---------------------------------------------------------------|
| 声明式并行   | `.parallel(4)` 只声明“我想用 4 个线程”，不立即启动             |
| 紧急关闭     | 内置 `emergencyShutdown()` 和 `std::set_terminate` 处理器      |
| 异常传播     | `submit()` 返回 `std::future`，异常安全地传播到主线程           |
| 成员                        | 类型             | 描述                             |
|-----------------------------|------------------|----------------------------------|
| `pool::pool`                | 全局线程池实例   | 程序级别的单例线程池，自动初始化 |
| `pool::pool.submit<A>(task)`| 方法             | 提交任务，返回 `std::future<A>`  |
| `pool::pool.emergencyShutdown()` | 方法       | 紧急关闭所有线程                 |

---

## 🔤 第三层：charsequence.h — Unicode 字符序列

`charsequence.h` 是一个完整的 Unicode 处理模块，提供字符序列的创建、转换和操作功能。🌍 它支持 UTF-8、UTF-16（LE/BE）、UTF-32（LE/BE）、ASCII 和 Latin1 等多种编码，能够正确检测和处理代理对，对无效码点返回标准的 U+FFFD 替代字符。

| 类型/函数       | 描述                                                                 |
|-----------------|----------------------------------------------------------------------|
| charset         | 枚举 ascii、utf8、utf16、utf16be、utf16le、utf32、utf32be、utf32le、latin1 |
| Meta            | 元数据包装器，存储无符号整数值                                       |
| Point           | Unicode 码点，支持代理对检测（`isSurrogate()`）和有效性验证（`isValidCodePoint()`） |
| Charsequence    | 不可变字符序列：split、replace、indexOf、lastIndexOf、sub、trim、toUpperCase、toLowerCase、reverse、startsWith、endsWith、contains、compare、getBytes、getPoints、getMetas、getCharacters、repeat、concat、count、join |
| Builder         | 可变字节构建器：prepend、insert、append（支持 bool、short、int、long、long long、float、double、long double 及对应的 unsigned 类型、char、unsigned char、Point、Charsequence、string_view） |
| Buffer          | 线程安全环形缓冲区：write、read、peek、prepend、append、clear、shrinkToFit、data、size、capacity、atomic（加锁批量操作） |
| PointIterator   | 双向迭代器，遍历 Unicode 码点                                       |
| sequenceLength()| 根据首字节判断 UTF-8 等多字节编码的序列长度                         |
| encode()        | 将单个码点编码为指定编码的字节序列                                   |
| decode()        | 从字节序列解码下一个码点，自动推进指针                               |
| convert()       | 编码转换（支持 string、vector<unsigned char>、deque<unsigned char> 输出） |

---

## ⚙️ 第四层：collector.h — 收集器框架

`collector.h` 实现了收集器模式，是终端操作的核心引擎。🔧

### 🧩 五阶段模型

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (可选短路)
```

这五个阶段像一条精密的流水线：首先准备好初始状态（Identity），然后逐个接收元素并更新中间结果（Accumulator），接着在并行场景下合并各线程的局部结果（Combiner），最后将中间结果转换为用户期望的最终形式（Finisher）。而 Interrupt 则像一位警觉的监工，随时可以在条件满足时喊停整个流程。🚨

| 类型别名         | 完整定义                            | 角色                     |
|------------------|-------------------------------------|--------------------------|
| Identity<A>      | function::Supplier<A>              | 提供初始值               |
| Accumulator<A,E> | function::TriFunction<A, E, Timestamp, A> | 累加元素             |
| Combiner<A>      | function::BiFunction<A, A, A>     | 合并并行结果             |
| Finisher<A,R>    | function::Function<A, R>           | 最终转换                 |
| Interrupt<E,A>   | function::TriPredicate<E, Timestamp, A> | 短路判断             |

### 🔧 框架函数

| 函数                                      | 描述                             |
|-------------------------------------------|----------------------------------|
| `useFull(identity, accumulator, combiner, finisher)` | 创建完整收集器（无短路）     |
| `useShortable(identity, interrupt, accumulator, combiner, finisher)` | 创建可短路收集器       |

### 🧱 `Collector<E,A,R>` 类

| 方法                          | 描述                               |
|-------------------------------|------------------------------------|
| `collect(generator, concurrent)` | 从 Generator 收集，支持并行       |
| `collect(container, concurrent)`  | 从标准容器收集，支持并行       |
| `collect(initializer_list, concurrent)` | 从初始化列表收集         |
| `collect(array, concurrent)`     | 从 std::array 收集             |
| `collect(deque, concurrent)`      | 从 std::deque 收集            |
| `collect(forward_list, concurrent)` | 从 std::forward_list 收集   |
| `collect(stack, concurrent)`      | 从 std::stack 收集            |
| `collect(queue, concurrent)`       | 从 std::queue 收集           |

### 🔀 并发支持

`Collector::collect()` 自动处理：数据分片（按索引模运算分发到各线程）、结果合并（通过 Combiner 归并局部结果）、异常传播（通过 `std::exception_ptr` 和 `std::atomic<bool>`）。🔀

---

## 📊 第五层：collectors.h — 收集器工厂

`collectors.h` 提供了丰富的预置收集器工厂函数。这里不是简单地“有几个工具函数”——这是一个完整的数据处理武器库，涵盖了从基础匹配到频域分析的全频谱操作。🔥

### ✅ 匹配操作

想知道流中所有元素是否满足某个条件？或者是否存在任何一个“叛逆者”？这三剑客瞬间给出答案，而且都支持短路求值——一旦确定答案就立即停止遍历。⚡

| 方法                  | 描述                                | 返回类型 |
|-----------------------|-------------------------------------|----------|
| `useAllMatch(predicate)` | 所有元素满足条件（全称量词）        | `bool`   |
| `useAnyMatch(predicate)` | 任意元素满足条件（存在量词）        | `bool`   |
| `useNoneMatch(predicate)`| 没有元素满足条件                    | `bool`   |

### 🔍 查找操作

从茫茫数据中找到你想要的那个“它”。负索引是 Semantic-Cpp 的独门绝技——`findAt(-1)` 直接拿到最后一个元素。🎯

| 方法                | 描述                                   | 返回类型         |
|---------------------|----------------------------------------|------------------|
| `useFindFirst()`    | 查找第一个元素                         | `std::optional<E>` |
| `useFindLast()`     | 查找最后一个元素                       | `std::optional<E>` |
| `useFindAny()`      | 随机查找元素                           | `std::optional<E>` |
| `useFindAt(index)`  | 精确定位：支持正索引和负索引           | `std::optional<E>` |
| `useFindMaximum()`  | 查找最大值（支持自定义比较器）         | `std::optional<E>` |
| `useFindMinimum()`  | 查找最小值（支持自定义比较器）         | `std::optional<E>` |

### 🔢 聚合操作

让数据说话，用数字总结一切。每个聚合方法都支持可选的 mapper 参数——先对每个元素做一个变换再聚合。📈

| 方法                | 描述                     | 返回类型         |
|---------------------|--------------------------|------------------|
| `useCount()`        | 元素总数                 | `Module`         |
| `useSummate<E,D>()` | 求和                     | `D`              |
| `useAverage<E,D>()` | 平均值                   | `D`              |
| `useRange<E,D>()`   | 数值范围（最大值减最小值）| `D`              |
| `useMinimum<E,D>()` | 最小值                   | `std::optional<D>` |
| `useMaximum<E,D>()` | 最大值                   | `std::optional<D>` |

### 📉 统计操作

从描述性统计到频域分析，统计学家的瑞士军刀。特别值得一提的是 `useMode()` 和 `useFrequency()`——它们使用索引相位编码技术，将每个元素出现的位置信息编码为复数平面上的角度，利用欧拉公式来捕捉数据中的周期性模式。🎼

| 方法                          | 描述                               | 返回类型         |
|-------------------------------|------------------------------------|------------------|
| `useVariance<E,D>()`         | 总体方差                           | `D`              |
| `useStandardDeviation<E,D>()`| 总体标准差                         | `D`              |
| `useSkewness<E,D>()`         | 偏度（分布的对称性）               | `D`              |
| `useKurtosis<E,D>()`         | 峰度（分布的尾部厚度）             | `D`              |
| `useMedian<E,D>()`           | 中位数                             | `std::optional<D>` |
| `useMode<E>()`               | 众数（基于频域分析）               | `std::optional<E>` |
| `usePercentile<E,D>(p)`      | 第 p 百分位数                      | `std::optional<D>` |
| `useFrequency<E>()`          | 频域特征（索引相位编码）           | `std::map<E, complex>` |
| `useDistribution<E>()`       | 空间分布特征（位置编码）           | `std::map<E, complex>` |

### 🔀 归约操作

归约（Reduce）是函数式编程中最强大的概念之一——它可以把一个流“折叠”成一个值。🎁

| 方法                          | 描述                                     | 返回类型         |
|-------------------------------|------------------------------------------|------------------|
| `useReduce(reducer)`          | 无初始值归约（流为空则返回 nullopt）     | `std::optional<E>` |
| `useReduce(identity, reducer)`| 带初始值归约                             | `E`              |
| `useReduce(id, red, comb, fin)` | 完全自定义：自定义累积、合并、收尾   | `R`              |

### 🧺 收集到容器操作

数据在流中漂泊了一生，最终需要“安家”。这里有 20+ 种标准库容器供你选择：🏡

| 方法                           | 描述                         | 返回类型               |
|--------------------------------|------------------------------|------------------------|
| `useToVector()`                | 收集为 vector，保持顺序      | `std::vector<E>`      |
| `useToList()`                  | 收集为 list                  | `std::list<E>`        |
| `useToDeque()`                 | 收集为 deque                 | `std::deque<E>`       |
| `useToForwardList()`           | 收集为 forward_list          | `std::forward_list<E>`|
| `useToArray<N>()`              | 收集为固定大小 array         | `std::array<E, N>`    |
| `useToSet()`                   | 收集为 set（去重排序）       | `std::set<E>`         |
| `useToMultiset()`              | 收集为 multiset              | `std::multiset<E>`    |
| `useToUnorderedSet()`          | 收集为 unordered_set         | `std::unordered_set<E>` |
| `useToUnorderedMultiset()`     | 收集为 unordered_multiset    | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)`       | 按键收集为 map               | `std::map<K, E>`      |
| `useToMap(keyExtractor, valueExtractor)` | 自定义键值收集为 map | `std::map<K, V>`      |
| `useToMultimap(keyExtractor)`  | 按键收集为 multimap          | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)` | 自定义键值收集为 multimap | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)` | 收集为 unordered_map | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)` | 按键收集为 unordered_multimap | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)` | 自定义键值收集为 unordered_multimap | `std::unordered_multimap<K, V>` |
| `useToStack()`                 | 收集为 stack                 | `std::stack<E>`       |
| `useToQueue()`                 | 收集为 queue                | `std::queue<E>`       |
| `useToPriorityQueue()`         | 收集为 priority_queue       | `std::priority_queue<E>` |

### 🧩 分组与分区操作

SQL 的 GROUP BY 在 C++ 中的等价物。`groupBy` 和 `partitionBy` 的二参数版本可以同时指定键提取器和值提取器——比如按部门分组员工，但只取员工姓名而非整个对象。👥

| 方法                                     | 描述                       | 返回类型                          |
|------------------------------------------|----------------------------|-----------------------------------|
| `useGroup(keyExtractor)`                 | 按键分组，保留完整元素     | `std::unordered_map<K, vector<E>>` |
| `useGroupBy(keyExtractor, valueExtractor)` | 按键分组，自定义值提取   | `std::unordered_map<K, vector<V>>` |
| `usePartition(size)`                     | 按固定大小分区             | `std::vector<vector<E>>`         |
| `usePartitionBy(keyExtractor)`           | 按索引键分区，保留完整元素 | `std::vector<vector<E>>`         |
| `usePartitionBy(keyExtractor, valueExtractor)` | 按索引键分区，自定义值提取 | `std::vector<vector<V>>` |

```cpp
// 按部门分组，只保留姓名
auto deptNames = people.toUnordered().groupBy(
    Person p { return p.department; },   // 键：部门
    Person p { return p.name; }          // 值：姓名
);
// 返回: unordered_map<string, vector<string>>
```

### 📄 字符串输出操作

`useJoin()`、`useOut()`、`useError()` 系列让你的数据以人类可读的格式呈现：📃

| 方法                               | 描述                 | 返回类型               |
|------------------------------------|----------------------|------------------------|
| `useJoin()`                        | 默认格式连接：`[elem1,elem2,...]` | `Charsequence`     |
| `useJoin(delimiter)`               | 自定义分隔符         | `Charsequence`         |
| `useJoin(prefix, delimiter, suffix)` | 完全自定义格式     | `Charsequence`         |
| `useOut()`                         | 输出到 stdout       | `Charsequence`         |
| `useOut(delimiter)`                | 带分隔符输出到 stdout | `Charsequence`       |
| `useOut(prefix, delimiter, suffix)` | 完全自定义输出到 stdout | `Charsequence`   |
| `useError()`                       | 输出到 stderr       | `Charsequence`         |
| `useError(delimiter)`              | 带分隔符输出到 stderr | `Charsequence`       |
| `useError(prefix, delimiter, suffix)` | 完全自定义输出到 stderr | `Charsequence` |

### 📊 数学工具

| 方法                                     | 描述                         | 返回类型                    |
|------------------------------------------|------------------------------|-----------------------------|
| `useDFT()`                               | 离散傅里叶变换               | `vector<complex<double>>`  |
| `useIDFT()`                              | 逆离散傅里叶变换             | `vector<complex<double>>`  |
| `useFFT()`                               | 快速傅里叶变换 (Cooley-Tukey) | `vector<complex<double>>`  |
| `useIFFT()`                              | 逆快速傅里叶变换             | `vector<complex<double>>`  |
| `useGradient(gradFunc, lr, iter, th)`    | 梯度下降（解析梯度）         | `vector<double>`           |
| `useGradient(costFunc, lr, iter, th, h)` | 梯度下降（数值梯度）         | `vector<double>`           |

---

## 📦 第六层：semantic.h — 流式中间操作与收集体系

---

### 🧩 核心设计：三阶段流水线 — `Semantic` → `Collectable` → `Collector`

很多流式框架将“中间操作”和“终端操作”混在同一类型中，让人摸不清什么时候数据真正被处理。Semantic-Cpp 采用了截然不同的三阶段职责分离设计，每一阶段各司其职，边界清晰：

```
┌──────────────────────────────────────────────────────────┐
│                 🌱 第一阶段：构建与变换                    │
│               Semantic<E>  (语义流)                     │
│                命名空间: semantic                        │
│                                                          │
│  · 惰性构建：useRange、useFrom、useGenerate...           │
│  · 中间变换：map、filter、takeWhile、distinct...         │
│  · 索引操作：reverse、translate、redirect...             │
│  · 并行声明：parallel(n)                                 │
│                                                          │
│  此时数据还“躺”在管道里，一动不动，什么都没发生。           │
│  就像你写好了一份详细的旅行计划，但还没踏出家门。           │
└──────────────────┬───────────────────────────────────────┘
                   │  toUnordered() / toOrdered() /
                   │  toWindow() / toStatistics() / sort()
                   ▼
┌──────────────────────────────────────────────────────────┐
│                📦 第二阶段：物化与收集                    │
│           Collectable<E>  (可收集对象)                   │
│            命名空间: collectable                         │
│                                                          │
│  · 触发数据流动：此时 Generator 被真正调用               │
│  · 选择数据结构：有序 map / 无序 unordered_map           │
│  · 物化所有元素到内存                                    │
│                                                          │
│  数据终于“活”了！它们从管道中涌出，被整理到合适的容器中。   │
│  就像你终于出发了，行李已经打包好，整装待发。              │
└──────────────────┬───────────────────────────────────────┘
                   │  toVector() / findFirst() / count() /
                   │  summate() / average() / join() / ...
                   ▼
┌──────────────────────────────────────────────────────────┐
│                ⚙️ 第三阶段：终端计算                      │
│              Collector<E,A,R>                            │
│              命名空间: collector                         │
│                                                          │
│  · 五阶段执行：Identity → Accumulate → Combine → Finish  │
│  · 支持并行：多线程分片累加，自动合并                     │
│  · 返回最终结果：vector、optional、double、bool...       │
│                                                          │
│  最终结果出来了！就像你到达目的地，拍下了最美的照片。      │
└──────────────────────────────────────────────────────────┘
```

**关键规则**：你必须先通过 `toUnordered()`、`toOrdered()`、`toWindow()`、`toStatistics()` 或 `sort()` 将 `Semantic<E>` 转换为 `Collectable<E>`，然后才能调用 `toVector()`、`findFirst()`、`count()` 等终端方法。 这些方法不在 Semantic 上——它们属于 `Collectable`。这不是疏忽，而是一个深思熟虑的设计决策。✅

---

### 🧭 五种物化路径，五种不同的命运

当你站在 `Semantic<E>` 的十字路口，有五个方向可供选择。每一个都决定了数据将以何种方式被组织、查询和使用。理解它们之间的区别，是掌握 Semantic-Cpp 的关键。🧠

| 转换方法         | 目标类型             | 底层数据结构         | 性能特征                | 典型终端方法               |
|------------------|----------------------|----------------------|-------------------------|---------------------------|
| `toUnordered()`  | UnorderedCollectable<E> | unordered_map<Timestamp,E> | 平均 O(1) 查找，最高吞吐 | toVector、findFirst、count、group... |
| `toOrdered()`    | OrderedCollectable<E>   | map<Timestamp,E>     | O(log n) 查找，按索引排序 | toVector、findAt、join、toMap... |
| `sort()`         | OrderedCollectable<E>   | map<Timestamp,E>     | 按值排序后物化           | 同上（但索引已被覆盖为值顺序） |
| `toWindow()`     | WindowCollectable<E>    | 继承有序集合         | 支持 slide / tumble      | slide、tumble、以及所有父类方法 |
| `toStatistics<D>()` | Statistics<E,D>     | 继承有序集合         | 30+ 内置统计方法        | summate、average、median、fft... |

### 🔄 五种路径的详细对比

| 对比维度       | toUnordered()           | toOrdered()             | sort()                  | toWindow()             | toStatistics<D>()       |
|----------------|-------------------------|-------------------------|-------------------------|------------------------|-------------------------|
| 是否保持索引顺序 | 否（无序）              | 是（按原始索引）        | 否（按值重新排序）      | 是（继承有序）         | 是（继承有序）          |
| 查找性能       | 最快 O(1)               | 较快 O(log n)           | 较快 O(log n)           | 较快 O(log n)          | 较快 O(log n)           |
| 内存占用       | 较低                    | 较低                    | 较低                    | 较低                   | 取决于统计方法         |
| 能否继续链式调用 | 是（终端方法）          | 是（终端方法）          | 是（终端方法）          | 是（slide/tumble 返回 Semantic） | 是（终端方法）         |
| 适用场景       | 不关心顺序的快速聚合    | 时间序列、保持生成顺序  | 按值排序、排名、分页    | 滑动窗口分析           | 数学统计建模            |

`sort()` 是唯一一个跳过 `toXxx()` 直接抵达 `Collectable` 的中间操作。调用后所有之前的索引操作（reverse、translate、redirect）都将被覆盖，元素按值重新分配自然顺序索引。🔀

**选择哪条路？问问自己**：🤔

- 我关心顺序吗？关心就用 `toOrdered()`，不关心就用 `toUnordered()`
- 我需要窗口分析吗？`toWindow()` 然后 `slide()` 或 `tumble()`
- 我需要统计吗？`toStatistics<double>()` 一站式搞定
- 我只要排序？`sort()` 一步到位

---

### 📋 collectable 命名空间 — 全部终端方法

#### ✅ 匹配操作

| 方法               | 返回类型 | 描述                 |
|--------------------|----------|----------------------|
| `anyMatch(predicate)`  | bool     | 任意元素满足条件     |
| `allMatch(predicate)`  | bool     | 所有元素满足条件     |
| `noneMatch(predicate)` | bool     | 没有元素满足条件     |

#### 🔍 查找操作

| 方法                     | 返回类型         | 描述                     |
|--------------------------|------------------|--------------------------|
| `findFirst()`            | std::optional<E> | 查找第一个元素           |
| `findLast()`             | std::optional<E> | 查找最后一个元素         |
| `findAny()`              | std::optional<E> | 随机查找元素             |
| `findAt(index)`          | std::optional<E> | 指定索引查找（支持负索引） |
| `findMaximum()`          | std::optional<E> | 查找最大值               |
| `findMaximum(comparator)`| std::optional<E> | 自定义比较查找最大值     |
| `findMinimum()`          | std::optional<E> | 查找最小值               |
| `findMinimum(comparator)`| std::optional<E> | 自定义比较查找最小值     |

#### 🔢 聚合操作

| 方法      | 返回类型        | 描述         |
|-----------|-----------------|--------------|
| `count()` | function::Module | 计算元素总数 |
| `empty()` | bool            | 流是否为空   |

#### 🔀 归约操作

| 方法                      | 返回类型         | 描述             |
|---------------------------|------------------|------------------|
| `reduce(accumulator)`     | std::optional<E> | 无初始值归约     |
| `reduce(identity, accumulator)` | E       | 带初始值归约     |
| `reduce(identity, acc, combiner)` | R | 完全自定义归约   |

#### 🧺 收集到序列容器

| 方法             | 返回类型               | 描述                 |
|------------------|------------------------|----------------------|
| `toVector()`     | std::vector<E>         | 收集为 vector，保持顺序 |
| `toList()`       | std::list<E>           | 收集为 list          |
| `toDeque()`      | std::deque<E>          | 收集为 deque         |
| `toForwardList()`| std::forward_list<E>   | 收集为 forward_list  |
| `toArray<N>()`   | std::array<E, N>       | 收集为固定大小 array |

#### 🔐 收集到关联容器

| 方法                                     | 返回类型                        | 描述                 |
|------------------------------------------|---------------------------------|----------------------|
| `toSet()`                                | std::set<E>                     | 收集为 set（去重排序） |
| `toMultiset()`                           | std::multiset<E>                | 收集为 multiset      |
| `toUnorderedSet()`                       | std::unordered_set<E>           | 收集为 unordered_set |
| `toUnorderedMultiset()`                  | std::unordered_multiset<E>      | 收集为 unordered_multiset |
| `toMap(keyExtractor)`                    | std::map<K, E>                  | 按键收集为 map       |
| `toMap(keyExtractor, valueExtractor)`    | std::map<K, V>                  | 自定义键值收集为 map |
| `toMultimap(keyExtractor)`               | std::multimap<K, E>             | 按键收集为 multimap  |
| `toMultimap(keyExtractor, valueExtractor)` | std::multimap<K, V>          | 自定义键值收集为 multimap |
| `toUnorderedMap(keyExtractor, valueExtractor)` | std::unordered_map<K, V> | 收集为 unordered_map |
| `toUnorderedMultimap(keyExtractor)`      | std::unordered_multimap<K, E>   | 按键收集为 unordered_multimap |
| `toUnorderedMultimap(keyExtractor, valueExtractor)` | std::unordered_multimap<K, V> | 自定义键值收集为 unordered_multimap |

#### 🧮 收集到适配器容器

| 方法                | 返回类型                  | 描述         |
|---------------------|---------------------------|--------------|
| `toStack()`         | std::stack<E>             | 收集为 stack |
| `toQueue()`         | std::queue<E>             | 收集为 queue |
| `toPriorityQueue()` | std::priority_queue<E>    | 收集为 priority_queue |

#### 👥 分组与分区

| 方法                                     | 返回类型                          | 描述                 |
|------------------------------------------|-----------------------------------|----------------------|
| `group(keyExtractor)`                    | std::unordered_map<K, std::vector<E>> | 按键分组，保留完整元素 |
| `groupBy(keyExtractor, valueExtractor)`  | std::unordered_map<K, std::vector<V>> | 按键分组，自定义值提取 |
| `partition(size)`                        | std::vector<std::vector<E>>       | 按固定大小分区       |
| `partitionBy(keyExtractor)`              | std::vector<std::vector<E>>       | 按索引键分区，保留完整元素 |
| `partitionBy(keyExtractor, valueExtractor)` | std::vector<std::vector<V>>   | 按索引键分区，自定义值提取 |

#### 📃 字符串输出操作

| 方法                               | 返回类型               | 描述                 |
|------------------------------------|------------------------|----------------------|
| `join()`                           | charsequence::Charsequence | 默认格式连接：`[elem1,elem2,...]` |
| `join(delimiter)`                  | charsequence::Charsequence | 自定义分隔符         |
| `join(prefix, delimiter, suffix)`  | charsequence::Charsequence | 完全自定义格式       |
| `out()`                            | charsequence::Charsequence | 输出到 stdout       |
| `out(delimiter)`                   | charsequence::Charsequence | 带分隔符输出到 stdout |
| `out(prefix, delimiter, suffix)`   | charsequence::Charsequence | 完全自定义输出到 stdout |
| `error()`                          | charsequence::Charsequence | 输出到 stderr       |
| `error(delimiter)`                 | charsequence::Charsequence | 带分隔符输出到 stderr |
| `error(prefix, delimiter, suffix)`| charsequence::Charsequence | 完全自定义输出到 stderr |

#### 🔧 自定义收集与遍历

| 方法                                 | 返回类型 | 描述                 |
|--------------------------------------|----------|----------------------|
| `collect(identity, acc, comb, fin)` | R        | 自定义四阶段收集     |
| `collect(identity, interrupt, acc, comb, fin)` | R | 自定义可中断收集     |
| `forEach(consumer)`                  | void     | 遍历每个元素执行副作用 |

---

### 📈 `Statistics<E,D>` — 统计方法

| 方法                     | 返回类型              | 描述                               |
|--------------------------|-----------------------|------------------------------------|
| `summate()`              | D                     | 求和                               |
| `average()`              | D                     | 平均值                             |
| `minimum()`              | std::optional<D>     | 最小值                             |
| `maximum()`              | std::optional<D>     | 最大值                             |
| `range()`                | D                     | 范围（最大值减最小值）             |
| `variance()`             | D                     | 总体方差                           |
| `standardDeviation()`    | D                     | 总体标准差                         |
| `median()`               | std::optional<D>     | 中位数                             |
| `mode()`                 | std::optional<E>     | 众数（基于频域分析）               |
| `percentile(p)`          | std::optional<D>     | 第 p 百分位数                      |
| `firstQuartile()`        | std::optional<D>     | 第一四分位数 (Q1)                  |
| `thirdQuartile()`        | std::optional<D>     | 第三四分位数 (Q3)                  |
| `interquartileRange()`   | std::optional<D>     | 四分位距 (IQR = Q3 - Q1)          |
| `skewness()`             | D                     | 偏度                               |
| `kurtosis()`             | D                     | 峰度                               |
| `frequency()`            | std::map<E, std::complex<double>> | 频域特征（索引相位编码）   |
| `distribute()`           | std::map<E, std::complex<double>> | 空间分布特征（位置编码）   |
| `dft()`                  | std::vector<std::complex<double>> | 离散傅里叶变换             |
| `idft()`                 | std::vector<std::complex<double>> | 逆离散傅里叶变换           |
| `fft()`                  | std::vector<std::complex<double>> | 快速傅里叶变换             |
| `ifft()`                 | std::vector<std::complex<double>> | 逆快速傅里叶变换           |
| `gradient(gradFunc, lr, iter, th)` | std::vector<double> | 梯度下降（解析梯度）   |
| `gradient(costFunc, lr, iter, th, h)` | std::vector<double> | 梯度下降（数值梯度）   |

以上方法均支持可选的 mapper 参数版本（如 `average(mapper)`），先对元素做变换再统计。

---

### 🧰 全新的容器特化：容器也是元素

这是 Semantic-Cpp 最独特的特性之一。在传统流式框架中，流里的每个元素通常是一个标量值——一个 int、一个 string。但在真实世界中，数据往往以批的形式存在：

- 一批传感器读数 = `std::vector<double>`
- 一组用户订单 = `std::list<Order>`
- 一个键值配置 = `std::map<string, string>`
- 一个固定大小的矩阵行 = `std::array<float, 4>`

Semantic-Cpp 为 **10 种标准库容器** 提供了 `Semantic` 模板特化：

| 特化类型                     | 说明               | 支持的操作                     |
|------------------------------|--------------------|--------------------------------|
| `Semantic<std::vector<E>>`   | 向量容器流         | 全部中间操作 + 全部终端方法     |
| `Semantic<std::list<E>>`     | 链表容器流         | 同上                           |
| `Semantic<std::deque<E>>`    | 双端队列容器流     | 同上                           |
| `Semantic<std::set<E>>`      | 有序集合容器流     | 同上                           |
| `Semantic<std::unordered_set<E>>` | 无序集合容器流 | 同上                           |
| `Semantic<std::map<K,V>>`    | 有序映射容器流     | 同上                           |
| `Semantic<std::unordered_map<K,V>>` | 无序映射容器流 | 同上                           |
| `Semantic<std::queue<E>>`    | 队列容器流         | 同上                           |
| `Semantic<std::stack<E>>`    | 栈容器流           | 同上                           |
| `Semantic<std::array<E,N>>`  | 固定数组容器流     | 同上                           |

所有容器特化都完整支持：`map`、`filter`、`takeWhile`、`dropWhile`、`distinct`、`sort`、`limit`、`skip`、`reverse`、`translate`、`redirect`、`sub`、`concatenate`、`peek`、`flatMap`、`flat`、`parallel` 以及全部终端转换方法。🔧

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

### 🔧 `Semantic<E>` 中间操作方法

| 类别       | 方法       | 描述                               |
|------------|------------|------------------------------------|
| 元素变换   | `map`      | 一对一映射转换                     |
|            | `flatMap`  | 一对多映射并展平（R 由返回类型自动推导） |
|            | `flat`     | 展平嵌套流（元素类型不变）         |
| 元素过滤   | `filter`   | 条件过滤                           |
|            | `takeWhile`| 条件满足时持续获取，一旦不满足立即停止 |
|            | `dropWhile`| 条件满足时持续丢弃，直到遇到第一个不满足的 |
|            | `distinct` | 去重（支持自定义比较器）           |
| 数量控制   | `limit`    | 限制元素数量                       |
|            | `skip`     | 跳过前 n 个元素                    |
|            | `sub`      | 截取子范围 [start, end)            |
| 索引操作   | `redirect` | 重新映射索引                       |
|            | `reverse`  | 反转索引（使用负索引）              |
|            | `translate`| 偏移索引（固定值或动态函数）         |
| 观察操作   | `peek`     | 观察每个元素（不修改流）             |
| 并行声明   | `parallel(n)` | 声明并行度                     |
| 连接操作   | `concatenate` | 连接另一个 Semantic 流或标准容器 |
| 终端转换   | `toUnordered` | 转为无序收集器                   |
|            | `toOrdered`  | 转为有序收集器                   |
|            | `toWindow`   | 转为窗口收集器                   |
|            | `toStatistics<D>` | 转为统计收集器               |

---

## 🔧 第七层：semantics.h — 流构建工厂

`sematics.h` 提供所有流构建工厂函数。如果说 `semantic.h` 是流的“大脑”（负责变换和调度），那么 `sematics.h` 就是流的“心脏”——一切数据之旅从此开始。❤️

### 🔢 数值范围生成

| 方法                          | 描述                         |
|-------------------------------|------------------------------|
| `useRange(start, end)`        | 生成 [start, end) 范围内的数值流 |
| `useRange(start, end, step)`  | 带步长的范围生成，支持负步长   |
| `useRangeClosed(start, end)`  | 生成 [start, end] 闭区间范围  |
| `useRangeClosed(start, end, step)` | 带步长的闭区间范围，支持负步长 |

### ♾️ 无限流生成

| 方法                          | 描述                     |
|-------------------------------|--------------------------|
| `useInfinite(seed, generator)`| 从种子值开始，无限迭代生成 |
| `useGenerate(supplier)`       | 无限调用供应者生成       |
| `useGenerate(supplier, limit)`| 有限次数调用供应者生成   |
| `useIterate(seed, generator)` | 从种子值开始无限迭代     |
| `useIterate(seed, generator, limit)` | 有限次数迭代生成 |
| `useRandom()`                 | 无限随机整数流           |
| `useRandom(min, max)`         | 指定范围的无限随机数流（自动区分整型/浮点） |
| `useRandom(min, max, count)`  | 指定范围和数量的随机数流 |

### 📦 容器与元素构建

| 方法                | 描述                             |
|---------------------|----------------------------------|
| `useEmpty()`        | 创建空流                         |
| `useOf(element)`    | 从单个元素创建流                 |
| `useOf(e1, e2)`     | 从两个元素创建流                 |
| `useOf(e1, e2, e3)` | 从三个元素创建流                 |
| `useOf({...})`      | 从初始化列表创建流               |
| `useFrom(container)` | 从任何标准容器创建流（支持移动语义） |
| `useFrom({...})`    | 从初始化列表创建流               |
| `useRepeat(element, count)` | 重复指定元素 n 次           |

### 📄 文本处理

| 方法                    | 描述                         |
|-------------------------|------------------------------|
| `useBlob(text)`         | 将字符串按字节拆分为 char 流 |
| `useBlob(text, start, end)` | 将字符串指定范围按字节拆分   |
| `useBlob(istream)`      | 从输入流按行读取             |
| `useBlob(istream, delimiter)` | 从输入流按分隔符读取     |
| `useText(text)`         | 将字符串作为整体文本流（Charsequence） |
| `useText(text, delimiter)` | 按分隔符拆分文本           |
| `useText(istream)`      | 从输入流读取整个内容         |
| `useText(istream, delimiter)` | 从输入流按分隔符读取     |

### 🌍 Unicode 处理

| 方法                          | 描述                                   |
|-------------------------------|----------------------------------------|
| `useSequence(charsequence)`   | 从字符序列创建码点流                   |
| `useSequence(charsequence, start, end)` | 从字符序列指定范围创建码点流       |
| `useSequence(text, encoding)` | 从文本创建指定编码的码点流             |
| `useSequence(istream, encoding)` | 从输入流创建指定编码的码点流       |
| `useCharsequence(charsequence)` | 将字符序列作为整体流                 |
| `useCharsequence(charsequence, delimiter)` | 按分隔符拆分字符序列             |
| `useCharsequence(istream, encoding)` | 从输入流读取整个字符序列         |
| `useCharsequence(istream, delimiter, encoding)` | 从输入流按分隔符读取字符序列 |

---

## 🔐 第八层：hash.h / less.h — 容器世界的通用语言

当你在 `Semantic<std::vector<E>>` 上调用 `distinct()`，它内部需要 `std::unordered_set<std::vector<E>>`。标准库没有为 `std::vector` 提供哈希特化——这是 C++ 标准委员会留下的“空白地带”。Semantic-Cpp 的 `hash.h` 和 `less.h` 填补了这一切：为所有标准库容器（包括嵌套容器）、pair、tuple、optional、variant、chrono 时间类型、complex 复数等提供了完整的哈希和比较支持。任意深度、任意组合的嵌套容器现在都可以作为 unordered_set 的键或 set 的元素。🌉

---

## 🚀 性能优化建议

1. **选择对的容器**：不关心顺序用 `toUnordered()`，需要排序用 `toOrdered()` 或 `sort()`
2. **善用并行**：数据量大或处理逻辑耗时时使用 `parallel()`，避免阻塞式 I/O
3. **优化操作顺序**：尽早 `filter`，明智 `sort`
4. **利用惰性求值**：中间操作不立即执行，`takeWhile` 和 `limit` 可提前终止

---

Semantic-Cpp — 用现代 C++ 构建高效、清晰的数据处理管道。🚀🎯✨
