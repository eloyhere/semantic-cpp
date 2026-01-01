# semantic-cpp – Una biblioteca moderna de procesamiento de streams en C++ con semántica temporal

**semantic-cpp** es una biblioteca ligera y de alto rendimiento para el procesamiento de streams en C++17 moderno. Consiste en un único archivo de cabecera (`semantic.h`) acompañado de un archivo de implementación separado (`semantic.cpp`). La biblioteca combina la fluidez de Java Streams, la evaluación perezosa de los generadores y Promises de JavaScript, las capacidades de ordenación reminiscentes de índices de bases de datos, y una conciencia temporal integrada esencial para aplicaciones financieras, procesamiento de datos IoT y sistemas basados en eventos.

## Principales características

- Cada elemento está asociado a un **Timestamp** (un `long long` con signo que soporta valores negativos) y a un **Module** (un `unsigned long long` utilizado para conteos y concurrencia).
- Los streams son evaluados de forma perezosa hasta su materialización mediante `.toOrdered()`, `.toUnordered()`, `.toWindow()` o `.toStatistics()`.
- La materialización **no** termina el pipeline — las llamadas en cadena siguen siendo completamente posibles después («streams post-terminal»).
- Soporte completo para ejecución paralela, ventanas deslizantes y tumbling, operaciones estadísticas avanzadas, y tareas asincrónicas mediante una clase **Promise** inspirada en JavaScript.

## ¿Por qué elegir semantic-cpp?

| Característica                   | Java Stream | ranges-v3 | semantic-cpp                          |
|----------------------------------|-------------|-----------|---------------------------------------|
| Evaluación perezosa              | Sí          | Sí        | Sí                                    |
| Índices temporales (timestamps con signo) | No          | No        | Sí (concepto central)                 |
| Ventanas deslizantes / tumbling  | No          | No        | Sí (soporte nativo)                   |
| Operaciones estadísticas integradas | No          | No        | Sí (media, mediana, moda, asimetría, curtosis, etc.) |
| Ejecución paralela (opt-in)      | Sí          | Sí        | Sí (pool de hilos + soporte Promise)  |
| Encadenamiento tras operación terminal | No          | No        | Sí (streams post-terminal)            |
| Promises asincrónicas            | No          | No        | Sí (estilo JavaScript)                |
| Cabecera única + archivo de implementación, C++17 | No          | Sí        | Sí                                    |

Si frecuentemente escribes código personalizado para ventanas, estadísticas o tareas asincrónicas en series temporales, feeds de mercado, streams de sensores o logs, semantic-cpp elimina ese código repetitivo.

## Inicio rápido

```cpp
#include "semantic.h"
#include <iostream>

using namespace semantic;

int main() {
    // Crear un stream a partir de valores (timestamps auto-incrementados desde 0)
    auto stream = of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    // Filtrar números pares, ejecución paralela, materialización para estadísticas
    auto stats = stream
        .filter([](int x) { return x % 2 == 0; })
        .parallel(8)
        .toStatistics<int>();

    std::cout << "Media: " << stats.mean() << '\n';      // 5
    std::cout << "Mediana: " << stats.median() << '\n';  // 5
    std::cout << "Moda: " << stats.mode() << '\n';      // cualquier par

    // Ejemplo de ventanas
    auto windows = of(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
        .toWindow()
        .getSlidingWindows(5, 2);  // ventanas de tamaño 5, paso 2

    return 0;
}
```

## Conceptos clave

### 1. Funciones fábrica – Creación de streams

Los streams se crean mediante funciones libres en el espacio de nombres `semantic`:

```cpp
auto s = of(1, 2, 3, 4, 5);                          // argumentos variádicos
auto empty = empty<int>();                          // stream vacío
auto filled = fill(42, 1'000'000);                   // valor repetido
auto supplied = fill([]{ return rand(); }, 1'000'000);
auto ranged = range(0, 100);                        // 0 .. 99
auto ranged_step = range(0, 100, 5);                // 0, 5, 10, ...
auto from_vec = from(std::vector<int>{1, 2, 3});
auto iterated = iterate([](auto push, auto stop) {
    for (int i = 0; i < 100; ++i) push(i, i);       // timestamps explícitos
});
```

Funciones auxiliares de E/S adicionales: `lines(stream)`, `chunks(stream, size)`, `text(stream)`, etc.

### 2. Semantic – El stream perezoso

El tipo central `Semantic<E>` soporta las operaciones estándar:

```cpp
stream.filter(...).map(...).skip(10).limit(100).peek(...)
```

Manipulación de timestamps:

```cpp
stream.translate(+1000)          // desplazar todos los timestamps
    .redirect([](auto elem, Timestamp t) { return t * 10; });
```

Paralelismo:

```cpp
stream.parallel()                // número de hilos predeterminado
    .parallel(12);               // especificar número de workers
```

### 3. Materialización

Convertir a forma coleccionable:

- `.toOrdered()` – conserva el orden, habilita ordenación
- `.toUnordered()` – el más rápido, sin orden
- `.toWindow()` – ordenado con soporte completo de ventanas
- `.toStatistics<R>(mapper)` – ordenado con métodos estadísticos

El encadenamiento sigue siendo posible después:

```cpp
auto result = from(huge_vector)
    .parallel()
    .filter(...)
    .toWindow()
    .getSlidingWindows(100, 50)
    .toVector();
```

### 4. Ventanas

```cpp
auto windows = stream.toWindow().getSlidingWindows(30, 10);
auto tumbling = stream.toWindow().getTumblingWindows(50);
```

Emitir streams de ventanas:

```cpp
stream.toWindow()
    .windowStream(50, 20)
    .map([](const std::vector<double>& w) { return mean(w); })
    .cout();
```

### 5. Estadísticas

```cpp
auto stats = from(prices)
    .toStatistics<double>([](auto p) { return p; });

std::cout << "Media:         " << stats.mean() << '\n';
std::cout << "Mediana:       " << stats.median() << '\n';
std::cout << "Desviación estándar: " << stats.standardDeviation() << '\n';
std::cout << "Asimetría:     " << stats.skewness() << '\n';
std::cout << "Curtosis:      " << stats.kurtosis() << '\n';
```

Los resultados se cachean agresivamente para mejorar el rendimiento.

### 6. Ejecución asincrónica con Promises

Inspirada en las Promises de JavaScript, la clase `Promise<T, E>` permite manejar tareas asincrónicas enviadas a un `ThreadPool`:

```cpp
ThreadPool pool(8);

auto promise = pool.submit<int>([] {
    // cálculo pesado
    return 42;
});

promise.then([](int result) {
        std::cout << "Resultado: " << result << '\n';
    })
    .except([](const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    })
    .finally([] {
        std::cout << "Completado\n";
    });

promise.wait();  // bloquear si es necesario
```

Ayudantes estáticos: `Promise<T>::all(...)`, `Promise<T>::any(...)`, `resolved(value)`, `rejected(error)`.

## Instalación

Copia `semantic.h` y `semantic.cpp` en tu proyecto o usa CMake:

```cmake
add_subdirectory(external/semantic-cpp)
target_link_libraries(your_target PRIVATE semantic::semantic)
```

## Compilación de ejemplos

```bash
g++ -std=c++17 -O3 -pthread examples/basic.cpp semantic.cpp -o basic && ./basic
```

## Benchmarks (Apple M2 Max, 2024)

| Operación                          | Java Stream | ranges-v3 | semantic-cpp (paralelo) |
|------------------------------------|-------------|-----------|-------------------------|
| 100 M enteros → suma               | 280 ms      | 190 ms    | **72 ms**               |
| 10 M doubles → media ventana deslizante | N/A         | N/A       | **94 ms** (ventana 30, paso 10) |
| 50 M enteros → toStatistics        | N/A         | N/A       | **165 ms**              |

## Contribuciones

¡Las contribuciones son bienvenidas! Áreas de interés:

- Coleccionadores adicionales (percentiles, covarianza, etc.)
- Mejor integración con bibliotecas de ranges
- Aceleración SIMD opcional

## Licencia

MIT © Eloy Kim

¡Disfruta de streams verdaderamente semánticos en C++!