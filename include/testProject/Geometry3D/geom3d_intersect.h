/**
 * @file geom3d_intersect.h
 * @brief Shape-shape intersection tests
 */
#ifndef GEOM3D_INTERSECT_H
#define GEOM3D_INTERSECT_H

#include "geom3d_types.h"

/*******************************************************************************
 * Shape-Shape Intersection Tests
 ******************************************************************************/

bool sphere_sphere(Sphere s1, Sphere s2);
bool sphere_aabb(Sphere sphere, AABB aabb);
bool sphere_obb(Sphere sphere, OBB obb);
bool sphere_plane(Sphere sphere, Plane plane);

bool aabb_aabb(AABB a1, AABB a2);
bool aabb_obb(AABB aabb, OBB obb);
bool aabb_plane(AABB aabb, Plane plane);

bool obb_obb(OBB o1, OBB o2);
bool obb_plane(OBB obb, Plane plane);

bool plane_plane(Plane p1, Plane p2);

bool triangle_sphere(Triangle t, Sphere s);
bool triangle_aabb(Triangle t, AABB aabb);
bool triangle_obb(Triangle t, OBB obb);
bool triangle_plane(Triangle t, Plane p);
bool triangle_triangle(Triangle t1, Triangle t2);
bool triangle_triangle_robust(Triangle t1, Triangle t2);

/* Argument order swap macros */
#define aabb_sphere(aabb, sphere)     sphere_aabb(sphere, aabb)
#define obb_sphere(obb, sphere)       sphere_obb(sphere, obb)
#define plane_sphere(plane, sphere)   sphere_plane(sphere, plane)
#define obb_aabb(obb, aabb)           aabb_obb(aabb, obb)
#define plane_aabb(plane, aabb)       aabb_plane(aabb, plane)
#define plane_obb(plane, obb)         obb_plane(obb, plane)

#define sphere_triangle(s, t)         triangle_sphere(t, s)
#define aabb_triangle(a, t)           triangle_aabb(t, a)
#define obb_triangle(o, t)            triangle_obb(t, o)
#define plane_triangle(p, t)          triangle_plane(t, p)

#endif /* GEOM3D_INTERSECT_H */
