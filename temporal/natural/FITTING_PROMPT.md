# 任务：从《寿星数据整录——原版.xlsx》拟合 natural 历法的共享 Waveform

你是一个数值拟合会话。你的任务是从表格中提取天文事件，拟合并输出可嵌入 C++17 的 Waveform 系数。

**不要修改 `temporal/` 下的 C++ 源码。** 只产出拟合脚本、JSON/CSV 中间数据、C++ 数据头文件和报告。主会话负责把这些系数接入 `temporal/natural/`。

## 1. 输入

工作目录：

```text
R:\project\semantic-cpp
```

数据文件：

```text
R:\project\semantic-cpp\寿星数据整录——原版.xlsx
```

- 37 MB，24 个 sheet：
  - `前10` .. `前1`：天文纪年 `-9999 .. 0`
  - `第1` .. `第14`：天文纪年 `1 .. 13500`
- 每个 sheet 第 2 行是表头，第 3 行起是数据。
- 列：
  - B：年份，只在当年第一行出现，后续行留空，必须向下填充；
  - C：月名，例如 `正月小`、`二月大`、`闰六月小`、`冬月大`、`腊月小`；
  - D：朔月时刻；
  - E/F/G：上弦/望月/下弦时刻；
  - H/I：主节气名 / 时刻；
  - J/K：主中气名 / 时刻；
  - L/M：副节中气名 / 时刻；
  - N：积月；
  - O：月末积日。
- 日期时间格式：`[-]YYYY-MM-DD HH:MM:SS`，天文纪年，存在 0 年；时区固定 **UTC+8**。
- 一个年份可能 12 或 13 个月；闰月月名以 `闰` 开头。

使用 Python 的 `openpyxl`（`read_only=True`）或 pandas 读取。不要用 `datetime` 直接解析负数年份/0 年；写一个 proleptic Gregorian 的儒略日转换函数，年份用整数，日期用 UTC+8 转 UTC 后再转 JD。

## 2. 历法约定

### 2.1 建元

- 建元为天文纪年 `-2696` 年。
- 建元正月初一 = `-2696-03-11`。
- 表格中该月朔时刻为 `-2696-03-11 01:45:49`。
- 建元所在的所有单位，其 `absolute = 1`、`relative = 1`。

### 2.2 绝对序号（没有 0）

年份：

```text
天文年 -2696 -> Year.absolute = 1
天文年 -2695 -> 2
天文年 -2697 -> -1
天文年 -2698 -> -2
```

公式（`astro` 为天文年）：

```text
if astro >= -2696:  yearAbsolute = astro + 2697
else:               yearAbsolute = astro + 2696
```

月份（`jiyue` 为表格 N 列）：

```text
if jiyue >= 3749:  monthAbsolute = jiyue - 3748
else:              monthAbsolute = jiyue - 3749
```

即：建元正月 `absolute = 1`，前一个月 `absolute = -1`，不存在 0。

闰月是独立的月，`absolute` 和 `relative` 都按上一个月 +1 计数。

### 2.3 时间

- 所有时刻统一转成 **JD（UTC）** 或 Unix 秒，拟合时再归一化。
- 日期/日界从子时开始：本地 `23:00` 属于下一天；但**天文事件的时刻**保持为瞬时，不要因为日界而移动事件时刻。
- 训练中建议用：

```text
x = (JD - 2451545.0) / 36525.0   // 儒略世纪，数值稳定
```

或对离散序号使用 `x = n / 1000.0`；每个模型必须写明归一化方式。

## 3. 需要拟合的模型

所有模型都输出 `mathematics::Waveform<double>` 形式的系数：

```text
W(x) = sum_k amplitude_k * exp(i * (omega_k * x + phase_k))
```

C++ 目标类型：

```cpp
struct Term
{
    double amplitude;
    double omega;
    double phase;
};
```

每个模型必须给出：

- 线性项/基准项；
- `Term` 数组；
- 归一化变量定义；
- 训练/验证误差；
- 系数数量。

### 3.1 太阳黄经 Waveform（节气从这个函数求）

拟合连续函数：

```cpp
double sunLongitude(double time);
```

- 训练观测：每个节气、中气时刻 -> 太阳黄经目标；
- 先拟合线性黄经速率，再对残差拟合 Waveform；
- 角度必须 unwrap，避免 ±π 跳变；
- 太阳黄经目标（度）：

```text
立春 315, 雨水 330, 惊蛰 345, 春分 0,
清明 15, 谷雨 30, 立夏 45, 小满 60,
芒种 75, 夏至 90, 小暑 105, 大暑 120,
立秋 135, 处暑 150, 白露 165, 秋分 180,
寒露 195, 霜降 210, 立冬 225, 小雪 240,
大雪 255, 冬至 270, 小寒 285, 大寒 300
```

- 24 个节气共用这一条 Waveform；不要分别训练 24 条。
- 该 Waveform 的值域按 360° 分成 24 段，每段 15°；每个节气对应一个段的边界。实现上等价于 `sunLongitude(t)` 的相位落在第 `termIndex` 段。
- 由 `sunLongitude(t) = termAngle` 反求 24 个节气时刻；报告反求结果与表格时刻的误差。
- 坐标系约定：春分点 0°，立春 315°，逆时针增加；若是视黄经/包含章动，报告必须说明。

### 3.2 日月黄经差 Waveform（朔从这个函数求）

拟合连续函数：

```cpp
double moonSunDifference(double time);
```

- 使用表格中每个月四个相位：
  - 朔月 D：黄经差 `0°`；
  - 上弦 E：`90°`；
  - 望月 F：`180°`；
  - 下弦 G：`270°`。
- 先拟合线性平均角速率（约 360° / 朔望月），再对残差拟合 Waveform；
- 角度必须 unwrap；
- 由 `moonSunDifference(t) = 0` 反求每个朔，报告与 D 列时刻的误差；
- 同时输出：

```cpp
double moonLongitude(double time); // sunLongitude(time) + moonSunDifference(time)
```

- 朔望月、节气都使用共享 Waveform；不要再单独训练 12/24 条事件函数。

### 3.3 积日/积月残差

- 表格 N 列“积月”、O 列“月末积日”直接作为观测。
- 拟合：

```text
积月(n) ≈ n + Re(W_month(n))
月末积日(n) ≈ meanDay * n + Re(W_day(n))
```

- 输出：

```cpp
double cumulativeMonthResidual(Timestamp monthAbsolute);
double cumulativeDayResidual(Timestamp monthAbsolute);
```

- 说明“积月/积日”的 n 是 natural 的 `absolute` 还是表格全局序号；建议统一转成 `monthAbsolute` 后再拟合。

### 3.4 SynodicYear / TropicalYear 长度

- `SynodicYear`：从正月初一到下一正月初一；长度（天）作为 `yearAbsolute` 的函数。
- `TropicalYear`：从立春到下一立春；长度（天）作为 `yearAbsolute` 的函数。
- 输出：

```cpp
double synodicYearLength(Timestamp yearAbsolute);
double tropicalYearLength(Timestamp yearAbsolute);
```

- 分别拟合成一个 Waveform（线性项 + 残差）；不要分别训练 24 条节气函数。

### 3.5 闰年 / 闰月拟合（替代 bitmap）

**不要使用 bitmap。** 分别拟合两个二分类函数：

```cpp
bool isLeapYear(Timestamp yearAbsolute);
bool isLeapMonth(Timestamp monthAbsolute);
```

标签：

- `isLeapYear(year)`：该 `SynodicYear` 有 13 个月 -> 1，否则 0；
- `isLeapMonth(month)`：该月月名以 `闰` 开头 -> 1，否则 0。

建议模型：

```text
score(x) = Re(Waveform(x))
label = score(x) > threshold
```

- 用 Fourier 特征 + logistic/最小二乘分类；
- 输出阈值、系数、混淆矩阵；
- 如果无法做到全表 100%，报告错误年份/月份区间，不要隐藏；
- 允许分段模型，但必须在报告中说明分段点。

## 4. 输出文件

在仓库中创建：

```text
temporal/natural/data/fit_manifest.json
temporal/natural/data/waveforms.h
temporal/natural/data/FIT_REPORT.md
tools/fit_natural.py
```

`waveforms.h` 结构建议：

```cpp
#pragma once

#include <array>

#include "mathematics/waveform.h"

namespace temporal::natural::data
{

struct Term
{
    double amplitude;
    double omega;
    double phase;
};

inline constexpr double SunLongitudeOrigin = ...;
inline constexpr double SunLongitudeRate = ...;
inline constexpr std::array<Term, N> SunLongitudeTerms = {{ ... }};

inline constexpr double MoonSunOrigin = ...;
inline constexpr double MoonSunRate = ...;
inline constexpr std::array<Term, M> MoonSunTerms = {{ ... }};

// 积日/积月、年长、闰年/闰月同理

} // namespace temporal::natural::data
```

`fit_manifest.json` 至少包含：

```json
{
  "source": "寿星数据整录——原版.xlsx",
  "range": [-9999, 13500],
  "models": {
    "sunLongitude": {"terms": 0, "maxErrorDegrees": 0, "maxErrorSeconds": 0},
    "moonSunDifference": {"terms": 0, "maxErrorDegrees": 0, "maxErrorSeconds": 0},
    "cumulative": {"maxErrorDays": 0},
    "synodicYearLength": {"maxErrorSeconds": 0},
    "tropicalYearLength": {"maxErrorSeconds": 0},
    "leapYear": {"accuracy": 0, "falsePositive": 0, "falseNegative": 0},
    "leapMonth": {"accuracy": 0, "falsePositive": 0, "falseNegative": 0}
  }
}
```

## 5. 验收标准

1. 表内所有事件解析成功，报告行数、事件数、缺失/重复；
2. 训练集/验证集误差：
   - 朔、节气、年长：给出 **最大误差秒数** 和平均误差；
   - 黄经：给出角度误差；
   - 闰年/闰月：给出混淆矩阵；
3. 验证集至少包括：
   - 随机 10%；
   - 最后 500 年；
   - 建元附近 ±100 年；
4. 系数数量、头文件大小、拟合耗时；
5. 所有结果可复现（固定随机种子、记录命令）；
6. 不修改 `temporal/` 源码，不提交 `.xlsx`。

## 6. 不要做

- 不要把 37 MB xlsx 嵌入仓库；
- 不要用 bitmap 存闰年/闰月；
- 不要用 Python `datetime` 解析 0 年/负年；
- 不要把 UTC+8 当作 UTC；
- 不要把“朔的时刻”与“初一日期”混为一谈；
- 不要在没有验证的情况下声称达到秒级精度。

## 7. 报告中的关键说明

请明确写出：

- 你的时间归一化变量；
- 每个模型的项数；
- 太阳黄经的坐标系/参考点；
- 节气时刻是视黄经还是平黄经；
- 闰年/闰月模型是否分段；
- 哪些区间误差最大；
- 如果秒级精度无法达到，差多少、需要多少项/什么模型。
