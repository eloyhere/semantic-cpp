# 🚀 Semantic-Cpp : Un cadre de traitement de flux intelligent orienté futur pour C++

Semantic-Cpp est une bibliothèque moderne de traitement de flux en C++, entièrement repensée depuis la base, avec une architecture modulaire **"plusieurs en-têtes, zéro dépendance externe"**. Chaque fichier d'en-tête a une responsabilité claire et unique, et est testable indépendamment ; ensemble, ils forment un écosystème complet de traitement de flux. Cette bibliothèque combine de manière innovante l'essence de multiples paradigmes de programmation :

*   **L'élégance et la fluidité de l'API Java Stream** : Appels chaînés, programmation déclarative, rendant le code aussi élégant que de la poésie ✨
*   **La paresse et la flexibilité des générateurs JavaScript** : Évaluation paresseuse, génération à la demande, respectueuse de la mémoire 🌱
*   **L'efficacité et l'ordre de l'indexation de bases de données** : Tri intelligent, piloté par index, un outil puissant pour le traitement de données chronologiques ⏱️
*   **La philosophie de traitement par lots de "Conteneur-en-tant qu'Élément"** : Vecteurs, listes, maps... Tout conteneur peut être un citoyen de première classe dans le flux, coulant librement 📦

Êtes-vous fatigué d'écrire des boucles `for` pour parcourir un `vector`, d'imbriquer un `if` pour filtrer, et d'appeler manuellement `push_back` sur un autre conteneur ? 😩
Avez-vous déjà débogué une erreur de décalage d'index tard dans la nuit, juste parce que vous vouliez le "troisième élément depuis la fin" lors d'une itération vers l'arrière ? 😵💫
Aspirez-vous à manipuler des données comme une base de données - localisation précise par index, analyse avec des fenêtres glissantes, compléter tout le parcours des données aux statistiques avec une seule chaîne d'appels ? 🤔

**Semantic-Cpp est né pour cela. 🔧**

Il abstrait le traitement de données comme des opérations sur des "éléments" et leurs "positions logiques (index)" - similaire aux "lignes" et "clés primaires" dans une base de données. Vous pouvez réorganiser, décaler et inverser librement les index sans toucher aux données elles-mêmes ; vous pouvez également passer n'importe quel conteneur (`vector`, `map`, `array`...) comme un tout indivisible dans le flux, et le "déballer" à nouveau au niveau élément à tout moment. Cette capacité de changer librement entre deux granularités manque dans les frameworks de flux traditionnels. 🎯

---

## 🏗️ Architecture du projet : Conception modulaire à sept couches

Semantic-Cpp se compose de sept fichiers d'en-tête principaux, construits en couches progressives. Chaque fichier a une responsabilité unique et est testé indépendamment. Cinq espaces de noms, chacun avec son propre domaine, travaillent ensemble pour former un pipeline complet depuis la source de données jusqu'au résultat final :

```
┌─────────────────────────────────────────────────┐
│               🌊 semantics.h                     │
│   Espace de noms : semantic                     │
│   Usines de construction de flux : plages       │
│   numériques, conteneurs, texte, Unicode        │
├─────────────────────────────────────────────────┤
│                 📦 semantic.h                   │
│   Espace de noms : semantic / collectable       │
│   Opérations intermédiaires de flux, système    │
│   Collectable, support de déballage de          │
│   conteneurs                                    │
├─────────────────────────────────────────────────┤
│                ⚙️ collector.h                   │
│   Espace de noms : collector                    │
│   Cadre de collecteurs + usines : correspondance,│
│   recherche, agrégation, statistiques, DFT/FFT │
├─────────────────────────────────────────────────┤
│                🔤 charsequence.h                │
│   Espace de noms : charsequence                 │
│   Séquences de caractères Unicode, conversion   │
│   multi-encodage, Builder, Buffer              │
├─────────────────────────────────────────────────┤
│                🧵 pool.h                        │
│   Espace de noms : pool                         │
│   Pool de threads global : soumission de tâches,│
│   arrêt d'urgence, propagation d'exceptions    │
├─────────────────────────────────────────────────┤
│                📄 function.h                    │
│   Espace de noms : function                     │
│   Définitions de types : alias pour Generator,  │
│   Supplier, Consumer, etc.                     │
├─────────────────────────────────────────────────┤
│               🔐 hash.h / less.h                │
│   Espace de noms : std (extensions)            │
│   Spécialisations de hachage et de comparaison │
│   de conteneurs de bibliothèque standard,      │
│   support d'imbrication arbitraire             │
└─────────────────────────────────────────────────┘
```

### 🧩 Graphe de dépendances
La chaîne de dépendances est claire et logique, comme un schéma de circuit conçu méticuleusement : le courant circule des définitions de types fondamentaux vers le haut, chaque couche ne dépendant que des couches inférieures. Finalement, tous les chemins convergent vers `semantic.h` et `semantics.h`, formant la capacité complète de traitement de flux.

```
function.h          ← Aucune dépendance, la base de types
pool.h              ← Dépend de function.h
charsequence.h      ← Module indépendant, traitement Unicode
collector.h         ← Dépend de function.h, pool.h
hash.h / less.h     ← Modules indépendants, extensions de bibliothèque standard
semantic.h          ← Dépend de tous les précédents
semantics.h         ← Dépend de semantic.h
```

---

## 🌍 Aperçu des espaces de noms

Semantic-Cpp conçoit méticuleusement cinq espaces de noms, chacun comme un "département" indépendant, avec des responsabilités distinctes mais collaborant étroitement :

| Espace de noms  | Fichier d'en-tête | Responsabilité                             | Types/Fonctions principaux                                                  |
| :-------------- | :---------------- | :---------------------------------------- | :------------------------------------------------------------------------- |
| function        | function.h        | Fondation du système de types             | `Timestamp`, `Module`, `Generator<T>`, `Supplier<R>`, `Consumer<T>`, `Predicate<T>` etc. |
| pool            | pool.h            | Moteur d'exécution concurrente            | `pool::pool` (pool de threads global), `submit()`, `emergencyShutdown()`  |
| charsequence    | charsequence.h    | Traitement de chaînes Unicode             | `charset`, `Meta`, `Point`, `Charsequence`, `Builder`, `Buffer` etc.      |
| collector       | collector.h       | Exécution de collecte terminale           | `Collector<E,A,R>`, `Identity<A>`, `Accumulator<A,E>` etc.                |
| collectable     | semantic.h        | Conteneurs de données matérialisés        | `Collectable<E>`, `OrderedCollectable<E>`, `UnorderedCollectable<E>` etc. |
| semantic        | semantic.h<br>semantics.h | Construction de flux et opérations intermédiaires | `Semantic<E>`, `useRange()`, `useFrom()` etc.                             |

### 🔁 Flux de collaboration entre espaces de noms
Le flux de données entre les espaces de noms est comme une chaîne de montage dans une usine - la matière première entre depuis `semantic`, subit un traitement couche par couche, et est finalement emballée et expédiée depuis `collector`. Chaque étape a une frontière de responsabilité claire :

```cpp
semantic::useRange(0, 100)          // ← espace de noms semantic : créer un flux
    .map(int x { return x * 2; })   // ← espace de noms semantic : transformation intermédiaire
    .filter(int x { return x > 50; }) // ← espace de noms semantic : filtre intermédiaire
    .toUnordered()                  // ← Convertir en espace de noms collectable
    .toVector();                    // ← Invoquer un collecteur de l'espace de noms collector
```

---

## 📦 Couche 1 : function.h — Fondation de types

`function.h` définit le système de types pour l'ensemble du cadre, la fondation commune pour tous les modules. 🔑

```cpp
namespace function {
    using Timestamp = long long;           // Type d'index, le "timestamp" des données dans le flux
    using Module = unsigned long long;     // Type de module/comptage
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept — recevoir un élément
        std::function<bool(T, Timestamp)>       // interrupt — devrions-nous arrêter ?
    )>;
}
```

`Generator` est l'abstraction centrale de tout le système de flux. 🌀 Il ne renvoie pas de données ; à la place, il accepte deux callbacks - `accept` ("Je suis prêt, veuillez accepter cet élément") et `interrupt` ("devrions-nous arrêter ?"). Cette conception d'inversion de contrôle signifie que le producteur de données n'a aucune connaissance du consommateur ; il "pousse" simplement des données au moment approprié. C'est l'essence de l'évaluation paresseuse : les données ne "coulent" vraiment que lorsque `accept` est appelé ; avant cela, tout n'est qu'une description.

| Alias de type    | Définition complète                            | Objectif                               |
| :--------------- | :--------------------------------------------- | :------------------------------------- |
| Timestamp        | long long                                      | Position logique d'un élément dans le flux |
| Module           | unsigned long long                             | Comptage, capacité, niveau de concurrence |
| Runnable         | std::function<void()>                          | Tâche sans paramètre renvoyant void    |
| Supplier<R>      | std::function<R()>                             | Fournisseur, crée à partir de rien     |
| Function<T,R>    | std::function<R(T)>                            | Fonction à un argument                 |
| BiFunction<T,U,R>| std::function<R(T,U)>                          | Fonction à deux arguments              |
| TriFunction<T,U,V,R> | std::function<R(T,U,V)>                     | Fonction à trois arguments             |
| Unary<T>         | std::function<T(T)>                            | Opération unaire                       |
| Binary<T>        | std::function<T(T,T)>                          | Opération binaire                      |
| Consumer<T>      | std::function<void(T)>                         | Consommateur                           |
| BiConsumer<T,U>  | std::function<void(T,U)>                       | Consommateur à deux arguments          |
| TriConsumer<T,U,V>| std::function<void(T,U,V)>                    | Consommateur à trois arguments         |
| Predicate<T>     | std::function<bool(T)>                         | Jugement de prédicat                   |
| BiPredicate<T,U> | std::function<bool(T,U)>                       | Prédicat à deux arguments              |
| TriPredicate<T,U,V>| std::function<bool(T,U,V)>                   | Prédicat à trois arguments             |
| Comparator<T>    | std::function<int(const T&,const T&)>          | Comparateur, retourne négatif/zéro/positif |
| Generator<T>     | BiConsumer<BiConsumer<T,Timestamp>,<br>BiPredicate<T,Timestamp>> | Abstraction centrale pour la génération de flux |

---

## 🧵 Couche 2 : pool.h — Fondation de concurrence

`pool.h` fournit le pool de threads global `pool::pool`, le moteur de concurrence pour tout le cadre. 🚀 Il utilise une conception de **parallélisme déclaratif** - lorsque vous écrivez `.parallel(4)`, il ne lance pas immédiatement quatre threads pour commencer le traitement. Cette ligne de code n'est qu'une "déclaration" : elle indique au cadre que "j'ai l'intention d'utiliser 4 threads pour le traitement parallèle". L'exécution parallèle réelle se produit lorsqu'une opération terminale est invoquée ; c'est-à-dire lorsque vous appelez des méthodes de collecte comme `toVector()`, `findFirst()`, `count()`, etc.

| Caractéristique    | Description                                                               |
| :----------------- | :------------------------------------------------------------------------ |
| Parallélisme déclaratif | `.parallel(4)` déclare seulement "je veux utiliser 4 threads", ne démarre pas immédiatement |
| Arrêt d'urgence    | Gestionnaire intégré `emergencyShutdown()` et `std::set_terminate`       |
| Propagation d'exceptions | `submit()` renvoie `std::future`, propageant les exceptions de manière sûre au thread principal |

---

## 🔤 Couche 3 : charsequence.h — Séquence de caractères Unicode

`charsequence.h` est un module complet de traitement Unicode, fournissant des fonctionnalités pour créer, convertir et manipuler des séquences de caractères. 🌍 Il prend en charge plusieurs encodages comme UTF-8, UTF-16 (LE/BE), UTF-32 (LE/BE), ASCII et Latin1. Il détecte et gère correctement les paires de substitution, renvoyant le caractère de remplacement standard U+FFFD pour les points de code non valides.

| Type/Fonction    | Description                                                               |
| :--------------- | :------------------------------------------------------------------------ |
| charset          | Enum : ascii, utf8, utf16, utf16be, utf16le, utf32, utf32be, utf32le, latin1 |
| Meta             | Enveloppe de métadonnées, stocke des valeurs entières non signées         |
| Point            | Point de code Unicode, prend en charge la détection de paires de substitution et les vérifications de validité |
| Charsequence     | Séquence de caractères immuable : split, replace, indexOf, lastIndexOf, sub, trim, toUpperCase, toLowerCase, reverse, startsWith, endsWith, contains, compare, getBytes, getPoints, getMetas, getCharacters, repeat, concat, count, join |
| Builder          | Constructeur d'octets mutable : prepend, insert, append (prend en charge les types primitifs, Point, Charsequence, string_view) |
| Buffer           | Tampon circulaire thread-safe : write, read, peek, prepend, append, clear, shrinkToFit, data, size, capacity, atomic |
| PointIterator    | Itérateur bidirectionnel pour parcourir les points de code Unicode         |
| encode()         | Encode un seul point de code en une séquence d'octets de l'encodage spécifié |
| decode()         | Décode le point de code suivant à partir d'une séquence d'octets, avance automatiquement le pointeur |
| convert()        | Conversion d'encodage (prend en charge la sortie string, vector, deque)   |

---

## ⚙️ Couche 4 : collector.h — Cadre de collecteurs et usines

`collector.h` est le module de collecteur central de Semantic-Cpp, combinant le cadre de collecteurs avec des fonctions d'usine.

### 🧩 Modèle à cinq étapes
```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (court-circuit optionnel)
```

| Alias de type    | Définition complète                      | Rôle                             |
| :--------------- | :--------------------------------------- | :------------------------------- |
| Identity<A>      | function::Supplier<A>                    | Fournit une valeur initiale      |
| Accumulator<A,E> | function::TriFunction<A, E, Timestamp, A>| Accumule des éléments            |
| Combiner<A>      | function::BiFunction<A, A, A>            | Combine des résultats parallèles |
| Finisher<A,R>    | function::Function<A, R>                 | Transformation finale            |
| Interrupt<E,A>   | function::TriPredicate<E, Timestamp, A>  | Jugement de court-circuit        |

### Fonctions d'usine de collecteurs

#### ✅ Opérations de correspondance
| Méthode                 | Description                | Type de retour |
| :---------------------- | :------------------------- | :------------- |
| `useAllMatch(predicate)`| Tous les éléments correspondent à la condition | `bool`     |
| `useAnyMatch(predicate)`| Un élément correspond à la condition | `bool`     |
| `useNoneMatch(predicate)`| Aucun élément ne correspond à la condition | `bool`     |

#### 🔍 Opérations de recherche
| Méthode            | Description                      | Type de retour     |
| :---------------- | :------------------------------- | :----------------- |
| `useFindFirst()`  | Trouve le premier élément        | `std::optional<E>` |
| `useFindLast()`   | Trouve le dernier élément        | `std::optional<E>` |
| `useFindAny()`    | Trouve n'importe quel élément (aléatoire) | `std::optional<E>` |
| `useFindAt(index)`| Localise à un index spécifique (prend en charge les négatifs) | `std::optional<E>` |
| `useFindMaximum()`| Trouve l'élément maximum         | `std::optional<E>` |
| `useFindMinimum()`| Trouve l'élément minimum         | `std::optional<E>` |

#### 🔢 Opérations d'agrégation
| Méthode           | Description  | Type de retour |
| :---------------- | :---------- | :------------- |
| `useCount()`      | Nombre total d'éléments | `Module` |
| `useSummate<E,D>()`| Somme      | `D`            |
| `useAverage<E,D>()`| Moyenne    | `D`            |
| `useRange<E,D>()` | Plage numérique (max - min) | `D` |

#### 📉 Opérations statistiques
| Méthode                      | Description                | Type de retour       |
| :-------------------------- | :------------------------- | :------------------ |
| `useVariance<E,D>()`        | Variance de la population  | `D`                 |
| `useStandardDeviation<E,D>()`| Écart-type de la population | `D`              |
| `useSkewness<E,D>()`        | Asymétrie                  | `D`                 |
| `useKurtosis<E,D>()`        | Kurtosis                   | `D`                 |
| `useMedian<E,D>()`          | Médiane                    | `std::optional<D>`  |
| `useMode<E>()`              | Mode (analyse de fréquence) | `std::optional<E>` |
| `usePercentile<E,D>(p)`     | p-ème centile             | `std::optional<D>`  |
| `useFrequency<E>()`         | Caractéristiques du domaine fréquentiel | `std::map<E, complex>` |
| `useDistribution<E>()`      | Caractéristiques de distribution spatiale | `std::map<E, complex>` |

#### 🔀 Opérations de réduction
| Méthode                        | Description          | Type de retour     |
| :---------------------------- | :------------------ | :----------------- |
| `useReduce(reducer)`          | Réduction sans identité | `std::optional<E>` |
| `useReduce(identity, reducer)`| Réduction avec identité | `E`              |
| `useReduce(id, red, comb, fin)`| Réduction complètement personnalisée | `R` |

#### 🧺 Collecte dans des conteneurs
| Méthode                                                            | Type de retour |
| :---------------------------------------------------------------- | :------------- |
| `useToVector()`                                                   | `std::vector<E>` |
| `useToList()`                                                     | `std::list<E>` |
| `useToDeque()`                                                    | `std::deque<E>` |
| `useToForwardList()`                                              | `std::forward_list<E>` |
| `useToArray<N>()`                                                 | `std::array<E, N>` |
| `useToSet()`                                                      | `std::set<E>` |
| `useToMultiset()`                                                 | `std::multiset<E>` |
| `useToUnorderedSet()`                                             | `std::unordered_set<E>` |
| `useToUnorderedMultiset()`                                        | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)`                                          | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)`                          | `std::map<K, V>` |
| `useToMultimap(keyExtractor)`                                     | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)`                     | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)`                 | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)`                            | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)`            | `std::unordered_multimap<K, V>` |
| `useToStack()`                                                    | `std::stack<E>` |
| `useToQueue()`                                                    | `std::queue<E>` |
| `useToPriorityQueue()`                                            | `std::priority_queue<E>` |

#### 🧩 Opérations de regroupement et partitionnement
| Méthode                                      | Type de retour |
| :------------------------------------------ | :------------- |
| `useGroup(keyExtractor)`                    | `std::unordered_map<K, vector<E>>` |
| `useGroupBy(keyExtractor, valueExtractor)` | `std::unordered_map<K, vector<V>>` |
| `usePartition(size)`                        | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)`              | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor, valueExtractor)` | `std::vector<vector<V>>` |

#### 📄 Sortie de chaîne
| Méthode                               | Type de retour |
| :----------------------------------- | :------------- |
| `useJoin()` / `useOut()` / `useError()` et leurs surcharges | `charsequence::Charsequence` |

#### 📊 Outils mathématiques
| Méthode          | Type de retour |
| :-------------- | :------------- |
| `useDFT()`      | `vector<complex<double>>` |
| `useIDFT()`     | `vector<complex<double>>` |
| `useFFT()`      | `vector<complex<double>>` |
| `useIFFT()`     | `vector<complex<double>>` |
| `useGradient()` | `vector<double>` |

---

## 📦 Couche 5 : semantic.h — Opérations intermédiaires de flux et système de collecte

### 🧩 Conception centrale : Pipeline à trois étapes
```
Semantic<E> (Construction et Transformation)
    ↓ toUnordered() / toOrdered() / sort() / toWindow() / toStatistics()
Collectable<E> (Matérialisation et Collecte)
    ↓ toVector() / findFirst() / count() / summate() / ...
Résultat final
```

**Règle importante** : Un `Semantic<E>` doit d'abord être converti en un `Collectable<E>` via `toUnordered()`, `toOrdered()`, `toWindow()`, `toStatistics()` ou `sort()` avant que les méthodes terminales puissent être appelées.

### 🧭 Cinq chemins de matérialisation
| Méthode de conversion | Type cible            | Structure de données sous-jacente | Caractéristique de performance |
| :------------------- | :------------------- | :------------------------------- | :---------------------------- |
| `toUnordered()`      | `UnorderedCollectable` | `unordered_map`                 | Recherche moyenne O(1)        |
| `toOrdered()`        | `OrderedCollectable`  | `map`                           | Recherche O(log n)            |
| `sort()`             | `OrderedCollectable`  | `map` (trié par valeur)         | Recherche O(log n)            |
| `toWindow()`         | `WindowCollectable`   | Hérite d'une collection ordonnée | Prend en charge slide/tumble  |
| `toStatistics<D>()`  | `Statistics<E,D>`     | Hérite d'une collection ordonnée | 30+ méthodes statistiques    |

### 📋 Collectable<E> — Toutes les méthodes terminales (ordre alphabétique)
| Méthode                                            | Type de retour             | Description                                     |
| :------------------------------------------------ | :------------------------- | :--------------------------------------------- |
| `allMatch(predicate)`                             | `bool`                     | Tous les éléments correspondent à la condition  |
| `anyMatch(predicate)`                             | `bool`                     | Un élément correspond à la condition           |
| `average<D>()`                                    | `D`                        | Moyenne                                        |
| `average<D>(mapper)`                              | `D`                        | Moyenne après mappage                          |
| `collect(identity, acc, comb, fin)`              | `R`                        | Collecte personnalisée en quatre étapes        |
| `collect(identity, interrupt, acc, comb, fin)`    | `R`                        | Collecte interrompable personnalisée           |
| `count()`                                         | `Module`                   | Nombre total d'éléments                        |
| `empty()`                                         | `bool`                     | Le flux est-il vide ?                          |
| `error()`                                         | `void`                     | Sortie vers stderr (prend en charge delimiter/prefix/suffix/converter) |
| `findAny()`                                       | `std::optional<E>`         | Trouve n'importe quel élément (aléatoire)      |
| `findAt(index)`                                   | `std::optional<E>`         | Trouve l'élément à l'index spécifié (prend en charge les négatifs) |
| `findFirst()`                                     | `std::optional<E>`         | Trouve le premier élément                      |
| `findLast()`                                      | `std::optional<E>`         | Trouve le dernier élément                      |
| `findMaximum()`                                   | `std::optional<E>`         | Trouve l'élément maximum                       |
| `findMaximum(comparator)`                         | `std::optional<E>`         | Trouve le maximum avec un comparateur personnalisé |
| `findMinimum()`                                   | `std::optional<E>`         | Trouve l'élément minimum                       |
| `findMinimum(comparator)`                         | `std::optional<E>`         | Trouve le minimum avec un comparateur personnalisé |
| `forEach(consumer)`                               | `void`                     | Exécute un effet de bord pour chaque élément   |
| `group(keyExtractor)`                             | `unordered_map<K, vector<E>>` | Regroupe par clé                            |
| `groupBy(keyExtractor, valueExtractor)`          | `unordered_map<K, vector<V>>` | Regroupe par clé et extrait la valeur       |
| `join()`                                          | `Charsequence`              | Joint avec le format par défaut                |
| `join(delimiter)`                                 | `Charsequence`              | Joint avec un délimiteur personnalisé          |
| `join(prefix, delimiter, suffix)`                | `Charsequence`              | Joint avec un format complètement personnalisé |
| `noneMatch(predicate)`                            | `bool`                     | Aucun élément ne correspond à la condition     |
| `out()`                                           | `Charsequence`              | Sortie vers stdout (prend en charge delimiter/prefix/suffix/converter) |
| `partition(size)`                                 | `vector<vector<E>>`         | Partitionne par taille fixe                    |
| `partitionBy(keyExtractor)`                       | `vector<vector<E>>`         | Partitionne par clé d'index                    |
| `partitionBy(keyExtractor, valueExtractor)`       | `vector<vector<V>>`         | Partitionne par clé d'index et extrait la valeur |
| `range<D>()`                                      | `D`                        | Plage numérique (max - min)                    |
| `range<D>(mapper)`                                | `D`                        | Plage numérique après mappage                  |
| `reduce(accumulator)`                             | `std::optional<E>`         | Réduction sans identité                        |
| `reduce(identity, accumulator)`                   | `E`                        | Réduction avec identité                        |
| `reduce(identity, acc, comb)`                     | `R`                        | Réduction complètement personnalisée           |
| `summate<D>()`                                    | `D`                        | Somme                                          |
| `summate<D>(mapper)`                              | `D`                        | Somme après mappage                            |
| `toArray<N>()`                                    | `std::array<E, N>`         | Collecte dans un array de taille fixe          |
| `toDeque()`                                       | `std::deque<E>`            | Collecte dans une deque                        |
| `toForwardList()`                                 | `std::forward_list<E>`     | Collecte dans une forward_list                 |
| `toList()`                                        | `std::list<E>`             | Collecte dans une list                         |
| `toMap(keyExtractor)`                             | `std::map<K, E>`           | Collecte dans une map par clé                  |
| `toMap(keyExtractor, valueExtractor)`             | `std::map<K, V>`           | Collecte dans une map avec clé et valeur personnalisés |
| `toMultimap(keyExtractor)`                        | `std::multimap<K, E>`      | Collecte dans une multimap par clé             |
| `toMultimap(keyExtractor, valueExtractor)`        | `std::multimap<K, V>`      | Collecte dans une multimap avec clé et valeur personnalisés |
| `toMultiset()`                                    | `std::multiset<E>`         | Collecte dans un multiset                      |
| `toPriorityQueue()`                               | `std::priority_queue<E>`   | Collecte dans une priority_queue               |
| `toQueue()`                                       | `std::queue<E>`            | Collecte dans une queue                        |
| `toSet()`                                         | `std::set<E>`              | Collecte dans un set (unique et trié)          |
| `toStack()`                                       | `std::stack<E>`            | Collecte dans une stack                        |
| `toUnorderedMap(keyExtractor, valueExtractor)`    | `std::unordered_map<K, V>` | Collecte dans une unordered_map                |
| `toUnorderedMultimap(keyExtractor)`               | `std::unordered_multimap<K, E>`| Collecte dans une unordered_multimap par clé |
| `toUnorderedMultimap(keyExtractor, valueExtractor)` | `std::unordered_multimap<K, V>`| Collecte dans une unordered_multimap avec clé et valeur personnalisés |
| `toUnorderedMultiset()`                           | `std::unordered_multiset<E>`| Collecte dans un unordered_multiset           |
| `toUnorderedSet()`                                | `std::unordered_set<E>`    | Collecte dans un unordered_set                 |
| `toVector()`                                      | `std::vector<E>`           | Collecte dans un vector                        |

### 📈 Statistics<E,D> — Méthodes statistiques
| Méthode             | Type de retour         | Description                     |
| :----------------- | :------------------- | :----------------------------- |
| `summate()`        | `D`                  | Somme                          |
| `average()`        | `D`                  | Moyenne                        |
| `minimum()`        | `std::optional<D>`  | Valeur minimum                 |
| `maximum()`        | `std::optional<D>`  | Valeur maximum                 |
| `range()`          | `D`                  | Plage (max - min)              |
| `variance()`       | `D`                  | Variance de la population      |
| `standardDeviation()`| `D`                | Écart-type de la population    |
| `median()`         | `std::optional<D>`  | Médiane                        |
| `mode()`           | `std::optional<E>`  | Mode                           |
| `percentile(p)`    | `std::optional<D>`  | p-ème centile                 |
| `firstQuartile()`  | `std::optional<D>`  | Premier quartile (Q1)          |
| `thirdQuartile()`  | `std::optional<D>`  | Troisième quartile (Q3)        |
| `interquartileRange()`| `std::optional<D>`| Écart interquartile (IQR)      |
| `skewness()`       | `D`                  | Asymétrie                      |
| `kurtosis()`       | `D`                  | Kurtosis                       |
| `frequency()`      | `map<E, complex>`   | Caractéristiques du domaine fréquentiel |
| `distribute()`     | `map<E, complex>`   | Caractéristiques de distribution spatiale |
| `dft()`            | `vector<complex<double>>` | Transformée de Fourier discrète |
| `idft()`           | `vector<complex<double>>` | Transformée de Fourier discrète inverse |
| `fft()`            | `vector<complex<double>>` | Transformée de Fourier rapide |
| `ifft()`           | `vector<complex<double>>` | Transformée de Fourier rapide inverse |
| `gradient(...)`    | `vector<double>`     | Descente de gradient           |

Toutes les méthodes ci-dessus prennent également en charge une version de paramètre `mapper` optionnelle.

### 🔧 Méthodes d'opérations intermédiaires de Semantic<E>
| Catégorie      | Méthode        | Description                                   |
| :------------ | :------------ | :------------------------------------------- |
| Transformation d'élément | map         | Transformation de mappage un-à-un            |
|               | flatMap     | Mappage un-à-plusieurs et aplatissement      |
|               | flat        | Aplatir des flux imbriqués (prend en charge Semantic et conteneurs) |
| Filtre d'élément | filter      | Filtrage conditionnel                        |
|               | takeWhile   | Prendre tant que la condition est vraie      |
|               | dropWhile   | Jeter tant que la condition est vraie        |
|               | distinct    | Supprimer les doublons (prend en charge un comparateur personnalisé) |
| Contrôle de taille | limit       | Limiter le nombre d'éléments                 |
|               | skip        | Sauter les n premiers éléments               |
|               | sub         | Extraire une sous-plage [start, end)         |
| Opérations d'index | redirect    | Réassigner les index                         |
|               | reverse     | Inverser les index                           |
|               | translate   | Décaler les index                            |
| Observation   | peek        | Observer chaque élément (ne modifie pas le flux) |
| Déclaration parallèle | parallel(n) | Déclarer le niveau de concurrence            |
| Concaténation | concatenate | Concaténer Semantic/éléments/générateurs/conteneurs |
| Conversion terminale | toUnordered / toOrdered / toWindow / toStatistics / sort | Convertir en Collectable |

---

## 🔧 Couche 6 : semantics.h — Usines de construction de flux

### 🔢 Génération de plages numériques
| Méthode                        | Description                     |
| :---------------------------- | :----------------------------- |
| `useRange(start, end)`       | Générer une plage [start, end) |
| `useRange(start, end, step)` | Plage avec pas (prend en charge les négatifs) |
| `useRangeClosed(start, end)` | Générer une plage fermée [start, end] |
| `useRangeClosed(start, end, step)` | Plage fermée avec pas      |

### ♾️ Génération de flux infinis
| Méthode                      | Description                         |
| :-------------------------- | :--------------------------------- |
| `useInfinite(seed, generator)`| Itération infinie à partir d'une valeur initiale |
| `useGenerate(supplier)`     | Appels infinis au fournisseur      |
| `useGenerate(supplier, limit)`| Nombre limité d'appels au fournisseur |
| `useIterate(seed, generator)` | Itération infinie à partir d'une valeur initiale |
| `useIterate(seed, generator, limit)` | Nombre limité d'itérations |
| `useRandom()`               | Flux infini d'entiers aléatoires   |
| `useRandom(min, max)`       | Flux de nombres aléatoires dans la plage spécifiée |
| `useRandom(min, max, count)`| Flux de nombres aléatoires avec plage et nombre spécifiés |

### 📦 Construction de conteneurs et d'éléments
| Méthode                  | Description                     |
| :---------------------- | :----------------------------- |
| `useEmpty()`            | Créer un flux vide             |
| `useOf(element)`        | Créer un flux à partir d'un seul élément |
| `useOf(e1, e2)`         | Créer un flux à partir de deux éléments |
| `useOf(e1, e2, e3)`     | Créer un flux à partir de trois éléments |
| `useOf({...})`          | Créer un flux à partir d'une liste d'initialisation |
| `useFrom(container)`    | Créer un flux à partir d'un conteneur standard |
| `useFrom({...})`        | Créer un flux à partir d'une liste d'initialisation |
| `useRepeat(element, count)` | Répéter l'élément n fois     |

### 📄 Traitement de texte et Unicode
| Méthode                      | Description                         |
| :-------------------------- | :--------------------------------- |
| `useBlob(text)`             | Diviser la chaîne octet par octet en flux de char |
| `useBlob(text, start, end)` | Diviser une plage spécifique octet par octet |
| `useBlob(istream)`          | Lire ligne par ligne depuis le flux d'entrée |
| `useBlob(istream, delimiter)` | Lire par délimiteur depuis le flux d'entrée |
| `useText(text)`             | Flux de texte complet (Charsequence) |
| `useText(text, delimiter)`  | Diviser le texte par délimiteur    |
| `useText(istream)`          | Lire tout le contenu depuis le flux d'entrée |
| `useSequence(charsequence)` | Créer un flux de points de code à partir d'une séquence de caractères |
| `useSequence(text, encoding)` | Créer un flux de points de code à partir de texte avec encodage spécifié |
| `useCharsequence(charsequence)` | Séquence de caractères comme flux complet |
| `useCharsequence(charsequence, delimiter)` | Diviser la séquence de caractères par délimiteur |

---

## 🔐 Couche 7 : hash.h / less.h — Le langage universel du monde des conteneurs

Fournit un support complet de hachage et de comparaison pour tous les conteneurs de bibliothèque standard (y compris les conteneurs imbriqués), `pair`, `tuple`, `optional`, `variant`, types temporels `chrono`, nombres `complex`, et plus. Les conteneurs imbriqués à n'importe quelle profondeur et combinaison peuvent maintenant être utilisés comme clés dans `unordered_set` ou éléments dans `set`. 🌉

---

## 🚀 Conseils d'optimisation des performances

1.  **Choisissez le bon conteneur** : Utilisez `toUnordered()` si l'ordre n'a pas d'importance, `toOrdered()` ou `sort()` si un tri est nécessaire.
2.  **Tirez parti du parallélisme** : Utilisez `parallel()` pour les grands ensembles de données.
3.  **Optimisez l'ordre des opérations** : Filtrez tôt, triez sagement.
4.  **Utilisez l'évaluation paresseuse** : `takeWhile` et `limit` peuvent se terminer tôt.

---

Semantic-Cpp — Construire des pipelines de traitement de données efficaces et clairs avec C++ moderne. 🚀🎯✨
