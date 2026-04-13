#include "Geometry2D/rectangle2d.h"

#include <math.h>

/*******************************************************************************
 * Rectangle2D Operations
 ******************************************************************************/

vec2 rectangle2d_get_min(Rectangle2D rect) {
    vec2 p1 = rect.origin;
    vec2 p2 = vec2_add(rect.origin, rect.size);
    return (vec2){ fminf(p1.x, p2.x), fminf(p1.y, p2.y) };
}

vec2 rectangle2d_get_max(Rectangle2D rect) {
    vec2 p1 = rect.origin;
    vec2 p2 = vec2_add(rect.origin, rect.size);
    return (vec2){ fmaxf(p1.x, p2.x), fmaxf(p1.y, p2.y) };
}

Rectangle2D rectangle2d_from_min_max(vec2 min, vec2 max) {
    return rectangle2d_create(min, vec2_sub(max, min));
}