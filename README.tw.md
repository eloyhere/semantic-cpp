# Semantic Stream —— 現代 C++ 函數式流庫

Semantic 是一個純標頭檔、高效能、完全惰性求值的 C++17 以上函數式流庫，設計靈感同時來自 JavaScript Generator、Java Stream API、Java Function 套件以及 MySQL 索引表的設計思想。

它將函數式程式設計的優雅表達力與真正的惰性求值、平行執行以及基於時間戳的元素索引完美結合，能輕鬆處理有限與無限序列。

## 主要特性

- **完全惰性求值** —— 所有中間操作僅在終結操作被呼叫時才真正執行。
- **原生支援無限流** —— 透過 Generator 可自然建構無限序列。
- **時間戳索引機制** —— 每個元素皆帶有隱式或顯式時間戳，實現類似 MySQL 索引的高效跳躍與定位。
- **一行開啟平行** —— 只需呼叫 `.parallel()` 或 `.parallel(n)` 即可轉為多執行緒流水線。
- **豐富的函數式操作** —— `map`、`filter`、`flatMap`、`reduce`、`collect`、`group`、統計等一應俱全。
- **完整的 Java 風格 Collector** —— 熟悉的 supplier、accumulator、combiner、finisher 四階段收集器。
- **強大統計功能** —— 平均值、中位數、眾數、變異數、標準差、四分位數、偏度、峰度等全部涵蓋。
- **多種建構方式** —— 支援從容器、陣列、範圍、生成器、填充等方式快速建立流。

## 設計理念

Semantic 將流視為「由 Generator 產生、帶有時間戳的元素序列」，核心靈感來源：

- **JavaScript Generator** —— 拉取式的惰性產生機制。
- **Java Stream** —— 流暢的鏈式呼叫與中間／終結操作分離。
- **Java Function 套件** —— `Function`、`Consumer`、`Predicate` 等函數式介面。
- **MySQL 索引表** —— 透過邏輯時間戳實現高效的 `skip`、`limit`、`redirect`、`translate` 等操作。

## 核心概念

```cpp
using Generator<E> = BiConsumer<
    BiConsumer<E, Timestamp>,   // yield(元素, 時間戳)
    Predicate<E>                // 取消判斷
>;
```

生成器會接收一個 yield 函數與一個取消檢查函數，此底層抽象驅動所有流來源，包含無限流。

## 快速範例

```cpp
using namespace semantic;

// 無限隨機整數流
auto s1 = Semantic<int>::iterate([](auto yield, auto cancel) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> d(1, 100);
    Timestamp ts = 0;
    while (!cancel(d(gen))) {
        yield(d(gen), ts++);
    }
});

// 惰性篩法生成質數（有限範圍）
auto primes = Semantic<long long>::range(2LL, 1'000'000LL)
    .filter([](long long n) {
        return Semantic<long long>::range(2LL, n)
            .takeWhile([n](long long d){ return d * d <= n; })
            .noneMatch([n](long long d){ return n % d == 0; });
    });

// 平行單字計數
auto wordCount = Semantic<std::string>::from(lines)
    .flatMap([](const std::string& line) {
        return Semantic<std::string>::from(split(line));
    })
    .parallel()
    .group([](const std::string& w){ return w; })
    .map([](auto& p){ return std::make_pair(p.first, p.second.size()); });
```

## 編譯需求

- C++17 或更高版本
- 純標頭檔 —— 只需 `#include "semantic.hpp"`
- 無任何外部相依

庫內建全域執行緒池 `semantic::globalThreadPool`，啟動時自動使用 `std::thread::hardware_concurrency()` 個執行緒初始化。

## 授權條款

MIT License —— 可自由用於商業與開源專案。

## 作者

融合當代函數式程式設計精華，為現代 C++ 量身打造的高效能實現。
