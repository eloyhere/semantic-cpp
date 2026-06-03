# 🚀 Semantic-Cpp: Un framework C++ de procesamiento de flujos inteligente orientado al futuro

Semantic-Cpp es una biblioteca de procesamiento de flujos C++ moderna completamente rediseñada, construida sobre una arquitectura modular de **"múltiples cabeceras, cero dependencias externas"**. Cada archivo de cabecera tiene responsabilidades claras, es independientemente comprobable y juntos forman un ecosistema completo de procesamiento de flujos. Esta biblioteca integra creativamente la esencia de múltiples paradigmas de programación:

*   **Elegancia y fluidez de la API Java Stream:** Llamadas encadenadas, programación declarativa, haciendo el código elegante como poesía ✨
*   **Perezosidad y flexibilidad de los Generadores de JavaScript:** Evaluación perezosa, generación bajo demanda, amigable con la memoria 🌱
*   **Eficiencia y orden de los índices de base de datos:** Ordenamiento inteligente, impulsado por índices, una herramienta poderosa para el procesamiento de series temporales ⏱️
*   **Filosofía de procesamiento por lotes de contenedor-como-elemento:** Vectores, listas, mapas... Cualquier contenedor puede ser un ciudadano de primera clase en el flujo, fluyendo libremente 📦

¿Está cansado de escribir manualmente bucles `for` para recorrer un `vector`, anidar un `if` para filtrar, y luego hacer `push_back` manualmente a otro contenedor? 😩
¿Ha depurado un error de índice fuera de límites tarde en la noche, solo porque quería el "tercer elemento desde el final" al iterar hacia atrás? 😵💫
¿Anhela operar como una base de datos: ubicación precisa por índice, análisis con ventanas deslizantes, completando el viaje completo desde datos hasta estadísticas con una sola cadena de llamadas? 🤔

Semantic-Cpp nació para este propósito. 🔧
Abstrae el procesamiento de datos como operaciones sobre **"elementos"** y sus **"posiciones lógicas (índices)"** — similar a "filas" y "claves primarias" en una base de datos. Puede reorganizar, desplazar e invertir índices libremente sin tocar los datos en sí; también puede tratar cualquier contenedor (`vector`, `map`, `array`...) como un todo indivisible que fluye a través del flujo, y "desempaquetarlo" de vuelta al nivel de elemento en cualquier momento. Esta capacidad de cambiar granularidad es algo que carecen los frameworks de streaming tradicionales. 🎯

---

## 🏗️ Arquitectura del proyecto: Diseño modular de ocho capas

Semantic-Cpp consta de **ocho archivos de cabecera principales**, construidos progresivamente, donde cada archivo tiene una sola responsabilidad claramente definida y es independientemente comprobable. Cinco espacios de nombres tienen cada uno su propio rol y trabajan juntos para formar una canalización completa desde la fuente de datos hasta el resultado final:

```
┌─────────────────────────────────────────────────┐
│               🔧 semantics.h                     │
│   Espacio de nombres: semantic                  │
│   Fábrica de flujos: rangos numéricos, contenedores, texto, Unicode │
├─────────────────────────────────────────────────┤
│                 📦 semantic.h                    │
│   Espacio de nombres: semantic / collectable    │
│   Operaciones intermedias de flujo, sistema Collectable, 10 especializaciones de contenedores │
├─────────────────────────────────────────────────┤
│                📊 collectors.h                   │
│   Espacio de nombres: collector                 │
│   Fábrica de colectores: coincidencia, búsqueda, agregación, estadísticas, DFT/FFT, etc. │
├─────────────────────────────────────────────────┤
│                ⚙️ collector.h                    │
│   Espacio de nombres: collector                 │
│   Marco de colector: modelo de cinco etapas, soporte de concurrencia y paralelismo │
├─────────────────────────────────────────────────┤
│                🔤 charsequence.h                 │
│   Espacio de nombres: charsequence              │
│   Secuencias de caracteres Unicode, conversión de múltiples codificaciones, Builder, Buffer │
├─────────────────────────────────────────────────┤
│                🧵 pool.h                         │
│   Espacio de nombres: pool                      │
│   Grupo de hilos global: envío de tareas, apagado de emergencia, propagación de excepciones │
├─────────────────────────────────────────────────┤
│                📄 function.h                     │
│   Espacio de nombres: function                  │
│   Definiciones de tipos: Generator, Supplier, Consumer y otros alias │
├─────────────────────────────────────────────────┤
│               🔐 hash.h / less.h                 │
│   Espacio de nombres: std (extensión)           │
│   Especializaciones de hash y comparación de contenedores de biblioteca estándar, soporta anidamiento arbitrario │
└─────────────────────────────────────────────────┘
```

### 🧩 Dependencias

```
function.h          ← Sin dependencias, fundamento de tipos
pool.h              ← Depende de function.h
charsequence.h      ← Módulo independiente, manejo de Unicode
collector.h         ← Depende de function.h, pool.h
collectors.h        ← Depende de collector.h, charsequence.h
hash.h / less.h     ← Módulos independientes, extensiones de biblioteca estándar
semantic.h          ← Depende de todos los anteriores
semantics.h         ← Depende de semantic.h
```

---

## 🌍 Visión general de espacios de nombres

Semantic-Cpp diseña cuidadosamente cinco espacios de nombres, cada uno como un "departamento" independiente, realizando sus propias tareas mientras colabora estrechamente:

| Espacio de nombres | Archivo              | Responsabilidad                    | Tipos/Funciones principales                                                              |
|--------------------|---------------------|-----------------------------------|-----------------------------------------------------------------------------------------|
| `function`         | function.h          | Base del sistema de tipos         | Timestamp, Module, Generator<T>, Supplier<R>, Consumer<T>, Predicate<T>, etc.         |
| `pool`             | pool.h              | Motor de ejecución concurrente     | pool::pool (grupo de hilos global), submit(), emergencyShutdown()                         |
| `charsequence`     | charsequence.h      | Manejo de cadenas Unicode         | charset, Meta, Point, Charsequence, Builder, Buffer, etc.                             |
| `collector`        | collector.h + collectors.h | Ejecución de recolección terminal | Collector<E,A,R>, Identity<A>, Accumulator<A,E>, etc.                                 |
| `collectable`      | semantic.h          | Contenedor de datos materializado  | Collectable<E>, OrderedCollectable<E>, UnorderedCollectable<E>, etc.                  |
| `semantic`         | semantic.h + semantics.h | Construcción de flujo y operaciones intermedias | Semantic<E>, useRange(), useFrom(), etc.                                             |

### 🔁 Flujo de colaboración de espacios de nombres

```cpp
semantic::useRange(0, 100)          // ← espacio de nombres semantic: Crear flujo
    .map(int x { ... })         // ← espacio de nombres semantic: Transformación intermedia
    .filter(int x { ... })      // ← espacio de nombres semantic: Filtrado intermedio
    .toUnordered()                  // ← Convertir a espacio de nombres collectable
    .toVector();                    // ← Invocar colector del espacio de nombres collector
```

---

## 📦 Capa 1: function.h — Base de tipos

`function.h` define el sistema de tipos para todo el framework, la base común para todos los módulos. 🔑

```cpp
namespace function {
    using Timestamp = long long;           // Tipo de índice, la "marca de tiempo" de los datos en el flujo
    using Module = unsigned long long;     // Tipo de módulo/recuento
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept — recibir un elemento
        std::function<bool(T, Timestamp)>       // interrupt — ¿debemos detenernos?
    )>;
}
```

`Generator` es la abstracción central de todo el sistema de streaming. 🌀 No devuelve datos; en su lugar, acepta dos callbacks: `accept` ("Estoy listo, por favor recibe este elemento") e `interrupt` ("¿Necesitamos detenernos?"). Este diseño de inversión de control significa que el productor de datos no tiene idea de quién es el consumidor; solo necesita "empujar" datos en el momento adecuado. Esta es la esencia de la evaluación perezosa: los datos solo "fluyen" realmente cuando se llama a `accept`; antes de eso, todo es solo una descripción.

| Alias de tipo     | Definición completa                                      | Propósito                               |
|-------------------|--------------------------------------------------------|-----------------------------------------|
| Timestamp         | long long                                              | Posición lógica de un elemento en el flujo |
| Module            | unsigned long long                                     | Recuento, capacidad, grado de concurrencia |
| Runnable          | std::function<void()>                                  | Tarea sin parámetros que devuelve void |
| Supplier<R>       | std::function<R()>                                     | Proveedor, crea desde la nada           |
| Function<T,R>     | std::function<R(T)>                                    | Función de un parámetro                 |
| BiFunction<T,U,R> | std::function<R(T,U)>                                  | Función de dos parámetros               |
| TriFunction<T,U,V,R>| std::function<R(T,U,V)>                            | Función de tres parámetros              |
| Unary<T>          | std::function<T(T)>                                    | Operación unaria                        |
| Binary<T>         | std::function<T(T,T)>                                  | Operación binaria                       |
| Consumer<T>       | std::function<void(T)>                                 | Consumidor                              |
| BiConsumer<T,U>   | std::function<void(T,U)>                               | Consumidor de dos parámetros            |
| TriConsumer<T,U,V>| std::function<void(T,U,V)>                             | Consumidor de tres parámetros           |
| Predicate<T>      | std::function<bool(T)>                                 | Juicio de predicado                     |
| BiPredicate<T,U>  | std::function<bool(T,U)>                               | Predicado de dos parámetros             |
| TriPredicate<T,U,V>| std::function<bool(T,U,V)>                         | Predicado de tres parámetros            |
| Comparator<T>     | std::function<int(const T&,const T&)>                  | Comparador, devuelve negativo/cero/positivo |
| Generator<T>      | BiConsumer<BiConsumer<T,Timestamp>, BiPredicate<T,Timestamp>> | Abstracción central del generador de flujos |

---

## 🧵 Capa 2: pool.h — Base de concurrencia

`pool.h` proporciona el grupo de hilos global `pool::pool`, el motor de concurrencia para todo el framework. 🚀 Emplea un diseño de paralelismo declarativo:

| Característica         | Descripción                                                                           |
|------------------------|---------------------------------------------------------------------------------------|
| Paralelismo declarativo | `.parallel(4)` solo declara "Quiero usar 4 hilos", no inicia inmediatamente           |
| Apagado de emergencia   | `emergencyShutdown()` incorporado y manejador `std::set_terminate`                    |
| Propagación de excepciones | `submit()` devuelve `std::future`, propagando excepciones de forma segura al hilo principal |
| Miembro                      | Tipo                | Descripción                                 |
|-----------------------------|---------------------|---------------------------------------------|
| `pool::pool`                | Instancia global de grupo de hilos | Grupo de hilos singleton a nivel de programa, inicializado automáticamente |
| `pool::pool.submit<A>(task)`| Método              | Envía una tarea, devuelve `std::future<A>`  |
| `pool::pool.emergencyShutdown()` | Método       | Apagado de emergencia de todos los hilos    |

---

## 🔤 Capa 3: charsequence.h — Secuencias de caracteres Unicode

`charsequence.h` es un módulo completo de procesamiento Unicode, que proporciona funcionalidad para crear, convertir y manipular secuencias de caracteres. 🌍 Admite múltiples codificaciones como UTF-8, UTF-16 (LE/BE), UTF-32 (LE/BE), ASCII y Latin1, detecta y maneja correctamente pares sustitutos, y devuelve el carácter de reemplazo estándar U+FFFD para puntos de código no válidos.

| Tipo/Función       | Descripción                                                                 |
|--------------------|-----------------------------------------------------------------------------|
| charset            | Enumeración: ascii, utf8, utf16, utf16be, utf16le, utf32, utf32be, utf32le, latin1 |
| Meta               | Envoltorio de metadatos, almacena valores enteros sin signo                 |
| Point              | Punto de código Unicode, admite detección de pares sustitutos (`isSurrogate()`) y verificación de validez (`isValidCodePoint()`) |
| Charsequence       | Secuencia de caracteres inmutable: split, replace, indexOf, lastIndexOf, sub, trim, toUpperCase, toLowerCase, reverse, startsWith, endsWith, contains, compare, getBytes, getPoints, getMetas, getCharacters, repeat, concat, count, join |
| Builder            | Constructor de bytes mutable: prepend, insert, append (admite bool, short, int, long, long long, float, double, long double y tipos sin signo correspondientes, char, unsigned char, Point, Charsequence, string_view) |
| Buffer             | Búfer circular seguro para hilos: write, read, peek, prepend, append, clear, shrinkToFit, data, size, capacity, atomic (operaciones por lotes bloqueadas) |
| PointIterator      | Iterador bidireccional, recorre puntos de código Unicode                   |
| sequenceLength()   | Determina la longitud de secuencia para codificaciones multibyte como UTF-8 basándose en el primer byte |
| encode()           | Codifica un solo punto de código en una secuencia de bytes de la codificación especificada |
| decode()           | Decodifica el siguiente punto de código desde una secuencia de bytes, avanza el puntero automáticamente |
| convert()          | Conversión de codificación (admite salida string, vector<unsigned char>, deque<unsigned char>) |

---

## ⚙️ Capa 4: collector.h — Marco de colectores

`collector.h` implementa el patrón colector, el motor central para operaciones terminales. 🔧

### 🧩 Modelo de cinco etapas

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (cortocircuito opcional)
```

Estas cinco etapas son como una línea de montaje precisa: primero prepara el estado inicial (Identity), luego recibe elementos uno por uno y actualiza resultados intermedios (Accumulator), después combina resultados parciales de cada hilo en escenarios paralelos (Combiner), y finalmente transforma el resultado intermedio en la forma final deseada por el usuario (Finisher). Interrupt actúa como un supervisor alerta, listo para detener todo el proceso en cualquier momento cuando se cumplan las condiciones. 🚨

| Alias de tipo    | Definición completa                               | Rol                           |
|------------------|--------------------------------------------------|-------------------------------|
| Identity<A>      | function::Supplier<A>                            | Proporciona valor inicial     |
| Accumulator<A,E> | function::TriFunction<A, E, Timestamp, A>        | Acumula elementos             |
| Combiner<A>      | function::BiFunction<A, A, A>                    | Combina resultados paralelos  |
| Finisher<A,R>    | function::Function<A, R>                         | Transformación final          |
| Interrupt<E,A>   | function::TriPredicate<E, Timestamp, A>          | Juicio de cortocircuito       |

### 🔧 Funciones del marco

| Función                                                               | Descripción                         |
|-----------------------------------------------------------------------|-------------------------------------|
| `useFull(identity, accumulator, combiner, finisher)`                  | Crea un colector completo (sin cortocircuito) |
| `useShortable(identity, interrupt, accumulator, combiner, finisher)`  | Crea un colector con cortocircuito |

### 🧱 Clase `Collector<E,A,R>`

| Método                               | Descripción                                 |
|--------------------------------------|---------------------------------------------|
| `collect(generator, concurrent)`     | Recoge desde un Generator, admite paralelismo |
| `collect(container, concurrent)`     | Recoge desde un contenedor estándar, admite paralelismo |
| `collect(initializer_list, concurrent)`| Recoge desde una lista de inicialización     |
| `collect(array, concurrent)`         | Recoge desde un std::array                 |
| `collect(deque, concurrent)`         | Recoge desde un std::deque                |
| `collect(forward_list, concurrent)`  | Recoge desde un std::forward_list         |
| `collect(stack, concurrent)`         | Recoge desde un std::stack               |
| `collect(queue, concurrent)`         | Recoge desde un std::queue              |

### 🔀 Soporte de concurrencia

`Collector::collect()` maneja automáticamente: partición de datos (distribución a hilos vía módulo de índice), fusión de resultados (combinación de resultados parciales vía Combiner), propagación de excepciones (vía `std::exception_ptr` y `std::atomic<bool>`). 🔀

---

## 📊 Capa 5: collectors.h — Fábrica de colectores

`collectors.h` proporciona un rico conjunto de funciones de fábrica de colectores predefinidas. Esto no es simplemente "algunas funciones de utilidad": es un arsenal completo de herramientas de procesamiento de datos, que cubre todo el espectro desde la coincidencia básica hasta el análisis de dominio de frecuencia. 🔥

### ✅ Operaciones de coincidencia

¿Quiere saber si todos los elementos en el flujo satisfacen una condición? ¿O si existe algún "rebelde"? Estos tres dan la respuesta al instante y todos admiten evaluación de cortocircuito: detienen el recorrido tan pronto como se determina la respuesta. ⚡

| Método                  | Descripción                                  | Tipo de retorno |
|-------------------------|----------------------------------------------|-----------------|
| `useAllMatch(predicate)` | Todos los elementos satisfacen la condición (cuantificador universal) | `bool` |
| `useAnyMatch(predicate)` | Algún elemento satisface la condición (cuantificador existencial) | `bool` |
| `useNoneMatch(predicate)`| Ningún elemento satisface la condición        | `bool` |

### 🔍 Operaciones de búsqueda

Encuentre el deseado "eso" desde el vasto mar de datos. Los índices negativos son la característica única de Semantic-Cpp: `findAt(-1)` obtiene directamente el último elemento. 🎯

| Método              | Descripción                                     | Tipo de retorno      |
|---------------------|-------------------------------------------------|----------------------|
| `useFindFirst()`    | Encuentra el primer elemento                    | `std::optional<E>`   |
| `useFindLast()`     | Encuentra el último elemento                    | `std::optional<E>`   |
| `useFindAny()`      | Encuentra cualquier elemento aleatoriamente     | `std::optional<E>`   |
| `useFindAt(index)`  | Posicionamiento preciso: admite índices positivos y negativos | `std::optional<E>` |
| `useFindMaximum()`  | Encuentra el valor máximo (admite comparador personalizado) | `std::optional<E>` |
| `useFindMinimum()`  | Encuentra el valor mínimo (admite comparador personalizado) | `std::optional<E>` |

### 🔢 Operaciones de agregación

Deje que los datos hablen, resuma todo con números. Cada método de agregación admite un parámetro de mapeador opcional: aplica una transformación a cada elemento antes de la agregación. 📈

| Método              | Descripción                    | Tipo de retorno      |
|---------------------|--------------------------------|----------------------|
| `useCount()`        | Número total de elementos      | `Module`             |
| `useSummate<E,D>()` | Sumatoria                      | `D`                  |
| `useAverage<E,D>()` | Promedio                       | `D`                  |
| `useRange<E,D>()`   | Rango numérico (max - min)     | `D`                  |
| `useMinimum<E,D>()` | Valor mínimo                   | `std::optional<D>`   |
| `useMaximum<E,D>()` | Valor máximo                   | `std::optional<D>`   |

### 📉 Operaciones estadísticas

Desde estadísticas descriptivas hasta análisis de dominio de frecuencia, una navaja suiza para estadísticos. Particularmente notables son `useMode()` y `useFrequency()`: utilizan tecnología de codificación de fase de índice, codifican la información de posición de cada aparición de un elemento como un ángulo en el plano complejo, y utilizan la fórmula de Euler para capturar patrones periódicos en los datos. 🎼

| Método                        | Descripción                                 | Tipo de retorno               |
|-------------------------------|---------------------------------------------|-------------------------------|
| `useVariance<E,D>()`         | Varianza de población                      | `D`                           |
| `useStandardDeviation<E,D>()`| Desviación estándar de población            | `D`                           |
| `useSkewness<E,D>()`         | Asimetría (simetría de distribución)        | `D`                           |
| `useKurtosis<E,D>()`         | Curtosis (grosor de cola de distribución)   | `D`                           |
| `useMedian<E,D>()`           | Mediana                                     | `std::optional<D>`            |
| `useMode<E>()`               | Moda (basado en análisis de dominio de frecuencia) | `std::optional<E>`            |
| `usePercentile<E,D>(p)`      | percentil p-ésimo                           | `std::optional<D>`            |
| `useFrequency<E>()`          | Características de dominio de frecuencia (codificación de fase de índice) | `std::map<E, complex>` |
| `useDistribution<E>()`       | Características de distribución espacial (codificación de posición) | `std::map<E, complex>` |

### 🔀 Operaciones de reducción

Reducción (Reduce) es uno de los conceptos más poderosos en programación funcional: puede "plegar" un flujo en un solo valor. 🎁

| Método                        | Descripción                                     | Tipo de retorno      |
|-------------------------------|-------------------------------------------------|----------------------|
| `useReduce(reducer)`          | Reducción sin valor inicial (devuelve nullopt si el flujo está vacío) | `std::optional<E>` |
| `useReduce(identity, reducer)`| Reducción con valor inicial                     | `E`                  |
| `useReduce(id, red, comb, fin)` | Completamente personalizado: acumulación, combinación, finalización personalizadas | `R`    |

### 🧺 Operaciones de recolección en contenedores

Los datos que han vagado por el flujo toda su vida finalmente necesitan un "hogar". Aquí hay 20+ contenedores de biblioteca estándar para elegir: 🏡

| Método                           | Descripción                         | Tipo de retorno                |
|----------------------------------|-------------------------------------|--------------------------------|
| `useToVector()`                  | Recoge en vector, manteniendo orden | `std::vector<E>`              |
| `useToList()`                    | Recoge en list                     | `std::list<E>`                |
| `useToDeque()`                   | Recoge en deque                    | `std::deque<E>`               |
| `useToForwardList()`             | Recoge en forward_list             | `std::forward_list<E>`        |
| `useToArray<N>()`                | Recoge en array de tamaño fijo     | `std::array<E, N>`            |
| `useToSet()`                     | Recoge en set (deduplicado, ordenado) | `std::set<E>`             |
| `useToMultiset()`                | Recoge en multiset                 | `std::multiset<E>`            |
| `useToUnorderedSet()`            | Recoge en unordered_set            | `std::unordered_set<E>`       |
| `useToUnorderedMultiset()`       | Recoge en unordered_multiset       | `std::unordered_multiset<E>`  |
| `useToMap(keyExtractor)`         | Recoge en map por clave            | `std::map<K, E>`              |
| `useToMap(keyExtractor, valueExtractor)` | Recoge en map con clave-valor personalizado | `std::map<K, V>` |
| `useToMultimap(keyExtractor)`    | Recoge en multimap por clave       | `std::multimap<K, E>`         |
| `useToMultimap(keyExtractor, valueExtractor)` | Recoge en multimap con clave-valor personalizado | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)` | Recoge en unordered_map | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)` | Recoge en unordered_multimap por clave | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)` | Recoge en unordered_multimap con clave-valor personalizado | `std::unordered_multimap<K, V>` |
| `useToStack()`                   | Recoge en stack                    | `std::stack<E>`               |
| `useToQueue()`                   | Recoge en queue                    | `std::queue<E>`               |
| `useToPriorityQueue()`           | Recoge en priority_queue           | `std::priority_queue<E>`      |

### 🧩 Operaciones de agrupación y partición

El equivalente de `GROUP BY` de SQL en C++. Las versiones de dos parámetros de `groupBy` y `partitionBy` permiten especificar simultáneamente tanto un extractor de clave como un extractor de valor, por ejemplo, agrupar empleados por departamento pero solo tomar nombres de empleados en lugar del objeto completo. 👥

| Método                                     | Descripción                       | Tipo de retorno                          |
|--------------------------------------------|-----------------------------------|------------------------------------------|
| `useGroup(keyExtractor)`                   | Agrupa por clave, mantiene elementos completos | `std::unordered_map<K, vector<E>>` |
| `useGroupBy(keyExtractor, valueExtractor)` | Agrupa por clave, extracción de valor personalizada | `std::unordered_map<K, vector<V>>` |
| `usePartition(size)`                       | Particiona por tamaño fijo        | `std::vector<vector<E>>`                |
| `usePartitionBy(keyExtractor)`             | Particiona por clave de índice, mantiene elementos completos | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor, valueExtractor)` | Particiona por clave de índice, extracción de valor personalizada | `std::vector<vector<V>>` |

```cpp
// Agrupar por departamento, solo mantener nombres
auto deptNames = people.toUnordered().groupBy(
    Person p { return p.department; },   // Clave: departamento
    Person p { return p.name; }          // Valor: nombre
);
// Devuelve: unordered_map<string, vector<string>>
```

### 📃 Operaciones de salida de cadena

Las series `useJoin()`, `useOut()`, `useError()` permiten que sus datos se presenten en un formato legible por humanos: 📃

| Método                               | Descripción                 | Tipo de retorno                |
|--------------------------------------|-----------------------------|--------------------------------|
| `useJoin()`                          | Unión de formato estándar: `[elem1,elem2,...]` | `Charsequence`     |
| `useJoin(delimiter)`                 | Delimitador personalizado   | `Charsequence`                 |
| `useJoin(prefix, delimiter, suffix)` | Formato completamente personalizado | `Charsequence`         |
| `useOut()`                           | Sale a stdout              | `Charsequence`                 |
| `useOut(delimiter)`                  | Sale a stdout con delimitador | `Charsequence`             |
| `useOut(prefix, delimiter, suffix)`  | Salida completamente personalizada a stdout | `Charsequence`     |
| `useError()`                         | Sale a stderr              | `Charsequence`                 |
| `useError(delimiter)`                | Sale a stderr con delimitador | `Charsequence`             |
| `useError(prefix, delimiter, suffix)`| Salida completamente personalizada a stderr | `Charsequence`     |

### 📊 Herramientas matemáticas

| Método                                     | Descripción                         | Tipo de retorno                     |
|--------------------------------------------|-------------------------------------|-------------------------------------|
| `useDFT()`                                 | Transformada de Fourier Discreta    | `vector<complex<double>>`           |
| `useIDFT()`                                | Transformada de Fourier Discreta Inversa | `vector<complex<double>>`       |
| `useFFT()`                                 | Transformada Rápida de Fourier (Cooley-Tukey) | `vector<complex<double>>` |
| `useIFFT()`                                | Transformada Rápida de Fourier Inversa | `vector<complex<double>>`       |
| `useGradient(gradFunc, lr, iter, th)`      | Descenso de Gradiente (gradiente analítico) | `vector<double>`          |
| `useGradient(costFunc, lr, iter, th, h)`   | Descenso de Gradiente (gradiente numérico) | `vector<double>`          |

---

## 📦 Capa 6: semantic.h — Operaciones intermedias de flujo y sistema de recolección

---

### 🧩 Diseño central: Tubería de tres etapas — `Semantic` → `Collectable` → `Collector`

Muchos frameworks de streaming mezclan "operaciones intermedias" y "operaciones terminales" en el mismo tipo, lo que hace que no esté claro cuándo se procesan realmente los datos. Semantic-Cpp adopta un diseño fundamentalmente diferente de separación de tres etapas, donde cada etapa tiene un deber claro y un límite distinto:

```
┌──────────────────────────────────────────────────────────┐
│                 🌱 Etapa 1: Construcción y Transformación │
│               Semantic<E>  (Flujo Semántico)             │
│                 Espacio de nombres: semantic             │
│                                                          │
│  · Construcción perezosa: useRange, useFrom, useGenerate... |
│  · Transformaciones intermedias: map, filter, takeWhile, distinct... |
│  · Operaciones de índice: reverse, translate, redirect... |
│  · Declaración paralela: parallel(n)                     │
│                                                          │
│  En este punto, los datos todavía están "tumbados" en la tubería, inmóviles, nada ha sucedido. │
│  Como si hubieras escrito un itinerario de viaje detallado, pero aún no has salido de casa. │
└──────────────────┬───────────────────────────────────────┘
                   │  toUnordered() / toOrdered() /
                   │  toWindow() / toStatistics() / sort()
                   ▼
┌──────────────────────────────────────────────────────────┐
│                📦 Etapa 2: Materialización y Recolección │
│           Collectable<E>  (Objeto Recolectable)          │
│             Espacio de nombres: collectable              │
│                                                          │
│  · Dispara el flujo de datos: El Generator ahora se invoca realmente │
│  · Selecciona la estructura de datos: map ordenado / unordered_map desordenado │
│  · Materializa todos los elementos en memoria           │
│                                                          │
│  ¡Los datos finalmente "cobran vida"! Fluyen desde la tubería, organizados en contenedores adecuados. │
│  Como si finalmente hubieras salido, equipaje empacado, listo para partir. │
└──────────────────┬───────────────────────────────────────┘
                   │  toVector() / findFirst() / count() /
                   │  summate() / average() / join() / ...
                   ▼
┌──────────────────────────────────────────────────────────┐
│                ⚙️ Etapa 3: Cálculo Terminal              │
│              Collector<E,A,R>                            │
│               Espacio de nombres: collector              │
│                                                          │
│  · Ejecución de cinco etapas: Identity → Accumulate → Combine → Finish │
│  · Soporte paralelo: Acumulación particionada multi-hilo, fusión automática │
│  · Devuelve resultado final: vector, optional, double, bool... │
│                                                          │
│  ¡El resultado final está listo! Como si hubieras llegado a tu destino y tomado la foto perfecta. │
└──────────────────────────────────────────────────────────┘
```

**Regla clave:** Primero debe convertir `Semantic<E>` a `Collectable<E>` vía `toUnordered()`, `toOrdered()`, `toWindow()`, `toStatistics()`, o `sort()`, antes de poder llamar a métodos terminales como `toVector()`, `findFirst()`, `count()`, etc. Estos métodos no están en `Semantic` — pertenecen a `Collectable`. Esto no es un descuido, sino una decisión de diseño deliberada. ✅

---

### 🧭 Cinco caminos de materialización, cinco destinos diferentes

Cuando se encuentra en la encrucijada de `Semantic<E>`, hay cinco direcciones para elegir. Cada una determina cómo se organizarán, consultarán y usarán los datos. Comprender las diferencias entre ellas es clave para dominar Semantic-Cpp. 🧠

| Método de conversión  | Tipo objetivo           | Estructura de datos subyacente | Características de rendimiento | Métodos terminales típicos       |
|-----------------------|-------------------------|-------------------------------|--------------------------------|----------------------------------|
| `toUnordered()`       | UnorderedCollectable<E> | unordered_map<Timestamp,E>    | Búsqueda promedio O(1), mayor rendimiento | toVector, findFirst, count, group... |
| `toOrdered()`         | OrderedCollectable<E>   | map<Timestamp,E>              | Búsqueda O(log n), ordenado por índice | toVector, findAt, join, toMap... |
| `sort()`              | OrderedCollectable<E>   | map<Timestamp,E>              | Materializado después de ordenar por valor | Igual que arriba (pero índice sobrescrito a orden de valor) |
| `toWindow()`          | WindowCollectable<E>    | Hereda colección ordenada     | Admite slide / tumble          | slide, tumble y todos los métodos padre |
| `toStatistics<D>()`   | Statistics<E,D>         | Hereda colección ordenada     | 30+ métodos estadísticos incorporados | summate, average, median, fft... |

### 🔄 Comparación detallada de los cinco caminos

| Dimensión de comparación | toUnordered()           | toOrdered()             | sort()                  | toWindow()              | toStatistics<D>()       |
|--------------------------|-------------------------|-------------------------|-------------------------|-------------------------|-------------------------|
| ¿Preserva orden de índice? | No (desordenado)        | Sí (por índice original) | No (reordenado por valor) | Sí (hereda ordenado)    | Sí (hereda ordenado)    |
| Rendimiento de búsqueda  | Más rápido O(1)         | Rápido O(log n)         | Rápido O(log n)         | Rápido O(log n)         | Rápido O(log n)         |
| Uso de memoria           | Bajo                    | Bajo                    | Bajo                    | Bajo                    | Depende del método estadístico |
| ¿Puede continuar cadena? | Sí (métodos terminales) | Sí (métodos terminales) | Sí (métodos terminales) | Sí (slide/tumble devuelve Semantic) | Sí (métodos terminales) |
| Caso de uso              | Agregación rápida donde el orden no importa | Series temporales, preservando orden de generación | Ordenar por valor, clasificación, paginación | Análisis de ventana deslizante | Modelado estadístico matemático |

`sort()` es la única operación intermedia que omite `toXxx()` y va directamente a `Collectable`. Después de llamarlo, todas las operaciones de índice anteriores (reverse, translate, redirect) se sobrescribirán, y los elementos se reasignan índices secuenciales naturales basados en sus valores ordenados. 🔀

**¿Qué camino elegir? Pregúntese:** 🤔

* ¿Me importa el orden? Si es sí, use `toOrdered()`; si no, use `toUnordered()`.
* ¿Necesito análisis de ventana? `toWindow()` luego `slide()` o `tumble()`.
* ¿Necesito estadísticas? `toStatistics<double>()` para una solución todo en uno.
* ¿Solo quiero ordenar? `sort()` en un paso.

---

### 📋 Espacio de nombres collectable — Todos los métodos terminales

#### ✅ Operaciones de coincidencia

| Método              | Tipo de retorno | Descripción                 |
|---------------------|-----------------|-----------------------------|
| `anyMatch(predicate)`  | bool            | Algún elemento satisface condición |
| `allMatch(predicate)`  | bool            | Todos los elementos satisfacen condición |
| `noneMatch(predicate)` | bool            | Ningún elemento satisface condición |

#### 🔍 Operaciones de búsqueda

| Método                     | Tipo de retorno      | Descripción                     |
|----------------------------|----------------------|---------------------------------|
| `findFirst()`              | std::optional<E>     | Encuentra el primer elemento    |
| `findLast()`               | std::optional<E>     | Encuentra el último elemento    |
| `findAny()`                | std::optional<E>     | Encuentra cualquier elemento aleatoriamente |
| `findAt(index)`            | std::optional<E>     | Encuentra elemento en índice especificado (admite negativo) |
| `findMaximum()`            | std::optional<E>     | Encuentra el valor máximo       |
| `findMaximum(comparator)`  | std::optional<E>     | Encuentra máximo con comparador personalizado |
| `findMinimum()`            | std::optional<E>     | Encuentra el valor mínimo       |
| `findMinimum(comparator)`  | std::optional<E>     | Encuentra mínimo con comparador personalizado |

#### 🔢 Operaciones de agregación

| Método    | Tipo de retorno       | Descripción         |
|-----------|-----------------------|---------------------|
| `count()` | function::Module      | Cuenta elementos totales |
| `empty()` | bool                  | Si el flujo está vacío |

#### 🔀 Operaciones de reducción

| Método                      | Tipo de retorno      | Descripción             |
|-----------------------------|----------------------|-------------------------|
| `reduce(accumulator)`       | std::optional<E>     | Reducción sin valor inicial |
| `reduce(identity, accumulator)` | E            | Reducción con valor inicial |
| `reduce(identity, acc, combiner)` | R | Reducción completamente personalizada |

#### 🧺 Recolección en contenedores de secuencia

| Método            | Tipo de retorno             | Descripción                 |
|-------------------|-----------------------------|-----------------------------|
| `toVector()`      | std::vector<E>              | Recoge en vector, manteniendo orden |
| `toList()`        | std::list<E>                | Recoge en list              |
| `toDeque()`       | std::deque<E>               | Recoge en deque             |
| `toForwardList()` | std::forward_list<E>        | Recoge en forward_list      |
| `toArray<N>()`    | std::array<E, N>            | Recoge en array de tamaño fijo |

#### 🔐 Recolección en contenedores asociativos

| Método                                     | Tipo de retorno                        | Descripción                 |
|--------------------------------------------|----------------------------------------|-----------------------------|
| `toSet()`                                  | std::set<E>                            | Recoge en set (deduplicado, ordenado) |
| `toMultiset()`                             | std::multiset<E>                       | Recoge en multiset          |
| `toUnorderedSet()`                         | std::unordered_set<E>                  | Recoge en unordered_set     |
| `toUnorderedMultiset()`                    | std::unordered_multiset<E>             | Recoge en unordered_multiset |
| `toMap(keyExtractor)`                      | std::map<K, E>                         | Recoge en map por clave     |
| `toMap(keyExtractor, valueExtractor)`      | std::map<K, V>                         | Recoge en map con clave-valor personalizado |
| `toMultimap(keyExtractor)`                 | std::multimap<K, E>                    | Recoge en multimap por clave |
| `toMultimap(keyExtractor, valueExtractor)` | std::multimap<K, V>                    | Recoge en multimap con clave-valor personalizado |
| `toUnorderedMap(keyExtractor, valueExtractor)` | std::unordered_map<K, V> | Recoge en unordered_map |
| `toUnorderedMultimap(keyExtractor)`        | std::unordered_multimap<K, E>          | Recoge en unordered_multimap por clave |
| `toUnorderedMultimap(keyExtractor, valueExtractor)` | std::unordered_multimap<K, V> | Recoge en unordered_multimap con clave-valor personalizado |

#### 🧮 Recolección en contenedores adaptadores

| Método              | Tipo de retorno                | Descripción         |
|---------------------|--------------------------------|---------------------|
| `toStack()`         | std::stack<E>                  | Recoge en stack     |
| `toQueue()`         | std::queue<E>                  | Recoge en queue     |
| `toPriorityQueue()` | std::priority_queue<E>         | Recoge en priority_queue |

#### 👥 Agrupación y partición

| Método                                     | Tipo de retorno                          | Descripción                 |
|--------------------------------------------|------------------------------------------|-----------------------------|
| `group(keyExtractor)`                      | std::unordered_map<K, std::vector<E>>    | Agrupa por clave, mantiene elementos completos |
| `groupBy(keyExtractor, valueExtractor)`    | std::unordered_map<K, std::vector<V>>    | Agrupa por clave, extracción de valor personalizada |
| `partition(size)`                          | std::vector<std::vector<E>>             | Particiona por tamaño fijo |
| `partitionBy(keyExtractor)`                | std::vector<std::vector<E>>             | Particiona por clave de índice, mantiene elementos completos |
| `partitionBy(keyExtractor, valueExtractor)` | std::vector<std::vector<V>>           | Particiona por clave de índice, extracción de valor personalizada |

#### 📃 Operaciones de salida de cadena

| Método                               | Tipo de retorno             | Descripción                 |
|--------------------------------------|-----------------------------|-----------------------------|
| `join()`                             | charsequence::Charsequence  | Unión de formato estándar: `[elem1,elem2,...]` |
| `join(delimiter)`                    | charsequence::Charsequence  | Delimitador personalizado   |
| `join(prefix, delimiter, suffix)`    | charsequence::Charsequence  | Formato completamente personalizado |
| `out()`                              | charsequence::Charsequence  | Sale a stdout              |
| `out(delimiter)`                     | charsequence::Charsequence  | Sale a stdout con delimitador |
| `out(prefix, delimiter, suffix)`     | charsequence::Charsequence  | Salida completamente personalizada a stdout |
| `error()`                            | charsequence::Charsequence  | Sale a stderr              |
| `error(delimiter)`                   | charsequence::Charsequence  | Sale a stderr con delimitador |
| `error(prefix, delimiter, suffix)`   | charsequence::Charsequence  | Salida completamente personalizada a stderr |

#### 🔧 Recolección y recorrido personalizado

| Método                                 | Tipo de retorno | Descripción                 |
|----------------------------------------|-----------------|-----------------------------|
| `collect(identity, acc, comb, fin)`    | R               | Recolección personalizada de cuatro etapas |
| `collect(identity, interrupt, acc, comb, fin)` | R | Recolección interrumpible personalizada |
| `forEach(consumer)`                    | void            | Realiza efecto secundario para cada elemento |

---

### 📈 `Statistics<E,D>` — Métodos estadísticos

| Método                     | Tipo de retorno              | Descripción                                 |
|----------------------------|------------------------------|---------------------------------------------|
| `summate()`                | D                            | Sumatoria                                   |
| `average()`                | D                            | Promedio                                    |
| `minimum()`                | std::optional<D>             | Valor mínimo                                |
| `maximum()`                | std::optional<D>             | Valor máximo                                |
| `range()`                  | D                            | Rango (max - min)                           |
| `variance()`               | D                            | Varianza de población                       |
| `standardDeviation()`      | D                            | Desviación estándar de población            |
| `median()`                 | std::optional<D>             | Mediana                                     |
| `mode()`                   | std::optional<E>             | Moda (basado en análisis de dominio de frecuencia) |
| `percentile(p)`            | std::optional<D>             | percentil p-ésimo                           |
| `firstQuartile()`          | std::optional<D>             | Primer cuartil (Q1)                         |
| `thirdQuartile()`          | std::optional<D>             | Tercer cuartil (Q3)                         |
| `interquartileRange()`     | std::optional<D>             | Rango intercuartílico (IQR = Q3 - Q1)       |
| `skewness()`               | D                            | Asimetría                                   |
| `kurtosis()`               | D                            | Curtosis                                    |
| `frequency()`              | std::map<E, std::complex<double>> | Características de dominio de frecuencia (codificación de fase de índice) |
| `distribute()`             | std::map<E, std::complex<double>> | Características de distribución espacial (codificación de posición) |
| `dft()`                    | std::vector<std::complex<double>> | Transformada de Fourier Discreta             |
| `idft()`                   | std::vector<std::complex<double>> | Transformada de Fourier Discreta Inversa   |
| `fft()`                    | std::vector<std::complex<double>> | Transformada Rápida de Fourier                 |
| `ifft()`                   | std::vector<std::complex<double>> | Transformada Rápida de Fourier Inversa       |
| `gradient(gradFunc, lr, iter, th)` | std::vector<double> | Descenso de Gradiente (gradiente analítico)   |
| `gradient(costFunc, lr, iter, th, h)` | std::vector<double> | Descenso de Gradiente (gradiente numérico) |

Todos los métodos anteriores también admiten una versión de parámetro de mapeador opcional (por ejemplo, `average(mapper)`), que transforma elementos primero antes de realizar estadísticas.

---

### 🧰 Nuevas especializaciones de contenedores: Contenedores como elementos

Esta es una de las características más únicas de Semantic-Cpp. En los frameworks de streaming tradicionales, cada elemento en el flujo es típicamente un valor escalar: un int, un string. Pero en el mundo real, los datos a menudo existen en lotes:

* Un lote de lecturas de sensores = `std::vector<double>`
* Un conjunto de pedidos de usuario = `std::list<Order>`
* Una configuración clave-valor = `std::map<string, string>`
* Una fila de matriz de tamaño fijo = `std::array<float, 4>`

Semantic-Cpp proporciona especializaciones de plantilla `Semantic` para **10 contenedores de biblioteca estándar**:

| Tipo especializado          | Descripción           | Operaciones admitidas                |
|-----------------------------|-----------------------|--------------------------------------|
| `Semantic<std::vector<E>>`  | Flujo de contenedor vector | Todas las intermedias + todas las terminales |
| `Semantic<std::list<E>>`    | Flujo de contenedor list   | Igual que arriba                     |
| `Semantic<std::deque<E>>`   | Flujo de contenedor deque  | Igual que arriba                     |
| `Semantic<std::set<E>>`     | Flujo de contenedor set ordenado | Igual que arriba              |
| `Semantic<std::unordered_set<E>>` | Flujo de contenedor set desordenado | Igual que arriba           |
| `Semantic<std::map<K,V>>`   | Flujo de contenedor map ordenado | Igual que arriba              |
| `Semantic<std::unordered_map<K,V>>` | Flujo de contenedor map desordenado | Igual que arriba           |
| `Semantic<std::queue<E>>`   | Flujo de contenedor queue   | Igual que arriba                     |
| `Semantic<std::stack<E>>`   | Flujo de contenedor stack   | Igual que arriba                     |
| `Semantic<std::array<E,N>>` | Flujo de contenedor array de tamaño fijo | Igual que arriba                |

Todas las especializaciones de contenedores admiten completamente: `map`, `filter`, `takeWhile`, `dropWhile`, `distinct`, `sort`, `limit`, `skip`, `reverse`, `translate`, `redirect`, `sub`, `concatenate`, `peek`, `flatMap`, `flat`, `parallel` y todos los métodos de conversión terminal. 🔧

```cpp
// Ejemplo de flujo de contenedor: Aplanar múltiples vectores en un flujo de elementos
auto flattened = semantic::useOf({
    std::vector<int>{1, 2},
    std::vector<int>{3, 4, 5}
}).flatMap(std::vector<int> v {
    return semantic::useFrom(v).map(int x { return x * 10; });
}).toOrdered().toVector();
// Salida: 10 20 30 40 50
```

---

### 🔧 Métodos de operación intermedia `Semantic<E>`

| Categoría     | Método       | Descripción                               |
|--------------|--------------|-------------------------------------------|
| Transformación de elementos | `map`      | Transformación de mapeo uno a uno         |
|              | `flatMap`  | Mapeo uno a muchos y aplanamiento (R deducido del tipo de retorno) |
|              | `flat`     | Aplana flujos anidados (tipo de elemento sin cambios) |
| Filtrado de elementos | `filter`   | Filtrado condicional                     |
|              | `takeWhile`| Toma mientras la condición es verdadera, se detiene inmediatamente cuando es falsa |
|              | `dropWhile`| Descarta mientras la condición es verdadera, hasta la primera falsa |
|              | `distinct` | Deduplicación (admite comparador personalizado) |
| Control de cantidad | `limit`    | Limita número de elementos               |
|              | `skip`     | Omite primeros n elementos               |
|              | `sub`      | Toma subrango [start, end)               |
| Operaciones de índice | `redirect` | Reasigna índices                         |
|              | `reverse`  | Invierte índices (usa índices negativos)  |
|              | `translate`| Desplaza índices (valor fijo o función dinámica) |
| Observación  | `peek`     | Observa cada elemento (no modifica flujo) |
| Declaración paralela | `parallel(n)` | Declara grado de paralelismo       |
| Concatenación | `concatenate` | Concatena otro flujo Semantic o contenedor estándar |
| Conversión terminal | `toUnordered` | Convierte a colector desordenado   |
|              | `toOrdered`  | Convierte a colector ordenado       |
|              | `toWindow`   | Convierte a colector de ventana     |
|              | `toStatistics<D>` | Convierte a colector estadístico |

---

## 🔧 Capa 7: semantics.h — Funciones de fábrica de flujos

`semantics.h` proporciona todas las funciones de fábrica de flujos. Si `semantic.h` es el "cerebro" del flujo (responsable de transformación y programación), entonces `semantics.h` es el "corazón": cada viaje de datos comienza aquí. ❤️

### 🔢 Generación de rango numérico

| Método                          | Descripción                         |
|---------------------------------|-------------------------------------|
| `useRange(start, end)`          | Genera flujo numérico en rango [start, end) |
| `useRange(start, end, step)`    | Generación de rango con paso, admite paso negativo |
| `useRangeClosed(start, end)`    | Genera flujo numérico en rango cerrado [start, end] |
| `useRangeClosed(start, end, step)` | Generación de rango cerrado con paso, admite paso negativo |

### ♾️ Generación de flujo infinito

| Método                          | Descripción                     |
|---------------------------------|---------------------------------|
| `useInfinite(seed, generator)`  | Iteración infinita desde valor inicial |
| `useGenerate(supplier)`         | Llamadas infinitas a proveedor      |
| `useGenerate(supplier, limit)`  | Número limitado de llamadas a proveedor |
| `useIterate(seed, generator)`   | Iteración infinita desde semilla    |
| `useIterate(seed, generator, limit)` | Iteración limitada desde semilla |
| `useRandom()`                   | Flujo infinito de enteros aleatorios  |
| `useRandom(min, max)`           | Flujo infinito de números aleatorios en rango especificado (detecta automáticamente int/float) |
| `useRandom(min, max, count)`    | Flujo de números aleatorios con rango y conteo especificados |

### 📦 Construcción de contenedores y elementos

| Método                | Descripción                             |
|-----------------------|-----------------------------------------|
| `useEmpty()`          | Crea un flujo vacío                     |
| `useOf(element)`      | Crea un flujo de un solo elemento       |
| `useOf(e1, e2)`       | Crea un flujo de dos elementos          |
| `useOf(e1, e2, e3)`   | Crea un flujo de tres elementos         |
| `useOf({...})`        | Crea un flujo de una lista de inicialización |
| `useFrom(container)`  | Crea un flujo de cualquier contenedor estándar (admite semántica de movimiento) |
| `useFrom({...})`      | Crea un flujo de una lista de inicialización |
| `useRepeat(element, count)` | Repite el elemento especificado n veces |

### 📄 Procesamiento de texto

| Método                    | Descripción                         |
|---------------------------|-------------------------------------|
| `useBlob(text)`           | Divide cadena en flujo char byte por byte |
| `useBlob(text, start, end)` | Divide rango especificado de cadena byte por byte |
| `useBlob(istream)`        | Lee desde flujo de entrada línea por línea     |
| `useBlob(istream, delimiter)` | Lee desde flujo de entrada por delimitador |
| `useText(text)`           | Trata cadena como flujo de texto completo (Charsequence) |
| `useText(text, delimiter)` | Divide texto por delimitador           |
| `useText(istream)`        | Lee contenido completo desde flujo de entrada |
| `useText(istream, delimiter)` | Lee desde flujo de entrada por delimitador |

### 🌍 Procesamiento Unicode

| Método                          | Descripción                                   |
|---------------------------------|-----------------------------------------------|
| `useSequence(charsequence)`     | Crea flujo de puntos de código desde secuencia de caracteres |
| `useSequence(charsequence, start, end)` | Crea flujo de puntos de código desde rango especificado de secuencia de caracteres |
| `useSequence(text, encoding)`   | Crea flujo de puntos de código desde texto con codificación especificada |
| `useSequence(istream, encoding)` | Crea flujo de puntos de código desde flujo de entrada con codificación especificada |
| `useCharsequence(charsequence)` | Trata secuencia de caracteres como flujo completo     |
| `useCharsequence(charsequence, delimiter)` | Divide secuencia de caracteres por delimitador |
| `useCharsequence(istream, encoding)` | Lee secuencia de caracteres completa desde flujo de entrada |
| `useCharsequence(istream, delimiter, encoding)` | Lee secuencia de caracteres desde flujo de entrada por delimitador |

---

## 🔐 Capa 8: hash.h / less.h — El lenguaje universal del mundo de contenedores

Cuando llama a `distinct()` en `Semantic<std::vector<E>>`, internamente necesita `std::unordered_set<std::vector<E>>`. La biblioteca estándar no proporciona especializaciones hash para `std::vector` — este es un "vacío" dejado por el comité de estándares de C++. Los archivos `hash.h` y `less.h` de Semantic-Cpp llenan completamente este vacío: proporcionan soporte completo de hash y comparación para todos los contenedores de biblioteca estándar (incluidos contenedores anidados), pares, tuplas, opcionales, variantes, tipos de tiempo chrono, números complejos, etc. Los contenedores de profundidad arbitraria y combinaciones arbitrarias ahora pueden ser claves en unordered_sets o elementos en sets. 🌉

---

## 🚀 Recomendaciones de optimización de rendimiento

1.  **Elija el contenedor correcto:** Use `toUnordered()` si el orden no importa, `toOrdered()` o `sort()` si se necesita ordenación.
2.  **Aproveche bien el paralelismo:** Use `parallel()` para conjuntos de datos grandes o lógica de procesamiento que consuma mucho tiempo, evite E/S de bloqueo.
3.  **Optimice el orden de operaciones:** `filter` temprano, `sort` sabiamente.
4.  **Aproveche la evaluación perezosa:** Las operaciones intermedias no se ejecutan inmediatamente; `takeWhile` y `limit` pueden terminar temprano.

---

Semantic-Cpp — Construyendo canalizaciones de procesamiento de datos eficientes y claras con C++ moderno. 🚀🎯✨
