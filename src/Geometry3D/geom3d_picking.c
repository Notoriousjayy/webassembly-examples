/**
 * @file geom3d_picking.c
 * @brief Unprojection and picking functions
 */
#include "geom3d_picking.h"
#include "compare.h"

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/

static void mat4_multiply_array(
    float *out,
    const float *in,
    int rows_a,
    int cols_a,
    const float *mat_b,
    int rows_b,
    int cols_b)
{
    (void)rows_b;
    /* Multiply a row-major (rows_a x cols_a) array by a row-major
       (rows_b x cols_b) matrix: out = in * mat_b. */
    for (int r = 0; r < rows_a; ++r) {
        for (int c = 0; c < cols_b; ++c) {
            float sum = 0.0f;
            for (int k = 0; k < cols_a; ++k) {
                sum += in[r * cols_a + k] * mat_b[k * cols_b + c];
            }
            out[r * cols_b + c] = sum;
        }
    }
}

/*******************************************************************************
 * Unprojection / Picking
 ******************************************************************************/

vec3 unproject(vec3 viewport_point, vec2 viewport_origin, vec2 viewport_size,
               mat4 view, mat4 projection) {
    /* Step 1: Normalize the input vector to the viewport */
    float normalized[4] = {
        (viewport_point.x - viewport_origin.x) / viewport_size.x,
        (viewport_point.y - viewport_origin.y) / viewport_size.y,
        viewport_point.z,
        1.0f
    };

    /* Step 2: Translate into NDC space */
    float ndc_space[4] = {
        normalized[0], normalized[1],
        normalized[2], normalized[3]
    };
    ndc_space[0] = ndc_space[0] * 2.0f - 1.0f;
    ndc_space[1] = 1.0f - ndc_space[1] * 2.0f;
    if (ndc_space[2] < 0.0f) ndc_space[2] = 0.0f;
    if (ndc_space[2] > 1.0f) ndc_space[2] = 1.0f;

    /* Step 3: NDC to Eye Space */
    mat4 inv_projection = mat4_inverse(projection);
    float eye_space[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    mat4_multiply_array(eye_space, ndc_space, 1, 4, inv_projection.m[0], 4, 4);

    /* Step 4: Eye Space to World Space */
    mat4 inv_view = mat4_inverse(view);
    float world_space[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    mat4_multiply_array(world_space, eye_space, 1, 4, inv_view.m[0], 4, 4);

    /* Step 5: Undo perspective divide */
    if (!CMP(world_space[3], 0.0f)) {
        world_space[0] /= world_space[3];
        world_space[1] /= world_space[3];
        world_space[2] /= world_space[3];
    }

    return vec3_make(world_space[0], world_space[1], world_space[2]);
}

Ray3D get_pick_ray(vec2 viewport_point, vec2 viewport_origin, vec2 viewport_size,
                   mat4 view, mat4 projection) {
    vec3 near_point = vec3_make(viewport_point.x, viewport_point.y, 0.0f);
    vec3 far_point = vec3_make(viewport_point.x, viewport_point.y, 1.0f);

    vec3 p_near = unproject(near_point, viewport_origin, viewport_size, view, projection);
    vec3 p_far = unproject(far_point, viewport_origin, viewport_size, view, projection);

    vec3 normal = vec3_normalized(vec3_sub(p_far, p_near));
    vec3 origin = p_near;

    return ray3d_create(origin, normal);
}
