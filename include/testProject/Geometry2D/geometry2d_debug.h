#ifndef GEOMETRY2D_DEBUG_H
#define GEOMETRY2D_DEBUG_H

#include "geometry2d_types.h"
#include <stdio.h>

/*******************************************************************************
 * Debug Print Functions
 ******************************************************************************/

#ifndef NO_EXTRAS
void line2d_print(FILE* stream, Line2D shape);
void circle_print(FILE* stream, Circle shape);
void rectangle2d_print(FILE* stream, Rectangle2D shape);
void oriented_rectangle_print(FILE* stream, OrientedRectangle shape);
#endif

#endif // GEOMETRY2D_DEBUG_H
