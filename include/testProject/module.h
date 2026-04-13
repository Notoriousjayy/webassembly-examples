#ifndef MODULE_H
#define MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#else
#ifndef EMSCRIPTEN_KEEPALIVE
#define EMSCRIPTEN_KEEPALIVE
#endif
#endif

/**
 * @brief A function exported in WebAssembly builds and available to JavaScript
 *        via Module.ccall().
 *
 * On non-Emscripten builds, EMSCRIPTEN_KEEPALIVE expands to nothing so this
 * header remains portable.
 */
EMSCRIPTEN_KEEPALIVE void myFunction(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULE_H */
