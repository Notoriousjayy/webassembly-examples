/**
 * @file geom3d_sat.h
 * @brief Separating Axis Theorem (SAT) functions and triangle utilities
 */
#ifndef GEOM3D_SAT_H
#define GEOM3D_SAT_H

#include "geom3d_types.h"

/*******************************************************************************
 * Interval / SAT Functions
 ******************************************************************************/

Interval3D interval3d_from_aabb(AABB aabb, vec3 axis);
Interval3D interval3d_from_obb(OBB obb, vec3 axis);
Interval3D interval3d_from_triangle(Triangle triangle, vec3 axis);

bool overlap_on_axis_aabb_obb(AABB aabb, OBB obb, vec3 axis);
bool overlap_on_axis_obb_obb(OBB obb1, OBB obb2, vec3 axis);
bool overlap_on_axis_aabb_triangle(AABB aabb, Triangle tri, vec3 axis);
bool overlap_on_axis_obb_triangle(OBB obb, Triangle tri, vec3 axis);
bool overlap_on_axis_triangle_triangle(Triangle t1, Triangle t2, vec3 axis);

/*******************************************************************************
 * Triangle Utilities
 ******************************************************************************/

vec3 barycentric(Point3D p, Triangle t);
vec3 sat_cross_edge(vec3 a, vec3 b, vec3 c, vec3 d);

#ifndef NO_EXTRAS
vec3 barycentric_optimized(Point3D p, Triangle t);
vec3 triangle_centroid(Triangle t);
#endif

#endif /* GEOM3D_SAT_H */
