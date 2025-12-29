# Floating-Point and Fixed-Point Interoperability

## Overview

Fixed-point and floating-point serve different purposes. This document covers:
1. Converting between formats
2. Emulating float operations with fixed-point
3. Hybrid approaches

## Conversion: Float to Fixed

### Basic Conversion
```
fixed = (int)(float_val * (1 << frac_bits))
```

### With Rounding
```
fixed = (int)(float_val * (1 << frac_bits) + 0.5)  // Round to nearest
```

### Saturation
If float value exceeds fixed-point range, clamp to min/max.

### Special Values
| Float Value | Fixed-Point Handling |
|-------------|---------------------|
| NaN | Saturate to 0 or max (configurable) |
| +Inf | Saturate to max |
| -Inf | Saturate to min |
| Denormal | Treat as 0 or convert (precision loss) |

## Conversion: Fixed to Float

### Basic Conversion
```
float_val = (float)fixed / (1 << frac_bits)
```

### Precision Considerations
- 32-bit fixed may lose precision converting to 32-bit float (24-bit mantissa)
- 64-bit fixed should use double for lossless conversion

## Floating-Point Emulation via Fixed-Point

### Why Emulate?
- Processors without FPU (Cortex-M0, some RISC-V)
- Deterministic behavior requirements
- Energy efficiency
- FPGA implementations

### Soft-Float Libraries
Commercial and open-source options exist:
- GCC's soft-float (`-msoft-float`)
- SEGGER emFloat
- Berkeley SoftFloat

### Performance
- Software float is 10-100x slower than hardware FPU
- Fixed-point alternative often faster than soft-float
- Example: Pigweed project saw 2x speedup using fixed-point vs soft-float

### When to Use Each
| Use Case | Recommendation |
|----------|----------------|
| Signal processing | Fixed-point |
| Scientific computing | Hardware float or soft-float |
| ML inference | Fixed-point (INT8/INT16) |
| Financial | Fixed-point (exact decimal) |
| General embedded | Depends on FPU availability |

## IEEE 754 Format Reference

### Single Precision (FP32)
- Sign: 1 bit
- Exponent: 8 bits (bias 127)
- Mantissa: 23 bits (24 with implicit 1)
- Range: ~1.2e-38 to ~3.4e38
- Precision: ~7 decimal digits

### Half Precision (FP16)
- Sign: 1 bit
- Exponent: 5 bits (bias 15)
- Mantissa: 10 bits (11 with implicit 1)
- Range: ~6e-5 to 65504
- Precision: ~3 decimal digits

### Comparison with Fixed-Point

| Format | Range | Precision |
|--------|-------|-----------|
| FP32 | 1.2e-38 to 3.4e38 | 24 bits (variable) |
| Q16.16 | -32768 to 32767.99998 | 16 bits (fixed) |
| Q0.31 | -1 to 0.9999999995 | 31 bits (fixed) |

## Hybrid Approaches

### Dynamic Range Mapping
1. Analyze algorithm's value ranges
2. Choose Q format that covers range with sufficient precision
3. Use fixed-point for compute
4. Convert to float only for I/O

### Block Floating Point
- Share exponent across a block of values
- Mantissas stored as fixed-point
- Reduces dynamic range issues
- Used in audio codecs (AC-3)

### Posit Numbers (Research)
- Alternative to IEEE 754
- Tapered precision (most accurate near 1)
- May complement fixed-point in future

## libfixp Interop Design

### Conversion Functions
```cpp
// Template-based conversion
template<int IntBits, int FracBits>
fixp<IntBits, FracBits> from_float(float f);

template<int IntBits, int FracBits>
float to_float(fixp<IntBits, FracBits> x);

// Runtime Q format
fixp_any from_float(float f, int int_bits, int frac_bits);
```

### Options
- Rounding mode: truncate, nearest, up, down
- Overflow handling: saturate, wrap, trap
- Special value handling: configurable

### SIMD Conversion
- `_mm_cvtepi32_ps` (int to float)
- `_mm_cvttps_epi32` (float to int, truncate)
- Batch conversion for vectors

### OpenCL Conversion
- `convert_float4(int4)`
- `convert_int4_sat_rte(float4)` (saturate, round to even)

## Accuracy Verification

### Test Strategy
1. Generate float values across range
2. Convert to fixed-point
3. Perform operation
4. Convert back to float
5. Compare with reference float result
6. Measure max/avg error in ULPs

### Expected Errors
- Conversion: 0.5 ULP of fixed-point
- Operations: Depends on algorithm
- Transcendentals: Polynomial degree dependent
