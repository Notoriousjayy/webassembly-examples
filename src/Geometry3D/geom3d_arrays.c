/**
 * @file geom3d_arrays.c
 * @brief Dynamic array implementations for geometry types
 */
#include "geom3d_arrays.h"

#include <stdlib.h>
#include <string.h>
#include <float.h>

/*******************************************************************************
 * Dynamic Array Implementations
 ******************************************************************************/

/* ContactArray */
void contact_array_init(ContactArray* arr) {
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void contact_array_free(ContactArray* arr) {
    if (arr->data) {
        free(arr->data);
        arr->data = NULL;
    }
    arr->count = 0;
    arr->capacity = 0;
}

void contact_array_reserve(ContactArray* arr, int capacity) {
    if (capacity > arr->capacity) {
        arr->data = realloc(arr->data, (size_t)capacity * sizeof(vec3));
        arr->capacity = capacity;
    }
}

void contact_array_push(ContactArray* arr, vec3 point) {
    if (arr->count >= arr->capacity) {
        int new_cap = arr->capacity == 0 ? 8 : arr->capacity * 2;
        contact_array_reserve(arr, new_cap);
    }
    arr->data[arr->count++] = point;
}

void contact_array_clear(ContactArray* arr) {
    arr->count = 0;
}

void contact_array_erase(ContactArray* arr, int index) {
    if (index >= 0 && index < arr->count) {
        memmove(&arr->data[index], &arr->data[index + 1],
                (size_t)(arr->count - index - 1) * sizeof(vec3));
        arr->count--;
    }
}

/* Line3DArray */
void line3d_array_init(Line3DArray* arr) {
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void line3d_array_free(Line3DArray* arr) {
    if (arr->data) {
        free(arr->data);
        arr->data = NULL;
    }
    arr->count = 0;
    arr->capacity = 0;
}

void line3d_array_reserve(Line3DArray* arr, int capacity) {
    if (capacity > arr->capacity) {
        arr->data = realloc(arr->data, (size_t)capacity * sizeof(Line3D));
        arr->capacity = capacity;
    }
}

void line3d_array_push(Line3DArray* arr, Line3D line) {
    if (arr->count >= arr->capacity) {
        int new_cap = arr->capacity == 0 ? 8 : arr->capacity * 2;
        line3d_array_reserve(arr, new_cap);
    }
    arr->data[arr->count++] = line;
}

/* PlaneArray */
void plane_array_init(PlaneArray* arr) {
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void plane_array_free(PlaneArray* arr) {
    if (arr->data) {
        free(arr->data);
        arr->data = NULL;
    }
    arr->count = 0;
    arr->capacity = 0;
}

void plane_array_push(PlaneArray* arr, Plane plane) {
    if (arr->count >= arr->capacity) {
        int new_cap = arr->capacity == 0 ? 8 : arr->capacity * 2;
        arr->data = realloc(arr->data, (size_t)new_cap * sizeof(Plane));
        arr->capacity = new_cap;
    }
    arr->data[arr->count++] = plane;
}

/*******************************************************************************
 * RaycastResult / CollisionManifold
 ******************************************************************************/

void raycast_result_reset(RaycastResult* result) {
    if (result) {
        result->t = -1.0f;
        result->hit = false;
        result->normal = vec3_make(0, 0, 1);
        result->point = vec3_make(0, 0, 0);
    }
}

void collision_manifold_init(CollisionManifold* result) {
    if (result) {
        result->colliding = false;
        result->normal = vec3_make(0, 0, 1);
        result->depth = FLT_MAX;
        contact_array_init(&result->contacts);
    }
}

void collision_manifold_free(CollisionManifold* result) {
    if (result) {
        contact_array_free(&result->contacts);
    }
}

void collision_manifold_reset(CollisionManifold* result) {
    if (result) {
        result->colliding = false;
        result->normal = vec3_make(0, 0, 1);
        result->depth = FLT_MAX;
        contact_array_clear(&result->contacts);
    }
}
