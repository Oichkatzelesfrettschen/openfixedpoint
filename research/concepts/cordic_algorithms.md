# CORDIC Algorithms

## Overview

CORDIC (COordinate Rotation DIgital Computer) is a shift-and-add algorithm for computing:
- Trigonometric functions (sin, cos, tan, atan, atan2)
- Hyperbolic functions (sinh, cosh, tanh, atanh)
- Square root
- Logarithms and exponentials
- Vector magnitude
- Division and multiplication

## Why CORDIC for Fixed-Point

1. **Only shift-add operations**: No multiplier needed
2. **Iterative convergence**: 1 bit of precision per iteration
3. **Hardware efficient**: Minimal gates for FPGA/ASIC
4. **Unified algorithm**: Same core computes many functions

## Coordinate Systems

### Circular (m = 1)
- Computes: sin, cos, tan, atan, vector magnitude
- Elementary angles: atan(2^(-i))
- Gain factor K ~ 1.6467 (must scale input or output)

### Hyperbolic (m = -1)
- Computes: sinh, cosh, tanh, atanh, exp, ln, sqrt
- Elementary angles: atanh(2^(-i))
- Requires iteration repetition: 4, 13, 40, 121, ... (3k+1 sequence)

### Linear (m = 0)
- Computes: multiplication, division
- Simpler but less common

## Operating Modes

### Rotation Mode
- Input: angle Z (to rotate by)
- Process: Drive Z toward 0
- Output: Rotated vector (X', Y')
- Use: sin/cos from angle

### Vectoring Mode
- Input: Vector (X, Y)
- Process: Drive Y toward 0
- Output: Angle Z, magnitude in X
- Use: atan2, vector magnitude

## Unified CORDIC Algorithm (Walther, 1971)

```
for i = 0 to n-1:
    sigma = sign(Z) in rotation mode
    sigma = -sign(Y) in vectoring mode

    X' = X - m * sigma * Y * 2^(-i)
    Y' = Y + sigma * X * 2^(-i)
    Z' = Z - sigma * e[i]  // e[i] = elementary angle
```

## Function Computation

| Function | Mode | m | Init X | Init Y | Init Z | Result |
|----------|------|---|--------|--------|--------|--------|
| sin(a)   | Rot  | 1 | 1/K    | 0      | a      | Y      |
| cos(a)   | Rot  | 1 | 1/K    | 0      | a      | X      |
| atan(a)  | Vec  | 1 | 1      | a      | 0      | Z      |
| atan2(y,x)| Vec | 1 | x      | y      | 0      | Z      |
| sqrt(a)  | Vec  | -1| a+0.25 | a-0.25 | 0      | X      |
| ln(a)    | Vec  | -1| a+1    | a-1    | 0      | 2*Z    |
| exp(a)   | Rot  | -1| 1/K'   | 0      | a      | X+Y    |

## Implementation Considerations

### Lookup Tables
- Store elementary angles: e[i] = atan(2^(-i)) or atanh(2^(-i))
- Table size = number of iterations (typically 16-32)
- Each entry is Q format matching working precision

### Gain Factor (K)
- Circular: K = prod(sqrt(1 + 2^(-2i))) ~ 1.6467
- Must pre-scale input by 1/K or post-scale output
- Can be absorbed into table or initial value

### Convergence Zone
- Circular: |Z| <= sum(atan(2^(-i))) ~ 1.7433 rad (~99.88 degrees)
- Need range reduction for larger angles
- Hyperbolic requires iteration repetition for convergence

### Fixed-Point Precision
- Working precision should be >= output precision + guard bits
- Typically use 2-4 extra bits internally
- Final result is truncated/rounded

## Variants

### Radix-4 CORDIC
- Process 2 bits per iteration
- Faster but more complex logic
- Requires modified angle tables

### Redundant CORDIC
- Uses redundant number representation
- Eliminates carry propagation delay
- Good for high-speed hardware

### Scaling-Free CORDIC
- Eliminates need for K compensation
- Modified iteration equations
- Trades accuracy for simplicity

### GH-CORDIC (Generalized Hyperbolic)
- Computes log_b and b^x for arbitrary base b
- Extension of standard hyperbolic CORDIC

## CORDIC vs Polynomial Approximation

| Aspect | CORDIC | Polynomial |
|--------|--------|------------|
| Hardware | Minimal (shift-add) | Multiplier needed |
| Speed | Iterative (N cycles) | Parallel (few cycles) |
| Precision | Linear in iterations | Depends on degree |
| Flexibility | Many functions | One function per poly |
| Area | Small | Medium |

## Design Decisions for libfixp

1. **All three coordinate systems**: Circular, hyperbolic, linear
2. **Both modes**: Rotation and vectoring
3. **Configurable iterations**: Match precision to performance needs
4. **Range reduction**: Pre-computed for common ranges
5. **SIMD vectorization**: Batch CORDIC for throughput
6. **Template specialization**: Optimal tables per Q format
