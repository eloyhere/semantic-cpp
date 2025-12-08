# semantic-cpp – Moderne C++ Semantic Stream Bibliothek

**semantic-cpp** ist eine header-only, hochperformante, lazy auswertende Stream-Bibliothek für C++17 und neuer. Sie vereint die besten Ideen aus JavaScript-Generatoren, Java Stream API, Kotlin Flow und Datenbank-indiziertem Zugriff – und führt sie erstmals konsequent in C++ zusammen.

- Echte **unendliche Streams**  
- Jedes Element trägt einen eigenen **logischen Index (Timestamp)**  
- **Parallelisierung ist Standard**  
- Reihenfolge muss **explizit** angefordert werden  
- Der Compiler **zwingt** dich, dich zwischen Geschwindigkeit und Ordnung zu entscheiden – keine versteckten Kosten  

MIT-Lizenz – uneingeschränkt für kommerzielle und Open-Source-Projekte nutzbar.

## Design-Philosophie

> Parallel ist kostenlos. Ordnung kostet eine einmalige Sortierung am Ende.

- Ohne `.ordered()` → schnellster ungeordneter Pfad (empfohlen für 90 % der Fälle)  
- Mit `.ordered()` → Ergebnis exakt in ursprünglicher Reihenfolge (Logs, Protokolle, Debugging)

Keine Grauzone, keine „vielleicht geordnet“. Der Compiler erzwingt deine Entscheidung.

## Schnellstart

```cpp
#include "semantic.h"
using namespace semantic;

int main() {
    // 1. Maximal schnell, ungeordnet (empfohlen)
    auto schnell = Semantic<int>::range(0, 1'000'000)
        .parallel(16)
        .map([](int x) { return x * x; })
        .toUnordered()          // muss explizit stehen
        .toVector();

    // 2. Streng geordnet (muss explizit angefordert werden)
    auto geordnet = Semantic<int>::range(0, 1'000'000)
        .parallel(16)
        .redirect([](auto&, auto i) { return -i; })  // Index komplett durcheinander
        .toOrdered()                                // explizit geordnet
        .toVector();        // Ergebnis trotzdem 0,1,2,…999999

    // 3. Geordnete Entduplizierung (Killer-Feature)
    auto einzigartig = Semantic<int>::of(3,1,4,1,5,9,2,6,5)
        .parallel(8)
        .toOrdered()
        .toSet();           // {3,1,4,5,9,2,6} – exakt in Erstauftretens-Reihenfolge
}
```

## Alle wichtigen Methoden mit Kettenbeispielen

```cpp
// Stream-Erzeugung
Semantic<int>::range(0, 100);
Semantic<long long>::range(0LL, 1LL<<60);           // riesige Bereiche
Semantic<int>::iterate([](auto yield, auto) {       // unendlicher Stream
    for (int i = 0;; ++i) yield(i);
});
Semantic<int>::of(1, 2, 3, 4, 5);
Semantic<int>::from(std::vector{1,2,3,4});
Semantic<int>::fill(42, 1'000'000);
Semantic<int>::fill([] { return rand(); }, 1000);

// Zwischenschritte (vollständig lazy)
stream.map([](int x) { return x * x; });
stream.filter([](int x) { return x % 2 == 0; });
stream.flatMap([](int x) { return Semantic<int>::range(0, x); });
stream.distinct();                     // Entduplizierung nach Wert
stream.skip(100);
stream.limit(50);
stream.takeWhile([](int x) { return x < 1000; });
stream.dropWhile([](int x) { return x < 100; });
stream.peek([](int x) { std::cout << x << ' '; });
stream.redirect([](auto&, auto i) { return -i; });        // umkehren
stream.redirect([](auto&, auto i) { return i % 10; });    // zyklisch
stream.parallel();                                        // alle Kerne
stream.parallel(8);                                       // feste Thread-Anzahl

// Pflicht: Explizite Wahl der Sammelstrategie
auto collector = stream.toOrdered();     // ich will Ordnung
auto schnell   = stream.toUnordered();   // ich will Geschwindigkeit

// Geordnete Terminaloperationen
collector.toVector();
collector.toList();
collector.toSet();                       // geordnet entduplizieren
collector.forEach([](int x) { std::cout << x << ' '; });
collector.cout();                        // direkte Ausgabe [1, 2, 3, …]

// Ungeordnete (schnelle) Terminaloperationen
schnell.toVector();
schnell.toList();
schnell.toSet();
schnell.forEach(...);
schnell.cout();

// Statistik (immer schnell, ungeordnet)
auto stat = stream.toUnordered().toStatistics();
std::cout << "Mittelwert = " << stat.mean([](int x) { return x; });

// Reduce-Beispiele
int summe = stream.toUnordered()
    .reduce(0, [](int a, int b) { return a + b; });

std::optional<int> erstePrim = stream
    .filter(istPrim)
    .toOrdered()
    .findFirst();
```

## Übersicht der wichtigsten Methoden

| Methode                | Beschreibung                              | Geordnet?      |
|------------------------|-------------------------------------------|----------------|
| `toOrdered()`          | Wechsel in den geordneten Modus (Pflicht) | Ja             |
| `toUnordered()`        | Wechsel in den schnellsten Modus          | Nein (Standard)|
| `toVector()` / `toList()` | In Container sammeln                   | je nach Wahl   |
| `toSet()`              | In Set sammeln (Entduplizierung)          | je nach Wahl   |
| `forEach(...)`         | Elementweise verarbeiten                  | je nach Wahl   |
| `cout()`               | Direkt ausgeben `[1, 2, 3, …]`            | je nach Wahl   |
| `reduce(...)`          | Reduktion                                 | ungeordnet schneller |
| `collect(...)`         | Eigener Collector                         | je nach Wahl   |
| `toStatistics()`      | Umfangreiche Statistik                    | ungeordnet schneller |

## Warum muss man explizit ordered / unordered wählen?

Weil Ordnung nach paralleler Verarbeitung nie kostenlos ist.  
Wir lehnen versteckte Performance-Fallen und vage Zusagen („vielleicht geordnet“) ab.

Du schreibst, was du willst – und bekommst exakt das.  
Keine Überraschungen, nur Geschwindigkeit und Determinismus.

## Build-Anforderungen

- C++17 oder höher  
- Nur `#include "semantic.h"` nötig  
- Keine externen Abhängigkeiten  
- Einzelne Header-Datei

```bash
g++ -std=c++17 -O3 -pthread semantic.cpp
```

## Lizenz

MIT-Lizenz – darf uneingeschränkt in kommerziellen, Open-Source- und privaten Projekten verwendet werden, auch ohne Quellenangabe.

---

**semantic-cpp**:  
**Parallel ist Standard, Ordnung ist deine Entscheidung, Index ist Bürger erster Klasse.**

Schreibe Streams, ohne raten zu müssen.  
Schreibe einfach nur klar: **„Ich will schnell – oder ich will Ordnung.“**
