# 🚀 Semantic-Cpp: 미래 지향적인 C++ 스마트 스트림 처리 프레임워크

Semantic-Cpp는 **“다중 헤더, 외부 의존성 제로”** 모듈형 아키텍처를 채택한 현대적인 C++ 스트림 처리 라이브러리입니다. 각 헤더 파일은 명확한 단일 책임을 가지며 독립적으로 테스트할 수 있어 완전한 스트림 처리 생태계를 구성합니다. 이 라이브러리는 여러 프로그래밍 패러다임의 장점을 창의적으로 융합했습니다:

- 🎯 **Java Stream API**의 우아함과 유연성: 메서드 체이닝과 선언적 프로그래밍으로 시처럼 아름다운 코드 작성
- ⚡ **JavaScript Generator**의 지연 평가와 유연성: 필요할 때만 생성하여 메모리 효율 극대화
- 🗄️ **데이터베이스 인덱스**의 효율성과 정렬: 인덱스 기반 설계로 시계열 데이터 처리에 최적화

기존 방식(수동 루프, 비동기 콜백)과 달리 Semantic-Cpp는 **타입 안전성, 높은 표현력, 그리고 고성능**을 제공하는 솔루션을 목표로 합니다. 핵심 설계 철학은 **정밀한 데이터 흐름 제어**입니다. 데이터는 필요할 때만 흐르며, 순서와 위치는 **“인덱스”**를 통해 세밀하게 조정되어 리소스를 최적으로 활용합니다.

---

## 📐 프로젝트 아키텍처: 7계층 모듈형 설계

Semantic-Cpp는 **7개의 핵심 헤더 파일**로 구성되며, 각 계층은 독립적인 역할을 수행합니다:

```
┌─────────────────────────────────────────────────┐
│                 semantics.h                     │
│    (스트림 팩토리: 숫자 범위, 컨테이너, 텍스트, Unicode) │
├─────────────────────────────────────────────────┤
│                  semantic.h                     │
│   (스트림 중간 연산, Collectable 체계, 컨테이너 특수화)  │
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│  (컬렉터 팩토리: 매칭, 검색, 집계, 통계, DFT/FFT 등)    │
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│     (컬렉터 프레임워크: 5단계 모델, 병렬 처리 지원)      │
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│  (Unicode 문자 시퀀스, 다중 인코딩 변환, Builder, Buffer)│
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│   (글로벌 스레드 풀: 작업 제출, 긴급 종료, 예외 전파)    │
├─────────────────────────────────────────────────┤
│                 function.h                      │
│  (타입 정의: Generator, Supplier, Consumer 등 별칭)  │
└─────────────────────────────────────────────────┘
```

### 의존성 관계

```
function.h          ← 의존성 없음, 타입 시스템의 기초
pool.h              ← function.h 에 의존
charsequence.h      ← 독립 모듈, Unicode 처리
collector.h         ← function.h, pool.h 에 의존
collectors.h        ← collector.h, charsequence.h 에 의존
semantic.h          ← collector.h, collectors.h, charsequence.h 에 의존
semantics.h         ← semantic.h 에 의존
```

각 헤더는 독립적으로 컴파일 및 테스트할 수 있으며, 필요에 따라 개별 포함이 가능합니다.  
예를 들어, 컬렉터 기능만 필요하다면 `collector.h`와 `collectors.h`만 포함하면 됩니다.

---

## 🏗️ 제1계층: function.h — 타입 기반

`function.h`는 전체 프레임워크의 타입 시스템을 정의하는 공통 기반입니다:

```cpp
namespace function {
    using Timestamp = long long;           // 인덱스 타입
    using Module = unsigned long long;     // 모듈 / 카운터 타입
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept
        std::function<bool(T, Timestamp)>       // interrupt
    )>;
}
```

`Generator`는 전체 스트림 시스템의 핵심 추상화로, `accept`(데이터 수신)과 `interrupt`(흐름 중단) 두 콜백을 통해 **“지연 풀(Lazy Pull)”** 모델을 구현합니다.

---

## ⚡ 제2계층: pool.h — 동시성 기반

`pool.h`는 글로벌 스레드 풀 `pool::pool`을 제공하며 프레임워크의 동시성 엔진입니다:

| 특징 | 설명 |
|----|----|
| 🎯 선언적 병렬 처리 | `parallel(n)`은 의도만 선언하며, 터미널 연산 시 자동 활성화 |
| 🛡️ 긴급 종료 | 내장 `emergencyShutdown()` 및 `std::set_terminate` 핸들러 |
| 🔄 예외 전파 | `submit()`은 `std::future`를 반환하여 안전한 예외 전파 지원 |

---

## 🔤 제3계층: charsequence.h — Unicode 문자 시퀀스

`charsequence.h`는 Unicode 처리를 위한 완전한 모듈입니다:

| 기능 | 설명 |
|----|----|
| 🌐 다중 인코딩 지원 | UTF‑8, UTF‑16 (LE/BE), UTF‑32 (LE/BE), ASCII, Latin1 |
| 🔍 코드 포인트 이터레이터 | `PointIterator`로 양방향 Unicode 코드 포인트 탐색 |
| 🏗️ 빌더 패턴 | `Builder` 클래스로 효율적인 바이트 수준 문자열 연결 |
| 📦 버퍼 | `Buffer` 클래스로 스레드 안전한 링 버퍼 제공 |
| 🔑 해시 및 비교 | 모든 핵심 타입에 `std::hash` 및 `std::less` 특수화 존재 |

### 핵심 타입

| 타입 | 설명 |
|----|----|
| `Meta` | 부호 없는 정숫값 저장 메타데이터 래퍼 |
| `Point` | 서로게이트 페어 감지 및 유효성 검증 지원 Unicode 코드 포인트 |
| `Charsequence` | 불변 문자 시퀀스 (분할, 치환, 검색, 대소문자 변환 등 지원) |
| `Builder` | 가변 바이트 빌더 (`prepend`, `insert`, `append` 및 다양한 데이터 타입 지원) |
| `Buffer` | 스레드 안전 링 버퍼 (읽기/쓰기, 프리페치, 용량 관리 지원) |

---

## 🔧 제4계층: collector.h — 컬렉터 프레임워크

`collector.h`는 터미널 연산의 핵심 엔진인 컬렉터 패턴을 구현합니다.

### 5단계 모델

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (선택적 단락)
```

### 타입 별칭

| 타입 | 정의 | 역할 |
|----|----|----|
| `Identity<A>` | `Supplier<A>` | 초기값 제공 |
| `Accumulator<A,E>` | `TriFunction<A,E,Timestamp,A>` | 요소 누적 |
| `Combiner<A>` | `BiFunction<A,A,A>` | 병렬 결과 병합 |
| `Finisher<A,R>` | `Function<A,R>` | 최종 변환 |
| `Interrupt<E,A>` | `TriPredicate<E,Timestamp,A>` | 단락 판단 |

### 동시성 지원

`Collector::collect()`는 다음을 자동 처리합니다:

- 📦 데이터 샤딩 (인덱스 모듈로 연산 분배)
- 🔗 결과 병합 (`Combiner`를 통한 로컬 결과 통합)
- ⚠️ 예외 전파 (`std::exception_ptr` 및 `std::atomic<bool>` 사용)

---

## 🏭 제5계층: collectors.h — 컬렉터 팩토리

`collectors.h`는 풍부한 사전 정의 컬렉터 팩토리 함수를 제공합니다.

### 📊 매칭 연산

| 메서드 | 설명 | 반환 타입 |
|----|----|----|
| `useAllMatch(predicate)` | 모든 요소가 조건 충족 | `bool` |
| `useAnyMatch(predicate)` | 어떤 요소든 조건 충족 | `bool` |
| `useNoneMatch(predicate)` | 어떤 요소도 조건 충족 안 함 | `bool` |

### 🔍 검색 연산

| 메서드 | 설명 | 반환 타입 |
|----|----|----|
| `useFindFirst()` | 첫 번째 요소 찾기 | `std::optional<E>` |
| `useFindLast()` | 마지막 요소 찾기 | `std::optional<E>` |
| `useFindAny()` | 임의 요소 찾기 | `std::optional<E>` |
| `useFindAt(index)` | 지정 인덱스 요소 찾기 (음수 인덱스 지원) | `std::optional<E>` |
| `useFindMaximum()` | 최대값 찾기 (사용자 정의 비교자 지원) | `std::optional<E>` |
| `useFindMinimum()` | 최소값 찾기 (사용자 정의 비교자 지원) | `std::optional<E>` |

### 📈 집계 연산

| 메서드 | 설명 | 반환 타입 |
|----|----|----|
| `useCount()` | 요소 총 개수 | `Module` |
| `useSummate<E,D>()` | 합계 | `D` |
| `useSummate<E,D>(mapper)` | 매핑 후 합계 | `D` |
| `useAverage<E,D>()` | 평균 | `D` |
| `useAverage<E,D>(mapper)` | 매핑 후 평균 | `D` |
| `useRange<E,D>()` | 범위 (최대−최소) | `D` |
| `useRange<E,D>(mapper)` | 매핑 후 범위 | `D` |
| `useMinimum<E,D>()` | 최소값 | `std::optional<D>` |
| `useMaximum<E,D>()` | 최대값 | `std::optional<D>` |

### 📊 통계 연산

| 메서드 | 설명 | 반환 타입 |
|----|----|----|
| `useVariance<E,D>()` | 모집단 분산 | `D` |
| `useStandardDeviation<E,D>()` | 모집단 표준 편차 | `D` |
| `useSkewness<E,D>()` | 왜도 | `D` |
| `useKurtosis<E,D>()` | 첨도 | `D` |
| `useMedian<E,D>()` | 중앙값 | `std::optional<D>` |
| `useMode<E>()` | 최빈값 (주파수 영역 기반) | `std::optional<E>` |
| `usePercentile<E,D>(p)` | p번째 백분위수 | `std::optional<D>` |
| `useFrequency<E>()` | 주파수 특성 (인덱스 위상 인코딩) | `std::map<E, complex>` |
| `useDistribution<E>()` | 공간 분포 특성 (위치 인코딩) | `std::map<E, complex>` |

### 🔗 리듀스 연산

| 메서드 | 설명 | 반환 타입 |
|----|----|----|
| `useReduce(reducer)` | 초기값 없는 리듀스 | `std::optional<E>` |
| `useReduce(identity, reducer)` | 초기값 있는 리듀스 | `E` |
| `useReduce(id, red, comb, fin)` | 완전 사용자 정의 리듀스 | `R` |

### 📦 컨테이너 수집 연산

| 메서드 | 설명 | 반환 타입 |
|----|----|----|
| `useToVector()` | `std::vector`로 수집 | `std::vector<E>` |
| `useToList()` | `std::list`로 수집 | `std::list<E>` |
| `useToDeque()` | `std::deque`로 수집 | `std::deque<E>` |
| `useToForwardList()` | `std::forward_list`로 수집 | `std::forward_list<E>` |
| `useToArray<N>()` | 고정 크기 `std::array`로 수집 | `std::array<E, N>` |
| `useToSet()` | `std::set`으로 수집 (중복 제거 및 정렬) | `std::set<E>` |
| `useToMultiset()` | `std::multiset`으로 수집 | `std::multiset<E>` |
| `useToUnorderedSet()` | `std::unordered_set`으로 수집 | `std::unordered_set<E>` |
| `useToUnorderedMultiset()` | `std::unordered_multiset`으로 수집 | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)` | `std::map`으로 수집 | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)` | 사용자 정의 값으로 `std::map` 수집 | `std::map<K, V>` |
| `useToMultimap(keyExtractor)` | `std::multimap`으로 수집 | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)` | 사용자 정의 값으로 `std::multimap` 수집 | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)` | `std::unordered_map`으로 수집 | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)` | `std::unordered_multimap`으로 수집 | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)` | 사용자 정의 값으로 `std::unordered_multimap` 수집 | `std::unordered_multimap<K, V>` |
| `useToStack()` | `std::stack`으로 수집 | `std::stack<E>` |
| `useToQueue()` | `std::queue`으로 수집 | `std::queue<E>` |
| `useToPriorityQueue()` | `std::priority_queue`으로 수집 | `std::priority_queue<E>` |

### 🔀 그룹화 및 파티션 연산

| 메서드 | 설명 | 반환 타입 |
|----|----|----|
| `useGroup(keyExtractor)` | 키별 그룹화 | `std::unordered_map<K, vector<E>>` |
| `usePartition(size)` | 고정 크기 파티션 분할 | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)` | 사용자 정의 키별 파티션 분할 | `std::vector<vector<E>>` |

### 🎨 문자열 출력 연산

| 메서드 | 설명 | 반환 타입 |
|----|----|----|
| `useJoin()` | 문자열 결합 (기본: 쉼표 구분, 대괄호) | `Charsequence` |
| `useJoin(delimiter)` | 사용자 정의 구분자로 결합 | `Charsequence` |
| `useJoin(prefix, delimiter, suffix)` | 완전 사용자 정의 형식 결합 | `Charsequence` |
| `useOut()` | 표준 출력으로 형식 출력 | `Charsequence` |
| `useOut(delimiter)` | 사용자 정의 구분자로 표준 출력 | `Charsequence` |
| `useOut(prefix, delimiter, suffix)` | 완전 사용자 정의 형식으로 표준 출력 | `Charsequence` |
| `useError()` | 표준 오류로 형식 출력 | `Charsequence` |
| `useError(delimiter)` | 사용자 정의 구분자로 표준 오류 출력 | `Charsequence` |
| `useError(prefix, delimiter, suffix)` | 완전 사용자 정의 형식으로 표준 오류 출력 | `Charsequence` |

### 🧮 수학 도구

| 메서드 | 설명 | 반환 타입 |
|----|----|----|
| `useDFT()` | 이산 푸리에 변환 | `vector<complex<double>>` |
| `useIDFT()` | 역 이산 푸리에 변환 | `vector<complex<double>>` |
| `useFFT()` | 고속 푸리에 변환 (Cooley–Tukey) | `vector<complex<double>>` |
| `useIFFT()` | 역 고속 푸리에 변환 | `vector<complex<double>>` |
| `useGradient(gradFunc, lr, iter, th)` | 경사 하강법 (해석적 그래디언트) | `vector<double>` |
| `useGradient(costFunc, lr, iter, th, h)` | 경사 하강법 (수치적 그래디언트) | `vector<double>` |

---

## 🌊 제6계층: semantic.h — 스트림 중간 연산 및 수집 체계

`semantic.h`는 프레임워크의 핵심이며 `collectable`과 `semantic` 두 네임스페이스를 포함합니다.

### collectable 네임스페이스

수집 가능 객체의 상속 계층을 제공합니다:

| 클래스 | 설명 | 내부 저장소 |
|----|----|----|
| `Collectable<E>` | 추상 기본 클래스 (순수 가상 `source()`) | — |
| `OrderedCollectable<E>` | 정렬된 수집 (사용자 정의 정렬 지원) | `std::map<Timestamp, E>` |
| `UnorderedCollectable<E>` | 비정렬 수집 (O(1) 조회) | `std::unordered_map<Timestamp, E>` |
| `Statistics<E, D>` | 통계 수집 (`OrderedCollectable` 상속) | 20개 이상 통계 메서드 |
| `WindowCollectable<E>` | 윈도우 수집 (`OrderedCollectable` 상속) | `slide` / `tumble` 지원 |

#### Collectable 기본 클래스 메서드

20개 이상의 `toXxx()` 터미널 수집 메서드와 함께  
`count()`, `findFirst()`, `findAny()`, `anyMatch()`, `allMatch()`, `noneMatch()`, `reduce()`, `join()`, `out()`, `error()`, `group()`, `partition()`, `partitionBy()` 등 제공.

#### Statistics 클래스 메서드

| 메서드 | 반환 타입 | 설명 |
|----|----|----|
| `summate()` / `summate(mapper)` | `D` | 합계 |
| `average()` / `average(mapper)` | `D` | 평균 |
| `minimum()` / `minimum(mapper)` | `std::optional<D>` | 최소값 |
| `maximum()` / `maximum(mapper)` | `std::optional<D>` | 최대값 |
| `range()` / `range(mapper)` | `D` | 범위 (최대−최소) |
| `variance()` / `variance(mapper)` | `D` | 모집단 분산 |
| `standardDeviation()` / `standardDeviation(mapper)` | `D` | 모집단 표준 편차 |
| `frequency()` / `frequency(mapper)` | `std::map<*, complex>` | 주파수 특성 |
| `distribute()` / `distribute(mapper)` | `std::map<*, complex>` | 공간 분포 특성 |
| `median()` / `median(mapper)` | `std::optional<D>` | 중앙값 |
| `mode()` | `std::optional<E>` | 최빈값 |
| `percentile(p)` / `percentile(p, mapper)` | `std::optional<D>` | p번째 백분위수 |
| `firstQuartile()` / `firstQuartile(mapper)` | `std::optional<D>` | 제1사분위수 (Q1) |
| `thirdQuartile()` / `thirdQuartile(mapper)` | `std::optional<D>` | 제3사분위수 (Q3) |
| `interquartileRange()` / `interquartileRange(mapper)` | `std::optional<D>` | 사분위 범위 (Q3−Q1) |
| `skewness()` / `skewness(mapper)` | `D` | 왜도 |
| `kurtosis()` / `kurtosis(mapper)` | `D` | 첨도 |
| `dft()` | `vector<complex<double>>` | 이산 푸리에 변환 |
| `idft()` | `vector<complex<double>>` | 역 이산 푸리에 변환 |
| `fft()` | `vector<complex<double>>` | 고속 푸리에 변환 |
| `ifft()` | `vector<complex<double>>` | 역 고속 푸리에 변환 |
| `gradient(gradFunc, lr, iter, th)` | `vector<double>` | 경사 하강법 (해석적) |
| `gradient(costFunc, lr, iter, th, h)` | `vector<double>` | 경사 하강법 (수치적) |

### semantic 네임스페이스

`Semantic<E>` 템플릿 클래스와 그 특수화 체계를 제공합니다.

#### 주요 템플릿 메서드 목록

| 카테고리 | 메서드 | 설명 |
|----|----|----|
| 🎨 요소 변환 | `map` | 일대일 매핑 |
| | `flatMap` | 일대다 매핑 및 평탄화 |
| | `flat` | 중첩 스트림 평탄화 |
| 🔍 요소 필터링 | `filter` | 조건 필터 |
| | `takeWhile` | 조건 충족 시 계속 가져오기 |
| | `dropWhile` | 조건 충족 시 버리기 |
| | `distinct` | 중복 제거 (사용자 정의 비교자 지원) |
| 📏 수량 제어 | `limit` | 요소 수 제한 |
| | `skip` | 앞에서 n개 건너뛰기 |
| | `sub` | 하위 범위 추출 |
| 📐 인덱스 연산 | `redirect` | 인덱스 재매핑 |
| | `reverse` | 인덱스 반전 |
| | `translate` | 인덱스 이동 (고정 또는 동적) |
| 👀 관찰 연산 | `peek` | 요소 관찰 (변경 없음) |
| ⚡ 병렬 선언 | `parallel` | 병렬 수준 선언 |
| 🔗 연결 연산 | `concatenate` | 다른 스트림 또는 컨테이너 연결 |
| 📤 터미널 변환 | `toUnordered` | 비정렬 수집기로 변환 |
| | `toOrdered` | 정렬 수집기로 변환 |
| | `toWindow` | 윈도우 수집기로 변환 |
| | `toStatistics` | 통계 수집기로 변환 |

#### 컨테이너 특수화 완전 지원

| 특수화 | 설명 |
|----|----|
| `Semantic<std::vector<E>>` | 벡터 컨테이너 스트림 (정렬, 중복 제거 등 지원) |
| `Semantic<std::list<E>>` | 리스트 컨테이너 스트림 (정렬, 중복 제거 등 지원) |

---

## 🏭 제7계층: semantics.h — 스트림 생성 팩토리

`semantics.h`는 모든 스트림 생성 팩토리 함수를 제공합니다.

### 📐 숫자 범위 생성

| 메서드 | 설명 |
|----|----|
| `useRange(start, end)` | `[start, end)` 범위 숫자 스트림 생성 |
| `useRange(start, end, step)` | 단계 크기 포함 숫자 스트림 생성 |
| `useRangeClosed(start, end)` | `[start, end]` 닫힌 구간 숫자 스트림 생성 |
| `useRangeClosed(start, end, step)` | 단계 크기 포함 닫힌 구간 숫자 스트림 생성 |

### ♾️ 무한 스트림 생성

| 메서드 | 설명 |
|----|----|
| `useInfinite(seed, generator)` | 시드값부터 무한 반복 생성 |
| `useGenerate(supplier)` | 무한 공급자 호출 |
| `useGenerate(supplier, limit)` | 제한된 횟수 공급자 호출 |
| `useIterate(seed, generator)` | 시드값부터 무한 반복 |
| `useIterate(seed, generator, limit)` | 제한된 횟수 반복 |
| `useRandom()` | 무한 난수 정수 스트림 |
| `useRandom(min, max)` | 지정 범위 무한 난수 스트림 |
| `useRandom(min, max, count)` | 지정 범위 및 개수 난수 스트림 |

### 📦 컨테이너 및 요소 구성

| 메서드 | 설명 |
|----|----|
| `useEmpty()` | 빈 스트림 생성 |
| `useOf(element)` | 단일 요소 스트림 |
| `useOf(e1, e2)` | 두 요소 스트림 |
| `useOf(e1, e2, e3)` | 세 요소 스트림 |
| `useOf({...})` | 초기화 리스트로부터 스트림 |
| `useFrom(container)` | 모든 표준 컨테이너로부터 스트림 |
| `useFrom({...})` | 초기화 리스트로부터 스트림 |
| `useRepeat(element, count)` | 요소 n회 반복 |

### 📝 텍스트 처리

| 메서드 | 설명 |
|----|----|
| `useBlob(text)` | 문자열을 바이트 단위로 `char` 스트림 분할 |
| `useBlob(text, start, end)` | 지정 범위 문자열을 바이트 단위로 분할 |
| `useBlob(istream)` | 입력 스트림을 행 단위로 읽기 |
| `useBlob(istream, delimiter)` | 구분자로 입력 스트림 읽기 |
| `useText(text)` | 문자열 전체를 텍스트 스트림으로 취급 |
| `useText(text, delimiter)` | 구분자로 텍스트 분할 |
| `useText(istream)` | 입력 스트림 전체 내용 읽기 |
| `useText(istream, delimiter)` | 구분자로 입력 스트림 읽기 |

### 🌐 Unicode 처리

| 메서드 | 설명 |
|----|----|
| `useSequence(charsequence)` | 문자 시퀀스로부터 코드 포인트 스트림 생성 |
| `useSequence(charsequence, start, end)` | 지정 범위 문자 시퀀스로부터 코드 포인트 스트림 생성 |
| `useSequence(text, encoding)` | 텍스트로부터 지정 인코딩 코드 포인트 스트림 생성 |
| `useSequence(istream, encoding)` | 입력 스트림으로부터 지정 인코딩 코드 포인트 스트림 생성 |
| `useCharsequence(charsequence)` | 문자 시퀀스를 전체 스트림으로 취급 |
| `useCharsequence(charsequence, delimiter)` | 구분자로 문자 시퀀스 분할 |
| `useCharsequence(istream, encoding)` | 입력 스트림으로부터 문자 시퀀스 전체 읽기 |
| `useCharsequence(istream, delimiter, encoding)` | 구분자로 입력 스트림으로부터 문자 시퀀스 읽기 |

---

## 🧠 핵심 개념: 인덱스 중심의 데이터 세계

Semantic-Cpp는 데이터 처리를 **“요소”**와 그 **“논리적 위치(인덱스)”**에 대한 연산으로 추상화합니다. 이를 이해하는 것이 라이브러리 마스터의 열쇠입니다.

### 1. 📐 기본 인덱스 변환

| 메서드 | 설명 |
|----|----|
| `redirect(fn)` | 핵심 메서드: 사용자 정의 함수로 인덱스 완전 재작성 |
| `reverse()` | 모든 인덱스 논리 반전 (`redirect`로 내부 구현) |
| `translate(offset)` | 고정 오프셋 이동 |
| `translate(translator)` | 요소와 인덱스 기반 동적 오프셋 계산 |

### 2. 📊 정렬의 “절대적” 규칙

> ⚠️ **`sort()`는 모든 것을 덮어씁니다**: 호출 시 이전 모든 인덱스 연산이 무시되며, 요소는 값 기반 자연 순서 인덱스로 재할당됩니다.

- `sort()` → 즉시 `OrderedCollectable`로 물질화, 값 순 정렬
- `sort(comparator)` → 사용자 정의 비교자 정렬

### 3. ⚡ 선언적 병렬 처리

- `parallel(n)`은 의도만 선언하며 즉시 스레드 시작 안 함
- 터미널 연산(`toUnordered()`, `count()` 등) 시 실제 병렬 실행
- 스레드 풀이 자동으로 작업 분배 및 결과 병합 처리

### 4. 🎯 최종 데이터 컨테이너 선택 방법

| 변환 메서드 | 내부 구조 | 성능 특성 | 최적 사용 사례 |
|----|----|----|----|
| `sort()` | `OrderedCollectable` | 값 순으로 물질화 | 값 정렬, 페이지네이션, 시계열 |
| `toOrdered()` | `OrderedCollectable` | 현재 인덱스 순 유지 | 사용자 정의 인덱스 순서 보존 |
| `toUnordered()` | `UnorderedCollectable` | 평균 O(1), 최고 성능 | 빠른 조회, 중복 제거, 집계 |
| `toWindow()` | `WindowCollectable` | 정렬 기반 구조 | 슬라이딩/롤링 윈도우 분석 |
| `toStatistics()` | `Statistics` | 20개 이상 통계 메서드 | 종합 통계 분석 |

---

## 🚀 빠른 시작 가이드

### 설치

모든 헤더 파일을 프로젝트 디렉토리에 넣고 C++17 이상을 지원하는 컴파일러를 준비하세요:

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
#include "semantics.h"  // 모든 의존성 자동 포함
```

---

## 🎯 기본 예제: 인덱스와 정렬 체험

```cpp
auto result = semantic::useRange(0, 10)
    .map(int x { return x * x; })
    .redirect(int value, auto index -> long long {
        return index * 2;
    })
    .reverse()
    .sort()                              // 값 정렬 강제, 모든 인덱스 연산 덮어쓰기
    .toVector();

// 출력: 0 1 4 9 16 25 36 49 64 81
```

## ⚡ 병렬 처리 예제

```cpp
auto count = semantic::useRange(1, 1000)
    .parallel(4)
    .filter(int x { return x % 2 == 0; })
    .toUnordered()
    .count();

// 출력: 짝수 개수: 500
```

## 📊 통계 분석 예제

```cpp
auto stats = semantic::useRange(1, 101)
    .toStatistics<int, double>();

auto avg = stats.average();               // 평균
auto med = stats.median();                // 중앙값
auto std = stats.standardDeviation();     // 표준 편차
auto q1  = stats.firstQuartile();          // 제1사분위수
auto q3  = stats.thirdQuartile();          // 제3사분위수
auto skew = stats.skewness();              // 왜도
```

## 🔬 주파수 영역 분석 예제

```cpp
auto freq = data.toUnordered().frequency();
for (const auto& [element, z] : freq) {
    auto magnitude = std::abs(z);  // 분포 집중도
    auto phase     = std::arg(z);  // 분포 중심 위상
}
```

## 🧮 FFT 변환 예제

```cpp
auto spectrum = semantic::useRange(0, 8)
    .map(int x -> std::complex<double> {
        return {static_cast<double>(x), 0.0};
    })
    .toUnordered()
    .collect(collector::useFFT<double>());
```

---

## ⚡ 성능 최적화 권장 사항

1. 🎯 **올바른 컨테이너 선택**
   - 동등 조회, 비정렬 집계 → `toUnordered()`
   - 범위 조회, 정렬, 페이지네이션 → `toOrdered()` 또는 `sort()`
   - 실시간 윈도우 분석 → `toWindow()`
2. ⚡ **병렬 처리 현명하게 활용**: 대용량 데이터나 무거운 연산 시 `parallel()` 사용, 블로킹 I/O 피하기
3. 📐 **연산 순서 최적화**: 조기에 `filter`, 신중하게 `sort`
4. 🔄 **지연 평가 활용**: 중간 연산은 터미널 시 실행, `takeWhile`과 `limit`으로 조기 종료 가능

---

## 📊 C++ 표준 라이브러리 및 경쟁 제품과의 비교

| 특성 | Semantic-Cpp | C++20/23 Ranges | 전통적 루프 |
|----|----|----|----|
| 🎯 핵심 패러다임 | 선언적, 인덱스 중심 | 뷰 중심, 함수형 조합 | 명령형, 절차적 |
| ⚡ 병렬 지원 | 선언적, 자동 스레드 풀 | 병렬 알고리즘 조합 필요 | 수동 구현 |
| 📐 정렬 및 인덱싱 | 인덱스 세밀 제어, 음수 인덱스 지원 | 파괴적 정렬 | 완전 수동 |
| 📊 통계 분석 | 20개 이상 내장 통계 메서드 | 미지원 | 서드파티 필요 |
| 🔬 주파수 분석 | 네이티브 DFT / FFT / 주파수 특성 | 미지원 | 서드파티 필요 |
| 🧮 경사 하강법 | 해석적 + 수치적 듀얼 모드 | 미지원 | 서드파티 필요 |
| 🌐 Unicode | 네이티브 다중 인코딩 지원 (UTF‑8/16/32 등) | 미지원 | 수동 처리 |
| 📦 컨테이너 수집 | 20개 이상 표준 컨테이너 완전 지원 | 부분 지원 | 수동 구현 |
| 📦 의존성 | 외부 의존성 제로, 7개 헤더 | 표준 라이브러리 | 없음 |

---

## 📜 라이선스

- 📄 **라이선스**: MIT

---

**Semantic-Cpp — 현대적인 C++로 효율적이고 명확한 데이터 처리 파이프라인을 구축하세요. 🚀**
