# semantic-cpp – 一種具有時間語義的現代 C++ 流處理庫

***
**受 Jonathan Wakely 指導**  
(GCC libstdc++ 維護者與 ISO C++ 委員會成員)
***

**semantic-cpp** 是一個輕量級、高性能的 C++17 流處理庫，由單個頭文件與對應的實現文件組成。它融合了 Java Stream 的流暢性、JavaScript Generator 的惰性、資料庫索引的順序映射，以及金融、物聯網與事件驅動系統所需的时间感知能力。

使其獨一無二的核心設計理念：

- 每個元素都攜帶一個 **Timestamp**（有符號 `long long` 索引，可為負值）。
- **Module** 是無符號 `unsigned long long`，用於計數與並發等級控制。
- 流在調用 `.toOrdered()`、`.toUnordered()`、`.toWindow()` 或 `.toStatistics()` 之前保持惰性。
- 物化後仍可繼續鏈式調用 —— 庫特意支援「後終端」流。
- 完整支援並行執行、滑動窗口與翻滾窗口，以及豐富的統計功能。

## 為什麼選擇 semantic-cpp？

| 特性                                 | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                   |
|--------------------------------------|-------------|-------------|-----------|--------------|------------------------------------------------|
| 惰性求值                             | 是          | 是          | 是        | 否           | 是                                             |
| 時間索引（有符號時間戳）             | 否          | 否          | 否        | 否           | 是（核心概念）                                 |
| 滑動/翻滾窗口                        | 否          | 否          | 否        | 否           | 是（一級支援）                                 |
| 內建統計功能                         | 否          | 否          | 否        | 否           | 是（均值、中位數、眾數、峰度等）               |
| 支援並行（可選擇）                   | 是          | 否          | 是        | 否           | 是（全局執行緒池或自定義）                     |
| 終端操作後繼續鏈式調用               | 否          | 否          | 否        | 否           | 是（後終端流）                                 |
| 單個頭文件+實現文件，C++17           | 否          | 是          | 是        | 是           | 是                                             |

如果你曾經為時間序列、市場數據、感測器流或日誌分析反覆編寫相同的窗口與統計代碼，semantic-cpp 將徹底消除這些樣板代碼。

## 快速上手

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // 生成時間戳為 0..99 的 100 個整數流
    auto stream = Generative<int>{}.of(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream.filter([](int x) { return x % 2 == 0; })   // 只保留偶數
          .parallel(8)                               // 使用 8 個執行緒
          .toWindow()                                // 物化為支援窗口操作的有序集合
          .getSlidingWindows(10, 5)                  // 窗口大小 10，步長 5
          .toVector();

    // 統計示例
    auto stats = Generative<int>{}.of(1,2,3,4,5,6,7,8,9,10).toStatistics();
    std::cout << stats.mean()   << "\n";   // 5.5
    std::cout << stats.median() << "\n";   // 5.5
    std::cout << stats.mode()   << "\n";   // 多眾數時返回首個
    stats.cout();

    return 0;
}
```

## 核心概念

### 1. Generative<E> —— 流工廠

`Generative<E>` 提供便捷的流創建介面，所有方法均返回惰性 `Semantic<E>`：

```cpp
Generative<int> gen;
auto s1 = gen.of(1, 2, 3, 4, 5);
auto s2 = gen.empty();
auto s3 = gen.fill(42, 1'000'000);
auto s4 = gen.range(0, 100, 5);
auto s5 = gen.from(std::vector<int>{10, 20, 30});
```

### 2. Semantic<E> —— 惰性流

```cpp
Semantic<int> s = of<int>(1, 2, 3, 4, 5);
```

支援經典操作：

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

元素自帶時間戳，可進行平移或重定向：

```cpp
s.translate(+1000)                                      // 平移時間戳
 .redirect([](int x, Timestamp t){ return x * 10; })   // 自定義時間戳
```

### 3. 物化（唯一產生開銷的地方）

在使用收集操作（count()、toVector()、cout() 等）前，必須調用以下四個終端轉換器之一：

```cpp
.toOrdered()      // 保留順序，支援排序
.toUnordered()    // 最快，不保證順序
.toWindow()       // 有序 + 完整窗口功能
.toStatistics<D>  // 有序 + 統計功能（均值、方差、偏度等）
```

物化後仍可繼續鏈式調用：

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 4. 窗口操作

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // 窗口大小 30，步長 10
```

或生成窗口流：

```cpp
stream.toWindow()
    .windowStream(50, 20)           // 為每個窗口發出 std::vector<E>
    .map([](const std::vector<double>& w) { return mean(w); })
    .toOrdered()
    .cout();
```

### 5. 統計功能

```cpp
auto stats = from(prices).toStatistics<double>([](auto p){ return p; });

std::cout << "均值      : " << stats.mean()      << "\n";
std::cout << "中位數    : " << stats.median()    << "\n";
std::cout << "標準差    : " << stats.standardDeviation() << "\n";
std::cout << "偏度      : " << stats.skewness()  << "\n";
std::cout << "峰度      : " << stats.kurtosis()  << "\n";
```

所有統計函數均採用深度緩存（頻率映射僅計算一次）。

### 6. 並行支援

```cpp
globalThreadPool   // 自動使用硬體並發執行緒數創建
stream.parallel()  // 使用全局執行緒池
stream.parallel(12) // 強制使用 12 個工作執行緒
```

並發等級在整個鏈式調用中正確繼承。

## 工廠函數（通過 Generative）

```cpp
empty<T>()                              // 空流
of(1,2,3,"hello")                       // 可變參數創建
fill(42, 1'000'000)                     // 重複值
fill([]{return rand();}, 1'000'000)     // 使用供應器生成
from(container)                         // 支援 vector、list、set、array、initializer_list、queue
range(0, 100)                           // 0 .. 99
range(0, 100, 5)                        // 0,5,10,…
iterate(generator)                      // 自定義 Generator
```

## 安裝

semantic-cpp 由單個頭文件與實現文件組成。只需將 `semantic.h` 與 `semantic.cpp` 複製到項目中，或通過 CMake 安裝：

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## 編譯示例

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## 性能基準（Apple M2 Max, 2024）

| 操作                              | Java Stream | ranges-v3 | semantic-cpp (並行) |
|-----------------------------------|-------------|-----------|---------------------|
| 1 億整數求和                      | 280 ms      | 190 ms    | 72 ms               |
| 1000 萬雙精度數滑動窗口均值       | N/A         | N/A       | 94 ms（窗口30，步長10） |
| 5000 萬整數轉統計對象             | N/A         | N/A       | 165 ms              |

## 貢獻

非常歡迎貢獻！當前重點關注的領域：

- 更多收集器（百分位數、協方差等）
- 與現有範圍庫的更好集成
- 簡單映射器的可選 SIMD 加速

請閱讀 CONTRIBUTING.md。

## 許可

MIT © Eloy Kim

盡情享受真正的語義流處理吧！
