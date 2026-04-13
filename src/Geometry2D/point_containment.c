#include "point_containment.h"
#include "line2d.h"
#include "Geometry2D/rectangle2d.h"
#include "geometry2d_internal.h"
#include "compare.h"

/*******************************************************************************
 * Point Containment Tests
 ******************************************************************************/

bool point_on_line2d(Point2D point, Line2D line) {
    // Find the slope
    float dx = line.end.x - line.start.x;
    float dy = line.end.y - line.start.y;
    
    // Handle vertical line
    if (CMP(dx, 0.0f)) {
        return CMP(point.x, line.start.x);
    }
    
    float M = dy / dx;
    float B = line.start.y - M * line.start.x;
    return CMP(point.y, M * point.x + B);
}

bool point_in_circle(Point2D point, Circle circle) {
    Line2D line = line2d_create(point, circle.position);
    float dist_sq = line2d_length_sq(line);
    return dist_sq < circle.radius * circle.radius;
}

bool point_in_rectangle2d(Point2D point, Rectangle2D rect) {
    vec2 min = rectangle2d_get_min(rect);
    vec2 max = rectangle2d_get_max(rect);

    return min.x <= point.x && min.y <= point.y &&
           point.x <= max.x && point.y <= max.y;
}

bool point_in_oriented_rectangle(Point2D point, OrientedRectangle rect) {
    // Create local-space rectangle (origin at 0,0, size = 2 * half_extents)
    Rectangle2D local_rect = rectangle2d_create(
        vec2_make(0.0f, 0.0f), 
        vec2_scale(rect.half_extents, 2.0f)
    );

    // Get vector from rectangle center to point
    vec2 rot_vector = vec2_sub(point, rect.position);

    // Rotate by inverse of rectangle's rotation
    float theta = -DEG2RAD(rect.rotation);
    rot_vector = rotate_vec2(rot_vector, theta);

    // Offset to account for Rectangle2D having origin at corner
    vec2 local_point = vec2_add(rot_vector, rect.half_extents);

    return point_in_rectangle2d(local_point, local_rect);
}
