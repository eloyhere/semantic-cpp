# semantic-cpp – Eine moderne C++-Stream-Bibliothek mit temporaler Semantik

**semantic-cpp** ist eine header-only, hochperformante Stream-Verarbeitungsbibliothek für C++17, die die Eleganz von Java Streams, die Trägheit von JavaScript-Generatoren, die Ordnungslogik von MySQL-Indizes und die zeitliche Sensibilität kombiniert, die in Finanzsystemen, IoT und Event-Driven-Architekturen benötigt wird.

Was sie einzigartig macht:

- Jedes Element trägt einen **Timestamp** (vorzeichenbehafteter `long long`, darf negativ sein).  
- **Module** ist eine vorzeichenlose Ganzzahl für Zählungen und Parallelitätsgrade.  
- Streams sind vollständig lazy, bis sie mit `.toOrdered()`, `.toUnordered()`, `.toWindow()` oder `.toStatistics()` materialisiert werden.  
- Nach der Materialisierung kann die Kette beliebig weitergeführt werden – die Bibliothek ist bewusst „post-terminal“-freundlich.  
- Native Unterstützung für parallele Ausführung, Sliding-/Tumbling-Window sowie umfangreiche statistische Collector.

## Warum semantic-cpp?

| Feature                                   | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                   |
|-------------------------------------------|-------------|-------------|-----------|--------------|------------------------------------------------|
| Lazy Evaluation                           | Yes         | Yes         | Yes       | No           | Yes                                            |
| Temporale Indizes (signed timestamps   | No          | No          | No        | No           | Yes (Kernkonzept)                              |
| Sliding / Tumbling Windows                | No          | No          | No        | No           | Yes (erstklassig)                              |
| Eingebaute Statistik-Collector            | No          | No          | No        | No           | Yes (Mittelwert, Median, Modus, Kurtosis …)    |
| Parallel standardmäßig (opt-in)           | Yes         | No          | Yes       | No           | Yes (globaler Thread-Pool oder individuell)    |
| Weiterverarbeitung nach Terminal-Operation| No          | No          | No        | No           | Yes (Post-Terminal-Streams)                    |
| Header-only, C++17                        | No          | Yes         | Yes       | Yes          | Yes                                            |

Wenn du schon einmal dieselbe Window-/Statistik-Logik für Marktdaten, Sensorströme oder Log-Analysen geschrieben hast – semantic-cpp macht diesen Boilerplate überflüssig.

## Schnellstart

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // Stream mit 100 Zahlen und Timestamps 0..99
    auto stream = from(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream
        .filter([](int x) { return x % 2 == 0; })      // nur gerade Zahlen
        .parallel(8)                                   // 8 Threads
        .toWindow()                                    // Materialisierung + Window-Funktionen
        .getSlidingWindows(10, 5)                      // Fenster à 10 Elemente, Schrittweite 5
        .toVector();                                   // → std::vector<std::vector<int>>

    // Statistik-Beispiel
    auto stats = from({1,2,3,4,5,6,7,8,9,10})
                  .toStatistics<double>();         // Mapper wird automatisch als Identity abgeleitet

    std::cout << "Mittelwert   : " << stats.mean()            << '\n';
    std::cout << "Median       : " << stats.median()          << '\n';
    std::cout << "Modus        : " << stats.mode()            << '\n';
    std::cout << "Standardabw. : " << stats.standardDeviation() << '\n';
    stats.cout();  // hübsche Gesamtausgabe

    return 0;
}
```

## Kernkonzepte

### 1. `Semantic<E>` – der lazy Stream

```cpp
Semantic<int> s = of(1, 2, 3, 4, 5);
```

Alle klassischen Operationen sind verfügbar:

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

Jedes Element wird zusammen mit einem Timestamp ausgegebenen Timestamp (signed index). Zeitstempel können verschoben oder komplett umgeleitet werden:

```cpp
s.translate(+1000)                                          // alles verschieben
   .redirect([](int x, Timestamp t){ return x * 10LL; })    // eigene Logik
```

### 2. Materialisierung – der einzige Ort, an dem gezahlt wird

Vor `count()`, `toVector()`, `cout()` usw. muss einer der vier Terminal-Converter aufgerufen werden:

```cpp
.toOrdered()      // behält Reihenfolge bei, ermöglicht Sortierung
.toUnordered()    // schnellste Variante, keine Ordnungsgarantie
.toWindow()       // geordnet + mächtige Window-API
.toStatistics<D>()// geordnet + statistische Methoden
```

Danach kann beliebig weitergekettet werden:

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 3. Windowing

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // Fenstergröße 30, Schritt 10
```

Oder direkt einen Stream aus Fenstern erzeugen:

```cpp
stream.toWindow()
      .windowStream(50, 20)           // liefert pro Fenster ein std::vector<E>
      .map([](const std::vector<double>& w){ return mean(w); })
      .toOrdered()
      .cout();
```

### 4. Statistik

```cpp
auto stats = from(preise)
              .toStatistics<double>([](double p){ return p; });

std::cout << "Mittelwert      : " << stats.mean()      << "\n";
std::cout << "Median          : " << stats.median()    << "\n";
std::cout << "Standardabweichung : " << stats.standardDeviation() << "\n";
std::cout << "Schiefe         : " << stats.skewness()  << "\n";
std::cout << "Kurtosis        : " << stats.kurtosis()  << "\n";
```

Alle statistischen Funktionen sind stark gecacht (Häufigkeitstabelle wird nur einmal berechnet).

### 5. Parallelität

```cpp
globalThreadPool          // wird beim Programmstart mit hardware_concurrency Threads erzeugt
stream.parallel()         // nutzt den globalen Pool
stream.parallel(12)       // erzwingt exakt 12 Worker-Threads
```

Jeder Collectable trägt seine eigene Parallelitätsstufe – sie wird korrekt vererbt.

## Factory-Funktionen

```cpp
empty<T>()                              // leerer Stream
of(1,2,3,"hello")                       // aus variadischen Argumenten
fill(42, 1'000'000)                     // eine Million Mal 42
fill([]{return rand();}, 1'000'000)     // per Supplier
from(container)                         // vector, list, set, array, initializer_list
range(0, 100)                           // 0 … 99
range(0, 100, 5)                        // 0,5,10,…
iterate(eigener_generator)              // aus eigenem Generator
```

## Installation

Header-only. Einfach `semantic.h` ins Projekt kopieren oder per CMake einbinden:

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(dein_target PRIVATE semantic::semantic)
```

## Beispiele kompilieren

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp -o basic
./basic
```

## Benchmarks (Apple M2 Max, 2024)

| Operation                                 | Java Stream | ranges-v3 | semantic-cpp (parallel) |
|-------------------------------------------|-------------|-----------|-------------------------|
| 100 Mio. Integer → Summe                  | 280 ms      | 190 ms    | 72 ms                   |
| 10 Mio. double → gleitendes Fenster-Mittel (30, Schritt 10) | N/A | N/A | 94 ms |
| 50 Mio. int → toStatistics                | N/A         | N/A       | 165 ms                  |

## Mitwirken

Beiträge sind herzlich willkommen! Besonders gesucht:

- Weitere Collector (Perzentile, Kovarianz …)
- Bessere Interoperabilität mit anderen Range-Bibliotheken
- Optionale SIMD-Beschleunigung für einfache Mapper

Bitte CONTRIBUTING.md lesen.

## Lizenz

MIT © Eloy Kim

Viel Spaß mit echten temporal-semantischen Streams in C++!
