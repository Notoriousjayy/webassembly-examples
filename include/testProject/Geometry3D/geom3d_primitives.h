/**
 * @file geom3d_primitives.h
 * @brief Basic primitive operations (Line3D, Ray3D, AABB, Plane)
 */
#ifndef GEOM3D_PRIMITIVES_H
#define GEOM3D_PRIMITIVES_H

#include "geom3d_types.h"

/*******************************************************************************
 * Line3D Operations
 ******************************************************************************/

float line3d_length(Line3D line);
float line3d_length_sq(Line3D line);

/*******************************************************************************
 * Ray3D Operations
 ******************************************************************************/

Ray3D ray3d_from_points(Point3D from, Point3D to);

/*******************************************************************************
 * AABB Operations
 ******************************************************************************/

vec3 aabb_get_min(AABB aabb);
vec3 aabb_get_max(AABB aabb);
AABB aabb_from_min_max(vec3 min, vec3 max);

/*******************************************************************************
 * Plane Operations
 ******************************************************************************/

float plane_equation(Point3D point, Plane plane);
Plane plane_from_triangle(Triangle t);

#ifndef NO_EXTRAS
float plane_equation_reversed(Plane plane, Point3D point);
#endif

/*******************************************************************************
 * Debug Print Functions
 ******************************************************************************/

#ifndef NO_EXTRAS
void line3d_print(FILE* stream, Line3D shape);
void ray3d_print(FILE* stream, Ray3D shape);
void sphere_print(FILE* stream, Sphere shape);
void aabb_print(FILE* stream, AABB shape);
void obb_print(FILE* stream, OBB shape);
void plane_print(FILE* stream, Plane shape);
void triangle_print(FILE* stream, Triangle shape);
#endif

#endif /* GEOM3D_PRIMITIVES_H */
