# 🚀 Semantic-Cpp: 미래를 향한 C++ 지능형 스트림 처리 프레임워크
**Semantic-Cpp**는 완전히 재설계된 현대적인 C++ 스트림 처리 라이브러리로, **"다중 헤더 파일, 제로 외부 의존성"**의 모듈식 아키텍처를 채택하고 있습니다. 각 헤더 파일은 명확한 책임을 가지며 독립적으로 테스트 가능하여, 함께 완전한 스트림 처리 생태계를 구성합니다.
본 라이브러리는 다양한 프로그래밍 패러다임의 정수를 창의적으로 융합했습니다:
 * **Java Stream API의 우아함과 유연함:** 체인 호출, 선언적 프로그래밍으로 코드를 시처럼 우아하게 ✨
 * **JavaScript Generator의 지연성과 유연성:** 지연 계산, 필요할 때 생성, 메모리 친화적 🌱
 * **데이터베이스 인덱스의 효율성과 정렬성:** 지능형 정렬, 인덱스 기반, 시계열 데이터 처리의 강력한 도구 ⏱️
 * **컨테이너를 요소로 하는 배치 처리 철학:** 벡터, 연결 리스트, 맵... 모든 컨테이너가 스트림의 일급 시민으로 자유롭게 흐름 📦
### 💡 도입 배경
 * 손수 for 루프를 작성해 vector를 순회하고, 다시 if로 필터링한 후, 또 다른 컨테이너에 수동으로 push_back하는 데 지치셨나요? 😩
 * 역방향 순회 시 **"뒤에서 세 번째"** 요소를 가져오려다 인덱스가 한 칸 어긋나는 버그를 밤늦게 디버깅한 적이 있으신가요? 😵‍💫
 * 데이터베이스를 다루듯 인덱스로 정밀하게 찾고, 윈도우 슬라이딩으로 분석하며, 단 한 줄의 체인 호출로 데이터부터 통계까지 완벽한 여정을 완성하고 싶지 않으신가요? 🤔
**Semantic-Cpp는 바로 이를 위해 탄생했습니다.** 🔧
데이터 처리를 **"요소"**와 그 **"논리적 위치(인덱스)"**에 대한 작업으로 추상화합니다. 마치 데이터베이스의 "행"과 "기본 키"처럼, 데이터 자체를 건드리지 않고도 인덱스를 자유롭게 재배열, 오프셋, 반전시킬 수 있으며, 어떤 컨테이너(vector, map, array...)든 스트림 내에서 분할할 수 없는 하나의 전체로 전달했다가 언제든지 다시 요소 수준으로 **"언패킹"**할 수 있습니다. 이처럼 두 가지 입도(granularity)를 자유롭게 전환하는 능력은 기존 스트림 프레임워크에는 없는 특징입니다. 🎯
## 🏗️ 프로젝트 아키텍처: 7계층 모듈식 설계
Semantic-Cpp는 **7개의 핵심 헤더 파일**로 구성되며, 계층별로 점진적으로 발전합니다. **5개의 네임스페이스**가 각자의 역할을 수행하며 데이터 소스부터 최종 결과까지 완전한 파이프라인을 함께 구성합니다:
```
┌─────────────────────────────────────────────────┐
│               🌊 semantics.h                    │
│   네임스페이스: semantic                         │
│   스트림 빌더 팩토리: 숫자 범위, 컨테이너, 텍스트, 유니코드 │
├─────────────────────────────────────────────────┤
│                 📦 semantic.h                     │
│   네임스페이스: semantic / collectable            │
│   스트림 중간 연산, Collectable 체계, 컨테이너 확장 지원  │
├─────────────────────────────────────────────────┤
│                ⚙️ collector.h                     │
│   네임스페이스: collector                        │
│   컬렉터 프레임워크 + 팩토리: 매칭, 검색, 집계, 통계, DFT/FFT│
├─────────────────────────────────────────────────┤
│                🔤 charsequence.h                   │
│   네임스페이스: charsequence                     │
│   유니코드 문자 시퀀스, 다중 인코딩 변환, Builder, Buffer │
├─────────────────────────────────────────────────┤
│                🧵 pool.h                          │
│   네임스페이스: pool                             │
│   전역 스레드 풀: 작업 제출, 긴급 종료, 예외 전파      │
├─────────────────────────────────────────────────┤
│                📄 function.h                      │
│   네임스페이스: function                         │
│   타입 정의: Generator, Supplier, Consumer 등 별칭   │
├─────────────────────────────────────────────────┤
│               🔐 hash.h / less.h                  │
│   네임스페이스: std (확장)                       │
│   표준 라이브러리 컨테이너 해시 및 비교 특수화, 임의 중첩 지원 │
└─────────────────────────────────────────────────┘

```
### 🧩 의존성 관계
의존성 체인은 정교하게 설계된 회로도처럼 명확합니다. 전류가 가장 아래층의 타입 정의에서 시작하여 단계별로 위로 올라가며, 각 계층은 자신의 아래 계층에만 의존합니다. 최종적으로 모든 회선이 semantic.h와 semantics.h로 모여 완전한 스트림 처리 능력을 형성합니다.
```
function.h          ← 의존성 없음, 타입 기초석
pool.h              ← function.h에 의존
charsequence.h      ← 독립 모듈, 유니코드 처리
collector.h         ← function.h, pool.h에 의존
hash.h / less.h     ← 독립 모듈, 표준 라이브러리 확장
semantic.h          ← 위의 모든 것에 의존
semantics.h         ← semantic.h에 의존

```
## 🌍 네임스페이스 파노라마
Semantic-Cpp는 5개의 네임스페이스를 정교하게 설계했으며, 각각은 마치 독립된 "부서"처럼 각자의 임무를 수행하면서도 긴밀하게 협력합니다:
| 네임스페이스 | 소속 파일 | 책임 | 핵심 타입/함수 |
|---|---|---|---|
| **function** | function.h | 타입 시스템 기초석 | Timestamp, Module, Generator<T>, Supplier<R>, Consumer<T>, Predicate<T> 등 |
| **pool** | pool.h | 동시성 실행 엔진 | pool::pool(전역 스레드 풀), submit(), emergencyShutdown() |
| **charsequence** | charsequence.h | 유니코드 문자열 처리 | charset, Meta, Point, Charsequence, Builder, Buffer 등 |
| **collector** | collector.h | 최종 수집 실행 | Collector<E,A,R>, Identity<A>, Accumulator<A,E> 등 |
| **collectable** | semantic.h | 구체화된 데이터 컨테이너 | Collectable<E>, OrderedCollectable<E>, UnorderedCollectable<E> 등 |
| **semantic** | semantic.h
semantics.h | 스트림 빌드 및 중간 연산 | Semantic<E>, useRange(), useFrom() 등 |
### 🔁 네임스페이스 협업 흐름
네임스페이스 간 데이터 흐름은 마치 공장의 조립 라인과 같습니다. 원자재는 semantic으로 들어와 여러 단계의 가공을 거쳐 최종적으로 collector에서 포장되어 출하됩니다. 각 단계마다 명확한 책임 경계가 있습니다:
```cpp
semantic::useRange(0, 100)          // ← semantic 네임스페이스: 스트림 생성
    .map(int x { return x * 2; })   // ← semantic 네임스페이스: 중간 변환
    .filter(int x { return x > 50; }) // ← semantic 네임스페이스: 중간 필터
    .toUnordered()                  // ← collectable 네임스페이스로 변환
    .toVector();                    // ← collector 네임스페이스의 컬렉터 호출

```
## 📦 제1계층: function.h — 타입 기초
function.h는 전체 프레임워크의 타입 시스템을 정의하며, 모든 모듈의 공통 기초석입니다. 🔑
```cpp
namespace function {
    using Timestamp = long long;           // 인덱스 타입, 스트림 내 데이터의 "타임스탬프"
    using Module = unsigned long long;     // 모듈/카운트 타입
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept — 요소 하나를 받음
        std::function<bool(T, Timestamp)>       // interrupt — 중단할까?
    )>;
}

```
Generator는 전체 스트림 시스템의 핵심 추상화입니다. 🌀 데이터를 반환하지 않고, 대신 두 개의 콜백을 받습니다—accept("준비됐어, 이 요소를 받아줘")와 interrupt("멈춰야 할까?").
이러한 제어 반전(Inversion of Control) 설계 덕분에 데이터 생산자는 소비자가 누군지 전혀 알 필요 없이, 적절한 시점에 데이터를 "푸시"하기만 하면 됩니다. 이것이 바로 **지연 평가(lazy evaluation)**의 정수입니다. 데이터는 accept가 호출될 때만 실제로 "흐르며", 그 전까지 모든 것은 단지 설명일 뿐입니다.
| 타입 별칭 | 완전한 정의 | 용도 |
|---|---|---|
| Timestamp | long long | 스트림 내 요소의 논리적 위치 |
| Module | unsigned long long | 카운트, 용량, 동시성 수준 |
| Runnable | std::function<void()> | 매개변수와 반환값이 없는 작업 |
| Supplier<R> | std::function<R()> | 공급자, 무에서 유를 창조 |
| Function<T,R> | std::function<R(T)> | 단일 매개변수 함수 |
| BiFunction<T,U,R> | std::function<R(T,U)> | 이중 매개변수 함수 |
| TriFunction<T,U,V,R> | std::function<R(T,U,V)> | 삼중 매개변수 함수 |
| Unary<T> | std::function<T(T)> | 단항 연산 |
| Binary<T> | std::function<T(T,T)> | 이항 연산 |
| Consumer<T> | std::function<void(T)> | 소비자 |
| BiConsumer<T,U> | std::function<void(T,U)> | 이중 매개변수 소비자 |
| TriConsumer<T,U,V> | std::function<void(T,U,V)> | 삼중 매개변수 소비자 |
| Predicate<T> | std::function<bool(T)> | 술어 판단 |
| BiPredicate<T,U> | std::function<bool(T,U)> | 이중 매개변수 술어 |
| TriPredicate<T,U,V> | std::function<bool(T,U,V)> | 삼중 매개변수 술어 |
| Comparator<T> | std::function<int(const T&,const T&)> | 비교자, 음/0/양 반환 |
| Generator<T> | BiConsumer<BiConsumer<T,Timestamp>, BiPredicate<T,Timestamp>> | 스트림 생성자 핵심 추상화 |
## 🧵 제2계층: pool.h — 동시성 기초석
pool.h는 전역 스레드 풀 pool::pool을 제공하며, 전체 프레임워크의 동시성 엔진입니다. 🚀
**선언적 병렬 처리(declarative parallelism)** 설계를 채택했습니다. .parallel(4)라고 작성했을 때, 즉시 4개의 스레드가 시작되어 처리를 시작하지 않습니다. 이 코드 줄은 단지 "선언"일 뿐입니다. 프레임워크에게 "4개의 스레드로 병렬 처리할 계획입니다"라고 알려주는 것입니다.
실제 병렬 실행은 최종 연산이 호출될 때—즉, toVector(), findFirst(), count()와 같은 수집 메서드를 호출하는 순간에 발생합니다.
| 기능 | 설명 |
|---|---|
| **선언적 병렬** | .parallel(4)는 단지 "스레드 4개로 병렬 처리하겠다"는 선언이며, 즉시 시작하지 않음 |
| **긴급 종료** | 내장 emergencyShutdown() 및 std::set_terminate 핸들러 |
| **예외 전파** | submit()은 std::future를 반환하며, 예외를 안전하게 주 스레드로 전파 |
## 🔤 제3계층: charsequence.h — 유니코드 문자 시퀀스
charsequence.h는 완전한 유니코드 처리 모듈로, 문자 시퀀스의 생성, 변환 및 조작 기능을 제공합니다. 🌍
UTF-8, UTF-16(LE/BE), UTF-32(LE/BE), ASCII, Latin1 등 다양한 인코딩을 지원하며, 서로게이트 페어(surrogate pair)를 올바르게 감지하고 처리하며, 유효하지 않은 코드 포인트에 대해 표준 U+FFFD 대체 문자를 반환합니다.
| 타입/함수 | 설명 |
|---|---|
| charset | 열거형: ascii, utf8, utf16, utf16be, utf16le, utf32, utf32be, utf32le, latin1 |
| Meta | 메타데이터 래퍼, 부호 없는 정수 값 저장 |
| Point | 유니코드 코드 포인트, 서로게이트 감지 및 유효성 검증 지원 |
| Charsequence | 불변 문자 시퀀스: split, replace, indexOf, lastIndexOf, sub, trim, toUpperCase, toLowerCase, reverse, startsWith, endsWith, contains, compare, getBytes, getPoints, getMetas, getCharacters, repeat, concat, count, join |
| Builder | 가변 바이트 빌더: prepend, insert, append (기본 타입, Point, Charsequence, string_view 지원) |
| Buffer | 스레드 안전 링 버퍼: write, read, peek, prepend, append, clear, shrinkToFit, data, size, capacity, atomic |
| PointIterator | 양방향 반복자, 유니코드 코드 포인트 순회 |
| encode() | 단일 코드 포인트를 지정된 인코딩의 바이트 시퀀스로 인코딩 |
| decode() | 바이트 시퀀스에서 다음 코드 포인트를 디코딩하고 포인터 자동 전진 |
| convert() | 인코딩 변환 (string, vector, deque 출력 지원) |
## ⚙️ 제4계층: collector.h — 컬렉터 프레임워크 및 팩토리
collector.h는 Semantic-Cpp의 컬렉터 핵심 모듈로, 컬렉터 프레임워크와 팩토리 함수를 하나로 통합했습니다.
### 🧩 5단계 모델
```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (선택적 단락)

```
| 타입 별칭 | 완전한 정의 | 역할 |
|---|---|---|
| Identity<A> | function::Supplier<A> | 초기값 제공 |
| Accumulator<A,E> | function::TriFunction<A, E, Timestamp, A> | 요소 누적 |
| Combiner<A> | function::BiFunction<A, A, A> | 병렬 결과 병합 |
| Finisher<A,R> | function::Function<A, R> | 최종 변환 |
| Interrupt<E,A> | function::TriPredicate<E, Timestamp, A> | 단락 판단 |
### 🛠️ 컬렉터 팩토리 함수
#### ✅ 매칭 연산
| 메서드 | 설명 | 반환 타입 |
|---|---|---|
| useAllMatch(predicate) | 모든 요소 조건 만족 | bool |
| useAnyMatch(predicate) | 임의 요소 조건 만족 | bool |
| useNoneMatch(predicate) | 어떤 요소도 조건 불만족 | bool |
#### 🔍 검색 연산
| 메서드 | 설명 | 반환 타입 |
|---|---|---|
| useFindFirst() | 첫 번째 요소 검색 | std::optional<E> |
| useFindLast() | 마지막 요소 검색 | std::optional<E> |
| useFindAny() | 임의 요소 검색 | std::optional<E> |
| useFindAt(index) | 정밀 위치 지정 (음수 인덱스 지원) | std::optional<E> |
| useFindMaximum() | 최대값 검색 | std::optional<E> |
| useFindMinimum() | 최소값 검색 | std::optional<E> |
#### 🔢 집계 연산
| 메서드 | 설명 | 반환 타입 |
|---|---|---|
| useCount() | 요소 총 개수 | Module |
| useSummate<E,D>() | 합계 | D |
| useAverage<E,D>() | 평균값 | D |
| useRange<E,D>() | 범위 | D |
#### 📉 통계 연산
| 메서드 | 설명 | 반환 타입 |
|---|---|---|
| useVariance<E,D>() | 모분산 | D |
| useStandardDeviation<E,D>() | 모표준편차 | D |
| useSkewness<E,D>() | 왜도 | D |
| useKurtosis<E,D>() | 첨도 | D |
| useMedian<E,D>() | 중앙값 | std::optional<D> |
| useMode<E>() | 최빈값 (주파수 분석) | std::optional<E> |
| usePercentile<E,D>(p) | 제 p 백분위수 | std::optional<D> |
| useFrequency<E>() | 주파수 특징 | std::map<E, complex> |
| useDistribution<E>() | 공간 분포 특징 | std::map<E, complex> |
#### 🔀 리듀스 연산
| 메서드 | 설명 | 반환 타입 |
|---|---|---|
| useReduce(reducer) | 초기값 없는 리듀스 | std::optional<E> |
| useReduce(identity, reducer) | 초기값 있는 리듀스 | E |
| useReduce(id, red, comb, fin) | 완전 커스텀 | R |
#### 🧺 컨테이너로 수집
| 메서드 | 반환 타입 |
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
#### 🧩 그룹화 및 파티셔닝
| 메서드 | 반환 타입 |
|---|---|
| useGroup(keyExtractor) | std::unordered_map<K, vector<E>> |
| useGroupBy(keyExtractor, valueExtractor) | std::unordered_map<K, vector<V>> |
| usePartition(size) | std::vector<vector<E>> |
| usePartitionBy(keyExtractor) | std::vector<vector<E>> |
| usePartitionBy(keyExtractor, valueExtractor) | std::vector<vector<V>> |
#### 📄 문자열 출력
| 메서드 | 반환 타입 |
|---|---|
| useJoin() / useOut() / useError() 및 그 다양한 오버로드 | charsequence::Charsequence |
#### 📊 수학 도구
| 메서드 | 반환 타입 |
|---|---|
| useDFT() | vector<complex<double>> |
| useIDFT() | vector<complex<double>> |
| useFFT() | vector<complex<double>> |
| useIFFT() | vector<complex<double>> |
| useGradient() | vector<double> |
## 📦 제5계층: semantic.h — 스트림 중간 연산 및 수집 체계
### 🧩 핵심 설계: 3단계 파이프라인
```
Semantic<E> (구축 및 변환)
    ↓ toUnordered() / toOrdered() / sort() / toWindow() / toStatistics()
Collectable<E> (구체화 및 수집)
    ↓ toVector() / findFirst() / count() / summate() / ...
최종 결과

```
> ⚠️ **핵심 규칙:** 반드시 먼저 toUnordered(), toOrdered(), toWindow(), toStatistics() 또는 sort()를 통해 Semantic<E>를 Collectable<E>로 변환한 후에야 최종 메서드를 호출할 수 있습니다.
> 
### 🧭 5가지 구체화 경로
| 변환 메서드 | 대상 타입 | 기반 데이터 구조 | 성능 특징 |
|---|---|---|---|
| toUnordered() | UnorderedCollectable | unordered_map | 평균 O(1) 검색 |
| toOrdered() | OrderedCollectable | map | O(\log n) 검색 |
| sort() | OrderedCollectable | map (값 기준 정렬) | O(\log n) 검색 |
| toWindow() | WindowCollectable | 정렬 컬렉션 상속 | slide/tumble 지원 |
| toStatistics<D>() | Statistics<E,D> | 정렬 컬렉션 상속 | 30+ 통계 메서드 |
### 📋 Collectable<E> — 전체 최종 메서드 (알파벳 순)
| 메서드 | 반환 타입 | 설명 |
|---|---|---|
| allMatch(predicate) | bool | 모든 요소가 조건 만족 |
| anyMatch(predicate) | bool | 임의의 요소가 조건 만족 |
| average<D>() | D | 평균값 |
| average<D>(mapper) | D | 매핑 후 평균값 |
| collect(identity, acc, comb, fin) | R | 사용자 정의 4단계 수집 |
| collect(identity, interrupt, acc, comb, fin) | R | 사용자 정의 중단 가능 수집 |
| count() | Module | 요소 총 개수 |
| empty() | bool | 스트림이 비어있는지 여부 |
| error() | void | stderr로 출력 (delimiter/prefix/suffix/converter 지원) |
| findAny() | std::optional<E> | 임의 요소 검색 |
| findAt(index) | std::optional<E> | 지정된 인덱스 검색 (음수 인덱스 지원) |
| findFirst() | std::optional<E> | 첫 번째 요소 검색 |
| findLast() | std::optional<E> | 마지막 요소 검색 |
| findMaximum() | std::optional<E> | 최대값 검색 |
| findMaximum(comparator) | std::optional<E> | 사용자 정의 비교로 최대값 검색 |
| findMinimum() | std::optional<E> | 최소값 검색 |
| findMinimum(comparator) | std::optional<E> | 사용자 정의 비교로 최소값 검색 |
| forEach(consumer) | void | 각 요소를 순회하며 부수 효과 실행 |
| group(keyExtractor) | unordered_map<K, vector<E>> | 키 기준 그룹화 |
| groupBy(keyExtractor, valueExtractor) | unordered_map<K, vector<V>> | 키 기준 그룹화 및 값 추출 |
| join() | Charsequence | 기본 형식으로 연결 |
| join(delimiter) | Charsequence | 사용자 정의 구분자로 연결 |
| join(prefix, delimiter, suffix) | Charsequence | 완전히 사용자 정의된 형식으로 연결 |
| noneMatch(predicate) | bool | 어떤 요소도 조건을 만족하지 않음 |
| out() | Charsequence | stdout으로 출력 (delimiter/prefix/suffix/converter 지원) |
| partition(size) | vector<vector<E>> | 고정 크기로 파티션 |
| partitionBy(keyExtractor) | vector<vector<E>> | 인덱스 키 기준 파티션 |
| partitionBy(keyExtractor, valueExtractor) | vector<vector<V>> | 인덱스 키 기준 파티션 및 값 추출 |
| range<D>() | D | 수치 범위 (최대값 - 최소값) |
| range<D>(mapper) | D | 매핑 후 수치 범위 |
| reduce(accumulator) | std::optional<E> | 초기값 없는 리듀스 |
| reduce(identity, accumulator) | E | 초기값 있는 리듀스 |
| reduce(identity, acc, comb) | R | 완전히 사용자 정의된 리듀스 |
| summate<D>() | D | 합계 |
| summate<D>(mapper) | D | 매핑 후 합계 |
| toArray<N>() | std::array<E, N> | 고정 크기 array로 수집 |
| toDeque() | std::deque<E> | deque로 수집 |
| toForwardList() | std::forward_list<E> | forward_list로 수집 |
| toList() | std::list<E> | list로 수집 |
| toMap(keyExtractor) | std::map<K, E> | 키 기준 map으로 수집 |
| toMap(keyExtractor, valueExtractor) | std::map<K, V> | 사용자 정의 키-값으로 map 수집 |
| toMultimap(keyExtractor) | std::multimap<K, E> | 키 기준 multimap으로 수집 |
| toMultimap(keyExtractor, valueExtractor) | std::multimap<K, V> | 사용자 정의 키-값으로 multimap 수집 |
| toMultiset() | std::multiset<E> | multiset으로 수집 |
| toPriorityQueue() | std::priority_queue<E> | priority_queue로 수집 |
| toQueue() | std::queue<E> | queue로 수집 |
| toSet() | std::set<E> | set으로 수집 (중복 제거 및 정렬) |
| toStack() | std::stack<E> | stack로 수집 |
| toUnorderedMap(keyExtractor, valueExtractor) | std::unordered_map<K, V> | unordered_map으로 수집 |
| toUnorderedMultimap(keyExtractor) | std::unordered_multimap<K, E> | 키 기준 unordered_multimap으로 수집 |
| toUnorderedMultimap(keyExtractor, valueExtractor) | std::unordered_multimap<K, V> | 사용자 정의 키-값으로 unordered_multimap 수집 |
| toUnorderedMultiset() | std::unordered_multiset<E> | unordered_multiset으로 수집 |
| toUnorderedSet() | std::unordered_set<E> | unordered_set으로 수집 |
| toVector() | std::vector<E> | vector로 수집 |
### 📈 Statistics<E,D> — 통계 메서드
| 메서드 | 반환 타입 | 설명 |
|---|---|---|
| summate() | D | 합계 |
| average() | D | 평균값 |
| minimum() | std::optional<D> | 최소값 |
| maximum() | std::optional<D> | 최대값 |
| range() | D | 범위 |
| variance() | D | 모분산 |
| standardDeviation() | D | 모표준편차 |
| median() | std::optional<D> | 중앙값 |
| mode() | std::optional<E> | 최빈값 |
| percentile(p) | std::optional<D> | 제 p 백분위수 |
| firstQuartile() | std::optional<D> | 제1사분위수 (Q1) |
| thirdQuartile() | std::optional<D> | 제3사분위수 (Q3) |
| interquartileRange() | std::optional<D> | 사분위 범위 (IQR) |
| skewness() | D | 왜도 |
| kurtosis() | D | 첨도 |
| frequency() | map<E, complex> | 주파수 특징 |
| distribute() | map<E, complex> | 공간 분포 특징 |
| dft() | vector<complex<double>> | 이산 푸리에 변환 |
| idft() | vector<complex<double>> | 역 이산 푸리에 변환 |
| fft() | vector<complex<double>> | 고속 푸리에 변환 |
| ifft() | vector<complex<double>> | 역 고속 푸리에 변환 |
| gradient(...) | vector<double> | 경사 하강법 |
> 💡 위 메서드들은 모두 선택적 mapper 매개변수 버전을 지원합니다.
> 
### 🔧 Semantic<E> 중간 연산 메서드
| 분류 | 메서드 | 설명 |
|---|---|---|
| **요소 변환** | map | 일대일 매핑 변환 |
|  | flatMap | 일대다 매핑 및 평탄화 |
|  | flat | 중첩 스트림 평탄화 (Semantic 및 컨테이너 지원) |
| **요소 필터** | filter | 조건 필터링 |
|  | takeWhile | 조건 만족 시 지속 획득 |
|  | dropWhile | 조건 만족 시 지속 폐기 |
|  | distinct | 중복 제거 (사용자 정의 비교자 지원) |
| **수량 제어** | limit | 요소 수량 제한 |
|  | skip | 처음 n개 요소 건너뛰기 |
|  | sub | 부분 범위 추출 [start, end) |
| **인덱스 연산** | redirect | 인덱스 재매핑 |
|  | reverse | 인덱스 반전 |
|  | translate | 인덱스 오프셋 |
| **관찰 연산** | peek | 각 요소 관찰 (스트림 수정 안 함) |
| **병렬 선언** | parallel(n) | 병렬도 선언 |
| **연결 연산** | concatenate | Semantic/요소/생성자/컨테이너 연결 |
| **최종 변환** | toUnordered / toOrdered / toWindow / toStatistics / sort | Collectable로 변환 |
## 🔧 제6계층: semantics.h — 스트림 빌더 팩토리
### 🔢 숫자 범위 생성
| 메서드 | 설명 |
|---|---|
| useRange(start, end) | [start, end) 범위 생성 |
| useRange(start, end, step) | 스텝 있는 범위, 음수 스텝 지원 |
| useRangeClosed(start, end) | [start, end] 닫힌 구간 생성 |
| useRangeClosed(start, end, step) | 스텝 있는 닫힌 구간 |
### ♾️ 무한 스트림 생성
| 메서드 | 설명 |
|---|---|
| useInfinite(seed, generator) | 시드 값부터 무한 반복 생성 |
| useGenerate(supplier) | 공급자 무한 호출 |
| useGenerate(supplier, limit) | 공급자 유한 횟수 호출 |
| useIterate(seed, generator) | 시드 값부터 무한 반복 |
| useIterate(seed, generator, limit) | 유한 횟수 반복 |
| useRandom() | 무한 랜덤 정수 스트림 |
| useRandom(min, max) | 지정 범위 랜덤 수 스트림 |
| useRandom(min, max, count) | 지정 범위 및 개수 랜덤 수 스트림 |
### 📦 컨테이너 및 요소 구축
| 메서드 | 설명 |
|---|---|
| useEmpty() | 빈 스트림 생성 |
| useOf(element) | 단일 요소로 스트림 생성 |
| useOf(e1, e2) | 두 요소로 스트림 생성 |
| useOf(e1, e2, e3) | 세 요소로 스트림 생성 |
| useOf({...}) | 초기화 리스트로 스트림 생성 |
| useFrom(container) | 표준 컨테이너로 스트림 생성 |
| useFrom({...}) | 초기화 리스트로 스트림 생성 |
| useRepeat(element, count) | 요소 n회 반복 |
### 📄 텍스트 및 유니코드 처리
| 메서드 | 설명 |
|---|---|
| useBlob(text) | 문자열을 바이트 단위로 char 스트림 분할 |
| useBlob(text, start, end) | 지정 범위를 바이트 단위로 분할 |
| useBlob(istream) | 입력 스트림에서 줄 단위로 읽기 |
| useBlob(istream, delimiter) | 입력 스트림에서 구분자 기준 읽기 |
| useText(text) | 전체 텍스트 스트림 (Charsequence) |
| useText(text, delimiter) | 구분자 기준 텍스트 분할 |
| useText(istream) | 입력 스트림에서 전체 내용 읽기 |
| useSequence(charsequence) | 문자 시퀀스에서 코드 포인트 스트림 생성 |
| useSequence(text, encoding) | 텍스트에서 지정 인코딩 코드 포인트 스트림 생성 |
| useCharsequence(charsequence) | 문자 시퀀스를 전체 스트림으로 |
| useCharsequence(charsequence, delimiter) | 구분자 기준 문자 시퀀스 분할 |
## 🔐 제7계층: hash.h / less.h — 컨테이너 세계의 공용어
모든 표준 라이브러리 컨테이너(중첩 컨테이너 포함), pair, tuple, optional, variant, chrono 시간 타입, complex 복소수 등에 대해 완전한 해시 및 비교 지원을 제공합니다. 이제 임의의 깊이와 조합의 중첩 컨테이너도 unordered_set의 키나 set의 요소로 사용할 수 있습니다. 🌉
## 🚀 성능 최적화 팁
 1. **적합한 컨테이너 선택:** 순서가 중요하지 않다면 toUnordered(), 정렬이 필요하다면 toOrdered() 또는 sort()
 2. **병렬 처리 활용:** 데이터 양이 많을 때 parallel() 사용
 3. **연산 순서 최적화:** 가능한 빨리 filter하고, 신중하게 sort할 것
 4. **지연 평가 활용:** takeWhile과 limit으로 조기 종료 가능
**Semantic-Cpp** — 현대적인 C++로 효율적이고 명확한 데이터 처리 파이프라인을 구축하세요. 🚀🎯✨
