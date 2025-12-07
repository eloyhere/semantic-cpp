# Semantic Stream — Bibliothèque moderne de streams fonctionnels en C++

Semantic est une bibliothèque de streams fonctionnels 100 % header-only, haute performance et à évaluation totalement paresseuse pour C++17 et supérieur.  
Elle s’inspire directement de JavaScript Generator, de l’API Java Stream, du package java.util.function et de la philosophie des index MySQL.

Elle allie l’élégance de la programmation fonctionnelle à une véritable évaluation paresseuse, à l’exécution parallèle et à un système d’indexation temporelle, permettant de manipuler avec fluidité des séquences finies comme infinies.

## Principales fonctionnalités

- **Évaluation totalement paresseuse** — Les opérations intermédiaires ne s’exécutent qu’au moment de l’opération terminale.
- **Support natif des streams infinis** — Les générateurs rendent les séquences infinies naturelles.
- **Indexation par timestamp** — Chaque élément porte un timestamp implicite ou explicite, offrant des sauts et recherches ultra-efficaces à la manière des index MySQL.
- **Parallélisation en une ligne** — `.parallel()` ou `.parallel(n)` transforme le pipeline en multithread.
- **Opérations fonctionnelles riches** — `map`, `filter`, `flatMap`, `reduce`, `collect`, `group`, statistiques, etc.
- **API Collector style Java** — supplier, accumulator, combiner, finisher.
- **Statistiques puissantes** — moyenne, médiane, mode, variance, écart-type, quartiles, skewness, kurtosis…
- **Nombreuses méthodes de création** — à partir de conteneurs, tableaux, plages, générateurs ou remplissage.

## Philosophie de conception

Semantic conçoit un stream comme une suite temporelle produite par un `Generator`. Les inspirations principales sont :

- **JavaScript Generator** — production à la demande (pull)
- **Java Stream** — API fluide et séparation claire opérations intermédiaires / terminales
- **java.util.function** — alias `Function`, `Consumer`, `Predicate`, etc.
- **Index MySQL** — timestamps logiques pour des `skip`, `limit`, `redirect`, `translate` ultra-rapides

## Concept central

```cpp
using Generator<E> = BiConsumer<
    BiConsumer<E, Timestamp>,   // yield(élément, timestamp)
    Predicate<E>                // condition d’annulation
>;
```

Le générateur reçoit une fonction de livraison et une condition d’annulation. Cette abstraction de bas niveau alimente toutes les sources de streams, finies ou infinies.

## Exemples rapides

```cpp
using namespace semantic;

// Stream infini de nombres aléatoires
auto s = Semantic<int>::iterate([](auto yield, auto cancel) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> d(1, 100);
    Timestamp ts = 0;
    while (!cancel(d(gen))) {
        yield(d(gen), ts++);
    }
});

// Nombres premiers avec évaluation paresseuse
auto primes = Semantic<long long>::range(2LL, 1'000'000LL)
    .filter([](long long n) {
        return Semantic<long long>::range(2LL, n)
            .takeWhile([n](long long d){ return d * d <= n; })
            .noneMatch([n](long long d){ return n % d == 0; });
    });

// Comptage de mots en parallèle
auto wordCount = Semantic<std::string>::from(lines)
    .flatMap([](const std::string& line) {
        return Semantic<std::string>::from(split(line));
    })
    .parallel()
    .group([](const std::string& w){ return w; })
    .map([](auto& p){ return std::make_pair(p.first, p.second.size()); });
```

## Prérequis de compilation

- C++17 ou supérieur
- Header-only — il suffit d’inclure `#include "semantic.hpp"`
- Aucune dépendance externe

Un pool de threads global (`semantic::globalThreadPool`) est automatiquement initialisé avec `std::thread::hardware_concurrency()` workers.

## Licence

MIT License — utilisation libre dans les projets commerciaux et open-source.

## Auteur

Implémentation performante et idiomatique qui regroupe le meilleur de la programmation fonctionnelle moderne au service du C++ actuel.

¡Et voilà ! Tu as maintenant `README.fr.md`.
