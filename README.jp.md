# 🚀 Semantic-Cpp：将来志向のC++インテリジェントストリーム処理フレームワーク

Semantic-Cppは、完全に再設計されたモダンなC++ストリーム処理ライブラリで、**「複数ヘッダー、外部依存ゼロ」**のモジュラーアーキテクチャを特徴としています。各ヘッダーファイルは明確で単一の責任を持ち、独立してテスト可能であり、全体で完全なストリーム処理エコシステムを形成します。このライブラリは、複数のプログラミングパラダイムの本質を革新的に融合しています：

*   **Java Stream APIの優雅さと流暢さ**：チェーン呼び出し、宣言的プログラミング、コードを詩のように優雅にする ✨
*   **JavaScriptジェネレータの遅延性と柔軟性**：遅延評価、オンデマンド生成、メモリに優しい 🌱
*   **データベースインデックス作成の効率性と順序性**：インテリジェントなソート、インデックス駆動、時系列データ処理の強力なツール ⏱️
*   **「コンテナとしての要素」のバッチ処理哲学**：ベクター、リスト、マップ… あらゆるコンテナがストリーム内の第一級市民となり、自由に流れる 📦

ベクターを反復処理するために`for`ループを書き、フィルタリングするために`if`をネストし、別のコンテナに手動で`push_back`するのにうんざりしていませんか？😩
逆方向に反復処理する際に「最後から3番目の」要素を取得したいために、深夜にオフバイワンインデックスのバグをデバッグしたことがありますか？😵💫
データベースのようにデータを操作したいと憧れていませんか？— インデックスで正確に位置特定、スライディングウィンドウで分析、単一の呼び出しチェーンでデータから統計までの完全な旅を完了する？🤔

**Semantic-Cppはこのために生まれました。🔧**

データ処理を「要素」とその「論理的位置（インデックス）」に対する操作として抽象化します — データベースの「行」と「主キー」に似ています。データ自体に触れることなく、インデックスを自由に並べ替え、オフセットし、反転できます。また、任意のコンテナ（`vector`、`map`、`array`…）をストリーム内で分割不可能な全体として渡し、いつでも要素レベルに「アンパック」できます。この2つの粒度を自由に切り替える能力は、従来のストリームフレームワークには欠けています。🎯

---

## 🏗️ プロジェクトアーキテクチャ：7層モジュラーデザイン

Semantic-Cppは、7つのコアヘッダーファイルで構成され、層状に構築されています。各ファイルは単一の責任を持ち、独立してテストされます。5つの名前空間がそれぞれの役割を持ち、データソースから最終結果までの完全なパイプラインを形成します：

```
┌─────────────────────────────────────────────────┐
│               🌊 semantics.h                     │
│   名前空間: semantic                            │
│   ストリーム構築ファクトリ：数値範囲、コンテナ、  │
│   テキスト、Unicode                             │
├─────────────────────────────────────────────────┤
│                 📦 semantic.h                   │
│   名前空間: semantic / collectable              │
│   ストリーム中間操作、Collectableシステム、      │
│   コンテナアンパックサポート                    │
├─────────────────────────────────────────────────┤
│                ⚙️ collector.h                   │
│   名前空間: collector                           │
│   コレクターフレームワーク＋ファクトリ：マッチング、│
│   検索、集約、統計、DFT/FFT                    │
├─────────────────────────────────────────────────┤
│                🔤 charsequence.h                │
│   名前空間: charsequence                        │
│   Unicode文字シーケンス、マルチエンコーディング  │
│   変換、Builder、Buffer                         │
├─────────────────────────────────────────────────┤
│                🧵 pool.h                        │
│   名前空間: pool                                │
│   グローバルスレッドプール：タスク送信、緊急停止、 │
│   例外伝播                                      │
├─────────────────────────────────────────────────┤
│                📄 function.h                    │
│   名前空間: function                            │
│   型定義：Generator、Supplier、Consumerなどの  │
│   エイリアス                                    │
├─────────────────────────────────────────────────┤
│               🔐 hash.h / less.h                │
│   名前空間: std (拡張)                          │
│   標準ライブラリコンテナハッシュ＆比較の特殊化、 │
│   任意のネストをサポート                        │
└─────────────────────────────────────────────────┘
```

### 🧩 依存関係グラフ
依存関係チェーンは、入念に設計された回路図のように明確で論理的です：電流は基本的な型定義から上向きに流れ、各層は下位層にのみ依存します。最終的に、すべてのパスは`semantic.h`と`semantics.h`に収束し、完全なストリーム処理能力を形成します。

```
function.h          ← 依存関係なし、型の基盤
pool.h              ← function.hに依存
charsequence.h      ← 独立モジュール、Unicode処理
collector.h         ← function.h、pool.hに依存
hash.h / less.h     ← 独立モジュール、標準ライブラリ拡張
semantic.h          ← 上記すべてに依存
semantics.h         ← semantic.hに依存
```

---

## 🌍 名前空間概要

Semantic-Cppは、5つの名前空間を入念に設計しています。各々は独立した「部門」のように、明確な責任を持ちながら密接に連携します：

| 名前空間    | ヘッダーファイル | 責任                                     | 主要な型/関数                                                          |
| :---------- | :--------------- | :--------------------------------------- | :--------------------------------------------------------------------- |
| function    | function.h       | 型システムの基盤                         | `Timestamp`、`Module`、`Generator<T>`、`Supplier<R>`、`Consumer<T>`、`Predicate<T>`など |
| pool        | pool.h           | 並行実行エンジン                         | `pool::pool`（グローバルスレッドプール）、`submit()`、`emergencyShutdown()` |
| charsequence| charsequence.h   | Unicode文字列処理                       | `charset`、`Meta`、`Point`、`Charsequence`、`Builder`、`Buffer`など    |
| collector   | collector.h      | 終端収集実行                             | `Collector<E,A,R>`、`Identity<A>`、`Accumulator<A,E>`など              |
| collectable | semantic.h       | 具体化されたデータコンテナ               | `Collectable<E>`、`OrderedCollectable<E>`、`UnorderedCollectable<E>`など |
| semantic    | semantic.h<br>semantics.h | ストリーム構築と中間操作             | `Semantic<E>`、`useRange()`、`useFrom()`など                           |

### 🔁 名前空間連携フロー
名前空間間のデータフローは、工場の組立ラインのようです — 原材料は`semantic`から入り、層ごとに処理を受け、最終的に`collector`からパッケージ化され出荷されます。各ステップには明確な責任の境界があります：

```cpp
semantic::useRange(0, 100)          // ← semantic名前空間：ストリーム作成
    .map(int x { return x * 2; })   // ← semantic名前空間：中間変換
    .filter(int x { return x > 50; }) // ← semantic名前空間：中間フィルター
    .toUnordered()                  // ← collectable名前空間に変換
    .toVector();                    // ← collector名前空間のコレクターを呼び出し
```

---

## 📦 レイヤー1：function.h — 型の基盤

`function.h`は、フレームワーク全体の型システムを定義し、すべてのモジュールの共通基盤です。🔑

```cpp
namespace function {
    using Timestamp = long long;           // インデックスタイプ、ストリーム内のデータの「タイムスタンプ」
    using Module = unsigned long long;     // モジュール/カウントタイプ
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept — 要素を受け取る
        std::function<bool(T, Timestamp)>       // interrupt — 停止すべきか？
    )>;
}
```

`Generator`は、ストリームシステム全体の中心的な抽象化です。🌀 データを返しません。代わりに、2つのコールバック — `accept`（「準備ができました、この要素を受け取ってください」）と`interrupt`（「停止すべきですか？」）を受け入れます。この制御の反転設計により、データプロデューサーはコンシューマーについて何も知りません。適切なタイミングでデータを「プッシュ」するだけです。これが遅延評価の本質です：データは`accept`が呼び出されたときにのみ実際に「流れ」ます。それまでは、すべては単なる記述です。

| 型エイリアス    | 完全な定義                                 | 目的                                     |
| :-------------- | :----------------------------------------- | :--------------------------------------- |
| Timestamp       | long long                                  | ストリーム内の要素の論理的位置           |
| Module          | unsigned long long                         | カウント、容量、並行性レベル             |
| Runnable        | std::function<void()>                      | パラメータなし、voidを返すタスク         |
| Supplier<R>     | std::function<R()>                         | 供給者、無から作成                       |
| Function<T,R>   | std::function<R(T)>                        | 単一引数関数                             |
| BiFunction<T,U,R>| std::function<R(T,U)>                     | 2引数関数                                |
| TriFunction<T,U,V,R>| std::function<R(T,U,V)>                 | 3引数関数                                |
| Unary<T>        | std::function<T(T)>                        | 単項演算                                 |
| Binary<T>       | std::function<T(T,T)>                      | 二項演算                                 |
| Consumer<T>     | std::function<void(T)>                     | 消費者                                   |
| BiConsumer<T,U> | std::function<void(T,U)>                   | 2引数消費者                              |
| TriConsumer<T,U,V>| std::function<void(T,U,V)>                | 3引数消費者                              |
| Predicate<T>    | std::function<bool(T)>                     | 述語判定                                 |
| BiPredicate<T,U>| std::function<bool(T,U)>                   | 2引数述語                                |
| TriPredicate<T,U,V>| std::function<bool(T,U,V)>               | 3引数述語                                |
| Comparator<T>   | std::function<int(const T&,const T&)>      | 比較子、負/ゼロ/正を返す                 |
| Generator<T>    | BiConsumer<BiConsumer<T,Timestamp>,<br>BiPredicate<T,Timestamp>> | ストリーム生成の中心的な抽象化 |

---

## 🧵 レイヤー2：pool.h — 並行性の基盤

`pool.h`は、グローバルスレッドプール`pool::pool`を提供し、フレームワーク全体の並行性エンジンです。🚀 **宣言的並列処理**の設計を採用しています — `.parallel(4)`と書いても、すぐに4つのスレッドを起動して処理を開始するわけではありません。このコード行は単なる「宣言」です：フレームワークに「並列処理に4つのスレッドを使用するつもりです」と伝えます。実際の並列実行は、終端操作が呼び出されたときに発生します — つまり、`toVector()`、`findFirst()`、`count()`などの収集メソッドを呼び出したときです。

| 特徴            | 説明                                                                 |
| :-------------- | :------------------------------------------------------------------- |
| 宣言的並列処理  | `.parallel(4)`は「4スレッド使用したい」と宣言するだけで、即座には開始しない |
| 緊急停止        | 組み込みの`emergencyShutdown()`と`std::set_terminate`ハンドラー      |
| 例外伝播        | `submit()`は`std::future`を返し、例外をメインスレッドに安全に伝播     |

---

## 🔤 レイヤー3：charsequence.h — Unicode文字シーケンス

`charsequence.h`は、完全なUnicode処理モジュールで、文字シーケンスの作成、変換、操作の機能を提供します。🌍 UTF-8、UTF-16（LE/BE）、UTF-32（LE/BE）、ASCII、Latin1などの様々なエンコーディングをサポートします。サロゲートペアを正しく検出・処理し、無効なコードポイントには標準のU+FFFD置換文字を返します。

| 型/関数       | 説明                                                                   |
| :------------ | :--------------------------------------------------------------------- |
| charset       | 列挙型：ascii、utf8、utf16、utf16be、utf16le、utf32、utf32be、utf32le、latin1 |
| Meta          | メタデータラッパー、符号なし整数値を格納                                 |
| Point         | Unicodeコードポイント、サロゲートペア検出と妥当性チェックをサポート       |
| Charsequence  | 不変文字シーケンス：split、replace、indexOf、lastIndexOf、sub、trim、toUpperCase、toLowerCase、reverse、startsWith、endsWith、contains、compare、getBytes、getPoints、getMetas、getCharacters、repeat、concat、count、join |
| Builder       | 可変バイトビルダー：prepend、insert、append（基本型、Point、Charsequence、string_viewをサポート） |
| Buffer        | スレッドセーフリングバッファ：write、read、peek、prepend、append、clear、shrinkToFit、data、size、capacity、atomic |
| PointIterator | Unicodeコードポイントを走査する双方向イテレータ                         |
| encode()      | 単一コードポイントを指定エンコーディングのバイトシーケンスにエンコード   |
| decode()      | バイトシーケンスから次のコードポイントをデコード、ポインタを自動的に進める |
| convert()     | エンコーディング変換（string、vector、deque出力をサポート）             |

---

## ⚙️ レイヤー4：collector.h — コレクターフレームワークとファクトリ

`collector.h`は、Semantic-Cppのコレクターコアモジュールで、コレクターフレームワークとファクトリ関数を組み合わせています。

### 🧩 5段階モデル
```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt（オプションの短絡）
```

| 型エイリアス    | 完全な定義                            | 役割                             |
| :-------------- | :------------------------------------ | :------------------------------- |
| Identity<A>     | function::Supplier<A>                 | 初期値を提供                     |
| Accumulator<A,E>| function::TriFunction<A, E, Timestamp, A> | 要素を累積                     |
| Combiner<A>     | function::BiFunction<A, A, A>         | 並列結果を結合                   |
| Finisher<A,R>   | function::Function<A, R>              | 最終変換                         |
| Interrupt<E,A>  | function::TriPredicate<E, Timestamp, A> | 短絡判定                       |

### コレクターファクトリ関数

#### ✅ マッチング操作
| メソッド                   | 説明                 | 戻り値の型 |
| :------------------------- | :------------------- | :--------- |
| `useAllMatch(predicate)`   | すべての要素が条件に一致 | `bool`     |
| `useAnyMatch(predicate)`   | いずれかの要素が条件に一致 | `bool`     |
| `useNoneMatch(predicate)`  | いずれの要素も条件に一致しない | `bool`     |

#### 🔍 検索操作
| メソッド           | 説明                       | 戻り値の型     |
| :----------------- | :------------------------- | :------------- |
| `useFindFirst()`   | 最初の要素を検索           | `std::optional<E>` |
| `useFindLast()`    | 最後の要素を検索           | `std::optional<E>` |
| `useFindAny()`     | いずれかの要素（ランダム）を検索 | `std::optional<E>` |
| `useFindAt(index)` | 指定インデックスに位置特定（負数サポート） | `std::optional<E>` |
| `useFindMaximum()` | 最大要素を検索             | `std::optional<E>` |
| `useFindMinimum()` | 最小要素を検索             | `std::optional<E>` |

#### 🔢 集約操作
| メソッド          | 説明  | 戻り値の型 |
| :---------------- | :---- | :--------- |
| `useCount()`      | 要素の総数 | `Module`   |
| `useSummate<E,D>()`| 合計  | `D`        |
| `useAverage<E,D>()`| 平均  | `D`        |
| `useRange<E,D>()` | 数値範囲（最大 - 最小） | `D` |

#### 📉 統計操作
| メソッド                    | 説明                 | 戻り値の型       |
| :------------------------- | :------------------- | :--------------- |
| `useVariance<E,D>()`       | 母分散               | `D`              |
| `useStandardDeviation<E,D>()`| 母標準偏差         | `D`              |
| `useSkewness<E,D>()`       | 歪度                 | `D`              |
| `useKurtosis<E,D>()`       | 尖度                 | `D`              |
| `useMedian<E,D>()`         | 中央値               | `std::optional<D>` |
| `useMode<E>()`             | 最頻値（頻度分析）   | `std::optional<E>` |
| `usePercentile<E,D>(p)`    | pパーセンタイル      | `std::optional<D>` |
| `useFrequency<E>()`        | 周波数領域特性       | `std::map<E, complex>` |
| `useDistribution<E>()`     | 空間分布特性         | `std::map<E, complex>` |

#### 🔀 縮約操作
| メソッド                      | 説明           | 戻り値の型     |
| :---------------------------- | :------------- | :------------- |
| `useReduce(reducer)`          | 単位元なし縮約 | `std::optional<E>` |
| `useReduce(identity, reducer)`| 単位元あり縮約 | `E`            |
| `useReduce(id, red, comb, fin)`| 完全にカスタムな縮約 | `R`        |

#### 🧺 コンテナへの収集
| メソッド                                                            | 戻り値の型 |
| :---------------------------------------------------------------- | :--------- |
| `useToVector()`                                                   | `std::vector<E>` |
| `useToList()`                                                     | `std::list<E>` |
| `useToDeque()`                                                    | `std::deque<E>` |
| `useToForwardList()`                                              | `std::forward_list<E>` |
| `useToArray<N>()`                                                 | `std::array<E, N>` |
| `useToSet()`                                                      | `std::set<E>` |
| `useToMultiset()`                                                 | `std::multiset<E>` |
| `useToUnorderedSet()`                                             | `std::unordered_set<E>` |
| `useToUnorderedMultiset()`                                        | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)`                                          | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)`                          | `std::map<K, V>` |
| `useToMultimap(keyExtractor)`                                     | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)`                     | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)`                 | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)`                            | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)`            | `std::unordered_multimap<K, V>` |
| `useToStack()`                                                    | `std::stack<E>` |
| `useToQueue()`                                                    | `std::queue<E>` |
| `useToPriorityQueue()`                                            | `std::priority_queue<E>` |

#### 🧩 グループ化とパーティショニング操作
| メソッド                                      | 戻り値の型 |
| :------------------------------------------ | :--------- |
| `useGroup(keyExtractor)`                    | `std::unordered_map<K, vector<E>>` |
| `useGroupBy(keyExtractor, valueExtractor)` | `std::unordered_map<K, vector<V>>` |
| `usePartition(size)`                        | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)`              | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor, valueExtractor)` | `std::vector<vector<V>>` |

#### 📄 文字列出力
| メソッド                               | 戻り値の型 |
| :----------------------------------- | :--------- |
| `useJoin()` / `useOut()` / `useError()` とそのオーバーロード | `charsequence::Charsequence` |

#### 📊 数学ツール
| メソッド        | 戻り値の型 |
| :------------- | :--------- |
| `useDFT()`     | `vector<complex<double>>` |
| `useIDFT()`    | `vector<complex<double>>` |
| `useFFT()`     | `vector<complex<double>>` |
| `useIFFT()`    | `vector<complex<double>>` |
| `useGradient()`| `vector<double>` |

---

## 📦 レイヤー5：semantic.h — ストリーム中間操作と収集システム

### 🧩 コアデザイン：3段階パイプライン
```
Semantic<E>（構築と変換）
    ↓ toUnordered() / toOrdered() / sort() / toWindow() / toStatistics()
Collectable<E>（具体化と収集）
    ↓ toVector() / findFirst() / count() / summate() / ...
最終結果
```

**重要ルール**：終端メソッドを呼び出す前に、`Semantic<E>`を`toUnordered()`、`toOrdered()`、`toWindow()`、`toStatistics()`、または`sort()`を介して`Collectable<E>`に変換する必要があります。

### 🧭 5つの具体化パス
| 変換メソッド   | ターゲット型         | 基盤データ構造       | パフォーマンス特性       |
| :------------- | :------------------- | :------------------- | :----------------------- |
| `toUnordered()`| `UnorderedCollectable` | `unordered_map`     | 平均O(1)ルックアップ     |
| `toOrdered()`  | `OrderedCollectable`  | `map`               | O(log n)ルックアップ     |
| `sort()`       | `OrderedCollectable`  | `map`（値でソート） | O(log n)ルックアップ     |
| `toWindow()`   | `WindowCollectable`   | 順序付きコレクションを継承 | slide/tumbleをサポート |
| `toStatistics<D>()`| `Statistics<E,D>`  | 順序付きコレクションを継承 | 30+の統計メソッド    |

### 📋 Collectable<E> — すべての終端メソッド（アルファベット順）
| メソッド                                            | 戻り値の型             | 説明                                     |
| :------------------------------------------------ | :------------------- | :--------------------------------------- |
| `allMatch(predicate)`                             | `bool`               | すべての要素が条件に一致                 |
| `anyMatch(predicate)`                             | `bool`               | いずれかの要素が条件に一致               |
| `average<D>()`                                    | `D`                  | 平均                                     |
| `average<D>(mapper)`                              | `D`                  | マッピング後の平均                       |
| `collect(identity, acc, comb, fin)`              | `R`                  | カスタム4段階収集                       |
| `collect(identity, interrupt, acc, comb, fin)`    | `R`                  | カスタム割り込み可能収集                 |
| `count()`                                         | `Module`             | 要素の総数                               |
| `empty()`                                         | `bool`               | ストリームは空か？                       |
| `error()`                                         | `void`               | stderrへの出力（区切り文字/接頭辞/接尾辞/コンバーターをサポート） |
| `findAny()`                                       | `std::optional<E>`   | いずれかの要素（ランダム）を検索         |
| `findAt(index)`                                   | `std::optional<E>`   | 指定インデックスの要素を検索（負数サポート） |
| `findFirst()`                                     | `std::optional<E>`   | 最初の要素を検索                         |
| `findLast()`                                      | `std::optional<E>`   | 最後の要素を検索                         |
| `findMaximum()`                                   | `std::optional<E>`   | 最大要素を検索                           |
| `findMaximum(comparator)`                         | `std::optional<E>`   | カスタム比較子で最大を検索               |
| `findMinimum()`                                   | `std::optional<E>`   | 最小要素を検索                           |
| `findMinimum(comparator)`                         | `std::optional<E>`   | カスタム比較子で最小を検索               |
| `forEach(consumer)`                               | `void`               | 各要素に対して副作用を実行               |
| `group(keyExtractor)`                             | `unordered_map<K, vector<E>>` | キーでグループ化                     |
| `groupBy(keyExtractor, valueExtractor)`          | `unordered_map<K, vector<V>>` | キーでグループ化し値を抽出         |
| `join()`                                          | `Charsequence`        | デフォルト形式で結合                     |
| `join(delimiter)`                                 | `Charsequence`        | カスタム区切り文字で結合                 |
| `join(prefix, delimiter, suffix)`                | `Charsequence`        | 完全にカスタム形式で結合                 |
| `noneMatch(predicate)`                            | `bool`               | いずれの要素も条件に一致しない           |
| `out()`                                           | `Charsequence`        | stdoutへの出力（区切り文字/接頭辞/接尾辞/コンバーターをサポート） |
| `partition(size)`                                 | `vector<vector<E>>`   | 固定サイズでパーティション分割           |
| `partitionBy(keyExtractor)`                       | `vector<vector<E>>`   | インデックスキーでパーティション分割     |
| `partitionBy(keyExtractor, valueExtractor)`       | `vector<vector<V>>`   | インデックスキーでパーティション分割し値を抽出 |
| `range<D>()`                                      | `D`                  | 数値範囲（最大 - 最小）                   |
| `range<D>(mapper)`                                | `D`                  | マッピング後の数値範囲                   |
| `reduce(accumulator)`                             | `std::optional<E>`   | 単位元なし縮約                           |
| `reduce(identity, accumulator)`                   | `E`                  | 単位元あり縮約                           |
| `reduce(identity, acc, comb)`                     | `R`                  | 完全にカスタムな縮約                     |
| `summate<D>()`                                    | `D`                  | 合計                                     |
| `summate<D>(mapper)`                              | `D`                  | マッピング後の合計                       |
| `toArray<N>()`                                    | `std::array<E, N>`   | 固定サイズ配列に収集                     |
| `toDeque()`                                       | `std::deque<E>`      | dequeに収集                              |
| `toForwardList()`                                 | `std::forward_list<E>`| forward_listに収集                      |
| `toList()`                                        | `std::list<E>`       | listに収集                               |
| `toMap(keyExtractor)`                             | `std::map<K, E>`     | キーでmapに収集                          |
| `toMap(keyExtractor, valueExtractor)`             | `std::map<K, V>`     | カスタムキーと値でmapに収集             |
| `toMultimap(keyExtractor)`                        | `std::multimap<K, E>`| キーでmultimapに収集                     |
| `toMultimap(keyExtractor, valueExtractor)`        | `std::multimap<K, V>`| カスタムキーと値でmultimapに収集         |
| `toMultiset()`                                    | `std::multiset<E>`   | multisetに収集                           |
| `toPriorityQueue()`                               | `std::priority_queue<E>`| priority_queueに収集                   |
| `toQueue()`                                       | `std::queue<E>`      | queueに収集                              |
| `toSet()`                                         | `std::set<E>`        | setに収集（一意でソート）                |
| `toStack()`                                       | `std::stack<E>`      | stackに収集                              |
| `toUnorderedMap(keyExtractor, valueExtractor)`    | `std::unordered_map<K, V>`| unordered_mapに収集                 |
| `toUnorderedMultimap(keyExtractor)`               | `std::unordered_multimap<K, E>`| キーでunordered_multimapに収集     |
| `toUnorderedMultimap(keyExtractor, valueExtractor)` | `std::unordered_multimap<K, V>`| カスタムキーと値でunordered_multimapに収集 |
| `toUnorderedMultiset()`                           | `std::unordered_multiset<E>`| unordered_multisetに収集            |
| `toUnorderedSet()`                                | `std::unordered_set<E>`| unordered_setに収集                   |
| `toVector()`                                      | `std::vector<E>`     | vectorに収集                             |

### 📈 Statistics<E,D> — 統計メソッド
| メソッド             | 戻り値の型         | 説明                     |
| :----------------- | :--------------- | :----------------------- |
| `summate()`        | `D`              | 合計                     |
| `average()`        | `D`              | 平均                     |
| `minimum()`        | `std::optional<D>`| 最小値                   |
| `maximum()`        | `std::optional<D>`| 最大値                   |
| `range()`          | `D`              | 範囲（最大 - 最小）       |
| `variance()`       | `D`              | 母分散                   |
| `standardDeviation()`| `D`            | 母標準偏差               |
| `median()`         | `std::optional<D>`| 中央値                 |
| `mode()`           | `std::optional<E>`| 最頻値                 |
| `percentile(p)`    | `std::optional<D>`| pパーセンタイル        |
| `firstQuartile()`  | `std::optional<D>`| 第1四分位数（Q1）       |
| `thirdQuartile()`  | `std::optional<D>`| 第3四分位数（Q3）       |
| `interquartileRange()`| `std::optional<D>`| 四分位範囲（IQR）   |
| `skewness()`       | `D`              | 歪度                     |
| `kurtosis()`       | `D`              | 尖度                     |
| `frequency()`      | `map<E, complex>`| 周波数領域特性           |
| `distribute()`     | `map<E, complex>`| 空間分布特性             |
| `dft()`            | `vector<complex<double>>`| 離散フーリエ変換 |
| `idft()`           | `vector<complex<double>>`| 逆離散フーリエ変換 |
| `fft()`            | `vector<complex<double>>`| 高速フーリエ変換 |
| `ifft()`           | `vector<complex<double>>`| 逆高速フーリエ変換 |
| `gradient(...)`    | `vector<double>` | 勾配降下法               |

上記のすべてのメソッドは、オプションの`mapper`パラメータバージョンもサポートします。

### 🔧 Semantic<E> 中間操作メソッド
| カテゴリ      | メソッド        | 説明                                     |
| :------------ | :------------ | :--------------------------------------- |
| 要素変換      | map           | 1対1マッピング変換                       |
|               | flatMap       | 1対多マッピングと平坦化                   |
|               | flat          | ネストされたストリームを平坦化（Semanticとコンテナをサポート） |
| 要素フィルター| filter        | 条件付きフィルタリング                   |
|               | takeWhile     | 条件が真の間要素を取得                   |
|               | dropWhile     | 条件が真の間要素を破棄                   |
|               | distinct      | 重複を削除（カスタム比較子をサポート）   |
| サイズ制御    | limit         | 要素数を制限                             |
|               | skip          | 最初のn要素をスキップ                     |
|               | sub           | 部分範囲を抽出 [start, end)              |
| インデックス操作| redirect      | インデックスを再割り当て                 |
|               | reverse       | インデックスを逆転                       |
|               | translate     | インデックスをシフト                     |
| 観察          | peek          | 各要素を観察（ストリームを変更しない）   |
| 並列宣言      | parallel(n)   | 並行性レベルを宣言                       |
| 連結          | concatenate   | Semantic/要素/ジェネレータ/コンテナを連結 |
| 終端変換      | toUnordered / toOrdered / toWindow / toStatistics / sort | Collectableに変換 |

---

## 🔧 レイヤー6：semantics.h — ストリーム構築ファクトリ

### 🔢 数値範囲生成
| メソッド                        | 説明                     |
| :---------------------------- | :----------------------- |
| `useRange(start, end)`       | 範囲 [start, end) を生成 |
| `useRange(start, end, step)` | ステップ付き範囲（負数サポート） |
| `useRangeClosed(start, end)` | 閉範囲 [start, end] を生成 |
| `useRangeClosed(start, end, step)` | ステップ付き閉範囲 |

### ♾️ 無限ストリーム生成
| メソッド                      | 説明                         |
| :-------------------------- | :--------------------------- |
| `useInfinite(seed, generator)`| 初期値からの無限反復         |
| `useGenerate(supplier)`     | 供給者への無限呼び出し       |
| `useGenerate(supplier, limit)`| 供給者への制限付き呼び出し   |
| `useIterate(seed, generator)` | 初期値からの無限反復         |
| `useIterate(seed, generator, limit)` | 制限付き反復       |
| `useRandom()`               | ランダム整数の無限ストリーム |
| `useRandom(min, max)`       | 指定範囲の乱数ストリーム     |
| `useRandom(min, max, count)`| 指定範囲と数の乱数ストリーム |

### 📦 コンテナと要素の構築
| メソッド                  | 説明                     |
| :---------------------- | :----------------------- |
| `useEmpty()`            | 空のストリームを作成     |
| `useOf(element)`        | 単一要素からストリームを作成 |
| `useOf(e1, e2)`         | 2要素からストリームを作成 |
| `useOf(e1, e2, e3)`     | 3要素からストリームを作成 |
| `useOf({...})`          | 初期化リストからストリームを作成 |
| `useFrom(container)`    | 標準コンテナからストリームを作成 |
| `useFrom({...})`        | 初期化リストからストリームを作成 |
| `useRepeat(element, count)` | 要素をn回繰り返す     |

### 📄 テキストとUnicode処理
| メソッド                      | 説明                         |
| :-------------------------- | :--------------------------- |
| `useBlob(text)`             | 文字列をバイトごとにcharストリームに分割 |
| `useBlob(text, start, end)` | 指定範囲をバイトごとに分割     |
| `useBlob(istream)`          | 入力ストリームから行ごとに読み込み |
| `useBlob(istream, delimiter)` | 区切り文字で入力ストリームから読み込み |
| `useText(text)`             | 完全なテキストストリーム（Charsequence） |
| `useText(text, delimiter)`  | 区切り文字でテキストを分割     |
| `useText(istream)`          | 入力ストリームから完全な内容を読み込み |
| `useSequence(charsequence)` | 文字シーケンスからコードポイントストリームを作成 |
| `useSequence(text, encoding)` | 指定エンコーディングのテキストからコードポイントストリームを作成 |
| `useCharsequence(charsequence)` | 文字シーケンス全体としてのストリーム |
| `useCharsequence(charsequence, delimiter)` | 区切り文字で文字シーケンスを分割 |

---

## 🔐 レイヤー7：hash.h / less.h — コンテナ世界の普遍言語

すべての標準ライブラリコンテナ（ネストされたコンテナを含む）、`pair`、`tuple`、`optional`、`variant`、`chrono`時間型、`complex`数などに対する完全なハッシュと比較サポートを提供します。任意の深さと組み合わせでネストされたコンテナを、`unordered_set`のキーや`set`の要素として使用できるようになりました。🌉

---

## 🚀 パフォーマンス最適化のヒント

1.  **適切なコンテナを選択**：順序が重要でない場合は`toUnordered()`、ソートが必要な場合は`toOrdered()`または`sort()`を使用
2.  **並列処理を活用**：大規模データセットには`parallel()`を使用
3.  **操作順序を最適化**：早期にフィルタリング、賢くソート
4.  **遅延評価を活用**：`takeWhile`と`limit`は早期終了可能

---

Semantic-Cpp — モダンなC++で効率的で明確なデータ処理パイプラインを構築。🚀🎯✨
