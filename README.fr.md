# 🚀 Semantic-Cpp : Un cadre de traitement de flux intelligents prêt pour l’avenir

Semantic-Cpp est une bibliothèque de traitement de flux C++ entièrement repensée selon une approche moderne. Elle adopte une architecture modulaire **« multi-fichiers d’en-tête, zéro dépendance externe »**. Chaque fichier d’en-tête possède une responsabilité claire, indépendante et testable, formant ensemble un écosystème complet pour le traitement de flux. Cette bibliothèque fusionne de manière créative le meilleur de plusieurs paradigmes de programmation :

- 🎯 L’élégance et la fluidité de l’**API Stream de Java** : appels chaînés, programmation déclarative pour un code aussi fluide qu’un poème.
- ⚡ La paresse et la flexibilité des **Générateurs JavaScript** : exécution différée, génération à la demande et efficacité mémoire.
- 🗄️ L’efficacité et l’ordonnancement des **index de bases de données** : tri intelligent, pilotage par index, idéal pour les données de séries temporelles.

Contrairement aux approches traditionnelles (boucles manuelles, callbacks asynchrones), Semantic-Cpp propose une solution **sûre en typage, expressive et haute performance**. Sa philosophie centrale est le **contrôle précis du flux de données** : les données ne circulent que lorsque cela est nécessaire, leur ordre et leur position étant régis finement par des **« index »** afin d’optimiser l’utilisation des ressources.

---

## 📐 Architecture du projet : conception modulaire à sept couches

Semantic-Cpp est composé de **sept fichiers d’en-tête principaux**, organisés de manière progressive, chacun ayant une responsabilité unique et indépendante :

```
┌─────────────────────────────────────────────────┐
│                 semantics.h                     │
│    (Fabriques de flux : plages numériques,      │
│     conteneurs, texte, Unicode)                 │
├─────────────────────────────────────────────────┤
│                  semantic.h                     │
│   (Opérations intermédiaires de flux, système   │
│    Collectable, spécialisations de conteneurs)  │
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│  (Fabriques de collecteurs : correspondance,    │
│   recherche, agrégation, statistiques, DFT/FFT…)│
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│     (Cadre de collecteurs : modèle à cinq étapes│
│      , prise en charge de la concurrence et du  │
│      parallélisme)                              │
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│  (Séquences de caractères Unicode, conversion   │
│   multi-encodage, Builder, Buffer)              │
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│   (Pool de threads global : soumission de tâches│
│    , arrêt d’urgence, propagation d’exceptions) │
├─────────────────────────────────────────────────┤
│                 function.h                      │
│  (Alias de types : Generator, Supplier, Consumer│
│   , etc.)                                       │
└─────────────────────────────────────────────────┘
```

### Graphe des dépendances

```
function.h          ← aucune dépendance, pierre angulaire des types
pool.h              ← dépend de function.h
charsequence.h      ← module indépendant, traitement Unicode
collector.h         ← dépend de function.h, pool.h
collectors.h        ← dépend de collector.h, charsequence.h
semantic.h          ← dépend de collector.h, collectors.h, charsequence.h
semantics.h         ← dépend de semantic.h
```

Chaque fichier d’en-tête peut être compilé et testé indépendamment ou inclus selon les besoins.  
Par exemple, si vous avez seulement besoin des fonctionnalités de collecte, il suffit d’inclure `collector.h` et `collectors.h`.

---

## 🏗️ Couche 1 : function.h — Fondement des types

`function.h` définit le système de types de toute la bibliothèque, base commune de tous les modules :

```cpp
namespace function {
    using Timestamp = long long;           // type d’index
    using Module = unsigned long long;     // type de module / compteur
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept
        std::function<bool(T, Timestamp)>       // interrupt
    )>;
}
```

`Generator` est l’abstraction centrale de tout le système de flux : il accepte deux callbacks — `accept` (pour recevoir les données) et `interrupt` (pour arrêter le traitement) — incarnant le modèle de **« tirage paresseux » (lazy pull)**.

---

## ⚡ Couche 2 : pool.h — Fondement de la concurrence

`pool.h` fournit le pool de threads global `pool::pool`, qui sert de moteur de concurrence au cadre :

| Fonctionnalité | Description |
|---------------|-------------|
| 🎯 Parallélisme déclaratif | `parallel(n)` ne fait que déclarer l’intention ; activé automatiquement lors des opérations terminales |
| 🛡️ Arrêt d’urgence | Intègre `emergencyShutdown()` et un gestionnaire `std::set_terminate` |
| 🔄 Propagation des exceptions | `submit()` renvoie `std::future`, assurant une propagation sûre des exceptions |

---

## 🔤 Couche 3 : charsequence.h — Séquences de caractères Unicode

`charsequence.h` est un module complet pour la gestion d’Unicode, offrant création, conversion et manipulation de séquences de caractères :

| Capacité | Description |
|----------|-------------|
| 🌐 Prise en charge multi-encodage | UTF‑8, UTF‑16 (LE/BE), UTF‑32 (LE/BE), ASCII, Latin1 |
| 🔍 Itérateur de points de code | `PointIterator` prend en charge le parcours bidirectionnel des points de code Unicode |
| 🏗️ Modèle Builder | La classe `Builder` permet des concaténations efficaces au niveau octet |
| 📦 Tampon | La classe `Buffer` fournit un tampon circulaire sûr pour les threads |
| 🔑 Hachage et comparaison | Tous les types principaux disposent de spécialisations `std::hash` et `std::less` |

### Types principaux

| Type | Description |
|------|-------------|
| `Meta` | Enveloppe de métadonnées stockant des valeurs entières non signées |
| `Point` | Point de code Unicode avec détection des paires de substitution et validation |
| `Charsequence` | Séquence de caractères immuable prenant en charge la division, le remplacement, la recherche, la casse, etc. |
| `Builder` | Constructeur d’octets mutable avec `prepend`, `insert`, `append` et divers types de données |
| `Buffer` | Tampon circulaire sûr pour les threads avec gestion de lecture/écriture, prefetch et capacité |

---

## 🔧 Couche 4 : collector.h — Cadre de collecteurs

`collector.h` implémente le modèle Collector, constituant le moteur central derrière les opérations terminales.

### Modèle à cinq étapes

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (court-circuit optionnel)
```

### Alias de types

| Type | Définition | Rôle |
|------|-----------|------|
| `Identity<A>` | `Supplier<A>` | Fournit la valeur initiale |
| `Accumulator<A,E>` | `TriFunction<A,E,Timestamp,A>` | Accumule les éléments |
| `Combiner<A>` | `BiFunction<A,A,A>` | Combine les résultats parallèles |
| `Finisher<A,R>` | `Function<A,R>` | Transformation finale |
| `Interrupt<E,A>` | `TriPredicate<E,Timestamp,A>` | Évaluation du court-circuit |

### Prise en charge de la concurrence

`Collector::collect()` gère automatiquement :

- 📦 Partitionnement des données (réparties entre les threads par modulo d’index)
- 🔗 Fusion des résultats (via `Combiner`)
- ⚠️ Propagation des exceptions (via `std::exception_ptr` et `std::atomic<bool>`)

---

## 🏭 Couche 5 : collectors.h — Fabrique de collecteurs

`collectors.h` propose une riche collection de fonctions fabriques de collecteurs prêtes à l’emploi.

### 📊 Opérations de correspondance

| Méthode | Description | Type de retour |
|--------|-------------|----------------|
| `useAllMatch(predicate)` | Tous les éléments satisfont la condition | `bool` |
| `useAnyMatch(predicate)` | Au moins un élément satisfait la condition | `bool` |
| `useNoneMatch(predicate)` | Aucun élément ne satisfait la condition | `bool` |

### 🔍 Opérations de recherche

| Méthode | Description | Type de retour |
|--------|-------------|----------------|
| `useFindFirst()` | Trouver le premier élément | `std::optional<E>` |
| `useFindLast()` | Trouver le dernier élément | `std::optional<E>` |
| `useFindAny()` | Trouver un élément aléatoire | `std::optional<E>` |
| `useFindAt(index)` | Trouver l’élément à l’index spécifié (indices négatifs pris en charge) | `std::optional<E>` |
| `useFindMaximum()` | Trouver le maximum (comparateur personnalisé pris en charge) | `std::optional<E>` |
| `useFindMinimum()` | Trouver le minimum (comparateur personnalisé pris en charge) | `std::optional<E>` |

### 📈 Opérations d’agrégation

| Méthode | Description | Type de retour |
|--------|-------------|----------------|
| `useCount()` | Compter le nombre total d’éléments | `Module` |
| `useSummate<E,D>()` | Somme | `D` |
| `useSummate<E,D>(mapper)` | Somme mappée | `D` |
| `useAverage<E,D>()` | Moyenne | `D` |
| `useAverage<E,D>(mapper)` | Moyenne mappée | `D` |
| `useRange<E,D>()` | Étendue numérique (max − min) | `D` |
| `useRange<E,D>(mapper)` | Étendue mappée | `D` |
| `useMinimum<E,D>()` | Valeur minimale | `std::optional<D>` |
| `useMaximum<E,D>()` | Valeur maximale | `std::optional<D>` |

### 📊 Opérations statistiques

| Méthode | Description | Type de retour |
|--------|-------------|----------------|
| `useVariance<E,D>()` | Variance de population | `D` |
| `useStandardDeviation<E,D>()` | Écart-type de population | `D` |
| `useSkewness<E,D>()` | Asymétrie | `D` |
| `useKurtosis<E,D>()` | Aplatissement (kurtosis) | `D` |
| `useMedian<E,D>()` | Médiane | `std::optional<D>` |
| `useMode<E>()` | Mode (basé sur le domaine fréquentiel) | `std::optional<E>` |
| `usePercentile<E,D>(p)` | Percentile p | `std::optional<D>` |
| `useFrequency<E>()` | Caractéristiques fréquentielles (codage de phase d’index) | `std::map<E, complex>` |
| `useDistribution<E>()` | Caractéristiques de distribution spatiale (codage de position) | `std::map<E, complex>` |

### 🔗 Opérations de réduction

| Méthode | Description | Type de retour |
|--------|-------------|----------------|
| `useReduce(reducer)` | Réduction sans valeur initiale | `std::optional<E>` |
| `useReduce(identity, reducer)` | Réduction avec valeur initiale | `E` |
| `useReduce(id, red, comb, fin)` | Réduction entièrement personnalisée | `R` |

### 📦 Opérations de collecte dans des conteneurs

| Méthode | Description | Type de retour |
|--------|-------------|----------------|
| `useToVector()` | Collecter dans `std::vector` | `std::vector<E>` |
| `useToList()` | Collecter dans `std::list` | `std::list<E>` |
| `useToDeque()` | Collecter dans `std::deque` | `std::deque<E>` |
| `useToForwardList()` | Collecter dans `std::forward_list` | `std::forward_list<E>` |
| `useToArray<N>()` | Collecter dans `std::array` de taille fixe | `std::array<E, N>` |
| `useToSet()` | Collecter dans `std::set` (dédupliqué et trié) | `std::set<E>` |
| `useToMultiset()` | Collecter dans `std::multiset` | `std::multiset<E>` |
| `useToUnorderedSet()` | Collecter dans `std::unordered_set` | `std::unordered_set<E>` |
| `useToUnorderedMultiset()` | Collecter dans `std::unordered_multiset` | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)` | Collecter dans `std::map` | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)` | Collecter dans `std::map` (valeur personnalisée) | `std::map<K, V>` |
| `useToMultimap(keyExtractor)` | Collecter dans `std::multimap` | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)` | Collecter dans `std::multimap` (valeur personnalisée) | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)` | Collecter dans `std::unordered_map` | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)` | Collecter dans `std::unordered_multimap` | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)` | Collecter dans `std::unordered_multimap` | `std::unordered_multimap<K, V>` |
| `useToStack()` | Collecter dans `std::stack` | `std::stack<E>` |
| `useToQueue()` | Collecter dans `std::queue` | `std::queue<E>` |
| `useToPriorityQueue()` | Collecter dans `std::priority_queue` | `std::priority_queue<E>` |

### 🔀 Opérations de groupement et de partition

| Méthode | Description | Type de retour |
|--------|-------------|----------------|
| `useGroup(keyExtractor)` | Grouper par clé | `std::unordered_map<K, vector<E>>` |
| `usePartition(size)` | Partitionner en blocs de taille fixe | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)` | Partitionner par clé personnalisée | `std::vector<vector<E>>` |

### 🎨 Opérations de sortie de chaînes

| Méthode | Description | Type de retour |
|--------|-------------|----------------|
| `useJoin()` | Joindre en chaîne (virgule par défaut, crochets) | `Charsequence` |
| `useJoin(delimiter)` | Joindre avec un délimiteur personnalisé | `Charsequence` |
| `useJoin(prefix, delimiter, suffix)` | Jointure formatée entièrement personnalisée | `Charsequence` |
| `useOut()` | Sortie formatée vers stdout | `Charsequence` |
| `useOut(delimiter)` | Sortie avec délimiteur personnalisé vers stdout | `Charsequence` |
| `useOut(prefix, delimiter, suffix)` | Sortie formatée entièrement personnalisée vers stdout | `Charsequence` |
| `useError()` | Sortie formatée vers stderr | `Charsequence` |
| `useError(delimiter)` | Sortie avec délimiteur personnalisé vers stderr | `Charsequence` |
| `useError(prefix, delimiter, suffix)` | Sortie formatée entièrement personnalisée vers stderr | `Charsequence` |

### 🧮 Outils mathématiques

| Méthode | Description | Type de retour |
|--------|-------------|----------------|
| `useDFT()` | Transformée de Fourier discrète | `vector<complex<double>>` |
| `useIDFT()` | Transformée de Fourier discrète inverse | `vector<complex<double>>` |
| `useFFT()` | Transformée de Fourier rapide (Cooley–Tukey) | `vector<complex<double>>` |
| `useIFFT()` | Transformée de Fourier rapide inverse | `vector<complex<double>>` |
| `useGradient(gradFunc, lr, iter, th)` | Descente de gradient (gradient analytique) | `vector<double>` |
| `useGradient(costFunc, lr, iter, th, h)` | Descente de gradient (gradient numérique) | `vector<double>` |

---

## 🌊 Couche 6 : semantic.h — Opérations intermédiaires de flux et système de collecte

`semantic.h` est le cœur du cadre et contient les espaces de noms `collectable` et `semantic`.

### Espace de noms collectable

Fournit la hiérarchie d’héritage des objets collectables :

| Classe | Description | Stockage sous-jacent |
|-------|-------------|----------------------|
| `Collectable<E>` | Classe de base abstraite avec méthode virtuelle pure `source()` | — |
| `OrderedCollectable<E>` | Collecte ordonnée avec tri personnalisé | `std::map<Timestamp, E>` |
| `UnorderedCollectable<E>` | Collecte non ordonnée, recherche O(1) | `std::unordered_map<Timestamp, E>` |
| `Statistics<E, D>` | Collecte statistique (hérite de `OrderedCollectable`) | Plus de 20 méthodes statistiques |
| `WindowCollectable<E>` | Collecte par fenêtres (hérite de `OrderedCollectable`) | Prend en charge `slide` / `tumble` |

#### Méthodes de la classe de base Collectable

Fournit toutes les méthodes de collecte terminale `toXxx()` (plus de 20 types de conteneurs), ainsi que :

`count()`, `findFirst()`, `findAny()`, `anyMatch()`, `allMatch()`, `noneMatch()`, `reduce()`, `join()`, `out()`, `error()`, `group()`, `partition()`, `partitionBy()`, etc.

#### Méthodes de la classe Statistics

| Méthode | Type de retour | Description |
|--------|---------------|-------------|
| `summate()` / `summate(mapper)` | `D` | Somme |
| `average()` / `average(mapper)` | `D` | Moyenne |
| `minimum()` / `minimum(mapper)` | `std::optional<D>` | Minimum |
| `maximum()` / `maximum(mapper)` | `std::optional<D>` | Maximum |
| `range()` / `range(mapper)` | `D` | Étendue (max − min) |
| `variance()` / `variance(mapper)` | `D` | Variance de population |
| `standardDeviation()` / `standardDeviation(mapper)` | `D` | Écart-type de population |
| `frequency()` / `frequency(mapper)` | `std::map<*, complex>` | Caractéristiques fréquentielles (codage de phase d’index) |
| `distribute()` / `distribute(mapper)` | `std::map<*, complex>` | Caractéristiques de distribution spatiale (codage de position) |
| `median()` / `median(mapper)` | `std::optional<D>` | Médiane |
| `mode()` | `std::optional<E>` | Mode |
| `percentile(p)` / `percentile(p, mapper)` | `std::optional<D>` | Percentile p |
| `firstQuartile()` / `firstQuartile(mapper)` | `std::optional<D>` | Premier quartile (Q1) |
| `thirdQuartile()` / `thirdQuartile(mapper)` | `std::optional<D>` | Troisième quartile (Q3) |
| `interquartileRange()` / `interquartileRange(mapper)` | `std::optional<D>` | Intervalle interquartile (Q3 − Q1) |
| `skewness()` / `skewness(mapper)` | `D` | Asymétrie |
| `kurtosis()` / `kurtosis(mapper)` | `D` | Aplatissement (kurtosis) |
| `dft()` | `vector<complex<double>>` | Transformée de Fourier discrète |
| `idft()` | `vector<complex<double>>` | Transformée de Fourier discrète inverse |
| `fft()` | `vector<complex<double>>` | Transformée de Fourier rapide |
| `ifft()` | `vector<complex<double>>` | Transformée de Fourier rapide inverse |
| `gradient(gradFunc, lr, iter, th)` | `vector<double>` | Descente de gradient (gradient analytique) |
| `gradient(costFunc, lr, iter, th, h)` | `vector<double>` | Descente de gradient (gradient numérique) |

### Espace de noms semantic

Fournit la classe modèle `Semantic<E>` et son système complet de spécialisation.

#### Résumé des méthodes principales du modèle

| Catégorie | Méthode | Description |
|---------|---------|-------------|
| 🎨 Transformation d’éléments | `map` | Mappage un-à-un |
| | `flatMap` | Mappage un-à-plusieurs avec aplatissement |
| | `flat` | Aplatir les flux imbriqués |
| 🔍 Filtrage d’éléments | `filter` | Filtrage conditionnel |
| | `takeWhile` | Prendre tant que la condition est vraie |
| | `dropWhile` | Ignorer tant que la condition est vraie |
| | `distinct` | Supprimer les doublons (comparateur personnalisé pris en charge) |
| 📏 Contrôle de quantité | `limit` | Limiter le nombre d’éléments |
| | `skip` | Sauter les n premiers éléments |
| | `sub` | Extraire une sous-plage |
| 📐 Opérations d’index | `redirect` | Remapper les index |
| | `reverse` | Inverser les index |
| | `translate` | Décaler les index (fixe ou dynamique) |
| 👀 Observation | `peek` | Observer les éléments sans les modifier |
| ⚡ Déclaration de parallélisme | `parallel` | Déclarer le degré de parallélisme |
| 🔗 Concaténation | `concatenate` | Concaténer un autre flux ou conteneur |
| 📤 Conversion terminale | `toUnordered` | Convertir en collecteur non ordonné |
| | `toOrdered` | Convertir en collecteur ordonné |
| | `toWindow` | Convertir en collecteur à fenêtres |
| | `toStatistics` | Convertir en collecteur statistique |

#### Prise en charge complète de la spécialisation des conteneurs

| Spécialisation | Description |
|---------------|-------------|
| `Semantic<std::vector<E>>` | Flux de vecteurs avec tri, suppression de doublons, etc. |
| `Semantic<std::list<E>>` | Flux de listes avec tri, suppression de doublons, etc. |

---

## 🏭 Couche 7 : semantics.h — Fabrique de construction de flux

`semantics.h` fournit toutes les fonctions fabriques de construction de flux.

### 📐 Génération de plages numériques

| Méthode | Description |
|--------|-------------|
| `useRange(start, end)` | Générer un flux numérique sur `[start, end)` |
| `useRange(start, end, step)` | Générer un flux numérique avec un pas |
| `useRangeClosed(start, end)` | Générer un flux numérique sur `[start, end]` |
| `useRangeClosed(start, end, step)` | Générer un flux numérique avec un pas sur un intervalle fermé |

### ♾️ Génération de flux infinis

| Méthode | Description |
|--------|-------------|
| `useInfinite(seed, generator)` | Itération infinie à partir d’une graine |
| `useGenerate(supplier)` | Appels infinis d’un fournisseur |
| `useGenerate(supplier, limit)` | Appels limités d’un fournisseur |
| `useIterate(seed, generator)` | Itération infinie à partir d’une graine |
| `useIterate(seed, generator, limit)` | Itération limitée |
| `useRandom()` | Flux infini d’entiers aléatoires |
| `useRandom(min, max)` | Flux infini d’aléas dans une plage |
| `useRandom(min, max, count)` | Aléas dans une plage avec un compte fixe |

### 📦 Construction de conteneurs et d’éléments

| Méthode | Description |
|--------|-------------|
| `useEmpty()` | Créer un flux vide |
| `useOf(element)` | Flux à un seul élément |
| `useOf(e1, e2)` | Flux à deux éléments |
| `useOf(e1, e2, e3)` | Flux à trois éléments |
| `useOf({...})` | Flux depuis une liste d’initialisation |
| `useFrom(container)` | Flux depuis n’importe quel conteneur standard |
| `useFrom({...})` | Flux depuis une liste d’initialisation |
| `useRepeat(element, count)` | Répéter un élément n fois |

### 📝 Traitement de texte

| Méthode | Description |
|--------|-------------|
| `useBlob(text)` | Diviser une chaîne en flux `char` par octets |
| `useBlob(text, start, end)` | Diviser une sous-chaîne en flux `char` par octets |
| `useBlob(istream)` | Lire les lignes depuis un flux d’entrée |
| `useBlob(istream, delimiter)` | Lire un flux d’entrée avec un délimiteur |
| `useText(text)` | Traiter une chaîne comme un flux de texte complet |
| `useText(text, delimiter)` | Diviser du texte par un délimiteur |
| `useText(istream)` | Lire tout le contenu d’un flux d’entrée |
| `useText(istream, delimiter)` | Lire un flux d’entrée avec un délimiteur |

### 🌐 Traitement Unicode

| Méthode | Description |
|--------|-------------|
| `useSequence(charsequence)` | Créer un flux de points de code depuis une séquence de caractères |
| `useSequence(charsequence, start, end)` | Créer un flux de points de code depuis une sous-séquence |
| `useSequence(text, encoding)` | Créer un flux de points de code depuis un texte avec encodage |
| `useSequence(istream, encoding)` | Créer un flux de points de code depuis un flux d’entrée avec encodage |
| `useCharsequence(charsequence)` | Traiter une séquence de caractères comme un flux complet |
| `useCharsequence(charsequence, delimiter)` | Diviser une séquence de caractères par un délimiteur |
| `useCharsequence(istream, encoding)` | Lire une séquence de caractères complète depuis un flux d’entrée |
| `useCharsequence(istream, delimiter, encoding)` | Lire une séquence de caractères depuis un flux d’entrée avec un délimiteur |

---

## 🧠 Concept central : un monde de données piloté par index

Semantic-Cpp abstrait le traitement des données comme des opérations sur des **« éléments »** et leurs **« positions logiques (index) »**. Comprendre ceci est la clé pour maîtriser la bibliothèque.

### 1. 📐 Transformations de base des index

| Méthode | Description |
|--------|-------------|
| `redirect(fn)` | Méthode centrale : réaffecter complètement les index via une fonction personnalisée |
| `reverse()` | Inverser toute la logique d’index (implémenté en interne via `redirect`) |
| `translate(offset)` | Décalage fixe |
| `translate(translator)` | Fonction de décalage dynamique calculée par élément et index |

### 2. 📊 La règle « autoritaire » du tri

> ⚠️ **`sort()` écrase tout** : après l’appel, toutes les opérations d’index précédentes sont ignorées ; les éléments reçoivent de nouveaux index d’ordre naturel basés sur leur valeur.

- `sort()` → Matérialisé immédiatement dans `OrderedCollectable`, trié naturellement par valeur d’élément
- `sort(comparator)` → Trié avec un comparateur personnalisé

### 3. ⚡ Traitement parallèle déclaratif

- `parallel(n)` ne fait que déclarer l’intention ; les threads ne démarrent pas immédiatement
- Les opérations terminales (`toUnordered()`, `count()`, etc.) déclenchent réellement le parallélisme
- Le pool de threads gère automatiquement la distribution des tâches et la fusion des résultats

### 4. 🎯 Comment choisir le bon conteneur final

| Méthode de conversion | Structure sous-jacente | Caractéristiques de performance | Cas d’usage optimal |
|----------------------|-----------------------|--------------------------------|----------------------|
| `sort()` | `OrderedCollectable` | Matérialisé et trié par valeur | Tri par valeur, pagination, séries temporelles |
| `toOrdered()` | `OrderedCollectable` | Conserve l’ordre d’index actuel | Conservation d’un ordre d’index personnalisé |
| `toUnordered()` | `UnorderedCollectable` | Moyenne O(1), performance maximale | Recherche rapide, dédoublonnage, agrégation |
| `toWindow()` | `WindowCollectable` | Basé sur des collections ordonnées | Analyse par fenêtres glissantes / défilantes |
| `toStatistics()` | `Statistics` | Plus de 20 méthodes statistiques | Analyse statistique complète |

---

## 🚀 Guide de démarrage rapide

### Installation

Placez tous les fichiers d’en-tête dans le répertoire de votre projet et assurez-vous que votre compilateur prend en charge **C++17 ou supérieur** :

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
#include "semantics.h"  // inclut automatiquement toutes les dépendances
```

---

## 🎯 Exemple de base : expérimenter les index et le tri

```cpp
auto result = semantic::useRange(0, 10)
    .map(int x { return x * x; })
    .redirect(int value, auto index -> long long {
        return index * 2;
    })
    .reverse()
    .sort()                              // force le tri par valeur, écrase toutes les opérations d’index
    .toVector();

// Sortie : 0 1 4 9 16 25 36 49 64 81
```

## ⚡ Exemple de traitement parallèle

```cpp
auto count = semantic::useRange(1, 1000)
    .parallel(4)
    .filter(int x { return x % 2 == 0; })
    .toUnordered()
    .count();

// Sortie : Nombre de pairs : 500
```

## 📊 Exemple d’analyse statistique

```cpp
auto stats = semantic::useRange(1, 101)
    .toStatistics<int, double>();

auto avg = stats.average();               // moyenne
auto med = stats.median();                // médiane
auto std = stats.standardDeviation();     // écart-type
auto q1  = stats.firstQuartile();          // premier quartile
auto q3  = stats.thirdQuartile();          // troisième quartile
auto skew = stats.skewness();              // asymétrie
```

## 🔬 Exemple d’analyse fréquentielle

```cpp
auto freq = data.toUnordered().frequency();
for (const auto& [element, z] : freq) {
    auto magnitude = std::abs(z);  // concentration de la distribution
    auto phase     = std::arg(z);  // phase centrale de la distribution
}
```

## 🧮 Exemple de FFT

```cpp
auto spectrum = semantic::useRange(0, 8)
    .map(int x -> std::complex<double> {
        return {static_cast<double>(x), 0.0};
    })
    .toUnordered()
    .collect(collector::useFFT<double>());
```

---

## ⚡ Recommandations d’optimisation des performances

1. 🎯 **Choisissez le bon conteneur**
   - Recherches d’égalité, agrégation non triée → `toUnordered()`
   - Requêtes d’intervalle, tri, pagination → `toOrdered()` ou `sort()`
   - Analyse de fenêtres en temps réel → `toWindow()`
2. ⚡ **Exploitez judicieusement le parallélisme** : utilisez `parallel()` pour les grands ensembles de données ou les traitements lourds ; évitez les E/S bloquantes.
3. 📐 **Optimisez l’ordre des opérations** : filtrez tôt, triez avec discernement.
4. 🔄 **Profitez de l’évaluation paresseuse** : les opérations intermédiaires ne s’exécutent qu’à la terminaison ; `takeWhile` et `limit` permettent une sortie anticipée.

---

## 📊 Comparaison avec la bibliothèque standard C++ et les alternatives

| Caractéristique | Semantic-Cpp | C++20/23 Ranges | Boucles traditionnelles |
|----------------|-------------|-----------------|------------------------|
| 🎯 Paradigme central | Déclaratif, piloté par index | Axé sur les vues, composition fonctionnelle | Impératif, procédural |
| ⚡ Prise en charge du parallélisme | Déclaratif, pool de threads automatique | Nécessite de combiner des algorithmes parallèles | Implémentation manuelle |
| 📐 Tri et indexation | Contrôle fin des index, prise en charge des index négatifs | Tri destructif | Totalement manuel |
| 📊 Analyse statistique | Plus de 20 méthodes statistiques intégrées | Non intégré | Nécessite des bibliothèques tierces |
| 🔬 Analyse fréquentielle | DFT / FFT / caractéristiques fréquentielles natives | Non pris en charge nativement | Nécessite des bibliothèques tierces |
| 🧮 Descente de gradient | Modes analytique et numérique | Non intégré | Nécessite des bibliothèques tierces |
| 🌐 Unicode | Prise en charge native multi-encodage (UTF‑8/16/32, etc.) | Non pris en charge nativement | Gestion manuelle |
| 📦 Collecte en conteneurs | Plus de 20 conteneurs standards couverts | Prise en charge partielle | Implémentation manuelle |
| 📦 Dépendances | Zéro dépendance externe, 7 en-têtes | Bibliothèque standard | Aucune |

---

## 📜 Licence

- 📄 **Licence** : MIT

---

**Semantic-Cpp — Construire des pipelines de traitement de données efficaces et clairs avec le C++ moderne. 🚀**
