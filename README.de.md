# 🚀 Semantic-Cpp: Ein zukunftsorientiertes intelligentes Stream-Processing-Framework für C++

Semantic-Cpp ist eine moderne C++-Stream-Processing-Bibliothek, die von Grund auf neu entwickelt wurde und eine **"mehrere Header, null externe Abhängigkeiten"**-modulare Architektur aufweist. Jede Header-Datei hat eine klare, eindeutige Verantwortung und ist unabhängig testbar und bildet gemeinsam ein vollständiges Stream-Processing-Ökosystem. Diese Bibliothek kombiniert innovativ das Wesen mehrerer Programmierparadigmen:

*   **Die Eleganz und Flüssigkeit der Java Stream API**: Verkettete Aufrufe, deklarative Programmierung, die Code so elegant wie Poesie macht ✨
*   **Die Faulheit und Flexibilität von JavaScript Generatoren**: Lazy Evaluation, bedarfsgesteuerte Erzeugung, speicherfreundlich 🌱
*   **Die Effizienz und Ordnung von Datenbankindizierung**: Intelligente Sortierung, indexgesteuert, ein leistungsstarkes Werkzeug für die Verarbeitung von Zeitreihendaten ⏱️
*   **Die Batch-Verarbeitungsphilosophie von 'Container-als-Element'**: Vektoren, Listen, Maps... Jeder Container kann ein Erstklassen-Bürger im Stream sein und frei fließen 📦

Haben Sie es satt, `for`-Schleifen zu schreiben, um einen `vector` zu durchlaufen, ein `if` zur Filterung zu verschachteln und manuell `push_back` auf einen anderen Container aufzurufen? 😩
Haben Sie schon einmal spät in der Nacht einen Off-by-One-Index-Fehler debuggt, nur weil Sie das "dritte Element vom Ende" bei der Rückwärtsiteration wollten? 😵💫
Sehnen Sie sich danach, Daten wie eine Datenbank zu manipulieren – punktgenaues Finden per Index, Analyse mit gleitenden Fenstern, den gesamten Weg von den Daten zur Statistik mit einem einzigen Aufruf zu absolvieren? 🤔

**Semantic-Cpp wurde genau dafür geschaffen. 🔧**

Es abstrahiert die Datenverarbeitung als Operationen auf "Elementen" und ihren "logischen Positionen (Indizes)" – ähnlich wie "Zeilen" und "Primärschlüssel" in einer Datenbank. Sie können Indizes frei umordnen, verschieben und umkehren, ohne die Daten selbst zu berühren; Sie können auch jeden Container (`vector`, `map`, `array`...) als unteilbares Ganzes innerhalb des Streams übergeben und ihn jederzeit wieder auf Elementebene "auspacken". Diese Fähigkeit, frei zwischen zwei Granularitäten zu wechseln, fehlt in traditionellen Stream-Frameworks. 🎯

---

## 🏗️ Projektarchitektur: Siebenschichtiges modulares Design

Semantic-Cpp besteht aus sieben Kern-Header-Dateien, die schichtweise aufgebaut sind. Jede Datei hat eine einzige Verantwortung und ist unabhängig testbar. Fünf Namensräume, jeder mit seiner eigenen Zuständigkeit, arbeiten zusammen, um eine vollständige Pipeline von der Datenquelle zum Endergebnis zu bilden:

```
┌─────────────────────────────────────────────────┐
│               🌊 semantics.h                     │
│   Namensraum: semantic                           │
│   Stream-Erzeugungs-Fabriken: numerische Bereiche,│
│   Container, Text, Unicode                       │
├─────────────────────────────────────────────────┤
│                 📦 semantic.h                   │
│   Namensraum: semantic / collectable            │
│   Stream-Zwischenoperationen, Collectable-      │
│   System, Container-Entpackungsunterstützung    │
├─────────────────────────────────────────────────┤
│                ⚙️ collector.h                   │
│   Namensraum: collector                          │
│   Collector-Framework + Fabriken: Matching,     │
│   Finden, Aggregation, Statistik, DFT/FFT      │
├─────────────────────────────────────────────────┤
│                🔤 charsequence.h               │
│   Namensraum: charsequence                      │
│   Unicode-Zeichenfolgen, Multi-Encoding-        │
│   Konvertierung, Builder, Buffer                │
├─────────────────────────────────────────────────┤
│                🧵 pool.h                        │
│   Namensraum: pool                              │
│   Globaler Thread-Pool: Aufgabenabgabe,         │
│   Notabschaltung, Ausnahmepropagation          │
├─────────────────────────────────────────────────┤
│                📄 function.h                    │
│   Namensraum: function                          │
│   Typdefinitionen: Aliase für Generator,        │
│   Supplier, Consumer, usw.                     │
├─────────────────────────────────────────────────┤
│               🔐 hash.h / less.h               │
│   Namensraum: std (Erweiterungen)               │
│   Standardbibliotheks-Container-Hash & Vergleich│
│   Spezialisierungen, Unterstützung beliebiger  │
│   Verschachtelung                               │
└─────────────────────────────────────────────────┘
```

### 🧩 Abhängigkeitsgraph
Die Abhängigkeitskette ist klar und logisch, wie ein sorgfältig entworfener Schaltplan: Der Strom fließt von den grundlegenden Typdefinitionen nach oben, wobei jede Schicht nur von den darunterliegenden Schichten abhängt. Letztendlich laufen alle Pfade in `semantic.h` und `semantics.h` zusammen und bilden die vollständige Stream-Processing-Fähigkeit.

```
function.h          ← Keine Abhängigkeiten, die Typgrundlage
pool.h              ← Hängt von function.h ab
charsequence.h      ← Unabhängiges Modul, Unicode-Verarbeitung
collector.h         ← Hängt von function.h, pool.h ab
hash.h / less.h     ← Unabhängige Module, Standardbibliotheks-Erweiterungen
semantic.h          ← Hängt von allen oben Genannten ab
semantics.h         ← Hängt von semantic.h ab
```

---

## 🌍 Namensraum-Überblick

Semantic-Cpp entwirft sorgfältig fünf Namensräume, jeder wie eine unabhängige "Abteilung" mit klaren Zuständigkeiten, die jedoch eng zusammenarbeiten:

| Namensraum    | Header-Datei       | Zuständigkeit                                 | Kern-Typen/Funktionen                                                       |
| :------------ | :----------------- | :------------------------------------------- | :------------------------------------------------------------------------- |
| function      | function.h         | Typsystem-Grundlage                          | `Timestamp`, `Module`, `Generator<T>`, `Supplier<R>`, `Consumer<T>`, `Predicate<T>` usw. |
| pool          | pool.h             | Nebenläufige Ausführungs-Engine              | `pool::pool` (globaler Thread-Pool), `submit()`, `emergencyShutdown()`     |
| charsequence  | charsequence.h     | Unicode-String-Verarbeitung                  | `charset`, `Meta`, `Point`, `Charsequence`, `Builder`, `Buffer` usw.       |
| collector     | collector.h        | Terminale Sammeloperationen                  | `Collector<E,A,R>`, `Identity<A>`, `Accumulator<A,E>` usw.                 |
| collectable   | semantic.h         | Materialisierte Datencontainer               | `Collectable<E>`, `OrderedCollectable<E>`, `UnorderedCollectable<E>` usw.  |
| semantic      | semantic.h<br>semantics.h | Stream-Erzeugung & Zwischenoperationen | `Semantic<E>`, `useRange()`, `useFrom()` usw.                              |

### 🔁 Namensraum-Kollaborationsfluss
Der Datenfluss zwischen Namensräumen ist wie eine Fließbandfertigung in einer Fabrik – Rohmaterial tritt von `semantic` ein, durchläuft schrittweise die Verarbeitung und wird schließlich von `collector` verpackt und versandt. Jeder Schritt hat eine klare Verantwortungsgrenze:

```cpp
semantic::useRange(0, 100)          // ← semantic Namensraum: Stream erzeugen
    .map(int x { return x * 2; })   // ← semantic Namensraum: Zwischentransformation
    .filter(int x { return x > 50; }) // ← semantic Namensraum: Zwischenfilter
    .toUnordered()                  // ← In collectable Namensraum konvertieren
    .toVector();                    // ← Collector aus collector Namensraum aufrufen
```

---

## 📦 Schicht 1: function.h — Typgrundlage

`function.h` definiert das Typsystem für das gesamte Framework, die gemeinsame Grundlage aller Module. 🔑

```cpp
namespace function {
    using Timestamp = long long;           // Indextyp, der "Zeitstempel" der Daten im Stream
    using Module = unsigned long long;     // Modul-/Zähltyp
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept — ein Element empfangen
        std::function<bool(T, Timestamp)>       // interrupt — sollen wir anhalten?
    )>;
}
```

`Generator` ist die Kernabstraktion des gesamten Stream-Systems. 🌀 Es gibt keine Daten zurück; stattdessen akzeptiert es zwei Callbacks – `accept` ("Ich bin bereit, bitte akzeptiere dieses Element") und `interrupt` ("sollen wir anhalten?"). Dieses Inversion-of-Control-Design bedeutet, dass der Datenproduzent nichts über den Consumer weiß; er "schiebt" Daten einfach zum geeigneten Zeitpunkt. Das ist die Essenz von Lazy Evaluation: Daten "fließen" erst wirklich, wenn `accept` aufgerufen wird; davor ist alles nur eine Beschreibung.

| Typ-Alias         | Vollständige Definition                              | Zweck                                 |
| :---------------- | :--------------------------------------------------- | :------------------------------------ |
| Timestamp         | long long                                            | Logische Position eines Elements im Stream |
| Module            | unsigned long long                                   | Zählen, Kapazität, Nebenläufigkeitsgrad |
| Runnable          | std::function<void()>                                | Parameterlose, void-rückgebende Aufgabe |
| Supplier<R>       | std::function<R()>                                   | Lieferant, erzeugt aus dem Nichts     |
| Function<T,R>     | std::function<R(T)>                                  | Einzelargument-Funktion               |
| BiFunction<T,U,R> | std::function<R(T,U)>                                | Zwei-Argument-Funktion                |
| TriFunction<T,U,V,R> | std::function<R(T,U,V)>                           | Drei-Argument-Funktion                |
| Unary<T>          | std::function<T(T)>                                  | Unäre Operation                       |
| Binary<T>         | std::function<T(T,T)>                                | Binäre Operation                      |
| Consumer<T>       | std::function<void(T)>                               | Consumer (Verbraucher)                |
| BiConsumer<T,U>   | std::function<void(T,U)>                             | Zwei-Argument-Consumer                |
| TriConsumer<T,U,V>| std::function<void(T,U,V)>                           | Drei-Argument-Consumer                |
| Predicate<T>      | std::function<bool(T)>                               | Prädikat-Beurteilung                  |
| BiPredicate<T,U>  | std::function<bool(T,U)>                             | Zwei-Argument-Prädikat                |
| TriPredicate<T,U,V>| std::function<bool(T,U,V)>                          | Drei-Argument-Prädikat                |
| Comparator<T>     | std::function<int(const T&,const T&)>                | Komparator, gibt negativ/null/positiv zurück |
| Generator<T>      | BiConsumer<BiConsumer<T,Timestamp>,<br>BiPredicate<T,Timestamp>> | Kernabstraktion für Stream-Erzeugung |

---

## 🧵 Schicht 2: pool.h — Nebenläufigkeitsgrundlage

`pool.h` stellt den globalen Thread-Pool `pool::pool` bereit, die Nebenläufigkeits-Engine für das gesamte Framework. 🚀 Es verwendet ein **deklaratives Parallelismus**-Design – wenn Sie `.parallel(4)` schreiben, startet es nicht sofort vier Threads zur Verarbeitung. Diese Codezeile ist lediglich eine "Deklaration": Sie teilt dem Framework mit, "dass ich 4 Threads zur parallelen Verarbeitung verwenden möchte". Die tatsächliche parallele Ausführung erfolgt, wenn eine terminale Operation aufgerufen wird – also wenn Sie Sammelmethoden wie `toVector()`, `findFirst()`, `count()` usw. aufrufen.

| Merkmal             | Beschreibung                                                               |
| :------------------ | :------------------------------------------------------------------------- |
| Deklarativer Parallelismus | `.parallel(4)` deklariert nur "ich möchte 4 Threads verwenden", startet nicht sofort |
| Notabschaltung      | Eingebaute `emergencyShutdown()` und `std::set_terminate`-Handler         |
| Ausnahmepropagation | `submit()` gibt `std::future` zurück, Ausnahmen werden sicher an den Hauptthread weitergegeben |

---

## 🔤 Schicht 3: charsequence.h — Unicode-Zeichenfolgen

`charsequence.h` ist ein vollständiges Unicode-Verarbeitungsmodul, das Funktionalität zum Erstellen, Konvertieren und Bearbeiten von Zeichenfolgen bereitstellt. 🌍 Es unterstützt verschiedene Kodierungen wie UTF-8, UTF-16 (LE/BE), UTF-32 (LE/BE), ASCII und Latin1. Es erkennt und verarbeitet Ersatzpaare korrekt und gibt für ungültige Codepoints das standardmäßige U+FFFD-Ersatzzeichen zurück.

| Typ/Funktion    | Beschreibung                                                               |
| :-------------- | :------------------------------------------------------------------------- |
| charset         | Enum: ascii, utf8, utf16, utf16be, utf16le, utf32, utf32be, utf32le, latin1 |
| Meta            | Metadaten-Wrapper, speichert vorzeichenlose Ganzzahlwerte                  |
| Point           | Unicode-Codepoint, unterstützt Ersatzpaarerkennung und Gültigkeitsprüfungen |
| Charsequence    | Unveränderliche Zeichenfolge: split, replace, indexOf, lastIndexOf, sub, trim, toUpperCase, toLowerCase, reverse, startsWith, endsWith, contains, compare, getBytes, getPoints, getMetas, getCharacters, repeat, concat, count, join |
| Builder         | Veränderlicher Byte-Builder: prepend, insert, append (unterstützt Grundtypen, Point, Charsequence, string_view) |
| Buffer          | Thread-sicherer Ringpuffer: write, read, peek, prepend, append, clear, shrinkToFit, data, size, capacity, atomic |
| PointIterator   | Bidirektionaler Iterator zum Durchlaufen von Unicode-Codepoints            |
| encode()        | Kodiert einen einzelnen Codepoint in eine Bytefolge der angegebenen Kodierung |
| decode()        | Dekodiert den nächsten Codepoint aus einer Bytefolge, schiebt Zeiger automatisch vor |
| convert()       | Kodierungskonvertierung (unterstützt string, vector, deque als Ausgabe)    |

---

## ⚙️ Schicht 4: collector.h — Collector-Framework & Fabriken

`collector.h` ist das Kern-Collector-Modul von Semantic-Cpp und vereint das Collector-Framework mit Fabrikfunktionen.

### 🧩 Fünf-Stufen-Modell
```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (optionaler Kurzschluss)
```

| Typ-Alias       | Vollständige Definition                        | Rolle                           |
| :-------------- | :--------------------------------------------- | :------------------------------ |
| Identity<A>     | function::Supplier<A>                          | Liefert Startwert               |
| Accumulator<A,E>| function::TriFunction<A, E, Timestamp, A>      | Akkumuliert Elemente            |
| Combiner<A>     | function::BiFunction<A, A, A>                  | Kombiniert parallele Ergebnisse |
| Finisher<A,R>   | function::Function<A, R>                       | Finale Transformation           |
| Interrupt<E,A>  | function::TriPredicate<E, Timestamp, A>        | Kurzschluss-Beurteilung         |

### Collector-Fabrikfunktionen

#### ✅ Vergleichsoperationen
| Methode                     | Beschreibung              | Rückgabetyp |
| :------------------------- | :------------------------ | :---------- |
| `useAllMatch(predicate)`   | Alle Elemente erfüllen Bedingung | `bool`      |
| `useAnyMatch(predicate)`   | Ein Element erfüllt Bedingung | `bool`      |
| `useNoneMatch(predicate)`  | Kein Element erfüllt Bedingung | `bool`      |

#### 🔍 Suchoperationen
| Methode             | Beschreibung                    | Rückgabetyp       |
| :----------------- | :------------------------------ | :---------------- |
| `useFindFirst()`   | Finde das erste Element         | `std::optional<E>` |
| `useFindLast()`    | Finde das letzte Element        | `std::optional<E>` |
| `useFindAny()`     | Finde ein (beliebiges) Element  | `std::optional<E>` |
| `useFindAt(index)` | Finde an spezifischem Index (unterstützt negativ) | `std::optional<E>` |
| `useFindMaximum()` | Finde maximales Element         | `std::optional<E>` |
| `useFindMinimum()` | Finde minimales Element         | `std::optional<E>` |

#### 🔢 Aggregationsoperationen
| Methode            | Beschreibung  | Rückgabetyp |
| :---------------- | :----------- | :---------- |
| `useCount()`      | Gesamtzahl der Elemente | `Module` |
| `useSummate<E,D>()`| Summation    | `D`         |
| `useAverage<E,D>()`| Durchschnitt | `D`         |
| `useRange<E,D>()` | Zahlenbereich (max - min) | `D` |

#### 📉 Statistische Operationen
| Methode                        | Beschreibung                | Rückgabetyp         |
| :---------------------------- | :------------------------- | :------------------ |
| `useVariance<E,D>()`          | Populationsvarianz          | `D`                 |
| `useStandardDeviation<E,D>()` | Populationsstandardabweichung | `D`              |
| `useSkewness<E,D>()`          | Schiefe                    | `D`                 |
| `useKurtosis<E,D>()`          | Kurtosis (Wölbung)         | `D`                 |
| `useMedian<E,D>()`            | Median                     | `std::optional<D>`  |
| `useMode<E>()`                | Modus (Häufigkeitsanalyse) | `std::optional<E>`  |
| `usePercentile<E,D>(p)`       | p-tes Perzentil           | `std::optional<D>`  |
| `useFrequency<E>()`           | Frequenzbereichsmerkmale   | `std::map<E, complex>` |
| `useDistribution<E>()`        | Räumliche Verteilungsmerkmale | `std::map<E, complex>` |

#### 🔀 Reduktionsoperationen
| Methode                          | Beschreibung          | Rückgabetyp       |
| :------------------------------ | :------------------- | :---------------- |
| `useReduce(reducer)`            | Reduktion ohne Identität | `std::optional<E>` |
| `useReduce(identity, reducer)` | Reduktion mit Identität | `E`              |
| `useReduce(id, red, comb, fin)`| Vollständig benutzerdefinierte Reduktion | `R` |

#### 🧺 Sammlung in Container
| Methode                                                              | Rückgabetyp |
| :------------------------------------------------------------------ | :---------- |
| `useToVector()`                                                     | `std::vector<E>` |
| `useToList()`                                                       | `std::list<E>` |
| `useToDeque()`                                                      | `std::deque<E>` |
| `useToForwardList()`                                                | `std::forward_list<E>` |
| `useToArray<N>()`                                                   | `std::array<E, N>` |
| `useToSet()`                                                        | `std::set<E>` |
| `useToMultiset()`                                                   | `std::multiset<E>` |
| `useToUnorderedSet()`                                               | `std::unordered_set<E>` |
| `useToUnorderedMultiset()`                                          | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)`                                            | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)`                            | `std::map<K, V>` |
| `useToMultimap(keyExtractor)`                                       | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)`                       | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)`                   | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)`                              | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)`              | `std::unordered_multimap<K, V>` |
| `useToStack()`                                                      | `std::stack<E>` |
| `useToQueue()`                                                      | `std::queue<E>` |
| `useToPriorityQueue()`                                              | `std::priority_queue<E>` |

#### 🧩 Gruppierungs- & Partitionierungsoperationen
| Methode                                          | Rückgabetyp |
| :---------------------------------------------- | :---------- |
| `useGroup(keyExtractor)`                        | `std::unordered_map<K, vector<E>>` |
| `useGroupBy(keyExtractor, valueExtractor)`     | `std::unordered_map<K, vector<V>>` |
| `usePartition(size)`                            | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)`                  | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor, valueExtractor)`  | `std::vector<vector<V>>` |

#### 📄 String-Ausgabe
| Methode                               | Rückgabetyp |
| :----------------------------------- | :---------- |
| `useJoin()` / `useOut()` / `useError()` und deren Überladungen | `charsequence::Charsequence` |

#### 📊 Mathematische Werkzeuge
| Methode            | Rückgabetyp |
| :---------------- | :---------- |
| `useDFT()`        | `vector<complex<double>>` |
| `useIDFT()`       | `vector<complex<double>>` |
| `useFFT()`        | `vector<complex<double>>` |
| `useIFFT()`       | `vector<complex<double>>` |
| `useGradient()`   | `vector<double>` |

---

## 📦 Schicht 5: semantic.h — Stream-Zwischenoperationen & Sammelsystem

### 🧩 Kerndesign: Drei-Stufen-Pipeline
```
Semantic<E> (Konstruktion & Transformation)
    ↓ toUnordered() / toOrdered() / sort() / toWindow() / toStatistics()
Collectable<E> (Materialisierung & Sammlung)
    ↓ toVector() / findFirst() / count() / summate() / ...
Endergebnis
```

**Wichtige Regel**: Ein `Semantic<E>` muss zuerst über `toUnordered()`, `toOrdered()`, `toWindow()`, `toStatistics()` oder `sort()` in ein `Collectable<E>` konvertiert werden, bevor terminale Methoden aufgerufen werden können.

### 🧭 Fünf Materialisierungspfade
| Konvertierungsmethode   | Zieltyp               | Zugrundeliegende Datenstruktur | Leistungsmerkmal         |
| :--------------------- | :------------------- | :---------------------------- | :----------------------- |
| `toUnordered()`        | `UnorderedCollectable` | `unordered_map`              | Durchschnittlich O(1) Nachschlag |
| `toOrdered()`          | `OrderedCollectable`  | `map`                        | O(log n) Nachschlag      |
| `sort()`               | `OrderedCollectable`  | `map` (wertesortiert)        | O(log n) Nachschlag      |
| `toWindow()`           | `WindowCollectable`   | Erbt geordnete Sammlung       | Unterstützt slide/tumble |
| `toStatistics<D>()`    | `Statistics<E,D>`     | Erbt geordnete Sammlung       | 30+ statistische Methoden |

### 📋 Collectable<E> — Alle terminalen Methoden (alphabetisch geordnet)
| Methode                                              | Rückgabetyp                 | Beschreibung                                     |
| :-------------------------------------------------- | :------------------------- | :---------------------------------------------- |
| `allMatch(predicate)`                               | `bool`                     | Alle Elemente erfüllen Bedingung                |
| `anyMatch(predicate)`                               | `bool`                     | Ein Element erfüllt Bedingung                   |
| `average<D>()`                                      | `D`                        | Durchschnitt                                    |
| `average<D>(mapper)`                                | `D`                        | Durchschnitt nach Mapping                        |
| `collect(identity, acc, comb, fin)`                | `R`                        | Benutzerdefinierte Vier-Stufen-Sammlung         |
| `collect(identity, interrupt, acc, comb, fin)`      | `R`                        | Benutzerdefinierte unterbrechbare Sammlung      |
| `count()`                                           | `Module`                   | Gesamtzahl der Elemente                         |
| `empty()`                                           | `bool`                     | Ist der Stream leer?                            |
| `error()`                                           | `void`                     | Ausgabe nach stderr (unterstützt delimiter/prefix/suffix/converter) |
| `findAny()`                                         | `std::optional<E>`         | Finde ein (beliebiges) Element                  |
| `findAt(index)`                                     | `std::optional<E>`         | Finde Element an spezifischem Index (unterstützt negativ) |
| `findFirst()`                                       | `std::optional<E>`         | Finde das erste Element                         |
| `findLast()`                                        | `std::optional<E>`         | Finde das letzte Element                        |
| `findMaximum()`                                     | `std::optional<E>`         | Finde das maximale Element                      |
| `findMaximum(comparator)`                           | `std::optional<E>`         | Finde Maximum mit benutzerdefiniertem Komparator |
| `findMinimum()`                                     | `std::optional<E>`         | Finde das minimale Element                      |
| `findMinimum(comparator)`                           | `std::optional<E>`         | Finde Minimum mit benutzerdefiniertem Komparator |
| `forEach(consumer)`                                 | `void`                     | Führe Seiteneffekt für jedes Element aus        |
| `group(keyExtractor)`                               | `unordered_map<K, vector<E>>` | Gruppiere nach Schlüssel                     |
| `groupBy(keyExtractor, valueExtractor)`            | `unordered_map<K, vector<V>>` | Gruppiere nach Schlüssel und extrahiere Wert  |
| `join()`                                            | `Charsequence`              | Verbinde mit Standardformat                     |
| `join(delimiter)`                                   | `Charsequence`              | Verbinde mit benutzerdefiniertem Trennzeichen   |
| `join(prefix, delimiter, suffix)`                  | `Charsequence`              | Verbinde mit vollständig benutzerdefiniertem Format |
| `noneMatch(predicate)`                              | `bool`                     | Kein Element erfüllt Bedingung                  |
| `out()`                                             | `Charsequence`              | Ausgabe nach stdout (unterstützt delimiter/prefix/suffix/converter) |
| `partition(size)`                                   | `vector<vector<E>>`         | Partitioniere nach fester Größe                 |
| `partitionBy(keyExtractor)`                         | `vector<vector<E>>`         | Partitioniere nach Indexschlüssel               |
| `partitionBy(keyExtractor, valueExtractor)`         | `vector<vector<V>>`         | Partitioniere nach Indexschlüssel und extrahiere Wert |
| `range<D>()`                                        | `D`                        | Zahlenbereich (max - min)                       |
| `range<D>(mapper)`                                  | `D`                        | Zahlenbereich nach Mapping                      |
| `reduce(accumulator)`                               | `std::optional<E>`         | Reduktion ohne Identität                        |
| `reduce(identity, accumulator)`                     | `E`                        | Reduktion mit Identität                         |
| `reduce(identity, acc, comb)`                       | `R`                        | Vollständig benutzerdefinierte Reduktion        |
| `summate<D>()`                                      | `D`                        | Summation                                       |
| `summate<D>(mapper)`                                | `D`                        | Summation nach Mapping                          |
| `toArray<N>()`                                      | `std::array<E, N>`         | Sammle in Array fester Größe                    |
| `toDeque()`                                         | `std::deque<E>`            | Sammle in deque                                 |
| `toForwardList()`                                   | `std::forward_list<E>`     | Sammle in forward_list                          |
| `toList()`                                          | `std::list<E>`             | Sammle in list                                  |
| `toMap(keyExtractor)`                               | `std::map<K, E>`           | Sammle in map nach Schlüssel                    |
| `toMap(keyExtractor, valueExtractor)`               | `std::map<K, V>`           | Sammle in map mit benutzerdefiniertem Schlüssel & Wert |
| `toMultimap(keyExtractor)`                          | `std::multimap<K, E>`      | Sammle in multimap nach Schlüssel               |
| `toMultimap(keyExtractor, valueExtractor)`          | `std::multimap<K, V>`      | Sammle in multimap mit benutzerdefiniertem Schlüssel & Wert |
| `toMultiset()`                                      | `std::multiset<E>`         | Sammle in multiset                              |
| `toPriorityQueue()`                                 | `std::priority_queue<E>`   | Sammle in priority_queue                        |
| `toQueue()`                                         | `std::queue<E>`            | Sammle in queue                                 |
| `toSet()`                                           | `std::set<E>`              | Sammle in set (eindeutig & sortiert)            |
| `toStack()`                                         | `std::stack<E>`            | Sammle in stack                                 |
| `toUnorderedMap(keyExtractor, valueExtractor)`      | `std::unordered_map<K, V>` | Sammle in unordered_map                         |
| `toUnorderedMultimap(keyExtractor)`                 | `std::unordered_multimap<K, E>`| Sammle in unordered_multimap nach Schlüssel |
| `toUnorderedMultimap(keyExtractor, valueExtractor)` | `std::unordered_multimap<K, V>`| Sammle in unordered_multimap mit benutzerdefiniertem Schlüssel & Wert |
| `toUnorderedMultiset()`                             | `std::unordered_multiset<E>`| Sammle in unordered_multiset                  |
| `toUnorderedSet()`                                  | `std::unordered_set<E>`    | Sammle in unordered_set                         |
| `toVector()`                                        | `std::vector<E>`           | Sammle in vector                                |

### 📈 Statistics<E,D> — Statistische Methoden
| Methode               | Rückgabetyp           | Beschreibung                     |
| :------------------- | :------------------- | :------------------------------ |
| `summate()`          | `D`                  | Summation                       |
| `average()`          | `D`                  | Durchschnitt                    |
| `minimum()`          | `std::optional<D>`  | Minimalwert                     |
| `maximum()`          | `std::optional<D>`  | Maximalwert                     |
| `range()`            | `D`                  | Bereich (max - min)             |
| `variance()`         | `D`                  | Populationsvarianz              |
| `standardDeviation()`| `D`                  | Populationsstandardabweichung    |
| `median()`           | `std::optional<D>`  | Median                          |
| `mode()`             | `std::optional<E>`  | Modus                           |
| `percentile(p)`      | `std::optional<D>`  | p-tes Perzentil                 |
| `firstQuartile()`    | `std::optional<D>`  | Erstes Quartil (Q1)             |
| `thirdQuartile()`    | `std::optional<D>`  | Drittes Quartil (Q3)            |
| `interquartileRange()`| `std::optional<D>` | Interquartilsabstand (IQR)      |
| `skewness()`         | `D`                  | Schiefe                         |
| `kurtosis()`         | `D`                  | Kurtosis (Wölbung)              |
| `frequency()`        | `map<E, complex>`   | Frequenzbereichsmerkmale        |
| `distribute()`       | `map<E, complex>`   | Räumliche Verteilungsmerkmale   |
| `dft()`              | `vector<complex<double>>` | Diskrete Fourier-Transformation |
| `idft()`             | `vector<complex<double>>` | Inverse Diskrete Fourier-Transformation |
| `fft()`              | `vector<complex<double>>` | Schnelle Fourier-Transformation |
| `ifft()`             | `vector<complex<double>>` | Inverse Schnelle Fourier-Transformation |
| `gradient(...)`      | `vector<double>`    | Gradientenabstieg               |

Alle oben genannten Methoden unterstützen auch eine optionale `mapper`-Parameterversion.

### 🔧 Semantic<E> Zwischenoperationsmethoden
| Kategorie      | Methode        | Beschreibung                                   |
| :------------ | :------------ | :-------------------------------------------- |
| Elementtransformation | map         | Eins-zu-eins-Mapping-Transformation           |
|               | flatMap     | Eins-zu-viele-Mapping und Abflachung          |
|               | flat        | Verschachtelte Streams abflachen (unterstützt Semantic und Container) |
| Elementfilter | filter      | Bedingte Filterung                            |
|               | takeWhile   | Nimm solange Bedingung erfüllt                |
|               | dropWhile   | Verwerfe solange Bedingung erfüllt            |
|               | distinct    | Duplikate entfernen (unterstützt benutzerdefinierten Komparator) |
| Größenkontrolle | limit       | Anzahl der Elemente begrenzen                 |
|               | skip        | Erste n Elemente überspringen                 |
|               | sub         | Teilbereich extrahieren [start, end)          |
| Indexoperationen | redirect    | Indizes neu zuordnen                          |
|               | reverse     | Indizes umkehren                              |
|               | translate   | Indizes verschieben                           |
| Beobachtung   | peek        | Jedes Element beobachten (ändert Stream nicht) |
| Parallele Deklaration | parallel(n) | Nebenläufigkeitsgrad deklarieren              |
| Verkettung    | concatenate | Semantic/Elemente/Generatoren/Container verketten |
| Terminale Konvertierung | toUnordered / toOrdered / toWindow / toStatistics / sort | In Collectable konvertieren |

---

## 🔧 Schicht 6: semantics.h — Stream-Erzeugungs-Fabriken

### 🔢 Numerische Bereichserzeugung
| Methode                        | Beschreibung                     |
| :---------------------------- | :------------------------------ |
| `useRange(start, end)`       | Erzeuge Bereich [start, end)    |
| `useRange(start, end, step)` | Bereich mit Schrittweite (unterstützt negativ) |
| `useRangeClosed(start, end)` | Erzeuge geschlossenen Bereich [start, end] |
| `useRangeClosed(start, end, step)` | Geschlossener Bereich mit Schrittweite |

### ♾️ Unendliche Stromerzeugung
| Methode                      | Beschreibung                         |
| :-------------------------- | :--------------------------------- |
| `useInfinite(seed, generator)`| Unendliche Iteration ab Startwert  |
| `useGenerate(supplier)`     | Unendliche Aufrufe des Lieferanten |
| `useGenerate(supplier, limit)`| Begrenzte Anzahl Aufrufe des Lieferanten |
| `useIterate(seed, generator)` | Unendliche Iteration ab Startwert  |
| `useIterate(seed, generator, limit)` | Begrenzte Anzahl Iterationen  |
| `useRandom()`               | Unendlicher Strom von Zufallszahlen |
| `useRandom(min, max)`       | Zufallszahlenstrom im angegebenen Bereich |
| `useRandom(min, max, count)`| Zufallszahlenstrom mit angegebenem Bereich und Anzahl |

### 📦 Container- & Elementerzeugung
| Methode                  | Beschreibung                     |
| :---------------------- | :----------------------------- |
| `useEmpty()`            | Leeren Stream erzeugen         |
| `useOf(element)`        | Stream aus einem einzelnen Element erzeugen |
| `useOf(e1, e2)`         | Stream aus zwei Elementen erzeugen |
| `useOf(e1, e2, e3)`     | Stream aus drei Elementen erzeugen |
| `useOf({...})`          | Stream aus Initialisierungsliste erzeugen |
| `useFrom(container)`    | Stream aus Standardcontainer erzeugen |
| `useFrom({...})`        | Stream aus Initialisierungsliste erzeugen |
| `useRepeat(element, count)` | Element n-mal wiederholen     |

### 📄 Text- & Unicode-Verarbeitung
| Methode                      | Beschreibung                         |
| :-------------------------- | :--------------------------------- |
| `useBlob(text)`             | Teile String Byte für Byte in char-Stream auf |
| `useBlob(text, start, end)` | Teile spezifischen Bereich Byte für Byte auf |
| `useBlob(istream)`          | Lese zeilenweise von Eingabestream |
| `useBlob(istream, delimiter)` | Lese nach Trennzeichen von Eingabestream |
| `useText(text)`             | Gesamter Textstream (Charsequence)   |
| `useText(text, delimiter)`  | Teile Text nach Trennzeichen        |
| `useText(istream)`          | Lese gesamten Inhalt von Eingabestream |
| `useSequence(charsequence)` | Erzeuge Codepoint-Stream aus Zeichenfolge |
| `useSequence(text, encoding)` | Erzeuge Codepoint-Stream aus Text mit angegebener Kodierung |
| `useCharsequence(charsequence)` | Zeichenfolge als gesamter Stream |
| `useCharsequence(charsequence, delimiter)` | Teile Zeichenfolge nach Trennzeichen |

---

## 🔐 Schicht 7: hash.h / less.h — Die universelle Sprache der Container-Welt

Bietet vollständige Hash- und Vergleichsunterstützung für alle Standardbibliotheks-Container (einschließlich verschachtelter Container), `pair`, `tuple`, `optional`, `variant`, `chrono`-Zeittypen, `complex`-Zahlen und mehr. Container, die in beliebiger Tiefe und Kombination verschachtelt sind, können nun als Schlüssel in `unordered_set` oder Elemente in `set` verwendet werden. 🌉

---

## 🚀 Leistungsoptimierungstipps

1.  **Wählen Sie den richtigen Container**: Verwenden Sie `toUnordered()`, wenn die Reihenfolge keine Rolle spielt, `toOrdered()` oder `sort()`, wenn Sortierung benötigt wird.
2.  **Nutzung von Parallelismus**: Verwenden Sie `parallel()` für große Datensätze.
3.  **Optimieren Sie die Operationsreihenfolge**: Filtern Sie früh, sortieren Sie weise.
4.  **Nutzen Sie Lazy Evaluation**: `takeWhile` und `limit` können frühzeitig beenden.

---

Semantic-Cpp — Erstellen effizienter, klarer Datenverarbeitungspipelines mit modernem C++. 🚀🎯✨
