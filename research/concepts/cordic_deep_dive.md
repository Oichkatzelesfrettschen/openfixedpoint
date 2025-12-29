# CORDIC Deep Dive: Advanced Variants

## Standard CORDIC Recap

### Basic Iteration (Radix-2)
```
x' = x - σ * y * 2^(-i)
y' = y + σ * x * 2^(-i)
z' = z - σ * arctan(2^(-i))
```

### Convergence
- 1 bit of precision per iteration
- ~N iterations for N-bit result
- Gain factor K ≈ 1.6467 (must compensate)

---

## Radix-4 CORDIC

### Concept
Process 2 bits per iteration instead of 1.

### Rotation Angles
- Standard: arctan(2^(-i))
- Radix-4: arctan(2^(-2i)) and arctan(2^(-2i-1))

### Selection Function
| σ | Condition |
|---|-----------|
| +2 | z >= 3/2 * arctan(2^(-2i)) |
| +1 | z >= 1/2 * arctan(2^(-2i)) |
| 0 | |z| < 1/2 * arctan(2^(-2i)) |
| -1 | z <= -1/2 * arctan(2^(-2i)) |
| -2 | z <= -3/2 * arctan(2^(-2i)) |

### Advantages
- Half the iterations (N/2 for N bits)
- 2x faster convergence

### Disadvantages
- More complex selection logic
- Variable scale factor
- Larger angle lookup tables

### Performance (from Changela et al., 2023)
- R4-LV CORDIC: half iterations of radix-2
- Hardware: N adders + N muxes (vs 2N adders)
- 19% less FPGA utilization than radix-2

---

## Radix-8 CORDIC

### Concept
Process 3 bits per iteration.

### Trade-offs
- Even faster convergence
- Significantly more complex selection
- Diminishing returns on iteration reduction

---

## Scaling-Free CORDIC

### The Problem
Standard CORDIC produces result scaled by:
```
K = ∏(sqrt(1 + 2^(-2i))) ≈ 1.6467... (circular)
```
Must multiply by 1/K (expensive) or pre-scale input.

### Solution: Taylor Approximation
For later iterations (large i), approximate:
```
cos(arctan(2^(-i))) ≈ 1
sin(arctan(2^(-i))) ≈ 2^(-i)
```
This gives:
```
x' = x - σ * y * 2^(-i)  // unchanged
y' = y + σ * x * 2^(-i)  // unchanged
```
With gain factor = 1 for these iterations.

### Hybrid Approach
1. First k iterations: standard CORDIC (pre-computed K)
2. Remaining iterations: scaling-free (K = 1)
3. Total K = K_first_k only

### Benefits
- No final scaling multiplication
- Simpler hardware
- Slight accuracy trade-off

---

## Low-Latency CORDIC

### Recoding Approach (ICCS 2025)
- Reduce operation count while maintaining accuracy
- Novel iteration selection scheme
- Optimized for FPGA implementation

### Techniques
1. **Merged iterations**: Combine consecutive iterations
2. **Carry-save arithmetic**: Reduce critical path
3. **Pipelining**: Trade latency for throughput
4. **Parallel residual**: Compute multiple paths

---

## Hyperbolic CORDIC

### Coordinate System Change
Replace circular rotations with hyperbolic:
```
x' = x + σ * y * 2^(-i)   // Note: + instead of -
y' = y + σ * x * 2^(-i)
z' = z - σ * arctanh(2^(-i))
```

### Repeat Iterations
For convergence, must repeat iterations at:
```
i = 4, 13, 40, 121, 364, ... (3k+1 sequence)
```

### Computable Functions
| Mode | Function | Initialization |
|------|----------|----------------|
| Rotation | sinh(z), cosh(z) | x=K', y=0 |
| Vectoring | arctanh(y/x) | - |
| | sqrt(x²-y²) | Vectoring result |
| | ln(x) | x=a+1, y=a-1, result=2z |
| | exp(z) | result = x + y |

### R4HR (Radix-4 Hyperbolic Rotation)
- Modified scaling-free approach
- Taylor approximation for sinh/cosh
- Used for computing exponential

---

## Givens Rotator Optimization (2025)

### New Algorithms
1. **Selective iteration**: Skip unnecessary iterations
2. **Optimized scaling factor**: Pre-computed per precision
3. **Scaling-free methodology**: Improved precision

### Results (Altera Cyclone V FPGA)
- 50% accuracy improvement
- 15% latency reduction
- Extended from ICCS-2024 work

---

## CORDIC for AI (SYCore)

### "CORDIC Is All You Need" (2024)
Novel application of CORDIC for neural networks.

### Architecture
- Systolic CORDIC array
- Output stationary dataflow
- Reconfigurable Processing Engine (RPE)

### Performance
- 40% pruning rate support
- 4.64x throughput enhancement
- 2.5x resource savings vs prior work
- 3x power reduction

### Supported Workloads
- Transformers
- RNNs/LSTMs
- DNNs
- Applications: LLMs, image detection, speech recognition

---

## Implementation Strategy for libfixp

### Multi-Variant Support
```cpp
namespace cordic {
    enum class Variant {
        Radix2,      // Simple, portable
        Radix4,      // Faster, more complex
        ScalingFree, // No final multiply
        LowLatency   // Optimized for speed
    };

    template<Variant V, typename QType>
    struct Engine {
        static QType sin(QType angle);
        static QType cos(QType angle);
        static std::pair<QType, QType> sincos(QType angle);
        static QType atan2(QType y, QType x);
        static QType sqrt(QType x);
    };
}
```

### Compile-Time Selection
```cpp
#if LIBFIXP_CORDIC_VARIANT == LIBFIXP_CORDIC_RADIX4
    using DefaultCordic = cordic::Engine<cordic::Variant::Radix4, q16_16>;
#else
    using DefaultCordic = cordic::Engine<cordic::Variant::Radix2, q16_16>;
#endif
```

### SIMD-Friendly Design
- Batch multiple CORDIC evaluations
- Per-lane independent computation
- Amortize table lookups

---

## Lookup Table Optimization

### Table Size
| Variant | Table Size (16-bit) |
|---------|---------------------|
| Radix-2 | 16 entries |
| Radix-4 | 32 entries |
| Scaling-Free Hybrid | 8 + 8 entries |

### Table Generation
```cpp
constexpr auto generate_arctan_table() {
    std::array<q16_16, 16> table;
    for (int i = 0; i < 16; i++) {
        table[i] = q16_16::from_double(std::atan(1.0 / (1 << i)));
    }
    return table;
}
```

---

## References

- [Radix-4 CORDIC (Nature 2023)](https://www.nature.com/articles/s41598-023-47890-3)
- [CORDIC Is All You Need (arXiv)](https://arxiv.org/pdf/2503.11685)
- [Low Latency CORDIC (ICCS 2025)](https://link.springer.com/chapter/10.1007/978-3-031-97632-2_6)
- [New CORDIC for Givens Rotator (ScienceDirect 2025)](https://www.sciencedirect.com/science/article/abs/pii/S1877750325000444)
- [CORDIC Wikipedia](https://en.wikipedia.org/wiki/CORDIC)
