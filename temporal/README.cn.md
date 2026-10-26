# temporal 妯″潡璁捐

> 鐘舵€侊細`gregory`銆乣Instant`銆乣Time`銆乣DateTime`銆乣OffsetDateTime`銆乣Zone`銆乣ZoneTemporal`銆乣Duration` 宸插疄鐜板苟閫氳繃娴嬭瘯锛汭ANA 鏃跺尯鏁版嵁搴撱€乣Clock`銆乣natural` 鍙繚鐣欐灦鏋勪綅缃紝鏆備笉瀹炵幇銆?
## 0. 鏈疆浼樺寲鎽樿

绗竴鐗堝彧鏈?`Date` + `Instant` + `Period` + `Unit`/`Field`銆傚鐓ф洿澶氫富娴佽瑷€鍚庯紝鏋舵瀯涓婄己鐨勪笉鏄洿澶氭棩鏈熺畻娉曪紝鑰屾槸**绫诲瀷灞傛**锛?
| 缂哄彛 | 浼樺寲 |
| :--- | :--- |
| 娌℃湁"涓€澶╁唴鏃跺埢"绫诲瀷锛宍Instant` 琚揩鍏煎仛閽熻〃 | 鏂板 `Time`锛堢撼绉掓棩鍐呭亸绉伙級 |
| 娌℃湁"鏈湴鏃ユ湡鏃堕棿"锛屾棩鏈熶笌閽熻〃鏃犳硶缁勫悎 | 鏂板 `BasicDateTime<Calendar>` / `gregory::DateTime` |
| 娌℃湁绮剧‘鏃堕暱锛宍Instant` API 鐩存帴鏆撮湶 `int64` 绾崇 | 鏂板 `Duration`锛堢 + 绾崇锛夛紝`Instant 卤 Duration`銆乣Instant - Instant -> Duration` |
| 娌℃湁鍥哄畾鍋忕Щ灞傦紝鏈湴鍊间笌 UTC 鏃犳硶姣旇緝/搴忓垪鍖?| 鏂板 `Offset` 涓?`BasicOffsetDateTime`/`gregory::OffsetDateTime`锛屾寜浠ｈ〃鐬棿姣旇緝骞跺彲涓?Unix 绉掍簰杞?|
| 娌℃湁鏃跺尯灞傦紝DST gap/overlap 鏃犳硶琛ㄨ揪 | 鏂板 `Zone`锛坕d + 鍒濆鍋忕Щ + 杞崲琛級涓?`ZoneTemporal`锛堟湰鍦版棩鏈熸椂闂?+ 鏃跺尯锛屽甫 `Disambiguation`锛?|
| `Field` 涓?`TimeField` 涓ゅ瀛楁璇嶆眹 | 鍚堝苟涓虹粺涓€鐨?`Field`锛屽悇绫诲瀷鐢?`isSupported` 鍥炵瓟 |
| 鏈堟湯鍔犳硶鍙湁 clamp锛屾棤娉曢€夋嫨鎷掔粷 | 鏂板 `Overflow::{Constrain, Reject}`锛圱emporal 鐨?overflow 閫夐」锛?|
| 渚濊禆鍏崇郴鍚硦 | 鏄庣‘ `Instant`锛堢粷瀵癸級 / `ZoneTemporal`锛堟椂鍖猴級 / `OffsetDateTime`锛堝亸绉伙級 / `DateTime`锛堟湰鍦帮級 / `Date`锛堟棩鍘嗭級 / `Time`锛堥挓琛級鍏眰锛屼簰涓嶉殣寮忚浆鎹?|

浠嶆湭鍋氾細IANA 鏃跺尯鏁版嵁搴撱€乣Clock`銆佹牸寮忓寲鍣ㄣ€乣Instant 鈫?natural::Date` 鐨勬ˉ銆傚師鍥犺 搂3.1銆伮?.9 涓?搂7銆?
---

## 1. 鑼冨洿涓庤竟鐣?
| 鍋?| 涓嶅仛锛堢暀缁欏悗缁?涓婂眰锛?|
| :--- | :--- |
| 姘戠敤鏃ユ湡銆両SO 鍛ㄦ棩鏈熴€佹椂鍒汇€佹湰鍦版棩鏈熸椂闂淬€佸浐瀹氬亸绉?| 鏃跺尯瑙勫垯搴撱€佸浠ゆ椂娑堟 |
| 绮剧‘鏃堕暱 `Duration` 涓庢棩鍘嗗懆鏈?`Period` | 鏈湴鍖栨湀浠藉悕銆丆LDR 鏍煎紡銆?2/24 灏忔椂鍒?|
| 涓ユ牸 ISO 8601 鏂囨湰 | 闂扮 |
| 缂栬瘧鏈熷巻娉曠瓥鐣ワ紙Gregorian銆佹湭鏉?Natural锛?| 杩愯鏈熺涓夋柟鍘嗘硶鎻掍欢 |
| `constexpr` 鍊艰繍绠椼€佺簿纭暣鏁拌繍绠?| 澶╂枃鍘嗚〃涓殑 螖T锛圢atural 鍐呴儴澶勭悊锛?|

C++17銆乭eader-only銆侀浂澶栭儴渚濊禆銆?
---

## 2. 璺ㄨ瑷€妯瘎

### 2.1 绗竴杞細鍏鏍稿績璁捐

| 璁捐 | 鍊煎緱鍊熼壌鐨勪寒鐐?| 蹇呴』瑙勯伩鐨勭棝鐐?|
| :--- | :--- | :--- |
| **Java `java.time`** | `Period`/`Duration` 鍒嗙锛沗ChronoUnit`/`TemporalField`锛沗WeekFields`/`IsoFields`锛沗LocalTime`/`LocalDateTime`/`OffsetDateTime`/`ZonedDateTime` 鍒嗗眰 | `LocalDate` 鍚屾椂鏄?`ChronoLocalDate`锛屼袱濂楁帴鍙ｉ噸杞芥贩涔憋紱`compareTo` 涓?`isBefore`/`timeLineOrder` 璇箟涓嶄竴鑷达紱`y`锛堢邯鍏冨勾锛変笌 `u`锛堝ぉ鏂囧勾锛夋槗閿欙紱娌℃湁鍐滃巻/甯屼集鏉ュ巻 |
| **TC39 Temporal** | `Instant`/`ZonedDateTime`/`PlainDate`/`PlainTime`/`PlainDateTime`/`Duration` 绫诲瀷鎷嗗垎锛沗overflow: constrain/reject`锛汥ST `disambiguation`锛涗弗鏍?RFC 3339/9557 | 2024 骞村垹闄ゅ彲鎻掓嫈 `Calendar`/`TimeZone` 鍗忚锛圼issue #2826](https://github.com/tc39/proposal-temporal/issues/2826)锛夛紱`month` 鏄簭鏁帮紝闂版湀闇€瑕?`monthCode`锛沗Duration` 娣峰悎骞存湀鏃ヤ笌鏃堕棿鍗曚綅锛岄『搴忛潬鏂囨。 |
| **C++20 `<chrono>` + Hinnant `date`** | 瀛楁绫诲瀷/搴忓垪绫诲瀷鍒嗙锛涘己绫诲瀷锛沗constexpr`锛沗ok()`锛汬innant era 绠楁硶 | 鍙湁鏍奸噷楂樺埄鍘嗭紱鏃?ISO 鍛紱`year_month_day` 鍙负 `!ok()`锛屾湀浠藉姞娉曞彲寰楀埌 2 鏈?31 鏃ワ紱`weekday` 鍛ㄦ棩 = 0锛涙椂鍖烘涔夐潬寮傚父 |
| **Noda Time** | 浜斾釜绫诲瀷涓ユ牸鍒嗙锛沗CalendarSystem` 涓嶅彲鍙橈紱`Period` 鎸夊瓧娈典粠澶у埌灏忋€佹湀鏈?clamp锛沗ZoneLocalMapping` 鎶婃涔変綔涓虹粨鏋?| 鍘嗘硶闆嗗悎灏侀棴锛涘笇浼潵鍘嗘牸寮忓寲/瑙ｆ瀽鍙楅棸鏈堥檺鍒讹紱鍘嗘硶 ID 绉佹湁鍖栵紱姣忓巻娉曡嚜鏈夊勾浠借寖鍥?|
| **.NET BCL** | `DateOnly` 鍙瓨 `DayNumber`锛沗TimeOnly`锛沗DateTimeOffset` 鎸夌灛闂存瘮杈冿紱`Calendar` 鍙弬鏁板寲 | `DateTime.Kind` 闄烽槺锛沗==` 鍙瘮 ticks锛沗TwoDigitYearMax` 鍦?.NET 8 浠?2029 鏀逛负 2049锛涙棤 year 0锛涙棤 1582 鍒囩偣 |
| **Rust `chrono` / `time`** | `Option`/`Result` 鏋勯€狅紱`checked_add_months` clamp锛沗IsoWeek`锛沗LocalResult` | 鍙湁 ISO锛涢棸绉掓槸琛ㄧず hack锛沗Date<Tz>` 寮冪敤鍒悕锛涙病鏈夐€氱敤鍗曚綅鎶借薄 |
| **Python `datetime`** | naive/aware 鏂囨。锛涗弗鏍?`fromisoformat`锛沗zoneinfo`/`fold`锛沷rdinal 1 = 0001-01-01 | naive/aware 鍚岀被鍨嬶紱`utcnow()` 杩斿洖 naive锛?.12 寮冪敤锛夛紱鏃?year 0/BCE锛涙棤鏈?骞寸畻鏈紱DST 鍔犳硶鏄涓婃椂閽熻涔?|
| **ICU / CLDR** | 鍘嗘硶绫诲瀷鏄?locale 鏁版嵁锛涘懆瑙勫垯鍙傛暟鍖栵紱闂版湀鐙珛瀛楁 `UCAL_IS_LEAP_MONTH`锛?582 鍒囩偣鍙厤缃?| `MONTH` 0 鍩猴紱榛樿 lenient锛涘彲鍙?set/get锛沗add`/`roll`/`fieldDifference` 涓夌璇箟锛涢棸鏈堜笌鍚屽悕鏈堝叡浜湀鍙凤紱鏍煎紡/鏃跺尯/鍦板尯鍏ㄥ湪涓€涓璞?|

### 2.2 绗簩杞細鍏朵綑涓绘祦璁捐

| 璁捐 | 浜偣 | 鐥涚偣 |
| :--- | :--- | :--- |
| **Go `time`** | `Time` 鍚屾椂鎼哄甫 wall clock銆佸崟璋冩椂閽熻鏁颁笌 `Location`锛沗Duration` 鏄嫭绔嬬殑鏈夌鍙风撼绉掔被鍨嬶紱`AddDate` 鏄庣‘瀹氫箟婧㈠嚭褰掍竴鍖栵紱甯冨眬寮忚В鏋?| `==` 鍙楀崟璋冭鏁颁笌 location 褰卞搷锛岃法杩涚▼/搴忓垪鍖栨瘮杈冭灏忓績锛沗AddDate` 浼氭妸 1 鏈?31 鏃?+ 1 鏈堝綊涓€鍖栧埌 3 鏈堬紱娌℃湁鐙珛 `LocalDate`/`LocalTime` 绫诲瀷锛屽瓧娈佃闂潬鏂规硶 |
| **Swift Foundation** | `Date` 鏄函鐬棿锛沗Calendar`/`DateComponents` 璐熻矗瀛楁涓庢棩鍘嗭紱`Calendar.date(from:)` 杩斿洖 Optional锛沗TimeZone` 鐙珛 | `Date` 鏈韩涓嶅彲璇诲瓧娈碉紝蹇呴』鍊?`Calendar`锛沗DateFormatter` 鏄傝吹涓?locale 鏁忔劅锛涙病鏈夊己绫诲瀷鐨?`LocalDate`/`LocalTime` |
| **Kotlin `kotlinx-datetime`** | `Instant`/`LocalDate`/`LocalTime`/`LocalDateTime`/`TimeZone`锛沗DatePeriod` 涓?`DateTimePeriod` 鍒嗙锛涗笌 java.time 姒傚康瀵归綈 | 浠嶄緷璧栧钩鍙版椂鍖烘暟鎹紱`LocalDateTime.toInstant` 闇€瑕佹椂鍖猴紱閮ㄥ垎 API 涓?java.time 鍛藉悕涓嶅畬鍏ㄤ竴鑷?|
| **Ruby** | `Date` 鐢?Rational 鍎掔暐鏃ワ紝鏃ユ湡绠楁湳绮剧‘锛沗Time` 鏄?epoch + 绾崇 + 鍋忕Щ锛沗strptime`/`iso8601` | `Date`/`DateTime`/`Time` 涓夊閲嶅彔绫诲瀷锛沗DateTime` 宸茶瑙嗕负閬楃暀锛涙椂鍖烘敮鎸佸急 |
| **PHP** | `DateTimeImmutable` 涓?`DateTime` 鏄庣‘鍖哄垎锛沗DateInterval`锛沗DateTimeZone` | 鍙彉/涓嶅彲鍙樹袱濂?API锛沗modify()` 鐨勬孩鍑?褰掍竴鍖栬涓哄井濡欙紱`IntlCalendar` 鏄彟涓€濂楁ā鍨?|
| **C / POSIX** | `time_t` 鏄?epoch 绉掞紱`tm` 鏄瓧娈电粨鏋勶紱`strftime` 鐢熸€佸箍 | `tm_mon` 0 鍩恒€乣tm_year` 浠?1900锛沗mktime` 鏃㈠綊涓€鍖栧張渚濊禆鏈湴鏃跺尯锛沗timegm` 涓嶅彲绉绘锛涙棤 year 0锛沗time_t` 瀹藉害鍘嗗彶闂 |
| **Elixir `Calendar`** | `Calendar` 鏄?behaviour锛宍Date`/`Time`/`NaiveDateTime`/`DateTime` 缁撴瀯浣撴惡甯?`calendar` 妯″潡锛沗Calendar.ISO` 涓庤嚜瀹氫箟鍘嗘硶鍙彃鎷旓紱sigil | 鍘嗘硶杩愯鏈熷垎鍙戯紝瀛楁/绠楁湳瑕佸洖璋冩ā鍧楋紱`DateTime` 闇€瑕佹椂鍖烘暟鎹簱锛涜嚜瀹氫箟鍘嗘硶鐨勬纭€ц礋鎷呭湪浣跨敤鏂?|
| **Dart** | `DateTime` 鏈?`isUtc` 鏍囧織锛沗Duration` 鐙珛锛沗parse`/`toIso8601String` 绠€鍗?| 鏈湴/UTC 娣峰湪涓€涓被鍨嬶紙`isUtc` 甯冨皵锛夛紱娌℃湁 IANA 鏃跺尯锛岄潬 `timezone` 鍖咃紱瀛楁璁块棶涓庢牸寮忓寲鍒嗙 |
| **Haskell `time`** | `UTCTime`/`LocalTime`/`ZonedTime`/`Day` 绫诲瀷鍒嗙锛沗ZonedTime` 鎼哄甫鍥哄畾鍋忕Щ | `ZonedTime` 鍙湁鍋忕Щ娌℃湁瑙勫垯锛涙牸寮忓寲/瑙ｆ瀽搴撳垎鏁?|

### 2.3 浜旀潯鍙嶅鍑虹幇鐨勬柇灞傜嚎

1. **瀛楁 vs 搴忓垪**锛氬瓧娈甸€傚悎灞曠ず/淇敼锛屽簭鍒楅€傚悎姣旇緝/瀛樺偍锛涗袱鑰呭繀椤昏兘鏃犳崯浜掓崲锛屼笉鑳藉悓鏃朵綔涓烘潈濞佺姸鎬併€?2. **绮剧‘ / 鏈湴 / 甯﹀亸绉?/ 甯︽椂鍖?*锛氫富娴佸仛娉曟槸鎷嗘垚涓嶅悓绫诲瀷锛汸ython 鐨?naive/aware 涓?.NET 鐨?`Kind` 鏄弽渚嬨€?3. **鍘嗘硶韬唤鏄被鍨嬭繕鏄暟鎹?*锛欽ava 鐨勮繍琛屾湡 `Chronology` 琚嚜瀹舵枃妗ｅ姖閫€锛汿emporal 鍒犻櫎鑷畾涔夊巻娉曞崗璁紱Noda Time 灏侀棴鍘嗘硶闆嗗悎锛汦lixir 璧?behaviour 璺嚎浣嗘妸澶嶆潅搴︾暀缁欎娇鐢ㄦ柟銆傛湰椤圭洰閫夋嫨**缂栬瘧鏈熴€佸皝闂殑绛栫暐**銆?4. **闈炴硶鍊兼€庝箞琛ㄨ揪**锛氬紓甯搞€佽皳璇嶏紙`ok()`锛夈€乣Option`/`Result`銆佹樉寮忕瓥鐣ワ紙Temporal `overflow`锛夈€傛渶鍗遍櫓鐨勬槸闈欓粯褰掍竴鍖栦笌"鍙〃绀轰絾闈炴硶"銆?5. **鍛?鍦板尯瑙勫垯灞炰簬鍝噷**锛欽ava `WeekFields`銆丯oda `WeekYearRules`銆両CU 鐨?first-day/min-days 閮芥妸鍛ㄨ鍒欐斁鍦ㄥ€肩被鍨嬩箣澶栵紱C++20 骞茶剢娌℃湁 ISO 鍛ㄣ€?
---

## 3. 浼樺寲鍚庣殑鏋舵瀯

### 3.1 绫诲瀷鍒嗗眰锛氱粷瀵?/ 鏃跺尯 / 鍋忕Щ / 鏈湴 / 閽熻〃 / 鍖洪棿

| 绫诲瀷 | 璇箟 | 鏃跺尯/鍋忕Щ | 鍘嗘硶 | 瀛樺偍 |
| :--- | :--- | :--- | :--- | :--- |
| `Instant` | 缁濆鏃跺埢锛圥roleptic 1 鍩烘椂闂寸嚎锛?| 鏃?| 鏃?| `Day`锛? 鍩?Proleptic 鏃ュ彿 + 绾崇锛?|
| `Day` | `Instant` 鐨勬棩鍙峰瓨鍌?| 鏃?| 鏃?| `int64 days` |
| `ZoneTemporal` | 鏈湴鏃ユ湡鏃堕棿 + 鏃跺尯瑙勫垯 | `Zone`锛堝凡瑙ｆ瀽鍋忕Щ锛?| 妯℃澘 `Calendar` | `OffsetDateTime` + `Zone` |
| `OffsetDateTime` | 甯﹀浐瀹氬亸绉荤殑鏈湴鏃ユ湡鏃堕棿 | 鍥哄畾 `Offset` | 妯℃澘 `Calendar` | `DateTime` + `Offset` |
| `DateTime` | 鏈湴鏃ユ湡鏃堕棿 | 鏃?| 妯℃澘 `Calendar` | `Date` + `Time` |
| `Date` | 姘戠敤鏃ユ湡 | 鏃?| 妯℃澘 `Calendar` | `int32` 搴忓垪鏃?|
| `Time` | 涓€澶╁唴鏃跺埢 | 鏃?| 鏃?| `int64` 鏃ュ唴绾崇 |
| `Zone` | 鏃跺尯瑙勫垯 | 鍒濆鍋忕Щ + 杞崲琛?| 鏃?| id + `initial` + `vector<transition>` |
| `Offset` | UTC 鍋忕Щ | 鍥哄畾绉掓暟锛屄?8:00 | 鏃?| `int` 绉?|
| `Duration` | 绮剧‘鏃堕暱 | 鏃?| 鏃?| `int64` 绉?+ `int32` 绾崇 |
| `Period` | 鏃ュ巻鍛ㄦ湡 | 鏃?| 鐢卞簲鐢ㄥ喅瀹?| 骞淬€佹湀銆佹棩 |

杩欎簺绫诲瀷涔嬮棿**娌℃湁闅愬紡杞崲**锛?
- `DateTime` 涓嶆槸 `Instant`锛氬悓涓€澧欎笂鏃堕棿鍦?DST 杈圭晫鍙兘瀵瑰簲涓や釜鐬棿锛?- `ZoneTemporal` 涓嶆槸 `OffsetDateTime`锛氬墠鑰呭甫瑙勫垯锛堝悓涓€澧欎笂鏃堕棿鍙兘涓嶅瓨鍦ㄦ垨鍑虹幇涓ゆ锛夛紝鍚庤€呭彧鏄浐瀹氬亸绉伙紱
- `OffsetDateTime` 涓嶆槸 `Instant`锛氬畠甯︿竴涓浐瀹氬亸绉伙紝浣嗘病鏈夋椂鍖鸿鍒欙紝涔熶笉鑳界洿鎺ヨ惤鍒拌嚜鐒跺巻鏃堕棿绾匡紱
- `Date` 涓嶆槸 `DateTime`锛氭棩鏈熸病鏈夐挓琛紱
- `Time` 涓嶆槸 `Duration`锛氫竴澶╁唴浣嶇疆涓嶆槸闀垮害锛?- `Offset` 涓嶆槸鏃跺尯锛氬亸绉绘槸甯告暟锛屾椂鍖烘槸甯?DST 绌烘礊/閲嶅彔鐨勮鍒欓泦鍚堬紱
- `Period` 涓嶆槸 `Duration`锛歅1M 娌℃湁鍥哄畾澶╂暟銆?
`Instant` 鐨勯浂鐐规槸鑷劧鍘嗙殑 `-2696-03-11`锛圥roleptic 绗?1 澶╋級锛宍DateTime`/`Date` 鏄牸閲岄珮鍒╁巻鐨勬湰鍦板€笺€備簩鑰呬箣闂?*鏁呮剰娌℃湁鐩存帴杞崲**锛氬叕鍘嗗鎺ㄤ笌鑷劧鍘嗛浂鐐圭浉宸?14 澶╋紙椤圭洰缁撹锛夛紝妗ュ繀椤荤敱 `natural` 鍦ㄧ煡閬撶簿纭亸绉诲悗鎻愪緵銆俇nix 鎹㈢畻涓嶅彈杩欎釜閿氱偣褰卞搷锛歚Instant::fromUnixSeconds(0)` 灏辨槸 1970-01-01T00:00:00Z銆?
`Zone` 鏄€肩被鍨嬶細id銆佸垵濮嬪亸绉诲拰鏈夊簭杞崲琛ㄣ€俙ZoneTemporal` 鐢?`Zone::getLocalResult` 鎶婃湰鍦板涓婃椂闂存槧灏勪负 `Single`/`Gap`/`Overlap`锛屽啀鎸?`Disambiguation` 閫夋嫨鍋忕Щ锛沗Compatible` 鏄?java.time/Temporal 鐨勯粯璁わ紙gap 鍚戝墠璺宠繃锛宱verlap 鍙栬緝鏃╃灛闂达級銆俙OffsetDateTime` 鏄?`DateTime` + 鍥哄畾 `Offset`锛屾寜浠ｈ〃鐬棿姣旇緝锛坄2023-12-31T16:00:00Z == 2024-01-01T00:00:00+08:00`锛夛紝骞舵彁渚涗笌 Unix 绉掔殑浜掕浆锛沗toInstant()` / `fromInstant()` 涓?`Instant` 绮剧‘妗ユ帴銆侷ANA 鏃跺尯鏁版嵁搴撳皻鏈帴鍏ワ紝`Zone` 鐩墠鐢辫皟鐢ㄦ柟鏄惧紡缁欏嚭杞崲琛ㄣ€?
### 3.2 鍘嗘硶瀛楁褰掑巻娉曟墍鏈?
閫氱敤灞傚彧璁よ瘑绛栫暐鎻愪緵鐨勫叧鑱旂被鍨嬶細

```cpp
Calendar::year_type      // gregory::Year
Calendar::month_type     // gregory::Month锛堥棸鏈堝巻娉曞彲浠ユ槸甯?leap 鏍囧織鐨勭被鍨嬶級
Calendar::weekday_type   // gregory::Weekday
Calendar::fields         // DateFields<year_type, month_type>
```

`Era`銆乣Year`銆乣Month`銆乣Weekday` 閮藉湪 `temporal/gregory/`銆俙Era` 鍙湁 BCE/CE 鏄?*鏍奸噷楂樺埄鍘嗙殑**绾厓璇嶆眹锛涜嚜鐒跺巻鏈夎嚜宸辩殑绾勾绫诲瀷锛岄€氱敤灞備笉鍑虹幇 `Era`銆俙Month` 鏄?1..12 鏋氫妇锛沗Weekday` 鏄?ISO 1..7 骞舵彁渚涙樉寮忕殑鍛ㄦ棩璧锋簮杞崲銆?
### 3.3 缁熶竴鐨?`Field` 涓?`Unit`

**Field**锛坄temporal/field.h`锛夌幇鍦ㄦ槸涓€濂楄瘝姹囷紝瑕嗙洊鏃ユ湡涓庢椂鍒伙細

| 绫诲埆 | 瀛楁 |
| :--- | :--- |
| 鏃ユ湡 | `Year`銆乣YearOfEra`銆乣Month`銆乣DayOfMonth`銆乣DayOfYear`銆乣DayOfWeek`銆乣WeekOfWeekYear`銆乣WeekYear`銆乣DaysInMonth`銆乣DaysInYear`銆乣IsLeapMonth` |
| 鏃跺埢 | `ProlepticDay`銆乣EpochDay`銆乣HourOfDay`銆乣MinuteOfHour`銆乣SecondOfMinute`銆乣NanosecondOfSecond` |

- 寮虹被鍨?getter 浠嶆槸涓?API锛沗get(Field)` 杩斿洖 `int64`锛屼緵琛ㄩ┍鍔ㄤ唬鐮?鏍煎紡鍖栧櫒浣跨敤銆?- 姣忎釜绫诲瀷鐢?`isSupported(Field)` 鏄庣‘鍥炵瓟锛涗笉鏀寔鐨勫瓧娈垫姏 `std::invalid_argument`锛屼笉杩斿洖鏃犳剰涔夋暟瀛椼€?- `IsLeapMonth` 鏄粰闂版湀鍘嗘硶棰勭暀鐨勶細`Month` 鏄簭鏁帮紝闂版湀鐢辫瀛楁鍖哄垎锛屽搴?Temporal 鐨?`monthCode` 涓?ICU 鐨?`IS_LEAP_MONTH`銆?
**Unit**锛坄temporal/unit.h`锛夛細

| 绫诲埆 | 鍗曚綅 | `Date` | `DateTime` | `Instant` |
| :--- | :--- | :--- | :--- | :--- |
| 绮剧‘鏃堕棿 | 绾崇鈥﹀皬鏃?| 鎷掔粷 | 绮剧‘ | 绮剧‘ |
| 绮剧‘鏃?| 澶┿€佸懆 | 绮剧‘鏃?| 绮剧‘ | 绮剧‘ 24h |
| 鏃ュ巻 | 鏈堛€佸勾 | clamp/reject | clamp/reject | 鎷掔粷 |

### 3.4 `Duration` 涓?`Period`

- `Duration`锛歚int64 seconds + int32 nanos`锛岃鑼冨寲 `[0, 1e9)`锛涙瀯閫?鍔犲噺鎺ュ彈璐熷€煎苟鍊熶綅锛沗toNanoseconds()` 鍦ㄨ秴鍑?卤292 骞存椂鎶涘紓甯搞€?- `Period`锛氬勾/鏈?鏃ワ紝**涓?*褰掍竴鍖栦负澶╂暟锛沗Date::plus(Period)` 鐨勯『搴忓浐瀹氫负骞?鈫?鏈?鈫?鏃ャ€?- `Instant - Instant -> Duration` 娌℃湁 292 骞撮檺鍒讹紙`Duration` 鐨勭瀛楁鏄?64 浣嶏級锛沗differenceInNanoseconds()` 淇濈暀缁欓渶瑕佺撼绉掓€婚噺鐨勫満鏅€?- `DateTime::durationUntil()` 鏄椂闂寸嚎璺濈锛涙棩鍘嗚窛绂荤敤 `getDate().until()`銆?
### 3.5 `Overflow` 绛栫暐

```cpp
date.plus(1, Unit::Months);                          // Constrain锛?024-01-31 -> 2024-02-29
date.plus(1, Unit::Months, Overflow::Reject);        // 鎶?std::out_of_range
date.plus(Period(0, 1, 0), Overflow::Reject);        // 鍚屼笂
```

榛樿 `Constrain` 涓?java.time銆丯oda Time銆乀emporal 鐨?`constrain` 涓€鑷达紱`Reject` 鏄?Temporal 鐨勬樉寮忛€夐」銆傝繖鏍锋棦涓嶄細鍍?C++20 閭ｆ牱浜х敓 `!ok()` 鏃ユ湡锛屼篃涓嶉渶瑕佹妸绛栫暐鍐欐銆?
### 3.6 `Instant` 涓?`Day` 鐨勭簿纭€?
```cpp
struct Day
{
    std::int64_t days;   // 1 鍩?Proleptic 鏃ュ彿
};

class Instant
{
    Day day;
    std::int64_t nanos;  // 鏃ュ唴鍋忕Щ锛岃鑼冨寲涓?[0, 86400e9)
};
```

- `Day` **鍙淮鎶?`days`**锛沗Instant` 棰濆鎸佹湁 `nanos`锛屽苟璐熻矗褰掍竴鍖栥€佽繘浣?鍊熶綅銆佹瘮杈冧笌鏃堕暱杩愮畻銆?- `Instant` 姘歌繙浣跨敤 **1 鍩?Proleptic 鍒诲害**锛歚-2696-03-11 = 1`锛沗toProleptic()` / `toEpoch()` 鍒嗗埆杩斿洖 Proleptic / Unix 鐨?1 鍩烘棩鍙凤紙`int64`锛夈€?- `fromUnixSeconds` / `toUnixSeconds` / `fromUnixNanoseconds` / `toUnixNanoseconds` / `now()` 鎻愪緵绮剧‘鐨?Unix 浜掓搷浣滐紱`fromUnixSeconds(0).toEpoch() == 1`銆?- `OffsetDateTime` 涓?`ZoneTemporal` 鎻愪緵 `toInstant()` / `fromInstant()`锛屾ˉ鎺ユ槸绮剧‘鐨?Unix 鎹㈢畻锛屼笉缁忚繃鑷劧鍘嗗鎺ㄣ€?- 涓ょ鍒诲害鐨勯敋鐐归泦涓湪 `ProlepticDayOfUnixEpoch`锛?970-01-01 鐨?Proleptic 鏃ュ彿锛夛紝鍙敼杩欎竴澶勫嵆鍙牎鍑嗚嚜鐒跺巻鐨?14 澶╁亸宸€?- 闆剁偣 `-2696-03-11` 鏄?*鑷劧鍘?*鐨勪竴澶╋紝涓嶆槸鏍奸噷楂樺埄鏃ユ湡锛沗Instant` 涓嶅仛鍏巻鍙嶆帹銆?- 鏃犻棸绉掋€佹棤鏃跺尯锛沗Instant` 鍙部鏃堕棿绾垮仛鏁存暟鍔犳硶銆?- `Instant + Duration`銆乣Instant - Instant -> Duration`锛涙棩鍘嗘湀/骞翠笉鑳戒綔鐢ㄤ簬 `Instant`銆?
### 3.6.1 骞淬€佹湀銆佸懆銆佺邯鍏冿細鏃ュ巻鍛ㄦ湡

`temporal::{Year, Month, Week, Era}` 鏄洓涓?*浜掍笉鐩稿悓鐨勬櫘閫氱被**锛堟病鏈夌户鎵匡級锛屾瘡涓兘淇濆瓨锛?
| 瀛楁 | 鍚箟 |
| :--- | :--- |
| `absolute` | 鍒板缓鍏冪殑搴忓垪鍙凤紱渚嬪鏈堜唤鑷巻鍏冭捣鐨勬€绘湀鏁?|
| `relative` | 鍦ㄤ笂灞傚懆鏈熷唴鐨勭浉瀵瑰簭鍙凤紱渚嬪 `Month` 鐨?`relative` 灏辨槸鏈堝彿锛? 鏈堜负 2 |
| `start` | 鍛ㄦ湡绗竴涓灛闂达紙鍚級 |
| `end` | 鍛ㄦ湡缁撴潫鍚庣殑绗竴涓灛闂达紙涓嶅惈锛?|

```cpp
const temporal::Month february(24289, 2, februaryStart, marchStart);
february.getRelative();   // 2
february.contains(instant);
february.getDuration();   // end - start
```

- 鍥涗釜绫诲悇鏈?`getAbsolute()` / `getRelative()` / `getStart()` / `getEnd()` / `contains()` / `getDuration()` 鍜屾瘮杈冭繍绠楃锛涚被鍨嬩笉鍚岋紝涓嶈兘浜掔浉璧嬪€笺€?- `absolute` 涓?`relative` 鐨勫垎宸ュ拰 `Day.days` 涓€鑷达細缁濆搴忓垪鐢ㄤ簬璺ㄥ懆鏈熻绠楋紝鐩稿搴忓垪鐢ㄤ簬鈥滆繖鏄鍑犱釜鏈?绗嚑鍛ㄢ€濄€?- `temporal::Year`/`Month` 鏄?*鏃ュ巻鍛ㄦ湡**锛沗temporal::gregory::Year`/`Month` 鏄?*鍘嗘硶瀛楁鍊?*锛堝勾浠姐€佹湀浠芥灇涓撅級銆備袱鑰呭悕瀛楃浉鍚屼絾鑱岃矗涓嶅悓锛屼娇鐢ㄦ椂鏄惧紡闄愬畾鍛藉悕绌洪棿銆?
### 3.7 鏂囨湰锛氫弗鏍?ISO锛屾牸寮忓湪鍊肩被鍨嬩箣澶?
- `Date`锛歚[-]YYYY-MM-DD`锛屽洓浣嶅勾锛涙嫆缁?`2024-2-9`銆乣2024-02-30`銆乣-0000-01-01`銆?- `Time`锛歚HH:MM`銆乣HH:MM:SS`銆乣HH:MM:SS.fffffffff`锛堣緭鍏?1..9 浣嶅皬鏁帮紝杈撳嚭鍘诲熬闆讹級銆?- `DateTime`锛歚Date + "T" + Time`銆?- `Offset`锛歚Z` 鎴?`卤HH:MM`锛沗OffsetDateTime`锛歚DateTime + Offset`锛屼緥濡?`2024-01-01T00:00:00+08:00`銆?- `ZoneTemporal`锛歊FC 9557 椋庢牸 `2024-03-31T03:30:00+02:00[Test/Spring]`锛涜В鏋愰渶瑕?zone 娉ㄥ唽琛紝鏆傚彧鎻愪緵鏍煎紡鍖栥€?- 瑙ｆ瀽澶辫触杩斿洖 `std::optional`锛沗parse()` 鎶?`std::invalid_argument`銆?- 涓嶅仛 locale 鏁版嵁锛涙椂鍖虹缉鍐欍€佹湀浠藉悕銆?2/24 灏忔椂鍒跺睘浜庢牸寮忓寲灞傘€?
### 3.8 閿欒绛栫暐

| 鍦烘櫙 | 绛栫暐 |
| :--- | :--- |
| 涓嶅彲淇¤緭鍏?| `tryOf` / `tryParse` 杩斿洖 `optional` |
| 绋嬪簭閿欒 | `of` / `parse` 鎶?`std::invalid_argument` |
| 绠楁湳瓒婄晫 | `std::overflow_error` / `std::out_of_range` |
| 闈炴硶鏃ユ湡 | 浠庝笉闈欓粯褰掍竴鍖栵紱`Overflow::Reject` 鍙樉寮忔嫆缁?|

### 3.9 灏氭湭钀藉湴鐨勫眰锛屼互鍙婁负浠€涔?
- **IANA 瑙勫垯婧?*锛歚Zone` 宸茬粡鑳借〃杈捐浆鎹㈣〃涓?gap/overlap锛屼絾杩樻病鏈夎В鏋?tzdb 鐨勫姞杞藉櫒銆傛帴鍏ユ椂搴斾繚鎸?`Zone` 鐨勪笉鍙彉鍊艰涔夛紝骞舵彁渚?`LocalResult`锛圫ingle/Gap/Overlap锛夎€屼笉鏄妸姝т箟钘忓湪寮傚父閲屻€?- **`Clock`**锛歚Instant::now()` 宸叉彁渚涳紝浣嗗彲娴嬭瘯鎬ч渶瑕佸彲娉ㄥ叆鐨?`Clock`锛圫ystem/Fixed/Offset/Tick锛夛紱鍊肩被鍨嬩笉鍐呭缓鏃堕挓锛宍now()` 鍙槸渚挎嵎鍏ュ彛銆?- **鏍煎紡鍖栧櫒**锛氫弗鏍?ISO 宸插氨浣嶏紱缂栬瘧鏈熸牸寮忎覆銆佸甫瀛楄妭鍋忕Щ鐨勮В鏋愰敊璇€乴ocale/strftime 杈撳嚭鏄嫭绔嬪眰銆?- **闂扮**锛氭槑纭笉琛ㄧず锛屼笌 java.time銆乀emporal銆乣std::chrono::sys_time` 涓€鑷淬€?
---

## 4. 鐩綍涓庝緷璧?
```
temporal/
鈹溾攢鈹€ field.h                 Field锛堟棩鏈?+ 鏃跺埢缁熶竴璇嶆眹锛?鈹溾攢鈹€ unit.h                  Unit銆佸崟浣嶈皳璇嶃€佺撼绉掓崲绠?鈹溾攢鈹€ overflow.h              Overflow::{Constrain, Reject}
鈹溾攢鈹€ period.h                Period锛堝勾銆佹湀銆佹棩锛?鈹溾攢鈹€ day.h                   Day锛堝彧瀛?days锛?鈹溾攢鈹€ duration.h              Duration锛堢 + 绾崇锛?鈹溾攢鈹€ era.h / year.h / month.h / week.h   鏃ュ巻鍛ㄦ湡锛坅bsolute/relative/[start,end)锛?鈹溾攢鈹€ time.h                  Time锛堜竴澶╁唴鏃跺埢锛?鈹溾攢鈹€ calendar.h              Days銆丏ayNumber銆丏ateFields銆丅asicDate<Calendar>
鈹溾攢鈹€ date_time.h             BasicDateTime<Calendar>
鈹溾攢鈹€ offset.h                Offset锛埪?8:00锛?鈹溾攢鈹€ offset_date_time.h      BasicOffsetDateTime<Calendar>
鈹溾攢鈹€ zone.h                  Zone锛坕d + 鍒濆鍋忕Щ + 杞崲琛級
鈹溾攢鈹€ zone_temporal.h         BasicZoneTemporal<Calendar>
鈹溾攢鈹€ instant.h               Instant锛堣嚜鐒跺巻澶╂暟 + 绾崇锛?鈹溾攢鈹€ temporal.h              浼炲ご鏂囦欢
鈹溾攢鈹€ gregory/
鈹?  鈹溾攢鈹€ era.h               Era锛圔ce/Ce锛?鈹?  鈹溾攢鈹€ year.h              Year锛堣寖鍥淬€佺邯鍏冭鍥撅級
鈹?  鈹溾攢鈹€ month.h             Month锛?..12锛?鈹?  鈹溾攢鈹€ weekday.h           Weekday锛圛SO 1..7锛?鈹?  鈹溾攢鈹€ gregory.h           Calendar 绛栫暐銆丏ate/DateTime 鍒悕銆両SO 鏂囨湰
鈹?  鈹斺攢鈹€ week_date.h         WeekDate
鈹斺攢鈹€ natural/                棰勭暀锛屾殏涓嶅疄鐜?```

渚濊禆锛?
```
field.h 鈹€鈹攢 unit.h 鈹€鈹攢 duration.h 鈹€鈹€ time.h 鈹€鈹€ date_time.h 鈹€鈹€ offset_date_time.h 鈹€鈹€ zone_temporal.h
         鈹?         鈹?                                   鈹?                                   鈻?         鈹溾攢 period.h鈹斺攢 calendar.h 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹?                                   鈹?         鈹?                                                                                  鈹?         鈹斺攢鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€ zone.h 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹?         鈹?         鈹斺攢 instant.h锛堝彧渚濊禆 field/unit/duration锛屼笉渚濊禆 gregory/zone锛?```

`Instant` 涓嶇煡閬?`Date`/`DateTime`/`OffsetDateTime`/`ZoneTemporal`锛沗DateTime` 涓嶇煡閬?`Instant`銆傝繖鏉¤竟鐣屾槸鍒绘剰鐨勩€?
---

## 5. API 閫熻

```cpp
#include "temporal/temporal.h"

using namespace temporal;
using namespace temporal::gregory;

const Date date = Date::of(Year(2024), Month::February, 29);
date.get(Field::DayOfYear);                 // 60
date.plus(1, Unit::Months);                 // 2024-03-29
date.plus(1, Unit::Months, Overflow::Reject);

const Time time = Time::of(23, 30);
time.plus(Duration::of(2, Unit::Hours));    // 01:30
time.until(Time::of(1, 30));                // Duration(-22h)

const DateTime moment = DateTime::of(Year(2024), Month::February, 29, 23, 30);
moment.plus(Duration::of(2, Unit::Hours));  // 2024-03-01T01:30:00
moment.plus(Period(0, 1, 0));               // 2024-03-29T23:30:00
moment.get(Field::HourOfDay);               // 23
DateTime::parse("2024-02-29T23:30:00.5");

const Instant instant(-1, 0);               // 鑷劧鍘嗛浂鐐瑰墠涓€澶╋紙Day{-1, 0}锛?instant + Duration::of(1, Unit::Days);      // 闆剁偣
instant - Instant(0, 0);                    // Duration(-1d)
Instant(0, 0).toProleptic();                // 0锛圥roleptic 绗?0 澶?= 闆剁偣鍓嶄竴澶╋級
Instant(1704153, 0).toEpoch();              // 1
Instant::fromUnixSeconds(0).toEpoch();      // 1
Instant::now();                             // 绯荤粺鏃堕挓锛孶nix 绮剧‘
instant.getDay().getDays();                 // -1
date.toEpochDay();                          // 1970-01-01 璧凤紝1 鍩?
const OffsetDateTime wire = OffsetDateTime::parse("2024-01-01T00:00:00+08:00");
wire.toUnixSeconds();                       // 1704038400
wire.withOffsetSameInstant(Offset());       // 2023-12-31T16:00:00Z

const Zone springZone = Zone::of(
    "Test/Spring", Offset::ofHours(1),
    {{OffsetDateTime::parse("2024-03-31T01:00:00Z").toUnixSeconds(), Offset::ofHours(2)}});
const ZoneTemporal local = ZoneTemporal::of(DateTime::of(Year(2024), Month::March, 31, 2, 30), springZone);
local.to_string();                          // 2024-03-31T03:30:00+02:00[Test/Spring]
local.withZoneSameInstant(Zone::utc());     // 2024-03-31T01:30:00Z[UTC]

const WeekDate week = WeekDate::from(date); // 2024-W09-4
```

---

## 6. 鍘嗘硶绛栫暐濂戠害

`BasicDate<Calendar>` 瑕佹眰鐨?`Calendar` 鎴愬憳锛?
| 鎴愬憳 | 浣滅敤 |
| :--- | :--- |
| `year_type` / `month_type` / `weekday_type` | 鍘嗘硶鑷繁鐨勫瓧娈电被鍨?|
| `fields` = `DateFields<year_type, month_type>` | 瑙ｇ爜缁撴灉 |
| `isLeapYear`銆乣getMonthsInYear`銆乣getDaysInMonth`銆乣getDaysInYear` | 骞?鏈堥暱搴?|
| `isValidDate`銆乣toDayNumber`銆乣fromDayNumber` | 瀛楁 鈫?搴忓垪鏃?|
| `getWeekday`銆乣getDayOfYear` | 鍛ㄥ嚑銆佸勾鍐呮棩 |
| `getField`銆乣isSupportedField` | 閫氱敤瀛楁璁块棶 |
| `addYears`銆乣addMonths`锛堝甫 `Overflow`锛?| 鏃ュ巻绠楁湳 |
| `until(start, end)`銆乣until(start, end, Unit)` | 鍛ㄦ湡/鏁村崟浣嶈窛绂?|
| `getMinimumDayNumber` / `getMaximumDayNumber` | 鍙〃绀虹獥鍙?|
| `format`銆乣tryParse` | 鏂囨湰琛ㄧず |

绛栫暐鏄紪璇戞湡銆佹棤鐘舵€佺殑锛氭病鏈夎櫄鍑芥暟銆佹病鏈夊爢鍒嗛厤銆佸彲鍦?`constexpr` 涓娇鐢ㄣ€傚畠鏄唴寤哄巻娉曠殑瀹炵幇濂戠害锛屼笉鎵胯绗笁鏂逛换鎰忔墿灞曘€?
---

## 7. `natural`锛氶鐣欒璁★紙鏆備笉瀹炵幇锛?
`natural` 鏄」鐩嚜宸辩殑鏃ュ巻绯荤粺锛岄浂鐐逛负 **-2696-03-11**锛堣嚜鐒跺巻鏃ユ湡锛屼笉鏄牸閲岄珮鍒╂棩鏈燂級銆傚疄鐜板墠鍥哄畾浠ヤ笅绾︽潫锛?
1. **缁濆闆剁偣**锛歚Instant` 鐨?Proleptic 绗?1 澶╁氨鏄繖涓€澶╋紱`Instant` 涓嶉€氳繃鍏巻鍙嶆帹銆俙natural::Calendar` 瀹炵幇鍚庢彁渚?`Instant 鈫?natural::Date/DateTime` 鐨勭簿纭崲绠楋紱ISO 鏃堕棿鎴充篃鏀惧湪閭ｉ噷銆?2. **鏈堢殑琛ㄧず鏄簭鏁?+ 闂版湀鏍囧織**锛歚monthsInYear` 杩斿洖 12 鎴?13锛沗Month` 鎼哄甫 `isLeapMonth`锛涙湀闀?29/30 鐢辨湐鏃ュ喅瀹氾紱闂版湀瑙勫垯涓轰袱涓啲鑷虫湀涔嬮棿鏈?13 涓湐鏈涙湀鏃剁疆闂帮紝鍙栫涓€涓棤涓皵鐨勬湀锛屽悕绉版部鐢ㄥ墠涓€涓湀銆?3. **骞磋竟鐣屼笌鏈堜唤缂栧彿**锛氬寘鍚啲鑷崇殑鏈旀湜鏈堜负鍗佷竴鏈堬紝姝ｆ湀鏄叾鍚庣殑绗笁涓潪闂版湀锛涚邯骞寸敱 `natural::Era` 鑷繁瀹氫箟銆?4. **澶╂枃鏁版嵁**锛氬畾鏈斻€佷簩鍗佸洓鑺傛皵/涓皵銆佸寳浜椂闂?UTC+8銆佄擳 灞炰簬 `natural` 鍐呴儴锛堝彲鍙傝€冨鏄熷ぉ鏂囧巻 Sxwnl锛夛紱閫氱敤灞傚彧瑕佹眰"缁欏畾鏃ュ彿鑳借В鐮佸勾銆佹湀銆佹棩銆侀棸鏈堟爣蹇?銆?5. **鍛ㄨ鍒欎笉鍋囪 7 澶?*锛歚weekday_type` 鐢辩瓥鐣ュ畾涔夛紱`WeekDate` 鍙€傜敤浜?ISO 鍛ㄣ€?6. **`Time`/`DateTime`/`OffsetDateTime`/`ZoneTemporal` 鍙鐢?*锛歚BasicDateTime<natural::Calendar>` 缁勫悎鑷劧鍘嗘棩鏈熶笌 `Time`锛沗BasicOffsetDateTime<natural::Calendar>` 鍐嶅姞鍥哄畾鍋忕Щ锛沗BasicZoneTemporal<natural::Calendar>` 鍐嶅彔鍔犳椂鍖鸿鍒欍€俙Time`/`Zone`/`Offset` 鏈韩涓庡巻娉曟棤鍏炽€?
`temporal/natural/` 褰撳墠涓虹┖銆?
---

## 8. 娴嬭瘯涓庨獙璇?
`tests/test_temporal.cpp`锛?8 涓敤渚嬶紱鍏ㄤ粨搴?120 涓祴璇曞叏閮ㄩ€氳繃锛夛細

- **鐙珛鏁版嵁**锛歅ython `date.toordinal()`/`isocalendar()` 鍚戦噺锛涜礋骞翠唤閫愭棩绱姞鐙珛鏍稿銆?- **寰€杩?*锛氬瓧娈?鈫?鏃ュ彿銆両SO 鍛?鈫?鏃ュ彿銆乣parse(to_string())`锛?900鈥?100 姣忓ぉ ISO 鍛ㄥ線杩斻€?- **杈圭晫**锛氭渶灏?鏈€澶ф棩鏈熴€佽秺鐣屻€侀棸骞达紙鍚?0銆?4銆?100銆?400锛夈€侀潪娉曟棩鏈熴€?- **绠楁湳**锛氳法鏈?璺ㄥ勾/闂版棩銆乧lamp銆乣Overflow::Reject`銆乣Period` 椤哄簭銆佹暣鏈堣涔夈€?- **绫诲瀷灞傛**锛歚Duration` 褰掍竴鍖栦笌婧㈠嚭銆乣Time` 鐜粫涓?ISO 鏂囨湰銆乣DateTime` 鐨?Duration/Period/Unit 娣峰悎銆乣Instant 卤 Duration`銆乣Instant - Instant`銆乣Offset` 瑙ｆ瀽銆乣OffsetDateTime` 鐨勭灛闂存瘮杈冧笌 Unix 绉掋€乣Zone` 鐨?gap/overlap 鏄犲皠銆乣ZoneTemporal` 鐨勫洓绉嶆秷姝т笌璺?DST 杩愮畻銆?- **`Field`/`Unit`**锛氱粺涓€瀛楁璋撹瘝銆佸悇绫诲瀷 `isSupported`銆丏ate 鎷掔粷鏃堕棿鍗曚綅銆両nstant 鎷掔粷鏈?骞淬€?- **缂栬瘧鏈?*锛歚static_assert` 瑕嗙洊闂版棩銆佸懆鍑犮€佸簭鍒楁棩銆佽礋澶╂暟銆乣Instant` 甯冨眬銆?- **鐙珛澶存枃浠?*锛氭瘡涓?public header 鍗曠嫭缂栬瘧杩愯锛汣Test 55 椤瑰叏閮ㄩ€氳繃銆?
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## 9. 鍙傝€冩枃鐚?
**Java**

- `ChronoLocalDate`锛?https://docs.oracle.com/javase/8/docs/api/java/time/chrono/ChronoLocalDate.html>
- `LocalDate`锛?https://docs.oracle.com/javase/8/docs/api/java/time/LocalDate.html>
- `WeekFields`锛?https://docs.oracle.com/javase/8/docs/api/java/time/temporal/WeekFields.html>
- `Period`锛?https://docs.oracle.com/javase/17/docs/api/java.base/java/time/Period.html>
- `Clock`锛?https://docs.oracle.com/javase/8/docs/api/java/time/Clock.html>

**TC39 Temporal**

- 鏂囨。锛?https://tc39.es/proposal-temporal/docs/>
- 鍘嗘硶锛?https://tc39.es/proposal-temporal/docs/calendars.html>
- 鍒犻櫎鑷畾涔夊巻娉曞崗璁細<https://github.com/tc39/proposal-temporal/issues/2826>

**C++**

- P0355R7锛?https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0355r7.html>
- Hinnant 鏃ユ湡绠楁硶锛?https://howardhinnant.github.io/date_algorithms.html>
- Hinnant `date`锛?https://howardhinnant.github.io/date/date.html>
- `year_month_day`锛?https://en.cppreference.com/w/cpp/chrono/year_month_day>

**Noda Time**

- `CalendarSystem`锛?https://nodatime.org/3.1.x/api/NodaTime.CalendarSystem.html>
- `ZoneLocalMapping`锛?https://www.nodatime.org/3.3.x/api/NodaTime.TimeZones.ZoneLocalMapping.html>
- 甯屼集鏉ュ巻鏈堢紪鍙凤細<https://nodatime.org/3.1.x/api/NodaTime.Calendars.HebrewMonthNumbering.html>
- 鏃ユ湡绠楁湳锛?https://blog.nodatime.org/2010/11/joys-of-datetime-arithmetic.html>

**.NET**

- `DateOnly`锛?https://learn.microsoft.com/en-us/dotnet/api/system.dateonly>
- `GregorianCalendar`锛?https://learn.microsoft.com/en-us/dotnet/api/system.globalization.gregoriancalendar>
- `TwoDigitYearMax` 鐨?.NET 8 鍙樺寲锛?https://learn.microsoft.com/en-us/dotnet/core/compatibility/globalization/8.0/twodigityearmax-default>

**Rust**

- `chrono::NaiveDate`锛?https://docs.rs/chrono/latest/chrono/naive/struct.NaiveDate.html>
- `time::Date`锛?https://docs.rs/time/latest/time/struct.Date.html>

**Python**

- `datetime`锛?https://docs.python.org/3/library/datetime.html>
- PEP 615锛?https://peps.python.org/pep-0615/>
- PEP 495锛?https://peps.python.org/pep-0495/>

**ICU / CLDR**

- ICU 鏃ュ巻锛?https://unicode-org.github.io/icu/userguide/datetime/calendar/>
- `ChineseCalendar`锛?https://unicode-org.github.io/icu-docs/apidoc/dev/icu4j/com/ibm/icu/util/ChineseCalendar.html>
- `UCAL_IS_LEAP_MONTH`锛?https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/ucal_8h.html>

**鍏朵粬璇█**

- Go `time`锛?https://pkg.go.dev/time>
- Swift `Date`锛?https://developer.apple.com/documentation/foundation/date>
- Swift `Calendar`锛?https://developer.apple.com/documentation/foundation/calendar>
- kotlinx-datetime锛?https://github.com/Kotlin/kotlinx-datetime>
- Ruby `Date`锛?https://docs.ruby-lang.org/en/master/Date.html>
- PHP `DateTimeImmutable`锛?https://www.php.net/manual/en/class.datetimeimmutable.php>
- C `struct tm`锛?https://en.cppreference.com/w/c/chrono/tm>
- Elixir `Calendar`锛?https://hexdocs.pm/elixir/Calendar.html>
- Dart `DateTime`锛?https://api.dart.dev/stable/dart-core/DateTime-class.html>
- Haskell `Data.Time`锛?https://hackage.haskell.org/package/time/docs/Data-Time.html>

**ISO / 鍘嗘硶绠楁硶**

- ISO 8601 鍛ㄦ棩鏈燂紙Kuhn锛夛細<https://www.cl.cam.ac.uk/~mgk25/iso-time.html>
- 鍘嗘硶 FAQ锛?https://www.tondering.dk/claus/cal/julperiod.php>
- RFC 3339锛?https://www.rfc-editor.org/rfc/rfc3339>
- RFC 9557锛?https://www.rfc-editor.org/rfc/rfc9557>
- 瀵挎槦澶╂枃鍘嗭細<https://github.com/sxwnl/sxwnl>
- Sxwnl 鍐滃巻瑙勫垯锛?https://github.com/sxwnl/sxwnl/blob/main/src/sm5.htm>
