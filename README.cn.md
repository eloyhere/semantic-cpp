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
- Collector 收集器

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

## 为什么选择semantic-cpp? (基于索引概念的革命)

- **redirect()**: 惰性索引化声明 — 无消耗修改索引元素关系
- **reindex()**: 物化建立索引一次 — 即可让sorted/distinct复杂度为O(1)
- 少量数据 (<32768 元素): 立即建立索引. 大数据: 不建立索引

```cpp
fromUnordered(huge_data)  // 无索引和顺序
    .redirect([](auto e, auto i){ return e.key; })  // 重定向/排序/去重均无效
    .filter(...)
    .reindex()  // 建立索引
    .sorted()   // O(1)!
    .toVector();
```
许可证

MIT License
