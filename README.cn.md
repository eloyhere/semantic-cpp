# Semantic - C++ 流式处理库

## 概述

Semantic 是一个受 JavaScript 生成器，MySQL索引和 Java Stream API 启发的 C++ 流式处理库，提供了函数式编程风格的惰性求值数据流操作。

- 核心特性

  - 惰性求值：所有操作都是惰性的，只有在终端操作时才会执行
  - 函数式编程：支持高阶函数、lambda 表达式和函数组合
  - 类型安全：基于模板的强类型系统
  - 缓存优化：统计操作自动缓存计算结果
  - 多种数据源：支持数组、容器、生成器等多种数据源

## 快速开始

- 基本用法
```c++
#include "semantic.h"

int main() {
    // 从数组创建流
    int data[] = {1, 2, 3, 4, 5};
    auto stream = semantic::from(data, 5);
    
    // 链式操作
    stream.filter([](int x) { return x % 2 == 0; })
          .map([](int x) { return x * 2; })
          .cout();  // 输出: 4 8
    
    return 0;
}
```
## 核心组件

### Semantic 流类

主要流处理类，提供丰富的中间操作和终端操作。

  - 创建流
    ```c++
    // 空流
    auto emptyStream = semantic::empty<int>();

    auto unorderedStream = semantic::fromUnordered<int>({1,2,3,4,5})// 创建一个无索引的流, 在调用reindex方法之前，redirect, distinct, sorted, reverse, translate,shuffle均无效。
    .redirect([](const int& element, const auto& index)-> auto{
        return -index;
    }) // 无效。
    .distinct() // 无效。
    .cout(); // [1,2,3,4,5];

    auto orderedStream = semantic::fromOrdered<int>({1,2,3,4,5}) // 创建一个可索引的流, redirect, distinct, sorted, reverse, translate, shuffle方法均有效，只有通过fromOrdered创建的流，以及调用过reindex的流可以正常使用上述方法。
    .redirect([](const int& element, const auto& index)-> auto{
        return -index; // 以逆序反转semantic。
    }).redirect([](const int& element, const auto& index)-> auto{
        return index + 3; // 把所有元素整体移动，正数右移，尾部的三个元素放到整体元素头部，负数左移，头部的元素放到尾部。
    }).cout(); //[3,2,1,5,4]

    // 从值创建
    auto single = semantic::of(42);
    auto multiple = semantic::of(1, 2, 3, 4, 5);

    // 从容器创建
    std::vector<int> vec = {1, 2, 3};
    auto fromVec = semantic::from(vec);
    auto fromList = semantic::from(std::list{1, 2, 3});

    // 从数组创建
    int arr[] = {1, 2, 3};
    auto fromArray = semantic::from(arr, 3);

    // 数值范围
    auto rangeStream = semantic::range(1, 10);      // 1到9
    auto stepStream = semantic::range(1, 10, 2);    // 1,3,5,7,9

    // 生成流
    auto generated = semantic::fill(42, 5);          // 5个42
    auto randomStream = semantic::fill([]{ return rand() % 100; }, 10);
    ```
  - 中间操作

    - 过滤操作
        ```c++
        semantic.filter(predicate)          // 过滤元素
        semantic.distinct()                 // 去重
        semantic.distinct(comparator)       // 自定义去重
        semantic.limit(n)                   // 限制数量
        semantic.skip(n)                    // 跳过前n个
        semantic.takeWhile(predicate)       // 取满足条件的连续元素
        semantic.dropWhile(predicate)       // 丢弃满足条件的连续元素
        ```
    - 转换操作
        ```c++
        semantic.map(mapper)                // 元素转换
        semantic.flatMap(mapper)            // 扁平化映射
        semantic.sorted()                   // 自然排序
        semantic.sorted(comparator)        // 自定义排序
        semantic.reindex(indexer)          // 重新索引
        semantic.reverse()                 // 逆序
        semantic.shuffle()                 // 随机重排
        ```
    - 调试操作
        ```c++
        semantic.peek(consumer)            // 查看元素但不修改流
        ```
  - 终端操作

    - 匹配检查
        ```c++
        semantic.anyMatch(predicate)       // 任意元素匹配
        semantic.allMatch(predicate)       // 所有元素匹配  
        semantic.noneMatch(predicate)      // 没有元素匹配
        ```
    - 查找操作
        ```c++
        semantic.findFirst()               // 查找第一个元素
        semantic.findAny()                 // 查找任意元素
        ```
    - 归约操作
        ```c++
        semantic.reduce(accumulator)       // 归约操作
        semantic.reduce(identity, accumulator) // 带初始值的归约
        ```
    - 收集操作
        ```c++
        semantic.toVector()               // 转换为vector
        semantic.toList()                 // 转换为list
        semantic.toSet()                  // 转换为set
        semantic.toMap(keyMapper, valueMapper) // 转换为map
        semantic.collect(collector)       // 自定义收集
        ```
    - 分组分区
        ```c++
        semantic.group(classifier)        // 按分类器分组
        semantic.partition(n)            // 按大小分区
        ```
    - 输出操作
        ```c++
        semantic.cout()                  // 输出到标准输出
        semantic.forEach(consumer)       // 对每个元素执行操作
        ```

### Statistics 统计类

提供丰富的统计计算功能，支持缓存优化。
  ```c++
  std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
  auto stats = semantic::Statistics<double, double>(data);
    // 基本统计
  auto count = stats.count();           // 数量
  auto sum = stats.sum();               // 总和
  auto mean = stats.mean();             // 均值
  auto min = stats.minimum();           // 最小值
  auto max = stats.maximum();           // 最大值
    // 离散统计
  auto variance = stats.variance();     // 方差
  auto stdDev = stats.standardDeviation(); // 标准差
  auto range = stats.range();           // 极差
  // 高级统计
  auto median = stats.median();         // 中位数
  auto mode = stats.mode();             // 众数
  auto quartiles = stats.quartiles();   // 四分位数
  auto skewness = stats.skewness();     // 偏度
  auto kurtosis = stats.kurtosis();     // 峰度
  // 频率分析
  auto frequency = stats.frequency();   // 频率分布
  ```
### Collector 收集器

  支持自定义收集策略。  
  ```c++  
  // 字符串连接收集器
  auto concatenator = semantic::Collector<std::string, std::string>(
      []() { return std::string(""); },
      [](std::string& acc, int value) { acc += std::to_string(value); },
      [](std::string a, std::string b) { return a + b; },
      [](std::string result) { return result; }
  );
  auto result = stream.collect(concatenator);
  ```  
## 高级特性

- 惰性求值示例
    ```c++
    auto stream = semantic::range(1, 1000)
        .filter([](int x) { 
            std::cout << "Filtering: " << x << std::endl;
            return x % 2 == 0; 
        })
        .map([](int x) {
            std::cout << "Mapping: " << x << std::endl;
            return x * 2;
        })
        .limit(3);  // 只处理前3个元素

    // 此时还没有执行，只有调用终端操作时才会执行
    auto result = stream.toVector();  // 开始执行
    ```
- 自定义生成器
    ```c++
    auto fibGenerator = [](const auto& consumer, const auto& interrupt, const auto& redirect) {
        int a = 0, b = 1;
        for (int i = 0; i < 10; ++i) {
            if (interrupt && interrupt(b)) break;
            if (consumer) consumer(b);
            int next = a + b;
            a = b;
            b = next;
        }
    };

    auto fibStream = semantic::iterate(fibGenerator);
    ```
## 性能特性

- 惰性求值：避免不必要的计算
- 缓存优化：统计结果自动缓存
- 零拷贝：尽可能使用引用避免拷贝
- 内存安全：智能指针管理资源

编译要求

- C++11 或更高版本
- 标准模板库支持

## 分区语义 —— 隐藏的超能力

在 `semantic-cpp` 中，`concat()`、`flat()`、`flatMap()` **不会**合并全局索引。  
它们保留每个源流各自的索引空间，等价于把每个拼接或展平的流视为一个独立的 **分区（partition）**。

这是有意设计，且极其强大。

### 实际含义

```cpp
auto s1 = of(1,2,3).reindex().reverse();     // [3,2,1]
auto s2 = of(4,5,6).reindex().reverse();     // [6,5,4]
auto s3 = of(7,8,9).reindex().reverse();     // [9,8,7]

auto merged = s1.concat(s2).concat(s3)
                 .flat();                    // 展平所有分区

merged.reverse().cout();     
// 输出：9 8 7  6 5 4  3 2 1
// → 每个分区内部独立逆序，然后顺序拼接
```

所有索引相关操作（`redirect`、`distinct`、`sorted`、`reverse`、`shuffle` 等）在通过 `concat` / `flat` / `flatMap` 组合的流中，仅作用于各自原始分区内部。

### 真实场景超能力

| 在 `flat()` / `flatMap()` 后的操作 | 实际效果 |
|------------------------------------|---------|
| `.sorted()`                        | 每个分组/分区内部独立排序 |
| `.distinct()`                      | 仅在每个分区内部去重 |
| `.reverse()`                       | 每个分组内部独立逆序 |
| `.redirect(...)`                   | 每个分区内部独立重新索引 |
| `.limit(n)` / `.skip(n)`           | 跨所有分区全局生效 |

### 常见用法

```cpp
// 分组排序（大数据经典模式）
logs_by_shard.flat().sorted().cout();   
// 每个分片内部按时间排序，全局局部有序

// 分组去重
events_by_node.flat().distinct().cout(); 
// 仅在每个节点内部去重

// 分组最新优先（按用户最新消息排序）
messages_by_user.flat().reverse().cout(); 
// 每个用户内部最新消息在前
```

### 需要全局索引时

若确实需要跨所有分区的统一索引：

```cpp
auto global = streams.flat().reindex();   // 重新建立全局索引
global.sorted();                          // 现在才是真正的全局排序
```

### 总结

`concat` / `flat` / `flatMap` + 索引机制 = **自动分区感知处理**。  
无需额外 API，无需手动分组，即可获得纯粹、可组合、分区局部语义。

这不是限制。  
这是零成本的内存级分布式计算。

## 为什么选择semantic-cpp? (基于索引概念的革命)

- **redirect()**: 声明索引元素映射
- **reindex()**: 建立索引，以启用redirect，distinct，sorted，reverse，translate，shuffle。
- 少量数据 (<可索引数量): 立即建立索引. 大数据: 不建立索引

```cpp
fromUnordered(huge_data)  // 无索引和顺序
    .reindex() //建立索引
    .redirect([](auto e, auto i){ return e.key; })  // 现在redirect，distinct，sorted，reverse，translate，shuffle均可正常操作数据。
    .filter(...)
    .sorted()   // O(1)!
    .toVector();
```
许可证

MIT License
