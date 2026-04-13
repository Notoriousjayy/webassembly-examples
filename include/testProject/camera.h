/**
 * camera.h - Camera System (C23)
 * 
 * Base Camera with projection management and OrbitCamera for target-based orbiting.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "matrices.h"
#include "Geometry3D/geom3d_types.h"
#include "Geometry3D/geom3d_frustum.h"

#include <stdbool.h>

/*******************************************************************************
 * Projection Mode Constants
 ******************************************************************************/

#define CAMERA_PROJECTION_PERSPECTIVE  0
#define CAMERA_PROJECTION_ORTHOGRAPHIC 1
#define CAMERA_PROJECTION_USER         2

/*******************************************************************************
 * Camera - Base camera type with projection and view matrix management
 ******************************************************************************/

typedef struct Camera {
    float fov;          /* Field of view (degrees) for perspective */
    float aspect;       /* Aspect ratio (width / height) */
    float near_plane;   /* Near clipping plane distance */
    float far_plane;    /* Far clipping plane distance */
    float width;        /* Orthographic width */
    float height;       /* Orthographic height */

    mat4 world_matrix;  /* World transform (position/orientation) */
    mat4 proj_matrix;   /* Projection matrix */
    int  projection_mode; /* CAMERA_PROJECTION_* constant */
} Camera;

/*******************************************************************************
 * Camera Initialization
 ******************************************************************************/

Camera camera_create(void);
Camera camera_create_perspective(float field_of_view, float aspect_ratio, 
                                 float near_plane, float far_plane);
Camera camera_create_orthographic(float width, float height,
                                  float near_plane, float far_plane);

/*******************************************************************************
 * Camera Matrix Access
 ******************************************************************************/

mat4 camera_get_world_matrix(const Camera* self);
mat4 camera_get_view_matrix(Camera* self);  /* May ortho-normalize internally */
mat4 camera_get_projection_matrix(const Camera* self);

/*******************************************************************************
 * Camera Properties
 ******************************************************************************/

float camera_get_aspect(const Camera* self);
bool  camera_is_orthographic(const Camera* self);
bool  camera_is_perspective(const Camera* self);

/*******************************************************************************
 * Camera Orthonormalization
 ******************************************************************************/

bool camera_is_orthonormal(const Camera* self);
void camera_orthonormalize(Camera* self);

/*******************************************************************************
 * Camera Configuration
 ******************************************************************************/

void camera_resize(Camera* self, int width, int height);
void camera_set_perspective(Camera* self, float fov, float aspect, 
                            float z_near, float z_far);
void camera_set_orthographic(Camera* self, float width, float height,
                             float z_near, float z_far);
void camera_set_projection(Camera* self, mat4 projection);
void camera_set_world(Camera* self, mat4 world);

/*******************************************************************************
 * Camera Frustum
 ******************************************************************************/

Frustum camera_get_frustum(Camera* self);

/*******************************************************************************
 * Camera Position/Orientation Helpers
 ******************************************************************************/

vec3 camera_get_position(const Camera* self);
vec3 camera_get_forward(const Camera* self);
vec3 camera_get_right(const Camera* self);
vec3 camera_get_up(const Camera* self);

void camera_set_position(Camera* self, vec3 position);
void camera_look_at(Camera* self, vec3 target, vec3 up);

/*******************************************************************************
 * OrbitCamera - Camera that orbits around a target point
 ******************************************************************************/

typedef struct OrbitCamera {
    Camera base;  /* "Inherits" from Camera via composition */

    vec3 target;
    vec2 pan_speed;

    float zoom_distance;
    vec2  zoom_distance_limit;  /* x = min, y = max */
    float zoom_speed;

    vec2 rotation_speed;
    vec2 y_rotation_limit;      /* x = min, y = max (pitch limits) */
    vec2 current_rotation;      /* x = yaw (horizontal), y = pitch (vertical) */
} OrbitCamera;

/*******************************************************************************
 * OrbitCamera Initialization
 ******************************************************************************/

OrbitCamera orbit_camera_create(void);
OrbitCamera orbit_camera_create_with_target(vec3 target, float distance);

/*******************************************************************************
 * OrbitCamera Controls
 ******************************************************************************/

void orbit_camera_rotate(OrbitCamera* self, vec2 delta_rot, float delta_time);
void orbit_camera_zoom(OrbitCamera* self, float delta_zoom, float delta_time);
void orbit_camera_pan(OrbitCamera* self, vec2 delta_pan, float delta_time);

/*******************************************************************************
 * OrbitCamera Update (call each frame)
 ******************************************************************************/

void orbit_camera_update(OrbitCamera* self, float delta_time);

/*******************************************************************************
 * OrbitCamera Setters
 ******************************************************************************/

void orbit_camera_set_target(OrbitCamera* self, vec3 new_target);
void orbit_camera_set_zoom(OrbitCamera* self, float zoom);
void orbit_camera_set_rotation(OrbitCamera* self, vec2 rotation);

/*******************************************************************************
 * OrbitCamera Getters
 ******************************************************************************/

vec3  orbit_camera_get_target(const OrbitCamera* self);
float orbit_camera_get_zoom(const OrbitCamera* self);
vec2  orbit_camera_get_rotation(const OrbitCamera* self);

/*******************************************************************************
 * OrbitCamera Utility
 ******************************************************************************/

float orbit_camera_clamp_angle(float angle, float min, float max);

#ifndef NO_EXTRAS
void orbit_camera_print_debug(const OrbitCamera* self, FILE* stream);
#endif

/*******************************************************************************
 * Convenience: Access base Camera from OrbitCamera
 ******************************************************************************/

static inline Camera* orbit_camera_as_camera(OrbitCamera* self) {
    return &self->base;
}

static inline const Camera* orbit_camera_as_camera_const(const OrbitCamera* self) {
    return &self->base;
}

#endif /* CAMERA_H */
