/**
 * @file q16_16.h
 * @brief Q16.16 Fixed-Point Implementation (Generated)
 *
 * Logical format: 1 sign bit, 16 integer bits, 16 fractional bits (Q16.16).
 * Underlying storage: int64_t (64-bit signed integer holding the logical value).
 */

#ifndef FIXP_GEN_Q16_16_H
#define FIXP_GEN_Q16_16_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t q16_16_t;

#ifdef __cplusplus
#define Q16_16_WRAP(raw) (q16_16_t{(raw)})
#else
#define Q16_16_WRAP(raw) ((q16_16_t)(raw))
#endif

#define Q16_16_RAW(x) (x)

#define Q16_16_FRAC_BITS 16
#define Q16_16_ONE Q16_16_WRAP(65536)
#define Q16_16_MAX Q16_16_WRAP(INT64_MAX)
#define Q16_16_MIN Q16_16_WRAP(INT64_MIN)

static inline q16_16_t q16_16_add(q16_16_t a, q16_16_t b) {
    // Use unsigned arithmetic to avoid signed overflow UB
    return Q16_16_WRAP((int64_t)((uint64_t)a + (uint64_t)b));
}

static inline q16_16_t q16_16_sub(q16_16_t a, q16_16_t b) {
    return Q16_16_WRAP((int64_t)((uint64_t)a - (uint64_t)b));
}

static inline q16_16_t q16_16_mul(q16_16_t a, q16_16_t b) {
    __int128_t prod = (__int128_t)a * (__int128_t)b;
    return Q16_16_WRAP((int64_t)((prod + (1 << (16-1))) >> 16));
}

static inline q16_16_t q16_16_div(q16_16_t a, q16_16_t b) {
    if (b == 0) return (a >= 0) ? Q16_16_MAX : Q16_16_MIN;
    __int128_t dividend = (__int128_t)a << 16;
    return Q16_16_WRAP((int64_t)(dividend / b));
}

static inline q16_16_t q16_16_from_double(double d) {
    return Q16_16_WRAP((int64_t)(d * 65536.0 + (d >= 0 ? 0.5 : -0.5)));
}

static inline double q16_16_to_double(q16_16_t a) {
    return (double)a / 65536.0;
}

#ifdef __cplusplus
}
#endif

#endif // FIXP_GEN_Q16_16_H
