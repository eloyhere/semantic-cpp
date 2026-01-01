# semantic-cpp – 時間意味論を備えた現代的な C++ ストリームライブラリ

**semantic-cpp** は、軽量かつ高性能な現代 C++17 向けストリーム処理ライブラリです。単一のヘッダーファイル（`semantic.h`）と独立した実装ファイル（`semantic.cpp`）で構成されています。このライブラリは、Java Stream の流暢さ、JavaScript Generator と Promise の遅延評価、データベース索引のような順序付け機能、そして金融アプリケーション、IoT データ処理、イベント駆動システムに不可欠な組み込みの時間意識を融合しています。

## 主な特徴

- 各要素は **Timestamp**（符号付き `long long` で負値対応）と **Module**（符号なし `long long` でカウントや並行処理に使用）に関連付けられています。
- `.toOrdered()`、`.toUnordered()`、`.toWindow()`、`.toStatistics()` で実体化するまでストリームは遅延評価されます。
- 実体化はパイプラインを終了させず、その後も完全にチェーン呼び出しが可能です（「ポストターミナル」ストリーム）。
- 並行実行、スライディングおよびタンブリングウィンドウ、先進的な統計演算、そして JavaScript スタイルの **Promise** クラスによる非同期タスクを全面的にサポート。

## なぜ semantic-cpp か？

| 機能                             | Java Stream | ranges-v3 | semantic-cpp                          |
|----------------------------------|-------------|-----------|---------------------------------------|
| 遅延評価                         | あり        | あり      | あり                                  |
| 時間索引（符号付きタイムスタンプ）| なし        | なし      | あり（コアコンセプト）                |
| スライディング／タンブリングウィンドウ | なし        | なし      | あり（ファーストクラスサポート）      |
| 組み込み統計演算                 | なし        | なし      | あり（平均、中央値、最頻値、歪度、尖度など）|
| 並行実行（オプトイン）           | あり        | あり      | あり（スレッドプール + Promise サポート）|
| ターミナル操作後もチェーン継続   | なし        | なし      | あり（ポストターミナルストリーム）    |
| 非同期 Promise                   | なし        | なし      | あり（JavaScript スタイル）           |
| 単一ヘッダー + 実装ファイル、C++17 | なし        | あり      | あり                                  |

時系列データ、マーケットフィード、センサーストリーム、ログに対してカスタムウィンドウ処理、統計、非同期コードを頻繁に書いている場合、semantic-cpp はそのボイラープレートを排除します。

## クイックスタート

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // 値からストリームを作成（タイムスタンプは 0 から順次自動割り当て）
    auto stream = of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    // 偶数をフィルタリング、並行実行、統計のために実体化
    auto stats = stream
        .filter([](int x) { return x % 2 == 0; })
        .parallel(8)
        .toStatistics<int>();

    std::cout << "平均: " << stats.mean() << '\n';      // 5
    std::cout << "中央値: " << stats.median() << '\n';  // 5
    std::cout << "最頻値: " << stats.mode() << '\n';    // 任意の偶数

    // ウィンドウ例
    auto windows = of(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
        .toWindow()
        .getSlidingWindows(5, 2);  // サイズ 5、ステップ 2 のスライディングウィンドウ

    return 0;
}
```

## コアコンセプト

### 1. ファクトリ関数 – ストリームの作成

`semantic` 名前空間のフリー関数でストリームを作成：

```cpp
auto s = of(1, 2, 3, 4, 5);                          // 可変引数
auto empty = empty<int>();                          // 空ストリーム
auto filled = fill(42, 1'000'000);                  // 繰り返し値
auto supplied = fill([]{ return rand(); }, 1'000'000);
auto ranged = range(0, 100);                        // 0 .. 99
auto ranged_step = range(0, 100, 5);                // 0, 5, 10, ...
auto from_vec = from(std::vector<int>{1, 2, 3});
auto iterated = iterate([](auto push, auto stop) {
    for (int i = 0; i < 100; ++i) push(i, i);       // 明示的タイムスタンプ
});
```

I/O ヘルパーも用意：`lines(stream)`、`chunks(stream, size)`、`text(stream)` など。

### 2. Semantic – 遅延ストリーム

コア型 `Semantic<E>` は標準操作をサポート：

```cpp
stream.filter(...).map(...).skip(10).limit(100).peek(...)
```

タイムスタンプ操作：

```cpp
stream.translate(+1000)          // すべてのタイムスタンプをシフト
    .redirect([](auto elem, Timestamp t) { return t * 10; });
```

並行処理：

```cpp
stream.parallel()                // デフォルトスレッド数
    .parallel(12);               // ワーカースレッド数を指定
```

### 3. 実体化

収集可能な形式に変換：

- `.toOrdered()` – 順序を保持、ソート有効
- `.toUnordered()` – 最速、無順序
- `.toWindow()` – 順序付き、完全ウィンドウサポート
- `.toStatistics<R>(mapper)` – 順序付き、統計メソッド

その後もチェーン可能：

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
auto windows = stream.toWindow().getSlidingWindows(30, 10);
auto tumbling = stream.toWindow().getTumblingWindows(50);
```

ウィンドウのストリームを発行：

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

std::cout << "平均:      " << stats.mean() << '\n';
std::cout << "中央値:    " << stats.median() << '\n';
std::cout << "標準偏差:  " << stats.standardDeviation() << '\n';
std::cout << "歪度:      " << stats.skewness() << '\n';
std::cout << "尖度:      " << stats.kurtosis() << '\n';
```

結果は積極的にキャッシュされ、パフォーマンスが向上します。

### 6. Promise による非同期実行

JavaScript Promise に着想を得た `Promise<T, E>` クラスで、`ThreadPool` に提出する非同期タスクを扱います：

```cpp
ThreadPool pool(8);

auto promise = pool.submit<int>([] {
    // 重い計算
    return 42;
});

promise.then([](int result) {
        std::cout << "結果: " << result << '\n';
    })
    .except([](const std::exception& e) {
        std::cerr << "エラー: " << e.what() << '\n';
    })
    .finally([] {
        std::cout << "完了\n";
    });

promise.wait();  // 必要に応じてブロック
```

静的ヘルパー：`Promise<T>::all(...)`、`Promise<T>::any(...)`、`resolved(value)`、`rejected(error)`。

## インストール

`semantic.h` と `semantic.cpp` をプロジェクトにコピーするか、CMake を使用：

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## 例のビルド

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## ベンチマーク（Apple M2 Max、2024）

| 操作                               | Java Stream | ranges-v3 | semantic-cpp（並行） |
|------------------------------------|-------------|-----------|----------------------|
| 1億整数 → 合計                     | 280 ms      | 190 ms    | **72 ms**            |
| 1000万倍精度 → スライディングウィンドウ平均 | N/A         | N/A       | **94 ms**（ウィンドウ 30、ステップ 10） |
| 5000万整数 → toStatistics          | N/A         | N/A       | **165 ms**           |

## コントリビューション

コントリビューションを心より歓迎します！注目すべき領域：

- 追加のコレクター（パーセンタイル、共分散など）
- レンジライブラリとのより良い統合
- オプションの SIMD アクセラレーション

## ライセンス

MIT © Eloy Kim

C++ で真の意味論的ストリームをお楽しみください！