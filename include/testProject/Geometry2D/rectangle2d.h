#ifndef RECTANGLE2D_H
#define RECTANGLE2D_H

#include "geometry2d_types.h"

/*******************************************************************************
 * Rectangle2D Operations
 ******************************************************************************/

vec2        rectangle2d_get_min(Rectangle2D rect);
vec2        rectangle2d_get_max(Rectangle2D rect);
Rectangle2D rectangle2d_from_min_max(vec2 min, vec2 max);

#endif // RECTANGLE2D_H
