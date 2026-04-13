/**
 * type_bridge.h - Unified Type Conversion Layer (C23)
 *
 * Bidirectional conversions between the game-math types (vec3, mat3, mat4)
 * and the Cyclone physics types (cyclone_Vector3, cyclone_Matrix3,
 * cyclone_Matrix4).
 *
 * Key structural differences bridged here:
 *
 *   vec3           float[3], no padding
 *   cyclone_Vector3  real[3] + pad, 4-word aligned
 *
 *   mat3           float[9], row-major, named fields (_11.._33)
 *   cyclone_Matrix3  real[9], row-major flat array
 *
 *   mat4           float[16], full 4x4 row-major
 *   cyclone_Matrix4  real[12], 3x4 row-major (implicit last row [0 0 0 1])
 *
 * All functions are static inline so they compile to zero overhead at -O1+.
 */

#ifndef TYPE_BRIDGE_H
#define TYPE_BRIDGE_H

#include "vectors.h"
#include "matrices.h"
#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Vector3 conversions
 ******************************************************************************/

/**
 * Convert cyclone_Vector3 (real, 4-word) → game-math vec3 (float, 3-word).
 */
static inline vec3
vec3_from_cyclone(cyclone_Vector3 cv)
{
    return vec3_make((float)cv.x, (float)cv.y, (float)cv.z);
}

/**
 * Convert game-math vec3 (float) → cyclone_Vector3 (real, 4-word aligned).
 */
static inline cyclone_Vector3
vec3_to_cyclone(vec3 v)
{
    return cyclone_vector3_make((real)v.x, (real)v.y, (real)v.z);
}

/*******************************************************************************
 * Matrix3 conversions
 *
 * Both are row-major, so the element order is identical.  The only difference
 * is the scalar type (float vs real) and the storage style (named union fields
 * vs flat array).
 ******************************************************************************/

/**
 * Convert cyclone_Matrix3 (real[9]) → game-math mat3 (float, named fields).
 */
static inline mat3
mat3_from_cyclone(const cyclone_Matrix3 *cm)
{
    return mat3_make(
        (float)cm->data[0], (float)cm->data[1], (float)cm->data[2],
        (float)cm->data[3], (float)cm->data[4], (float)cm->data[5],
        (float)cm->data[6], (float)cm->data[7], (float)cm->data[8]
    );
}

/**
 * Convert game-math mat3 (float) → cyclone_Matrix3 (real[9]).
 */
static inline cyclone_Matrix3
mat3_to_cyclone(mat3 m)
{
    return cyclone_matrix3_make(
        (real)m._11, (real)m._12, (real)m._13,
        (real)m._21, (real)m._22, (real)m._23,
        (real)m._31, (real)m._32, (real)m._33
    );
}

/*******************************************************************************
 * Matrix4 conversions
 *
 * mat4 is a full 4x4 row-major matrix (16 floats):
 *
 *   _11 _12 _13 _14       [0]  [1]  [2]  [3]
 *   _21 _22 _23 _24       [4]  [5]  [6]  [7]
 *   _31 _32 _33 _34       [8]  [9]  [10] [11]
 *   _41 _42 _43 _44       [12] [13] [14] [15]
 *
 * cyclone_Matrix4 is a 3x4 row-major matrix (12 reals) with an implicit
 * bottom row of [0 0 0 1]:
 *
 *   data[0]  data[1]  data[2]  data[3]      rotation + translation row 0
 *   data[4]  data[5]  data[6]  data[7]      rotation + translation row 1
 *   data[8]  data[9]  data[10] data[11]     rotation + translation row 2
 *   (0)      (0)      (0)      (1)          implicit
 ******************************************************************************/

/**
 * Expand cyclone_Matrix4 (3x4, real) → mat4 (4x4, float).
 * The implicit bottom row [0 0 0 1] is written explicitly.
 */
static inline mat4
mat4_from_cyclone(const cyclone_Matrix4 *cm)
{
    return mat4_make(
        (float)cm->data[0],  (float)cm->data[1],  (float)cm->data[2],  (float)cm->data[3],
        (float)cm->data[4],  (float)cm->data[5],  (float)cm->data[6],  (float)cm->data[7],
        (float)cm->data[8],  (float)cm->data[9],  (float)cm->data[10], (float)cm->data[11],
        0.0f,                0.0f,                0.0f,                1.0f
    );
}

/**
 * Compress mat4 (4x4, float) → cyclone_Matrix4 (3x4, real).
 * The bottom row of mat4 is discarded (it should be [0 0 0 1] for affine
 * transforms; no validation is performed).
 */
static inline cyclone_Matrix4
mat4_to_cyclone(mat4 m)
{
    cyclone_Matrix4 cm;
    cm.data[0]  = (real)m._11;
    cm.data[1]  = (real)m._12;
    cm.data[2]  = (real)m._13;
    cm.data[3]  = (real)m._14;
    cm.data[4]  = (real)m._21;
    cm.data[5]  = (real)m._22;
    cm.data[6]  = (real)m._23;
    cm.data[7]  = (real)m._24;
    cm.data[8]  = (real)m._31;
    cm.data[9]  = (real)m._32;
    cm.data[10] = (real)m._33;
    cm.data[11] = (real)m._34;
    return cm;
}

/*******************************************************************************
 * Quaternion → mat4 convenience
 *
 * Builds a full 4x4 game-math mat4 from a Cyclone quaternion + position,
 * going through cyclone_Matrix4 internally.
 ******************************************************************************/

/**
 * Build a mat4 world matrix from a Cyclone quaternion orientation and a
 * Cyclone position vector.  Useful for positioning physics-driven objects
 * in the rendering pipeline.
 */
static inline mat4
mat4_from_cyclone_transform(const cyclone_Quaternion *orientation,
                            const cyclone_Vector3 *position)
{
    cyclone_Matrix4 cm = cyclone_matrix4_identity();
    cyclone_matrix4_set_orientation_and_pos(&cm, orientation, position);
    return mat4_from_cyclone(&cm);
}

/**
 * Extract a Cyclone position vector from a game-math mat4.
 * Reads the translation column (column 3 / _14, _24, _34).
 */
static inline cyclone_Vector3
cyclone_position_from_mat4(mat4 m)
{
    return cyclone_vector3_make((real)m._14, (real)m._24, (real)m._34);
}

/**
 * Extract a 3x3 rotation block from a game-math mat4 into a
 * cyclone_Matrix3 (for feeding inertia tensor operations, etc.).
 */
static inline cyclone_Matrix3
cyclone_rotation_from_mat4(mat4 m)
{
    return cyclone_matrix3_make(
        (real)m._11, (real)m._12, (real)m._13,
        (real)m._21, (real)m._22, (real)m._23,
        (real)m._31, (real)m._32, (real)m._33
    );
}

/*******************************************************************************
 * GL Upload helpers
 *
 * cyclone_matrix4_fill_gl_array() already exists in core.h for Cyclone types.
 * These complement it for the game-math types.
 ******************************************************************************/

/**
 * Fill a float[16] column-major array suitable for glUniformMatrix4fv
 * from a row-major game-math mat4.
 */
static inline void
mat4_fill_gl_array(const mat4 *m, float out[16])
{
    /* Transpose from row-major to column-major for OpenGL */
    out[0]  = m->_11;  out[1]  = m->_21;  out[2]  = m->_31;  out[3]  = m->_41;
    out[4]  = m->_12;  out[5]  = m->_22;  out[6]  = m->_32;  out[7]  = m->_42;
    out[8]  = m->_13;  out[9]  = m->_23;  out[10] = m->_33;  out[11] = m->_43;
    out[12] = m->_14;  out[13] = m->_24;  out[14] = m->_34;  out[15] = m->_44;
}

#ifdef __cplusplus
}
#endif

#endif /* TYPE_BRIDGE_H */
