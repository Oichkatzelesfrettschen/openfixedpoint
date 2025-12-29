# SIMD Fixed-Point Patterns

## Overview

SIMD (Single Instruction Multiple Data) processes multiple fixed-point values in parallel. Key architectures:
- x86/x64: SSE, AVX, AVX2, AVX-512
- ARM: NEON, SVE, SVE2
- RISC-V: V extension
- GPU: SIMT (not true SIMD but related)

## Vector Widths

| Architecture | Extension | Width | Elements (16-bit) | Elements (32-bit) |
|--------------|-----------|-------|-------------------|-------------------|
| x86/x64 | SSE2 | 128-bit | 8 | 4 |
| x86/x64 | AVX2 | 256-bit | 16 | 8 |
| x86/x64 | AVX-512 | 512-bit | 32 | 16 |
| ARM | NEON | 128-bit | 8 | 4 |
| ARM | SVE | 128-2048-bit | 8-128 | 4-64 |
| RISC-V | V | Scalable | Scalable | Scalable |

## Fixed-Point SIMD Operations

### Basic Arithmetic

#### Addition (Regular)
- x86: `_mm_add_epi16`, `_mm256_add_epi16`, `_mm512_add_epi16`
- ARM NEON: `vaddq_s16`, `vaddq_s32`
- Wrapping behavior, no overflow detection

#### Addition (Saturating)
- x86: `_mm_adds_epi16` (signed), `_mm_adds_epu16` (unsigned)
- ARM NEON: `vqaddq_s16`, `vqaddq_s32`
- Clamps to min/max on overflow

#### Subtraction
- Similar patterns: `sub` for regular, `subs` for saturating

#### Multiplication
- Produces double-width result
- x86: `_mm_mullo_epi16` (low bits), `_mm_mulhi_epi16` (high bits)
- Need both for Q format multiplication

### Q-Format Multiplication Pattern

For Q15 * Q15 -> Q15:
```
// Conceptual (x86)
__m128i a, b;           // Q15 values
__m128i lo = _mm_mullo_epi16(a, b);   // Low 16 bits of 32-bit product
__m128i hi = _mm_mulhi_epi16(a, b);   // High 16 bits
// Q30 result needs >> 15 to become Q15
// Complex bit manipulation to extract middle 16 bits
```

Better approach - use 32-bit intermediates:
```
__m128i a16, b16;
__m128i a32_lo = _mm_cvtepi16_epi32(a16);
__m128i b32_lo = _mm_cvtepi16_epi32(b16);
__m128i prod = _mm_mullo_epi32(a32_lo, b32_lo);  // 32-bit product
__m128i result = _mm_srai_epi32(prod, 15);        // Shift for Q15
// Pack back to 16-bit
```

### Shift Operations
- Logical: `_mm_slli_epi16`, `_mm_srli_epi16`
- Arithmetic: `_mm_srai_epi16` (sign-extending right shift)
- Essential for Q format conversion and multiplication

### Comparison and Selection
- `_mm_cmpgt_epi16` - compare greater than
- `_mm_blendv_epi8` - conditional select
- Used for saturation, abs, min/max

## Scalable Vector Architectures

### ARM SVE/SVE2
- Vector length agnostic (VLA) programming
- Predicate registers for masking
- No fixed vector size at compile time
- svint16_t, svint32_t types

```c
// SVE example (conceptual)
svint16_t a = svld1_s16(pred, ptr_a);
svint16_t b = svld1_s16(pred, ptr_b);
svint16_t c = svqadd_s16(a, b);  // Saturating add
svst1_s16(pred, ptr_c, c);
```

### RISC-V V Extension
- Also scalable/vector-length agnostic
- Similar programming model to SVE
- vsetvl for setting vector length

## SIMD Abstraction Strategies

### 1. Direct Intrinsics
- Maximum performance
- Architecture-specific code
- Maintenance burden

### 2. Compiler Auto-vectorization
- Portable source
- Often suboptimal
- Unpredictable

### 3. SIMD Wrapper Libraries
- Examples: xsimd, Vc, highway
- Abstract over architectures
- Some overhead

### 4. Expression Templates (for libfixp)
- Generate optimal SIMD at compile time
- Type-safe
- Lazy evaluation for fusion

## Design Considerations for libfixp

### Abstraction Layer
```cpp
namespace simd {
    template<typename T, size_t Width> struct vec;

    // Specializations
    template<> struct vec<fix16, 8> {
        #if defined(__AVX2__)
        __m128i data;  // 8x 16-bit
        #elif defined(__ARM_NEON)
        int16x8_t data;
        #endif
    };
}
```

### Operation Selection
- Compile-time: best performance
- Runtime: CPU feature detection + dispatch

### Scalable Vector Support
- Must handle VLA for SVE/RISC-V V
- Cannot assume fixed vector length
- Use predication for variable-length

### Memory Alignment
- Aligned loads/stores faster
- alignas(32) or alignas(64) for AVX/AVX-512
- Unaligned fallback needed

## Key SIMD Fixed-Point Operations to Implement

1. **Vector arithmetic**: add, sub, mul (with Q adjustment)
2. **Saturating variants**: qadd, qsub
3. **Shifts**: logical, arithmetic (for Q conversion)
4. **Horizontal operations**: reduce_add, reduce_max
5. **Shuffles/permutes**: for interleaved data
6. **Conversions**: between Q formats, to/from float
7. **Transcendentals**: vectorized sin/cos/sqrt

## References

- [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html)
- [ARM NEON Intrinsics Reference](https://developer.arm.com/architectures/instruction-sets/intrinsics/)
- [ARM ACLE for SVE](https://arm-software.github.io/acle/main/acle.html)
