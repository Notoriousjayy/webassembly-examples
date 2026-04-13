#ifndef SHAPE_COLLISION_H
#define SHAPE_COLLISION_H

#include "geometry2d_types.h"

/*******************************************************************************
 * Shape-Shape Intersection Tests
 ******************************************************************************/

bool circle_circle(Circle c1, Circle c2);
bool circle_rectangle(Circle circle, Rectangle2D rect);
bool circle_oriented_rectangle(Circle circle, OrientedRectangle rect);
bool rectangle_rectangle(Rectangle2D r1, Rectangle2D r2);
bool rectangle_oriented_rectangle(Rectangle2D rect, OrientedRectangle oriented);
bool oriented_rectangle_oriented_rectangle(OrientedRectangle r1, OrientedRectangle r2);

/*******************************************************************************
 * Convenience Macros (Argument Order Swapping)
 ******************************************************************************/

#define rectangle_circle(rect, circle)           circle_rectangle(circle, rect)
#define oriented_rectangle_circle(rect, circle)  circle_oriented_rectangle(circle, rect)
#define oriented_rectangle_rectangle(o, r)       rectangle_oriented_rectangle(r, o)

#endif // SHAPE_COLLISION_H
