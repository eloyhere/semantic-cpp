# 🚀 Semantic-Cpp: Высокоинтеллектуальный фреймворк потоковой обработки данных на современном C++
**Semantic-Cpp** — это полностью переосмысленная современная библиотека для потоковой обработки данных на C++, построенная по модульному принципу **«множество заголовочных файлов, ноль внешних зависимостей»**. Каждый заголовочный файл наделен четко очерченной зоной ответственности и может тестироваться независимо, формируя вместе с остальными полноценную экосистему обработки потоков.
Эта библиотека творчески объединяет в себе лучшие черты различных парадигм программирования:
 * **Элегантность и гибкость Java Stream API:** цепочки вызовов и декларативное программирование делают ваш код изящным, словно стихи ✨
 * **Отложенность и гибкость JavaScript Generator:** ленивые вычисления, генерация данных по требованию и бережное отношение к памяти 🌱
 * **Эффективность и упорядоченность баз данных:** интеллектуальная сортировка, работа на основе индексов — мощный инструмент для обработки временных рядов ⏱️
 * **Философия пакетной обработки с контейнерами в качестве элементов:** векторы, связные списки, словари... любые контейнеры становятся «гражданами первого класса» в потоке и перемещаются абсолютно свободно 📦
### 💡 Зачем это нужно?
 * Вы тоже устали вручную писать циклы for для обхода vector, затем фильтровать данные через if, а после вручную вызывать push_back для заполнения другого контейнера? 😩
 * Случалось ли вам засиживаться допоздна, отлаживая баг с ошибкой на единицу (off-by-one) из-за того, что при обратном обходе вы пытались получить «третий элемент с конца»? 😵‍💫
 * Хотели бы вы работать с данными так же точно, как в базах данных (используя индексы), анализировать их с помощью скользящего окна и превращать сырой набор данных в готовую статистику всего одной цепочкой вызовов? 🤔
**Semantic-Cpp создан именно для этого.** 🔧
Фреймворк абстрагирует обработку данных до операций над **«элементом»** и его **«логической позицией (индексом)»**. Подобно «строкам» и «первичным ключам» в базах данных, вы можете свободно переставлять, смещать и инвертировать индексы, вообще не затрагивая сами данные. Более того, любой контейнер (vector, map, array...) можно передать в поток как неделимое целое, а затем в любой момент снова «распаковать» до уровня отдельных элементов. Способность так легко переключаться между двумя уровнями детализации (гранулярности) — уникальная черта, которой нет в традиционных потоковых фреймворках. 🎯
## 🏗️ Архитектура проекта: 7-уровневая модульная структура
Semantic-Cpp состоит из **7 ключевых заголовочных файлов**, плавно развивающихся от уровня к уровню. **5 пространств имен** распределяют обязанности между собой, формируя сквозной конвейер (pipeline) от источника данных до финального результата:
```
┌─────────────────────────────────────────────────┐
│               🌊 semantics.h                    │
│   Пространство имен: semantic                   │
│   Фабрика фабрик потоков: диапазоны, контейнеры, текст, Unicode │
├─────────────────────────────────────────────────┤
│                 📦 semantic.h                     │
│   Пространство имен: semantic / collectable            │
│   Промежуточные операции, система Collectable, поддержка контейнеров │
├─────────────────────────────────────────────────┤
│                ⚙️ collector.h                     │
│   Пространство имен: collector                        │
│   Фреймворк коллекторов + фабрика: поиск, агрегация, DFT/FFT │
├─────────────────────────────────────────────────┤
│                🔤 charsequence.h                   │
│   Пространство имен: charsequence                     │
│   Последовательности Unicode, конвертация кодировок, Builder, Buffer │
├─────────────────────────────────────────────────┤
│                🧵 pool.h                          │
│   Пространство имен: pool                             │
│   Глобальный пул потоков: отправка задач, экстренное завершение │
├─────────────────────────────────────────────────┤
│                📄 function.h                      │
│   Пространство имен: function                         │
│   Система типов: алиасы для Generator, Supplier, Consumer и др. │
├─────────────────────────────────────────────────┤
│               🔐 hash.h / less.h                  │
│   Пространство имен: std (расширение)              │
│   Специализация хэшей и сравнений для STL, поддержка любой вложенности │
└─────────────────────────────────────────────────┘

```
### 🧩 Взаимосвязь зависимостей
Цепочка зависимостей прозрачна, как схема четко спроектированной электронной платы. Ток зарождается в базовых определениях типов на самом нижнем уровне и поднимается вверх шаг за шагом — каждый уровень зависит строго от нижележащего. В итоге все линии сходятся в semantic.h и semantics.h, формируя монолитную функциональность потоковой обработки.
```
function.h          ← Нет зависимостей, фундамент системы типов
pool.h              ← Зависит от function.h
charsequence.h      ← Изолированный модуль для работы с Unicode
collector.h         ← Зависит от function.h, pool.h
hash.h / less.h     ← Изолированный модуль, расширение STL
semantic.h          ← Зависит от всего вышеперечисленного
semantics.h         ← Зависит от semantic.h

```
## 🌍 Панорама пространств имен
В Semantic-Cpp четко выстроена структура из 5 пространств имен. Каждое из них работает как автономный «департамент», выполняющий свою задачу, но при этом они тесно взаимодействуют друг с другом:
| Пространство имен | Файл | Обязанности | Ключевые типы / функции |
|---|---|---|---|
| **function** | function.h | Фундамент системы типов | Timestamp, Module, Generator<T>, Supplier<R>, Consumer<T>, Predicate<T> и др. |
| **pool** | pool.h | Движок параллельного выполнения | pool::pool (глобальный пул), submit(), emergencyShutdown() |
| **charsequence** | charsequence.h | Обработка Unicode-строк | charset, Meta, Point, Charsequence, Builder, Buffer и др. |
| **collector** | collector.h | Финальный сбор данных | Collector<E,A,R>, Identity<A>, Accumulator<A,E> и др. |
| **collectable** | semantic.h | Материализованные контейнеры данных | Collectable<E>, OrderedCollectable<E>, UnorderedCollectable<E> и др. |
| **semantic** | semantic.h
semantics.h | Создание потока и промежуточные операции | Semantic<E>, useRange(), useFrom() и др. |
### 🔁 Схема взаимодействия пространств имен
Потоки данных между пространствами имен напоминают сборочный конвейер на заводе. Сырье поступает в semantic, проходит через несколько стадий обработки и трансформации, после чего упаковывается в collector и отгружается. На каждом этапе границы ответственности строго разграничены:
```cpp
semantic::useRange(0, 100)          // ← Пространство semantic: создание потока
    .map(int x { return x * 2; })   // ← Пространство semantic: промежуточное преобразование
    .filter(int x { return x > 50; }) // ← Пространство semantic: промежуточная фильтрация
    .toUnordered()                  // ← Переход в пространство collectable
    .toVector();                    // ← Вызов коллектора из пространства collector

```
## 📦 Уровень 1: function.h — Основа системы типов
Файл function.h задает систему типов для всего фреймворка и служит общим фундаментом для всех без исключения модулей. 🔑
```cpp
namespace function {
    using Timestamp = long long;           // Тип индекса, «таймстамп» данных внутри потока
    using Module = unsigned long long;     // Тип счетчика / объема / модуля
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept — принимает один элемент
        std::function<bool(T, Timestamp)>       // interrupt — нужно ли прервать поток?
    )>;
}

```
Generator — это главная абстракция всей потоковой системы. 🌀 Он не возвращает данные напрямую, а принимает два колбэка: accept («все готово, прими этот элемент») и interrupt («надо ли остановиться?»).
Благодаря архитектуре **инверсии управления (Inversion of Control)** поставщик данных вообще ничего не знает о потребителе — ему достаточно «проталкивать» (Push) данные в нужный момент. В этом и заключается суть **ленивых вычислений (Lazy Evaluation)**. Данные начинают «течь» только тогда, когда вызывается accept. До этого моменты весь код — лишь декларативное описание.
| Алиас типа | Полное определение | Назначение |
|---|---|---|
| Timestamp | long long | Логическая позиция элемента в потоке |
| Module | unsigned long long | Счетчик, емкость, уровень параллелизма |
| Runnable | std::function<void()> | Задача без параметров и возвращаемого значения |
| Supplier<R> | std::function<R()> | Поставщик, создающий объект из ничего |
| Function<T,R> | std::function<R(T)> | Функция с одним параметром |
| BiFunction<T,U,R> | std::function<R(T,U)> | Функция с двумя параметрами |
| TriFunction<T,U,V,R> | std::function<R(T,U,V)> | Функция с тремя параметрами |
| Unary<T> | std::function<T(T)> | Унарная операция |
| Binary<T> | std::function<T(T,T)> | Бинарная операция |
| Consumer<T> | std::function<void(T)> | Потребитель |
| BiConsumer<T,U> | std::function<void(T,U)> | Потребитель с двумя параметрами |
| TriConsumer<T,U,V> | std::function<void(T,U,V)> | Потребитель с тремя параметрами |
| Predicate<T> | std::function<bool(T)> | Предикат (условие) |
| BiPredicate<T,U> | std::function<bool(T,U)> | Предикат с двумя параметрами |
| TriPredicate<T,U,V> | std::function<bool(T,U,V)> | Предикат с тремя параметрами |
| Comparator<T> | std::function<int(const T&,const T&)> | Компаратор, возвращает отриц./0/полож. |
| Generator<T> | BiConsumer<BiConsumer<T,Timestamp>, BiPredicate<T,Timestamp>> | Ключевая абстракция генератора потока |
## 🧵 Уровень 2: pool.h — База параллелизма
Файл pool.h предоставляет глобальный пул потоков pool::pool, который выступает многопоточным движком фреймворка. 🚀
Здесь применяется концепция **декларативного параллелизма (Declarative Parallelism)**. Когда вы пишете .parallel(4), четыре потока не запускаются сию же секунду. Эта строка кода — всего лишь намерение. Вы сообщаете фреймворку: «Я планирую обработать это в 4 потока».
Реальное параллельное выполнение начнется только в момент вызова терминальной (финальной) операции — то есть при вызове методов сбора данных, таких как toVector(), findFirst(), count().
| Возможность | Описание |
|---|---|
| **Декларативный параллелизм** | .parallel(4) — это декларация намерения выделить 4 потока; мгновенного старта не происходит |
| **Аварийный останов** | Встроенный метод emergencyShutdown() и хэндлер std::set_terminate |
| **Проброс исключений** | Метод submit() возвращает std::future, безопасно транслируя исключения в главный поток |
## 🔤 Уровень 3: charsequence.h — Unicode-последовательности
Модуль charsequence.h представляет собой полноценный инструмент для работы с Unicode, обеспечивающий создание, конвертацию и манипулирование символьными последовательностями. 🌍
Он поддерживает кодировки UTF-8, UTF-16(LE/BE), UTF-32(LE/BE), ASCII, Latin1, корректно определяет суррогатные пары (Surrogate Pairs) и возвращает стандартный символ замены U+FFFD в случае обнаружения некорректных кодовых точек (Code Points).
| Тип / Функция | Описание |
|---|---|
| charset | Перечисление кодировок: ascii, utf8, utf16, utf16be, utf16le, utf32, utf32be, utf32le, latin1 |
| Meta | Обертка метаданных, хранящая беззнаковое целое число |
| Point | Кодовая точка Unicode с поддержкой проверки валидности и суррогатных пар |
| Charsequence | Неизменяемая строка: split, replace, indexOf, lastIndexOf, sub, trim, toUpperCase, toLowerCase, reverse, startsWith, endsWith, contains, compare, getBytes, getPoints, getMetas, getCharacters, repeat, concat, count, join |
| Builder | Изменяемый байтовый билдер: prepend, insert, append (поддерживает базовые типы, Point, Charsequence, string_view) |
| Buffer | Потокобезопасный кольцевой буфер: write, read, peek, prepend, append, clear, shrinkToFit, data, size, capacity, atomic |
| PointIterator | Двунаправленный итератор по кодовым точкам Unicode |
| encode() | Кодирует одну кодовую точку в байтовую последовательность заданной кодировки |
| decode() | Декодирует следующую кодовую точку из байтового потока и автоматически сдвигает указатель |
| convert() | Конвертация кодировок (поддерживает вывод в string, vector, deque) |
## ⚙️ Уровень 4: collector.h — Архитектура коллекторов и фабрика
Файл collector.h содержит в себе ядро системы сборщиков данных Semantic-Cpp, объединяя в одном месте общую инфраструктуру коллекторов и фабричные функции.
### 🧩 5-стадийная модель
```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (Опциональное короткое замыкание)

```
| Алиас типа | Полное определение | Роль |
|---|---|---|
| Identity<A> | function::Supplier<A> | Предоставление начального значения |
| Accumulator<A,E> | function::TriFunction<A, E, Timestamp, A> | Накопление элементов |
| Combiner<A> | function::BiFunction<A, A, A> | Слияние результатов при параллельной обработке |
| Finisher<A,R> | function::Function<A, R> | Финальное преобразование |
| Interrupt<E,A> | function::TriPredicate<E, Timestamp, A> | Оценка условий для досрочного прерывания |
### 🛠️ Фабричные функции коллекторов
#### ✅ Операции сопоставления (Matching)
| Метод | Описание | Возвращаемый тип |
|---|---|---|
| useAllMatch(predicate) | Все элементы соответствуют условию | bool |
| useAnyMatch(predicate) | Хотя бы один элемент соответствует условию | bool |
| useNoneMatch(predicate) | Ни один элемент не соответствует условию | bool |
#### 🔍 Операции поиска (Finding)
| Метод | Описание | Возвращаемый тип |
|---|---|---|
| useFindFirst() | Поиск первого элемента | std::optional<E> |
| useFindLast() | Поиск последнего элемента | std::optional<E> |
| useFindAny() | Поиск любого случайного элемента | std::optional<E> |
| useFindAt(index) | Точечный поиск по позиции (поддерживает отрицательные индексы) | std::optional<E> |
| useFindMaximum() | Поиск максимального значения | std::optional<E> |
| useFindMinimum() | Поиск минимального значения | std::optional<E> |
#### 🔢 Операции агрегации (Aggregation)
| Метод | Описание | Возвращаемый тип |
|---|---|---|
| useCount() | Общее количество элементов | Module |
| useSummate<E,D>() | Сумма | D |
| useAverage<E,D>() | Среднее значение | D |
| useRange<E,D>() | Диапазон (разность макс. и мин.) | D |
#### 📉 Операции статистики (Statistics)
| Метод | Описание | Возвращаемый тип |
|---|---|---|
| useVariance<E,D>() | Дисперсия выборки | D |
| useStandardDeviation<E,D>() | Стандартное отклонение | D |
| useSkewness<E,D>() | Коэффициент асимметрии | D |
| useKurtosis<E,D>() | Коэффициент эксцесса | D |
| useMedian<E,D>() | Медиана | std::optional<D> |
| useMode<E>() | Мода (частотный анализ) | std::optional<E> |
| usePercentile<E,D>(p) | p-й процентиль | std::optional<D> |
| useFrequency<E>() | Частотные характеристики | std::map<E, complex> |
| useDistribution<E>() | Характеристики пространственного распределения | std::map<E, complex> |
#### 🔀 Операции свёртки (Reduce)
| Метод | Описание | Возвращаемый тип |
|---|---|---|
| useReduce(reducer) | Свёртка без начального значения | std::optional<E> |
| useReduce(identity, reducer) | Свёртка с начальным значением | E |
| useReduce(id, red, comb, fin) | Полностью кастомная свёртка | R |
#### 🧺 Сбор в контейнеры
| Метод | Возвращаемый тип |
|---|---|
| useToVector() | std::vector<E> |
| useToList() | std::list<E> |
| useToDeque() | std::deque<E> |
| useToForwardList() | std::forward_list<E> |
| useToArray<N>() | std::array<E, N> |
| useToSet() | std::set<E> |
| useToMultiset() | std::multiset<E> |
| useToUnorderedSet() | std::unordered_set<E> |
| useToUnorderedMultiset() | std::unordered_multiset<E> |
| useToMap(keyExtractor) | std::map<K, E> |
| useToMap(keyExtractor, valueExtractor) | std::map<K, V> |
| useToMultimap(keyExtractor) | std::multimap<K, E> |
| useToMultimap(keyExtractor, valueExtractor) | std::multimap<K, V> |
| useToUnorderedMap(keyExtractor, valueExtractor) | std::unordered_map<K, V> |
| useToUnorderedMultimap(keyExtractor) | std::unordered_multimap<K, E> |
| useToUnorderedMultimap(keyExtractor, valueExtractor) | std::unordered_multimap<K, V> |
| useToStack() | std::stack<E> |
| useToQueue() | std::queue<E> |
| useToPriorityQueue() | std::priority_queue<E> |
#### 🧩 Группировка и партиционирование
| Метод | Возвращаемый тип |
|---|---|
| useGroup(keyExtractor) | std::unordered_map<K, vector<E>> |
| useGroupBy(keyExtractor, valueExtractor) | std::unordered_map<K, vector<V>> |
| usePartition(size) | std::vector<vector<E>> |
| usePartitionBy(keyExtractor) | std::vector<vector<E>> |
| usePartitionBy(keyExtractor, valueExtractor) | std::vector<vector<V>> |
#### 📄 Строковый вывод
| Метод | Возвращаемый тип |
|---|---|
| useJoin() / useOut() / useError() и их различные перегрузки | charsequence::Charsequence |
#### 📊 Математический инструментарий
| Метод | Возвращаемый тип |
|---|---|
| useDFT() | vector<complex<double>> |
| useIDFT() | vector<complex<double>> |
| useFFT() | vector<complex<double>> |
| useIFFT() | vector<complex<double>> |
| useGradient() | vector<double> |
## 📦 Уровень 5: semantic.h — Промежуточные операции и конвейер сборки
### 🧩 Архитектура: 3-стадийный пайплайн
```
Semantic<E> (Создание и трансформация)
    ↓ toUnordered() / toOrdered() / sort() / toWindow() / toStatistics()
Collectable<E> (Материализация и сборка)
    ↓ toVector() / findFirst() / count() / summate() / ...
Конечный результат

```
> ⚠️ **Главное правило:** Сначала необходимо преобразовать Semantic<E> в Collectable<E> с помощью методов toUnordered(), toOrdered(), toWindow(), toStatistics() или sort(), и только после этого можно вызывать финальные (терминальные) методы.
> 
### 🧭 5 путей материализации
| Метод трансформации | Целевой тип | Базовая структура данных | Характеристики производительности |
|---|---|---|---|
| toUnordered() | UnorderedCollectable | unordered_map | Поиск в среднем за O(1) |
| toOrdered() | OrderedCollectable | map | Поиск за O(\log n) |
| sort() | OrderedCollectable | map (сортировка по значению) | Поиск за O(\log n) |
| toWindow() | WindowCollectable | Наследует упорядоченную коллекцию | Поддержка slide/tumble окон |
| toStatistics<D>() | Statistics<E,D> | Наследует упорядоченную коллекцию | Более 30 статистических методов |
### 📋 Collectable<E> — Все терминальные методы (в алфавитном порядке)
| Метод | Возвращаемый тип | Описание |
|---|---|---|
| allMatch(predicate) | bool | Все элементы удовлетворяют условию |
| anyMatch(predicate) | bool | Хотя бы один элемент удовлетворяет условию |
| average<D>() | D | Среднее значение |
| average<D>(mapper) | D | Среднее значение после маппинга |
| collect(identity, acc, comb, fin) | R | Кастомный 4-стадийный сбор |
| collect(identity, interrupt, acc, comb, fin) | R | Кастомный сбор с возможностью прерывания |
| count() | Module | Общее количество элементов |
| empty() | bool | Проверка, пуст ли поток |
| error() | void | Вывод в stderr (поддерживает delimiter/prefix/suffix/converter) |
| findAny() | std::optional<E> | Поиск любого элемента |
| findAt(index) | std::optional<E> | Поиск по индексу (включая отрицательные значения) |
| findFirst() | std::optional<E> | Поиск первого элемента |
| findLast() | std::optional<E> | Поиск последнего элемента |
| findMaximum() | std::optional<E> | Поиск максимума |
| findMaximum(comparator) | std::optional<E> | Поиск максимума с пользовательским компаратором |
| findMinimum() | std::optional<E> | Поиск минимума |
| findMinimum(comparator) | std::optional<E> | Поиск минимума с пользовательским компаратором |
| forEach(consumer) | void | Обход элементов с выполнением сайд-эффектов |
| group(keyExtractor) | unordered_map<K, vector<E>> | Группировка по ключу |
| groupBy(keyExtractor, valueExtractor) | unordered_map<K, vector<V>> | Группировка по ключу с извлечением значений |
| join() | Charsequence | Соединение в строку в дефолтном формате |
| join(delimiter) | Charsequence | Соединение строк через разделитель |
| join(prefix, delimiter, suffix) | Charsequence | Соединение строк в полностью кастомном формате |
| noneMatch(predicate) | bool | Ни один элемент не удовлетворяет условию |
| out() | Charsequence | Вывод в stdout (поддерживает delimiter/prefix/suffix/converter) |
| partition(size) | vector<vector<E>> | Разбиение на блоки фиксированного размера |
| partitionBy(keyExtractor) | vector<vector<E>> | Разбиение на блоки по индексному ключу |
| partitionBy(keyExtractor, valueExtractor) | vector<vector<V>> | Разбиение по ключу с извлечением значений |
| range<D>() | D | Числовой диапазон (макс. - мин.) |
| range<D>(mapper) | D | Числовой диапазон после маппинга |
| reduce(accumulator) | std::optional<E> | Свёртка без начального значения |
| reduce(identity, accumulator) | E | Свёртка с начальным значением |
| reduce(identity, acc, comb) | R | Полностью настраиваемая свёртка |
| summate<D>() | D | Сумма |
| summate<D>(mapper) | D | Сумма после маппинга |
| toArray<N>() | std::array<E, N> | Сбор в array фиксированного размера |
| toDeque() | std::deque<E> | Сбор в deque |
| toForwardList() | std::forward_list<E> | Сбор в forward_list |
| toList() | std::list<E> | Сбор в list |
| toMap(keyExtractor) | std::map<K, E> | Сбор в map на основе ключа |
| toMap(keyExtractor, valueExtractor) | std::map<K, V> | Сбор в map с кастомными парами ключ-значение |
| toMultimap(keyExtractor) | std::multimap<K, E> | Сбор в multimap по ключу |
| toMultimap(keyExtractor, valueExtractor) | std::multimap<K, V> | Сбор в multimap с кастомными парами ключ-значение |
| toMultiset() | std::multiset<E> | Сбор в multiset |
| toPriorityQueue() | std::priority_queue<E> | Сбор в priority_queue |
| toQueue() | std::queue<E> | Сбор в queue |
| toSet() | std::set<E> | Сбор в set (с сортировкой и удалением дубликатов) |
| toStack() | std::stack<E> | Сбор в stack |
| toUnorderedMap(keyExtractor, valueExtractor) | std::unordered_map<K, V> | Сбор в unordered_map |
| toUnorderedMultimap(keyExtractor) | std::unordered_multimap<K, E> | Сбор в unordered_multimap по ключу |
| toUnorderedMultimap(keyExtractor, valueExtractor) | std::unordered_multimap<K, V> | Сбор в unordered_multimap с кастомными парами |
| toUnorderedMultiset() | std::unordered_multiset<E> | Сбор в unordered_multiset |
| toUnorderedSet() | std::unordered_set<E> | Сбор в unordered_set |
| toVector() | std::vector<E> | Сбор в vector |
### 📈 Statistics<E,D> — Статистические методы
| Метод | Возвращаемый тип | Описание |
|---|---|---|
| summate() | D | Сумма |
| average() | D | Среднее значение |
| minimum() | std::optional<D> | Минимальное значение |
| maximum() | std::optional<D> | Максимальное значение |
| range() | D | Диапазон |
| variance() | D | Дисперсия выборки |
| standardDeviation() | D | Стандартное отклонение |
| median() | std::optional<D> | Медиана |
| mode() | std::optional<E> | Мода |
| percentile(p) | std::optional<D> | p-й процентиль |
| firstQuartile() | std::optional<D> | Первый квартиль (Q1) |
| thirdQuartile() | std::optional<D> | Третий квартиль (Q3) |
| interquartileRange() | std::optional<D> | Межквартильный размах (IQR) |
| skewness() | D | Асимметрия |
| kurtosis() | D | Эксцесс |
| frequency() | map<E, complex> | Частотные характеристики |
| distribute() | map<E, complex> | Пространственное распределение |
| dft() | vector<complex<double>> | Дискретное преобразование Фурье |
| idft() | vector<complex<double>> | Обратное дискретное преобразование Фурье |
| fft() | vector<complex<double>> | Быстрое преобразование Фурье |
| ifft() | vector<complex<double>> | Обратное быстрое преобразование Фурье |
| gradient(...) | vector<double> | Градиентный спуск |
> 💡 Все вышеперечисленные методы поддерживают альтернативные версии с необязательным аргументом mapper.
> 
### 🔧 Semantic<E> Методы промежуточных операций
| Категория | Метод | Описание |
|---|---|---|
| **Трансформация** | map | Преобразование «один к одному» |
|  | flatMap | Преобразование «один ко многим» со сглаживанием |
|  | flat | Сглаживание вложенных потоков (поддержка контейнеров и Semantic) |
| **Фильтрация** | filter | Фильтрация по условию |
|  | takeWhile | Получение элементов, пока выполняется условие |
|  | dropWhile | Пропуск элементов, пока выполняется условие |
|  | distinct | Удаление дубликатов (с кастомным компаратором) |
| **Контроль объема** | limit | Ограничение количества элементов |
|  | skip | Пропуск первых n элементов |
|  | sub | Извлечение поддиапазона [start, end) |
| **Индексация** | redirect | Перемаппинг индексов |
|  | reverse | Инверсия индексов (разворот) |
|  | translate | Смещение индексов |
| **Наблюдение** | peek | Просмотр элементов (без модификации потока) |
| **Параллелизм** | parallel(n) | Объявление уровня многопоточности |
| **Слияние** | concatenate | Объединение Semantic / элементов / генераторов / контейнеров |
| **Финальный шаг** | toUnordered / toOrdered / toWindow / toStatistics / sort | Преобразование в Collectable |
## 🔧 Уровень 6: semantics.h — Фабрика конвейеров потока
### 🔢 Генерация числовых диапазонов
| Метод | Описание |
|---|---|
| useRange(start, end) | Создание диапазона вида [start, end) |
| useRange(start, end, step) | Числовой диапазон с шагом (поддерживается отрицательный шаг) |
| useRangeClosed(start, end) | Создание замкнутого диапазона вида [start, end] |
| useRangeClosed(start, end, step) | Замкнутый числовой диапазон с шагом |
### ♾️ Генерация бесконечных потоков
| Метод | Описание |
|---|---|
| useInfinite(seed, generator) | Бесконечный поток на основе начального значения и генератора |
| useGenerate(supplier) | Бесконечный вызов поставщика данных |
| useGenerate(supplier, limit) | Вызов поставщика фиксированное число раз |
| useIterate(seed, generator) | Бесконечная итерация начиная с seed |
| useIterate(seed, generator, limit) | Итерация фиксированное число раз |
| useRandom() | Бесконечный поток случайных чисел |
| useRandom(min, max) | Поток случайных чисел в заданном диапазоне |
| useRandom(min, max, count) | Поток случайных чисел заданного объема в диапазоне |
### 📦 Потоки из контейнеров и элементов
| Метод | Описание |
|---|---|
| useEmpty() | Создание пустого потока |
| useOf(element) | Поток из одного элемента |
| useOf(e1, e2) | Поток из двух элементов |
| useOf(e1, e2, e3) | Поток из трех элементов |
| useOf({...}) | Поток из списка инициализации |
| useFrom(container) | Поток из стандартного STL-контейнера |
| useFrom({...}) | Поток из списка инициализации |
| useRepeat(element, count) | Повторение элемента n раз |
### 📄 Обработка текста и Unicode
| Метод | Описание |
|---|---|
| useBlob(text) | Разделение строки на поток char побайтово |
| useBlob(text, start, end) | Побайтовое разделение указанного диапазона строки |
| useBlob(istream) | Построчное чтение из входного потока |
| useBlob(istream, delimiter) | Чтение из входного потока по разделителю |
| useText(text) | Полноценный текстовый поток (Charsequence) |
| useText(text, delimiter) | Разделение текста по разделителю |
| useText(istream) | Чтение всего содержимого из входного потока |
| useSequence(charsequence) | Поток кодовых точек из символьной последовательности |
| useSequence(text, encoding) | Поток кодовых точек заданной кодировки из текста |
| useCharsequence(charsequence) | Символьная последовательность как единый поток |
| useCharsequence(charsequence, delimiter) | Разделение последовательности по разделителю |
## 🔐 Уровень 7: hash.h / less.h — Общий язык для мира контейнеров
Предоставляет полноценную поддержку хэширования и сравнения для всех контейнеров стандартной библиотеки (включая контейнеры любой глубины вложенности), а также для pair, tuple, optional, variant, типов времени chrono, комплексных чисел complex и многого другого. Теперь вложенные контейнеры произвольной структуры можно напрямую использовать в качестве ключей в unordered_set или элементов в set. 🌉
## 🚀 Советы по оптимизации производительности
 1. **Выбирайте правильный тип материализации:** Если порядок элементов не важен, используйте toUnordered(). Если нужна сортировка — вызывайте toOrdered() или sort().
 2. **Используйте многопоточность с умом:** При работе с большими массивами данных подключайте метод parallel().
 3. **Оптимизируйте порядок вызовов:** Старайтесь вызывать фильтрацию (filter) как можно раньше, а к сортировке (sort) прибегайте только при явной необходимости.
 4. **Используйте преимущества ленивых вычислений:** Методы takeWhile и limit позволяют завершить обработку потока досрочно (Early Exit).
**Semantic-Cpp** — создавайте эффективные и прозрачные конвейеры обработки данных на современном C++. 🚀🎯✨
