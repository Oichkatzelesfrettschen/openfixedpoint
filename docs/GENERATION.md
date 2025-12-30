# Header Generation Guide

The `libfixp` C interface is powered by a generator script that produces highly optimized, self-contained C headers for any requested Q-format.

## Generator Script

Location: `scripts/generate_headers.py`

### Usage

1. Open `scripts/generate_headers.py`.
2. Locate the `main()` function.
3. Add a call to `generate_header(m, n, output_dir)`:
   - `m`: Number of integer bits (excluding sign bit if signed).
   - `n`: Number of fractional bits.
   - `output_dir`: Target directory (usually `include/fixp/gen`).

### Example

To generate a Q2.14 format (1 sign + 2 integer + 13 fractional? No, Q2.14 usually means 2 integer bits. Total bits depends on sign).

The generator assumes **Signed** types by default.
Total bits = m + n + 1 (sign).

- **Q2.13** (16-bit): `generate_header(2, 13, ...)` -> 1+2+13 = 16 bits.
- **Q15.16** (32-bit): `generate_header(15, 16, ...)` -> 1+15+16 = 32 bits.

### Naming Convention

Generated files are named `qM_N.h`.
Types are named `qM_N_t`.
Macros use `QM_N_` prefix.

### Supported Sizes

- 8-bit (`int8_t`)
- 16-bit (`int16_t`)
- 32-bit (`int32_t`)
- 64-bit (`int64_t`)

128-bit support is available in the C++ template but not yet fully exposed in the C generator (requires compiler extensions for `__int128_t` math which are used in the script but requires environment support).
