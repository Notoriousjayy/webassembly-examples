#include "line_intersection.h"
#include "line2d.h"
#include "Geometry2D/rectangle2d.h"
#include "point_containment.h"
#include "geometry2d_internal.h"

#include <math.h>

/*******************************************************************************
 * Line Intersection Tests
 ******************************************************************************/

bool line2d_circle(Line2D line, Circle circle) {
    vec2 ab = vec2_sub(line.end, line.start);

    // Project circle position onto line segment
    float t = vec2_dot(vec2_sub(circle.position, line.start), ab) / vec2_dot(ab, ab);

    // Check if closest point is within segment
    if (t < 0.0f || t > 1.0f) {
        return false;
    }

    // Find closest point on segment
    Point2D closest = vec2_add(line.start, vec2_scale(ab, t));

    Line2D to_closest = line2d_create(circle.position, closest);
    return line2d_length_sq(to_closest) < circle.radius * circle.radius;
}

bool line2d_rectangle(Line2D line, Rectangle2D rect) {
    // Quick check: either endpoint inside rectangle
    if (point_in_rectangle2d(line.start, rect) || 
        point_in_rectangle2d(line.end, rect)) {
        return true;
    }

    vec2 dir = vec2_sub(line.end, line.start);
    vec2 norm = vec2_normalized(dir);
    
    // Avoid division by zero
    norm.x = (norm.x != 0) ? 1.0f / norm.x : 0;
    norm.y = (norm.y != 0) ? 1.0f / norm.y : 0;

    vec2 min = rectangle2d_get_min(rect);
    vec2 max = rectangle2d_get_max(rect);

    vec2 t_min = vec2_mul(vec2_sub(min, line.start), norm);
    vec2 t_max = vec2_mul(vec2_sub(max, line.start), norm);

    float tmin = fmaxf(fminf(t_min.x, t_max.x), fminf(t_min.y, t_max.y));
    float tmax = fminf(fmaxf(t_min.x, t_max.x), fmaxf(t_min.y, t_max.y));

    if (tmax < 0 || tmin > tmax) {
        return false;
    }

    float t = (tmin < 0.0f) ? tmax : tmin;
    return t > 0.0f && t * t < line2d_length_sq(line);
}

bool line2d_oriented_rectangle(Line2D line, OrientedRectangle rect) {
    float theta = -DEG2RAD(rect.rotation);

    // Transform line endpoints to local space
    vec2 rot_start = vec2_sub(line.start, rect.position);
    rot_start = rotate_vec2(rot_start, theta);
    
    vec2 rot_end = vec2_sub(line.end, rect.position);
    rot_end = rotate_vec2(rot_end, theta);

    Line2D local_line = {
        .start = vec2_add(rot_start, rect.half_extents),
        .end = vec2_add(rot_end, rect.half_extents)
    };

    Rectangle2D local_rect = rectangle2d_create(
        (Point2D){0, 0},
        vec2_scale(rect.half_extents, 2.0f)
    );

    return line2d_rectangle(local_line, local_rect);
}