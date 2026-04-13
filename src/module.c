// src/module.c
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
// Define EMSCRIPTEN_KEEPALIVE as empty for native builds
#define EMSCRIPTEN_KEEPALIVE
#endif

#ifdef __cplusplus
# define EXTERN extern "C"
#else
# define EXTERN
#endif

EXTERN EMSCRIPTEN_KEEPALIVE void myFunction(void) {
    printf("MyFunction Called\n");
}
