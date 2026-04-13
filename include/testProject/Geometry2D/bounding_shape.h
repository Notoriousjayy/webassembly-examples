#ifndef BOUNDING_SHAPE_H
#define BOUNDING_SHAPE_H

#include "geometry2d_types.h"

/*******************************************************************************
 * BoundingShape Tests
 ******************************************************************************/

bool point_in_bounding_shape(BoundingShape shape, Point2D point);

#ifndef NO_EXTRAS
bool line2d_bounding_shape(Line2D line, BoundingShape shape);
bool circle_bounding_shape(Circle circle, BoundingShape shape);
bool rectangle_bounding_shape(Rectangle2D rect, BoundingShape shape);
bool oriented_rectangle_bounding_shape(OrientedRectangle rect, BoundingShape shape);
#endif

/*******************************************************************************
 * Convenience Macros (Argument Order Swapping)
 ******************************************************************************/

#ifndef NO_EXTRAS
#define bounding_shape_line2d(shape, line)                  line2d_bounding_shape(line, shape)
#define bounding_shape_circle(shape, circle)                circle_bounding_shape(circle, shape)
#define bounding_shape_rectangle(shape, rect)               rectangle_bounding_shape(rect, shape)
#define bounding_shape_oriented_rectangle(shape, rect)      oriented_rectangle_bounding_shape(rect, shape)
#endif

#endif // BOUNDING_SHAPE_H
