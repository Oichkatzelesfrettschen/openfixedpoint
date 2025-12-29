/**
 * @file fixp_q16_16.h
 * @brief Q16.16 (32-bit signed) fixed-point arithmetic
 *
 * Q16.16 format: 1 sign bit, 15 integer bits, 16 fractional bits
 * Range: [-32768.0, +32767.99998474]
 * Resolution: 2^-16 ≈ 0.0000153
 *
 * This is a header-only implementation for maximum portability.
 * All functions are static inline for zero overhead.
 *
 * @copyright Public Domain / CC0
 */

#ifndef FIXP_Q16_16_H
#define FIXP_Q16_16_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Type Definitions                                                          */
/*===========================================================================*/

/**
 * @brief Q16.16 fixed-point type
 *
 * Transparent struct wrapper for type safety.
 * Use Q16_16_RAW() to access raw int32_t value.
 */
typedef struct q16_16 {
    int32_t v;
} q16_16_t;

/**
 * @brief Access raw value
 */
#define Q16_16_RAW(x)      ((x).v)

/**
 * @brief Wrap raw int32_t as q16_16_t
 */
#define Q16_16_WRAP(raw)   ((q16_16_t){.v = (raw)})

/*===========================================================================*/
/* Constants                                                                 */
/*===========================================================================*/

/** Number of fractional bits */
#define Q16_16_FRAC_BITS  16

/** Number of integer bits (excluding sign) */
#define Q16_16_INT_BITS   15

/** Value representing 1.0 */
#define Q16_16_ONE        Q16_16_WRAP(0x00010000)

/** Value representing -1.0 */
#define Q16_16_NEG_ONE    Q16_16_WRAP(0xFFFF0000)

/** Maximum value (~32767.99998) */
#define Q16_16_MAX        Q16_16_WRAP(0x7FFFFFFF)

/** Minimum value (-32768.0) */
#define Q16_16_MIN        Q16_16_WRAP(0x80000000)

/** Zero */
#define Q16_16_ZERO       Q16_16_WRAP(0x00000000)

/** Epsilon (smallest positive value) = 2^-16 */
#define Q16_16_EPSILON    Q16_16_WRAP(0x00000001)

/** Pi (3.14159...) */
#define Q16_16_PI         Q16_16_WRAP(0x0003243F)

/** Pi/2 */
#define Q16_16_PI_2       Q16_16_WRAP(0x0001921F)

/** Pi/4 */
#define Q16_16_PI_4       Q16_16_WRAP(0x0000C90F)

/** 2*Pi */
#define Q16_16_TWO_PI     Q16_16_WRAP(0x0006487F)

/** e (2.71828...) */
#define Q16_16_E          Q16_16_WRAP(0x0002B7E1)

/** sqrt(2) */
#define Q16_16_SQRT2      Q16_16_WRAP(0x00016A09)

/** 1/sqrt(2) = sqrt(2)/2 */
#define Q16_16_SQRT2_2    Q16_16_WRAP(0x0000B504)

/*===========================================================================*/
/* Conversion Functions                                                      */
/*===========================================================================*/

/**
 * @brief Convert double to Q16.16
 */
static inline q16_16_t q16_16_from_double(double d) {
    if (d >= 32768.0) return Q16_16_MAX;
    if (d < -32768.0) return Q16_16_MIN;
    return Q16_16_WRAP((int32_t)(d * 65536.0 + (d >= 0 ? 0.5 : -0.5)));
}

/**
 * @brief Convert Q16.16 to double
 */
static inline double q16_16_to_double(q16_16_t q) {
    return (double)Q16_16_RAW(q) / 65536.0;
}

/**
 * @brief Convert float to Q16.16
 */
static inline q16_16_t q16_16_from_float(float f) {
    if (f >= 32768.0f) return Q16_16_MAX;
    if (f < -32768.0f) return Q16_16_MIN;
    return Q16_16_WRAP((int32_t)(f * 65536.0f + (f >= 0 ? 0.5f : -0.5f)));
}

/**
 * @brief Convert Q16.16 to float
 */
static inline float q16_16_to_float(q16_16_t q) {
    return (float)Q16_16_RAW(q) / 65536.0f;
}

/**
 * @brief Convert integer to Q16.16
 */
static inline q16_16_t q16_16_from_int(int32_t i) {
    return Q16_16_WRAP(i << 16);
}

/**
 * @brief Convert Q16.16 to integer (truncate)
 */
static inline int32_t q16_16_to_int(q16_16_t q) {
    return Q16_16_RAW(q) >> 16;
}

/**
 * @brief Convert Q16.16 to integer (round to nearest)
 */
static inline int32_t q16_16_to_int_rnd(q16_16_t q) {
    int32_t raw = Q16_16_RAW(q);
    if (raw >= 0) {
        return (raw + 0x8000) >> 16;
    } else {
        return (raw + 0x7FFF) >> 16;
    }
}

/**
 * @brief Get fractional part only
 */
static inline q16_16_t q16_16_frac(q16_16_t q) {
    return Q16_16_WRAP(Q16_16_RAW(q) & 0xFFFF);
}

/**
 * @brief Get integer part only (as Q16.16)
 */
static inline q16_16_t q16_16_floor(q16_16_t q) {
    return Q16_16_WRAP(Q16_16_RAW(q) & 0xFFFF0000);
}

/*===========================================================================*/
/* Basic Arithmetic - Wrapping                                               */
/*===========================================================================*/

/**
 * @brief Add two Q16.16 values (wrapping on overflow)
 */
static inline q16_16_t q16_16_add(q16_16_t a, q16_16_t b) {
    return Q16_16_WRAP(Q16_16_RAW(a) + Q16_16_RAW(b));
}

/**
 * @brief Subtract two Q16.16 values (wrapping on overflow)
 */
static inline q16_16_t q16_16_sub(q16_16_t a, q16_16_t b) {
    return Q16_16_WRAP(Q16_16_RAW(a) - Q16_16_RAW(b));
}

/**
 * @brief Negate a Q16.16 value (wrapping)
 */
static inline q16_16_t q16_16_neg(q16_16_t a) {
    return Q16_16_WRAP(-Q16_16_RAW(a));
}

/**
 * @brief Multiply two Q16.16 values
 * @return (a * b) >> 16 with rounding
 */
static inline q16_16_t q16_16_mul(q16_16_t a, q16_16_t b) {
    int64_t product = (int64_t)Q16_16_RAW(a) * (int64_t)Q16_16_RAW(b);
    /* Round to nearest */
    return Q16_16_WRAP((int32_t)((product + 0x8000) >> 16));
}

/**
 * @brief Multiply Q16.16 by integer
 */
static inline q16_16_t q16_16_mul_int(q16_16_t a, int32_t b) {
    return Q16_16_WRAP(Q16_16_RAW(a) * b);
}

/**
 * @brief Divide Q16.16 by Q16.16
 * @return (a << 16) / b
 * @note Division by zero returns MAX or MIN based on sign
 */
static inline q16_16_t q16_16_div(q16_16_t a, q16_16_t b) {
    if (Q16_16_RAW(b) == 0) {
        return (Q16_16_RAW(a) >= 0) ? Q16_16_MAX : Q16_16_MIN;
    }
    int64_t dividend = (int64_t)Q16_16_RAW(a) << 16;
    return Q16_16_WRAP((int32_t)(dividend / Q16_16_RAW(b)));
}

/**
 * @brief Divide Q16.16 by integer
 */
static inline q16_16_t q16_16_div_int(q16_16_t a, int32_t b) {
    if (b == 0) {
        return (Q16_16_RAW(a) >= 0) ? Q16_16_MAX : Q16_16_MIN;
    }
    return Q16_16_WRAP(Q16_16_RAW(a) / b);
}

/*===========================================================================*/
/* Basic Arithmetic - Saturating                                             */
/*===========================================================================*/

/**
 * @brief Add two Q16.16 values with saturation
 */
static inline q16_16_t q16_16_add_sat(q16_16_t a, q16_16_t b) {
    int64_t sum = (int64_t)Q16_16_RAW(a) + (int64_t)Q16_16_RAW(b);
    if (sum > INT32_MAX) return Q16_16_MAX;
    if (sum < INT32_MIN) return Q16_16_MIN;
    return Q16_16_WRAP((int32_t)sum);
}

/**
 * @brief Subtract two Q16.16 values with saturation
 */
static inline q16_16_t q16_16_sub_sat(q16_16_t a, q16_16_t b) {
    int64_t diff = (int64_t)Q16_16_RAW(a) - (int64_t)Q16_16_RAW(b);
    if (diff > INT32_MAX) return Q16_16_MAX;
    if (diff < INT32_MIN) return Q16_16_MIN;
    return Q16_16_WRAP((int32_t)diff);
}

/**
 * @brief Negate a Q16.16 value with saturation
 */
static inline q16_16_t q16_16_neg_sat(q16_16_t a) {
    if (Q16_16_RAW(a) == INT32_MIN) return Q16_16_MAX;
    return Q16_16_WRAP(-Q16_16_RAW(a));
}

/**
 * @brief Multiply two Q16.16 values with saturation
 */
static inline q16_16_t q16_16_mul_sat(q16_16_t a, q16_16_t b) {
    int64_t product = (int64_t)Q16_16_RAW(a) * (int64_t)Q16_16_RAW(b);
    int64_t result = (product + 0x8000) >> 16;
    if (result > INT32_MAX) return Q16_16_MAX;
    if (result < INT32_MIN) return Q16_16_MIN;
    return Q16_16_WRAP((int32_t)result);
}

/*===========================================================================*/
/* Comparison Operations                                                     */
/*===========================================================================*/

static inline bool q16_16_eq(q16_16_t a, q16_16_t b) {
    return Q16_16_RAW(a) == Q16_16_RAW(b);
}

static inline bool q16_16_ne(q16_16_t a, q16_16_t b) {
    return Q16_16_RAW(a) != Q16_16_RAW(b);
}

static inline bool q16_16_lt(q16_16_t a, q16_16_t b) {
    return Q16_16_RAW(a) < Q16_16_RAW(b);
}

static inline bool q16_16_le(q16_16_t a, q16_16_t b) {
    return Q16_16_RAW(a) <= Q16_16_RAW(b);
}

static inline bool q16_16_gt(q16_16_t a, q16_16_t b) {
    return Q16_16_RAW(a) > Q16_16_RAW(b);
}

static inline bool q16_16_ge(q16_16_t a, q16_16_t b) {
    return Q16_16_RAW(a) >= Q16_16_RAW(b);
}

/**
 * @brief Three-way comparison
 * @return -1 if a < b, 0 if a == b, +1 if a > b
 */
static inline int q16_16_cmp(q16_16_t a, q16_16_t b) {
    int32_t av = Q16_16_RAW(a);
    int32_t bv = Q16_16_RAW(b);
    return (av > bv) - (av < bv);
}

/*===========================================================================*/
/* Utility Operations                                                        */
/*===========================================================================*/

/**
 * @brief Absolute value (saturating)
 */
static inline q16_16_t q16_16_abs(q16_16_t a) {
    int32_t v = Q16_16_RAW(a);
    if (v == INT32_MIN) return Q16_16_MAX;
    return Q16_16_WRAP((v < 0) ? -v : v);
}

/**
 * @brief Minimum of two values
 */
static inline q16_16_t q16_16_min(q16_16_t a, q16_16_t b) {
    return (Q16_16_RAW(a) < Q16_16_RAW(b)) ? a : b;
}

/**
 * @brief Maximum of two values
 */
static inline q16_16_t q16_16_max(q16_16_t a, q16_16_t b) {
    return (Q16_16_RAW(a) > Q16_16_RAW(b)) ? a : b;
}

/**
 * @brief Clamp value to range [lo, hi]
 */
static inline q16_16_t q16_16_clamp(q16_16_t x, q16_16_t lo, q16_16_t hi) {
    if (Q16_16_RAW(x) < Q16_16_RAW(lo)) return lo;
    if (Q16_16_RAW(x) > Q16_16_RAW(hi)) return hi;
    return x;
}

/**
 * @brief Sign of value (-1, 0, or +1)
 */
static inline q16_16_t q16_16_sign(q16_16_t a) {
    int32_t v = Q16_16_RAW(a);
    if (v > 0) return Q16_16_ONE;
    if (v < 0) return Q16_16_NEG_ONE;
    return Q16_16_ZERO;
}

/**
 * @brief Left shift
 */
static inline q16_16_t q16_16_shl(q16_16_t a, int shift) {
    return Q16_16_WRAP(Q16_16_RAW(a) << shift);
}

/**
 * @brief Arithmetic right shift
 */
static inline q16_16_t q16_16_shr(q16_16_t a, int shift) {
    return Q16_16_WRAP(Q16_16_RAW(a) >> shift);
}

/*===========================================================================*/
/* Interpolation                                                             */
/*===========================================================================*/

/**
 * @brief Linear interpolation: a + t*(b-a)
 * @param a Start value
 * @param b End value
 * @param t Interpolation factor [0, 1]
 */
static inline q16_16_t q16_16_lerp(q16_16_t a, q16_16_t b, q16_16_t t) {
    q16_16_t diff = q16_16_sub(b, a);
    q16_16_t scaled = q16_16_mul(diff, t);
    return q16_16_add(a, scaled);
}

/*===========================================================================*/
/* Square Root (Newton-Raphson)                                              */
/*===========================================================================*/

/**
 * @brief Square root using Newton-Raphson iteration
 * @param x Non-negative Q16.16 value
 * @return sqrt(x) in Q16.16
 */
static inline q16_16_t q16_16_sqrt(q16_16_t x) {
    int32_t val = Q16_16_RAW(x);

    if (val <= 0) return Q16_16_ZERO;
    if (val == 0x10000) return Q16_16_ONE;  /* sqrt(1) = 1 */

    /* Initial guess: shift to approximate sqrt */
    uint32_t uval = (uint32_t)val;
    int leading_zeros = __builtin_clz(uval);
    int shift = (16 - leading_zeros) / 2;
    uint32_t guess = uval >> shift;

    /* Newton-Raphson: x_{n+1} = (x_n + val/x_n) / 2 */
    /* We need to work in Q16.16 properly */

    /* Convert to 64-bit for precision */
    uint64_t val64 = (uint64_t)val << 16;  /* Q32.16 */

    uint32_t root = guess;
    for (int i = 0; i < 8; i++) {
        if (root == 0) break;
        uint32_t quotient = (uint32_t)(val64 / root);
        root = (root + quotient) >> 1;
    }

    return Q16_16_WRAP((int32_t)root);
}

/**
 * @brief Reciprocal square root: 1/sqrt(x)
 */
static inline q16_16_t q16_16_rsqrt(q16_16_t x) {
    q16_16_t sq = q16_16_sqrt(x);
    if (Q16_16_RAW(sq) == 0) return Q16_16_MAX;
    return q16_16_div(Q16_16_ONE, sq);
}

/*===========================================================================*/
/* Trigonometry (CORDIC-based)                                               */
/*===========================================================================*/

/* CORDIC angle table (arctan(2^-i) in Q16.16 format) */
static const int32_t _q16_16_cordic_angles[16] = {
    0x0000C90F,  /* arctan(2^0)  = 0.7854 = pi/4 */
    0x000076B1,  /* arctan(2^-1) = 0.4636 */
    0x00003EB6,  /* arctan(2^-2) = 0.2449 */
    0x00001FD5,  /* arctan(2^-3) = 0.1244 */
    0x00000FFE,  /* arctan(2^-4) = 0.0624 */
    0x000007FF,  /* arctan(2^-5) = 0.0312 */
    0x00000400,  /* arctan(2^-6) = 0.0156 */
    0x00000200,  /* arctan(2^-7) = 0.0078 */
    0x00000100,  /* arctan(2^-8) */
    0x00000080,  /* arctan(2^-9) */
    0x00000040,  /* arctan(2^-10) */
    0x00000020,  /* arctan(2^-11) */
    0x00000010,  /* arctan(2^-12) */
    0x00000008,  /* arctan(2^-13) */
    0x00000004,  /* arctan(2^-14) */
    0x00000002,  /* arctan(2^-15) */
};

/* 1/K (CORDIC gain compensation) in Q16.16 ≈ 0.6072529... */
#define _Q16_16_CORDIC_K_INV  0x00009B74

/**
 * @brief Compute sin and cos simultaneously using CORDIC
 * @param angle Angle in radians (Q16.16)
 * @param sin_out Pointer to store sin result
 * @param cos_out Pointer to store cos result
 */
static inline void q16_16_sincos(q16_16_t angle, q16_16_t* sin_out, q16_16_t* cos_out) {
    int32_t z = Q16_16_RAW(angle);

    /* Range reduction to [-pi, pi] */
    const int32_t pi = 0x0003243F;
    const int32_t two_pi = 0x0006487F;

    while (z > pi) z -= two_pi;
    while (z < -pi) z += two_pi;

    /* Handle quadrants: reduce to [-pi/2, pi/2] */
    int negate_cos = 0;
    if (z > (pi >> 1)) {
        z = pi - z;
        negate_cos = 1;
    } else if (z < -(pi >> 1)) {
        z = -pi - z;
        negate_cos = 1;
    }

    /* Initialize with 1/K */
    int32_t x = _Q16_16_CORDIC_K_INV;
    int32_t y = 0;

    /* CORDIC iterations */
    for (int i = 0; i < 16; i++) {
        int32_t x_new, y_new;

        if (z >= 0) {
            x_new = x - (y >> i);
            y_new = y + (x >> i);
            z -= _q16_16_cordic_angles[i];
        } else {
            x_new = x + (y >> i);
            y_new = y - (x >> i);
            z += _q16_16_cordic_angles[i];
        }

        x = x_new;
        y = y_new;
    }

    if (negate_cos) x = -x;

    if (cos_out) *cos_out = Q16_16_WRAP(x);
    if (sin_out) *sin_out = Q16_16_WRAP(y);
}

/**
 * @brief Sine function
 * @param angle Angle in radians (Q16.16)
 * @return sin(angle) in Q16.16
 */
static inline q16_16_t q16_16_sin(q16_16_t angle) {
    q16_16_t s;
    q16_16_sincos(angle, &s, NULL);
    return s;
}

/**
 * @brief Cosine function
 * @param angle Angle in radians (Q16.16)
 * @return cos(angle) in Q16.16
 */
static inline q16_16_t q16_16_cos(q16_16_t angle) {
    q16_16_t c;
    q16_16_sincos(angle, NULL, &c);
    return c;
}

/**
 * @brief Tangent function
 * @param angle Angle in radians (Q16.16)
 * @return tan(angle) in Q16.16
 */
static inline q16_16_t q16_16_tan(q16_16_t angle) {
    q16_16_t s, c;
    q16_16_sincos(angle, &s, &c);
    return q16_16_div(s, c);
}

/**
 * @brief Arctangent of y/x (four-quadrant)
 * @param y Y coordinate
 * @param x X coordinate
 * @return atan2(y, x) in radians, range [-pi, pi]
 */
static inline q16_16_t q16_16_atan2(q16_16_t y, q16_16_t x) {
    int32_t xv = Q16_16_RAW(x);
    int32_t yv = Q16_16_RAW(y);

    if (xv == 0 && yv == 0) return Q16_16_ZERO;

    /* Handle special cases */
    const int32_t pi = 0x0003243F;
    const int32_t pi_2 = 0x0001921F;

    if (xv == 0) {
        return Q16_16_WRAP((yv > 0) ? pi_2 : -pi_2);
    }

    /* CORDIC vectoring mode */
    int32_t z = 0;

    /* Ensure x is positive (track sign for final adjustment) */
    int negate_result = 0;
    if (xv < 0) {
        xv = -xv;
        yv = -yv;
        negate_result = 1;
    }

    /* CORDIC iterations */
    for (int i = 0; i < 16; i++) {
        int32_t x_new, y_new;

        if (yv >= 0) {
            x_new = xv + (yv >> i);
            y_new = yv - (xv >> i);
            z += _q16_16_cordic_angles[i];
        } else {
            x_new = xv - (yv >> i);
            y_new = yv + (xv >> i);
            z -= _q16_16_cordic_angles[i];
        }

        xv = x_new;
        yv = y_new;
    }

    if (negate_result) {
        z = (Q16_16_RAW(y) >= 0) ? (pi - z) : (-pi - z);
    }

    return Q16_16_WRAP(z);
}

/**
 * @brief Arctangent
 * @param x Input value
 * @return atan(x) in radians
 */
static inline q16_16_t q16_16_atan(q16_16_t x) {
    return q16_16_atan2(x, Q16_16_ONE);
}

/*===========================================================================*/
/* Multiply-Accumulate                                                       */
/*===========================================================================*/

/**
 * @brief 64-bit accumulator for Q16.16 operations
 */
typedef int64_t q16_16_acc_t;

/**
 * @brief Initialize accumulator
 */
static inline q16_16_acc_t q16_16_acc_init(void) {
    return 0;
}

/**
 * @brief Accumulate a product: acc += a * b
 */
static inline q16_16_acc_t q16_16_acc_mac(q16_16_acc_t acc, q16_16_t a, q16_16_t b) {
    return acc + (int64_t)Q16_16_RAW(a) * (int64_t)Q16_16_RAW(b);
}

/**
 * @brief Extract Q16.16 from accumulator
 */
static inline q16_16_t q16_16_acc_to_q16_16(q16_16_acc_t acc) {
    int64_t result = (acc + 0x8000) >> 16;
    if (result > INT32_MAX) return Q16_16_MAX;
    if (result < INT32_MIN) return Q16_16_MIN;
    return Q16_16_WRAP((int32_t)result);
}

#ifdef __cplusplus
}

/* C++ namespace and operator overloads */
namespace fixp {

class Q16_16 {
public:
    q16_16_t value;

    Q16_16() : value(Q16_16_ZERO) {}
    explicit Q16_16(double d) : value(q16_16_from_double(d)) {}
    explicit Q16_16(float f) : value(q16_16_from_float(f)) {}
    explicit Q16_16(int32_t i) : value(q16_16_from_int(i)) {}
    Q16_16(q16_16_t q) : value(q) {}

    static Q16_16 from_raw(int32_t raw) { return Q16_16(Q16_16_WRAP(raw)); }
    int32_t raw() const { return Q16_16_RAW(value); }

    double to_double() const { return q16_16_to_double(value); }
    float to_float() const { return q16_16_to_float(value); }
    int32_t to_int() const { return q16_16_to_int(value); }

    Q16_16 operator+(Q16_16 other) const { return Q16_16(q16_16_add(value, other.value)); }
    Q16_16 operator-(Q16_16 other) const { return Q16_16(q16_16_sub(value, other.value)); }
    Q16_16 operator*(Q16_16 other) const { return Q16_16(q16_16_mul(value, other.value)); }
    Q16_16 operator/(Q16_16 other) const { return Q16_16(q16_16_div(value, other.value)); }
    Q16_16 operator-() const { return Q16_16(q16_16_neg(value)); }

    Q16_16& operator+=(Q16_16 other) { value = q16_16_add(value, other.value); return *this; }
    Q16_16& operator-=(Q16_16 other) { value = q16_16_sub(value, other.value); return *this; }
    Q16_16& operator*=(Q16_16 other) { value = q16_16_mul(value, other.value); return *this; }
    Q16_16& operator/=(Q16_16 other) { value = q16_16_div(value, other.value); return *this; }

    bool operator==(Q16_16 other) const { return q16_16_eq(value, other.value); }
    bool operator!=(Q16_16 other) const { return q16_16_ne(value, other.value); }
    bool operator<(Q16_16 other) const { return q16_16_lt(value, other.value); }
    bool operator<=(Q16_16 other) const { return q16_16_le(value, other.value); }
    bool operator>(Q16_16 other) const { return q16_16_gt(value, other.value); }
    bool operator>=(Q16_16 other) const { return q16_16_ge(value, other.value); }
};

inline Q16_16 sin(Q16_16 x) { return Q16_16(q16_16_sin(x.value)); }
inline Q16_16 cos(Q16_16 x) { return Q16_16(q16_16_cos(x.value)); }
inline Q16_16 tan(Q16_16 x) { return Q16_16(q16_16_tan(x.value)); }
inline Q16_16 sqrt(Q16_16 x) { return Q16_16(q16_16_sqrt(x.value)); }
inline Q16_16 abs(Q16_16 x) { return Q16_16(q16_16_abs(x.value)); }

} /* namespace fixp */

#endif /* __cplusplus */

#endif /* FIXP_Q16_16_H */
