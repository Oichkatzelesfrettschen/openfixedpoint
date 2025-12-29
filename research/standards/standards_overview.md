# Fixed-Point Standards Overview

## ISO/IEC TR 18037:2008 - Embedded C

### Scope
Technical Report specifying extensions to C for embedded processors:
1. Fixed-point arithmetic
2. Named address spaces
3. Basic I/O hardware addressing

### Fixed-Point Types
Defines `_Fract` and `_Accum` types:

| Type | Range | Typical Use |
|------|-------|-------------|
| short _Fract | [-1, 1) | Q7 equivalent |
| _Fract | [-1, 1) | Q15 equivalent |
| long _Fract | [-1, 1) | Q31 equivalent |
| short _Accum | Wider range | Accumulator |
| _Accum | Wider range | Accumulator |
| long _Accum | Wider range | Accumulator |

### Saturation
- Default: undefined behavior on overflow
- `_Sat` qualifier enables saturation
- Example: `_Sat _Fract x;`

### Literals
- `0.5r` - _Fract literal
- `0.5lr` - long _Fract literal
- `0.5k` - _Accum literal

### Compiler Support
- Very limited (GCC partial, few commercial compilers)
- Most developers use library implementations instead

### Key Concepts for libfixp
- Range [-1, 1) is fundamental for normalized signals
- Saturation as explicit qualifier
- Accumulator types for precision during computation

---

## ARM ACLE (Arm C Language Extensions)

### Relevant Features

#### Saturation Intrinsics
- `__ssat(val, n)` - Signed saturate to n bits
- `__usat(val, n)` - Unsigned saturate to n bits
- `__qadd(a, b)` - Saturating add (32-bit)
- `__qsub(a, b)` - Saturating subtract
- `__qdbl(x)` - Saturating double

#### Q Flag
- `__saturation_occurred()` - Read sticky Q bit
- `__set_saturation_occurred(val)` - Reset Q bit
- Deprecated on A-profile, supported M/R-profile

#### Feature Macros
- `__ARM_FEATURE_QBIT` - Q flag available
- `__ARM_FEATURE_SAT` - Saturation intrinsics available
- `__ARM_FEATURE_DSP` - DSP multiply-accumulate

### SVE/SVE2 Fixed-Point
- Scalable vector fixed-point operations
- Complex integer multiply-add with rotate
- svqadd, svqsub for saturating arithmetic

---

## TI DSP Conventions

### Q Notation (TI Style)
- Qm.n where m = integer bits, n = fractional bits
- Sign bit implicit (not counted in m)
- Q15 means Q0.15 (all fractional, 16-bit signed)

### Common Formats
- Q15: Standard 16-bit DSP
- Q31: Standard 32-bit DSP
- Q0.31: High precision accumulator
- Q4.12: 16-bit with 4 integer bits

### IQmath Library (TI-specific)
- Q format fixed-point for TI DSPs
- Provides standard math functions
- Highly optimized for TI hardware

---

## IEEE 754 (Floating-Point Reference)

### Why Relevant
- libfixp must convert to/from floating-point
- Understanding float representation aids conversion

### Key Formats
| Format | Sign | Exponent | Mantissa | Total |
|--------|------|----------|----------|-------|
| FP16 | 1 | 5 | 10 | 16 |
| FP32 | 1 | 8 | 23 | 32 |
| FP64 | 1 | 11 | 52 | 64 |

### Conversion Considerations
- Denormals may need special handling
- NaN/Inf map to saturation values in fixed-point
- Rounding mode affects conversion accuracy

---

## BLAS/LAPACK Conventions

### Relevance
- Matrix operations often use fixed-point in embedded
- Standard APIs provide interoperability model

### Concepts
- Row-major vs column-major storage
- Leading dimension parameters
- Alpha/beta scaling (relevant for Q format)

---

## Posit (Alternative Number Format)

### Overview
- Proposed replacement for IEEE 754
- Dynamic precision allocation
- Tapering accuracy at extremes

### Relevance to Fixed-Point
- Hybrid fixed/posit formats proposed
- Research area for libfixp extension

---

## Design Implications for libfixp

### Standard Compatibility Goals

1. **ISO/IEC TR 18037 Conceptual Alignment**
   - Support [-1, 1) normalized types
   - Saturation as explicit option
   - Accumulator pattern (wider intermediate)

2. **ARM ACLE Feature Parity**
   - Saturation intrinsic equivalents
   - Overflow detection mechanism
   - DSP operation patterns

3. **TI Q Notation Adoption**
   - Use Q(m.n) notation
   - Sign bit implicit in m
   - Standard Q15/Q31 as first-class citizens

4. **IEEE 754 Interop**
   - Clean conversion to/from float/double
   - Handle special values gracefully
   - Configurable rounding

5. **Novel Extensions**
   - Runtime policy switching (not in any standard)
   - GPU portability (not standardized)
   - Scalable vector support (beyond ARM ACLE)

### API Naming Conventions

Consider alignment with:
- C standard library (`sin`, `cos`, `sqrt`)
- ACLE patterns (`__qadd` style for intrinsics)
- C++ conventions (operator overloading, templates)
