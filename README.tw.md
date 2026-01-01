# semantic-cpp – 一個現代 C++ 流處理庫，具備時間語義

**semantic-cpp** 是一個輕量級、高效能的現代 C++17 流處理庫。它僅由單一標頭檔（`semantic.h`）搭配獨立的實作檔（`semantic.cpp`）構成。該庫融合了 Java Stream 的流暢性、JavaScript Generator 與 Promise 的惰性評估、類似資料庫索引的排序能力，以及對金融應用、IoT 資料處理與事件驅動系統至關重要的內建時間意識。

## 主要特色

- 每個元素皆關聯一個 **Timestamp**（有號號的 `long long`，支援負值）以及一個 **Module**（無號號的 `long long`，用於計數與並行）。
- 流在透過 `.toOrdered()`、`.toUnordered()`、`.toWindow()` 或 `.toStatistics()` 實體化之前均為惰性評估。
- 實體化 **不** 會終止管線 — 之後仍可完整鏈式呼叫（「後終端」流）。
- 全面支援並行執行、滑動與翻滾視窗、先進統計運算，以及透過 JavaScript 風格的 **Promise** 類別進行非同步任務。

## 為何選擇 semantic-cpp？

| 功能                             | Java Stream | ranges-v3 | semantic-cpp                          |
|----------------------------------|-------------|-----------|---------------------------------------|
| 惰性評估                         | 是          | 是        | 是                                    |
| 時間索引（有號時間戳）           | 否          | 否        | 是（核心概念）                        |
| 滑動／翻滾視窗                    | 否          | 否        | 是（原生支援）                        |
| 內建統計運算                     | 否          | 否        | 是（平均、中位數、眾數、偏度、峰度等）|
| 並行執行（選擇性）               | 是          | 是        | 是（執行緒池 + Promise 支援）         |
| 終端操作後繼續鏈式呼叫           | 否          | 否        | 是（後終端流）                        |
| 非同步 Promise                   | 否          | 否        | 是（JavaScript 風格）                 |
| 單標頭 + 實作檔，C++17            | 否          | 是        | 是                                    |

若您經常為時間序列資料、市場行情、感測器流或日誌撰寫自訂視窗、統計或非同步程式碼，semantic-cpp 可省去這些樣板程式碼。

## 快速入門

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // 從值建立流（時間戳從 0 開始自動遞增）
    auto stream = of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    // 篩選偶數，並行執行，實體化以進行統計
    auto stats = stream
        .filter([](int x) { return x % 2 == 0; })
        .parallel(8)
        .toStatistics<int>();

    std::cout << "平均值: " << stats.mean() << '\n';      // 5
    std::cout << "中位數: " << stats.median() << '\n';  // 5
    std::cout << "眾數: " << stats.mode() << '\n';      // 任一偶數

    // 視窗範例
    auto windows = of(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
        .toWindow()
        .getSlidingWindows(5, 2);  // 大小 5，步長 2 的滑動視窗

    return 0;
}
```

## 核心概念

### 1. 工廠函式 – 建立流

透過 `semantic` 命名空間中的自由函式建立流：

```cpp
auto s = of(1, 2, 3, 4, 5);                          // 可變參數
auto empty = empty<int>();                          // 空流
auto filled = fill(42, 1'000'000);                   // 重複值
auto supplied = fill([]{ return rand(); }, 1'000'000);
auto ranged = range(0, 100);                        // 0 .. 99
auto ranged_step = range(0, 100, 5);                // 0, 5, 10, ...
auto from_vec = from(std::vector<int>{1, 2, 3});
auto iterated = iterate([](auto push, auto stop) {
    for (int i = 0; i < 100; ++i) push(i, i);       // 明確時間戳
});
```

另有 I/O 輔助函式：`lines(stream)`、`chunks(stream, size)`、`text(stream)` 等。

### 2. Semantic – 惰性流

核心類型 `Semantic<E>` 支援標準操作：

```cpp
stream.filter(...).map(...).skip(10).limit(100).peek(...)
```

時間戳操作：

```cpp
stream.translate(+1000)          // 位移所有時間戳
    .redirect([](auto elem, Timestamp t) { return t * 10; });
```

並行：

```cpp
stream.parallel()                // 使用預設執行緒數
    .parallel(12);               // 指定工作者執行緒數
```

### 3. 實體化

轉換為可收集形式：

- `.toOrdered()` – 保留順序，啟用排序
- `.toUnordered()` – 最快，無順序
- `.toWindow()` – 有序，完整視窗支援
- `.toStatistics<R>(mapper)` – 有序，統計方法

之後仍可鏈式呼叫：

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 4. 視窗處理

```cpp
auto windows = stream.toWindow().getSlidingWindows(30, 10);
auto tumbling = stream.toWindow().getTumblingWindows(50);
```

發出視窗流：

```cpp
stream.toWindow()
    .windowStream(50, 20)
    .map([](const std::vector<double>& w) { return mean(w); })
    .cout();
```

### 5. 統計

```cpp
auto stats = from(prices)
    .toStatistics<double>([](auto p) { return p; });

std::cout << "平均值:    " << stats.mean() << '\n';
std::cout << "中位數:    " << stats.median() << '\n';
std::cout << "標準差:    " << stats.standardDeviation() << '\n';
std::cout << "偏度:      " << stats.skewness() << '\n';
std::cout << "峰度:      " << stats.kurtosis() << '\n';
```

結果會積極快取以提升效能。

### 6. 使用 Promise 進行非同步執行

受 JavaScript Promise 啟發，該庫提供 `Promise<T, E>` 類別，用於提交至 `ThreadPool` 的非同步任務：

```cpp
ThreadPool pool(8);

auto promise = pool.submit<int>([] {
    // 繁重計算
    return 42;
});

promise.then([](int result) {
        std::cout << "結果: " << result << '\n';
    })
    .except([](const std::exception& e) {
        std::cerr << "錯誤: " << e.what() << '\n';
    })
    .finally([] {
        std::cout << "完成\n";
    });

promise.wait();  // 若需阻塞則等待完成
```

靜態輔助：`Promise<T>::all(...)`、`Promise<T>::any(...)`、`resolved(value)`、`rejected(error)`。

## 安裝

將 `semantic.h` 與 `semantic.cpp` 複製至您的專案，或使用 CMake：

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## 編譯範例

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## 效能基準（Apple M2 Max，2024）

| 操作                               | Java Stream | ranges-v3 | semantic-cpp（並行） |
|------------------------------------|-------------|-----------|----------------------|
| 1 億整數 → 總和                    | 280 ms      | 190 ms    | **72 ms**            |
| 1 千萬雙精度 → 滑動視窗平均        | N/A         | N/A       | **94 ms**（視窗 30，步長 10） |
| 5 千萬整數 → toStatistics          | N/A         | N/A       | **165 ms**           |

## 貢獻

誠摯歡迎貢獻！目前需要關注的領域：

- 額外收集器（百分位數、共變異數等）
- 與範圍庫更好的整合
- 可選的 SIMD 加速

## 許可證

MIT © Eloy Kim

盡情享受真正的語義流處理於 C++！