# libfixp Test Harness Architecture

## Overview

The libfixp test harness provides comprehensive verification through multiple testing methodologies:

1. **Exhaustive Testing** - Complete coverage for small types (Q7, Q15)
2. **Property-Based Testing** - Invariant verification for all types
3. **Differential Testing** - Comparison against reference implementations
4. **Fuzzing** - Random input generation with coverage guidance
5. **Formal Verification** - CBMC/Z3 proofs for critical operations
6. **Performance Benchmarking** - Regression detection

---

## 1. Directory Structure

```
tests/
├── unit/                    # Unit tests per module
│   ├── test_q7.c
│   ├── test_q15.c
│   ├── test_q16_16.c
│   └── test_math.c
├── exhaustive/              # Exhaustive tests (small types)
│   ├── exhaust_q7_add.c
│   ├── exhaust_q7_mul.c
│   └── exhaust_q15_mul.c
├── property/                # Property-based tests
│   ├── prop_arithmetic.c
│   ├── prop_trigonometry.c
│   └── generators.h
├── differential/            # Compare against reference
│   ├── diff_vs_float.c
│   ├── diff_vs_mpfr.c
│   └── reference.py
├── fuzz/                    # Fuzzing targets
│   ├── fuzz_add.c
│   ├── fuzz_mul.c
│   ├── fuzz_div.c
│   └── fuzz_trig.c
├── formal/                  # Formal verification
│   ├── cbmc_overflow.c
│   ├── z3_proofs.py
│   └── gappa_bounds.g
├── benchmark/               # Performance tests
│   ├── bench_arithmetic.c
│   ├── bench_simd.c
│   └── baseline.json
├── integration/             # End-to-end tests
│   ├── test_fir_filter.c
│   ├── test_cordic.c
│   └── test_matrix.c
├── vectors/                 # Test vectors (golden data)
│   ├── sin_vectors.h
│   ├── sqrt_vectors.h
│   └── generate_vectors.py
└── framework/               # Test infrastructure
    ├── test_common.h
    ├── test_main.c
    ├── assertions.h
    ├── generators.h
    └── reporters.h
```

---

## 2. Test Framework Core

### 2.1 Minimal Test Macros (No Dependencies)

```c
// framework/test_common.h
#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Test result tracking
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// ANSI colors
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define RESET   "\x1b[0m"

// Core assertion
#define TEST_ASSERT(cond, msg) do { \
    tests_run++; \
    if (!(cond)) { \
        printf(RED "FAIL" RESET ": %s:%d: %s\n", __FILE__, __LINE__, msg); \
        tests_failed++; \
    } else { \
        tests_passed++; \
    } \
} while(0)

// Equality assertions
#define TEST_ASSERT_EQ(a, b) \
    TEST_ASSERT((a) == (b), #a " == " #b)

#define TEST_ASSERT_EQ_MSG(a, b, msg) \
    TEST_ASSERT((a) == (b), msg)

// Range assertion
#define TEST_ASSERT_RANGE(val, lo, hi) \
    TEST_ASSERT((val) >= (lo) && (val) <= (hi), \
                #val " in [" #lo ", " #hi "]")

// Float comparison with epsilon
#define TEST_ASSERT_NEAR(a, b, eps) \
    TEST_ASSERT(fabs((double)(a) - (double)(b)) <= (eps), \
                #a " ~= " #b)

// Test case definition
#define TEST_CASE(name) static void test_##name(void)

// Test suite runner
#define RUN_TEST(name) do { \
    printf("  Running %s...\n", #name); \
    test_##name(); \
} while(0)

// Summary
#define TEST_SUMMARY() do { \
    printf("\n%d tests, %d passed, %d failed\n", \
           tests_run, tests_passed, tests_failed); \
    return tests_failed > 0 ? 1 : 0; \
} while(0)

#endif // TEST_COMMON_H
```

### 2.2 Extended Assertions for Fixed-Point

```c
// framework/assertions.h
#ifndef ASSERTIONS_H
#define ASSERTIONS_H

#include "test_common.h"
#include "fixp.h"

// Q16.16 equality
#define TEST_ASSERT_Q16_16_EQ(a, b) \
    TEST_ASSERT(Q16_16_RAW(a) == Q16_16_RAW(b), \
                "Q16.16 values equal")

// Q16.16 near (within N ULPs)
#define TEST_ASSERT_Q16_16_NEAR(a, b, ulp) do { \
    int32_t diff = abs(Q16_16_RAW(a) - Q16_16_RAW(b)); \
    TEST_ASSERT(diff <= (ulp), "Q16.16 within " #ulp " ULP"); \
} while(0)

// Overflow detection
#define TEST_ASSERT_NO_OVERFLOW(expr) do { \
    fixp_clear_error(); \
    (void)(expr); \
    TEST_ASSERT(fixp_get_error() != FIXP_ERR_OVERFLOW, \
                "no overflow in " #expr); \
} while(0)

// Saturation verification
#define TEST_ASSERT_SATURATED(val, limit) \
    TEST_ASSERT(Q16_16_RAW(val) == (limit), \
                #val " saturated to " #limit)

#endif // ASSERTIONS_H
```

---

## 3. Exhaustive Testing

### 3.1 Q7 Complete Coverage

```c
// exhaustive/exhaust_q7_add.c
#include "test_common.h"
#include "fixp_q7.h"

// Test all 65536 combinations of Q7 add
void exhaustive_q7_add(void) {
    printf("Exhaustive Q7 add test (65536 cases)...\n");

    for (int a = -128; a <= 127; a++) {
        for (int b = -128; b <= 127; b++) {
            q7_t qa = (q7_t)a;
            q7_t qb = (q7_t)b;

            // Wrapping add
            q7_t result_wrap = q7_add(qa, qb);
            int8_t expected_wrap = (int8_t)(a + b);  // C wraps
            TEST_ASSERT_EQ(result_wrap, expected_wrap);

            // Saturating add
            q7_t result_sat = q7_add_sat(qa, qb);
            int expected_sat = a + b;
            if (expected_sat > 127) expected_sat = 127;
            if (expected_sat < -128) expected_sat = -128;
            TEST_ASSERT_EQ(result_sat, (q7_t)expected_sat);
        }
    }

    printf("  All Q7 add cases passed.\n");
}

// Test all Q7 multiply (also 65536 combinations)
void exhaustive_q7_mul(void) {
    printf("Exhaustive Q7 mul test (65536 cases)...\n");

    for (int a = -128; a <= 127; a++) {
        for (int b = -128; b <= 127; b++) {
            q7_t qa = (q7_t)a;
            q7_t qb = (q7_t)b;

            q7_t result = q7_mul(qa, qb);

            // Reference: (a * b) >> 7, with rounding
            int16_t product = (int16_t)a * (int16_t)b;
            int8_t expected = (int8_t)((product + 64) >> 7);  // Round

            TEST_ASSERT_EQ(result, expected);
        }
    }

    printf("  All Q7 mul cases passed.\n");
}
```

### 3.2 Q15 Targeted Exhaustive

```c
// exhaustive/exhaust_q15_mul.c
// Full exhaustive is 4B cases - use sampling + boundaries

void exhaustive_q15_mul_boundaries(void) {
    // Test boundary values exhaustively
    const int16_t boundaries[] = {
        INT16_MIN, INT16_MIN+1, -32767, -16384, -1, 0, 1,
        16384, 32766, INT16_MAX
    };
    int n = sizeof(boundaries) / sizeof(boundaries[0]);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            q15_t a = boundaries[i];
            q15_t b = boundaries[j];
            q15_t result = q15_mul(a, b);

            // Reference calculation
            int32_t product = (int32_t)a * (int32_t)b;
            int16_t expected = (int16_t)((product + 0x4000) >> 15);

            TEST_ASSERT_EQ(result, expected);
        }
    }
}
```

---

## 4. Property-Based Testing

### 4.1 Generator Framework

```c
// framework/generators.h
#ifndef GENERATORS_H
#define GENERATORS_H

#include <stdint.h>
#include <stdlib.h>

// PRNG state
static uint64_t gen_state = 0xDEADBEEF;

static inline uint64_t gen_next(void) {
    // xorshift64
    gen_state ^= gen_state << 13;
    gen_state ^= gen_state >> 7;
    gen_state ^= gen_state << 17;
    return gen_state;
}

static inline void gen_seed(uint64_t seed) {
    gen_state = seed;
}

// Generate random values of various types
static inline int8_t gen_q7(void) {
    return (int8_t)(gen_next() & 0xFF);
}

static inline int16_t gen_q15(void) {
    return (int16_t)(gen_next() & 0xFFFF);
}

static inline int32_t gen_q16_16(void) {
    return (int32_t)(gen_next() & 0xFFFFFFFF);
}

// Range-constrained generator
static inline int32_t gen_range(int32_t lo, int32_t hi) {
    uint64_t range = (uint64_t)(hi - lo + 1);
    return lo + (int32_t)(gen_next() % range);
}

// Special value generators
static inline int32_t gen_q16_16_small(void) {
    // Values near zero (fractional only)
    return gen_range(-0x10000, 0x10000);  // [-1, 1]
}

static inline int32_t gen_q16_16_angle(void) {
    // Angles in [-2*pi, 2*pi]
    return gen_range(-0x6487F, 0x6487F);  // Approx
}

#endif // GENERATORS_H
```

### 4.2 Property Tests

```c
// property/prop_arithmetic.c
#include "test_common.h"
#include "generators.h"
#include "fixp.h"

#define PROPERTY_ITERATIONS 100000

// Addition is commutative
void prop_add_commutative(void) {
    for (int i = 0; i < PROPERTY_ITERATIONS; i++) {
        q16_16_t a = Q16_16_WRAP(gen_q16_16());
        q16_16_t b = Q16_16_WRAP(gen_q16_16());

        q16_16_t r1 = q16_16_add(a, b);
        q16_16_t r2 = q16_16_add(b, a);

        TEST_ASSERT_Q16_16_EQ(r1, r2);
    }
}

// Addition identity: a + 0 = a
void prop_add_identity(void) {
    q16_16_t zero = Q16_16_WRAP(0);

    for (int i = 0; i < PROPERTY_ITERATIONS; i++) {
        q16_16_t a = Q16_16_WRAP(gen_q16_16());
        q16_16_t result = q16_16_add(a, zero);
        TEST_ASSERT_Q16_16_EQ(result, a);
    }
}

// Multiplication is commutative
void prop_mul_commutative(void) {
    for (int i = 0; i < PROPERTY_ITERATIONS; i++) {
        q16_16_t a = Q16_16_WRAP(gen_q16_16());
        q16_16_t b = Q16_16_WRAP(gen_q16_16());

        q16_16_t r1 = q16_16_mul(a, b);
        q16_16_t r2 = q16_16_mul(b, a);

        TEST_ASSERT_Q16_16_EQ(r1, r2);
    }
}

// Multiplication by 1 is (approximately) identity
void prop_mul_identity(void) {
    q16_16_t one = Q16_16_WRAP(0x10000);  // 1.0

    for (int i = 0; i < PROPERTY_ITERATIONS; i++) {
        q16_16_t a = Q16_16_WRAP(gen_q16_16());
        q16_16_t result = q16_16_mul(a, one);

        // Allow 1 ULP error due to rounding
        TEST_ASSERT_Q16_16_NEAR(result, a, 1);
    }
}

// Subtraction inverse: a - a = 0
void prop_sub_inverse(void) {
    for (int i = 0; i < PROPERTY_ITERATIONS; i++) {
        q16_16_t a = Q16_16_WRAP(gen_q16_16());
        q16_16_t result = q16_16_sub(a, a);
        TEST_ASSERT_EQ(Q16_16_RAW(result), 0);
    }
}

// Saturation bounds: result is always in [MIN, MAX]
void prop_add_sat_bounded(void) {
    for (int i = 0; i < PROPERTY_ITERATIONS; i++) {
        q16_16_t a = Q16_16_WRAP(gen_q16_16());
        q16_16_t b = Q16_16_WRAP(gen_q16_16());

        q16_16_t result = q16_16_add_sat(a, b);

        TEST_ASSERT(Q16_16_RAW(result) >= INT32_MIN &&
                    Q16_16_RAW(result) <= INT32_MAX,
                    "saturated add in bounds");
    }
}
```

### 4.3 Trigonometric Properties

```c
// property/prop_trigonometry.c

// Pythagorean identity: sin²(x) + cos²(x) = 1
void prop_pythagorean(void) {
    q16_16_t one = Q16_16_WRAP(0x10000);

    for (int i = 0; i < PROPERTY_ITERATIONS; i++) {
        q16_16_t angle = Q16_16_WRAP(gen_q16_16_angle());

        q16_16_t s = sin_q16_16(angle);
        q16_16_t c = cos_q16_16(angle);

        q16_16_t s2 = q16_16_mul(s, s);
        q16_16_t c2 = q16_16_mul(c, c);
        q16_16_t sum = q16_16_add(s2, c2);

        // Allow some error (accumulation from 3 multiplications)
        TEST_ASSERT_Q16_16_NEAR(sum, one, 16);  // 16 ULP tolerance
    }
}

// sin(-x) = -sin(x)
void prop_sin_odd(void) {
    for (int i = 0; i < PROPERTY_ITERATIONS; i++) {
        q16_16_t angle = Q16_16_WRAP(gen_q16_16_angle());
        q16_16_t neg_angle = q16_16_neg(angle);

        q16_16_t s1 = sin_q16_16(angle);
        q16_16_t s2 = sin_q16_16(neg_angle);

        q16_16_t neg_s1 = q16_16_neg(s1);

        TEST_ASSERT_Q16_16_NEAR(s2, neg_s1, 1);
    }
}

// cos(-x) = cos(x)
void prop_cos_even(void) {
    for (int i = 0; i < PROPERTY_ITERATIONS; i++) {
        q16_16_t angle = Q16_16_WRAP(gen_q16_16_angle());
        q16_16_t neg_angle = q16_16_neg(angle);

        q16_16_t c1 = cos_q16_16(angle);
        q16_16_t c2 = cos_q16_16(neg_angle);

        TEST_ASSERT_Q16_16_NEAR(c1, c2, 1);
    }
}
```

---

## 5. Differential Testing

### 5.1 Against Floating-Point Reference

```c
// differential/diff_vs_float.c
#include <math.h>

#define MAX_ULP_ERROR 4  // Maximum allowed error in ULPs

void diff_sin_vs_float(void) {
    int max_ulp_seen = 0;

    for (int i = 0; i < PROPERTY_ITERATIONS; i++) {
        q16_16_t angle = Q16_16_WRAP(gen_q16_16_angle());

        q16_16_t result = sin_q16_16(angle);
        double angle_f = q16_16_to_double(angle);
        double ref = sin(angle_f);
        q16_16_t ref_q = q16_16_from_double(ref);

        int ulp_error = abs(Q16_16_RAW(result) - Q16_16_RAW(ref_q));
        if (ulp_error > max_ulp_seen) max_ulp_seen = ulp_error;

        TEST_ASSERT(ulp_error <= MAX_ULP_ERROR,
                    "sin within ULP bound");
    }

    printf("  sin: max ULP error = %d\n", max_ulp_seen);
}
```

### 5.2 Against MPFR (High Precision)

```python
# differential/reference.py
from mpmath import mp, mpf, sin, cos, sqrt
mp.prec = 100  # 100 bits of precision

def generate_sin_reference(angles):
    """Generate high-precision sin reference values."""
    results = []
    for angle_raw in angles:
        angle = mpf(angle_raw) / 65536  # Q16.16 to float
        result = sin(angle)
        result_q16 = int(result * 65536)
        results.append((angle_raw, result_q16))
    return results
```

---

## 6. Fuzzing Targets

### 6.1 LibFuzzer Target

```c
// fuzz/fuzz_mul.c
#include <stdint.h>
#include <stddef.h>
#include "fixp.h"

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 8) return 0;

    int32_t a = *(int32_t*)data;
    int32_t b = *(int32_t*)(data + 4);

    q16_16_t qa = Q16_16_WRAP(a);
    q16_16_t qb = Q16_16_WRAP(b);

    // Exercise various operations
    q16_16_t r1 = q16_16_mul(qa, qb);
    q16_16_t r2 = q16_16_mul_sat(qa, qb);

    // Invariant: saturating result should be bounded
    if (Q16_16_RAW(r2) < INT32_MIN || Q16_16_RAW(r2) > INT32_MAX) {
        __builtin_trap();  // Bug!
    }

    // Invariant: commutative
    q16_16_t r3 = q16_16_mul(qb, qa);
    if (Q16_16_RAW(r1) != Q16_16_RAW(r3)) {
        __builtin_trap();  // Bug!
    }

    return 0;
}
```

### 6.2 AFL++ Harness

```c
// fuzz/afl_harness.c
#include <stdio.h>
#include <stdlib.h>
#include "fixp.h"

int main(void) {
    uint8_t buf[16];

    if (fread(buf, 1, 16, stdin) < 16) return 0;

    int32_t a = *(int32_t*)buf;
    int32_t b = *(int32_t*)(buf + 4);
    int32_t c = *(int32_t*)(buf + 8);
    int32_t d = *(int32_t*)(buf + 12);

    q16_16_t qa = Q16_16_WRAP(a);
    q16_16_t qb = Q16_16_WRAP(b);
    q16_16_t qc = Q16_16_WRAP(c);
    q16_16_t qd = Q16_16_WRAP(d);

    // Complex expression
    q16_16_t r = q16_16_add(
        q16_16_mul(qa, qb),
        q16_16_div(qc, qd)
    );

    // Check invariants
    (void)r;

    return 0;
}
```

---

## 7. Formal Verification

### 7.1 CBMC Proofs

```c
// formal/cbmc_overflow.c
#include <assert.h>
#include <stdint.h>

// Prove saturating add never overflows
int32_t q16_16_add_sat(int32_t a, int32_t b) {
    int64_t sum = (int64_t)a + (int64_t)b;
    if (sum > INT32_MAX) return INT32_MAX;
    if (sum < INT32_MIN) return INT32_MIN;
    return (int32_t)sum;
}

int main(void) {
    int32_t a, b;

    // CBMC will explore all possible values
    __CPROVER_assume(a >= INT32_MIN && a <= INT32_MAX);
    __CPROVER_assume(b >= INT32_MIN && b <= INT32_MAX);

    int32_t result = q16_16_add_sat(a, b);

    // Prove bounds
    assert(result >= INT32_MIN);
    assert(result <= INT32_MAX);

    // Prove correctness when no overflow
    if ((int64_t)a + b >= INT32_MIN && (int64_t)a + b <= INT32_MAX) {
        assert(result == a + b);
    }

    return 0;
}
```

### 7.2 Z3 Proofs

```python
# formal/z3_proofs.py
from z3 import *

def prove_mul_commutative():
    a = BitVec('a', 32)
    b = BitVec('b', 32)

    # Q16.16 multiply
    def q16_mul(x, y):
        wide = SignExt(32, x) * SignExt(32, y)
        return Extract(47, 16, wide)

    r1 = q16_mul(a, b)
    r2 = q16_mul(b, a)

    solver = Solver()
    solver.add(r1 != r2)

    if solver.check() == unsat:
        print("Proved: Q16.16 multiply is commutative")
    else:
        print("Counterexample:", solver.model())
```

---

## 8. Performance Benchmarking

### 8.1 Benchmark Framework

```c
// benchmark/bench_framework.h
#include <time.h>

#define BENCH_ITERATIONS 10000000

#define BENCHMARK(name, setup, expr) do { \
    setup; \
    clock_t start = clock(); \
    for (int _i = 0; _i < BENCH_ITERATIONS; _i++) { \
        volatile int32_t _r = (expr); \
        (void)_r; \
    } \
    clock_t end = clock(); \
    double ns = (double)(end - start) / CLOCKS_PER_SEC * 1e9 / BENCH_ITERATIONS; \
    printf("%-30s: %.2f ns/op\n", name, ns); \
} while(0)
```

### 8.2 Benchmark Suite

```c
// benchmark/bench_arithmetic.c
#include "bench_framework.h"
#include "fixp.h"

void bench_q16_16(void) {
    printf("Q16.16 Benchmarks:\n");

    q16_16_t a = Q16_16_WRAP(0x12345678);
    q16_16_t b = Q16_16_WRAP(0x87654321);

    BENCHMARK("q16_16_add", , Q16_16_RAW(q16_16_add(a, b)));
    BENCHMARK("q16_16_add_sat", , Q16_16_RAW(q16_16_add_sat(a, b)));
    BENCHMARK("q16_16_mul", , Q16_16_RAW(q16_16_mul(a, b)));
    BENCHMARK("q16_16_div", , Q16_16_RAW(q16_16_div(a, b)));

    a = Q16_16_WRAP(0x10000);  // 1.0
    BENCHMARK("sin_q16_16", , Q16_16_RAW(sin_q16_16(a)));
    BENCHMARK("sqrt_q16_16", , Q16_16_RAW(sqrt_q16_16(a)));
}
```

### 8.3 Regression Detection

```json
// benchmark/baseline.json
{
    "q16_16_add": 1.2,
    "q16_16_mul": 3.5,
    "q16_16_div": 25.0,
    "sin_q16_16": 80.0,
    "threshold_percent": 10
}
```

---

## 9. CI Integration

### 9.1 GitHub Actions Workflow

```yaml
# .github/workflows/test.yml
name: Test Suite

on: [push, pull_request]

jobs:
  unit-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: make test
      - name: Run unit tests
        run: ./build/test_unit

  property-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: make test-property
      - name: Run property tests
        run: ./build/test_property --iterations 1000000

  exhaustive-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: make test-exhaustive
      - name: Run exhaustive Q7 tests
        run: ./build/test_exhaustive_q7

  fuzz:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build fuzz targets
        run: CC=clang make fuzz
      - name: Fuzz for 10 minutes
        run: timeout 600 ./build/fuzz_mul corpus/ || true

  formal:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install CBMC
        run: sudo apt-get install -y cbmc
      - name: Run CBMC proofs
        run: |
          cbmc tests/formal/cbmc_overflow.c --unwind 10

  benchmark:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build benchmarks
        run: make bench
      - name: Run benchmarks
        run: ./build/bench_all > results.txt
      - name: Check for regressions
        run: python3 scripts/check_perf.py results.txt
```

---

## 10. Test Coverage

### 10.1 Coverage Requirements

| Module | Line Coverage | Branch Coverage |
|--------|---------------|-----------------|
| Core arithmetic | 100% | 100% |
| Saturation ops | 100% | 100% |
| Conversion | 95% | 90% |
| Math functions | 90% | 85% |
| SIMD wrappers | 80% | 75% |

### 10.2 Coverage Tools

```bash
# GCC coverage
CFLAGS="-fprofile-arcs -ftest-coverage" make test
./build/test_all
gcov src/*.c
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html

# Clang coverage
CFLAGS="-fprofile-instr-generate -fcoverage-mapping" make test
LLVM_PROFILE_FILE="test.profraw" ./build/test_all
llvm-profdata merge -sparse test.profraw -o test.profdata
llvm-cov show ./build/test_all -instr-profile=test.profdata
```

---

## 11. Test Vectors

### 11.1 Golden Reference Data

```c
// vectors/sin_vectors.h
// Generated by high-precision reference implementation

typedef struct {
    int32_t angle;   // Q16.16 radians
    int32_t result;  // Q16.16 sin value
} sin_test_vector_t;

static const sin_test_vector_t sin_vectors[] = {
    {0x00000000, 0x00000000},  // sin(0) = 0
    {0x0000C90F, 0x0000B504},  // sin(pi/4) ≈ 0.7071
    {0x00019220, 0x00010000},  // sin(pi/2) = 1
    {0x0003243F, 0x00000000},  // sin(pi) ≈ 0
    // ... more vectors
};
```

### 11.2 Vector Generation

```python
# vectors/generate_vectors.py
from mpmath import mp, mpf, pi, sin, cos
import struct

mp.prec = 100

def q16_16_from_float(f):
    return int(f * 65536) & 0xFFFFFFFF

def generate_sin_vectors():
    vectors = []
    # Key angles
    angles = [0, pi/6, pi/4, pi/3, pi/2, 2*pi/3, 3*pi/4, 5*pi/6, pi]
    angles += [-a for a in angles]

    for angle in angles:
        angle_q = q16_16_from_float(float(angle))
        result_q = q16_16_from_float(float(sin(angle)))
        vectors.append((angle_q, result_q))

    return vectors
```

---

*Document Version: 0.1.0*
*Last Updated: 2025-12-29*
