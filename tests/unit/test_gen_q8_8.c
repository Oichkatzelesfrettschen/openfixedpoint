#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "fixp/gen/q8_8.h"

int main(void) {
    // Q8.8 range: -128.0 to 127.996

    // Test conversion
    q8_8_t a = q8_8_from_double(10.5);
    // 10.5 * 256 = 2688
    assert(Q8_8_RAW(a) == 2688);
    assert(fabs(q8_8_to_double(a) - 10.5) < 0.01);

    // Test Addition
    q8_8_t b = q8_8_from_double(2.25);
    q8_8_t sum = q8_8_add(a, b);
    // 12.75
    assert(fabs(q8_8_to_double(sum) - 12.75) < 0.01);

    // Test Multiplication
    q8_8_t prod = q8_8_mul(a, b);
    // 10.5 * 2.25 = 23.625
    assert(fabs(q8_8_to_double(prod) - 23.625) < 0.01);

    printf("Q8.8 Generated Tests Passed\n");
    return 0;
}
