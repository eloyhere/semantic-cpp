# Semantic Stream —— 現代 C++ 関数型ストリームライブラリ

Semantic は、ヘッダオンリー・高性能・完全遅延評価の C++17 以降向け関数型ストリームライブラリです。  
JavaScript Generator、Java Stream API、Java Function パッケージ、そして MySQL のインデックス設計思想を融合して生まれました。

有限シーケンスだけでなく、無限シーケンスも自然に扱えるよう設計されており、関数型プログラミングの表現力と真の遅延評価・並列実行・時間印ベースのインデックスを兼ね備えています。

## 主な特長

- **完全遅延評価** — 中間操作は終端操作が呼ばれるまで一切実行されません。
- **無限ストリームのネイティブサポート** — Generator により無限シーケンスを簡単に構築可能。
- **時間印インデックス機構** — すべての要素に暗黙的または明示的なタイムスタンプが付与され、MySQL のインデックスのような高速スキップ・シークを実現。
- **ワンライナーで並列化** — `.parallel()` または `.parallel(n)` のみでマルチスレッドパイプラインに変換。
- **豊富な関数型操作** — `map`、`filter`、`flatMap`、`reduce`、`collect`、`group`、統計処理など一通り完備。
- **Java 互換 Collector API** — supplier、accumulator、combiner、finisher の4段階構成。
- **強力な統計機能** — 平均・中央値・最頻値・分散・標準偏差・四分位数・歪度・尖度など。
- **多様な生成方法** — コンテナ、配列、範囲、ジェネレータ、fill などから即座にストリーム作成。

## 設計思想

Semantic は「時間印付きの要素列を Generator が生成する」というモデルを採用しています。以下の技術からインスピレーションを得ています：

- **JavaScript Generator** — プル型遅延生成
- **Java Stream** — 流暢なチェーン構文と中間・終端操作の分離
- **Java Function パッケージ** — `Function`、`Consumer`、`Predicate` などの型エイリアス
- **MySQL インデックス** — 論理タイムスタンプによる効率的な `skip`、`limit`、`redirect`、`translate`

## コアコンセプト

```cpp
using Generator<E> = BiConsumer<
    BiConsumer<E, Timestamp>,   // yield(要素, タイムスタンプ)
    Predicate<E>                // キャンセル判定
>;
```

ジェネレータは yield 関数とキャンセル判定関数を受け取ります。この低レベル抽象が有限・無限すべてのストリームの源泉です。

## クイック例

```cpp
using namespace semantic;

// 無限乱数ストリーム
auto s = Semantic<int>::iterate([](auto yield, auto cancel) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> d(1, 100);
    Timestamp ts = 0;
    while (!cancel(d(gen))) {
        yield(d(gen), ts++);
    }
});

// 遅延評価による素数生成（有限範囲）
auto primes = Semantic<long long>::range(2LL, 1'000'000LL)
    .filter([](long long n) {
        return Semantic<long long>::range(2LL, n)
            .takeWhile([n](long long d){ return d * d <= n; })
            .noneMatch([n](long long d){ return n % d == 0; });
    });

// 並列単語カウント
auto wordCount = Semantic<std::string>::from(lines)
    .flatMap([](const std::string& line) {
        return Semantic<std::string>::from(split(line));
    })
    .parallel()
    .group([](const std::string& w){ return w; })
    .map([](auto& p){ return std::make_pair(p.first, p.second.size()); });
```

## ビルド要件

- C++17 以上
- ヘッダオンリー — `#include "semantic.hpp"` のみで使用可能
- 外部依存なし

グローバルスレッドプール `semantic::globalThreadPool` は起動時に自動的に `std::thread::hardware_concurrency()` 個のワーカースレッドで初期化されます。

## ライセンス

MIT License — 商用・オープンソース問わず自由に利用可能です。

## 作者

現代関数型プログラミングの優れたアイデアを C++ 向けに高性能かつイディオマティックに再構築したライブラリです。
