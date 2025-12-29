# Testing Strategies for Fixed-Point Libraries

## Overview

Fixed-point libraries require rigorous testing due to:
- Subtle overflow/underflow bugs
- Precision loss accumulation
- Platform-specific behavior
- Rounding mode sensitivities

---

## Testing Approaches

### 1. Exhaustive Testing (Small Types)

For 8-bit and 16-bit types, exhaustive testing is feasible:

```c
// Test all Q7 × Q7 combinations
void test_q7_mul_exhaustive(void) {
    for (int a = -128; a <= 127; a++) {
        for (int b = -128; b <= 127; b++) {
            int8_t result = q7_mul((int8_t)a, (int8_t)b);
            double expected = ((double)a / 128.0) * ((double)b / 128.0) * 128.0;
            int8_t ref = saturate_q7((int8_t)round(expected));
            assert(result == ref);
        }
    }
}
// 65536 test cases - runs in seconds
```

### 2. Property-Based Testing

Test invariants that should always hold:

```python
# Using Hypothesis
from hypothesis import given, strategies as st

@given(st.integers(-32768, 32767), st.integers(-32768, 32767))
def test_q15_add_commutative(a, b):
    assert q15_add(a, b) == q15_add(b, a)

@given(st.integers(-32768, 32767))
def test_q15_add_identity(a):
    assert q15_add(a, 0) == a

@given(st.integers(-32768, 32767))
def test_q15_mul_by_one(a):
    Q15_ONE = 0x7FFF
    # Multiply by ~1.0 should give approximately same value
    result = q15_mul(a, Q15_ONE)
    assert abs(result - a) <= 1  # Allow 1 LSB error
```

### 3. Fuzzing

Random input generation with coverage guidance:

```c
// AFL/LibFuzzer target
int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 8) return 0;

    q16_16 a = *(q16_16*)data;
    q16_16 b = *(q16_16*)(data + 4);

    // Exercise operations
    q16_16 sum = q16_16_add(a, b);
    q16_16 diff = q16_16_sub(a, b);
    q16_16 prod = q16_16_mul(a, b);

    // Check invariants
    assert(q16_16_sub(sum, a) == b || is_overflow(a, b));

    return 0;
}
```

### 4. Differential Testing

Compare against reference implementations:

```c
void test_sin_vs_reference(void) {
    for (int i = 0; i < 10000; i++) {
        q16_16 angle = random_q16_16();
        q16_16 result = sin_q16_16(angle);
        double ref = sin(q16_16_to_double(angle));
        double error = fabs(q16_16_to_double(result) - ref);
        assert(error < MAX_ALLOWED_ERROR);
    }
}
```

---

## Test Categories

### Unit Tests

```c
// Basic operation tests
void test_q16_16_add(void) {
    // Normal cases
    assert(q16_16_add(0x10000, 0x10000) == 0x20000);  // 1.0 + 1.0 = 2.0

    // Saturation
    assert(q16_16_add(0x7FFFFFFF, 1) == 0x7FFFFFFF);  // Max + 1 = Max

    // Negative
    assert(q16_16_add(0xFFFF0000, 0x10000) == 0);  // -1.0 + 1.0 = 0
}
```

### Boundary Tests

```c
void test_boundaries(void) {
    // Maximum values
    test_at(Q16_16_MAX);
    test_at(Q16_16_MIN);
    test_at(0);
    test_at(1);
    test_at(-1);
    test_at(Q16_16_ONE);  // 1.0
    test_at(-Q16_16_ONE);  // -1.0

    // Near boundaries
    test_at(Q16_16_MAX - 1);
    test_at(Q16_16_MIN + 1);
}
```

### Overflow Tests

```c
void test_overflow_detection(void) {
    // Add positive overflow
    q16_16 a = 0x7FFF0000;  // Near max
    q16_16 b = 0x00020000;  // 2.0
    q16_16 result = q16_16_add_checked(a, b, &overflow);
    assert(overflow == true);

    // Multiply overflow
    a = 0x01000000;  // 256.0
    b = 0x01000000;  // 256.0
    result = q16_16_mul_checked(a, b, &overflow);
    assert(overflow == true);  // 65536 doesn't fit
}
```

### Accuracy Tests

```c
void test_sin_accuracy(void) {
    int max_ulp_error = 0;

    for (int i = 0; i < NUM_TEST_ANGLES; i++) {
        q16_16 angle = test_angles[i];
        q16_16 result = sin_q16_16(angle);
        q16_16 reference = double_to_q16_16(sin(q16_16_to_double(angle)));

        int ulp_error = abs(result - reference);
        if (ulp_error > max_ulp_error) {
            max_ulp_error = ulp_error;
        }
    }

    assert(max_ulp_error <= MAX_SIN_ULP_ERROR);
    printf("sin_q16_16 max error: %d ULP\n", max_ulp_error);
}
```

---

## Test Vector Generation

### Mathematical Derivation

```python
# Generate known-correct test vectors
import mpmath
mpmath.mp.prec = 100  # High precision

def generate_sin_test_vectors():
    vectors = []
    for angle_deg in range(0, 360):
        angle_rad = mpmath.mpf(angle_deg) * mpmath.pi / 180
        sin_val = mpmath.sin(angle_rad)

        # Convert to Q16.16
        angle_fixed = int(angle_rad * 65536)
        sin_fixed = int(sin_val * 65536)

        vectors.append((angle_fixed, sin_fixed))
    return vectors
```

### Corner Case Generation

```python
def generate_corner_cases():
    cases = [
        (0, 0),           # Zero
        (0x10000, 0),     # pi/2
        (0x20000, 0),     # pi
        (0x30000, 0),     # 3pi/2
        (0x7FFF, 0),      # Just under pi/2
        (0x8001, 0),      # Just over pi/2
    ]
    return cases
```

---

## Property Invariants

### Arithmetic Properties

| Operation | Property |
|-----------|----------|
| Add | Commutative: a + b = b + a |
| Add | Associative: (a + b) + c = a + (b + c) (modulo rounding) |
| Add | Identity: a + 0 = a |
| Mul | Commutative: a × b = b × a |
| Mul | Identity: a × 1 = a (approximately) |
| Mul | Zero: a × 0 = 0 |
| Sub | Inverse: a - a = 0 |

### Trigonometric Properties

```c
// sin²(x) + cos²(x) = 1
void test_pythagorean_identity(q16_16 x) {
    q16_16 s = sin_q16_16(x);
    q16_16 c = cos_q16_16(x);
    q16_16 sum = q16_16_add(q16_16_mul(s, s), q16_16_mul(c, c));
    // Should be close to 1.0 (0x10000)
    assert(abs(sum - 0x10000) < TOLERANCE);
}

// sin(-x) = -sin(x)
void test_sin_odd(q16_16 x) {
    assert(sin_q16_16(-x) == -sin_q16_16(x));
}
```

---

## Platform Testing

### Cross-Platform Consistency

```yaml
# CI matrix
test_matrix:
  - arch: x86_64
    compiler: [gcc, clang]
  - arch: arm64
    compiler: [gcc, clang]
  - arch: arm32
    compiler: gcc
    flags: [-mcpu=cortex-m4]
  - arch: wasm
    runtime: node
```

### Endianness Testing

```c
void test_endianness_independent(void) {
    uint8_t bytes[] = {0x12, 0x34, 0x56, 0x78};
    q16_16 value = q16_16_from_bytes(bytes);
    uint8_t out[4];
    q16_16_to_bytes(value, out);
    assert(memcmp(bytes, out, 4) == 0);
}
```

---

## Continuous Integration

### Test Pipeline

```yaml
stages:
  - lint:
      - clang-format check
      - cppcheck
  - unit_tests:
      - exhaustive_8bit
      - unit_16bit
      - unit_32bit
  - property_tests:
      - hypothesis (Python bindings)
      - proptest (Rust bindings)
  - fuzz:
      - libfuzzer (10 min)
      - afl++ (if time permits)
  - benchmark:
      - compare against baseline
      - flag regressions > 5%
```

### Coverage Requirements

```yaml
coverage:
  minimum: 95%
  exclude:
    - platform-specific fallbacks
    - debug-only code
```

---

## Benchmark Tests

### Performance Regression Detection

```c
void benchmark_q16_16_mul(benchmark_state* state) {
    q16_16 a = 0x12345678;
    q16_16 b = 0x87654321;
    volatile q16_16 result;

    while (state_keep_running(state)) {
        for (int i = 0; i < 1000; i++) {
            result = q16_16_mul(a, b);
        }
    }
}

// Flag if >5% slower than baseline
```

### Accuracy vs Speed Tradeoffs

```c
void benchmark_sin_variants(void) {
    benchmark("sin_cordic_16iter", sin_cordic<16>);
    benchmark("sin_cordic_12iter", sin_cordic<12>);
    benchmark("sin_poly_5", sin_polynomial<5>);
    benchmark("sin_poly_7", sin_polynomial<7>);
    benchmark("sin_table_256", sin_table<256>);
}
```

---

## Tools

| Tool | Purpose |
|------|---------|
| GoogleTest | C++ unit testing |
| Catch2 | C++ BDD-style testing |
| Hypothesis | Python property-based |
| proptest | Rust property-based |
| AFL++ | Coverage-guided fuzzing |
| libFuzzer | LLVM-integrated fuzzing |
| Valgrind | Memory error detection |
| UBSan | Undefined behavior detection |
| ASan | Address sanitizer |

---

## References

- [Property-Based Testing Is Fuzzing](https://blog.nelhage.com/post/property-testing-is-fuzzing/)
- [Fuzzing vs Property Testing](https://www.tedinski.com/2018/12/11/fuzzing-and-property-testing.html)
- [Hypothesis Documentation](https://hypothesis.readthedocs.io/)
