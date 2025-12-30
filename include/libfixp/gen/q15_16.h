/**
 * @file q15_16.h
 * @brief Q15.16 Fixed-Point Implementation (Generated)
 *
 * Format: 1 sign bit, 15 integer bits, 16 fractional bits.
 * Underlying storage: int32_t (32-bit signed integer).
 */

#ifndef LIBFIXP_GEN_Q15_16_H
#define LIBFIXP_GEN_Q15_16_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t q15_16_t;

#ifdef __cplusplus
#define Q15_16_WRAP(raw) (q15_16_t{(raw)})
#else
#define Q15_16_WRAP(raw) ((q15_16_t)(raw))
#endif

#define Q15_16_RAW(x) (x)

#define Q15_16_FRAC_BITS 16
#define Q15_16_ONE Q15_16_WRAP(65536)
#define Q15_16_MAX Q15_16_WRAP(INT32_MAX)
#define Q15_16_MIN Q15_16_WRAP(INT32_MIN)

static inline q15_16_t q15_16_add(q15_16_t a, q15_16_t b) {
    return Q15_16_WRAP((int32_t)((uint32_t)a + (uint32_t)b));
}

static inline q15_16_t q15_16_sub(q15_16_t a, q15_16_t b) {
    return Q15_16_WRAP((int32_t)((uint32_t)a - (uint32_t)b));
}

static inline q15_16_t q15_16_mul(q15_16_t a, q15_16_t b) {
    int64_t prod = (int64_t)a * (int64_t)b;
    return Q15_16_WRAP((int32_t)((prod + (1 << (16-1))) >> 16));
}

static inline q15_16_t q15_16_div(q15_16_t a, q15_16_t b) {
    if (b == 0) return (a >= 0) ? Q15_16_MAX : Q15_16_MIN;
    int64_t dividend = (int64_t)a << 16;
    return Q15_16_WRAP((int32_t)(dividend / b));
}

static inline q15_16_t q15_16_from_double(double d) {
    return Q15_16_WRAP((int32_t)(d * 65536.0 + (d >= 0 ? 0.5 : -0.5)));
}

static inline double q15_16_to_double(q15_16_t a) {
    return (double)a / 65536.0;
}

#ifdef __cplusplus
}
#endif

#endif // LIBFIXP_GEN_Q15_16_H
