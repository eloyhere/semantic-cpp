# semantic-cpp —— 現代 C++ 時序流處理庫（支援時間戳語義）

**semantic-cpp** 是一款純標頭檔（header-only）、高效能的 C++17 流處理函式庫，融合了 Java Stream 的流暢寫法、JavaScript Generator 的惰性求值、MySQL 索引的有序映射，以及金融、物聯網、事件驅動系統所需的**時序感知**能力。

真正與眾不同的核心設計：

- 每個元素都原生攜帶 **Timestamp**（有號 `long long`，允許負數）  
- **Module** 為無號整數，用於計數與並行度控制  
- 流在呼叫 `.toOrdered()` / `.toUnordered()` / `.toWindow()` / `.toStatistics()` 之前完全惰性  
- 終端操作後仍可繼續鏈式呼叫 —— 刻意設計為「**後終端流**」（post-terminal）友好  
- 原生支援並行執行、滑動窗／翻滾窗（sliding/tumbling window）、豐富統計收集器  

## 為什麼選擇 semantic-cpp？

| 特性                                 | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                 |
|--------------------------------------|-------------|-------------|-----------|--------------|----------------------------------------------|
| 惰性求值                             | Yes         | Yes         | Yes       | No           | Yes                                          |
| 時序索引（有號時間戳）               | No          | No          | No        | No           | Yes（核心概念）                              |
| 滑動窗／翻滾窗                        | No          | No          | No        | No           | Yes（原生支援）                              |
| 內建統計收集器                       | No          | No          | No        | No           | Yes（平均值、中位數、眾數、峰度、偏度…）    |
| 預設並行（可選）                     | Yes         | No          | Yes       | No           | Yes（全域執行緒池或自訂）                    |
| 終端操作後仍可繼續鏈式處理           | No          | No          | No        | No           | Yes（後終端流）                              |
| 純標頭檔、C++17                      | No          | Yes         | Yes       | Yes          | Yes                                          |

如果你曾經為行情資料、感測器串流、日志分析反覆撰寫窗口與統計程式碼，**semantic-cpp** 能幫你徹底擺脫這些樣板程式碼。

## 快速上手

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // 產生時間戳 0~99 的 100 個整數流
    auto stream = from(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream
        .filter([](int x) { return x % 2 == 0; })     // 只保留偶數
        .parallel(8)                                  // 使用 8 個執行緒
        .toWindow()                                   // 物化為有序集合，啟用窗口功能
        .getSlidingWindows(10, 5)                     // 窗口大小 10，步長 5
        .toVector();                                  // 轉為 vector<vector<int>>

    // 統計範例
    auto stats = from({1,2,3,4,5,6,7,8,9,10})
                  .toStatistics<double>();        // mapper 自動推導為 identity

    std::cout << "平均值   : " << stats.mean()            << '\n';
    std::cout << "中位數   : " << stats.median()          << '\n';
    std::cout << "眾數     : " << stats.mode()            << '\n';
    std::cout << "標準差   : " << stats.standardDeviation() << '\n';
    stats.cout();  // 一鍵美觀輸出

    return 0;
}
```

## 核心概念

### 1. `Semantic<E>` —— 惰性流

```cpp
Semantic<int> s = of(1, 2, 3, 4, 5);
```

支援所有經典操作：

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

每個元素都會附帶 **Timestamp**（有號索引），可平移或完全重新導向時間戳：

```cpp
s.translate(+1000)                                   // 整體平移時間戳
   .redirect([](int x, Timestamp t){ return x * 10; }) // 自訂時間戳
```

### 2. 物化（Materialisation）—— 唯一需要「付費」的地方

必須先呼叫以下四個終端轉換器之一，才能使用 `count()`、`toVector()`、`cout()` 等收集操作：

```cpp
.toOrdered()      // 保留原始順序，支援排序
.toUnordered()    // 最快，無序
.toWindow()       // 有序 + 強大窗口 API
.toStatistics<D>()// 有序 + 統計方法（平均、變異數、偏度、峰度…）
```

物化後仍可繼續鏈式呼叫：

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 3. 窗口操作

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // 窗口大小 30，步長 10
```

也可直接產生窗口流：

```cpp
stream.toWindow()
      .windowStream(50, 20)           // 每個元素為 std::vector<E>
      .map([](const std::vector<double>& w) { return mean(w); })
      .toOrdered()
      .cout();
```

### 4. 統計功能

```cpp
auto stats = from(prices)
              .toStatistics<double>([](double p){ return p; });

std::cout << "平均值     : " << stats.mean()      << "\n";
std::cout << "中位數     : " << stats.median()    << "\n";
std::cout << "標準差     : " << stats.standardDeviation() << "\n";
std::cout << "偏度       : " << stats.skewness()  << "\n";
std::cout << "峰度       : " << stats.kurtosis()  << "\n";
```

所有統計函式皆深度快取（頻率表僅計算一次）。

### 5. 並行

```cpp
globalThreadPool          // 程式啟動時自動建立，執行緒數 = hardware_concurrency
stream.parallel()         // 使用全域執行緒池
stream.parallel(12)       // 強制使用 12 個工作執行緒
```

每個 `Collectable` 都攜帶自己的並行度，整條鏈會正確繼承。

## 工廠函式

```cpp
empty<T>()                              // 空流
of(1,2,3,"你好")                        // 可變參數建構
fill(42, 1'000'000)                     // 一百萬個 42
fill([]{return rand();}, 1'000'000)     // 動態產生
from(container)                         // 支援 vector/list/set/array/initializer_list
range(0, 100)                           // 0..99
range(0, 100, 5)                        // 0,5,10,…
iterate(custom_generator)               // 自訂 Generator
```

## 安裝

純標頭檔，直接複製 `semantic.h` 到專案即可，也支援 CMake：

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## 編譯範例

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp -o basic
./basic
```

## 效能基準（Apple M2 Max，2024 年）

| 運算                                    | Java Stream | ranges-v3 | semantic-cpp（並行） |
|-----------------------------------------|-------------|-----------|----------------------|
| 1 億整數求和                            | 280 ms      | 190 ms    | 72 ms                |
| 1000 萬 double 滑動窗口平均（30 步 10） | N/A         | N/A       | 94 ms                |
| 5000 萬整數 toStatistics                | N/A         | N/A       | 165 ms               |

## 歡迎貢獻

非常歡迎貢獻！目前特別需要的方向：

- 更多收集器（百分位數、共變異數等）
- 與現有 range 函式庫更好的互通性
- 對簡單 mapper 的可選 SIMD 加速

請閱讀 CONTRIBUTING.md。

## 授權

MIT © Eloy Kim

祝你用 semantic-cpp 玩轉真正的「帶時間語義」的 C++ 流！
