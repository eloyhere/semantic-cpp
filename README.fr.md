# semantic-cpp – Bibliothèque moderne de streams sémantiques en C++

**semantic-cpp** est une bibliothèque header-only, C++17+, à évaluation paresseuse et hautes performances.  
Son principe fondamental : **chaque élément porte dès sa création un index logique (Timestamp)** – toutes les opérations qui touchent à l’ordre ne sont que des transformations de cet index.

- Vrais streams infinis  
- L’index est un citoyen de première classe et peut être remappé librement  
- Parallélisme activé par défaut  
- Seule la phase de collecte décide si l’on respecte ou ignore toutes les opérations d’index  

Licence MIT – utilisation totalement libre dans les projets commerciaux et open-source.

## Philosophie de conception

> **L’index détermine l’ordre – l’exécution ne détermine que la vitesse.**

- `redirect`, `reverse`, `shuffle`, `cycle`… ne modifient que l’index logique  
- Seul `.toOrdered()` respecte ces transformations → le résultat est trié exactement selon l’index final  
- `.toUnordered()` ignore toutes les opérations d’index → vitesse maximale, pas d’ordre

```text
.toOrdered()   → toutes les opérations d’index sont effectives
.toUnordered() → toutes les opérations d’index sont ignorées (chemin le plus rapide)
```

## Démarrage rapide

```cpp
#include "semantic.h"
using namespace semantic;

int main() {
    // 1. Les opérations d’index ont un effet
    auto v1 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })   // inversion
        .toOrdered()                                  // obligatoire !
        .toVector();   // résultat : [99, 98, …, 0]

    // 2. Les opérations d’index sont ignorées (vitesse maximale)
    auto v2 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })
        .toUnordered()                                // index ignoré
        .toVector();   // résultat : désordonné, mais ultra-rapide

    // 3. Stream cyclique infini + collecte ordonnée
    auto top10 = Semantic<int>::range(0, 1'000'000'000)
        .redirect([](auto&, auto i) { return i % 1000; }) // cycle 0-999
        .toOrdered()
        .limit(10)
        .toVector();   // [0,1,2,3,4,5,6,7,8,9]

    return 0;
}
```

## Tous les appels chaînés importants

```cpp
// Création de streams
Semantic<int>::range(0, 1000);
Semantic<long long>::range(0, 1LL<<60);
Semantic<int>::iterate([](auto yield, auto) {      // stream infini
    for (int i = 0;; ++i) yield(i);
});
Semantic<int>::of(1, 2, 3, 4, 5);
Semantic<int>::from(std::vector{1,2,3,4});
Semantic<int>::fill(42, 1'000'000);
Semantic<int>::fill([] { return rand(); }, 1000);

// Opérations intermédiaires (100 % paresseuses)
stream.map([](int x) { return x * x; });
stream.filter([](int x) { return x % 2 == 0; });
stream.flatMap([](int x) { return Semantic<int>::range(0, x); });
stream.distinct();                                     // suppression des doublons par valeur
stream.skip(100);
stream.limit(50);
stream.takeWhile([](int x) { return x < 1000; });
stream.dropWhile([](int x) { return x < 100; });
stream.peek([](int x) { std::cout << x << ' '; });

// Transformations d’index (prennent effet uniquement avec toOrdered())
stream.redirect([](auto&, auto i) { return -i; });           // inverser
stream.redirect([](auto&, auto i) { return i % 100; });      // cyclique
stream.redirect([](auto e, auto i) { return std::hash<int>{}(e); }); // mélanger
stream.reverse();                                            // équivalent à redirect(-i)
stream.shuffle();

// Parallélisme
stream.parallel();       // tous les cœurs
stream.parallel(8);      // nombre fixe de threads

// Choix obligatoire : respecter ou ignorer l’index
auto ordered   = stream.toOrdered();     // toutes les opérations d’index ont un effet
auto unordered = stream.toUnordered();   // toutes les opérations d’index sont ignorées (plus rapide)

// Opérations terminales ordonnées (l’index final détermine l’ordre)
ordered.toVector();
ordered.toList();
ordered.toSet();                 // déduplication selon l’index final
ordered.forEach([](int x) { std::cout << x << ' '; });
ordered.cout();                  // [99, 98, 97, …]

// Opérations terminales non ordonnées (vitesse maximale)
unordered.toVector();
unordered.toList();
unordered.toSet();
unordered.forEach(...);
unordered.cout();

// Statistiques (toujours sur le chemin rapide)
auto stats = stream.toUnordered().toStatistics();

// Reduce (recommandé avec unordered)
int somme = stream.toUnordered()
    .reduce(0, [](int a, int b) { return a + b; });
```

## Tableau récapitulatif des méthodes

| Méthode                         | Description                                       | Respecte les opérations d’index ? |
|---------------------------------|---------------------------------------------------|-----------------------------------|
| `toOrdered()`                   | Mode sémantique – toutes les opérations d’index sont effectives | Oui                               |
| `toUnordered()`                 | Mode performance – toutes les opérations d’index sont ignorées | Non (chemin le plus rapide)       |
| `toVector()` / `toList()`       | Collecte dans un conteneur                       | dépend                            |
| `toSet()`                       | Collecte dans un set (sans doublons)             | dépend                            |
| `forEach` / `cout`              | Parcours ou affichage                            | dépend                            |
| `redirect` / `reverse` / `shuffle` | Transformation de l’index logique          | uniquement avec toOrdered()       |
| `parallel`                      | Exécution parallèle                              | dans les deux modes               |

## Pourquoi faut-il choisir explicitement toOrdered / toUnordered ?

Parce que **transformation d’index** et **exécution parallèle** sont deux dimensions totalement orthogonales :

- `redirect` etc. sont des opérations sémantiques (où l’élément doit-il apparaître logiquement ?)  
- `parallel` n’est qu’une stratégie d’exécution (à quelle vitesse le calculer ?)

Seule la décision explicite lors de la collecte permet d’obtenir à la fois la vitesse maximale et le contrôle précis de l’ordre.

## Prérequis de compilation

- C++17 ou supérieur  
- Uniquement `#include "semantic.h"`  
- Zéro dépendances externes  
- Bibliothèque mono-fichier d’en-tête

```bash
g++ -std=c++17 -O3 -pthread semantic.cpp
```

## Licence

Licence MIT – peut être utilisée sans restriction dans les projets commerciaux, open-source et privés.

---

**semantic-cpp** :  
**L’index décide de l’ordre – l’exécution ne décide que de la vitesse.**  
Écris `redirect` et cela inverse vraiment. Écris `toUnordered()` et c’est vraiment le plus rapide.  
Aucun compromis – seulement des choix clairs.

Écris des streams sans jamais deviner.  
Dis simplement : **« Je veux la sémantique – ou je veux la vitesse pure. »**
