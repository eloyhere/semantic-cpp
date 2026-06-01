# 🚀 Semantic‑Cpp: Un Marco de Procesamiento de Flujos C++ Orientado al Futuro

Semantic‑Cpp es una biblioteca de procesamiento de flujos C++ moderna y completamente rediseñada que adopta una arquitectura modular de **"múltiples cabeceras, cero dependencias externas"**. Cada archivo de cabecera tiene una responsabilidad clara, es independiente y comprobable, formando en conjunto un ecosistema completo para el procesamiento de flujos de datos. Esta biblioteca fusiona creativamente lo mejor de múltiples paradigmas de programación:

- 🎯 **La elegancia y fluidez de la API de Streams de Java**: encadenamiento de llamadas, programación declarativa, haciendo que el código sea poético.
- ⚡ **La pereza y flexibilidad de los Generadores de JavaScript**: evaluación diferida, generación bajo demanda, amigable con la memoria.
- 🗄️ **La eficiencia y orden de los índices de bases de datos**: ordenamiento inteligente, impulsado por índices, ideal para el procesamiento de datos de series temporales.

A diferencia de los enfoques tradicionales (bucles escritos a mano, callbacks asíncronos), Semantic‑Cpp busca proporcionar una solución **segura de tipos, expresiva y de alto rendimiento**. Su filosofía de diseño central es el **control preciso del flujo de datos**: los datos fluyen solo cuando se necesitan, y el orden y la posición se regulan finamente mediante **"índices"**, logrando una utilización óptima de los recursos.

---

## 📐 Arquitectura del Proyecto: Diseño Modular de Siete Capas

Semantic‑Cpp consta de **siete cabeceras principales**, organizadas progresivamente, cada una con una responsabilidad única y comprobable:

```
┌─────────────────────────────────────────────────┐
│                  semantic.h                     │
│  (operaciones intermedias de flujo, especialización de contenedores, Collectable)
├─────────────────────────────────────────────────┤
│                 semantics.h                     │
│   (constructores de flujos, métodos de fábrica, procesamiento de texto)
├─────────────────────────────────────────────────┤
│                 collectors.h                    │
│  (fábricas de colectores: count, group, reduce, DFT, etc.)
├─────────────────────────────────────────────────┤
│                 collector.h                     │
│   (marco de colectores: modelo de cinco etapas, soporte de concurrencia)
├─────────────────────────────────────────────────┤
│                charsequence.h                   │
│  (secuencias de caracteres Unicode, conversión de codificación, soporte regex)
├─────────────────────────────────────────────────┤
│                   pool.h                        │
│  (grupo de hilos: apagado de emergencia, propagación de excepciones)
├─────────────────────────────────────────────────┤
│                 function.h                      │
│   (definiciones de tipos: Generator, Supplier, Consumer, etc.)
└─────────────────────────────────────────────────┘
```

### Relaciones de Dependencia

```
function.h          ← sin dependencias, piedra angular de tipos
pool.h              ← depende de function.h
charsequence.h      ← módulo independiente, manejo de Unicode
collector.h         ← depende de function.h, pool.h
collectors.h        ← depende de collector.h, charsequence.h
semantic.h          ← depende de collector.h, collectors.h, charsequence.h
semantics.h         ← depende de semantic.h
```

> Cada cabecera puede compilarse y probarse de forma independiente, o incluirse según sea necesario.  
> Por ejemplo, si solo requiere funcionalidades de recolección, basta con incluir `collector.h` y `collectors.h`.

---

## 🏗️ Capa 1: `function.h` — Fundamento de Tipos

`function.h` define el sistema de tipos de toda la biblioteca y sirve como base común para todos los módulos:

```cpp
namespace function {
    using Timestamp = long long;           // tipo de índice
    using Module = unsigned long long;     // tipo de módulo / contador
    using Generator = BiConsumer<
        BiConsumer<T, Timestamp>,
        BiPredicate<T, Timestamp>
    >;
}
```

`Generator` es la abstracción central de todo el sistema de flujos: acepta dos callbacks —`accept` (para recibir datos) e `interrupt` (para terminar el flujo)— encarnando el modelo de **"extracción perezosa"**.

---

## ⚡ Capa 2: `pool.h` — Cimientos de la Concurrencia

`pool.h` proporciona el grupo de hilos global `pool::pool`, que actúa como el motor de concurrencia del marco.

| Característica | Descripción |
|--------------|-------------|
| 🎯 Paralelismo declarativo | `parallel(n)` solo declara la intención; la ejecución comienza en las operaciones terminales |
| 🛡️ Apagado de emergencia | Incluye `emergencyShutdown()` y un manejador `std::set_terminate` |
| 🔄 Propagación de excepciones | `submit()` devuelve `std::future`, soportando propagación segura de excepciones |

---

## 🔤 Capa 3: `charsequence.h` — Secuencias de Caracteres Unicode

`charsequence.h` es un módulo completo para el manejo de Unicode.

| Capacidad | Descripción |
|---------|-------------|
| 🌐 Soporte multiformato | UTF‑8, UTF‑16, UTF‑32, Latin‑1, GBK, etc. |
| 🔍 Iterador de puntos de código | `PointIterator` soporta recorrido bidireccional |
| 🏗️ Patrón Constructor | Clase `Builder` para concatenación eficiente de cadenas |
| 📐 Expresiones regulares | Clase `Regex` envuelve `std::regex` |
| 🔑 Hash y comparación | Todos los tipos principales están especializados para `std::hash` y `std::less` |

---

## 🔧 Capa 4: `collector.h` — Marco de Colectores

`collector.h` implementa el patrón de colector y sirve como el motor central de las operaciones terminales.

### Modelo de Cinco Etapas

```
Identity → Accumulator → Combiner → Finisher
              ↑
           Interrupt (cortocircuito opcional)
```

### Alias de Tipos

| Tipo | Definición | Rol |
|----|-----------|------|
| `Identity<A>` | `Supplier<A>` | Proporciona el valor inicial |
| `Accumulator<A,E>` | `TriFunction<A,E,Timestamp,A>` | Acumula elementos |
| `Combiner<A>` | `BiFunction<A,A,A>` | Combina resultados paralelos |
| `Finisher<A,R>` | `Function<A,R>` | Transformación final |
| `Interrupt<E,A>` | `TriPredicate<E,Timestamp,A>` | Condición de cortocircuito |

### Soporte de Concurrencia

`Collector::collect()` gestiona automáticamente:

- 📦 Particionamiento de datos (distribuidos a hilos vía módulo de índice)
- 🔗 Fusión de resultados (vía `Combiner`)
- ⚠️ Propagación de excepciones (vía `std::exception_ptr` y `std::atomic<bool>`)

---

## 🏭 Capa 5: `collectors.h` — Fábrica de Colectores

`collectors.h` ofrece una amplia gama de funciones de fábrica de colectores predefinidas.

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
| `useFindAt(index)` | Encuentra el elemento en el índice dado (admite negativos) | `std::optional<E>` |
| `useFindMaximum()` | Encuentra el máximo | `std::optional<E>` |
| `useFindMinimum()` | Encuentra el mínimo | `std::optional<E>` |

### 📈 Operaciones de Agregación

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useCount()` | Cuenta el número total de elementos | `Module` |
| `useSummate<E,D>()` | Suma | `D` |
| `useSummate<E,D>(mapper)` | Suma mapeada | `D` |
| `useAverage<E,D>()` | Promedio | `D` |
| `useAverage<E,D>(mapper)` | Promedio mapeado | `D` |
| `useRange<E,D>()` | Rango numérico | `D` |
| `useRange<E,D>(mapper)` | Rango numérico mapeado | `D` |
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
| `useMode<E>()` | Moda | `std::optional<E>` |
| `usePercentile<E,D>(p)` | Percentil p | `std::optional<D>` |
| `useFrequency<E>()` | Características del dominio de frecuencia | `std::map<E, complex>` |
| `useDistribution<E>()` | Características de distribución espacial | `std::map<E, complex>` |

### 🔗 Operaciones de Reducción

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useReduce(reducer)` | Reducción sin identidad | `std::optional<E>` |
| `useReduce(identity, reducer)` | Reducción con identidad | `E` |
| `useReduce(id, red, comb, fin)` | Reducción totalmente personalizada | `R` |

### 📦 Operaciones de Recogida

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useToVector()` | Recoge en `vector` | `std::vector<E>` |
| `useToList()` | Recoge en `list` | `std::list<E>` |
| `useToSet()` | Recoge en `set` (elimina duplicados) | `std::set<E>` |
| `useToMap(keyExtractor)` | Recoge en `map` | `std::map<K,E>` |
| `useGroup(keyExtractor)` | Agrupa por clave | `std::unordered_map<K,vector<E>>` |
| `usePartition(size)` | Particiona por tamaño | `std::vector<vector<E>>` |

### 🎨 Operaciones de Salida

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useJoin()` | Une en cadena de texto | `Charsequence` |
| `useOut()` | Envía a stdout | `Charsequence` |
| `useError()` | Envía a stderr | `Charsequence` |

### 🧮 Utilidades Matemáticas

| Método | Descripción | Tipo de Retorno |
|------|-------------|-----------------|
| `useDFT()` | Transformada Discreta de Fourier | `vector<complex<double>>` |
| `useIDFT()` | Transformada Inversa de Fourier | `vector<complex<double>>` |
| `useFFT()` | Transformada Rápida de Fourier (Cooley‑Tukey) | `vector<complex<double>>` |
| `useIFFT()` | Transformada Rápida de Fourier Inversa | `vector<complex<double>>` |
| `useGradient(gradFunc, lr, iter, th)` | Descenso de gradiente (gradiente analítico) | `vector<double>` |
| `useGradient(costFunc, lr, iter, th, h)` | Descenso de gradiente (gradiente numérico) | `vector<double>` |

---

## 🌊 Capa 6: `semantic.h` — Operaciones Intermedias de Flujo

`semantic.h` es el núcleo del marco, que comprende los espacios de nombres `collectable` y `semantic`.

### Espacio de Nombres `collectable`

Proporciona una jerarquía de herencia para objetos recolectables.

| Clase | Descripción | Almacenamiento Subyacente |
|------|-------------|----------------------------|
| `Collectable<E>` | Clase base abstracta con `source()` virtual pura | — |
| `OrderedCollectable<E>` | Colección ordenada | `std::map<Timestamp, E>` |
| `UnorderedCollectable<E>` | Colección desordenada | `std::unordered_map<Timestamp, E>` |
| `Statistics<E, D>` | Colección estadística (hereda de `OrderedCollectable`) | 20+ métodos estadísticos |
| `WindowCollectable<E>` | Colección basada en ventanas | Soporta ventanas deslizantes / tumbling |

#### Métodos de la Clase `Statistics`

| Método | Tipo de Retorno | Descripción |
|------|-----------------|-------------|
| `summate()` / `summate(mapper)` | `D` | Suma |
| `average()` / `average(mapper)` | `D` | Promedio |
| `minimum()` / `minimum(mapper)` | `std::optional<D>` | Mínimo |
| `maximum()` / `maximum(mapper)` | `std::optional<D>` | Máximo |
| `range()` / `range(mapper)` | `D` | Rango (máx − mín) |
| `variance()` / `variance(mapper)` | `D` | Varianza poblacional |
| `standardDeviation()` / `standardDeviation(mapper)` | `D` | Desviación estándar |
| `frequency()` / `frequency(mapper)` | `std::map<*, complex>` | Características de frecuencia |
| `distribute()` / `distribute(mapper)` | `std::map<*, complex>` | Características de distribución espacial |
| `median()` / `median(mapper)` | `std::optional<D>` | Mediana |
| `mode()` | `std::optional<E>` | Moda |
| `percentile(p)` / `percentile(p, mapper)` | `std::optional<D>` | Percentil p |
| `firstQuartile()` / `firstQuartile(mapper)` | `std::optional<D>` | Primer cuartil (Q1) |
| `thirdQuartile()` / `thirdQuartile(mapper)` | `std::optional<D>` | Tercer cuartil (Q3) |
| `interquartileRange()` / `interquartileRange(mapper)` | `std::optional<D>` | Rango intercuartílico (Q3 − Q1) |
| `skewness()` / `skewness(mapper)` | `D` | Asimetría |
| `kurtosis()` / `kurtosis(mapper)` | `D` | Curtosis |
| `dft()` | `vector<complex<double>>` | Transformada Discreta de Fourier |
| `idft()` | `vector<complex<double>>` | Transformada Inversa de Fourier |
| `fft()` | `vector<complex<double>>` | Transformada Rápida de Fourier |
| `ifft()` | `vector<complex<double>>` | Transformada Rápida de Fourier Inversa |
| `gradient(gradFunc, lr, iter, th)` | `vector<double>` | Descenso de gradiente (analítico) |
| `gradient(costFunc, lr, iter, th, h)` | `vector<double>` | Descenso de gradiente (numérico) |

### Espacio de Nombres `semantic`

Proporciona la clase plantilla `Semantic<E>` y su sistema completo de especializaciones.

#### Resumen de Métodos de la Plantilla Principal

| Categoría | Métodos |
|---------|---------|
| 🎨 Transformación de elementos | `map`, `flatMap`, `flat` |
| 🔍 Filtrado de elementos | `filter`, `takeWhile`, `dropWhile`, `distinct` |
| 📏 Control de cantidad | `limit`, `skip`, `sub` |
| 📐 Manipulación de índices | `redirect`, `reverse`, `translate`, `sort` |
| 👀 Observación | `peek` |
| ⚡ Declaración paralela | `parallel` |
| 🔗 Concatenación | `concatenate` |
| 📤 Conversiones terminales | `toUnordered`, `toOrdered`, `toWindow`, `toStatistics` |

#### Soporte Completo de Especializaciones de Contenedores

| Especialización | Descripción |
|-----------------|-------------|
| `Semantic<std::vector<E>>` | Flujo de vector |
| `Semantic<std::list<E>>` | Flujo de lista |
| `Semantic<std::set<E>>` | Flujo de conjunto ordenado |
| `Semantic<std::unordered_set<E>>` | Flujo de conjunto desordenado |
| `Semantic<std::deque<E>>` | Flujo de deque |
| `Semantic<std::queue<E>>` | Flujo de cola |
| `Semantic<std::stack<E>>` | Flujo de pila |
| `Semantic<std::map<K,V>>` | Flujo de mapa |
| `Semantic<std::unordered_map<K,V>>` | Flujo de mapa desordenado |
| `Semantic<std::initializer_list<E>>` | Flujo de lista de inicializadores |
| `Semantic<Semantic<E>>` | Aplanamiento de flujos anidados |

---

## 🏭 Capa 7: `semantics.h` — Constructores de Flujos

`semantics.h` proporciona todas las funciones de fábrica para construir flujos.

### 📐 Rangos Numéricos

| Método | Descripción |
|------|-------------|
| `useRange(start, end)` | Genera valores en el rango `[start, end)` |

### 📦 Construcción de Contenedores

| Método | Descripción |
|------|-------------|
| `useFrom(container)` | Crea un flujo desde cualquier contenedor estándar |
| `useOf(args...)` | Crea un flujo desde argumentos variables |

### 📝 Procesamiento de Texto

| Método | Descripción |
|------|-------------|
| `useBlob(text)` | Divide la cadena en un flujo de `char` por bytes |
| `useText(text)` | Trata la cadena como un flujo de texto completo |
| `useText(text, delimiter)` | Divide el texto por delimitador |
| `useText(istream)` | Lee todo el contenido desde un flujo de entrada |

### 🌐 Manejo de Unicode

| Método | Descripción |
|------|-------------|
| `useSequence(charsequence)` | Crea un flujo de puntos de código desde una secuencia de caracteres |
| `useSequence(text, encoding)` | Crea un flujo de puntos de código con codificación específica |
| `useCharsequence(charsequence)` | Trata la secuencia de caracteres como un flujo completo |
| `useCharsequence(charsequence, delimiter)` | Divide la secuencia de caracteres por delimitador |

---

## 🧠 Concepto Central: El Mundo de Datos Impulsado por Índices

Semantic‑Cpp abstrae el procesamiento de datos como operaciones sobre **"elementos"** y sus **"posiciones lógicas (índices)"**. Comprender esto es la clave para dominar la biblioteca.

### 1. 📐 Transformaciones Básicas de Índices

| Método | Descripción |
|------|-------------|
| `redirect(fn)` | Método central: reescribe completamente los índices mediante una función personalizada |
| `reverse()` | Invierte toda la lógica de índices (implementado internamente vía `redirect`) |
| `translate(offset)` | Desplazamiento fijo |
| `translate(translator)` | Función de desplazamiento dinámico |

### 2. 📊 La Regla "Autoritaria" del Ordenamiento

⚠️ **`sort()` sobrescribe todo**: tras invocarlo, todas las manipulaciones previas de índices se descartan y los elementos reciben nuevos índices de orden natural basados en sus valores.

- `sort()` → materializado inmediatamente como `OrderedCollectable`
- `sort(comparator)` → ordenado usando un comparador personalizado

### 3. ⚡ Procesamiento Paralelo Declarativo

- `parallel(n)` **solo declara la intención**, no inicia hilos de inmediato
- Las operaciones terminales (`toUnordered()`, `count()`, etc.) activan realmente el paralelismo
- El grupo de hilos gestiona automáticamente la distribución de tareas y la fusión de resultados

### 4. 🎯 Cómo Elegir el Contenedor Final Adecuado

| Conversión | Estructura Subyacente | Características de Rendimiento | Casos de Uso Ideales |
|-----------|----------------------|--------------------------------|----------------------|
| `sort()` | `OrderedCollectable` | Materializado tras ordenar, preserva orden de valores | Ordenación por valor, paginación, series temporales |
| `toOrdered()` | `OrderedCollectable` | Preserva el orden de índices actual | Mantener orden de índices personalizado |
| `toUnordered()` | `UnorderedCollectable` | Promedio O(1), máximo rendimiento | Búsqueda rápida, deduplicación, agregación |
| `toWindow()` | `WindowCollectable` | Basado en colecciones ordenadas | Análisis de ventanas deslizantes / tumbling |
| `toStatistics()` | `Statistics` | 20+ métodos estadísticos | Análisis estadístico exhaustivo |

---

## 🚀 Guía de Inicio Rápido

### Instalación

Coloque todas las cabeceras en el directorio de su proyecto y asegúrese de que su compilador soporte **C++17 o superior**:

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

### 🎯 Ejemplo Básico: Experimentando con Índices y Ordenamiento

```cpp
auto result = semantic::useRange(0, 10)
    .map(int x -> int { return x * x; })
    .redirect(int value, auto index -> long long {
        return index * 2;
    })
    .reverse()
    .sort()                              // fuerza ordenamiento por valor, sobrescribiendo operaciones de índice
    .toVector();

// Salida: 0 1 4 9 16 25 36 49 64 81
```

### ⚡ Ejemplo de Procesamiento Paralelo

```cpp
auto count = semantic::useRange(1, 1000)
    .parallel(4)
    .filter(int x { return x % 2 == 0; })
    .toUnordered()
    .count();

// Salida: Cantidad de pares: 500
```

### 📊 Ejemplo de Análisis Estadístico

```cpp
auto stats = semantic::useRange(1, 101)
    .toStatistics<int, double>();

auto avg = stats.average();               // promedio
auto med = stats.median();                // mediana
auto std = stats.standardDeviation();     // desviación estándar
auto q1  = stats.firstQuartile();         // primer cuartil
auto q3  = stats.thirdQuartile();         // tercer cuartil
auto skew = stats.skewness();             // asimetría
```

### 🔬 Ejemplo de Análisis en el Dominio de la Frecuencia

```cpp
auto freq = data.toUnordered().frequency();
for (const auto& [element, z] : freq) {
    auto magnitude = std::abs(z);  // concentración de la distribución
    auto phase     = std::arg(z);  // fase central de la distribución
}
```

### 🧮 Ejemplo de FFT

```cpp
auto spectrum = semantic::useRange(0, 8)
    .map(int x -> std::complex<double> {
        return {x, 0};
    })
    .toUnordered()
    .collect(collector::useFFT<double>());
```

---

## ⚡ Recomendaciones de Optimización de Rendimiento

1. 🎯 **Elija el contenedor correcto**
   - Búsquedas por igualdad, agregaciones sin orden → `toUnordered()`
   - Consultas de rango, ordenamiento, paginación → `toOrdered()` o `sort()`
   - Análisis de ventanas en tiempo real → `toWindow()`
2. ⚡ **Utilice el paralelismo sabiamente**: aplique `parallel()` para grandes volúmenes de datos o cargas intensivas; evite E/S bloqueante
3. 📐 **Optimice el orden de las operaciones**: aplique `filter` tempranamente, use `sort` juiciosamente
4. 🔄 **Aproveche la evaluación perezosa**: las operaciones intermedias no se ejecutan de inmediato; `takeWhile` y `limit` pueden terminar anticipadamente

---

## 📊 Comparativa con la Biblioteca Estándar de C++ y Alternativas

| Característica | Semantic‑Cpp | C++20/23 Ranges | Bucles Tradicionales |
|---------------|-------------|----------------|----------------------|
| 🎯 Paradigma central | Declarativo, impulsado por índices | Basado en vistas, composición funcional | Imperativo, procedimental |
| ⚡ Soporte paralelo | Declarativo, grupo de hilos automático | Requiere algoritmos paralelos explícitos | Implementación manual |
| 📐 Ordenamiento e índices | Control fino de índices | Ordenamiento destructivo | Totalmente manual |
| 📊 Análisis estadístico | 20+ métodos estadísticos integrados | No incluido | Requiere bibliotecas de terceros |
| 🔬 Análisis de frecuencia | DFT / FFT / características de frecuencia nativos | No soportado de forma nativa | Requiere bibliotecas de terceros |
| 🧮 Descenso de gradiente | Modos analítico y numérico | No incluido | Requiere bibliotecas de terceros |
| 🌐 Unicode | Soporte nativo multiformato | No soportado de forma nativa | Manejo manual |
| 📦 Dependencias | Cero dependencias externas, 7 cabeceras | Biblioteca estándar | Ninguna |

---

## 📜 Licencia y Soporte

- 📄 **Licencia**: Licencia MIT  

---

**Semantic‑Cpp — Construyendo tuberías de procesamiento de datos eficientes y claras con C++ moderno. 🚀**
