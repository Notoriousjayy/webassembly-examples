/**
 * @file geom3d_queries.h
 * @brief Point containment tests and closest point functions
 */
#ifndef GEOM3D_QUERIES_H
#define GEOM3D_QUERIES_H

#include "geom3d_types.h"

/*******************************************************************************
 * Point Containment Tests
 ******************************************************************************/

bool point_in_sphere(Point3D point, Sphere sphere);
bool point_in_aabb(Point3D point, AABB aabb);
bool point_in_obb(Point3D point, OBB obb);
bool point_on_plane(Point3D point, Plane plane);
bool point_on_line3d(Point3D point, Line3D line);
bool point_on_ray3d(Point3D point, Ray3D ray);
bool point_in_triangle(Point3D point, Triangle triangle);

#ifndef NO_EXTRAS
/* Alias functions for convenience */
bool point_in_plane(Point3D point, Plane plane);
bool point_in_line3d(Point3D point, Line3D line);
bool point_in_ray3d(Point3D point, Ray3D ray);

/* ContainsPoint variants (all map to point_in_* functions) */
#define contains_point_sphere_point(sphere, point) point_in_sphere(point, sphere)
#define contains_point_point_sphere(point, sphere) point_in_sphere(point, sphere)
#define contains_point_aabb_point(aabb, point)     point_in_aabb(point, aabb)
#define contains_point_point_aabb(point, aabb)     point_in_aabb(point, aabb)
#define contains_point_obb_point(obb, point)       point_in_obb(point, obb)
#define contains_point_point_obb(point, obb)       point_in_obb(point, obb)
#define contains_point_plane_point(plane, point)   point_on_plane(point, plane)
#define contains_point_point_plane(point, plane)   point_on_plane(point, plane)
#define contains_point_line_point(line, point)     point_on_line3d(point, line)
#define contains_point_point_line(point, line)     point_on_line3d(point, line)
#define contains_point_ray_point(ray, point)       point_on_ray3d(point, ray)
#define contains_point_point_ray(point, ray)       point_on_ray3d(point, ray)
#endif

/*******************************************************************************
 * Closest Point Functions
 ******************************************************************************/

Point3D closest_point_on_sphere(Sphere sphere, Point3D point);
Point3D closest_point_on_aabb(AABB aabb, Point3D point);
Point3D closest_point_on_obb(OBB obb, Point3D point);
Point3D closest_point_on_plane(Plane plane, Point3D point);
Point3D closest_point_on_line3d(Line3D line, Point3D point);
Point3D closest_point_on_ray3d(Ray3D ray, Point3D point);
Point3D closest_point_on_triangle(Triangle triangle, Point3D point);

#ifndef NO_EXTRAS
/* Reversed argument order variants */
#define closest_point_point_sphere(point, sphere)     closest_point_on_sphere(sphere, point)
#define closest_point_point_aabb(point, aabb)         closest_point_on_aabb(aabb, point)
#define closest_point_point_obb(point, obb)           closest_point_on_obb(obb, point)
#define closest_point_point_plane(point, plane)       closest_point_on_plane(plane, point)
#define closest_point_point_line3d(point, line)       closest_point_on_line3d(line, point)
#define closest_point_point_ray3d(point, ray)         closest_point_on_ray3d(ray, point)
#define closest_point_point_triangle(point, triangle) closest_point_on_triangle(triangle, point)
#endif

#endif /* GEOM3D_QUERIES_H */
