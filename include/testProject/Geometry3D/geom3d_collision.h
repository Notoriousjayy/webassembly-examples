/**
 * @file geom3d_collision.h
 * @brief Collision manifold generation and OBB helper functions
 */
#ifndef GEOM3D_COLLISION_H
#define GEOM3D_COLLISION_H

#include "geom3d_types.h"

/*******************************************************************************
 * OBB Helper Functions for Collision
 ******************************************************************************/

/* Output to fixed-size arrays */
void obb_get_vertices(OBB obb, vec3* out_vertices);      /* out_vertices[8] */
void obb_get_edges(OBB obb, Line3D* out_edges);          /* out_edges[12] */
void obb_get_planes(OBB obb, Plane* out_planes);         /* out_planes[6] */

/*******************************************************************************
 * Clipping and Penetration
 ******************************************************************************/

bool  clip_to_plane(Plane plane, Line3D line, Point3D* out_point);
int   clip_edges_to_obb(const Line3D* edges, int num_edges, OBB obb,
                        Point3D* out_points, int max_points);
float penetration_depth(OBB o1, OBB o2, vec3 axis, bool* out_should_flip);

/*******************************************************************************
 * Collision Manifold Functions
 ******************************************************************************/

CollisionManifold find_collision_features_sphere_sphere(Sphere a, Sphere b);
CollisionManifold find_collision_features_obb_sphere(OBB a, Sphere b);
CollisionManifold find_collision_features_obb_obb(OBB a, OBB b);

#endif /* GEOM3D_COLLISION_H */
