# 🚀 Semantic-Cpp: Un Marco de Procesamiento de Flujos Inteligente Preparado para el Futuro

Semantic-Cpp es una biblioteca de procesamiento de flujos en C++ diseñada desde cero con un enfoque moderno. Adopta una arquitectura modular de **"múltiples cabeceras y cero dependencias externas"**. Cada archivo de cabecera tiene una responsabilidad clara, independiente y comprobable, formando juntos un ecosistema completo para el procesamiento de flujos. Esta biblioteca fusiona creativamente lo mejor de múltiples paradigmas de programación:

- 🎯 La elegancia y fluidez de la **API de Streams de Java**: llamadas encadenadas y programación declarativa para un código tan fluido como la poesía.
- ⚡ La pereza y flexibilidad de los **Generadores de JavaScript**: ejecución diferida, generación bajo demanda y eficiencia de memoria.
- 🗄️ La eficiencia y orden de los **índices de bases de datos**: ordenación inteligente, control impulsado por índices, ideal para datos de series temporales.

A diferencia de los enfoques tradicionales de procesamiento de datos (bucles escritos a mano, callbacks asíncronos), Semantic-Cpp pretende proporcionar una solución **segura en tipos, expresiva y de alto rendimiento**. Su filosofía de diseño central es el **control preciso del flujo de datos**: los datos fluyen solo cuando es necesario, y su orden y posición se rigen finamente mediante **"índices"** para optimizar el uso de recursos.

---

## 📐 Arquitectura del Proyecto: Diseño Modular de Siete Capas

Semantic-Cpp consta de **siete archivos de cabecera principales**, dispuestos progresivamente, cada uno con una única responsabilidad independiente:

```
┌─────────────────────────────────────────────────┐
│                 semantics.h                     │
│    (Factorías de flujos: rangos numéricos,      │
│     contenedores, texto, Unicode)               │
├─────────────────────────────────────────────────┤
│                  semantic.h                     │
│   (Operaciones intermedias de flujo, sistema     │
│    Collectable, especializaciones de contenedor) │
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│  (Factorías de colectores: coincidencia, búsqueda,│
│   agregación, estadísticas, DFT/FFT, etc.)       │
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│     (Marco de colectores: modelo de cinco etapas,│
│      soporte de concurrencia y paralelismo)     │
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│  (Secuencias de caracteres Unicode, conversión  │
│   multiformato, Builder, Buffer)                │
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│   (Pool de hilos global: envío de tareas,       │
│    apagado de emergencia, propagación de excepciones) │
├─────────────────────────────────────────────────┤
│                 function.h                      │
│  (Alias de tipos: Generator, Supplier, Consumer, │
│   etc.)                                         │
└─────────────────────────────────────────────────┘
```

### Grafo de Dependencias

```
function.h          ← sin dependencias, piedra angular de tipos
pool.h              ← depende de function.h
charsequence.h      ← módulo independiente, manejo de Unicode
collector.h         ← depende de function.h, pool.h
collectors.h        ← depende de collector.h, charsequence.h
semantic.h          ← depende de collector.h, collectors.h, charsequence.h
semantics.h         ← depende de semantic.h
```

Cada cabecera puede compilarse y probarse de forma independiente o incluirse según sea necesario.  
Por ejemplo, si solo requiere funcionalidades de recolección, basta con incluir `collector.h` y `collectors.h`.

---

## 🏗️ Capa Uno: function.h — Fundamento de Tipos

`function.h` define el sistema de tipos que sustenta toda la biblioteca, la base común de todos los módulos:

```cpp
namespace function {
    using Timestamp = long long;           // tipo de índice
    using Module = unsigned long long;     // tipo de módulo / contador
    
    template <typename T>
    using Generator = std::function<void(
        std::function<void(T, Timestamp)>,      // accept
        std::function<bool(T, Timestamp)>       // interrupt
    )>;
}
```

`Generator` es la abstracción central de todo el sistema de flujos: acepta dos callbacks — `accept` (para recibir datos) e `interrupt` (para detener el procesamiento) — encarnando el modelo de **"extracción perezosa" (lazy pull)**.

---

## ⚡ Capa Dos: pool.h — Piedra Angular de la Concurrencia

`pool.h` proporciona el pool de hilos global `pool::pool`, que actúa como el motor de concurrencia del marco:

| Característica | Descripción |
|--------------|-------------|
| 🎯 Paralelismo declarativo | `parallel(n)` solo declara la intención; se activa automáticamente en las operaciones terminales |
| 🛡️ Apagado de emergencia | Incluye `emergencyShutdown()` y un manejador `std::set_terminate` |
| 🔄 Propagación de excepciones | `submit()` devuelve `std::future`, permitiendo una propagación segura de excepciones |

---

## 🔤 Capa Tres: charsequence.h — Secuencias de Caracteres Unicode

`charsequence.h` es un módulo integral para el manejo de Unicode, que ofrece creación, conversión y manipulación de secuencias de caracteres:

| Capacidad | Descripción |
|----------|-------------|
| 🌐 Soporte multiformato | UTF‑8, UTF‑16 (LE/BE), UTF‑32 (LE/BE), ASCII, Latin1 |
| 🔍 Iterador de puntos de código | `PointIterator` soporta el recorrido bidireccional de puntos de código Unicode |
| 🏗️ Patrón Builder | La clase `Builder` permite concatenaciones eficientes de bytes |
| 📦 Buffer | La clase `Buffer` proporciona un búfer circular seguro para hilos |
| 🔑 Hash y comparación | Todos los tipos principales tienen especializaciones de `std::hash` y `std::less` |

### Tipos Principales

| Tipo | Descripción |
|-----|-------------|
| `Meta` | Envoltorio de metadatos para almacenar valores enteros sin signo |
| `Point` | Punto de código Unicode con detección de parejas suplentes y validación |
| `Charsequence` | Secuencia de caracteres inmutable que soporta división, reemplazo, búsqueda, cambio de mayúsculas/minúsculas, etc. |
| `Builder` | Constructor de bytes mutable con `prepend`, `insert`, `append` y varios tipos de datos |
| `Buffer` | Búfer circular seguro para hilos con soporte de lectura/escritura, prefetch y gestión de capacidad |

---

## 🔧 Capa Cuatro: collector.h — Marco de Colectores

`collector.h` implementa el patrón Collector, formando el motor central tras las operaciones terminales.

### Modelo de Cinco Etapas

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (cortocircuito opcional)
```

### Alias de Tipos

| Tipo | Definición | Rol |
|-----|-----------|-----|
| `Identity<A>` | `Supplier<A>` | Proporciona el valor inicial |
| `Accumulator<A,E>` | `TriFunction<A,E,Timestamp,A>` | Acumula elementos |
| `Combiner<A>` | `BiFunction<A,A,A>` | Combina resultados paralelos |
| `Finisher<A,R>` | `Function<A,R>` | Transformación final |
| `Interrupt<E,A>` | `TriPredicate<E,Timestamp,A>` | Evaluación de cortocircuito |

### Soporte de Concurrencia

`Collector::collect()` gestiona automáticamente:

- 📦 Particionamiento de datos (distribuidos entre hilos mediante módulo de índice)
- 🔗 Fusión de resultados (a través de `Combiner`)
- ⚠️ Propagación de excepciones (mediante `std::exception_ptr` y `std::atomic<bool>`)

---

## 🏭 Capa Cinco: collectors.h — Fábrica de Colectores

`collectors.h` ofrece una amplia gama de funciones fábrica de colectores listas para usar.

### 📊 Operaciones de Coincidencia

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useAllMatch(predicate)` | Todos los elementos cumplen la condición | `bool` |
| `useAnyMatch(predicate)` | Algún elemento cumple la condición | `bool` |
| `useNoneMatch(predicate)` | Ningún elemento cumple la condición | `bool` |

### 🔍 Operaciones de Búsqueda

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useFindFirst()` | Encuentra el primer elemento | `std::optional<E>` |
| `useFindLast()` | Encuentra el último elemento | `std::optional<E>` |
| `useFindAny()` | Encuentra un elemento aleatorio | `std::optional<E>` |
| `useFindAt(index)` | Encuentra el elemento en el índice especificado (soporta índices negativos) | `std::optional<E>` |
| `useFindMaximum()` | Encuentra el máximo (permite comparador personalizado) | `std::optional<E>` |
| `useFindMinimum()` | Encuentra el mínimo (permite comparador personalizado) | `std::optional<E>` |

### 📈 Operaciones de Agregación

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useCount()` | Cuenta el número total de elementos | `Module` |
| `useSummate<E,D>()` | Suma | `D` |
| `useSummate<E,D>(mapper)` | Suma mapeada | `D` |
| `useAverage<E,D>()` | Promedio | `D` |
| `useAverage<E,D>(mapper)` | Promedio mapeado | `D` |
| `useRange<E,D>()` | Rango numérico (máx − mín) | `D` |
| `useRange<E,D>(mapper)` | Rango mapeado | `D` |
| `useMinimum<E,D>()` | Valor mínimo | `std::optional<D>` |
| `useMaximum<E,D>()` | Valor máximo | `std::optional<D>` |

### 📊 Operaciones Estadísticas

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useVariance<E,D>()` | Varianza poblacional | `D` |
| `useStandardDeviation<E,D>()` | Desviación estándar poblacional | `D` |
| `useSkewness<E,D>()` | Asimetría | `D` |
| `useKurtosis<E,D>()` | Curtosis | `D` |
| `useMedian<E,D>()` | Mediana | `std::optional<D>` |
| `useMode<E>()` | Moda (basada en dominio de frecuencia) | `std::optional<E>` |
| `usePercentile<E,D>(p)` | Percentil p | `std::optional<D>` |
| `useFrequency<E>()` | Características de frecuencia (codificación de fase de índice) | `std::map<E, complex>` |
| `useDistribution<E>()` | Características de distribución espacial (codificación de posición) | `std::map<E, complex>` |

### 🔗 Operaciones de Reducción

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useReduce(reducer)` | Reducción sin valor inicial | `std::optional<E>` |
| `useReduce(identity, reducer)` | Reducción con valor inicial | `E` |
| `useReduce(id, red, comb, fin)` | Reducción totalmente personalizada | `R` |

### 📦 Operaciones de Recolección en Contenedores

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useToVector()` | Recoger en `std::vector` | `std::vector<E>` |
| `useToList()` | Recoger en `std::list` | `std::list<E>` |
| `useToDeque()` | Recoger en `std::deque` | `std::deque<E>` |
| `useToForwardList()` | Recoger en `std::forward_list` | `std::forward_list<E>` |
| `useToArray<N>()` | Recoger en `std::array` de tamaño fijo | `std::array<E, N>` |
| `useToSet()` | Recoger en `std::set` (elimina duplicados y ordena) | `std::set<E>` |
| `useToMultiset()` | Recoger en `std::multiset` | `std::multiset<E>` |
| `useToUnorderedSet()` | Recoger en `std::unordered_set` | `std::unordered_set<E>` |
| `useToUnorderedMultiset()` | Recoger en `std::unordered_multiset` | `std::unordered_multiset<E>` |
| `useToMap(keyExtractor)` | Recoger en `std::map` | `std::map<K, E>` |
| `useToMap(keyExtractor, valueExtractor)` | Recoger en `std::map` (valor personalizado) | `std::map<K, V>` |
| `useToMultimap(keyExtractor)` | Recoger en `std::multimap` | `std::multimap<K, E>` |
| `useToMultimap(keyExtractor, valueExtractor)` | Recoger en `std::multimap` (valor personalizado) | `std::multimap<K, V>` |
| `useToUnorderedMap(keyExtractor, valueExtractor)` | Recoger en `std::unordered_map` | `std::unordered_map<K, V>` |
| `useToUnorderedMultimap(keyExtractor)` | Recoger en `std::unordered_multimap` | `std::unordered_multimap<K, E>` |
| `useToUnorderedMultimap(keyExtractor, valueExtractor)` | Recoger en `std::unordered_multimap` | `std::unordered_multimap<K, V>` |
| `useToStack()` | Recoger en `std::stack` | `std::stack<E>` |
| `useToQueue()` | Recoger en `std::queue` | `std::queue<E>` |
| `useToPriorityQueue()` | Recoger en `std::priority_queue` | `std::priority_queue<E>` |

### 🔀 Operaciones de Agrupación y Partición

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useGroup(keyExtractor)` | Agrupar por clave | `std::unordered_map<K, vector<E>>` |
| `usePartition(size)` | Particionar en bloques de tamaño fijo | `std::vector<vector<E>>` |
| `usePartitionBy(keyExtractor)` | Particionar por clave personalizada | `std::vector<vector<E>>` |

### 🎨 Operaciones de Salida de Cadenas

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useJoin()` | Unir en cadena (coma por defecto, corchetes) | `Charsequence` |
| `useJoin(delimiter)` | Unir con delimitador personalizado | `Charsequence` |
| `useJoin(prefix, delimiter, suffix)` | Unión con formato totalmente personalizado | `Charsequence` |
| `useOut()` | Salida formateada a stdout | `Charsequence` |
| `useOut(delimiter)` | Salida con delimitador personalizado a stdout | `Charsequence` |
| `useOut(prefix, delimiter, suffix)` | Salida formateada totalmente personalizada a stdout | `Charsequence` |
| `useError()` | Salida formateada a stderr | `Charsequence` |
| `useError(delimiter)` | Salida con delimitador personalizado a stderr | `Charsequence` |
| `useError(prefix, delimiter, suffix)` | Salida formateada totalmente personalizada a stderr | `Charsequence` |

### 🧮 Utilidades Matemáticas

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useDFT()` | Transformada Discreta de Fourier | `vector<complex<double>>` |
| `useIDFT()` | Transformada Discreta Inversa de Fourier | `vector<complex<double>>` |
| `useFFT()` | Transformada Rápida de Fourier (Cooley–Tukey) | `vector<complex<double>>` |
| `useIFFT()` | Transformada Rápida Inversa de Fourier | `vector<complex<double>>` |
| `useGradient(gradFunc, lr, iter, th)` | Descenso de gradiente (gradiente analítico) | `vector<double>` |
| `useGradient(costFunc, lr, iter, th, h)` | Descenso de gradiente (gradiente numérico) | `vector<double>` |

---

## 🌊 Capa Seis: semantic.h — Operaciones Intermedias de Flujo y Sistema de Recolección

`semantic.h` es el núcleo del marco y contiene los espacios de nombres `collectable` y `semantic`.

### Espacio de Nombres collectable

Proporciona la jerarquía de herencia para objetos recolectables:

| Clase | Descripción | Almacenamiento Subyacente |
|------|-------------|---------------------------|
| `Collectable<E>` | Clase base abstracta con método virtual puro `source()` | — |
| `OrderedCollectable<E>` | Recolección ordenada con ordenamiento personalizado | `std::map<Timestamp, E>` |
| `UnorderedCollectable<E>` | Recolección desordenada, búsqueda O(1) | `std::unordered_map<Timestamp, E>` |
| `Statistics<E, D>` | Recolección estadística (hereda de `OrderedCollectable`) | Más de 20 métodos estadísticos |
| `WindowCollectable<E>` | Recolección en ventanas (hereda de `OrderedCollectable`) | Soporta `slide` / `tumble` |

#### Métodos de la Clase Base Collectable

Proporciona todos los métodos de recolección terminal `toXxx()` (más de 20 tipos de contenedores), además de:

`count()`, `findFirst()`, `findAny()`, `anyMatch()`, `allMatch()`, `noneMatch()`, `reduce()`, `join()`, `out()`, `error()`, `group()`, `partition()`, `partitionBy()`, etc.

#### Métodos de la Clase Statistics

| Método | Tipo de Retorno | Descripción |
|------|-----------------|-------------|
| `summate()` / `summate(mapper)` | `D` | Suma |
| `average()` / `average(mapper)` | `D` | Promedio |
| `minimum()` / `minimum(mapper)` | `std::optional<D>` | Mínimo |
| `maximum()` / `maximum(mapper)` | `std::optional<D>` | Máximo |
| `range()` / `range(mapper)` | `D` | Rango (máx − mín) |
| `variance()` / `variance(mapper)` | `D` | Varianza poblacional |
| `standardDeviation()` / `standardDeviation(mapper)` | `D` | Desviación estándar poblacional |
| `frequency()` / `frequency(mapper)` | `std::map<*, complex>` | Características de frecuencia (codificación de fase de índice) |
| `distribute()` / `distribute(mapper)` | `std::map<*, complex>` | Características de distribución espacial (codificación de posición) |
| `median()` / `median(mapper)` | `std::optional<D>` | Mediana |
| `mode()` | `std::optional<E>` | Moda |
| `percentile(p)` / `percentile(p, mapper)` | `std::optional<D>` | Percentil p |
| `firstQuartile()` / `firstQuartile(mapper)` | `std::optional<D>` | Primer cuartil (Q1) |
| `thirdQuartile()` / `thirdQuartile(mapper)` | `std::optional<D>` | Tercer cuartil (Q3) |
| `interquartileRange()` / `interquartileRange(mapper)` | `std::optional<D>` | Rango intercuartílico (Q3 − Q1) |
| `skewness()` / `skewness(mapper)` | `D` | Asimetría |
| `kurtosis()` / `kurtosis(mapper)` | `D` | Curtosis |
| `dft()` | `vector<complex<double>>` | Transformada Discreta de Fourier |
| `idft()` | `vector<complex<double>>` | Transformada Discreta Inversa de Fourier |
| `fft()` | `vector<complex<double>>` | Transformada Rápida de Fourier |
| `ifft()` | `vector<complex<double>>` | Transformada Rápida Inversa de Fourier |
| `gradient(gradFunc, lr, iter, th)` | `vector<double>` | Descenso de gradiente (gradiente analítico) |
| `gradient(costFunc, lr, iter, th, h)` | `vector<double>` | Descenso de gradiente (gradiente numérico) |

### Espacio de Nombres semantic

Proporciona la plantilla de clase `Semantic<E>` y su sistema completo de especialización.

#### Resumen de Métodos de la Plantilla Principal

| Categoría | Método | Descripción |
|---------|--------|-------------|
| 🎨 Transformación de Elementos | `map` | Mapeo uno a uno |
| | `flatMap` | Mapeo uno a muchos con aplanado |
| | `flat` | Aplanar flujos anidados |
| 🔍 Filtrado de Elementos | `filter` | Filtrado por condición |
| | `takeWhile` | Tomar mientras se cumpla la condición |
| | `dropWhile` | Soltar mientras se cumpla la condición |
| | `distinct` | Eliminar duplicados (permite comparador personalizado) |
| 📏 Control de Cantidad | `limit` | Limitar número de elementos |
| | `skip` | Saltar los primeros n elementos |
| | `sub` | Extraer subrango |
| 📐 Operaciones de Índice | `redirect` | Remapear índices |
| | `reverse` | Invertir índices |
| | `translate` | Desplazar índices (fijo o dinámico) |
| 👀 Observación | `peek` | Observar elementos sin modificarlos |
| ⚡ Declaración de Paralelismo | `parallel` | Declarar grado de paralelismo |
| 🔗 Concatenación | `concatenate` | Concatenar otro flujo o contenedor |
| 📤 Conversión Terminal | `toUnordered` | Convertir a colector desordenado |
| | `toOrdered` | Convertir a colector ordenado |
| | `toWindow` | Convertir a colector de ventanas |
| | `toStatistics` | Convertir a colector estadístico |

#### Soporte Completo de Especialización de Contenedores

| Especialización | Descripción |
|----------------|-------------|
| `Semantic<std::vector<E>>` | Flujo de vector con ordenación, eliminación de duplicados, etc. |
| `Semantic<std::list<E>>` | Flujo de lista con ordenación, eliminación de duplicados, etc. |

---

## 🏭 Capa Siete: semantics.h — Fábrica de Construcción de Flujos

`semantics.h` proporciona todas las funciones fábrica de construcción de flujos.

### 📐 Generación de Rangos Numéricos

| Método | Descripción |
|------|-------------|
| `useRange(start, end)` | Generar flujo numérico sobre `[start, end)` |
| `useRange(start, end, step)` | Generar flujo numérico con paso |
| `useRangeClosed(start, end)` | Generar flujo numérico sobre `[start, end]` |
| `useRangeClosed(start, end, step)` | Generar flujo numérico con paso sobre intervalo cerrado |

### ♾️ Generación de Flujos Infinitos

| Método | Descripción |
|------|-------------|
| `useInfinite(seed, generator)` | Iteración infinita desde semilla |
| `useGenerate(supplier)` | Invocación infinita de proveedor |
| `useGenerate(supplier, limit)` | Invocación limitada de proveedor |
| `useIterate(seed, generator)` | Iteración infinita desde semilla |
| `useIterate(seed, generator, limit)` | Iteración limitada |
| `useRandom()` | Flujo infinito de enteros aleatorios |
| `useRandom(min, max)` | Flujo infinito de aleatorios dentro de rango |
| `useRandom(min, max, count)` | Aleatorios dentro de rango con conteo fijo |

### 📦 Construcción de Contenedores y Elementos

| Método | Descripción |
|------|-------------|
| `useEmpty()` | Crear flujo vacío |
| `useOf(element)` | Flujo de un solo elemento |
| `useOf(e1, e2)` | Flujo de dos elementos |
| `useOf(e1, e2, e3)` | Flujo de tres elementos |
| `useOf({...})` | Flujo desde lista de inicialización |
| `useFrom(container)` | Flujo desde cualquier contenedor estándar |
| `useFrom({...})` | Flujo desde lista de inicialización |
| `useRepeat(element, count)` | Repetir elemento n veces |

### 📝 Procesamiento de Texto

| Método | Descripción |
|------|-------------|
| `useBlob(text)` | Dividir cadena en flujo `char` por bytes |
| `useBlob(text, start, end)` | Dividir subcadena en flujo `char` por bytes |
| `useBlob(istream)` | Leer líneas desde flujo de entrada |
| `useBlob(istream, delimiter)` | Leer flujo de entrada usando delimitador |
| `useText(text)` | Tratar cadena como flujo de texto completo |
| `useText(text, delimiter)` | Dividir texto por delimitador |
| `useText(istream)` | Leer contenido completo del flujo de entrada |
| `useText(istream, delimiter)` | Leer flujo de entrada usando delimitador |

### 🌐 Manejo de Unicode

| Método | Descripción |
|------|-------------|
| `useSequence(charsequence)` | Crear flujo de puntos de código desde secuencia de caracteres |
| `useSequence(charsequence, start, end)` | Crear flujo de puntos de código desde subsecuencia |
| `useSequence(text, encoding)` | Crear flujo de puntos de código desde texto con codificación |
| `useSequence(istream, encoding)` | Crear flujo de puntos de código desde flujo de entrada con codificación |
| `useCharsequence(charsequence)` | Tratar secuencia de caracteres como flujo completo |
| `useCharsequence(charsequence, delimiter)` | Dividir secuencia de caracteres por delimitador |
| `useCharsequence(istream, encoding)` | Leer secuencia de caracteres completa desde flujo de entrada |
| `useCharsequence(istream, delimiter, encoding)` | Leer secuencia de caracteres desde flujo de entrada usando delimitador |

---

## 🧠 Concepto Central: Un Mundo de Datos Impulsado por Índices

Semantic-Cpp abstrae el procesamiento de datos como operaciones sobre **"elementos"** y sus **"posiciones lógicas (índices)"**. Comprender esto es clave para dominar la biblioteca.

### 1. 📐 Transformaciones Básicas de Índices

| Método | Descripción |
|------|-------------|
| `redirect(fn)` | Método central: reasignar índices completamente mediante función personalizada |
| `reverse()` | Invertir toda la lógica de índices (implementado internamente mediante `redirect`) |
| `translate(offset)` | Desplazamiento fijo |
| `translate(translator)` | Función de desplazamiento dinámico calculada por elemento e índice |

### 2. 📊 La Regla "Autoritaria" del Ordenamiento

> ⚠️ **`sort()` sobrescribe todo**: Tras la llamada, todas las operaciones de índice previas serán descartadas; los elementos reciben nuevos índices de orden natural basados en su valor.

- `sort()` → Materializado inmediatamente en `OrderedCollectable`, ordenado naturalmente por valor de elemento
- `sort(comparator)` → Ordenado usando comparador personalizado

### 3. ⚡ Procesamiento Paralelo Declarativo

- `parallel(n)` solo declara la intención; los hilos no se inician de inmediato
- Las operaciones terminales (`toUnordered()`, `count()`, etc.) activan realmente el paralelismo
- El pool de hilos gestiona automáticamente la distribución de tareas y la fusión de resultados

### 4. 🎯 Cómo Elegir el Contenedor Final Correcto

| Método de Conversión | Estructura Subyacente | Características de Rendimiento | Caso de Uso Ideal |
|---------------------|----------------------|-------------------------------|-------------------|
| `sort()` | `OrderedCollectable` | Materializado y ordenado por valor | Ordenación por valor, paginación, series temporales |
| `toOrdered()` | `OrderedCollectable` | Conserva el orden de índices actual | Mantener orden de índices personalizado |
| `toUnordered()` | `UnorderedCollectable` | Promedio O(1), máximo rendimiento | Búsqueda rápida, deduplicación, agregación |
| `toWindow()` | `WindowCollectable` | Basado en colecciones ordenadas | Análisis de ventanas deslizantes / rodantes |
| `toStatistics()` | `Statistics` | Más de 20 métodos estadísticos | Análisis estadístico exhaustivo |

---

## 🚀 Guía de Inicio Rápido

### Instalación

Coloque todos los archivos de cabecera en el directorio de su proyecto y asegúrese de que su compilador soporte **C++17 o superior**:

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
#include "semantics.h"  // incluye automáticamente todas las dependencias
```

---

## 🎯 Ejemplo Básico: Experimentar con Índices y Ordenamiento

```cpp
auto result = semantic::useRange(0, 10)
    .map(int x { return x * x; })
    .redirect(int value, auto index -> long long {
        return index * 2;
    })
    .reverse()
    .sort()                              // fuerza ordenamiento por valor, sobrescribe todas las operaciones de índice
    .toVector();

// Salida: 0 1 4 9 16 25 36 49 64 81
```

## ⚡ Ejemplo de Procesamiento Paralelo

```cpp
auto count = semantic::useRange(1, 1000)
    .parallel(4)
    .filter(int x { return x % 2 == 0; })
    .toUnordered()
    .count();

// Salida: Cantidad de números pares: 500
```

## 📊 Ejemplo de Análisis Estadístico

```cpp
auto stats = semantic::useRange(1, 101)
    .toStatistics<int, double>();

auto avg = stats.average();               // media
auto med = stats.median();                // mediana
auto std = stats.standardDeviation();     // desviación estándar
auto q1  = stats.firstQuartile();          // primer cuartil
auto q3  = stats.thirdQuartile();          // tercer cuartil
auto skew = stats.skewness();              // asimetría
```

## 🔬 Ejemplo de Análisis en el Dominio de la Frecuencia

```cpp
auto freq = data.toUnordered().frequency();
for (const auto& [element, z] : freq) {
    auto magnitude = std::abs(z);  // concentración de la distribución
    auto phase     = std::arg(z);  // fase central de la distribución
}
```

## 🧮 Ejemplo de FFT

```cpp
auto spectrum = semantic::useRange(0, 8)
    .map(int x -> std::complex<double> {
        return {static_cast<double>(x), 0.0};
    })
    .toUnordered()
    .collect(collector::useFFT<double>());
```

---

## ⚡ Recomendaciones de Optimización del Rendimiento

1. 🎯 **Elija el contenedor adecuado**
   - Búsquedas por igualdad, agregación sin ordenar → `toUnordered()`
   - Consultas de rango, ordenación, paginación → `toOrdered()` o `sort()`
   - Análisis de ventanas en tiempo real → `toWindow()`
2. ⚡ **Aproveche el paralelismo sabiamente**: use `parallel()` para grandes conjuntos de datos o lógica computacionalmente costosa; evite la E/S bloqueante.
3. 📐 **Optimice el orden de las operaciones**: filtre temprano, ordene con criterio.
4. 🔄 **Aproveche la evaluación perezosa**: las operaciones intermedias no se ejecutan hasta la terminal; `takeWhile` y `limit` permiten terminación anticipada.

---

## 📊 Comparativa con la Biblioteca Estándar de C++ y Alternativas

| Característica | Semantic-Cpp | C++20/23 Ranges | Bucles Tradicionales |
|--------------|-------------|-----------------|---------------------|
| 🎯 Paradigma Central | Declarativo, impulsado por índices | Basado en vistas, composición funcional | Imperativo, procedimental |
| ⚡ Soporte Paralelo | Declarativo, pool de hilos automático | Requiere combinar algoritmos paralelos | Implementación manual |
| 📐 Ordenación e Indexación | Control fino de índices, soporta índices negativos | Ordenación destructiva | Totalmente manual |
| 📊 Análisis Estadístico | Más de 20 métodos estadísticos integrados | No integrado | Requiere bibliotecas de terceros |
| 🔬 Análisis en Frecuencia | DFT / FFT / características de frecuencia nativas | No soportado de forma nativa | Requiere bibliotecas de terceros |
| 🧮 Descenso de Gradiente | Modos analítico y numérico | No integrado | Requiere bibliotecas de terceros |
| 🌐 Unicode | Soporte nativo multiformato (UTF‑8/16/32, etc.) | No soportado de forma nativa | Manejo manual |
| 📦 Recolección en Contenedores | Más de 20 contenedores estándar cubiertos | Soporte parcial | Implementación manual |
| 📦 Dependencias | Cero dependencias externas, 7 cabeceras | Biblioteca estándar | Ninguna |

---

## 📜 Licencia

- 📄 **Licencia**: MIT

---

**Semantic-Cpp — Construyendo pipelines de procesamiento de datos eficientes y claros con C++ moderno. 🚀**
