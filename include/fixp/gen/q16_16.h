/**
 * @file q16_16.h
 * @brief Fixed-point implementation with 16 fractional bits (Generated)
 *
 * Logical format: 1 sign bit, 16 integer bits, 16 fractional bits (33 bits total),
 * stored in a 64-bit integer type (int64_t) to provide extra headroom.
 *
 * Note: In some literature, "Q16.16" refers to a 32-bit format with 16 fractional bits
 * (1 sign bit + 15 integer bits + 16 fractional bits, often described as Q15.16).
 * This header instead uses the name q16_16_t for the above 64-bit representation
 * with 16 fractional bits.
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
