#ifndef GEOMETRY2D_INTERNAL_H
#define GEOMETRY2D_INTERNAL_H

#include "geometry2d_types.h"
#include <math.h>

/*******************************************************************************
 * Internal Macros
 ******************************************************************************/

#ifndef DEG2RAD
    #define DEG2RAD(deg) ((deg) * 0.0174532925199433f)  // deg * (PI / 180)
#endif

#define CLAMP(number, minimum, maximum) \
    ((number) = ((number) < (minimum)) ? (minimum) : \
        (((number) > (maximum)) ? (maximum) : (number)))

#define OVERLAP(minA, maxA, minB, maxB) \
    (((minB) <= (maxA)) && ((minA) <= (maxB)))

/*******************************************************************************
 * 2x2 Rotation Helper
 ******************************************************************************/

static inline vec2 rotate_vec2(vec2 v, float theta_rad) {
    float c = cosf(theta_rad);
    float s = sinf(theta_rad);
    return vec2_make(
        v.x * c + v.y * (-s),
        v.x * s + v.y * c
    );
}

#endif // GEOMETRY2D_INTERNAL_H
