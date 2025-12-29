# Numerical Analysis for Fixed-Point

## Error Sources in Fixed-Point Arithmetic

### 1. Quantization Error
When converting continuous values to fixed-point representation.

```
Quantization step: q = 2^(-n) for n fractional bits
Quantization error: e ∈ [-q/2, q/2] (rounding)
                    e ∈ [0, q] (truncation)
```

### 2. Round-off Error
Accumulates during computation as intermediate results are rounded.

```
After N operations with independent errors:
Expected error growth: O(√N) for random errors
Worst case: O(N) for correlated errors
```

### 3. Overflow/Underflow
When results exceed representable range.

- **Overflow**: Result too large → saturation or wrap
- **Underflow**: Result too small → rounds to zero

### 4. Precision Loss During Alignment
When combining values with different Q formats:

```
Q8.8 + Q4.12 requires alignment
Left-shifting may cause overflow
Right-shifting loses LSB precision
```

---

## Error Propagation Analysis

### Multiplication Error
```
True result: T = a × b
Fixed result: F = round((a + εa)(b + εb) >> n)

Error bound: |T - F| ≤ |a||εb| + |b||εa| + |εa||εb| + q/2
```

For Q15 × Q15 → Q15:
- Input error: ε ≤ 2^(-15)
- Multiplication error: ε × 2 ≈ 2^(-14)
- Rounding error: 2^(-16)
- Total: ~2^(-14)

### Addition Error
```
Error propagates directly:
(a + εa) + (b + εb) = (a + b) + (εa + εb)
```

### Division Error
Division amplifies errors:
```
(a + εa) / (b + εb) ≈ a/b + (εa/b - a×εb/b²)
```
Sensitive to small denominators!

---

## Stability Analysis

### Condition Number
Measures sensitivity of output to input perturbations:
```
κ = |f'(x)| × |x| / |f(x)|
```

High condition number → numerically unstable

### IIR Filter Stability
Poles must be inside unit circle. In fixed-point:
```
Pole z = r×e^(jω)
Quantized: z' = r'×e^(jω')

Stability margin shrinks with fewer bits
```

For high Q, narrow-band filters:
- Need extra precision (2×log2(fs/fc) bits)
- Or use double-precision state variables

---

## Stochastic Rounding

### Traditional Rounding
```
round(x) = floor(x + 0.5)
```
Biased: always rounds 0.5 up

### Stochastic Rounding
```
stoch_round(x) = floor(x) with probability (1 - frac(x))
               = ceil(x)  with probability frac(x)
```

### Benefits
- Unbiased in expectation
- Errors cancel over many operations
- Used in neural network training
- Better for iterative algorithms

### Implementation
```c
int16_t stoch_round_q15(int32_t x, uint16_t random) {
    uint16_t frac = x & 0x7FFF;  // Fractional part
    int16_t result = x >> 15;    // Integer part
    if (random < frac) result++; // Probabilistic round-up
    return result;
}
```

---

## Quantization Noise in DSP

### Signal-to-Quantization-Noise Ratio (SQNR)
For uniform quantization with b bits:
```
SQNR ≈ 6.02×b + 1.76 dB
```

| Bits | SQNR (dB) |
|------|-----------|
| 8 | 49.9 |
| 12 | 74.0 |
| 16 | 98.1 |
| 24 | 146.2 |
| 32 | 194.4 |

### Noise in Cascaded Systems
```
Total noise power = Σ (noise_i × gain_i²)
```

Gain stages amplify earlier quantization noise.

---

## Error Mitigation Strategies

### 1. Extended Precision Accumulator
Use wider accumulator for sums:
```c
int32_t acc = 0;  // Q31 accumulator
for (int i = 0; i < N; i++) {
    acc += (int32_t)a[i] * b[i];  // Q15 × Q15 = Q30
}
return (int16_t)(acc >> 15);  // Final round to Q15
```

### 2. Guard Bits
Extra LSBs to absorb rounding errors:
```
Working precision: Q16 + 4 guard bits = Q20 internal
Output precision: Q16
```

### 3. Block Normalization
Scale data per block to maximize precision:
```c
int16_t max_val = find_max(block);
int shift = count_leading_zeros(max_val) - 1;
scale_block(block, shift);
// Process with maximum precision
// Rescale output
```

### 4. Kahan Summation
Compensated summation for reduced error:
```c
q16_16 kahan_sum(q16_16* arr, int n) {
    q16_16 sum = 0, c = 0;
    for (int i = 0; i < n; i++) {
        q16_16 y = arr[i] - c;
        q16_16 t = sum + y;
        c = (t - sum) - y;  // Captures lost bits
        sum = t;
    }
    return sum;
}
```

---

## Formal Verification

### Static Analysis Approach
1. For each operation, compute error bound
2. Propagate bounds through program
3. Compare final bound to requirements

### Tools
- Frama-C with FLUCTUAT
- Gappa (error bound prover)
- MATLAB Fixed-Point Designer

### Tight Error Analysis (from Bemporad et al.)
Re-compute operations in higher precision:
```
error_bound = |high_prec_result - fixed_point_result|
```
Propagate through entire computation graph.

---

## Practical Guidelines

### Rule of Thumb: Word Length
For N-bit input precision with K operations:
```
Intermediate bits ≈ N + ceil(log2(K))
```

### Coefficient Quantization
Filter coefficients often need more bits than data:
```
Coefficient bits ≈ data bits + filter order/2
```

### Dynamic Range Requirements
```
Required bits = ceil(log2(max_value / min_value)) + precision_bits
```

---

## References

- [Tight Error Analysis in Fixed-Point](https://dl.acm.org/doi/full/10.1145/3524051)
- [Stochastic Rounding for Neural ODEs](https://royalsocietypublishing.org/doi/10.1098/rsta.2019.0052)
- [Fixed-Point DSP Lecture Notes](https://schaumont.dyn.wpi.edu/ece4703b21/lecture6.html)
