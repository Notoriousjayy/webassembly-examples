#include "geometry2d_debug.h"
#include "Geometry2D/rectangle2d.h"

/*******************************************************************************
 * Debug Print Functions
 ******************************************************************************/

#ifndef NO_EXTRAS
void line2d_print(FILE* stream, Line2D shape) {
    fprintf(stream, "start: (%.3f, %.3f), end: (%.3f, %.3f)",
            shape.start.x, shape.start.y,
            shape.end.x, shape.end.y);
}

void circle_print(FILE* stream, Circle shape) {
    fprintf(stream, "position: (%.3f, %.3f), radius: %.3f",
            shape.position.x, shape.position.y, shape.radius);
}

void rectangle2d_print(FILE* stream, Rectangle2D shape) {
    vec2 min = rectangle2d_get_min(shape);
    vec2 max = rectangle2d_get_max(shape);
    fprintf(stream, "min: (%.3f, %.3f), max: (%.3f, %.3f)",
            min.x, min.y, max.x, max.y);
}

void oriented_rectangle_print(FILE* stream, OrientedRectangle shape) {
    fprintf(stream, "position: (%.3f, %.3f), half size: (%.3f, %.3f), rotation: %.3f",
            shape.position.x, shape.position.y,
            shape.half_extents.x, shape.half_extents.y,
            shape.rotation);
}
#endif