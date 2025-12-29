# GPU and OpenCL Fixed-Point Patterns

## Overview

GPUs traditionally optimize for floating-point. Fixed-point on GPUs is challenging but valuable for:
- Deterministic computation
- Memory bandwidth (smaller types)
- Integer-only accelerators
- FPGA via OpenCL

## OpenCL Fixed-Point Landscape

### Native Integer Support
- OpenCL supports char, short, int, long (8/16/32/64-bit)
- Vector types: int2, int4, int8, int16, short8, short16, etc.
- Saturating built-ins: add_sat(), sub_sat(), mul_hi()

### Missing Pieces
- No native Q-format types
- No fixed-point math functions
- Must implement transcendentals in software

### Half-Precision (cl_khr_fp16)
- Some GPUs support (AMD, ARM Mali)
- NVIDIA OpenCL does NOT support despite hardware capability
- Compilation workarounds with #ifdef

## Fixed-Point in OpenCL

### Type Definitions
```opencl
typedef short fix16_t;   // Q15 or configurable
typedef int   fix32_t;   // Q31 or configurable

#define FRAC_BITS 15
#define FIX_ONE (1 << FRAC_BITS)
```

### Arithmetic Operations
```opencl
// Addition (with saturation)
fix16_t fix_add(fix16_t a, fix16_t b) {
    return add_sat(a, b);
}

// Multiplication Q15 * Q15 -> Q15
fix16_t fix_mul(fix16_t a, fix16_t b) {
    int product = (int)a * (int)b;
    return (fix16_t)(product >> FRAC_BITS);
}

// Division via reciprocal (Newton-Raphson)
fix16_t fix_div(fix16_t num, fix16_t denom) {
    // Implementation via reciprocal iteration
}
```

### Vector Operations
```opencl
typedef short8 fix16x8_t;

fix16x8_t fix_add_vec(fix16x8_t a, fix16x8_t b) {
    return add_sat(a, b);
}

fix16x8_t fix_mul_vec(fix16x8_t a, fix16x8_t b) {
    int8 prod = convert_int8(a) * convert_int8(b);
    return convert_short8(prod >> FRAC_BITS);
}
```

## Performance Considerations

### GPU Architecture Realities
1. **Integer throughput**: Often lower than FP32 on consumer GPUs
2. **Memory bandwidth**: Smaller types = faster memory
3. **Warp/wavefront divergence**: Avoid branches in fixed-point saturation
4. **Register pressure**: 16-bit = 2x values per register

### When Fixed-Point Wins on GPU
- Memory-bound kernels (smaller data = faster transfer)
- Determinism requirements
- Integer-only accelerators (some neural network chips)
- FPGA-based OpenCL

### When Floating-Point Wins
- Compute-bound on modern GPUs
- Complex transcendentals (native FPU)
- Dynamic range requirements

## Challenges

### No Native Transcendentals
- Must implement sin/cos/sqrt in software
- CORDIC is shift-add friendly
- Polynomial approximation needs multiply (available)

### Branching Penalty
- GPU saturation without branches:
```opencl
// Branchless saturation
int sat_add(int a, int b) {
    int sum = a + b;
    int overflow = (a ^ sum) & (b ^ sum);
    int max_val = (a >> 31) ^ 0x7FFFFFFF;
    return select(sum, max_val, overflow < 0);
}
```

### Software Fixed-Point Overhead
- Research shows OpenCL software fixed-point can be slower than CPU
- Need careful optimization and kernel design
- Consider hybrid: fixed-point data, float compute

## FPGA via OpenCL

### Intel FPGA OpenCL SDK
- Supports arbitrary bit-width integers
- Fixed-point can save significant resources
- DSP blocks available for multiplication

### Best Practices
- Match bit-width to precision needs
- Use OpenCL vector types for parallelism
- Pipeline operations for throughput

## Design Patterns for libfixp OpenCL

### Kernel Structure
```opencl
// Single kernel file with configurable Q format
#ifndef FRAC_BITS
#define FRAC_BITS 15
#endif

typedef short fix_t;
typedef int fix_wide_t;

__kernel void fixed_vector_add(
    __global const fix_t* a,
    __global const fix_t* b,
    __global fix_t* c,
    int n
) {
    int gid = get_global_id(0);
    if (gid < n) {
        c[gid] = add_sat(a[gid], b[gid]);
    }
}
```

### Header Generation
- Generate OpenCL headers from C++ templates
- Ensure consistency between host and device code
- Compile-time configuration of Q format

### Memory Layout
- AOS (Array of Structs) vs SOA (Struct of Arrays)
- SOA generally better for GPU coalescing
- Consider 128-bit aligned access

## Vulkan Compute (Future)

### SPIR-V Support
- Vulkan uses SPIR-V intermediate representation
- Can compile OpenCL-style kernels to SPIR-V
- Better cross-platform than OpenCL

### Integer Operations
- Full 8/16/32/64-bit integer support
- Explicit specialization constants for Q format
- Subgroup operations for reductions

## Design Decisions for libfixp

1. **OpenCL 1.2+ baseline**: Wide compatibility
2. **Configurable via preprocessor**: FRAC_BITS, WORD_SIZE
3. **Software transcendentals**: CORDIC + polynomial
4. **Host-device consistency**: Generated headers
5. **FPGA-friendly**: Arbitrary precision support
6. **Future Vulkan path**: SPIR-V generation
