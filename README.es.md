# semantic-cpp – Una biblioteca moderna de streams en C++ con semántica temporal

***
**Guiado por algunos comentarios**  
***

**semantic-cpp** es una biblioteca ligera y de alto rendimiento para procesamiento de streams en C++17, compuesta por un único archivo de cabecera y un archivo de implementación separado. Combina la fluidez de Java Streams, la pereza de los generadores de JavaScript, el mapeo de orden de los índices de bases de datos y la conciencia temporal necesaria para aplicaciones financieras, IoT y sistemas orientados a eventos.

Ideas clave que la hacen única:

- Cada elemento lleva un **Timestamp** (índice `long long` con signo que puede ser negativo).
- **Module** es un `unsigned long long` sin signo utilizado para conteos y niveles de concurrencia.
- Los streams son perezosos hasta que se materializan con `.toOrdered()`, `.toUnordered()`, `.toWindow()` o `.toStatistics()`.
- Tras la materialización se puede continuar encadenando operaciones — la biblioteca soporta deliberadamente streams “post-terminales”.
- Soporte completo para ejecución paralela, ventanas deslizantes y rodantes, y operaciones estadísticas avanzadas.

## ¿Por qué semantic-cpp?

| Característica                       | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                   |
|--------------------------------------|-------------|-------------|-----------|--------------|------------------------------------------------|
| Evaluación perezosa                  | Sí          | Sí          | Sí        | No           | Sí                                             |
| Índices temporales (timestamps con signo) | No      | No          | No        | No           | Sí (concepto central)                          |
| Ventanas deslizantes / rodantes      | No          | No          | No        | No           | Sí (soporte de primera clase)                  |
| Operaciones estadísticas integradas  | No          | No          | No        | No           | Sí (media, mediana, moda, curtosis, …)         |
| Ejecución paralela (opt-in)          | Sí          | No          | Sí        | No           | Sí (pool global o personalizado)               |
| Continuar encadenando tras operación terminal | No  | No          | No        | No           | Sí (streams post-terminales)                   |
| Cabecera única + archivo impl., C++17 | No          | Sí          | Sí        | Sí           | Sí                                             |

Si alguna vez te has encontrado escribiendo el mismo código de ventanas o estadísticas para datos de mercado, streams de sensores o análisis de logs, semantic-cpp elimina ese código repetitivo.

## Inicio rápido

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // Crea un stream de 100 números con timestamps 0..99
    auto stream = Generative<int>{}.of(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream.filter([](int x) { return x % 2 == 0; })   // solo números pares
          .parallel(8)                               // usa 8 hilos
          .toWindow()                                // materializa con soporte para ventanas
          .getSlidingWindows(10, 5)                  // ventanas de tamaño 10, paso 5
          .toVector();

    // Ejemplo de estadísticas
    auto stats = Generative<int>{}.of(1,2,3,4,5,6,7,8,9,10).toStatistics();
    std::cout << stats.mean()   << "\n";   // 5.5
    std::cout << stats.median() << "\n";   // 5.5
    std::cout << stats.mode()   << "\n";   // en multimodalidad devuelve el primero encontrado
    stats.cout();

    return 0;
}
```

## Conceptos principales

### 1. Generative<E> – Fábrica de streams

`Generative<E>` ofrece una interfaz cómoda para crear streams:

```cpp
Generative<int> gen;
auto s = gen.of(1, 2, 3, 4, 5);
auto empty = gen.empty();
auto filled = gen.fill(42, 1'000'000);
auto ranged = gen.range(0, 100, 5);
```

Todas las funciones de fábrica devuelven un stream perezoso `Semantic<E>`.

### 2. Semantic<E> – El stream perezoso

```cpp
Semantic<int> s = of<int>(1, 2, 3, 4, 5);
```

Operaciones clásicas disponibles:

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

Los elementos llevan timestamps que se pueden manipular:

```cpp
s.translate(+1000)                                      // desplaza todos los timestamps
 .redirect([](int x, Timestamp t){ return x * 10; })   // lógica personalizada de timestamp
```

### 3. Materialización

Antes de usar operaciones de recolección (count(), toVector(), cout(), …) debe llamarse a uno de los cuatro convertidores terminales:

```cpp
.toOrdered()      // conserva el orden, permite ordenación
.toUnordered()    // la más rápida, sin garantía de orden
.toWindow()       // ordenado + API completa de ventanas
.toStatistics<D>  // ordenado + métodos estadísticos
```

Tras la materialización se puede seguir encadenando:

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 4. Operaciones con ventanas

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // tamaño 30, paso 10
```

O generar un stream de ventanas:

```cpp
stream.toWindow()
    .windowStream(50, 20)           // emite std::vector<E> por cada ventana
    .map([](const std::vector<double>& w) { return mean(w); })
    .toOrdered()
    .cout();
```

### 5. Estadísticas

```cpp
auto stats = from(prices).toStatistics<double>([](auto p){ return p; });

std::cout << "Media     : " << stats.mean()      << "\n";
std::cout << "Mediana   : " << stats.median()    << "\n";
std::cout << "Desv. Est.: " << stats.standardDeviation() << "\n";
std::cout << "Asimetría : " << stats.skewness()  << "\n";
std::cout << "Curtosis  : " << stats.kurtosis()  << "\n";
```

Todas las funciones estadísticas están fuertemente cacheadas (el mapa de frecuencias se calcula solo una vez).

### 6. Paralelismo

```cpp
globalThreadPool   // creado automáticamente con hardware_concurrency hilos
stream.parallel()  // usa el pool global
stream.parallel(12) // fuerza exactamente 12 hilos trabajadores
```

El nivel de concurrencia se hereda correctamente a lo largo de toda la cadena.

## Funciones de fábrica (a través de Generative)

```cpp
empty<T>()                              // stream vacío
of(1,2,3,"hello")                       // argumentos variádicos
fill(42, 1'000'000)                     // valor repetido
fill([]{return rand();}, 1'000'000)     // valores suministrados
from(container)                         // vector, list, set, array, initializer_list, queue
range(0, 100)                           // 0 .. 99
range(0, 100, 5)                        // 0,5,10,…
iterate(generator)                      // generador personalizado
```

## Instalación

semantic-cpp consta de un archivo de cabecera y uno de implementación. Copia simplemente `semantic.h` y `semantic.cpp` en tu proyecto o intégralo vía CMake:

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## Compilar ejemplos

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## Benchmarks (Apple M2 Max, 2024)

| Operación                         | Java Stream | ranges-v3 | semantic-cpp (paralelo) |
|-----------------------------------|-------------|-----------|--------------------------|
| Suma de 100 M enteros             | 280 ms      | 190 ms    | 72 ms                    |
| Media en ventana deslizante de 10 M doubles | N/A | N/A       | 94 ms (ventana 30, paso 10) |
| toStatistics de 50 M enteros      | N/A         | N/A       | 165 ms                   |

## Contribuir

¡Las contribuciones son muy bienvenidas! Áreas que necesitan atención:

- Más colectores (percentiles, covarianza, etc.)
- Mejor integración con bibliotecas de ranges existentes
- Aceleración SIMD opcional para mappers simples

Por favor, lee CONTRIBUTING.md.

## Licencia

MIT © Eloy Kim

¡Disfruta de streams verdaderamente semánticos en C++!
