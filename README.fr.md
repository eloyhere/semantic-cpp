# semantic-cpp – Une bibliothèque moderne de streams C++ avec sémantique temporelle

***
**Guidée par Jonathan Wakely**  
(Maintainer de GCC libstdc++ et membre du comité ISO C++)
***

**semantic-cpp** est une bibliothèque légère et performante de traitement de streams pour C++17, composée d’un unique fichier d’en-tête et d’un fichier d’implémentation séparé. Elle combine la fluidité des Java Streams, la paresse des générateurs JavaScript, le mappage d’ordre des index de bases de données et la conscience temporelle indispensable aux applications financières, IoT et systèmes orientés événements.

Idées clés qui la rendent unique :

- Chaque élément porte un **Timestamp** (index `long long` signé pouvant être négatif).
- **Module** est un `unsigned long long` utilisé pour les comptages et les niveaux de concurrence.
- Les streams restent paresseux jusqu’à leur matérialisation avec `.toOrdered()`, `.toUnordered()`, `.toWindow()` ou `.toStatistics()`.
- Après matérialisation, il est possible de continuer à enchaîner les opérations — la bibliothèque supporte délibérément les streams « post-terminaux ».
- Support complet de l’exécution parallèle, des fenêtres glissantes et roulantes, ainsi que des opérations statistiques avancées.

## Pourquoi semantic-cpp ?

| Fonctionnalité                       | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                   |
|--------------------------------------|-------------|-------------|-----------|--------------|------------------------------------------------|
| Évaluation paresseuse                | Oui         | Oui         | Oui       | Non          | Oui                                            |
| Indices temporels (timestamps signés)| Non         | Non         | Non       | Non          | Oui (concept central)                          |
| Fenêtres glissantes / roulantes      | Non         | Non         | Non       | Non          | Oui (support de première classe)               |
| Opérations statistiques intégrées    | Non         | Non         | Non       | Non          | Oui (moyenne, médiane, mode, kurtosis, …)      |
| Exécution parallèle (opt-in)         | Oui         | Non         | Oui       | Non          | Oui (pool global ou personnalisé)              |
| Chaînage après opération terminale   | Non         | Non         | Non       | Non          | Oui (streams post-terminaux)                   |
| Fichier unique + implémentation, C++17| Non        | Oui         | Oui       | Oui          | Oui                                            |

Si vous avez déjà écrit plusieurs fois le même code de fenêtrage ou de statistiques pour des données de marché, des flux de capteurs ou de l’analyse de logs, semantic-cpp élimine ce code répétitif.

## Démarrage rapide

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // Crée un stream de 100 nombres avec timestamps 0..99
    auto stream = Generative<int>{}.of(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream.filter([](int x) { return x % 2 == 0; })   // nombres pairs uniquement
          .parallel(8)                               // utilise 8 threads
          .toWindow()                                // matérialise avec support des fenêtres
          .getSlidingWindows(10, 5)                  // fenêtres de taille 10, pas 5
          .toVector();

    // Exemple statistique
    auto stats = Generative<int>{}.of(1,2,3,4,5,6,7,8,9,10).toStatistics();
    std::cout << stats.mean()   << "\n";   // 5.5
    std::cout << stats.median() << "\n";   // 5.5
    std::cout << stats.mode()   << "\n";   // en cas de multimodalité, retourne le premier trouvé
    stats.cout();

    return 0;
}
```

## Concepts principaux

### 1. Generative<E> – Fabrique de streams

`Generative<E>` offre une interface pratique pour créer des streams :

```cpp
Generative<int> gen;
auto s = gen.of(1, 2, 3, 4, 5);
auto empty = gen.empty();
auto filled = gen.fill(42, 1'000'000);
auto ranged = gen.range(0, 100, 5);
```

Toutes les fonctions de fabrique retournent un stream paresseux `Semantic<E>`.

### 2. Semantic<E> – Le stream paresseux

```cpp
Semantic<int> s = of<int>(1, 2, 3, 4, 5);
```

Opérations classiques disponibles :

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

Les éléments portent des timestamps que l’on peut manipuler :

```cpp
s.translate(+1000)                                      // décale tous les timestamps
 .redirect([](int x, Timestamp t){ return x * 10; })   // logique personnalisée de timestamp
```

### 3. Matérialisation

Avant d’utiliser les opérations de collecte (count(), toVector(), cout(), …), il faut appeler l’un des quatre convertisseurs terminaux :

```cpp
.toOrdered()      // conserve l’ordre, permet le tri
.toUnordered()    // la plus rapide, sans garantie d’ordre
.toWindow()       // ordonné + API complète de fenêtres
.toStatistics<D>  // ordonné + méthodes statistiques
```

Après matérialisation, le chaînage reste possible :

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 4. Fenêtrage

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // taille 30, pas 10
```

Ou générer un stream de fenêtres :

```cpp
stream.toWindow()
    .windowStream(50, 20)           // émet un std::vector<E> pour chaque fenêtre
    .map([](const std::vector<double>& w) { return mean(w); })
    .toOrdered()
    .cout();
```

### 5. Statistiques

```cpp
auto stats = from(prices).toStatistics<double>([](auto p){ return p; });

std::cout << "Moyenne   : " << stats.mean()      << "\n";
std::cout << "Médiane   : " << stats.median()    << "\n";
std::cout << "Écart-type: " << stats.standardDeviation() << "\n";
std::cout << "Asymétrie : " << stats.skewness()  << "\n";
std::cout << "Kurtosis  : " << stats.kurtosis()  << "\n";
```

Toutes les fonctions statistiques sont fortement mises en cache (la carte de fréquences n’est calculée qu’une fois).

### 6. Parallélisme

```cpp
globalThreadPool   // créé automatiquement avec le nombre de cœurs matériels
stream.parallel()  // utilise le pool global
stream.parallel(12) // force exactement 12 threads workers
```

Le niveau de concurrence est correctement hérité tout au long de la chaîne.

## Fonctions de fabrique (via Generative)

```cpp
empty<T>()                              // stream vide
of(1,2,3,"hello")                       // arguments variadiques
fill(42, 1'000'000)                     // valeur répétée
fill([]{return rand();}, 1'000'000)     // valeurs fournies
from(container)                         // vector, list, set, array, initializer_list, queue
range(0, 100)                           // 0 .. 99
range(0, 100, 5)                        // 0,5,10,…
iterate(generator)                      // générateur personnalisé
```

## Installation

semantic-cpp se compose d’un fichier d’en-tête et d’un fichier d’implémentation. Copiez simplement `semantic.h` et `semantic.cpp` dans votre projet ou intégrez-les via CMake :

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## Compilation des exemples

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## Benchmarks (Apple M2 Max, 2024)

| Opération                         | Java Stream | ranges-v3 | semantic-cpp (parallèle) |
|-----------------------------------|-------------|-----------|---------------------------|
| Somme de 100 M entiers            | 280 ms      | 190 ms    | 72 ms                     |
| Moyenne glissante sur 10 M doubles| N/A         | N/A       | 94 ms (fenêtre 30, pas 10) |
| toStatistics sur 50 M entiers     | N/A         | N/A       | 165 ms                    |

## Contribuer

Les contributions sont les bienvenues ! Domaines nécessitant une attention particulière :

- Collecteurs supplémentaires (percentiles, covariance, etc.)
- Meilleure intégration avec les bibliothèques de ranges existantes
- Accélération SIMD optionnelle pour les mappers simples

Veuillez lire CONTRIBUTING.md.

## Licence

MIT © Eloy Kim

Profitez de streams véritablement sémantiques en C++ !
