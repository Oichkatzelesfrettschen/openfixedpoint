# Universal Fixed-Point Mathematics Library (libfixp)

A modern, clean-room synthesized fixed-point arithmetic library for C23 and C++23.

## Features

- **Universal Coverage**: Supports any Qm.n format (e.g., Q16.16, Q0.7, Q2.30) via C++ templates or generated C headers.
- **Modern Standards**: Written in pure C23 and C++23.
- **High Performance**: Header-only, inline, `constexpr` capable.
- **Cross-Platform**: Zero dependencies (standard library only).

## Usage (C++23)

Use the `libfixp::FixedPoint` template for type-safe, compile-time optimized fixed-point arithmetic.

```cpp
#include <libfixp/fixed_point.hpp>
#include <print> // C++23

using Q16_16 = libfixp::FixedPoint<32, 16>;
using Q0_7   = libfixp::FixedPoint<8, 7>;

int main() {
    Q16_16 a(1.5);
    Q16_16 b(2.0);
    Q16_16 c = a + b; // 3.5

    // Saturation support
    using SatQ7 = libfixp::FixedPoint<8, 7, true, libfixp::OverflowPolicy::Saturate>;
    SatQ7 max_val = SatQ7::max();
    SatQ7 overflow = max_val + SatQ7(0.1); // Saturates, doesn't wrap
}
```

## Usage (C23)

For C, use the generated headers in `include/libfixp/gen/`.

```c
#include <libfixp/gen/q15_16.h> // Standard Q16.16 (32-bit)

void example() {
    q15_16_t a = q15_16_from_double(1.5);
    q15_16_t b = q15_16_from_double(2.0);
    q15_16_t c = q15_16_add(a, b);
}
```

### Generating Custom Formats

If you need a specific format (e.g., Q4.12) that isn't pre-generated, use the Python script:

```bash
python3 scripts/generate_headers.py
```

Modify the `main()` function in the script to include your desired formats.

## Build Instructions

### Prerequisites
- CMake 3.25+
- C++23 compliant compiler (GCC 13+, Clang 16+, MSVC 19.36+)
- Python 3 (for generation script)

### Building Tests

```bash
mkdir build && cd build
cmake -DBUILD_TESTING=ON ..
make
ctest
```

## Legacy Support

The library provides backward compatibility headers `libfixp_q16_16.h` and `libfixp_q7.h` which map to the new modern implementations.

## License

Public Domain / CC0
