#include "shape_collision.h"
#include "line2d.h"
#include "Geometry2D/rectangle2d.h"
#include "sat2d.h"
#include "geometry2d_internal.h"

/*******************************************************************************
 * Shape-Shape Intersection Tests
 ******************************************************************************/

bool circle_circle(Circle c1, Circle c2) {
    Line2D line = line2d_create(c1.position, c2.position);
    float radii_sum = c1.radius + c2.radius;
    return line2d_length_sq(line) <= radii_sum * radii_sum;
}

bool circle_rectangle(Circle circle, Rectangle2D rect) {
    vec2 min = rectangle2d_get_min(rect);
    vec2 max = rectangle2d_get_max(rect);

    // Find closest point on rectangle to circle center
    Point2D closest = circle.position;
    closest.x = (closest.x < min.x) ? min.x : 
                (closest.x > max.x) ? max.x : closest.x;
    closest.y = (closest.y < min.y) ? min.y : 
                (closest.y > max.y) ? max.y : closest.y;

    Line2D line = line2d_create(circle.position, closest);
    return line2d_length_sq(line) <= circle.radius * circle.radius;
}

bool circle_oriented_rectangle(Circle circle, OrientedRectangle rect) {
    float theta = -DEG2RAD(rect.rotation);

    // Transform circle to local space
    vec2 rot_pos = vec2_sub(circle.position, rect.position);
    rot_pos = rotate_vec2(rot_pos, theta);

    Circle local_circle = circle_create(
        vec2_add(rot_pos, rect.half_extents),
        circle.radius
    );

    Rectangle2D local_rect = rectangle2d_create(
        vec2_make(0.0f, 0.0f),
        vec2_scale(rect.half_extents, 2.0f)
    );

    return circle_rectangle(local_circle, local_rect);
}

bool rectangle_rectangle(Rectangle2D r1, Rectangle2D r2) {
    vec2 a_min = rectangle2d_get_min(r1);
    vec2 a_max = rectangle2d_get_max(r1);
    vec2 b_min = rectangle2d_get_min(r2);
    vec2 b_max = rectangle2d_get_max(r2);

    bool x_overlap = (b_min.x <= a_max.x) && (a_min.x <= b_max.x);
    bool y_overlap = (b_min.y <= a_max.y) && (a_min.y <= b_max.y);

    return x_overlap && y_overlap;
}

bool rectangle_oriented_rectangle(Rectangle2D r1, OrientedRectangle r2) {
    float theta = DEG2RAD(r2.rotation);

    // Axes to test: 2 from AABB + 2 from oriented rectangle
    vec2 axes[4] = {
        vec2_make(1.0f, 0.0f),
        vec2_make(0.0f, 1.0f),
        rotate_vec2(vec2_make(1.0f, 0.0f), theta),
        rotate_vec2(vec2_make(0.0f, 1.0f), theta)
    };

    for (int i = 0; i < 4; ++i) {
        if (!overlap_on_axis_rect_oriented(r1, r2, axes[i])) {
            return false;
        }
    }

    return true;
}

bool oriented_rectangle_oriented_rectangle(OrientedRectangle r1, OrientedRectangle r2) {
    // Transform r2 into r1's local space
    Rectangle2D local_r1 = rectangle2d_create(
        vec2_make(0.0f, 0.0f),
        vec2_scale(r1.half_extents, 2.0f)
    );

    OrientedRectangle local_r2 = r2;
    local_r2.rotation = r2.rotation - r1.rotation;

    float theta = -DEG2RAD(r1.rotation);
    vec2 rot_pos = vec2_sub(r2.position, r1.position);
    rot_pos = rotate_vec2(rot_pos, theta);
    local_r2.position = vec2_add(rot_pos, r1.half_extents);

    return rectangle_oriented_rectangle(local_r1, local_r2);
}
