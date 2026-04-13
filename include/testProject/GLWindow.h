#ifndef TESTPROJECT_GLWINDOW_H
#define TESTPROJECT_GLWINDOW_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Lightweight window-state compatibility header.
 *
 * This project is written in C, so this interface intentionally avoids stale
 * C++ references. Platform-specific implementations may provide these accessors
 * where needed; code that only needs the type can include this header safely on
 * every target.
 */
typedef struct GLWindowState {
    int width;
    int height;
    bool is_fullscreen;
    bool is_visible;
} GLWindowState;

/* Optional platform-specific accessors. */
int glwindow_get_width(void);
int glwindow_get_height(void);
float glwindow_get_aspect(void);

/*
 * Legacy compatibility aliases.
 * Keep older callers building while steering new code toward the lowercase API.
 */
static inline int GetWindowWidth(void) {
    return glwindow_get_width();
}

static inline int GetWindowHeight(void) {
    return glwindow_get_height();
}

static inline float GetWindowAspect(void) {
    return glwindow_get_aspect();
}

#ifdef __cplusplus
}
#endif

#endif /* TESTPROJECT_GLWINDOW_H */
