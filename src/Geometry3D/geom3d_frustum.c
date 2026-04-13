/**
 * @file geom3d_frustum.c
 * @brief Frustum operations and classification functions
 */
#include "geom3d_frustum.h"
#include "compare.h"

#include <math.h>

/*******************************************************************************
 * Frustum Operations
 ******************************************************************************/

Point3D plane_intersection(Plane p1, Plane p2, Plane p3) {
    mat3 D = mat3_make(
        p1.normal.x, p2.normal.x, p3.normal.x,
        p1.normal.y, p2.normal.y, p3.normal.y,
        p1.normal.z, p2.normal.z, p3.normal.z
    );

    vec3 A = vec3_make(-p1.distance, -p2.distance, -p3.distance);

    mat3 Dx = D, Dy = D, Dz = D;
    Dx.m[0][0] = A.x; Dx.m[0][1] = A.y; Dx.m[0][2] = A.z;
    Dy.m[1][0] = A.x; Dy.m[1][1] = A.y; Dy.m[1][2] = A.z;
    Dz.m[2][0] = A.x; Dz.m[2][1] = A.y; Dz.m[2][2] = A.z;

    float det_d = mat3_determinant(D);

    if (CMP(det_d, 0.0f)) {
        return vec3_make(0, 0, 0);
    }

    float det_dx = mat3_determinant(Dx);
    float det_dy = mat3_determinant(Dy);
    float det_dz = mat3_determinant(Dz);

    return vec3_make(det_dx / det_d, det_dy / det_d, det_dz / det_d);
}

void frustum_get_corners(Frustum f, vec3* out_corners) {
    out_corners[0] = plane_intersection(f.near_plane, f.top, f.left);
    out_corners[1] = plane_intersection(f.near_plane, f.top, f.right);
    out_corners[2] = plane_intersection(f.near_plane, f.bottom, f.left);
    out_corners[3] = plane_intersection(f.near_plane, f.bottom, f.right);
    out_corners[4] = plane_intersection(f.far_plane, f.top, f.left);
    out_corners[5] = plane_intersection(f.far_plane, f.top, f.right);
    out_corners[6] = plane_intersection(f.far_plane, f.bottom, f.left);
    out_corners[7] = plane_intersection(f.far_plane, f.bottom, f.right);
}

float classify_aabb(AABB aabb, Plane plane) {
    float r = fabsf(aabb.size.x * plane.normal.x) +
              fabsf(aabb.size.y * plane.normal.y) +
              fabsf(aabb.size.z * plane.normal.z);

    float d = vec3_dot(plane.normal, aabb.position) + plane.distance;

    if (fabsf(d) < r) {
        return 0.0f;
    }
    else if (d < 0.0f) {
        return d + r;
    }
    return d - r;
}

float classify_obb(OBB obb, Plane plane) {
    vec3 normal = mat3_multiply_vector(plane.normal, obb.orientation);

    float r = fabsf(obb.size.x * normal.x) +
              fabsf(obb.size.y * normal.y) +
              fabsf(obb.size.z * normal.z);

    float d = vec3_dot(plane.normal, obb.position) + plane.distance;

    if (fabsf(d) < r) {
        return 0.0f;
    }
    else if (d < 0.0f) {
        return d + r;
    }
    return d - r;
}

bool frustum_intersects_point(Frustum f, Point3D p) {
    for (int i = 0; i < 6; ++i) {
        vec3 normal = f.planes[i].normal;
        float dist = f.planes[i].distance;
        float side = vec3_dot(p, normal) + dist;
        if (side < 0.0f) {
            return false;
        }
    }
    return true;
}

bool frustum_intersects_sphere(Frustum f, Sphere s) {
    for (int i = 0; i < 6; ++i) {
        vec3 normal = f.planes[i].normal;
        float dist = f.planes[i].distance;
        float side = vec3_dot(s.position, normal) + dist;
        if (side < -s.radius) {
            return false;
        }
    }
    return true;
}

bool frustum_intersects_aabb(Frustum f, AABB aabb) {
    for (int i = 0; i < 6; ++i) {
        float side = classify_aabb(aabb, f.planes[i]);
        if (side < 0) {
            return false;
        }
    }
    return true;
}

bool frustum_intersects_obb(Frustum f, OBB obb) {
    for (int i = 0; i < 6; ++i) {
        float side = classify_obb(obb, f.planes[i]);
        if (side < 0) {
            return false;
        }
    }
    return true;
}
