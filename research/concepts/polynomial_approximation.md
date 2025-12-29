# Polynomial Approximation Methods

## Overview

Polynomial approximations compute transcendental functions using only multiply-add operations. Key methods:
1. Taylor series
2. Chebyshev polynomials
3. Minimax (Remez) polynomials
4. Pade approximants

## Taylor Series

### Theory
f(x) = sum(f^(n)(0) * x^n / n!)

### Example: sin(x)
sin(x) = x - x^3/6 + x^5/120 - x^7/5040 + ...

### Pros
- Simple derivation
- Exact at expansion point

### Cons
- Error grows rapidly away from origin
- Slow convergence
- Not optimal for fixed-precision

## Chebyshev Polynomials

### Why Chebyshev
- Spread error uniformly over interval
- Near-minimax property
- Faster convergence than Taylor
- Can achieve 5 ULP accuracy for sin(x) with degree-7 polynomial

### Chebyshev Polynomials Tn(x)
- T0(x) = 1
- T1(x) = x
- Tn+1(x) = 2x*Tn(x) - Tn-1(x)

Or equivalently: Tn(cos(theta)) = cos(n*theta)

### Approximation Process
1. Sample function at Chebyshev nodes
2. Compute Chebyshev coefficients via discrete cosine transform
3. Truncate to desired degree
4. Optionally convert to power basis for evaluation

### Key Property
For smooth functions, Chebyshev truncation error is nearly uniform across interval (equiripple), unlike Taylor which blows up at edges.

## Minimax (Remez Algorithm)

### Goal
Find polynomial P that minimizes max|f(x) - P(x)| over interval

### Properties
- Optimal in infinity norm
- Equiripple error (alternating peaks)
- Chebyshev is good approximation to minimax

### Process
1. Start with Chebyshev approximation
2. Iteratively adjust coefficients
3. Exchange points where max error occurs
4. Converge to true minimax

### Tools
- Sollya: Open-source polynomial approximation tool
- Maple/Mathematica minimax functions
- Custom Remez implementations

## Fixed-Point Considerations

### Range Reduction
Before approximation, reduce argument to small range:
- sin/cos: reduce to [0, pi/4] using symmetry
- exp: reduce to [0, ln(2)] via exp(x) = 2^n * exp(r)
- log: reduce via log(m * 2^e) = log(m) + e*log(2)

### Coefficient Representation
- Coefficients may not be exactly representable in fixed-point
- Round coefficients, then re-optimize if needed
- Higher-order coefficients need more precision

### Evaluation Methods

#### Horner's Method
P(x) = a0 + x*(a1 + x*(a2 + x*(...)))
- Minimal multiplications: n multiplies for degree-n
- Sequential dependency chain

#### Estrin's Method
P(x) = (a0 + a1*x) + x^2*(a2 + a3*x) + x^4*(...)
- Parallelizable: compute terms independently
- Good for SIMD/superscalar

### Accuracy vs Polynomial Degree

| Function | Range | Degree | Max Error (ULP) |
|----------|-------|--------|-----------------|
| sin(x)   | [0,pi/4] | 5   | ~20             |
| sin(x)   | [0,pi/4] | 7   | ~5              |
| sin(x)   | [0,pi/4] | 9   | ~1              |
| cos(x)   | [0,pi/4] | 6   | ~5              |
| exp(x)   | [0,ln2]  | 6   | ~2              |
| log(x)   | [1,2]    | 7   | ~2              |

## Hybrid Approaches

### Table + Polynomial
1. Use lookup table for coarse approximation
2. Add polynomial correction for fine detail
3. Reduces polynomial degree needed

### Piecewise Polynomial
1. Divide range into segments
2. Different polynomial per segment
3. Table lookup selects coefficients

### CORDIC + Polynomial
1. Use CORDIC for primary computation
2. Polynomial for refinement or range reduction

## Numerical Stability

### Catastrophic Cancellation
- Avoid subtracting nearly-equal values
- Use identity transformations (e.g., 1-cos(x) via sin^2)

### Guard Bits
- Use extra precision internally
- Round only at final result

### Coefficient Ordering
- Sum smallest terms first when possible
- Consider compensated summation for high precision

## Design Decisions for libfixp

1. **Multiple accuracy levels**: Fast (degree 5), balanced (degree 7), precise (degree 9+)
2. **Pre-computed tables**: Coefficients for common Q formats
3. **Range reduction**: Built into function wrappers
4. **SIMD Estrin evaluation**: For vector paths
5. **Configurable**: User can provide custom coefficients
6. **Sollya integration**: For coefficient generation tooling
