/**
 * @file q0_7.h
 * @brief Q0.7 Fixed-Point Implementation (Generated)
 *
 * Format: 1 sign bit, 0 integer bits, 7 fractional bits.
 * Underlying storage: int8_t (8-bit).
 */

#ifndef FIXP_GEN_Q0_7_H
#define FIXP_GEN_Q0_7_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t q0_7_t;

#ifdef __cplusplus
#define Q0_7_WRAP(raw) (q0_7_t{(raw)})
#else
#define Q0_7_WRAP(raw) ((q0_7_t)(raw))
#endif

#define Q0_7_RAW(x) (x)

#define Q0_7_FRAC_BITS 7
#define Q0_7_ONE Q0_7_WRAP(INT8_MAX)
#define Q0_7_MAX Q0_7_WRAP(INT8_MAX)
#define Q0_7_MIN Q0_7_WRAP(INT8_MIN)

static inline q0_7_t q0_7_add(q0_7_t a, q0_7_t b) {
    // Use unsigned arithmetic to avoid signed overflow UB
    return Q0_7_WRAP((int8_t)((uint8_t)a + (uint8_t)b));
}

static inline q0_7_t q0_7_sub(q0_7_t a, q0_7_t b) {
    return Q0_7_WRAP((int8_t)((uint8_t)a - (uint8_t)b));
}

static inline q0_7_t q0_7_mul(q0_7_t a, q0_7_t b) {
    int16_t prod = (int16_t)a * (int16_t)b;
    return Q0_7_WRAP((int8_t)((prod + (1 << (7-1))) >> 7));
}

static inline q0_7_t q0_7_div(q0_7_t a, q0_7_t b) {
    if (b == 0) return (a >= 0) ? Q0_7_MAX : Q0_7_MIN;
    int16_t dividend = (int16_t)a << 7;
    return Q0_7_WRAP((int8_t)(dividend / b));
}

static inline q0_7_t q0_7_from_double(double d) {
    return Q0_7_WRAP((int8_t)(d * 128.0 + (d >= 0 ? 0.5 : -0.5)));
}

static inline double q0_7_to_double(q0_7_t a) {
    return (double)a / 128.0;
}

#ifdef __cplusplus
}
#endif

#endif // FIXP_GEN_Q0_7_H
