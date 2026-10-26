# fit_natural_cpp

Independent C++17 fitter for the natural-calendar data.  It reads the binary
event tables produced by `tools/fit_natural.py` and solves the same two compact
least-squares models as the Python reference, using chunked normal equations and
a Cholesky solve.  No third-party libraries are required.

## Build

Windows / MSVC (from the repository root):

```powershell
& "R:\visual studio\ide\VC\Auxiliary\Build\vcvars64.bat"
cmake -S tools/fit_natural_cpp -B build-natural-fit -G Ninja
cmake --build build-natural-fit
```

Linux / macOS:

```sh
cmake -S tools/fit_natural_cpp -B build-natural-fit -G Ninja
cmake --build build-natural-fit
```

## Run

```powershell
# Generate temporal/natural/data/cpp/*.bin first:
python tools/fit_natural.py extract

build-natural-fit/fit_natural_cpp.exe temporal/natural/data temporal/natural/data
```

Outputs `temporal/natural/data/cpp_coefficients.json` and
`cpp_metrics.json`.  `tools/fit_natural.py fit` uses the C++ coefficients when
their RMS agrees with the Python reference (relative difference < 1e-3).

## Data format

`cpp/*.bin` files start with the 8-byte magic `NFIT0001`, then an `int64`
count, then the arrays in the order documented in `main.cpp`.

- `sun_events.bin`: `double jd`, `double angle`, `int32 year`, `int32 month`,
  `int32 kind`
- `moon_events.bin`: `double jd`, `double angle`, `int32 year`, `int32 month`

The event tables are extracted from `寿星数据整录——原版.xlsx` with the
historical Julian/Gregorian cutover (1582-10-15) and UTC+8 time.
