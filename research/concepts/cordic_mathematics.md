# CORDIC: Mathematical Foundations and Convergence Proofs

## Theoretical Foundation

### Core Principle: Rotation by Pseudo-Rotations

A true rotation in 2D by angle theta:
```
[x']   [cos(theta) -sin(theta)] [x]
[y'] = [sin(theta)  cos(theta)] [y]
```

CORDIC replaces this with a sequence of "pseudo-rotations":
```
[x']   [1      -sigma*tan(alpha_i)] [x]
[y'] = [sigma*tan(alpha_i)    1   ] [y]
```

Where:
- `sigma_i = +1 or -1` (rotation direction)
- `alpha_i = arctan(2^(-i))` (fixed angles)

### Key Insight: tan(alpha_i) = 2^(-i)

By choosing angles where tangent equals powers of 2:
```
tan(alpha_i) = 2^(-i)
alpha_i = arctan(2^(-i))
```

The multiplication by tan(alpha_i) becomes a **bit shift**:
```
x_new = x - sigma * (y >> i)
y_new = y + sigma * (x >> i)
```

---

## Convergence Theorem

### Theorem 1: CORDIC Angle Convergence

**Statement**: For any target angle theta in the range [-pi/2, pi/2], the CORDIC algorithm converges to theta within 1 ULP after n iterations, where n equals the number of fractional bits.

**Proof**:

Define:
- z_0 = theta (target angle)
- z_{i+1} = z_i - sigma_i * alpha_i
- sigma_i = sign(z_i)

The total rotation after n iterations is:
```
Sigma(i=0 to n-1) sigma_i * alpha_i
```

**Key Identity**: The sum of all positive CORDIC angles:
```
Sum(i=0 to n-1) alpha_i = Sum(i=0 to n-1) arctan(2^(-i))
```

**Lemma 1.1**: For i >= 1:
```
arctan(2^(-i)) < arctan(2^(-(i-1)))
```
And specifically:
```
arctan(2^(-i)) < Sum(j=i+1 to infinity) arctan(2^(-j))
```

This means each angle is smaller than the sum of all remaining angles, allowing the algorithm to "undo" any overshoot.

**Proof of Lemma 1.1**:
```
Using: arctan(x) = x - x^3/3 + x^5/5 - ...
For small x: arctan(x) ≈ x

Sum(j=i+1 to infinity) 2^(-j) = 2^(-i)
```

Therefore each angle can be compensated by subsequent angles.

**Error Bound**:
After n iterations, the angle error is:
```
|z_n| <= alpha_{n-1} = arctan(2^(-(n-1)))
```

For n fractional bits, this is less than 1 ULP in the angle representation.

---

## Scaling Factor Analysis

### The CORDIC Gain K

Each pseudo-rotation scales the vector by:
```
k_i = sqrt(1 + tan^2(alpha_i)) = sqrt(1 + 2^(-2i)) = 1/cos(alpha_i)
```

The cumulative scaling factor after n iterations:
```
K_n = Product(i=0 to n-1) k_i = Product(i=0 to n-1) sqrt(1 + 2^(-2i))
```

### Theorem 2: K Converges

**Statement**: K_n converges as n -> infinity:
```
K_infinity ≈ 1.6467602581210656483...
```

**Proof**:
```
ln(K_n) = (1/2) * Sum(i=0 to n-1) ln(1 + 2^(-2i))

Using Taylor: ln(1+x) ≈ x - x^2/2 + x^3/3 - ...

For large i: ln(1 + 2^(-2i)) ≈ 2^(-2i)

Sum converges because 2^(-2i) forms geometric series with ratio 1/4.
```

**First 15 values of K_n**:
| n | K_n |
|---|-----|
| 1 | 1.41421356 |
| 2 | 1.58113883 |
| 3 | 1.62980060 |
| 4 | 1.64248407 |
| 5 | 1.64568892 |
| 10 | 1.64676025 |
| 15 | 1.64676026 |

### Pre-Scaling Options

1. **Pre-divide by K**: x_0 = x/K, y_0 = y/K
2. **Post-divide by K**: x_final = x_n/K, y_final = y_n/K
3. **Initialize with 1/K**: For sine/cosine, start with x_0 = 1/K

For Q16.16:
```c
#define CORDIC_K_INV  0x9B74  // 1/K in Q0.16 ≈ 0.6072529...
```

---

## Mode-Specific Proofs

### Rotation Mode

**Goal**: Rotate vector (x_0, y_0) by angle z_0

**Invariant**: At each iteration:
```
x_i^2 + y_i^2 = K_i^2 * (x_0^2 + y_0^2)
arctan(y_i/x_i) = arctan(y_0/x_0) + sum of applied rotations
```

**Termination**: When z_n ≈ 0:
```
[x_n]   [cos(z_0) -sin(z_0)] [x_0]
[y_n] = K * [sin(z_0)  cos(z_0)] [y_0]
```

### Vectoring Mode

**Goal**: Rotate vector to positive x-axis, accumulate angle

**Decision**: sigma_i = -sign(y_i) (opposite to rotation mode)

**Invariant**:
```
x_i^2 + y_i^2 = K_i^2 * (x_0^2 + y_0^2)
z_i = z_0 + arctan(y_0/x_0) - arctan(y_i/x_i)
```

**Termination**: When y_n ≈ 0:
```
z_n = z_0 + arctan(y_0/x_0)
x_n = K * sqrt(x_0^2 + y_0^2)
```

---

## Hyperbolic CORDIC

### Coordinate System Transformation

Replace circular rotations with hyperbolic "rotations":
```
[x']   [1      sigma*tanh(alpha_i)] [x]
[y'] = [sigma*tanh(alpha_i)    1  ] [y]
```

With angles:
```
alpha_i = arctanh(2^(-i))
```

### Critical Difference: Iteration Indices

**Warning**: Hyperbolic CORDIC does NOT converge with indices 0, 1, 2, 3, ...

**Theorem 3**: Hyperbolic CORDIC requires repeated iterations:
```
i = 1, 2, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, ...
```

**Pattern**: Repeat iteration at i = 4, 13, 40, 121, ... (i_{k+1} = 3*i_k + 1)

**Proof sketch**:
For hyperbolic CORDIC, convergence requires:
```
alpha_i < Sum(j=i+1 to infinity) alpha_j + alpha_{repeat}
```

Without repetition, arctanh angles don't satisfy this bound.

### Hyperbolic Scaling Factor

```
K_hyp = Product sqrt(1 - 2^(-2i)) for repeated index set
K_hyp ≈ 0.8281593609602...
```

---

## Error Analysis

### Quantization Error Per Iteration

For n-bit fixed-point representation:

1. **Shift truncation**: y >> i loses LSBs
   - Error bound: |e_shift| <= 2^(-n)

2. **Angle quantization**: alpha_i stored as fixed-point
   - Error bound: |e_angle| <= 2^(-n)

### Error Propagation

**Theorem 4**: After N CORDIC iterations with n-bit arithmetic:

```
|x_error| <= N * 2^(-n+1) * |x_0|
|y_error| <= N * 2^(-n+1) * |y_0|
|z_error| <= N * 2^(-n) radians
```

**Proof**:
Each iteration introduces at most 2 rounding errors (x and y shifts).
These accumulate linearly in worst case.

**Practical bound** (statistical):
```
|error| ≈ sqrt(N) * 2^(-n) (random walk)
```

### Reducing Error

1. **Guard bits**: Use (n+g) bits internally, round to n at end
2. **Rounding**: Round shifts instead of truncate: (y + (1 << (i-1))) >> i
3. **More iterations**: Accuracy improves roughly 1 bit per iteration

---

## Angle Range Extension

### Problem: Basic CORDIC is Limited

Direct CORDIC only handles:
- Circular: |theta| <= Sum(arctan(2^(-i))) ≈ 1.7433... radians ≈ 99.88 degrees

### Solution: Angle Reduction

**Theorem 5**: Any angle can be reduced to CORDIC range.

For circular functions:
```
sin(theta) = sin(theta mod 2*pi)  // Periodicity
sin(theta) = sin(pi - theta)      // Reflection
sin(-theta) = -sin(theta)         // Odd function
```

**Pre-processing algorithm**:
```c
// Reduce to [0, 2*pi]
theta = theta mod (2 * PI);

// Reduce to [0, pi]
if (theta > PI) {
    theta = theta - PI;
    negate_result = true;
}

// Reduce to [0, pi/2]
if (theta > PI/2) {
    theta = PI - theta;
}

// Now theta in [0, pi/2] ≈ [0, 1.5708], within CORDIC range
```

---

## Special Functions via CORDIC

### Sine and Cosine

**Rotation mode** with x_0 = 1/K, y_0 = 0, z_0 = theta:
```
x_n = cos(theta)
y_n = sin(theta)
```

### Arctangent

**Vectoring mode** with z_0 = 0:
```
z_n = arctan(y_0/x_0)
```

### Magnitude and Phase (Polar Conversion)

**Vectoring mode**:
```
x_n = K * sqrt(x_0^2 + y_0^2)  // Magnitude
z_n = arctan(y_0/x_0)          // Phase
```

### Hyperbolic Functions

**Hyperbolic rotation mode**:
```
x_n = K_hyp * cosh(z_0)
y_n = K_hyp * sinh(z_0)
```

### Exponential

Since exp(z) = cosh(z) + sinh(z):
```
exp(z) = x_n + y_n = K_hyp * (cosh(z) + sinh(z))
```

### Logarithm

**Hyperbolic vectoring mode** with x_0 = a+1, y_0 = a-1:
```
z_n = (1/2) * ln(a)
```
Because tanh(z) = (a-1)/(a+1) when a = exp(2z).

### Square Root

Using:
```
sqrt(a) = sqrt((a+1/4)/(1)) via CORDIC
```

**Linear CORDIC** or **hyperbolic vectoring** with:
```
x_0 = a + 0.25
y_0 = a - 0.25
x_n = K_hyp * sqrt(x_0^2 - y_0^2) = K_hyp * sqrt(a)
```

---

## Fixed-Point Implementation Details

### Q16.16 Angle Table

```c
// arctan(2^(-i)) in Q16.16 (scaled to represent radians)
const int32_t cordic_angles[16] = {
    0x0000C90F,  // arctan(1)    = 0.7854... = pi/4
    0x000076B1,  // arctan(1/2)  = 0.4636...
    0x00003EB6,  // arctan(1/4)  = 0.2449...
    0x00001FD5,  // arctan(1/8)  = 0.1244...
    0x00000FFE,  // arctan(1/16) = 0.0624...
    0x000007FF,  // arctan(1/32) = 0.0312...
    0x00000400,  // arctan(1/64) = 0.0156...
    0x00000200,  // arctan(1/128)
    0x00000100,  // arctan(1/256)
    0x00000080,  // arctan(1/512)
    0x00000040,  // arctan(1/1024)
    0x00000020,  // arctan(1/2048)
    0x00000010,  // arctan(1/4096)
    0x00000008,  // arctan(1/8192)
    0x00000004,  // arctan(1/16384)
    0x00000002,  // arctan(1/32768)
};
```

### Convergence Guarantee for Q16.16

With 16 fractional bits and 16 iterations:
- Angle error < arctan(2^(-15)) < 0.00003 radians
- Result accuracy: ~15-16 bits

### Bit-Exact Reference Implementation

```c
// Returns (cos, sin) in Q16.16
void cordic_sincos_q16_16(int32_t angle, int32_t* cos_out, int32_t* sin_out) {
    // Pre-scaled by 1/K
    int32_t x = 0x00009B74;  // 1/K in Q16.16
    int32_t y = 0;
    int32_t z = angle;

    for (int i = 0; i < 16; i++) {
        int32_t x_new, y_new;

        if (z >= 0) {
            // Rotate counter-clockwise
            x_new = x - (y >> i);
            y_new = y + (x >> i);
            z -= cordic_angles[i];
        } else {
            // Rotate clockwise
            x_new = x + (y >> i);
            y_new = y - (x >> i);
            z += cordic_angles[i];
        }

        x = x_new;
        y = y_new;
    }

    *cos_out = x;
    *sin_out = y;
}
```

---

## Radix-4 CORDIC: Mathematical Basis

### Extended Angle Set

In radix-4, at each macro-iteration we can rotate by:
- 0 * alpha_i
- +1 * alpha_i
- -1 * alpha_i
- +2 * alpha_i (two elementary rotations)
- -2 * alpha_i

### Theorem 6: Radix-4 Halves Iterations

**Statement**: Radix-4 CORDIC achieves n-bit accuracy in n/2 iterations.

**Proof**:
Each radix-4 step effectively performs 2 radix-2 steps.
The decision logic uses 2 bits of z to select sigma in {-2, -1, 0, +1, +2}.

**Trade-off**: More complex decision logic, but fewer iterations.

---

## Convergence Speed Comparison

| Method | Iterations for n bits | Multiplies | Adds/Shifts |
|--------|----------------------|------------|-------------|
| Radix-2 CORDIC | n | 0 | 2n |
| Radix-4 CORDIC | n/2 | 0 | ~1.5n |
| Polynomial (Horner) | k terms | k | k |
| Taylor 5th order | - | 5 | 5 |

For Q16.16 (16-bit fraction):
- Radix-2: 16 iterations
- Radix-4: 8 iterations
- Taylor 5th: 5 multiplies (but restricted range)

---

## Precision Limits

### Theorem 7: Fundamental Precision Bound

For n-bit fixed-point CORDIC:
```
Maximum achievable accuracy ≈ (n - log2(N)) bits
```

Where N is iteration count.

**Interpretation**:
- 16-bit CORDIC with 16 iterations: ~12-14 bits accuracy
- 32-bit CORDIC with 32 iterations: ~27-29 bits accuracy

### Improving Precision

1. **Double iterations**: Use 2n iterations for n-bit accuracy
2. **Guard bits**: Internal (n+4) bits, output n bits
3. **Error compensation**: Track and compensate accumulated error

---

## References

- Volder, J.E. (1959). "The CORDIC Trigonometric Computing Technique"
- Walther, J.S. (1971). "A Unified Algorithm for Elementary Functions"
- Hu, Y.H. (1992). "CORDIC-Based VLSI Architectures for Digital Signal Processing"
- Andraka, R. (1998). "A Survey of CORDIC Algorithms for FPGA-Based Computers"
- Muller, J.-M. (2006). "Elementary Functions: Algorithms and Implementation" (Ch. 6)
