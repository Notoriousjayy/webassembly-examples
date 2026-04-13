/**
 * @file geom3d_bvh.c
 * @brief BVH (Bounding Volume Hierarchy) and Mesh operations
 */
#include "geom3d_bvh.h"
#include "geom3d_primitives.h"
#include "geom3d_intersect.h"
#include "geom3d_raycast.h"

#include <stdlib.h>
#include <math.h>

/*******************************************************************************
 * BVH Stack (replaces std::list<BVHNode*>)
 ******************************************************************************/

typedef struct BVHStack {
    BVHNode** nodes;
    int       count;
    int       capacity;
} BVHStack;

static void bvh_stack_init(BVHStack* s, int cap) {
    s->nodes = malloc((size_t)cap * sizeof(BVHNode*));
    s->count = 0;
    s->capacity = cap;
}

static void bvh_stack_free(BVHStack* s) {
    free(s->nodes);
    s->nodes = NULL;
    s->count = 0;
    s->capacity = 0;
}

static void bvh_stack_push(BVHStack* s, BVHNode* node) {
    if (s->count >= s->capacity) {
        s->capacity *= 2;
        s->nodes = realloc(s->nodes, (size_t)s->capacity * sizeof(BVHNode*));
    }
    s->nodes[s->count++] = node;
}

static BVHNode* bvh_stack_pop(BVHStack* s) {
    return s->count > 0 ? s->nodes[--s->count] : NULL;
}

static bool bvh_stack_empty(BVHStack* s) {
    return s->count == 0;
}

/*******************************************************************************
 * BVH / Mesh Operations
 ******************************************************************************/

void mesh_accelerate(Mesh* mesh) {
    if (mesh->accelerator != NULL) {
        return;
    }

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

    mesh->accelerator = malloc(sizeof(BVHNode));
    *mesh->accelerator = bvhnode_default();
    mesh->accelerator->bounds = aabb_from_min_max(min, max);
    mesh->accelerator->children = NULL;
    mesh->accelerator->num_triangles = mesh->num_triangles;
    mesh->accelerator->triangles = malloc((size_t)mesh->num_triangles * sizeof(int));

    for (int i = 0; i < mesh->num_triangles; ++i) {
        mesh->accelerator->triangles[i] = i;
    }

    bvhnode_split(mesh->accelerator, mesh, 3);
}

void bvhnode_split(BVHNode* node, const Mesh* mesh, int depth) {
    if (depth-- <= 0) {
        return;
    }

    if (node->children == NULL) {
        if (node->num_triangles > 0) {
            node->children = malloc(8 * sizeof(BVHNode));

            vec3 c = node->bounds.position;
            vec3 e = vec3_scale(node->bounds.size, 0.5f);

            vec3 offsets[8];
            offsets[0] = vec3_make(-e.x, +e.y, -e.z);
            offsets[1] = vec3_make(+e.x, +e.y, -e.z);
            offsets[2] = vec3_make(-e.x, +e.y, +e.z);
            offsets[3] = vec3_make(+e.x, +e.y, +e.z);
            offsets[4] = vec3_make(-e.x, -e.y, -e.z);
            offsets[5] = vec3_make(+e.x, -e.y, -e.z);
            offsets[6] = vec3_make(-e.x, -e.y, +e.z);
            offsets[7] = vec3_make(+e.x, -e.y, +e.z);

            for (int i = 0; i < 8; ++i) {
                node->children[i] = bvhnode_default();
                node->children[i].bounds = aabb_create(vec3_add(c, offsets[i]), e);
            }
        }
    }

    if (node->children != NULL && node->num_triangles > 0) {
        for (int i = 0; i < 8; ++i) {
            node->children[i].num_triangles = 0;
            for (int j = 0; j < node->num_triangles; ++j) {
                Triangle t = mesh->triangles[node->triangles[j]];
                if (triangle_aabb(t, node->children[i].bounds)) {
                    node->children[i].num_triangles += 1;
                }
            }

            if (node->children[i].num_triangles == 0) {
                continue;
            }

            node->children[i].triangles = malloc((size_t)node->children[i].num_triangles * sizeof(int));
            int index = 0;
            for (int j = 0; j < node->num_triangles; ++j) {
                Triangle t = mesh->triangles[node->triangles[j]];
                if (triangle_aabb(t, node->children[i].bounds)) {
                    node->children[i].triangles[index++] = node->triangles[j];
                }
            }
        }

        node->num_triangles = 0;
        free(node->triangles);
        node->triangles = NULL;

        for (int i = 0; i < 8; ++i) {
            bvhnode_split(&node->children[i], mesh, depth);
        }
    }
}

void bvhnode_free(BVHNode* node) {
    if (node->children != NULL) {
        for (int i = 0; i < 8; ++i) {
            bvhnode_free(&node->children[i]);
        }
        free(node->children);
        node->children = NULL;
    }

    if (node->num_triangles != 0 || node->triangles != NULL) {
        free(node->triangles);
        node->triangles = NULL;
        node->num_triangles = 0;
    }
}

bool mesh_aabb(const Mesh* mesh, AABB aabb) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            if (triangle_aabb(mesh->triangles[i], aabb)) {
                return true;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    if (triangle_aabb(mesh->triangles[node->triangles[i]], aabb)) {
                        bvh_stack_free(&stack);
                        return true;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    if (aabb_aabb(node->children[i].bounds, aabb)) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return false;
}

bool linetest_mesh(const Mesh* mesh, Line3D line) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            if (linetest_triangle(mesh->triangles[i], line)) {
                return true;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    if (linetest_triangle(mesh->triangles[node->triangles[i]], line)) {
                        bvh_stack_free(&stack);
                        return true;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    if (linetest_aabb(node->children[i].bounds, line)) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return false;
}

bool mesh_sphere(const Mesh* mesh, Sphere sphere) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            if (triangle_sphere(mesh->triangles[i], sphere)) {
                return true;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    if (triangle_sphere(mesh->triangles[node->triangles[i]], sphere)) {
                        bvh_stack_free(&stack);
                        return true;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    if (sphere_aabb(sphere, node->children[i].bounds)) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return false;
}

bool mesh_obb(const Mesh* mesh, OBB obb) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            if (triangle_obb(mesh->triangles[i], obb)) {
                return true;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    if (triangle_obb(mesh->triangles[node->triangles[i]], obb)) {
                        bvh_stack_free(&stack);
                        return true;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    if (aabb_obb(node->children[i].bounds, obb)) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return false;
}

bool mesh_plane(const Mesh* mesh, Plane plane) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            if (triangle_plane(mesh->triangles[i], plane)) {
                return true;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    if (triangle_plane(mesh->triangles[node->triangles[i]], plane)) {
                        bvh_stack_free(&stack);
                        return true;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    if (aabb_plane(node->children[i].bounds, plane)) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return false;
}

bool mesh_triangle(const Mesh* mesh, Triangle triangle) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            if (triangle_triangle(mesh->triangles[i], triangle)) {
                return true;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    if (triangle_triangle(mesh->triangles[node->triangles[i]], triangle)) {
                        bvh_stack_free(&stack);
                        return true;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    if (triangle_aabb(triangle, node->children[i].bounds)) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return false;
}

float mesh_ray(const Mesh* mesh, Ray3D ray) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            RaycastResult raycast;
            raycast_triangle(mesh->triangles[i], ray, &raycast);
            float result = raycast.t;
            if (result >= 0) {
                return result;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    RaycastResult raycast;
                    raycast_triangle(mesh->triangles[node->triangles[i]], ray, &raycast);
                    float r = raycast.t;
                    if (r >= 0) {
                        bvh_stack_free(&stack);
                        return r;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    RaycastResult raycast;
                    raycast_aabb(node->children[i].bounds, ray, &raycast);
                    if (raycast.t >= 0) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return -1.0f;
}

#ifndef NO_EXTRAS
float raycast_mesh(const Mesh* mesh, Ray3D ray) {
    return mesh_ray(mesh, ray);
}
#endif
