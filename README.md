# TSExam

C++20 / CMake project: Problem 1 (triangle mesh, STL I/O, void detection) and Problem 2 (polyline).

## Prerequisites

- CMake 3.14+
- C++20 compiler (GCC 10+, Clang 10+, or MSVC 2019+)
- Git (used by CMake FetchContent for Google Test)

## Build

From the project root:

```bash
cmake -B build -S .
cmake --build build
```

- **Windows (MSVC):** add `--config Debug` or `--config Release` to the build step. Executables are under `build\Debug\` or `build\Release\`.
- **Release (Linux/macOS):** configure with `-DCMAKE_BUILD_TYPE=Release`.

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

Or run the test executables directly:

```bash
./build/problem1_tests
./build/problem2_tests
```

On Windows: `build\Debug\problem1_tests.exe` and `build\Debug\problem2_tests.exe`.

## Project layout

- `src/problem_1/`, `src/problem_2/` — libraries (`mesh`, `polyline`)
- `tests/problem_1/`, `tests/problem_2/` — test sources
- Google Test is fetched automatically via CMake FetchContent.
