# libfixp Type System Design Specification

## Overview

This document specifies the type system for libfixp, a fixed-point arithmetic library supporting:
- All Q(m.n) format combinations from Q0.7 to Q63.64
- Multiple word sizes: 8, 16, 32, 64, and 128+ bits
- Signed and unsigned variants
- Configurable overflow semantics
- Multi-language generation (C, C++, OpenCL, OpenGL GLSL)

---

## 1. Type Naming Convention

### 1.1 Standard Q-Format Types

```
q<int_bits>_<frac_bits>      // Signed
uq<int_bits>_<frac_bits>     // Unsigned

Examples:
q7_t     = Q0.7  (8-bit signed, 7 fractional bits, range [-1, +0.992])
q15_t    = Q0.15 (16-bit signed, 15 fractional bits, range [-1, +0.99997])
q16_16_t = Q16.16 (32-bit signed, 16 each, range [-32768, +32767.99998])
uq8_8_t  = UQ8.8 (16-bit unsigned, range [0, 255.996])
```

### 1.2 Common Aliases

| Alias | Full Name | Total Bits | Use Case |
|-------|-----------|------------|----------|
| `q7_t` | `q0_7_t` | 8 | Neural network weights |
| `q15_t` | `q0_15_t` | 16 | Audio samples |
| `q31_t` | `q0_31_t` | 32 | High-precision audio |
| `q16_16_t` | `q16_16_t` | 32 | General purpose |
| `q32_32_t` | `q32_32_t` | 64 | Extended precision |

### 1.3 Generic Type Macro

For compile-time configurable formats:

```c
// FIXP(total_bits, frac_bits, signed)
typedef FIXP(32, 16, true)  q16_16_t;
typedef FIXP(32, 16, false) uq16_16_t;
```

---

## 2. Underlying Storage Types

### 2.1 C Storage Mapping

| Total Bits | Signed Type | Unsigned Type | Wide Type |
|------------|-------------|---------------|-----------|
| 8 | `int8_t` | `uint8_t` | `int16_t` |
| 16 | `int16_t` | `uint16_t` | `int32_t` |
| 32 | `int32_t` | `uint32_t` | `int64_t` |
| 64 | `int64_t` | `uint64_t` | `__int128` (if available) |
| 128 | `__int128` | `unsigned __int128` | emulated |

### 2.2 Type Traits (C++ / Code Generation)

```c
// Compile-time type information
#define FIXP_TOTAL_BITS(T)    // Total bits in type
#define FIXP_FRAC_BITS(T)     // Fractional bits
#define FIXP_INT_BITS(T)      // Integer bits (excl. sign for signed)
#define FIXP_IS_SIGNED(T)     // true if signed
#define FIXP_ONE(T)           // Value representing 1.0
#define FIXP_MIN(T)           // Minimum representable value
#define FIXP_MAX(T)           // Maximum representable value
#define FIXP_EPSILON(T)       // Smallest increment (2^-frac_bits)
```

---

## 3. Type Structure

### 3.1 C Structure (Transparent)

```c
// Header-only, type-safe wrapper
typedef struct q16_16 {
    int32_t v;
} q16_16_t;

// Raw value access
#define Q16_16_RAW(x)      ((x).v)
#define Q16_16_WRAP(raw)   ((q16_16_t){.v = (raw)})
```

### 3.2 C++ Class Template

```cpp
template<int TotalBits, int FracBits, bool Signed = true>
struct Fixed {
    using storage_t = /* select based on TotalBits */;
    using wide_t = /* double-width for multiplication */;

    static constexpr int total_bits = TotalBits;
    static constexpr int frac_bits = FracBits;
    static constexpr int int_bits = TotalBits - FracBits - (Signed ? 1 : 0);
    static constexpr storage_t one = storage_t(1) << FracBits;

    storage_t value;

    // Constructors, operators defined below
};
```

---

## 4. Overflow Policy System

### 4.1 Policy Enumeration

```c
typedef enum {
    FIXP_OVERFLOW_WRAP,      // Modular arithmetic (default)
    FIXP_OVERFLOW_SATURATE,  // Clamp to min/max
    FIXP_OVERFLOW_TRAP,      // Abort/trap on overflow
    FIXP_OVERFLOW_UNDEFINED  // UB (for optimization)
} fixp_overflow_t;
```

### 4.2 Global Policy

```c
// Set global policy (affects all subsequent operations)
void fixp_set_overflow_policy(fixp_overflow_t policy);
fixp_overflow_t fixp_get_overflow_policy(void);

// Thread-local variant (if threading enabled)
void fixp_set_overflow_policy_tl(fixp_overflow_t policy);
```

### 4.3 Per-Operation Suffix Convention

```c
// Explicit overflow behavior
q16_16_t q16_16_add(q16_16_t a, q16_16_t b);      // Uses global policy
q16_16_t q16_16_add_wrap(q16_16_t a, q16_16_t b); // Always wrap
q16_16_t q16_16_add_sat(q16_16_t a, q16_16_t b);  // Always saturate
q16_16_t q16_16_add_chk(q16_16_t a, q16_16_t b, bool* overflow); // Check
```

### 4.4 C++ Policy Template

```cpp
template<typename Policy = DefaultPolicy>
struct Fixed<32, 16, true, Policy> {
    // Policy-aware operations
};

// Usage:
using Q16_16_Sat = Fixed<32, 16, true, SaturatePolicy>;
using Q16_16_Wrap = Fixed<32, 16, true, WrapPolicy>;
```

---

## 5. Rounding Modes

### 5.1 Rounding Enumeration

```c
typedef enum {
    FIXP_ROUND_TRUNC,        // Truncate (toward zero)
    FIXP_ROUND_FLOOR,        // Floor (toward -infinity)
    FIXP_ROUND_CEIL,         // Ceiling (toward +infinity)
    FIXP_ROUND_NEAREST_EVEN, // Banker's rounding
    FIXP_ROUND_NEAREST_UP,   // Round half up
    FIXP_ROUND_STOCHASTIC    // Probabilistic (for ML)
} fixp_rounding_t;
```

### 5.2 Default: Truncation

Most operations use truncation by default (fastest). Rounding variants available:

```c
q16_16_t q16_16_mul(q16_16_t a, q16_16_t b);           // Truncate
q16_16_t q16_16_mul_rnd(q16_16_t a, q16_16_t b);       // Round nearest
q16_16_t q16_16_mul_rne(q16_16_t a, q16_16_t b);       // Round nearest even
```

---

## 6. Core Operations

### 6.1 Arithmetic Operations

| Operation | Function | Notes |
|-----------|----------|-------|
| Add | `q_add(a, b)` | Same Q format required |
| Subtract | `q_sub(a, b)` | Same Q format required |
| Negate | `q_neg(a)` | Beware MIN value |
| Multiply | `q_mul(a, b)` | Result same format |
| Divide | `q_div(a, b)` | Result same format |
| Multiply-Accumulate | `q_mac(acc, a, b)` | acc += a*b |

### 6.2 Comparison Operations

```c
bool q16_16_eq(q16_16_t a, q16_16_t b);   // a == b
bool q16_16_ne(q16_16_t a, q16_16_t b);   // a != b
bool q16_16_lt(q16_16_t a, q16_16_t b);   // a < b
bool q16_16_le(q16_16_t a, q16_16_t b);   // a <= b
bool q16_16_gt(q16_16_t a, q16_16_t b);   // a > b
bool q16_16_ge(q16_16_t a, q16_16_t b);   // a >= b
int  q16_16_cmp(q16_16_t a, q16_16_t b);  // -1, 0, +1
```

### 6.3 Bit Operations

```c
q16_16_t q16_16_shl(q16_16_t a, int shift);  // Left shift
q16_16_t q16_16_shr(q16_16_t a, int shift);  // Arithmetic right shift
q16_16_t q16_16_abs(q16_16_t a);             // Absolute value
q16_16_t q16_16_min(q16_16_t a, q16_16_t b); // Minimum
q16_16_t q16_16_max(q16_16_t a, q16_16_t b); // Maximum
q16_16_t q16_16_clamp(q16_16_t x, q16_16_t lo, q16_16_t hi);
```

### 6.4 Conversion Operations

```c
// Between fixed-point formats
q31_t    q16_16_to_q31(q16_16_t a);       // Widen
q16_16_t q31_to_q16_16(q31_t a);          // Narrow (may saturate)

// To/from floating-point
q16_16_t q16_16_from_float(float f);
q16_16_t q16_16_from_double(double d);
float    q16_16_to_float(q16_16_t a);
double   q16_16_to_double(q16_16_t a);

// To/from integer
q16_16_t q16_16_from_int(int32_t i);      // i * 2^16
int32_t  q16_16_to_int(q16_16_t a);       // Truncate fractional part
int32_t  q16_16_to_int_rnd(q16_16_t a);   // Round to nearest
```

---

## 7. Math Functions

### 7.1 Trigonometric

```c
q16_16_t sin_q16_16(q16_16_t angle);      // angle in radians
q16_16_t cos_q16_16(q16_16_t angle);
q16_16_t tan_q16_16(q16_16_t angle);
q16_16_t asin_q16_16(q16_16_t x);         // x in [-1, 1]
q16_16_t acos_q16_16(q16_16_t x);
q16_16_t atan_q16_16(q16_16_t x);
q16_16_t atan2_q16_16(q16_16_t y, q16_16_t x);

// Degree variants
q16_16_t sind_q16_16(q16_16_t degrees);
q16_16_t cosd_q16_16(q16_16_t degrees);
```

### 7.2 Exponential and Logarithmic

```c
q16_16_t exp_q16_16(q16_16_t x);          // e^x
q16_16_t exp2_q16_16(q16_16_t x);         // 2^x
q16_16_t log_q16_16(q16_16_t x);          // ln(x)
q16_16_t log2_q16_16(q16_16_t x);         // log2(x)
q16_16_t log10_q16_16(q16_16_t x);        // log10(x)
q16_16_t pow_q16_16(q16_16_t base, q16_16_t exp);
```

### 7.3 Root and Power

```c
q16_16_t sqrt_q16_16(q16_16_t x);
q16_16_t rsqrt_q16_16(q16_16_t x);        // 1/sqrt(x)
q16_16_t cbrt_q16_16(q16_16_t x);         // Cube root
q16_16_t hypot_q16_16(q16_16_t x, q16_16_t y); // sqrt(x^2 + y^2)
```

### 7.4 Interpolation

```c
q16_16_t lerp_q16_16(q16_16_t a, q16_16_t b, q16_16_t t);
q16_16_t smoothstep_q16_16(q16_16_t edge0, q16_16_t edge1, q16_16_t x);
```

---

## 8. Type Hierarchy for 8-Bit Primitives

### 8.1 Modular 8-Bit Core

All operations decompose to 8-bit primitives for maximum portability:

```c
// Core 8-bit operations (always available)
typedef int8_t q7_t;

q7_t q7_add(q7_t a, q7_t b);
q7_t q7_add_sat(q7_t a, q7_t b);
q7_t q7_sub(q7_t a, q7_t b);
q7_t q7_mul(q7_t a, q7_t b);  // Returns Q7 (high byte of 16-bit product)

// Multi-byte operations built on 8-bit primitives
typedef struct { q7_t bytes[2]; } q15_t_split;
typedef struct { q7_t bytes[4]; } q31_t_split;
```

### 8.2 Wide Operations via Composition

```c
// 32-bit add using 8-bit primitives (for platforms without 32-bit ops)
q16_16_t q16_16_add_8bit(q16_16_t a, q16_16_t b) {
    uint8_t* ap = (uint8_t*)&a.v;
    uint8_t* bp = (uint8_t*)&b.v;
    uint8_t* rp = (uint8_t*)&result.v;

    uint16_t carry = 0;
    for (int i = 0; i < 4; i++) {
        uint16_t sum = ap[i] + bp[i] + carry;
        rp[i] = sum & 0xFF;
        carry = sum >> 8;
    }
    return result;
}
```

---

## 9. SIMD Abstraction Layer

### 9.1 Vector Types

```c
// Platform-agnostic vector types
typedef struct { q15_t v[8]; } q15x8_t;   // 8 Q15 values
typedef struct { q15_t v[16]; } q15x16_t; // 16 Q15 values
typedef struct { q16_16_t v[4]; } q16_16x4_t; // 4 Q16.16 values
typedef struct { q16_16_t v[8]; } q16_16x8_t; // 8 Q16.16 values
```

### 9.2 Vector Operations

```c
q15x8_t q15x8_add(q15x8_t a, q15x8_t b);
q15x8_t q15x8_add_sat(q15x8_t a, q15x8_t b);
q15x8_t q15x8_mul(q15x8_t a, q15x8_t b);
q15x8_t q15x8_mac(q15x8_t acc, q15x8_t a, q15x8_t b);

// Reduction operations
q15_t q15x8_sum(q15x8_t a);     // Sum all elements
q15_t q15x8_min(q15x8_t a);     // Minimum element
q15_t q15x8_max(q15x8_t a);     // Maximum element
```

### 9.3 Platform Dispatch

```c
// At runtime or compile time, select optimal implementation
#if defined(__AVX2__)
    #define q15x8_add_sat  q15x8_add_sat_avx2
#elif defined(__ARM_NEON)
    #define q15x8_add_sat  q15x8_add_sat_neon
#else
    #define q15x8_add_sat  q15x8_add_sat_scalar
#endif
```

---

## 10. Error Handling

### 10.1 Error Codes

```c
typedef enum {
    FIXP_OK = 0,
    FIXP_ERR_OVERFLOW,
    FIXP_ERR_UNDERFLOW,
    FIXP_ERR_DIVIDE_BY_ZERO,
    FIXP_ERR_DOMAIN,         // e.g., sqrt of negative
    FIXP_ERR_PRECISION_LOSS
} fixp_error_t;
```

### 10.2 Error Checking API

```c
// Get last error (thread-local)
fixp_error_t fixp_get_error(void);
void fixp_clear_error(void);
const char* fixp_error_string(fixp_error_t err);

// Checked operations
q16_16_t q16_16_div_checked(q16_16_t a, q16_16_t b, fixp_error_t* err);
q16_16_t sqrt_q16_16_checked(q16_16_t x, fixp_error_t* err);
```

---

## 11. Header Organization

### 11.1 File Structure

```
include/
├── fixp.h              # Master include (includes all)
├── fixp_config.h       # Build configuration
├── fixp_types.h        # Type definitions
├── fixp_q7.h           # Q7 operations
├── fixp_q15.h          # Q15 operations
├── fixp_q31.h          # Q31 operations
├── fixp_q16_16.h       # Q16.16 operations
├── fixp_q32_32.h       # Q32.32 operations
├── fixp_math.h         # Transcendental functions
├── fixp_simd.h         # SIMD abstractions
├── fixp_simd_sse.h     # SSE implementation
├── fixp_simd_avx.h     # AVX implementation
├── fixp_simd_neon.h    # NEON implementation
└── fixp_simd_sve.h     # SVE implementation
```

### 11.2 Include Guards and Namespaces

```c
// C header pattern
#ifndef FIXP_Q16_16_H
#define FIXP_Q16_16_H

#ifdef __cplusplus
extern "C" {
#endif

// C declarations...

#ifdef __cplusplus
}

// C++ namespace wrapper
namespace fixp {
    using ::q16_16_t;
    using ::q16_16_add;
    // Or full C++ class wrappers
}
#endif

#endif // FIXP_Q16_16_H
```

---

## 12. Configuration Options

### 12.1 Compile-Time Options

```c
// fixp_config.h

// Enable/disable features
#define FIXP_ENABLE_OVERFLOW_CHECKING  1
#define FIXP_ENABLE_ROUNDING           1
#define FIXP_ENABLE_SIMD               1
#define FIXP_ENABLE_128BIT             0  // Requires __int128

// Default policies
#define FIXP_DEFAULT_OVERFLOW  FIXP_OVERFLOW_WRAP
#define FIXP_DEFAULT_ROUNDING  FIXP_ROUND_TRUNC

// Platform hints
#define FIXP_TARGET_EMBEDDED   0  // Minimize code size
#define FIXP_TARGET_DSP        0  // Optimize for DSP patterns
```

### 12.2 Single-Header Mode

```c
// For single-header distribution
#define FIXP_IMPLEMENTATION
#include "fixp.h"
// This includes all implementations inline
```

---

## 13. OpenCL/OpenGL GLSL Generation

### 13.1 OpenCL Type Mapping

```opencl
// Generated opencl_fixp.cl
typedef short q15_t;
typedef int q16_16_t;
typedef long q32_32_t;

#define Q15_FRAC_BITS 15
#define Q16_16_FRAC_BITS 16

q16_16_t q16_16_mul(q16_16_t a, q16_16_t b) {
    long product = (long)a * (long)b;
    return (q16_16_t)(product >> Q16_16_FRAC_BITS);
}

q16_16_t q16_16_add_sat(q16_16_t a, q16_16_t b) {
    long sum = (long)a + (long)b;
    return clamp(sum, (long)INT_MIN, (long)INT_MAX);
}
```

### 13.2 GLSL Type Mapping

```glsl
// Generated glsl_fixp.glsl
#define q16_16_t int
#define Q16_16_ONE 65536

int q16_16_mul(int a, int b) {
    // GLSL has no 64-bit int, need workaround
    // Split into high/low parts
    int a_hi = a >> 16;
    int a_lo = a & 0xFFFF;
    int b_hi = b >> 16;
    int b_lo = b & 0xFFFF;

    int result = a_hi * b_hi;
    result += (a_hi * b_lo) >> 16;
    result += (a_lo * b_hi) >> 16;
    return result;
}
```

---

## 14. Versioning

```c
#define FIXP_VERSION_MAJOR 0
#define FIXP_VERSION_MINOR 1
#define FIXP_VERSION_PATCH 0
#define FIXP_VERSION_STRING "0.1.0"

// API stability markers
#define FIXP_API_STABLE      // Won't change
#define FIXP_API_UNSTABLE    // May change
#define FIXP_API_DEPRECATED  // Scheduled for removal
```

---

## Appendix A: Complete Q Format Matrix

| Name | Total | Int | Frac | Range (approx) | Resolution |
|------|-------|-----|------|----------------|------------|
| Q0.7 | 8 | 0 | 7 | [-1, 0.992] | 0.0078 |
| Q3.4 | 8 | 3 | 4 | [-8, 7.94] | 0.0625 |
| Q7.0 | 8 | 7 | 0 | [-128, 127] | 1 |
| Q0.15 | 16 | 0 | 15 | [-1, 0.99997] | 3.05e-5 |
| Q7.8 | 16 | 7 | 8 | [-128, 127.996] | 0.0039 |
| Q8.8 | 16 | 8 | 8 | [-256, 255.996] | 0.0039 |
| Q15.0 | 16 | 15 | 0 | [-32768, 32767] | 1 |
| Q0.31 | 32 | 0 | 31 | [-1, 0.9999999995] | 4.66e-10 |
| Q16.16 | 32 | 16 | 16 | [-32768, 32767.99998] | 1.53e-5 |
| Q24.8 | 32 | 24 | 8 | [-8M, 8M-0.004] | 0.0039 |
| Q8.24 | 32 | 8 | 24 | [-128, 127.999999] | 5.96e-8 |
| Q32.32 | 64 | 32 | 32 | [-2B, 2B-2.3e-10] | 2.33e-10 |
| Q48.16 | 64 | 48 | 16 | [-140T, 140T] | 1.53e-5 |

---

## Appendix B: C23/C++23 Considerations

With C23 `_BitInt` and C++23 features:

```c
// C23 arbitrary-width integers
typedef _BitInt(24) q8_16_storage_t;  // Exactly 24 bits
typedef _BitInt(48) q16_32_storage_t;

// C++23 `std::fixed_point` proposal (if adopted)
#include <fixed_point>
using Q16_16 = std::fixed_point<int32_t, -16>;
```

---

*Document Version: 0.1.0*
*Last Updated: 2025-12-29*
