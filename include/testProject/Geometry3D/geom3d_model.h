/**
 * @file geom3d_model.h
 * @brief Model operations (hierarchical mesh transformations)
 */
#ifndef GEOM3D_MODEL_H
#define GEOM3D_MODEL_H

#include "geom3d_types.h"

/*******************************************************************************
 * Model Operations
 ******************************************************************************/

void  model_set_content(Model* model, Mesh* mesh);
Mesh* model_get_mesh(const Model* model);
AABB  model_get_bounds(const Model* model);
mat4  model_get_world_matrix(const Model* model);
OBB   model_get_obb(const Model* model);

float model_ray(const Model* model, Ray3D ray);
bool  linetest_model(const Model* model, Line3D line);
bool  model_sphere(const Model* model, Sphere sphere);
bool  model_aabb(const Model* model, AABB aabb);
bool  model_obb(const Model* model, OBB obb);
bool  model_plane(const Model* model, Plane plane);
bool  model_triangle(const Model* model, Triangle triangle);

#ifndef NO_EXTRAS
float raycast_model(const Model* model, Ray3D ray);
#endif

#endif /* GEOM3D_MODEL_H */
