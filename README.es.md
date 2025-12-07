# Semantic Stream — Biblioteca moderna de streams funcionales en C++

Semantic es una biblioteca de streams funcionales de sólo cabecera, de alto rendimiento y evaluación completamente perezosa para C++17 y superiores. Está inspirada en JavaScript Generator, Java Stream API, el paquete java.util.function y en el diseño de índices de MySQL.

Combina la expresividad de la programación funcional con evaluación verdaderamente perezosa, ejecución paralela y un sistema de indexación basado en marcas de tiempo, permitiendo procesar de forma elegante tanto secuencias finitas como infinitas.

## Características principales

- **Evaluación completamente perezosa** — Las operaciones intermedias sólo se ejecutan al invocar una operación terminal.
- **Soporte nativo para streams infinitos** — Los generadores permiten crear secuencias infinitas de forma natural.
- **Indexación por timestamp** — Cada elemento lleva un timestamp implícito o explícito, habilitando saltos y búsquedas eficientes al estilo de índices MySQL.
- **Paralelismo con una sola línea** — `.parallel()` o `.parallel(n)` convierte el pipeline en multihilo.
- **Amplio conjunto de operaciones funcionales** — `map`, `filter`, `flatMap`, `reduce`, `collect`, `group`, estadísticas, etc.
- **API Collector al estilo Java** — supplier, accumulator, combiner y finisher.
- **Potentes funciones estadísticas** — media, mediana, moda, varianza, desviación estándar, cuartiles, asimetría, curtosis…
- **Múltiples formas de construcción** — desde contenedores, arrays, rangos, generadores o relleno.

## Filosofía de diseño

Semantic modela un stream como una secuencia temporal generada por un `Generator`. Las fuentes de inspiración son:

- **JavaScript Generator** — generación bajo demanda (pull).
- **Java Stream** — API fluida y separación entre operaciones intermedias y terminales.
- **Paquete java.util.function** — aliases para `Function`, `Consumer`, `Predicate`, etc.
- **Índices de MySQL** — uso de timestamps lógicos para `skip`, `limit`, `redirect`, `translate` eficientes.

## Concepto central

```cpp
using Generator<E> = BiConsumer<
    BiConsumer<E, Timestamp>,   // yield(elemento, timestamp)
    Predicate<E>                // condición de cancelación
>;
```

El generador recibe una función de entrega y una condición de cancelación. Esta abstracción de bajo nivel impulsa todas las fuentes de streams, tanto finitas como infinitas.

## Ejemplos rápidos

```cpp
using namespace semantic;

// Stream infinito de números aleatorios
auto s = Semantic<int>::iterate([](auto yield, auto cancel) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> d(1, 100);
    Timestamp ts = 0;
    while (!cancel(d(gen))) {
        yield(d(gen), ts++);
    }
});

// Números primos con evaluación perezosa
auto primes = Semantic<long long>::range(2LL, 1'000'000LL)
    .filter([](long long n) {
        return Semantic<long long>::range(2LL, n)
            .takeWhile([n](long long d){ return d * d <= n; })
            .noneMatch([n](long long d){ return n % d == 0; });
    });

// Conteo de palabras en paralelo
auto wordCount = Semantic<std::string>::from(lines)
    .flatMap([](const std::string& line) {
        return Semantic<std::string>::from(split(line));
    })
    .parallel()
    .group([](const std::string& w){ return w; })
    .map([](auto& p){ return std::make_pair(p.first, p.second.size()); });
```

## Requisitos de compilación

- C++17 o superior
- Sólo cabecera — basta con `#include "semantic.hpp"`
- Sin dependencias externas

Se crea automáticamente un pool de hilos global (`semantic::globalThreadPool`) con `std::thread::hardware_concurrency()` trabajadores.

## Licencia

MIT License — uso libre en proyectos comerciales y de código abierto.

## Autor

Implementación de alto rendimiento que reúne las mejores ideas de la programación funcional moderna, adaptadas al C++ idiomático actual.

¡Listo! Ahora tienes también `README.es.md`.
