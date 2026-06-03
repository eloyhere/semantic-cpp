# 🚀 Semantic-Cpp: Ein zukunftsorientiertes C++-Intelligentes Stream-Verarbeitungsframework

Semantic-Cpp ist eine komplett neu gestaltete, moderne C++-Stream-Verarbeitungsbibliothek, die auf einer **"Multi-Header, keine externen Abhängigkeiten"** modularen Architektur aufbaut. Jede Header-Datei hat klare Verantwortlichkeiten, ist unabhängig testbar und gemeinsam bilden sie ein vollständiges Stream-Verarbeitungs-Ökosystem. Diese Bibliothek integriert kreativ das Wesen mehrerer Programmierparadigmen:

*   **Eleganz und Flüssigkeit der Java Stream API:** Verkettete Aufrufe, deklarative Programmierung, macht Code elegant wie Poesie ✨
*   **Faulheit und Flexibilität von JavaScript Generatoren:** Faule Auswertung, bedarfsgerechte Erzeugung, speicherfreundlich 🌱
*   **Effizienz und Ordnung von Datenbank-Indizes:** Intelligente Sortierung, indexgetrieben, ein leistungsstarkes Werkzeug für Zeitreihendatenverarbeitung ⏱️
*   **Batch-Verarbeitungsphilosophie von Container-als-Element:** Vektoren, Listen, Maps... Jeder Container kann ein First-Class-Citizen im Stream sein, frei fließend 📦

Sind Sie es leid, manuell `for`-Schleifen zu schreiben, um einen `vector` zu durchlaufen, ein `if` zu verschachteln, um zu filtern, und dann manuell in einen anderen Container zu `push_back`? 😩
Haben Sie spät in der Nacht einen Off-by-One-Indexfehler debuggt, nur weil Sie das "drittletzte" Element beim Rückwärtsiterieren wollten? 😵💫
Sehnen Sie sich danach, wie eine Datenbank zu operieren – präzise Positionierung per Index, Analyse mit gleitenden Fenstern, Abschluss der gesamten Reise von Daten zu Statistiken mit einem einzigen Aufruf? 🤔

Semantic-Cpp wurde für diesen Zweck geboren. 🔧
Es abstrahiert die Datenverarbeitung als Operationen an **"Elementen"** und ihren **"logischen Positionen (Indizes)"** – ähnlich wie "Zeilen" und "Primärschlüssel" in einer Datenbank. Sie können Indizes frei umordnen, verschieben und umkehren, ohne die Daten selbst zu berühren; Sie können auch jeden Container (`vector`, `map`, `array`...) als ein unteilbares Ganzes behandeln, das durch den Stream fließt, und es jederzeit auf Elementebene "auspacken". Diese Fähigkeit, die Granularität zu wechseln, ist etwas, was traditionelle Streaming-Frameworks nicht bieten. 🎯

---

## 🏗️ Projektarchitektur: Acht-Schichten-Moduldesign

Semantic-Cpp besteht aus **acht Kern-Header-Dateien**, schrittweise aufgebaut, wobei jede Datei eine einzige, klar definierte Verantwortung hat und unabhängig testbar ist. Fünf Namespaces haben jeweils ihre eigene Rolle und arbeiten zusammen, um eine vollständige Pipeline von der Datenquelle zum Endergebnis zu bilden:

```
┌─────────────────────────────────────────────────┐
│               🔧 semantics.h                     │
│   Namespace: semantic                           │
│   Stream-Fabrik: numerische Bereiche, Container, Text, Unicode │
├─────────────────────────────────────────────────┤
│                 📦 semantic.h                    │
│   Namespace: semantic / collectable             │
│   Stream-Zwischenoperationen, Collectable-System, 10 Container-Spezialisierungen │
├─────────────────────────────────────────────────┤
│                📊 collectors.h                   │
│   Namespace: collector                          │
│   Collector-Fabrik: Matching, Finden, Aggregation, Statistik, DFT/FFT usw. │
├─────────────────────────────────────────────────┤
│                ⚙️ collector.h                    │
│   Namespace: collector                          │
│   Collector-Framework: Fünf-Phasen-Modell, Nebenläufigkeits- und Parallelitätsunterstützung │
├─────────────────────────────────────────────────┤
│                🔤 charsequence.h                 │
│   Namespace: charsequence                       │
│   Unicode-Zeichensequenzen, Multi-Codierungsumwandlung, Builder, Buffer │
├─────────────────────────────────────────────────┤
│                🧵 pool.h                         │
│   Namespace: pool                               │
│   Globaler Thread-Pool: Aufgabenübermittlung, Notabschaltung, Ausnahmepropagation │
├─────────────────────────────────────────────────┤
│                📄 function.h                     │
│   Namespace: function                           │
│   Typdefinitionen: Generator, Supplier, Consumer und andere Aliase │
├─────────────────────────────────────────────────┤
│               🔐 hash.h / less.h                 │
│   Namespace: std (Erweiterung)                  │
│   Standardbibliotheks-Container-Hash- und Vergleichs-Spezialisierungen, unterstützt beliebige Verschachtelung │
└─────────────────────────────────────────────────┘
```

### 🧩 Abhängigkeiten

```
function.h          ← Keine Abhängigkeiten, Typfundament
pool.h              ← Hängt von function.h ab
charsequence.h      ← Unabhängiges Modul, Unicode-Behandlung
collector.h         ← Hängt von function.h, pool.h ab
collectors.h        ← Hängt von collector.h, charsequence.h ab
hash.h / less.h     ← Unabhängige Module, Standardbibliothekserweiterungen
semantic.h          ← Hängt von allen oben genannten ab
semantics.h         ← Hängt von semantic.h ab
```

---

## 🌍 Namespace-Übersicht

Semantic-Cpp entwirft sorgfältig fünf Namespaces, jeder wie eine unabhängige "Abteilung", die ihre eigenen Aufgaben erfüllt und eng zusammenarbeitet:

| Namespace     | Datei              | Verantwortung                     | Kerntypen/-funktionen                                                                 |
|---------------|-------------------|------------------------------------|--------------------------------------------------------------------------------------|
| `function`    | function.h        | Typsystemfundament             | Timestamp, Module, Generator<T>, Supplier<R>, Consumer<T>, Predicate<T>, etc.       |
| `pool`        | pool.h            | Nebenläufige Ausführungs-Engine        | pool::pool (globaler Thread-Pool), submit(), emergencyShutdown()                       |
| `charsequence`| charsequence.h    | Unicode-String-Behandlung            | charset, Meta, Point, Charsequence, Builder, Buffer, etc.                           |
| `collector`   | collector.h + collectors.h | Terminale Sammlungsausführung | Collector<E,A,R>, Identity<A>, Accumulator<A,E>, etc.                               |
| `collectable` | semantic.h        | Materialisierter Datencontainer        | Collectable<E>, OrderedCollectable<E>, UnorderedCollectable<E>, etc.                |
| `semantic`    | semantic.h + semantics.h | Stream-Konstruktion und Zwischenoperationen | Semantic<E>, useRange(), useFrom(), etc.                                             |

### 🔁 Namespace-Kollaborationsfluss

```cpp
semantic::useRange(0, 100)          // ← semantic Namespace: Stream erstellen
    .map(int x { ... })         // ← semantic Namespace: Zwischentransformation
    .filter(int x { ... })      // ← semantic Namespace: Zwischenfilterung
    .toUnordered()                  // ← In collectable Namespace konvertieren
    .toVector();                    // ← Collector aus collector Namespace aufrufen
```

---

## 📦 Schicht 1: function.h — Typfundament

`function.h` definiert das Typsystem für das gesamte Framework, das gemeinsame Fundament für alle Module. 🔑

```cpp
namespace function {
    using Timestamp = long long;           // Indextyp, der "Zeitstempel" der Daten im Stream
    using Module = unsigned long long;     // Modul-/Zähltyp
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept – ein Element empfangen
        std::function<bool(T, Timestamp)>       // interrupt – sollen wir stoppen?
    )>;
}
```

`Generator` ist die Kernabstraktion des gesamten Streamingsystems. 🌀 Es gibt keine Daten zurück; stattdessen akzeptiert es zwei Callbacks – `accept` ("Ich bin bereit, bitte empfange dieses Element") und `interrupt` ("Müssen wir stoppen?"). Dieser Inversion-of-Control-Entwurf bedeutet, dass der Datenproduzent keine Ahnung hat, wer der Verbraucher ist; er muss nur Daten zum geeigneten Zeitpunkt "pushen". Dies ist die Essenz der faulen Auswertung: Daten "fließen" erst wirklich, wenn `accept` aufgerufen wird; davor ist alles nur eine Beschreibung.

| Typalias       | Vollständige Definition                                    | Zweck                             |
|------------------|----------------------------------------------------|-------------------------------------|
| Timestamp        | long long                                          | Logische Position eines Elements im Stream |
| Module           | unsigned long long                                 | Anzahl, Kapazität, Nebenläufigkeitsgrad |
| Runnable         | std::function<void()>                              | Parameterlose, void-zurückgebende Aufgabe |
| Supplier<R>      | std::function<R()>                                 | Lieferant, erzeugt aus dem Nichts      |
| Function<T,R>    | std::function<R(T)>                                | Einparameterfunktion           |
| BiFunction<T,U,R>| std::function<R(T,U)>                              | Zweiparameterfunktion              |
| TriFunction<T,U,V,R>| std::function<R(T,U,V)>                        | Dreiparameterfunktion            |
| Unary<T>         | std::function<T(T)>                                | Unäre Operation                     |
| Binary<T>        | std::function<T(T,T)>                              | Binäre Operation                    |
| Consumer<T>      | std::function<void(T)>                             | Verbraucher                            |
| BiConsumer<T,U>  | std::function<void(T,U)>                           | Zweiparameterverbraucher              |
| TriConsumer<T,U,V>| std::function<void(T,U,V)>                         | Dreiparameterverbraucher            |
| Predicate<T>     | std::function<bool(T)>                             | Prädikatsbeurteilung                 |
| BiPredicate<T,U> | std::function<bool(T,U)>                           | Zweiparameterprädikat             |
| TriPredicate<T,U,V>| std::function<bool(T,U,V)>                     | Dreiparameterprädikat           |
| Comparator<T>    | std::function<int(const T&,const T&)>              | Komparator, gibt negativ/null/positiv zurück |
| Generator<T>     | BiConsumer<BiConsumer<T,Timestamp>, BiPredicate<T,Timestamp>> | Kernabstraktion des Stream-Generators |

---

## 🧵 Schicht 2: pool.h — Nebenläufigkeitsfundament

`pool.h` stellt den globalen Thread-Pool `pool::pool` bereit, die Nebenläufigkeits-Engine für das gesamte Framework. 🚀 Es verwendet ein deklaratives Parallelitätsdesign:

| Merkmal            | Beschreibung                                                                         |
|--------------------|-------------------------------------------------------------------------------------|
| Deklarativer Parallelismus | `.parallel(4)` deklariert nur "Ich möchte 4 Threads verwenden", startet nicht sofort |
| Notabschaltung   | Eingebaute `emergencyShutdown()` und `std::set_terminate`-Handler                    |
| Ausnahmepropagation | `submit()` gibt `std::future` zurück, Ausnahmen sicher zum Hauptthread propagierend |
| Mitglied                       | Typ                | Beschreibung                             |
|------------------------------|---------------------|-----------------------------------------|
| `pool::pool`                 | Globale Thread-Pool-Instanz | Programmweiser Singleton-Thread-Pool, automatisch initialisiert |
| `pool::pool.submit<A>(task)` | Methode              | Übermittelt eine Aufgabe, gibt `std::future<A>` zurück |
| `pool::pool.emergencyShutdown()` | Methode       | Notabschaltung aller Threads       |

---

## 🔤 Schicht 3: charsequence.h — Unicode-Zeichensequenzen

`charsequence.h` ist ein vollständiges Unicode-Verarbeitungsmodul, das Funktionen zum Erstellen, Konvertieren und Manipulieren von Zeichensequenzen bereitstellt. 🌍 Es unterstützt mehrere Codierungen wie UTF-8, UTF-16 (LE/BE), UTF-32 (LE/BE), ASCII und Latin1, erkennt und behandelt Ersatzpaare korrekt und gibt das standardmäßige U+FFFD-Ersatzzeichen für ungültige Codepoints zurück.

| Typ/Funktion     | Beschreibung                                                                 |
|-------------------|-----------------------------------------------------------------------------|
| charset           | Aufzählung: ascii, utf8, utf16, utf16be, utf16le, utf32, utf32be, utf32le, latin1 |
| Meta              | Metadaten-Wrapper, speichert vorzeichenlose Ganzzahlwerte                          |
| Point             | Unicode-Codepoint, unterstützt Ersatzpaarerfassung (`isSurrogate()`) und Gültigkeitsüberprüfung (`isValidCodePoint()`) |
| Charsequence      | Unveränderliche Zeichensequenz: split, replace, indexOf, lastIndexOf, sub, trim, toUpperCase, toLowerCase, reverse, startsWith, endsWith, contains, compare, getBytes, getPoints, getMetas, getCharacters, repeat, concat, count, join |
| Builder           | Veränderlicher Byte-Builder: prepend, insert, append (unterstützt bool, short, int, long, long long, float, double, long double und entsprechende vorzeichenlose Typen, char, unsigned char, Point, Charsequence, string_view) |
| Buffer            | Thread-sicherer Ringpuffer: write, read, peek, prepend, append, clear, shrinkToFit, data, size, capacity, atomic (gesperrte Batch-Operationen) |
| PointIterator     | Bidirektionaler Iterator, durchläuft Unicode-Codepoints                      |
| sequenceLength()  | Bestimmt die Sequenzlänge für Mehrbyte-Codierungen wie UTF-8 basierend auf dem ersten Byte |
| encode()          | Codiert einen einzelnen Codepoint in eine Byte-Sequenz der angegebenen Codierung |
| decode()          | Decodiert den nächsten Codepoint aus einer Byte-Sequenz, bewegt den Zeiger automatisch vorwärts |
| convert()         | Codierungsumwandlung (unterstützt string, vector<unsigned char>, deque<unsigned char> Ausgabe) |

---

## ⚙️ Schicht 4: collector.h — Collector-Framework

`collector.h` implementiert das Collector-Muster, die Kern-Engine für terminale Operationen. 🔧

### 🧩 Fünf-Phasen-Modell

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (optionaler Kurzschluss)
```

Diese fünf Phasen sind wie eine präzise Montagelinie: zuerst den Anfangszustand vorbereiten (Identity), dann Elemente einzeln empfangen und Zwischenergebnisse aktualisieren (Accumulator), danach Teilergebnisse jedes Threads in parallelen Szenarien zusammenführen (Combiner) und schließlich das Zwischenergebnis in die vom Benutzer gewünschte Endform umwandeln (Finisher). Interrupt fungiert wie ein aufmerksamer Aufseher, bereit, den gesamten Prozess jederzeit zu stoppen, wenn Bedingungen erfüllt sind. 🚨

| Typalias       | Vollständige Definition                                 | Rolle                         |
|------------------|-------------------------------------------------|------------------------------|
| Identity<A>      | function::Supplier<A>                           | Liefert Anfangswert       |
| Accumulator<A,E> | function::TriFunction<A, E, Timestamp, A>       | Akkumuliert Elemente         |
| Combiner<A>      | function::BiFunction<A, A, A>                   | Führt parallele Ergebnisse zusammen      |
| Finisher<A,R>    | function::Function<A, R>                        | Endgültige Transformation         |
| Interrupt<E,A>   | function::TriPredicate<E, Timestamp, A>         | Kurzschlussbeurteilung      |

### 🔧 Framework-Funktionen

| Funktion                                                               | Beschreibung                         |
|------------------------------------------------------------------------|-------------------------------------|
| `useFull(identity, accumulator, combiner, finisher)`                   | Erstellt einen vollständigen Collector (ohne Kurzschluss) |
| `useShortable(identity, interrupt, accumulator, combiner, finisher)`   | Erstellt einen kurzschließbaren Collector |

### 🧱 `Collector<E,A,R>` Klasse

| Methode                                 | Beschreibung                               |
|----------------------------------------|-------------------------------------------|
| `collect(generator, concurrent)`       | Sammelt von einem Generator, unterstützt Parallelismus |
| `collect(container, concurrent)`       | Sammelt von einem Standardcontainer, unterstützt Parallelismus |
| `collect(initializer_list, concurrent)`| Sammelt von einer Initialisierungsliste         |
| `collect(array, concurrent)`           | Sammelt von einem std::array               |
| `collect(deque, concurrent)`           | Sammelt von einem std::deque              |
| `collect(forward_list, concurrent)`    | Sammelt von einem std::forward_list       |
| `collect(stack, concurrent)`           | Sammelt von einem std::stack             |
| `collect(queue, concurrent)`           | Sammelt von einem std::queue            |

### 🔀 Nebenläufigkeitsunterstützung

`Collector::collect()` behandelt automatisch: Datenpartitionierung (Verteilung auf Threads via Index-Modulo), Ergebniszusammenführung (Zusammenführen von Teilergebnissen via Combiner), Ausnahmepropagation (via `std::exception_ptr` und `std::atomic<bool>`). 🔀

---

## 📊 Schicht 5: collectors.h — Collector-Fabrik

`collectors.h` stellt einen reichen Satz vordefinierter Collector-Fabrikfunktionen bereit. Dies ist nicht einfach "ein paar Hilfsfunktionen" – es ist ein vollständiges Arsenal von Datenverarbeitungswerkzeugen, das das gesamte Spektrum vom grundlegenden Matching bis zur Frequenzbereichsanalyse abdeckt. 🔥

### ✅ Matching-Operationen

Möchten Sie wissen, ob alle Elemente im Stream eine Bedingung erfüllen? Oder ob es irgendeinen "Rebellen" gibt? Diese drei geben sofort die Antwort und unterstützen alle Kurzschlussauswertung – Stoppen der Traversierung, sobald die Antwort feststeht. ⚡

| Methode                  | Beschreibung                                | Rückgabetyp |
|-------------------------|--------------------------------------------|-------------|
| `useAllMatch(predicate)` | Alle Elemente erfüllen die Bedingung (universeller Quantor) | `bool` |
| `useAnyMatch(predicate)` | Irgendein Element erfüllt die Bedingung (existenzieller Quantor) | `bool` |
| `useNoneMatch(predicate)`| Kein Element erfüllt die Bedingung         | `bool` |

### 🔍 Suchoperationen

Finden Sie das gewünschte "es" aus dem weiten Datenmeer. Negative Indizes sind Semantic-Cpps einzigartiges Merkmal – `findAt(-1)` erhält direkt das letzte Element. 🎯

| Methode               | Beschreibung                                   | Rückgabetyp         |
|----------------------|-----------------------------------------------|---------------------|
| `useFindFirst()`     | Findet das erste Element                       | `std::optional<E>`  |
| `useFindLast()`      | Findet das letzte Element                        | `std::optional<E>`  |
| `useFindAny()`       | Findet irgendein Element zufällig                    | `std::optional<E>`  |
| `useFindAt(index)`   | Präzise Positionierung: unterstützt positive und negative Indizes | `std::optional<E>` |
| `useFindMaximum()`   | Findet den Maximalwert (unterstützt benutzerdefinierten Komparator) | `std::optional<E>` |
| `useFindMinimum()`   | Findet den Minimalwert (unterstützt benutzerdefinierten Komparator) | `std::optional<E>` |

### 🔢 Aggregationsoperationen

Lassen Sie die Daten sprechen, fassen Sie alles mit Zahlen zusammen. Jede Aggregationsmethode unterstützt einen optionalen Mapper-Parameter – Wendet zuerst eine Transformation auf jedes Element an, bevor aggregiert wird. 📈

| Methode               | Beschreibung                     | Rückgabetyp         |
|----------------------|---------------------------------|---------------------|
| `useCount()`         | Gesamtanzahl der Elemente        | `Module`            |
| `useSummate<E,D>()`  | Summation                       | `D`                 |
| `useAverage<E,D>()`  | Durchschnitt                         | `D`                 |
| `useRange<E,D>()`    | Numerischer Bereich (max - min)       | `D`                 |
| `useMinimum<E,D>()`  | Minimalwert                   | `std::optional<D>`  |
| `useMaximum<E,D>()`  | Maximalwert                   | `std::optional<D>`  |

### 📉 Statistische Operationen

Von deskriptiver Statistik bis zur Frequenzbereichsanalyse, ein Schweizer Taschenmesser für Statistiker. Besonders erwähnenswert sind `useMode()` und `useFrequency()` – sie verwenden Index-Phasen-Codierungstechnologie, kodieren die Positionsinformation jedes Vorkommens eines Elements als Winkel in der komplexen Ebene und nutzen die Euler-Formel, um periodische Muster in den Daten zu erfassen. 🎼

| Methode                          | Beschreibung                               | Rückgabetyp              |
|---------------------------------|-------------------------------------------|--------------------------|
| `useVariance<E,D>()`           | Populationsvarianz                       | `D`                      |
| `useStandardDeviation<E,D>()`  | Populationsstandardabweichung             | `D`                      |
| `useSkewness<E,D>()`           | Schiefe (Symmetrie der Verteilung)       | `D`                      |
| `useKurtosis<E,D>()`           | Wölbung (Schwanzdicke der Verteilung) | `D`                      |
| `useMedian<E,D>()`             | Median                                    | `std::optional<D>`       |
| `useMode<E>()`                 | Modus (basierend auf Frequenzbereichsanalyse) | `std::optional<E>`       |
| `usePercentile<E,D>(p)`        | p-tes Perzentil                           | `std::optional<D>`       |
| `useFrequency<E>()`            | Frequenzbereichsmerkmale (Index-Phasen-Codierung) | `std::map<E, complex>` |
| `useDistribution<E>()`         | Räumliche Verteilungsmerkmale (Positionscodierung) | `std::map<E, complex>` |

### 🔀 Reduktionsoperationen

Reduktion (Reduce) ist eines der mächtigsten Konzepte in der funktionalen Programmierung – es kann einen Stream in einen einzelnen Wert "falten". 🎁

| Methode                          | Beschreibung                                 | Rückgabetyp         |
|---------------------------------|---------------------------------------------|---------------------|
| `useReduce(reducer)`            | Reduktion ohne Anfangswert (gibt nullopt zurück, wenn Stream leer ist) | `std::optional<E>` |
| `useReduce(identity, reducer)`  | Reduktion mit Anfangswert                | `E`                 |
| `useReduce(id, red, comb, fin)` | Vollständig benutzerdefiniert: benutzerdefinierte Akkumulation, Kombination, Abschluss | `R`    |

### 🧺 Sammeln in Container-Operationen

Daten, die ihr ganzes Leben durch den Stream gewandert sind, brauchen schließlich ein "Zuhause". Hier sind 20+ Standardbibliothekscontainer zur Auswahl: 🏡

| Methode                           | Beschreibung                         | Rückgabetyp               |
|----------------------------------|-------------------------------------|---------------------------|
| `useToVector()`                  | Sammelt in vector, Reihenfolge beibehaltend | `std::vector<E>`         |
| `useToList()`                    | Sammelt in list                 | `std::list<E>`           |
| `useToDeque()`                   | Sammelt in deque                | `std::deque<E>`          |
| `useToForwardList()`             | Sammelt in forward_list         | `std::forward_list<E>`   |
| `useToArray<N>()`                | Sammelt in feste Größe array     | `std::array<E, N>`       |
| `useToSet()`                     | Sammelt in set (dedupliziert, sortiert) | `std::set<E>`        |
| `useToMultiset()`                | Sammelt in multiset             | `std::multiset<E>`       |
| `useToUnorderedSet()`            | Sammelt in unordered_set        | `std::unordered_set<E>`  |
| `useToUnorderedMultiset()`       | Sammelt in unordered_multiset   | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)`         | Sammelt in map nach Schlüssel           | `std::map<K, E>`         |
| `useToMap(keyExtractor, valueExtractor)` | Sammelt in map mit benutzerdefiniertem Schlüssel-Wert | `std::map<K, V>` |
| `useToMultimap(keyExtractor)`    | Sammelt in multimap nach Schlüssel      | `std::multimap<K, E>`    |
| `useToMultimap(keyExtractor, valueExtractor)` | Sammelt in multimap mit benutzerdefiniertem Schlüssel-Wert | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)` | Sammelt in unordered_map | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)` | Sammelt in unordered_multimap nach Schlüssel | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)` | Sammelt in unordered_multimap mit benutzerdefiniertem Schlüssel-Wert | `std::unordered_multimap<K, V>` |
| `useToStack()`                   | Sammelt in stack                | `std::stack<E>`          |
| `useToQueue()`                   | Sammelt in queue                | `std::queue<E>`          |
| `useToPriorityQueue()`           | Sammelt in priority_queue       | `std::priority_queue<E>` |

### 🧩 Gruppierungs- und Partitionierungsoperationen

Das Äquivalent von SQLs `GROUP BY` in C++. Die Zweiparameter-Versionen von `groupBy` und `partitionBy` ermöglichen es, gleichzeitig sowohl einen Schlüsselextraktor als auch einen Wertextraktor anzugeben – zum Beispiel Mitarbeiter nach Abteilung gruppieren, aber nur Mitarbeiternamen anstelle des gesamten Objekts nehmen. 👥

| Methode                                     | Beschreibung                       | Rückgabetyp                          |
|--------------------------------------------|-----------------------------------|---------------------------------------|
| `useGroup(keyExtractor)`                   | Gruppiert nach Schlüssel, behält vollständige Elemente bei | `std::unordered_map<K, vector<E>>` |
| `useGroupBy(keyExtractor, valueExtractor)` | Gruppiert nach Schlüssel, benutzerdefinierte Wertextraktion | `std::unordered_map<K, vector<V>>` |
| `usePartition(size)`                       | Partitioniert nach fester Größe         | `std::vector<vector<E>>`             |
| `usePartitionBy(keyExtractor)`             | Partitioniert nach Indexschlüssel, behält vollständige Elemente bei | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor, valueExtractor)` | Partitioniert nach Indexschlüssel, benutzerdefinierte Wertextraktion | `std::vector<vector<V>>` |

```cpp
// Nach Abteilung gruppieren, nur Namen behalten
auto deptNames = people.toUnordered().groupBy(
    Person p { return p.department; },   // Schlüssel: Abteilung
    Person p { return p.name; }          // Wert: Name
);
// Gibt zurück: unordered_map<string, vector<string>>
```

### 📃 String-Ausgabeoperationen

Die `useJoin()`, `useOut()`, `useError()`-Serie lässt Ihre Daten in einem menschenlesbaren Format darstellen: 📃

| Methode                               | Beschreibung                 | Rückgabetyp               |
|--------------------------------------|-----------------------------|---------------------------|
| `useJoin()`                          | Standardformat-Verbindung: `[elem1,elem2,...]` | `Charsequence`     |
| `useJoin(delimiter)`                 | Benutzerdefiniertes Trennzeichen           | `Charsequence`            |
| `useJoin(prefix, delimiter, suffix)` | Vollständig benutzerdefiniertes Format       | `Charsequence`            |
| `useOut()`                           | Gibt an stdout aus         | `Charsequence`            |
| `useOut(delimiter)`                  | Gibt an stdout mit Trennzeichen aus | `Charsequence`        |
| `useOut(prefix, delimiter, suffix)`  | Vollständig benutzerdefinierte Ausgabe an stdout | `Charsequence`        |
| `useError()`                         | Gibt an stderr aus         | `Charsequence`            |
| `useError(delimiter)`                | Gibt an stderr mit Trennzeichen aus | `Charsequence`        |
| `useError(prefix, delimiter, suffix)`| Vollständig benutzerdefinierte Ausgabe an stderr | `Charsequence`        |

### 📊 Mathematische Werkzeuge

| Methode                                     | Beschreibung                         | Rückgabetyp                    |
|--------------------------------------------|-------------------------------------|--------------------------------|
| `useDFT()`                                 | Diskrete Fourier-Transformation         | `vector<complex<double>>`      |
| `useIDFT()`                                | Inverse Diskrete Fourier-Transformation | `vector<complex<double>>`      |
| `useFFT()`                                 | Schnelle Fourier-Transformation (Cooley-Tukey) | `vector<complex<double>>`  |
| `useIFFT()`                                | Inverse Schnelle Fourier-Transformation     | `vector<complex<double>>`      |
| `useGradient(gradFunc, lr, iter, th)`      | Gradientenabstieg (analytischer Gradient) | `vector<double>`         |
| `useGradient(costFunc, lr, iter, th, h)`   | Gradientenabstieg (numerischer Gradient) | `vector<double>`         |

---

## 📦 Schicht 6: semantic.h — Stream-Zwischenoperationen und Sammelsystem

---

### 🧩 Kerndesign: Drei-Phasen-Pipeline — `Semantic` → `Collectable` → `Collector`

Viele Streaming-Frameworks vermischen "Zwischenoperationen" und "terminale Operationen" im selben Typ, was unklar macht, wann Daten tatsächlich verarbeitet werden. Semantic-Cpp übernimmt ein grundlegend anderes Drei-Phasen-Trennungsdesign, bei dem jede Phase eine klare Aufgabe und eine deutliche Grenze hat:

```
┌──────────────────────────────────────────────────────────┐
│                 🌱 Phase 1: Konstruktion & Transformation │
│               Semantic<E>  (Semantischer Stream)             │
│                 Namespace: semantic                       │
│                                                          │
│  · Faule Konstruktion: useRange, useFrom, useGenerate... │
│  · Zwischentransformationen: map, filter, takeWhile, distinct... |
│  · Indexoperationen: reverse, translate, redirect...    │
│  · Parallele Deklaration: parallel(n)                     │
│                                                          │
│  An diesem Punkt liegen die Daten immer noch "im" Pipeline, bewegungslos, nichts ist passiert. │
│  Wie wenn Sie einen detaillierten Reiseplan geschrieben haben, aber noch nicht von zu Hause weggegangen sind. │
└──────────────────┬───────────────────────────────────────┘
                   │  toUnordered() / toOrdered() /
                   │  toWindow() / toStatistics() / sort()
                   ▼
┌──────────────────────────────────────────────────────────┐
│                📦 Phase 2: Materialisierung & Sammlung  │
│           Collectable<E>  (Sammelbares Objekt)           │
│             Namespace: collectable                       │
│                                                          │
│  · Löst Datenfluss aus: Der Generator wird jetzt tatsächlich aufgerufen │
│  · Wählt Datenstruktur: geordnete map / ungeordnete unordered_map │
│  · Materialisiert alle Elemente im Speicher                │
│                                                          │
│  Die Daten "erwachen" endlich! Sie strömen aus der Pipeline, organisiert in geeignete Container. │
│  Wie wenn Sie endlich aufgebrochen sind, Gepäck gepackt, bereit zu gehen. │
└──────────────────┬───────────────────────────────────────┘
                   │  toVector() / findFirst() / count() /
                   │  summate() / average() / join() / ...
                   ▼
┌──────────────────────────────────────────────────────────┐
│                ⚙️ Phase 3: Terminalberechnung           │
│              Collector<E,A,R>                            │
│               Namespace: collector                       │
│                                                          │
│  · Fünf-Phasen-Ausführung: Identity → Accumulate → Combine → Finish │
│  · Parallelitätsunterstützung: Mehrfädige partitionierte Akkumulation, automatisches Zusammenführen │
│  · Gibt Endergebnis zurück: vector, optional, double, bool... │
│                                                          │
│  Das Endergebnis ist da! Wie wenn Sie Ihr Ziel erreicht und das perfekte Foto gemacht haben. │
└──────────────────────────────────────────────────────────┘
```

**Schlüsselregel:** Sie müssen zuerst `Semantic<E>` via `toUnordered()`, `toOrdered()`, `toWindow()`, `toStatistics()` oder `sort()` in `Collectable<E>` konvertieren, bevor Sie terminale Methoden wie `toVector()`, `findFirst()`, `count()` usw. aufrufen können. Diese Methoden sind nicht auf `Semantic` – sie gehören zu `Collectable`. Dies ist kein Versehen, sondern eine bewusste Designentscheidung. ✅

---

### 🧭 Fünf Materialisierungspfade, fünf verschiedene Schicksale

Wenn Sie am Scheideweg von `Semantic<E>` stehen, gibt es fünf Richtungen zur Wahl. Jede bestimmt, wie die Daten organisiert, abgefragt und verwendet werden. Die Unterschiede zwischen ihnen zu verstehen, ist der Schlüssel zur Beherrschung von Semantic-Cpp. 🧠

| Konvertierungsmethode  | Zieltyp            | Unterliegende Datenstruktur  | Leistungsmerkmale | Typische terminale Methoden       |
|--------------------|------------------------|----------------------------|----------------------------|--------------------------------|
| `toUnordered()`    | UnorderedCollectable<E>| unordered_map<Timestamp,E> | Durchschnittl. O(1)-Suche, höchster Durchsatz | toVector, findFirst, count, group... |
| `toOrdered()`      | OrderedCollectable<E>  | map<Timestamp,E>           | O(log n)-Suche, nach Index sortiert | toVector, findAt, join, toMap... |
| `sort()`           | OrderedCollectable<E>  | map<Timestamp,E>           | Nach Wert sortiert materialisiert | Gleich wie oben (aber Index durch Wertreihenfolge überschrieben) |
| `toWindow()`       | WindowCollectable<E>   | Erbt geordnete Sammlung| Unterstützt slide / tumble    | slide, tumble und alle Elternmethoden |
| `toStatistics<D>()`| Statistics<E,D>        | Erbt geordnete Sammlung| 30+ eingebaute statistische Methoden | summate, average, median, fft... |

### 🔄 Detaillierter Vergleich der fünf Pfade

| Vergleichsdimension | toUnordered()         | toOrdered()           | sort()                | toWindow()            | toStatistics<D>()    |
|----------------------|-----------------------|-----------------------|-----------------------|-----------------------|----------------------|
| Bewahrt Indexreihenfolge? | Nein (ungeordnet)      | Ja (nach ursprünglichem Index) | Nein (nach Wert neu sortiert) | Ja (erbt geordnet) | Ja (erbt geordnet) |
| Suchleistung   | Schnellste O(1)         | Schneller O(log n)       | Schneller O(log n)       | Schneller O(log n)       | Schneller O(log n)      |
| Speicherverbrauch         | Niedriger                 | Niedriger                 | Niedriger                 | Niedriger                 | Hängt von statistischer Methode ab |
| Kann Kette fortsetzen?  | Ja (terminale Methoden) | Ja (terminale Methoden) | Ja (terminale Methoden) | Ja (slide/tumble gibt Semantic zurück) | Ja (terminale Methoden) |
| Anwendungsfall             | Schnelle Aggregation, wo Reihenfolge keine Rolle spielt | Zeitreihen, Erzeugungsreihenfolge bewahren | Nach Wert sortieren, Rangfolge, Paginierung | Gleitende Fensteranalyse | Mathematische statistische Modellierung |

`sort()` ist der einzige Zwischenoperation, der `toXxx()` überspringt und direkt zu `Collectable` geht. Nach dem Aufruf werden alle vorherigen Indexoperationen (reverse, translate, redirect) überschrieben, und Elemente erhalten natürliche sequenzielle Indizes basierend auf ihren sortierten Werten neu zugewiesen. 🔀

**Welchen Pfad wählen? Fragen Sie sich:** 🤔

* Kümmere ich mich um die Reihenfolge? Wenn ja, verwenden Sie `toOrdered()`; wenn nein, verwenden Sie `toUnordered()`.
* Brauche ich Fensteranalyse? `toWindow()` dann `slide()` oder `tumble()`.
* Brauche ich Statistik? `toStatistics<double>()` für eine All-in-One-Lösung.
* Will ich nur Sortierung? `sort()` in einem Schritt.

---

### 📋 collectable Namespace — Alle terminalen Methoden

#### ✅ Matching-Operationen

| Methode               | Rückgabetyp | Beschreibung                 |
|----------------------|-------------|-----------------------------|
| `anyMatch(predicate)`  | bool        | Irgendein Element erfüllt Bedingung |
| `allMatch(predicate)`  | bool        | Alle Elemente erfüllen Bedingung |
| `noneMatch(predicate)` | bool        | Kein Element erfüllt Bedingung |

#### 🔍 Suchoperationen

| Methode                     | Rückgabetyp         | Beschreibung                     |
|----------------------------|---------------------|---------------------------------|
| `findFirst()`              | std::optional<E>    | Findet das erste Element         |
| `findLast()`               | std::optional<E>    | Findet das letzte Element          |
| `findAny()`                | std::optional<E>    | Findet irgendein Element zufällig      |
| `findAt(index)`            | std::optional<E>    | Findet Element an spezifiziertem Index (unterstützt negativ) |
| `findMaximum()`            | std::optional<E>    | Findet den Maximalwert         |
| `findMaximum(comparator)`  | std::optional<E>    | Findet Maximum mit benutzerdefiniertem Komparator |
| `findMinimum()`            | std::optional<E>    | Findet den Minimalwert         |
| `findMinimum(comparator)`  | std::optional<E>    | Findet Minimum mit benutzerdefiniertem Komparator |

#### 🔢 Aggregationsoperationen

| Methode      | Rückgabetyp        | Beschreibung         |
|-------------|-------------------|---------------------|
| `count()`   | function::Module  | Zählt Gesamtelemente |
| `empty()`   | bool              | Ob Stream leer ist |

#### 🔀 Reduktionsoperationen

| Methode                      | Rückgabetyp         | Beschreibung             |
|-----------------------------|---------------------|-------------------------|
| `reduce(accumulator)`       | std::optional<E>    | Reduktion ohne Anfangswert |
| `reduce(identity, accumulator)` | E           | Reduktion mit Anfangswert |
| `reduce(identity, acc, combiner)` | R | Vollständig benutzerdefinierte Reduktion |

#### 🧺 Sammeln in Sequenzcontainer

| Methode             | Rückgabetyp               | Beschreibung                 |
|--------------------|---------------------------|-----------------------------|
| `toVector()`       | std::vector<E>            | Sammelt in vector, Reihenfolge beibehaltend |
| `toList()`         | std::list<E>              | Sammelt in list          |
| `toDeque()`        | std::deque<E>             | Sammelt in deque         |
| `toForwardList()`  | std::forward_list<E>      | Sammelt in forward_list  |
| `toArray<N>()`     | std::array<E, N>          | Sammelt in feste Größe array |

#### 🔐 Sammeln in assoziative Container

| Methode                                     | Rückgabetyp                        | Beschreibung                 |
|--------------------------------------------|------------------------------------|-----------------------------|
| `toSet()`                                  | std::set<E>                        | Sammelt in set (dedupliziert, sortiert) |
| `toMultiset()`                             | std::multiset<E>                   | Sammelt in multiset      |
| `toUnorderedSet()`                         | std::unordered_set<E>              | Sammelt in unordered_set |
| `toUnorderedMultiset()`                    | std::unordered_multiset<E>         | Sammelt in unordered_multiset |
| `toMap(keyExtractor)`                      | std::map<K, E>                     | Sammelt in map nach Schlüssel    |
| `toMap(keyExtractor, valueExtractor)`      | std::map<K, V>                     | Sammelt in map mit benutzerdefiniertem Schlüssel-Wert |
| `toMultimap(keyExtractor)`                 | std::multimap<K, E>                | Sammelt in multimap nach Schlüssel |
| `toMultimap(keyExtractor, valueExtractor)` | std::multimap<K, V>                | Sammelt in multimap mit benutzerdefiniertem Schlüssel-Wert |
| `toUnorderedMap(keyExtractor, valueExtractor)` | std::unordered_map<K, V> | Sammelt in unordered_map |
| `toUnorderedMultimap(keyExtractor)`        | std::unordered_multimap<K, E>      | Sammelt in unordered_multimap nach Schlüssel |
| `toUnorderedMultimap(keyExtractor, valueExtractor)` | std::unordered_multimap<K, V> | Sammelt in unordered_multimap mit benutzerdefiniertem Schlüssel-Wert |

#### 🧮 Sammeln in Adapter-Container

| Methode                | Rückgabetyp                  | Beschreibung         |
|-----------------------|-----------------------------|---------------------|
| `toStack()`           | std::stack<E>               | Sammelt in stack |
| `toQueue()`           | std::queue<E>               | Sammelt in queue |
| `toPriorityQueue()`   | std::priority_queue<E>      | Sammelt in priority_queue |

#### 👥 Gruppierung und Partitionierung

| Methode                                     | Rückgabetyp                          | Beschreibung                 |
|--------------------------------------------|--------------------------------------|-----------------------------|
| `group(keyExtractor)`                      | std::unordered_map<K, std::vector<E>> | Gruppiert nach Schlüssel, behält vollständige Elemente bei |
| `groupBy(keyExtractor, valueExtractor)`    | std::unordered_map<K, std::vector<V>> | Gruppiert nach Schlüssel, benutzerdefinierte Wertextraktion |
| `partition(size)`                          | std::vector<std::vector<E>>         | Partitioniert nach fester Größe   |
| `partitionBy(keyExtractor)`                | std::vector<std::vector<E>>         | Partitioniert nach Indexschlüssel, behält vollständige Elemente bei |
| `partitionBy(keyExtractor, valueExtractor)` | std::vector<std::vector<V>>       | Partitioniert nach Indexschlüssel, benutzerdefinierte Wertextraktion |

#### 📃 String-Ausgabeoperationen

| Methode                               | Rückgabetyp               | Beschreibung                 |
|--------------------------------------|---------------------------|-----------------------------|
| `join()`                             | charsequence::Charsequence | Standardformat-Verbindung: `[elem1,elem2,...]` |
| `join(delimiter)`                    | charsequence::Charsequence | Benutzerdefiniertes Trennzeichen           |
| `join(prefix, delimiter, suffix)`    | charsequence::Charsequence | Vollständig benutzerdefiniertes Format       |
| `out()`                              | charsequence::Charsequence | Gibt an stdout aus         |
| `out(delimiter)`                     | charsequence::Charsequence | Gibt an stdout mit Trennzeichen aus |
| `out(prefix, delimiter, suffix)`     | charsequence::Charsequence | Vollständig benutzerdefinierte Ausgabe an stdout |
| `error()`                            | charsequence::Charsequence | Gibt an stderr aus         |
| `error(delimiter)`                   | charsequence::Charsequence | Gibt an stderr mit Trennzeichen aus |
| `error(prefix, delimiter, suffix)`   | charsequence::Charsequence | Vollständig benutzerdefinierte Ausgabe an stderr |

#### 🔧 Benutzerdefinierte Sammlung und Traversierung

| Methode                                 | Rückgabetyp | Beschreibung                 |
|----------------------------------------|-------------|-----------------------------|
| `collect(identity, acc, comb, fin)`    | R           | Benutzerdefinierte Vier-Phasen-Sammlung |
| `collect(identity, interrupt, acc, comb, fin)` | R | Benutzerdefinierte unterbrechbare Sammlung |
| `forEach(consumer)`                    | void        | Führt Seiteneffekt für jedes Element aus |

---

### 📈 `Statistics<E,D>` — Statistische Methoden

| Methode                     | Rückgabetyp              | Beschreibung                               |
|----------------------------|--------------------------|-------------------------------------------|
| `summate()`                | D                        | Summation                                 |
| `average()`                | D                        | Durchschnitt                                   |
| `minimum()`                | std::optional<D>         | Minimalwert                             |
| `maximum()`                | std::optional<D>         | Maximalwert                             |
| `range()`                  | D                        | Bereich (max - min)                         |
| `variance()`               | D                        | Populationsvarianz                       |
| `standardDeviation()`      | D                        | Populationsstandardabweichung             |
| `median()`                 | std::optional<D>         | Median                                    |
| `mode()`                   | std::optional<E>         | Modus (basierend auf Frequenzbereichsanalyse) |
| `percentile(p)`            | std::optional<D>         | p-tes Perzentil                           |
| `firstQuartile()`          | std::optional<D>         | Erstes Quartil (Q1)                       |
| `thirdQuartile()`          | std::optional<D>         | Drittes Quartil (Q3)                       |
| `interquartileRange()`     | std::optional<D>         | Interquartilsabstand (IQR = Q3 - Q1)      |
| `skewness()`               | D                        | Schiefe                                  |
| `kurtosis()`               | D                        | Wölbung                                  |
| `frequency()`              | std::map<E, std::complex<double>> | Frequenzbereichsmerkmale (Index-Phasen-Codierung) |
| `distribute()`             | std::map<E, std::complex<double>> | Räumliche Verteilungsmerkmale (Positionscodierung) |
| `dft()`                    | std::vector<std::complex<double>> | Diskrete Fourier-Transformation             |
| `idft()`                   | std::vector<std::complex<double>> | Inverse Diskrete Fourier-Transformation   |
| `fft()`                    | std::vector<std::complex<double>> | Schnelle Fourier-Transformation                 |
| `ifft()`                   | std::vector<std::complex<double>> | Inverse Schnelle Fourier-Transformation       |
| `gradient(gradFunc, lr, iter, th)` | std::vector<double> | Gradientenabstieg (analytischer Gradient)   |
| `gradient(costFunc, lr, iter, th, h)` | std::vector<double> | Gradientenabstieg (numerischer Gradient) |

Alle oben genannten Methoden unterstützen auch eine optionale Mapper-Parameterversion (z.B. `average(mapper)`), die Elemente zuerst transformiert, bevor Statistik durchgeführt wird.

---

### 🧰 Neue Container-Spezialisierungen: Container als Elemente

Dies ist eine der einzigartigsten Eigenschaften von Semantic-Cpp. In traditionellen Streaming-Frameworks ist jedes Element im Stream typischerweise ein Skalarwert – ein int, ein string. Aber in der realen Welt existieren Daten oft in Batches:

* Ein Batch von Sensorablesungen = `std::vector<double>`
* Ein Satz von Benutzeraufträgen = `std::list<Order>`
* Eine Schlüssel-Wert-Konfiguration = `std::map<string, string>`
* Eine feste Größe Matrixzeile = `std::array<float, 4>`

Semantic-Cpp stellt `Semantic`-Template-Spezialisierungen für **10 Standardbibliothekscontainer** bereit:

| Spezialisierter Typ                     | Beschreibung          | Unterstützte Operationen               |
|--------------------------------------|----------------------|------------------------------------|
| `Semantic<std::vector<E>>`           | Vector-Container-Stream | Alle Zwischen- + alle terminalen Operationen |
| `Semantic<std::list<E>>`             | List-Container-Stream   | Gleich wie oben                     |
| `Semantic<std::deque<E>>`            | Deque-Container-Stream  | Gleich wie oben                     |
| `Semantic<std::set<E>>`              | Geordneter Set-Container-Stream | Gleich wie oben              |
| `Semantic<std::unordered_set<E>>`    | Ungeordneter Set-Container-Stream | Gleich wie oben           |
| `Semantic<std::map<K,V>>`            | Geordneter Map-Container-Stream | Gleich wie oben              |
| `Semantic<std::unordered_map<K,V>>`  | Ungeordneter Map-Container-Stream | Gleich wie oben           |
| `Semantic<std::queue<E>>`            | Queue-Container-Stream   | Gleich wie oben                     |
| `Semantic<std::stack<E>>`            | Stack-Container-Stream   | Gleich wie oben                     |
| `Semantic<std::array<E,N>>`          | Fester Array-Container-Stream | Gleich wie oben                |

Alle Container-Spezialisierungen unterstützen vollständig: `map`, `filter`, `takeWhile`, `dropWhile`, `distinct`, `sort`, `limit`, `skip`, `reverse`, `translate`, `redirect`, `sub`, `concatenate`, `peek`, `flatMap`, `flat`, `parallel` und alle terminalen Konversionsmethoden. 🔧

```cpp
// Container-Stream-Beispiel: Mehrere Vektoren in einen Element-Stream abflachen
auto flattened = semantic::useOf({
    std::vector<int>{1, 2},
    std::vector<int>{3, 4, 5}
}).flatMap(std::vector<int> v {
    return semantic::useFrom(v).map(int x { return x * 10; });
}).toOrdered().toVector();
// Ausgabe: 10 20 30 40 50
```

---

### 🔧 `Semantic<E>` Zwischenoperationsmethoden

| Kategorie     | Methode       | Beschreibung                               |
|--------------|--------------|-------------------------------------------|
| Elementtransformation | `map`      | Eins-zu-eins-Mapping-Transformation         |
|              | `flatMap`  | Eins-zu-viele-Mapping und Abflachung (R vom Rückgabetyp abgeleitet) |
|              | `flat`     | Abflacht verschachtelte Streams (Elementtyp unverändert) |
| Elementfilterung | `filter`   | Bedingte Filterung                     |
|              | `takeWhile`| Nimmt, solange Bedingung wahr ist, stoppt sofort, wenn falsch |
|              | `dropWhile`| Wirft weg, solange Bedingung wahr ist, bis erstes falsch |
|              | `distinct` | Deduplizierung (unterstützt benutzerdefinierten Komparator) |
| Mengenkontrolle | `limit`    | Begrenzt Anzahl der Elemente                 |
|              | `skip`     | Überspringt erste n Elemente                    |
|              | `sub`      | Nimmt Teilbereich [start, end)              |
| Indexoperationen | `redirect` | Ordnet Indizes neu zu                           |
|              | `reverse`  | Kehrt Indizes um (verwendet negative Indizes)  |
|              | `translate`| Verschiebt Indizes (fester Wert oder dynamische Funktion) |
| Beobachtung  | `peek`     | Beobachtet jedes Element (modifiziert Stream nicht) |
| Parallele Deklaration | `parallel(n)` | Deklariert Parallelitätsgrad        |
| Verkettung | `concatenate` | Verkettet einen anderen Semantic-Stream oder Standardcontainer |
| Terminalkonversion | `toUnordered` | Konvertiert zu ungeordnetem Collector   |
|              | `toOrdered`  | Konvertiert zu geordnetem Collector     |
|              | `toWindow`   | Konvertiert zu Fenster-Collector      |
|              | `toStatistics<D>` | Konvertiert zu statistischem Collector |

---

## 🔧 Schicht 7: semantics.h — Stream-Fabrikfunktionen

`semantics.h` stellt alle Stream-Fabrikfunktionen bereit. Wenn `semantic.h` das "Gehirn" des Streams ist (verantwortlich für Transformation und Planung), dann ist `semantics.h` das "Herz" – jede Datenreise beginnt hier. ❤️

### 🔢 Numerische Bereichserzeugung

| Methode                          | Beschreibung                         |
|---------------------------------|-------------------------------------|
| `useRange(start, end)`          | Erzeugt numerischen Stream im Bereich [start, end) |
| `useRange(start, end, step)`    | Bereichserzeugung mit Schritt, unterstützt negativen Schritt |
| `useRangeClosed(start, end)`    | Erzeugt numerischen Stream im geschlossenen Bereich [start, end] |
| `useRangeClosed(start, end, step)` | Geschlossene Bereichserzeugung mit Schritt, unterstützt negativen Schritt |

### ♾️ Unendliche Stream-Erzeugung

| Methode                          | Beschreibung                     |
|---------------------------------|---------------------------------|
| `useInfinite(seed, generator)`  | Unendliche Iteration von Startwert |
| `useGenerate(supplier)`         | Unendliche Aufrufe an Lieferant      |
| `useGenerate(supplier, limit)`  | Begrenzte Anzahl Aufrufe an Lieferant |
| `useIterate(seed, generator)`   | Unendliche Iteration von Startwert    |
| `useIterate(seed, generator, limit)` | Begrenzte Iteration von Startwert |
| `useRandom()`                   | Unendlicher zufälliger Integer-Stream  |
| `useRandom(min, max)`           | Unendlicher Zufallszahlenstream in spezifiziertem Bereich (erkennt automatisch Integer/Float) |
| `useRandom(min, max, count)`    | Zufallszahlenstream mit spezifiziertem Bereich und Anzahl |

### 📦 Container- und Elementkonstruktion

| Methode                | Beschreibung                             |
|-----------------------|-----------------------------------------|
| `useEmpty()`          | Erzeugt einen leeren Stream                 |
| `useOf(element)`      | Erzeugt einen Stream von einem einzelnen Element  |
| `useOf(e1, e2)`       | Erzeugt einen Stream von zwei Elementen      |
| `useOf(e1, e2, e3)`   | Erzeugt einen Stream von drei Elementen    |
| `useOf({...})`        | Erzeugt einen Stream von einer Initialisierungsliste |
| `useFrom(container)`  | Erzeugt einen Stream von einem beliebigen Standardcontainer (unterstützt Move-Semantik) |
| `useFrom({...})`      | Erzeugt einen Stream von einer Initialisierungsliste |
| `useRepeat(element, count)` | Wiederholt das spezifizierte Element n-mal |

### 📄 Textverarbeitung

| Methode                    | Beschreibung                         |
|---------------------------|-------------------------------------|
| `useBlob(text)`           | Teilt String in char-Stream Byte für Byte |
| `useBlob(text, start, end)` | Teilt spezifizierten Bereich eines Strings Byte für Byte |
| `useBlob(istream)`        | Liest von Eingabestream Zeile für Zeile     |
| `useBlob(istream, delimiter)` | Liest von Eingabestream nach Trennzeichen |
| `useText(text)`           | Behandelt String als ganzen Textstream (Charsequence) |
| `useText(text, delimiter)` | Teilt Text nach Trennzeichen           |
| `useText(istream)`        | Liest gesamten Inhalt von Eingabestream |
| `useText(istream, delimiter)` | Liest von Eingabestream nach Trennzeichen |

### 🌍 Unicode-Verarbeitung

| Methode                          | Beschreibung                                   |
|---------------------------------|-----------------------------------------------|
| `useSequence(charsequence)`     | Erzeugt Codepoint-Stream von Zeichensequenz |
| `useSequence(charsequence, start, end)` | Erzeugt Codepoint-Stream von spezifiziertem Bereich der Zeichensequenz |
| `useSequence(text, encoding)`   | Erzeugt Codepoint-Stream von Text mit spezifizierter Codierung |
| `useSequence(istream, encoding)` | Erzeugt Codepoint-Stream von Eingabestream mit spezifizierter Codierung |
| `useCharsequence(charsequence)` | Behandelt Zeichensequenz als ganzen Stream     |
| `useCharsequence(charsequence, delimiter)` | Teilt Zeichensequenz nach Trennzeichen |
| `useCharsequence(istream, encoding)` | Liest gesamte Zeichensequenz von Eingabestream |
| `useCharsequence(istream, delimiter, encoding)` | Liest Zeichensequenz von Eingabestream nach Trennzeichen |

---

## 🔐 Schicht 8: hash.h / less.h — Die universelle Sprache der Container-Welt

Wenn Sie `distinct()` auf `Semantic<std::vector<E>>` aufrufen, benötigt es intern `std::unordered_set<std::vector<E>>`. Die Standardbibliothek stellt keine Hash-Spezialisierungen für `std::vector` bereit – dies ist eine "Leerstelle", die vom C++-Standardkomitee hinterlassen wurde. Semantic-Cpps `hash.h` und `less.h` schließen diese Lücke vollständig: Bereitstellung vollständiger Hash- und Vergleichsunterstützung für alle Standardbibliothekscontainer (einschließlich verschachtelter Container), Paare, Tupel, Optionals, Varianten, Chrono-Zeittypen, komplexe Zahlen usw. Container beliebiger Tiefe und beliebiger Kombinationen können nun Schlüssel in unordered_sets oder Elemente in sets sein. 🌉

---

## 🚀 Leistungsoptimierungsempfehlungen

1.  **Wählen Sie den richtigen Container:** Verwenden Sie `toUnordered()`, wenn die Reihenfolge keine Rolle spielt, `toOrdered()` oder `sort()`, wenn Sortierung benötigt wird.
2.  **Nutzen Sie Parallelität gut:** Verwenden Sie `parallel()` für große Datensätze oder zeitaufwändige Verarbeitungslogik, vermeiden Sie blockierende E/A.
3.  **Optimieren Sie die Operationsreihenfolge:** `filter` früh, `sort` weise.
4.  **Nutzen Sie faule Auswertung:** Zwischenoperationen führen nicht sofort aus; `takeWhile` und `limit` können frühzeitig beenden.

---

Semantic-Cpp — Erstellen effizienter, klarer Datenverarbeitungspipelines mit modernem C++. 🚀🎯✨
