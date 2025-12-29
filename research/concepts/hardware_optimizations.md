# Hardware-Specific Optimizations for Fixed-Point

## x86/x64 Microarchitecture

### Instruction Latency/Throughput Reference

Data from [Agner Fog's Optimization Resources](https://www.agner.org/optimize/)

#### Integer Operations (Skylake/Ice Lake)

| Operation | Latency (cycles) | Throughput (ops/cycle) |
|-----------|-----------------|----------------------|
| ADD/SUB (32-bit) | 1 | 4 |
| ADD/SUB (64-bit) | 1 | 4 |
| IMUL (32×32→32) | 3 | 1 |
| IMUL (32×32→64) | 3 | 1 |
| IMUL (64×64→64) | 3 | 1 |
| IMUL (64×64→128) | 4 | 1 |
| IDIV (32-bit) | 10-26 | 0.06-0.1 |
| IDIV (64-bit) | 17-90 | 0.01-0.06 |
| SAR/SHL (imm) | 1 | 2 |
| SAR/SHL (cl) | 1 | 1 |

#### Key Insights

**Multiplication is Fast**:
- Modern x86 has 3-cycle 64-bit multiply
- Q16.16 multiply (32×32→64→shift) ≈ 4-5 cycles total

**Division is Slow**:
- 10-90 cycles depending on operand sizes
- Avoid in tight loops
- Use reciprocal multiplication instead

**Saturation Requires Branching**:
```c
// Naive (branches)
if (sum > INT32_MAX) sum = INT32_MAX;
if (sum < INT32_MIN) sum = INT32_MIN;

// Branchless (better)
int64_t clamped = (sum < INT32_MIN) ? INT32_MIN :
                  (sum > INT32_MAX) ? INT32_MAX : sum;

// Best: SIMD paddsw/paddss
```

### Reciprocal Division Technique

Replace division by multiplication with reciprocal:
```c
// Instead of: result = a / 1000
// Use: result = (a * reciprocal_1000) >> shift

// For Q16.16 division by 1000:
// reciprocal = 2^48 / 1000 = 281474976710656 / 1000 = 0x431BDE82
#define RECIP_1000_Q32  0x10624DD3ULL  // 2^32 / 1000

int32_t div_by_1000(int32_t x) {
    int64_t temp = (int64_t)x * RECIP_1000_Q32;
    return (int32_t)(temp >> 32);
}
```

### Memory Alignment

| Access Type | Unaligned Penalty |
|-------------|-------------------|
| SSE (pre-AVX) | Fault or ~10 cycles |
| AVX | ~1-2 cycles |
| AVX-512 | ~1-2 cycles |
| Cache line split | +5-10 cycles |

**Best Practice**:
```c
alignas(64) int32_t buffer[1024];  // Cache line aligned
```

---

## ARM Cortex-M Optimization

### Cortex-M4/M7 DSP Extensions

**SIMD Instructions** (2×16-bit packed):
```asm
; QADD16: Saturating add of two packed Q15 values
QADD16 r2, r0, r1    ; r2 = sat(r0[15:0]+r1[15:0]) | sat(r0[31:16]+r1[31:16])

; SMLAD: Dual multiply-accumulate
; acc += (a[15:0] * b[15:0]) + (a[31:16] * b[31:16])
SMLAD r3, r0, r1, r3
```

**Saturating Instructions**:
```asm
QADD    ; 32-bit saturating add
QSUB    ; 32-bit saturating subtract
SSAT    ; Signed saturate to N bits
USAT    ; Unsigned saturate to N bits
```

**Cycle Counts (Cortex-M4)**:

| Instruction | Cycles |
|-------------|--------|
| ADD/SUB | 1 |
| MUL | 1 |
| MLA | 1 |
| SMULL (32×32→64) | 1 |
| SMLAL (32×32+64→64) | 1 |
| SDIV | 2-12 |
| QADD16 | 1 |
| SMLAD | 1 |

### CMSIS-DSP Optimization Guidelines

From [ARM CMSIS-DSP](https://github.com/ARM-software/CMSIS-DSP):

**Memory Placement**:
```c
// Place frequently accessed data in DTCM (Data Tightly Coupled Memory)
__attribute__((section(".dtcm"))) q15_t filter_coeffs[64];
__attribute__((section(".dtcm"))) q15_t delay_line[64];
```

**Compiler Flags**:
```bash
# Recommended for Cortex-M4/M7
-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard
-Ofast -ffast-math -funroll-loops
```

**Intrinsics for Q15**:
```c
#include "arm_math.h"

// Use intrinsics for saturating operations
q15_t a, b, result;
result = __QADD16(a, b);  // Packed Q15 saturating add

// For FIR filter, use ARM's optimized function
arm_fir_q15(&fir_instance, input, output, block_size);
```

### Cortex-M0/M0+ (No DSP, No FPU)

**Challenges**:
- No single-cycle multiply (M0)
- No saturating instructions
- No SIMD

**Optimization Strategies**:
```c
// 1. Use lookup tables for multiply
// 2. Unroll loops manually
// 3. Use bit manipulation for saturation

static inline int16_t sat_add_q15(int16_t a, int16_t b) {
    int32_t sum = (int32_t)a + b;
    // Branchless saturation
    if (sum > 32767) return 32767;
    if (sum < -32768) return -32768;
    return (int16_t)sum;
}
```

---

## ARM Cortex-A (NEON/SVE)

### NEON Optimization

**Vector Length**: Fixed 128-bit

**Key Q15/Q31 Operations**:
```c
#include <arm_neon.h>

// Q15 saturating multiply-accumulate
int16x8_t q15_mac(int16x8_t acc, int16x8_t a, int16x8_t b) {
    // vqrdmulhq: saturating rounding doubling multiply high
    int16x8_t prod = vqrdmulhq_s16(a, b);
    return vqaddq_s16(acc, prod);
}

// Q15 FIR filter kernel (8 samples at a time)
void fir_q15_neon(const int16_t* x, const int16_t* h,
                  int16_t* y, int n, int taps) {
    for (int i = 0; i < n; i += 8) {
        int32x4_t acc_lo = vdupq_n_s32(0x4000);  // Rounding bias
        int32x4_t acc_hi = vdupq_n_s32(0x4000);

        for (int j = 0; j < taps; j++) {
            int16x8_t xv = vld1q_s16(&x[i + j]);
            int16x8_t hv = vdupq_n_s16(h[j]);

            acc_lo = vmlal_s16(acc_lo, vget_low_s16(xv), vget_low_s16(hv));
            acc_hi = vmlal_s16(acc_hi, vget_high_s16(xv), vget_high_s16(hv));
        }

        // Shift down to Q15
        int16x4_t out_lo = vqshrn_n_s32(acc_lo, 15);
        int16x4_t out_hi = vqshrn_n_s32(acc_hi, 15);
        vst1q_s16(&y[i], vcombine_s16(out_lo, out_hi));
    }
}
```

### SVE/SVE2 Optimization

**Vector Length**: Scalable (128-2048 bits)

**Programming Model**:
```c
#include <arm_sve.h>

// Vector-length agnostic Q15 add
void q15_add_sve(const int16_t* a, const int16_t* b,
                 int16_t* c, int n) {
    for (int i = 0; i < n; i += svcnth()) {
        svbool_t pg = svwhilelt_b16(i, n);
        svint16_t va = svld1_s16(pg, &a[i]);
        svint16_t vb = svld1_s16(pg, &b[i]);
        svint16_t vc = svqadd_s16(va, vb);  // Saturating
        svst1_s16(pg, &c[i], vc);
    }
}
```

**SVE2 DSP Instructions**:
```c
// Saturating rounding doubling multiply high (Q15)
svint16_t result = svqrdmulh_s16(a, b);

// Complex multiply-accumulate (for DSP)
result = svcmla_s16_m(pg, acc, a, b, 0);   // Real part
result = svcmla_s16_m(pg, acc, a, b, 90);  // Imag part
```

---

## RISC-V Vector Extension

### RVV 1.0 Fixed-Point Features

**Unique CSRs**:
```c
// vxsat: Saturation flag (sticky)
// Set to 1 when any operation saturates

// vxrm: Rounding mode
// 0: round-to-nearest-up (rnu)
// 1: round-to-nearest-even (rne)
// 2: round-down (truncate) (rdn)
// 3: round-to-odd (rod)
```

**Saturating Operations**:
```c
#include <riscv_vector.h>

vint16m1_t q15_add_sat(vint16m1_t a, vint16m1_t b, size_t vl) {
    return vsadd_vv_i16m1(a, b, vl);  // Saturating add
}

vint16m1_t q15_mul(vint16m1_t a, vint16m1_t b, size_t vl) {
    // Set rounding mode to round-to-nearest
    // Then use vmulh for high bits with rounding
    return vmulh_vv_i16m1(a, b, vl);  // (a * b) >> 15
}
```

**Narrowing with Saturation**:
```c
// Narrow 32-bit to 16-bit with saturation and rounding
vint16m1_t narrow_q31_to_q15(vint32m2_t wide, size_t vl) {
    return vnclip_wx_i16m1(wide, 16, vl);  // Uses vxrm rounding mode
}
```

---

## FPGA Implementation

### Verilog Fixed-Point Multiply

```verilog
// Q16.16 multiply with registered output
module q16_16_mul (
    input wire clk,
    input wire signed [31:0] a,
    input wire signed [31:0] b,
    output reg signed [31:0] result
);

    wire signed [63:0] full_product;
    assign full_product = a * b;

    always @(posedge clk) begin
        // Round to nearest and extract middle 32 bits
        result <= (full_product + 32'h8000) >>> 16;
    end

endmodule
```

### Pipelining for Throughput

```verilog
// 4-stage pipelined Q16.16 multiply-accumulate
module q16_16_mac_pipe (
    input wire clk,
    input wire signed [31:0] a,
    input wire signed [31:0] b,
    input wire signed [63:0] acc_in,
    output reg signed [63:0] acc_out
);

    // Stage 1: Register inputs
    reg signed [31:0] a_r1, b_r1;
    reg signed [63:0] acc_r1;

    // Stage 2: Partial products
    reg signed [63:0] product_r2;
    reg signed [63:0] acc_r2;

    // Stage 3: Accumulate
    reg signed [63:0] sum_r3;

    // Stage 4: Output
    always @(posedge clk) begin
        // Stage 1
        a_r1 <= a;
        b_r1 <= b;
        acc_r1 <= acc_in;

        // Stage 2
        product_r2 <= a_r1 * b_r1;
        acc_r2 <= acc_r1;

        // Stage 3
        sum_r3 <= (product_r2 >>> 16) + acc_r2;

        // Stage 4
        acc_out <= sum_r3;
    end

endmodule
```

### Resource Usage (Typical Xilinx)

| Operation | LUTs | DSP48 Slices | Fmax |
|-----------|------|--------------|------|
| Q16.16 Add | ~32 | 0 | 500 MHz |
| Q16.16 Mul | ~0 | 1 | 400 MHz |
| Q16.16 MAC | ~32 | 1 | 400 MHz |
| Q16.16 Div | ~500 | 0 | 200 MHz |

---

## GPU Compute Optimization

### Memory Coalescing

**Critical for Performance**:
```opencl
// Good: Coalesced access (consecutive threads access consecutive memory)
__kernel void q16_add_good(__global int* a, __global int* b,
                           __global int* c) {
    int gid = get_global_id(0);
    c[gid] = a[gid] + b[gid];  // Threads 0,1,2,3 access indices 0,1,2,3
}

// Bad: Strided access (threads access non-consecutive memory)
__kernel void q16_add_bad(__global int* a, __global int* b,
                          __global int* c, int stride) {
    int gid = get_global_id(0);
    c[gid * stride] = a[gid * stride] + b[gid * stride];  // Memory divergence
}
```

### Occupancy vs. Register Usage

**Trade-off**:
- More registers → fewer threads → lower latency hiding
- Fewer registers → more threads → better memory latency hiding

**For Fixed-Point**:
- Integer ops use fewer registers than float
- Pack multiple Q15 values in 32-bit registers

### Vectorization

```opencl
// Process 4 Q15 values per work-item using short4
__kernel void q15_mul_vec(__global short4* a, __global short4* b,
                          __global short4* c) {
    int gid = get_global_id(0);
    short4 va = a[gid];
    short4 vb = b[gid];

    // Widen, multiply, narrow
    int4 prod;
    prod.x = (int)va.x * vb.x;
    prod.y = (int)va.y * vb.y;
    prod.z = (int)va.z * vb.z;
    prod.w = (int)va.w * vb.w;

    // Round and shift
    c[gid] = convert_short4_sat((prod + 0x4000) >> 15);
}
```

---

## Summary: Platform-Specific Best Practices

| Platform | Key Optimization | Avoid |
|----------|------------------|-------|
| x86 (scalar) | Use 64-bit intermediate | Division in loops |
| x86 (SIMD) | Use pmulhrsw (Q15) | Unaligned access |
| ARM Cortex-M4 | Use SMLAD, QADD16 | SDIV (slow) |
| ARM Cortex-A | Use NEON vqrdmulh | Scalar loops |
| ARM SVE | Use predicates, svqrdmulh | Fixed-length assumptions |
| RISC-V V | Use vxrm CSR, vnclip | Ignoring saturation flag |
| FPGA | Pipeline, use DSP slices | Behavioral division |
| GPU | Coalesce memory, vectorize | Thread divergence |

---

## References

- [Agner Fog's Instruction Tables](https://www.agner.org/optimize/instruction_tables.pdf)
- [Agner Fog's Microarchitecture Guide](https://www.agner.org/optimize/microarchitecture.pdf)
- [ARM Cortex-M4 Technical Reference](https://developer.arm.com/documentation/ddi0439/b/)
- [ARM NEON Programmer's Guide](https://developer.arm.com/documentation/den0018/a/)
- [ARM SVE Programmer's Guide](https://developer.arm.com/documentation/102131/latest)
- [RISC-V Vector Extension](https://github.com/riscv/riscv-v-spec)
- [uops.info](https://uops.info/) - Measured instruction timings
