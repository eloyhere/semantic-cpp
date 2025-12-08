# semantic-cpp – Biblioteca moderna de streams semánticos en C++

**semantic-cpp** es una biblioteca header-only, C++17+, de evaluación perezosa y alto rendimiento.  
Su idea central: **cada elemento lleva desde su nacimiento un índice lógico (Timestamp)** – todas las operaciones que afectan al orden son simplemente transformaciones de ese índice.

- Streams verdaderamente infinitos  
- El índice es ciudadano de primera clase y puede remapearse libremente  
- Paralelismo por defecto  
- Solo al recolectar decides si respetar o ignorar todas las operaciones de índice  

Licencia MIT – uso ilimitado en proyectos comerciales y de código abierto.

## Filosofía de diseño

> **El índice decide el orden – la ejecución solo decide la velocidad.**

- `redirect`, `reverse`, `shuffle`, `cycle` etc. modifican exclusivamente el índice lógico  
- Solo con `.toOrdered()` se respetan esas operaciones → el resultado se ordena exactamente según el índice final  
- Con `.toUnordered()` todas las operaciones de índice se ignoran → velocidad máxima, sin orden

```text
.toOrdered()   → todas las operaciones de índice tienen efecto
.toUnordered() → todas las operaciones de índice se ignoran (camino más rápido)
```

## Inicio rápido

```cpp
#include "semantic.h"
using namespace semantic;

int main() {
    // 1. Las operaciones de índice tienen efecto
    auto v1 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })   // invertir
        .toOrdered()                                  // ¡obligatorio!
        .toVector();   // resultado: [99, 98, …, 0]

    // 2. Las operaciones de índice se ignoran (máxima velocidad)
    auto v2 = Semantic<int>::range(0, 100)
        .redirect([](auto&, auto i) { return -i; })
        .toUnordered()                                // índice ignorado
        .toVector();   // resultado: desordenado, pero ultrarrápido

    // 3. Stream cíclico infinito + recolección ordenada
    auto top10 = Semantic<int>::range(0, 1'000'000'000)
        .redirect([](auto&, auto i) { return i % 1000; }) // cíclico 0-999
        .toOrdered()
        .limit(10)
        .toVector();   // [0,1,2,3,4,5,6,7,8,9]

    return 0;
}
```

## Todos los métodos importantes con ejemplos de encadenamiento

```cpp
// Creación de streams
Semantic<int>::range(0, 1000);
Semantic<long long>::range(0, 1LL<<60);
Semantic<int>::iterate([](auto yield, auto) {      // stream infinito
    for (int i = 0;; ++i) yield(i);
});
Semantic<int>::of(1, 2, 3, 4, 5);
Semantic<int>::from(std::vector{1,2,3,4});
Semantic<int>::fill(42, 1'000'000);
Semantic<int>::fill([] { return rand(); }, 1000);

// Operaciones intermedias (100 % perezosas)
stream.map([](int x) { return x * x; });
stream.filter([](int x) { return x % 2 == 0; });
stream.flatMap([](int x) { return Semantic<int>::range(0, x); });
stream.distinct();                                     // eliminación de duplicados por valor
stream.skip(100);
stream.limit(50);
stream.takeWhile([](int x) { return x < 1000; });
stream.dropWhile([](int x) { return x < 100; });
stream.peek([](int x) { std::cout << x << ' '; });

// Transformaciones de índice (solo tienen efecto con toOrdered())
stream.redirect([](auto&, auto i) { return -i; });           // invertir
stream.redirect([](auto&, auto i) { return i % 100; });      // cíclico
stream.redirect([](auto e, auto i) { return std::hash<int>{}(e); }); // mezclar
stream.reverse();                                            // equivalente a redirect(-i)
stream.shuffle();

// Paralelismo
stream.parallel();       // todos los núcleos
stream.parallel(8);      // número fijo de hilos

// Obligatorio: elegir explícitamente si se respeta el índice
auto ordered   = stream.toOrdered();     // todas las operaciones de índice tienen efecto
auto unordered = stream.toUnordered();   // todas las operaciones de índice se ignoran (más rápido)

// Operaciones terminales ordenadas (el índice determina el orden final)
ordered.toVector();
ordered.toList();
ordered.toSet();                 // deduplicar según índice final
ordered.forEach([](int x) { std::cout << x << ' '; });
ordered.cout();                  // [99, 98, 97, …]

// Operaciones terminales sin orden (velocidad máxima)
unordered.toVector();
unordered.toList();
unordered.toSet();
unordered.forEach(...);
unordered.cout();

// Estadísticas (siempre por el camino rápido)
auto stats = stream.toUnordered().toStatistics();

// Reduce (recomendado con unordered)
int suma = stream.toUnordered()
    .reduce(0, [](int a, int b) { return a + b; });
```

## Resumen de métodos

| Método                          | Descripción                                       | ¿Respeta operaciones de índice? |
|---------------------------------|---------------------------------------------------|---------------------------------|
| `toOrdered()`                   | Modo semántico – todas las operaciones de índice tienen efecto | Sí                              |
| `toUnordered()`                 | Modo rendimiento – todas las operaciones de índice se ignoran | No (camino más rápido)          |
| `toVector()` / `toList()`       | Recolectar en contenedor                         | depende                         |
| `toSet()`                       | Recolectar en set (sin duplicados)               | depende                         |
| `forEach` / `cout`              | Recorrer o imprimir                              | depende                         |
| `redirect` / `reverse` / `shuffle` | Transformar el índice lógico                | solo con toOrdered()            |
| `parallel`                      | Ejecución paralela                               | en ambos modos                  |

## ¿Por qué hay que elegir explícitamente toOrdered / toUnordered?

Porque **transformación de índice** y **ejecución paralela** son dos dimensiones completamente ortogonales:

- `redirect` etc. son operaciones semánticas (¿dónde debe aparecer lógicamente el elemento?)  
- `parallel` es solo estrategia de ejecución (¿cuánto de rápido lo calculamos?)

Solo con la elección explícita al recolectar se consigue tanto la máxima velocidad como el control preciso del orden.

## Requisitos de compilación

- C++17 o superior  
- Solo `#include "semantic.h"`  
- Cero dependencias externas  
- Única cabecera

```bash
g++ -std=c++17 -O3 -pthread semantic.cpp
```

## Licencia

Licencia MIT – puede usarse sin restricciones en proyectos comerciales, de código abierto y privados.

---

**semantic-cpp**:  
**El índice decide el orden – la ejecución solo decide la velocidad.**  
Escribe `redirect` y realmente invierte. Escribe `toUnordered()` y realmente es lo más rápido.  
Sin compromisos – solo decisiones claras.

Escribe streams sin tener que adivinar.  
Solo di claramente: **«Quiero semántica – o quiero pura velocidad.»**
