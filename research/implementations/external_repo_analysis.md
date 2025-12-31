# External Repository Feature Analysis

## Overview
This document summarizes the features, algorithms, and approaches found in 15 external fixed-point and SIMD libraries. Following clean-room methodology, this documents WHAT features exist and WHY they're useful, not HOW they're implemented.

## Repositories Analyzed

1. **fpm** - C++ header-only fixed-point with comprehensive math
2. **libfixmath** - C Q16.16 implementation with trig and transcendental functions
3. **cordic** - CORDIC algorithm generator for multiple bit widths
4. **liquid-fpm** - DSP-focused fixed-point for software-defined radio
5. **fix32** - Simple Q16.16 implementation
6. **fixed_point_cnl** - Compositional Number Library (deprecated, superseded by CNL)
7. **fptc-lib** - Basic fixed-point C library
8. **qdsp** - Audio DSP fixed-point for embedded systems
9. **CMSIS-DSP** - ARM's official DSP library with extensive fixed-point support
10. **universal** - Universal number types including posits, fixed-point, etc.
11. **fixed_math** - C++ fixed-point math library
12. **MFixedPoint** - Embedded-focused fixed-point
13. **eve** - Expression-based SIMD library
14. **highway** - Google's portable SIMD library
15. **xsimd** - xtensor SIMD abstractions

## Feature Categories

### 1. Trigonometric Functions

**Found in:** fpm, libfixmath, fixed_math, CMSIS-DSP, universal, eve, highway, xsimd

**Common Functions:**
- sin, cos, tan
- asin, acos, atan, atan2

**Approaches Observed:**
- **Lookup tables (LUT):** Pre-computed values with interpolation for speed
- **CORDIC:** Iterative rotation algorithm, good for hardware/embedded
- **Polynomial approximation:** Taylor series, Chebyshev polynomials, minimax
- **Parabolic approximation:** Simpler approximation for lower accuracy needs
- **Hybrid:** Coarse LUT + fine interpolation or polynomial

**Why Needed:**
- Essential for graphics, robotics, physics simulations
- Game engines require fast sin/cos for rotations
- Signal processing uses trig for frequency analysis

### 2. CORDIC Algorithms

**Found in:** cordic, libfixmath, liquid-fpm, CMSIS-DSP, universal

**Variants:**
- **Circular mode (rotation):** For sin/cos/atan
- **Linear mode:** For multiplication/division
- **Hyperbolic mode:** For sinh/cosh/tanh/exp/log

**Bit Widths Supported:**
- 4-bit, 8-bit, 12-bit, 16-bit, 24-bit, 32-bit, 48-bit, 64-bit

**Why Needed:**
- Hardware-friendly (only shifts and adds, no multiplies)
- Deterministic iteration count
- Good accuracy/performance tradeoff
- Scalable to different precision requirements

### 3. Exponential and Logarithmic Functions

**Found in:** fpm, libfixmath, liquid-fpm, fptc-lib, CMSIS-DSP, universal, eve, highway, xsimd

**Functions:**
- exp, exp2
- log, log2, log10
- pow
- sqrt, cbrt

**Approaches:**
- **Bit manipulation:** For log2/exp2 using fixed-point properties
- **Taylor series:** For exp around zero
- **Lookup + polynomial:** For arbitrary ranges
- **Newton-Raphson:** For sqrt/cbrt (iterative refinement)

**Why Needed:**
- Machine learning: activation functions, normalization
- Audio: dB conversion, envelope processing
- Financial: compound interest calculations
- Scientific computing: statistical distributions

### 4. DSP Functions

**Found in:** libfixmath, liquid-fpm, CMSIS-DSP, universal, MFixedPoint

**Functions:**
- **FFT/IFFT:** Fast Fourier Transform (various sizes: 16, 32, 64, 128, 256, 512, 1024, 2048, 4096)
- **FIR filters:** Finite Impulse Response
- **IIR filters:** Infinite Impulse Response
- **Convolution**
- **Correlation**
- **Window functions:** Hann, Hamming, Blackman, etc.

**Why Needed:**
- Audio processing: EQ, effects, analysis
- Communications: modulation, demodulation, filtering
- Sensor data processing: noise reduction
- Real-time systems where floating-point is too slow

### 5. Rounding and Comparison Functions

**Found in:** All repositories

**Functions:**
- floor, ceil, round, trunc
- min, max
- abs
- clamp/saturate

**Why Needed:**
- Essential for any numeric type
- Saturation prevents overflow in cascaded operations
- Rounding modes affect accuracy and bias

### 6. Overflow Handling

**Found in:** fpm, CMSIS-DSP, universal, fixed_math

**Policies:**
- **Wrap:** Modulo arithmetic (fastest)
- **Saturate:** Clamp to min/max (safer)
- **Trap/Exception:** For debugging
- **Undefined:** Let optimizer decide (fastest when overflow impossible)

**Why Needed:**
- Different domains have different requirements
- Audio needs saturation to prevent clipping artifacts
- Graphics might accept wrap for performance
- Safety-critical systems need trap/exception

### 7. SIMD Optimizations

**Found in:** highway, xsimd, eve, CMSIS-DSP, fixed_point_cnl

**Architectures:**
- **x86/x64:** SSE, SSE2, SSE4, AVX, AVX2, AVX-512
- **ARM:** NEON, SVE, SVE2
- **RISC-V:** V extension (scalable vectors)

**Operations:**
- Vectorized arithmetic (parallel add/sub/mul/div)
- Horizontal operations (sum, min, max across lanes)
- Shuffles and permutations
- Gather/scatter
- Fused multiply-add (FMA)

**Why Needed:**
- 2-16x speedup for batch operations
- Essential for real-time audio/video
- Machine learning inference
- High-performance computing

### 8. Type Safety and Policies

**Found in:** fpm, fixed_point_cnl, fixed_math, universal

**Features:**
- Template-based type system
- Compile-time checked conversions
- Policy-based overflow handling
- Custom rounding modes
- Constexpr support for compile-time computation

**Why Needed:**
- Prevent accidental precision loss
- Catch errors at compile-time
- Zero-cost abstractions
- Generic programming support

## Missing Features in Our Repository

### Critical (Must-Have)
1. ✗ Trigonometric functions (sin, cos, tan, atan, atan2)
2. ✗ Exponential functions (exp, log, pow, sqrt)
3. ✗ CORDIC implementation
4. ✗ Rounding functions (floor, ceil, round, trunc)
5. ✗ Math constants (pi, e, etc.)

### Important (Should-Have)
6. ✗ FFT/IFFT implementations
7. ✗ SIMD optimizations
8. ✗ Saturation arithmetic
9. ✗ String conversion (to/from string)
10. ✗ DSP filter functions

### Nice-to-Have
11. ✗ GPU/OpenCL kernels
12. ✗ Hyperbolic functions (sinh, cosh, tanh)
13. ✗ Statistical functions (mean, variance, etc.)
14. ✗ Matrix operations
15. ✗ Special functions (gamma, erf, etc.)

## Implementation Priority

### Phase 1: Core Math
1. Basic rounding (floor, ceil, round, trunc)
2. abs, min, max, clamp
3. sqrt using Newton-Raphson
4. Math constants

### Phase 2: Trigonometry
5. CORDIC implementation (circular mode)
6. sin/cos via CORDIC
7. atan/atan2 via CORDIC
8. Lookup table generation tools

### Phase 3: Transcendentals
9. exp/log via bit manipulation + polynomial
10. pow via exp/log
11. exp2/log2 optimized versions

### Phase 4: DSP
12. FFT (radix-2, power-of-2 sizes)
13. Basic FIR filter
14. Window functions

### Phase 5: SIMD
15. SSE/AVX wrappers
16. NEON wrappers
17. Portable SIMD abstraction

## Algorithm Concepts

### CORDIC (COordinate Rotation DIgital Computer)

**Purpose:** Compute trig, exp, log using only shifts and adds

**Circular Mode:**
- Start with unit vector (K, 0) where K = scaling factor ≈ 0.607
- Rotate by pre-computed angles from lookup table
- Each iteration halves the angle granularity
- Result converges to (cos(θ), sin(θ))

**Why It Works:**
- Rotation matrix multiplication simplifies to shifts when angles are atan(2^-i)
- Trade multiplies for iterations
- Accuracy scales with iteration count

### Lookup Tables with Interpolation

**Purpose:** Balance speed vs memory

**Approach:**
- Store samples at regular intervals
- Interpolate between samples (linear, cubic, etc.)
- Often combined with range reduction

**Trade-offs:**
- More samples = better accuracy, more memory
- Fewer samples = less memory, needs better interpolation
- Typical: 64-256 entries for sin/cos

### Newton-Raphson for Square Root

**Purpose:** Compute sqrt iteratively

**Method:**
- Start with estimate (often from bit-scan)
- Iterate: x_new = (x_old + N/x_old) / 2
- Converges quadratically (doubles precision per iteration)

**Why Fixed-Point:**
- All operations are adds, shifts, and one division
- Division can be pre-shifted for efficiency
- 3-4 iterations typically sufficient

### Polynomial Approximations

**Purpose:** Approximate functions in limited range

**Types:**
- Taylor series: Easy to derive, not optimal
- Chebyshev: Better min-max error
- Minimax: Optimal error minimization
- Remez algorithm: Compute minimax coefficients

**Range Reduction:**
- Reduce input to small range (e.g., [0, π/4] for sin)
- Use identities to map result back
- Enables lower-degree polynomials

## Standards and Compatibility

### ARM ACLE (ARM C Language Extensions)

**Fixed-Point Types:**
- `__sat`, `__ssat`, `__usat` - saturating arithmetic
- Q format intrinsics
- NEON intrinsics for parallel operations

### DSP-C / Embedded-C (ISO/IEC TR 18037)

**Features:**
- `_Fract`, `_Accum` types
- `_Sat` for saturation
- Named bit operators
- Hardware register access

### CMSIS-DSP Compatibility

**Common Formats:**
- Q7 (s8, 7 fractional bits)
- Q15 (s16, 15 fractional bits)
- Q31 (s32, 31 fractional bits)

**Naming Convention:**
- `arm_<function>_<type>` e.g., `arm_sin_q31`

## Performance Considerations

### Accuracy vs Speed Tradeoffs

1. **Lookup tables:** Fast but memory-intensive
2. **CORDIC:** Moderate speed, low resources, deterministic
3. **Polynomial:** Fast for reduced ranges, needs pre-reduction
4. **Iterative (Newton):** Predictable, can early-exit

### Memory Hierarchy

- L1 cache: 32-64 KB typical, keep hot LUTs here
- L2 cache: 256 KB - 1 MB, larger tables
- ROM: Pre-computed tables in embedded systems

### Instruction-Level Parallelism

- CORDIC iterations are sequential
- Polynomial evaluation can use Horner's method
- SIMD enables data parallelism across multiple values

## Testing Strategies

### Accuracy Testing

- Compare against high-precision reference (mpmath, MPFR)
- Test special values (0, ±1, ±max, ±min)
- Test edge cases (near overflow, denormals)
- Measure ULP (Units in Last Place) error

### Performance Testing

- Benchmark hot loops with representative data
- Test with random inputs to avoid branch prediction
- Profile cache behavior
- Compare against compiler optimized float code

### Compliance Testing

- Test against standards (IEEE 754 for rounding modes)
- Test ACLE/DSP-C compatibility if claimed
- Test saturation behavior matches specs

## Conclusion

The external repositories provide a wealth of proven algorithms and approaches. Key takeaways:

1. **CORDIC is essential** for hardware-friendly trig/exp/log
2. **Lookup tables** are widely used for speed
3. **Saturation arithmetic** is critical for DSP and audio
4. **SIMD support** is expected in modern libraries
5. **Policy-based design** enables flexibility without runtime cost

Our implementation should prioritize clean C23/C++23 code that can be extended with SIMD, maintain accuracy comparable to existing libraries, and provide both speed-optimized and accuracy-optimized variants.
