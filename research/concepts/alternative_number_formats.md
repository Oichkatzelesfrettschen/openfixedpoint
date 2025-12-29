# Alternative Number Formats

Beyond Q-format fixed-point, several alternative representations offer unique tradeoffs.

---

## Posit (Unum Type III)

### Structure
```
| sign | regime | exponent | fraction |
|  1   | variable | variable | variable |
```

- **Regime**: Run of 0s or 1s, terminated by opposite bit
- **Exponent**: Optional, 0 to es bits
- **Fraction**: Remaining bits

### Advantages
- Higher precision near 1.0 (where most computation occurs)
- Larger dynamic range than same-bit IEEE float
- No NaN (only one exception value: ±∞)
- Simpler hardware than IEEE 754
- Deterministic: same result on all platforms

### Disadvantages
- Variable field widths require serial decoding
- Precision degrades dramatically at extremes
- Addition/subtraction more complex than fixed-point
- Limited hardware/software support (emerging)

### Posit vs Fixed-Point
| Aspect | Posit | Fixed-Point |
|--------|-------|-------------|
| Dynamic range | Wide | Narrow |
| Precision near 1 | Excellent | Good |
| Add/Sub speed | Medium | Fast |
| Mul/Div speed | Medium | Medium |
| Hardware cost | Medium | Low |
| Decode complexity | High | Zero |

### When to Use
- Deep learning (tapered precision suits weight distributions)
- Scientific computing with wide value ranges
- Drop-in IEEE float replacement

### Libraries
- [stillwater-sc/universal](https://github.com/stillwater-sc/universal) (comprehensive)
- [libcg/bfp](https://github.com/libcg/bfp) (C/C++)
- SoftPosit reference implementation

---

## Block Floating-Point (BFP)

### Structure
```
Block of N mantissas + 1 shared exponent
| m0 | m1 | m2 | ... | mN-1 | exp |
```

### Concept
- Group of values shares single exponent
- Each value has independent mantissa
- Common exponent = max exponent in block
- Acts as automatic gain control

### Advantages
- Extended dynamic range on fixed-point hardware
- Lower memory than full floating-point
- Good for signal processing (samples often similar magnitude)
- Industry standard in audio codecs (AC-3, Dolby)

### Disadvantages
- Precision loss when block has outliers
- Block boundary artifacts
- Complexity in choosing block size

### Applications
- FFT implementations (per-stage scaling)
- Audio compression
- Neural network quantization (MX format)

### MX Format (Microscaling)
- Industry standard by AMD, ARM, Intel, Meta, Microsoft, NVIDIA, Qualcomm
- Block of 8 numbers, 8-bit shared exponent
- Each number: 1 sign + 7 mantissa bits (BFP16)
- Designed for AI/ML workloads

### Implementation Pattern
```c
struct bfp16_block {
    int8_t mantissa[8];  // 8 values
    int8_t exponent;     // Shared exponent
};

float bfp16_to_float(bfp16_block blk, int idx) {
    return ldexpf(blk.mantissa[idx], blk.exponent);
}
```

---

## Logarithmic Number System (LNS)

### Structure
```
Value = sign * base^(fixed_point_exponent)
| sign | fixed-point exponent |
|  1   |       N-1 bits       |
```

### Multiplication/Division (The Win)
```
A * B = sign_a ^ sign_b * 2^(exp_a + exp_b)
A / B = sign_a ^ sign_b * 2^(exp_a - exp_b)
```
Multiplication becomes addition! Division becomes subtraction!

### Addition/Subtraction (The Cost)
```
A + B = 2^exp_a * (1 + 2^(exp_b - exp_a))  // when exp_a > exp_b
```
Requires lookup table or iterative algorithm. Non-trivial.

### Advantages
- Multiply/divide: single integer add/sub
- Square root: shift right by 1
- Powers: multiply exponent
- Wide dynamic range

### Disadvantages
- Addition/subtraction requires LUT or approximation
- Not closed under addition (approximate results)
- Less intuitive than fixed-point

### Performance Comparison
| Operation | LNS | Fixed-Point |
|-----------|-----|-------------|
| Multiply | ~1 cycle (add) | ~N cycles |
| Divide | ~1 cycle (sub) | ~N cycles |
| Add | ~10+ cycles (LUT) | ~1 cycle |
| Subtract | ~10+ cycles (LUT) | ~1 cycle |

### Best Use Cases
- Multiplication-heavy algorithms
- FPGA implementations
- Neural networks (weight multiplication dominant)
- Signal processing (filters, convolutions)

### European Logarithmic Microprocessor (ELM)
- 32-bit LNS processor
- Demonstrated as "more accurate alternative to floating-point"
- Uses cotransformation for addition

---

## Comparison Matrix

| Format | Add/Sub | Mul/Div | Range | Precision | Hardware |
|--------|---------|---------|-------|-----------|----------|
| Q-format | Fast | Medium | Narrow | Uniform | Simple |
| Posit | Medium | Medium | Wide | Tapered | Medium |
| BFP | Fast* | Medium | Medium | Block-uniform | Simple |
| LNS | Slow | Fast | Wide | Log-distributed | Medium |

*BFP addition within same block is fast

---

## Hybrid Approaches for libfixp

### Q-format Core + Extensions
1. Primary: Full Q(m.n) fixed-point support
2. Extension: BFP for signal processing blocks
3. Extension: LNS for multiply-heavy paths
4. Future: Posit interoperability

### Conversion Utilities
```cpp
namespace libfixp {
    // Q-format to other formats
    template<typename QType>
    posit32 to_posit(QType q);

    template<typename QType>
    bfp16_block to_bfp16(std::span<QType, 8> block);

    // LNS operations with Q conversion
    template<typename QType>
    QType lns_mul(QType a, QType b);  // Internally uses log-add
}
```

---

## References

- [Posit Standard 2022](https://posithub.org/docs/Posits4.pdf)
- [Beating Floating Point](http://www.johngustafson.net/pdfs/BeatingFloatingPoint.pdf)
- [Block Floating Point - Wikipedia](https://en.wikipedia.org/wiki/Block_floating_point)
- [MX Format for AI](https://quark.docs.amd.com/latest/pytorch/tutorial_bfp16.html)
- [LNS Arithmetic (UCSB)](https://web.ece.ucsb.edu/~parhami/pubs_folder/parh20-caee-comput-w-lns-arith.pdf)
