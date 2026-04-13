/**
 * @file geom3d_sat.c
 * @brief Separating Axis Theorem (SAT) functions and triangle utilities
 */
#include "geom3d_sat.h"
#include "geom3d_primitives.h"
#include "compare.h"

#include <math.h>

/*******************************************************************************
 * Interval / SAT Functions
 ******************************************************************************/

Interval3D interval3d_from_triangle(Triangle triangle, vec3 axis) {
    Interval3D result;
    result.min = vec3_dot(axis, triangle.points[0]);
    result.max = result.min;

    for (int i = 1; i < 3; ++i) {
        float value = vec3_dot(axis, triangle.points[i]);
        result.min = fminf(result.min, value);
        result.max = fmaxf(result.max, value);
    }
    return result;
}

Interval3D interval3d_from_aabb(AABB aabb, vec3 axis) {
    vec3 mn = aabb_get_min(aabb);
    vec3 mx = aabb_get_max(aabb);

    vec3 vertices[8];
    vertices[0] = vec3_make(mn.x, mx.y, mx.z);
    vertices[1] = vec3_make(mn.x, mx.y, mn.z);
    vertices[2] = vec3_make(mn.x, mn.y, mx.z);
    vertices[3] = vec3_make(mn.x, mn.y, mn.z);
    vertices[4] = vec3_make(mx.x, mx.y, mx.z);
    vertices[5] = vec3_make(mx.x, mx.y, mn.z);
    vertices[6] = vec3_make(mx.x, mn.y, mx.z);
    vertices[7] = vec3_make(mx.x, mn.y, mn.z);

    Interval3D result;
    result.min = result.max = vec3_dot(axis, vertices[0]);

    for (int i = 1; i < 8; ++i) {
        float proj = vec3_dot(axis, vertices[i]);
        result.min = fminf(result.min, proj);
        result.max = fmaxf(result.max, proj);
    }
    return result;
}

Interval3D interval3d_from_obb(OBB obb, vec3 axis) {
    vec3 C = obb.position;
    vec3 E = obb.size;
    vec3 A[3];
    A[0] = vec3_make(obb.orientation.m[0][0], obb.orientation.m[0][1], obb.orientation.m[0][2]);
    A[1] = vec3_make(obb.orientation.m[1][0], obb.orientation.m[1][1], obb.orientation.m[1][2]);
    A[2] = vec3_make(obb.orientation.m[2][0], obb.orientation.m[2][1], obb.orientation.m[2][2]);

    vec3 vertices[8];
    vertices[0] = vec3_add(vec3_add(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[1] = vec3_add(vec3_add(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[2] = vec3_add(vec3_sub(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[3] = vec3_sub(vec3_add(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[4] = vec3_sub(vec3_sub(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[5] = vec3_sub(vec3_sub(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[6] = vec3_sub(vec3_add(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[7] = vec3_add(vec3_sub(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));

    Interval3D result;
    result.min = result.max = vec3_dot(axis, vertices[0]);

    for (int i = 1; i < 8; ++i) {
        float proj = vec3_dot(axis, vertices[i]);
        result.min = fminf(result.min, proj);
        result.max = fmaxf(result.max, proj);
    }
    return result;
}

bool overlap_on_axis_aabb_obb(AABB aabb, OBB obb, vec3 axis) {
    Interval3D a = interval3d_from_aabb(aabb, axis);
    Interval3D b = interval3d_from_obb(obb, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool overlap_on_axis_obb_obb(OBB obb1, OBB obb2, vec3 axis) {
    Interval3D a = interval3d_from_obb(obb1, axis);
    Interval3D b = interval3d_from_obb(obb2, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool overlap_on_axis_aabb_triangle(AABB aabb, Triangle tri, vec3 axis) {
    Interval3D a = interval3d_from_aabb(aabb, axis);
    Interval3D b = interval3d_from_triangle(tri, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool overlap_on_axis_obb_triangle(OBB obb, Triangle tri, vec3 axis) {
    Interval3D a = interval3d_from_obb(obb, axis);
    Interval3D b = interval3d_from_triangle(tri, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool overlap_on_axis_triangle_triangle(Triangle t1, Triangle t2, vec3 axis) {
    Interval3D a = interval3d_from_triangle(t1, axis);
    Interval3D b = interval3d_from_triangle(t2, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

/*******************************************************************************
 * Triangle Utilities
 ******************************************************************************/

#ifndef NO_EXTRAS
vec3 barycentric_optimized(Point3D p, Triangle t) {
    vec3 v0 = vec3_sub(t.b, t.a);
    vec3 v1 = vec3_sub(t.c, t.a);
    vec3 v2 = vec3_sub(p, t.a);

    float d00 = vec3_dot(v0, v0);
    float d01 = vec3_dot(v0, v1);
    float d11 = vec3_dot(v1, v1);
    float d20 = vec3_dot(v2, v0);
    float d21 = vec3_dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;

    if (CMP(denom, 0.0f)) {
        return (vec3){{{0, 0, 0}}};
    }

    vec3 result;
    result.y = (d11 * d20 - d01 * d21) / denom;
    result.z = (d00 * d21 - d01 * d20) / denom;
    result.x = 1.0f - result.y - result.z;
    return result;
}

vec3 triangle_centroid(Triangle t) {
    vec3 result;
    result.x = t.a.x + t.b.x + t.c.x;
    result.y = t.a.y + t.b.y + t.c.y;
    result.z = t.a.z + t.b.z + t.c.z;
    return vec3_scale(result, 1.0f / 3.0f);
}
#endif

vec3 barycentric(Point3D p, Triangle t) {
    vec3 ap = vec3_sub(p, t.a);
    vec3 bp = vec3_sub(p, t.b);
    vec3 cp = vec3_sub(p, t.c);

    vec3 ab = vec3_sub(t.b, t.a);
    vec3 ac = vec3_sub(t.c, t.a);
    vec3 bc = vec3_sub(t.c, t.b);
    vec3 cb = vec3_sub(t.b, t.c);
    vec3 ca = vec3_sub(t.a, t.c);

    vec3 v = vec3_sub(ab, vec3_project(ab, cb));
    float a_coord = 1.0f - (vec3_dot(v, ap) / vec3_dot(v, ab));

    v = vec3_sub(bc, vec3_project(bc, ac));
    float b_coord = 1.0f - (vec3_dot(v, bp) / vec3_dot(v, bc));

    v = vec3_sub(ca, vec3_project(ca, ab));
    float c_coord = 1.0f - (vec3_dot(v, cp) / vec3_dot(v, ca));

    return vec3_make(a_coord, b_coord, c_coord);
}

vec3 sat_cross_edge(vec3 a, vec3 b, vec3 c, vec3 d) {
    vec3 ab = vec3_sub(b, a);
    vec3 cd = vec3_sub(d, c);

    vec3 result = vec3_cross(ab, cd);
    if (!CMP(vec3_magnitude_sq(result), 0.0f)) {
        return result;
    }

    vec3 axis = vec3_cross(ab, vec3_sub(c, a));
    result = vec3_cross(ab, axis);
    if (!CMP(vec3_magnitude_sq(result), 0.0f)) {
        return result;
    }

    return vec3_make(0, 0, 0);
}
