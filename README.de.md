# semantic-cpp – Eine moderne C++-Stream-Bibliothek mit temporaler Semantik

***
**Geleitet durch einige Rückmeldungen**  
***

**semantic-cpp** ist eine leichte, hochperformante Stream-Verarbeitungsbibliothek für C++17, bestehend aus einer Header-Datei und einer separaten Implementierungsdatei. Sie verbindet die Flüssigkeit von Java Streams, die Trägheit von JavaScript-Generatoren, die Ordnungslogik von Datenbankindizes und die zeitliche Sensibilität, die für Finanzanwendungen, IoT und ereignisgesteuerte Systeme erforderlich ist.

Einzigartige Kernideen:

- Jedes Element trägt einen **Timestamp** (ein vorzeichenbehafteter `long long`-Index, der negativ sein kann).
- **Module** ist ein vorzeichenloser `long long` für Zählungen und Parallelitätsstufen.
- Streams bleiben träge, bis sie mit `.toOrdered()`, `.toUnordered()`, `.toWindow()` oder `.toStatistics()` materialisiert werden.
- Nach der Materialisierung kann die Kette fortgesetzt werden – die Bibliothek unterstützt bewusst „post-terminale“ Streams.
- Vollständige Unterstützung für parallele Ausführung, gleitende und rollende Fenster sowie umfangreiche statistische Operationen.

## Warum semantic-cpp?

| Feature                              | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                   |
|--------------------------------------|-------------|-------------|-----------|--------------|------------------------------------------------|
| Trägheit (Lazy Evaluation)           | Ja          | Ja          | Ja        | Nein         | Ja                                             |
| Temporale Indizes (signed Timestamps) | Nein        | Nein        | Nein      | Nein         | Ja (Kernkonzept)                               |
| Gleitende/rollende Fenster           | Nein        | Nein        | Nein      | Nein         | Ja (erstklassige Unterstützung)                |
| Eingebaute Statistikfunktionen       | Nein        | Nein        | Nein      | Nein         | Ja (Mittelwert, Median, Modus, Kurtosis …)     |
| Parallele Ausführung (opt-in)        | Ja          | Nein        | Ja        | Nein         | Ja (globaler Thread-Pool oder benutzerdefiniert)|
| Weiterverkettung nach Terminal-Op    | Nein        | Nein        | Nein      | Nein         | Ja (post-terminale Streams)                    |
| Einzelne Header + Impl.-Datei, C++17 | Nein        | Ja          | Ja        | Ja           | Ja                                             |

Wenn Sie häufig eigene Fenster- oder Statistik-Code für Zeitreihen, Marktdaten, Sensorströme oder Log-Analysen schreiben müssen – semantic-cpp beseitigt diesen Boilerplate.

## Schnellstart

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // Erzeugt einen Stream mit 100 Zahlen und Timestamps 0..99
    auto stream = Generative<int>{}.of(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream.filter([](int x) { return x % 2 == 0; })   // nur gerade Zahlen
          .parallel(8)                               // 8 Threads nutzen
          .toWindow()                                // materialisieren mit Fensterunterstützung
          .getSlidingWindows(10, 5)                  // Fenstergröße 10, Schritt 5
          .toVector();

    // Statistik-Beispiel
    auto stats = Generative<int>{}.of(1,2,3,4,5,6,7,8,9,10).toStatistics();
    std::cout << stats.mean()   << "\n";   // 5.5
    std::cout << stats.median() << "\n";   // 5.5
    std::cout << stats.mode()   << "\n";   // bei Multimodalität der erste gefundene
    stats.cout();

    return 0;
}
```

## Kernkonzepte

### 1. Generative<E> – Stream-Fabrik

`Generative<E>` bietet eine komfortable Schnittstelle zur Stream-Erstellung:

```cpp
Generative<int> gen;
auto s = gen.of(1, 2, 3, 4, 5);
auto empty = gen.empty();
auto filled = gen.fill(42, 1'000'000);
auto ranged = gen.range(0, 100, 5);
```

Alle Fabrikfunktionen liefern einen trägen `Semantic<E>`-Stream.

### 2. Semantic<E> – Der träge Stream

```cpp
Semantic<int> s = of<int>(1, 2, 3, 4, 5);
```

Klassische Operationen stehen zur Verfügung:

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

Elemente tragen Timestamps, die manipuliert werden können:

```cpp
s.translate(+1000)                                      // alle Timestamps verschieben
 .redirect([](int x, Timestamp t){ return x * 10; })   // benutzerdefinierte Timestamp-Logik
```

### 3. Materialisierung

Vor dem Aufruf von Sammeloperationen (count(), toVector(), cout() usw.) muss einer der vier Terminal-Konverter aufgerufen werden:

```cpp
.toOrdered()      // behält Reihenfolge bei, ermöglicht Sortierung
.toUnordered()    // schnellste Variante, keine Reihenfolgegarantie
.toWindow()       // geordnet + vollständige Fenster-API
.toStatistics<D>  // geordnet + statistische Methoden
```

Nach der Materialisierung kann die Kette fortgesetzt werden:

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 4. Fensteroperationen

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // Größe 30, Schritt 10
```

Oder einen Stream aus Fenstern erzeugen:

```cpp
stream.toWindow()
    .windowStream(50, 20)           // gibt std::vector<E> für jedes Fenster aus
    .map([](const std::vector<double>& w) { return mean(w); })
    .toOrdered()
    .cout();
```

### 5. Statistik

```cpp
auto stats = from(prices).toStatistics<double>([](auto p){ return p; });

std::cout << "Mittelwert : " << stats.mean()      << "\n";
std::cout << "Median     : " << stats.median()    << "\n";
std::cout << "StdAbw.    : " << stats.standardDeviation() << "\n";
std::cout << "Schiefe    : " << stats.skewness()  << "\n";
std::cout << "Kurtosis   : " << stats.kurtosis()  << "\n";
```

Alle statistischen Funktionen sind stark gecacht (Häufigkeitsmap wird nur einmal berechnet).

### 6. Parallelität

```cpp
globalThreadPool   // wird automatisch mit hardware_concurrency Threads erzeugt
stream.parallel()  // nutzt globalen Pool
stream.parallel(12) // erzwingt genau 12 Worker-Threads
```

Die Parallelitätsstufe wird korrekt durch die gesamte Kette vererbt.

## Fabrikfunktionen (über Generative)

```cpp
empty<T>()                              // leerer Stream
of(1,2,3,"hello")                       // variadische Argumente
fill(42, 1'000'000)                     // wiederholter Wert
fill([]{return rand();}, 1'000'000)     // mit Supplier erzeugt
from(container)                         // vector, list, set, array, initializer_list, queue
range(0, 100)                           // 0 .. 99
range(0, 100, 5)                        // 0,5,10,…
iterate(generator)                      // benutzerdefinierter Generator
```

## Installation

semantic-cpp besteht aus einer Header- und einer Implementierungsdatei. Kopieren Sie einfach `semantic.h` und `semantic.cpp` in Ihr Projekt oder integrieren Sie es via CMake:

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## Beispiele kompilieren

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## Benchmarks (Apple M2 Max, 2024)

| Operation                         | Java Stream | ranges-v3 | semantic-cpp (parallel) |
|-----------------------------------|-------------|-----------|-------------------------|
| 100 Mio. Integer → Summe          | 280 ms      | 190 ms    | 72 ms                   |
| 10 Mio. Double → gleitender Mittelwert | N/A    | N/A       | 94 ms (Fenster 30, Schritt 10) |
| 50 Mio. Int → toStatistics        | N/A         | N/A       | 165 ms                  |

## Mitwirken

Beiträge sind herzlich willkommen! Bereiche, die Aufmerksamkeit benötigen:

- Weitere Collector (Perzentile, Kovarianz usw.)
- Bessere Integration mit bestehenden Range-Bibliotheken
- Optionale SIMD-Beschleunigung für einfache Mapper

Bitte lesen Sie CONTRIBUTING.md.

## Lizenz

MIT © Eloy Kim

Viel Spaß mit wirklich semantischen Streams in C++!
