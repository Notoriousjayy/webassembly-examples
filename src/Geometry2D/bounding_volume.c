#include "Geometry2D/bounding_volume.h"
#include "Geometry2D/rectangle2d.h"

#include <math.h>

/*******************************************************************************
 * Bounding Volume Generation
 ******************************************************************************/

Circle containing_circle(Point2D* points, int count) {
    if (count <= 0) {
        return circle_default();
    }

    // Compute centroid
    Point2D center = {0, 0};
    for (int i = 0; i < count; ++i) {
        center = vec2_add(center, points[i]);
    }
    center = vec2_scale(center, 1.0f / (float)count);

    // Find maximum squared distance from center
    float radius_sq = vec2_magnitude_sq(vec2_sub(center, points[0]));
    for (int i = 1; i < count; ++i) {
        float dist_sq = vec2_magnitude_sq(vec2_sub(center, points[i]));
        if (dist_sq > radius_sq) {
            radius_sq = dist_sq;
        }
    }

    return circle_create(center, sqrtf(radius_sq));
}

#ifndef NO_EXTRAS
Circle containing_circle_alt(Point2D* points, int count) {
    if (count <= 0) {
        return circle_default();
    }

    vec2 min = points[0];
    vec2 max = points[0];

    for (int i = 1; i < count; ++i) {
        min.x = (points[i].x < min.x) ? points[i].x : min.x;
        min.y = (points[i].y < min.y) ? points[i].y : min.y;
        max.x = (points[i].x > max.x) ? points[i].x : max.x;
        max.y = (points[i].y > max.y) ? points[i].y : max.y;
    }

    vec2 center = vec2_scale(vec2_add(min, max), 0.5f);
    float radius = vec2_magnitude(vec2_sub(max, min)) * 0.5f;

    return circle_create(center, radius);
}
#endif

Rectangle2D containing_rectangle(Point2D* points, int count) {
    if (count <= 0) {
        return rectangle2d_default();
    }

    vec2 min = points[0];
    vec2 max = points[0];

    for (int i = 1; i < count; ++i) {
        min.x = (points[i].x < min.x) ? points[i].x : min.x;
        min.y = (points[i].y < min.y) ? points[i].y : min.y;
        max.x = (points[i].x > max.x) ? points[i].x : max.x;
        max.y = (points[i].y > max.y) ? points[i].y : max.y;
    }

    return rectangle2d_from_min_max(min, max);
}