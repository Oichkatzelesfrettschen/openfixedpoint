#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "fixp_q7.h"

int main(void) {
    // Test conversion
    q7_t a = q7_from_double(0.5);
    assert(a == 64);
    assert(fabs(q7_to_double(a) - 0.5) < 0.01);

    q7_t b = q7_from_double(-0.5);
    assert(b == -64);
    assert(fabs(q7_to_double(b) + 0.5) < 0.01);

    // Test arithmetic
    q7_t sum = q7_add(a, a); // 0.5 + 0.5 = 1.0 -> Saturates to max or wraps?
    // q7_add is wrapping. 64 + 64 = 128 -> -128 (overflow)
    // Wait, 128 in int8_t is -128.
    // 0.5 is 64 (0x40).
    // 0x40 + 0x40 = 0x80 (-128).
    // So wrapping addition of 0.5 + 0.5 is -1.0. Correct behavior for wrapping.
    assert(sum == (q7_t)-128);

    // Test saturating
    q7_t sat_sum = q7_add_sat(a, a); // 0.5 + 0.5 = 1.0 -> Saturates to MAX (0.99...)
    assert(sat_sum == Q7_MAX);

    printf("Q7 C Tests Passed\n");
    return 0;
}
