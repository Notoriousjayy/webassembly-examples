/**
 * @file geom3d_arrays.h
 * @brief Dynamic array implementations for geometry types
 */
#ifndef GEOM3D_ARRAYS_H
#define GEOM3D_ARRAYS_H

#include "geom3d_types.h"

/*******************************************************************************
 * Dynamic Array Operations
 ******************************************************************************/

/* ContactArray */
void contact_array_init(ContactArray* arr);
void contact_array_free(ContactArray* arr);
void contact_array_push(ContactArray* arr, vec3 point);
void contact_array_clear(ContactArray* arr);
void contact_array_reserve(ContactArray* arr, int capacity);
void contact_array_erase(ContactArray* arr, int index);

/* Line3DArray */
void line3d_array_init(Line3DArray* arr);
void line3d_array_free(Line3DArray* arr);
void line3d_array_push(Line3DArray* arr, Line3D line);
void line3d_array_reserve(Line3DArray* arr, int capacity);

/* PlaneArray */
void plane_array_init(PlaneArray* arr);
void plane_array_free(PlaneArray* arr);
void plane_array_push(PlaneArray* arr, Plane plane);

/*******************************************************************************
 * RaycastResult / CollisionManifold
 ******************************************************************************/

void raycast_result_reset(RaycastResult* result);
void collision_manifold_reset(CollisionManifold* result);
void collision_manifold_init(CollisionManifold* result);
void collision_manifold_free(CollisionManifold* result);

#endif /* GEOM3D_ARRAYS_H */
