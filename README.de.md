# Semantic Stream — Moderne funktionale Stream-Bibliothek für C++

Semantic ist eine reine Header-Only-, hochperformante und vollständig lazy evaluierte funktionale Stream-Bibliothek für C++17 und neuer.  
Sie vereint Ideen aus JavaScript Generator, Java Stream API, dem java.util.function-Paket sowie dem Index-Design von MySQL.

Sie kombiniert die Ausdruckskraft funktionaler Programmierung mit echter Lazy-Evaluation, paralleler Ausführung und zeitbasierter Element-Indexierung – damit lassen sich sowohl endliche als auch unendliche Sequenzen elegant verarbeiten.

## Wichtigste Merkmale

- **Vollständige Lazy-Evaluation** — Zwischenschritte werden erst bei einer terminalen Operation ausgeführt.
- **Native Unterstützung für unendliche Streams** — Generatoren ermöglichen natürliche unendliche Sequenzen.
- **Zeitstempel-basierte Indexierung** — Jedes Element trägt einen impliziten oder expliziten Timestamp, wodurch MySQL-ähnliche effiziente Sprünge und Suchen möglich werden.
- **Parallele Ausführung per Einzeiler** — `.parallel()` oder `.parallel(n)` macht den gesamten Pipeline mehrthreaded.
- **Umfangreiche funktionale Operationen** — `map`, `filter`, `flatMap`, `reduce`, `collect`, `group`, Statistik u. v. m.
- **Java-ähnliche Collector-API** — supplier, accumulator, combiner, finisher.
- **Leistungsstarke Statistikfunktionen** — Mittelwert, Median, Modalwert, Varianz, Standardabweichung, Quartile, Schiefe, Kurtosis usw.
- **Viele Erzeugungsmöglichkeiten** — aus Containern, Arrays, Ranges, Generatoren oder Füllfunktionen.

## Design-Philosophie

Semantic betrachtet einen Stream als zeitlich indizierte Folge, die von einem `Generator` erzeugt wird. Kerninspirationen:

- **JavaScript Generator** — Pull-basierte, träge Wertproduktion
- **Java Stream** — Flüssige Verkettung und klare Trennung intermediär/terminal
- **java.util.function** — Typ-Aliase wie `Function`, `Consumer`, `Predicate` usw.
- **MySQL-Index** — Logische Timestamps für effiziente `skip`, `limit`, `redirect`, `translate`

## Zentrales Konzept

```cpp
using Generator<E> = BiConsumer<
    BiConsumer<E, Timestamp>,   // yield(Element, Timestamp)
    Predicate<E>                // Abbruchbedingung
>;
```

Der Generator erhält eine Yield-Funktion und eine Abbruchprüfung. Diese Low-Level-Abstraktion treibt sämtliche Stream-Quellen an – endliche wie unendliche.

## Kurze Beispiele

```cpp
using namespace semantic;

// Unendlicher Zufalls-Stream
auto s = Semantic<int>::iterate([](auto yield, auto cancel) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> d(1, 100);
    Timestamp ts = 0;
    while (!cancel(d(gen))) {
        yield(d(gen), ts++);
    }
});

// Lazy Primzahlen (begrenzter Bereich)
auto primes = Semantic<long long>::range(2LL, 1'000'000LL)
    .filter([](long long n) {
        return Semantic<long long>::range(2LL, n)
            .takeWhile([n](long long d){ return d * d <= n; })
            .noneMatch([n](long long d){ return n % d == 0; });
    });

// Parallele Wortzählung
auto wordCount = Semantic<std::string>::from(lines)
    .flatMap([](const std::string& line) {
        return Semantic<std::string>::from(split(line));
    })
    .parallel()
    .group([](const std::string& w){ return w; })
    .map([](auto& p){ return std::make_pair(p.first, p.second.size()); });
```

## Build-Anforderungen

- C++17 oder höher
- Reine Header-Only-Bibliothek — einfach `#include "semantic.hpp"`
- Keine externen Abhängigkeiten

Ein globaler Thread-Pool (`semantic::globalThreadPool`) wird automatisch mit `std::thread::hardware_concurrency()` Worker-Threads gestartet.

## Lizenz

MIT License — freie Nutzung in kommerziellen und Open-Source-Projekten erlaubt.

## Autor

Hochperformante, idiomatische Umsetzung der besten Ideen moderner funktionaler Programmierung für aktuelles C++.

Fertig! Du hast jetzt auch `README.de.md`.
