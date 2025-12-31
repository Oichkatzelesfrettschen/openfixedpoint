#define _USE_MATH_DEFINES
#include <fixp/gen/q15_16_math.h>
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

#define TEST_TOLERANCE 0.02

void test_function(const char* name, double result, double expected) {
    double error = fabs(result - expected);
    int passed = error <= TEST_TOLERANCE;
    
    printf("%-20s: result=%10.6f, expected=%10.6f, error=%10.6f [%s]\n",
           name, result, expected, error, passed ? "PASS" : "FAIL");
}

int main(void) {
    printf("Testing C23 Fixed-Point Math Functions\n");
    printf("======================================\n\n");
    
    // Test constants
    printf("Constants:\n");
    test_function("pi", (double)Q15_16_PI / (1 << 16), M_PI);
    test_function("e", (double)Q15_16_E / (1 << 16), M_E);
    printf("\n");
    
    // Test basic operations
    printf("Basic Operations:\n");
    q15_16_t val = q15_16_abs(-(5 << 16));
    test_function("abs(-5)", (double)val / (1 << 16), 5.0);
    
    val = q15_16_min(3 << 16, 5 << 16);
    test_function("min(3,5)", (double)val / (1 << 16), 3.0);
    
    val = q15_16_max(3 << 16, 5 << 16);
    test_function("max(3,5)", (double)val / (1 << 16), 5.0);
    printf("\n");
    
    // Test rounding
    printf("Rounding:\n");
    val = q15_16_floor((int32_t)(2.7 * (1 << 16)));
    test_function("floor(2.7)", (double)val / (1 << 16), 2.0);
    
    val = q15_16_ceil((int32_t)(2.3 * (1 << 16)));
    test_function("ceil(2.3)", (double)val / (1 << 16), 3.0);
    
    val = q15_16_round((int32_t)(2.5 * (1 << 16)));
    test_function("round(2.5)", (double)val / (1 << 16), 3.0);
    printf("\n");
    
    // Test sqrt
    printf("Square Root:\n");
    val = q15_16_sqrt(4 << 16);
    test_function("sqrt(4)", (double)val / (1 << 16), 2.0);
    
    val = q15_16_sqrt(9 << 16);
    test_function("sqrt(9)", (double)val / (1 << 16), 3.0);
    
    val = q15_16_sqrt(2 << 16);
    test_function("sqrt(2)", (double)val / (1 << 16), sqrt(2.0));
    printf("\n");
    
    // Test trigonometry
    printf("Trigonometry:\n");
    val = q15_16_sin(0);
    test_function("sin(0)", (double)val / (1 << 16), 0.0);
    
    val = q15_16_sin(Q15_16_PI / 2);
    test_function("sin(pi/2)", (double)val / (1 << 16), 1.0);
    
    val = q15_16_cos(0);
    test_function("cos(0)", (double)val / (1 << 16), 1.0);
    
    val = q15_16_cos(Q15_16_PI / 2);
    test_function("cos(pi/2)", (double)val / (1 << 16), 0.0);
    printf("\n");
    
    // Test atan
    printf("Inverse Trigonometry:\n");
    val = q15_16_atan(Q15_16_ONE);
    test_function("atan(1)", (double)val / (1 << 16), M_PI/4);
    
    val = q15_16_atan2(Q15_16_ONE, Q15_16_ONE);
    test_function("atan2(1,1)", (double)val / (1 << 16), M_PI/4);
    printf("\n");
    
    // Test exponential
    printf("Exponential:\n");
    val = q15_16_exp(0);
    test_function("exp(0)", (double)val / (1 << 16), 1.0);
    
    val = q15_16_log(Q15_16_ONE);
    test_function("log(1)", (double)val / (1 << 16), 0.0);
    
    val = q15_16_pow(2 << 16, 3 << 16);
    test_function("pow(2,3)", (double)val / (1 << 16), 8.0);
    printf("\n");
    
    printf("All tests completed!\n");
    
    return 0;
}
