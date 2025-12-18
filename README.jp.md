# semantic-cpp – 時間的意味を持つ現代的な C++ ストリームライブラリ

***
**Jonathan Wakely 氏の指導のもと**  
(GCC libstdc++ メンテナおよび ISO C++ 委員会メンバー)
***

**semantic-cpp** は、ヘッダーファイル1つと実装ファイル1つで構成される軽量・高性能な C++17 用ストリーム処理ライブラリです。Java Stream の流暢さ、JavaScript Generator の遅延評価、データベースインデックスの順序マッピング、そして金融・IoT・イベント駆動型システムに必要な時間的意識を融合しています。

独自性を支える主要な設計思想：

- すべての要素は **Timestamp**（符号付き `long long` インデックス、負も可）を持ちます。
- **Module** は符号なし `unsigned long long` で、カウントと並行度レベルに使用されます。
- `.toOrdered()`、`.toUnordered()`、`.toWindow()`、`.toStatistics()` を呼び出すまでストリームは遅延評価を維持します。
- 終端操作後もチェーンを継続可能——ライブラリは意図的に「ポストターミナル」ストリームをサポートしています。
- 並列実行、スライディング/タンブリングウィンドウ、豊富な統計機能の完全対応。

## semantic-cpp を選ぶ理由

| 機能                                 | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                   |
|--------------------------------------|-------------|-------------|-----------|--------------|------------------------------------------------|
| 遅延評価                             | あり        | あり        | あり      | なし         | あり                                           |
| 時間インデックス（符号付きタイムスタンプ） | なし        | なし        | なし      | なし         | あり（コアコンセプト）                         |
| スライディング/タンブリングウィンドウ | なし        | なし        | なし      | なし         | あり（ファーストクラスサポート）               |
| 組み込み統計機能                     | なし        | なし        | なし      | なし         | あり（平均、中央値、最頻値、尖度など）         |
| 並列実行（オプトイン）               | あり        | なし        | あり      | なし         | あり（グローバルスレッドプールまたはカスタム） |
| 終端操作後もチェーン継続             | なし        | なし        | なし      | なし         | あり（ポストターミナルストリーム）             |
| 単一ヘッダー＋実装ファイル、C++17   | なし        | あり        | あり      | あり         | あり                                           |

市場データ、センサーストリーム、ログ解析などで同じウィンドウ処理や統計コードを繰り返し書いている場合、semantic-cpp はそのボイラープレートを排除します。

## クイックスタート

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // タイムスタンプ 0..99 の 100 個の数値ストリームを作成
    auto stream = Generative<int>{}.of(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream.filter([](int x) { return x % 2 == 0; })   // 偶数のみ
          .parallel(8)                               // 8 スレッド使用
          .toWindow()                                // ウィンドウ対応で具現化
          .getSlidingWindows(10, 5)                  // ウィンドウサイズ 10、ステップ 5
          .toVector();

    // 統計例
    auto stats = Generative<int>{}.of(1,2,3,4,5,6,7,8,9,10).toStatistics();
    std::cout << stats.mean()   << "\n";   // 5.5
    std::cout << stats.median() << "\n";   // 5.5
    std::cout << stats.mode()   << "\n";   // 多峰性の場合は最初に見つかった値
    stats.cout();

    return 0;
}
```

## コアコンセプト

### 1. Generative<E> – ストリームファクトリ

`Generative<E>` はストリーム作成のための便利なインターフェースを提供します：

```cpp
Generative<int> gen;
auto s = gen.of(1, 2, 3, 4, 5);
auto empty = gen.empty();
auto filled = gen.fill(42, 1'000'000);
auto ranged = gen.range(0, 100, 5);
```

すべてのファクトリ関数は遅延評価の `Semantic<E>` を返します。

### 2. Semantic<E> – 遅延ストリーム

```cpp
Semantic<int> s = of<int>(1, 2, 3, 4, 5);
```

標準的な操作が利用可能です：

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

要素はタイムスタンプを持ち、操作可能です：

```cpp
s.translate(+1000)                                      // すべてのタイムスタンプをシフト
 .redirect([](int x, Timestamp t){ return x * 10; })   // カスタムタイムスタンプロジック
```

### 3. 具現化（コストが発生する唯一の場所）

収集操作（count()、toVector()、cout() など）を使用する前に、以下の4つの終端変換子のいずれかを呼び出す必要があります：

```cpp
.toOrdered()      // 順序を保持し、ソートを可能に
.toUnordered()    // 最速、順序保証なし
.toWindow()       // 順序付き + 完全なウィンドウ API
.toStatistics<D>  // 順序付き + 統計メソッド
```

具現化後もチェーンを継続できます：

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 4. ウィンドウ処理

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // サイズ 30、ステップ 10
```

またはウィンドウのストリームを生成：

```cpp
stream.toWindow()
    .windowStream(50, 20)           // 各ウィンドウごとに std::vector<E> を発行
    .map([](const std::vector<double>& w) { return mean(w); })
    .toOrdered()
    .cout();
```

### 5. 統計機能

```cpp
auto stats = from(prices).toStatistics<double>([](auto p){ return p; });

std::cout << "平均      : " << stats.mean()      << "\n";
std::cout << "中央値    : " << stats.median()    << "\n";
std::cout << "標準偏差  : " << stats.standardDeviation() << "\n";
std::cout << "歪度      : " << stats.skewness()  << "\n";
std::cout << "尖度      : " << stats.kurtosis()  << "\n";
```

すべての統計関数は強力にキャッシュされます（頻度マップは1回のみ計算）。

### 6. 並列処理

```cpp
globalThreadPool   // ハードウェア同時実行数で自動作成
stream.parallel()  // グローバルプールを使用
stream.parallel(12) // 厳密に12ワーカースレッドを強制
```

並行度レベルはチェーン全体で正しく継承されます。

## ファクトリ関数（Generative 経由）

```cpp
empty<T>()                              // 空ストリーム
of(1,2,3,"hello")                       // 可変引数
fill(42, 1'000'000)                     // 繰り返し値
fill([]{return rand();}, 1'000'000)     // サプライヤーによる生成
from(container)                         // vector、list、set、array、initializer_list、queue 対応
range(0, 100)                           // 0 .. 99
range(0, 100, 5)                        // 0,5,10,…
iterate(generator)                      // カスタム Generator
```

## インストール

semantic-cpp はヘッダーと実装ファイルで構成されます。`semantic.h` と `semantic.cpp` をプロジェクトにコピーするか、CMake で統合してください：

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## 例のコンパイル

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## ベンチマーク（Apple M2 Max, 2024）

| 操作                              | Java Stream | ranges-v3 | semantic-cpp (並列) |
|-----------------------------------|-------------|-----------|---------------------|
| 1億整数の合計                     | 280 ms      | 190 ms    | 72 ms               |
| 1000万倍精度数のスライディング平均 | N/A         | N/A       | 94 ms（ウィンドウ30、ステップ10） |
| 5000万整数の toStatistics         | N/A         | N/A       | 165 ms              |

## 貢献

貢献を大歓迎します！現在特に必要な領域：

- 追加のコレクター（パーセンタイル、共分散など）
- 既存レンジライブラリとのより良い統合
- 単純マッパー向けのオプション SIMD 高速化

CONTRIBUTING.md をお読みください。

## ライセンス

MIT © Eloy Kim

真に意味のある C++ ストリームをお楽しみください！
