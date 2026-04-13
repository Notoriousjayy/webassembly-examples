#include "Geometry2D/bounding_shape.h"
#include "point_containment.h"
#include "line_intersection.h"
#include "shape_collision.h"

/*******************************************************************************
 * BoundingShape Tests
 ******************************************************************************/

bool point_in_bounding_shape(BoundingShape shape, Point2D point) {
    for (int i = 0; i < shape.num_circles; ++i) {
        if (point_in_circle(point, shape.circles[i])) {
            return true;
        }
    }
    for (int i = 0; i < shape.num_rectangles; ++i) {
        if (point_in_rectangle2d(point, shape.rectangles[i])) {
            return true;
        }
    }
    return false;
}

#ifndef NO_EXTRAS
bool line2d_bounding_shape(Line2D line, BoundingShape shape) {
    for (int i = 0; i < shape.num_circles; ++i) {
        if (line2d_circle(line, shape.circles[i])) {
            return true;
        }
    }
    for (int i = 0; i < shape.num_rectangles; ++i) {
        if (line2d_rectangle(line, shape.rectangles[i])) {
            return true;
        }
    }
    return false;
}

bool circle_bounding_shape(Circle circle, BoundingShape shape) {
    for (int i = 0; i < shape.num_circles; ++i) {
        if (circle_circle(circle, shape.circles[i])) {
            return true;
        }
    }
    for (int i = 0; i < shape.num_rectangles; ++i) {
        if (circle_rectangle(circle, shape.rectangles[i])) {
            return true;
        }
    }
    return false;
}

bool rectangle_bounding_shape(Rectangle2D rect, BoundingShape shape) {
    for (int i = 0; i < shape.num_circles; ++i) {
        if (rectangle_circle(rect, shape.circles[i])) {
            return true;
        }
    }
    for (int i = 0; i < shape.num_rectangles; ++i) {
        if (rectangle_rectangle(rect, shape.rectangles[i])) {
            return true;
        }
    }
    return false;
}

bool oriented_rectangle_bounding_shape(OrientedRectangle rect, BoundingShape shape) {
    for (int i = 0; i < shape.num_circles; ++i) {
        if (oriented_rectangle_circle(rect, shape.circles[i])) {
            return true;
        }
    }
    for (int i = 0; i < shape.num_rectangles; ++i) {
        if (oriented_rectangle_rectangle(rect, shape.rectangles[i])) {
            return true;
        }
    }
    return false;
}
#endif
