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

## 🏗️ 项目架构：四模块分层设计

Semantic-Cpp 按职责划分为四个模块目录，每个头文件职责单一、独立可测；从编码与 bunch 原语开始，逐层构建到流处理 API：

```
semantic-cpp/
├── structure/          基础层：类型别名、线程池、数据结构
│   ├── function.h
│   ├── pool.h
│   ├── hash.h
│   ├── less.h
│   ├── node.h          树节点模板
│   ├── tree.h          二叉搜索树
│   ├── ast.h           抽象语法树
│   ├── trie.h          前缀树（trie）
│   ├── segment_tree.h  线段树
│   └── character_search_tree.h  三叉字符搜索树
├── bunch/              Unicode 字符、编码与字符串
│   ├── charset.h       编码选择器
│   ├── encode.h        编解码（UTF-8/16/32、ASCII、Latin1）
│   ├── meta.h          Meta
│   ├── point.h         Point / PointIterator
│   ├── character.h     Character
│   ├── unicode_data.h  生成的 Unicode 15 数据表
│   ├── unicode.h       规范化（NFC/NFD/NFKC/NFKD）与文本样式
│   ├── sequence.h      Bunch
│   ├── builder.h       Builder
│   ├── buffer.h        Buffer
│   └── bunch.h         聚合头文件
├── localisation/       消息本地化
│   ├── localisation.h
│   └── unicode.md      Unicode 概念与纠错说明
└── semantic/           流构建与收集
    ├── collector.h
    └── semantic.h
```

### 🧩 依赖关系
每一层只依赖它下方的层级。编码与值类型相互独立，bunch 类型构建在其上，流 API 再构建在 bunch 与基础模块之上：

```
structure/function.h       ← 无依赖，类型基石
structure/pool.h           ← structure/function.h
structure/hash.h, less.h   ← 标准库扩展，无内部依赖
structure/node.h           ← 树节点模板，无内部依赖
structure/tree.h           ← structure/node.h（二叉搜索树）
structure/ast.h            ← 仅依赖标准库（自持有 AST 节点）
structure/trie.h           ← structure/node.h（前缀树）
structure/segment_tree.h   ← 仅依赖标准库（迭代式线段树）
structure/character_search_tree.h ← structure/node.h（三叉字符搜索树）
bunch/charset.h            ← 编码原语
bunch/encode.h             ← bunch/charset.h（编解码）
bunch/meta.h               ← 应用元数据，无内部依赖
bunch/point.h              ← Point / PointIterator，无内部依赖
bunch/character.h          ← bunch/meta.h、bunch/point.h
bunch/unicode_data.h       ← 生成的 Unicode 15 数据表，无内部依赖
bunch/unicode.h            ← bunch/charset.h、bunch/encode.h、bunch/unicode_data.h
bunch/sequence.h           ← charset.h、encode.h、meta.h、point.h
bunch/builder.h            ← bunch/point.h、bunch/sequence.h
bunch/buffer.h             ← bunch/sequence.h、bunch/builder.h
bunch/bunch.h              ← 聚合全部 bunch 头
localisation/localisation.h ← bunch 头（消息本地化与文本样式）
semantic/collector.h       ← structure/function.h、structure/pool.h、bunch/bunch.h
semantic/semantic.h        ← 以上全部
```

---

## 🌍 命名空间全景

Semantic-Cpp 精心设计了多个命名空间，每个都像一个独立的“部门”，各司其职又紧密协作：

| 命名空间    | 所在文件                                | 职责                                   | 核心类型/函数                                                                  |
| :---------- | :-------------------------------------- | :------------------------------------- | :----------------------------------------------------------------------------- |
| function    | `structure/function.h`                  | 类型系统基石                           | `Timestamp`、`Module`、`Generator<T>`、`Supplier<R>`、`Consumer<T>`、`Predicate<T>` 等 |
| pool        | `structure/pool.h`                      | 并发执行引擎                           | `pool::ThreadPool`、`submit()`、`emergencyShutdown()`                           |
| node        | `structure/node.h`                      | 可复用的树节点模板                     | `BinaryNode<T>`、`TrieNode<Char,T>`、`TernaryNode<Char,T>`                       |
| tree        | `structure/tree.h`                      | 有序二叉搜索树                         | `tree::BinarySearchTree<T>`、`insert()`、`erase()`、`inorder()` 等               |
| ast         | `structure/ast.h`                       | 抽象语法树                             | `ast::Tree<Kind,Value>`、`ast::Node<Kind,Value>`、`SourceRange`、遍历与查询      |
| trie        | `structure/trie.h`                      | 字符序列前缀树                         | `trie::Trie<Char,Value>`、`keysWithPrefix()`、`longestPrefixOf()`                |
| segment     | `structure/segment_tree.h`              | 点更新与区间查询                       | `segment::SegmentTree<T,Operation>`、`query()`、`set()`、`update()`              |
| character   | `structure/character_search_tree.h`     | 字节字符串三叉搜索树                   | `character::CharacterSearchTree<Value>`、`keysWithPrefix()`、`match()`           |
| localisation | `localisation/localisation.h`          | 消息本地化与文本样式                   | `Locale`、`Catalog`、`Message`、`Arguments`、`TextFormat`、复数/数字格式化       |
| bunch       | `bunch/bunch.h`                        | Unicode 字符、编码与字符串             | `charset`、`Character`、`Meta`、`Point`、`PointIterator`、`Bunch`、`Builder`、`Buffer`、`NormalizationForm`、`TextStyle` |
| collector   | `semantic/collector.h`                  | 终端收集执行                           | `Collector<E,A,R>`、`Identity<A>`、`Accumulator<A,E>` 等                        |
| collectable | `semantic/semantic.h`                   | 物化数据容器                           | `Collectable<E>`、`OrderedCollectable<E>`、`UnorderedCollectable<E>` 等         |
| semantic    | `semantic/semantic.h`                   | 流构建与中间操作                       | `Semantic<E>`、`useRange()`、`useFrom()` 等                                      |

### 🔁 命名空间协作流程
数据在命名空间之间的流转，就像工厂里的流水线——原料从 `semantic` 进入，经过层层加工，最终在 `collector` 包装出厂。每一步都有明确的职责边界：

```cpp
semantic::useRange(0, 100)          // ← semantic 命名空间：创建流
    .map([](int x) { return x * 2; })   // ← semantic 命名空间：中间变换
    .filter([](int x) { return x > 50; }) // ← semantic 命名空间：中间过滤
    .toUnordered()                  // ← 转换为 collectable 命名空间
    .toVector();                    // ← 调用 collector 命名空间的收集器
```

---

## 🛠️ 构建、测试与安装

**环境要求：** CMake 3.21+ 与 C++17 编译器（MSVC 2019+、GCC 9+、Clang 10+）。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSEMANTIC_CPP_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

安装纯头文件库及 CMake 包配置：

```bash
cmake --install build --config Release --prefix <安装前缀>
```

在其他 CMake 项目中使用：

```cmake
find_package(semantic-cpp CONFIG REQUIRED)
target_link_libraries(app PRIVATE semantic-cpp::semantic-cpp)
```

```cpp
#include <semantic-cpp/semantic/semantic.h>
#include <semantic-cpp/bunch/bunch.h>
```

在本仓库内开发时，按仓库根目录的相对路径包含头文件（例如 `#include "semantic/semantic.h"`）。测试套件会逐个独立编译所有公开头文件，因此每个头文件都自带完整依赖。

---

## 📦 第一层：structure/function.h — 类型基础

`structure/function.h` 定义了整个框架的类型系统，是所有模块的共同基石。🔑

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

## 🧵 第二层：structure/pool.h — 并发基石

`structure/pool.h` 提供全局线程池 `pool::ThreadPool`，是整个框架的并发引擎。🚀 它采用**声明式并行**设计——当你写下 `.parallel(4)` 时，并没有立即启动 4 个线程开始处理。这行代码仅仅是一个“声明”：告诉框架“我打算用 4 个线程来并行处理”。真正的并行执行发生在终端操作被调用时——也就是当你调用 `toVector()`、`findFirst()`、`count()` 等收集方法的那一刻。

| 特性         | 说明                                                   |
| :----------- | :----------------------------------------------------- |
| 声明式并行   | `.parallel(4)` 只声明“我想用 4 个线程”，不立即启动     |
| 紧急关闭     | 内置 `emergencyShutdown()` 和 `std::set_terminate` 处理器 |
| 异常传播     | `submit()` 返回 `std::future`，异常安全地传播到主线程  |

---

## 🔤 第三层：bunch/ + localisation/ — Unicode 字符与字符串

`bunch/charset.h` 定义编码选择器，`bunch/encode.h` 实现编解码，`bunch/unicode.h` 提供 NFC/NFD/NFKC/NFKD 规范化、大小写映射与文本样式；`bunch/` 其余部分在其上构建码点、字符与不可变/可变序列。🌍 共同支持 UTF-8、UTF-16（LE/BE）、UTF-32（LE/BE）、ASCII 和 Latin1；无效序列替换为 U+FFFD。精确术语与当前实现限制见 [Unicode 概念与纠错](localisation/unicode.md)。`localisation/localisation.h` 复用这些原语实现按 locale 的消息处理。

| 类型/函数       | 所在头文件                      | 描述                                                                     |
| :-------------- | :------------------------------ | :----------------------------------------------------------------------- |
| charset         | `bunch/charset.h`               | 编码/编码方案选择器：ascii、utf8、utf16、utf16be、utf16le、utf32、utf32be、utf32le、latin1 |
| encode()        | `bunch/encode.h`                | 将单个码点编码为指定编码的码元                                           |
| decode()        | `bunch/encode.h`                | 从码元解码下一个码点，自动推进输入                                       |
| convert()       | `bunch/encode.h`                | 编码转换（支持 string、vector、deque 输出）                               |
| NormalizationForm | `bunch/unicode.h`             | NFC/NFD/NFKC/NFKD 形式选择器                                              |
| normalize()     | `bunch/unicode.h`               | Unicode 规范化：分解、规范排序、重组                                     |
| TextStyle       | `bunch/unicode.h`               | plain / lowercase / uppercase / capitalize / title                        |
| TextFormat      | `localisation/localisation.h`   | 绑定 Locale 的文本样式与可选规范化                                        |
| Meta            | `bunch/meta.h`                  | 应用元数据包装器；不是 Unicode 属性                                      |
| Point           | `bunch/point.h`                 | Unicode 码点数值，提供标量值/非字符/代理项判断                           |
| PointIterator   | `bunch/point.h`                 | 码点双向迭代器                                                           |
| Character       | `bunch/character.h`             | 码点 + 应用元数据                                                        |
| Bunch           | `bunch/sequence.h`              | 不可变码点序列：split、replace、indexOf、lastIndexOf、sub、trim、toUpperCase、toLowerCase、reverse、startsWith、endsWith、contains、compare、getBytes、getPoints、getMetas、getCharacters、repeat、concat、count、join |
| Builder         | `bunch/builder.h`               | 可变字节构建器：prepend、insert、append（支持基本类型、Point、Bunch、string_view）、toBunch() |
| Buffer          | `bunch/buffer.h`                | 线程安全字节环形缓冲区：write、read、peek、prepend、append、clear、shrinkToFit、data、size、capacity、atomic |

---

## ⚙️ 第四层：semantic/collector.h — 收集器框架与工厂

`semantic/collector.h` 是 Semantic-Cpp 的收集器核心模块，将收集器框架与工厂函数合二为一。

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
| `useJoin()` / `useOut()` / `useError()` 及其各种重载 | `bunch::Bunch` |

#### 📊 数学工具
| 方法             | 返回类型 |
| :--------------- | :------- |
| `useDFT()`       | `vector<complex<double>>` |
| `useIDFT()`      | `vector<complex<double>>` |
| `useFFT()`       | `vector<complex<double>>` |
| `useIFFT()`      | `vector<complex<double>>` |
| `useGradient()`  | `vector<double>` |

---

## 📦 第五层：semantic/semantic.h — 流式中间操作与收集体系

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
| `join()`                                            | `Bunch`               | 默认格式连接                             |
| `join(delimiter)`                                   | `Bunch`               | 自定义分隔符连接                         |
| `join(prefix, delimiter, suffix)`                  | `Bunch`               | 完全自定义格式连接                       |
| `noneMatch(predicate)`                              | `bool`                       | 没有元素满足条件                         |
| `out()`                                             | `Bunch`               | 输出到 stdout（支持 delimiter/prefix/suffix/converter） |
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

## 🔧 第六层：semantic/semantic.h — 流构建工厂

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
| `useText(text)`             | 整体文本流（Bunch）   |
| `useText(text, delimiter)`  | 按分隔符拆分文本             |
| `useText(istream)`          | 从输入流读取整个内容         |
| `useSequence(string)` | 从字符序列创建码点流         |
| `useSequence(text, encoding)` | 从文本创建指定编码码点流   |
| `useBunch(bunch)` | 码点序列作为整体流       |
| `useBunch(bunch, delimiter)` | 按分隔符拆分码点序列 |

---

## 🔐 第七层：structure/hash.h / structure/less.h — 容器世界的通用语言

为所有标准库容器（包括嵌套容器）、pair、tuple、optional、variant、chrono 时间类型、complex 复数等提供完整的哈希和比较支持。任意深度、任意组合的嵌套容器现在都可以作为 unordered_set 的键或 set 的元素。🌉

---

## 🚀 性能优化建议

1.  **选择对的容器**：不关心顺序用 `toUnordered()`，需要排序用 `toOrdered()` 或 `sort()`
2.  **善用并行**：数据量大时使用 `parallel()`
3.  **优化操作顺序**：尽早 filter，明智 sort
4.  **利用惰性求值**：`takeWhile` 和 `limit` 可提前终止

---

Semantic-Cpp — 用现代 C++ 构建高效、清晰的数据处理管道。🚀🎯✨
