# 🚀 Semantic-Cpp: Eine zukunftssichere C++-Smart-Stream-Processing-Bibliothek

Semantic-Cpp ist eine moderne, vollständig neu konzipierte C++-Stream-Processing-Bibliothek, die eine **„Multi-Header, null externe Abhängigkeiten“**-Modulararchitektur verfolgt. Jede Header-Datei besitzt eine klare, eigenständige Verantwortlichkeit und ist unabhängig testbar. Gemeinsam bilden sie ein vollständiges Ökosystem für die Stream-Verarbeitung. Diese Bibliothek vereint auf kreative Weise das Beste verschiedener Programmierparadigmen:

- 🎯 **Java Stream API** Eleganz und Flüssigkeit: Verkettbare Aufrufe und deklarative Programmierung für poetischen Code.
- ⚡ **JavaScript Generator** Trägheit und Flexibilität: Verzögerte Ausführung, bedarfsgerechte Generierung und Speichereffizienz.
- 🗄️ **Datenbank-Indexierung** Effizienz und Ordnung: Intelligentes Sortieren, indexgesteuertes Design – ideal für Zeitreihendaten.

Im Gegensatz zu herkömmlichen Datenverarbeitungsansätzen (manuell geschriebene Schleifen, asynchrone Callbacks) bietet Semantic-Cpp eine **typsichere, ausdrucksstarke und leistungsstarke** Lösung. Die zentrale Designphilosophie ist die **präzise Datenflusssteuerung**: Daten fließen nur bei Bedarf, wobei Reihenfolge und Position durch **„Indizes“** fein abgestimmt gesteuert werden, um die Ressourcennutzung zu optimieren.

---

## 📐 Projektarchitektur: Sieben-Schichten-Moduldesign

Semantic-Cpp besteht aus **sieben Kern-Headern**, die schrittweise aufeinander aufbauen und jeweils eine eigenständige Aufgabe erfüllen:

```
┌─────────────────────────────────────────────────┐
│                 semantics.h                     │
│    (Stream-Fabriken: Zahlenbereiche, Container,  │
│     Text, Unicode)                               │
├─────────────────────────────────────────────────┤
│                  semantic.h                     │
│   (Stream-Zwischenoperationen, Collectable-System│
│    , Containerspezialisierungen)                 │
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│  (Collector-Fabriken: Matching, Suchen,         │
│   Aggregation, Statistik, DFT/FFT usw.)          │
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│     (Collector-Framework: Fünf-Stufen-Modell,   │
│      Nebenläufigkeits- & Parallelitätsunterstützung) │
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│  (Unicode-Zeichensequenzen, Multi-Encoding-     │
│   Konvertierung, Builder, Buffer)                │
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│   (Globaler Thread-Pool: Task-Übermittlung,     │
│    Not-Aus, Ausnahmefortpflanzung)               │
├─────────────────────────────────────────────────┤
│                 function.h                      │
│  (Typ-Aliasse: Generator, Supplier, Consumer,    │
│   usw.)                                          │
└─────────────────────────────────────────────────┘
```

### Abhängigkeitsgraph

```
function.h          ← keine Abhängigkeiten, Typ-Grundlage
pool.h              ← abhängig von function.h
charsequence.h      ← unabhängiges Modul, Unicode-Verarbeitung
collector.h         ← abhängig von function.h, pool.h
collectors.h        ← abhängig von collector.h, charsequence.h
semantic.h          ← abhängig von collector.h, collectors.h, charsequence.h
semantics.h         ← abhängig von semantic.h
```

Jede Header-Datei kann separat kompiliert und getestet oder bedarfsgerecht eingebunden werden.  
Benötigen Sie beispielsweise nur Collector-Funktionalität, genügt es, `collector.h` und `collectors.h` einzubinden.

---

## 🏗️ Schicht Eins: function.h – Typfundament

`function.h` definiert das Typsystem der gesamten Bibliothek – die gemeinsame Grundlage aller Module:

```cpp
namespace function {
    using Timestamp = long long;           // Indextyp
    using Module = unsigned long long;     // Modul-/Zähler-Typ
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept
        std::function<bool(T, Timestamp)>       // interrupt
    )>;
}
```

`Generator` ist die zentrale Abstraktion des gesamten Stream-Systems: Er akzeptiert zwei Callbacks – `accept` (zur Datenaufnahme) und `interrupt` (zum Verarbeitungsabbruch) – und verkörpert das **„Lazy-Pull“-Modell**.

---

## ⚡ Schicht Zwei: pool.h – Nebenläufigkeits-Grundlage

`pool.h` stellt den globalen Thread-Pool `pool::pool` bereit, der als Nebenläufigkeitsmotor des Frameworks dient:

| Merkmal | Beschreibung |
|-------|--------------|
| 🎯 Deklarative Parallelität | `parallel(n)` erklärt lediglich die Absicht; wird erst bei Terminaloperationen aktiviert |
| 🛡️ Not-Aus | Integrierte `emergencyShutdown()`-Funktion und `std::set_terminate`-Handler |
| 🔄 Ausnahmefortpflanzung | `submit()` liefert `std::future` und unterstützt sichere Ausnahmefortpflanzung |

---

## 🔤 Schicht Drei: charsequence.h – Unicode-Zeichensequenzen

`charsequence.h` ist ein umfassendes Unicode-Verarbeitungsmodul, das Erstellung, Konvertierung und Manipulation von Zeichensequenzen ermöglicht:

| Fähigkeit | Beschreibung |
|----------|--------------|
| 🌐 Mehrfachkodierungsunterstützung | UTF‑8, UTF‑16 (LE/BE), UTF‑32 (LE/BE), ASCII, Latin1 |
| 🔍 Code-Point-Iterator | `PointIterator` unterstützt bidirektionale Traversierung von Unicode-Code-Points |
| 🏗️ Builder-Muster | `Builder`-Klasse ermöglicht effiziente Byte-Ebene-String-Konkatenation |
| 📦 Puffer | `Buffer`-Klasse stellt einen threadsicheren Ringpuffer bereit |
| 🔑 Hashing & Vergleich | Alle Kerntypen besitzen `std::hash`- und `std::less`-Spezialisierungen |

### Kerntypen

| Typ | Beschreibung |
|-----|--------------|
| `Meta` | Metadaten-Wrapper zur Speicherung vorzeichenloser Ganzzahlwerte |
| `Point` | Unicode-Code-Point mit Surrogatpaar-Erkennung und Gültigkeitsprüfung |
| `Charsequence` | Unveränderliche Zeichensequenz mit Unterstützung für Aufteilung, Ersetzung, Suche, Groß-/Kleinschreibung usw. |
| `Builder` | Veränderlicher Byte-Builder mit `prepend`, `insert`, `append` und verschiedenen Datentypen |
| `Buffer` | Thread-sicherer Ringpuffer mit Lese-/Schreib-, Prefetch- und Kapazitätsverwaltung |

---

## 🔧 Schicht Vier: collector.h – Collector-Framework

`collector.h` implementiert das Collector-Muster und bildet den Kernmotor hinter Terminaloperationen.

### Fünf-Stufen-Modell

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (optionaler Kurzschluss)
```

### Typ-Aliasse

| Typ | Definition | Rolle |
|-----|-----------|-------|
| `Identity<A>` | `Supplier<A>` | Stellt Initialwert bereit |
| `Accumulator<A,E>` | `TriFunction<A,E,Timestamp,A>` | Akkumuliert Elemente |
| `Combiner<A>` | `BiFunction<A,A,A>` | Führt parallele Ergebnisse zusammen |
| `Finisher<A,R>` | `Function<A,R>` | Endgültige Transformation |
| `Interrupt<E,A>` | `TriPredicate<E,Timestamp,A>` | Kurzschlussauswertung |

### Nebenläufigkeitsunterstützung

`Collector::collect()` übernimmt automatisch:

- 📦 Datenpartitionierung (Verteilung über Index-Modulo)
- 🔗 Ergebniszusammenführung (über `Combiner`)
- ⚠️ Ausnahmefortpflanzung (über `std::exception_ptr` und `std::atomic<bool>`)

---

## 🏭 Schicht Fünf: collectors.h – Collector-Fabrik

`collectors.h` bietet eine umfangreiche Sammlung vorgefertigter Collector-Fabrikfunktionen.

### 📊 Matching-Operationen

| Methode | Beschreibung | Rückgabetyp |
|--------|--------------|-------------|
| `useAllMatch(predicate)` | Alle Elemente erfüllen Bedingung | `bool` |
| `useAnyMatch(predicate)` | Mindestens ein Element erfüllt Bedingung | `bool` |
| `useNoneMatch(predicate)` | Kein Element erfüllt Bedingung | `bool` |

### 🔍 Suchoperationen

| Methode | Beschreibung | Rückgabetyp |
|--------|--------------|-------------|
| `useFindFirst()` | Erstes Element finden | `std::optional<E>` |
| `useFindLast()` | Letztes Element finden | `std::optional<E>` |
| `useFindAny()` | Ein beliebiges Element finden | `std::optional<E>` |
| `useFindAt(index)` | Element am angegebenen Index finden (negative Indizes unterstützt) | `std::optional<E>` |
| `useFindMaximum()` | Maximum finden (benutzerdefinierter Vergleich unterstützt) | `std::optional<E>` |
| `useFindMinimum()` | Minimum finden (benutzerdefinierter Vergleich unterstützt) | `std::optional<E>` |

### 📈 Aggregationsoperationen

| Methode | Beschreibung | Rückgabetyp |
|--------|--------------|-------------|
| `useCount()` | Gesamtzahl der Elemente | `Module` |
| `useSummate<E,D>()` | Summation | `D` |
| `useSummate<E,D>(mapper)` | Gemappte Summation | `D` |
| `useAverage<E,D>()` | Durchschnitt | `D` |
| `useAverage<E,D>(mapper)` | Gemappter Durchschnitt | `D` |
| `useRange<E,D>()` | Wertebereich (Max − Min) | `D` |
| `useRange<E,D>(mapper)` | Gemappter Wertebereich | `D` |
| `useMinimum<E,D>()` | Minimalwert | `std::optional<D>` |
| `useMaximum<E,D>()` | Maximalwert | `std::optional<D>` |

### 📊 Statistische Operationen

| Methode | Beschreibung | Rückgabetyp |
|--------|--------------|-------------|
| `useVariance<E,D>()` | Populationsvarianz | `D` |
| `useStandardDeviation<E,D>()` | Populationsstandardabweichung | `D` |
| `useSkewness<E,D>()` | Schiefe | `D` |
| `useKurtosis<E,D>()` | Wölbung | `D` |
| `useMedian<E,D>()` | Median | `std::optional<D>` |
| `useMode<E>()` | Modalwert (frequenzbasiert) | `std::optional<E>` |
| `usePercentile<E,D>(p)` | p-tes Perzentil | `std::optional<D>` |
| `useFrequency<E>()` | Frequenzbereichsmerkmale (Index-Phasenkodierung) | `std::map<E, complex>` |
| `useDistribution<E>()` | Räumliche Verteilungsmerkmale (Positionskodierung) | `std::map<E, complex>` |

### 🔗 Reduktionsoperationen

| Methode | Beschreibung | Rückgabetyp |
|--------|--------------|-------------|
| `useReduce(reducer)` | Reduktion ohne Initialwert | `std::optional<E>` |
| `useReduce(identity, reducer)` | Reduktion mit Initialwert | `E` |
| `useReduce(id, red, comb, fin)` | Vollständig benutzerdefinierte Reduktion | `R` |

### 📦 Container-Sammeloperationen

| Methode | Beschreibung | Rückgabetyp |
|--------|--------------|-------------|
| `useToVector()` | In `std::vector` sammeln | `std::vector<E>` |
| `useToList()` | In `std::list` sammeln | `std::list<E>` |
| `useToDeque()` | In `std::deque` sammeln | `std::deque<E>` |
| `useToForwardList()` | In `std::forward_list` sammeln | `std::forward_list<E>` |
| `useToArray<N>()` | In fixgroßes `std::array` sammeln | `std::array<E, N>` |
| `useToSet()` | In `std::set` sammeln (entfernt Duplikate, sortiert) | `std::set<E>` |
| `useToMultiset()` | In `std::multiset` sammeln | `std::multiset<E>` |
| `useToUnorderedSet()` | In `std::unordered_set` sammeln | `std::unordered_set<E>` |
| `useToUnorderedMultiset()` | In `std::unordered_multiset` sammeln | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)` | In `std::map` sammeln | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)` | In `std::map` sammeln (benutzerdefinierter Wert) | `std::map<K, V>` |
| `useToMultimap(keyExtractor)` | In `std::multimap` sammeln | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)` | In `std::multimap` sammeln (benutzerdefinierter Wert) | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)` | In `std::unordered_map` sammeln | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)` | In `std::unordered_multimap` sammeln | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)` | In `std::unordered_multimap` sammeln | `std::unordered_multimap<K, V>` |
| `useToStack()` | In `std::stack` sammeln | `std::stack<E>` |
| `useToQueue()` | In `std::queue` sammeln | `std::queue<E>` |
| `useToPriorityQueue()` | In `std::priority_queue` sammeln | `std::priority_queue<E>` |

### 🔀 Gruppierungs- und Partitionsoperationen

| Methode | Beschreibung | Rückgabetyp |
|--------|--------------|-------------|
| `useGroup(keyExtractor)` | Nach Schlüssel gruppieren | `std::unordered_map<K, vector<E>>` |
| `usePartition(size)` | In gleich große Blöcke partitionieren | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)` | Nach benutzerdefiniertem Schlüssel partitionieren | `std::vector<vector<E>>` |

### 🎨 Zeichenketten-Ausgabeoperationen

| Methode | Beschreibung | Rückgabetyp |
|--------|--------------|-------------|
| `useJoin()` | Zu Zeichenkette verbinden (Standard: Komma-getrennt, eckige Klammern) | `Charsequence` |
| `useJoin(delimiter)` | Mit benutzerdefiniertem Trennzeichen verbinden | `Charsequence` |
| `useJoin(prefix, delimiter, suffix)` | Vollständig benutzerdefiniert formatierte Verbindung | `Charsequence` |
| `useOut()` | Formatierte Ausgabe nach stdout | `Charsequence` |
| `useOut(delimiter)` | Mit benutzerdefiniertem Trennzeichen nach stdout ausgeben | `Charsequence` |
| `useOut(prefix, delimiter, suffix)` | Vollständig benutzerdefiniert formatierte Ausgabe nach stdout | `Charsequence` |
| `useError()` | Formatierte Ausgabe nach stderr | `Charsequence` |
| `useError(delimiter)` | Mit benutzerdefiniertem Trennzeichen nach stderr ausgeben | `Charsequence` |
| `useError(prefix, delimiter, suffix)` | Vollständig benutzerdefiniert formatierte Ausgabe nach stderr | `Charsequence` |

### 🧮 Mathematische Hilfsmittel

| Methode | Beschreibung | Rückgabetyp |
|--------|--------------|-------------|
| `useDFT()` | Diskrete Fourier-Transformation | `vector<complex<double>>` |
| `useIDFT()` | Inverse diskrete Fourier-Transformation | `vector<complex<double>>` |
| `useFFT()` | Schnelle Fourier-Transformation (Cooley–Tukey) | `vector<complex<double>>` |
| `useIFFT()` | Inverse schnelle Fourier-Transformation | `vector<complex<double>>` |
| `useGradient(gradFunc, lr, iter, th)` | Gradientenabstieg (analytischer Gradient) | `vector<double>` |
| `useGradient(costFunc, lr, iter, th, h)` | Gradientenabstieg (numerischer Gradient) | `vector<double>` |

---

## 🌊 Schicht Sechs: semantic.h – Stream-Zwischenoperationen & Collectable-System

`semantic.h` ist das Herzstück des Frameworks und enthält die Namespaces `collectable` und `semantic`.

### collectable-Namespace

Stellt die Vererbungshierarchie für Collectable-Objekte bereit:

| Klasse | Beschreibung | Zugrunde liegende Speicherstruktur |
|-------|--------------|-----------------------------------|
| `Collectable<E>` | Abstrakte Basisklasse mit rein virtueller `source()` | — |
| `OrderedCollectable<E>` | Geordnete Sammlung mit benutzerdefiniertem Sortieren | `std::map<Timestamp, E>` |
| `UnorderedCollectable<E>` | Ungeordnete Sammlung, O(1)-Suche | `std::unordered_map<Timestamp, E>` |
| `Statistics<E, D>` | Statistische Sammlung (erbt von `OrderedCollectable`) | 20+ statistische Methoden |
| `WindowCollectable<E>` | Fensterbasierte Sammlung (erbt von `OrderedCollectable`) | Unterstützt `slide` / `tumble` |

#### Collectable-Basismethoden

Bietet alle `toXxx()`-Terminal-Sammelmethoden (über 20 Containertypen) sowie:

`count()`, `findFirst()`, `findAny()`, `anyMatch()`, `allMatch()`, `noneMatch()`, `reduce()`, `join()`, `out()`, `error()`, `group()`, `partition()`, `partitionBy()` usw.

#### Statistics-Klassenmethoden

| Methode | Rückgabetyp | Beschreibung |
|--------|-------------|--------------|
| `summate()` / `summate(mapper)` | `D` | Summation |
| `average()` / `average(mapper)` | `D` | Durchschnitt |
| `minimum()` / `minimum(mapper)` | `std::optional<D>` | Minimum |
| `maximum()` / `maximum(mapper)` | `std::optional<D>` | Maximum |
| `range()` / `range(mapper)` | `D` | Bereich (Max − Min) |
| `variance()` / `variance(mapper)` | `D` | Populationsvarianz |
| `standardDeviation()` / `standardDeviation(mapper)` | `D` | Populationsstandardabweichung |
| `frequency()` / `frequency(mapper)` | `std::map<*, complex>` | Frequenzbereichsmerkmale (Index-Phasenkodierung) |
| `distribute()` / `distribute(mapper)` | `std::map<*, complex>` | Räumliche Verteilungsmerkmale (Positionskodierung) |
| `median()` / `median(mapper)` | `std::optional<D>` | Median |
| `mode()` | `std::optional<E>` | Modalwert |
| `percentile(p)` / `percentile(p, mapper)` | `std::optional<D>` | p-tes Perzentil |
| `firstQuartile()` / `firstQuartile(mapper)` | `std::optional<D>` | Erstes Quartil (Q1) |
| `thirdQuartile()` / `thirdQuartile(mapper)` | `std::optional<D>` | Drittes Quartil (Q3) |
| `interquartileRange()` / `interquartileRange(mapper)` | `std::optional<D>` | Interquartilsabstand (Q3 − Q1) |
| `skewness()` / `skewness(mapper)` | `D` | Schiefe |
| `kurtosis()` / `kurtosis(mapper)` | `D` | Wölbung |
| `dft()` | `vector<complex<double>>` | Diskrete Fourier-Transformation |
| `idft()` | `vector<complex<double>>` | Inverse diskrete Fourier-Transformation |
| `fft()` | `vector<complex<double>>` | Schnelle Fourier-Transformation |
| `ifft()` | `vector<complex<double>>` | Inverse schnelle Fourier-Transformation |
| `gradient(gradFunc, lr, iter, th)` | `vector<double>` | Gradientenabstieg (analytischer Gradient) |
| `gradient(costFunc, lr, iter, th, h)` | `vector<double>` | Gradientenabstieg (numerischer Gradient) |

### semantic-Namespace

Stellt die `Semantic<E>`-Template-Klasse und ihr vollständiges Spezialisierungssystem bereit.

#### Haupt-Template-Methodenübersicht

| Kategorie | Methode | Beschreibung |
|---------|---------|--------------|
| 🎨 Elementtransformation | `map` | Ein-zu-eins-Mapping |
| | `flatMap` | Ein-zu-viele-Mapping mit Abflachung |
| | `flat` | Verschachtelte Streams abflachen |
| 🔍 Elementfilterung | `filter` | Bedingungsfilter |
| | `takeWhile` | Übernehmen, solange Bedingung gilt |
| | `dropWhile` | Verwerfen, solange Bedingung gilt |
| | `distinct` | Duplikatentfernung (benutzerdefinierter Vergleich unterstützt) |
| 📏 Mengensteuerung | `limit` | Anzahl der Elemente begrenzen |
| | `skip` | Erste n Elemente überspringen |
| | `sub` | Teilbereich extrahieren |
| 📐 Indexoperationen | `redirect` | Indizes neu mappen |
| | `reverse` | Indizes umkehren |
| | `translate` | Indizes verschieben (fest oder dynamisch) |
| 👀 Beobachtung | `peek` | Elemente beobachten, ohne sie zu ändern |
| ⚡ Parallelitätserklärung | `parallel` | Parallelitätsgrad festlegen |
| 🔗 Verknüpfung | `concatenate` | Anderen Stream oder Container anhängen |
| 📤 Terminalkonvertierung | `toUnordered` | In ungeordneten Collector umwandeln |
| | `toOrdered` | In geordneten Collector umwandeln |
| | `toWindow` | In fensterbasierten Collector umwandeln |
| | `toStatistics` | In statistischen Collector umwandeln |

#### Vollständige Containerspezialisierungsunterstützung

| Spezialisierung | Beschreibung |
|----------------|--------------|
| `Semantic<std::vector<E>>` | Vektor-Stream mit Sortier-, Duplikatentfernungsfunktion usw. |
| `Semantic<std::list<E>>` | Listen-Stream mit Sortier-, Duplikatentfernungsfunktion usw. |

---

## 🏭 Schicht Sieben: semantics.h – Stream-Konstruktionsfabrik

`semantics.h` stellt alle Stream-Konstruktionsfabrikfunktionen bereit.

### 📐 Zahlenbereichserzeugung

| Methode | Beschreibung |
|--------|--------------|
| `useRange(start, end)` | Zahlenstrom über `[start, end)` erzeugen |
| `useRange(start, end, step)` | Zahlenstrom mit Schrittweite erzeugen |
| `useRangeClosed(start, end)` | Zahlenstrom über `[start, end]` erzeugen |
| `useRangeClosed(start, end, step)` | Zahlenstrom mit Schrittweite über geschlossenes Intervall erzeugen |

### ♾️ Unendliche Stromerzeugung

| Methode | Beschreibung |
|--------|--------------|
| `useInfinite(seed, generator)` | Unendliche Iteration ab Startwert |
| `useGenerate(supplier)` | Unendliche Lieferantenaufrufe |
| `useGenerate(supplier, limit)` | Begrenzte Lieferantenaufrufe |
| `useIterate(seed, generator)` | Unendliche Iteration ab Startwert |
| `useIterate(seed, generator, limit)` | Begrenzte Iteration |
| `useRandom()` | Unendlicher Zufallszahlenstrom |
| `useRandom(min, max)` | Unendlicher Zufallszahlenstrom im Bereich |
| `useRandom(min, max, count)` | Begrenzte Zufallszahlen im Bereich |

### 📦 Container- & Elementkonstruktion

| Methode | Beschreibung |
|--------|--------------|
| `useEmpty()` | Leeren Stream erzeugen |
| `useOf(element)` | Ein-Element-Strom |
| `useOf(e1, e2)` | Zwei-Element-Strom |
| `useOf(e1, e2, e3)` | Drei-Element-Strom |
| `useOf({...})` | Stream aus Initialisierungsliste |
| `useFrom(container)` | Stream aus beliebigem Standardcontainer |
| `useFrom({...})` | Stream aus Initialisierungsliste |
| `useRepeat(element, count)` | Element n-mal wiederholen |

### 📝 Textverarbeitung

| Methode | Beschreibung |
|--------|--------------|
| `useBlob(text)` | Zeichenkette byteweise in `char`-Stream zerlegen |
| `useBlob(text, start, end)` | Teilzeichenkette byteweise in `char`-Stream zerlegen |
| `useBlob(istream)` | Zeilen aus Eingabestream lesen |
| `useBlob(istream, delimiter)` | Eingabestream mit Trennzeichen lesen |
| `useText(text)` | Zeichenkette als ganzen Textstream behandeln |
| `useText(text, delimiter)` | Text nach Trennzeichen aufteilen |
| `useText(istream)` | Gesamten Inhalt des Eingabestreams lesen |
| `useText(istream, delimiter)` | Eingabestream mit Trennzeichen lesen |

### 🌐 Unicode-Verarbeitung

| Methode | Beschreibung |
|--------|--------------|
| `useSequence(charsequence)` | Code-Point-Strom aus Zeichensequenz erzeugen |
| `useSequence(charsequence, start, end)` | Code-Point-Strom aus Teilsequenz erzeugen |
| `useSequence(text, encoding)` | Code-Point-Strom aus Text mit Kodierung erzeugen |
| `useSequence(istream, encoding)` | Code-Point-Strom aus Eingabestream mit Kodierung erzeugen |
| `useCharsequence(charsequence)` | Zeichensequenz als ganzen Stream behandeln |
| `useCharsequence(charsequence, delimiter)` | Zeichensequenz nach Trennzeichen aufteilen |
| `useCharsequence(istream, encoding)` | Gesamte Zeichensequenz aus Eingabestream lesen |
| `useCharsequence(istream, delimiter, encoding)` | Zeichensequenz aus Eingabestream mit Trennzeichen lesen |

---

## 🧠 Kernkonzept: Eine indexgesteuerte Datenwelt

Semantic-Cpp abstrahiert die Datenverarbeitung als Operationen auf **„Elementen“** und ihren **„logischen Positionen (Indizes)“**. Dies zu verstehen, ist entscheidend für die Beherrschung der Bibliothek.

### 1. 📐 Grundlegende Index-Transformationen

| Methode | Beschreibung |
|--------|--------------|
| `redirect(fn)` | Kernmethode: Indizes vollständig über benutzerdefinierte Funktion neu zuweisen |
| `reverse()` | Alle Indexlogik umkehren (intern über `redirect` realisiert) |
| `translate(offset)` | Feste Verschiebung |
| `translate(translator)` | Dynamische Verschiebungsfunktion pro Element und Index |

### 2. 📊 Die „autoritäre“ Regel des Sortierens

> ⚠️ **`sort()` überschreibt alles**: Nach dem Aufruf werden alle vorherigen Indexoperationen verworfen; Elemente erhalten neue natürliche Ordnungsindizes basierend auf ihrem Wert.

- `sort()` → Sofort in `OrderedCollectable` materialisiert, nach Elementwert natürlich sortiert
- `sort(comparator)` → Benutzerdefiniert sortiert

### 3. ⚡ Deklarative Parallelverarbeitung

- `parallel(n)` erklärt lediglich die Absicht; Threads werden nicht sofort gestartet
- Terminaloperationen (`toUnordered()`, `count()` usw.) lösen die Parallelverarbeitung aus
- Der Thread-Pool übernimmt automatisch die Aufgabenverteilung und Ergebniszusammenführung

### 4. 🎯 Wahl des richtigen Zielcontainers

| Konvertierungsmethode | Zugrunde liegende Struktur | Leistungsmerkmale | Bester Anwendungsfall |
|----------------------|----------------------------|-------------------|-----------------------|
| `sort()` | `OrderedCollectable` | Nach Werten sortiert materialisiert | Wertbasiertes Sortieren, Paginierung, Zeitreihen |
| `toOrdered()` | `OrderedCollectable` | Beibehaltung der aktuellen Indexreihenfolge | Beibehaltung benutzerdefinierter Indexreihenfolge |
| `toUnordered()` | `UnorderedCollectable` | Ø O(1), höchster Durchsatz | Schnelle Suche, Deduplizierung, Aggregation |
| `toWindow()` | `WindowCollectable` | Auf geordneten Sammlungen basierend | Gleitende / rollende Fensteranalyse |
| `toStatistics()` | `Statistics` | 20+ statistische Methoden | Umfassende statistische Analyse |

---

## 🚀 Schnellstart-Anleitung

### Installation

Platzieren Sie alle Header-Dateien in Ihrem Projektverzeichnis und stellen Sie sicher, dass Ihr Compiler **C++17 oder neuer** unterstützt:

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

## 🎯 Grundlegendes Beispiel: Indizes & Sortierung erleben

```cpp
auto result = semantic::useRange(0, 10)
    .map(int x { return x * x; })
    .redirect(int value, auto index -> long long {
        return index * 2;
    })
    .reverse()
    .sort()                              // erzwingt wertbasiertes Sortieren, überschreibt alle Indexoperationen
    .toVector();

// Ausgabe: 0 1 4 9 16 25 36 49 64 81
```

## ⚡ Parallele Verarbeitung – Beispiel

```cpp
auto count = semantic::useRange(1, 1000)
    .parallel(4)
    .filter(int x { return x % 2 == 0; })
    .toUnordered()
    .count();

// Ausgabe: Anzahl gerader Zahlen: 500
```

## 📊 Statistikanalyse – Beispiel

```cpp
auto stats = semantic::useRange(1, 101)
    .toStatistics<int, double>();

auto avg = stats.average();               // Mittelwert
auto med = stats.median();                // Median
auto std = stats.standardDeviation();     // Standardabweichung
auto q1  = stats.firstQuartile();          // Erstes Quartil
auto q3  = stats.thirdQuartile();          // Drittes Quartil
auto skew = stats.skewness();              // Schiefe
```

## 🔬 Frequenzbereichsanalyse – Beispiel

```cpp
auto freq = data.toUnordered().frequency();
for (const auto& [element, z] : freq) {
    auto magnitude = std::abs(z);  // Verteilungskonzentration
    auto phase     = std::arg(z);  // Phasenlage des Verteilungszentrums
}
```

## 🧮 FFT – Beispiel

```cpp
auto spectrum = semantic::useRange(0, 8)
    .map(int x -> std::complex<double> {
        return {static_cast<double>(x), 0.0};
    })
    .toUnordered()
    .collect(collector::useFFT<double>());
```

---

## ⚡ Leistungsoptimierungs-Empfehlungen

1. 🎯 **Wählen Sie den passenden Container**
   - Gleichheitssuche, unsortierte Aggregation → `toUnordered()`
   - Bereichsabfragen, Sortierung, Paginierung → `toOrdered()` oder `sort()`
   - Echtzeit-Fensteranalyse → `toWindow()`
2. ⚡ **Nutzen Sie Parallelität sinnvoll**: Verwenden Sie `parallel()` bei großen Datenmengen oder rechenintensiver Logik; vermeiden Sie blockierende I/O-Vorgänge.
3. 📐 **Optimieren Sie die Operationsreihenfolge**: Filtern Sie früh, sortieren Sie gezielt.
4. 🔄 **Nutzen Sie Lazy Evaluation**: Zwischenoperationen werden erst bei Terminaloperationen ausgeführt; `takeWhile` und `limit` ermöglichen vorzeitigen Abbruch.

---

## 📊 Vergleich mit der C++-Standardbibliothek & Alternativen

| Merkmal | Semantic-Cpp | C++20/23 Ranges | Traditionelle Schleifen |
|--------|-------------|-----------------|-------------------------|
| 🎯 Kernparadigma | Deklarativ, indexgesteuert | Ansichtsgesteuert, funktionale Komposition | Imperativ, prozedural |
| ⚡ Parallelität | Deklarativ, automatischer Thread-Pool | Erfordert explizite parallele Algorithmen | Manuelle Implementierung |
| 📐 Sortierung & Indizierung | Feinkörnige Indexsteuerung, negative Indizes unterstützt | Destruktives Sortieren | Vollständig manuell |
| 📊 Statistische Analyse | 20+ integrierte statistische Methoden | Nicht integriert | Erfordert Drittbibliotheken |
| 🔬 Frequenzbereichsanalyse | Native DFT / FFT / frequenzbasierte Merkmale | Nicht nativ unterstützt | Erfordert Drittbibliotheken |
| 🧮 Gradientenabstieg | Analytischer + numerischer Dualmodus | Nicht integriert | Erfordert Drittbibliotheken |
| 🌐 Unicode | Native Mehrfachkodierungsunterstützung (UTF‑8/16/32 usw.) | Nicht nativ unterstützt | Manuelle Handhabung |
| 📦 Container-Sammlung | 20+ Standardcontainer vollständig abgedeckt | Teilweise Unterstützung | Manuelle Implementierung |
| 📦 Abhängigkeiten | Null externe Abhängigkeiten, 7 Header | Standardbibliothek | Keine |

---

## 📜 Lizenz

- 📄 **Lizenz**: MIT

---

**Semantic-Cpp — Effiziente, übersichtliche Datenverarbeitungs-Pipelines mit modernem C++ bauen. 🚀**
