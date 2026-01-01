# semantic-cpp – Une bibliothèque de traitement de flux moderne en C++ avec sémantique temporelle

**semantic-cpp** est une bibliothèque légère et performante de traitement de flux pour C++17 moderne. Elle se compose d’un seul fichier d’en-tête (`semantic.h`) accompagné d’un fichier d’implémentation séparé (`semantic.cpp`). Cette bibliothèque combine la fluidité des Java Streams, l’évaluation paresseuse des générateurs et Promises JavaScript, les capacités d’ordonnancement rappelant les index de bases de données, et une conscience temporelle intégrée essentielle pour les applications financières, le traitement de données IoT et les systèmes événementiels.

## Principales caractéristiques

- Chaque élément est associé à un **Timestamp** (un `long long` signé supportant les valeurs négatives) et à un **Module** (un `unsigned long long` utilisé pour les comptages et la concurrence).
- Les flux sont évalués paresseusement jusqu’à leur matérialisation via `.toOrdered()`, `.toUnordered()`, `.toWindow()` ou `.toStatistics()`.
- La matérialisation **ne** termine **pas** le pipeline — les appels en chaîne restent pleinement possibles ensuite (« flux post-terminal »).
- Support complet de l’exécution parallèle, des fenêtres glissantes et tumbling, des opérations statistiques avancées, et des tâches asynchrones via une classe **Promise** inspirée de JavaScript.

## Pourquoi choisir semantic-cpp ?

| Fonctionnalité                   | Java Stream | ranges-v3 | semantic-cpp                          |
|----------------------------------|-------------|-----------|---------------------------------------|
| Évaluation paresseuse            | Oui         | Oui       | Oui                                   |
| Index temporels (timestamps signés) | Non         | Non       | Oui (concept central)                 |
| Fenêtres glissantes / tumbling   | Non         | Non       | Oui (support natif)                   |
| Opérations statistiques intégrées| Non         | Non       | Oui (moyenne, médiane, mode, asymétrie, aplatissement, etc.) |
| Exécution parallèle (opt-in)     | Oui         | Oui       | Oui (pool de threads + support Promise)|
| Chaînage après opération terminale | Non         | Non       | Oui (flux post-terminal)              |
| Promises asynchrones             | Non         | Non       | Oui (style JavaScript)                |
| En-tête unique + fichier d’implémentation, C++17 | Non         | Oui       | Oui                                   |

Si vous écrivez souvent du code personnalisé pour les fenêtres, les statistiques ou les tâches asynchrones sur des données temporelles, des flux de marché, des capteurs ou des logs, semantic-cpp élimine ce code répétitif.

## Démarrage rapide

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // Créer un flux à partir de valeurs (timestamps auto-incrémentés à partir de 0)
    auto stream = of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    // Filtrer les nombres pairs, exécution parallèle, matérialisation pour statistiques
    auto stats = stream
        .filter([](int x) { return x % 2 == 0; })
        .parallel(8)
        .toStatistics<int>();

    std::cout << "Moyenne : " << stats.mean() << '\n';      // 5
    std::cout << "Médiane : " << stats.median() << '\n';    // 5
    std::cout << "Mode : " << stats.mode() << '\n';         // n’importe quel pair

    // Exemple de fenêtrage
    auto windows = of(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
        .toWindow()
        .getSlidingWindows(5, 2);  // fenêtres de taille 5, pas de 2

    return 0;
}
```

## Concepts clés

### 1. Fonctions fabriques – Création de flux

Les flux sont créés via des fonctions libres dans l’espace de noms `semantic` :

```cpp
auto s = of(1, 2, 3, 4, 5);                          // arguments variadiques
auto empty = empty<int>();                          // flux vide
auto filled = fill(42, 1'000'000);                   // valeur répétée
auto supplied = fill([]{ return rand(); }, 1'000'000);
auto ranged = range(0, 100);                        // 0 .. 99
auto ranged_step = range(0, 100, 5);                // 0, 5, 10, ...
auto from_vec = from(std::vector<int>{1, 2, 3});
auto iterated = iterate([](auto push, auto stop) {
    for (int i = 0; i < 100; ++i) push(i, i);       // timestamps explicites
});
```

Fonctions d’aide E/S supplémentaires : `lines(stream)`, `chunks(stream, size)`, `text(stream)`, etc.

### 2. Semantic – Le flux paresseux

Le type central `Semantic<E>` supporte les opérations classiques :

```cpp
stream.filter(...).map(...).skip(10).limit(100).peek(...)
```

Manipulation des timestamps :

```cpp
stream.translate(+1000)          // décaler tous les timestamps
    .redirect([](auto elem, Timestamp t) { return t * 10; });
```

Parallélisme :

```cpp
stream.parallel()                // nombre de threads par défaut
    .parallel(12);               // spécifier le nombre de workers
```

### 3. Matérialisation

Convertir en forme collectable :

- `.toOrdered()` – conserve l’ordre, active le tri
- `.toUnordered()` – le plus rapide, sans ordre
- `.toWindow()` – ordonné avec support complet des fenêtres
- `.toStatistics<R>(mapper)` – ordonné avec méthodes statistiques

Le chaînage reste possible ensuite :

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
auto windows = stream.toWindow().getSlidingWindows(30, 10);
auto tumbling = stream.toWindow().getTumblingWindows(50);
```

Émettre des flux de fenêtres :

```cpp
stream.toWindow()
    .windowStream(50, 20)
    .map([](const std::vector<double>& w) { return mean(w); })
    .cout();
```

### 5. Statistiques

```cpp
auto stats = from(prices)
    .toStatistics<double>([](auto p) { return p; });

std::cout << "Moyenne :     " << stats.mean() << '\n';
std::cout << "Médiane :     " << stats.median() << '\n';
std::cout << "Écart-type :  " << stats.standardDeviation() << '\n';
std::cout << "Asymétrie :   " << stats.skewness() << '\n';
std::cout << "Aplatissement :" << stats.kurtosis() << '\n';
```

Les résultats sont mis en cache de façon agressive pour les performances.

### 6. Exécution asynchrone avec Promises

Inspirée des Promises JavaScript, la classe `Promise<T, E>` permet de gérer les tâches asynchrones soumises à un `ThreadPool` :

```cpp
ThreadPool pool(8);

auto promise = pool.submit<int>([] {
    // calcul lourd
    return 42;
});

promise.then([](int result) {
        std::cout << "Résultat : " << result << '\n';
    })
    .except([](const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << '\n';
    })
    .finally([] {
        std::cout << "Terminé\n";
    });

promise.wait();  // bloquer si nécessaire
```

Aides statiques : `Promise<T>::all(...)`, `Promise<T>::any(...)`, `resolved(value)`, `rejected(error)`.

## Installation

Copiez `semantic.h` et `semantic.cpp` dans votre projet ou utilisez CMake :

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## Compilation des exemples

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## Benchmarks (Apple M2 Max, 2024)

| Opération                          | Java Stream | ranges-v3 | semantic-cpp (parallèle) |
|------------------------------------|-------------|-----------|--------------------------|
| 100 M entiers → somme              | 280 ms      | 190 ms    | **72 ms**                |
| 10 M doubles → moyenne fenêtre glissante | N/A         | N/A       | **94 ms** (fenêtre 30, pas 10) |
| 50 M entiers → toStatistics        | N/A         | N/A       | **165 ms**               |

## Contributions

Les contributions sont les bienvenues ! Domaines à améliorer :

- Collecteurs supplémentaires (percentiles, covariance, etc.)
- Meilleure intégration avec les bibliothèques de ranges
- Accélération SIMD optionnelle

## Licence

MIT © Eloy Kim

Profitez de véritables flux sémantiques en C++ !