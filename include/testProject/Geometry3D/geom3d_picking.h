/**
 * @file geom3d_picking.h
 * @brief Unprojection and picking functions
 */
#ifndef GEOM3D_PICKING_H
#define GEOM3D_PICKING_H

#include "geom3d_types.h"

/*******************************************************************************
 * Unprojection / Picking
 ******************************************************************************/

vec3  unproject(vec3 viewport_point, vec2 viewport_origin, vec2 viewport_size,
                mat4 view, mat4 projection);
Ray3D get_pick_ray(vec2 viewport_point, vec2 viewport_origin, vec2 viewport_size,
                   mat4 view, mat4 projection);

#endif /* GEOM3D_PICKING_H */
