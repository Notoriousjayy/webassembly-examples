/**
 * @file geom3d_model.c
 * @brief Model operations (hierarchical mesh transformations)
 */
#include "geom3d_model.h"
#include "geom3d_primitives.h"
#include "geom3d_bvh.h"

#include <math.h>

/*******************************************************************************
 * Model Operations
 ******************************************************************************/

void model_set_content(Model* model, Mesh* mesh) {
    model->content = mesh;
    if (mesh != NULL) {
        vec3 min = mesh->vertices[0];
        vec3 max = mesh->vertices[0];

        for (int i = 1; i < mesh->num_triangles * 3; ++i) {
            min.x = fminf(mesh->vertices[i].x, min.x);
            min.y = fminf(mesh->vertices[i].y, min.y);
            min.z = fminf(mesh->vertices[i].z, min.z);

            max.x = fmaxf(mesh->vertices[i].x, max.x);
            max.y = fmaxf(mesh->vertices[i].y, max.y);
            max.z = fmaxf(mesh->vertices[i].z, max.z);
        }
        model->bounds = aabb_from_min_max(min, max);
    }
}

Mesh* model_get_mesh(const Model* model) {
    return model->content;
}

AABB model_get_bounds(const Model* model) {
    return model->bounds;
}

mat4 model_get_world_matrix(const Model* model) {
    mat4 translation = mat4_translation_vec3(model->position);
    mat4 rotation = Rotation(model->rotation.x, model->rotation.y, model->rotation.z);
    mat4 local_mat = mat4_mul(rotation, translation);

    mat4 parent_mat = mat4_identity();
    if (model->parent != NULL) {
        parent_mat = model_get_world_matrix(model->parent);
    }

    return mat4_mul(local_mat, parent_mat);
}

OBB model_get_obb(const Model* model) {
    mat4 world = model_get_world_matrix(model);
    AABB aabb = model->bounds;
    OBB obb;

    obb.size = aabb.size;
    obb.position = MultiplyPoint(aabb.position, world);
    obb.orientation = mat4_cut(world, 3, 3);

    return obb;
}

float model_ray(const Model* model, Ray3D ray) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    Ray3D local;
    local.origin = MultiplyPoint(ray.origin, inv);
    local.direction = mat4_multiply_vector(ray.direction, inv);
    ray3d_normalize_direction(&local);

    if (model->content != NULL) {
        return mesh_ray(model->content, local);
    }
    return -1.0f;
}

bool linetest_model(const Model* model, Line3D line) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    Line3D local;
    local.start = MultiplyPoint(line.start, inv);
    local.end = MultiplyPoint(line.end, inv);

    if (model->content != NULL) {
        return linetest_mesh(model->content, local);
    }
    return false;
}

bool model_sphere(const Model* model, Sphere sphere) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    Sphere local;
    local.position = MultiplyPoint(sphere.position, inv);
    local.radius = sphere.radius;

    if (model->content != NULL) {
        return mesh_sphere(model->content, local);
    }
    return false;
}

bool model_aabb(const Model* model, AABB aabb) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    OBB local;
    local.size = aabb.size;
    local.position = MultiplyPoint(aabb.position, inv);
    local.orientation = mat4_cut(inv, 3, 3);

    if (model->content != NULL) {
        return mesh_obb(model->content, local);
    }
    return false;
}

bool model_obb(const Model* model, OBB obb) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    OBB local;
    local.size = obb.size;
    local.position = MultiplyPoint(obb.position, inv);
    local.orientation = mat3_mul(obb.orientation, mat4_cut(inv, 3, 3));

    if (model->content != NULL) {
        return mesh_obb(model->content, local);
    }
    return false;
}

bool model_plane(const Model* model, Plane plane) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    Plane local;
    local.normal = mat4_multiply_vector(plane.normal, inv);
    local.distance = plane.distance;

    if (model->content != NULL) {
        return mesh_plane(model->content, local);
    }
    return false;
}

bool model_triangle(const Model* model, Triangle triangle) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    Triangle local;
    local.a = MultiplyPoint(triangle.a, inv);
    local.b = MultiplyPoint(triangle.b, inv);
    local.c = MultiplyPoint(triangle.c, inv);

    if (model->content != NULL) {
        return mesh_triangle(model->content, local);
    }
    return false;
}

#ifndef NO_EXTRAS
float raycast_model(const Model* model, Ray3D ray) {
    return model_ray(model, ray);
}
#endif
