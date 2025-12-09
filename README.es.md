# semantic-cpp – Una moderna biblioteca de streams en C++ con semántica temporal

**semantic-cpp** es una biblioteca de procesamiento de streams de alto rendimiento, *header-only* y escrita en C++17 que combina la fluidez de Java Streams, la evaluación perezosa de los generadores de JavaScript, el mapeo ordenado de índices de MySQL y la conciencia temporal que requieren los sistemas financieros, IoT y basados en eventos.

Características que la hacen única:

- Cada elemento lleva asociado un **Timestamp** (`long long` con signo, puede ser negativo).  
- **Module** es un entero sin signo utilizado para conteos y niveles de concurrencia.  
- Los streams son completamente *lazy* hasta que se materializan con `.toOrdered()`, `.toUnordered()`, `.toWindow()` o `.toStatistics()`.  
- Después de materializar, se puede seguir encadenando operaciones — la biblioteca está diseñada deliberadamente para ser “post-terminal”.  
- Soporte nativo para ejecución paralela, ventanas deslizantes y de salto (sliding/tumbling windows) y colectores estadísticos muy completos.

## ¿Por qué semantic-cpp?

| Característica                               | Java Stream | Boost.Range | ranges-v3 | spdlog::sink | semantic-cpp                                          |
|---------------------------------------------|-------------|-------------|-----------|--------------|--------------------------------------------------------|
| Evaluación perezosa                         | Yes         | Yes         | Yes       | No           | Yes                                                    |
| Índices temporales (timestamps con signo)  | No          | No          | No        | No           | Yes (concepto central)                                 |
| Ventanas deslizantes / de salto             | No          | No          | No        | No           | Yes (de primera clase)                                 |
| Colector estadístico integrado              | No          | No          | No        | No           | Yes (media, mediana, moda, kurtosis, asimetría…)       |
| Paralelismo por defecto (opt-in)            | Yes         | No          | Yes       | No           | Yes (pool global o personalizado)                      |
| Continuar encadenando tras operación terminal | No          | No          | No        | No           | Yes (streams post-terminal)                            |
| Header-only, C++17                          | No          | Yes         | Yes       | Yes          | Yes                                                    |

Si alguna vez te has encontrado escribiendo una y otra vez código de ventanas y estadísticas para datos de mercado, sensores o análisis de logs, **semantic-cpp** elimina todo ese código repetitivo.

## Inicio rápido

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // Stream de 100 números con timestamps 0..99
    auto stream = from(std::vector<int>(100, 0))
        .map([](int, Timestamp t) { return static_cast<int>(t); });

    stream
        .filter([](int x) { return x % 2 == 0; })      // sólo números pares
        .parallel(8)                                   // 8 hilos
        .toWindow()                                    // materializa y activa funciones de ventana
        .getSlidingWindows(10, 5)                      // ventanas de 10, paso 5
        .toVector();                                   // → std::vector<std::vector<int>>

    // Ejemplo estadístico
    auto stats = from({1,2,3,4,5,6,7,8,9,10})
                  .toStatistics<double>();         // mapper identity inferido automáticamente

    std::cout << "Media          : " << stats.mean()            << '\n';
    std::cout << "Mediana        : " << stats.median()          << '\n';
    std::cout << "Moda           : " << stats.mode()            << '\n';
    std::cout << "Desviación std : " << stats.standardDeviation() << '\n';
    stats.cout();  // salida bonita con un solo llamado

    return 0;
}
```

## Conceptos principales

### 1. `Semantic<E>` – el stream perezoso

```cpp
Semantic<int> s = of(1, 2, 3, 4, 5);
```

Todas las operaciones clásicas están disponibles:

```cpp
s.filter(...).map(...).skip(10).limit(100).parallel().peek(...)
```

Cada elemento se emite junto con un **Timestamp** (índice con signo). Puedes desplazar o redirigir completamente los timestamps:

```cpp
s.translate(+1000)                                           // desplazar todos
   .redirect([](int x, Timestamp t){ return x * 10LL; })     // lógica personalizada
```

### 2. Materialización – el único punto donde “pagas”

Debes invocar uno de los cuatro convertidores terminales antes de usar operaciones de recolección (`count()`, `toVector()`, `cout()`, …):

```cpp
.toOrdered()        // mantiene el orden original, permite ordenación
.toUnordered()      // la más rápida, sin garantías de orden
.toWindow()         // ordenado + potente API de ventanas
.toStatistics<D>()  // ordenado + métodos estadísticos
```

Después de materializar, puedes seguir encadenando:

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 3. Ventanas (Windowing)

```cpp
auto windows = stream
    .toWindow()
    .getSlidingWindows(30, 10);     // ventanas de 30 elementos, paso 10
```

O generar directamente un stream de ventanas:

```cpp
stream.toWindow()
      .windowStream(50, 20)           // emite std::vector<E> por ventana
      .map([](const std::vector<double>& w){ return media(w); })
      .toOrdered()
      .cout();
```

### 4. Estadísticas

```cpp
auto stats = from(precios)
              .toStatistics<double>([](double p){ return p; });

std::cout << "Media          : " << stats.mean()      << "\n";
std::cout << "Mediana        : " << stats.median()    << "\n";
std::cout << "Desviación std : " << stats.standardDeviation() << "\n";
std::cout << "Asimetría      : " << stats.skewness()  << "\n";
std::cout << "Curtosis       : " << stats.kurtosis()  << "\n";
```

Todas las funciones estadísticas están fuertemente cacheadas (el mapa de frecuencias se calcula una sola vez).

### 5. Paralelismo

```cpp
globalThreadPool          // creado automáticamente con hardware_concurrency hilos
stream.parallel()         // usa el pool global
stream.parallel(12)       // fuerza exactamente 12 hilos de trabajo
```

Cada `Collectable` lleva su propio nivel de concurrencia, que se hereda correctamente a lo largo de la cadena.

## Funciones de fábrica

```cpp
empty<T>()                              // stream vacío
of(1,2,3,"hola")                        // desde argumentos variádicos
fill(42, 1'000'000)                     // un millón de 42
fill([]{return rand();}, 1'000'000)     // valores generados
from(contenedor)                        // vector, list, set, array, initializer_list
range(0, 100)                           // 0 .. 99
range(0, 100, 5)                        // 0,5,10,…
iterate(generador_propio)               // desde un Generator personalizado
```

## Instalación

Es *header-only*. Copia `semantic.h` en tu proyecto o intégrala con CMake:

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(tu_objetivo PRIVATE semantic::semantic)
```

## Compilar los ejemplos

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp -o basic
./basic
```

## Benchmarks (Apple M2 Max, 2024)

| Operación                                   | Java Stream | ranges-v3 | semantic-cpp (paralelo) |
|---------------------------------------------|-------------|-----------|--------------------------|
| Suma de 100 M de enteros                    | 280 ms      | 190 ms    | 72 ms                    |
| Media móvil de 10 M doubles (ventana 30, paso 10) | N/A   | N/A       | 94 ms                    |
| 50 M de enteros → toStatistics              | N/A         | N/A       | 165 ms                   |

## Contribuir

¡Las contribuciones son muy bienvenidas! Áreas especialmente necesitadas:

- Más colectores (percentiles, covarianza, etc.)
- Mejor interoperabilidad con otras bibliotecas de ranges
- Aceleración SIMD opcional para mappers simples

Por favor, lee CONTRIBUTING.md.

## Licencia

MIT © Eloy Kim

¡Disfruta de streams verdaderamente semánticos en C++!
