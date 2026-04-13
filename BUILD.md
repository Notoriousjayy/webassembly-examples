# Build Guide

This project supports WebAssembly builds with Emscripten and native desktop/mobile
builds through CMake presets. The codebase is configured for **C23**, not C99.

## Prerequisites

### Common requirements

- **CMake** 3.20 or higher
- **Ninja** (recommended) or another supported generator
- A **C23-capable compiler**
  - GCC 13+
  - Clang 16+
  - MSVC with C23 support enabled for this project configuration

### Native renderer requirements

Native renderer builds require **SDL3** to be installed and discoverable by CMake.
Unlike some earlier documentation, the current `CMakeLists.txt` does **not**
download SDL3 automatically.

If SDL3 is installed in a non-standard location, configure CMake with one of:

```bash
-DCMAKE_PREFIX_PATH=/path/to/sdl3
-DSDL3_DIR=/path/to/sdl3/lib/cmake/SDL3
```

If you only want the math/core modules, use the `native-no-sdl` preset instead.

### WebAssembly requirements

Install and activate the Emscripten SDK:

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

## Quick start

### Native debug build with SDL3 + OpenGL

```bash
cmake --preset native-debug
cmake --build --preset native-debug
./build-native/testProject
```

### Native debug build without SDL3

```bash
cmake --preset native-no-sdl
cmake --build --preset native-no-sdl
./build-native/testProject
```

### WebAssembly debug build

```bash
source /path/to/emsdk/emsdk_env.sh
emcmake cmake --preset wasm-debug
cmake --build --preset wasm-debug
cmake --build build-wasm --target serve
# Open http://localhost:8000
```

### WebAssembly release build

```bash
source /path/to/emsdk/emsdk_env.sh
emcmake cmake --preset wasm-release
cmake --build --preset wasm-release
```

## Presets

### Canonical presets

| Preset | Platform | Graphics | Build type | Description |
|--------|----------|----------|------------|-------------|
| `wasm-debug` | WebAssembly | WebGL2 | Debug | Debug-friendly WASM build |
| `wasm-release` | WebAssembly | WebGL2 | Release | Optimized WASM build |
| `native-debug` | Desktop | SDL3 + OpenGL | Debug | Native desktop renderer build |
| `native-release` | Desktop | SDL3 + OpenGL | Release | Optimized native desktop build |
| `native-no-sdl` | Desktop | None | Debug | Core/math build without renderer |
| `android-debug` | Android | SDL3 + OpenGL ES 3.0 | Debug | Android debug build |
| `android-release` | Android | SDL3 + OpenGL ES 3.0 | Release | Android release build |
| `ios-debug` | iOS | SDL3 + OpenGL ES 3.0 | Debug | iOS debug build |
| `ios-release` | iOS | SDL3 + OpenGL ES 3.0 | Release | iOS release build |

### Legacy preset aliases

The preset file also preserves these compatibility aliases for older scripts and
notes:

- `native-sdl3-debug` → `native-debug`
- `native-sdl3-release` → `native-release`
- `native-headless` → `native-no-sdl`

## Platform notes

### Linux packages

```bash
sudo apt install build-essential cmake ninja-build \
                 libgl1-mesa-dev libxext-dev
```

### macOS tools

```bash
xcode-select --install
brew install cmake ninja
```

### Windows

Visual Studio or MinGW/MSYS2 can be used, but the preset names remain the same:

```powershell
cmake --preset native-debug
cmake --build --preset native-debug
```

## Useful commands

List presets:

```bash
cmake --list-presets
```

Clean build directories:

```bash
rm -rf build-*
```

Use stronger release optimization for WASM:

```bash
emcmake cmake --preset wasm-release -DCMAKE_C_FLAGS="-O3 -flto"
```

Minimize binary size for WASM:

```bash
emcmake cmake --preset wasm-release -DCMAKE_C_FLAGS="-Oz -flto"
```

## Troubleshooting

### `emcmake: command not found`

Activate Emscripten first:

```bash
source /path/to/emsdk/emsdk_env.sh
emcc --version
```

### SDL3 not found

Install SDL3 and point CMake at it explicitly:

```bash
cmake --preset native-debug -DCMAKE_PREFIX_PATH=/usr/local
```

Or use the non-renderer preset:

```bash
cmake --preset native-no-sdl
```

### WebGL context creation failed

- Verify that the browser supports WebGL2
- Check the browser console for details
- Try Chrome, Firefox, or Edge

### No C compiler found

Install a C toolchain:

```bash
sudo apt install build-essential
```

## Continuous integration

The repository includes workflows for:

- WebAssembly builds
- GitHub Pages deployment
- CodeQL scanning
- Release packaging
- Dependency review

See `.github/workflows/` for the concrete workflow definitions.
