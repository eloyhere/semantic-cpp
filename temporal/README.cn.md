# temporal 模块设计

> 状态：`gregory`、`Instant`、`LocalDate`、`LocalTime`、`LocalDateTime`、`OffsetGregoryDateTime`、`ZonedGregoryDateTime`、`Duration`、`Period`、`Zone` 已实现；IANA 时区数据库、`Clock`、`natural` 尚未实现。

## 1. 分层

| 层 | 类型 | 职责 |
| :--- | :--- | :--- |
| 时间线 | `Instant`、`Day`、`Duration` | 精确的日号 + 纳秒；不做公历反推 |
| 通用计数 | `Timestamp`、`Module` | `Timestamp = int64_t`，`Module = uint64_t` |
| 本地值 | `LocalTime`、`LocalDate`、`LocalDateTime` | 无偏移、无时区 |
| 偏移/时区 | `Offset`、`OffsetGregoryDateTime`、`Zone`、`ZonedGregoryDateTime` | 固定偏移与 IANA 风格规则 |
| 日历周期 | `Era`、`Year`、`Month`、`Week` | `absolute` / `relative` / `[start,end)` / 子周期 |
| 历法字段 | `Field`、`Unit`、`Period`、`Overflow` | 统一字段词汇与算术单位 |

`Instant` 固定使用 1 基 Proleptic 日号（`-2696-03-11 = 1`），Unix 换算是精确的：

```cpp
Instant::fromUnixSeconds(0).toEpoch();     // 1
Instant::fromUnixSeconds(0).toProleptic(); // 1704153
```

## 2. 本地类型

- `LocalTime` 是独立的钟表时间类，不依赖历法；
- `LocalDate` 是 Gregorian 本地日期（当前为 `BasicDate<gregory::Calendar>` 的别名）；
- `LocalDateTime` 是 `LocalDate + LocalTime` 的具体类，定义在 `date_time.h`；
- `OffsetGregoryDateTime` 在 `LocalDateTime` 上加固定 `Offset`；
- `ZonedGregoryDateTime` 在 `OffsetGregoryDateTime` 上加 `Zone` 规则；
- `gregory::Date` / `gregory::DateTime` / `gregory::OffsetDateTime` / `gregory::ZoneTemporal` 是兼容别名。

`LocalDateTime` 的 `Duration` 运算沿精确时间线跨天；`Period` 只改日历日期，保留墙上时间。

## 3. 日历周期

`Era`、`Year`、`Month`、`Week` 是四个独立类，没有继承，保留各自的重复字段：

| 字段 | 含义 |
| :--- | :--- |
| `absolute` | 到建元的绝对序列号 |
| `relative` | 在上层周期中的相对序号，例如 `Month` 的 2 月为 2 |
| `start` / `end` | 半开区间 `[start, end)` |
| 子周期 vector | `Era::years`、`Year::months`、`Month::weeks`、`Week::days` |
| 父级指针 | `Year::era`、`Month::year`、`Week::month`，非拥有指针 |

`lengthOfYear()` / `lengthOfMonth()` / `lengthOfWeek()` / `lengthOfDay()` 返回 `Module`：

- 有子周期时按子周期数量或子周期长度求和；
- 没有子周期时按 `(end - start)` 的整秒数做向下取整；
- `Week` 的 `lengthOfDay()` 来自 `days`，没有 days 时按 duration 计算。

父级使用非拥有指针，因为 `Era` 持有 `vector<Year>`，`Year` 再按值持有 `Era` 会造成无限递归。

## 4. 单位与字段

`Unit` 中 `Nanoseconds`、`Microseconds`、`Milliseconds`、`Seconds`、`Minutes`、`Hours` 是通用时间单位；`Days`、`Weeks` 是 24 小时的整数倍；`Months`、`Years` 由历法解释。

`Field::ProlepticDay` / `Field::EpochDay` 是日号字段，`isDayField` 与 `isDateField` / `isTimeField` 分开；`Date`、`DateTime`、`OffsetDateTime`、`ZoneTemporal` 都能读取日号字段。

## 5. 测试

- `tests/test_temporal.cpp`：41 个用例；
- 全仓库：123 个测试通过；
- CTest：56 项全部通过，包含每个 public header 的独立编译测试。

## 6. 尚未完成

- `Zone` 的 IANA tzdb 加载器；
- 可注入的 `Clock`；
- 非 Gregorian 历法的具体 `LocalDate<Calendar>`；
- locale/格式化层与 RFC 9557 解析；
- `MonthDay`、`YearMonth`、`Interval` 等部分日期/区间类型。
