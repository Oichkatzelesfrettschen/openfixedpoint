# Existing Fixed-Point Libraries (Concept Extraction)

## Purpose of This Document

This document catalogs existing fixed-point libraries for **concept extraction only**.
We analyze WHAT they do and WHY, NOT HOW they implement it.
All implementation in libfixp must be clean-room from first principles.

---

## libfixmath (C)

### Overview
- Platform-independent Q16.16 fixed-point library
- C99, minimal dependencies (just stdint.h)
- Targets embedded systems without FPU

### Concepts to Extract
- Single fixed format (Q16.16) simplifies API
- Similar interface to standard math.h
- Conditional compilation for 64-bit support
- Focus on common DSP functions

### What They Provide
- Basic arithmetic (+, -, *, /)
- Trigonometry (sin, cos, tan, asin, acos, atan, atan2)
- Exponentials (exp, log, sqrt)
- Saturating operations

### Design Philosophy
- Simplicity over flexibility
- Portability over performance
- Single format reduces template complexity

---

## fpm (C++ Header-Only)

### Overview
- Modern C++ (C++11+) header-only library
- Drop-in replacement for floating-point types
- Guards against accidental float conversion

### Concepts to Extract
- Type safety via strong typing
- Template-based precision selection
- Comprehensive standard library coverage
- Unit testing methodology

### What They Provide
- fpm::fixed<BaseType, IntermediateType, FractionBits>
- Operator overloading for natural syntax
- std::numeric_limits specialization
- Math functions matching <cmath>

### Design Philosophy
- Type safety paramount
- Compile-time precision specification
- Deterministic behavior emphasis

---

## fixed_math (C++23/C++17)

### Overview
- High-performance Q48.16 arithmetic
- Modern C++ features (concepts, constexpr)
- Performance-oriented

### Concepts to Extract
- Modern C++ features for fixed-point
- 48-bit integer for extended range
- Constexpr evaluation for compile-time math

### Design Philosophy
- Performance over flexibility
- Modern language features
- Specific use case (graphics/games)

---

## fixedptc (C)

### Overview
- Macro-based fixed-point library
- Liberal licensing
- Portable across architectures

### Concepts to Extract
- Macro-based approach (alternative to templates)
- Compile-time format configuration
- Simplicity for embedded use

---

## ARM CMSIS-DSP

### Overview
- ARM's official DSP library
- Q7, Q15, Q31 format support
- SIMD optimized (NEON, Helium)

### Concepts to Extract
- Industry-standard Q formats
- SIMD optimization patterns
- DSP algorithm implementations
- Matrix/vector operations

### What They Provide
- FFT/IFFT
- FIR/IIR filters
- Matrix operations
- Statistics functions
- Complex math

---

## Common Patterns Across Libraries

### Format Selection
1. **Single format** (libfixmath): Simple, limited
2. **Template parameter** (fpm): Flexible, complex
3. **Macro configuration** (fixedptc): C-compatible flexibility
4. **Predefined set** (CMSIS): Standard formats only

### Function Coverage
Most libraries provide:
- Basic arithmetic (with overflow handling)
- Trigonometry (at least sin/cos)
- Square root
- Division

Advanced libraries add:
- Exponentials/logarithms
- Matrix operations
- DSP functions (FFT, filters)

### Saturation Handling
- Most default to saturating arithmetic
- Some provide wrapping alternative
- Few provide trapping
- None found with runtime switching (opportunity for libfixp!)

### SIMD Support
- CMSIS: Extensive ARM SIMD
- Most others: None or minimal
- Gap: Generic SIMD abstraction layer

---

## Gaps and Opportunities for libfixp

1. **No universal Q-format library**: All specialize in specific formats
2. **No runtime policy switching**: All compile-time only
3. **No GPU support**: OpenCL fixed-point is underserved
4. **No scalable SIMD**: No SVE/RISC-V V support found
5. **Limited accuracy options**: Usually one implementation per function
6. **No clean API for all platforms**: C, C++, OpenCL fragmented

## References (for concept analysis only)

- [fpm GitHub](https://github.com/MikeLankamp/fpm)
- [libfixmath GitHub](https://github.com/PetteriAimonen/libfixmath)
- [fixed_math GitHub](https://github.com/arturbac/fixed_math)
- [fixedptc SourceForge](https://sourceforge.net/projects/fixedptc/)
- [libfixmath Wikipedia](https://en.wikipedia.org/wiki/Libfixmath)
