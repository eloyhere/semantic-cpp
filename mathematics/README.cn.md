# mathematics 模块设计

> 状态：`Complex`、`Epicycle`、`Waveform` 已实现；采样拟合、DFT、序列化尚未实现。

## 1. 分层

| 层 | 类型 | 职责 |
| :--- | :--- | :--- |
| 常量与工具 | `Pi`、`DoublePi`、`PiOverTwo`、`Euler`、`Zero`、`One`、`DefaultTolerance`、`normalizeArgument` | 弧度常量、角度归一化 |
| 复数 | `Complex<Number>` | 直角坐标复数，提供极坐标派生量 |
| 旋转项 | `Epicycle<Number>` | 单一旋转相量 `z(t) = amplitude · e^(i(ωt + φ))` |
| 叠加 | `Waveform<Number>` | 若干 `Epicycle` 之和，容器 + 求和 |

三个类都是模板，`Number` 必须是浮点类型，否则 `static_assert` 失败。

| 文件 | 内容 |
| :--- | :--- |
| `numerical.h` | 常量、`normalizeArgument`、`Complex` |
| `epicycle.h` | `Epicycle`；底部包含 `waveform.h`，使 `Epicycle + Epicycle` 单独包含本头文件即可使用 |
| `waveform.h` | `Waveform` 与跨类型运算符 |
| `mathematics.h` | 汇总头文件，一次包含全部 |

## 2. Complex

`Complex<Number>` 同时提供直角坐标访问与极坐标派生量：

- 构造：默认、`explicit Complex(real)`、`Complex(real, imaginary)`、拷贝、移动、`fromPolar(magnitude, argument)`；
- 访问：`getReal()`、`getImaginary()`、`setReal()`、`setImaginary()`、`squaredMagnitude()`、`magnitude()`、`argument()`、`isZero()`；
- 运算：`+`、`-`、`*`、`/`（复数与任意算术标量，双向）、复合赋值、一元负号、`==`、`!=`、`closeTo()`、`operator<<`；
- 派生：`conjugate()`、`reciprocal()`、`similarity()`、`tendency()`。

其中 `a = this->argument()`、`b = other.argument()`、`ma`、`mb` 为各自模长：

| 方法 | 结果 |
| :--- | :--- |
| `similarity(other)` | `[cos(b - a), sin(b - a)]`，单位相量 |
| `tendency(other)` | `[ma · mb · cos(b - a), ma · mb · sin(b - a)]` |

`argument()` 返回 `(-π, π]` 内的主值；零向量的辐角无定义，按约定返回 `0`，不抛异常。除以零（`Complex`、标量、`reciprocal()`）抛 `std::domain_error`。

```cpp
using namespace mathematics;

const Complex<double> left = Complex<double>::fromPolar(2.0, Pi<double> / 6.0);
const Complex<double> right(0.0, 3.0);

left.argument();            // π/6
left.similarity(right);     // [cos(π/2 − π/6), sin(π/2 − π/6)] = [0.5, 0.866]
left.tendency(right);       // 2 · 3 · [0.5, 0.866] = [3, 5.196]
left.conjugate();           // 2 · e^(−iπ/6)
```

## 3. Epicycle

`Epicycle<Number>` 表示一个旋转项：`z(t) = amplitude · e^(i(ωt + φ))`。

`amplitude` 始终非负：构造时负幅值被折叠进相位（`(−A, ω, φ)` 等价于 `(|A|, ω, φ + π)`），`value(t)` 不变。相位统一归一化到 `(-π, π]`。

| 方法 | 含义 |
| :--- | :--- |
| `value(t)` | `z(t)`，返回 `Complex<Number>` |
| `magnitude(t)` | `|z(t)| = amplitude`，与 `t` 无关 |
| `argument(t)` | `z(t)` 的主辐角；`amplitude == 0` 时返回 `0` |
| `similarity(other, t)` | 同一时刻两个旋转项的相位差单位相量 |
| `tendency(other, t)` | 再乘上 `magnitude(t) · other.magnitude(t)` |
| `conjugate()` | `(A, −ω, −φ)`，即 `conj(z(t))` |
| `derivate()` | `(A·ω, ω, φ + π/2)` |
| `derivate(t)` | `A·ω·e^(i(ωt + φ + π/2))` |
| `integral()` | 原函数 `(A/ω, ω, φ − π/2)`；`ω == 0` 抛 `std::domain_error` |
| `integral(start, end)` | `(z(end) − z(start)) / (iω)`；`ω == 0` 时为 `z(start) · (end − start)` |

运算符：

- `==` / `!=` 按字段精确比较；`closeTo(other, tolerance)` 中幅值与 ω 用绝对误差，相位用模 2π 的角距离；
- 一元 `-` 相位加 π；
- 与标量、`Complex` 的 `*=`、`/=`，以及 `*`、`/`；
- `Epicycle * Epicycle` 仍闭合为一个旋转项：`(A₁A₂, ω₁ω₂, φ₁ + φ₂)`；
- `Epicycle + Epicycle` 得到 `Waveform`（不同 ω 的两项一般不能合并）。

```cpp
const Epicycle<double> term(2.0, 3.0, Pi<double> / 4.0);
term.value(0.0);       // 2e^(iπ/4)
term.derivate();       // amplitude = 6, omega = 3, phase = 3π/4
term.integral();       // amplitude = 2/3, omega = 3, phase = −π/4
```

## 4. Waveform

`Waveform<Number>` 是 `std::deque<Epicycle<Number>>` 的轻量封装，按插入顺序保存各旋转项，求值顺序无关。

- 构造：默认、`initializer_list`、迭代器区间、`std::deque`（`explicit`）、拷贝、移动；
- 容器：`size()`、`empty()`、`operator[]`、`at()`、`begin()` / `end()`、`terms()`、`add()`、`clear()`；
- `value(t)` 对所有项求和，空波形为 `0 + 0i`；
- `argument(t)` / `magnitude(t)` 由 `value(t)` 得出；`similarity` / `tendency` 使用聚合后的相量；
- `conjugate()`、`derivate()`、`integral()` 逐项处理；`integral()` 只要有一项 `ω == 0` 就抛 `std::domain_error`，`integral(start, end)` 允许 `ω == 0`；
- 运算符：`+` 为拼接（`Waveform + Waveform`、`Waveform + Epicycle`、`Epicycle + Waveform`），`-` 为左项加上右项取反，一元 `-`、标量 `*` / `/`、`* Complex`、`+=` / `-=` / `*=` / `/=`、`==` / `!=` / `closeTo()`、`operator<<`。

```cpp
const Waveform<double> wave{Epicycle<double>(2.0, 3.0, Pi<double> / 4.0),
                            Epicycle<double>(1.0, 5.0, 0.0)};

wave.value(0.5);             // 两项在 t = 0.5 的相量之和
wave.derivate().value(0.5);  // 等于 wave.derivate(0.5)
wave.integral(0.0, 1.0);     // 两项定积分之和
wave.conjugate().value(t);   // 等于 wave.value(t).conjugate()
```

## 5. 数学约定

- 所有角度为弧度，归一化到 `(-π, π]`；
- 零幅值：`Complex(0, 0).argument() == 0`；幅值为 0 的 `Epicycle` 的 `argument(t) == 0`；空 `Waveform` 的 `value`、`argument`、`magnitude` 均为 0；
- 零幅值不抛异常：`similarity` 仍返回单位相量，`tendency` 返回 0；
- 除零（复数除法、`reciprocal()`、`Epicycle` / `Waveform` 的标量或复数除法）抛 `std::domain_error`；
- `ω == 0`：`derivate()` 为零旋转项；`integral()` 抛 `std::domain_error`（原函数关于 `t` 线性，不再是旋转项）；`integral(start, end)` 返回常数相量乘以区间宽度；
- `operator==` 精确比较字段；`closeTo(other, tolerance = 1e-9)` 为绝对误差，相位按角距离比较；
- 移动构造/赋值后源对象被清零（`Complex`、`Epicycle`）；`Waveform` 的移动遵循 `std::deque` 的语义。

## 6. 测试

- `tests/test_mathematics.cpp`：18 个用例，覆盖构造、移动、四则运算、`similarity` / `tendency`、导数与积分、`ω == 0` 边界、容器与流输出；
- 全仓库：142 个测试通过；
- CTest：61 项全部通过，其中包含 4 个 mathematics 头文件的独立编译测试。

## 7. 尚未完成

- 从离散点列拟合 `Waveform`（DFT / 最小二乘）；
- `std::complex` 互操作与转换；
- 序列化、格式化与解析；
- 波形切片、区间与频谱分析。
