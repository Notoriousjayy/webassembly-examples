/**
 * @file geom3d_collision.c
 * @brief Collision manifold generation and OBB helper functions
 */
#include "geom3d_collision.h"
#include "geom3d_arrays.h"
#include "geom3d_queries.h"
#include "geom3d_intersect.h"
#include "geom3d_sat.h"
#include "compare.h"

#include <math.h>
#include <float.h>

/*******************************************************************************
 * OBB Helper Functions for Collision
 ******************************************************************************/

void obb_get_vertices(OBB obb, vec3* out_vertices) {
    vec3 C = obb.position;
    vec3 E = obb.size;
    vec3 A[3];
    A[0] = vec3_make(obb.orientation.m[0][0], obb.orientation.m[0][1], obb.orientation.m[0][2]);
    A[1] = vec3_make(obb.orientation.m[1][0], obb.orientation.m[1][1], obb.orientation.m[1][2]);
    A[2] = vec3_make(obb.orientation.m[2][0], obb.orientation.m[2][1], obb.orientation.m[2][2]);

    out_vertices[0] = vec3_add(vec3_add(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[1] = vec3_add(vec3_add(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[2] = vec3_add(vec3_sub(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[3] = vec3_sub(vec3_add(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[4] = vec3_sub(vec3_sub(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[5] = vec3_sub(vec3_sub(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[6] = vec3_sub(vec3_add(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[7] = vec3_add(vec3_sub(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
}

void obb_get_edges(OBB obb, Line3D* out_edges) {
    vec3 v[8];
    obb_get_vertices(obb, v);

    int index[12][2] = {
        {6, 1}, {6, 3}, {6, 4}, {2, 7}, {2, 5}, {2, 0},
        {0, 1}, {0, 3}, {7, 1}, {7, 4}, {4, 5}, {5, 3}
    };

    for (int j = 0; j < 12; ++j) {
        out_edges[j] = line3d_create(v[index[j][0]], v[index[j][1]]);
    }
}

void obb_get_planes(OBB obb, Plane* out_planes) {
    vec3 c = obb.position;
    vec3 e = obb.size;
    vec3 a[3];
    a[0] = vec3_make(obb.orientation.m[0][0], obb.orientation.m[0][1], obb.orientation.m[0][2]);
    a[1] = vec3_make(obb.orientation.m[1][0], obb.orientation.m[1][1], obb.orientation.m[1][2]);
    a[2] = vec3_make(obb.orientation.m[2][0], obb.orientation.m[2][1], obb.orientation.m[2][2]);

    out_planes[0] = plane_create(a[0], vec3_dot(a[0], vec3_add(c, vec3_scale(a[0], e.x))));
    out_planes[1] = plane_create(vec3_scale(a[0], -1.0f), -vec3_dot(a[0], vec3_sub(c, vec3_scale(a[0], e.x))));
    out_planes[2] = plane_create(a[1], vec3_dot(a[1], vec3_add(c, vec3_scale(a[1], e.y))));
    out_planes[3] = plane_create(vec3_scale(a[1], -1.0f), -vec3_dot(a[1], vec3_sub(c, vec3_scale(a[1], e.y))));
    out_planes[4] = plane_create(a[2], vec3_dot(a[2], vec3_add(c, vec3_scale(a[2], e.z))));
    out_planes[5] = plane_create(vec3_scale(a[2], -1.0f), -vec3_dot(a[2], vec3_sub(c, vec3_scale(a[2], e.z))));
}

/*******************************************************************************
 * Clipping and Penetration
 ******************************************************************************/

bool clip_to_plane(Plane plane, Line3D line, Point3D* out_point) {
    vec3 ab = vec3_sub(line.end, line.start);

    float n_a = vec3_dot(plane.normal, line.start);
    float n_ab = vec3_dot(plane.normal, ab);

    if (CMP(n_ab, 0.0f)) {
        return false;
    }

    float t = (plane.distance - n_a) / n_ab;
    if (t >= 0.0f && t <= 1.0f) {
        if (out_point != NULL) {
            *out_point = vec3_add(line.start, vec3_scale(ab, t));
        }
        return true;
    }
    return false;
}

int clip_edges_to_obb(const Line3D* edges, int num_edges, OBB obb,
                      Point3D* out_points, int max_points) {
    int count = 0;
    Point3D intersection;

    Plane planes[6];
    obb_get_planes(obb, planes);

    for (int i = 0; i < 6 && count < max_points; ++i) {
        for (int j = 0; j < num_edges && count < max_points; ++j) {
            if (clip_to_plane(planes[i], edges[j], &intersection)) {
                if (point_in_obb(intersection, obb)) {
                    out_points[count++] = intersection;
                }
            }
        }
    }
    return count;
}

float penetration_depth(OBB o1, OBB o2, vec3 axis, bool* out_should_flip) {
    Interval3D i1 = interval3d_from_obb(o1, vec3_normalized(axis));
    Interval3D i2 = interval3d_from_obb(o2, vec3_normalized(axis));

    if (!((i2.min <= i1.max) && (i1.min <= i2.max))) {
        return 0.0f;
    }

    float len1 = i1.max - i1.min;
    float len2 = i2.max - i2.min;
    float min_val = fminf(i1.min, i2.min);
    float max_val = fmaxf(i1.max, i2.max);
    float length = max_val - min_val;

    if (out_should_flip != NULL) {
        *out_should_flip = (i2.min < i1.min);
    }

    return (len1 + len2) - length;
}

/*******************************************************************************
 * Collision Manifold Functions
 ******************************************************************************/

CollisionManifold find_collision_features_sphere_sphere(Sphere A, Sphere B) {
    CollisionManifold result;
    collision_manifold_init(&result);

    float r = A.radius + B.radius;
    vec3 d = vec3_sub(B.position, A.position);

    if (vec3_magnitude_sq(d) - r * r > 0 || vec3_magnitude_sq(d) == 0.0f) {
        return result;
    }
    d = vec3_normalized(d);

    result.colliding = true;
    result.normal = d;
    result.depth = fabsf(vec3_magnitude(d) - r) * 0.5f;

    float dtp = A.radius - result.depth;
    Point3D contact = vec3_add(A.position, vec3_scale(d, dtp));

    contact_array_push(&result.contacts, contact);

    return result;
}

CollisionManifold find_collision_features_obb_sphere(OBB A, Sphere B) {
    CollisionManifold result;
    collision_manifold_init(&result);

    Point3D closest_point = closest_point_on_obb(A, B.position);

    float distance_sq = vec3_magnitude_sq(vec3_sub(closest_point, B.position));
    if (distance_sq > B.radius * B.radius) {
        return result;
    }

    vec3 normal;
    if (CMP(distance_sq, 0.0f)) {
        if (CMP(vec3_magnitude_sq(vec3_sub(closest_point, A.position)), 0.0f)) {
            return result;
        }
        normal = vec3_normalized(vec3_sub(closest_point, A.position));
    }
    else {
        normal = vec3_normalized(vec3_sub(B.position, closest_point));
    }

    Point3D outside_point = vec3_sub(B.position, vec3_scale(normal, B.radius));
    float distance = vec3_magnitude(vec3_sub(closest_point, outside_point));

    result.colliding = true;
    contact_array_push(&result.contacts, 
        vec3_add(closest_point, vec3_scale(vec3_sub(outside_point, closest_point), 0.5f)));
    result.normal = normal;
    result.depth = distance * 0.5f;

    return result;
}

CollisionManifold find_collision_features_obb_obb(OBB A, OBB B) {
    CollisionManifold result;
    collision_manifold_init(&result);

    /* Early out with bounding sphere test */
    Sphere s1 = sphere_create(A.position, vec3_magnitude(A.size));
    Sphere s2 = sphere_create(B.position, vec3_magnitude(B.size));

    if (!sphere_sphere(s1, s2)) {
        return result;
    }

    vec3 test[15];
    test[0] = vec3_make(A.orientation.m[0][0], A.orientation.m[0][1], A.orientation.m[0][2]);
    test[1] = vec3_make(A.orientation.m[1][0], A.orientation.m[1][1], A.orientation.m[1][2]);
    test[2] = vec3_make(A.orientation.m[2][0], A.orientation.m[2][1], A.orientation.m[2][2]);
    test[3] = vec3_make(B.orientation.m[0][0], B.orientation.m[0][1], B.orientation.m[0][2]);
    test[4] = vec3_make(B.orientation.m[1][0], B.orientation.m[1][1], B.orientation.m[1][2]);
    test[5] = vec3_make(B.orientation.m[2][0], B.orientation.m[2][1], B.orientation.m[2][2]);

    for (int i = 0; i < 3; ++i) {
        test[6 + i * 3 + 0] = vec3_cross(test[i], test[3]);
        test[6 + i * 3 + 1] = vec3_cross(test[i], test[4]);
        test[6 + i * 3 + 2] = vec3_cross(test[i], test[5]);
    }

    vec3* hit_normal = NULL;
    bool should_flip;

    for (int i = 0; i < 15; ++i) {
        if (test[i].x < 0.000001f) test[i].x = 0.0f;
        if (test[i].y < 0.000001f) test[i].y = 0.0f;
        if (test[i].z < 0.000001f) test[i].z = 0.0f;
        if (vec3_magnitude_sq(test[i]) < 0.001f) {
            continue;
        }

        float depth = penetration_depth(A, B, test[i], &should_flip);
        if (depth <= 0.0f) {
            return result;
        }
        else if (depth < result.depth) {
            if (should_flip) {
                test[i] = vec3_scale(test[i], -1.0f);
            }
            result.depth = depth;
            hit_normal = &test[i];
        }
    }

    if (hit_normal == NULL) {
        return result;
    }
    vec3 axis = vec3_normalized(*hit_normal);

    /* Clip edges */
    Line3D edges_b[12], edges_a[12];
    obb_get_edges(B, edges_b);
    obb_get_edges(A, edges_a);

    Point3D clip_buffer[72];  /* 12 edges * 6 planes max */
    int c1_count = clip_edges_to_obb(edges_b, 12, A, clip_buffer, 36);
    int c2_count = clip_edges_to_obb(edges_a, 12, B, clip_buffer + c1_count, 36);

    contact_array_reserve(&result.contacts, c1_count + c2_count);
    for (int i = 0; i < c1_count + c2_count; ++i) {
        contact_array_push(&result.contacts, clip_buffer[i]);
    }

    Interval3D interval = interval3d_from_obb(A, axis);
    float distance = (interval.max - interval.min) * 0.5f - result.depth * 0.5f;
    vec3 point_on_plane = vec3_add(A.position, vec3_scale(axis, distance));

    /* Project contacts onto collision plane and remove duplicates */
    for (int i = result.contacts.count - 1; i >= 0; --i) {
        vec3 contact = result.contacts.data[i];
        result.contacts.data[i] = vec3_add(contact, 
            vec3_scale(axis, vec3_dot(axis, vec3_sub(point_on_plane, contact))));

        for (int j = result.contacts.count - 1; j > i; --j) {
            if (vec3_magnitude_sq(vec3_sub(result.contacts.data[j], result.contacts.data[i])) < 0.0001f) {
                contact_array_erase(&result.contacts, j);
                break;
            }
        }
    }

    result.colliding = true;
    result.normal = axis;

    return result;
}
