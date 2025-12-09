# semantic-cpp – 時系列を意識したモダン C++ ストリームライブラリ

**semantic-cpp** は C++17 向けのヘッダーオンリー、高性能ストリーム処理ライブラリです。  
Java Stream の流暢さ、JavaScript Generator の遅延評価、MySQL インデックスの順序付け、そして金融・IoT・イベント駆動システムに不可欠な**時系列（temporal）意識**を融合しています。

これが他と一線を画すポイント：

- すべての要素は **Timestamp**（符号付き `long long`、負も可）を必ず保持  
- **Module** は非負の `unsigned long long` で、カウントと並列度を表現  
- `.toOrdered()` / `.toUnordered()` / `.toWindow()` / `.toStatistics()` を呼ぶまで完全に遅延評価  
- 終端操作後もそのままチェーンを続けられる「**後終端（post-terminal）**」設計  
- 並列実行、スライディング／タンブリングウィンドウ、豊富な統計コレクターを標準搭載  

## なぜ semantic-cpp なのか？

| 機能                              | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                               |
|-----------------------------------|-------------|-------------|-----------|--------------|--------------------------------------------|
| 遅延評価                          | Yes         | Yes         | Yes       | No           | Yes                                        |
| 符号付きタイムスタンプ（時系列）  | No          | No          | No        | No           | Yes（中核機能）                            |
| スライディング／タンブリングウィンドウ | No          | No          | No        | No           | Yes（第一級サポート）                      |
| 統計コレクター内蔵                | No          | No          | No        | No           | Yes（平均・中央値・最頻値・尖度・歪度…）   |
| デフォルト並列（opt-in）          | Yes         | No          | Yes       | No           | Yes（グローバルスレッドプールorカスタム） |
| 終端操作後も継続チェーン可能      | No          | No          | No        | No           | Yes（後終端ストリーム）                    |
| ヘッダーオンリー・C++17           | No          | Yes         | Yes       | Yes          | Yes                                        |

市場データ、センサーデータ、ログ解析などで毎回同じウィンドウ・統計コードを書いているなら、**semantic-cpp** がそのボイラープレートを根こそぎ消してくれます。

## クイックスタート

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // タイムスタンプ 0〜99 の100個の整数ストリーム
    auto stream = from(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream
        .filter([](int x) { return x % 2 == 0; })   // 偶数のみ
        .parallel(8)                                // 8スレッド
        .toWindow()                                 // 物化＋ウィンドウ機能有効化
        .getSlidingWindows(10, 5)                   // サイズ10、ステップ5のスライディングウィンドウ
        .toVector();                                // → std::vector<std::vector<int>>

    // 統計例
    auto stats = from({1,2,3,4,5,6,7,8,9,10})
                  .toStatistics<double>();      // mapperは自動的にidentity

    std::cout << "平均値       : " << stats.mean()            << '\n';
    std::cout << "中央値       : " << stats.median()          << '\n';
    std::cout << "最頻値       : " << stats.mode()            << '\n';
    std::cout << "標準偏差     : " << stats.standardDeviation() << '\n';
    stats.cout();  // 一発で綺麗に出力

    return 0;
}
```

## 主要概念

### 1. `Semantic<E>` – 遅延ストリーム

```cpp
Semantic<int> s = of(1, 2, 3, 4, 5);
```

すべての古典的操作が使えます：

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

各要素は **Timestamp**（符号付きインデックス）と一緒に放出されます。タイムスタンプは自由に変換可能：

```cpp
s.translate(+1000)                                          // 一括シフト
   .redirect([](int x, Timestamp t){ return x * 10LL; })    // 任意ロジック
```

### 2. 物化（Materialisation） – コストが発生する唯一の場所

`count()`、`toVector()`、`cout()` などを使う前に、以下のいずれかを呼ぶ必要があります：

```cpp
.toOrdered()        // 元の順序を保持、ソート可能
.toUnordered()      // 最速、無順序保証
.toWindow()         // 順序付き＋強力なウィンドウAPI
.toStatistics<D>()  // 順序付き＋統計メソッド
```

物化後もチェーンを継続できます：

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 3. ウィンドウ処理

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // サイズ30、ステップ10
```

ウィンドウ自体をストリーム化：

```cpp
stream.toWindow()
      .windowStream(50, 20)           // 各要素が std::vector<E>
      .map([](const std::vector<double>& w){ return mean(w); })
      .toOrdered()
      .cout();
```

### 4. 統計機能

```cpp
auto stats = from(prices)
              .toStatistics<double>([](double p){ return p; });

std::cout << "平均値       : " << stats.mean()      << "\n";
std::cout << "中央値       : " << stats.median()    << "\n";
std::cout << "標準偏差     : " << stats.standardDeviation() << "\n";
std::cout << "歪度         : " << stats.skewness()  << "\n";
std::cout << "尖度         : " << stats.kurtosis()  << "\n";
```

頻度テーブルは初回のみ計算され、その後は完全にキャッシュされます。

### 5. 並列実行

```cpp
globalThreadPool          // 起動時に hardware_concurrency 分自動作成
stream.parallel()         // グローバルプール使用
stream.parallel(12)       // 強制12スレッド
```

各 `Collectable` が自分の並列度を保持し、チェーン全体で正しく継承されます。

## ファクトリ関数

```cpp
empty<T>()                              // 空ストリーム
of(1,2,3,"こんにちは")                 // 可変引数
fill(42, 1'000'000)                     // 100万個の42
fill([]{return rand();}, 1'000'000)     // 動的生成
from(コンテナ)                          // vector/list/set/array/initializer_list
range(0, 100)                           // 0〜99
range(0, 100, 5)                        // 0,5,10,…
iterate(カスタムジェネレータ)            // 独自Generator
```

## インストール

ヘッダーオンリーです。`semantic.h` をプロジェクトにコピーするか、CMake で：

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## 例のビルド

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp -o basic
./basic
```

## ベンチマーク（Apple M2 Max, 2024）

| 操作                                    | Java Stream | ranges-v3 | semantic-cpp（並列） |
|-----------------------------------------|-------------|-----------|----------------------|
| 1億整数の合計                           | 280 ms      | 190 ms    | 72 ms                |
| 1000万 double のスライディング平均（30窓、ステップ10） | N/A | N/A       | 94 ms                |
| 5000万整数の toStatistics               | N/A         | N/A       | 165 ms               |

## コントリビューション大歓迎！

特に欲しい機能：

- 追加のコレクター（パーセンタイル、共分散など）
- 既存 range ライブラリとの相互運用性向上
- 単純 mapper 向けの SIMD 最適化（オプション）

CONTRIBUTING.md をご一読ください。

## ライセンス

MIT © Eloy Kim

C++ で本物の**時系列ストリーム**を楽しみましょう！
