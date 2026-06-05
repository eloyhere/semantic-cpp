# 🚀 Semantic-Cpp: 邁向未來的 C++ 智慧型串流處理框架
**Semantic-Cpp** 是一款完全重新設計的現代 C++ 串流處理函式庫，採用了 **「多標頭檔、零外部依賴」** 的模組化架構。每個標頭檔皆具備明確的職責且可獨立進行測試，共同構建出一個完整的串流處理生態系。
本函式庫創造性地融合了多種程式設計範式的精髓：
 * **Java Stream API 的優雅與靈活性：** 連鎖呼叫、宣告式程式設計，讓程式碼如詩般優雅 ✨
 * **JavaScript Generator 的延遲性與靈活性：** 延遲計算、按需生成、記憶體友善 🌱
 * **資料庫索引的效率與排序性：** 智慧排序、基於索引、時序資料處理的強大工具 ⏱️
 * **以容器為元素的批次處理哲學：** 向量、鏈結串列、映射……所有容器皆作為串流的一等公民自由流動 📦
### 💡 導入背景
 * 您是否早已厭倦了親手編寫 for 迴圈來走訪 vector，接著用 if 進行過濾，最後再手動 push_back 到另一個容器中？ 😩
 * 您是否曾在深夜除錯時，因為想在逆向走訪時獲取「倒數第三個」元素，卻因索引偏移了一格而抓狂？ 😵‍💫
 * 您是否渴望像操作資料庫一樣，透過索引精準定位、利用滑動窗口進行分析，僅需一行連鎖呼叫就能完美完成從數據到統計的完整旅程？ 🤔
**Semantic-Cpp 正是為此而生。** 🔧
它將資料處理抽象化為對**「元素」**及其**「邏輯位置（索引）」**的操作。就像資料庫中的「資料列（Row）」與「主鍵（Primary Key）」一樣，在完全不更動資料本身的情況下，即可自由地對索引進行重新排列、偏移與反轉；且任何容器（vector、map、array……）都能在串流中作為一個不可分割的整體進行傳遞，並能隨時再次「解包（Unpack）」至元素層級。這種在兩種粒度（Granularity）之間自由切換的能力，是傳統串流框架所不具備的特點。 🎯
## 🏗️ 專案架構：7 層模組化設計
Semantic-Cpp 由 **7 個核心標頭檔** 組成，層層遞進。**5 個命名空間** 各司其職，共同構建出從資料源到最終結果 satisfaction 的完整管線（Pipeline）：
```
┌─────────────────────────────────────────────────┐
│               🌊 semantics.h                    │
│   命名空間: semantic                            │
│   串流構建器工廠: 數字範圍、容器、文本、Unicode       │
├─────────────────────────────────────────────────┤
│                 📦 semantic.h                     │
│   命名空間: semantic / collectable            │
│   串流中間操作、Collectable 體系、容器擴充支援       │
├─────────────────────────────────────────────────┤
│                ⚙️ collector.h                     │
│   命名空間: collector                           │
│   收集器框架 + 工廠: 匹配、搜尋、聚合、統計、DFT/FFT │
├─────────────────────────────────────────────────┤
│                🔤 charsequence.h                   │
│   命名空間: charsequence                        │
│   Unicode 字元序列、多編碼轉換、Builder、Buffer    │
├─────────────────────────────────────────────────┤
│                🧵 pool.h                          │
│   命名空間: pool                                │
│   全域執行緒池: 工作提交、緊急關閉、例外傳播         │
├─────────────────────────────────────────────────┤
│                📄 function.h                      │
│   命名空間: function                            │
│   型態定義: Generator, Supplier, Consumer 等別名  │
├─────────────────────────────────────────────────┤
│               🔐 hash.h / less.h                  │
│   命名空間: std (擴充)                           │
│   標準函式庫容器雜湊及比較特例化，支援任意巢狀結構    │
└─────────────────────────────────────────────────┘

```
### 🧩 依賴關係
依賴鏈如同精心設計的電路圖般清晰明瞭。電流從最底層的型態定義開始，逐層向上延伸，每一層僅依賴其下方的層級。最終，所有線路皆匯集於 semantic.h 與 semantics.h，形成完整的串流處理能力。
```
function.h          ← 無依賴，型態基石
pool.h              ← 依賴 function.h
charsequence.h      ← 獨立模組，Unicode 處理
collector.h         ← 依賴 function.h, pool.h
hash.h / less.h     ← 獨立模組，標準函式庫擴充
semantic.h          ← 依賴上述所有項目
semantics.h         ← 依賴 semantic.h

```
## 🌍 命名空間全景
Semantic-Cpp 精心設計了 5 個命名空間，每個命名空間就像是一個獨立的「部門」，各自履行職責卻又緊密協作：
| 命名空間 | 所屬檔案 | 職責 | 核心型態 / 函式 |
|---|---|---|---|
| **function** | function.h | 型態系統基石 | Timestamp, Module, Generator<T>, Supplier<R>, Consumer<T>, Predicate<T> 等 |
| **pool** | pool.h | 同步執行引擎 | pool::pool（全域執行緒池）、submit()、emergencyShutdown() |
| **charsequence** | charsequence.h | Unicode 字串處理 | charset, Meta, Point, Charsequence, Builder, Buffer 等 |
| **collector** | collector.h | 最終收集執行 | Collector<E,A,R>, Identity<A>, Accumulator<A,E> 等 |
| **collectable** | semantic.h | 具體化數據容器 | Collectable<E>, OrderedCollectable<E>, UnorderedCollectable<E> 等 |
| **semantic** | semantic.h
semantics.h | 串流構建及中間操作 | Semantic<E>, useRange(), useFrom() 等 |
### 🔁 命名空間協作流程
命名空間之間的資料流動就像是工廠的組裝流水線。原始物料進入 semantic，經過多個階段的加工變換，最終在 collector 中打包出貨。每個階段皆有清晰的責任邊界：
```cpp
semantic::useRange(0, 100)          // ← semantic 命名空間: 串流生成
    .map(int x { return x * 2; })   // ← semantic 命名空間: 中間轉換
    .filter(int x { return x > 50; }) // ← semantic 命名空間: 中間過濾
    .toUnordered()                  // ← collectable 命名空間轉換
    .toVector();                    // ← 呼叫 collector 命名空間的收集器

```
## 📦 第 1 層：function.h — 型態基礎
function.h 定義了整個框架的型態系統，是所有模組的共同基石。 🔑
```cpp
namespace function {
    using Timestamp = long long;           // 索引型態，串流內數據的「時間戳記」
    using Module = unsigned long long;     // 模組 / 計數型態
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept — 接收一個元素
        std::function<bool(T, Timestamp)>       // interrupt — 是否中斷？
    )>;
}

```
Generator 是整個串流系統的核心抽象。 🌀 它不返回資料，而是接收兩個回呼（Callback）——accept（「準備就緒，請接收此元素」）與 interrupt（「是否該停止？」）。
得益於這種**控制反轉（Inversion of Control）**設計，資料生產者完全不需知道消費者是誰，只需在適當時機將資料「推（Push）」出即可。這正是**延遲求值（Lazy Evaluation）**的精髓。資料只有在 accept 被呼叫時才會真正「流動」，在此之前，一切都不過是宣告與描述。
| 型態別名 | 完整定義 | 用途 |
|---|---|---|
| Timestamp | long long | 串流內元素的邏輯位置 |
| Module | unsigned long long | 計數、容量、並行度 |
| Runnable | std::function<void()> | 無參數且無傳回值的工作 |
| Supplier<R> | std::function<R()> | 供應者，從無到有創造 |
| Function<T,R> | std::function<R(T)> | 單參數函式 |
| BiFunction<T,U,R> | std::function<R(T,U)> | 雙參數函式 |
| TriFunction<T,U,V,R> | std::function<R(T,U,V)> | 三參數函式 |
| Unary<T> | std::function<T(T)> | 單元運算 |
| Binary<T> | std::function<T(T,T)> | 二元運算 |
| Consumer<T> | std::function<void(T)> | 消費者 |
| BiConsumer<T,U> | std::function<void(T,U)> | 雙參數消費者 |
| TriConsumer<T,U,V> | std::function<void(T,U,V)> | 三參數消費者 |
| Predicate<T> | std::function<bool(T)> | 述詞判斷（條件篩選） |
| BiPredicate<T,U> | std::function<bool(T,U)> | 雙參數述詞 |
| TriPredicate<T,U,V> | std::function<bool(T,U,V)> | 三參數述詞 |
| Comparator<T> | std::function<int(const T&,const T&)> | 比較器，傳回 負數/0/正數 |
| Generator<T> | BiConsumer<BiConsumer<T,Timestamp>, BiPredicate<T,Timestamp>> | 串流生成器核心抽象 |
## 🧵 第 2 層：pool.h — 同行基礎
pool.h 提供了全域執行緒池 pool::pool，是整個框架的並行引擎。 🚀
採用了**宣告式並行處理（Declarative Parallelism）**設計。當您寫下 .parallel(4) 時，並不會立即啟動 4 個執行緒並開始處理。這行程式碼僅僅是一個「宣告」：告訴框架「我計劃使用 4 個執行緒進行並行處理」。
真正的並行執行會發生在最終運算被呼叫之時——也就是呼叫 toVector()、findFirst()、count() 等收集方法的那一刻。
| 功能 | 說明 |
|---|---|
| **宣告式並行** | .parallel(4) 僅是「宣告將使用 4 個執行緒進行並行處理」，不會立即啟動 |
| **緊急關閉** | 內建 emergencyShutdown() 及 std::set_terminate 處理常式 |
| **例外傳播** | submit() 傳回 std::future，可將例外安全地傳播至主執行緒 |
## 🔤 第 3 層：charsequence.h — Unicode 字元序列
charsequence.h 是一個完整的 Unicode 處理模組，提供了字元序列的生成、轉換及操作功能。 🌍
支援 UTF-8、UTF-16(LE/BE)、UTF-32(LE/BE)、ASCII、Latin1 等多種編碼，能正確偵測並處理代理對（Surrogate Pair），且針對無效的碼位（Code Point）會傳回標準的 U+FFFD 替代字元。
| 型態 / 函式 | 說明 |
|---|---|
| charset | 列舉：ascii, utf8, utf16, utf16be, utf16le, utf32, utf32be, utf32le, latin1 |
| Meta | 中介資料包裝器，儲存無號整數值 |
| Point | Unicode 碼位，支援代理對偵測與有效性驗證 |
| Charsequence | 不可變字元序列：split, replace, indexOf, lastIndexOf, sub, trim, toUpperCase, toLowerCase, reverse, startsWith, endsWith, contains, compare, getBytes, getPoints, getMetas, getCharacters, repeat, concat, count, join |
| Builder | 可變位元組構建器：prepend, insert, append （支援基本型態、Point、Charsequence、string_view） |
| Buffer | 執行緒安全環形緩衝區：write, read, peek, prepend, append, clear, shrinkToFit, data, size, capacity, atomic |
| PointIterator | 雙向迭代器，走訪 Unicode 碼位 |
| encode() | 將單一碼位編碼為指定編碼的位元組序列 |
| decode() | 從位元組序列中解碼下一個碼位並自動推進指標 |
| convert() | 編碼轉換（支援輸出為 string、vector、deque） |
## ⚙️ 第 4 層：collector.h — 收集器框架與工廠
collector.h 是 Semantic-Cpp 的 收集器核心模組，將收集器框架與工廠函式合而為一。
### 🧩 5 階段模型
```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (選擇性短路)

```
| 型態別名 | 完整定義 | 角色 |
|---|---|---|
| Identity<A> | function::Supplier<A> | 提供初始值 |
| Accumulator<A,E> | function::TriFunction<A, A E, Timestamp,> | 元素累積 |
| Combiner<A> | function::BiFunction<A, A A,> | 併發（並行）結果合併 |
| Finisher<A,R> | function::Function<A, R> | 最終轉換 |
| Interrupt<E,A> | function::TriPredicate<E, A Timestamp,> | 短路判斷 |
### 🛠️ 收集器工廠函式
#### ✅ 匹配運算
| 方法 | 說明 | 傳回型態 |
|---|---|---|
| useAllMatch(predicate) | 所有元素皆滿足條件 | bool |
| useAnyMatch(predicate) | 任一元素滿足條件 | bool |
| useNoneMatch(predicate) | 所有元素皆不滿足條件 | bool |
#### 🔍 搜尋運算
| 方法 | 說明 | 傳回型態 |
|---|---|---|
| useFindFirst() | 搜尋第一個元素 | std::optional<E> |
| useFindLast() | 搜尋最後一個元素 | std::optional<E> |
| useFindAny() | 搜尋任一元素 | std::optional<E> |
| useFindAt(index) | 精準定位位置（支援負數索引） | std::optional<E> |
| useFindMaximum() | 搜尋最大值 | std::optional<E> |
| useFindMinimum() | 搜尋最小值 | std::optional<E> |
#### 🔢 總計運算
| 方法 | 說明 | 傳回型態 |
|---|---|---|
| useCount() | 元素總個數 | Module |
| useSummate<E,D>() | 總和 | D |
| useAverage<E,D>() | 平均值 | D |
| useRange<E,D>() | 全距（範圍） | D |
#### 📉 統計運算
| 方法 | 說明 | 傳回型態 |
|---|---|---|
| useVariance<E,D>() | 母體變異數 | D |
| useStandardDeviation<E,D>() | 母體標準差 | D |
| useSkewness<E,D>() | 偏度 | D |
| useKurtosis<E,D>() | 峰度 | D |
| useMedian<E,D>() | 中位數 | std::optional<D> |
| useMode<E>() | 眾數（次數分析） | std::optional<E> |
| usePercentile<E,D>(p) | 第 p 百分位數 | std::optional<D> |
| useFrequency<E>() | 頻率特徵 | std::map<E, complex> |
| useDistribution<E>() | 空間分布特徵 | std::map<E, complex> |
#### 🔀 歸納（Reduce）運算
| 方法 | 說明 | 傳回型態 |
|---|---|---|
| useReduce(reducer) | 無初始值的歸納 | std::optional<E> |
| useReduce(identity, reducer) | 有初始值的歸納 | E |
| useReduce(id, red, comb, fin) | 完全自訂歸納 | R |
#### 🧺 收集至容器
| 方法 | 傳回型態 |
|---|---|
| useToVector() | std::vector<E> |
| useToList() | std::list<E> |
| useToDeque() | std::deque<E> |
| useToForwardList() | std::forward_list<E> |
| useToArray<N>() | std::array<E, N> |
| useToSet() | std::set<E> |
| useToMultiset() | std::multiset<E> |
| useToUnorderedSet() | std::unordered_set<E> |
| useToUnorderedMultiset() | std::unordered_multiset<E> |
| useToMap(keyExtractor) | std::map<K, E> |
| useToMap(keyExtractor, valueExtractor) | std::map<K, V> |
| useToMultimap(keyExtractor) | std::multimap<K, E> |
| useToMultimap(keyExtractor, valueExtractor) | std::multimap<K, V> |
| useToUnorderedMap(keyExtractor, valueExtractor) | std::unordered_map<K, V> |
| useToUnorderedMultimap(keyExtractor) | std::unordered_multimap<K, E> |
| useToUnorderedMultimap(keyExtractor, valueExtractor) | std::unordered_multimap<K, V> |
| useToStack() | std::stack<E> |
| useToQueue() | std::queue<E> |
| useToPriorityQueue() | std::priority_queue<E> |
#### 🧩 分組與分區
| 方法 | 傳回型態 |
|---|---|
| useGroup(keyExtractor) | std::unordered_map<K, vector<E>> |
| useGroupBy(keyExtractor, valueExtractor) | std::unordered_map<K, vector<V>> |
| usePartition(size) | std::vector<vector<E>> |
| usePartitionBy(keyExtractor) | std::vector<vector<E>> |
| usePartitionBy(keyExtractor, valueExtractor) | std::vector<vector<V>> |
#### 📄 字串輸出
| 方法 | 傳回型態 |
|---|---|
| useJoin() / useOut() / useError() 及其多種多載版本 | charsequence::Charsequence |
#### 📊 數學工具
| 方法 | 傳回型態 |
|---|---|
| useDFT() | vector<complex<double>> |
| useIDFT() | vector<complex<double>> |
| useFFT() | vector<complex<double>> |
| useIFFT() | vector<complex<double>> |
| useGradient() | vector<double> |
## 📦 第 5 層：semantic.h — 串流中間操作與收集體系
### 🧩 核心設計：3 階段管線
```
Semantic<E> (構建與變換)
    ↓ toUnordered() / toOrdered() / sort() / toWindow() / toStatistics()
Collectable<E> (具體化與收集)
    ↓ toVector() / findFirst() / count() / summate() / ...
最終結果

```
> ⚠️ **核心規則：** 必須先透過 toUnordered()、toOrdered()、toWindow()、toStatistics() 或 sort() 將 Semantic<E> 轉換為 Collectable<E> 後，才能呼叫最終方法。
> 
### 🧭 5 種具體化路徑
| 轉換方法 | 對象型態 | 基礎資料結構 | 效能特徵 |
|---|---|---|---|
| toUnordered() | UnorderedCollectable | unordered_map | 平均 O(1) 搜尋 |
| toOrdered() | OrderedCollectable | map | O(\log n) 搜尋 |
| sort() | OrderedCollectable | map （依數值排序） | O(\log n) 搜尋 |
| toWindow() | WindowCollectable | 繼承自排序集合 | 支援 slide/tumble |
| toStatistics<D>() | Statistics<E,D> | 繼承自排序集合 | 30+ 種統計方法 |
### 📋 Collectable<E> — 完整最終方法 (依英文字母排序)
| 方法 | 傳回型態 | 說明 |
|---|---|---|
| allMatch(predicate) | bool | 所有元素皆滿足條件 |
| anyMatch(predicate) | bool | 任一元素滿足條件 |
| average<D>() | D | 平均值 |
| average<D>(mapper) | D | 映射後平均值 |
| collect(identity, acc, comb, fin) | R | 自訂 4 階段收集 |
| collect(identity, interrupt, acc, comb, fin) | R | 自訂可中斷收集 |
| count() | Module | 元素總個數 |
| empty() | bool | 串流是否為空 |
| error() | void | 輸出至 stderr （支援 delimiter/prefix/suffix/converter） |
| findAny() | std::optional<E> | 搜尋任一元素 |
| findAt(index) | std::optional<E> | 搜尋指定索引（支援負數索引） |
| findFirst() | std::optional<E> | 搜尋第一個元素 |
| findLast() | std::optional<E> | 搜尋最後一個元素 |
| findMaximum() | std::optional<E> | 搜尋最大值 |
| findMaximum(comparator) | std::optional<E> | 透過自訂比較子搜尋最大值 |
| findMinimum() | std::optional<E> | 搜尋最小值 |
| findMinimum(comparator) | std::optional<E> | 透過自訂比較子搜尋最小值 |
| forEach(consumer) | void | 走訪每個元素並執行副作用 |
| group(keyExtractor) | unordered_map<K, vector<E>> | 依鍵值分組 |
| groupBy(keyExtractor, valueExtractor) | unordered_map<K, vector<V>> | 依鍵值分組並擷取值 |
| join() | Charsequence | 以預設格式連接 |
| join(delimiter) | Charsequence | 以自訂分隔符號連接 |
| join(prefix, delimiter, suffix) | Charsequence | 以完全自訂的格式連接 |
| noneMatch(predicate) | bool | 所有元素皆不滿足條件 |
| out() | Charsequence | 輸出至 stdout （支援 delimiter/prefix/suffix/converter） |
| partition(size) | vector<vector<E>> | 以固定大小分區 |
| partitionBy(keyExtractor) | vector<vector<E>> | 依索引鍵分區 |
| partitionBy(keyExtractor, valueExtractor) | vector<vector<V>> | 依索引鍵分區並擷取值 |
| range<D>() | D | 數值全距（最大值 - 最小值） |
| range<D>(mapper) | D | 映射後數值全距 |
| reduce(accumulator) | std::optional<E> | 無初始值的歸納 |
| reduce(identity, accumulator) | E | 有初始值的歸納 |
| reduce(identity, acc, comb) | R | 完全自訂的歸納 |
| summate<D>() | D | 總和 |
| summate<D>(mapper) | D | 映射後總和 |
| toArray<N>() | std::array<E, N> | 收集至固定大小的 array |
| toDeque() | std::deque<E> | 收集至 deque |
| toForwardList() | std::forward_list<E> | 收集至 forward_list |
| toList() | std::list<E> | 收集至 list |
| toMap(keyExtractor) | std::map<K, E> | 依鍵值收集至 map |
| toMap(keyExtractor, valueExtractor) | std::map<K, V> | 依自訂鍵值收集至 map |
| toMultimap(keyExtractor) | std::multimap<K, E> | 依鍵值收集至 multimap |
| toMultimap(keyExtractor, valueExtractor) | std::multimap<K, V> | 依自訂鍵值收集至 multimap |
| toMultiset() | std::multiset<E> | 收集至 multiset |
| toPriorityQueue() | std::priority_queue<E> | 收集至 priority_queue |
| toQueue() | std::queue<E> | 收集至 queue |
| toSet() | std::set<E> | 收集至 set （去重複且排序） |
| toStack() | std::stack<E> | 收集至 stack |
| toUnorderedMap(keyExtractor, valueExtractor) | std::unordered_map<K, V> | 收集至 unordered_map |
| toUnorderedMultimap(keyExtractor) | std::unordered_multimap<K, E> | 依鍵值收集至 unordered_multimap |
| toUnorderedMultimap(keyExtractor, valueExtractor) | std::unordered_multimap<K, V> | 依自訂鍵值收集至 unordered_multimap |
| toUnorderedMultiset() | std::unordered_multiset<E> | 收集至 unordered_multiset |
| toUnorderedSet() | std::unordered_set<E> | 收集至 unordered_set |
| toVector() | std::vector<E> | 收集至 vector |
### 📈 Statistics<E,D> — 統計方法
| 方法 | 傳回型態 | 說明 |
|---|---|---|
| summate() | D | 總和 |
| average() | D | 平均值 |
| minimum() | std::optional<D> | 最小值 |
| maximum() | std::optional<D> | 最大值 |
| range() | D | 全距（範圍） |
| variance() | D | 母體變異數 |
| standardDeviation() | D | 母體標準差 |
| median() | std::optional<D> | 中位數 |
| mode() | std::optional<E> | 眾數 |
| percentile(p) | std::optional<D> | 第 p 百分位數 |
| firstQuartile() | std::optional<D> | 第一四分位數 (Q1) |
| thirdQuartile() | std::optional<D> | 第三四分位數 (Q3) |
| interquartileRange() | std::optional<D> | 四分位距 (IQR) |
| skewness() | D | 偏度 |
| kurtosis() | D | 峰度 |
| frequency() | map<E, complex> | 頻率特徵 |
| distribute() | map<E, complex> | 空間分布特徵 |
| dft() | vector<complex<double>> | 離散傅立葉轉換 |
| idft() | vector<complex<double>> | 逆離散傅立葉轉換 |
| fft() | vector<complex<double>> | 快速傅立葉轉換 |
| ifft() | vector<complex<double>> | 逆快速傅立葉轉換 |
| gradient(...) | vector<double> | 梯度下降法 |
> 💡 以上方法皆支援選擇性的 mapper 參數版本。
> 
### 🔧 Semantic<E> 中間操作方法
| 分類 | 方法 | 說明 |
|---|---|---|
| **元素變換** | map | 一對一映射變換 |
|  | flatMap | 一對多映射並平坦化 |
|  | flat | 平坦化巢狀串流（支援 Semantic 及容器） |
| **元素過濾** | filter | 條件過濾 |
|  | takeWhile | 滿足條件時持續獲取 |
|  | dropWhile | 滿足條件時持續捨棄 |
|  | distinct | 去除重複項（支援自訂比較子） |
| **數量控制** | limit | 限制元素數量 |
|  | skip | 跳過前 n 個元素 |
|  | sub | 擷取部分範圍 [start, end) |
| **索引運算** | redirect | 索引重新映射 |
|  | reverse | 索引反轉 |
|  | translate | 索引偏移 |
| **觀察運算** | peek | 觀察各個元素（不修改串流） |
| **並行宣告** | parallel(n) | 宣告並行度 |
| **連接運算** | concatenate | 連接 Semantic / 元素 / 生成器 / 容器 |
| **最終轉換** | toUnordered / toOrdered / toWindow / toStatistics / sort | 轉換為 Collectable |
## 🔧 第 6 層：semantics.h — 串流構建器工廠
### 🔢 數字範圍生成
| 方法 | 說明 |
|---|---|
| useRange(start, end) | 生成 [start, end) 範圍 |
| useRange(start, end, step) | 具備步進的範圍，支援負數步進 |
| useRangeClosed(start, end) | 生成 [start, end] 閉區間 |
| useRangeClosed(start, end, step) | 具備步進的閉區間 |
### ♾️ 無限串流生成
| 方法 | 說明 |
|---|---|
| useInfinite(seed, generator) | 從種子值開始無限循環生成 |
| useGenerate(supplier) | 無限次呼叫供應者 |
| useGenerate(supplier, limit) | 有限次數呼叫供應者 |
| useIterate(seed, generator) | 從種子值開始無限反覆運算 |
| useIterate(seed, generator, limit) | 有限次數反覆運算 |
| useRandom() | 無限隨機整數串流 |
| useRandom(min, max) | 指定範圍的隨機數串流 |
| useRandom(min, max, count) | 指定範圍與數量的隨機數串流 |
### 📦 容器與元素建構
| 方法 | 說明 |
|---|---|
| useEmpty() | 生成空串流 |
| useOf(element) | 以單一元素生成串流 |
| useOf(e1, e2) | 以兩個元素生成串流 |
| useOf(e1, e2, e3) | 以三個元素生成串流 |
| useOf({...}) | 以初始化清單生成串流 |
| useFrom(container) | 以標準容器生成串流 |
| useFrom({...}) | 以初始化清單生成串流 |
| useRepeat(element, count) | 重複元素 n 次 |
### 📄 文本與 Unicode 處理
| 方法 | 說明 |
|---|---|
| useBlob(text) | 將字串以位元組為單位分割為 char 串流 |
| useBlob(text, start, end) | 將指定範圍以位元組為單位進行分割 |
| useBlob(istream) | 從輸入串流中按行讀取 |
| useBlob(istream, delimiter) | 從輸入串流中依分隔符號讀取 |
| useText(text) | 完整文本串流（Charsequence） |
| useText(text, delimiter) | 依分隔符號分割文本 |
| useText(istream) | 從輸入串流中讀取完整內容 |
| useSequence(charsequence) | 從字元序列生成碼位串流 |
| useSequence(text, encoding) | 從文字中生成指定編碼的碼位串流 |
| useCharsequence(charsequence) | 將字元序列轉換為完整串流 |
| useCharsequence(charsequence, delimiter) | 依分隔符號分割字元序列 |
## 🔐 第 7 層：hash.h / less.h — 容器世界的共通語言
針對所有標準函式庫容器（包含巢狀容器）、pair、tuple、optional、variant、chrono 時間型態、complex 複數等，提供了完整的雜湊與比較支援。現在，即便具有任意深度與組合的巢狀容器，也能直接作為 unordered_set 的鍵值（Key）或 set 的元素使用。 🌉
## 🚀 效能優化秘笈
 1. **選擇適合的容器：** 若順序無關緊要，請使用 toUnordered()；若需要排序，請使用 toOrdered() 或 sort()。
 2. **善用並行處理：** 當資料量龐大時，請搭配使用 parallel()。
 3. **優化操作順序：** 盡可能及早進行 filter，並謹慎使用 sort。
 4. **利用延遲求值：** 透過 takeWhile 與 limit 實現提早結束（Early Exit）。
**Semantic-Cpp** — 用現代化的 C++ 建構高效、清晰的資料處理管線。 🚀🎯✨
