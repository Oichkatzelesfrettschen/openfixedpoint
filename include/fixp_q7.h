#ifndef FIXP_Q7_H
#define FIXP_Q7_H

#ifdef __cplusplus
    #include "fixp/fixed_point.hpp"
    #include "fixp/gen/q0_7.h"
#else
    #include "fixp/gen/q0_7.h"
#endif

// Map legacy names to generated names
// Legacy: q7_t (int8_t)
// Generated: q0_7_t (int8_t)

typedef q0_7_t q7_t;

// Constants
#define Q7_MAX Q0_7_MAX
#define Q7_MIN Q0_7_MIN
#define Q7_ONE Q0_7_ONE

#ifdef __cplusplus
extern "C" {
#endif

// Legacy functions
static inline q7_t q7_add(q7_t a, q7_t b) { return q0_7_add(a, b); }
static inline q7_t q7_sub(q7_t a, q7_t b) { return q0_7_sub(a, b); }
static inline q7_t q7_mul(q7_t a, q7_t b) { return q0_7_mul(a, b); }
// q0_7_div exists

// Add back saturation functions (not in generator yet)
static inline q7_t q7_add_sat(q7_t a, q7_t b) {
    int16_t sum = (int16_t)a + (int16_t)b;
    if (sum > 127) return Q7_MAX;
    if (sum < -128) return Q7_MIN;
    return (q7_t)sum;
}

static inline q7_t q7_from_double(double d) { return q0_7_from_double(d); }
static inline double q7_to_double(q7_t a) { return q0_7_to_double(a); }

// Extras required by tests
static inline q7_t q7_abs(q7_t a) {
    if (a == Q7_MIN) return Q7_MAX;
    return (q7_t)((a < 0) ? -a : a);
}

#ifdef __cplusplus
}
#endif

#endif
