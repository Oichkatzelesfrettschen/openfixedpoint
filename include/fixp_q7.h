/**
 * @file fixp_q7.h
 * @brief Q0.7 (8-bit signed) fixed-point arithmetic
 *
 * Q7 format: 1 sign bit, 7 fractional bits
 * Range: [-1.0, +0.9921875] ([-128, +127] raw)
 * Resolution: 2^-7 = 0.0078125
 *
 * This is a header-only implementation for maximum portability.
 * All functions are static inline for zero overhead.
 *
 * @copyright Public Domain / CC0
 */

#ifndef FIXP_Q7_H
#define FIXP_Q7_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Type Definitions                                                          */
/*===========================================================================*/

/**
 * @brief Q0.7 fixed-point type (8-bit signed, 7 fractional bits)
 */
typedef int8_t q7_t;

/**
 * @brief UQ0.8 fixed-point type (8-bit unsigned, 8 fractional bits)
 */
typedef uint8_t uq8_t;

/*===========================================================================*/
/* Constants                                                                 */
/*===========================================================================*/

/** Number of fractional bits in Q7 */
#define Q7_FRAC_BITS  7

/** Value representing 1.0 in Q7 (actually max representable ~0.992) */
#define Q7_ONE        ((q7_t)0x7F)

/** Maximum Q7 value (0.9921875) */
#define Q7_MAX        ((q7_t)0x7F)

/** Minimum Q7 value (-1.0) */
#define Q7_MIN        ((q7_t)0x80)

/** Zero in Q7 */
#define Q7_ZERO       ((q7_t)0x00)

/** Epsilon (smallest positive value) = 2^-7 */
#define Q7_EPSILON    ((q7_t)0x01)

/** Number of fractional bits in UQ8 */
#define UQ8_FRAC_BITS 8

/** Value representing 1.0 in UQ8 (actually max representable ~0.996) */
#define UQ8_ONE       ((uq8_t)0xFF)

/** Maximum UQ8 value */
#define UQ8_MAX       ((uq8_t)0xFF)

/** Minimum UQ8 value (0) */
#define UQ8_MIN       ((uq8_t)0x00)

/*===========================================================================*/
/* Conversion Functions                                                      */
/*===========================================================================*/

/**
 * @brief Convert double to Q7
 * @param d Double value (should be in [-1.0, +1.0))
 * @return Q7 representation
 */
static inline q7_t q7_from_double(double d) {
    if (d >= 1.0) return Q7_MAX;
    if (d < -1.0) return Q7_MIN;
    return (q7_t)(d * 128.0 + (d >= 0 ? 0.5 : -0.5));
}

/**
 * @brief Convert Q7 to double
 * @param q Q7 value
 * @return Double representation
 */
static inline double q7_to_double(q7_t q) {
    return (double)q / 128.0;
}

/**
 * @brief Convert float to Q7
 */
static inline q7_t q7_from_float(float f) {
    if (f >= 1.0f) return Q7_MAX;
    if (f < -1.0f) return Q7_MIN;
    return (q7_t)(f * 128.0f + (f >= 0 ? 0.5f : -0.5f));
}

/**
 * @brief Convert Q7 to float
 */
static inline float q7_to_float(q7_t q) {
    return (float)q / 128.0f;
}

/**
 * @brief Convert double to UQ8
 * @param d Double value (should be in [0.0, 1.0))
 * @return UQ8 representation
 */
static inline uq8_t uq8_from_double(double d) {
    if (d >= 1.0) return UQ8_MAX;
    if (d < 0.0) return UQ8_MIN;
    return (uq8_t)(d * 256.0 + 0.5);
}

/**
 * @brief Convert UQ8 to double
 */
static inline double uq8_to_double(uq8_t q) {
    return (double)q / 256.0;
}

/*===========================================================================*/
/* Basic Arithmetic - Wrapping                                               */
/*===========================================================================*/

/**
 * @brief Add two Q7 values (wrapping on overflow)
 */
static inline q7_t q7_add(q7_t a, q7_t b) {
    return (q7_t)((int8_t)a + (int8_t)b);
}

/**
 * @brief Subtract two Q7 values (wrapping on overflow)
 */
static inline q7_t q7_sub(q7_t a, q7_t b) {
    return (q7_t)((int8_t)a - (int8_t)b);
}

/**
 * @brief Negate a Q7 value (wrapping)
 * @note q7_neg(Q7_MIN) wraps to Q7_MIN
 */
static inline q7_t q7_neg(q7_t a) {
    return (q7_t)(-(int8_t)a);
}

/**
 * @brief Multiply two Q7 values
 * @return Q7 result: (a * b) >> 7 with rounding
 */
static inline q7_t q7_mul(q7_t a, q7_t b) {
    int16_t product = (int16_t)a * (int16_t)b;
    /* Round to nearest: add 0.5 in Q14, then shift */
    return (q7_t)((product + 64) >> 7);
}

/**
 * @brief Divide Q7 by Q7
 * @return Q7 result: (a << 7) / b
 * @note Division by zero returns Q7_MAX or Q7_MIN based on sign
 */
static inline q7_t q7_div(q7_t a, q7_t b) {
    if (b == 0) {
        return (a >= 0) ? Q7_MAX : Q7_MIN;
    }
    int16_t dividend = (int16_t)a << 7;
    return (q7_t)(dividend / (int16_t)b);
}

/*===========================================================================*/
/* Basic Arithmetic - Saturating                                             */
/*===========================================================================*/

/**
 * @brief Add two Q7 values with saturation
 */
static inline q7_t q7_add_sat(q7_t a, q7_t b) {
    int16_t sum = (int16_t)a + (int16_t)b;
    if (sum > 127) return Q7_MAX;
    if (sum < -128) return Q7_MIN;
    return (q7_t)sum;
}

/**
 * @brief Subtract two Q7 values with saturation
 */
static inline q7_t q7_sub_sat(q7_t a, q7_t b) {
    int16_t diff = (int16_t)a - (int16_t)b;
    if (diff > 127) return Q7_MAX;
    if (diff < -128) return Q7_MIN;
    return (q7_t)diff;
}

/**
 * @brief Negate a Q7 value with saturation
 * @note q7_neg_sat(Q7_MIN) returns Q7_MAX
 */
static inline q7_t q7_neg_sat(q7_t a) {
    if (a == Q7_MIN) return Q7_MAX;
    return (q7_t)(-(int8_t)a);
}

/**
 * @brief Multiply two Q7 values with saturation
 */
static inline q7_t q7_mul_sat(q7_t a, q7_t b) {
    int16_t product = (int16_t)a * (int16_t)b;
    int16_t result = (product + 64) >> 7;
    if (result > 127) return Q7_MAX;
    if (result < -128) return Q7_MIN;
    return (q7_t)result;
}

/*===========================================================================*/
/* Comparison Operations                                                     */
/*===========================================================================*/

static inline bool q7_eq(q7_t a, q7_t b) { return a == b; }
static inline bool q7_ne(q7_t a, q7_t b) { return a != b; }
static inline bool q7_lt(q7_t a, q7_t b) { return a < b; }
static inline bool q7_le(q7_t a, q7_t b) { return a <= b; }
static inline bool q7_gt(q7_t a, q7_t b) { return a > b; }
static inline bool q7_ge(q7_t a, q7_t b) { return a >= b; }

/**
 * @brief Three-way comparison
 * @return -1 if a < b, 0 if a == b, +1 if a > b
 */
static inline int q7_cmp(q7_t a, q7_t b) {
    return (a > b) - (a < b);
}

/*===========================================================================*/
/* Utility Operations                                                        */
/*===========================================================================*/

/**
 * @brief Absolute value
 * @note q7_abs(Q7_MIN) returns Q7_MAX (saturating)
 */
static inline q7_t q7_abs(q7_t a) {
    if (a == Q7_MIN) return Q7_MAX;
    return (a < 0) ? -a : a;
}

/**
 * @brief Minimum of two Q7 values
 */
static inline q7_t q7_min(q7_t a, q7_t b) {
    return (a < b) ? a : b;
}

/**
 * @brief Maximum of two Q7 values
 */
static inline q7_t q7_max(q7_t a, q7_t b) {
    return (a > b) ? a : b;
}

/**
 * @brief Clamp Q7 value to range [lo, hi]
 */
static inline q7_t q7_clamp(q7_t x, q7_t lo, q7_t hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

/**
 * @brief Left shift
 */
static inline q7_t q7_shl(q7_t a, int shift) {
    return (q7_t)(a << shift);
}

/**
 * @brief Arithmetic right shift
 */
static inline q7_t q7_shr(q7_t a, int shift) {
    return (q7_t)(a >> shift);
}

/*===========================================================================*/
/* Multiply-Accumulate                                                       */
/*===========================================================================*/

/**
 * @brief Multiply-accumulate: acc + (a * b)
 * @param acc Accumulator (Q7)
 * @param a First operand (Q7)
 * @param b Second operand (Q7)
 * @return acc + (a * b) >> 7
 */
static inline q7_t q7_mac(q7_t acc, q7_t a, q7_t b) {
    int16_t product = (int16_t)a * (int16_t)b;
    int16_t result = (int16_t)acc + ((product + 64) >> 7);
    return (q7_t)result;
}

/**
 * @brief Saturating multiply-accumulate
 */
static inline q7_t q7_mac_sat(q7_t acc, q7_t a, q7_t b) {
    int16_t product = (int16_t)a * (int16_t)b;
    int16_t result = (int16_t)acc + ((product + 64) >> 7);
    if (result > 127) return Q7_MAX;
    if (result < -128) return Q7_MIN;
    return (q7_t)result;
}

/*===========================================================================*/
/* Extended Precision Accumulator                                            */
/*===========================================================================*/

/**
 * @brief 16-bit accumulator for Q7 operations
 *
 * Use for summing many Q7 products without intermediate overflow.
 * Final result can be extracted with q7_acc_to_q7().
 */
typedef int16_t q7_acc_t;

/**
 * @brief Initialize accumulator to zero
 */
static inline q7_acc_t q7_acc_init(void) {
    return 0;
}

/**
 * @brief Initialize accumulator from Q7 value
 */
static inline q7_acc_t q7_acc_from_q7(q7_t a) {
    return (q7_acc_t)a;
}

/**
 * @brief Accumulate a Q7 product: acc += (a * b) >> 7
 */
static inline q7_acc_t q7_acc_mac(q7_acc_t acc, q7_t a, q7_t b) {
    int16_t product = (int16_t)a * (int16_t)b;
    return acc + ((product + 64) >> 7);
}

/**
 * @brief Extract Q7 from accumulator with saturation
 */
static inline q7_t q7_acc_to_q7(q7_acc_t acc) {
    if (acc > 127) return Q7_MAX;
    if (acc < -128) return Q7_MIN;
    return (q7_t)acc;
}

/*===========================================================================*/
/* UQ8 Operations                                                            */
/*===========================================================================*/

/**
 * @brief Add two UQ8 values (wrapping)
 */
static inline uq8_t uq8_add(uq8_t a, uq8_t b) {
    return (uq8_t)(a + b);
}

/**
 * @brief Add two UQ8 values with saturation
 */
static inline uq8_t uq8_add_sat(uq8_t a, uq8_t b) {
    uint16_t sum = (uint16_t)a + (uint16_t)b;
    return (sum > 255) ? UQ8_MAX : (uq8_t)sum;
}

/**
 * @brief Subtract two UQ8 values (wrapping)
 */
static inline uq8_t uq8_sub(uq8_t a, uq8_t b) {
    return (uq8_t)(a - b);
}

/**
 * @brief Subtract two UQ8 values with saturation (floor at 0)
 */
static inline uq8_t uq8_sub_sat(uq8_t a, uq8_t b) {
    return (a > b) ? (uq8_t)(a - b) : UQ8_MIN;
}

/**
 * @brief Multiply two UQ8 values
 * @return (a * b) >> 8
 */
static inline uq8_t uq8_mul(uq8_t a, uq8_t b) {
    uint16_t product = (uint16_t)a * (uint16_t)b;
    return (uq8_t)((product + 128) >> 8);
}

/**
 * @brief Minimum of two UQ8 values
 */
static inline uq8_t uq8_min(uq8_t a, uq8_t b) {
    return (a < b) ? a : b;
}

/**
 * @brief Maximum of two UQ8 values
 */
static inline uq8_t uq8_max(uq8_t a, uq8_t b) {
    return (a > b) ? a : b;
}

/*===========================================================================*/
/* Format Conversion                                                         */
/*===========================================================================*/

/**
 * @brief Convert Q7 to UQ8 (clamp negative to zero)
 */
static inline uq8_t q7_to_uq8(q7_t a) {
    if (a < 0) return UQ8_MIN;
    /* Q7 max is 127 (0.992), scale to UQ8: 127 * 2 = 254 */
    return (uq8_t)((uint16_t)a << 1);
}

/**
 * @brief Convert UQ8 to Q7 (may lose top bit)
 */
static inline q7_t uq8_to_q7(uq8_t a) {
    /* UQ8 range [0, 255] -> Q7 range [-128, 127] needs shifting */
    /* Interpret UQ8 [0, 0.996] as Q7 [0, ~0.5] */
    return (q7_t)(a >> 1);
}

#ifdef __cplusplus
}

/* C++ namespace wrapper */
namespace fixp {
    using ::q7_t;
    using ::uq8_t;

    /* Inline wrappers with operator overloading could be added here */
}
#endif

#endif /* FIXP_Q7_H */
