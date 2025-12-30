/**
 * @file q8_8.h
 * @brief Q8.8 Fixed-Point Implementation (Generated)
 *
 * Format: 1 sign bit, 8 integer bits, 8 fractional bits.
 * Underlying storage: int32_t (32-bit).
 */

#ifndef FIXP_GEN_Q8_8_H
#define FIXP_GEN_Q8_8_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t q8_8_t;

#ifdef __cplusplus
#define Q8_8_WRAP(raw) (q8_8_t{(raw)})
#else
#define Q8_8_WRAP(raw) ((q8_8_t)(raw))
#endif

#define Q8_8_RAW(x) (x)

#define Q8_8_FRAC_BITS 8
#define Q8_8_ONE Q8_8_WRAP(256)
#define Q8_8_MAX Q8_8_WRAP(INT32_MAX)
#define Q8_8_MIN Q8_8_WRAP(INT32_MIN)

static inline q8_8_t q8_8_add(q8_8_t a, q8_8_t b) {
    return Q8_8_WRAP(a + b);
}

static inline q8_8_t q8_8_sub(q8_8_t a, q8_8_t b) {
    return Q8_8_WRAP(a - b);
}

static inline q8_8_t q8_8_mul(q8_8_t a, q8_8_t b) {
    int64_t prod = (int64_t)a * (int64_t)b;
    return Q8_8_WRAP((int32_t)((prod + (1 << (8-1))) >> 8));
}

static inline q8_8_t q8_8_div(q8_8_t a, q8_8_t b) {
    if (b == 0) return (a >= 0) ? Q8_8_MAX : Q8_8_MIN;
    int64_t dividend = (int64_t)a << 8;
    return Q8_8_WRAP((int32_t)(dividend / b));
}

static inline q8_8_t q8_8_from_double(double d) {
    return Q8_8_WRAP((int32_t)(d * 256.0 + (d >= 0 ? 0.5 : -0.5)));
}

static inline double q8_8_to_double(q8_8_t a) {
    return (double)a / 256.0;
}

#ifdef __cplusplus
}
#endif

#endif // FIXP_GEN_Q8_8_H
