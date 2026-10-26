# Unicode 概念与本项目映射

> 检索/整理日期：2026-10-26。依据 Unicode Consortium 官方术语表、Unicode Standard 第 3 章、UAX #15、UAX #29、UTF/BOM FAQ、UCD `PropList.txt` 与 RFC 3629。
>
> 本文只讨论概念与代码映射，不把某一条 Unicode 规则当作静态“字符表”。Unicode 版本会更新，项目中的编码算法与属性判断应按所引用的规范版本复核。

## 1. 从“文本”到“字节”的层次

```
用户看到的字形 / 字素簇
        ↑ 字体、排版、语言规则
Unicode 抽象字符（abstract character）
        ↑ 编码（encoding）
码点（code point，U+0000..U+10FFFF）
        ↑ 编码形式（encoding form）
码元序列（code unit sequence：8/16/32 位）
        ↑ 编码方案（encoding scheme：字节序、BOM 等）
字节序列
```

这条链上每一层都不是同一个概念。最常见的错误，是把“字符”“码点”“码元”“字节”当成同义词。

## 2. 核心术语

| 中文 | 英文/Unicode 术语 | 含义 | 本项目中的对应 |
| :--- | :--- | :--- | :--- |
| 字符 | character / abstract character | 用于组织、控制或表示文本数据的抽象信息单位；没有具体字形，也不等同于用户感知的字符 | 不能用单个类完全表示；`Character` 是“码点 + 元数据”的应用值，不是抽象字符 |
| 字符集 | character set | 用于表示文本信息的元素集合 | `charset` 这个名字在项目中实际表示“编码/编码方案选择器”，不是严格的字符集 |
| 编码字符集 | coded character set (CCS) | 给每个字符分配一个数字码点的字符集；Unicode 本身是编码字符集 | `charset::utf8` 等是编码形式，不是 CCS；`ascii`/`latin1` 是遗留字符集/编码 |
| 码点 | code point | Unicode 码空间中的任意整数，范围 U+0000..U+10FFFF；不一定已分配，也不一定是标量值 | `bunch::Point`、`bunch::CodePoint`（同一类型） |
| 码点类型 | code point type | Graphic / Format / Control / Private-Use / Surrogate / Noncharacter / Reserved 七类 | `Point::isNoncharacter()`、`isSurrogate()` 等辅助判断 |
| Unicode 标量值 | Unicode scalar value | 除代理码点以外的码点；UTF-8/16/32 只能编码标量值 | `Point::isScalarValue()`；`encode()` 对非标量值替换为 U+FFFD |
| 码元 | code unit | 编码形式的最小位组合：UTF-8 为 8 位、UTF-16 为 16 位、UTF-32 为 32 位 | 由 `bunch/encode.h` 的 `encode`/`decode` 处理 |
| 编码形式 | character encoding form | 把码点映射为码元序列的规则 | `charset::utf8`、`utf16`、`utf32`；`encode()` |
| 编码方案 | character encoding scheme | 编码形式 + 字节序列化（大端/小端、BOM） | `utf16le`/`utf16be`、`utf32le`/`utf32be` |
| UTF | Unicode transformation format | 可逆地把码点（代理码点除外）映射为字节序列的算法 | UTF-8、UTF-16、UTF-32 |
| 代理项 | surrogate | UTF-16 中 D800..DFFF 的码点/码元，用于组成代理对表示增补码点；本身不是抽象字符 | `Point::isSurrogate()`；`decode` 将未配对代理替换为 U+FFFD |
| 代理对 | surrogate pair | UTF-16 中一个高代理码元加一个低代理码元，共同表示一个增补码点 | `encode`/`decode` 的 UTF-16 分支 |
| 非字符 | noncharacter | U+FDD0..U+FDEF 以及各平面末位 U+nFFFE/U+nFFFF；是合法码点，但不用于交换 | `Point::isNoncharacter()`；解码时不应当作错误替换 |
| 字素簇 | grapheme cluster | UAX #29 定义的、用户感知为“一个字符”的文本片段；可能由多个码点组成 | 本项目尚未实现 UAX #29 分段；`Bunch::size()` 是码点数 |
| 字形 | glyph | 字体中用于显示字符的具体图形；一个字符可有多个字形 | 不属于编码层；本项目不处理字体/排版 |
| 规范化 | normalization | 把等价序列转换为统一形式（NFC/NFD/NFKC/NFKD） | 本项目尚未实现 UAX #15；因此 `Bunch` 的相等比较不是规范等价比较 |
| 大小写映射 | case mapping | Unicode 默认大小写算法；可能有上下文依赖，且可能改变长度 | `Bunch::toUpperCase()`/`toLowerCase()` 目前只做 ASCII 简单映射 |

## 3. 编码形式与编码方案

| 名称 | 码元宽度 | 每个码点的码元数 | 字节序 / BOM | 说明 |
| :--- | :---: | :---: | :--- | :--- |
| UTF-8 | 8 位 | 1–4 | 无字节序；BOM 只作为签名 | ASCII 是它的子集；最长 4 字节（RFC 3629） |
| UTF-16 | 16 位 | 1–2 | 无标记时默认大端；可有 BOM | BMP 一个码元，增补字符用代理对 |
| UTF-16BE | 16 位 | 1–2 | 大端；不得使用 BOM | 显式字节序方案 |
| UTF-16LE | 16 位 | 1–2 | 小端；不得使用 BOM | 显式字节序方案 |
| UTF-32 | 32 位 | 1 | 无标记时默认大端；可有 BOM | 一个码元对应一个码点/标量值 |
| UTF-32BE | 32 位 | 1 | 大端；不得使用 BOM | 显式字节序方案 |
| UTF-32LE | 32 位 | 1 | 小端；不得使用 BOM | 显式字节序方案 |

关键点：

- **UTF-16/UTF-32 的“无标记”形式默认是大端**，不是小端。BOM 只用于无标记文本的签名；显式 `UTF-16BE`/`UTF-16LE`/`UTF-32BE`/`UTF-32LE` 不应再带 BOM。
- **UTF-8 没有字节序问题**；UTF-8 BOM（EF BB BF）只是签名，不是字节序标记。
- **码元不是字符**：UTF-16 的一个码元可能只是代理对的一半；UTF-8 的一个码元是字节，不是“字符”。

本项目当前对无标记 `utf16`/`utf32` 采用**大端默认值**；`decode` 不会自动剥离 BOM，BOM 会作为 U+FEFF 出现在结果中。需要 BOM 语义的调用方应先识别并剥离 BOM，或直接使用显式的 LE/BE 编码。

## 4. 码点、标量值、代理项、非字符

- Unicode 码空间是 **0..0x10FFFF**。
- **代理码点** D800..DFFF 是码点，但**不是 Unicode 标量值**，不能出现在 UTF-8/16/32 的合法编码中。
- **非字符**是永久保留的码点，但仍是合法码点，并且可以合法地出现在 UTF 序列中；只是不应公开交换。解析时不应把非字符替换为 U+FFFD。
- 未分配码点可以被未来版本分配；符合性实现应能容忍未知/未分配码点，而不应把它们误认为“非法 UTF”。

`Point::isValidCodePoint()` 是历史命名，实际语义是 `isScalarValue()`；本项目新增了 `isScalarValue()`、`isUnicodeCodePoint()`、`isNoncharacter()` 以便区分这些概念。

## 5. 用户感知字符：字素簇

一个码点不一定等于一个“用户看到的字符”：

- `e` + `U+0301 COMBINING ACUTE ACCENT` 是两个码点，但通常是一个字素簇。
- 很多 emoji 由 ZWJ 序列、变体选择符或多个码点组成。
- 韩文字母、组合附加符号、区域指示符等都有分段规则。

UAX #29 定义了扩展字素簇边界。与“字符数”相关的 API 应基于字素簇，而不是码点数。

本项目：

- `Bunch` 内部存储**码点序列**；`size()` 返回码点数，不是字素簇数。
- `reverse()`、`sub()`、`at()` 都按码点工作，可能拆开字素簇。
- 尚未实现 UAX #29 分段；如需用户感知字符计数、光标移动、删除等，应基于字素簇算法另行实现。

## 6. 规范化与等价

Unicode 有两种重要等价：

- **规范等价（canonical equivalence）**：例如 `é`（U+00E9）与 `e` + U+0301 在规范意义下等价；显示和行为应一致。
- **兼容等价（compatibility equivalence）**：例如全角/半角、连字、上标等；NFKC/NFKD 会消除这些区别。

四种规范化形式：

| 形式 | 含义 |
| :--- | :--- |
| NFD | 规范分解 |
| NFC | 规范分解 + 规范组合 |
| NFKD | 兼容分解 |
| NFKC | 兼容分解 + 规范组合 |

注意：

- 规范化形式**不保证对字符串拼接封闭**；拼接两个 NFC 字符串后可能需要重新规范化。
- 码点序列相等不等价于 Unicode 规范等价；比较前通常应先规范化，或明确只做二进制比较。
- NFKC/NFKD 会抹去格式区别，不能盲目用于任意文本。

本项目尚未实现 UAX #15；`Bunch::compare()`、`operator==` 进行的是码点序列比较，不是规范等价比较。

## 7. 大小写映射

Unicode 的大小写转换不是简单的 1:1 码点映射：

- `ß`（U+00DF）的大写是 `SS`，长度会变化。
- 土耳其语等有语言相关的 `i`/`I` 规则。
- 希腊语、标题大小写、词首/词中规则都需要上下文。
- `toUpperCase()`/`toLowerCase()` 若只按单个码点处理，无法表达这些情况。

本项目当前的 `Bunch::toUpperCase()`/`toLowerCase()` 只对 ASCII `a-z`/`A-Z` 做简单映射，其他码点保持不变；这是明确的限制，不应被理解为完整 Unicode 大小写转换。

## 8. 常见错误与纠正

| 错误说法 | 正确理解 |
| :--- | :--- |
| “Unicode 是 16 位编码。” | Unicode 是编码字符集，码空间为 0..0x10FFFF（21 位）；UTF-16 只是其中一种编码形式。 |
| “一个码点就是一个字符。” | 码点是数值；抽象字符、字素簇、字形都不是码点。一个抽象字符可由多个码点表示。 |
| “UTF-16 就是 UCS-2。” | UCS-2 不含代理对，只能表示 BMP；UTF-16 用代理对表示增补码点，二者不同。 |
| “UTF-16/UTF-32 默认小端。” | 无标记 UTF-16/UTF-32 默认大端；小端必须用 LE 方案或 BOM 指明。 |
| “代理项是字符。” | 代理码点不是标量值，不能作为字符单独编码；它们只用于 UTF-16 代理对。 |
| “非字符非法，解码时应替换。” | 非字符是合法码点/合法 UTF 序列，只是不应交换；不应仅因是非字符就替换。 |
| “UTF-8 最多 6 字节。” | 现代 UTF-8（RFC 3629/Unicode）最多 4 字节；5/6 字节是历史错误扩展。 |
| “ASCII 等于 UTF-8。” | ASCII 是 UTF-8 的子集；UTF-8 还能表示非 ASCII 字符。 |
| “Latin-1 是 Unicode 编码。” | Latin-1 是遗留字符集；其字节 0x00..0xFF 映射到 U+0000..U+00FF，不是 Unicode 编码形式。 |
| “大小写转换不会改变长度。” | 可能改变长度（`ß` → `SS`），也可能依赖语言/上下文。 |
| “码点序列相等就是 Unicode 文本相等。” | 规范等价文本可能有不同码点序列；需要规范化后再比较。 |
| “`size()` 就是用户看到的字符数。” | 多数实现按码点或码元计数；用户感知字符应按字素簇分段。 |
| “按码点反转/截取总是安全的。” | 可能拆开组合序列、代理对（若按码元）或 emoji ZWJ 序列。 |
| “BOM 是普通文本字符。” | BOM 是 U+FEFF 在流首的特殊用法；协议决定是否剥离；显式 BE/LE 方案不应使用 BOM。 |
| “非法 UTF-8 只需随便替换一个 U+FFFD。” | Unicode 建议按“最大子部分（maximal subpart）”替换；不要吞掉后续合法字节，也不要制造额外替换符。 |
| “Unicode 可以编码任意字节序列。” | UTF 只生成合法码元序列；任意字节不等于合法 UTF。 |

## 9. 本项目的文件、类型与职责

| 文件/类型 | 职责 | 不负责 |
| :--- | :--- | :--- |
| `bunch/charset.h` | `bunch::charset` 枚举与流输入输出；它是编码/编码方案选择器 | 不是严格的“字符集”，不表示字符 repertoire |
| `bunch/encode.h` | `sequenceLength`、`encode`、`decode`、`convert`、`appendUtf8`：码点 ↔ 码元/字节 | 不做规范化、字素分段、大小写 |
| `bunch/unicode.h` | NFC/NFD/NFKC/NFKD 规范化、大小写映射与 `TextStyle` | 不负责 locale 消息目录与复数选择 |
| `bunch/unicode_data.h` | 由 `tools/gen_unicode.py` 生成的 Unicode 15 分解/组合/大小写表 | 不是手写代码 |
| `localisation/localisation.h` | 消息本地化（Locale、Catalog、复数规则、数字格式、`TextFormat`） | 不是 Unicode 编码层 |
| `bunch/meta.h` | `Meta`：应用元数据包装器 | 不是 Unicode 属性/字符 |
| `bunch/point.h` | `Point`/`CodePoint`：码点值；`PointIterator`：码点双向迭代器 | 不是抽象字符/字素簇；不做分段 |
| `bunch/character.h` | `Character`：码点 + `Meta` 的应用值，带码点谓词 | 不是抽象字符；不等于用户感知字符 |
| `bunch/sequence.h` | `Bunch`：码点序列；字符串查找、替换、切分等 | 不自动规范化；不按字素簇索引 |
| `bunch/builder.h` | `Builder`：按目标编码构建**字节** | 位置/长度是字节，不是码点/字素 |
| `bunch/buffer.h` | `Buffer`：线程安全字节环形缓冲区 | 不做字符编码解释 |
| `bunch/bunch.h` | 聚合头：包含上述 bunch 头与编码头 | 不是独立的编码实现 |

### 9.1 已修正的行为

- **UTF-32 编码非法值**：对代理码点、>U+10FFFF 的值替换为 U+FFFD，不再生成非法 UTF-32 码元。
- **UTF-8 非法序列**：按最大子部分消费，避免把一个非法序列拆成多个错误的 U+FFFD，也避免吞掉后续合法字节。
- **无标记 UTF-16/UTF-32**：默认大端（符合 Unicode 无 BOM 时的默认值）；BOM 不被自动剥离，见上文限制。
- **`trim()`**：使用 Unicode `White_Space` 属性（U+0009..U+000D、U+0020、U+0085、U+00A0、U+1680、U+2000..U+200A、U+2028、U+2029、U+202F、U+205F、U+3000），不再只识别 ASCII 空白。
- **空分隔符 `split()`**：抛出 `std::invalid_argument`，避免旧实现中的无限循环。
- **`Point::isScalarValue()` / `isUnicodeCodePoint()` / `isNoncharacter()`**：区分码点、标量值和非字符；`isValidCodePoint()` 保留为历史别名。

### 9.2 明确未实现

- UAX #15 规范化（NFC/NFD/NFKC/NFKD）及规范等价比较。
- UAX #29 字素簇/单词/句子分段。
- 完整 Unicode 大小写映射、大小写折叠、语言相关 tailoring。
- 字符属性数据库（General_Category、Script、Bidi_Class 等）的完整查询。
- BOM 自动识别/剥离、编码自动探测。
- 排序（UCA）、双向算法（UBA）、字体/字形处理。

这些限制应在使用 `Bunch`、`Builder`、`Buffer` 的 API 时明确考虑；不要把码点层面的操作当作完整的文本处理。

## 10. 参考资料

- [Unicode Glossary of Unicode Terms](https://www.unicode.org/glossary/)：abstract character、code point、code unit、encoding form/scheme、grapheme cluster 等定义。
- [The Unicode Standard, Chapter 3: Conformance](https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/)：D7–D12、D77、UTF-16/UTF-32、U+FFFD 最大子部分等。
- [Unicode Standard Annex #29: Unicode Text Segmentation](https://www.unicode.org/reports/tr29/)：扩展字素簇与用户感知字符。
- [Unicode Standard Annex #15: Unicode Normalization Forms](https://www.unicode.org/reports/tr15/)：规范/兼容等价与 NFC/NFD/NFKC/NFKD。
- [Unicode FAQ: UTF-8, UTF-16, UTF-32 & BOM](https://www.unicode.org/faq/utf_bom.html)：UTF 定义、代理对、默认字节序、BOM 规则。
- [UCD PropList.txt](https://www.unicode.org/Public/UCD/latest/ucd/PropList.txt)：`White_Space` 等属性值。
- [RFC 3629: UTF-8](https://www.rfc-editor.org/info/rfc3629/)：UTF-8 的字节范围与最长 4 字节约束。
- [Unicode Case Charts](https://www.unicode.org/charts/case/)：大小写映射示例（含 `ß` → `SS` 等多对多映射）。
- [UTR #17: Unicode Character Encoding Model](https://www.unicode.org/reports/tr17/)：字符集、编码字符集、编码形式、编码方案的层次模型。
