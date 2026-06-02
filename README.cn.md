# 🚀 Semantic-Cpp：面向未来的 C++ 智能流处理框架

**Semantic-Cpp** 是一个完全重构设计的现代化 C++ 流处理库，采用 **“多头文件、零外部依赖”** 的模块化架构。每个头文件职责清晰、独立可测，共同构成了一个完整的流处理生态系统。本库创造性地融合了多种编程范式的精华：

- 🎯 **Java Stream API 的优雅与流畅**：链式调用，声明式编程，让代码如诗般优雅
- ⚡ **JavaScript Generator 的惰性与灵活**：延迟计算，按需生成，内存友好
- 🗄️ **数据库索引的高效与有序**：智能排序，索引驱动，时序数据处理利器
- 📦 **容器即元素的批处理哲学**：向量、链表、映射……任何容器都能作为流中一等公民，自由流动

---

**你是否厌倦了手写 for 循环遍历 vector，再嵌套一个 if 过滤，再手动 push_back 到另一个容器？**

**你是否在深夜调试过索引偏移一格的 Bug，只因为想在反向遍历时取“倒数第三个”元素？**

**你是否渴望像操作数据库一样——按索引精确定位、按窗口滑动分析、用一行链式调用完成从数据到统计的完整旅程？**

**Semantic-Cpp 正是为此而生。**

它将数据处理抽象为对 **“元素”** 及其 **“逻辑位置（索引）”** 的操作——就像数据库中的“行”和“主键”，你可以在不触碰数据本身的情况下，随意重排、偏移、反转索引；也可以把任何一个容器（vector、map、array……）当作一个不可分割的整体在流中传递，然后随时“拆包”回元素级别。这种双粒度自由切换的能力，是传统流式框架所不具备的。

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

| 命名空间    | 所在文件                     | 职责                                                                     | 核心类型/函数示例                                                                                                                                |
| :---------- | :--------------------------- | :----------------------------------------------------------------------- | :----------------------------------------------------------------------------------------------------------------------------------------------- |
| **function**    | function.h                   | 类型系统基石                                                             | `Timestamp`、`Module`、`Generator<T>`、`Supplier<R>`、`Consumer<T>`、`Predicate<T>`、`Comparator<T>`、`randomly()`                               |
| **pool**        | pool.h                       | 并发执行引擎                                                             | `pool::pool`（全局线程池）、`submit()`、`emergencyShutdown()`                                                                                      |
| **charsequence** | charsequence.h               | Unicode 字符串处理                                                       | `charset`、`Meta`、`Point`、`Charsequence`、`Builder`、`Buffer`、`PointIterator`                                                                  |
| **collector**   | collector.h + collectors.h   | 终端收集执行                                                             | `Collector<E,A,R>`（五阶段模型）、`Identity<A>`、`Accumulator<A,E>`、`Combiner<A>`、`Finisher<A,R>`、`useXxx()` 工厂函数（100+）                          |
| **collectable** | semantic.h                   | 物化数据容器                                                             | `Collectable<E>`（抽象基类）、`OrderedCollectable<E>`、`UnorderedCollectable<E>`、`WindowCollectable<E>`、`Statistics<E,D>`                         |
| **semantic**    | semantic.h + semantics.h     | 流构建与中间操作                                                         | `Semantic<E>`（主模板）、`Semantic<Semantic<E>>`（嵌套特化）、10 种容器特化、`useRange()`、`useFrom()`、`useText()` 等 30+ 工厂函数、`map()`、`filter()` 等中间操作 |

---

## 🔗 命名空间协作流程

```cpp
semantic::useRange(0, 100)          // ← semantic 命名空间：创建流
    .map(int x { ... })         // ← semantic 命名空间：中间变换
    .filter(int x { ... })      // ← semantic 命名空间：中间过滤
    .toUnordered()                  // ← 转换为 collectable 命名空间
    .toVector();                    // ← 调用 collector 命名空间的收集器
```

---

## 📋 命名空间职责详解

### function — 类型基石
这是最底层的命名空间，定义了整个框架的类型语言。`Generator<T>` 是整个流系统的核心抽象，使用控制反转实现惰性求值。所有后续命名空间都依赖这里的类型别名来保持 API 的一致性和可读性。

### pool — 并发引擎
全局线程池 `pool::pool` 是所有并行操作的后端。它在程序启动时自动初始化，在程序退出时自动清理。用户通常不需要直接与它交互——只需在流上调用 `.parallel(4)`，框架会自动将任务提交给线程池。

### charsequence — Unicode 专家
这是框架中唯一完全不依赖其他命名空间的独立模块。它处理所有与字符编码相关的复杂性——从 UTF‑8 的变长字节序列解析，到代理对的正确配对，到大小写转换和字符串分割。`Builder` 和 `Buffer` 提供了高效的可变字节操作。

### collector — 终端执行引擎
这是“真正干活”的地方。`Collector<E,A,R>` 的五阶段模型（Identity → Accumulate → Combine → Finish，外加可选的 Interrupt 短路）是所有终端操作的基础。`collectors.h` 中的 100+ 工厂函数覆盖了从简单计数到 FFT 频域分析的全频谱操作。

### collectable — 物化数据的中转站
当流从 `Semantic<E>` 转换为 `Collectable<E>` 时，数据被真正“物化”到内存中。`OrderedCollectable` 使用 `std::map` 保持索引顺序，`UnorderedCollectable` 使用 `std::unordered_map` 追求极致性能，`WindowCollectable` 支持滑动和滚动窗口，`Statistics` 提供一站式统计接口。

### semantic — 用户最常打交道的入口
这是用户最常使用的命名空间。流构建工厂（`useRange`、`useFrom`、`useText` 等）和中间操作（`map`、`filter`、`takeWhile`、`distinct` 等）都在这里。`Semantic<E>` 的主模板和 10 种容器特化提供了统一的操作接口，让元素级和容器级流处理无缝切换。

---

## 🏗️ 第一层：function.h — 类型基础
`function.h` 定义了整个框架的类型系统，是所有模块的共同基石。它就像一个精密钟表的齿轮系统——每一个类型别名都是一枚精心打磨的齿轮，在后续的流处理引擎中精确咬合：

```cpp
namespace function {
    using Timestamp = long long;           // 索引类型，数据在流中的“时间戳”
    using Module = unsigned long long;     // 模块/计数类型，无符号的批量操作单位

    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept — 接收一个元素
        std::function<bool(T, Timestamp)>       // interrupt — 是否中断？
    )>;
}
```

`Generator` 是整个流系统的核心抽象。它不返回数据，而是接受两个回调——`accept`（“我准备好了，请接收这个元素”）和 `interrupt`（“需要停下来吗？”）。这种控制反转的设计让数据生产者完全不知道消费者是谁，只需在合适的时机“推送”数据。这正是惰性求值的精髓：数据只在 `accept` 被调用时才真正“流动”，在此之前一切都只是描述。🌊

| 类型别名              | 完整定义                                              | 用途                                 |
| :-------------------- | :---------------------------------------------------- | :----------------------------------- |
| **Timestamp**         | `long long`                                           | 元素在流中的逻辑位置                 |
| **Module**            | `unsigned long long`                                  | 计数、容量、并发度                   |
| **Runnable**          | `std::function<void()>`                              | 无参无返回值任务                     |
| **Supplier<R>**       | `std::function<R()>`                                 | 供应者，无中生有                     |
| **Function<T,R>**     | `std::function<R(T)>`                                | 单参数函数                           |
| **BiFunction<T,U,R>** | `std::function<R(T,U)>`                              | 双参数函数                           |
| **TriFunction<T,U,V,R>** | `std::function<R(T,U,V)>`                          | 三参数函数                           |
| **Unary<T>**          | `std::function<T(T)>`                                | 一元运算                             |
| **Binary<T>**         | `std::function<T(T,T)>`                              | 二元运算                             |
| **Consumer<T>**       | `std::function<void(T)>`                             | 消费者                               |
| **BiConsumer<T,U>**   | `std::function<void(T,U)>`                           | 双参数消费者                         |
| **TriConsumer<T,U,V>** | `std::function<void(T,U,V)>`                        | 三参数消费者                         |
| **Predicate<T>**      | `std::function<bool(T)>`                             | 谓词判断                             |
| **BiPredicate<T,U>**  | `std::function<bool(T,U)>`                           | 双参数谓词                           |
| **TriPredicate<T,U,V>** | `std::function<bool(T,U,V)>`                        | 三参数谓词                           |
| **Comparator<T>**     | `std::function<int(const T&, const T&)>`             | 比较器                               |
| **Generator<T>**      | `BiConsumer<BiConsumer<T, Timestamp>, BiPredicate<T, Timestamp>>` | 流生成器核心抽象                     |

---

## ⚡ 第二层：pool.h — 并发基石
`pool.h` 提供全局线程池 `pool::pool`，是整个框架的并发引擎。它采用声明式并行设计：

| 特性              | 说明                                                                 |
| :---------------- | :------------------------------------------------------------------- |
| 🎯 **声明式并行**  | `.parallel(4)` 只声明“我想用 4 个线程”，不立即启动                   |
| 🛡️ **紧急关闭**    | 内置 `emergencyShutdown()` 和 `std::set_terminate` 处理器             |
| 🔄 **异常传播**    | `submit()` 返回 `std::future`，异常安全地传播到主线程                 |

**pool 命名空间 API**

| 成员                       | 类型             | 描述                                   |
| :------------------------- | :--------------- | :------------------------------------- |
| `pool::pool`               | 全局线程池实例     | 程序级别的单例线程池，自动初始化           |
| `pool::pool.submit<A>(task)` | 方法             | 提交任务，返回 `std::future<A>`         |
| `pool::pool.emergencyShutdown()` | 方法             | 紧急关闭所有线程                         |

当你在流上调用 `.parallel(4).toUnordered().count()` 时，收集器的 `collect()` 方法会自动将数据按索引分片，提交给 `pool::pool` 并行处理，最后通过 `Combiner` 合并各线程的局部结果。这一切对用户完全透明——你只需要声明意图，框架负责执行。

---

## 🔤 第三层：charsequence.h — Unicode 字符序列
`charsequence.h` 是一个完整的 Unicode 处理模块，提供字符序列的创建、转换和操作功能。

**charsequence 命名空间 API**

| 类型/函数                    | 描述                                                                 |
| :--------------------------- | :------------------------------------------------------------------- |
| **charset**                  | 枚举 ascii、utf8、utf16、utf16be、utf16le、utf32、utf32be、utf32le、latin1 |
| **Meta**                     | 元数据包装器，存储无符号整数值                                       |
| **Point**                    | Unicode 码点，支持代理对检测（`isSurrogate()`）和有效性验证（`isValidCodePoint()`） |
| **Charsequence**             | 不可变字符序列，支持 `split`、`replace`、`indexOf`、`sub`、`trim`、`toUpperCase`、`toLowerCase`、`reverse`、`startsWith`、`endsWith`、`contains`、`compare`、`getBytes`、`getPoints`、`getMetas`、`join` 等 |
| **Builder**                  | 可变字节构建器，支持 `prepend`、`insert`、`append`（支持 bool、整数、浮点、字符串等多种类型）、`toCharsequence` |
| **Buffer**                   | 线程安全环形缓冲区，支持 `write`、`read`、`peek`、`prepend`、`append`、`clear`、`shrinkToFit`、`atomic`（加锁批量操作） |
| **PointIterator**            | 双向迭代器，遍历 Unicode 码点                                        |
| **sequenceLength()**         | 根据首字节判断 UTF‑8 序列长度                                        |
| **encode()**                 | 将码点编码为指定编码的字节序列                                         |
| **decode()**                 | 从字节序列解码为码点                                                 |
| **convert()**                | 编码转换                                                             |

---

## 🔧 第四层：collector.h — 收集器框架
`collector.h` 实现了收集器模式，是终端操作的核心引擎。

### 五阶段模型
```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (可选短路)
```

这五个阶段像一条精密的流水线：首先准备好初始状态（Identity），然后逐个接收元素并更新中间结果（Accumulator），接着在并行场景下合并各线程的局部结果（Combiner），最后将中间结果转换为用户期望的最终形式（Finisher）。而 Interrupt 则像一位警觉的监工，随时可以在条件满足时喊停整个流程。🛑

**collector 命名空间核心类型**

| 类型别名             | 完整定义                                      | 角色         |
| :------------------- | :-------------------------------------------- | :----------- |
| **Identity<A>**      | `function::Supplier<A>`                       | 提供初始值   |
| **Accumulator<A,E>** | `function::TriFunction<A, E, function::Timestamp, A>` | 累加元素     |
| **Combiner<A>**      | `function::BiFunction<A, A, A>`               | 合并并行结果 |
| **Finisher<A,R>**    | `function::Function<A, R>`                    | 最终转换     |
| **Interrupt<E,A>**   | `function::TriPredicate<E, function::Timestamp, A>` | 短路判断     |

**collector 命名空间框架函数**

| 函数                                         | 描述                         |
| :------------------------------------------- | :--------------------------- |
| `useFull(identity, accumulator, combiner, finisher)` | 创建完整收集器（无短路）       |
| `useShortable(identity, interrupt, accumulator, combiner, finisher)` | 创建可短路收集器             |

**Collector<E,A,R> 类**

| 方法                                                       | 描述                               |
| :--------------------------------------------------------- | :--------------------------------- |
| `collect(generator, concurrent)`                           | 从 Generator 收集，支持并行         |
| `collect(container, concurrent)`                           | 从标准容器收集，支持并行             |
| `collect(initializer_list, concurrent)`                    | 从初始化列表收集                     |
| `collect(array, concurrent)`                               | 从 `std::array` 收集                |
| `collect(forward_list, concurrent)`                        | 从 `forward_list` 收集              |
| `collect(deque, concurrent)`                               | 从 `deque` 收集                     |
| `collect(stack, concurrent)`                               | 从 `stack` 收集                     |
| `collect(queue, concurrent)`                               | 从 `queue` 收集                     |

**并发支持**

`Collector::collect()` 自动处理数据分片（按索引模运算分发到各线程）、结果合并（通过 Combiner 归并局部结果）、异常传播（通过 `std::exception_ptr` 和 `std::atomic<bool>`）。

---

## 🏭 第五层：collectors.h — 收集器工厂
`collectors.h` 提供了丰富的预置收集器工厂函数。这里不是简单地“有几个工具函数”——这是一个完整的数据处理武器库，涵盖了从基础匹配到频域分析的全频谱操作。

### 📊 匹配操作 — 一探究竟
想知道流中所有元素是否满足某个条件？或者是否存在任何一个“叛逆者”？这三剑客瞬间给出答案：

| 方法                    | 描述                                 | 返回类型       |
| :---------------------- | :----------------------------------- | :------------- |
| `useAllMatch(predicate)` | 所有元素满足条件（全称量词 ∀）         | `bool`         |
| `useAnyMatch(predicate)` | 任意元素满足条件（存在量词 ∃）         | `bool`         |
| `useNoneMatch(predicate)` | 没有元素满足条件                     | `bool`         |

它们都支持**短路求值**——一旦确定答案就立即停止遍历，不会浪费时间在剩余数据上。就像一个有经验的侦探，一旦找到关键证据就结案，不会翻遍整个档案室。

### 🔍 查找操作 — 大海捞针
从茫茫数据中找到你想要的那个“它”：

| 方法                | 描述                                     | 返回类型             |
| :------------------ | :--------------------------------------- | :------------------- |
| `useFindFirst()`    | 查找第一个元素                           | `std::optional<E>`   |
| `useFindLast()`     | 查找最后一个元素                         | `std::optional<E>`   |
| `useFindAny()`      | 随机查找一个元素                         | `std::optional<E>`   |
| `useFindAt(index)`  | 精确定位：支持正索引和负索引             | `std::optional<E>`   |
| `useFindMaximum()`  | 查找最大值                               | `std::optional<E>`   |
| `useFindMinimum()`  | 查找最小值                               | `std::optional<E>`   |

**负索引是 Semantic-Cpp 的独门绝技** 👉 `findAt(-1)` 直接拿到最后一个元素，`findAt(-2)` 拿到倒数第二个。不需要先 `reverse()` 再 `findFirst()`——一步到位，干净利落。

### 📈 聚合操作 — 化零为整
让数据说话，用数字总结一切：

| 方法                | 描述                                 | 返回类型             |
| :------------------ | :----------------------------------- | :------------------- |
| `useCount()`        | 元素总数                             | `Module`             |
| `useSummate<E,D>()` | 求和                                 | `D`                  |
| `useAverage<E,D>()` | 平均值                               | `D`                  |
| `useRange<E,D>()`   | 数值范围（最大值 − 最小值）            | `D`                  |
| `useMinimum<E,D>()` | 最小值                               | `std::optional<D>`   |
| `useMaximum<E,D>()` | 最大值                               | `std::optional<D>`   |

每个聚合方法都支持可选的 **mapper 参数**——先对每个元素做一个变换再聚合。例如，你有一批 `Person` 对象，想求平均年龄：
`.collect(useAverage<Person, double>(Person p { return p.age; }))`

### 📊 统计操作 — 数据科学家の工具箱
从描述性统计到频域分析，统计学家的瑞士军刀：

| 方法                               | 描述                         | 返回类型             |
| :--------------------------------- | :--------------------------- | :------------------- |
| `useVariance<E,D>()`              | 总体方差                     | `D`                  |
| `useStandardDeviation<E,D>()`     | 总体标准差                   | `D`                  |
| `useSkewness<E,D>()`              | 偏度（分布的对称性）           | `D`                  |
| `useKurtosis<E,D>()`              | 峰度（分布的尾部厚度）         | `D`                  |
| `useMedian<E,D>()`                | 中位数                       | `std::optional<D>`   |
| `useMode<E>()`                    | 众数（基于频域分析！）         | `std::optional<E>`   |
| `usePercentile<E,D>(p)`           | 第 p 百分位数                | `std::optional<D>`   |
| `useFrequency<E>()`               | 频域特征（索引相位编码）       | `std::map<E, complex>` |
| `useDistribution<E>()`            | 空间分布特征（位置编码）       | `std::map<E, complex>` |

特别值得一提的是 `useMode()` 和 `useFrequency()`——它们不是简单地数数，而是使用索引相位编码技术，将每个元素出现的位置信息编码为复数平面上的角度，利用欧拉公式 e^(iθ) 的周期性来捕捉数据中的周期性模式。这是一种优雅的频域分析方法，让你的代码瞬间有了“信号处理”的味道。📡

### 🔗 归约操作 — 万物皆可折叠
归约（Reduce）是函数式编程中最强大的概念之一——它可以把一个流“折叠”成一个值：

| 方法                                | 描述                                 | 返回类型             |
| :---------------------------------- | :----------------------------------- | :------------------- |
| `useReduce(reducer)`               | 无初始值归约（流为空则返回 nullopt）  | `std::optional<E>`   |
| `useReduce(identity, reducer)`     | 带初始值归约                         | `E`                  |
| `useReduce(id, red, comb, fin)`    | 完全自定义：自定义累积、合并、收尾     | `R`                  |

### 📦 收集到容器 — 落叶归根
数据在流中漂泊了一生，最终需要“安家”。这里有 20+ 种标准库容器供你选择——从 `vector`、`list`、`deque` 到 `set`、`map`、`unordered_map`，从 `stack`、`queue` 到 `priority_queue`，甚至固定大小的 `array`。无论你想要什么数据结构，一个 `toXxx()` 就能搞定。

### 🎨 字符串输出 — 让数据开口说话
`useJoin()`、`useOut()`、`useError()` 系列让你的数据以人类可读的格式呈现。支持自定义前缀、分隔符、后缀——默认使用逗号分隔、方括号包围的格式（如 `[1, 2, 3]`），输出到 stdout 或 stderr，并同时返回格式化后的字符串。

### 🧮 数学工具 — 当流遇上数学
`useDFT()`、`useIDFT()`、`useFFT()`、`useIFFT()` 提供离散傅里叶变换和快速傅里叶变换（Cooley‑Tukey 算法），`useGradient()` 支持解析梯度和数值梯度两种模式的梯度下降优化。你不需要引入任何第三方数学库——这一切都在 Semantic-Cpp 中开箱即用。

---

## 🌊 第六层：semantic.h — 流式中间操作与收集体系
`semantic.h` 是整个框架的核心。但这里有一个你必须理解的关键设计 👇

---

## 🔑 核心设计：三阶段流水线 — Semantic → Collectable → Collector

很多流式框架将“中间操作”和“终端操作”混在同一类型中，让人摸不清什么时候数据真正被处理。**Semantic-Cpp 采用了截然不同的三阶段职责分离设计**，每一阶段各司其职，边界清晰：

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
│  就像你写好了一份详细的旅行计划，但还没踏出家门。              │
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
│  就像你终于出发了，行李已经打包好，整装待发。                   │
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
│                                                          │
│  最终结果出来了！就像你到达目的地，拍下了最美的照片。            │
└──────────────────────────────────────────────────────────┘
```

**关键规则 👉 你必须先通过 `toUnordered()`、`toOrdered()`、`toWindow()`、`toStatistics()` 或 `sort()` 将 `Semantic<E>` 转换为 `Collectable<E>`，然后才能调用 `toVector()`、`findFirst()`、`count()` 等终端方法。** 这些方法不在 `Semantic` 上——它们属于 `Collectable`。这不是疏忽，而是一个深思熟虑的设计决策。

---

## 🎯 五种物化路径，五种不同的命运

当你站在 `Semantic<E>` 的十字路口，有五个方向可供选择。每一个都决定了数据将以何种方式被组织、查询和使用：

| 转换方法         | 目标类型                      | 底层数据结构              | 性能特征                     | 最佳适用场景                          |
| :--------------- | :---------------------------- | :------------------------ | :--------------------------- | :------------------------------------ |
| `toUnordered()` | `UnorderedCollectable<E>`     | `unordered_map<Timestamp,E>` | 平均 O(1) 查找，最高吞吐       | 快速查找、去重、聚合、不关心顺序        |
| `toOrdered()`   | `OrderedCollectable<E>`       | `map<Timestamp,E>`         | O(log n) 查找，按索引排序     | 保留自定义索引顺序、时间序列            |
| `toWindow()`    | `WindowCollectable<E>`        | 继承有序集合                | 支持 `slide` / `tumble`      | 滑动窗口、滚动窗口分析                  |
| `toStatistics()`| `Statistics<E,D>`             | 继承有序集合                | 20+ 内置统计方法              | 全面统计分析、数学建模                  |
| `sort()`        | `OrderedCollectable<E>`       | `map<Timestamp,E>`         | 按值排序后物化                | 按值排序、分页、排名                    |

⚠️ **`sort()` 会覆盖一切**：调用后所有之前的索引操作（`reverse`、`translate`、`redirect`）都将被覆盖，元素按值重新分配自然顺序索引。它直接返回 `OrderedCollectable`，所以调用 `sort()` 后就可以直接使用终端方法了。它是唯一一个跳过 `toXxx()` 直接抵达 `Collectable` 的中间操作。

**选择哪条路？问问自己：**

- 我关心顺序吗？→ 关心就用 `toOrdered()`，不关心就用 `toUnordered()`
- 我需要窗口分析吗？→ `toWindow()` 然后 `slide()` 或 `tumble()`
- 我需要统计吗？→ `toStatistics<double>()` 一站式搞定
- 我只要排序？→ `sort()` 一步到位

---

## 📦 全新的容器特化：容器也是元素

这是 **Semantic-Cpp 最独特的特性之一**。在传统流式框架中，流里的每个元素通常是一个标量值——一个 int、一个 string。但在真实世界中，数据往往以批的形式存在：

- 📊 一批传感器读数 = `std::vector<double>`
- 📋 一组用户订单 = `std::list<Order>`
- 🗂️ 一个键值配置 = `std::map<string, string>`
- 🎯 一个固定大小的矩阵行 = `std::array<float, 4>`

**Semantic-Cpp 为 10 种标准库容器** 提供了 `Semantic` 模板特化，让容器本身成为流中的一等公民：

| 特化类型                          | 说明               | 支持的操作                                     |
| :-------------------------------- | :----------------- | :--------------------------------------------- |
| `Semantic<std::vector<E>>`       | 向量容器流         | `map`、`filter`、`distinct`、`sort`、`flatMap`... |
| `Semantic<std::list<E>>`         | 链表容器流         | 同上                                           |
| `Semantic<std::deque<E>>`        | 双端队列容器流     | 同上                                           |
| `Semantic<std::set<E>>`          | 有序集合容器流     | 同上                                           |
| `Semantic<std::unordered_set<E>>`| 无序集合容器流     | 同上                                           |
| `Semantic<std::map<K,V>>`        | 有序映射容器流     | 同上                                           |
| `Semantic<std::unordered_map<K,V>>` | 无序映射容器流   | 同上                                           |
| `Semantic<std::queue<E>>`        | 队列容器流         | 同上                                           |
| `Semantic<std::stack<E>>`        | 栈容器流           | 同上                                           |
| `Semantic<std::array<E,N>>`      | 固定数组容器流     | 同上                                           |

所有容器特化都完整支持：`map`、`filter`、`takeWhile`、`dropWhile`、`distinct`、`sort`、`limit`、`skip`、`reverse`、`translate`、`redirect`、`sub`、`concatenate`、`peek`、`flatMap`、`flat`、`parallel` 以及全部终端转换方法。

**容器流的妙用**

```cpp
// 场景：你有一批向量数据，想筛选出长度大于 2 的，然后按字典序排序
auto result = semantic::useOf({
    std::vector<int>{3, 1, 2},
    std::vector<int>{1, 2},
    std::vector<int>{5, 4, 0}
}).filter(std::vector<int> v { return v.size() > 2; })
  .sort()
  .toVector();

// 场景：展开容器流——把多个向量展平成一个元素流
auto flattened = semantic::useOf({
    std::vector<int>{1, 2},
    std::vector<int>{3, 4, 5}
}).flatMap(std::vector<int> v {
    return semantic::useFrom(v).map(int x { return x * 10; });
}).toOrdered().toVector();
// 输出: 10 20 30 40 50
```

这就像给每个容器装上了流式引擎——它们可以在管道中自由穿梭，按需拆分、合并、筛选、变换，最终汇聚成你想要的结果。🎯

---

**collectable 命名空间 API**

| 类                       | 描述                     | 关键方法                                                                                             |
| :----------------------- | :----------------------- | :--------------------------------------------------------------------------------------------------- |
| **Collectable<E>**         | 抽象基类                 | `count()`、`findFirst()`、`findAny()`、`anyMatch()`、`allMatch()`、`noneMatch()`、`reduce()`、`join()`、`out()`、`error()`、`group()`、`partition()`、`partitionBy()`、`toVector()`、`toList()`、`toSet()`、`toMap()`... 共 30+ 终端方法 |
| **OrderedCollectable<E>**  | 有序收集                 | 按索引顺序遍历，支持自定义排序构造                                                                     |
| **UnorderedCollectable<E>**| 无序收集                 | 高性能 O(1) 查找                                                                                       |
| **WindowCollectable<E>**   | 窗口收集                 | `slide(size, step)`、`tumble(size)`                                                                  |
| **Statistics<E,D>**        | 统计收集                 | `summate()`、`average()`、`median()`、`variance()`、`standardDeviation()`、`skewness()`、`kurtosis()`、`mode()`、`percentile(p)`、`fft()`、`gradient()`... 共 30+ 统计方法 |

---

## 🏭 第七层：semantics.h — 流构建工厂
`semantics.h` 提供所有流构建工厂函数。如果说 `semantic.h` 是流的“大脑”（负责变换和调度），那么 `semantics.h` 就是流的“心脏”——一切数据之旅从此开始。

### 📐 数值范围生成 — 最经典的起点
从 `useRange(0, 10)` 到 `useRangeClosed(0, 10, 2)`，数值范围是流世界中最常用的入口。支持开区间、闭区间、正步长、负步长。每个元素都有独立的索引，确保在 `toOrdered()` 中按生成顺序排列。🚶♂️➡️🚶♀️➡️🚶

### ♾️ 无限流生成 — 永不枯竭的数据源泉
`useInfinite(seed, generator)`、`useGenerate(supplier)`、`useIterate(seed, generator)`、`useRandom()`……这些函数创造的是“无限流”——它们会一直产生数据，直到被 `limit()` 截断或被 `takeWhile()` 叫停。你可以生成斐波那契数列、随机数流、计数器序列——任何可以按规则递推的数据。无限流配合 `limit()` 使用，就像给永动机装上了刹车，让无穷变为有限。⚡

### 📦 容器与元素构建 — 万物皆可流
`useOf(element)` 从一个元素创建流，`useOf(e1, e2, e3)` 从最多三个元素创建流，`useOf({...})` 从初始化列表创建流。`useFrom(container)` 可以将任何标准容器（`vector`、`list`、`set`……）拆解为元素流。`useRepeat(element, count)` 则像一个复读机，把同一个元素重复 n 次。📢

### 📝 文本处理 — 让字符串流动起来
`useBlob(text)` 将字符串按字节拆分为 `char` 流，适合底层二进制处理。`useText(text)` 则将整个字符串作为一个整体放入流中（类型为 `Charsequence`），而 `useText(text, delimiter)` 按分隔符拆分文本——内部使用 `Charsequence::split` 实现，正确处理 Unicode 编码。同样的操作也支持 `istream` 输入流。📜

### 🌐 Unicode 处理 — 字符的“原子”视角
`useSequence(charsequence)` 将字符序列拆解为 Unicode 码点（`Point`） 流——每一个元素代表一个完整的 Unicode 字符，包括代理对正确处理。`useCharsequence(charsequence)` 则将整个字符序列作为整体放入流中。支持从 `istream` 读取并指定编码，覆盖 UTF‑8、UTF‑16、UTF‑32 等多种编码。这让你可以在字符级别精确操控文本数据，而不必担心多字节编码的边界问题。🔤

---

## 📦 第八层：hash.h / less.h — 容器世界的通用语言
当你在 `Semantic<std::vector<E>>` 上调用 `distinct()`，它内部需要 `std::unordered_set<std::vector<E>>`。标准库没有为 `std::vector` 提供哈希特化——这是 C++ 标准委员会留下的“空白地带”。**Semantic-Cpp 的 hash.h 和 less.h 填补了这一切**：为所有标准库容器（包括嵌套容器）、`pair`、`tuple`、`optional`、`variant`、`chrono` 时间类型、`complex` 复数等提供了完整的哈希和比较支持。任意深度、任意组合的嵌套容器现在都可以作为 `unordered_set` 的键或 `set` 的元素。🧩

---

## ⚡ 性能优化建议

1.  **🎯 选择对的容器**：不关心顺序用 `toUnordered()`，需要排序用 `toOrdered()` 或 `sort()`
2.  **⚡ 善用并行**：数据量大时使用 `parallel()`，避免阻塞式 I/O
3.  **📐 优化操作顺序**：尽早 `filter`，明智 `sort`
4.  **🔄 利用惰性求值**：`takeWhile` 和 `limit` 可提前终止，节省计算

---

**Semantic-Cpp** — 用现代 C++ 构建高效、清晰的数据处理管道。🚀
