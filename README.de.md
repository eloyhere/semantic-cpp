# Semantic-Cpp im Detail: Ein zukunftsweisendes intelligentes Stream-Verarbeitungs-Framework für C++

Semantic-Cpp ist eine vollständig neu gestaltete, moderne C++-Stream-Verarbeitungsbibliothek mit einer "Single-Header, Zero-Dependencies"-Architektur. Ihre Kernfunktionalität ist in der Datei `semantic.h` gebündelt. Die Bibliothek vereint auf kreative Weise die Vorzüge verschiedener Programmierparadigmen:

-   **Die Eleganz und Flüssigkeit der Java Stream API**: Bietet eine deklarative Programmiererfahrung mit Methodenverkettung.
-   **Die Faulheit und Flexibilität von JavaScript Generatoren**: Unterstützt verzögerte Berechnung (Lazy Evaluation) und bedarfsgerechte Datengenerierung.
-   **Die Effizienz und Ordnung von Datenbank-Indizes**: Integriert intelligente Sortier- und indexgetriebene Mechanismen, besonders geeignet für die Verarbeitung von Zeitreihen- und Ereignisdaten.

Im Gegensatz zu traditionellen Datenverarbeitungsansätzen (wie manuell geschriebenen Schleifen oder komplexen asynchronen Callbacks) zielt Semantic-Cpp darauf ab, eine **typsichere, ausdrucksstarke und leistungsstarke** Lösung zu bieten. Ihre zentrale Designphilosophie ist die **präzise Kontrolle des Datenflusses**: Daten fließen nur dann durch die "Verarbeitungspipeline", wenn sie explizit benötigt werden, und die "Reihenfolge" und "Position" des Flusses können über "Indizes" feinjustiert werden, um eine optimale Ressourcennutzung zu erreichen.

---

## Die Kernidee: Eine indexgetriebene Datenwelt

Semantic-Cpp abstrahiert die Datenverarbeitung als Operationen an "Elementen" und deren "logischen Positionen (Indizes)". Dies zu verstehen, ist der Schlüssel zum Beherrschen dieser Bibliothek.

### 1. Grundlegende Index-Transformationen

Der Index bestimmt die logische Reihenfolge eines Elements in der Verarbeitungskette und kann flexibel manipuliert werden:

-   **`redirect(Umleitungsfunktion)`**: Die Kernmethode. Sie können mit einer benutzerdefinierten Funktion die Indizes der Elemente vollständig neu schreiben. Zum Beispiel können Sie Indizes verdoppeln oder neue Indizes basierend auf Elementwerten generieren.
-   **`reverse()`**: Eine Hilfsmethode, intern über `redirect` implementiert, die alle aktuellen Indizes logisch umkehrt (z.B. positive Indizes werden negativ).
-   **`translate(Versatz)`**: Addiert einen festen Versatz zu allen Indizes.

### 2. Die "dominante" Regel der Sortierung

Die Sortieroperation (`sorted`) hat in der Bibliothek die höchste Priorität und ihr Verhalten ist deterministisch:

-   **`sorted()` überschreibt alles**: Egal, welche komplexen Indextransformationen Sie zuvor mit `redirect` oder `reverse` durchgeführt haben – sobald `sorted()` aufgerufen wird, werden **alle vorherigen Indexoperationen überschrieben**. Das System weist den Elementen basierend auf ihren **tatsächlichen Werten** neue Indizes in natürlicher Reihenfolge ab 0 zu.
-   **Sofortige "Materialisierung" als geordnete Sammlung**: Um unnötige wiederholte Sortierkosten in nachfolgenden Operationen zu vermeiden, gibt die Methode `sorted()` **sofort** ein Objekt vom Typ `OrderedCollectable` zurück. Das bedeutet, die Daten sind zu diesem Zeitpunkt bereits gesammelt und sortiert.

### 3. Deklarative Parallelverarbeitung

Parallele Verarbeitung wird einfach und intuitiv:

-   **`parallel(Anzahl Threads)` ist nur eine Deklaration**: Der Aufruf dieser Methode drückt lediglich die Absicht aus, "dass nachfolgende Operationen parallel ausgeführt werden sollen", und gibt die gewünschte Thread-Anzahl an. Es werden **keine Threads sofort gestartet oder Aufgaben übergeben**.
-   **Terminale Operationen lösen Parallelität aus**: Die eigentliche parallele Berechnung wird erst durch den Aufruf **terminaler Operationsmethoden** wie `toUnordered()`, `toOrdered()`, `count()` usw. ausgelöst. Zu diesem Zeitpunkt teilt der integrierte Thread-Pool der Bibliothek die Daten basierend auf der deklarierten Thread-Anzahl auf und überträgt die Aufgaben.
-   **Keine manuelle Verwaltung nötig**: Sie müssen sich nicht um Thread-Erstellung, Aufgabenverteilung oder Ergebniszusammenführung kümmern; die Bibliothek erledigt dies automatisch für Sie.

### 4. Wie wähle ich den endgültigen Datencontainer?

Basierend auf Ihren Leistungsanforderungen und dem Operationstyp können Sie verschiedene terminale Transformationsmethoden wählen:

| Transformationsmethode | Unterliegende Datenstruktur | Leistungsprofil | Beste Anwendungsfälle |
| :--- | :--- | :--- | :--- |
| **`sorted()`** | `std::map<Index, Element>` | O(log n) Zugriff, streng geordnete Elementreihenfolge. | Paging, Bereichsabfragen, Zeitreihenanalyse, laufende Statistiken. |
| **`sorted(comparator)`** | `std::map<Index, Element>` | O(log n) Zugriff, Sortierung nach benutzerdefinierten Regeln. | Paging oder Bereichsabfragen mit benutzerdefinierten Sortierregeln. |
| **`toOrdered()`** | `std::map<Index, Element>` | O(log n) Zugriff, behält die **aktuelle Indexreihenfolge** bei. | Wenn Sie die durch `redirect` definierte Indexreihenfolge beibehalten und geordnete Operationen durchführen möchten. |
| **`toUnordered()`** | `std::unordered_map<Index, Element>` | Durchschnittlich O(1) Zugriff, **höchste Leistung**, aber keine Reihenfolgegarantie. | Schnelle Suche, Deduplizierung, Aggregationsberechnungen – Szenarien, in denen die Reihenfolge irrelevant ist. |
| **`toWindow()`** | Fensteransicht basierend auf `std::map` | O(log n), unterstützt gleitende oder rollierende Fenster auf geordneten Datensätzen. | Echtzeit-Stream-Datenanalyse, gleitende Fensteraggregation, Ereignissitzungsaufteilung. |

> **Wichtiger Hinweis**: `WindowCollectable` (zurückgegeben von `toWindow()`) basiert intern auf einer geordneten Sammlung (implementiert über `toOrdered()`), um sicherzustellen, dass gleitende und rollierende Fensteroperationen korrekt auf einer deterministischen Reihenfolge ausgeführt werden können.

---

## Schnellstart-Anleitung

### Installation

Fügen Sie einfach die Header-Datei `semantic.h` zu Ihrem Projekt hinzu und stellen Sie sicher, dass Ihr Compiler C++17 oder höher unterstützt.

```cpp
#include "semantic.h"
// Optional: Verwenden Sie den semantischen Namensraum
using namespace semantic;
```

### Grundlegendes Beispiel: Indizes und Sortierung erleben

```cpp
#include <iostream>
#include "semantic.h"

int main() {
    auto result = semantic::useRange(0, 10) // 1. Erzeuge einen Integer-Stream von 0 bis 9
        .map(int x { return x * x; })  // 2. Quadriere jedes Element (0,1,4,9...81)
        .redirect(int val, auto idx {  // 3. Index-Umleitung: Verdopple den Index
            return idx * 2; // Jetzt sind die Indizes 0,2,4,6...
        })
        .reverse()                          // 4. Logische Indexumkehr (...,6,4,2,0)
        .sorted()                           // 5. ⚠️ Erzwinge Sortierung nach Elementwerten (1,4,9...)!
        // Alle vorherigen Indexoperationen werden überschrieben, Indizes werden 0,1,2...
        .toList();                          // 6. Sammle in std::vector

    // Ausgabe: 0 1 4 9 16 25 36 49 64 81 (sortiert)
    for (auto& item : result) {
        std::cout << item << " ";
    }
    return 0;
}
```

### Beispiel für Parallelverarbeitung

```cpp
#include <iostream>
#include "semantic.h"

int main() {
    // 1. Baue eine Stream-Pipeline und deklariere, dass 4 Threads verwendet werden sollen.
    auto dataStream = semantic::useRange(1, 1000)
        .parallel(4) // Deklariere Parallelität, noch nicht ausgeführt
        .filter(int x -> bool { return x % 2 == 0; }) // Filtere gerade Zahlen
        .filter(int x, auto index -> bool { return index < 5LL; }); // Filtere Elemente mit Index < 5

    // 2. Die terminale Operation `count()` löst die eigentliche parallele Berechnung aus.
    // Der Thread-Pool startet, Daten werden aufgeteilt, vier Threads zählen parallel, Ergebnisse werden automatisch zusammengeführt.
    auto result = dataStream
        .toUnordered() // In eine ungeordnete Sammlung für parallele Verarbeitung umwandeln
        .count();       // Endgültige Anzahl der Elemente zählen

    std::cout << "Anzahl gefilterter Elemente: " << result << std::endl;
    return 0;
}
```

### Beispiel für Zeitreihen- und Fensteranalyse

```cpp
#include <iostream>
#include "semantic.h"

int main() {
    // Simuliere Zeitreihendaten (z.B. Aktienkurse)
    auto timeSeries = semantic::useFrom(std::vector<double>{1.1, 2.2, 3.3, 4.4, 5.5});

    // 1. In eine Fensteransicht umwandeln
    auto windowStats = timeSeries
        .toWindow()                 // In WindowCollectable umwandeln
        .slide(3, 1)               // Gleitendes Fenster der Größe 3, Schrittweite 1 definieren
        // Fenster 1: {1.1, 2.2, 3.3}
        // Fenster 2: {2.2, 3.3, 4.4}
        // Fenster 3: {3.3, 4.4, 5.5}
        .sub(1, 4)                 // Fenster mit Index 1 bis 3 nehmen (d.h. Fenster 2 und 3)
        .map(auto&& window -> double { // Verarbeite jedes Fenster
            // Berechne den Durchschnitt jedes Fensters
            return window
                .toStatistics<double, double>() // Fenster in Statistics für mathematische Berechnungen umwandeln
                .average();
        })
        .toStatistics<double, double>() // Statistische Berechnungen auf dem Ergebnis (Durchschnittssequenz)
        .summate(); // Summiere alle Fensterdurchschnitte

    std::cout << "Summe der Durchschnitte ausgewählter gleitender Fenster: " << windowStats << std::endl;
    // Ausgabe: Ergebnis von ( (2.2+3.3+4.4)/3 + (3.3+4.4+5.5)/3 )
    return 0;
}
```

---

## Kurzreferenz der Kern-APIs

### Stream-Quellen (Stream Sources)

| Methode | Beschreibung | Beispiel |
| :--- | :--- | :--- |
| `useRange(start, end)` | Erzeugt einen Integer-Stream innerhalb eines numerischen Bereichs. | `useRange(0, 10)` |
| `useFrom(container)` | Erzeugt einen Stream aus einem Standardcontainer (z.B. vector, list). | `useFrom(std::vector{1,2,3})` |
| `useOf(args...)` | Erzeugt einen Stream aus einer variablen Argumentliste. | `useOf(1, 2, 3, 4, 5)` |
| `useText(text, encoding)` | Teilt einen String in einen Stream von Zeichen oder Codepunkten auf. | `useText("Hello", charset::UTF8)` |

### Intermediäre Operationen (Intermediate Operations)

| Methode | Beschreibung | Hinweise |
| :--- | :--- | :--- |
| `map(Transformationsfunktion)` | Wandelt Elemente in eine andere Form um. | Funktion kann `(Element)` oder `(Element, Index)` empfangen. |
| `filter(Prädikatsfunktion)` | Filtert Elemente, die eine Bedingung erfüllen. | Prädikat kann auf `(Element)` oder `(Element, Index)` basieren. |
| `distinct()` | Entfernt doppelte Elemente. | Kann einen benutzerdefinierten `Komparator` übergeben. |
| `limit(n)` | Begrenzt die Anzahl der Elemente im Stream auf die ersten `n`. | |
| `skip(n)` | Überspringt die ersten `n` Elemente im Stream. | |
| `sub(start, end)` | Extrahiert einen Sub-Stream mit Indizes im Bereich `[start, end)`. | Ähnlich wie `substr` bei Strings. |

### Index-Operationen (Index Operations)

| Methode | Beschreibung | Schlüsseleigenschaft |
| :--- | :--- | :--- |
| `redirect(Umleitungsfunktion)` | Kernmethode, erlaubt vollständige Kontrolle über den Index jedes Elements. | Funktionssignatur: `(Element, alter Index) -> neuer Index`. |
| `reverse()` | Kehrt die Logik aller aktuellen Indizes um. | Intern über `redirect` implementiert. |
| `translate(offset)` | Addiert einen festen Versatz zu allen Indizes. | |
| **`sorted()`** | **Erzwingt Sortierung**. Sortiert aufsteigend nach Elementwerten, **überschreibt alle vorhandenen Indizes**. | Gibt sofort ein `OrderedCollectable` zurück. |
| **`sorted(Komparator)`** | Erzwingt Sortierung mit einem benutzerdefinierten Komparator. | Gibt sofort ein `OrderedCollectable` zurück. |

### Parallele Deklaration (Parallel Declaration)

| Methode | Beschreibung | Ausführungszeitpunkt |
| :--- | :--- | :--- |
| `parallel()` | Deklariert die Verwendung der Standard-Parallelstrategie (normalerweise Anzahl der CPU-Kerne). | Wird durch nachfolgende **terminale Operationen** ausgelöst. |
| `parallel(n)` | Deklariert den Wunsch, `n` Threads für die parallele Verarbeitung zu verwenden. | Wird durch nachfolgende **terminale Operationen** ausgelöst. |

### Terminale Transformationen (Terminal Conversions - Löst Berechnung aus)

| Methode | Beschreibung | Interner Status |
| :--- | :--- | :--- |
| `toOrdered()` | Wandelt in eine geordnete Sammlung um, behält die **aktuelle Indexreihenfolge** bei. | Materialisiert als `std::map<Index, Value>`. |
| `toUnordered()` | Wandelt in eine ungeordnete Sammlung um für höchste Leistung. | Materialisiert als `std::unordered_map<Index, Value>`. |
| `toWindow()` | Wandelt in eine Fenstersammlung um, für gleitende/rollierende Analyse. | Intern basierend auf `toOrdered()`. |

### Terminale Operationen (Terminal Actions - Erzeugt Endergebnis)

| Methode | Beschreibung | Rückgabetyp |
| :--- | :--- | :--- |
| `count()` | Zählt die Gesamtzahl der Elemente im Stream. | `Module` (`unsigned long long`) |
| `average()` | Berechnet den Durchschnitt numerischer Elemente. | Durchschnitt des Elementtyps (z.B. `double`). |
| `min()` / `max()` | Findet den minimalen/maximalen Wert im Stream. | `std::optional<Elementtyp>` |
| `reduce(Startwert, Akkumulator)` | Reduziert den Stream auf einen einzelnen Wert (z.B. Summierung). | Typ des Akkumulatorergebnisses. |
| `collect(Kollektor)` | Führt komplexe Aggregationen mit einem benutzerdefinierten Kollektor durch. | Vom Kollektor definierter Rückgabetyp. |
| `toList()` / `toVector()` | Sammelt alle Elemente in einer Liste/Vektor. | `std::vector<Elementtyp>` |
| `toSet()` | Sammelt alle Elemente in einer Menge (dedupliziert). | `std::set<Elementtyp>` |

---

## Fortgeschrittene Themen und Best Practices

### Architektonische Essenz: Lazy Evaluation und präzise Rückrufkontrolle

Jede Stream-Operation basiert auf einem "Generator", der zwei Rückruffunktionen akzeptiert:

-   **`accept(Element, Index)`**: Wenn eine nachgelagerte Operation bereit ist, Daten zu verarbeiten, wird dieser Rückruf aufgerufen, um ein Element "anzufordern".
-   **`interrupt(Element, Index)`**: Wird vor der Verarbeitung jedes Elements aufgerufen. Gibt die Rückgabe `true` zurück, wird die gesamte Verarbeitungskette **sofort beendet**.

Dieser Mechanismus stellt sicher, dass Daten "bei Bedarf abgerufen" werden und die Verarbeitung jederzeit vorzeitig beendet werden kann, um unnötige Berechnungen zu vermeiden.

### Leistungsoptimierungs-Empfehlungen

1.  **Wähle den richtigen Container**:
    *   Gleichheitssuche, Deduplizierung, unsortierte Aggregation → Bevorzuge `toUnordered()`.
    *   Bereichsabfragen, Sortierung, Paging benötigt → Verwende `toOrdered()` oder `sorted()`.
    *   Echtzeit-Fensteranalyse → Verwende `toWindow()`.
2.  **Nutze Parallelität effektiv**:
    *   Bei großen Datenmengen (z.B. >1000) oder rechenintensiver Verarbeitungslogik (`map`, `filter`) kann die Verwendung von `parallel()` in der Regel Leistungsvorteile bringen.
    *   Vermeide blockierende I/O-Operationen in parallelen Streams.
3.  **Optimiere die Operationsreihenfolge**:
    *   **Filtere frühzeitig (`filter`)** : Reduziere die Datenmenge durch `filter`, bevor teure `map`-Transformationen angewendet werden.
    *   **Sortiere klug**: Sortierung ist rechenintensiv. Wenn nachfolgende Operationen (wie `distinct`) nicht von der Reihenfolge abhängen, führe sie vor der Sortierung durch.

### Benutzerdefinierte Kollektoren

Wenn die integrierten terminalen Operationen nicht ausreichen, können Sie benutzerdefinierte Kollektoren für komplexe Reduktionslogik erstellen.

```cpp
// Erstelle einen Kollektor, der Zahlen zu einem formatierten String verbindet
auto myCollector = semantic::collector::useFull<int, std::string, std::string>(
     -> std::string { return ""; }, // Supplier: Liefert initialen Akkumulatorwert
    std::string acc, int val, auto idx -> std::string { // Akkumulator
        if (!acc.empty()) acc += "|";
        return acc + "Num(" + std::to_string(val) + ")";
    },
    std::string a, std::string b -> std::string { // Kombinator (für Parallelität)
        if (a.empty()) return b;
        if (b.empty()) return a;
        return a + "|" + b;
    },
    std::string acc -> std::string { // Finisher: Endgültige Nachbearbeitung
        return "[" + acc + "]";
    }
);

auto result = semantic::useRange(1, 5)
    .toOrdered()         // Berechnung auslösen
    .collect(myCollector); // Benutzerdefinierten Kollektor verwenden

std::cout << result << std::endl; // Ausgabe: [Num(1)|Num(2)|Num(3)|Num(4)]
```

### Textverarbeitungsbeispiel

```cpp
auto text = semantic::useText("Hello 世界！")
    .map(const std::string& text -> std::string {
        return "<" + text + ">";
    })
    .toOrdered()
    .join(" "); // Verbinde alle Zeichen mit Leerzeichen

std::cout << text << std::endl; // Ausgabe: <H><e><l><l><o>< ><世><界><！>
```

---

## Lizenz und Support

-   **Lizenz**: Dieses Projekt ist unter der MIT-Lizenz quelloffen.
-   **Probleme und Feedback**: Wenn Sie Fehler finden oder Vorschläge für neue Funktionen haben, können Sie diese gerne auf der Seite https://github.com/eloyhere/semantic-cpp/issues einreichen.
-   **Diskussion und Austausch**: Sie können auch unter https://github.com/eloyhere/semantic-cpp/discussions eine Diskussion starten.

**Semantic-Cpp** – Erstellen Sie effiziente, klare Datenverarbeitungspipelines mit modernem C++. 🚀
