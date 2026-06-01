# 🚀 Semantic-Cpp：面向未來的 C++ 智慧串流處理框架

Semantic-Cpp 是一個完全重構設計的現代化 C++ 串流處理函式庫，採用「多標頭檔、零外部相依」的模組化架構。每個標頭檔職責清晰、獨立可測，共同構成了一個完整的串流處理生態系。本函式庫創造性地融合了多種程式設計典範的精華：

- 🎯 **Java Stream API** 的優雅與流暢：鏈式呼叫，宣告式程式設計，讓程式碼如詩般優雅
- ⚡ **JavaScript Generator** 的惰性與靈活：延遲計算，按需產生，記憶體友善
- 🗄️ **資料庫索引** 的高效與有序：智慧排序，索引驅動，時序資料處理利器

與傳統的資料處理方式（手寫迴圈、非同步回呼）不同，Semantic-Cpp 旨在提供一種 **型別安全、表達力強且高效能** 的解決方案。其核心設計哲學是 **精準的資料流控制**：資料僅在需要時流動，順序和位置透過「索引」精細調控，實現資源最佳利用。

---

## 📐 專案架構：七層模組化設計

Semantic-Cpp 由 **七個核心標頭檔** 組成，層層遞進，每個檔案職責單一、獨立可測：

```
┌─────────────────────────────────────────────────┐
│                 semantics.h                     │
│    (串流建構工廠：數值範圍、容器、文字、Unicode)      │
├─────────────────────────────────────────────────┤
│                  semantic.h                     │
│   (串流中介操作、Collectable 體系、容器特化)        │
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│  (收集器工廠：比對、搜尋、聚合、統計、DFT/FFT 等)   │
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│     (收集器框架：五階段模型、並行平行支援)          │
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│  (Unicode 字元序列、多編碼轉換、Builder、Buffer)   │
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│   (全域執行緒池：任務提交、緊急關閉、例外傳播)        │
├─────────────────────────────────────────────────┤
│                 function.h                      │
│  (型別定義：Generator、Supplier、Consumer 等別名)   │
└─────────────────────────────────────────────────┘
```

### 相依關係

```
function.h          ← 無相依，型別基石
pool.h              ← 相依於 function.h
charsequence.h      ← 獨立模組，Unicode 處理
collector.h         ← 相依於 function.h、pool.h
collectors.h        ← 相依於 collector.h、charsequence.h
semantic.h          ← 相依於 collector.h、collectors.h、charsequence.h
semantics.h         ← 相依於 semantic.h
```

每個標頭檔均可獨立編譯測試，也支援按需引入。  
例如，只需收集器功能時，僅引入 `collector.h` 和 `collectors.h` 即可。

---

## 🏗️ 第一層：function.h — 型別基礎

`function.h` 定義了整個框架的型別系統，是所有模組的共同基石：

```cpp
namespace function {
    using Timestamp = long long;           // 索引型別
    using Module = unsigned long long;     // 模組／計數型別
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept
        std::function<bool(T, Timestamp)>       // interrupt
    )>;
}
```

`Generator` 是整個串流系統的核心抽象：接受 `accept`（接收資料）和 `interrupt`（中斷流程）兩個回呼，體現了「惰性拉取」模型。

---

## ⚡ 第二層：pool.h — 並行基石

`pool.h` 提供全域執行緒池 `pool::pool`，是整個框架的並行引擎：

| 特性 | 說明 |
|----|----|
| 🎯 宣告式平行處理 | `parallel(n)` 僅宣告意圖，終端操作時自動啟動 |
| 🛡️ 緊急關閉 | 內建 `emergencyShutdown()` 和 `std::set_terminate` 處理器 |
| 🔄 例外傳播 | `submit()` 回傳 `std::future`，支援例外安全傳播 |

---

## 🔤 第三層：charsequence.h — Unicode 字元序列

`charsequence.h` 是一個完整的 Unicode 處理模組，提供字元序列的建立、轉換和操作功能：

| 功能 | 描述 |
|----|----|
| 🌐 多編碼支援 | UTF‑8、UTF‑16（LE/BE）、UTF‑32（LE/BE）、ASCII、Latin1 |
| 🔍 碼點迭代器 | `PointIterator` 支援雙向遍歷 Unicode 碼點 |
| 🏗️ 建構器模式 | `Builder` 類別支援高效位元組級字串拼接 |
| 📦 緩衝器 | `Buffer` 類別提供執行緒安全的環形緩衝區 |
| 🔑 雜湊與比較 | 所有核心型別均有 `std::hash` 和 `std::less` 特化 |

### 核心型別

| 型別 | 描述 |
|----|----|
| `Meta` | 元資料包裝器，儲存無號整數值 |
| `Point` | Unicode 碼點，支援代理對檢測和有效性驗證 |
| `Charsequence` | 不可變字元序列，支援分割、取代、搜尋、大小寫轉換等操作 |
| `Builder` | 可變位元組建構器，支援 `prepend`、`insert`、`append` 及各種資料型別 |
| `Buffer` | 執行緒安全環形緩衝區，支援讀寫、預取、容量管理 |

---

## 🔧 第四層：collector.h — 收集器框架

`collector.h` 實作了收集器模式，是終端操作的核心引擎。

### 五階段模型

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (可選短路)
```

### 型別別名

| 型別 | 定義 | 角色 |
|----|----|----|
| `Identity<A>` | `Supplier<A>` | 提供初始值 |
| `Accumulator<A,E>` | `TriFunction<A,E,Timestamp,A>` | 累加元素 |
| `Combiner<A>` | `BiFunction<A,A,A>` | 合併平行結果 |
| `Finisher<A,R>` | `Function<A,R>` | 最終轉換 |
| `Interrupt<E,A>` | `TriPredicate<E,Timestamp,A>` | 短路判斷 |

### 並行支援

`Collector::collect()` 自動處理：

- 📦 資料分片（按索引模運算分發到各執行緒）
- 🔗 結果合併（透過 `Combiner` 歸併局部結果）
- ⚠️ 例外傳播（透過 `std::exception_ptr` 和 `std::atomic<bool>`）

---

## 🏭 第五層：collectors.h — 收集器工廠

`collectors.h` 提供了豐富的預置收集器工廠函式。

### 📊 比對操作

| 方法 | 描述 | 回傳型別 |
|----|----|----|
| `useAllMatch(predicate)` | 所有元素滿足條件 | `bool` |
| `useAnyMatch(predicate)` | 任一元素滿足條件 | `bool` |
| `useNoneMatch(predicate)` | 沒有元素滿足條件 | `bool` |

### 🔍 搜尋操作

| 方法 | 描述 | 回傳型別 |
|----|----|----|
| `useFindFirst()` | 搜尋第一個元素 | `std::optional<E>` |
| `useFindLast()` | 搜尋最後一個元素 | `std::optional<E>` |
| `useFindAny()` | 隨機搜尋元素 | `std::optional<E>` |
| `useFindAt(index)` | 搜尋指定索引元素（支援負索引） | `std::optional<E>` |
| `useFindMaximum()` | 搜尋最大值（支援自訂比較器） | `std::optional<E>` |
| `useFindMinimum()` | 搜尋最小值（支援自訂比較器） | `std::optional<E>` |

### 📈 聚合操作

| 方法 | 描述 | 回傳型別 |
|----|----|----|
| `useCount()` | 計算元素總數 | `Module` |
| `useSummate<E,D>()` | 求和 | `D` |
| `useSummate<E,D>(mapper)` | 帶映射的求和 | `D` |
| `useAverage<E,D>()` | 平均值 | `D` |
| `useAverage<E,D>(mapper)` | 帶映射的平均值 | `D` |
| `useRange<E,D>()` | 數值範圍（最大減最小） | `D` |
| `useRange<E,D>(mapper)` | 帶映射的範圍 | `D` |
| `useMinimum<E,D>()` | 最小值 | `std::optional<D>` |
| `useMaximum<E,D>()` | 最大值 | `std::optional<D>` |

### 📊 統計操作

| 方法 | 描述 | 回傳型別 |
|----|----|----|
| `useVariance<E,D>()` | 總體變異數 | `D` |
| `useStandardDeviation<E,D>()` | 總體標準差 | `D` |
| `useSkewness<E,D>()` | 偏度 | `D` |
| `useKurtosis<E,D>()` | 峰度 | `D` |
| `useMedian<E,D>()` | 中位數 | `std::optional<D>` |
| `useMode<E>()` | 眾數（基於頻域分析） | `std::optional<E>` |
| `usePercentile<E,D>(p)` | 第 p 百分位數 | `std::optional<D>` |
| `useFrequency<E>()` | 頻域特徵（索引相位編碼） | `std::map<E, complex>` |
| `useDistribution<E>()` | 空間分布特徵（位置編碼） | `std::map<E, complex>` |

### 🔗 歸約操作

| 方法 | 描述 | 回傳型別 |
|----|----|----|
| `useReduce(reducer)` | 無初始值歸約 | `std::optional<E>` |
| `useReduce(identity, reducer)` | 帶初始值歸約 | `E` |
| `useReduce(id, red, comb, fin)` | 完全自訂歸約 | `R` |

### 📦 收集到容器操作

| 方法 | 描述 | 回傳型別 |
|----|----|----|
| `useToVector()` | 收集為 vector | `std::vector<E>` |
| `useToList()` | 收集為 list | `std::list<E>` |
| `useToDeque()` | 收集為 deque | `std::deque<E>` |
| `useToForwardList()` | 收集為 forward_list | `std::forward_list<E>` |
| `useToArray<N>()` | 收集為固定大小 array | `std::array<E, N>` |
| `useToSet()` | 收集為 set（去重排序） | `std::set<E>` |
| `useToMultiset()` | 收集為 multiset | `std::multiset<E>` |
| `useToUnorderedSet()` | 收集為 unordered_set | `std::unordered_set<E>` |
| `useToUnorderedMultiset()` | 收集為 unordered_multiset | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)` | 收集為 map | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)` | 收集為 map（自訂值） | `std::map<K, V>` |
| `useToMultimap(keyExtractor)` | 收集為 multimap | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)` | 收集為 multimap（自訂值） | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)` | 收集為 unordered_map | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)` | 收集為 unordered_multimap | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)` | 收集為 unordered_multimap（自訂值） | `std::unordered_multimap<K, V>` |
| `useToStack()` | 收集為 stack | `std::stack<E>` |
| `useToQueue()` | 收集為 queue | `std::queue<E>` |
| `useToPriorityQueue()` | 收集為 priority_queue | `std::priority_queue<E>` |

### 🔀 分組與分區操作

| 方法 | 描述 | 回傳型別 |
|----|----|----|
| `useGroup(keyExtractor)` | 按鍵分組 | `std::unordered_map<K, vector<E>>` |
| `usePartition(size)` | 按固定大小分區 | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)` | 按自訂鍵分區 | `std::vector<vector<E>>` |

### 🎨 字串輸出操作

| 方法 | 描述 | 回傳型別 |
|----|----|----|
| `useJoin()` | 連接為字串（預設逗號分隔，方括號包圍） | `Charsequence` |
| `useJoin(delimiter)` | 自訂分隔符連接 | `Charsequence` |
| `useJoin(prefix, delimiter, suffix)` | 完全自訂格式化連接 | `Charsequence` |
| `useOut()` | 格式化輸出到 stdout | `Charsequence` |
| `useOut(delimiter)` | 自訂分隔符輸出到 stdout | `Charsequence` |
| `useOut(prefix, delimiter, suffix)` | 完全自訂格式化輸出到 stdout | `Charsequence` |
| `useError()` | 格式化輸出到 stderr | `Charsequence` |
| `useError(delimiter)` | 自訂分隔符輸出到 stderr | `Charsequence` |
| `useError(prefix, delimiter, suffix)` | 完全自訂格式化輸出到 stderr | `Charsequence` |

### 🧮 數學工具

| 方法 | 描述 | 回傳型別 |
|----|----|----|
| `useDFT()` | 離散傅立葉轉換 | `vector<complex<double>>` |
| `useIDFT()` | 逆離散傅立葉轉換 | `vector<complex<double>>` |
| `useFFT()` | 快速傅立葉轉換 (Cooley-Tukey) | `vector<complex<double>>` |
| `useIFFT()` | 逆快速傅立葉轉換 | `vector<complex<double>>` |
| `useGradient(gradFunc, lr, iter, th)` | 梯度下降（解析梯度） | `vector<double>` |
| `useGradient(costFunc, lr, iter, th, h)` | 梯度下降（數值梯度） | `vector<double>` |

---

## 🌊 第六層：semantic.h — 串流中介操作與收集體系

`semantic.h` 是整個框架的核心，包含 `collectable` 和 `semantic` 兩個命名空間。

### collectable 命名空間

提供可收集物件的繼承體系：

| 類別 | 描述 | 底層儲存 |
|----|----|----|
| `Collectable<E>` | 抽象基類，純虛函式 `source()` | — |
| `OrderedCollectable<E>` | 有序收集，支援自訂排序 | `std::map<Timestamp, E>` |
| `UnorderedCollectable<E>` | 無序收集，高效 O(1) 查詢 | `std::unordered_map<Timestamp, E>` |
| `Statistics<E, D>` | 統計收集（繼承 `OrderedCollectable`） | 提供 20+ 統計方法 |
| `WindowCollectable<E>` | 視窗收集（繼承 `OrderedCollectable`） | 支援 `slide` / `tumble` |

#### Collectable 基類方法

提供所有 `toXxx()` 終端收集方法（共 20+ 種容器），以及 `count()`、`findFirst()`、`findAny()`、`anyMatch()`、`allMatch()`、`noneMatch()`、`reduce()`、`join()`、`out()`、`error()`、`group()`、`partition()`、`partitionBy()` 等操作。

#### Statistics 類別方法

| 方法 | 回傳型別 | 描述 |
|----|----|----|
| `summate()` / `summate(mapper)` | `D` | 求和 |
| `average()` / `average(mapper)` | `D` | 平均值 |
| `minimum()` / `minimum(mapper)` | `std::optional<D>` | 最小值 |
| `maximum()` / `maximum(mapper)` | `std::optional<D>` | 最大值 |
| `range()` / `range(mapper)` | `D` | 範圍（最大 − 最小） |
| `variance()` / `variance(mapper)` | `D` | 總體變異數 |
| `standardDeviation()` / `standardDeviation(mapper)` | `D` | 總體標準差 |
| `frequency()` / `frequency(mapper)` | `std::map<*, complex>` | 頻域特徵（索引相位編碼） |
| `distribute()` / `distribute(mapper)` | `std::map<*, complex>` | 空間分布特徵（位置編碼） |
| `median()` / `median(mapper)` | `std::optional<D>` | 中位數 |
| `mode()` | `std::optional<E>` | 眾數 |
| `percentile(p)` / `percentile(p, mapper)` | `std::optional<D>` | 第 p 百分位數 |
| `firstQuartile()` / `firstQuartile(mapper)` | `std::optional<D>` | 第一四分位數 (Q1) |
| `thirdQuartile()` / `thirdQuartile(mapper)` | `std::optional<D>` | 第三四分位數 (Q3) |
| `interquartileRange()` / `interquartileRange(mapper)` | `std::optional<D>` | 四分位距 (Q3 − Q1) |
| `skewness()` / `skewness(mapper)` | `D` | 偏度 |
| `kurtosis()` / `kurtosis(mapper)` | `D` | 峰度 |
| `dft()` | `vector<complex<double>>` | 離散傅立葉轉換 |
| `idft()` | `vector<complex<double>>` | 逆離散傅立葉轉換 |
| `fft()` | `vector<complex<double>>` | 快速傅立葉轉換 |
| `ifft()` | `vector<complex<double>>` | 逆快速傅立葉轉換 |
| `gradient(gradFunc, lr, iter, th)` | `vector<double>` | 梯度下降（解析梯度） |
| `gradient(costFunc, lr, iter, th, h)` | `vector<double>` | 梯度下降（數值梯度） |

### semantic 命名空間

提供 `Semantic<E>` 樣板類別及其完整的特化體系。

#### 主樣板方法清單

| 類別 | 方法 | 描述 |
|----|----|----|
| 🎨 元素轉換 | `map` | 一對一映射轉換 |
| | `flatMap` | 一對多映射並展平 |
| | `flat` | 展平巢狀串流 |
| 🔍 元素過濾 | `filter` | 條件過濾 |
| | `takeWhile` | 條件滿足時持續獲取 |
| | `dropWhile` | 條件滿足時持續丟棄 |
| | `distinct` | 去重（支援自訂比較器） |
| 📏 數量控制 | `limit` | 限制元素數量 |
| | `skip` | 跳過前 n 個元素 |
| | `sub` | 截取子範圍 |
| 📐 索引操作 | `redirect` | 重新映射索引 |
| | `reverse` | 反轉索引 |
| | `translate` | 偏移索引（固定或動態） |
| 👀 觀察操作 | `peek` | 觀察每個元素（不修改串流） |
| ⚡ 平行宣告 | `parallel` | 宣告平行度 |
| 🔗 連接操作 | `concatenate` | 連接另一個串流或容器 |
| 📤 終端轉換 | `toUnordered` | 轉為無序收集器 |
| | `toOrdered` | 轉為有序收集器 |
| | `toWindow` | 轉為視窗收集器 |
| | `toStatistics` | 轉為統計收集器 |

#### 容器特化完整支援

| 特化型別 | 描述 |
|----|----|
| `Semantic<std::vector<E>>` | 向量容器串流，支援排序、去重等操作 |
| `Semantic<std::list<E>>` | 鍊錶容器串流，支援排序、去重等操作 |

---

## 🏭 第七層：semantics.h — 串流建構工廠

`semantics.h` 提供所有串流建構工廠函式。

### 📐 數值範圍產生

| 方法 | 描述 |
|----|----|
| `useRange(start, end)` | 產生 `[start, end)` 範圍內的數值串流 |
| `useRange(start, end, step)` | 帶步長的範圍產生 |
| `useRangeClosed(start, end)` | 產生 `[start, end]` 閉區間範圍 |
| `useRangeClosed(start, end, step)` | 帶步長的閉區間範圍 |

### ♾️ 無限串流產生

| 方法 | 描述 |
|----|----|
| `useInfinite(seed, generator)` | 從種子值開始，無限迭代產生 |
| `useGenerate(supplier)` | 無限呼叫供應者產生 |
| `useGenerate(supplier, limit)` | 有限次數呼叫供應者產生 |
| `useIterate(seed, generator)` | 從種子值開始無限迭代 |
| `useIterate(seed, generator, limit)` | 有限次數迭代產生 |
| `useRandom()` | 無限隨機整數串流 |
| `useRandom(min, max)` | 指定範圍的無限隨機數串流 |
| `useRandom(min, max, count)` | 指定範圍和數量的隨機數串流 |

### 📦 容器與元素建構

| 方法 | 描述 |
|----|----|
| `useEmpty()` | 建立空串流 |
| `useOf(element)` | 從單一元素建立串流 |
| `useOf(e1, e2)` | 從兩個元素建立串流 |
| `useOf(e1, e2, e3)` | 從三個元素建立串流 |
| `useOf({...})` | 從初始化列表建立串流 |
| `useFrom(container)` | 從任何標準容器建立串流 |
| `useFrom({...})` | 從初始化列表建立串流 |
| `useRepeat(element, count)` | 重複指定元素 n 次 |

### 📝 文字處理

| 方法 | 描述 |
|----|----|
| `useBlob(text)` | 將字串按位元組拆分為 char 串流 |
| `useBlob(text, start, end)` | 將字串指定範圍按位元組拆分 |
| `useBlob(istream)` | 從輸入串流按行讀取 |
| `useBlob(istream, delimiter)` | 從輸入串流按分隔符讀取 |
| `useText(text)` | 將字串作為整體文字串流 |
| `useText(text, delimiter)` | 按分隔符拆分文字 |
| `useText(istream)` | 從輸入串流讀取整個內容 |
| `useText(istream, delimiter)` | 從輸入串流按分隔符讀取 |

### 🌐 Unicode 處理

| 方法 | 描述 |
|----|----|
| `useSequence(charsequence)` | 從字元序列建立碼點串流 |
| `useSequence(charsequence, start, end)` | 從字元序列指定範圍建立碼點串流 |
| `useSequence(text, encoding)` | 從文字建立指定編碼的碼點串流 |
| `useSequence(istream, encoding)` | 從輸入串流建立指定編碼的碼點串流 |
| `useCharsequence(charsequence)` | 將字元序列作為整體串流 |
| `useCharsequence(charsequence, delimiter)` | 按分隔符拆分字元序列 |
| `useCharsequence(istream, encoding)` | 從輸入串流讀取整個字元序列 |
| `useCharsequence(istream, delimiter, encoding)` | 從輸入串流按分隔符讀取字元序列 |

---

## 🧠 核心概念：索引驅動的資料世界

Semantic-Cpp 將資料處理抽象為對「元素」及其「邏輯位置（索引）」的操作。理解這一點是掌握本函式庫的關鍵。

### 1. 📐 基礎索引變換

| 方法 | 描述 |
|----|----|
| `redirect(fn)` | 核心方法：自訂函式完全重寫元素索引 |
| `reverse()` | 將所有索引邏輯反轉（內部透過 `redirect` 實作） |
| `translate(offset)` | 固定偏移 |
| `translate(translator)` | 動態偏移函式，根據元素和索引計算新索引 |

### 2. 📊 排序的「霸道」規則

> ⚠️ **`sort()` 會覆蓋一切**：呼叫後所有之前的索引操作都將被覆蓋，元素按值重新分配自然順序索引。

- `sort()` → 立即物化為 `OrderedCollectable`，按元素值自然排序
- `sort(comparator)` → 自訂比較器排序

### 3. ⚡ 宣告式平行處理

- `parallel(n)` 僅宣告意圖，不立即啟動執行緒
- 終端操作（`toUnordered()`、`count()` 等）才真正觸發平行處理
- 執行緒池自動處理任務分發和結果合併

### 4. 🎯 如何選擇最終的資料容器？

| 轉換方法 | 底層資料結構 | 效能特徵 | 最佳適用場景 |
|----|----|----|----|
| `sort()` | `OrderedCollectable` | 排序後物化，保持值順序 | 按值排序、分頁、時間序列 |
| `toOrdered()` | `OrderedCollectable` | 保持當前索引順序 | 保留自訂索引順序 |
| `toUnordered()` | `UnorderedCollectable` | 平均 O(1)，最高效能 | 快速查詢、去重、聚合 |
| `toWindow()` | `WindowCollectable` | 基於有序集合 | 滑動 / 滾動視窗分析 |
| `toStatistics()` | `Statistics` | 20+ 統計方法 | 全面統計分析 |

---

## 🚀 快速上手指南

### 安裝

將所有標頭檔放入專案目錄，確保編譯器支援 **C++17 或更高標準**：

```
include/
├── function.h
├── pool.h
├── charsequence.h
├── collector.h
├── collectors.h
├── semantic.h
└── semantics.h
```

```cpp
#include "semantics.h"  // 自動包含其他相依項
```

---

## 🎯 基礎範例：體驗索引與排序

```cpp
auto result = semantic::useRange(0, 10)
    .map(int x { return x * x; })
    .redirect(int value, auto index -> long long {
        return index * 2;
    })
    .reverse()
    .sort()                              // 強制按值排序，覆蓋所有索引操作
    .toVector();

// 輸出: 0 1 4 9 16 25 36 49 64 81
```

## ⚡ 平行處理範例

```cpp
auto count = semantic::useRange(1, 1000)
    .parallel(4)
    .filter(int x { return x % 2 == 0; })
    .toUnordered()
    .count();

// 輸出: 偶數數量: 500
```

## 📊 統計分析範例

```cpp
auto stats = semantic::useRange(1, 101)
    .toStatistics<int, double>();

auto avg = stats.average();               // 平均值
auto med = stats.median();                // 中位數
auto std = stats.standardDeviation();     // 標準差
auto q1  = stats.firstQuartile();          // 第一四分位數
auto q3  = stats.thirdQuartile();          // 第三四分位數
auto skew = stats.skewness();              // 偏度
```

## 🔬 頻域分析範例

```cpp
auto freq = data.toUnordered().frequency();
for (const auto& [element, z] : freq) {
    auto magnitude = std::abs(z);  // 分布集中度
    auto phase     = std::arg(z);  // 分布中心相位
}
```

## 🧮 FFT 轉換範例

```cpp
auto spectrum = semantic::useRange(0, 8)
    .map(int x -> std::complex<double> {
        return {static_cast<double>(x), 0.0};
    })
    .toUnordered()
    .collect(collector::useFFT<double>());
```

---

## ⚡ 效能最佳化建議

1. 🎯 **選擇對的容器**
   - 等值查詢、不排序的聚合 → `toUnordered()`
   - 範圍查詢、排序、分頁 → `toOrdered()` 或 `sort()`
   - 即時視窗分析 → `toWindow()`
2. ⚡ **善用平行處理**：資料量大或處理邏輯耗時時使用 `parallel()`，避免阻斷式 I/O
3. 📐 **優化操作順序**：儘早 `filter`，明智 `sort`
4. 🔄 **利用惰性求值**：中間操作不立即執行，`takeWhile` 和 `limit` 可提前終止

---

## 📊 與 C++ 標準函式庫及競品對比

| 特性 | Semantic-Cpp | C++20/23 ranges | 傳統迴圈 |
|----|----|----|----|
| 🎯 核心典範 | 宣告式、索引驅動 | 視圖驅動、函數式組合 | 命令式、程序化 |
| ⚡ 平行支援 | 宣告式，自動執行緒池 | 需組合平行演算法 | 手動實作 |
| 📐 排序與索引 | 索引精細控制，支援負索引 | 破壞性排序 | 完全手動 |
| 📊 統計分析 | 20+ 內建統計方法 | 不內建 | 需第三方函式庫 |
| 🔬 頻域分析 | 原生 DFT / FFT / 頻域特徵 | 不原生支援 | 需第三方函式庫 |
| 🧮 梯度下降 | 解析 + 數值雙模式 | 不內建 | 需第三方函式庫 |
| 🌐 Unicode | 原生多編碼支援（UTF‑8/16/32 等） | 不原生支援 | 手動處理 |
| 📦 容器收集 | 20+ 標準容器全覆蓋 | 部分支援 | 手動實作 |
| 📦 相依性 | 零外部相依，7 標頭檔 | 標準函式庫 | 無 |

---

## 📜 授權

- 📄 **授權條款**：基於 MIT 開源授權

---

**Semantic-Cpp — 用現代 C++ 建構高效、清晰的資料處理管線。🚀**
