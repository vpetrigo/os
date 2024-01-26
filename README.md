## Requirements

- [CMake](https://cmake.org/download/) `>= 3.27`
- GNU GCC
- [Ninja](https://github.com/ninja-build/ninja/releases/tag/v1.11.1)

## Build

- Project configuration

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -S . -B cmake-build-debug
```

- Project build

```bash
cmake --build cmake-build-debug
```
