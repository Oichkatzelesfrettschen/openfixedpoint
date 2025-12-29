# SIMD Deep Dive: AVX-512, SVE2, RISC-V V

## Intel AVX-512

### Architecture Overview
- 512-bit vectors (64 x 8-bit, 32 x 16-bit, 16 x 32-bit, 8 x 64-bit)
- 32 vector registers (zmm0-zmm31)
- 8 mask registers (k0-k7) for predication
- Multiple sub-extensions (F, BW, DQ, VL, etc.)

### Fixed-Point Relevant Extensions

#### AVX-512BW (Byte & Word)
Required for 8/16-bit saturating operations:
- `_mm512_adds_epi8/16` - Signed saturating add
- `_mm512_adds_epu8/16` - Unsigned saturating add
- `_mm512_subs_epi8/16` - Signed saturating subtract
- `_mm512_subs_epu8/16` - Unsigned saturating subtract

#### AVX-512VNNI (Neural Network)
- `_mm512_dpbusd_epi32` - Multiply uint8 by int8, accumulate to int32
- Optimized for ML inference

### Fixed-Point Multiplication Pattern
```cpp
// Q15 x Q15 -> Q15 (conceptual)
__m512i mul_q15(__m512i a, __m512i b) {
    // Expand to 32-bit
    __m512i a_lo = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(a, 0));
    __m512i b_lo = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(b, 0));
    __m512i prod_lo = _mm512_mullo_epi32(a_lo, b_lo);
    // Shift right by 15, pack back
    prod_lo = _mm512_srai_epi32(prod_lo, 15);
    // Similar for high half, then pack
    return _mm512_packs_epi32(prod_lo, prod_hi);
}
```

### AVX-512 vs AVX10
- AVX10.1 released Q3 2024 (Granite Rapids)
- Unified extension, replaces fragmented AVX-512 sub-extensions
- AVX10.2 coming with Diamond Rapids/Nova Lake

---

## ARM SVE2

### Scalable Vector Extension
- Vector length agnostic (VLA): 128-2048 bits
- Predicate registers for per-element masking
- Hardware determines vector length at runtime

### DSP Operations in SVE2
- **SQADD/SQSUB**: Signed saturating add/subtract
- **UQADD/UQSUB**: Unsigned saturating add/subtract
- **SQDMULH**: Signed saturating doubling multiply high
- **SQRDMULH**: Signed saturating rounding doubling multiply high

### Fixed-Point Complex Arithmetic
- **CMLA**: Complex integer multiply-add with rotate
- Widening/narrowing operations (Top/Bottom)

### SVE Intrinsics Pattern
```c
#include <arm_sve.h>

svint16_t add_q15_sve(svint16_t a, svint16_t b, svbool_t pg) {
    return svqadd_s16(a, b);  // Saturating add
}

svint16_t mul_q15_sve(svint16_t a, svint16_t b, svbool_t pg) {
    return svqdmulh_s16(a, b);  // Saturating doubling multiply high
}
```

### VLA Programming Model
```c
void process_q15(int16_t* out, const int16_t* a, const int16_t* b, size_t n) {
    size_t i = 0;
    svbool_t pg = svwhilelt_b16(i, n);
    do {
        svint16_t va = svld1_s16(pg, a + i);
        svint16_t vb = svld1_s16(pg, b + i);
        svint16_t vc = svqadd_s16(va, vb);
        svst1_s16(pg, out + i, vc);
        i += svcnth();  // Vector length in halfwords
        pg = svwhilelt_b16(i, n);
    } while (svptest_any(svptrue_b16(), pg));
}
```

---

## RISC-V V Extension

### Architecture
- Scalable vector length (VLEN: 32-65536 bits)
- Configurable element width (SEW: 8, 16, 32, 64 bits)
- LMUL for grouping multiple vector registers

### Fixed-Point CSRs
- **vxrm**: Vector fixed-point rounding mode
- **vxsat**: Saturation flag (sticky bit)
- **vcsr**: Combined status register

### Fixed-Point Operations
- **vsadd/vsaddu**: Saturating add (signed/unsigned)
- **vssub/vssubu**: Saturating subtract
- **vclip**: Clipping instructions (vclipb, vcliph, vclipw)
- **vaaddu/vaadd**: Averaging add
- **vsmul**: Signed saturating and rounding fractional multiply

### Rounding Modes (vxrm)
| Mode | Description |
|------|-------------|
| 0 | Round to nearest, ties to even |
| 1 | Round to nearest, ties to max |
| 2 | Round down (floor) |
| 3 | Round to odd |

### Example Assembly
```asm
# Q15 saturating add
vsetvli t0, a0, e16, m1    # Set 16-bit elements
vle16.v v1, (a1)           # Load vector a
vle16.v v2, (a2)           # Load vector b
vsadd.vv v3, v1, v2        # Saturating add
vse16.v v3, (a3)           # Store result
```

### P Extension (Packed SIMD)
- Alternative to V for smaller cores
- SIMD within 32/64-bit integer registers
- May be reworked for fixed-point focus

---

## Comparison Table

| Feature | AVX-512BW | SVE2 | RISC-V V |
|---------|-----------|------|----------|
| Max Width | 512 bits | 2048 bits | 65536 bits |
| Fixed Width | Yes | No (VLA) | No (VLA) |
| Predication | Mask registers | Predicate regs | Mask (tail/inactive) |
| Sat. Add 8-bit | Yes | Yes | Yes |
| Sat. Add 16-bit | Yes | Yes | Yes |
| Sat. Mul Q15 | Manual | SQDMULH | vsmul |
| Sat. Flag | No | No | vxsat |
| Rounding Control | No | Limited | vxrm |

---

## libfixp SIMD Strategy

### Abstraction Layer
```cpp
namespace simd {
    // Type-level vector width selection
    template<typename T, size_t N> struct vec;

    // Backend-specific specializations
    #if defined(__AVX512BW__)
    template<> struct vec<q15, 32> { __m512i data; };
    #elif defined(__ARM_FEATURE_SVE2)
    template<> struct vec<q15, dynamic> { svint16_t data; };
    #endif
}
```

### Operation Dispatch
```cpp
template<typename Policy>
auto add(vec<q15> a, vec<q15> b) {
    if constexpr (Policy == Saturate) {
        #if AVX512BW
        return _mm512_adds_epi16(a.data, b.data);
        #elif SVE2
        return svqadd_s16(a.data, b.data);
        #elif RISCV_V
        return vsadd(a.data, b.data);
        #endif
    }
}
```

### Fallback Chain
1. AVX-512BW / SVE2 / RVV (native)
2. AVX2 / NEON (narrower)
3. Scalar (portable)

---

## References

- [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html)
- [ARM ACLE for SVE](https://arm-software.github.io/acle/main/acle.html)
- [RISC-V V Extension Spec](https://github.com/riscvarchive/riscv-v-spec)
- [ARM SVE2 Introduction](https://developer.arm.com/documentation/102340/latest/)
