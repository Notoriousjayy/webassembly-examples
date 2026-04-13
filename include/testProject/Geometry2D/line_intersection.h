#ifndef LINE_INTERSECTION_H
#define LINE_INTERSECTION_H

#include "geometry2d_types.h"

/*******************************************************************************
 * Line/Shape Intersection Tests
 ******************************************************************************/

bool line2d_circle(Line2D line, Circle circle);
bool line2d_rectangle(Line2D line, Rectangle2D rect);
bool line2d_oriented_rectangle(Line2D line, OrientedRectangle rect);

/*******************************************************************************
 * Convenience Macros (Argument Order Swapping)
 ******************************************************************************/

#define circle_line2d(circle, line)           line2d_circle(line, circle)
#define rectangle_line2d(rect, line)          line2d_rectangle(line, rect)
#define oriented_rectangle_line2d(rect, line) line2d_oriented_rectangle(line, rect)

#endif /* LINE_INTERSECTION_H */
