/**
 * @file geom3d_frustum.h
 * @brief Frustum operations and classification functions
 */
#ifndef GEOM3D_FRUSTUM_H
#define GEOM3D_FRUSTUM_H

#include "geom3d_types.h"

/*******************************************************************************
 * Frustum Operations
 ******************************************************************************/

Point3D plane_intersection(Plane p1, Plane p2, Plane p3);
void    frustum_get_corners(Frustum f, vec3* out_corners);  /* out_corners[8] */

float classify_aabb(AABB aabb, Plane plane);
float classify_obb(OBB obb, Plane plane);

bool frustum_intersects_point(Frustum f, Point3D p);
bool frustum_intersects_sphere(Frustum f, Sphere s);
bool frustum_intersects_aabb(Frustum f, AABB aabb);
bool frustum_intersects_obb(Frustum f, OBB obb);

#endif /* GEOM3D_FRUSTUM_H */
