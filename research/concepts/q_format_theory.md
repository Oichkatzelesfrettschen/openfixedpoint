# Q-Format Fixed-Point Theory

## Overview

Q notation specifies parameters of binary fixed-point number format:
- How many bits for integer portion
- How many bits for fractional portion
- Whether there is a sign bit

## Notation Variants

### TI Notation (Qm.n)
- m = integer bits (excluding sign)
- n = fractional bits
- Total bits = 1 + m + n (signed) or m + n (unsigned)
- Example: Q15 means Q0.15 (16-bit signed, all fraction)

### ARM Notation
- Counts sign bit in m
- Q16.0 in ARM = Q15.0 in TI for 16-bit signed integer
- Unsigned formats identical across both

### Our Notation (libfixp)
Design decision needed: Which notation to follow?

## Common Formats

| Format | Word Size | Range | Resolution | Use Case |
|--------|-----------|-------|------------|----------|
| Q7 (Q0.7) | 8-bit | [-1, 0.9921875] | 2^-7 | Audio samples |
| Q15 (Q0.15) | 16-bit | [-1, 0.999969] | 2^-15 | DSP standard |
| Q16.16 | 32-bit | [-32768, 32767.9999847] | 2^-16 | Graphics |
| Q31 (Q0.31) | 32-bit | [-1, 0.9999999995] | 2^-31 | High-precision DSP |
| Q32.32 | 64-bit | [-2^31, 2^31-2^-32] | 2^-32 | Scientific |

## Full Permutation Matrix

For an N-bit word, valid Q(m.n) combinations where m + n = N (signed) or m + n = N (unsigned):

### 8-bit signed (Q formats)
- Q0.7, Q1.6, Q2.5, Q3.4, Q4.3, Q5.2, Q6.1, Q7.0

### 16-bit signed
- Q0.15 through Q15.0 (16 formats)

### 32-bit signed
- Q0.31 through Q31.0 (32 formats)

### 64-bit signed
- Q0.63 through Q63.0 (64 formats)

## Key Insight: Asymmetric Formats

Not all formats are equally useful:
- **Q0.n (full fractional)**: Values in [-1, 1), ideal for normalized signals
- **Qn.0 (full integer)**: Standard two's complement, no fractional precision
- **Balanced (Qn.n)**: Good compromise for general use
- **Asymmetric**: Application-specific (e.g., Q4.12 for coordinates)

## Arithmetic Properties

### Addition/Subtraction
- Operands must have same Q format
- Result has same Q format
- May overflow (policy needed)

### Multiplication
- Q(m1.n1) * Q(m2.n2) = Q(m1+m2.n1+n2)
- Double-width intermediate result
- Must shift right by n1 (or n2) and truncate for same-format result
- Example: Q15 * Q15 = Q30, shift right 15 to get Q15 result

### Division
- Q(m1.n1) / Q(m2.n2) requires careful handling
- Typically: pre-shift dividend left, divide, adjust result
- Newton-Raphson reciprocal preferred for performance

## Resolution and Error

Resolution of Q(m.n) = 2^(-n)

Maximum representable value = 2^m - 2^(-n)
Minimum representable value = -2^m (two's complement)

Quantization error: [-2^(-n-1), 2^(-n-1)] for rounding
                    [0, 2^(-n)] for truncation

## Design Decisions for libfixp

1. **Notation**: Use Qm.n with m excluding sign bit (TI-style)
2. **Template parameters**: `fixp<WordSize, IntBits, Signed=true>`
3. **Compile-time validation**: m + n must fit word size
4. **Runtime flexibility**: Allow format specification at runtime for some operations
