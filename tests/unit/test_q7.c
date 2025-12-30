#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "libfixp_q7.h"

int main(void) {
    // Test conversion
    q7_t a = q7_from_double(0.5);
    assert(a == 64);
    assert(fabs(q7_to_double(a) - 0.5) < 0.01);

    q7_t b = q7_from_double(-0.5);
    assert(b == -64);
    assert(fabs(q7_to_double(b) + 0.5) < 0.01);

    // ========================================================================
    // Test wrapping addition (intentional overflow behavior)
    // ========================================================================
    // q7_add uses wrapping arithmetic, which means overflow wraps around.
    // This is intentional behavior, not a bug.
    // Example: 0.5 + 0.5 = 1.0, but 1.0 cannot be represented in Q7 format.
    // The raw calculation: 64 + 64 = 128 (0x80), which wraps to -128 in int8_t.
    // Result: 0.5 + 0.5 wraps to -1.0 in Q7 format.
    q7_t sum = q7_add(a, a);
    assert(sum == (q7_t)-128); // Intentional wrap-around from overflow

    // ========================================================================
    // Test saturating addition (overflow clamped to maximum)
    // ========================================================================
    // q7_add_sat uses saturating arithmetic, which clamps overflow to the max value.
    // Example: 0.5 + 0.5 = 1.0, which saturates to Q7_MAX (127/128 ≈ 0.9921875).
    q7_t sat_sum = q7_add_sat(a, a);
    assert(sat_sum == Q7_MAX); // Saturates to maximum value instead of wrapping

    printf("Q7 C Tests Passed\n");
    return 0;
}
