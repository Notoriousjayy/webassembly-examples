#include "camera.h"
#include "compare.h"   /* CMP comes from here */

#include <math.h>
#include <float.h>
#include <stdio.h>

/*******************************************************************************
 * Constants and Macros
 ******************************************************************************/

#ifndef DEG2RAD
    #define DEG2RAD(deg) ((deg) * 0.0174532925199433f)  /* deg * (PI / 180) */
#endif

#ifndef RAD2DEG
    #define RAD2DEG(rad) ((rad) * 57.2957795130823f)    /* rad * (180 / PI) */
#endif

/* Remove the local #define CMP(...) */

#define CLAMP(val, min_val, max_val) \
    (((val) < (min_val)) ? (min_val) : (((val) > (max_val)) ? (max_val) : (val)))


/*******************************************************************************
 * Camera Initialization
 ******************************************************************************/

Camera camera_create(void) {
    Camera cam = {
        .fov = 60.0f,
        .aspect = 1.3f,
        .near_plane = 0.01f,
        .far_plane = 1000.0f,
        .width = 1.0f,
        .height = 1.0f,
        .world_matrix = mat4_identity(),
        .proj_matrix = mat4_identity(),
        .projection_mode = CAMERA_PROJECTION_PERSPECTIVE
    };

    /* Initialize with default perspective projection */
    cam.proj_matrix = mat4_perspective(cam.fov, cam.aspect, cam.near_plane, cam.far_plane);

    return cam;
}

Camera camera_create_perspective(float field_of_view, float aspect_ratio,
                                 float near_plane, float far_plane) {
    Camera cam = {
        .fov = field_of_view,
        .aspect = aspect_ratio,
        .near_plane = near_plane,
        .far_plane = far_plane,
        .width = 1.0f,
        .height = 1.0f,
        .world_matrix = mat4_identity(),
        .proj_matrix = mat4_perspective(field_of_view, aspect_ratio, near_plane, far_plane),
        .projection_mode = CAMERA_PROJECTION_PERSPECTIVE
    };

    return cam;
}

Camera camera_create_orthographic(float width, float height,
                                  float near_plane, float far_plane) {
    Camera cam = {
        .fov = 60.0f,
        .aspect = width / height,
        .near_plane = near_plane,
        .far_plane = far_plane,
        .width = width,
        .height = height,
        .world_matrix = mat4_identity(),
        .proj_matrix = mat4_ortho(-width / 2.0f, width / 2.0f, 
                                  -height / 2.0f, height / 2.0f,
                                  near_plane, far_plane),
        .projection_mode = CAMERA_PROJECTION_ORTHOGRAPHIC
    };

    return cam;
}

/*******************************************************************************
 * Camera Matrix Access
 ******************************************************************************/

mat4 camera_get_world_matrix(const Camera* self) {
    return self->world_matrix;
}

mat4 camera_get_view_matrix(Camera* self) {
    /* Ensure the world matrix is orthonormal before computing view */
    if (!camera_is_orthonormal(self)) {
        camera_orthonormalize(self);
    }

    /* View matrix is the inverse of the world matrix */
    return mat4_inverse(self->world_matrix);
}

mat4 camera_get_projection_matrix(const Camera* self) {
    return self->proj_matrix;
}

/*******************************************************************************
 * Camera Properties
 ******************************************************************************/

float camera_get_aspect(const Camera* self) {
    return self->aspect;
}

bool camera_is_orthographic(const Camera* self) {
    return self->projection_mode == CAMERA_PROJECTION_ORTHOGRAPHIC;
}

bool camera_is_perspective(const Camera* self) {
    return self->projection_mode == CAMERA_PROJECTION_PERSPECTIVE;
}

/*******************************************************************************
 * Camera Orthonormalization
 ******************************************************************************/

bool camera_is_orthonormal(const Camera* self) {
    /* Extract the 3x3 rotation portion of the world matrix */
    vec3 right   = {{{ self->world_matrix.m[0][0], self->world_matrix.m[0][1], self->world_matrix.m[0][2] }}};
    vec3 up      = {{{ self->world_matrix.m[1][0], self->world_matrix.m[1][1], self->world_matrix.m[1][2] }}};
    vec3 forward = {{{ self->world_matrix.m[2][0], self->world_matrix.m[2][1], self->world_matrix.m[2][2] }}};

    /* Check if each axis is unit length */
    if (!CMP(vec3_magnitude_sq(right), 1.0f)) return false;
    if (!CMP(vec3_magnitude_sq(up), 1.0f)) return false;
    if (!CMP(vec3_magnitude_sq(forward), 1.0f)) return false;

    /* Check orthogonality (dot products should be 0) */
    if (!CMP(vec3_dot(right, up), 0.0f)) return false;
    if (!CMP(vec3_dot(right, forward), 0.0f)) return false;
    if (!CMP(vec3_dot(up, forward), 0.0f)) return false;

    return true;
}

void camera_orthonormalize(Camera* self) {
    /* Extract axes from world matrix */
    vec3 right   = {{{ self->world_matrix.m[0][0], self->world_matrix.m[0][1], self->world_matrix.m[0][2] }}};
    vec3 up      = {{{ self->world_matrix.m[1][0], self->world_matrix.m[1][1], self->world_matrix.m[1][2] }}};
    vec3 forward = {{{ self->world_matrix.m[2][0], self->world_matrix.m[2][1], self->world_matrix.m[2][2] }}};

    /* Gram-Schmidt orthonormalization */
    forward = vec3_normalized(forward);
    right = vec3_normalized(vec3_cross(up, forward));
    up = vec3_cross(forward, right);

    /* Write back to world matrix */
    self->world_matrix.m[0][0] = right.x;
    self->world_matrix.m[0][1] = right.y;
    self->world_matrix.m[0][2] = right.z;

    self->world_matrix.m[1][0] = up.x;
    self->world_matrix.m[1][1] = up.y;
    self->world_matrix.m[1][2] = up.z;

    self->world_matrix.m[2][0] = forward.x;
    self->world_matrix.m[2][1] = forward.y;
    self->world_matrix.m[2][2] = forward.z;
}

/*******************************************************************************
 * Camera Configuration
 ******************************************************************************/

void camera_resize(Camera* self, int width, int height) {
    self->aspect = (float)width / (float)height;

    if (self->projection_mode == CAMERA_PROJECTION_PERSPECTIVE) {
        self->proj_matrix = mat4_perspective(self->fov, self->aspect, 
                                              self->near_plane, self->far_plane);
    }
    else if (self->projection_mode == CAMERA_PROJECTION_ORTHOGRAPHIC) {
        self->width = (float)width;
        self->height = (float)height;
        self->proj_matrix = mat4_ortho(-self->width / 2.0f, self->width / 2.0f,
                                        -self->height / 2.0f, self->height / 2.0f,
                                        self->near_plane, self->far_plane);
    }
    /* User-defined projection is not modified */
}

void camera_set_perspective(Camera* self, float fov, float aspect,
                            float z_near, float z_far) {
    self->fov = fov;
    self->aspect = aspect;
    self->near_plane = z_near;
    self->far_plane = z_far;
    self->projection_mode = CAMERA_PROJECTION_PERSPECTIVE;
    self->proj_matrix = mat4_perspective(fov, aspect, z_near, z_far);
}

void camera_set_orthographic(Camera* self, float width, float height,
                             float z_near, float z_far) {
    self->width = width;
    self->height = height;
    self->aspect = width / height;
    self->near_plane = z_near;
    self->far_plane = z_far;
    self->projection_mode = CAMERA_PROJECTION_ORTHOGRAPHIC;
    self->proj_matrix = mat4_ortho(-width / 2.0f, width / 2.0f,
                                    -height / 2.0f, height / 2.0f,
                                    z_near, z_far);
}

void camera_set_projection(Camera* self, mat4 projection) {
    self->proj_matrix = projection;
    self->projection_mode = CAMERA_PROJECTION_USER;
}

void camera_set_world(Camera* self, mat4 world) {
    self->world_matrix = world;
}

/*******************************************************************************
 * Camera Frustum
 ******************************************************************************/

Frustum camera_get_frustum(Camera* self) {
    Frustum result;

    /* Compute view-projection matrix */
    mat4 view = camera_get_view_matrix(self);
    mat4 vp = mat4_mul(view, self->proj_matrix);

    /* Extract frustum planes from view-projection matrix */
    /* Left plane: row 4 + row 1 */
    result.left.normal.x = vp.m[0][3] + vp.m[0][0];
    result.left.normal.y = vp.m[1][3] + vp.m[1][0];
    result.left.normal.z = vp.m[2][3] + vp.m[2][0];
    result.left.distance = vp.m[3][3] + vp.m[3][0];

    /* Right plane: row 4 - row 1 */
    result.right.normal.x = vp.m[0][3] - vp.m[0][0];
    result.right.normal.y = vp.m[1][3] - vp.m[1][0];
    result.right.normal.z = vp.m[2][3] - vp.m[2][0];
    result.right.distance = vp.m[3][3] - vp.m[3][0];

    /* Bottom plane: row 4 + row 2 */
    result.bottom.normal.x = vp.m[0][3] + vp.m[0][1];
    result.bottom.normal.y = vp.m[1][3] + vp.m[1][1];
    result.bottom.normal.z = vp.m[2][3] + vp.m[2][1];
    result.bottom.distance = vp.m[3][3] + vp.m[3][1];

    /* Top plane: row 4 - row 2 */
    result.top.normal.x = vp.m[0][3] - vp.m[0][1];
    result.top.normal.y = vp.m[1][3] - vp.m[1][1];
    result.top.normal.z = vp.m[2][3] - vp.m[2][1];
    result.top.distance = vp.m[3][3] - vp.m[3][1];

    /* Near plane: row 4 + row 3 */
    result.near_plane.normal.x = vp.m[0][3] + vp.m[0][2];
    result.near_plane.normal.y = vp.m[1][3] + vp.m[1][2];
    result.near_plane.normal.z = vp.m[2][3] + vp.m[2][2];
    result.near_plane.distance = vp.m[3][3] + vp.m[3][2];

    /* Far plane: row 4 - row 3 */
    result.far_plane.normal.x = vp.m[0][3] - vp.m[0][2];
    result.far_plane.normal.y = vp.m[1][3] - vp.m[1][2];
    result.far_plane.normal.z = vp.m[2][3] - vp.m[2][2];
    result.far_plane.distance = vp.m[3][3] - vp.m[3][2];

    /* Normalize all planes */
    for (int i = 0; i < 6; ++i) {
        float len = vec3_magnitude(result.planes[i].normal);
        if (len > 0.0f) {
            result.planes[i].normal = vec3_scale(result.planes[i].normal, 1.0f / len);
            result.planes[i].distance /= len;
        }
    }

    return result;
}

/*******************************************************************************
 * Camera Position/Orientation Helpers
 ******************************************************************************/

vec3 camera_get_position(const Camera* self) {
    return (vec3){{{
        self->world_matrix.m[3][0],
        self->world_matrix.m[3][1],
        self->world_matrix.m[3][2]
    }}};
}

vec3 camera_get_forward(const Camera* self) {
    /* Forward is the negative Z axis in OpenGL convention */
    return (vec3){{{
        -self->world_matrix.m[2][0],
        -self->world_matrix.m[2][1],
        -self->world_matrix.m[2][2]
    }}};
}

vec3 camera_get_right(const Camera* self) {
    return (vec3){{{
        self->world_matrix.m[0][0],
        self->world_matrix.m[0][1],
        self->world_matrix.m[0][2]
    }}};
}

vec3 camera_get_up(const Camera* self) {
    return (vec3){{{
        self->world_matrix.m[1][0],
        self->world_matrix.m[1][1],
        self->world_matrix.m[1][2]
    }}};
}

void camera_set_position(Camera* self, vec3 position) {
    self->world_matrix.m[3][0] = position.x;
    self->world_matrix.m[3][1] = position.y;
    self->world_matrix.m[3][2] = position.z;
}

void camera_look_at(Camera* self, vec3 target, vec3 up) {
    vec3 position = camera_get_position(self);

    vec3 forward = vec3_normalized(vec3_sub(target, position));
    vec3 right = vec3_normalized(vec3_cross(up, forward));
    vec3 new_up = vec3_cross(forward, right);

    /* Note: Store negative forward for OpenGL convention */
    self->world_matrix.m[0][0] = right.x;
    self->world_matrix.m[0][1] = right.y;
    self->world_matrix.m[0][2] = right.z;
    self->world_matrix.m[0][3] = 0.0f;

    self->world_matrix.m[1][0] = new_up.x;
    self->world_matrix.m[1][1] = new_up.y;
    self->world_matrix.m[1][2] = new_up.z;
    self->world_matrix.m[1][3] = 0.0f;

    self->world_matrix.m[2][0] = -forward.x;
    self->world_matrix.m[2][1] = -forward.y;
    self->world_matrix.m[2][2] = -forward.z;
    self->world_matrix.m[2][3] = 0.0f;

    /* Position remains in column 3 */
    self->world_matrix.m[3][0] = position.x;
    self->world_matrix.m[3][1] = position.y;
    self->world_matrix.m[3][2] = position.z;
    self->world_matrix.m[3][3] = 1.0f;
}

/*******************************************************************************
 * OrbitCamera Initialization
 ******************************************************************************/

OrbitCamera orbit_camera_create(void) {
    OrbitCamera orbit = {
        .base = camera_create(),
        .target = {{{0.0f, 0.0f, 0.0f}}},
        .pan_speed = {{{180.0f, 180.0f}}},
        .zoom_distance = 10.0f,
        .zoom_distance_limit = {{{3.0f, 80.0f}}},
        .zoom_speed = 300.0f,
        .rotation_speed = {{{250.0f, 120.0f}}},
        .y_rotation_limit = {{{-20.0f, 80.0f}}},
        .current_rotation = {{{0.0f, 0.0f}}}
    };

    /* Initial update to position the camera */
    orbit_camera_update(&orbit, 0.0f);

    return orbit;
}

OrbitCamera orbit_camera_create_with_target(vec3 target, float distance) {
    OrbitCamera orbit = orbit_camera_create();
    orbit.target = target;
    orbit.zoom_distance = CLAMP(distance, orbit.zoom_distance_limit.x, orbit.zoom_distance_limit.y);
    orbit_camera_update(&orbit, 0.0f);
    return orbit;
}

/*******************************************************************************
 * OrbitCamera Controls
 ******************************************************************************/

void orbit_camera_rotate(OrbitCamera* self, vec2 delta_rot, float delta_time) {
    self->current_rotation.x += delta_rot.x * self->rotation_speed.x * delta_time;
    self->current_rotation.y += delta_rot.y * self->rotation_speed.y * delta_time;

    /* Clamp pitch (Y rotation) */
    self->current_rotation.y = orbit_camera_clamp_angle(
        self->current_rotation.y,
        self->y_rotation_limit.x,
        self->y_rotation_limit.y
    );

    /* Wrap yaw (X rotation) to 0-360 range */
    while (self->current_rotation.x < 0.0f) {
        self->current_rotation.x += 360.0f;
    }
    while (self->current_rotation.x >= 360.0f) {
        self->current_rotation.x -= 360.0f;
    }
}

void orbit_camera_zoom(OrbitCamera* self, float delta_zoom, float delta_time) {
    self->zoom_distance += delta_zoom * self->zoom_speed * delta_time;

    /* Clamp zoom distance */
    self->zoom_distance = CLAMP(
        self->zoom_distance,
        self->zoom_distance_limit.x,
        self->zoom_distance_limit.y
    );
}

void orbit_camera_pan(OrbitCamera* self, vec2 delta_pan, float delta_time) {
    /* Get camera's right and up vectors for panning */
    vec3 right = camera_get_right(&self->base);
    vec3 up = camera_get_up(&self->base);

    /* Pan in camera's local space */
    vec3 pan_offset = vec3_add(
        vec3_scale(right, -delta_pan.x * self->pan_speed.x * delta_time),
        vec3_scale(up, delta_pan.y * self->pan_speed.y * delta_time)
    );

    self->target = vec3_add(self->target, pan_offset);
}

/*******************************************************************************
 * OrbitCamera Update
 ******************************************************************************/

void orbit_camera_update(OrbitCamera* self, float delta_time) {
    (void)delta_time;  /* Currently unused, but available for interpolation */

    /* Convert rotation angles to radians */
    float yaw_rad = DEG2RAD(self->current_rotation.x);
    float pitch_rad = DEG2RAD(self->current_rotation.y);

    /* Calculate camera position on sphere around target */
    /* Using spherical coordinates: */
    /* x = r * cos(pitch) * sin(yaw) */
    /* y = r * sin(pitch) */
    /* z = r * cos(pitch) * cos(yaw) */
    vec3 offset;
    offset.x = self->zoom_distance * cosf(pitch_rad) * sinf(yaw_rad);
    offset.y = self->zoom_distance * sinf(pitch_rad);
    offset.z = self->zoom_distance * cosf(pitch_rad) * cosf(yaw_rad);

    /* Set camera position */
    vec3 position = vec3_add(self->target, offset);
    camera_set_position(&self->base, position);

    /* Look at target */
    camera_look_at(&self->base, self->target, (vec3){{{0.0f, 1.0f, 0.0f}}});
}

/*******************************************************************************
 * OrbitCamera Setters
 ******************************************************************************/

void orbit_camera_set_target(OrbitCamera* self, vec3 new_target) {
    self->target = new_target;
}

void orbit_camera_set_zoom(OrbitCamera* self, float zoom) {
    self->zoom_distance = CLAMP(
        zoom,
        self->zoom_distance_limit.x,
        self->zoom_distance_limit.y
    );
}

void orbit_camera_set_rotation(OrbitCamera* self, vec2 rotation) {
    self->current_rotation.x = rotation.x;
    self->current_rotation.y = orbit_camera_clamp_angle(
        rotation.y,
        self->y_rotation_limit.x,
        self->y_rotation_limit.y
    );
}

/*******************************************************************************
 * OrbitCamera Getters
 ******************************************************************************/

vec3 orbit_camera_get_target(const OrbitCamera* self) {
    return self->target;
}

float orbit_camera_get_zoom(const OrbitCamera* self) {
    return self->zoom_distance;
}

vec2 orbit_camera_get_rotation(const OrbitCamera* self) {
    return self->current_rotation;
}

/*******************************************************************************
 * OrbitCamera Utility
 ******************************************************************************/

float orbit_camera_clamp_angle(float angle, float min, float max) {
    /* Handle wrapping for angles */
    while (angle < -180.0f) {
        angle += 360.0f;
    }
    while (angle > 180.0f) {
        angle -= 360.0f;
    }

    return CLAMP(angle, min, max);
}

#ifndef NO_EXTRAS
void orbit_camera_print_debug(const OrbitCamera* self, FILE* stream) {
    vec3 pos = camera_get_position(&self->base);

    fprintf(stream, "OrbitCamera Debug Info:\n");
    fprintf(stream, "  Target:     (%.2f, %.2f, %.2f)\n", 
            self->target.x, self->target.y, self->target.z);
    fprintf(stream, "  Position:   (%.2f, %.2f, %.2f)\n", 
            pos.x, pos.y, pos.z);
    fprintf(stream, "  Rotation:   yaw=%.2f, pitch=%.2f\n",
            self->current_rotation.x, self->current_rotation.y);
    fprintf(stream, "  Zoom:       %.2f (min=%.2f, max=%.2f)\n",
            self->zoom_distance, 
            self->zoom_distance_limit.x, 
            self->zoom_distance_limit.y);
    fprintf(stream, "  Projection: %s\n",
            camera_is_perspective(&self->base) ? "Perspective" :
            camera_is_orthographic(&self->base) ? "Orthographic" : "User-defined");
    fprintf(stream, "  Aspect:     %.3f\n", self->base.aspect);
    fprintf(stream, "  FOV:        %.1f degrees\n", self->base.fov);
    fprintf(stream, "  Near/Far:   %.3f / %.1f\n", 
            self->base.near_plane, self->base.far_plane);
}
#endif
