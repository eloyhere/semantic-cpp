# semantic-cpp —— 現代 C++ 語義流函式庫（Semantic Stream）

**semantic-cpp** 為純標頭檔、C++17 起、零外部依賴的高效能惰性流函式庫。  
其核心理念：**每一個元素天生就攜帶一個邏輯索引（Timestamp）**，所有與順序相關的操作本質上都只是對這個索引的重新映射。

- 真正支援無限流  
- 索引為第一級公民，可任意重映射  
- 並行為預設行為  
- 是否尊重索引操作（redirect 等）僅在收集中明確決定  

MIT 授權 — 商用與開源專案皆可完全自由使用。

## 設計理念

> **索引決定順序，執行只決定速度。**

- `redirect`、`reverse`、`shuffle`、`cycle` 等僅改變邏輯索引  
- 只有呼叫 `.toOrdered()` 收集合，這些索引操作才會生效（結果嚴格按照最終索引排序）  
- 呼叫 `.toUnordered()` 時，所有索引操作都會被忽略（極致效能，無順序）

```text
.toOrdered()   → 所有索引操作生效（redirect 等有效）
.toUnordered() → 所有索引操作被忽略（最快路徑）
```

## 快速入門

```cpp
#include "semantic.h"
using namespace semantic;

int main() {
    // 1. 索引操作全部生效
    auto v1 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })   // 反轉
        .toOrdered()                                  // 關鍵！
        .toVector();   // 結果：[99, 98, ..., 0]

    // 2. 索引操作全部被忽略（極致效能）
    auto v2 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })
        .toUnordered()                                // 索引被忽略
        .toVector();   // 結果：無序，但最快

    // 3. 無限循環流 + 有序收集
    auto top10 = Semantic<int>::range(0, 1'000'000'000)
        .redirect([](auto&, auto i) { return i % 1000; }) // 0~999 循環
        .toOrdered()
        .limit(10)
        .toVector();   // [0,1,2,3,4,5,6,7,8,9]

    return 0;
}
```

## 完整鏈式呼叫範例

```cpp
// 流建立
Semantic<int>::range(0, 1000);
Semantic<long long>::range(0, 1LL<<60);
Semantic<int>::iterate([](auto yield, auto) {      // 無限流
    for (int i = 0;; ++i) yield(i);
});
Semantic<int>::of(1, 2, 3, 4, 5);
Semantic<int>::from(std::vector{1,2,3,4});
Semantic<int>::fill(42, 1'000'000);
Semantic<int>::fill([] { return rand(); }, 1000);

// 中間操作（全部惰性）
stream.map([](int x) { return x * x; });
stream.filter([](int x) { return x % 2 == 0; });
stream.flatMap([](int x) { return Semantic<int>::range(0, x); });
stream.distinct();                                     // 值去重
stream.skip(100);
stream.limit(50);
stream.takeWhile([](int x) { return x < 1000; });
stream.dropWhile([](int x) { return x < 100; });
stream.peek([](int x) { std::cout << x << ' '; });

// 關鍵：索引重映射操作（僅在 toOrdered 時生效）
stream.redirect([](auto&, auto i) { return -i; });           // 反轉
stream.redirect([](auto&, auto i) { return i % 100; });      // 循環
stream.redirect([](auto e, auto i) { return std::hash<int>{}(e); }); // 打亂
stream.reverse();                                            // 等同 redirect(-i)
stream.shuffle();

// 並行
stream.parallel();       // 使用所有核心
stream.parallel(8);      // 指定執行緒數

// 必須明確選擇是否尊重索引
auto ordered   = stream.toOrdered();     // 所有 redirect 等操作生效
auto unordered = stream.toUnordered();   // 所有 redirect 等操作被忽略（最快）

// 有序收集（最終索引決定順序）
ordered.toVector();
ordered.toList();
ordered.toSet();                 // 依最終索引去重
ordered.forEach([](int x) { std::cout << x << ' '; });
ordered.cout();                  // [99, 98, 97, …]

// 無序收集（最快路徑）
unordered.toVector();
unordered.toList();
unordered.toSet();
unordered.forEach(...);
unordered.cout();

// 統計（永遠走最快路徑）
auto stats = stream.toUnordered().toStatistics();

// reduce（建議使用無序路徑）
int sum = stream.toUnordered()
    .reduce(0, [](int a, int b) { return a + b; });
```

## 方法總覽

| 方法                             | 說明                                               | 是否尊重索引操作 |
|----------------------------------|----------------------------------------------------|------------------|
| `toOrdered()`                    | 進入「語義模式」— 所有索引操作生效               | 是               |
| `toUnordered()`                  | 進入「效能模式」— 所有索引操作被忽略             | 否（最快）       |
| `toVector()` / `toList()`        | 收集至容器                                        | 依選擇而定       |
| `toSet()`                        | 收集至 set（去重）                                | 依選擇而定       |
| `forEach` / `cout`               | 遍歷或輸出                                        | 依選擇而定       |
| `redirect` / `reverse` / `shuffle` | 改變邏輯索引                                  | 僅在 toOrdered 時生效 |
| `parallel`                       | 並行執行                                          | 兩者皆支援       |

## 為何必須明確選擇 toOrdered / toUnordered？

因為「索引轉換」與「並行執行」是完全正交的兩個維度：

- `redirect` 等屬於語義操作（這個元素在邏輯上應該出現在哪裡？）  
- `parallel` 僅是執行策略（要算多快？）

只有在收集時明確選擇，才能同時擁有極致速度與精確順序控制。

## 編譯需求

- C++17 或更高版本  
- 僅需 `#include "semantic.h"`  
- 零外部依賴  
- 單一標頭檔函式庫

```bash
g++ -std=c++17 -O3 -pthread semantic.cpp
```

## 授權

MIT 授權 — 商用、開源、私有專案皆可無限制使用。

---

**semantic-cpp**：  
**索引決定順序，執行只管快慢。**  
寫 `redirect` 就真的反轉，寫 `toUnordered()` 就真的最快。  
沒有妥協，只有選擇。

寫流，不再需要猜。  
只說一句：**「我要語義，還是要效能？」**
