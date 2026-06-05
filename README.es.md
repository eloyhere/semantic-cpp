# 🚀 Semantic-Cpp: Un marco de procesamiento de flujos inteligente orientado al futuro para C++

Semantic-Cpp es una biblioteca moderna de procesamiento de flujos en C++, rediseñada desde cero, que presenta una arquitectura modular de **"múltiples encabezados, cero dependencias externas"**. Cada archivo de encabezado tiene una responsabilidad clara y única, y es probable de forma independiente; juntos forman un ecosistema completo de procesamiento de flujos. Esta biblioteca combina de manera innovadora la esencia de múltiples paradigmas de programación:

*   **La elegancia y fluidez de Java Stream API**: Llamadas encadenadas, programación declarativa, haciendo que el código sea tan elegante como la poesía ✨
*   **La pereza y flexibilidad de los Generadores de JavaScript**: Evaluación perezosa, generación bajo demanda, amigable con la memoria 🌱
*   **La eficiencia y orden de la indexación de bases de datos**: Clasificación inteligente, impulsada por índices, una herramienta poderosa para el procesamiento de datos de series temporales ⏱️
*   **La filosofía de procesamiento por lotes de "Contenedor-como-Elemento"**: Vectores, listas, mapas... Cualquier contenedor puede ser un ciudadano de primera clase en el flujo, fluyendo libremente 📦

¿Está cansado de escribir bucles `for` para iterar sobre un `vector`, anidar un `if` para filtrar, y llamar manualmente a `push_back` en otro contenedor? 😩
¿Alguna vez ha depurado un error de índice fuera de límites hasta altas horas de la noche, solo porque quería el "tercer elemento desde el final" al iterar hacia atrás? 😵💫
¿Anhela manipular datos como una base de datos: localizar con precisión por índice, analizar con ventanas deslizantes, completar todo el viaje desde los datos hasta las estadísticas con una única cadena de llamadas? 🤔

**Semantic-Cpp nació con este propósito. 🔧**

Abstrae el procesamiento de datos como operaciones sobre "elementos" y sus "posiciones lógicas (índices)", similar a "filas" y "claves primarias" en una base de datos. Puede reorganizar, desplazar e invertir índices libremente sin tocar los datos en sí; también puede pasar cualquier contenedor (`vector`, `map`, `array`...) como un todo indivisible dentro del flujo, y "desempaquetarlo" de nuevo a nivel de elemento en cualquier momento. Esta capacidad de cambiar libremente entre dos granularidades falta en los marcos de flujos tradicionales. 🎯

---

## 🏗️ Arquitectura del Proyecto: Diseño modular de siete capas

Semantic-Cpp consta de siete archivos de encabezado principales, construidos en capas progresivas. Cada archivo tiene una única responsabilidad y es probado de forma independiente. Cinco espacios de nombres, cada uno con su propio ámbito, trabajan juntos para formar una canalización completa desde la fuente de datos hasta el resultado final:

```
┌─────────────────────────────────────────────────┐
│               🌊 semantics.h                     │
│   Espacio de nombres: semantic                  │
│   Fábricas de construcción de flujos: rangos    │
│   numéricos, contenedores, texto, Unicode       │
├─────────────────────────────────────────────────┤
│                 📦 semantic.h                   │
│   Espacio de nombres: semantic / collectable    │
│   Operaciones intermedias de flujo, sistema     │
│   Collectable, soporte de desempaquetado de     │
│   contenedores                                  │
├─────────────────────────────────────────────────┤
│                ⚙️ collector.h                   │
│   Espacio de nombres: collector                 │
│   Marco de colectores + fábricas: coincidencia, │
│   búsqueda, agregación, estadísticas, DFT/FFT  │
├─────────────────────────────────────────────────┤
│                🔤 charsequence.h                │
│   Espacio de nombres: charsequence              │
│   Secuencias de caracteres Unicode, conversión  │
│   multi-codificación, Builder, Buffer          │
├─────────────────────────────────────────────────┤
│                🧵 pool.h                        │
│   Espacio de nombres: pool                      │
│   Grupo de hilos global: envío de tareas,       │
│   apagado de emergencia, propagación de         │
│   excepciones                                   │
├─────────────────────────────────────────────────┤
│                📄 function.h                    │
│   Espacio de nombres: function                  │
│   Definiciones de tipos: alias para Generator,  │
│   Supplier, Consumer, etc.                     │
├─────────────────────────────────────────────────┤
│               🔐 hash.h / less.h                │
│   Espacio de nombres: std (extensiones)         │
│   Especializaciones de Hash y Comparación de    │
│   contenedores de biblioteca estándar, soporte  │
│   de anidamiento arbitrario                    │
└─────────────────────────────────────────────────┘
```

### 🧩 Gráfico de dependencias
La cadena de dependencias es clara y lógica, como un diagrama de circuitos diseñado meticulosamente: la corriente fluye desde las definiciones de tipo fundamentales hacia arriba, donde cada capa depende solo de las capas inferiores. Finalmente, todas las rutas convergen en `semantic.h` y `semantics.h`, formando la capacidad completa de procesamiento de flujos.

```
function.h          ← Sin dependencias, la base de tipos
pool.h              ← Depende de function.h
charsequence.h      ← Módulo independiente, procesamiento Unicode
collector.h         ← Depende de function.h, pool.h
hash.h / less.h     ← Módulos independientes, extensiones de biblioteca estándar
semantic.h          ← Depende de todos los anteriores
semantics.h         ← Depende de semantic.h
```

---

## 🌍 Vista general de espacios de nombres

Semantic-Cpp diseña meticulosamente cinco espacios de nombres, cada uno como un "departamento" independiente, con responsabilidades distintas pero colaborando estrechamente:

| Espacio de nombres | Archivo de encabezado | Responsabilidad                               | Tipos/Funciones principales                                                  |
| :----------------- | :-------------------- | :------------------------------------------- | :-------------------------------------------------------------------------- |
| function           | function.h            | Base del sistema de tipos                    | `Timestamp`, `Module`, `Generator<T>`, `Supplier<R>`, `Consumer<T>`, `Predicate<T>` etc. |
| pool               | pool.h                | Motor de ejecución concurrente               | `pool::pool` (grupo de hilos global), `submit()`, `emergencyShutdown()`    |
| charsequence       | charsequence.h        | Procesamiento de cadenas Unicode             | `charset`, `Meta`, `Point`, `Charsequence`, `Builder`, `Buffer` etc.       |
| collector          | collector.h           | Ejecución de colección terminal              | `Collector<E,A,R>`, `Identity<A>`, `Accumulator<A,E>` etc.                 |
| collectable        | semantic.h            | Contenedores de datos materializados         | `Collectable<E>`, `OrderedCollectable<E>`, `UnorderedCollectable<E>` etc.  |
| semantic           | semantic.h<br>semantics.h | Construcción de flujo y operaciones intermedias | `Semantic<E>`, `useRange()`, `useFrom()` etc.                              |

### 🔁 Flujo de colaboración entre espacios de nombres
El flujo de datos entre espacios de nombres es como una línea de montaje en una fábrica: la materia prima entra desde `semantic`, pasa por procesamiento capa por capa, y finalmente se empaqueta y envía desde `collector`. Cada paso tiene un límite de responsabilidad claro:

```cpp
semantic::useRange(0, 100)          // ← espacio de nombres semantic: crear flujo
    .map(int x { return x * 2; })   // ← espacio de nombres semantic: transformación intermedia
    .filter(int x { return x > 50; }) // ← espacio de nombres semantic: filtro intermedio
    .toUnordered()                  // ← Convertir al espacio de nombres collectable
    .toVector();                    // ← Invocar colector del espacio de nombres collector
```

---

## 📦 Capa 1: function.h — Base de tipos

`function.h` define el sistema de tipos para todo el marco, la base común para todos los módulos. 🔑

```cpp
namespace function {
    using Timestamp = long long;           // Tipo de índice, la "marca de tiempo" de los datos en el flujo
    using Module = unsigned long long;     // Tipo de módulo/conteo
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept — recibir un elemento
        std::function<bool(T, Timestamp)>       // interrupt — ¿deberíamos detenernos?
    )>;
}
```

`Generator` es la abstracción central de todo el sistema de flujos. 🌀 No devuelve datos; en cambio, acepta dos callbacks: `accept` ("Estoy listo, por favor acepta este elemento") e `interrupt` ("¿deberíamos detenernos?"). Este diseño de inversión de control significa que el productor de datos no tiene conocimiento del consumidor; simplemente "empuja" datos en el momento apropiado. Esta es la esencia de la evaluación perezosa: los datos solo "fluyen" realmente cuando se llama a `accept`; antes de eso, todo es meramente una descripción.

| Alias de tipo     | Definición completa                               | Propósito                                |
| :---------------- | :------------------------------------------------ | :--------------------------------------- |
| Timestamp         | long long                                         | Posición lógica de un elemento en el flujo |
| Module            | unsigned long long                                | Conteo, capacidad, nivel de concurrencia |
| Runnable          | std::function<void()>                             | Tarea sin parámetros que devuelve void   |
| Supplier<R>       | std::function<R()>                                | Proveedor, crea a partir de la nada      |
| Function<T,R>     | std::function<R(T)>                               | Función de un solo argumento             |
| BiFunction<T,U,R> | std::function<R(T,U)>                             | Función de dos argumentos                |
| TriFunction<T,U,V,R> | std::function<R(T,U,V)>                        | Función de tres argumentos               |
| Unary<T>          | std::function<T(T)>                               | Operación unaria                         |
| Binary<T>         | std::function<T(T,T)>                             | Operación binaria                        |
| Consumer<T>       | std::function<void(T)>                            | Consumidor                               |
| BiConsumer<T,U>   | std::function<void(T,U)>                          | Consumidor de dos argumentos             |
| TriConsumer<T,U,V>| std::function<void(T,U,V)>                        | Consumidor de tres argumentos            |
| Predicate<T>      | std::function<bool(T)>                            | Juicio de predicado                      |
| BiPredicate<T,U>  | std::function<bool(T,U)>                          | Predicado de dos argumentos              |
| TriPredicate<T,U,V>| std::function<bool(T,U,V)>                       | Predicado de tres argumentos             |
| Comparator<T>     | std::function<int(const T&,const T&)>             | Comparador, devuelve negativo/cero/positivo |
| Generator<T>      | BiConsumer<BiConsumer<T,Timestamp>,<br>BiPredicate<T,Timestamp>> | Abstracción central para generación de flujos |

---

## 🧵 Capa 2: pool.h — Base de concurrencia

`pool.h` proporciona el grupo de hilos global `pool::pool`, el motor de concurrencia para todo el marco. 🚀 Emplea un diseño de **paralelismo declarativo**; cuando escribe `.parallel(4)`, no lanza inmediatamente cuatro hilos para comenzar el procesamiento. Esta línea de código es meramente una "declaración": le dice al marco que "tengo la intención de usar 4 hilos para el procesamiento paralelo". La ejecución paralela real ocurre cuando se invoca una operación terminal; es decir, cuando llama a métodos de colección como `toVector()`, `findFirst()`, `count()`, etc.

| Característica        | Descripción                                                               |
| :-------------------- | :------------------------------------------------------------------------ |
| Paralelismo declarativo | `.parallel(4)` solo declara "quiero usar 4 hilos", no comienza inmediatamente |
| Apagado de emergencia  | Manejador incorporado `emergencyShutdown()` y `std::set_terminate`       |
| Propagación de excepciones | `submit()` devuelve `std::future`, propagando excepciones de manera segura al hilo principal |

---

## 🔤 Capa 3: charsequence.h — Secuencias de caracteres Unicode

`charsequence.h` es un módulo completo de procesamiento Unicode, que proporciona funcionalidad para crear, convertir y manipular secuencias de caracteres. 🌍 Admite varias codificaciones como UTF-8, UTF-16 (LE/BE), UTF-32 (LE/BE), ASCII y Latin1. Detecta y maneja correctamente pares sustitutos, devolviendo el carácter de reemplazo estándar U+FFFD para puntos de código no válidos.

| Tipo/Función    | Descripción                                                               |
| :-------------- | :------------------------------------------------------------------------ |
| charset         | Enum: ascii, utf8, utf16, utf16be, utf16le, utf32, utf32be, utf32le, latin1 |
| Meta            | Envoltura de metadatos, almacena valores enteros sin signo                |
| Point           | Punto de código Unicode, admite detección de pares sustitutos y comprobaciones de validez |
| Charsequence    | Secuencia de caracteres inmutable: split, replace, indexOf, lastIndexOf, sub, trim, toUpperCase, toLowerCase, reverse, startsWith, endsWith, contains, compare, getBytes, getPoints, getMetas, getCharacters, repeat, concat, count, join |
| Builder         | Constructor de bytes mutable: prepend, insert, append (admite tipos primitivos, Point, Charsequence, string_view) |
| Buffer          | Búfer circular seguro para hilos: write, read, peek, prepend, append, clear, shrinkToFit, data, size, capacity, atomic |
| PointIterator   | Iterador bidireccional para recorrer puntos de código Unicode             |
| encode()        | Codifica un solo punto de código a una secuencia de bytes de la codificación especificada |
| decode()        | Decodifica el siguiente punto de código desde una secuencia de bytes, avanza el puntero automáticamente |
| convert()       | Conversión de codificación (admite salida string, vector, deque)         |

---

## ⚙️ Capa 4: collector.h — Marco de colectores y fábricas

`collector.h` es el módulo central de colectores de Semantic-Cpp, que combina el marco de colectores con funciones de fábrica.

### 🧩 Modelo de cinco etapas
```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (cortocircuito opcional)
```

| Alias de tipo    | Definición completa                         | Rol                              |
| :--------------- | :------------------------------------------ | :------------------------------- |
| Identity<A>      | function::Supplier<A>                       | Proporciona valor inicial        |
| Accumulator<A,E> | function::TriFunction<A, E, Timestamp, A>   | Acumula elementos                |
| Combiner<A>      | function::BiFunction<A, A, A>               | Combina resultados paralelos     |
| Finisher<A,R>    | function::Function<A, R>                    | Transformación final             |
| Interrupt<E,A>   | function::TriPredicate<E, Timestamp, A>     | Juicio de cortocircuito          |

### Funciones de fábrica de colectores

#### ✅ Operaciones de coincidencia
| Método                   | Descripción                | Tipo de retorno |
| :----------------------- | :------------------------- | :-------------- |
| `useAllMatch(predicate)` | Todos los elementos cumplen la condición | `bool`      |
| `useAnyMatch(predicate)` | Algún elemento cumple la condición | `bool`      |
| `useNoneMatch(predicate)`| Ningún elemento cumple la condición | `bool`      |

#### 🔍 Operaciones de búsqueda
| Método             | Descripción                      | Tipo de retorno      |
| :----------------- | :------------------------------- | :------------------- |
| `useFindFirst()`   | Encuentra el primer elemento     | `std::optional<E>`   |
| `useFindLast()`    | Encuentra el último elemento     | `std::optional<E>`   |
| `useFindAny()`     | Encuentra cualquier elemento (aleatorio) | `std::optional<E>` |
| `useFindAt(index)` | Localiza en índice específico (admite negativo) | `std::optional<E>` |
| `useFindMaximum()` | Encuentra elemento máximo        | `std::optional<E>`   |
| `useFindMinimum()` | Encuentra elemento mínimo        | `std::optional<E>`   |

#### 🔢 Operaciones de agregación
| Método            | Descripción  | Tipo de retorno |
| :---------------- | :----------- | :-------------- |
| `useCount()`      | Número total de elementos | `Module` |
| `useSummate<E,D>()`| Sumatoria   | `D`             |
| `useAverage<E,D>()`| Promedio    | `D`             |
| `useRange<E,D>()` | Rango numérico (max - min) | `D` |

#### 📉 Operaciones estadísticas
| Método                        | Descripción                | Tipo de retorno        |
| :---------------------------- | :------------------------- | :--------------------- |
| `useVariance<E,D>()`          | Varianza de la población   | `D`                    |
| `useStandardDeviation<E,D>()` | Desviación estándar de la población | `D`         |
| `useSkewness<E,D>()`          | Asimetría                  | `D`                    |
| `useKurtosis<E,D>()`          | Curtosis                   | `D`                    |
| `useMedian<E,D>()`            | Mediana                    | `std::optional<D>`     |
| `useMode<E>()`                | Moda (análisis de frecuencia) | `std::optional<E>`  |
| `usePercentile<E,D>(p)`       | Percentil p-ésimo         | `std::optional<D>`     |
| `useFrequency<E>()`           | Características del dominio de frecuencia | `std::map<E, complex>` |
| `useDistribution<E>()`        | Características de distribución espacial | `std::map<E, complex>` |

#### 🔀 Operaciones de reducción
| Método                          | Descripción          | Tipo de retorno      |
| :------------------------------ | :------------------- | :------------------- |
| `useReduce(reducer)`            | Reducción sin identidad | `std::optional<E>` |
| `useReduce(identity, reducer)`  | Reducción con identidad | `E`               |
| `useReduce(id, red, comb, fin)` | Reducción completamente personalizada | `R` |

#### 🧺 Colección en contenedores
| Método                                                              | Tipo de retorno |
| :------------------------------------------------------------------ | :-------------- |
| `useToVector()`                                                     | `std::vector<E>` |
| `useToList()`                                                       | `std::list<E>` |
| `useToDeque()`                                                      | `std::deque<E>` |
| `useToForwardList()`                                                | `std::forward_list<E>` |
| `useToArray<N>()`                                                   | `std::array<E, N>` |
| `useToSet()`                                                        | `std::set<E>` |
| `useToMultiset()`                                                   | `std::multiset<E>` |
| `useToUnorderedSet()`                                               | `std::unordered_set<E>` |
| `useToUnorderedMultiset()`                                          | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)`                                            | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)`                            | `std::map<K, V>` |
| `useToMultimap(keyExtractor)`                                       | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)`                       | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)`                   | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)`                              | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)`              | `std::unordered_multimap<K, V>` |
| `useToStack()`                                                      | `std::stack<E>` |
| `useToQueue()`                                                      | `std::queue<E>` |
| `useToPriorityQueue()`                                              | `std::priority_queue<E>` |

#### 🧩 Operaciones de agrupación y partición
| Método                                          | Tipo de retorno |
| :---------------------------------------------- | :-------------- |
| `useGroup(keyExtractor)`                        | `std::unordered_map<K, vector<E>>` |
| `useGroupBy(keyExtractor, valueExtractor)`     | `std::unordered_map<K, vector<V>>` |
| `usePartition(size)`                            | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)`                  | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor, valueExtractor)`  | `std::vector<vector<V>>` |

#### 📄 Salida de cadenas
| Método                               | Tipo de retorno |
| :----------------------------------- | :-------------- |
| `useJoin()` / `useOut()` / `useError()` y sus sobrecargas | `charsequence::Charsequence` |

#### 📊 Herramientas matemáticas
| Método            | Tipo de retorno |
| :---------------- | :-------------- |
| `useDFT()`        | `vector<complex<double>>` |
| `useIDFT()`       | `vector<complex<double>>` |
| `useFFT()`        | `vector<complex<double>>` |
| `useIFFT()`       | `vector<complex<double>>` |
| `useGradient()`   | `vector<double>` |

---

## 📦 Capa 5: semantic.h — Operaciones intermedias de flujo y sistema de colección

### 🧩 Diseño central: Canalización de tres etapas
```
Semantic<E> (Construcción y Transformación)
    ↓ toUnordered() / toOrdered() / sort() / toWindow() / toStatistics()
Collectable<E> (Materialización y Colección)
    ↓ toVector() / findFirst() / count() / summate() / ...
Resultado final
```

**Regla importante**: Un `Semantic<E>` primero debe convertirse en un `Collectable<E>` a través de `toUnordered()`, `toOrdered()`, `toWindow()`, `toStatistics()` o `sort()` antes de que se puedan llamar métodos terminales.

### 🧭 Cinco caminos de materialización
| Método de conversión   | Tipo objetivo          | Estructura de datos subyacente | Característica de rendimiento  |
| :--------------------- | :--------------------- | :---------------------------- | :----------------------------- |
| `toUnordered()`        | `UnorderedCollectable` | `unordered_map`               | Búsqueda promedio O(1)         |
| `toOrdered()`          | `OrderedCollectable`   | `map`                         | Búsqueda O(log n)              |
| `sort()`               | `OrderedCollectable`   | `map` (ordenado por valor)    | Búsqueda O(log n)              |
| `toWindow()`           | `WindowCollectable`    | Hereda colección ordenada     | Admite slide/tumble            |
| `toStatistics<D>()`    | `Statistics<E,D>`      | Hereda colección ordenada     | 30+ métodos estadísticos       |

### 📋 Collectable<E> — Todos los métodos terminales (orden alfabético)
| Método                                              | Tipo de retorno             | Descripción                                     |
| :-------------------------------------------------- | :------------------------- | :--------------------------------------------- |
| `allMatch(predicate)`                               | `bool`                     | Todos los elementos cumplen la condición        |
| `anyMatch(predicate)`                               | `bool`                     | Algún elemento cumple la condición             |
| `average<D>()`                                      | `D`                        | Promedio                                       |
| `average<D>(mapper)`                                | `D`                        | Promedio después del mapeo                     |
| `collect(identity, acc, comb, fin)`                | `R`                        | Colección personalizada de cuatro etapas       |
| `collect(identity, interrupt, acc, comb, fin)`      | `R`                        | Colección interrumpible personalizada          |
| `count()`                                           | `Module`                   | Número total de elementos                      |
| `empty()`                                           | `bool`                     | ¿Está el flujo vacío?                          |
| `error()`                                           | `void`                     | Salida a stderr (admite delimiter/prefix/suffix/converter) |
| `findAny()`                                         | `std::optional<E>`         | Encuentra cualquier elemento (aleatorio)       |
| `findAt(index)`                                     | `std::optional<E>`         | Encuentra elemento en índice específico (admite negativo) |
| `findFirst()`                                       | `std::optional<E>`         | Encuentra el primer elemento                   |
| `findLast()`                                        | `std::optional<E>`         | Encuentra el último elemento                   |
| `findMaximum()`                                     | `std::optional<E>`         | Encuentra el elemento máximo                   |
| `findMaximum(comparator)`                           | `std::optional<E>`         | Encuentra máximo con comparador personalizado  |
| `findMinimum()`                                     | `std::optional<E>`         | Encuentra el elemento mínimo                   |
| `findMinimum(comparator)`                           | `std::optional<E>`         | Encuentra mínimo con comparador personalizado  |
| `forEach(consumer)`                                 | `void`                     | Ejecuta efecto secundario para cada elemento   |
| `group(keyExtractor)`                               | `unordered_map<K, vector<E>>` | Agrupa por clave                             |
| `groupBy(keyExtractor, valueExtractor)`            | `unordered_map<K, vector<V>>` | Agrupa por clave y extrae valor              |
| `join()`                                            | `Charsequence`              | Une con formato predeterminado                 |
| `join(delimiter)`                                   | `Charsequence`              | Une con delimitador personalizado              |
| `join(prefix, delimiter, suffix)`                  | `Charsequence`              | Une con formato completamente personalizado    |
| `noneMatch(predicate)`                              | `bool`                     | Ningún elemento cumple la condición            |
| `out()`                                             | `Charsequence`              | Salida a stdout (admite delimiter/prefix/suffix/converter) |
| `partition(size)`                                   | `vector<vector<E>>`         | Particiona por tamaño fijo                     |
| `partitionBy(keyExtractor)`                         | `vector<vector<E>>`         | Particiona por clave de índice                 |
| `partitionBy(keyExtractor, valueExtractor)`         | `vector<vector<V>>`         | Particiona por clave de índice y extrae valor  |
| `range<D>()`                                        | `D`                        | Rango numérico (max - min)                     |
| `range<D>(mapper)`                                  | `D`                        | Rango numérico después del mapeo               |
| `reduce(accumulator)`                               | `std::optional<E>`         | Reducción sin identidad                        |
| `reduce(identity, accumulator)`                     | `E`                        | Reducción con identidad                        |
| `reduce(identity, acc, comb)`                       | `R`                        | Reducción completamente personalizada          |
| `summate<D>()`                                      | `D`                        | Sumatoria                                      |
| `summate<D>(mapper)`                                | `D`                        | Sumatoria después del mapeo                    |
| `toArray<N>()`                                      | `std::array<E, N>`         | Recoge en array de tamaño fijo                 |
| `toDeque()`                                         | `std::deque<E>`            | Recoge en deque                                |
| `toForwardList()`                                   | `std::forward_list<E>`     | Recoge en forward_list                         |
| `toList()`                                          | `std::list<E>`             | Recoge en list                                 |
| `toMap(keyExtractor)`                               | `std::map<K, E>`           | Recoge en map por clave                        |
| `toMap(keyExtractor, valueExtractor)`               | `std::map<K, V>`           | Recoge en map con clave y valor personalizados |
| `toMultimap(keyExtractor)`                          | `std::multimap<K, E>`      | Recoge en multimap por clave                   |
| `toMultimap(keyExtractor, valueExtractor)`          | `std::multimap<K, V>`      | Recoge en multimap con clave y valor personalizados |
| `toMultiset()`                                      | `std::multiset<E>`         | Recoge en multiset                             |
| `toPriorityQueue()`                                 | `std::priority_queue<E>`   | Recoge en priority_queue                       |
| `toQueue()`                                         | `std::queue<E>`            | Recoge en queue                                |
| `toSet()`                                           | `std::set<E>`              | Recoge en set (único y ordenado)               |
| `toStack()`                                         | `std::stack<E>`            | Recoge en stack                                |
| `toUnorderedMap(keyExtractor, valueExtractor)`      | `std::unordered_map<K, V>` | Recoge en unordered_map                        |
| `toUnorderedMultimap(keyExtractor)`                 | `std::unordered_multimap<K, E>`| Recoge en unordered_multimap por clave     |
| `toUnorderedMultimap(keyExtractor, valueExtractor)` | `std::unordered_multimap<K, V>`| Recoge en unordered_multimap con clave y valor personalizados |
| `toUnorderedMultiset()`                             | `std::unordered_multiset<E>`| Recoge en unordered_multiset                 |
| `toUnorderedSet()`                                  | `std::unordered_set<E>`    | Recoge en unordered_set                        |
| `toVector()`                                        | `std::vector<E>`           | Recoge en vector                               |

### 📈 Statistics<E,D> — Métodos estadísticos
| Método               | Tipo de retorno          | Descripción                     |
| :------------------- | :---------------------- | :------------------------------ |
| `summate()`          | `D`                     | Sumatoria                       |
| `average()`          | `D`                     | Promedio                        |
| `minimum()`          | `std::optional<D>`     | Valor mínimo                    |
| `maximum()`          | `std::optional<D>`     | Valor máximo                    |
| `range()`            | `D`                     | Rango (max - min)               |
| `variance()`         | `D`                     | Varianza de la población        |
| `standardDeviation()`| `D`                     | Desviación estándar de la población |
| `median()`           | `std::optional<D>`     | Mediana                         |
| `mode()`             | `std::optional<E>`     | Moda                            |
| `percentile(p)`      | `std::optional<D>`     | Percentil p-ésimo              |
| `firstQuartile()`    | `std::optional<D>`     | Primer cuartil (Q1)             |
| `thirdQuartile()`    | `std::optional<D>`     | Tercer cuartil (Q3)             |
| `interquartileRange()`| `std::optional<D>`    | Rango intercuartílico (IQR)     |
| `skewness()`         | `D`                     | Asimetría                       |
| `kurtosis()`         | `D`                     | Curtosis                        |
| `frequency()`        | `map<E, complex>`      | Características del dominio de frecuencia |
| `distribute()`       | `map<E, complex>`      | Características de distribución espacial |
| `dft()`              | `vector<complex<double>>` | Transformada Discreta de Fourier |
| `idft()`             | `vector<complex<double>>` | Transformada Discreta Inversa de Fourier |
| `fft()`              | `vector<complex<double>>` | Transformada Rápida de Fourier |
| `ifft()`             | `vector<complex<double>>` | Transformada Rápida Inversa de Fourier |
| `gradient(...)`      | `vector<double>`       | Descenso de gradiente           |

Todos los métodos anteriores también admiten una versión de parámetro `mapper` opcional.

### 🔧 Métodos de operaciones intermedias de Semantic<E>
| Categoría      | Método        | Descripción                                   |
| :------------ | :------------ | :------------------------------------------- |
| Transformación de elementos | map         | Transformación de mapeo uno a uno            |
|               | flatMap     | Mapeo uno a muchos y aplanamiento            |
|               | flat        | Aplanar flujos anidados (admite Semantic y contenedores) |
| Filtro de elementos | filter      | Filtrado condicional                         |
|               | takeWhile   | Tomar mientras se cumple la condición        |
|               | dropWhile   | Descartar mientras se cumple la condición    |
|               | distinct    | Eliminar duplicados (admite comparador personalizado) |
| Control de tamaño | limit       | Limitar número de elementos                  |
|               | skip        | Omitir primeros n elementos                  |
|               | sub         | Extraer subrango [start, end)                |
| Operaciones de índice | redirect    | Reasignar índices                            |
|               | reverse     | Invertir índices                             |
|               | translate   | Desplazar índices                            |
| Observación   | peek        | Observar cada elemento (no modifica el flujo) |
| Declaración paralela | parallel(n) | Declarar nivel de concurrencia               |
| Concatenación | concatenate | Concatenar Semantic/elementos/generadores/contenedores |
| Conversión terminal | toUnordered / toOrdered / toWindow / toStatistics / sort | Convertir a Collectable |

---

## 🔧 Capa 6: semantics.h — Fábricas de construcción de flujos

### 🔢 Generación de rangos numéricos
| Método                        | Descripción                     |
| :---------------------------- | :------------------------------ |
| `useRange(start, end)`       | Generar rango [start, end)      |
| `useRange(start, end, step)` | Rango con tamaño de paso (admite negativo) |
| `useRangeClosed(start, end)` | Generar rango cerrado [start, end] |
| `useRangeClosed(start, end, step)` | Rango cerrado con tamaño de paso |

### ♾️ Generación de flujos infinitos
| Método                      | Descripción                         |
| :-------------------------- | :--------------------------------- |
| `useInfinite(seed, generator)`| Iteración infinita desde valor inicial |
| `useGenerate(supplier)`     | Llamadas infinitas al proveedor    |
| `useGenerate(supplier, limit)`| Número limitado de llamadas al proveedor |
| `useIterate(seed, generator)` | Iteración infinita desde valor inicial |
| `useIterate(seed, generator, limit)` | Número limitado de iteraciones |
| `useRandom()`               | Flujo infinito de enteros aleatorios |
| `useRandom(min, max)`       | Flujo de números aleatorios en rango especificado |
| `useRandom(min, max, count)`| Flujo de números aleatorios con rango y número especificados |

### 📦 Construcción de contenedores y elementos
| Método                  | Descripción                     |
| :---------------------- | :----------------------------- |
| `useEmpty()`            | Crear un flujo vacío           |
| `useOf(element)`        | Crear flujo a partir de un solo elemento |
| `useOf(e1, e2)`         | Crear flujo a partir de dos elementos |
| `useOf(e1, e2, e3)`     | Crear flujo a partir de tres elementos |
| `useOf({...})`          | Crear flujo a partir de lista de inicialización |
| `useFrom(container)`    | Crear flujo a partir de contenedor estándar |
| `useFrom({...})`        | Crear flujo a partir de lista de inicialización |
| `useRepeat(element, count)` | Repetir elemento n veces     |

### 📄 Procesamiento de texto y Unicode
| Método                      | Descripción                         |
| :-------------------------- | :--------------------------------- |
| `useBlob(text)`             | Dividir cadena byte a byte en flujo de char |
| `useBlob(text, start, end)` | Dividir rango específico byte a byte |
| `useBlob(istream)`          | Leer línea por línea desde flujo de entrada |
| `useBlob(istream, delimiter)` | Leer por delimitador desde flujo de entrada |
| `useText(text)`             | Flujo de texto completo (Charsequence) |
| `useText(text, delimiter)`  | Dividir texto por delimitador      |
| `useText(istream)`          | Leer contenido completo desde flujo de entrada |
| `useSequence(charsequence)` | Crear flujo de puntos de código a partir de secuencia de caracteres |
| `useSequence(text, encoding)` | Crear flujo de puntos de código a partir de texto con codificación especificada |
| `useCharsequence(charsequence)` | Secuencia de caracteres como flujo completo |
| `useCharsequence(charsequence, delimiter)` | Dividir secuencia de caracteres por delimitador |

---

## 🔐 Capa 7: hash.h / less.h — El lenguaje universal del mundo de los contenedores

Proporciona soporte completo de hash y comparación para todos los contenedores de la biblioteca estándar (incluidos contenedores anidados), `pair`, `tuple`, `optional`, `variant`, tipos de tiempo `chrono`, números `complex`, y más. Los contenedores anidados en cualquier profundidad y combinación ahora pueden usarse como claves en `unordered_set` o elementos en `set`. 🌉

---

## 🚀 Consejos de optimización de rendimiento

1.  **Elija el contenedor correcto**: Use `toUnordered()` si el orden no importa, `toOrdered()` o `sort()` si se necesita ordenación.
2.  **Aproveche el paralelismo**: Use `parallel()` para conjuntos de datos grandes.
3.  **Optimice el orden de las operaciones**: Filtre temprano, ordene sabiamente.
4.  **Utilice la evaluación perezosa**: `takeWhile` y `limit` pueden terminar temprano.

---

Semantic-Cpp — Construyendo canalizaciones de procesamiento de datos eficientes y claras con C++ moderno. 🚀🎯✨
