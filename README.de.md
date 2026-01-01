# semantic-cpp – Eine moderne C++-Stream-Bibliothek mit temporaler Semantik

**semantic-cpp** ist eine leichte und hochperformante Stream-Verarbeitungsbibliothek für modernes C++17. Sie besteht aus einer einzelnen Header-Datei (`semantic.h`) und einer separaten Implementierungsdatei (`semantic.cpp`). Die Bibliothek vereint die Flüssigkeit von Java Streams, die träge Auswertung von JavaScript-Generatoren und Promises, die Ordnungsfunktionen von Datenbankindizes sowie eine eingebaute temporale Semantik, die für Finanzanwendungen, IoT-Datenverarbeitung und ereignisgesteuerte Systeme unverzichtbar ist.

## Hauptmerkmale

- Jedes Element ist mit einem **Timestamp** (ein vorzeichenbehaftetes `long long`, das negative Werte unterstützt) und einem **Module** (ein vorzeichenloses `long long` für Zählungen und Parallelität) verknüpft.
- Streams werden träge ausgewertet, bis sie durch `.toOrdered()`, `.toUnordered()`, `.toWindow()` oder `.toStatistics()` materialisiert werden.
- Die Materialisierung **beendet** die Pipeline **nicht** – anschließende verkettete Aufrufe sind weiterhin vollständig möglich („Post-Terminal“-Streams).
- Umfassende Unterstützung für parallele Ausführung, gleitende und tumbling Fenster, fortgeschrittene statistische Operationen sowie asynchrone Aufgaben über eine JavaScript-inspirierte **Promise**-Klasse.

## Warum semantic-cpp?

| Merkmal                                  | Java Stream | ranges-v3 | semantic-cpp                              |
|------------------------------------------|-------------|-----------|-------------------------------------------|
| Trägheit (Lazy Evaluation)               | Ja          | Ja        | Ja                                        |
| Temporale Indizes (vorzeichenbehaftete Timestamps) | Nein        | Nein      | Ja (Kernkonzept)                          |
| Gleitende / Tumbling-Fenster              | Nein        | Nein      | Ja (nativ)                                |
| Eingebaute statistische Operationen      | Nein        | Nein      | Ja (Mittelwert, Median, Modus, Schiefe, Kurtosis usw.) |
| Parallele Ausführung (opt-in)            | Ja          | Ja        | Ja (Thread-Pool + Promise-Unterstützung)  |
| Verkettung nach Terminal-Operation       | Nein        | Nein      | Ja (Post-Terminal-Streams)                |
| Asynchrone Promises                      | Nein        | Nein      | Ja (JavaScript-Stil)                      |
| Einzelne Header + Implementierungsdatei, C++17 | Nein        | Ja        | Ja                                        |

Wenn Sie häufig eigenen Code für Fenster, Statistiken oder asynchrone Verarbeitung von Zeitreihendaten, Marktdaten-Feeds, Sensor-Streams oder Logs schreiben, eliminiert semantic-cpp diesen Boilerplate-Code.

## Schnellstart

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // Stream aus Werten erzeugen (Timestamps werden ab 0 sequenziell vergeben)
    auto stream = of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    // Gerade Zahlen filtern, parallel ausführen, für Statistiken materialisieren
    auto stats = stream
        .filter([](int x) { return x % 2 == 0; })
        .parallel(8)
        .toStatistics<int>();

    std::cout << "Mittelwert: " << stats.mean() << '\n';      // 5
    std::cout << "Median:     " << stats.median() << '\n';    // 5
    std::cout << "Modus:      " << stats.mode() << '\n';      // beliebige gerade Zahl

    // Fenster-Beispiel
    auto windows = of(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
        .toWindow()
        .getSlidingWindows(5, 2);  // Fenster der Größe 5, Schrittweite 2

    return 0;
}
```

## Kernkonzepte

### 1. Fabrikfunktionen – Streams erzeugen

Streams werden über freie Funktionen im Namensraum `semantic` erstellt:

```cpp
auto s = of(1, 2, 3, 4, 5);                          // variadische Argumente
auto empty = empty<int>();                          // leerer Stream
auto filled = fill(42, 1'000'000);                   // wiederholter Wert
auto supplied = fill([]{ return rand(); }, 1'000'000);
auto ranged = range(0, 100);                        // 0 .. 99
auto ranged_step = range(0, 100, 5);                // 0, 5, 10, ...
auto from_vec = from(std::vector<int>{1, 2, 3});
auto iterated = iterate([](auto push, auto stop) {
    for (int i = 0; i < 100; ++i) push(i, i);       // explizite Timestamps
});
```

Zusätzliche I/O-Hilfsfunktionen: `lines(stream)`, `chunks(stream, size)`, `text(stream)` usw.

### 2. Semantic – Der träge Stream

Der zentrale Typ `Semantic<E>` unterstützt Standardoperationen:

```cpp
stream.filter(...).map(...).skip(10).limit(100).peek(...)
```

Timestamp-Manipulation:

```cpp
stream.translate(+1000)          // alle Timestamps verschieben
    .redirect([](auto elem, Timestamp t) { return t * 10; });
```

Parallelität:

```cpp
stream.parallel()                // Standard-Thread-Anzahl
    .parallel(12);               // Worker-Threads angeben
```

### 3. Materialisierung

In sammelbare Form umwandeln:

- `.toOrdered()` – Ordnung erhalten, Sortierung möglich
- `.toUnordered()` – schnellste Variante, ungeordnet
- `.toWindow()` – geordnet mit vollständiger Fensterunterstützung
- `.toStatistics<R>(mapper)` – geordnet mit statistischen Methoden

Verkettung bleibt danach möglich:

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 4. Fensterung

```cpp
auto windows = stream.toWindow().getSlidingWindows(30, 10);
auto tumbling = stream.toWindow().getTumblingWindows(50);
```

Fenster-Streams erzeugen:

```cpp
stream.toWindow()
    .windowStream(50, 20)
    .map([](const std::vector<double>& w) { return mean(w); })
    .cout();
```

### 5. Statistiken

```cpp
auto stats = from(prices)
    .toStatistics<double>([](auto p) { return p; });

std::cout << "Mittelwert:       " << stats.mean() << '\n';
std::cout << "Median:          " << stats.median() << '\n';
std::cout << "Standardabweichung: " << stats.standardDeviation() << '\n';
std::cout << "Schiefe:         " << stats.skewness() << '\n';
std::cout << "Kurtosis:        " << stats.kurtosis() << '\n';
```

Ergebnisse werden aggressiv gecacht, um die Performance zu steigern.

### 6. Asynchrone Ausführung mit Promises

Inspiriert von JavaScript-Promises stellt die Klasse `Promise<T, E>` asynchrone Aufgaben bereit, die an einen `ThreadPool` übergeben werden:

```cpp
ThreadPool pool(8);

auto promise = pool.submit<int>([] {
    // rechenintensive Aufgabe
    return 42;
});

promise.then([](int result) {
        std::cout << "Ergebnis: " << result << '\n';
    })
    .except([](const std::exception& e) {
        std::cerr << "Fehler: " << e.what() << '\n';
    })
    .finally([] {
        std::cout << "Fertig\n";
    });

promise.wait();  // bei Bedarf blockieren
```

Statische Hilfsfunktionen: `Promise<T>::all(...)`, `Promise<T>::any(...)`, `resolved(value)`, `rejected(error)`.

## Installation

Kopieren Sie `semantic.h` und `semantic.cpp` in Ihr Projekt oder verwenden Sie CMake:

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## Beispiele kompilieren

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## Benchmarks (Apple M2 Max, 2024)

| Operation                                | Java Stream | ranges-v3 | semantic-cpp (parallel) |
|------------------------------------------|-------------|-----------|-------------------------|
| 100 Mio. Ganzzahlen → Summe              | 280 ms      | 190 ms    | **72 ms**               |
| 10 Mio. Doubles → gleitender Fenster-Mittelwert | N/A         | N/A       | **94 ms** (Fenster 30, Schritt 10) |
| 50 Mio. Ganzzahlen → toStatistics        | N/A         | N/A       | **165 ms**              |

## Beiträge

Beiträge sind herzlich willkommen! Bereiche, die Aufmerksamkeit benötigen:

- Zusätzliche Collector (Perzentile, Kovarianz usw.)
- Bessere Integration mit Range-Bibliotheken
- Optionale SIMD-Beschleunigung

## Lizenz

MIT © Eloy Kim

Viel Spaß mit wirklich semantischen Streams in C++!