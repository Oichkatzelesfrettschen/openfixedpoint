# Modular 8-bit Primitive Architecture

## Philosophy

Everything decomposes to 8-bit operations. Larger operations (16, 32, 64, 128, 256, 512-bit) are built from 8-bit primitives. When SIMD is available, these primitives automatically vectorize.

## Core Principles

1. **8-bit is the Atom**: All operations ultimately reduce to 8-bit building blocks
2. **Composition over Complexity**: Larger types compose smaller types
3. **SIMD-Transparent**: Same code path for scalar and vector
4. **Platform-Agnostic**: Same algorithm, different backends

## Type Hierarchy

```
                    fixp512  (64 x 8-bit)
                       |
                    fixp256  (32 x 8-bit)
                       |
                    fixp128  (16 x 8-bit)
                       |
                    fixp64   (8 x 8-bit)
                       |
                    fixp32   (4 x 8-bit)
                       |
                    fixp16   (2 x 8-bit)
                       |
                    fixp8    (1 x 8-bit) <-- PRIMITIVE
```

## 8-bit Primitive Operations

### Arithmetic
```
// All operations defined at 8-bit level
uint8_t add8(uint8_t a, uint8_t b, uint8_t* carry);
uint8_t sub8(uint8_t a, uint8_t b, uint8_t* borrow);
uint8_t mul8_lo(uint8_t a, uint8_t b);  // Low 8 bits of product
uint8_t mul8_hi(uint8_t a, uint8_t b);  // High 8 bits of product
```

### With Carry/Borrow Propagation
```
uint8_t adc8(uint8_t a, uint8_t b, uint8_t carry_in, uint8_t* carry_out);
uint8_t sbc8(uint8_t a, uint8_t b, uint8_t borrow_in, uint8_t* borrow_out);
```

### Saturation
```
uint8_t add8_sat(uint8_t a, uint8_t b);  // Saturating add
int8_t add8s_sat(int8_t a, int8_t b);    // Signed saturating add
```

## Composing Larger Types

### 16-bit from 2x 8-bit
```
struct fixp16 {
    uint8_t lo;  // bits [0:7]
    uint8_t hi;  // bits [8:15]
};

fixp16 add16(fixp16 a, fixp16 b) {
    uint8_t carry;
    fixp16 result;
    result.lo = add8(a.lo, b.lo, &carry);
    result.hi = adc8(a.hi, b.hi, carry, NULL);
    return result;
}
```

### 32-bit from 4x 8-bit
```
struct fixp32 {
    uint8_t bytes[4];  // Little-endian: [0]=LSB, [3]=MSB
};

fixp32 add32(fixp32 a, fixp32 b) {
    uint8_t carry = 0;
    fixp32 result;
    for (int i = 0; i < 4; i++) {
        result.bytes[i] = adc8(a.bytes[i], b.bytes[i], carry, &carry);
    }
    return result;
}
```

### Generic N-bit
```
void addN(uint8_t* result, const uint8_t* a, const uint8_t* b, size_t n_bytes) {
    uint8_t carry = 0;
    for (size_t i = 0; i < n_bytes; i++) {
        result[i] = adc8(a[i], b[i], carry, &carry);
    }
}
```

## SIMD Acceleration

### Scalar Path (Fallback)
```
// Pure C, works everywhere
for (int i = 0; i < n_bytes; i++) {
    result[i] = adc8(a[i], b[i], carry, &carry);
}
```

### SIMD Path (When Available)
```
// x86 AVX2: Process 32 bytes at once
__m256i va = _mm256_loadu_si256(a);
__m256i vb = _mm256_loadu_si256(b);
__m256i vsum = _mm256_add_epi8(va, vb);
// Handle carries across lanes...
```

### Automatic Selection
```
#if defined(__AVX512BW__)
    #define SIMD_WIDTH 64
    #define add8_vec _mm512_add_epi8
#elif defined(__AVX2__)
    #define SIMD_WIDTH 32
    #define add8_vec _mm256_add_epi8
#elif defined(__SSE2__)
    #define SIMD_WIDTH 16
    #define add8_vec _mm_add_epi8
#elif defined(__ARM_NEON)
    #define SIMD_WIDTH 16
    #define add8_vec vaddq_s8
#else
    #define SIMD_WIDTH 1
    // Scalar fallback
#endif
```

## Multiplication Strategy

### 8x8 -> 16 (Full Product)
```
uint16_t mul8x8(uint8_t a, uint8_t b) {
    return (uint16_t)a * (uint16_t)b;
}
```

### 16x16 -> 32 (via 8-bit)
```
uint32_t mul16x16(uint16_t a, uint16_t b) {
    uint8_t a_lo = a & 0xFF, a_hi = a >> 8;
    uint8_t b_lo = b & 0xFF, b_hi = b >> 8;

    //   a_hi * b_hi * 2^16
    // + a_hi * b_lo * 2^8
    // + a_lo * b_hi * 2^8
    // + a_lo * b_lo

    uint16_t p0 = mul8x8(a_lo, b_lo);
    uint16_t p1 = mul8x8(a_lo, b_hi);
    uint16_t p2 = mul8x8(a_hi, b_lo);
    uint16_t p3 = mul8x8(a_hi, b_hi);

    // Combine with proper shifts and carries
    return p0 + ((uint32_t)(p1 + p2) << 8) + ((uint32_t)p3 << 16);
}
```

### Karatsuba for Large Widths
For 256-bit and above, use Karatsuba multiplication to reduce 8-bit multiplies.

## Division Strategy

### Newton-Raphson Reciprocal
1. Initial estimate via lookup table
2. Iterate: x' = x * (2 - d * x)
3. Multiply dividend by reciprocal

### Each iteration uses 8-bit primitives
- Subtraction: 8-bit with borrow chain
- Multiplication: 8-bit × 8-bit schoolbook

## Q-Format Integration

### Fixed Radix Point
```
struct q8_8 {
    int8_t int_part;   // 8 bits integer
    uint8_t frac_part; // 8 bits fraction
};

struct q16_16 {
    int16_t int_part;  // 16 bits integer (2 x 8-bit)
    uint16_t frac_part;// 16 bits fraction (2 x 8-bit)
};
```

### Multiplication with Q Adjustment
```
// Q8.8 * Q8.8 -> Q8.8
// Full product is Q16.16, need to shift right by 8 (frac bits)
q8_8 mul_q8_8(q8_8 a, q8_8 b) {
    // Treat as 16-bit integers, multiply, shift result
    uint32_t product = (uint32_t)(uint16_t)a * (uint16_t)b;
    return (q8_8)(product >> 8);
}
```

## Phase Structure

### Phase 1: 8-bit Core
- add8, sub8, mul8, adc8, sbc8
- Saturating variants
- Carry/borrow handling

### Phase 2: 16-bit Composition
- Compose from 8-bit
- Optimize for common cases

### Phase 3: 32/64-bit
- Extend composition pattern
- Consider loop unrolling

### Phase 4: 128/256/512-bit
- Karatsuba multiplication
- SIMD-friendly layouts
- Cache-aware algorithms

### Phase 5: SIMD Backends
- SSE2/AVX2/AVX-512
- NEON/SVE
- RISC-V V
- Scalar fallback

## Memory Layout

### Little-Endian Bytes
```
fixp64 value; // 0x0102030405060708
// value.bytes[0] = 0x08 (LSB)
// value.bytes[7] = 0x01 (MSB)
```

### SIMD-Friendly Alignment
```
alignas(64) uint8_t data[64];  // AVX-512 friendly
alignas(32) uint8_t data[32];  // AVX2 friendly
```

## Code Generation Strategy

All 8-bit primitives can be generated for:

| Target | Approach |
|--------|----------|
| K&R C | Function pointers, manual carry |
| C99/C11/C17 | inline, stdint.h |
| C++11-23 | Templates, constexpr |
| OpenCL | Kernel functions |
| OpenGL CS | Compute shader functions |

## Testing Strategy

### Unit Test Each Level
1. Test 8-bit primitives exhaustively (256^2 = 65536 cases per binary op)
2. Test 16-bit composition with edge cases
3. Sample 32/64-bit ranges
4. Property-based testing for larger widths

### Carry Chain Validation
Ensure carries propagate correctly across byte boundaries.

## Performance Considerations

### Scalar Performance
- Modern CPUs can do 8-bit ops very fast
- But carry chains are sequential bottleneck
- Consider 32/64-bit native ops when available

### SIMD Performance
- Horizontal carry propagation is expensive
- Use SIMD for parallel independent operations
- Consider SIMD-within-a-word techniques

### Hybrid Approach
- Use native 32/64-bit for small types (fast path)
- Use 8-bit composition for 128+ bit (general path)
- SIMD for batch operations
