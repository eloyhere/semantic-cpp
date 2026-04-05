# Semantic-Cpp im Detail: Ein zukunftsorientiertes C++-Framework für intelligente Stream-Verarbeitung

Semantic-Cpp ist eine vollständig neu gestaltete moderne C++-Stream-Verarbeitungsbibliothek, die eine „Single-Header, Zero-Dependency“-Architektur verwendet. Die zentrale Datei `semantic.h` integriert die gesamte Funktionalität der Bibliothek. Die Bibliothek verbindet kreativ die besten Merkmale mehrerer Programmierparadigmen:

-   **Die Eleganz und Flüssigkeit der Java Stream API**: mit kettenartigen Aufrufen und einem deklarativen Programmiererlebnis.
-   **Die Trägheit und Flexibilität von JavaScript-Generatoren**: mit Unterstützung für verzögerte Auswertung und bedarfsgesteuerte Datengenerierung.
-   **Die Effizienz und Ordnung von Datenbank-Indizes**: mit integrierten intelligenten Sortier- und indexgesteuerten Mechanismen – besonders geeignet für Zeitreihen- und Ereignisdaten.

Im Gegensatz zu traditionellen Datenverarbeitungsansätzen (wie handgeschriebenen Schleifen oder komplexen asynchronen Callbacks) zielt Semantic-Cpp auf eine **typsichere, hoch ausdrucksstarke und hochperformante** Lösung ab. Die zentrale Design-Philosophie ist die **präzise Steuerung des Datenflusses**: Daten fließen nur dann durch die „Verarbeitungspipeline“, wenn dies explizit erforderlich ist, und die „Reihenfolge“ sowie „Position“ dieses Flusses können über „Indizes“ fein abgestimmt werden – so wird eine optimale Ressourcennutzung erreicht.

---

## Die Kernseele: Eine indexgesteuerte Datenwelt

Semantic-Cpp abstrahiert die Datenverarbeitung als Operationen auf „Elementen“ und deren „logischen Positionen (Indizes)“. Das Verständnis dieses Konzepts ist der Schlüssel zum Meistern der Bibliothek.

### 1. Grundlegende Index-Transformationen
Indizes bestimmen die logische Reihenfolge der Elemente in der Verarbeitungskette und lassen sich flexibel manipulieren:
-   **`redirect(Weiterleitungsfunktion)`**: die zentrale Methode. Sie können den Index eines Elements mit einer benutzerdefinierten Funktion vollständig neu schreiben – z. B. den Index verdoppeln oder einen neuen Index basierend auf dem Elementwert erzeugen.
-   **`reverse()`**: eine praktische Methode, die intern über `redirect` implementiert ist und die aktuellen Indizes logisch umkehrt (z. B. positive Indizes werden negativ).
-   **`translate(Offset)`**: addiert einen festen Offset zu allen Indizes.

### 2. Die „überlagernden“ Regeln der Sortierung
Die Sortieroperation (`sorted`) hat in der Bibliothek die höchste Priorität und verhält sich deterministisch:
-   **`sorted()` überschreibt alles**: Egal wie komplex die vorherigen Index-Transformationen über `redirect` oder `reverse` auch waren – ein Aufruf von `sorted()` **überschreibt** alle früheren Index-Operationen. Das System weist den Elementen basierend auf ihren **tatsächlichen Werten** neue natürliche Indizes ab 0 zu.
-   **Sofortige Materialisierung in eine geordnete Sammlung**: Um spätere wiederholte Sortierungen zu vermeiden, gibt die Methode `sorted()` **sofort** ein Objekt vom Typ `OrderedCollectable` zurück. Die Daten sind damit bereits gesammelt und sortiert.

### 3. Deklarative Parallelverarbeitung
Parallelverarbeitung wird bemerkenswert einfach und intuitiv:
-   **`parallel(Anzahl Threads)` ist lediglich eine Deklaration**: Der Aufruf dieser Methode drückt lediglich die Absicht aus „Ich möchte, dass nachfolgende Operationen parallel ausgeführt werden“ und gibt die gewünschte Thread-Anzahl an; **es werden keine Threads gestartet oder Aufgaben sofort übergeben**.
-   **Terminal-Operationen lösen die Parallelität aus**: Die echte parallele Berechnung wird erst bei Aufruf einer **Terminal-Operation** wie `toUnordered()`, `toOrdered()`, `count()` usw. ausgelöst. Dann teilt der integrierte Thread-Pool der Bibliothek die Daten automatisch auf und übermittelt die Aufgaben entsprechend der angegebenen Thread-Anzahl.
-   **Keine manuelle Verwaltung nötig**: Sie müssen sich nicht um Thread-Erstellung, Aufgabenverteilung oder Ergebniszusammenführung kümmern – die Bibliothek erledigt alles automatisch.

### 4. Welcher finale Datencontainer ist der richtige?
Je nach Leistungsanforderungen und Operationstyp können Sie unterschiedliche Terminal-Konvertierungsmethoden wählen:

| Konvertierungsmethode | Zugrunde liegende Datenstruktur | Leistungsmerkmale | Beste Einsatzszenarien |
| :--- | :--- | :--- | :--- |
| **`sorted()`** | `std::map<Index, Element>` | O(log n) Zugriff, streng erhaltene Elementreihenfolge. | Paginierung, Bereichsabfragen, Zeitreihenanalyse, rollierende Statistiken. |
| **`sorted(comparator)`** | `std::map<Index, Element>` | O(log n) Zugriff, Sortierung nach benutzerdefinierten Regeln. | Paginierung oder Bereichsabfragen mit eigener Sortierung. |
| **`toOrdered()`** | `std::map<Index, Element>` | O(log n) Zugriff, Erhalt der **aktuellen Index**-Reihenfolge. | Wenn Sie die durch Operationen wie `redirect` definierte Index-Reihenfolge beibehalten und geordnete Operationen ausführen möchten. |
| **`toUnordered()`** | `std::unordered_map<Index, Element>` | Durchschnittlich O(1) Zugriff, **höchste Performance**, Reihenfolge jedoch nicht garantiert. | Schnelle Nachschläge, Deduplizierungsstatistiken, Aggregationsberechnungen – Szenarien, in denen die Reihenfolge keine Rolle spielt. |
| **`toWindow()`** | Map-basierte Fensteransicht | O(log n), Unterstützung für Sliding- oder Rolling-Windows auf geordneten Datensätzen. | Echtzeit-Stream-Analyse, Sliding-Window-Aggregation, Ereignis-Session-Aufteilung. |

> **Wichtiger Hinweis**: `WindowCollectable` (Rückgabewert von `toWindow()`) basiert intern auf einer geordneten Sammlung (implementiert über `toOrdered()`), damit Sliding- und Rolling-Operationen auf einer deterministischen Reihenfolge korrekt ausgeführt werden können.

---

## Schnellstart-Anleitung

### Installation
Legen Sie einfach die Header-Datei `semantic.h` in Ihr Projekt und stellen Sie sicher, dass Ihr Compiler C++17 oder höher unterstützt.
```cpp
#include "semantic.h"
// Optional: Semantic-Namespace nutzen
using namespace semantic;
```

### Grundbeispiel: Indizes und Sortierung erleben
```cpp
#include <iostream>
#include "semantic.h"

int main() {
    auto result = semantic::useRange(0, 10)   // 1. Erzeugt einen Integer-Stream von 0 bis 9
        .map([](int x) -> int { return x * x; })    // 2. Quadriert jedes Element (0,1,4,9...81)
        .redirect([](int value, auto index) -> long long {    // 3. Index-Weiterleitung: Index verdoppeln
            return index * 2;                  // Indizes sind jetzt 0,2,4,6...
        })
        .reverse()                           // 4. Logische Umkehrung der Indizes (...,6,4,2,0)
        .sorted()                            // 5. ⚠️ Erzwingt Neusortierung nach Elementwert (1,4,9...)!
                                             //    Alle vorherigen Index-Operationen werden überschrieben; Indizes werden 0,1,2...
        .toList();                           // 6. Sammelt in std::vector

    // Ausgabe: 0 1 4 9 16 25 36 49 64 81 (sortiert)
    for (auto& item : result) {
        std::cout << item << " ";
    }
    return 0;
}
```

### Parallelverarbeitungs-Beispiel
```cpp
#include <iostream>
#include "semantic.h"

int main() {
    // 1. Erzeugt eine Stream-Verarbeitungspipeline und deklariert den Wunsch nach 4 Threads für parallele Ausführung.
    auto dataStream = semantic::useRange(1, 1000)
        .parallel(4)                         // Parallelität deklarieren; noch nicht ausgeführt
        .filter([](int x) -> bool {
            return x % 2 == 0;               // Nur gerade Zahlen filtern
        })
        .filter([](int x, auto index) -> bool {
            return index < 5LL;              // Weiteres Filtern nach logischem Index < 5
        });

    // 2. Die Terminal-Operation `count()` löst die echte parallele Berechnung aus
    //    Der Thread-Pool startet, Daten werden aufgeteilt, vier Threads zählen gleichzeitig, Ergebnisse werden automatisch zusammengeführt.
    auto result = dataStream
        .toUnordered()                       // In ungeordnete Sammlung für parallele Verarbeitung konvertieren
        .count();                            // Anzahl der finalen Elemente zählen

    std::cout << "Anzahl der Elemente nach Filterung: " << result << std::endl;
    return 0;
}
```

### Zeitreihen- und Window-Analyse-Beispiel
```cpp
#include <iostream>
#include "semantic.h"

int main() {
    // Simuliert einen Zeitreihen-Datensatz (z. B. Aktienkurse)
    auto timeSeries = semantic::useFrom(std::vector<double>{1.1, 2.2, 3.3, 4.4, 5.5});

    // 1. In Window-Ansicht konvertieren
    auto windowStats = timeSeries
        .toWindow()                     // In WindowCollectable konvertieren
        .slide(3, 1)                    // Sliding-Window der Größe 3 mit Schritt 1 definieren
                                        // Fenster 1: {1.1, 2.2, 3.3}
                                        // Fenster 2: {2.2, 3.3, 4.4}
                                        // Fenster 3: {3.3, 4.4, 5.5}
        .sub(1, 4)                      // Fenster mit Indizes 1 bis 3 nehmen (d. h. Fenster 2 und 3)
        .map([](auto&& window) -> double { // Jedes Fenster verarbeiten
            // Durchschnitt jedes Fensters berechnen
            return window
                .toStatistics<double, double>() // Fenster in Statistics für mathematische Operationen umwandeln
                .average();
        })
        .toStatistics<double, double>() // Geordnete Statistik der Durchschnittswerte erstellen
        .summate();                     // Summe aller ausgewählten Fenster-Durchschnitte

    std::cout << "Summe der Durchschnittswerte der ausgewählten Sliding-Windows: " << windowStats << std::endl;
    // Ausgabe: Ergebnis von ( (2.2+3.3+4.4)/3 + (3.3+4.4+5.5)/3 )
    return 0;
}
```

---

## Core API – Schnellreferenz

### Stream-Erzeuger (Stream Sources)
| Methode | Beschreibung | Beispiel |
| :--- | :--- | :--- |
| `useRange(start, end)` | Erzeugt einen Integer-Stream innerhalb eines numerischen Bereichs. | `useRange(0, 10)` |
| `useFrom(container)` | Erzeugt einen Stream aus einem Standard-Container (z. B. vector, list). | `useFrom(std::vector{1,2,3})` |
| `useOf(args...)` | Erzeugt einen Stream aus einer variadischen Argumentliste. | `useOf(1, 2, 3, 4, 5)` |
| `useBlob(text)` | Zerlegt einen String zeichenweise in einen Stream. | `useBlob("Hello")` |
| `useBlob(text, start, end)` | Zerlegt einen String zeichenweise in einem Bereich in einen Stream. | `useBlob("Hello", 0, 3)` |
| `useBlob(istream)` | Zerlegt einen Input-Stream zeichenweise in einen Stream. | `useBlob(istream)` |
| `useBlob(istream, start, end)` | Zerlegt einen Input-Stream zeichenweise in einem Bereich in einen Stream. | `useBlob(istream, 0, 3)` |
| `useText(text)` | Behandelt den gesamten Text als einzelnes Element im Stream. | `useText("Hello")` |
| `useText(text, delimiter)` | Zerlegt Text anhand eines Trennzeichens in einen Stream. | `useText("Hello", 'e')` |

### Intermediate Operations
| Methode | Beschreibung | Hinweise |
| :--- | :--- | :--- |
| `map(Transformationsfunktion)` | Wandelt Elemente in eine andere Form um. | Die Funktion kann `(Element)` oder `(Element, Index)` erhalten. |
| `filter(Prädikatsfunktion)` | Filtert Elemente, die die Bedingung erfüllen. | Prädikat kann auf `(Element)` oder `(Element, Index)` basieren. |
| `distinct()` | Entfernt doppelte Elemente. | Ein benutzerdefinierter Comparator kann übergeben werden. |
| `limit(n)` | Begrenzt den Stream auf die ersten `n` Elemente. | |
| `skip(n)` | Überspringt die ersten `n` Elemente des Streams. | |
| `sub(start, end)` | Liefert einen Teil-Stream mit Indizes im Bereich `[start, end)`. | Ähnlich wie `substr` bei Strings. |

### Index-Operationen
| Methode | Beschreibung | Wichtige Merkmale |
| :--- | :--- | :--- |
| `redirect(Weiterleitungsfunktion)` | Kernmethode zur vollständigen Kontrolle jedes Element-Index. | Funktionssignatur: `(Element, alter Index) -> neuer Index`. |
| `reverse()` | Kehrt die Indizes aller aktuellen Elemente logisch um. | Intern über `redirect` implementiert. |
| `translate(offset)` | Addiert einen festen Offset zu allen Element-Indizes. | |
| **`sorted()`** | **Erzwingt Sortierung**. Sortiert Elemente aufsteigend nach Wert und **überschreibt alle vorhandenen Indizes**. | Gibt sofort `OrderedCollectable` zurück. |
| **`sorted(comparator)`** | Erzwingt Sortierung mit benutzerdefiniertem Comparator. | Gibt sofort `OrderedCollectable` zurück. |

### Parallel-Deklaration
| Methode | Beschreibung | Ausführungszeitpunkt |
| :--- | :--- | :--- |
| `parallel()` | Deklariert die Standard-Parallel-Strategie (meist Anzahl der CPU-Kerne). | Wird durch nachfolgende **Terminal-Operationen** ausgelöst. |
| `parallel(n)` | Deklariert den Wunsch, `n` Threads für parallele Verarbeitung zu verwenden. | Wird durch nachfolgende **Terminal-Operationen** ausgelöst. |

### Terminal Conversions (lösen Berechnung aus)
| Methode | Beschreibung | Interner Zustand |
| :--- | :--- | :--- |
| `toOrdered()` | Konvertiert in eine geordnete Sammlung und behält die **aktuelle Index-Reihenfolge** bei. | Materialisiert als `std::map<Index, Value>`. |
| `toUnordered()` | Konvertiert in eine ungeordnete Sammlung für maximale Performance. | Materialisiert als `std::unordered_map<Index, Value>`. |
| `toWindow()` | Konvertiert in eine Fenster-Sammlung für Sliding-/Rolling-Analyse. | Intern basierend auf `toOrdered()`. |

### Terminal Actions (erzeugen finales Ergebnis)
| Methode | Beschreibung | Rückgabetyp |
| :--- | :--- | :--- |
| `anyMatch(predicate)` | Prüft, ob mindestens ein Element die Bedingung erfüllt; bricht bei Fund sofort ab. | Boolean |
| `allMatch(predicate)` | Prüft, ob alle Elemente die Bedingung erfüllen; bricht bei erstem Fehlschlag ab. | Boolean |
| `noneMatch(predicate)` | Prüft, ob kein Element die Bedingung erfüllt; bricht bei erstem Erfolg ab. | Boolean |
| `forEach(consumer)` | Iteriert über alle Elemente im Stream. | Void |
| `count()` | Zählt die Gesamtzahl der Elemente im Stream. | `Module` (`unsigned long long`) |
| `average()` | Berechnet den Durchschnitt numerischer Elemente. | Durchschnitt des Elementtyps (z. B. `double`). |
| `findAny()` | Findet ein beliebiges Element zufällig. | Ein zufälliges Element aus dem Stream. |
| `findFirst()` | Findet das erste Element. | Das erste Element im Stream. |
| `findLast()` | Findet das letzte Element. | Das letzte Element im Stream. |
| `findAt(gegebenenfalls negativer Index)` | Findet das n-te Element; bei negativem Wert das (size + index)-te Element. | Das Element an der angegebenen Index-Position. |
| `findMinimum()` / `findMaximum()` | Findet den Minimal-/Maximalwert im Stream. | `std::optional<ElementType>` |
| `reduce(accumulator)` | Reduziert den Stream auf einen einzelnen Wert (z. B. Summe). | Typ des Accumulator-Ergebnisses. |
| `reduce(identity, accumulator)` | Reduziert den Stream auf einen einzelnen Wert (z. B. Summe). | Typ des Accumulator-Ergebnisses. |
| `collect(collector)` | Führt komplexe Aggregation mit einem benutzerdefinierten Collector durch. | Vom Collector definierter Typ. |
| `toList()` / `toVector()` | Sammelt alle Elemente in eine Liste/Vektor. | `std::vector<E>` |
| `toSet()` | Sammelt alle Elemente in ein Set (dedupliziert). | `std::set<ElementType>` |
| `group(keyExtractor)` | Gruppiert in eine Map (dedupliziert). | `std::map<K, std::vector<E>>` |
| `toMap(keyExtractor)` | Sammelt in eine Map (dedupliziert). | `std::map<K, E>` |

---

## Fortgeschrittene Themen und Best Practices

### Architektur-Essenz: Trägheit und präzise Callback-Steuerung
Hinter jeder Stream-Operation steht ein „Generator“, der zwei Callback-Funktionen akzeptiert:
-   **`accept(Element, Index)`**: Wird von nachgelagerten Operationen aufgerufen, sobald sie bereit sind, Daten zu empfangen – „pullt“ ein Element bei Bedarf.
-   **`interrupt(Element, Index)`**: Wird vor der Verarbeitung jedes Elements aufgerufen; gibt er `true` zurück, wird die gesamte Verarbeitungskette **sofort beendet**.
Dieser Mechanismus stellt sicher, dass Daten „on-demand“ gezogen werden und die Verarbeitung jederzeit vorzeitig abgebrochen werden kann – unnötige Berechnungen werden vermieden.

### Performance-Optimierungsempfehlungen
1.  **Den richtigen Container wählen**:
    -   Für Gleichheitsnachschläge, Deduplizierung oder unsortierte Aggregation → bevorzugen Sie `toUnordered()`.
    -   Für Bereichsabfragen, Sortierung oder Paginierung → verwenden Sie `toOrdered()` oder `sorted()`.
    -   Für Echtzeit-Window-Analyse → verwenden Sie `toWindow()`.
2.  **Parallelität sinnvoll einsetzen**:
    -   Parallelität über `parallel()` lohnt sich meist bei großen Datensätzen (z. B. > 1.000 Elemente) oder rechenintensiven Operationen (`map`, `filter`).
    -   Vermeiden Sie blockierende I/O-Operationen innerhalb paralleler Streams.
3.  **Operationsreihenfolge optimieren**:
    -   **Früh filtern (`filter`)**: Reduzieren Sie die Datenmenge mit `filter`, bevor teure `map`-Transformationen erfolgen.
    -   **Sortierung strategisch platzieren**: Sortierung ist teuer. Wenn nachfolgende Operationen (z. B. `distinct`) keine Reihenfolge benötigen, führen Sie diese vorher aus.

### Benutzerdefinierte Collector
Wenn die integrierten Terminal-Operationen nicht ausreichen, können Sie eigene Collector erstellen, um komplexe Reduktionslogik zu implementieren.
```cpp
// Erzeugt einen Collector, der Zahlen in eine speziell formatierte Zeichenkette verbindet
auto myCollector = semantic::collector::useFull<int, std::string, std::string>(
     []() -> std::string { return ""; }, // Supplier: initialer Akkumulator-Wert
    [](std::string acc, int val, auto idx) -> std::string { // Akkumulator
        if (!acc.empty()) acc += "|";
        return acc + "Num(" + std::to_string(val) + ")";
    },
    [](std::string a, std::string b) -> std::string { // Combiner (für Parallelität)
        if (a.empty()) return b;
        if (b.empty()) return a;
        return a + "|" + b;
    },
    [](std::string acc) -> std::string { // Finisher: abschließende Verarbeitung des Ergebnisses
        return "[" + acc + "]";
    }
);

auto result = semantic::useRange(1, 5)
    .toOrdered() // Berechnung auslösen
    .collect(myCollector); // Benutzerdefinierten Collector verwenden

std::cout << result << std::endl; // Ausgabe: [Num(1)|Num(2)|Num(3)|Num(4)]
```

### Textverarbeitungs-Beispiel
```cpp
auto text = semantic::useText("Hello 世界！")
    .map([](const std::string& text) -> std::string {
        return "<" + text + ">";
    })
    .toOrdered()
    .join(" "); // Alle Zeichen mit Leerzeichen verbinden

std::cout << text << std::endl;
// Ausgabe: <H><e><l><l><o>< ><世><界><！>
```

---

### Vergleich mit der C++-Standardbibliothek und anderen Mitbewerbern

Um Ihnen die Design-Positionierung und Einsatzszenarien von Semantic-Cpp besser zu verdeutlichen, vergleicht die folgende Tabelle die Bibliothek mit mehreren gängigen Datenverarbeitungslösungen der C++-Community.

| Merkmal / Bibliothek | **Semantic-Cpp** | **C++20/23 `std::ranges` + `std::views`** | **Range-v3-Bibliothek** | **Traditionelle handgeschriebene Schleifen** |
| :--- | :--- | :--- | :--- | :--- |
| **Kernparadigma** | **Deklarative, indexgesteuerte** Stream-Verarbeitung. Daten werden als „Element + logischer Index“-Pipeline abstrahiert und betonen **Reihenfolgensteuerung**. | **Deklarative, view-gesteuerte** funktionale Komposition. Stellt Adapter (`views::transform`, `views::filter`) für verzögerte Berechnung bereit. | **Deklarative, range-gesteuerte** funktionale Komposition. Blaupause und Vorgänger von `std::ranges` mit umfangreicherem Funktionsumfang. | **Imperative, prozedurale** Programmierung. Direkte Manipulation von Iteratoren und Containern. |
| **Kern-Design-Philosophie** | **Präzise Steuerung** der logischen Position und Flussreihenfolge der Daten in der Pipeline über **Indizes** – optimale Ressourcennutzung. | Bietet komponierbare, verzögert ausgewertete **View-Adapter** für effiziente generische Algorithmen. | Bietet ein vollständiges, komponierbares Set aus **Range-Algorithmen und Views** – Grundstein der modernen C++-funktionalen Programmierung. | Die vollständige Kontrolle über Berechnungsfluss und Zustand liegt allein beim Entwickler. |
| **Parallel-Unterstützung** | **Deklarative Parallelität**. `.parallel(n)` deklariert die Absicht; Terminal-Operationen lösen den Thread-Pool automatisch aus. Keine manuelle Verwaltung nötig. | C++17/20 stellt parallele Algorithmen (`std::for_each(std::execution::par, ...)`) bereit, diese sind jedoch **nicht deklarativ** und müssen mit Views kombiniert werden. | Bietet keine direkten parallelen Algorithmen, kann aber mit externen Bibliotheken wie TBB oder HPX kombiniert werden. | Erfordert manuelle Implementierung (z. B. `std::thread`, `std::async` oder parallele Algorithmen); hohe Komplexität. |
| **Sortierung & Indizierung** | **`sorted()` hat höchste Priorität** und überschreibt alle vorherigen Index-Transformationen. Bietet feingranulare Operationen wie `redirect` und `reverse`; ein Kernmerkmal. | Stellt Sortieralgorithmen (`std::ranges::sort`) bereit, diese sind jedoch **in-place und destruktiv** und brechen die View-Kette. Kein eigenes „Index“-Konzept. | Ähnlich wie `std::ranges`; Sortierung ist destruktiv. Keine „Index“-Abstraktion. | Der Entwickler muss Sortierlogik selbst implementieren und Datenbeziehungen vor/nach der Sortierung verwalten. |
| **Window-/Sliding-Analyse** | **Native Unterstützung**. `.toWindow()` zusammen mit `.slide()`, `.tumble()` etc. baut Sliding-/Rolling-Windows direkt auf; First-Class-Citizens für fortgeschrittene Analysen. | Nicht nativ unterstützt. Erfordert Kombination mehrerer Views (z. B. `views::slide` in C++23 oder `views::adjacent`) und manuelle Behandlung; Code relativ komplex. | Bietet Komponenten wie `ranges::views::slide` (vor C++20), fortgeschrittene Window-Aggregation muss jedoch manuell zusammengesetzt werden. | Erfordert handgeschriebene verschachtelte Schleifen und Zustandsverwaltung; Code umfangreich und fehleranfällig. |
| **Datenstrukturen** | Klar auf `std::map` (geordnet), `std::unordered_map` (ungeordnet), `std::vector` usw. abgebildet. Terminal-Operationen bestimmen die finale Struktur. | Algorithmen arbeiten auf Ranges ohne erzwungenen finalen Container. `std::ranges::to` (C++23) oder handgeschriebener Code speichert Ergebnisse. | Ähnlich wie `std::ranges`; Ergebnisse werden über `ranges::to<Container>` gespeichert. | Vollständig vom Entwickler ausgewählt und verwaltet. |
| **Benutzerfreundlichkeit & Ausdruckskraft** | **Hoch**. Flüssiger kettenartiger Stil, API nach Vorbild von Java Stream; geringe Lernkurve. Fokussiert auf „was“ statt „wie“. | **Mittel**. View-Komposition sehr mächtig, aber Syntax (Pipe-Operator `\|`, Projektionen `std::identity`) hat Lernkurve für Einsteiger; Compiler-Fehlermeldungen können komplex sein. | **Mittel-Hoch**. Reichhaltigstes Set an Views und Algorithmen, aber steilste Lernkurve. | **Niedrig**. Komplexe Logik führt zu großem Codeumfang, unklarer Absicht und leichter Einführung von Fehlern. |
| **Performance-Merkmale** | Optimiert durch vordefinierte Datenstrukturen bei Index-Steuerung und Window-Berechnung. Deklarative Parallelität vereinfacht die Nebenläufigkeitsprogrammierung. | **Höchste Performance**. Verzögerte View-Komposition und Compile-Time-Optimierungen erzeugen Code, der mit handgeschriebenen Schleifen vergleichbar oder besser ist (z. B. Eliminierung temporärer Zwischenobjekte). | Wie `std::ranges`; Performance ist eines der Kernziele. | **Theoretisches Maximum hoch**. Erfahrene Entwickler können extreme Mikro-Optimierungen erreichen, Implementierungs- und Wartungskosten sind jedoch sehr hoch. |
| **Typische Einsatzszenarien** | 1. **Zeitreihen-/Ereignis-Stream-Verarbeitung** (Logs, Sensordaten).<br>2. Daten-Transformationen mit **komplexer Reihenfolgensteuerung**.<br>3. **Deklarative Parallelberechnung**.<br>4. **Echtzeit-Sliding-Window-Analyse**. | 1. **Allgemeine, hochperformante Container-Daten-Transformation und -Filterung**.<br>2. Erstellung wiederverwendbarer **generischer Komponenten**.<br>3. Nahtlose Integration mit dem bestehenden STL-Algorithmus- und Container-Ökosystem. | 1. Moderne Range-Bibliotheksfunktionen für Projekte, die C++20 nicht nutzen können.<br>2. Forschung und Experimente mit den neuesten Range-Vorschlägen. | 1. Performance-kritische Szenarien mit extrem einfacher Logik.<br>2. Spezielle Low-Level-Anforderungen, die keine Bibliothek erfüllen kann. |
| **Abhängigkeiten & Integration** | **Zero-Dependency, Single-Header**. Extrem einfache Integration. | Teil der C++20/23-Standardbibliothek; keine zusätzlichen Abhängigkeiten. | Muss als Drittanbieter-Bibliothek integriert werden; funktionsreich, erhöht aber Projektabhängigkeiten. | Keine. |

**Zusammenfassende Empfehlungen:**
- **Wählen Sie Semantic-Cpp**, wenn Ihr Projekt **stark von feingranularer Steuerung der Datenreihenfolge/Indizes** abhängt, **komplexe Sliding-Window-Analysen** benötigt oder Sie **deklarative Parallelität mit geringster kognitiver Last** wünschen. Semantic-Cpp bietet eine hochabstrakte, maßgeschneiderte Lösung.
- **Wählen Sie `std::ranges`**, wenn Ihr Projekt bereits C++20/23 nutzt und Ihre Anforderungen **allgemeine, hochperformante Daten-Transformation und -Abfragen** sind, Sie nahtlose Integration mit dem STL-Ökosystem wünschen und mit einer moderaten Lernkurve einverstanden sind. `std::ranges` ist die standardmäßigste und zukunftsverträglichste Wahl.
- **Wählen Sie Range-v3**, wenn Compiler-Einschränkungen C++20 verhindern, Sie aber dennoch Funktionalität ähnlich wie `std::ranges` benötigen.
- **Wählen Sie handgeschriebene Schleifen** nur, wenn die Logik extrem einfach ist, Sie Nanosekunden-Performance benötigen und Bibliotheksabstraktionen tatsächlich zum Flaschenhals werden.

---

## Lizenz und Support
- **Lizenz**: Dieses Projekt wird unter der MIT-Lizenz veröffentlicht.
- **Fehler und Feedback**: Bei Fehlern oder Funktionsvorschlägen reichen Sie diese bitte auf der Seite https://github.com/eloyhere/semantic-cpp/issues ein.
- **Diskussion und Austausch**: Sie können auch eine Diskussion unter https://github.com/eloyhere/semantic-cpp/discussions starten.

**Semantic-Cpp** — Effiziente, klare Datenverarbeitungspipelines mit modernem C++ bauen. 🚀