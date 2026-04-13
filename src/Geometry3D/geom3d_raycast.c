/**
 * @file geom3d_raycast.c
 * @brief Raycasting and line test functions
 */
#include "geom3d_raycast.h"
#include "geom3d_arrays.h"
#include "geom3d_primitives.h"
#include "geom3d_queries.h"
#include "geom3d_sat.h"
#include "compare.h"

#include <math.h>

/*******************************************************************************
 * Raycasting
 ******************************************************************************/

bool raycast_sphere(Sphere sphere, Ray3D ray, RaycastResult* out_result) {
    raycast_result_reset(out_result);

    vec3 e = vec3_sub(sphere.position, ray.origin);
    float r_sq = sphere.radius * sphere.radius;
    float e_sq = vec3_magnitude_sq(e);
    float a = vec3_dot(e, ray.direction);
    float b_sq = e_sq - (a * a);
    float f = sqrtf(fabsf(r_sq - b_sq));

    float t = a - f;

    if (r_sq - (e_sq - a * a) < 0.0f) {
        return false;
    }
    if (e_sq < r_sq) {
        t = a + f;
    }

    if (out_result) {
        out_result->t = t;
        out_result->hit = true;
        out_result->point = vec3_add(ray.origin, vec3_scale(ray.direction, t));
        out_result->normal = vec3_normalized(vec3_sub(out_result->point, sphere.position));
    }
    return true;
}

bool raycast_aabb(AABB aabb, Ray3D ray, RaycastResult* out_result) {
    raycast_result_reset(out_result);

    vec3 min = aabb_get_min(aabb);
    vec3 max = aabb_get_max(aabb);

    float t1 = (min.x - ray.origin.x) / (CMP(ray.direction.x, 0.0f) ? 0.00001f : ray.direction.x);
    float t2 = (max.x - ray.origin.x) / (CMP(ray.direction.x, 0.0f) ? 0.00001f : ray.direction.x);
    float t3 = (min.y - ray.origin.y) / (CMP(ray.direction.y, 0.0f) ? 0.00001f : ray.direction.y);
    float t4 = (max.y - ray.origin.y) / (CMP(ray.direction.y, 0.0f) ? 0.00001f : ray.direction.y);
    float t5 = (min.z - ray.origin.z) / (CMP(ray.direction.z, 0.0f) ? 0.00001f : ray.direction.z);
    float t6 = (max.z - ray.origin.z) / (CMP(ray.direction.z, 0.0f) ? 0.00001f : ray.direction.z);

    float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
    float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

    if (tmax < 0 || tmin > tmax) {
        return false;
    }

    float t_result = (tmin < 0.0f) ? tmax : tmin;

    if (out_result) {
        out_result->t = t_result;
        out_result->hit = true;
        out_result->point = vec3_add(ray.origin, vec3_scale(ray.direction, t_result));

        vec3 normals[6];
        normals[0] = vec3_make(-1, 0, 0);
        normals[1] = vec3_make(1, 0, 0);
        normals[2] = vec3_make(0, -1, 0);
        normals[3] = vec3_make(0, 1, 0);
        normals[4] = vec3_make(0, 0, -1);
        normals[5] = vec3_make(0, 0, 1);
        float t_vals[6] = {t1, t2, t3, t4, t5, t6};
        for (int i = 0; i < 6; ++i) {
            if (CMP(t_result, t_vals[i])) {
                out_result->normal = normals[i];
            }
        }
    }
    return true;
}

bool raycast_obb(OBB obb, Ray3D ray, RaycastResult* out_result) {
    raycast_result_reset(out_result);

    vec3 p = vec3_sub(obb.position, ray.origin);

    vec3 X = vec3_make(obb.orientation.m[0][0], obb.orientation.m[0][1], obb.orientation.m[0][2]);
    vec3 Y = vec3_make(obb.orientation.m[1][0], obb.orientation.m[1][1], obb.orientation.m[1][2]);
    vec3 Z = vec3_make(obb.orientation.m[2][0], obb.orientation.m[2][1], obb.orientation.m[2][2]);

    vec3 f = vec3_make(vec3_dot(X, ray.direction), vec3_dot(Y, ray.direction), vec3_dot(Z, ray.direction));
    vec3 e = vec3_make(vec3_dot(X, p), vec3_dot(Y, p), vec3_dot(Z, p));

    float t[6] = {0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 3; ++i) {
        if (CMP(f.v[i], 0.0f)) {
            if (-e.v[i] - obb.size.v[i] > 0 || -e.v[i] + obb.size.v[i] < 0) {
                return false;
            }
            f.v[i] = 0.00001f;
        }
        t[i * 2 + 0] = (e.v[i] + obb.size.v[i]) / f.v[i];
        t[i * 2 + 1] = (e.v[i] - obb.size.v[i]) / f.v[i];
    }

    float tmin = fmaxf(fmaxf(fminf(t[0], t[1]), fminf(t[2], t[3])), fminf(t[4], t[5]));
    float tmax = fminf(fminf(fmaxf(t[0], t[1]), fmaxf(t[2], t[3])), fmaxf(t[4], t[5]));

    if (tmax < 0 || tmin > tmax) {
        return false;
    }

    float t_result = (tmin < 0.0f) ? tmax : tmin;

    if (out_result) {
        out_result->hit = true;
        out_result->t = t_result;
        out_result->point = vec3_add(ray.origin, vec3_scale(ray.direction, t_result));

        vec3 normals[6] = {X, vec3_scale(X, -1.0f), Y, vec3_scale(Y, -1.0f), Z, vec3_scale(Z, -1.0f)};
        for (int i = 0; i < 6; ++i) {
            if (CMP(t_result, t[i])) {
                out_result->normal = vec3_normalized(normals[i]);
            }
        }
    }
    return true;
}

bool raycast_plane(Plane plane, Ray3D ray, RaycastResult* out_result) {
    raycast_result_reset(out_result);

    float nd = vec3_dot(ray.direction, plane.normal);
    float pn = vec3_dot(ray.origin, plane.normal);

    if (nd >= 0.0f) {
        return false;
    }

    float t = (plane.distance - pn) / nd;

    if (t >= 0.0f) {
        if (out_result) {
            out_result->t = t;
            out_result->hit = true;
            out_result->point = vec3_add(ray.origin, vec3_scale(ray.direction, t));
            out_result->normal = vec3_normalized(plane.normal);
        }
        return true;
    }
    return false;
}

bool raycast_triangle(Triangle triangle, Ray3D ray, RaycastResult* out_result) {
    raycast_result_reset(out_result);

    Plane plane = plane_from_triangle(triangle);
    RaycastResult plane_result;
    if (!raycast_plane(plane, ray, &plane_result)) {
        return false;
    }
    float t = plane_result.t;

    Point3D result_point = vec3_add(ray.origin, vec3_scale(ray.direction, t));
    vec3 bary = barycentric(result_point, triangle);

    if (bary.x >= 0.0f && bary.x <= 1.0f &&
        bary.y >= 0.0f && bary.y <= 1.0f &&
        bary.z >= 0.0f && bary.z <= 1.0f) {

        if (out_result) {
            out_result->t = t;
            out_result->hit = true;
            out_result->point = result_point;
            out_result->normal = plane.normal;
        }
        return true;
    }
    return false;
}

/*******************************************************************************
 * Line Tests
 ******************************************************************************/

bool linetest_sphere(Sphere sphere, Line3D line) {
    Point3D closest = closest_point_on_line3d(line, sphere.position);
    float dist_sq = vec3_magnitude_sq(vec3_sub(sphere.position, closest));
    return dist_sq <= (sphere.radius * sphere.radius);
}

bool linetest_plane(Plane plane, Line3D line) {
    vec3 ab = vec3_sub(line.end, line.start);
    float n_a = vec3_dot(plane.normal, line.start);
    float n_ab = vec3_dot(plane.normal, ab);

    if (CMP(n_ab, 0.0f)) {
        return false;
    }

    float t = (plane.distance - n_a) / n_ab;
    return t >= 0.0f && t <= 1.0f;
}

bool linetest_aabb(AABB aabb, Line3D line) {
    Ray3D ray = ray3d_create(line.start, vec3_sub(line.end, line.start));
    RaycastResult raycast;
    if (!raycast_aabb(aabb, ray, &raycast)) {
        return false;
    }
    float t = raycast.t;
    return t >= 0 && t * t <= line3d_length_sq(line);
}

bool linetest_obb(OBB obb, Line3D line) {
    if (vec3_magnitude_sq(vec3_sub(line.end, line.start)) < 0.0000001f) {
        return point_in_obb(line.start, obb);
    }
    Ray3D ray = ray3d_create(line.start, vec3_sub(line.end, line.start));
    RaycastResult result;
    if (!raycast_obb(obb, ray, &result)) {
        return false;
    }
    float t = result.t;
    return t >= 0 && t * t <= line3d_length_sq(line);
}

bool linetest_triangle(Triangle triangle, Line3D line) {
    Ray3D ray = ray3d_create(line.start, vec3_sub(line.end, line.start));
    RaycastResult raycast;
    if (!raycast_triangle(triangle, ray, &raycast)) {
        return false;
    }
    float t = raycast.t;
    return t >= 0 && t * t <= line3d_length_sq(line);
}
