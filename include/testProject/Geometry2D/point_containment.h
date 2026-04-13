#ifndef POINT_CONTAINMENT_H
#define POINT_CONTAINMENT_H

#include "geometry2d_types.h"

/*******************************************************************************
 * Point Containment Tests
 ******************************************************************************/

bool point_on_line2d(Point2D point, Line2D line);
bool point_in_circle(Point2D point, Circle circle);
bool point_in_rectangle2d(Point2D point, Rectangle2D rect);
bool point_in_oriented_rectangle(Point2D point, OrientedRectangle rect);

/*******************************************************************************
 * Convenience Macros (Argument Order Swapping)
 ******************************************************************************/

#define point_line2d(point, line)   point_on_line2d(point, line)
#define line2d_point(line, point)   point_on_line2d(point, line)

#ifndef NO_EXTRAS
#define point_circle(point, circle)              point_in_circle(point, circle)
#define circle_point(circle, point)              point_in_circle(point, circle)
#define point_rectangle(point, rect)             point_in_rectangle2d(point, rect)
#define rectangle_point(rect, point)             point_in_rectangle2d(point, rect)
#define point_oriented_rectangle(point, rect)    point_in_oriented_rectangle(point, rect)
#define oriented_rectangle_point(rect, point)    point_in_oriented_rectangle(point, rect)
#endif

#endif // POINT_CONTAINMENT_H
