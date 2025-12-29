# Embedded Constraints: Cortex-M0, AVR, PIC

## Overview

Resource-constrained embedded systems require careful fixed-point design.

---

## ARM Cortex-M0/M0+

### Architecture
- 32-bit ARMv6-M architecture
- Von Neumann (single bus)
- Only 56 base instructions
- No hardware divide
- No DSP extensions (no SIMD)
- 1-cycle 32x32->32 multiply

### Memory Constraints
- Typical: 2KB - 32KB RAM
- Flash: 8KB - 256KB
- No floating-point unit

### Fixed-Point Considerations
- 32-bit arithmetic native
- No saturation instructions (must implement in software)
- No barrel shifter variable shifts (some operations slower)
- Single-cycle multiply helps Q format operations

### Code Density
- Thumb-2 subset only
- Excellent code density (better than 8-bit PICs)
- 16-bit instruction encoding

### libfixp Strategy
```c
// Cortex-M0 optimized Q15 multiply
int16_t mul_q15_m0(int16_t a, int16_t b) {
    // Use 32-bit multiply, no saturation HW
    int32_t prod = (int32_t)a * (int32_t)b;
    int16_t result = (int16_t)(prod >> 15);
    // Manual saturation check
    if (prod > 0x3FFFFFFF) return 0x7FFF;
    if (prod < -0x40000000) return 0x8000;
    return result;
}
```

---

## ARM Cortex-M4/M7

### Additional Features
- DSP extension (SIMD within 32-bit word)
- Saturation instructions (QADD, QSUB, SSAT, USAT)
- Single-cycle MAC
- Optional FPU

### DSP Instructions
- `SMLAD`: Dual signed multiply accumulate
- `SMUAD`: Dual signed multiply add
- `QADD16/QSUB16`: Packed Q15 saturating operations

### CMSIS-DSP Integration
```c
#include "arm_math.h"

void process_q15(q15_t* dst, q15_t* src1, q15_t* src2, uint32_t n) {
    arm_add_q15(src1, src2, dst, n);  // Optimized
}
```

---

## AVR (8-bit)

### Architecture
- 8-bit RISC
- Modified Harvard architecture
- 32 x 8-bit registers (R0-R31)
- 1-2 cycle instructions
- No hardware multiply (ATmega has 8x8 multiply)

### Memory Constraints
- RAM: 128 bytes - 16KB
- Flash: 1KB - 256KB
- EEPROM: 64 bytes - 4KB

### Fixed-Point Considerations
- 8-bit native operations
- 16-bit requires two registers (r25:r24 pairs)
- 32-bit operations are multi-instruction
- Software multiply for non-ATmega

### Q7 and Q8 Focus
```c
// AVR-optimized Q7 multiply
int8_t mul_q7_avr(int8_t a, int8_t b) {
    // ATmega has fmuls for Q7
    // Returns high byte of a*b*2
    int16_t prod = ((int16_t)a * (int16_t)b) >> 7;
    // Saturate
    if (prod > 127) return 127;
    if (prod < -128) return -128;
    return (int8_t)prod;
}
```

### Assembly Optimization
```asm
; Q7 multiply on ATmega
; Input: r24 = a, r25 = b
; Output: r24 = result
mul_q7:
    fmuls r24, r25    ; Fractional multiply signed
    mov r24, r1       ; Result in r1:r0
    clr r1            ; Clear r1 (gcc convention)
    ret
```

---

## PIC Microcontrollers

### PIC18 (8-bit)
- 8-bit data path
- Hardware 8x8 multiply
- 16 file select registers
- Bank-based memory

### Code Density Issues
- PIC18 has poor code density
- Factor of 5x worse than Cortex-M3 in some tests
- Assembly often necessary for performance

### Fixed-Point on PIC
```c
// PIC18 Q7 operations
// Compiler-specific, often needs assembly
#define Q7_ONE 0x7F
#define Q7_MUL(a, b) (((int16_t)(a) * (int16_t)(b)) >> 7)
```

### PIC24/dsPIC (16-bit)
- 16-bit data path
- DSP features (MAC, barrel shifter)
- Much better for fixed-point

---

## Memory/Code Tradeoffs

### Table-Based vs Computational

| Method | Code Size | RAM | Speed |
|--------|-----------|-----|-------|
| CORDIC (iterative) | Small | Tiny | Medium |
| Polynomial (5th order) | Medium | None | Fast |
| Lookup + Interp | Large | Table | Very Fast |

### For 2KB RAM Targets
- Avoid large lookup tables
- Use iterative algorithms (CORDIC)
- Minimize stack usage
- Consider code in flash (execute from ROM)

---

## 8-bit Primitive Strategy

### libfixp Modular Design
```
fixp8 (native) ──────────────────────┐
    │                                 │
fixp16 (2 x fixp8) ──────────────────┤
    │                                 │
fixp32 (4 x fixp8) ───── on 32-bit: native
    │
fixp64 (8 x fixp8)
```

### Platform Selection
```c
#if defined(__AVR__)
    #define FIXP_NATIVE_BITS 8
    typedef int8_t fixp_native_t;
#elif defined(__ARM_ARCH_6M__)  // Cortex-M0
    #define FIXP_NATIVE_BITS 32
    typedef int32_t fixp_native_t;
#elif defined(__PIC18)
    #define FIXP_NATIVE_BITS 8
    typedef int8_t fixp_native_t;
#endif
```

---

## Optimization Guidelines

### For 8-bit Targets (AVR, PIC18)
1. Prefer Q7/Q8 formats
2. Minimize 32-bit operations
3. Use PROGMEM for tables (AVR)
4. Assembly for hot paths
5. Avoid division

### For Cortex-M0
1. Q15/Q16.16 work well
2. No saturation HW - add checks
3. Leverage 32-bit multiply
4. Consider M0+ for lower power

### For Cortex-M4+
1. Full Q15/Q31 support
2. Use CMSIS-DSP
3. Leverage SIMD (packed operations)
4. Use saturation instructions

---

## Testing on Embedded

### Simulation
- QEMU for ARM
- simavr for AVR
- gpsim for PIC

### Hardware-in-Loop
- Generate test vectors on host
- Run on target
- Compare results

### Constraint Verification
- Stack usage analysis
- Cycle counting
- Memory map verification

---

## References

- [ARM Cortex-M0 Reference](https://developer.arm.com/ip-products/processors/cortex-m/cortex-m0)
- [AVR Instruction Set](https://ww1.microchip.com/downloads/en/devicedoc/atmel-0856-avr-instruction-set-manual.pdf)
- [Definitive Guide to Cortex-M0](https://www.sciencedirect.com/book/9780123854773)
- [CMSIS-DSP Documentation](https://arm-software.github.io/CMSIS-DSP/main/)
