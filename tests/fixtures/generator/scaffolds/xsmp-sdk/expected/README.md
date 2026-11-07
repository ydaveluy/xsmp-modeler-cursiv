# project-name

Project description.

## System Requirements

Check [xsmp-sdk system requirements](https://thalesgroup.github.io/xsmp-sdk/requirements.html).

## How to Build

```bash
cmake -B ./build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build --config Release
```

## How to Test

```bash
cd build && ctest -C Release --output-on-failure
```
