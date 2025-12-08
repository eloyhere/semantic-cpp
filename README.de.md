# semantic-cpp – Moderne C++ Semantic-Stream-Bibliothek

**semantic-cpp** ist eine header-only, C++17-kompatible, hochperformante und vollständig lazy auswertende Stream-Bibliothek.  
Ihr Kernprinzip: **Jedes Element trägt von Geburt an einen logischen Index (Timestamp)** – alle Operationen, die die Reihenfolge betreffen, sind nichts anderes als Umformungen dieses Index.

- Echte unendliche Streams  
- Der Index ist Bürger erster Klasse und kann beliebig umgemappt werden  
- Parallelisierung ist Standard  
- Ob Index-Operationen (redirect usw.) wirksam werden, entscheidet allein die Sammelphase  

MIT-Lizenz – uneingeschränkt für kommerzielle und Open-Source-Projekte nutzbar.

## Design-Philosophie

> **Der Index bestimmt die Reihenfolge – nicht die Ausführung.**

- `redirect`, `reverse`, `shuffle`, `cycle` usw. verändern ausschließlich den logischen Index  
- Nur bei `.toOrdered()` werden diese Index-Operationen respektiert → Ergebnis exakt nach dem finalen Index sortiert  
- Bei `.toUnordered()` werden alle Index-Operationen ignoriert → maximale Geschwindigkeit, keine Ordnung

```text
.toOrdered()   → alle Index-Operationen wirksam (redirect etc. haben Effekt)
.toUnordered() → alle Index-Operationen werden ignoriert (schnellster Pfad)
```

## Schnellstart

```cpp
#include "semantic.h"
using namespace semantic;

int main() {
    // 1. Index-Operationen wirken (gewünschte Reihenfolge)
    auto v1 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })   // umkehren
        .toOrdered()                                  // wichtig!
        .toVector();   // Ergebnis: [99, 98, …, 0]

    // 2. Index-Operationen werden ignoriert (maximale Performance)
    auto v2 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })
        .toUnordered()                                // Index wird ignoriert
        .toVector();   // Ergebnis: ungeordnet, aber blitzschnell

    // 3. Zyklischer unendlicher Stream + geordnete Sammlung
    auto top10 = Semantic<int>::range(0, 1'000'000'000)
        .redirect([](auto&, auto i) { return i % 1000; }) // 0-999 zyklisch
        .toOrdered()
        .limit(10)
        .toVector();   // [0,1,2,3,4,5,6,7,8,9]

    return 0;
}
```

## Alle wichtigen Methoden mit Kettenbeispielen

```cpp
// Stream-Erzeugung
Semantic<int>::range(0, 1000);
Semantic<long long>::range(0, 1LL<<60);
Semantic<int>::iterate([](auto yield, auto) {      // unendlicher Stream
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
stream.distinct();                                     // Wert-Entduplizierung
stream.skip(100);
stream.limit(50);
stream.takeWhile([](int x) { return x < 1000; });
stream.dropWhile([](int x) { return x < 100; });
stream.peek([](int x) { std::cout << x << ' '; });

// Index-Transformationen (nur bei toOrdered() wirksam)
stream.redirect([](auto&, auto i) { return -i; });           // umkehren
stream.redirect([](auto&, auto i) { return i % 100; });      // zyklisch
stream.redirect([](auto e, auto i) { return std::hash<int>{}(e); }); // mischen
stream.reverse();                                            // gleichwertig redirect(-i)
stream.shuffle();

// Parallelisierung
stream.parallel();       // alle Kerne
stream.parallel(8);      // feste Thread-Anzahl

// Pflicht: explizite Wahl, ob Index respektiert wird
auto ordered   = stream.toOrdered();     // alle redirect-Operationen wirksam
auto unordered = stream.toUnordered();   // alle redirect-Operationen ignoriert (schnellster Pfad)

// Geordnete Terminaloperationen (Index bestimmt die Reihenfolge)
ordered.toVector();
ordered.toList();
ordered.toSet();                 // nach finalem Index entduplizieren
ordered.forEach([](int x) { std::cout << x << ' '; });
ordered.cout();                  // [99, 98, 97, …]

// Ungeordnete Terminaloperationen (maximale Geschwindigkeit)
unordered.toVector();
unordered.toList();
unordered.toSet();
unordered.forEach(...);
unordered.cout();

// Statistik (immer auf dem schnellsten Pfad)
auto stats = stream.toUnordered().toStatistics();

// Reduce (empfohlen mit unordered)
int sum = stream.toUnordered()
    .reduce(0, [](int a, int b) { return a + b; });
```

## Methodenübersicht

| Methode                         | Beschreibung                                      | Respektiert Index-Operationen? |
|---------------------------------|---------------------------------------------------|--------------------------------|
| `toOrdered()`                   | „Semantischer Modus“ – alle Index-Operationen wirksam | Ja                             |
| `toUnordered()`                 | „Performance-Modus“ – alle Index-Operationen ignoriert | Nein (schnellster Pfad)        |
| `toVector()` / `toList()`       | In Container sammeln                             | je nach Wahl                   |
| `toSet()`                       | In Set sammeln (Entduplizierung)                 | je nach Wahl                   |
| `forEach` / `cout`              | Traversieren bzw. ausgeben                       | je nach Wahl                   |
| `redirect` / `reverse` / `shuffle` | Logischen Index verändern                   | nur bei toOrdered()            |
| `parallel`                      | Parallele Ausführung                             | in beiden Modi                 |

## Warum muss man explizit toOrdered / toUnordered wählen?

Weil **Index-Transformation** und **parallele Ausführung** zwei völlig unabhängige Dimensionen sind:

- `redirect` etc. sind semantische Operationen (wo soll das Element logisch stehen?)  
- `parallel` ist reine Ausführungsstrategie (wie schnell soll gerechnet werden?)

Nur durch die explizite Wahl beim Sammeln erhält man sowohl höchste Performance als auch präzise Kontrolle über die Reihenfolge.

## Build-Anforderungen

- C++17 oder höher  
- Nur `#include "semantic.h"`  
- Keine externen Abhängigkeiten  
- Einzelne Header-Datei

```bash
g++ -std=c++17 -O3 -pthread semantic.cpp
```

## Lizenz

MIT-Lizenz – darf uneingeschränkt in kommerziellen, Open-Source- und privaten Projekten verwendet werden.

---

**semantic-cpp**:  
**Der Index bestimmt die Reihenfolge – die Ausführung nur die Geschwindigkeit.**  
Schreibe `redirect` und es kehrt wirklich um. Schreibe `toUnordered()` und es ist wirklich am schnellsten.  
Kein Kompromiss – nur klare Entscheidungen.

Schreibe Streams, ohne raten zu müssen.  
Sag einfach: **„Ich will Semantik – oder ich will pure Geschwindigkeit.“**
