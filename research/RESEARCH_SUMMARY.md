# libfixp Research Summary

## Executive Summary

This document synthesizes research findings to inform libfixp architecture decisions. The library aims to be a universal, clean-room fixed-point mathematics library supporting:
- All Q(m.n) format permutations
- C, C++, and OpenCL
- CPU (scalar, SIMD) and GPU
- Runtime-switchable overflow policies

---

## Key Research Findings

### 1. Q-Format Landscape

**Industry Standards:**
- Q15, Q31: DSP industry standard (CMSIS, TI, ADI)
- Q16.16: Graphics/games standard
- Q7: Low-precision audio, neural networks

**Full Permutation Reality:**
- Theoretically possible: Q(m.n) for any m+n <= word_size
- Practically useful: ~20% of combinations commonly used
- Template approach: Generate all, optimize common cases

**Recommendation:** Default to common formats (Q15, Q16.16, Q31), support full matrix via templates.

---

### 2. Overflow Semantics

**Existing Approaches:**
- ARM ACLE: Q flag (sticky saturation indicator), deprecated on A-profile
- x86 SIMD: Saturating intrinsics for 8/16-bit only
- C/C++: Signed overflow is UB, unsigned wraps

**Novel Opportunity:**
No existing library offers runtime-switchable overflow policy. This is a differentiating feature for libfixp.

**Recommendation:**
```
Compile-time default + Thread-local override + Per-operation override
```

---

### 3. Transcendental Functions

**CORDIC Advantages:**
- Shift-add only (no multiplier)
- Unified algorithm for many functions
- 1 bit/iteration convergence
- Ideal for hardware/FPGA

**Polynomial Advantages:**
- Fewer iterations on modern CPUs with fast multiply
- SIMD-friendly (Estrin's method)
- Tunable accuracy via degree

**CORDIC Variants:**
- Circular: sin, cos, tan, atan
- Hyperbolic: sinh, cosh, exp, ln, sqrt
- Linear: multiply, divide
- Radix-4: 2 bits/iteration

**Recommendation:** Provide both CORDIC and polynomial implementations. User selects based on hardware/accuracy needs.

---

### 4. SIMD Architecture

**Current State:**
- x86: SSE2 baseline, AVX2 common, AVX-512 on servers
- ARM: NEON ubiquitous, SVE/SVE2 emerging
- RISC-V: V extension gaining adoption

**Key Challenge:**
Scalable vectors (SVE, RISC-V V) require vector-length-agnostic (VLA) programming.

**Saturating Operations:**
- x86: Only for 8/16-bit (AVX-512BW)
- ARM: Full support in NEON/SVE

**Recommendation:**
Abstract SIMD layer with:
- Fixed-width backend (SSE, AVX, NEON)
- Scalable backend (SVE, RISC-V V)
- Fallback scalar implementation

---

### 5. GPU/OpenCL

**Challenges:**
- No native Q-format types
- Integer throughput often lower than float
- Branching penalty for saturation

**Opportunities:**
- Memory bandwidth (smaller types)
- FPGA via Intel OpenCL SDK
- Deterministic compute

**Recommendation:**
- OpenCL 1.2+ baseline
- Preprocessor-configurable Q format
- Software transcendentals (CORDIC/polynomial)
- Branchless saturation

---

### 6. Standards Compatibility

**Must Support:**
- ISO/IEC TR 18037 concepts ([-1,1) range, saturation)
- ARM ACLE patterns (saturation intrinsics)
- TI Q notation (Qm.n with implicit sign)
- IEEE 754 conversion

**Novel Extensions:**
- Runtime policy switching
- GPU portability
- Scalable vector support
- Universal Q-format matrix

---

### 7. Existing Library Gaps

| Gap | libfixp Opportunity |
|-----|---------------------|
| Single Q format only | Full Q(m.n) matrix |
| Compile-time policy | Runtime switching |
| No GPU support | OpenCL kernels |
| No SVE/RISC-V V | Scalable vector support |
| Limited accuracy options | Multiple implementations |

---

## Architecture Recommendations

### Type System

```cpp
// Core type: compile-time Q format
template<int IntBits, int FracBits, typename Storage = auto_storage_t<IntBits+FracBits>>
class fixp;

// Common aliases
using q15 = fixp<0, 15, int16_t>;
using q31 = fixp<0, 31, int32_t>;
using q16_16 = fixp<16, 16, int32_t>;

// Runtime Q format (for dynamic use)
class fixp_dynamic;
```

### Overflow Policy

```cpp
enum class overflow_policy { wrap, saturate, trap, undefined };

// Global default (compile-time)
template<overflow_policy P = overflow_policy::saturate>
class fixp;

// Thread-local override
void set_overflow_policy(overflow_policy p);

// Per-operation override
auto result = add<overflow_policy::wrap>(a, b);
```

### Platform Abstraction

```
libfixp/
  core/           - Portable C++ implementation
  simd/
    x86/          - SSE/AVX/AVX-512
    arm/          - NEON/SVE
    riscv/        - V extension
    generic/      - Scalar fallback
  gpu/
    opencl/       - OpenCL kernels
    vulkan/       - Future Vulkan compute
```

### API Layers

1. **C API** - Portable, linkable
2. **C++ API** - Templates, operators, type safety
3. **OpenCL API** - Kernel functions, preprocessor config

---

## Implementation Priority

### Phase 1: Foundation
1. Core Q15/Q16.16/Q31 types
2. Basic arithmetic (+, -, *, /)
3. Overflow policies (compile-time)
4. Float conversion
5. Scalar implementation

### Phase 2: Functions
1. CORDIC transcendentals
2. Polynomial transcendentals
3. Square root, reciprocal
4. Division (Newton-Raphson)

### Phase 3: SIMD
1. x86 SSE2/AVX2
2. ARM NEON
3. Fallback scalar

### Phase 4: Advanced
1. Runtime overflow switching
2. SVE/RISC-V V support
3. OpenCL kernels
4. Full Q-format matrix

### Phase 5: Polish
1. Comprehensive testing
2. Documentation
3. Benchmarks
4. Package management

---

## Open Questions

1. **Storage type selection**: Auto-detect or user-specified?
2. **Unsigned Q formats**: First-class or derived?
3. **128-bit support**: Via compiler extension or emulation?
4. **C API naming**: `fixp_add_q15()` or `q15_add()`?
5. **Header-only vs compiled**: Trade-offs?

---

## References

### Academic
- CORDIC: Volder (1959), Walther (1971)
- Chebyshev approximation: Lanczos (1952)
- Fixed-point quantization: Various ML papers (2020-2025)

### Standards
- ISO/IEC TR 18037:2008
- ARM ACLE (arm-software.github.io/acle)
- Intel Intrinsics Guide

### Libraries (Concept Reference Only)
- fpm, libfixmath, fixed_math, CMSIS-DSP
