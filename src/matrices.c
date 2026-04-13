#include "matrices.h"
#include "compare.h"   /* use global CMP / float compare utilities */

#include <math.h>
#include <float.h>
#include <stdio.h>

/* Forward declarations for vector helpers implemented in vectors.c */
float Dot(vec3 a, vec3 b);
vec3  Cross(vec3 a, vec3 b);
float Magnitude(vec3 v);
float MagnitudeSq(vec3 v);
vec3  Normalized(vec3 v);
float DEG2RAD(float degrees);

/* ---------- simple constructors / helpers ---------- */

mat2 mat2_identity(void) {
    mat2 m = {
        ._11 = 1.0f, ._12 = 0.0f,
        ._21 = 0.0f, ._22 = 1.0f
    };
    return m;
}

mat2 mat2_make(float f11, float f12,
               float f21, float f22) {
    mat2 m = {
        ._11 = f11, ._12 = f12,
        ._21 = f21, ._22 = f22
    };
    return m;
}

mat3 mat3_identity(void) {
    mat3 m = {
        ._11 = 1.0f, ._12 = 0.0f, ._13 = 0.0f,
        ._21 = 0.0f, ._22 = 1.0f, ._23 = 0.0f,
        ._31 = 0.0f, ._32 = 0.0f, ._33 = 1.0f
    };
    return m;
}

mat3 mat3_make(float f11, float f12, float f13,
               float f21, float f22, float f23,
               float f31, float f32, float f33) {
    mat3 m = {
        ._11 = f11, ._12 = f12, ._13 = f13,
        ._21 = f21, ._22 = f22, ._23 = f23,
        ._31 = f31, ._32 = f32, ._33 = f33
    };
    return m;
}

mat4 mat4_identity(void) {
    mat4 m = {
        ._11 = 1.0f, ._12 = 0.0f, ._13 = 0.0f, ._14 = 0.0f,
        ._21 = 0.0f, ._22 = 1.0f, ._23 = 0.0f, ._24 = 0.0f,
        ._31 = 0.0f, ._32 = 0.0f, ._33 = 1.0f, ._34 = 0.0f,
        ._41 = 0.0f, ._42 = 0.0f, ._43 = 0.0f, ._44 = 1.0f
    };
    return m;
}

mat4 mat4_make(float f11, float f12, float f13, float f14,
               float f21, float f22, float f23, float f24,
               float f31, float f32, float f33, float f34,
               float f41, float f42, float f43, float f44) {
    mat4 m = {
        ._11 = f11, ._12 = f12, ._13 = f13, ._14 = f14,
        ._21 = f21, ._22 = f22, ._23 = f23, ._24 = f24,
        ._31 = f31, ._32 = f32, ._33 = f33, ._34 = f34,
        ._41 = f41, ._42 = f42, ._43 = f43, ._44 = f44
    };
    return m;
}

float *mat2_row(mat2 *m, int row) {
    return &m->asArray[row * 2];
}

const float *mat2_row_const(const mat2 *m, int row) {
    return &m->asArray[row * 2];
}

float *mat3_row(mat3 *m, int row) {
    return &m->asArray[row * 3];
}

const float *mat3_row_const(const mat3 *m, int row) {
    return &m->asArray[row * 3];
}

float *mat4_row(mat4 *m, int row) {
    return &m->asArray[row * 4];
}

const float *mat4_row_const(const mat4 *m, int row) {
    return &m->asArray[row * 4];
}

/* ---------- equality / printing / fast inverse ---------- */

#ifndef NO_EXTRAS

bool mat2_equal(mat2 l, mat2 r) {
    for (int i = 0; i < 4; ++i) {
        if (!CMP(l.asArray[i], r.asArray[i])) {
            return false;
        }
    }
    return true;
}

bool mat3_equal(mat3 l, mat3 r) {
    for (int i = 0; i < 9; ++i) {
        if (!CMP(l.asArray[i], r.asArray[i])) {
            return false;
        }
    }
    return true;
}

bool mat4_equal(mat4 l, mat4 r) {
    for (int i = 0; i < 16; ++i) {
        if (!CMP(l.asArray[i], r.asArray[i])) {
            return false;
        }
    }
    return true;
}

bool mat2_not_equal(mat2 l, mat2 r) {
    return !mat2_equal(l, r);
}

bool mat3_not_equal(mat3 l, mat3 r) {
    return !mat3_equal(l, r);
}

bool mat4_not_equal(mat4 l, mat4 r) {
    return !mat4_equal(l, r);
}

void mat2_fprintf(FILE *stream, const mat2 *m) {
    fprintf(stream, "%f, %f\n", m->_11, m->_12);
    fprintf(stream, "%f, %f",  m->_21, m->_22);
}

void mat3_fprintf(FILE *stream, const mat3 *m) {
    fprintf(stream, "%f, %f, %f\n", m->_11, m->_12, m->_13);
    fprintf(stream, "%f, %f, %f\n", m->_21, m->_22, m->_23);
    fprintf(stream, "%f, %f, %f",  m->_31, m->_32, m->_33);
}

void mat4_fprintf(FILE *stream, const mat4 *m) {
    fprintf(stream, "%f, %f, %f, %f\n", m->_11, m->_12, m->_13, m->_14);
    fprintf(stream, "%f, %f, %f, %f\n", m->_21, m->_22, m->_23, m->_24);
    fprintf(stream, "%f, %f, %f, %f\n", m->_31, m->_32, m->_33, m->_34);
    fprintf(stream, "%f, %f, %f, %f",  m->_41, m->_42, m->_43, m->_44);
}

mat3 mat3_fast_inverse(mat3 mat) {
    return mat3_transpose(mat);
}

mat4 mat4_fast_inverse(mat4 mat) {
    mat4 inverse = mat4_transpose(mat);
    inverse._41 = inverse._14 = 0.0f;
    inverse._42 = inverse._24 = 0.0f;
    inverse._43 = inverse._34 = 0.0f;

    vec3 right    = vec3_make(mat._11, mat._12, mat._13);
    vec3 up       = vec3_make(mat._21, mat._22, mat._23);
    vec3 forward  = vec3_make(mat._31, mat._32, mat._33);
    vec3 position = vec3_make(mat._41, mat._42, mat._43);

    inverse._41 = -Dot(right, position);
    inverse._42 = -Dot(up, position);
    inverse._43 = -Dot(forward, position);

    return inverse;
}

#endif /* NO_EXTRAS */

/* ---------- transpose ---------- */

void Transpose(const float *srcMat, float *dstMat, int srcRows, int srcCols) {
    for (int r = 0; r < srcRows; ++r) {
        for (int c = 0; c < srcCols; ++c) {
            dstMat[c * srcRows + r] = srcMat[r * srcCols + c];
        }
    }
}

mat2 mat2_transpose(mat2 matrix) {
    mat2 result;
    Transpose(matrix.asArray, result.asArray, 2, 2);
    return result;
}

mat3 mat3_transpose(mat3 matrix) {
    mat3 result;
    Transpose(matrix.asArray, result.asArray, 3, 3);
    return result;
}

mat4 mat4_transpose(mat4 matrix) {
    mat4 result;
    Transpose(matrix.asArray, result.asArray, 4, 4);
    return result;
}

/* ---------- scalar multiply ---------- */

mat2 mat2_mul_scalar(mat2 matrix, float scalar) {
    mat2 result;
    for (int i = 0; i < 4; ++i) {
        result.asArray[i] = matrix.asArray[i] * scalar;
    }
    return result;
}

mat3 mat3_mul_scalar(mat3 matrix, float scalar) {
    mat3 result;
    for (int i = 0; i < 9; ++i) {
        result.asArray[i] = matrix.asArray[i] * scalar;
    }
    return result;
}

mat4 mat4_mul_scalar(mat4 matrix, float scalar) {
    mat4 result;
    for (int i = 0; i < 16; ++i) {
        result.asArray[i] = matrix.asArray[i] * scalar;
    }
    return result;
}

/* ---------- generic multiply + typed wrappers ---------- */

bool Multiply(float *out,
              const float *matA, int aRows, int aCols,
              const float *matB, int bRows, int bCols) {
    if (aCols != bRows) {
        return false;
    }

    for (int i = 0; i < aRows; ++i) {
        for (int j = 0; j < bCols; ++j) {
            out[bCols * i + j] = 0.0f;
            for (int k = 0; k < bRows; ++k) {
                out[bCols * i + j] += matA[aCols * i + k] * matB[bCols * k + j];
            }
        }
    }

    return true;
}

mat2 mat2_mul(mat2 matrixA, mat2 matrixB) {
    mat2 result;
    Multiply(result.asArray, matrixA.asArray, 2, 2, matrixB.asArray, 2, 2);
    return result;
}

mat3 mat3_mul(mat3 matrixA, mat3 matrixB) {
    mat3 result;
    Multiply(result.asArray, matrixA.asArray, 3, 3, matrixB.asArray, 3, 3);
    return result;
}

mat4 mat4_mul(mat4 matrixA, mat4 matrixB) {
    mat4 result;
    Multiply(result.asArray, matrixA.asArray, 4, 4, matrixB.asArray, 4, 4);
    return result;
}

/* ---------- determinants / minors / cofactors ---------- */

float mat2_determinant(mat2 matrix) {
    return matrix._11 * matrix._22 - matrix._12 * matrix._21;
}

mat2 mat3_cut(mat3 mat, int row, int col) {
    mat2 result;
    int index = 0;

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (i == row || j == col) {
                continue;
            }
            result.asArray[index++] = mat.asArray[3 * i + j];
        }
    }

    return result;
}

mat3 mat4_cut(mat4 mat, int row, int col) {
    mat3 result;
    int index = 0;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == row || j == col) {
                continue;
            }
            result.asArray[index++] = mat.asArray[4 * i + j];
        }
    }

    return result;
}

mat3 mat3_minor(mat3 mat) {
    mat3 result;

    for (int i = 0; i < 3; ++i) {
        float *row = mat3_row(&result, i);
        for (int j = 0; j < 3; ++j) {
            row[j] = mat2_determinant(mat3_cut(mat, i, j));
        }
    }

    return result;
}

mat2 mat2_minor(mat2 mat) {
    mat2 result = {
        ._11 = mat._22, ._12 = mat._21,
        ._21 = mat._12, ._22 = mat._11
    };
    return result;
}

void Cofactor(float *out, const float *minor, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            out[cols * j + i] = minor[cols * j + i] * powf(-1.0f, (float)(i + j));
        }
    }
}

mat2 mat2_cofactor(mat2 mat) {
    mat2 result;
    mat2 m = mat2_minor(mat);
    Cofactor(result.asArray, m.asArray, 2, 2);
    return result;
}

mat3 mat3_cofactor(mat3 mat) {
    mat3 result;
    mat3 m = mat3_minor(mat);
    Cofactor(result.asArray, m.asArray, 3, 3);
    return result;
}

float mat3_determinant(mat3 mat) {
    float result = 0.0f;

    mat3 cofactor = mat3_cofactor(mat);
    float *cofactor_row0 = mat3_row(&cofactor, 0);
    for (int j = 0; j < 3; ++j) {
        result += mat.asArray[3 * 0 + j] * cofactor_row0[j];
    }

    return result;
}

mat4 mat4_minor(mat4 mat) {
    mat4 result;

    for (int i = 0; i < 4; ++i) {
        float *row = mat4_row(&result, i);
        for (int j = 0; j < 4; ++j) {
            row[j] = mat3_determinant(mat4_cut(mat, i, j));
        }
    }

    return result;
}

mat4 mat4_cofactor(mat4 mat) {
    mat4 result;
    mat4 m = mat4_minor(mat);
    Cofactor(result.asArray, m.asArray, 4, 4);
    return result;
}

float mat4_determinant(mat4 m) {
    float result = 0.0f;

    mat4 cofactor = mat4_cofactor(m);
    float *cofactor_row0 = mat4_row(&cofactor, 0);
    for (int j = 0; j < 4; ++j) {
        result += m.asArray[4 * 0 + j] * cofactor_row0[j];
    }

    return result;
}

/* ---------- adjugate / inverse ---------- */

mat2 mat2_adjugate(mat2 mat) {
    return mat2_transpose(mat2_cofactor(mat));
}

mat3 mat3_adjugate(mat3 mat) {
    return mat3_transpose(mat3_cofactor(mat));
}

mat4 mat4_adjugate(mat4 mat) {
    return mat4_transpose(mat4_cofactor(mat));
}

mat2 mat2_inverse(mat2 mat) {
    float det = mat2_determinant(mat);
    if (CMP(det, 0.0f)) {
        return mat2_identity();
    }
    return mat2_mul_scalar(mat2_adjugate(mat), 1.0f / det);
}

mat3 mat3_inverse(mat3 mat) {
    float det = mat3_determinant(mat);
    if (CMP(det, 0.0f)) {
        return mat3_identity();
    }
    return mat3_mul_scalar(mat3_adjugate(mat), 1.0f / det);
}

mat4 mat4_inverse(mat4 m) {
    float det =
        m._11 * m._22 * m._33 * m._44 + m._11 * m._23 * m._34 * m._42 + m._11 * m._24 * m._32 * m._43 +
        m._12 * m._21 * m._34 * m._43 + m._12 * m._23 * m._31 * m._44 + m._12 * m._24 * m._33 * m._41 +
        m._13 * m._21 * m._32 * m._44 + m._13 * m._22 * m._34 * m._41 + m._13 * m._24 * m._31 * m._42 +
        m._14 * m._21 * m._33 * m._42 + m._14 * m._22 * m._31 * m._43 + m._14 * m._23 * m._32 * m._41 -
        m._11 * m._22 * m._34 * m._43 - m._11 * m._23 * m._32 * m._44 - m._11 * m._24 * m._33 * m._42 -
        m._12 * m._21 * m._33 * m._44 - m._12 * m._23 * m._34 * m._41 - m._12 * m._24 * m._31 * m._43 -
        m._13 * m._21 * m._34 * m._42 - m._13 * m._22 * m._31 * m._44 - m._13 * m._24 * m._32 * m._41 -
        m._14 * m._21 * m._32 * m._43 - m._14 * m._22 * m._33 * m._41 - m._14 * m._23 * m._31 * m._42;

    if (CMP(det, 0.0f)) {
        return mat4_identity();
    }

    float i_det = 1.0f / det;

    mat4 result;
    result._11 = (m._22 * m._33 * m._44 + m._23 * m._34 * m._42 + m._24 * m._32 * m._43 - m._22 * m._34 * m._43 - m._23 * m._32 * m._44 - m._24 * m._33 * m._42) * i_det;
    result._12 = (m._12 * m._34 * m._43 + m._13 * m._32 * m._44 + m._14 * m._33 * m._42 - m._12 * m._33 * m._44 - m._13 * m._34 * m._42 - m._14 * m._32 * m._43) * i_det;
    result._13 = (m._12 * m._23 * m._44 + m._13 * m._24 * m._42 + m._14 * m._22 * m._43 - m._12 * m._24 * m._43 - m._13 * m._22 * m._44 - m._14 * m._23 * m._42) * i_det;
    result._14 = (m._12 * m._24 * m._33 + m._13 * m._22 * m._34 + m._14 * m._23 * m._32 - m._12 * m._23 * m._34 - m._13 * m._24 * m._32 - m._14 * m._22 * m._33) * i_det;
    result._21 = (m._21 * m._34 * m._43 + m._23 * m._31 * m._44 + m._24 * m._33 * m._41 - m._21 * m._33 * m._44 - m._23 * m._34 * m._41 - m._24 * m._31 * m._43) * i_det;
    result._22 = (m._11 * m._33 * m._44 + m._13 * m._34 * m._41 + m._14 * m._31 * m._43 - m._11 * m._34 * m._43 - m._13 * m._31 * m._44 - m._14 * m._33 * m._41) * i_det;
    result._23 = (m._11 * m._24 * m._43 + m._13 * m._21 * m._44 + m._14 * m._23 * m._41 - m._11 * m._23 * m._44 - m._13 * m._24 * m._41 - m._14 * m._21 * m._43) * i_det;
    result._24 = (m._11 * m._23 * m._34 + m._13 * m._24 * m._31 + m._14 * m._21 * m._33 - m._11 * m._24 * m._33 - m._13 * m._21 * m._34 - m._14 * m._23 * m._31) * i_det;
    result._31 = (m._21 * m._32 * m._44 + m._22 * m._34 * m._41 + m._24 * m._31 * m._42 - m._21 * m._34 * m._42 - m._22 * m._31 * m._44 - m._24 * m._32 * m._41) * i_det;
    result._32 = (m._11 * m._34 * m._42 + m._12 * m._31 * m._44 + m._14 * m._32 * m._41 - m._11 * m._32 * m._44 - m._12 * m._34 * m._41 - m._14 * m._31 * m._42) * i_det;
    result._33 = (m._11 * m._22 * m._44 + m._12 * m._24 * m._41 + m._14 * m._21 * m._42 - m._11 * m._24 * m._42 - m._12 * m._21 * m._44 - m._14 * m._22 * m._41) * i_det;
    result._34 = (m._11 * m._24 * m._32 + m._12 * m._21 * m._34 + m._14 * m._22 * m._31 - m._11 * m._22 * m._34 - m._12 * m._24 * m._31 - m._14 * m._21 * m._32) * i_det;
    result._41 = (m._21 * m._33 * m._42 + m._22 * m._31 * m._43 + m._23 * m._32 * m._41 - m._21 * m._32 * m._43 - m._22 * m._33 * m._41 - m._23 * m._31 * m._42) * i_det;
    result._42 = (m._11 * m._32 * m._43 + m._12 * m._33 * m._41 + m._13 * m._31 * m._42 - m._11 * m._33 * m._42 - m._12 * m._31 * m._43 - m._13 * m._32 * m._41) * i_det;
    result._43 = (m._11 * m._23 * m._42 + m._12 * m._21 * m._43 + m._13 * m._22 * m._41 - m._11 * m._22 * m._43 - m._12 * m._23 * m._41 - m._13 * m._21 * m._42) * i_det;
    result._44 = (m._11 * m._22 * m._33 + m._12 * m._23 * m._31 + m._13 * m._21 * m._32 - m._11 * m._23 * m._32 - m._12 * m._21 * m._33 - m._13 * m._22 * m._31) * i_det;

#ifdef DO_SANITY_TESTS
#ifndef NO_EXTRAS
    mat4 sanity = mat4_mul(result, m);
    if (!mat4_equal(sanity, mat4_identity())) {
        fprintf(stderr, "ERROR! Expecting matrix x inverse to equal identity!\n");
    }
#endif
#endif

    return result;
}

/* ---------- column-major helpers ---------- */

mat4 mat4_to_column_major(mat4 mat) {
    return mat4_transpose(mat);
}

mat3 mat3_to_column_major(mat3 mat) {
    return mat3_transpose(mat);
}

mat4 mat4_from_column_major_mat4(mat4 mat) {
    return mat4_transpose(mat);
}

mat3 mat3_from_column_major_mat3(mat3 mat) {
    return mat3_transpose(mat);
}

mat4 mat4_from_column_major_array(const float *mat) {
    mat4 m = {
        ._11 = mat[0],  ._12 = mat[1],  ._13 = mat[2],  ._14 = mat[3],
        ._21 = mat[4],  ._22 = mat[5],  ._23 = mat[6],  ._24 = mat[7],
        ._31 = mat[8],  ._32 = mat[9],  ._33 = mat[10], ._34 = mat[11],
        ._41 = mat[12], ._42 = mat[13], ._43 = mat[14], ._44 = mat[15]
    };
    return mat4_transpose(m);
}

/* ---------- translation / scale / accessors ---------- */

mat4 mat4_translation_xyz(float x, float y, float z) {
    mat4 m = mat4_make(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x,    y,    z,    1.0f
    );
    return m;
}

mat4 mat4_translation_vec3(vec3 pos) {
    return mat4_translation_xyz(pos.x, pos.y, pos.z);
}

#ifndef NO_EXTRAS
mat4 mat4_translate_xyz(float x, float y, float z) {
    return mat4_translation_xyz(x, y, z);
}

mat4 mat4_translate_vec3(vec3 pos) {
    return mat4_translation_vec3(pos);
}
#endif

mat4 mat4_from_mat3(mat3 mat) {
    mat4 result = mat4_identity();

    result._11 = mat._11;
    result._12 = mat._12;
    result._13 = mat._13;

    result._21 = mat._21;
    result._22 = mat._22;
    result._23 = mat._23;

    result._31 = mat._31;
    result._32 = mat._32;
    result._33 = mat._33;

    return result;
}

vec3 mat4_get_translation(mat4 mat) {
    return vec3_make(mat._41, mat._42, mat._43);
}

mat4 mat4_scale_xyz(float x, float y, float z) {
    mat4 m = mat4_make(
        x,    0.0f, 0.0f, 0.0f,
        0.0f, y,    0.0f, 0.0f,
        0.0f, 0.0f, z,    0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    return m;
}

mat4 mat4_scale_vec3(vec3 vec) {
    return mat4_scale_xyz(vec.x, vec.y, vec.z);
}

vec3 mat4_get_scale(mat4 mat) {
    return vec3_make(mat._11, mat._22, mat._33);
}

/* ---------- rotation builders (degrees) ---------- */

mat4 Rotation(float pitch, float yaw, float roll) {
    mat4 z = ZRotation(roll);
    mat4 x = XRotation(pitch);
    mat4 y = YRotation(yaw);
    return mat4_mul(mat4_mul(z, x), y);
}

mat3 Rotation3x3(float pitch, float yaw, float roll) {
    mat3 z = ZRotation3x3(roll);
    mat3 x = XRotation3x3(pitch);
    mat3 y = YRotation3x3(yaw);
    return mat3_mul(mat3_mul(z, x), y);
}

#ifndef NO_EXTRAS
mat2 Rotation2x2(float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    mat2 m = {
        ._11 = c,  ._12 = s,
        ._21 = -s, ._22 = c
    };
    return m;
}

mat4 YawPitchRoll(float yaw, float pitch, float roll) {
    yaw   = DEG2RAD(yaw);
    pitch = DEG2RAD(pitch);
    roll  = DEG2RAD(roll);

    mat4 out = mat4_identity(); /* z * x * y */
    out._11 = (cosf(roll) * cosf(yaw)) + (sinf(roll) * sinf(pitch) * sinf(yaw));
    out._12 = (sinf(roll) * cosf(pitch));
    out._13 = (cosf(roll) * -sinf(yaw)) + (sinf(roll) * sinf(pitch) * cosf(yaw));
    out._21 = (-sinf(roll) * cosf(yaw)) + (cosf(roll) * sinf(pitch) * sinf(yaw));
    out._22 = (cosf(roll) * cosf(pitch));
    out._23 = (sinf(roll) * sinf(yaw)) + (cosf(roll) * sinf(pitch) * cosf(yaw));
    out._31 = (cosf(pitch) * sinf(yaw));
    out._32 = -sinf(pitch);
    out._33 = (cosf(pitch) * cosf(yaw));
    out._44 = 1.0f;
    return out;
}
#endif

mat4 XRotation(float angle) {
    angle = DEG2RAD(angle);
    float c = cosf(angle);
    float s = sinf(angle);
    mat4 m = mat4_make(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, c,    s,    0.0f,
        0.0f, -s,   c,    0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    return m;
}

mat3 XRotation3x3(float angle) {
    angle = DEG2RAD(angle);
    float c = cosf(angle);
    float s = sinf(angle);
    mat3 m = mat3_make(
        1.0f, 0.0f, 0.0f,
        0.0f, c,    s,
        0.0f, -s,   c
    );
    return m;
}

mat4 YRotation(float angle) {
    angle = DEG2RAD(angle);
    float c = cosf(angle);
    float s = sinf(angle);
    mat4 m = mat4_make(
        c,    0.0f, -s,   0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        s,    0.0f, c,    0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    return m;
}

mat3 YRotation3x3(float angle) {
    angle = DEG2RAD(angle);
    float c = cosf(angle);
    float s = sinf(angle);
    mat3 m = mat3_make(
        c,    0.0f, -s,
        0.0f, 1.0f, 0.0f,
        s,    0.0f, c
    );
    return m;
}

mat4 ZRotation(float angle) {
    angle = DEG2RAD(angle);
    float c = cosf(angle);
    float s = sinf(angle);
    mat4 m = mat4_make(
        c,    s,    0.0f, 0.0f,
        -s,   c,    0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    return m;
}

mat3 ZRotation3x3(float angle) {
    angle = DEG2RAD(angle);
    float c = cosf(angle);
    float s = sinf(angle);
    mat3 m = mat3_make(
        c,    s,    0.0f,
        -s,   c,    0.0f,
        0.0f, 0.0f, 1.0f
    );
    return m;
}

/* ---------- orthogonalization ---------- */

#ifndef NO_EXTRAS
mat4 mat4_orthogonalize(mat4 mat) {
    vec3 xAxis = vec3_make(mat._11, mat._12, mat._13);
    vec3 yAxis = vec3_make(mat._21, mat._22, mat._23);
    vec3 zAxis = Cross(xAxis, yAxis);

    xAxis = Cross(yAxis, zAxis);
    yAxis = Cross(zAxis, xAxis);
    zAxis = Cross(xAxis, yAxis);

    mat4 m = mat4_make(
        xAxis.x, xAxis.y, xAxis.z, mat._14,
        yAxis.x, yAxis.y, yAxis.z, mat._24,
        zAxis.x, zAxis.y, zAxis.z, mat._34,
        mat._41, mat._42, mat._43, mat._44
    );
    return m;
}

mat3 mat3_orthogonalize(mat3 mat) {
    vec3 xAxis = vec3_make(mat._11, mat._12, mat._13);
    vec3 yAxis = vec3_make(mat._21, mat._22, mat._23);
    vec3 zAxis = Cross(xAxis, yAxis);

    xAxis = Cross(yAxis, zAxis);
    yAxis = Cross(zAxis, xAxis);
    zAxis = Cross(xAxis, yAxis);

    mat3 m = mat3_make(
        xAxis.x, xAxis.y, xAxis.z,
        yAxis.x, yAxis.y, yAxis.z,
        zAxis.x, zAxis.y, zAxis.z
    );
    return m;
}
#endif

/* ---------- axis–angle ---------- */

mat4 AxisAngle(vec3 axis, float angle) {
    angle = DEG2RAD(angle);
    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;

    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    if (!CMP(MagnitudeSq(axis), 1.0f)) {
        float inv_len = 1.0f / Magnitude(axis);
        x *= inv_len;
        y *= inv_len;
        z *= inv_len;
    }

    mat4 m = mat4_make(
        t * (x * x) + c,   t * x * y + s * z, t * x * z - s * y, 0.0f,
        t * x * y - s * z, t * (y * y) + c,   t * y * z + s * x, 0.0f,
        t * x * z + s * y, t * y * z - s * x, t * (z * z) + c,   0.0f,
        0.0f,              0.0f,              0.0f,              1.0f
    );
    return m;
}

mat3 AxisAngle3x3(vec3 axis, float angle) {
    angle = DEG2RAD(angle);
    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;

    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    if (!CMP(MagnitudeSq(axis), 1.0f)) {
        float inv_len = 1.0f / Magnitude(axis);
        x *= inv_len;
        y *= inv_len;
        z *= inv_len;
    }

    mat3 m = mat3_make(
        t * (x * x) + c,   t * x * y + s * z, t * x * z - s * y,
        t * x * y - s * z, t * (y * y) + c,   t * y * z + s * x,
        t * x * z + s * y, t * y * z - s * x, t * (z * z) + c
    );
    return m;
}

/* ---------- multiply points / vectors ---------- */

vec3 MultiplyPoint(vec3 vec, mat4 mat) {
    vec3 result;
    result.x = vec.x * mat._11 + vec.y * mat._21 + vec.z * mat._31 + mat._41;
    result.y = vec.x * mat._12 + vec.y * mat._22 + vec.z * mat._32 + mat._42;
    result.z = vec.x * mat._13 + vec.y * mat._23 + vec.z * mat._33 + mat._43;
    return result;
}

vec3 mat4_multiply_vector(vec3 vec, mat4 mat) {
    vec3 result;
    result.x = vec.x * mat._11 + vec.y * mat._21 + vec.z * mat._31;
    result.y = vec.x * mat._12 + vec.y * mat._22 + vec.z * mat._32;
    result.z = vec.x * mat._13 + vec.y * mat._23 + vec.z * mat._33;
    return result;
}

vec3 mat3_multiply_vector(vec3 vec, mat3 mat) {
    vec3 c0 = vec3_make(mat._11, mat._21, mat._31);
    vec3 c1 = vec3_make(mat._12, mat._22, mat._32);
    vec3 c2 = vec3_make(mat._13, mat._23, mat._33);

    vec3 result;
    result.x = Dot(vec, c0);
    result.y = Dot(vec, c1);
    result.z = Dot(vec, c2);
    return result;
}

/* ---------- high-level transform builders ---------- */

mat4 TransformEuler(vec3 scale, vec3 eulerRotation, vec3 translate) {
    mat4 s = mat4_scale_vec3(scale);
    mat4 r = Rotation(eulerRotation.x, eulerRotation.y, eulerRotation.z);
    mat4 t = mat4_translation_vec3(translate);
    return mat4_mul(mat4_mul(s, r), t);
}

mat4 TransformAxisAngle(vec3 scale, vec3 rotationAxis, float rotationAngle, vec3 translate) {
    mat4 s = mat4_scale_vec3(scale);
    mat4 r = AxisAngle(rotationAxis, rotationAngle);
    mat4 t = mat4_translation_vec3(translate);
    return mat4_mul(mat4_mul(s, r), t);
}

/* ---------- view / projection / ortho ---------- */

mat4 LookAt(vec3 position, vec3 target, vec3 up) {
    vec3 forward = Normalized(vec3_make(
        target.x - position.x,
        target.y - position.y,
        target.z - position.z
    ));
    vec3 right   = Normalized(Cross(up, forward));
    vec3 newUp   = Cross(forward, right);

#ifdef DO_SANITY_TESTS
    mat4 viewPosition = mat4_translation_vec3(position);
    mat4 viewOrientation = mat4_make(
        right.x,   right.y,   right.z,   0.0f,
        newUp.x,   newUp.y,   newUp.z,   0.0f,
        forward.x, forward.y, forward.z, 0.0f,
        0.0f,      0.0f,      0.0f,      1.0f
    );

    mat4 view = mat4_inverse(mat4_mul(viewOrientation, viewPosition));
    mat4 result =
#else
    mat4 result =
#endif
        mat4_make(
            right.x,  newUp.x,  forward.x,  0.0f,
            right.y,  newUp.y,  forward.y,  0.0f,
            right.z,  newUp.z,  forward.z,  0.0f,
            -Dot(right, position),
            -Dot(newUp, position),
            -Dot(forward, position),
            1.0f
        );
#ifdef DO_SANITY_TESTS
#ifndef NO_EXTRAS
    if (!mat4_equal(result, view)) {
        fprintf(stderr, "Error, result and view do not match in an expected manner!\n");
        fprintf(stderr, "view:\n");
        mat4_fprintf(stderr, &view);
        fprintf(stderr, "\n\nresult:\n");
        mat4_fprintf(stderr, &result);
        fprintf(stderr, "\n");
    }
#endif
#endif
    return result;
}

mat4 Projection(float fov, float aspect, float zNear, float zFar) {
    float tanHalfFov = tanf(DEG2RAD(fov * 0.5f));

    mat4 result = mat4_identity();

    float fovY = 1.0f / tanHalfFov;      /* cot(fov/2) */
    float fovX = fovY / aspect;          /* cot(fov/2) / aspect */

    result._11 = fovX;
    result._22 = fovY;
    result._33 = zFar / (zFar - zNear);
    result._34 = 1.0f;
    result._43 = -zNear * result._33;
    result._44 = 0.0f;

    return result;
}

mat4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
    float _11 = 2.0f / (right - left);
    float _22 = 2.0f / (top - bottom);
    float _33 = 1.0f / (zFar - zNear);
    float _41 = (left + right) / (left - right);
    float _42 = (top + bottom) / (bottom - top);
    float _43 = (zNear) / (zNear - zFar);

    mat4 m = mat4_make(
        _11,  0.0f, 0.0f, 0.0f,
        0.0f, _22,  0.0f, 0.0f,
        0.0f, 0.0f, _33,  0.0f,
        _41,  _42,  _43,  1.0f
    );
    return m;
}

/* ---------- decompose rotation ---------- */

vec3 Decompose(mat3 rot1) {
    mat3 rot = mat3_transpose(rot1);

    float sy = sqrtf(rot._11 * rot._11 + rot._21 * rot._21);

    bool singular = sy < 1e-6f;

    float x, y, z;
    if (!singular) {
        x = atan2f(rot._32, rot._33);
        y = atan2f(-rot._31, sy);
        z = atan2f(rot._21, rot._11);
    } else {
        x = atan2f(-rot._23, rot._22);
        y = atan2f(-rot._31, sy);
        z = 0.0f;
    }

    return vec3_make(x, y, z);
}


mat4 mat4_perspective(float fov, float aspect, float zNear, float zFar) {
    return Projection(fov, aspect, zNear, zFar);
}


mat4 mat4_ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
    return Ortho(left, right, bottom, top, zNear, zFar);
}
