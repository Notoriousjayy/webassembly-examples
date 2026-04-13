/**
 * @file geom3d_bvh.h
 * @brief BVH (Bounding Volume Hierarchy) and Mesh operations
 */
#ifndef GEOM3D_BVH_H
#define GEOM3D_BVH_H

#include "geom3d_types.h"

/*******************************************************************************
 * BVH / Mesh Operations
 ******************************************************************************/

void mesh_accelerate(Mesh* mesh);
void bvhnode_split(BVHNode* node, const Mesh* mesh, int depth);
void bvhnode_free(BVHNode* node);

bool  linetest_mesh(const Mesh* mesh, Line3D line);
bool  mesh_sphere(const Mesh* mesh, Sphere sphere);
bool  mesh_aabb(const Mesh* mesh, AABB aabb);
bool  mesh_obb(const Mesh* mesh, OBB obb);
bool  mesh_plane(const Mesh* mesh, Plane plane);
bool  mesh_triangle(const Mesh* mesh, Triangle triangle);
float mesh_ray(const Mesh* mesh, Ray3D ray);

#ifndef NO_EXTRAS
float raycast_mesh(const Mesh* mesh, Ray3D ray);
#endif

#endif /* GEOM3D_BVH_H */
