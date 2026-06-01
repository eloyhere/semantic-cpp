# 🚀 Semantic-Cpp：未来を見据えた C++ インテリジェント・ストリーム処理フレームワーク

Semantic-Cpp は、ゼロ外部依存・マルチヘッダーのモジュール設計により完全に再設計された、モダンな C++ ストリーム処理ライブラリです。各ヘッダーは単一責任を持ち、独立してテスト可能で、ストリーム処理エコシステムを構成します。本ライブラリは、複数のプログラミングパラダイムの精髓を創造的に融合しています：

- 🎯 **Java Stream API** の優雅さと流暢さ：メソッドチェーンと宣言型プログラミングによる詩的なコード
- ⚡ **JavaScript Generator** の遅延評価と柔軟性：オンデマンド生成によるメモリ効率の良さ
- 🗄️ **データベース・インデックス** の効率と秩序：スマートなソートとインデックス駆動による時系列データ処理

従来の手書きループや非同期コールバックとは異なり、Semantic-Cpp は **型安全で表現力豊か、かつ高性能** なソリューションを提供します。その中核となる設計哲学は、**精密なデータフロー制御**です。データは必要なときにのみ流れ、順序と位置は「インデックス」によって精緻に制御され、リソースを最適に活用します。

---

## 📐 プロジェクトアーキテクチャ：7層モジュール設計

Semantic-Cpp は、**7つのコアヘッダー**から構成され、各層が独立した役割を持ちます：

```
┌─────────────────────────────────────────────────┐
│                 semantics.h                     │
│    (ストリーム生成：数値範囲、コンテナ、テキスト、Unicode) │
├─────────────────────────────────────────────────┤
│                  semantic.h                     │
│   (ストリーム中間操作、Collectable体系、コンテナ特殊化)   │
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│  (コレクター生成：照合、検索、集約、統計、DFT/FFTなど)   │
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│     (コレクターフレームワーク：5段階モデル、並列処理対応) │
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│  (Unicode文字シーケンス、マルチエンコーディング変換、Builder、Buffer) │
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│   (グローバルスレッドプール：タスク投入、緊急停止、例外伝播) │
├─────────────────────────────────────────────────┤
│                 function.h                      │
│  (型定義：Generator、Supplier、Consumerなどのエイリアス) │
└─────────────────────────────────────────────────┘
```

### 依存関係

```
function.h          ← 依存なし、型システムの基盤
pool.h              ← function.h に依存
charsequence.h      ← 独立モジュール、Unicode処理
collector.h         ← function.h、pool.h に依存
collectors.h        ← collector.h、charsequence.h に依存
semantic.h          ← collector.h、collectors.h、charsequence.h に依存
semantics.h         ← semantic.h に依存
```

各ヘッダーは独立してコンパイルおよびテスト可能で、必要に応じて個別にインクルードできます。  
例えば、コレクター機能のみが必要な場合は `collector.h` と `collectors.h` だけをインクルードすれば十分です。

---

## 🏗️ 第1層：function.h — 型の基盤

`function.h` はフレームワーク全体の型システムを定義する共通基盤です：

```cpp
namespace function {
    using Timestamp = long long;           // インデックス型
    using Module = unsigned long long;     // モジュール／カウンター型
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept
        std::function<bool(T, Timestamp)>       // interrupt
    )>;
}
```

`Generator` はストリームシステム全体の中心的な抽象であり、「遅延プル（Lazy Pull）」モデルを体現しています。

---

## ⚡ 第2層：pool.h — 並行処理の基盤

`pool.h` はグローバルスレッドプール `pool::pool` を提供し、フレームワークの並行処理エンジンとなります：

| 特徴 | 説明 |
|----|----|
| 🎯 宣言的並列処理 | `parallel(n)` は意図を宣言するのみで、終端操作時に自動起動 |
| 🛡️ 緊急停止 | `emergencyShutdown()` と `std::set_terminate` ハンドラを内蔵 |
| 🔄 例外伝播 | `submit()` は `std::future` を返し、安全な例外伝播をサポート |

---

## 🔤 第3層：charsequence.h — Unicode 文字シーケンス

`charsequence.h` は Unicode 処理のための完全なモジュールです：

| 機能 | 説明 |
|----|----|
| 🌐 マルチエンコーディング対応 | UTF‑8、UTF‑16（LE/BE）、UTF‑32（LE/BE）、ASCII、Latin1 |
| 🔍 コードポイント反復子 | `PointIterator` による双方向走査 |
| 🏗️ ビルダーパターン | `Builder` クラスによる効率的なバイト結合 |
| 📦 バッファー | `Buffer` クラスによるスレッドセーフなリングバッファ |
| 🔑 ハッシュと比較 | 全コア型に `std::hash` および `std::less` の特殊化あり |

### コア型

| 型 | 説明 |
|----|----|
| `Meta` | 符号なし整数値を格納するメタデータラッパー |
| `Point` | Unicode コードポイント（サロゲートペア検出と妥当性検証付き） |
| `Charsequence` | 不変文字シーケンス（分割、置換、検索、大文字小文字変換など対応） |
| `Builder` | 可変バイトビルダー（`prepend`、`insert`、`append` 等対応） |
| `Buffer` | スレッドセーフなリングバッファ（読み書き、プリフェッチ、容量管理対応） |

---

## 🔧 第4層：collector.h — コレクターフレームワーク

`collector.h` は終端操作の中核となるコレクターパターンを実装します。

### 5段階モデル

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt（短絡評価、任意）
```

### 型エイリアス

| 型 | 定義 | 役割 |
|----|----|----|
| `Identity<A>` | `Supplier<A>` | 初期値を提供 |
| `Accumulator<A,E>` | `TriFunction<A,E,Timestamp,A>` | 要素を蓄積 |
| `Combiner<A>` | `BiFunction<A,A,A>` | 並列結果を統合 |
| `Finisher<A,R>` | `Function<A,R>` | 最終変換 |
| `Interrupt<E,A>` | `TriPredicate<E,Timestamp,A>` | 短絡判定 |

### 並列処理対応

`Collector::collect()` は以下を自動処理します：

- 📦 データ分割（インデックス剰余によるスレッド分散）
- 🔗 結果統合（`Combiner` によるマージ）
- ⚠️ 例外伝播（`std::exception_ptr` と `std::atomic<bool>` による安全な伝播）

---

## 🏭 第5層：collectors.h — コレクターファクトリー

`collectors.h` は豊富な事前定義コレクターファクトリ関数を提供します。

### 📊 照合操作

| メソッド | 説明 | 戻り値型 |
|----|----|----|
| `useAllMatch(predicate)` | すべての要素が条件を満たす | `bool` |
| `useAnyMatch(predicate)` | いずれかの要素が条件を満たす | `bool` |
| `useNoneMatch(predicate)` | どの要素も条件を満たさない | `bool` |

### 🔍 検索操作

| メソッド | 説明 | 戻り値型 |
|----|----|----|
| `useFindFirst()` | 最初の要素を検索 | `std::optional<E>` |
| `useFindLast()` | 最後の要素を検索 | `std::optional<E>` |
| `useFindAny()` | 任意の要素を検索 | `std::optional<E>` |
| `useFindAt(index)` | 指定インデックスの要素を検索（負数対応） | `std::optional<E>` |
| `useFindMaximum()` | 最大値を検索（比較関数指定可） | `std::optional<E>` |
| `useFindMinimum()` | 最小値を検索（比較関数指定可） | `std::optional<E>` |

### 📈 集約操作

| メソッド | 説明 | 戻り値型 |
|----|----|----|
| `useCount()` | 要素総数 | `Module` |
| `useSummate<E,D>()` | 合計 | `D` |
| `useSummate<E,D>(mapper)` | マップ後合計 | `D` |
| `useAverage<E,D>()` | 平均 | `D` |
| `useAverage<E,D>(mapper)` | マップ後平均 | `D` |
| `useRange<E,D>()` | 範囲（最大−最小） | `D` |
| `useRange<E,D>(mapper)` | マップ後範囲 | `D` |
| `useMinimum<E,D>()` | 最小値 | `std::optional<D>` |
| `useMaximum<E,D>()` | 最大値 | `std::optional<D>` |

### 📊 統計操作

| メソッド | 説明 | 戻り値型 |
|----|----|----|
| `useVariance<E,D>()` | 母分散 | `D` |
| `useStandardDeviation<E,D>()` | 母標準偏差 | `D` |
| `useSkewness<E,D>()` | 歪度 | `D` |
| `useKurtosis<E,D>()` | 尖度 | `D` |
| `useMedian<E,D>()` | 中央値 | `std::optional<D>` |
| `useMode<E>()` | 最頻値（周波数領域ベース） | `std::optional<E>` |
| `usePercentile<E,D>(p)` | pパーセンタイル | `std::optional<D>` |
| `useFrequency<E>()` | 周波数特性（インデックス位相符号化） | `std::map<E, complex>` |
| `useDistribution<E>()` | 空間分布特性（位置符号化） | `std::map<E, complex>` |

### 🔗 リダクション操作

| メソッド | 説明 | 戻り値型 |
|----|----|----|
| `useReduce(reducer)` | 初期値なしリダクション | `std::optional<E>` |
| `useReduce(identity, reducer)` | 初期値ありリダクション | `E` |
| `useReduce(id, red, comb, fin)` | 完全カスタムリダクション | `R` |

### 📦 コンテナ収集操作

| メソッド | 説明 | 戻り値型 |
|----|----|----|
| `useToVector()` | `std::vector` へ収集 | `std::vector<E>` |
| `useToList()` | `std::list` へ収集 | `std::list<E>` |
| `useToDeque()` | `std::deque` へ収集 | `std::deque<E>` |
| `useToForwardList()` | `std::forward_list` へ収集 | `std::forward_list<E>` |
| `useToArray<N>()` | 固定長 `std::array` へ収集 | `std::array<E, N>` |
| `useToSet()` | `std::set` へ収集（重複除去・整列） | `std::set<E>` |
| `useToMultiset()` | `std::multiset` へ収集 | `std::multiset<E>` |
| `useToUnorderedSet()` | `std::unordered_set` へ収集 | `std::unordered_set<E>` |
| `useToUnorderedMultiset()` | `std::unordered_multiset` へ収集 | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)` | `std::map` へ収集 | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)` | カスタム値で `std::map` へ収集 | `std::map<K, V>` |
| `useToMultimap(keyExtractor)` | `std::multimap` へ収集 | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)` | カスタム値で `std::multimap` へ収集 | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)` | `std::unordered_map` へ収集 | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)` | `std::unordered_multimap` へ収集 | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)` | カスタム値で `std::unordered_multimap` へ収集 | `std::unordered_multimap<K, V>` |
| `useToStack()` | `std::stack` へ収集 | `std::stack<E>` |
| `useToQueue()` | `std::queue` へ収集 | `std::queue<E>` |
| `useToPriorityQueue()` | `std::priority_queue` へ収集 | `std::priority_queue<E>` |

### 🔀 グルーピング＆パーティション操作

| メソッド | 説明 | 戻り値型 |
|----|----|----|
| `useGroup(keyExtractor)` | キーによるグルーピング | `std::unordered_map<K, vector<E>>` |
| `usePartition(size)` | 固定サイズでのパーティション分割 | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)` | カスタムキーでのパーティション分割 | `std::vector<vector<E>>` |

### 🎨 文字列出力操作

| メソッド | 説明 | 戻り値型 |
|----|----|----|
| `useJoin()` | 文字列結合（デフォルト：カンマ区切り・角括弧） | `Charsequence` |
| `useJoin(delimiter)` | カスタム区切り文字列結合 | `Charsequence` |
| `useJoin(prefix, delimiter, suffix)` | 完全カスタムフォーマット結合 | `Charsequence` |
| `useOut()` | 標準出力へフォーマット出力 | `Charsequence` |
| `useOut(delimiter)` | カスタム区切りで標準出力へ | `Charsequence` |
| `useOut(prefix, delimiter, suffix)` | 完全カスタムフォーマットで標準出力へ | `Charsequence` |
| `useError()` | 標準エラー出力へフォーマット出力 | `Charsequence` |
| `useError(delimiter)` | カスタム区切りで標準エラー出力へ | `Charsequence` |
| `useError(prefix, delimiter, suffix)` | 完全カスタムフォーマットで標準エラー出力へ | `Charsequence` |

### 🧮 数学ユーティリティ

| メソッド | 説明 | 戻り値型 |
|----|----|----|
| `useDFT()` | 離散フーリエ変換 | `vector<complex<double>>` |
| `useIDFT()` | 逆離散フーリエ変換 | `vector<complex<double>>` |
| `useFFT()` | 高速フーリエ変換（Cooley–Tukey） | `vector<complex<double>>` |
| `useIFFT()` | 逆高速フーリエ変換 | `vector<complex<double>>` |
| `useGradient(gradFunc, lr, iter, th)` | 勾配降下法（解析的勾配） | `vector<double>` |
| `useGradient(costFunc, lr, iter, th, h)` | 勾配降下法（数値的勾配） | `vector<double>` |

---

## 🌊 第6層：semantic.h — ストリーム中間操作と収集体系

`semantic.h` はフレームワークの中核であり、`collectable` と `semantic` の2つの名前空間を含みます。

### collectable 名前空間

収集可能オブジェクトの継承体系を提供します：

| クラス | 説明 | 内部ストレージ |
|----|----|----|
| `Collectable<E>` | 抽象基底クラス（純仮想 `source()`） | — |
| `OrderedCollectable<E>` | 順序付き収集（カスタムソート対応） | `std::map<Timestamp, E>` |
| `UnorderedCollectable<E>` | 順序なし収集（O(1) 検索） | `std::unordered_map<Timestamp, E>` |
| `Statistics<E, D>` | 統計収集（`OrderedCollectable` 継承） | 20以上の統計メソッド |
| `WindowCollectable<E>` | ウィンドウ収集（`OrderedCollectable` 継承） | `slide` / `tumble` 対応 |

#### Collectable 基底クラスメソッド

20種類以上の `toXxx()` 終端収集メソッドに加え、  
`count()`、`findFirst()`、`findAny()`、`anyMatch()`、`allMatch()`、`noneMatch()`、`reduce()`、`join()`、`out()`、`error()`、`group()`、`partition()`、`partitionBy()` などを提供。

#### Statistics クラスメソッド

| メソッド | 戻り値型 | 説明 |
|----|----|----|
| `summate()` / `summate(mapper)` | `D` | 合計 |
| `average()` / `average(mapper)` | `D` | 平均 |
| `minimum()` / `minimum(mapper)` | `std::optional<D>` | 最小値 |
| `maximum()` / `maximum(mapper)` | `std::optional<D>` | 最大値 |
| `range()` / `range(mapper)` | `D` | 範囲（最大−最小） |
| `variance()` / `variance(mapper)` | `D` | 母分散 |
| `standardDeviation()` / `standardDeviation(mapper)` | `D` | 母標準偏差 |
| `frequency()` / `frequency(mapper)` | `std::map<*, complex>` | 周波数特性 |
| `distribute()` / `distribute(mapper)` | `std::map<*, complex>` | 空間分布特性 |
| `median()` / `median(mapper)` | `std::optional<D>` | 中央値 |
| `mode()` | `std::optional<E>` | 最頻値 |
| `percentile(p)` / `percentile(p, mapper)` | `std::optional<D>` | pパーセンタイル |
| `firstQuartile()` / `firstQuartile(mapper)` | `std::optional<D>` | 第1四分位数（Q1） |
| `thirdQuartile()` / `thirdQuartile(mapper)` | `std::optional<D>` | 第3四分位数（Q3） |
| `interquartileRange()` / `interquartileRange(mapper)` | `std::optional<D>` | 四分位範囲（Q3−Q1） |
| `skewness()` / `skewness(mapper)` | `D` | 歪度 |
| `kurtosis()` / `kurtosis(mapper)` | `D` | 尖度 |
| `dft()` | `vector<complex<double>>` | 離散フーリエ変換 |
| `idft()` | `vector<complex<double>>` | 逆離散フーリエ変換 |
| `fft()` | `vector<complex<double>>` | 高速フーリエ変換 |
| `ifft()` | `vector<complex<double>>` | 逆高速フーリエ変換 |
| `gradient(gradFunc, lr, iter, th)` | `vector<double>` | 勾配降下法（解析的） |
| `gradient(costFunc, lr, iter, th, h)` | `vector<double>` | 勾配降下法（数値的） |

### semantic 名前空間

`Semantic<E>` テンプレートクラスとその特殊化体系を提供します。

#### 主テンプレートメソッド一覧

| カテゴリ | メソッド | 説明 |
|----|----|----|
| 🎨 要素変換 | `map` | 一対一マッピング |
| | `flatMap` | 一対多マッピング＋平坦化 |
| | `flat` | ネストされたストリームの平坦化 |
| 🔍 要素フィルタ | `filter` | 条件フィルタ |
| | `takeWhile` | 条件が真の間取得 |
| | `dropWhile` | 条件が真の間破棄 |
| | `distinct` | 重複除去（カスタム比較関数対応） |
| 📏 数量制御 | `limit` | 要素数制限 |
| | `skip` | 先頭n個スキップ |
| | `sub` | 部分範囲抽出 |
| 📐 インデックス操作 | `redirect` | インデックス再マッピング |
| | `reverse` | インデックス反転 |
| | `translate` | インデックス移動（固定／動的） |
| 👀 観察操作 | `peek` | 要素観察（変更なし） |
| ⚡ 並列宣言 | `parallel` | 並列度宣言 |
| 🔗 連結操作 | `concatenate` | 他ストリーム／コンテナとの連結 |
| 📤 終端変換 | `toUnordered` | 順序なし収集へ変換 |
| | `toOrdered` | 順序付き収集へ変換 |
| | `toWindow` | ウィンドウ収集へ変換 |
| | `toStatistics` | 統計収集へ変換 |

#### コンテナ特殊化対応

| 特殊化 | 説明 |
|----|----|
| `Semantic<std::vector<E>>` | ベクターストリーム（ソート・重複除去対応） |
| `Semantic<std::list<E>>` | リストストリーム（ソート・重複除去対応） |

---

## 🏭 第7層：semantics.h — ストリーム生成ファクトリー

`semantics.h` はすべてのストリーム生成ファクトリ関数を提供します。

### 📐 数値範囲生成

| メソッド | 説明 |
|----|----|
| `useRange(start, end)` | `[start, end)` の数値ストリーム生成 |
| `useRange(start, end, step)` | ステップ付き数値ストリーム生成 |
| `useRangeClosed(start, end)` | `[start, end]` 閉区間数値ストリーム生成 |
| `useRangeClosed(start, end, step)` | ステップ付き閉区間数値ストリーム生成 |

### ♾️ 無限ストリーム生成

| メソッド | 説明 |
|----|----|
| `useInfinite(seed, generator)` | シード値から無限反復生成 |
| `useGenerate(supplier)` | 無限回サプライヤ呼び出し |
| `useGenerate(supplier, limit)` | 有限回サプライヤ呼び出し |
| `useIterate(seed, generator)` | シード値から無限反復 |
| `useIterate(seed, generator, limit)` | 有限回反復 |
| `useRandom()` | 無限乱数整数ストリーム |
| `useRandom(min, max)` | 指定範囲の無限乱数ストリーム |
| `useRandom(min, max, count)` | 指定範囲・個数の乱数ストリーム |

### 📦 コンテナ＆要素構築

| メソッド | 説明 |
|----|----|
| `useEmpty()` | 空ストリーム作成 |
| `useOf(element)` | 単一要素ストリーム |
| `useOf(e1, e2)` | 2要素ストリーム |
| `useOf(e1, e2, e3)` | 3要素ストリーム |
| `useOf({...})` | 初期化リストからストリーム |
| `useFrom(container)` | 標準コンテナからストリーム |
| `useFrom({...})` | 初期化リストからストリーム |
| `useRepeat(element, count)` | 要素をn回繰り返し |

### 📝 テキスト処理

| メソッド | 説明 |
|----|----|
| `useBlob(text)` | 文字列をバイト単位で `char` ストリーム化 |
| `useBlob(text, start, end)` | 部分文字列をバイト単位でストリーム化 |
| `useBlob(istream)` | 入力ストリームを行単位で読込 |
| `useBlob(istream, delimiter)` | 区切り文字で入力ストリーム読込 |
| `useText(text)` | 文字列全体をテキストストリームとして扱う |
| `useText(text, delimiter)` | 区切り文字でテキスト分割 |
| `useText(istream)` | 入力ストリーム全体を読込 |
| `useText(istream, delimiter)` | 区切り文字で入力ストリーム読込 |

### 🌐 Unicode 処理

| メソッド | 説明 |
|----|----|
| `useSequence(charsequence)` | 文字シーケンスからコードポイントストリーム生成 |
| `useSequence(charsequence, start, end)` | 部分シーケンスからコードポイントストリーム生成 |
| `useSequence(text, encoding)` | テキストから指定エンコーディングのコードポイントストリーム生成 |
| `useSequence(istream, encoding)` | 入力ストリームから指定エンコーディングのコードポイントストリーム生成 |
| `useCharsequence(charsequence)` | 文字シーケンス全体をストリームとして扱う |
| `useCharsequence(charsequence, delimiter)` | 区切り文字で文字シーケンス分割 |
| `useCharsequence(istream, encoding)` | 入力ストリームから文字シーケンス全体を読込 |
| `useCharsequence(istream, delimiter, encoding)` | 区切り文字で入力ストリームから文字シーケンス読込 |

---

## 🧠 コアコンセプト：インデックス駆動のデータ世界

Semantic-Cpp はデータ処理を「要素」とその「論理的位置（インデックス）」への操作として抽象化します。これを理解することが本ライブラリを習得する鍵です。

### 1. 📐 基本インデックス変換

| メソッド | 説明 |
|----|----|
| `redirect(fn)` | コアメソッド：カスタム関数でインデックスを完全再定義 |
| `reverse()` | 全インデックス論理を反転（`redirect` 経由で実装） |
| `translate(offset)` | 固定オフセット移動 |
| `translate(translator)` | 要素とインデックスに基づく動的オフセット計算 |

### 2. 📊 ソートの「強力な」ルール

> ⚠️ **`sort()` はすべてを上書きします**：呼び出すとそれ以前のすべてのインデックス操作が破棄され、要素は値に基づいて自然順インデックスに再割当てされます。

- `sort()` → 即座に `OrderedCollectable` へ物質化、値順ソート
- `sort(comparator)` → カスタム比較関数によるソート

### 3. ⚡ 宣言的並列処理

- `parallel(n)` は意図を宣言するのみで、直ちにスレッドは起動しません
- 終端操作（`toUnordered()`、`count()` など）で初めて並列実行
- スレッドプールが自動的にタスク分散と結果統合を処理

### 4. 🎯 最終コンテナの選択指針

| 変換メソッド | 内部構造 | 性能特性 | 最適用途 |
|----|----|----|----|
| `sort()` | `OrderedCollectable` | 値順で物質化 | 値ソート、ページネーション、時系列 |
| `toOrdered()` | `OrderedCollectable` | 現在のインデックス順維持 | カスタムインデックス順保持 |
| `toUnordered()` | `UnorderedCollectable` | 平均 O(1)、最高性能 | 高速検索、重複除去、集約 |
| `toWindow()` | `WindowCollectable` | 順序付き集合ベース | スライディング／タンブリング窓分析 |
| `toStatistics()` | `Statistics` | 20以上の統計メソッド | 包括的統計分析 |

---

## 🚀 クイックスタートガイド

### インストール

すべてのヘッダーをプロジェクトディレクトリに配置し、C++17 以上をサポートするコンパイラを用意してください：

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
#include "semantics.h"  // 依存関係を自動解決
```

---

## 🎯 基本例：インデックスとソートの体験

```cpp
auto result = semantic::useRange(0, 10)
    .map(int x { return x * x; })
    .redirect(int value, auto index -> long long {
        return index * 2;
    })
    .reverse()
    .sort()                              // 値ソートを強制、全インデックス操作を上書き
    .toVector();

// 出力：0 1 4 9 16 25 36 49 64 81
```

## ⚡ 並列処理例

```cpp
auto count = semantic::useRange(1, 1000)
    .parallel(4)
    .filter(int x { return x % 2 == 0; })
    .toUnordered()
    .count();

// 出力：偶数の数：500
```

## 📊 統計分析例

```cpp
auto stats = semantic::useRange(1, 101)
    .toStatistics<int, double>();

auto avg = stats.average();               // 平均
auto med = stats.median();                // 中央値
auto std = stats.standardDeviation();     // 標準偏差
auto q1  = stats.firstQuartile();          // 第1四分位数
auto q3  = stats.thirdQuartile();          // 第3四分位数
auto skew = stats.skewness();              // 歪度
```

## 🔬 周波数領域分析例

```cpp
auto freq = data.toUnordered().frequency();
for (const auto& [element, z] : freq) {
    auto magnitude = std::abs(z);  // 分布集中度
    auto phase     = std::arg(z);  // 分布中心位相
}
```

## 🧮 FFT 変換例

```cpp
auto spectrum = semantic::useRange(0, 8)
    .map(int x -> std::complex<double> {
        return {static_cast<double>(x), 0.0};
    })
    .toUnordered()
    .collect(collector::useFFT<double>());
```

---

## ⚡ パフォーマンス最適化推奨事項

1. 🎯 **適切なコンテナを選択**
   - 等価検索、非ソート集約 → `toUnordered()`
   - 範囲検索、ソート、ページネーション → `toOrdered()` または `sort()`
   - リアルタイム窓分析 → `toWindow()`
2. ⚡ **並列処理を賢く活用**：大量データや重い処理で `parallel()` を使用し、ブロッキング I/O は避ける
3. 📐 **操作順序を最適化**：早めに `filter`、慎重に `sort`
4. 🔄 **遅延評価を活用**：中間操作は終端時に実行、`takeWhile` と `limit` で早期終了可能

---

## 📊 C++標準ライブラリおよび競合製品との比較

| 特性 | Semantic-Cpp | C++20/23 Ranges | 従来のループ |
|----|----|----|----|
| 🎯 核心パラダイム | 宣言的、インデックス駆動 | ビュー駆動、関数的合成 | 命令的、手続き型 |
| ⚡ 並列対応 | 宣言的、自動スレッドプール | 並列アルゴリズムの組み合わせが必要 | 手動実装 |
| 📐 ソートとインデックス | インデックス精密制御、負数インデックス対応 | 破壊的ソート | 完全手動 |
| 📊 統計分析 | 20以上の組み込み統計メソッド | 非搭載 | 要サードパーティ |
| 🔬 周波数領域分析 | ネイティブ DFT / FFT / 周波数特性 | 非対応 | 要サードパーティ |
| 🧮 勾配降下法 | 解析的＋数値的両対応 | 非搭載 | 要サードパーティ |
| 🌐 Unicode | ネイティブ多言語エンコーディング対応 | 非対応 | 手動処理 |
| 📦 コンテナ収集 | 20以上の標準コンテナ完全網羅 | 一部対応 | 手動実装 |
| 📦 依存関係 | 外部依存ゼロ、7ヘッダー | 標準ライブラリ | なし |

---

## 📜 ライセンス

- 📄 **ライセンス**：MIT

---

**Semantic-Cpp — モダンな C++ で効率的かつ明快なデータ処理パイプラインを構築する。🚀**
