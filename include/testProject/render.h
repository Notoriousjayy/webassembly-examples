#ifndef RENDER_H
#define RENDER_H

#include <emscripten/emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Browser renderer entry points.
 *
 * The project targets WebAssembly only and initializes an SDL3 + WebGL2
 * renderer from the JavaScript ES-module bootstrap.
 *
 * The renderer drives a 3D scene using the OrbitCamera, geometry primitives
 * (AABB, OBB, Sphere), frustum culling, and a proper MVP pipeline.
 */

/* Initialize SDL3, create the GL context, build GPU state, and prepare the 3D scene. */
EMSCRIPTEN_KEEPALIVE
int initWebGL(void);

/* Start the browser-driven main loop. */
EMSCRIPTEN_KEEPALIVE
void startMainLoop(void);

#ifdef __cplusplus
}
#endif

#endif /* RENDER_H */