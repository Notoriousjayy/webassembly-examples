/**
 * @file geom3d_primitives.c
 * @brief Basic primitive operations (Line3D, Ray3D, AABB, Plane)
 */
#include "geom3d_primitives.h"

#include <math.h>

/*******************************************************************************
 * Line3D Operations
 ******************************************************************************/

float line3d_length(Line3D line) {
    return vec3_magnitude(vec3_sub(line.start, line.end));
}

float line3d_length_sq(Line3D line) {
    return vec3_magnitude_sq(vec3_sub(line.start, line.end));
}

/*******************************************************************************
 * Ray3D Operations
 ******************************************************************************/

Ray3D ray3d_from_points(Point3D from, Point3D to) {
    return ray3d_create(from, vec3_sub(to, from));
}

/*******************************************************************************
 * AABB Operations
 ******************************************************************************/

vec3 aabb_get_min(AABB aabb) {
    vec3 p1 = vec3_add(aabb.position, aabb.size);
    vec3 p2 = vec3_sub(aabb.position, aabb.size);
    return vec3_make(
        fminf(p1.x, p2.x),
        fminf(p1.y, p2.y),
        fminf(p1.z, p2.z)
    );
}

vec3 aabb_get_max(AABB aabb) {
    vec3 p1 = vec3_add(aabb.position, aabb.size);
    vec3 p2 = vec3_sub(aabb.position, aabb.size);
    return vec3_make(
        fmaxf(p1.x, p2.x),
        fmaxf(p1.y, p2.y),
        fmaxf(p1.z, p2.z)
    );
}

AABB aabb_from_min_max(vec3 min, vec3 max) {
    return aabb_create(
        vec3_scale(vec3_add(min, max), 0.5f),
        vec3_scale(vec3_sub(max, min), 0.5f)
    );
}

/*******************************************************************************
 * Plane Operations
 ******************************************************************************/

float plane_equation(Point3D point, Plane plane) {
    return vec3_dot(point, plane.normal) - plane.distance;
}

#ifndef NO_EXTRAS
float plane_equation_reversed(Plane plane, Point3D point) {
    return vec3_dot(point, plane.normal) - plane.distance;
}
#endif

Plane plane_from_triangle(Triangle t) {
    Plane result;
    result.normal = vec3_normalized(vec3_cross(
        vec3_sub(t.b, t.a),
        vec3_sub(t.c, t.a)
    ));
    result.distance = vec3_dot(result.normal, t.a);
    return result;
}

/*******************************************************************************
 * Debug Print Functions
 ******************************************************************************/

#ifndef NO_EXTRAS
void line3d_print(FILE* stream, Line3D shape) {
    fprintf(stream, "start: (%.4f, %.4f, %.4f), end: (%.4f, %.4f, %.4f)",
            shape.start.x, shape.start.y, shape.start.z,
            shape.end.x, shape.end.y, shape.end.z);
}

void ray3d_print(FILE* stream, Ray3D shape) {
    fprintf(stream, "origin: (%.4f, %.4f, %.4f), direction: (%.4f, %.4f, %.4f)",
            shape.origin.x, shape.origin.y, shape.origin.z,
            shape.direction.x, shape.direction.y, shape.direction.z);
}

void sphere_print(FILE* stream, Sphere shape) {
    fprintf(stream, "position: (%.4f, %.4f, %.4f), radius: %.4f",
            shape.position.x, shape.position.y, shape.position.z, shape.radius);
}

void aabb_print(FILE* stream, AABB shape) {
    vec3 min = aabb_get_min(shape);
    vec3 max = aabb_get_max(shape);
    fprintf(stream, "min: (%.4f, %.4f, %.4f), max: (%.4f, %.4f, %.4f)",
            min.x, min.y, min.z, max.x, max.y, max.z);
}

void plane_print(FILE* stream, Plane shape) {
    fprintf(stream, "normal: (%.4f, %.4f, %.4f), distance: %.4f",
            shape.normal.x, shape.normal.y, shape.normal.z, shape.distance);
}

void triangle_print(FILE* stream, Triangle shape) {
    fprintf(stream, "a: (%.4f, %.4f, %.4f), b: (%.4f, %.4f, %.4f), c: (%.4f, %.4f, %.4f)",
            shape.a.x, shape.a.y, shape.a.z,
            shape.b.x, shape.b.y, shape.b.z,
            shape.c.x, shape.c.y, shape.c.z);
}

void obb_print(FILE* stream, OBB shape) {
    fprintf(stream, "position: (%.4f, %.4f, %.4f), ", 
            shape.position.x, shape.position.y, shape.position.z);
    fprintf(stream, "size: (%.4f, %.4f, %.4f), ",
            shape.size.x, shape.size.y, shape.size.z);
    fprintf(stream, "x basis: (%.4f, %.4f, %.4f), ",
            shape.orientation.m[0][0], shape.orientation.m[1][0], shape.orientation.m[2][0]);
    fprintf(stream, "y basis: (%.4f, %.4f, %.4f), ",
            shape.orientation.m[0][1], shape.orientation.m[1][1], shape.orientation.m[2][1]);
    fprintf(stream, "z basis: (%.4f, %.4f, %.4f)",
            shape.orientation.m[0][2], shape.orientation.m[1][2], shape.orientation.m[2][2]);
}
#endif
