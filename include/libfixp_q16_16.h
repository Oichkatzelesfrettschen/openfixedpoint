#ifndef LIBFIXP_Q16_16_H
#define LIBFIXP_Q16_16_H

// Map "Q16.16" to the Q15.16 implementation (32-bit)
// which corresponds to the standard Q16.16 usage (16-bit fractional).
// Q15.16 has 1 sign, 15 integer, 16 fractional = 32 bits.

#ifdef __cplusplus
    #include "libfixp/fixed_point.hpp"
    #include "libfixp/gen/q15_16.h"
#else
    #include "libfixp/gen/q15_16.h"
#endif

// Typedef map
typedef q15_16_t q16_16_t;

// Macro map
#define Q16_16_WRAP(raw) Q15_16_WRAP(raw)
#define Q16_16_RAW(x)    Q15_16_RAW(x)
#define Q16_16_ONE       Q15_16_ONE
#define Q16_16_MAX       Q15_16_MAX
#define Q16_16_MIN       Q15_16_MIN
#define Q16_16_ZERO      Q16_16_WRAP(0)

#ifdef __cplusplus
extern "C" {
#endif

// Constants not in generator
#define Q16_16_EPSILON    Q16_16_WRAP(0x00000001)
#define Q16_16_PI         Q16_16_WRAP(0x0003243F)
#define Q16_16_PI_2       Q16_16_WRAP(0x0001921F)
#define Q16_16_PI_4       Q16_16_WRAP(0x0000C90F)
#define Q16_16_TWO_PI     Q16_16_WRAP(0x0006487F)
#define Q16_16_E          Q16_16_WRAP(0x0002B7E1)
#define Q16_16_SQRT2      Q16_16_WRAP(0x00016A09)
#define Q16_16_SQRT2_2    Q16_16_WRAP(0x0000B504)

// Legacy functions mapped to Q15.16
static inline q16_16_t q16_16_add(q16_16_t a, q16_16_t b) { return q15_16_add(a, b); }
static inline q16_16_t q16_16_sub(q16_16_t a, q16_16_t b) { return q15_16_sub(a, b); }
static inline q16_16_t q16_16_mul(q16_16_t a, q16_16_t b) { return q15_16_mul(a, b); }
static inline q16_16_t q16_16_div(q16_16_t a, q16_16_t b) { return q15_16_div(a, b); }
static inline q16_16_t q16_16_from_double(double d) { return q15_16_from_double(d); }
static inline double q16_16_to_double(q16_16_t a) { return q15_16_to_double(a); }

static inline q16_16_t q16_16_from_int(int32_t i) {
    return Q16_16_WRAP(i << 16);
}

static inline int32_t q16_16_to_int(q16_16_t q) {
    return Q16_16_RAW(q) >> 16;
}

// Saturating Arithmetic (Legacy)
static inline q16_16_t q16_16_add_sat(q16_16_t a, q16_16_t b) {
    int32_t res;
    if (__builtin_add_overflow(Q16_16_RAW(a), Q16_16_RAW(b), &res)) {
        return (Q16_16_RAW(a) > 0) ? Q16_16_MAX : Q16_16_MIN;
    }
    return Q16_16_WRAP(res);
}

static inline q16_16_t q16_16_sub_sat(q16_16_t a, q16_16_t b) {
    int32_t res;
    int32_t ra = Q16_16_RAW(a);
    int32_t rb = Q16_16_RAW(b);
    if (__builtin_sub_overflow(ra, rb, &res)) {
        /* For a - b:
         * - If a >= 0 and b < 0, the result overflows positively -> clamp to MAX.
         * - If a < 0 and b > 0, the result overflows negatively -> clamp to MIN.
         * In all other (unexpected) overflow cases, saturate toward the sign of a - b,
         * which is consistent with these two primary cases.
         */
        if (ra >= 0 && rb < 0) {
            return Q16_16_MAX;
        } else {
            return Q16_16_MIN;
        }
    }
    return Q16_16_WRAP(res);
}

static inline q16_16_t q16_16_mul_sat(q16_16_t a, q16_16_t b) {
    int64_t product = (int64_t)Q16_16_RAW(a) * (int64_t)Q16_16_RAW(b);
    int64_t result = (product + 0x8000) >> 16;
    if (result > 2147483647) return Q16_16_MAX;
    if (result < -2147483648LL) return Q16_16_MIN;
    return Q16_16_WRAP((int32_t)result);
}

static inline q16_16_t q16_16_neg_sat(q16_16_t a) {
    if (Q16_16_RAW(a) == (int32_t)0x80000000u) return Q16_16_MAX;
    return Q16_16_WRAP(-Q16_16_RAW(a));
}

static inline q16_16_t q16_16_floor(q16_16_t q) {
    return Q16_16_WRAP(Q16_16_RAW(q) & 0xFFFF0000);
}

// CORDIC and Math implementation
static inline q16_16_t q16_16_sqrt(q16_16_t x) {
    int32_t val = Q16_16_RAW(x);
    if (val <= 0) return Q16_16_ZERO;
    if (val == 0x10000) return Q16_16_ONE;

    uint32_t uval = (uint32_t)val;
    int leading_zeros = __builtin_clz(uval);
    int shift = (16 - leading_zeros) / 2;
    uint32_t guess = uval >> shift;

    uint64_t val64 = (uint64_t)val << 16;
    uint32_t root = guess;
    for (int i = 0; i < 8; i++) {
        if (root == 0) break;
        uint32_t quotient = (uint32_t)(val64 / root);
        root = (root + quotient) >> 1;
    }
    return Q16_16_WRAP((int32_t)root);
}

static const int32_t _q16_16_cordic_angles[16] = {
    0x0000C90F, 0x000076B1, 0x00003EB6, 0x00001FD5,
    0x00000FFE, 0x000007FF, 0x00000400, 0x00000200,
    0x00000100, 0x00000080, 0x00000040, 0x00000020,
    0x00000010, 0x00000008, 0x00000004, 0x00000002
};

#define _Q16_16_CORDIC_K_INV  0x00009B74

static inline void q16_16_sincos(q16_16_t angle, q16_16_t* sin_out, q16_16_t* cos_out) {
    int32_t z = Q16_16_RAW(angle);
    const int32_t pi = 0x0003243F;
    const int32_t two_pi = 0x0006487F;

    while (z > pi) z -= two_pi;
    while (z < -pi) z += two_pi;

    int negate_cos = 0;
    if (z > (pi >> 1)) {
        z = pi - z;
        negate_cos = 1;
    } else if (z < -(pi >> 1)) {
        z = -pi - z;
        negate_cos = 1;
    }

    int32_t x = _Q16_16_CORDIC_K_INV;
    int32_t y = 0;

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

static inline q16_16_t q16_16_sin(q16_16_t angle) {
    q16_16_t s;
    q16_16_sincos(angle, &s, NULL);
    return s;
}

static inline q16_16_t q16_16_cos(q16_16_t angle) {
    q16_16_t c;
    q16_16_sincos(angle, NULL, &c);
    return c;
}

static inline q16_16_t q16_16_tan(q16_16_t angle) {
    q16_16_t s, c;
    q16_16_sincos(angle, &s, &c);
    return q16_16_div(s, c);
}

#ifdef __cplusplus
}

namespace libfixp {
    inline Q16_16 sin(Q16_16 x) { return Q16_16::from_raw(q16_16_sin(x.raw())); }
    inline Q16_16 cos(Q16_16 x) { return Q16_16::from_raw(q16_16_cos(x.raw())); }
    inline Q16_16 tan(Q16_16 x) { return Q16_16::from_raw(q16_16_tan(x.raw())); }
    inline Q16_16 sqrt(Q16_16 x) { return Q16_16::from_raw(q16_16_sqrt(x.raw())); }
}
#endif

#endif // LIBFIXP_Q16_16_H
