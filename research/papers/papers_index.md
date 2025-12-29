# Academic Papers Index

## Fixed-Point Neural Network Quantization

### QFX: Trainable Fixed-Point Quantization for Deep Learning Acceleration on FPGAs
- **arXiv**: [2401.17544](https://arxiv.org/abs/2401.17544)
- **Date**: January 2024
- **Key Concepts**:
  - Automatically learns binary-point position during training
  - PyTorch library that emulates fixed-point in differentiable manner
  - Reduces DSP usage by limiting "1"s in fixed-point representation
  - Substitutes multiplications with additions and shifts
- **Relevance to libfixp**: Novel Q-format optimization techniques

### Low-Precision Training of Large Language Models
- **arXiv**: [2505.01043](https://arxiv.org/html/2505.01043v1)
- **Date**: May 2025
- **Key Concepts**:
  - Classification of low-precision training techniques by numerical format
  - Fixed-point, integer, floating-point, customized representations
  - Stochastic rounding to prevent gradient information loss
  - Nearest rounding causes small values to round to zero
- **Relevance to libfixp**: Rounding mode strategies

### Quantized Neural Networks for Microcontrollers
- **arXiv**: [2508.15008](https://arxiv.org/html/2508.15008v1)
- **Date**: August 2025
- **Key Concepts**:
  - Quantization aligns with MCU integer arithmetic units
  - "Fake" quantization for training (quantize then dequantize)
  - Comprehensive review of TinyML techniques
- **Relevance to libfixp**: Embedded deployment patterns

### A Survey of Quantization Methods for Efficient Neural Network Inference
- **arXiv**: [2103.13630](https://arxiv.org/abs/2103.13630)
- **Date**: 2021 (foundational)
- **Key Concepts**:
  - 4-bit fixed-point can achieve 16x memory/latency reduction
  - Survey of quantization-aware training techniques
- **Relevance to libfixp**: Understanding quantization landscape

### A White Paper on Neural Network Quantization (Qualcomm)
- **arXiv**: [2106.08295](https://arxiv.org/pdf/2106.08295)
- **Date**: 2021 (foundational)
- **Key Concepts**:
  - Uniform quantization for efficient fixed-point arithmetic
  - Industry perspective on practical quantization
- **Relevance to libfixp**: Industry best practices

---

## CORDIC Algorithms

### Low Latency Recoding CORDIC Algorithm for FPGA Implementation
- **Source**: ICCS 2025, Springer LNCS vol 15905
- **Date**: 2025
- **Key Concepts**:
  - Reduces number of operations while maintaining accuracy
  - Addresses iterative method challenges (complex structure, high resource consumption)
  - Designed for signal processing, HPC, edge computing
- **Relevance to libfixp**: Optimized CORDIC for hardware

### Evaluation of New CORDIC Algorithms for Givens Rotator
- **Source**: ScienceDirect, 2025
- **Date**: 2025
- **Key Concepts**:
  - Two modified CORDIC algorithms for Givens rotator
  - Selective iteration scheme with optimized scaling factor
  - Scaling-free methodology
  - 50% accuracy improvement, 15% latency reduction
  - Implemented on Altera Cyclone V FPGA
- **Relevance to libfixp**: Improved CORDIC variants

### CORDIC Is All You Need
- **arXiv**: [2503.11685](https://arxiv.org/pdf/2503.11685)
- **Date**: 2024
- **Key Concepts**:
  - SYCore: Systolic CORDIC engine for AI workloads
  - 2.5x resource savings, 3x power reduction vs prior work
  - 4.64x throughput enhancement with 40% pruning
  - Output stationary dataflow
  - Supports Transformers, RNNs/LSTMs, DNNs
- **Relevance to libfixp**: CORDIC for modern AI/ML

### FPGA Implementation of Reconfigurable CORDIC Algorithm
- **Source**: IEEE, 2022
- **Date**: 2022
- **Key Concepts**:
  - All possible CORDIC configurations in single design
  - Two approaches: multiplier-less and single multiplier
  - 0.4483 Gbit/s throughput on Artix-7 FPGA
- **Relevance to libfixp**: Reconfigurable CORDIC design

---

## Fixed-Point DSP

### Fixed-point Quantization of Convolutional Neural Networks
- **arXiv**: [2102.02147](https://arxiv.org/abs/2102.02147)
- **Date**: 2021
- **Key Concepts**:
  - Quantized inference on embedded platforms
  - CNN fixed-point conversion techniques
- **Relevance to libfixp**: CNN deployment patterns

### Providing Standardized Fixed-Point Arithmetics for Embedded C Programs
- **Source**: Springer
- **Key Concepts**:
  - ISO/IEC TR 18037 implementation
  - AVR microcontroller target
  - Standard library approach
- **Relevance to libfixp**: Standards compliance

---

## Papers to Acquire (Future)

### Classic Papers
- Volder, J.E. (1959). "The CORDIC Trigonometric Computing Technique"
- Walther, J.S. (1971). "A Unified Algorithm for Elementary Functions"
- Lanczos, C. (1952). Chebyshev approximation contributions

### Recent Research Targets
- 2024-2025 papers on:
  - Mixed-precision training
  - Hardware-aware quantization
  - Scalable vector fixed-point (SVE)
  - RISC-V fixed-point extensions

---

## Citation Format

When referencing papers in specifications:
```
[Author2024] Author et al., "Title", arXiv:XXXX.XXXXX, 2024.
```

## Notes

- All papers are for **concept extraction** only
- Implementation must be independent (clean-room)
- Focus on understanding "what" and "why", not "how"
