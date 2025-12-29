# Number Format Comparison: Q-Format vs Alternatives

## Executive Summary

This document provides detailed performance, precision, and use-case comparisons between:
1. Q-format fixed-point (traditional)
2. IEEE 754 floating-point
3. Posit/Unum
4. Logarithmic Number System (LNS)
5. Block Floating-Point (BFP)
6. Residue Number System (RNS)

---

## Format Characteristics

### Q-Format Fixed-Point

| Property | Value |
|----------|-------|
| Representation | Integer scaled by implicit 2^(-n) |
| Range | [-2^(m-1), 2^(m-1) - 2^(-n)] for Qm.n |
| Resolution | Uniform: 2^(-n) everywhere |
| Overflow | Explicit (saturation or wrap) |
| Zero | Exact |
| Denormals | N/A (no concept) |

**Strengths**:
- Exact addition/subtraction (no rounding in same Q)
- Deterministic timing
- SIMD-friendly (integer ops)
- No special values (NaN, Inf)

**Weaknesses**:
- Limited dynamic range
- Manual scaling required
- Precision/range trade-off fixed at design time

### IEEE 754 Floating-Point

| Property | Float32 | Float64 |
|----------|---------|---------|
| Total bits | 32 | 64 |
| Sign | 1 | 1 |
| Exponent | 8 | 11 |
| Mantissa | 23 | 52 |
| Range | ~1.2e-38 to 3.4e38 | ~2.2e-308 to 1.8e308 |
| Precision | ~7 decimal digits | ~16 decimal digits |

**Strengths**:
- Huge dynamic range
- Hardware support ubiquitous
- Automatic scaling
- IEEE standardized behavior

**Weaknesses**:
- Non-deterministic on some platforms
- Variable precision across range
- NaN propagation complexity
- Rounding errors accumulate

### Posit (Type III Unum)

| Property | Posit<32,2> | Posit<16,1> |
|----------|-------------|-------------|
| Total bits | 32 | 16 |
| es (exponent bits) | 2 | 1 |
| Max value | ~1.4e17 | 4096 |
| Min positive | ~7.5e-18 | ~2.4e-4 |
| Zero | Exact (all zeros) |
| NaR | Single (all ones) |

**Strengths**:
- Tapered precision (best near 1.0)
- Single exception value (NaR vs NaN/Inf)
- Better accuracy-per-bit than float
- Bit-exact reproducibility

**Weaknesses**:
- Variable-length decode
- No hardware (yet)
- Conversion overhead
- Lower precision at extremes

### Logarithmic Number System (LNS)

```
Value = sign * 2^(integer.fraction)
```

| Property | LNS-32 |
|----------|--------|
| Representation | Sign + fixed-point log |
| Multiplication | Add exponents (fast!) |
| Addition | Lookup table (slow) |
| Range | Enormous (exponential) |

**Strengths**:
- Multiplication = addition
- Division = subtraction
- Huge dynamic range
- Good for multiplicative chains

**Weaknesses**:
- Addition requires lookup/interpolation
- Complex conversion to/from linear
- Memory for tables

### Block Floating-Point (BFP)

```
Block of N values share one exponent:
values[i] = mantissa[i] * 2^(shared_exponent)
```

| Property | BFP-16 (block of 16) |
|----------|---------------------|
| Storage | 16 × 8-bit mantissa + 1 × 8-bit exp |
| Bits/value | 8.5 average |
| Range | Limited by shared exponent |
| Precision | 8 bits within block |

**Strengths**:
- Compact storage
- Simple SIMD operations
- Good for correlated data
- Used in MX format (AI)

**Weaknesses**:
- Precision loss if block values vary widely
- Block alignment overhead
- Extra exponent management

---

## Performance Benchmarks

### Operation Latency (CPU Cycles, Modern x86-64)

| Operation | Q16.16 | Q32.32 | Float32 | Float64 | Posit32 |
|-----------|--------|--------|---------|---------|---------|
| Add | 1 | 1 | 3-4 | 3-4 | 15-25 |
| Sub | 1 | 1 | 3-4 | 3-4 | 15-25 |
| Mul | 3-4 | 5-10 | 4-5 | 4-5 | 30-50 |
| Div | 15-25 | 30-50 | 10-15 | 15-20 | 50-100 |
| Sqrt | 20-40 | 40-80 | 12-18 | 18-25 | 80-150 |
| Sin | 50-100 | 100-200 | 50-100 | 80-150 | 150-300 |

**Notes**:
- Fixed-point division and transcendentals are software
- Posit numbers software-only (no hardware)
- Float has dedicated FPU hardware

### Throughput (Operations/Second, Single Core @ 3GHz)

| Operation | Q16.16 | Float32 | Posit32 |
|-----------|--------|---------|---------|
| Add | ~3B | ~1B | ~150M |
| Mul | ~1B | ~800M | ~60M |
| FMA | N/A | ~1B | ~40M |
| Div | ~120M | ~200M | ~30M |

### SIMD Throughput (AVX-512, 512-bit vectors)

| Type | Elements/Vector | Adds/Cycle | Muls/Cycle |
|------|-----------------|------------|------------|
| Q7 | 64 | 64 | ~32 |
| Q15 | 32 | 32 | 32 |
| Q31 | 16 | 16 | 8 |
| Float32 | 16 | 16 | 16 |
| Float64 | 8 | 8 | 8 |
| Posit32 | N/A | N/A | N/A |

### Memory Bandwidth Efficiency

| Format | Bits | Elements/Cache Line (64B) | Bandwidth for 1M ops |
|--------|------|---------------------------|---------------------|
| Q7 | 8 | 64 | 1 MB |
| Q15 | 16 | 32 | 2 MB |
| Q31 | 32 | 16 | 4 MB |
| Float16 | 16 | 32 | 2 MB |
| Float32 | 32 | 16 | 4 MB |
| Float64 | 64 | 8 | 8 MB |

---

## Precision Analysis

### Bits of Precision Across Range

#### Q16.16 (32-bit total)
```
Range: [-32768, +32767.99998]
Precision: 16 bits EVERYWHERE (uniform)
Resolution: 0.0000153 (2^-16)
```

#### Float32
```
Near 1.0: 23 bits mantissa
Near 1e6: 23 bits mantissa
Near 1e-6: 23 bits mantissa
(Relative precision constant, absolute varies)
```

#### Posit<32,2>
```
Near 1.0: ~28 bits effective
Near 256: ~20 bits
Near 1e6: ~10 bits
Near 0.001: ~20 bits
(Tapered: best around 1, degrades at extremes)
```

### Precision vs. Dynamic Range Trade-off

| Format | Dynamic Range (dB) | Min Precision (bits) | Max Precision (bits) |
|--------|-------------------|---------------------|---------------------|
| Q8.8 | 48 | 8 | 8 |
| Q16.16 | 96 | 16 | 16 |
| Q32.32 | 192 | 32 | 32 |
| Float16 | ~78 | 10 | 10 |
| Float32 | ~276 | 23 | 23 |
| Posit16 | ~130 | 0 | ~13 |
| Posit32 | ~278 | 0 | ~28 |

### Error Accumulation

#### Summing 10000 Values = 0.0001 Each

| Format | Exact Sum | Computed Sum | Absolute Error |
|--------|-----------|--------------|----------------|
| Q16.16 | 1.0 | 1.0 | 0.0 (exact) |
| Q8.8 | 1.0 | 0.996 | 0.004 (truncation) |
| Float32 | 1.0 | 0.99999994 | 6e-8 |
| Float64 | 1.0 | 1.0000000000... | ~1e-16 |

**Analysis**: Fixed-point with sufficient bits gives EXACT sums (no rounding in add). Float accumulates rounding errors.

### Multiplication Error Chains

#### Computing x^100 for x = 0.99

| Format | Exact Result | Computed | Relative Error |
|--------|--------------|----------|----------------|
| Q0.32 | 0.366 | 0.366 | ~10^-9 |
| Float32 | 0.366 | 0.366000 | ~10^-7 |
| Float64 | 0.366 | 0.366032... | ~10^-15 |
| Posit32 | 0.366 | 0.366 | ~10^-8 |

---

## Domain-Specific Recommendations

### Audio/DSP

**Winner: Q15/Q31**

| Criterion | Q15 | Float32 | Rationale |
|-----------|-----|---------|-----------|
| SNR for 16-bit audio | 96dB | 96dB | Both sufficient |
| Filter stability | Excellent | Good | Fixed coefficients predictable |
| MAC throughput | Higher | Lower | Integer MAC faster |
| Memory efficiency | 2 bytes | 4 bytes | Half memory bandwidth |
| Determinism | Perfect | Good | No FP environment |

**Typical Audio Pipeline**:
```
Input: 16-bit PCM (Q15)
Processing: Q31 accumulator
Output: Q15 or Q23 (24-bit DAC)
```

### 3D Graphics

**Winner: Float32 (GPU) or Q16.16 (retro/embedded)**

| Criterion | Q16.16 | Float32 | Rationale |
|-----------|--------|---------|-----------|
| Transform range | Limited | Huge | World coordinates |
| Matrix precision | Adequate | Good | 16 frac bits OK |
| GPU support | Poor | Native | All GPUs float |
| Embedded/retro | Native | Requires FPU | Q excels here |

**Modern GPU**: Float32 is the only choice
**Retro/Embedded 3D**: Q16.16 with careful range management

### Machine Learning Inference

**Winner: Q8/INT8 or BFP (MX format)**

| Criterion | INT8 | Float16 | Float32 | Rationale |
|-----------|------|---------|---------|-----------|
| Throughput | 4x | 2x | 1x | Tensor cores favor int |
| Memory | 1x | 2x | 4x | Memory-bound often |
| Accuracy | -0.5% | -0.1% | Baseline | Negligible for most |

**Trend**: MX (Microscaling) format for next-gen AI:
- Block of 32 INT8 values share FP8 scale
- 2x compression vs. FP16
- Native in upcoming hardware

### Financial/Scientific

**Winner: Float64 or Q64.64**

| Criterion | Q32.32 | Float64 | Rationale |
|-----------|--------|---------|-----------|
| Currency | Risky | Risky | Use decimal or integer cents |
| Scientific | Poor range | Excellent | Float dominates |
| Reproducibility | Perfect | Platform-dependent | Fixed wins for regulation |

**Best Practice**:
- Currency: Integer cents or decimal types
- Scientific: Float64 or arbitrary precision
- Regulated: Fixed-point for bit-exact audits

### Embedded Systems

**Winner: Q-format (Q7 to Q31)**

| Criterion | Q16.16 | Float32 | Rationale |
|-----------|--------|---------|-----------|
| Code size | Small | Large | No float library |
| Cycle count | Predictable | Variable | WCET analysis |
| Power | Lower | Higher | Integer units efficient |
| Cortex-M0 | Native | Software | No FPU on M0 |

---

## Conversion Costs

### Cycles to Convert Between Formats

| From → To | Cycles (x86) | Method |
|-----------|-------------|--------|
| Q16.16 → Float32 | 5-10 | Cast + scale |
| Float32 → Q16.16 | 5-15 | Scale + truncate |
| Q16.16 → Posit32 | 50-100 | Software decode |
| Float32 → LNS32 | 20-40 | Log2 + scale |
| LNS32 → Float32 | 20-40 | Exp2 + scale |

### Implementation

```c
// Q16.16 to Float32
float q16_16_to_float(int32_t q) {
    return (float)q / 65536.0f;  // ~5 cycles
}

// Float32 to Q16.16 (with saturation)
int32_t float_to_q16_16(float f) {
    float scaled = f * 65536.0f;
    if (scaled > 2147483647.0f) return 0x7FFFFFFF;
    if (scaled < -2147483648.0f) return 0x80000000;
    return (int32_t)scaled;  // ~10 cycles
}
```

---

## Memory Layout Comparison

### 1000 Values Storage

| Format | Bytes | Cache Lines (64B) | L1 Cache Fits |
|--------|-------|-------------------|---------------|
| Q7 | 1000 | 16 | 32K values |
| Q15 | 2000 | 32 | 16K values |
| Q31 | 4000 | 63 | 8K values |
| Float16 | 2000 | 32 | 16K values |
| Float32 | 4000 | 63 | 8K values |
| Float64 | 8000 | 125 | 4K values |

### Alignment Requirements

| Format | Optimal Alignment | Penalty for Misalign |
|--------|-------------------|----------------------|
| Q7 | 1 byte | None |
| Q15 | 2 bytes | 0-10% slowdown |
| Q31 | 4 bytes | 0-20% slowdown |
| Float32 | 4 bytes | 0-50% slowdown |
| AVX vector | 32 bytes | Fault or split |
| AVX-512 | 64 bytes | Fault or split |

---

## Benchmark Code Templates

### Addition Benchmark

```c
#include <time.h>
#include <stdint.h>

#define N 10000000

// Q16.16
void bench_q16_add(void) {
    int32_t a[N], b[N], c[N];
    // Initialize...
    clock_t start = clock();
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];  // No saturation check
    }
    clock_t end = clock();
    printf("Q16.16 add: %.2f ns/op\n",
           (double)(end - start) / CLOCKS_PER_SEC / N * 1e9);
}

// Float32
void bench_float_add(void) {
    float a[N], b[N], c[N];
    clock_t start = clock();
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    clock_t end = clock();
    printf("Float32 add: %.2f ns/op\n",
           (double)(end - start) / CLOCKS_PER_SEC / N * 1e9);
}
```

### Multiplication Benchmark

```c
// Q16.16 with proper shift
void bench_q16_mul(void) {
    int32_t a[N], b[N], c[N];
    clock_t start = clock();
    for (int i = 0; i < N; i++) {
        c[i] = (int32_t)(((int64_t)a[i] * b[i]) >> 16);
    }
    clock_t end = clock();
    printf("Q16.16 mul: %.2f ns/op\n",
           (double)(end - start) / CLOCKS_PER_SEC / N * 1e9);
}
```

### SIMD Comparison

```c
#include <immintrin.h>

// AVX2 Q15 multiply (8 elements at once)
void bench_q15_mul_avx2(int16_t* a, int16_t* b, int16_t* c, int n) {
    for (int i = 0; i < n; i += 16) {
        __m256i va = _mm256_load_si256((__m256i*)&a[i]);
        __m256i vb = _mm256_load_si256((__m256i*)&b[i]);
        __m256i vc = _mm256_mulhrs_epi16(va, vb);  // Q15 multiply!
        _mm256_store_si256((__m256i*)&c[i], vc);
    }
}

// AVX2 Float32 multiply (8 elements at once)
void bench_float_mul_avx2(float* a, float* b, float* c, int n) {
    for (int i = 0; i < n; i += 8) {
        __m256 va = _mm256_load_ps(&a[i]);
        __m256 vb = _mm256_load_ps(&b[i]);
        __m256 vc = _mm256_mul_ps(va, vb);
        _mm256_store_ps(&c[i], vc);
    }
}
```

---

## Decision Matrix

### Choose Q-Format When:
- [ ] Target has no FPU (Cortex-M0, AVR, low-end RISC-V)
- [ ] Bit-exact reproducibility required
- [ ] Known, bounded value range
- [ ] Memory bandwidth is limiting factor
- [ ] WCET (worst-case execution time) needed
- [ ] DSP/audio with 16-bit or 24-bit I/O
- [ ] SIMD integer ops faster than SIMD float

### Choose Floating-Point When:
- [ ] Unknown or huge dynamic range
- [ ] GPU computation (float native)
- [ ] Numeric libraries expect float (BLAS, FFT)
- [ ] Development speed > performance
- [ ] Scientific computing with standard algorithms

### Choose Posit When:
- [ ] Research/experimental context
- [ ] Accuracy near 1.0 critical
- [ ] Willing to use software implementation
- [ ] Single NaR vs. multiple NaN values preferred

### Choose LNS When:
- [ ] Multiplication-heavy workload
- [ ] Very wide dynamic range
- [ ] Addition is rare
- [ ] Lookup tables acceptable

### Choose BFP When:
- [ ] ML inference with quantization
- [ ] Audio/video coding
- [ ] Correlated data blocks
- [ ] MX format hardware available

---

## References

- [Posit Standard 2022](https://posithub.org/docs/posit_standard-2.pdf)
- [MX Format Specification](https://www.opencompute.org/documents/ocp-microscaling-formats-mx-v1-0-spec-final-pdf)
- [Comparing Number Systems for Deep Learning](https://arxiv.org/abs/2003.07793)
- [LNS Survey: Coleman et al. 2000](https://ieeexplore.ieee.org/document/866647)
- [Fixed vs. Floating-Point: A Tutorial](https://www.superkits.net/whitepapers/Fixed%20Point%20Representation%20&%20Fractional%20Math.pdf)
