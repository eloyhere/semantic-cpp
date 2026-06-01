# 🚀 Semantic‑Cpp: Eine Zukunftsorientierte C++‑Stream‑Verarbeitungsbibliothek

Semantic‑Cpp ist eine komplett neu konzipierte, moderne C++‑Bibliothek zur **Stream‑Verarbeitung**. Sie verfolgt eine **„Multi‑Header, Null‑Abhängigkeiten“**‑Architektur: Jede Header‑Datei besitzt eine klar definierte Verantwortlichkeit, ist unabhängig testbar und bildet gemeinsam ein vollständiges Ökosystem für datenstrombasierte Anwendungen. Die Bibliothek vereint auf kreative Weise die Stärken verschiedener Programmierparadigmen:

- 🎯 **Eleganz und Flüssigkeit der Java Stream API**: Verkettbare Aufrufe, deklarative Programmierung – Code liest sich wie Poesie  
- ⚡ **Laziness und Flexibilität von JavaScript Generatoren**: Verzögerte Auswertung (*lazy evaluation*), bedarfsgerechte Erzeugung, speichereffizient  
- 🗄️ **Effizienz und Ordnung von Datenbankindizes**: Intelligentes Sortieren, indexgesteuerte Verarbeitung – ideal für Zeitreihendaten  

Im Gegensatz zu herkömmlichen Ansätzen (manuell geschriebene Schleifen, asynchrone Callbacks) bietet Semantic‑Cpp eine **typsichere, ausdrucksstarke und leistungsfähige** Lösung. Das zentrale Designprinzip lautet **präzise Datenflusskontrolle**: Daten fließen nur dann, wenn sie benötigt werden; Reihenfolge und Position werden durch **Indizes** feingranular gesteuert – für optimale Ressourcennutzung.

---

## 📐 Projektarchitektur: Sieben‑Schichten‑Modell

Semantic‑Cpp besteht aus **sieben Kern‑Headern**, die schrittweise aufeinander aufbauen und jeweils eine eigene, testbare Verantwortlichkeit tragen:

```
┌─────────────────────────────────────────────────┐
│                  semantic.h                     │
│  (Zwischenoperationen, Containerspezialisierungen, Collectable)
├─────────────────────────────────────────────────┤
│                 semantics.h                     │
│   (Stream‑Builder, Fabrikmethoden, Textverarbeitung)
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│  (Collector‑Fabriken: count, group, reduce, DFT usw.)
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│   (Collector‑Framework: Fünf‑Phasen‑Modell, Concurrency)
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│  (Unicode‑Zeichensequenzen, Kodierung, Regex)
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│  (Thread‑Pool: Notabschaltung, Ausnahmeweitergabe)
├─────────────────────────────────────────────────┤
│                 function.h                      │
│   (Typdefinitionen: Generator, Supplier, Consumer usw.)
└─────────────────────────────────────────────────┘
```

### Abhängigkeitsgraph

```
function.h          ← keine Abhängigkeiten, Typfundament
pool.h              ← abhängig von function.h
charsequence.h      ← eigenständiges Modul, Unicode‑Verarbeitung
collector.h         ← abhängig von function.h, pool.h
collectors.h        ← abhängig von collector.h, charsequence.h
semantic.h          ← abhängig von collector.h, collectors.h, charsequence.h
semantics.h         ← abhängig von semantic.h
```

> Jeder Header kann separat kompiliert und getestet oder gezielt eingebunden werden.  
> Benötigen Sie beispielsweise nur Collector‑Funktionalität, genügt das Einbinden von `collector.h` und `collectors.h`.

---

## 🏗️ Schicht 1: `function.h` – Typfundament

`function.h` definiert das gesamte Typsystem der Bibliothek und bildet die gemeinsame Grundlage aller Module:

```cpp
namespace function {
    using Timestamp = long long;           // Indextyp
    using Module = unsigned long long;     // Modul‑ / Zähler‑Typ
    using Generator = BiConsumer<
        BiConsumer<T, Timestamp>,
        BiPredicate<T, Timestamp>
    >;
}
```

`Generator` ist die zentrale Abstraktion des gesamten Stream‑Systems: Er akzeptiert zwei Callbacks – `accept` (zum Empfangen von Daten) und `interrupt` (zum Abbrechen des Datenflusses) – und verkörpert das **„Lazy‑Pull“‑Modell**.

---

## ⚡ Schicht 2: `pool.h` – Concurrency‑Grundlage

`pool.h` stellt den globalen Thread‑Pool `pool::pool` bereit, der als Concurrency‑Motor der Bibliothek dient.

| Merkmal | Beschreibung |
|-------|--------------|
| 🎯 Deklarative Parallelität | `parallel(n)` deklariert lediglich die Absicht; Ausführung erfolgt bei Terminaloperationen |
| 🛡️ Notabschaltung | Integrierte `emergencyShutdown()`‑Funktion und `std::set_terminate`‑Handler |
| 🔄 Ausnahmeweitergabe | `submit()` liefert `std::future` und unterstützt sichere Ausnahmeweitergabe |

---

## 🔤 Schicht 3: `charsequence.h` – Unicode‑Zeichensequenzen

`charsequence.h` ist ein vollwertiges Unicode‑Verarbeitungsmodul.

| Funktion | Beschreibung |
|--------|--------------|
| 🌐 Mehrfachkodierung | UTF‑8, UTF‑16, UTF‑32, Latin‑1, GBK usw. |
| 🔍 Codepoint‑Iterator | `PointIterator` unterstützt bidirektionale Traversierung |
| 🏗️ Builder‑Muster | `Builder`‑Klasse für effizientes String‑Concatenation |
| 📐 Reguläre Ausdrücke | `Regex`‑Klasse kapselt `std::regex` |
| 🔑 Hash & Vergleich | Alle Kerntypen sind für `std::hash` und `std::less` spezialisiert |

---

## 🔧 Schicht 4: `collector.h` – Collector‑Framework

`collector.h` implementiert das Collector‑Pattern und bildet den Kern der Terminaloperationen.

### Fünf‑Phasen‑Modell

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (optionaler Kurzschluss)
```

### Typaliase

| Typ | Definition | Rolle |
|----|-----------|------|
| `Identity<A>` | `Supplier<A>` | Liefert Startwert |
| `Accumulator<A,E>` | `TriFunction<A,E,Timestamp,A>` | Akkumuliert Elemente |
| `Combiner<A>` | `BiFunction<A,A,A>` | Kombiniert parallele Teilergebnisse |
| `Finisher<A,R>` | `Function<A,R>` | Endgültige Transformation |
| `Interrupt<E,A>` | `TriPredicate<E,Timestamp,A>` | Abbruchbedingung |

### Concurrency‑Unterstützung

`Collector::collect()` übernimmt automatisch:

- 📦 Datensplittung (verteilt über Index‑Modulo auf Threads)
- 🔗 Ergebniszusammenführung (via `Combiner`)
- ⚠️ Ausnahmeweitergabe (über `std::exception_ptr` und `std::atomic<bool>`)

---

## 🏭 Schicht 5: `collectors.h` – Collector‑Fabrik

`collectors.h` stellt zahlreiche vordefinierte Collector‑Fabriken bereit.

### 📊 Matching‑Operationen

| Methode | Beschreibung | Rückgabetyp |
|-------|--------------|--------------|
| `useAllMatch(predicate)` | Alle Elemente erfüllen das Prädikat | `bool` |
| `useAnyMatch(predicate)` | Mindestens ein Element erfüllt das Prädikat | `bool` |
| `useNoneMatch(predicate)` | Kein Element erfüllt das Prädikat | `bool` |

### 🔍 Suchoperationen

| Methode | Beschreibung | Rückgabetyp |
|-------|--------------|--------------|
| `useFindFirst()` | Erstes Element finden | `std::optional<E>` |
| `useFindLast()` | Letztes Element finden | `std::optional<E>` |
| `useFindAny()` | Beliebiges Element finden | `std::optional<E>` |
| `useFindAt(index)` | Element am angegebenen Index (auch negative Indizes) | `std::optional<E>` |
| `useFindMaximum()` | Maximales Element | `std::optional<E>` |
| `useFindMinimum()` | Minimales Element | `std::optional<E>` |

### 📈 Aggregationsoperationen

| Methode | Beschreibung | Rückgabetyp |
|-------|--------------|--------------|
| `useCount()` | Anzahl der Elemente | `Module` |
| `useSummate<E,D>()` | Summation | `D` |
| `useSummate<E,D>(mapper)` | Gemappte Summation | `D` |
| `useAverage<E,D>()` | Durchschnitt | `D` |
| `useAverage<E,D>(mapper)` | Gemappter Durchschnitt | `D` |
| `useRange<E,D>()` | Wertebereich | `D` |
| `useRange<E,D>(mapper)` | Gemappter Wertebereich | `D` |
| `useMinimum<E,D>()` | Minimum | `std::optional<D>` |
| `useMaximum<E,D>()` | Maximum | `std::optional<D>` |

### 📊 Statistische Operationen

| Methode | Beschreibung | Rückgabetyp |
|-------|--------------|--------------|
| `useVariance<E,D>()` | Varianz (Grundgesamtheit) | `D` |
| `useStandardDeviation<E,D>()` | Standardabweichung | `D` |
| `useSkewness<E,D>()` | Schiefe | `D` |
| `useKurtosis<E,D>()` | Wölbung (Kurtosis) | `D` |
| `useMedian<E,D>()` | Median | `std::optional<D>` |
| `useMode<E>()` | Modalwert | `std::optional<E>` |
| `usePercentile<E,D>(p)` | p‑tes Perzentil | `std::optional<D>` |
| `useFrequency<E>()` | Frequenzbereichsmerkmale | `std::map<E, complex>` |
| `useDistribution<E>()` | Raumverteilungsmerkmale | `std::map<E, complex>` |

### 🔗 Reduktionsoperationen

| Methode | Beschreibung | Rückgabetyp |
|-------|--------------|--------------|
| `useReduce(reducer)` | Reduktion ohne Identität | `std::optional<E>` |
| `useReduce(identity, reducer)` | Reduktion mit Identität | `E` |
| `useReduce(id, red, comb, fin)` | Vollständig benutzerdefinierte Reduktion | `R` |

### 📦 Sammeloperationen

| Methode | Beschreibung | Rückgabetyp |
|-------|--------------|--------------|
| `useToVector()` | In `vector` sammeln | `std::vector<E>` |
| `useToList()` | In `list` sammeln | `std::list<E>` |
| `useToSet()` | In `set` sammeln (Duplikate entfernt) | `std::set<E>` |
| `useToMap(keyExtractor)` | In `map` sammeln | `std::map<K,E>` |
| `useGroup(keyExtractor)` | Nach Schlüssel gruppieren | `std::unordered_map<K,vector<E>>` |
| `usePartition(size)` | Nach Größe partitionieren | `std::vector<vector<E>>` |

### 🎨 Ausgabeoperationen

| Methode | Beschreibung | Rückgabetyp |
|-------|--------------|--------------|
| `useJoin()` | Zu Zeichenkette verbinden | `Charsequence` |
| `useOut()` | Nach stdout ausgeben | `Charsequence` |
| `useError()` | Nach stderr ausgeben | `Charsequence` |

### 🧮 Mathematische Hilfsmittel

| Methode | Beschreibung | Rückgabetyp |
|-------|--------------|--------------|
| `useDFT()` | Diskrete Fourier‑Transformation | `vector<complex<double>>` |
| `useIDFT()` | Inverse diskrete Fourier‑Transformation | `vector<complex<double>>` |
| `useFFT()` | Schnelle Fourier‑Transformation (Cooley‑Tukey) | `vector<complex<double>>` |
| `useIFFT()` | Inverse schnelle Fourier‑Transformation | `vector<complex<double>>` |
| `useGradient(gradFunc, lr, iter, th)` | Gradientenabstieg (analytischer Gradient) | `vector<double>` |
| `useGradient(costFunc, lr, iter, th, h)` | Gradientenabstieg (numerischer Gradient) | `vector<double>` |

---

## 🌊 Schicht 6: `semantic.h` – Stream‑Zwischenoperationen

`semantic.h` ist das Herzstück der Bibliothek und umfasst die Namensräume `collectable` und `semantic`.

### `collectable`‑Namensraum

Stellt eine Vererbungshierarchie für sammelbare Objekte bereit.

| Klasse | Beschreibung | Interner Speicher |
|------|--------------|------------------|
| `Collectable<E>` | Abstrakte Basisklasse mit rein virtueller `source()` | — |
| `OrderedCollectable<E>` | Geordnete Sammlung | `std::map<Timestamp, E>` |
| `UnorderedCollectable<E>` | Ungeordnete Sammlung | `std::unordered_map<Timestamp, E>` |
| `Statistics<E, D>` | Statistische Sammlung (erbt von `OrderedCollectable`) | 20+ statistische Methoden |
| `WindowCollectable<E>` | Fensterbasierte Sammlung | Unterstützt Sliding‑ / Tumbling‑Windows |

#### Methoden der `Statistics`‑Klasse

| Methode | Rückgabetyp | Beschreibung |
|-------|--------------|--------------|
| `summate()` / `summate(mapper)` | `D` | Summation |
| `average()` / `average(mapper)` | `D` | Durchschnitt |
| `minimum()` / `minimum(mapper)` | `std::optional<D>` | Minimum |
| `maximum()` / `maximum(mapper)` | `std::optional<D>` | Maximum |
| `range()` / `range(mapper)` | `D` | Spannweite (Max − Min) |
| `variance()` / `variance(mapper)` | `D` | Varianz |
| `standardDeviation()` / `standardDeviation(mapper)` | `D` | Standardabweichung |
| `frequency()` / `frequency(mapper)` | `std::map<*, complex>` | Frequenzbereichsmerkmale |
| `distribute()` / `distribute(mapper)` | `std::map<*, complex>` | Raumverteilungsmerkmale |
| `median()` / `median(mapper)` | `std::optional<D>` | Median |
| `mode()` | `std::optional<E>` | Modalwert |
| `percentile(p)` / `percentile(p, mapper)` | `std::optional<D>` | p‑tes Perzentil |
| `firstQuartile()` / `firstQuartile(mapper)` | `std::optional<D>` | Erstes Quartil (Q1) |
| `thirdQuartile()` / `thirdQuartile(mapper)` | `std::optional<D>` | Drittes Quartil (Q3) |
| `interquartileRange()` / `interquartileRange(mapper)` | `std::optional<D>` | Interquartilsabstand (Q3 − Q1) |
| `skewness()` / `skewness(mapper)` | `D` | Schiefe |
| `kurtosis()` / `kurtosis(mapper)` | `D` | Wölbung |
| `dft()` | `vector<complex<double>>` | Diskrete Fourier‑Transformation |
| `idft()` | `vector<complex<double>>` | Inverse diskrete Fourier‑Transformation |
| `fft()` | `vector<complex<double>>` | Schnelle Fourier‑Transformation |
| `ifft()` | `vector<complex<double>>` | Inverse schnelle Fourier‑Transformation |
| `gradient(gradFunc, lr, iter, th)` | `vector<double>` | Gradientenabstieg (analytisch) |
| `gradient(costFunc, lr, iter, th, h)` | `vector<double>` | Gradientenabstieg (numerisch) |

### `semantic`‑Namensraum

Stellt die Vorlagenklasse `Semantic<E>` und deren vollständige Spezialisierungen bereit.

#### Methoden der Hauptvorlage

| Kategorie | Methoden |
|---------|----------|
| 🎨 Elementtransformation | `map`, `flatMap`, `flat` |
| 🔍 Filterung | `filter`, `takeWhile`, `dropWhile`, `distinct` |
| 📏 Mengensteuerung | `limit`, `skip`, `sub` |
| 📐 Indexmanipulation | `redirect`, `reverse`, `translate`, `sort` |
| 👀 Beobachtung | `peek` |
| ⚡ Parallelität | `parallel` |
| 🔗 Verknüpfung | `concatenate` |
| 📤 Terminalkonvertierung | `toUnordered`, `toOrdered`, `toWindow`, `toStatistics` |

#### Unterstützte Containerspezialisierungen

| Spezialisierung | Beschreibung |
|----------------|--------------|
| `Semantic<std::vector<E>>` | Vektor‑Stream |
| `Semantic<std::list<E>>` | Listen‑Stream |
| `Semantic<std::set<E>>` | Geordneter Mengen‑Stream |
| `Semantic<std::unordered_set<E>>` | Ungeordneter Mengen‑Stream |
| `Semantic<std::deque<E>>` | Deque‑Stream |
| `Semantic<std::queue<E>>` | Queue‑Stream |
| `Semantic<std::stack<E>>` | Stack‑Stream |
| `Semantic<std::map<K,V>>` | Map‑Stream |
| `Semantic<std::unordered_map<K,V>>` | Ungeordneter Map‑Stream |
| `Semantic<std::initializer_list<E>>` | Initialisiererlisten‑Stream |
| `Semantic<Semantic<E>>` | Geschachtelte Streams (Flattening) |

---

## 🏭 Schicht 7: `semantics.h` – Stream‑Builder

`semantics.h` stellt alle Fabrikmethoden zur Stream‑Erzeugung bereit.

### 📐 Numerische Bereiche

| Methode | Beschreibung |
|-------|--------------|
| `useRange(start, end)` | Erzeugt Werte im Bereich `[start, end)` |

### 📦 Container‑Konstruktion

| Methode | Beschreibung |
|-------|--------------|
| `useFrom(container)` | Erzeugt Stream aus beliebigem Standardcontainer |
| `useOf(args...)` | Erzeugt Stream aus variadischen Argumenten |

### 📝 Textverarbeitung

| Methode | Beschreibung |
|-------|--------------|
| `useBlob(text)` | Zerlegt Zeichenkette byteweise in `char`‑Stream |
| `useText(text)` | Behandelt Zeichenkette als ganzen Text‑Stream |
| `useText(text, delimiter)` | Zerlegt Text nach Trennzeichen |
| `useText(istream)` | Liest gesamten Inhalt aus Eingabestrom |

### 🌐 Unicode‑Verarbeitung

| Methode | Beschreibung |
|-------|--------------|
| `useSequence(charsequence)` | Erzeugt Codepoint‑Stream aus Zeichensequenz |
| `useSequence(text, encoding)` | Erzeugt Codepoint‑Stream mit angegebener Kodierung |
| `useCharsequence(charsequence)` | Behandelt Zeichensequenz als ganzen Stream |
| `useCharsequence(charsequence, delimiter)` | Zerlegt Zeichensequenz nach Trennzeichen |

---

## 🧠 Kernkonzept: Indexgetriebene Datenwelt

Semantic‑Cpp abstrahiert Datenverarbeitung als Operationen auf **Elementen** und deren **logischen Positionen (Indices)**. Dies zu verstehen, ist der Schlüssel zur Beherrschung der Bibliothek.

### 1. 📐 Grundlegende Index‑Transformationen

| Methode | Beschreibung |
|-------|--------------|
| `redirect(fn)` | Kernmethode: vollständige Neudefinition von Indices über benutzerdefinierte Funktion |
| `reverse()` | Kehrt die gesamte Indexlogik um (intern über `redirect` realisiert) |
| `translate(offset)` | Fester Versatz |
| `translate(translator)` | Dynamischer Versatz |

### 2. 📊 Die „autoritäre“ Regel des Sortierens

⚠️ **`sort()` überschreibt alles**: Nach dem Aufruf werden sämtliche vorherigen Index‑Manipulationen verworfen. Elemente erhalten neue, wertbasierte Indices.

- `sort()` → sofortige Materialisierung als `OrderedCollectable`
- `sort(comparator)` → benutzerdefiniertes Sortierkriterium

### 3. ⚡ Deklarative Parallelverarbeitung

- `parallel(n)` **deklariert lediglich die Absicht**, startet jedoch keine Threads sofort
- Terminaloperationen (`toUnordered()`, `count()` usw.) lösen die Ausführung tatsächlich aus
- Der Thread‑Pool übernimmt automatisch Aufgabenverteilung und Ergebniszusammenführung

### 4. 🎯 Wahl des passenden Zielcontainers

| Konvertierung | Interner Speicher | Leistungsmerkmale | Ideale Einsatzszenarien |
|--------------|------------------|-------------------|-------------------------|
| `sort()` | `OrderedCollectable` | Nach Sortierung materialisiert, werterhaltend geordnet | Wertbasiertes Sortieren, Paginierung, Zeitreihen |
| `toOrdered()` | `OrderedCollectable` | Erhält aktuelle Indexreihenfolge | Beibehaltung benutzerdefinierter Indexordnung |
| `toUnordered()` | `UnorderedCollectable` | Ø O(1), höchster Durchsatz | Schnelle Suche, Deduplizierung, Aggregation |
| `toWindow()` | `WindowCollectable` | Fensterbasiert | Sliding‑ / Tumbling‑Window‑Analysen |
| `toStatistics()` | `Statistics` | 20+ statistische Methoden | Umfassende statistische Analysen |

---

## 🚀 Schnellstart

### Installation

Kopieren Sie alle Header‑Dateien in Ihr Projektverzeichnis und stellen Sie sicher, dass Ihr Compiler **C++17 oder neuer** unterstützt:

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
#include "semantics.h"  // bindet automatisch alle Abhängigkeiten ein
```

---

### 🎯 Beispiel: Indizes und Sortierung

```cpp
auto result = semantic::useRange(0, 10)
    .map(int x -> int { return x * x; })
    .redirect(int value, auto index -> long long {
        return index * 2;
    })
    .reverse()
    .sort()                              // erzwingt wertbasiertes Sortieren
    .toVector();

// Ausgabe: 0 1 4 9 16 25 36 49 64 81
```

### ⚡ Parallele Verarbeitung

```cpp
auto count = semantic::useRange(1, 1000)
    .parallel(4)
    .filter(int x { return x % 2 == 0; })
    .toUnordered()
    .count();

// Ausgabe: Anzahl gerader Zahlen: 500
```

### 📊 Statistische Analyse

```cpp
auto stats = semantic::useRange(1, 101)
    .toStatistics<int, double>();

auto avg = stats.average();               // Mittelwert
auto med = stats.median();                // Median
auto std = stats.standardDeviation();     // Standardabweichung
auto q1  = stats.firstQuartile();         // Erstes Quartil
auto q3  = stats.thirdQuartile();         // Drittes Quartil
auto skew = stats.skewness();             // Schiefe
```

### 🔬 Frequenzbereichsanalyse

```cpp
auto freq = data.toUnordered().frequency();
for (const auto& [element, z] : freq) {
    auto magnitude = std::abs(z);  // Konzentration der Verteilung
    auto phase     = std::arg(z);  // Phasenlage der Verteilung
}
```

### 🧮 FFT‑Beispiel

```cpp
auto spectrum = semantic::useRange(0, 8)
    .map(int x -> std::complex<double> {
        return {x, 0};
    })
    .toUnordered()
    .collect(collector::useFFT<double>());
```

---

## ⚡ Leistungsoptimierung

1. 🎯 **Passenden Container wählen**
   - Gleichheitssuche, unsortierte Aggregation → `toUnordered()`
   - Bereichsabfragen, Sortierung, Paginierung → `toOrdered()` oder `sort()`
   - Echtzeit‑Fensteranalyse → `toWindow()`
2. ⚡ **Parallelität gezielt einsetzen**: `parallel()` bei großen Datenmengen oder rechenintensiven Aufgaben; blockierende I/O vermeiden
3. 📐 **Operationsreihenfolge optimieren**: `filter` früh anwenden, `sort` bewusst einsetzen
4. 🔄 **Lazy Evaluation nutzen**: Zwischenoperationen werden nicht sofort ausgeführt; `takeWhile` und `limit` können früh abbrechen

---

## 📊 Vergleich mit der C++‑Standardbibliothek und Alternativen

| Merkmal | Semantic‑Cpp | C++20/23 Ranges | Traditionelle Schleifen |
|-------|-------------|----------------|--------------------------|
| 🎯 Paradigma | Deklarativ, indexgetrieben | View‑basiert, funktionale Komposition | Imperativ, prozedural |
| ⚡ Parallelität | Deklarativ, automatischer Thread‑Pool | Erfordert explizite parallele Algorithmen | Manuell |
| 📐 Sortierung & Indizes | Feingranulare Indexsteuerung | Destruktives Sortieren | Vollständig manuell |
| 📊 Statistik | 20+ integrierte Methoden | Nicht enthalten | Externe Bibliotheken erforderlich |
| 🔬 Frequenzanalyse | Native DFT / FFT | Nicht nativ unterstützt | Externe Bibliotheken erforderlich |
| 🧮 Gradientenabstieg | Analytisch + numerisch | Nicht enthalten | Externe Bibliotheken erforderlich |
| 🌐 Unicode | Native Mehrfachkodierung | Nicht nativ | Manuell |
| 📦 Abhängigkeiten | Keine externen, 7 Header | Standardbibliothek | Keine |

---

## 📜 Lizenz und Support

- 📄 **Lizenz**: MIT‑Lizenz  
---

**Semantic‑Cpp — Effiziente, elegante Datenverarbeitungspipelines mit modernem C++. 🚀**
