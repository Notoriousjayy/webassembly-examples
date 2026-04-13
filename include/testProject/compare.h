/**
 * compare.h - Floating-Point Comparison Utilities (C23)
 * 
 * Based on techniques from:
 * - http://realtimecollisiondetection.net/pubs/Tolerances/
 * - https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
 */

#ifndef COMPARE_H
#define COMPARE_H

#include <math.h>
#include <float.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>  /* For abs() */

/*******************************************************************************
 * Float_t Union - Access floating-point bit representation
 ******************************************************************************/

typedef union Float_t {
    int32_t i;
    float   f;
} Float_t;

/*******************************************************************************
 * Float_t Helper Functions
 ******************************************************************************/

static inline Float_t float_t_create(float num) {
    Float_t result;
    result.f = num;
    return result;
}

static inline bool float_t_negative(Float_t ft) {
    return ft.i < 0;
}

static inline int32_t float_t_raw_mantissa(Float_t ft) {
    return ft.i & ((1 << 23) - 1);
}

static inline int32_t float_t_raw_exponent(Float_t ft) {
    return (ft.i >> 23) & 0xFF;
}

/*******************************************************************************
 * Comparison Functions
 ******************************************************************************/

/**
 * Almost equal using relative difference.
 * Good for comparing numbers that are expected to be similar in magnitude.
 */
static inline bool almost_equal_relative(float a, float b, float max_rel_diff) {
    float diff = fabsf(a - b);
    float abs_a = fabsf(a);
    float abs_b = fabsf(b);
    float largest = (abs_b > abs_a) ? abs_b : abs_a;

    return diff <= largest * max_rel_diff;
}

/**
 * Almost equal using ULPs (Units in Last Place) and absolute difference.
 * ULPs comparison is good for numbers that have been computed through 
 * similar operations, as errors accumulate in a predictable way.
 */
static inline bool almost_equal_ulps_and_abs(float a, float b, float max_diff, int max_ulps_diff) {
    /* Check if the numbers are really close -- needed when comparing numbers near zero */
    float abs_diff = fabsf(a - b);
    if (abs_diff <= max_diff) {
        return true;
    }

    Float_t ua = float_t_create(a);
    Float_t ub = float_t_create(b);

    /* Different signs means they do not match */
    if (float_t_negative(ua) != float_t_negative(ub)) {
        return false;
    }

    /* Find the difference in ULPs */
    int ulps_diff = abs(ua.i - ub.i);
    return ulps_diff <= max_ulps_diff;
}

/**
 * Almost equal using both relative and absolute difference.
 * This is the most robust comparison for general use:
 * - Absolute difference handles numbers near zero
 * - Relative difference handles larger numbers
 */
static inline bool almost_equal_relative_and_abs(float a, float b, float max_diff, float max_rel_diff) {
    /* Check if the numbers are really close -- needed when comparing numbers near zero */
    float diff = fabsf(a - b);
    if (diff <= max_diff) {
        return true;
    }

    float abs_a = fabsf(a);
    float abs_b = fabsf(b);
    float largest = (abs_b > abs_a) ? abs_b : abs_a;

    return diff <= largest * max_rel_diff;
}

/*******************************************************************************
 * Convenience Wrappers with Default Parameters
 ******************************************************************************/

/**
 * Almost equal relative with default epsilon.
 */
static inline bool almost_equal_relative_default(float a, float b) {
    return almost_equal_relative(a, b, FLT_EPSILON);
}

/**
 * Almost equal relative and absolute with default relative epsilon.
 */
static inline bool almost_equal_relative_and_abs_default(float a, float b, float max_diff) {
    return almost_equal_relative_and_abs(a, b, max_diff, FLT_EPSILON);
}

/*******************************************************************************
 * CMP Macro - Primary Comparison Interface
 ******************************************************************************/

/**
 * Default comparison macro using relative and absolute tolerance.
 * Uses 0.005f as the absolute tolerance for numbers near zero.
 */
#define CMP(x, y) \
    almost_equal_relative_and_abs((x), (y), 0.005f, FLT_EPSILON)

/*******************************************************************************
 * Alternative CMP Macros (Commented - Choose One)
 ******************************************************************************/

/*
 * Simple relative comparison from Real-Time Collision Detection:
 * 
 * #define CMP(x, y) \
 *     (fabsf((x) - (y)) <= FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))))
 */

/*
 * Simpler relative comparison:
 * 
 * #define CMP(x, y) \
 *     (fabsf((x) - (y)) <= ((fabsf(y) > fabsf(x)) ? fabsf(y) : fabsf(x)) * FLT_EPSILON)
 */

/*******************************************************************************
 * Strict Comparison (No Tolerance)
 ******************************************************************************/

/**
 * Exact equality check - use sparingly, floating-point rarely equals exactly.
 */
static inline bool float_exact_equal(float a, float b) {
    Float_t ua = float_t_create(a);
    Float_t ub = float_t_create(b);
    return ua.i == ub.i;
}

/*******************************************************************************
 * Debug Utilities
 ******************************************************************************/

#ifndef NO_EXTRAS
#include <stdio.h>

/**
 * Print floating-point bit representation for debugging.
 */
static inline void float_print_bits(FILE* stream, float f) {
    Float_t ft = float_t_create(f);
    
    int32_t sign = (ft.i >> 31) & 0x1;
    int32_t exponent = float_t_raw_exponent(ft);
    int32_t mantissa = float_t_raw_mantissa(ft);
    
    fprintf(stream, "Float: %f\n", f);
    fprintf(stream, "  Hex:      0x%08X\n", (uint32_t)ft.i);
    fprintf(stream, "  Sign:     %d (%s)\n", sign, sign ? "negative" : "positive");
    fprintf(stream, "  Exponent: %d (biased), %d (actual)\n", exponent, exponent - 127);
    fprintf(stream, "  Mantissa: 0x%06X (%d)\n", mantissa, mantissa);
}

/**
 * Compare two floats and print detailed comparison info.
 */
static inline void float_compare_debug(FILE* stream, float a, float b) {
    Float_t ua = float_t_create(a);
    Float_t ub = float_t_create(b);
    
    int ulps_diff = abs(ua.i - ub.i);
    float abs_diff = fabsf(a - b);
    float rel_diff = (a != 0.0f || b != 0.0f) ? 
                     abs_diff / fmaxf(fabsf(a), fabsf(b)) : 0.0f;
    
    fprintf(stream, "Comparing: %f vs %f\n", a, b);
    fprintf(stream, "  Absolute diff: %e\n", abs_diff);
    fprintf(stream, "  Relative diff: %e\n", rel_diff);
    fprintf(stream, "  ULPs diff:     %d\n", ulps_diff);
    fprintf(stream, "  CMP result:    %s\n", CMP(a, b) ? "EQUAL" : "NOT EQUAL");
}
#endif /* NO_EXTRAS */

#endif /* COMPARE_H */
