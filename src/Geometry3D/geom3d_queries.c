/**
 * @file geom3d_queries.c
 * @brief Point containment tests and closest point functions
 */
#include "geom3d_queries.h"
#include "geom3d_primitives.h"
#include "compare.h"

#include <math.h>

/*******************************************************************************
 * Point Containment Tests
 ******************************************************************************/

bool point_in_sphere(Point3D point, Sphere sphere) {
    float dist_sq = vec3_magnitude_sq(vec3_sub(point, sphere.position));
    return dist_sq < sphere.radius * sphere.radius;
}

bool point_on_plane(Point3D point, Plane plane) {
    return CMP(vec3_dot(point, plane.normal) - plane.distance, 0.0f);
}

bool point_in_aabb(Point3D point, AABB aabb) {
    vec3 min = aabb_get_min(aabb);
    vec3 max = aabb_get_max(aabb);

    if (point.x < min.x || point.y < min.y || point.z < min.z) {
        return false;
    }
    if (point.x > max.x || point.y > max.y || point.z > max.z) {
        return false;
    }
    return true;
}

bool point_in_obb(Point3D point, OBB obb) {
    vec3 dir = vec3_sub(point, obb.position);

    for (int i = 0; i < 3; ++i) {
        vec3 axis = vec3_make(
            obb.orientation.m[i][0],
            obb.orientation.m[i][1],
            obb.orientation.m[i][2]
        );
        float distance = vec3_dot(dir, axis);
        float extent = obb.size.v[i];

        if (distance > extent || distance < -extent) {
            return false;
        }
    }
    return true;
}

bool point_on_line3d(Point3D point, Line3D line) {
    Point3D closest = closest_point_on_line3d(line, point);
    float dist_sq = vec3_magnitude_sq(vec3_sub(closest, point));
    return CMP(dist_sq, 0.0f);
}

bool point_on_ray3d(Point3D point, Ray3D ray) {
    if (vec3_equal(point, ray.origin)) {
        return true;
    }
    vec3 norm = vec3_normalized(vec3_sub(point, ray.origin));
    float diff = vec3_dot(norm, ray.direction);
    return CMP(diff, 1.0f);
}

bool point_in_triangle(Point3D p, Triangle t) {
    vec3 a = vec3_sub(t.a, p);
    vec3 b = vec3_sub(t.b, p);
    vec3 c = vec3_sub(t.c, p);

    vec3 norm_pbc = vec3_cross(b, c);
    vec3 norm_pca = vec3_cross(c, a);
    vec3 norm_pab = vec3_cross(a, b);

    if (vec3_dot(norm_pbc, norm_pca) < 0.0f) {
        return false;
    }
    if (vec3_dot(norm_pbc, norm_pab) < 0.0f) {
        return false;
    }
    return true;
}

#ifndef NO_EXTRAS
bool point_in_plane(Point3D point, Plane plane) {
    return point_on_plane(point, plane);
}

bool point_in_line3d(Point3D point, Line3D line) {
    return point_on_line3d(point, line);
}

bool point_in_ray3d(Point3D point, Ray3D ray) {
    return point_on_ray3d(point, ray);
}
#endif

/*******************************************************************************
 * Closest Point Functions
 ******************************************************************************/

Point3D closest_point_on_sphere(Sphere sphere, Point3D point) {
    vec3 dir = vec3_sub(point, sphere.position);
    dir = vec3_normalized(dir);
    dir = vec3_scale(dir, sphere.radius);
    return vec3_add(dir, sphere.position);
}

Point3D closest_point_on_aabb(AABB aabb, Point3D point) {
    Point3D result = point;
    vec3 min = aabb_get_min(aabb);
    vec3 max = aabb_get_max(aabb);

    /* Clamp to min */
    result.x = (result.x < min.x) ? min.x : result.x;
    result.y = (result.y < min.y) ? min.y : result.y;
    result.z = (result.z < min.z) ? min.z : result.z;

    /* Clamp to max */
    result.x = (result.x > max.x) ? max.x : result.x;
    result.y = (result.y > max.y) ? max.y : result.y;
    result.z = (result.z > max.z) ? max.z : result.z;

    return result;
}

Point3D closest_point_on_obb(OBB obb, Point3D point) {
    Point3D result = obb.position;
    vec3 dir = vec3_sub(point, obb.position);

    for (int i = 0; i < 3; ++i) {
        vec3 axis = vec3_make(
            obb.orientation.m[i][0],
            obb.orientation.m[i][1],
            obb.orientation.m[i][2]
        );
        float distance = vec3_dot(dir, axis);
        float extent = obb.size.v[i];

        if (distance > extent) {
            distance = extent;
        }
        if (distance < -extent) {
            distance = -extent;
        }
        result = vec3_add(result, vec3_scale(axis, distance));
    }
    return result;
}

Point3D closest_point_on_plane(Plane plane, Point3D point) {
    float distance = vec3_dot(plane.normal, point) - plane.distance;
    return vec3_sub(point, vec3_scale(plane.normal, distance));
}

Point3D closest_point_on_line3d(Line3D line, Point3D point) {
    vec3 line_vec = vec3_sub(line.end, line.start);
    float t = vec3_dot(vec3_sub(point, line.start), line_vec) / vec3_dot(line_vec, line_vec);
    t = fmaxf(t, 0.0f);
    t = fminf(t, 1.0f);
    return vec3_add(line.start, vec3_scale(line_vec, t));
}

Point3D closest_point_on_ray3d(Ray3D ray, Point3D point) {
    float t = vec3_dot(vec3_sub(point, ray.origin), ray.direction);
    t = fmaxf(t, 0.0f);
    return vec3_add(ray.origin, vec3_scale(ray.direction, t));
}

Point3D closest_point_on_triangle(Triangle t, Point3D p) {
    Plane plane = plane_from_triangle(t);
    Point3D closest = closest_point_on_plane(plane, p);

    if (point_in_triangle(closest, t)) {
        return closest;
    }

    Point3D c1 = closest_point_on_line3d(line3d_create(t.a, t.b), closest);
    Point3D c2 = closest_point_on_line3d(line3d_create(t.b, t.c), closest);
    Point3D c3 = closest_point_on_line3d(line3d_create(t.c, t.a), closest);

    float mag_sq1 = vec3_magnitude_sq(vec3_sub(closest, c1));
    float mag_sq2 = vec3_magnitude_sq(vec3_sub(closest, c2));
    float mag_sq3 = vec3_magnitude_sq(vec3_sub(closest, c3));

    if (mag_sq1 < mag_sq2 && mag_sq1 < mag_sq3) {
        return c1;
    }
    else if (mag_sq2 < mag_sq1 && mag_sq2 < mag_sq3) {
        return c2;
    }
    return c3;
}
