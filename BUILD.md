# Build Guide

This project is now **WebAssembly-only** and targets the browser through
**Emscripten + SDL3 + OpenGL ES / WebGL2**. The build emits a
**modularized ES-module output** rather than a generated HTML shell.

The codebase is configured for **C23**.

## What the build produces

A successful build places these primary artifacts in `build-wasm/`:

- `testProject.js` — generated modularized ES module loader
- `testProject.wasm` — compiled WebAssembly binary
- `index.html` — static browser entry page
- `bootstrap.js` — ES-module bootstrap that imports `testProject.js`
- `*.data` — optional data package, if asset bundling is added later

## Prerequisites

### Required tools

- **Emscripten SDK** (latest stable recommended)
- **CMake** 3.20 or higher
- **Ninja** (recommended)
- A **C23-capable compiler** through Emscripten's toolchain

Install and activate Emscripten:

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

## SDL3 and OpenGL in the browser

This project continues to use **SDL3** and **OpenGL-style rendering** in the
browser build. SDL3 is provided through the Emscripten port layer, so you do
**not** install a native desktop SDL package for this project configuration.

The relevant integration is handled by the build flags in `CMakeLists.txt`:

- `-sUSE_SDL=3`
- `-sUSE_WEBGL2=1`
- `-sMIN_WEBGL_VERSION=2`
- `-sMAX_WEBGL_VERSION=2`

## Quick start

### Debug build

```bash
source /path/to/emsdk/emsdk_env.sh
emcmake cmake --preset wasm-debug
cmake --build --preset wasm-debug
python3 -m http.server 8000 -d build-wasm
# Open http://localhost:8000
```

### Release build

```bash
source /path/to/emsdk/emsdk_env.sh
emcmake cmake --preset wasm-release
cmake --build --preset wasm-release
python3 -m http.server 8000 -d build-wasm
```

## Presets

| Preset | Platform | Graphics | Build type | Description |
|--------|----------|----------|------------|-------------|
| `wasm-debug` | WebAssembly | SDL3 + WebGL2 | Debug | Debug-friendly WASM build |
| `wasm-release` | WebAssembly | SDL3 + WebGL2 | Release | Optimized WASM build |

## JavaScript integration model

The generated JavaScript output is a **modularized ES module**. The browser does
not rely on Emscripten generating an HTML wrapper. Instead:

1. `index.html` loads `bootstrap.js` using `<script type="module">`
2. `bootstrap.js` imports the generated `testProject.js`
3. the module factory is awaited
4. the initialized module instance calls:
   - `initWebGL()`
   - `startMainLoop()`

This keeps the browser bootstrap explicit and easier to integrate into a larger
frontend later.

## Useful commands

List presets:

```bash
cmake --list-presets
```

Clean build directories:

```bash
rm -rf build-*
```

Use stronger release optimization:

```bash
emcmake cmake --preset wasm-release -DCMAKE_C_FLAGS="-O3 -flto"
```

Minimize binary size:

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

### `This project now requires the Emscripten toolchain`

You attempted to configure with a non-Emscripten compiler. Re-run the configure
step through `emcmake`:

```bash
source /path/to/emsdk/emsdk_env.sh
emcmake cmake --preset wasm-debug
```

### Browser loads the HTML page but nothing renders

Check:

- the browser console for module import errors
- that `testProject.js` and `testProject.wasm` are present in `build-wasm/`
- that the files are being served over HTTP rather than opened directly from disk

### SDL or GL initialization fails

Verify the build was produced with the Emscripten toolchain and that the browser
supports **WebGL2**.

## Continuous integration

The repository workflows should now treat the project as a **browser-only
WebAssembly target**. Pages and release packaging must publish:

- `index.html`
- `bootstrap.js`
- `testProject.js`
- `testProject.wasm`
- optional `*.data` files
