#include "sat2d.h"
#include "Geometry2D/rectangle2d.h"
#include "geometry2d_internal.h"

/*******************************************************************************
 * SAT Functions
 ******************************************************************************/

Interval2D interval2d_from_rectangle(Rectangle2D rect, vec2 axis) {
    vec2 min = rectangle2d_get_min(rect);
    vec2 max = rectangle2d_get_max(rect);

    vec2 verts[4] = {
        vec2_make(min.x, min.y),
        vec2_make(min.x, max.y),
        vec2_make(max.x, max.y),
        vec2_make(max.x, min.y)
    };

    Interval2D result;
    result.min = vec2_dot(axis, verts[0]);
    result.max = result.min;

    for (int i = 1; i < 4; ++i) {
        float proj = vec2_dot(axis, verts[i]);
        result.min = (proj < result.min) ? proj : result.min;
        result.max = (proj > result.max) ? proj : result.max;
    }

    return result;
}

Interval2D interval2d_from_oriented_rectangle(OrientedRectangle rect, vec2 axis) {
    // Create non-oriented rect centered at position
    vec2 min = vec2_sub(rect.position, rect.half_extents);
    vec2 max = vec2_add(rect.position, rect.half_extents);

    vec2 verts[4] = {
        min,
        max,
        vec2_make(min.x, max.y),
        vec2_make(max.x, min.y)
    };

    float theta = DEG2RAD(rect.rotation);

    // Rotate vertices around center
    for (int i = 0; i < 4; ++i) {
        vec2 rel = vec2_sub(verts[i], rect.position);
        rel = rotate_vec2(rel, theta);
        verts[i] = vec2_add(rel, rect.position);
    }

    Interval2D result;
    result.min = result.max = vec2_dot(axis, verts[0]);

    for (int i = 1; i < 4; ++i) {
        float proj = vec2_dot(axis, verts[i]);
        result.min = (proj < result.min) ? proj : result.min;
        result.max = (proj > result.max) ? proj : result.max;
    }

    return result;
}

bool overlap_on_axis_rect_rect(Rectangle2D r1, Rectangle2D r2, vec2 axis) {
    Interval2D a = interval2d_from_rectangle(r1, axis);
    Interval2D b = interval2d_from_rectangle(r2, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool overlap_on_axis_rect_oriented(Rectangle2D r1, OrientedRectangle r2, vec2 axis) {
    Interval2D a = interval2d_from_rectangle(r1, axis);
    Interval2D b = interval2d_from_oriented_rectangle(r2, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool overlap_on_axis_oriented_oriented(OrientedRectangle r1, OrientedRectangle r2, vec2 axis) {
    Interval2D a = interval2d_from_oriented_rectangle(r1, axis);
    Interval2D b = interval2d_from_oriented_rectangle(r2, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool rectangle_rectangle_sat(Rectangle2D r1, Rectangle2D r2) {
    vec2 axes[2] = {
        vec2_make(1.0f, 0.0f),
        vec2_make(0.0f, 1.0f)
    };

    for (int i = 0; i < 2; ++i) {
        if (!overlap_on_axis_rect_rect(r1, r2, axes[i])) {
            return false;
        }
    }

    return true;
}

bool oriented_rectangle_oriented_rectangle_sat(OrientedRectangle r1, OrientedRectangle r2) {
    float theta1 = DEG2RAD(r1.rotation);
    float theta2 = DEG2RAD(r2.rotation);

    vec2 axes[6] = {
        vec2_make(1.0f, 0.0f),
        vec2_make(0.0f, 1.0f),
        rotate_vec2(vec2_make(1.0f, 0.0f), theta2),
        rotate_vec2(vec2_make(0.0f, 1.0f), theta2),
        rotate_vec2(vec2_make(1.0f, 0.0f), theta1),
        rotate_vec2(vec2_make(0.0f, 1.0f), theta1)
    };

    for (int i = 0; i < 6; ++i) {
        if (!overlap_on_axis_oriented_oriented(r1, r2, axes[i])) {
            return false;
        }
    }

    return true;
}
