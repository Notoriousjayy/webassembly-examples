/**
 * @file geom3d_intersect.c
 * @brief Shape-shape intersection tests
 */
#include "geom3d_intersect.h"
#include "geom3d_primitives.h"
#include "geom3d_queries.h"
#include "geom3d_sat.h"
#include "compare.h"

#include <math.h>

/*******************************************************************************
 * Shape-Shape Intersection Tests
 ******************************************************************************/

bool sphere_sphere(Sphere s1, Sphere s2) {
    float radii_sum = s1.radius + s2.radius;
    float dist_sq = vec3_magnitude_sq(vec3_sub(s1.position, s2.position));
    return dist_sq < radii_sum * radii_sum;
}

bool sphere_aabb(Sphere sphere, AABB aabb) {
    Point3D closest = closest_point_on_aabb(aabb, sphere.position);
    float dist_sq = vec3_magnitude_sq(vec3_sub(sphere.position, closest));
    float radius_sq = sphere.radius * sphere.radius;
    return dist_sq < radius_sq;
}

bool sphere_obb(Sphere sphere, OBB obb) {
    Point3D closest = closest_point_on_obb(obb, sphere.position);
    float dist_sq = vec3_magnitude_sq(vec3_sub(sphere.position, closest));
    float radius_sq = sphere.radius * sphere.radius;
    return dist_sq < radius_sq;
}

bool sphere_plane(Sphere sphere, Plane plane) {
    Point3D closest = closest_point_on_plane(plane, sphere.position);
    float dist_sq = vec3_magnitude_sq(vec3_sub(sphere.position, closest));
    float radius_sq = sphere.radius * sphere.radius;
    return dist_sq < radius_sq;
}

bool aabb_aabb(AABB a1, AABB a2) {
    vec3 a_min = aabb_get_min(a1);
    vec3 a_max = aabb_get_max(a1);
    vec3 b_min = aabb_get_min(a2);
    vec3 b_max = aabb_get_max(a2);

    return (a_min.x <= b_max.x && a_max.x >= b_min.x) &&
           (a_min.y <= b_max.y && a_max.y >= b_min.y) &&
           (a_min.z <= b_max.z && a_max.z >= b_min.z);
}

bool aabb_obb(AABB aabb, OBB obb) {
    vec3 test[15];
    test[0] = vec3_make(1, 0, 0);  /* AABB axis 1 */
    test[1] = vec3_make(0, 1, 0);  /* AABB axis 2 */
    test[2] = vec3_make(0, 0, 1);  /* AABB axis 3 */
    test[3] = vec3_make(obb.orientation.m[0][0], obb.orientation.m[0][1], obb.orientation.m[0][2]);
    test[4] = vec3_make(obb.orientation.m[1][0], obb.orientation.m[1][1], obb.orientation.m[1][2]);
    test[5] = vec3_make(obb.orientation.m[2][0], obb.orientation.m[2][1], obb.orientation.m[2][2]);

    for (int i = 0; i < 3; ++i) {
        test[6 + i * 3 + 0] = vec3_cross(test[i], test[3]);
        test[6 + i * 3 + 1] = vec3_cross(test[i], test[4]);
        test[6 + i * 3 + 2] = vec3_cross(test[i], test[5]);
    }

    for (int i = 0; i < 15; ++i) {
        if (!overlap_on_axis_aabb_obb(aabb, obb, test[i])) {
            return false;
        }
    }
    return true;
}

bool aabb_plane(AABB aabb, Plane plane) {
    float p_len = aabb.size.x * fabsf(plane.normal.x) +
                  aabb.size.y * fabsf(plane.normal.y) +
                  aabb.size.z * fabsf(plane.normal.z);
    float dist = vec3_dot(plane.normal, aabb.position) - plane.distance;
    return fabsf(dist) <= p_len;
}

bool obb_obb(OBB o1, OBB o2) {
    vec3 test[15];
    test[0] = vec3_make(o1.orientation.m[0][0], o1.orientation.m[0][1], o1.orientation.m[0][2]);
    test[1] = vec3_make(o1.orientation.m[1][0], o1.orientation.m[1][1], o1.orientation.m[1][2]);
    test[2] = vec3_make(o1.orientation.m[2][0], o1.orientation.m[2][1], o1.orientation.m[2][2]);
    test[3] = vec3_make(o2.orientation.m[0][0], o2.orientation.m[0][1], o2.orientation.m[0][2]);
    test[4] = vec3_make(o2.orientation.m[1][0], o2.orientation.m[1][1], o2.orientation.m[1][2]);
    test[5] = vec3_make(o2.orientation.m[2][0], o2.orientation.m[2][1], o2.orientation.m[2][2]);

    for (int i = 0; i < 3; ++i) {
        test[6 + i * 3 + 0] = vec3_cross(test[i], test[3]);
        test[6 + i * 3 + 1] = vec3_cross(test[i], test[4]);
        test[6 + i * 3 + 2] = vec3_cross(test[i], test[5]);
    }

    for (int i = 0; i < 15; ++i) {
        if (!overlap_on_axis_obb_obb(o1, o2, test[i])) {
            return false;
        }
    }
    return true;
}

bool obb_plane(OBB obb, Plane plane) {
    vec3 rot[3];
    rot[0] = vec3_make(obb.orientation.m[0][0], obb.orientation.m[0][1], obb.orientation.m[0][2]);
    rot[1] = vec3_make(obb.orientation.m[1][0], obb.orientation.m[1][1], obb.orientation.m[1][2]);
    rot[2] = vec3_make(obb.orientation.m[2][0], obb.orientation.m[2][1], obb.orientation.m[2][2]);

    float p_len = obb.size.x * fabsf(vec3_dot(plane.normal, rot[0])) +
                  obb.size.y * fabsf(vec3_dot(plane.normal, rot[1])) +
                  obb.size.z * fabsf(vec3_dot(plane.normal, rot[2]));
    float dist = vec3_dot(plane.normal, obb.position) - plane.distance;
    return fabsf(dist) <= p_len;
}

bool plane_plane(Plane p1, Plane p2) {
    vec3 d = vec3_cross(p1.normal, p2.normal);
    return !CMP(vec3_dot(d, d), 0.0f);
}

bool triangle_sphere(Triangle t, Sphere s) {
    Point3D closest = closest_point_on_triangle(t, s.position);
    float mag_sq = vec3_magnitude_sq(vec3_sub(closest, s.position));
    return mag_sq <= s.radius * s.radius;
}

bool triangle_aabb(Triangle t, AABB a) {
    vec3 f0 = vec3_sub(t.b, t.a);
    vec3 f1 = vec3_sub(t.c, t.b);
    vec3 f2 = vec3_sub(t.a, t.c);

    vec3 u0 = vec3_make(1.0f, 0.0f, 0.0f);
    vec3 u1 = vec3_make(0.0f, 1.0f, 0.0f);
    vec3 u2 = vec3_make(0.0f, 0.0f, 1.0f);

    vec3 test[13] = {
        u0, u1, u2,
        vec3_cross(f0, f1),
        vec3_cross(u0, f0), vec3_cross(u0, f1), vec3_cross(u0, f2),
        vec3_cross(u1, f0), vec3_cross(u1, f1), vec3_cross(u1, f2),
        vec3_cross(u2, f0), vec3_cross(u2, f1), vec3_cross(u2, f2)
    };

    for (int i = 0; i < 13; ++i) {
        if (!overlap_on_axis_aabb_triangle(a, t, test[i])) {
            return false;
        }
    }
    return true;
}

bool triangle_obb(Triangle t, OBB o) {
    vec3 f0 = vec3_sub(t.b, t.a);
    vec3 f1 = vec3_sub(t.c, t.b);
    vec3 f2 = vec3_sub(t.a, t.c);

    vec3 u0 = vec3_make(o.orientation.m[0][0], o.orientation.m[0][1], o.orientation.m[0][2]);
    vec3 u1 = vec3_make(o.orientation.m[1][0], o.orientation.m[1][1], o.orientation.m[1][2]);
    vec3 u2 = vec3_make(o.orientation.m[2][0], o.orientation.m[2][1], o.orientation.m[2][2]);

    vec3 test[13] = {
        u0, u1, u2,
        vec3_cross(f0, f1),
        vec3_cross(u0, f0), vec3_cross(u0, f1), vec3_cross(u0, f2),
        vec3_cross(u1, f0), vec3_cross(u1, f1), vec3_cross(u1, f2),
        vec3_cross(u2, f0), vec3_cross(u2, f1), vec3_cross(u2, f2)
    };

    for (int i = 0; i < 13; ++i) {
        if (!overlap_on_axis_obb_triangle(o, t, test[i])) {
            return false;
        }
    }
    return true;
}

bool triangle_plane(Triangle t, Plane p) {
    float side1 = plane_equation(t.a, p);
    float side2 = plane_equation(t.b, p);
    float side3 = plane_equation(t.c, p);

    if (CMP(side1, 0.0f) && CMP(side2, 0.0f) && CMP(side3, 0.0f)) {
        return true;
    }
    if (side1 > 0 && side2 > 0 && side3 > 0) {
        return false;
    }
    if (side1 < 0 && side2 < 0 && side3 < 0) {
        return false;
    }
    return true;
}

bool triangle_triangle(Triangle t1, Triangle t2) {
    vec3 t1_f0 = vec3_sub(t1.b, t1.a);
    vec3 t1_f1 = vec3_sub(t1.c, t1.b);
    vec3 t1_f2 = vec3_sub(t1.a, t1.c);

    vec3 t2_f0 = vec3_sub(t2.b, t2.a);
    vec3 t2_f1 = vec3_sub(t2.c, t2.b);
    vec3 t2_f2 = vec3_sub(t2.a, t2.c);

    vec3 axes[11] = {
        vec3_cross(t1_f0, t1_f1),
        vec3_cross(t2_f0, t2_f1),
        vec3_cross(t2_f0, t1_f0), vec3_cross(t2_f0, t1_f1), vec3_cross(t2_f0, t1_f2),
        vec3_cross(t2_f1, t1_f0), vec3_cross(t2_f1, t1_f1), vec3_cross(t2_f1, t1_f2),
        vec3_cross(t2_f2, t1_f0), vec3_cross(t2_f2, t1_f1), vec3_cross(t2_f2, t1_f2)
    };

    for (int i = 0; i < 11; ++i) {
        if (!overlap_on_axis_triangle_triangle(t1, t2, axes[i])) {
            return false;
        }
    }
    return true;
}

bool triangle_triangle_robust(Triangle t1, Triangle t2) {
    vec3 axes[11] = {
        sat_cross_edge(t1.a, t1.b, t1.b, t1.c),
        sat_cross_edge(t2.a, t2.b, t2.b, t2.c),
        sat_cross_edge(t2.a, t2.b, t1.a, t1.b),
        sat_cross_edge(t2.a, t2.b, t1.b, t1.c),
        sat_cross_edge(t2.a, t2.b, t1.c, t1.a),
        sat_cross_edge(t2.b, t2.c, t1.a, t1.b),
        sat_cross_edge(t2.b, t2.c, t1.b, t1.c),
        sat_cross_edge(t2.b, t2.c, t1.c, t1.a),
        sat_cross_edge(t2.c, t2.a, t1.a, t1.b),
        sat_cross_edge(t2.c, t2.a, t1.b, t1.c),
        sat_cross_edge(t2.c, t2.a, t1.c, t1.a)
    };

    for (int i = 0; i < 11; ++i) {
        if (!overlap_on_axis_triangle_triangle(t1, t2, axes[i])) {
            if (!CMP(vec3_magnitude_sq(axes[i]), 0.0f)) {
                return false;
            }
        }
    }
    return true;
}
