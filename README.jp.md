# semantic-cpp —— 現代 C++ 語義ストリームライブラリ

**semantic-cpp** はヘッダオンリー、C++17 以降対応、ゼロ外部依存の高性能遅延評価ストリームライブラリです。  
核心思想：**すべての要素は生まれながらにして論理インデックス（Timestamp）を持つ**。順序に関するすべての操作は、このインデックスの変換に過ぎません。

- 真の無限ストリーム対応  
- インデックスは第一級市民、自由にリマップ可能  
- 並列実行がデフォルト  
- インデックス操作（redirect 等）を有効にするか無視するかは、**収集時のみ**で明示的に決定  

MIT ライセンス — 商用・オープンソース問わず自由に使用可能。

## 設計思想

> **インデックスが順序を決め、実行は速度だけを決める。**

- `redirect`、`reverse`、`shuffle`、`cycle` 等は論理インデックスだけを変更  
- `.toOrdered()` で収集 → すべてのインデックス操作が有効（最終インデックス順にソート）  
- `.toUnordered()` で収集 → すべてのインデックス操作を無視（最速、無順序）

```text
.toOrdered()   → すべてのインデックス操作が有効
.toUnordered() → すべてのインデックス操作が無視される（最速パス）
```

## クイックスタート

```cpp
#include "semantic.h"
using namespace semantic;

int main() {
    // 1. インデックス操作がすべて有効になる
    auto v1 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })   // 逆順
        .toOrdered()                                  // 必須！
        .toVector();   // 結果: [99, 98, ..., 0]

    // 2. インデックス操作がすべて無視される（最速）
    auto v2 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })
        .toUnordered()                                // インデックス無視
        .toVector();   // 結果: 順序不定だが最速

    // 3. 無限循環ストリーム + 有序収集
    auto top10 = Semantic<int>::range(0, 1'000'000'000)
        .redirect([](auto&, auto i) { return i % 1000; }) // 0-999 循環
        .toOrdered()
        .limit(10)
        .toVector();   // [0,1,2,3,4,5,6,7,8,9]

    return 0;
}
```

## 主要メソッドとチェーン例

```cpp
// ストリーム生成
Semantic<int>::range(0, 1000);
Semantic<long long>::range(0, 1LL<<60);
Semantic<int>::iterate([](auto yield, auto) {      // 無限ストリーム
    for (int i = 0;; ++i) yield(i);
});
Semantic<int>::of(1, 2, 3, 4, 5);
Semantic<int>::from(std::vector{1,2,3,4});
Semantic<int>::fill(42, 1'000'000);
Semantic<int>::fill([] { return rand(); }, 1000);

// 中間操作（すべて遅延評価）
stream.map([](int x) { return x * x; });
stream.filter([](int x) { return x % 2 == 0; });
stream.flatMap([](int x) { return Semantic<int>::range(0, x); });
stream.distinct();                                     // 値ベース重複除去
stream.skip(100);
stream.limit(50);
stream.takeWhile([](int x) { return x < 1000; });
stream.dropWhile([](int x) { return x < 100; });
stream.peek([](int x) { std::cout << x << ' '; });

// インデックス変換操作（toOrdered() のときのみ有効）
stream.redirect([](auto&, auto i) { return -i; });           // 逆順
stream.redirect([](auto&, auto i) { return i % 100; });      // 循環
stream.redirect([](auto e, auto i) { return std::hash<int>{}(e); }); // シャッフル
stream.reverse();                                            // redirect(-i) と同等
stream.shuffle();

// 並列実行
stream.parallel();       // 全コア使用
stream.parallel(8);      // スレッド数指定

// 必須：インデックスを尊重するか明示的に選択
auto ordered   = stream.toOrdered();     // すべてのインデックス操作が有効
auto unordered = stream.toUnordered();   // すべてのインデックス操作が無視される（最速）

// 有序収集（最終インデックス順に並ぶ）
ordered.toVector();
ordered.toList();
ordered.toSet();                 // 最終インデックス順に重複除去
ordered.forEach([](int x) { std::cout << x << ' '; });
ordered.cout();                  // [99, 98, 97, …]

// 無序収集（最速パス）
unordered.toVector();
unordered.toList();
unordered.toSet();
unordered.forEach(...);
unordered.cout();

// 統計（常に最速パス）
auto stats = stream.toUnordered().toStatistics();

// reduce（unordered推奨）
int sum = stream.toUnordered()
    .reduce(0, [](int a, int b) { return a + b; });
```

## メソッド一覧

| メソッド                         | 説明                                               | インデックス操作を尊重するか |
|----------------------------------|----------------------------------------------------|------------------------------|
| `toOrdered()`                    | 意味的モード — すべてのインデックス操作が有効     | はい                         |
| `toUnordered()`                  | 性能優先モード — すべてのインデックス操作が無視   | いいえ（最速）               |
| `toVector()` / `toList()`        | コンテナへ収集                                    | 選択による                   |
| `toSet()`                        | set へ収集（重複除去）                            | 選択による                   |
| `forEach` / `cout`               | 走査・出力                                        | 選択による                   |
| `redirect` / `reverse` / `shuffle` | 論理インデックス変換                        | toOrdered() のときのみ有効   |
| `parallel`                       | 並列実行                                          | 両モード対応                 |

## なぜ toOrdered / toUnordered を明示的に選ぶ必要があるのか？

**インデックス変換**と**並列実行**は完全に直交する2つの次元だからです。

- `redirect` 等は意味的（semantic）な操作 → 「この要素は論理的にどこにいるべきか？」  
- `parallel` は実行戦略 → 「どれだけ速く計算するか？」

収集時に明示的に選択することで、最高速と正確な順序制御の両方を手に入れられます。

## コンパイル要件

- C++17 以上  
- `#include "semantic.h"` のみ  
- 外部依存ゼロ  
- 単一ヘッダライブラリ

```bash
g++ -std=c++17 -O3 -pthread semantic.cpp
```

## ライセンス

MIT ライセンス — 商用・オープンソース・プライベートプロジェクトで無制限に使用可能。

---

**semantic-cpp**：  
**インデックスが順序を決め、実行は速度だけを決める。**  
`redirect` と書けば本当に逆順になり、`toUnordered()` と書けば本当に最速になる。  
妥協なし、選択のみ。

ストリームを書くとき、もう推測する必要はありません。  
ただ一言：**「意味が欲しいか、速度が欲しいか？」**
