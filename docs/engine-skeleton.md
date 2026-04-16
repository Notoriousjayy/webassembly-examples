# Engine Skeleton Design — C/C++, SDL3, OpenGL ES 3, WebAssembly

**Status:** Phase 0 (Skeleton) design.
**Audience:** Engine architect / lead implementer.
**Goal:** A repository that compiles natively and to WASM, opens a window/canvas with a cleared GL framebuffer, and exposes the module seams every later subsystem will plug into.

---

## 1. Executive Summary

This document specifies the initial repository, module boundaries, and bootstrap plan for a custom game engine targeting native desktop and WebAssembly through a shared codepath. The design follows the layered architecture described in *Game Engine Architecture* (Gregory, ch. 1): a strict dependency direction from a Platform Independence Layer upward through Core Systems, Resources, Rendering, Collision/Physics, Animation, and finally Gameplay Foundations and Game-Specific code. Every module either sits at one tier and depends only on tiers below, or is explicitly platform-agnostic.

The skeleton is deliberately small. It establishes:

- A canonical folder layout with non-negotiable boundaries between `engine/`, `game/`, `tools/`, `platform/`, `third_party/`, and `assets/`.
- A CMake build with presets for native (Linux/macOS/Windows) and Emscripten/WASM targets sharing a single source tree.
- A platform abstraction layer (PAL) thin enough to be implemented on day one and stable enough not to leak SDL3 or OpenGL into engine modules above it.
- A common GL surface (OpenGL ES 3.0 / WebGL 2.0) so the same render backend runs everywhere without `#ifdef` thickets.
- Stubbed module folders (`physics/`, `animation/`, `audio/`) so the dependency graph is correct from the start, even before code lands.

The skeleton is "done" when both targets build from a clean checkout, both open a window/canvas, both clear to a known color, both log through the same sink, and both quit cleanly. Everything else — meshes, materials, scenes, physics — is deferred to Phase 1+.

---

## 2. Recommended Repository Layout

```
engine-root/
├── .github/
│   └── workflows/
│       ├── ci-native.yml
│       └── ci-wasm.yml
├── .clang-format
├── .clang-tidy
├── .editorconfig
├── .gitignore
├── .gitattributes
├── CMakeLists.txt
├── CMakePresets.json
├── LICENSE
├── README.md
│
├── cmake/
│   ├── CompilerWarnings.cmake
│   ├── EngineModule.cmake          # add_engine_module() helper
│   ├── Sanitizers.cmake
│   ├── Toolchain-Emscripten.cmake
│   └── ThirdParty.cmake
│
├── docs/
│   ├── 00-engine-skeleton.md       # this document
│   ├── 01-coding-standard.md
│   ├── 02-module-dependency-rules.md
│   ├── 03-build-and-bootstrap.md
│   ├── 04-asset-pipeline.md
│   └── adr/                        # architecture decision records
│       └── 0001-pal-over-sdl3.md
│
├── scripts/
│   ├── bootstrap.sh                # one-shot dev env setup
│   ├── build_native.sh
│   ├── build_wasm.sh
│   ├── run_native.sh
│   ├── run_wasm.sh                 # spins up a static server
│   ├── format.sh
│   └── lint.sh
│
├── third_party/
│   ├── CMakeLists.txt
│   ├── sdl3/                       # submodule or FetchContent
│   ├── glad/                       # GL ES 3.0 / WebGL 2 loader (native only)
│   ├── stb/                        # stb_image, stb_truetype
│   ├── cgltf/                      # glTF reader (deferred use)
│   ├── meshoptimizer/              # deferred use
│   └── doctest/                    # unit test framework
│
├── engine/
│   ├── core/
│   │   ├── include/core/
│   │   │   ├── types.h
│   │   │   ├── assert.h
│   │   │   ├── log.h
│   │   │   ├── memory.h
│   │   │   ├── allocator.h
│   │   │   ├── result.h
│   │   │   ├── hash.h
│   │   │   ├── span.h
│   │   │   ├── string_view.h
│   │   │   ├── handle.h
│   │   │   ├── profile.h
│   │   │   ├── config.h
│   │   │   └── math/
│   │   │       ├── scalar.h
│   │   │       ├── vec.h
│   │   │       ├── mat.h
│   │   │       ├── quat.h
│   │   │       ├── transform.h
│   │   │       └── aabb.h
│   │   ├── src/
│   │   └── CMakeLists.txt
│   │
│   ├── platform/
│   │   ├── include/platform/
│   │   │   ├── platform.h          # umbrella header, capability flags
│   │   │   ├── window.h
│   │   │   ├── input.h
│   │   │   ├── clock.h
│   │   │   ├── filesystem.h
│   │   │   ├── log_sink.h
│   │   │   ├── thread.h            # stubbed in WASM
│   │   │   ├── atomics.h
│   │   │   ├── dynlib.h            # stubbed in WASM
│   │   │   └── gl_context.h
│   │   ├── src/
│   │   │   ├── sdl3/               # default backend for both native + WASM
│   │   │   │   ├── window_sdl3.c
│   │   │   │   ├── input_sdl3.c
│   │   │   │   ├── clock_sdl3.c
│   │   │   │   ├── filesystem_sdl3.c
│   │   │   │   └── gl_context_sdl3.c
│   │   │   └── posix/              # POSIX-only fallbacks (threads, dynlib)
│   │   └── CMakeLists.txt
│   │
│   ├── render/
│   │   ├── include/render/
│   │   │   ├── gpu.h               # RHI-style opaque handles
│   │   │   ├── gpu_types.h
│   │   │   ├── frame.h
│   │   │   └── view.h
│   │   ├── src/
│   │   │   └── gl/                 # GL ES 3.0 / WebGL 2 backend
│   │   │       ├── gpu_gl.c
│   │   │       ├── buffer_gl.c
│   │   │       ├── texture_gl.c
│   │   │       ├── shader_gl.c
│   │   │       └── pipeline_gl.c
│   │   └── CMakeLists.txt
│   │
│   ├── resource/
│   │   ├── include/resource/
│   │   │   ├── resource.h          # handle-based resource manager
│   │   │   ├── pak.h               # packfile reader
│   │   │   └── loader.h
│   │   └── CMakeLists.txt
│   │
│   ├── scene/
│   │   ├── include/scene/
│   │   │   ├── scene.h
│   │   │   ├── transform_node.h
│   │   │   └── camera.h
│   │   └── CMakeLists.txt
│   │
│   ├── physics/                    # stub: header + empty .c, no impl yet
│   │   ├── include/physics/
│   │   │   └── physics.h
│   │   └── CMakeLists.txt
│   │
│   ├── animation/                  # stub
│   │   ├── include/animation/
│   │   │   └── animation.h
│   │   └── CMakeLists.txt
│   │
│   ├── audio/                      # stub
│   │   ├── include/audio/
│   │   │   └── audio.h
│   │   └── CMakeLists.txt
│   │
│   └── runtime/
│       ├── include/runtime/
│       │   ├── engine.h            # Engine_Init / Engine_Tick / Engine_Shutdown
│       │   ├── subsystem.h
│       │   └── app.h               # interface every game implements
│       ├── src/
│       │   ├── engine.c
│       │   └── main_loop.c
│       └── CMakeLists.txt
│
├── platform/
│   ├── native/
│   │   ├── main_native.c           # int main(int, char**)
│   │   └── CMakeLists.txt
│   └── wasm/
│       ├── main_wasm.c             # emscripten_set_main_loop entry
│       ├── shell.html
│       ├── pre.js
│       └── CMakeLists.txt
│
├── game/
│   └── sample/
│       ├── include/sample/
│       │   └── sample_app.h
│       ├── src/
│       │   └── sample_app.c        # implements runtime/app.h
│       └── CMakeLists.txt
│
├── editor/                         # deferred; folder created, README only
│   └── README.md
│
├── tools/
│   ├── blender/
│   │   ├── io_engine_export/       # Blender addon, Python
│   │   │   ├── __init__.py
│   │   │   ├── exporter.py
│   │   │   └── README.md
│   │   └── README.md
│   ├── asset_compiler/             # native CLI: source assets -> .pak
│   │   ├── src/
│   │   └── CMakeLists.txt
│   └── shader_compiler/            # GLSL ES preprocess + validate
│       ├── src/
│       └── CMakeLists.txt
│
├── assets/
│   ├── source/                     # .blend, .png, .wav — author-side
│   │   └── README.md
│   ├── cooked/                     # engine-ready binaries (gitignored)
│   │   └── .gitkeep
│   └── shaders/                    # GLSL ES 3.00 sources
│       ├── fullscreen.vert
│       └── clear.frag
│
└── tests/
    ├── unit/
    │   ├── core/
    │   │   ├── test_math.c
    │   │   └── test_allocator.c
    │   ├── platform/
    │   │   └── test_clock.c
    │   └── CMakeLists.txt
    └── integration/
        ├── test_window_open.c      # opens window, draws 60 frames, quits
        └── CMakeLists.txt
```

**Hard rules embedded in the layout:**

- Every engine module owns a single `include/<module>/` directory. **Public headers live there and nowhere else.** `src/` is private.
- `third_party/` is the *only* place vendored code lives. No copy-pasted snippets in engine modules.
- `assets/source/` is committed; `assets/cooked/` is generated and gitignored.
- `platform/native/` and `platform/wasm/` contain *only* the entry points and packaging glue, never engine logic.
- `editor/` exists from day one as a placeholder so its dependency direction is correct from the first commit.

---

## 3. Engine Module Boundaries

The layering is the architecture. *Game Engine Architecture* §1.6 is the reference model. Each module below names what it owns, what it must not touch, and the stable interface it exposes.

### 3.1 `engine/core` — Foundation Layer

**Owns:** fixed-width types, asserts, logging API (not sinks), allocator interface and concrete allocators (linear, stack, pool, page, tracking), containers (`span`, `array`, `hashmap`, `ring`), strings (`string_view`, interned `string_id`), opaque `Handle32`, math (vec/mat/quat/transform/AABB/sphere), result/error type, profiling macros, configuration parsing.

**Must not touch:** SDL, OpenGL, the filesystem, threads, or anything platform-specific. No `#include <SDL3/...>`. No `#include <GLES3/...>`. Pure C11 + C++20 where useful, freestanding-friendly.

**Stable interface:** all of `engine/core/include/core/*.h`. Every other module includes from here.

**Justification:** Gregory's "Core Systems" tier (§3, §5) and Ericson's emphasis on a clean math layer (*Real-Time Collision Detection* §3) both require a foundation that is independent of OS and renderer so it can be unit-tested in isolation and reused by tools.

### 3.2 `engine/platform` — Platform Independence Layer (PAL)

**Owns:** the abstraction over windowing, input, monotonic and wall clocks, virtual filesystem, a log sink (file/stderr/console), threads/atomics, dynamic library loading, and GL context creation.

**Must not touch:** rendering (no GL calls beyond context bring-up), resources, scene, gameplay. May depend on `core/`.

**Stable interface:** opaque handles + free functions in `platform/*.h`. **No SDL types appear in any public header.** SDL3 is an implementation detail of `src/sdl3/`.

**Justification:** Gregory §4 ("Platform Independence Layer") makes the case explicitly: hiding the OS/SDK behind a thin C interface is what lets the engine port without bleeding `#ifdef SDL_PLATFORM_*` into game code. SDL3 already abstracts most of this; the PAL exists so that *swapping SDL3 out is possible* and so that engine code never grows a transitive dependency on it.

### 3.3 `engine/render` — Render Hardware Interface (RHI) + GL Backend

**Owns:** an opaque, handle-based GPU abstraction (`Gpu_BufferHandle`, `Gpu_TextureHandle`, `Gpu_PipelineHandle`, `Gpu_PassHandle`), command submission, GL ES 3.0 / WebGL 2.0 backend, shader module loading. No materials, no scene knowledge, no cameras with semantics — just GPU resources and draw submission.

**Must not touch:** scene, gameplay, asset cooking. Receives shader bytes/source from `resource/`, does not load files itself.

**Stable interface:** `render/gpu.h`. Backend is selected at compile time (`ENGINE_GFX_GL`).

**Justification:** *Computer Graphics: Principles and Practice* §38 (rendering pipeline), and *PBRT* §1 on the importance of a clean separation between scene description and rendering execution. We start with one backend (GL ES 3) but the RHI shape is what allows a Vulkan/WebGPU backend later without rewriting render-graph code.

### 3.4 `engine/resource` — Resource Manager

**Owns:** packfile (`.pak`) reader, asset handles, async load queue (single-threaded at first, threaded later), reference counting, hot-reload hooks (deferred).

**Must not touch:** rendering specifics (it returns bytes and metadata, not GPU handles). Cooking lives in `tools/asset_compiler/`, not here.

**Stable interface:** `resource/resource.h`.

**Justification:** Gregory §7. The resource manager sits between the PAL and the consumer subsystems and is the one component that everything depends on for assets — keeping it thin and handle-based prevents the "everyone reaches into the filesystem" anti-pattern.

### 3.5 `engine/scene` — Scene Graph

**Owns:** transform hierarchy, camera, basic culling primitives, world bounds. Not an ECS yet — a flat array of nodes with parent indices is sufficient for the skeleton.

**Must not touch:** GPU, physics solvers, animation evaluation. Holds data; other systems consume it.

**Justification:** Gregory §15. Defer the ECS decision (§15.5) until you have a real workload that justifies it — picking too early locks you into a bad shape.

### 3.6 `engine/physics`, `engine/animation`, `engine/audio` — Stubbed

Each exists as a header + empty translation unit. Their job in Phase 0 is to **establish the dependency direction** so that when implementations land they cannot accidentally reach into rendering or scene state they shouldn't.

- `physics/` will eventually own the broadphase, narrowphase, and constraint solver (Ericson; Eberly *Game Physics* ch. 5–6). For fluid extensions: Bridson and Kim. None of this exists yet.
- `animation/` will own skeleton, pose, blend tree, IK (Parent ch. 4–6). Stub only.
- `audio/` will own mixer + sources. Stub only.

### 3.7 `engine/runtime` — Engine Orchestration

**Owns:** subsystem registration and init/shutdown order, the main loop (fixed-step simulation, variable render — Gregory §8.5), the `App` interface games implement, frame timing and the high-level tick.

**Must not touch:** specifics of any subsystem. Calls subsystem APIs through their stable headers only.

**Stable interface:** `runtime/engine.h`, `runtime/app.h`.

### 3.8 `game/sample` — Sample Application

**Owns:** an implementation of `runtime/app.h` that initializes the engine, requests a window, clears the screen to a color, and quits on ESC. This is the scaffolding the skeleton ships with.

### 3.9 `tools/*` — Offline Tools

Tools are native-only executables that depend on `engine/core` (and possibly `engine/resource`) but **never** on `engine/render`, `engine/runtime`, or any platform window/input code. They produce inputs to the engine; they don't run the engine.

---

## 4. Dependency Direction Rules

The dependency graph is a DAG. A module may depend only on modules **strictly below** it.

```
                      ┌──────────────┐
                      │  game/sample │
                      └──────┬───────┘
                             │
                      ┌──────▼───────┐
                      │   runtime    │
                      └──────┬───────┘
        ┌──────────────┬─────┼──────┬──────────────┐
        │              │     │      │              │
   ┌────▼────┐   ┌─────▼──┐  │  ┌───▼────┐   ┌─────▼─────┐
   │ physics │   │animation│  │  │ audio  │   │   scene   │
   └────┬────┘   └─────┬──┘  │  └───┬────┘   └─────┬─────┘
        │              │     │      │              │
        └──────────────┴─────┼──────┴──────────────┘
                             │
                      ┌──────▼───────┐
                      │   resource   │
                      └──────┬───────┘
                             │
                      ┌──────▼───────┐
                      │    render    │
                      └──────┬───────┘
                             │
                      ┌──────▼───────┐
                      │   platform   │ (PAL — wraps SDL3, GL ctx)
                      └──────┬───────┘
                             │
                      ┌──────▼───────┐
                      │     core     │ (no deps but libc/libc++)
                      └──────────────┘
```

**Enforced rules:**

1. `core` depends on nothing but the C/C++ standard library.
2. `platform` depends only on `core` and on `third_party/sdl3`.
3. `render` depends on `core`, `platform`, and the GL loader. It does **not** depend on `resource`.
4. `resource` depends on `core` and `platform`. It does **not** depend on `render`.
5. `scene`, `physics`, `animation`, `audio` depend on `core`, `platform`, `render` (for handles only), and `resource`. They are siblings; none depend on each other in Phase 0.
6. `runtime` may depend on every engine module. Game code depends only on `runtime` headers and the public engine module headers it explicitly opts into.
7. `tools/` depends on `core` and `resource` only.

**CMake enforcement:** `EngineModule.cmake` defines `add_engine_module(NAME ... DEPS ...)` which calls `target_link_libraries(... PRIVATE ${DEPS})` and uses `target_include_directories(... INTERFACE ${include})` so that consumers get the public headers and *only* the public headers. This is how the layering survives contact with people who are tired on a Friday.

---

## 5. Build System and Script Layout

### 5.1 Recommendation: CMake 3.27+ with Presets

CMake is chosen for one practical reason: it is the only build system with first-class Emscripten support, first-class IDE integration (CLion, VS, VSCode, Xcode), and a vendored ecosystem (FetchContent for SDL3). Premake and xmake are nicer to write but lose on toolchain compatibility, especially for WASM CI.

### 5.2 Files to Create First

**Root:**

- `CMakeLists.txt` — declares project, language standards (C11, C++20), includes `cmake/*.cmake`, adds subdirectories.
- `CMakePresets.json` — defines `native-debug`, `native-release`, `native-asan`, `wasm-debug`, `wasm-release`.

**`cmake/`:**

- `CompilerWarnings.cmake` — enables `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wstrict-prototypes` for GCC/Clang, `/W4 /permissive-` for MSVC, escalates to `-Werror` in CI.
- `Sanitizers.cmake` — opt-in ASan/UBSan for `native-asan` preset.
- `Toolchain-Emscripten.cmake` — sets `CMAKE_TOOLCHAIN_FILE` integration, defines `EMSCRIPTEN=ON`, sets shell file, exported functions, `-sUSE_SDL=3` linker flag, `-sFULL_ES3=1`, `-sALLOW_MEMORY_GROWTH=1`, `-sINITIAL_MEMORY=64MB`, `--preload-file assets/cooked@/assets`.
- `EngineModule.cmake` — `add_engine_module()` helper enforcing layout.
- `ThirdParty.cmake` — `FetchContent_Declare` for SDL3 (native only — WASM uses the Emscripten port via `-sUSE_SDL=3`), glad (native only), doctest, stb. Pinned by tag.

**`scripts/`:**

- `bootstrap.sh` — installs/checks emsdk, ensures SDL3 build deps on Linux/macOS, runs CMake configure for both presets.
- `build_native.sh` — `cmake --build --preset native-debug -j`.
- `build_wasm.sh` — sources `emsdk_env.sh`, `cmake --build --preset wasm-debug -j`.
- `run_native.sh` — runs the sample binary with sensible defaults.
- `run_wasm.sh` — `python3 -m http.server 8080 --directory build/wasm-debug/platform/wasm` then opens browser.
- `format.sh` / `lint.sh` — clang-format + clang-tidy over `engine/`, `game/`, `tools/`.

### 5.3 Compiler Flag Conventions

| Profile | Optimization | Asserts | Sanitizers | Symbols |
|---|---|---|---|---|
| `native-debug` | `-O0 -g3` | on | off (separate preset) | full |
| `native-asan` | `-O1 -g3 -fno-omit-frame-pointer` | on | ASan + UBSan | full |
| `native-release` | `-O3 -g1 -DNDEBUG` | off (engine asserts on) | off | line tables |
| `wasm-debug` | `-O0 -g3 -gsource-map` | on | off | source maps |
| `wasm-release` | `-O3 -g0 -DNDEBUG -flto` | off (engine asserts on) | off | none |

**Engine asserts are independent of `NDEBUG`.** `core/assert.h` defines `ENGINE_ASSERT` (always on) and `ENGINE_DEV_ASSERT` (debug-only). Gregory §3.2.3.

### 5.4 Third-Party Integration Convention

- Each dep is either a git submodule or `FetchContent` with a pinned tag — never a moving branch.
- Wrappers in `third_party/<name>/CMakeLists.txt` set warning level to `none` for the dep so vendor warnings don't drown your own.
- Engine code includes vendor headers via `<sdl3/SDL.h>`-style angle-bracket paths only inside `engine/platform/src/sdl3/`. Anywhere else is a build error by convention (and a `clang-tidy` check later).

---

## 6. Platform Abstraction Layer Design

The PAL is the most important interface in the skeleton. Get this wrong and every later module bleeds OS knowledge.

### 6.1 Headers to Create on Day One

| Header | Purpose | Backed by |
|---|---|---|
| `platform/platform.h` | Capability flags, platform IDs (`PLATFORM_LINUX`, `PLATFORM_MACOS`, `PLATFORM_WINDOWS`, `PLATFORM_WEB`), init/shutdown of the PAL itself. | SDL3 init/quit |
| `platform/window.h` | `Window* window_create(const WindowDesc*)`, resize callbacks, `window_should_close()`, `window_swap()`. Opaque type. | SDL_Window |
| `platform/input.h` | Keyboard/mouse/gamepad state polled per frame. No event queue exposed; events are translated into a frame-local snapshot. | SDL events |
| `platform/clock.h` | `u64 clock_now_ns()`, `f64 clock_seconds()`, monotonic only. | `SDL_GetTicksNS` |
| `platform/filesystem.h` | `fs_read_all(path, allocator, out_bytes, out_size)`, `fs_exists`, virtual mount points, both real FS and packfile-backed. | SDL_IOStream + native or `--preload-file` on web |
| `platform/log_sink.h` | Pluggable sink interface; default sinks: stderr, OutputDebugString on Windows, `console.log` on web via `EM_ASM`. | platform-specific |
| `platform/thread.h` | `Thread`, `Mutex`, `CondVar`. **Stubbed to single-threaded on WASM** until pthreads are needed. | SDL_Thread / no-op |
| `platform/atomics.h` | C11 `<stdatomic.h>` wrappers for portability across MSVC. | stdatomic |
| `platform/dynlib.h` | `dynlib_open/close/sym`. Stub on WASM. | dlopen / LoadLibrary |
| `platform/gl_context.h` | `gl_context_create(window)`, `gl_context_make_current()`, `gl_context_swap()`, queries GL ES 3 capability. | SDL_GL_* |

### 6.2 What to Abstract Immediately vs Defer

**Abstract immediately** (before any engine subsystem code):

- Window, GL context, input snapshot, monotonic clock, file read, log sink, basic memory allocator. These are touched by the very first frame and changing them later means rewriting everything that uses them.

**Leave concrete for now** (use SDL3 / libc directly *inside* `platform/src/`, but do not yet expose):

- Threads beyond `thread_create/join` — no thread pool, no job system.
- Async file I/O — synchronous reads only.
- Audio device management — `audio/` is stubbed.
- Networking — does not exist yet.
- Touch input — gated behind a capability flag, not in the input API yet.
- Hot-reload file watching — Phase 2.

### 6.3 The SDL3 Question

SDL3 already is a PAL. Why wrap it? Two reasons:

1. **WASM and native share the SDL3 surface, but not the same SDL3.** Native links a vendored SDL3 build; WASM uses `-sUSE_SDL=3` which is Emscripten's port. Wrapping isolates the small differences (file system semantics, threading availability) in one place.
2. **The PAL is the contract.** If in two years you want to swap SDL3 for GLFW + raw audio, or for a console SDK, the change is bounded to `engine/platform/src/`. No engine code needs to know.

The wrapper should be thin: usually one PAL function maps to two or three SDL calls. Resist the urge to add features that SDL doesn't already provide.

---

## 7. Native vs WASM Target Strategy

### 7.1 One Source Tree, Two Toolchains

The same `engine/` and `game/` source compile for both targets. Differences are confined to:

- `platform/native/main_native.c` vs `platform/wasm/main_wasm.c` — entry point shape.
- `engine/platform/src/` — small `#ifdef __EMSCRIPTEN__` blocks for filesystem mount points and threading no-ops.
- CMake target-link flags via `Toolchain-Emscripten.cmake`.

### 7.2 GL Common Subset

Target **OpenGL ES 3.0 / GLSL ES 3.00** as the canonical surface. WebGL 2.0 is a near-exact superset-compatible subset. On native, request a GL 4.3 Core context and use only the ES 3.0-compatible subset (validated by a `clang-tidy`-ish header that defines forbidden GL symbols). This avoids two shader paths.

Shaders live in `assets/shaders/` as `.vert`/`.frag` GLSL ES 3.00 source. The shader compiler tool (Phase 1) will preprocess `#include` and validate. Phase 0 just loads them as text.

### 7.3 Main Loop Shape

- **Native:** classical `while (!quit) { tick(); render(); }` loop in `runtime/src/main_loop.c`.
- **WASM:** `emscripten_set_main_loop_arg(tick_render, ctx, 0, 1)`. The runtime exposes a `runtime_tick_once(ctx)` that both loops call. The loop *shape* lives in the runtime; the *driver* lives in the platform/ entry.

### 7.4 Filesystem on Web

WASM has no real filesystem. Two options the skeleton supports from day one:

1. **Preload bundle** (`--preload-file assets/cooked@/assets`) — bakes cooked assets into the `.data` file. Best for small builds.
2. **Lazy fetch** via `emscripten_async_wget` — for larger games. Wired through `filesystem.h` with the same API.

The PAL `filesystem.h` returns bytes; callers don't care which mechanism produced them.

### 7.5 Threading on Web

Defer pthreads on WASM until you have a reason. `thread.h` stubs on web cause `thread_create` to immediately run the function inline (or return an error if the caller assumed concurrency). Audit all `thread_create` callers when you eventually flip pthreads on with `-pthread` and the COOP/COEP headers.

---

## 8. Blender / Asset Pipeline Integration Point

The engine consumes **cooked**, engine-native binaries — not `.blend`, not `.fbx`, not raw PNGs. The pipeline is one-way and offline.

```
   ┌──────────────────────┐
   │  assets/source/*.blend│  authored
   └──────────┬───────────┘
              │
              │ Blender + io_engine_export addon
              ▼
   ┌──────────────────────┐
   │ assets/source/*.gltf │  intermediate (committed for diffability)
   │ assets/source/*.bin  │
   │ assets/source/*.png  │
   └──────────┬───────────┘
              │
              │ tools/asset_compiler  (native CLI)
              ▼
   ┌──────────────────────┐
   │  assets/cooked/*.pak │  engine-ready, gitignored
   └──────────┬───────────┘
              │
              │ resource/pak.h  (mmap on native, preload on web)
              ▼
   ┌──────────────────────┐
   │     engine runtime   │
   └──────────────────────┘
```

**Phase 0 deliverables for the pipeline:**

1. `tools/blender/io_engine_export/` — a Blender addon stub with `__init__.py`, `bl_info` dict, an `export_scene(filepath)` operator that calls `bpy.ops.export_scene.gltf()` with the engine's preset (Y-up, +Z forward, embedded buffers off, KTX2 textures off for now). The convention is **export to glTF 2.0**; do not invent a custom format yet.
2. `tools/asset_compiler/` — a native CLI that reads glTF via `cgltf` and writes a trivial `.pak` (header + TOC + concatenated blobs). Phase 0 needs the *binary format declared and the tool buildable*, not full feature coverage.
3. Documented coordinate-system + units convention in `docs/04-asset-pipeline.md`. Pick one (right-handed, Y-up, 1 unit = 1 meter, *Game Engine Architecture* §3.5.1) and enforce it in the exporter.

What you do **not** do in Phase 0: skinning, animation export, materials beyond base color, LOD generation, mesh optimization. The pipeline exists so the seam is real; the content is a single textured cube.

---

## 9. Startup Checklist

Execute in order. Each step has a clear validation.

**Repository scaffolding**

1. `git init`, add `.gitignore`, `.gitattributes`, `.editorconfig`, `.clang-format`, `.clang-tidy`.
2. Create the full directory tree from §2 with `.gitkeep` files in empty dirs.
3. Add `LICENSE`, `README.md`, this document as `docs/00-engine-skeleton.md`.
4. Commit: "skeleton: initial layout".

**Build system bring-up**

5. Write root `CMakeLists.txt` with C11, C++20, and `add_subdirectory()` for every engine module (most are empty headers at this point).
6. Write `CMakePresets.json` with `native-debug` and `wasm-debug` presets.
7. Write `cmake/CompilerWarnings.cmake`, `cmake/EngineModule.cmake`, `cmake/Toolchain-Emscripten.cmake`.
8. Vendor SDL3 via `FetchContent` (native) and confirm `-sUSE_SDL=3` works for WASM.
9. Vendor glad for native GL ES 3 loader.
10. Run `cmake --preset native-debug` — must configure cleanly with zero warnings. **Validation:** configure succeeds, no warnings.

**Core layer**

11. Implement `core/types.h`, `core/assert.h`, `core/log.h` (forward-declared sink), `core/result.h`, `core/span.h`.
12. Implement a linear allocator and a tracking allocator in `core/allocator.h` + one `.c`.
13. Add doctest harness in `tests/unit/core/`, write 2 tests (`test_math` for vec3 dot, `test_allocator` for linear alloc/reset).
14. Run native build + tests. **Validation:** all unit tests pass.

**Platform layer**

15. Implement `platform/log_sink.h` with a stderr sink.
16. Implement `platform/clock.h` backed by `SDL_GetTicksNS`.
17. Implement `platform/window.h` opening an SDL3 window with a GL ES 3.0 context.
18. Implement `platform/input.h` with a per-frame keyboard snapshot.
19. Implement `platform/filesystem.h` with `fs_read_all` backed by `SDL_IOStream`.
20. **Validation:** a tiny native test program opens a window, polls for ESC, closes cleanly. No GL calls yet.

**Render layer (minimal)**

21. Implement `render/gpu.h` with just enough to clear the screen: `Gpu_Init`, `Gpu_BeginFrame`, `Gpu_Clear(color)`, `Gpu_EndFrame`.
22. Backend `src/gl/gpu_gl.c` issues `glViewport` + `glClearColor` + `glClear`.
23. **Validation:** native window opens and shows a solid color (e.g., `#1e2030`).

**Runtime + sample**

24. Implement `runtime/app.h` (`App_Init`, `App_Tick`, `App_Render`, `App_Shutdown`) and `runtime/engine.h` orchestrating PAL → render → app.
25. Implement `runtime/main_loop.c` with a fixed-timestep accumulator (Gregory §8.5.5).
26. Implement `game/sample/sample_app.c` that returns a `WindowDesc{ 1280, 720, "Engine Skeleton" }` and clears to a color.
27. Implement `platform/native/main_native.c` that calls `engine_init → loop → engine_shutdown`.
28. **Validation:** `./build/native-debug/game/sample/sample` opens a window, clears to color, ESC quits, no leaks under ASan.

**WASM target**

29. Implement `platform/wasm/main_wasm.c` calling `engine_init` then `emscripten_set_main_loop_arg(engine_tick_once, ctx, 0, 1)`.
30. Author `platform/wasm/shell.html` with a canvas and a status div.
31. Configure `--preload-file assets/cooked@/assets` (even if cooked/ is empty; the mount must exist).
32. `cmake --build --preset wasm-debug` — must produce `sample.html`, `sample.js`, `sample.wasm`, `sample.data`.
33. `scripts/run_wasm.sh` — open in browser. **Validation:** canvas appears with the same clear color, console shows engine logs, ESC quits cleanly.

**Tooling stub**

34. Create `tools/blender/io_engine_export/__init__.py` with `bl_info` and an empty `register()`/`unregister()`. Confirm Blender loads it without errors.
35. Create `tools/asset_compiler/` with a `main.c` that prints "asset_compiler v0.1" and exits. **Validation:** builds in `native-debug`.

**CI**

36. Write `.github/workflows/ci-native.yml` building the `native-asan` preset on Linux and macOS, running unit tests.
37. Write `.github/workflows/ci-wasm.yml` building the `wasm-debug` preset and uploading the `.html`/`.wasm` as artifacts.
38. **Validation:** both workflows green on the first push.

---

## 10. Completion Signal

The skeleton phase is **done** when *all* of the following are true. Treat it as a checklist; partial completion is not completion.

1. **Layout:** every directory in §2 exists in `main`, with `.gitkeep` or real files. `tree -L 3 .` matches the spec.
2. **Build — native:** from a clean clone, `./scripts/bootstrap.sh && ./scripts/build_native.sh` produces a runnable `sample` binary on Linux, macOS, and Windows. Zero warnings at `-Wall -Wextra -Wpedantic`.
3. **Build — WASM:** from the same clean clone, `./scripts/build_wasm.sh` produces `sample.html` + `sample.wasm` + `sample.data` that loads in Firefox and Chromium. Zero warnings.
4. **Runtime — native:** `sample` opens a 1280×720 window titled "Engine Skeleton", clears to a known color, runs at the display's refresh rate, responds to ESC and window-close, exits cleanly with zero ASan/UBSan reports.
5. **Runtime — WASM:** `sample.html` served over a local HTTP server displays the same canvas with the same clear color, logs engine init lines to the browser console, responds to ESC, and quits cleanly (`emscripten_cancel_main_loop`).
6. **PAL:** all headers in §6.1 exist and compile against both targets. `window.h`, `clock.h`, `filesystem.h`, `input.h`, `log_sink.h`, `gl_context.h` are implemented; `thread.h`, `dynlib.h`, `atomics.h` are at minimum declared with stub implementations on WASM.
7. **Boundaries:** `grep -r "SDL" engine/` returns hits only in `engine/platform/src/sdl3/`. `grep -r "GL/" engine/render/include/` returns nothing. These two greps must be in `scripts/lint.sh`.
8. **Tests:** `ctest --preset native-debug` runs ≥4 unit tests, all pass. One integration test opens a window for 60 frames headlessly (or at least confirms `App_Init`/`App_Shutdown` succeed) and passes.
9. **Tooling:** `tools/asset_compiler/` builds. Blender addon loads in Blender 4.x without error.
10. **CI:** both GitHub Actions workflows are green on the latest commit on `main`.
11. **Docs:** `docs/00-engine-skeleton.md` (this file), `docs/01-coding-standard.md`, `docs/02-module-dependency-rules.md`, and `docs/04-asset-pipeline.md` are committed. ADR `0001-pal-over-sdl3.md` exists.

When all eleven are checked, tag the commit `v0.1.0-skeleton`. That tag is the "Phase 0 complete" marker and the baseline every subsequent phase branches from.

---

## 11. Risks and Common Early Mistakes

These are the failure modes the skeleton phase usually hits. Avoid them deliberately.

1. **Letting OpenGL types leak into engine headers.** The moment `GLuint` appears in `render/gpu.h`, every consumer transitively depends on the GL loader. Use opaque `Gpu_*Handle` typedefs (`typedef struct { u32 id; } Gpu_BufferHandle;`).
2. **Letting SDL types leak into PAL headers.** Same pattern. `SDL_Window*` never appears in `platform/window.h`. Use `Window*` as opaque.
3. **Building an ECS first.** It is the most-cited premature decision in engine projects. A flat array of transform nodes works for everything you'll do in the first two months. Gregory §15.5 acknowledges this explicitly.
4. **Building a job system first.** Jobs are seductive, but every bug under a job system is twice as hard to find. Stay single-threaded until a profile demands otherwise.
5. **A material system before a triangle.** PBRT §1.1 spends pages making this point obliquely: render one thing correctly before generalizing. A "shader" in Phase 0 is a vertex/fragment pair the sample app loads by name.
6. **Custom mesh formats before glTF works end-to-end.** glTF is the lingua franca; ship the cube, then optimize.
7. **Threading on WASM by default.** `-pthread` requires COOP/COEP HTTP headers and breaks `file://` testing. Single-threaded WASM is fine for years.
8. **Cooked assets in git.** `assets/cooked/` is gitignored from commit zero. Source-of-truth lives in `assets/source/`.
9. **`#ifdef __EMSCRIPTEN__` scattered across engine code.** It belongs in `engine/platform/src/`. If you find one elsewhere, that's a leak — fix the abstraction.
10. **Header-only "convenience" libraries inside `engine/core/`.** Translation units exist for a reason: compile times. Put implementations in `.c`/`.cpp`.
11. **Skipping `EngineModule.cmake`.** Without a helper enforcing `target_link_libraries(... PRIVATE)` and proper INTERFACE include dirs, dependency leaks happen by accident in the second sprint.
12. **A "utility" module that depends on everything.** `core` is the only utility module. If you find yourself wanting to make `engine/util/`, you are about to violate the layering. Push the code down into `core/` or up into the consumer.
13. **Asserts that don't fire in release.** `ENGINE_ASSERT` should be on in release builds; only `ENGINE_DEV_ASSERT` is debug-only. Silent failures in shipped code are debugging tax forever.
14. **Picking the wrong handedness/units late.** Decide right-handed Y-up, 1 unit = 1 meter, in Phase 0. Changing it later means re-exporting every asset and finding every hardcoded constant.

---

## 12. Recommended First 2 Weeks of Implementation

A realistic single-engineer cadence. Each "day" is roughly half a working day; pad as needed.

### Week 1 — Native scaffold and clear screen

| Day | Work | Validation |
|---|---|---|
| 1 | Create full directory tree, `.gitignore`, `.editorconfig`, `.clang-format`, root `README.md`, this design doc committed. | `git ls-tree -r HEAD` matches §2. |
| 2 | Root `CMakeLists.txt`, `CMakePresets.json` with `native-debug`. `cmake/CompilerWarnings.cmake`, `cmake/EngineModule.cmake`. Empty module CMakeLists for every engine module. | `cmake --preset native-debug` configures cleanly. |
| 3 | `core/types.h`, `core/assert.h`, `core/log.h`, `core/result.h`. doctest vendored. First two unit tests. | `ctest` shows 2/2 passing. |
| 4 | `core/allocator.h` + linear allocator implementation. `core/math/vec.h`, `core/math/mat.h` with the operators the renderer will need. Unit tests for vec/mat. | `ctest` shows 6/6 passing. |
| 5 | SDL3 vendored. `platform/log_sink.h` (stderr sink), `platform/clock.h`, `platform/window.h`. `platform/input.h` with keyboard snapshot. | A throwaway `tools/sandbox/` opens an SDL window, polls ESC, exits clean. |
| 6 | `platform/gl_context.h` creating GL ES 3 context. `platform/filesystem.h` (`fs_read_all`). `render/gpu.h` minimal (init/clear/end). GL backend `gpu_gl.c`. | Sandbox now clears the window to `#1e2030`. |
| 7 | `runtime/app.h`, `runtime/engine.h`, `runtime/main_loop.c` with fixed-step accumulator. `game/sample/sample_app.c`. `platform/native/main_native.c`. Wire the lint script (the two `grep` invariants). | `./build/native-debug/game/sample/sample` opens window, clears, ESC quits, ASan clean. **End-of-week milestone: native skeleton runs.** |

### Week 2 — WASM target and pipeline seam

| Day | Work | Validation |
|---|---|---|
| 8 | Install/verify emsdk via `scripts/bootstrap.sh`. Add `wasm-debug` preset. `cmake/Toolchain-Emscripten.cmake` with `-sUSE_SDL=3`, `-sFULL_ES3=1`, `-sALLOW_MEMORY_GROWTH=1`. | `cmake --preset wasm-debug` configures, configures only — no build yet. |
| 9 | `platform/wasm/main_wasm.c` with `emscripten_set_main_loop_arg`. `platform/wasm/shell.html`. Stub `thread.h`/`dynlib.h` no-ops behind `__EMSCRIPTEN__`. | `cmake --build --preset wasm-debug` produces `sample.html`/`sample.js`/`sample.wasm`. |
| 10 | `scripts/run_wasm.sh`. Debug whatever the first WASM run breaks (usually filesystem mount or GL context attribs). | Browser canvas shows the clear color. ESC quits cleanly. **Mid-week milestone: WASM skeleton runs.** |
| 11 | `.github/workflows/ci-native.yml` (Ubuntu + macOS, ASan preset). `.github/workflows/ci-wasm.yml` (Ubuntu + emsdk). Both must pass on push. | Green CI badges. |
| 12 | Tooling stubs: `tools/asset_compiler/` builds and prints version. `tools/blender/io_engine_export/__init__.py` loads in Blender. `assets/source/cube.blend` committed with a single cube. `assets/source/cube.gltf` exported and committed. | Blender addon registers without error; addon's "Export" menu item is visible. |
| 13 | `docs/01-coding-standard.md`, `docs/02-module-dependency-rules.md`, `docs/04-asset-pipeline.md`, ADR `0001-pal-over-sdl3.md`. Update root README with quickstart. | Docs render on GitHub. |
| 14 | Hardening pass: run `clang-tidy`, fix warnings. Run integration test (window opens for N frames, exits clean) on both targets. Tag `v0.1.0-skeleton`. | Tag exists, all 11 completion-signal items pass, the engine is ready for Phase 1. |

After the tag, Phase 1 begins: a real RHI surface (vertex/index buffers, shaders, draw call), the resource manager promotion from "read bytes" to "manage handles", and the first non-trivial mesh render. *Real-Time Collision Detection* and *Game Physics* enter the picture in Phase 2 (broadphase, contact resolution); *Computer Animation: Algorithms and Techniques* in Phase 3 (skeletal animation); *PBRT* and *Computer Graphics: Principles and Practice* shape the rendering roadmap from Phase 2 onward; *Fluid Simulation for Computer Graphics* and *Fluid Engine Development* are Phase 4+ when the simulation subsystem is real.

The skeleton's job is to make every one of those phases additive, not destructive.
