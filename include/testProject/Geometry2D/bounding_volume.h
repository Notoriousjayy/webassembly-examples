#ifndef BOUNDING_VOLUME_H
#define BOUNDING_VOLUME_H

#include "geometry2d_types.h"

/*******************************************************************************
 * Bounding Volume Generation
 ******************************************************************************/

Circle      containing_circle(Point2D* points, int count);
Rectangle2D containing_rectangle(Point2D* points, int count);

#ifndef NO_EXTRAS
Circle      containing_circle_alt(Point2D* points, int count);
#endif

#endif // BOUNDING_VOLUME_H
