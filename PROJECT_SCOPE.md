# libfixp - Universal Fixed-Point Mathematics Library

## Project Vision

A clean-room synthesized, universal fixed-point mathematics library supporting:

- **Bit widths**: 8-bit, 16-bit, 32-bit, 64-bit, 128-bit
- **Q-formats**: Full permutation matrix Q(m.n) where m+n fits word size
- **Platforms**: C, C++, OpenCL
- **Acceleration**: SIMD (SSE, AVX, AVX-512, NEON, SVE/SVE2, scalable vectors), GPU compute
- **Floating-point interop**: FP16, FP32, FP64 conversion and emulation

## Target Use Cases

- Embedded systems / DSP
- Game engines / Graphics
- Financial / Scientific computing
- Machine Learning inference
- UI/UX / Window managers / Compositors
- General-purpose deterministic arithmetic

## Clean-Room Synthesis Methodology

This project follows strict clean-room principles:

1. **Research Phase** (`research/`): Concept extraction from academic papers, standards documents, and existing implementations. Documentation of *what* and *why*, never *how*.

2. **Synthesis Phase** (`src/`): Independent implementation from first principles, referencing only the concept documentation.

3. **Team Simulation**: Conceptual separation between "research team" (documents concepts) and "implementation team" (writes code from specs only).

### Directory Structure

```
research/
  papers/       - Academic paper summaries and citations
  concepts/     - Extracted concepts and algorithms (no code)
  standards/    - Standards document analysis (ARM ACLE, DSP-C, etc.)
  implementations/ - Notes on existing libraries (concepts only)
  notes/        - General research notes

src/
  core/         - Core fixed-point types and operations
  simd/         - SIMD-optimized implementations
  gpu/          - OpenCL kernels
  tests/        - Test suite

docs/           - API documentation and guides
external/       - Submodules (reference only, not for code copying)
```

## Technical Requirements

### Q-Format Coverage

- All valid Q(m.n) combinations where m + n <= word_size
- Asymmetric formats fully supported (Q1.31, Q31.1, Q0.32, etc.)
- Compile-time configurable with sensible defaults
- Template/macro-generated optimal code paths

### Overflow Semantics (Novel)

- **Policies**: Wrap, Saturate, Trap, Undefined (optimizer-friendly)
- **Granularity**: Per-operation or global
- **Switching**: Build-time OR runtime (hot-switchable without restart)
- **Innovation target**: Zero-overhead policy switching where possible

### Trigonometric Functions

Comprehensive coverage with accuracy/speed tradeoffs:
- CORDIC (all variants: rotating, vectoring, hyperbolic)
- Polynomial approximations (Taylor, Chebyshev, minimax)
- Lookup tables with interpolation
- Fast approximations for real-time use
- High-precision versions for scientific use

### Standards Compatibility

Design as superset of:
- ARM ACLE fixed-point extensions
- DSP-C / Embedded-C extensions
- ISO/IEC TR 18037 (Embedded C)
- Existing de-facto standards (Q15, Q31, etc.)

While also providing novel APIs for modern use cases.

## SIMD Architecture Support

| Width | x86/x64 | ARM | RISC-V | Notes |
|-------|---------|-----|--------|-------|
| 128-bit | SSE2/SSE4 | NEON | V-ext | Baseline SIMD |
| 256-bit | AVX/AVX2 | SVE | V-ext | Wide vectors |
| 512-bit | AVX-512 | SVE2 | V-ext | Very wide |
| Scalable | - | SVE/SVE2 | V-ext | Vector-length agnostic |
| 1024-bit+ | Future | SVE (up to 2048) | V-ext | Future-proofing |

## GPU Compute

- OpenCL 1.2+ kernels
- Vulkan compute shaders (future)
- CUDA consideration (future, licensing permitting)
- Focus on warp/wavefront efficient algorithms

## License

TBD - Will be permissive (MIT/BSD/Apache-2.0)

## Status

**Phase**: Deep Research
**Started**: December 2025
