# semantic-cpp – Une bibliothèque moderne de streams C++ avec sémantique temporelle

**semantic-cpp** est une bibliothèque de traitement de flux haute performance, *header-only*, écrite en C++17. Elle combine la fluidité de Java Streams, la paresse des générateurs JavaScript, le mapping ordonné des index MySQL et la conscience temporelle indispensable aux systèmes financiers, IoT et event-driven.

Ce qui la rend unique :

- Chaque élément porte un **Timestamp** (`long long` signé, peut être négatif).  
- **Module** est un entier non signé servant aux comptages et aux niveaux de concurrence.  
- Les streams sont totalement *lazy* jusqu’à ce qu’on les matérialise avec `.toOrdered()`, `.toUnordered()`, `.toWindow()` ou `.toStatistics()`.  
- Après matérialisation, on peut continuer à chaîner indéfiniment – la bibliothèque est volontairement « post-terminal »-friendly.  
- Support natif de l’exécution parallèle, des fenêtres glissantes et roulantes (sliding/tumbling windows) ainsi que de collecteurs statistiques très riches.

## Pourquoi choisir semantic-cpp ?

| Fonctionnalité                               | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                          |
|----------------------------------------------|-------------|-------------|-----------|--------------|--------------------------------------------------------|
| Évaluation paresseuse                        | Oui         | Oui         | Oui       | Non          | Oui                                                    |
| Indices temporels (timestamps signés)        | Non         | Non         | Non       | Non          | Oui (concept central)                                  |
| Fenêtres glissantes / roulantes              | Non         | Non         | Non       | Non          | Oui (première classe)                                  |
| Collecteur statistique intégré               | Non         | Non         | Non       | Non          | Oui (moyenne, médiane, mode, kurtosis, asymétrie…)     |
| Parallélisme par défaut (opt-in)             | Oui         | Non         | Oui       | Non          | Oui (pool global ou personnalisé)                      |
| Continuation après opération terminale       | Non         | Non         | Non       | Non          | Oui (streams post-terminal)                            |
| Header-only, C++17                           | Non         | Oui         | Oui       | Oui          | Oui                                                    |

Si vous avez déjà réécrit mille fois le même code de fenêtrage et de statistiques pour des données de marché, des capteurs ou des logs, **semantic-cpp** supprime tout ce code répétitif.

## Démarrage rapide

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // Stream de 100 nombres avec timestamps 0..99
    auto stream = from(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream
        .filter([](int x) { return x % 2 == 0; })     // nombres pairs uniquement
        .parallel(8)                                  // 8 threads
        .toWindow()                                   // matérialisation + fonctions de fenêtre
        .getSlidingWindows(10, 5)                     // fenêtres de 10, pas de 5
        .toVector();                                  // → std::vector<std::vector<int>>

    // Exemple statistique
    auto stats = from({1,2,3,4,5,6,7,8,9,10})
                  .toStatistics<double>();        // mapper identité déduit automatiquement

    std::cout << "Moyenne       : " << stats.mean()            << '\n';
    std::cout << "Médiane       : " << stats.median()          << '\n';
    std::cout << "Mode          : " << stats.mode()            << '\n';
    std::cout << "Écart-type    : " << stats.standardDeviation() << '\n';
    stats.cout();  // affichage complet en une ligne

    return 0;
}
```

## Concepts clés

### 1. `Semantic<E>` – le stream paresseux

```cpp
Semantic<int> s = of(1, 2, 3, 4, 5);
```

Toutes les opérations classiques sont disponibles :

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

Chaque élément est émis avec un **Timestamp** (indice signé). On peut décaler ou rediriger totalement les timestamps :

```cpp
s.translate(+1000)                                          // décalage global
   .redirect([](int x, Timestamp t){ return x * 10LL; })    // logique personnalisée
```

### 2. Matérialisation – le seul endroit où l’on « paye »

Il faut appeler l’un des quatre convertisseurs terminaux avant d’utiliser `count()`, `toVector()`, `cout()`, etc. :

```cpp
.toOrdered()        // conserve l’ordre d’origine, autorise le tri
.toUnordered()      // le plus rapide, aucune garantie d’ordre
.toWindow()         // ordonné + puissante API de fenêtres
.toStatistics<D>()  // ordonné + méthodes statistiques
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

### 3. Fenêtrage

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // fenêtres de 30 éléments, pas de 10
```

Ou générer directement un stream de fenêtres :

```cpp
stream.toWindow()
      .windowStream(50, 20)           // émet un std::vector<E> par fenêtre
      .map([](const std::vector<double>& w){ return moyenne(w); })
      .toOrdered()
      .cout();
```

### 4. Statistiques

```cpp
auto stats = from(prix)
              .toStatistics<double>([](double p){ return p; });

std::cout << "Moyenne        : " << stats.mean()      << "\n";
std::cout << "Médiane        : " << stats.median()    << "\n";
std::cout << "Écart-type     : " << stats.standardDeviation() << "\n";
std::cout << "Asymétrie      : " << stats.skewness()  << "\n";
std::cout << "Kurtosis       : " << stats.kurtosis()  << "\n";
```

Toutes les fonctions statistiques sont fortement mises en cache (la table de fréquences n’est calculée qu’une seule fois).

### 5. Parallélisme

```cpp
globalThreadPool          // créé automatiquement avec hardware_concurrency threads
stream.parallel()         // utilise le pool global
stream.parallel(12)       // force exactement 12 threads de travail
```

Chaque `Collectable` transporte son propre niveau de concurrence, correctement hérité dans toute la chaîne.

## Fonctions de fabrique

```cpp
empty<T>()                              // stream vide
of(1,2,3,"bonjour")                     // à partir d’arguments variadiques
fill(42, 1'000'000)                     // un million de 42
fill([]{return rand();}, 1'000'000)     // valeurs générées
from(conteneur)                         // vector, list, set, array, initializer_list
range(0, 100)                           // 0 .. 99
range(0, 100, 5)                        // 0,5,10,…
iterate(generateur_personnalise)        // depuis un Generator personnalisé
```

## Installation

Header-only. Copiez simplement `semantic.h` dans votre projet ou intégrez-le via CMake :

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(votre_cible PRIVATE semantic::semantic)
```

## Compiler les exemples

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp -o basic
./basic
```

## Benchmarks (Apple M2 Max, 2024)

| Opération                                      | Java Stream | ranges-v3 | semantic-cpp (parallèle) |
|------------------------------------------------|-------------|-----------|---------------------------|
| Somme de 100 M entiers                         | 280 ms      | 190 ms    | 72 ms                     |
| Moyenne glissante sur 10 M doubles (fenêtre 30, pas 10) | N/A | N/A       | 94 ms                     |
| 50 M entiers → toStatistics                    | N/A         | N/A       | 165 ms                    |

## Contribuer

Les contributions sont les bienvenues ! Domaines particulièrement recherchés :

- Collecteurs supplémentaires (percentiles, covariance, etc.)
- Meilleure interopérabilité avec les autres bibliothèques de ranges
- Accélération SIMD optionnelle pour les mappers simples

Merci de lire CONTRIBUTING.md.

## Licence

MIT © Eloy Kim

Profitez de véritables streams à sémantique temporelle en C++ !
