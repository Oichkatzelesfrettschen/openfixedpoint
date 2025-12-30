#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "fixp_q16_16.h"

// Simple C test runner
int main(void) {
    // Test construction
    q16_16_t a = q16_16_from_double(1.0);
    assert(Q16_16_RAW(a) == 0x10000);
    assert(fabs(q16_16_to_double(a) - 1.0) < 0.0001);

    q16_16_t b = q16_16_from_double(2.0);
    q16_16_t sum = q16_16_add(a, b);
    assert(fabs(q16_16_to_double(sum) - 3.0) < 0.0001);

    q16_16_t prod = q16_16_mul(a, b);
    assert(fabs(q16_16_to_double(prod) - 2.0) < 0.0001);

    printf("Q16.16 C Tests Passed\n");
    return 0;
}
