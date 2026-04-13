/**
 * @file geom3d_raycast.h
 * @brief Raycasting and line test functions
 */
#ifndef GEOM3D_RAYCAST_H
#define GEOM3D_RAYCAST_H

#include "geom3d_types.h"

/*******************************************************************************
 * Raycasting
 ******************************************************************************/

bool raycast_sphere(Sphere sphere, Ray3D ray, RaycastResult* out_result);
bool raycast_aabb(AABB aabb, Ray3D ray, RaycastResult* out_result);
bool raycast_obb(OBB obb, Ray3D ray, RaycastResult* out_result);
bool raycast_plane(Plane plane, Ray3D ray, RaycastResult* out_result);
bool raycast_triangle(Triangle triangle, Ray3D ray, RaycastResult* out_result);

#ifndef NO_EXTRAS
/* Reversed argument order variants */
#define raycast_ray_sphere(ray, sphere, result) raycast_sphere(sphere, ray, result)
#define raycast_ray_aabb(ray, aabb, result)     raycast_aabb(aabb, ray, result)
#define raycast_ray_obb(ray, obb, result)       raycast_obb(obb, ray, result)
#define raycast_ray_plane(ray, plane, result)   raycast_plane(plane, ray, result)
#endif

/*******************************************************************************
 * Line Tests
 ******************************************************************************/

bool linetest_sphere(Sphere sphere, Line3D line);
bool linetest_aabb(AABB aabb, Line3D line);
bool linetest_obb(OBB obb, Line3D line);
bool linetest_plane(Plane plane, Line3D line);
bool linetest_triangle(Triangle triangle, Line3D line);

#ifndef NO_EXTRAS
/* Reversed argument order variants */
#define linetest_line_sphere(line, sphere) linetest_sphere(sphere, line)
#define linetest_line_aabb(line, aabb)     linetest_aabb(aabb, line)
#define linetest_line_obb(line, obb)       linetest_obb(obb, line)
#define linetest_line_plane(line, plane)   linetest_plane(plane, line)
#endif

#endif /* GEOM3D_RAYCAST_H */
