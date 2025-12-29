# Multi-Language Code Generation Strategy

## Overview

libfixp generates optimized code for multiple targets from a single source of truth.

## Target Languages/Standards

### C Variants
| Standard | Features | Target Use |
|----------|----------|------------|
| K&R C | Pre-ANSI, no prototypes | Legacy compilers |
| ANSI C (C89) | Prototypes, const | Broad compatibility |
| C99 | stdint.h, inline, // comments | Modern embedded |
| C11 | _Alignas, _Static_assert | Modern systems |
| C17 | Bug fixes to C11 | Current standard |

### Embedded C
| Variant | Features | Target Use |
|---------|----------|------------|
| ISO TR 18037 | _Fract, _Accum types | DSP compilers |
| MISRA C | Safety-critical subset | Automotive, medical |

### C++ Variants
| Standard | Features | Target Use |
|----------|----------|------------|
| C++17 | if constexpr, fold expressions | Modern compilers |
| C++20 | Concepts, modules, consteval | Cutting edge |
| C++23 | std::expected, deducing this | Latest features |

### Embedded C++
| Variant | Features | Target Use |
|---------|----------|------------|
| EC++ | No exceptions/RTTI | Constrained embedded |
| Modern EC++ | Limited templates | ARM Cortex-M |

### GPU/Parallel
| Target | Features | Use Case |
|--------|----------|----------|
| OpenCL 1.2 | Wide compatibility | GPGPU baseline |
| OpenCL 3.0 | Optional features | Modern GPUs |
| OpenGL 2.0 GLSL | Shader model 1.1 | Legacy graphics |
| OpenGL 3.3 GLSL | Shader model 3.3 | Core profile |
| OpenGL 4.6 GLSL | Compute shaders | Modern compute |

## Generation Architecture

```
                    ┌─────────────────┐
                    │  Master Spec    │
                    │  (Language-     │
                    │   Agnostic)     │
                    └────────┬────────┘
                             │
              ┌──────────────┼──────────────┐
              ▼              ▼              ▼
        ┌─────────┐    ┌─────────┐    ┌─────────┐
        │ C Gen   │    │ C++ Gen │    │ GPU Gen │
        └────┬────┘    └────┬────┘    └────┬────┘
             │              │              │
    ┌────────┼────────┐     │     ┌────────┼────────┐
    ▼        ▼        ▼     │     ▼        ▼        ▼
  K&R     C99     C17      │   OCL1.2  OCL3.0  GLSL4.6
                           │
              ┌────────────┼────────────┐
              ▼            ▼            ▼
            C++17       C++20        C++23
```

## Master Specification Format

### Function Definition (Pseudo-DSL)
```yaml
function:
  name: add_q15
  description: "Add two Q15 fixed-point values with saturation"
  inputs:
    - name: a
      type: q15
      description: "First operand"
    - name: b
      type: q15
      description: "Second operand"
  outputs:
    - name: result
      type: q15
      description: "Saturated sum"
  algorithm: |
    sum = (int32)a + (int32)b
    if sum > Q15_MAX: return Q15_MAX
    if sum < Q15_MIN: return Q15_MIN
    return (q15)sum
  complexity:
    time: O(1)
    space: O(1)
  simd_vectorizable: true
  overflow_policy: configurable
```

## C Generation

### K&R Style
```c
/* add_q15 - Add two Q15 values with saturation */
short add_q15(a, b)
short a, b;
{
    long sum;
    sum = (long)a + (long)b;
    if (sum > 32767) return 32767;
    if (sum < -32768) return -32768;
    return (short)sum;
}
```

### C99 Style
```c
#include <stdint.h>

// Add two Q15 values with saturation
static inline int16_t add_q15(int16_t a, int16_t b) {
    int32_t sum = (int32_t)a + (int32_t)b;
    if (sum > INT16_MAX) return INT16_MAX;
    if (sum < INT16_MIN) return INT16_MIN;
    return (int16_t)sum;
}
```

### C11 Style
```c
#include <stdint.h>
#include <assert.h>

_Static_assert(sizeof(int16_t) == 2, "Q15 requires 16-bit integers");

static inline int16_t add_q15(int16_t a, int16_t b) {
    _Alignas(4) int32_t sum = (int32_t)a + (int32_t)b;
    return (sum > INT16_MAX) ? INT16_MAX :
           (sum < INT16_MIN) ? INT16_MIN :
           (int16_t)sum;
}
```

## C++ Generation

### C++17 Style
```cpp
#include <cstdint>
#include <limits>

namespace libfixp {

template<typename Policy = SaturatePolicy>
[[nodiscard]] constexpr auto add_q15(int16_t a, int16_t b) noexcept {
    int32_t sum = static_cast<int32_t>(a) + static_cast<int32_t>(b);

    if constexpr (std::is_same_v<Policy, SaturatePolicy>) {
        return static_cast<int16_t>(
            std::clamp(sum,
                static_cast<int32_t>(std::numeric_limits<int16_t>::min()),
                static_cast<int32_t>(std::numeric_limits<int16_t>::max()))
        );
    } else if constexpr (std::is_same_v<Policy, WrapPolicy>) {
        return static_cast<int16_t>(sum);
    }
}

} // namespace libfixp
```

### C++20 Style
```cpp
#include <cstdint>
#include <concepts>

namespace libfixp {

template<typename T>
concept OverflowPolicy = requires {
    { T::handle(int32_t{}) } -> std::same_as<int16_t>;
};

template<OverflowPolicy Policy = SaturatePolicy>
[[nodiscard]] constexpr int16_t add_q15(int16_t a, int16_t b) noexcept {
    int32_t sum = static_cast<int32_t>(a) + static_cast<int32_t>(b);
    return Policy::handle(sum);
}

} // namespace libfixp
```

### C++23 Style
```cpp
#include <cstdint>
#include <expected>

namespace libfixp {

consteval bool is_q15_safe(int32_t value) {
    return value >= -32768 && value <= 32767;
}

[[nodiscard]] constexpr std::expected<int16_t, OverflowError>
add_q15_checked(int16_t a, int16_t b) noexcept {
    int32_t sum = static_cast<int32_t>(a) + static_cast<int32_t>(b);
    if (!is_q15_safe(sum)) {
        return std::unexpected(OverflowError{sum});
    }
    return static_cast<int16_t>(sum);
}

} // namespace libfixp
```

## OpenCL Generation

### OpenCL 1.2
```opencl
// Q15 addition with saturation
short add_q15(__private short a, __private short b) {
    return add_sat(a, b);  // Built-in saturating add
}

// Q16.16 addition
int add_q16_16(__private int a, __private int b) {
    long sum = (long)a + (long)b;
    return (sum > 0x7FFFFFFF) ? 0x7FFFFFFF :
           (sum < (long)0xFFFFFFFF80000000) ? 0x80000000 :
           (int)sum;
}

__kernel void add_q15_vec(
    __global const short* a,
    __global const short* b,
    __global short* result,
    int n
) {
    int gid = get_global_id(0);
    if (gid < n) {
        result[gid] = add_q15(a[gid], b[gid]);
    }
}
```

### OpenCL 3.0
```opencl
// Using OpenCL 3.0 features where available
#if __OPENCL_VERSION__ >= 300
    // Use generic address space
    short add_q15(short a, short b) {
        return add_sat(a, b);
    }
#endif
```

## OpenGL Compute Shader Generation

### GLSL 4.30+ (Compute)
```glsl
#version 430 core

layout(local_size_x = 64) in;

layout(std430, binding = 0) readonly buffer InputA { int dataA[]; };
layout(std430, binding = 1) readonly buffer InputB { int dataB[]; };
layout(std430, binding = 2) writeonly buffer Output { int result[]; };

uniform int n;

// Q16.16 saturating add
int add_q16_16_sat(int a, int b) {
    int sum = a + b;
    // Detect overflow via sign bits
    int overflow = ((a ^ sum) & (b ^ sum)) >> 31;
    int max_val = (a >> 31) ^ 0x7FFFFFFF;
    return mix(sum, max_val, overflow != 0);
}

void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx < n) {
        result[idx] = add_q16_16_sat(dataA[idx], dataB[idx]);
    }
}
```

## Header Organization

### Single-Header Include
```c
// libfixp.h - Master include
#ifndef LIBFIXP_H
#define LIBFIXP_H

// Detect target
#if defined(__cplusplus)
    #if __cplusplus >= 202302L
        #include "libfixp_cpp23.hpp"
    #elif __cplusplus >= 202002L
        #include "libfixp_cpp20.hpp"
    #elif __cplusplus >= 201703L
        #include "libfixp_cpp17.hpp"
    #else
        #include "libfixp_cpp11.hpp"
    #endif
#elif defined(__STDC_VERSION__)
    #if __STDC_VERSION__ >= 201710L
        #include "libfixp_c17.h"
    #elif __STDC_VERSION__ >= 201112L
        #include "libfixp_c11.h"
    #elif __STDC_VERSION__ >= 199901L
        #include "libfixp_c99.h"
    #else
        #include "libfixp_c89.h"
    #endif
#else
    #include "libfixp_kr.h"
#endif

#endif // LIBFIXP_H
```

### Granular Includes
```
libfixp/
  core/
    q8.h        - 8-bit types
    q16.h       - 16-bit types
    q32.h       - 32-bit types
    q64.h       - 64-bit types
    q128.h      - 128-bit types
    q256.h      - 256-bit types
    q512.h      - 512-bit types
  math/
    trig.h      - sin, cos, tan, etc.
    exp.h       - exp, log, pow
    sqrt.h      - sqrt, rsqrt
  simd/
    detect.h    - SIMD detection
    x86.h       - SSE/AVX
    arm.h       - NEON/SVE
    generic.h   - Scalar fallback
```

## Build System Integration

### CMake Configuration
```cmake
option(LIBFIXP_C_STANDARD "C standard to use" "C17")
option(LIBFIXP_CXX_STANDARD "C++ standard to use" "CXX20")
option(LIBFIXP_OPENCL "Generate OpenCL headers" ON)
option(LIBFIXP_OPENGL "Generate GLSL shaders" ON)
```

### Generation Script
```python
# generate.py - Master generator
def generate_all():
    spec = load_spec("libfixp.spec")

    # C variants
    generate_c(spec, "c89", "out/c89/")
    generate_c(spec, "c99", "out/c99/")
    generate_c(spec, "c11", "out/c11/")
    generate_c(spec, "c17", "out/c17/")

    # C++ variants
    generate_cpp(spec, "cpp17", "out/cpp17/")
    generate_cpp(spec, "cpp20", "out/cpp20/")
    generate_cpp(spec, "cpp23", "out/cpp23/")

    # GPU
    generate_opencl(spec, "out/opencl/")
    generate_glsl(spec, "out/glsl/")
```

## Verification

### Cross-Platform Testing
1. Generate for all targets
2. Compile with respective compilers
3. Run identical test vectors
4. Verify bit-exact results

### Consistency Checks
- Same algorithm, different syntax
- Identical numerical results
- Matching API semantics
