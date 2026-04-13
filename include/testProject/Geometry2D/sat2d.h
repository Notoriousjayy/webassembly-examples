#ifndef SAT2D_H
#define SAT2D_H

#include "geometry2d_types.h"

/*******************************************************************************
 * SAT (Separating Axis Theorem) Functions
 ******************************************************************************/

Interval2D interval2d_from_rectangle(Rectangle2D rect, vec2 axis);
Interval2D interval2d_from_oriented_rectangle(OrientedRectangle rect, vec2 axis);

bool overlap_on_axis_rect_rect(Rectangle2D r1, Rectangle2D r2, vec2 axis);
bool overlap_on_axis_rect_oriented(Rectangle2D r1, OrientedRectangle r2, vec2 axis);
bool overlap_on_axis_oriented_oriented(OrientedRectangle r1, OrientedRectangle r2, vec2 axis);

bool rectangle_rectangle_sat(Rectangle2D r1, Rectangle2D r2);
bool oriented_rectangle_oriented_rectangle_sat(OrientedRectangle r1, OrientedRectangle r2);

#endif // SAT2D_H
