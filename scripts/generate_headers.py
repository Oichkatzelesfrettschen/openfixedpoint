#!/usr/bin/env python3
import os
import sys

def generate_header(m, n, output_dir, filename_override=None):
    total_bits = m + n + 1 # +1 for sign
    # Round up to nearest standard size
    if total_bits <= 8:
        storage_bits = 8
        storage_type = "int8_t"
        unsigned_type = "uint8_t"
        wide_type = "int16_t"
        storage_max = "INT8_MAX"
        storage_min = "INT8_MIN"
    elif total_bits <= 16:
        storage_bits = 16
        storage_type = "int16_t"
        unsigned_type = "uint16_t"
        wide_type = "int32_t"
        storage_max = "INT16_MAX"
        storage_min = "INT16_MIN"
    elif total_bits <= 32:
        storage_bits = 32
        storage_type = "int32_t"
        unsigned_type = "uint32_t"
        wide_type = "int64_t"
        storage_max = "INT32_MAX"
        storage_min = "INT32_MIN"
    elif total_bits <= 64:
        storage_bits = 64
        storage_type = "int64_t"
        unsigned_type = "uint64_t"
        wide_type = "__int128_t"
        storage_max = "INT64_MAX"
        storage_min = "INT64_MIN"
    else:
        print(f"Skipping Q{m}.{n}: Too large ({total_bits} bits needed)")
        return

    if filename_override:
        filename = filename_override
        guard_base = filename.replace(".", "_").upper()
        guard = f"FIXP_GEN_{guard_base}"
        type_name = f"q{m}_{n}_t"
        macro_prefix = f"Q{m}_{n}"
    else:
        filename = f"q{m}_{n}.h"
        guard = f"FIXP_GEN_Q{m}_{n}_H"
        type_name = f"q{m}_{n}_t"
        macro_prefix = f"Q{m}_{n}"

    # Calculate constants
    one_val = 1 << n

    content = f"""/**
 * @file {filename}
 * @brief Q{m}.{n} Fixed-Point Implementation (Generated)
 *
 * Format: 1 sign bit, {m} integer bits, {n} fractional bits.
 * Underlying storage: {storage_type} ({storage_bits}-bit).
 */

#ifndef {guard}
#define {guard}

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {{
#endif

typedef {storage_type} {type_name};

#ifdef __cplusplus
#define {macro_prefix}_WRAP(raw) ({type_name}{{(raw)}})
#else
#define {macro_prefix}_WRAP(raw) (({type_name})(raw))
#endif

#define {macro_prefix}_RAW(x) (x)

#define {macro_prefix}_FRAC_BITS {n}
#define {macro_prefix}_ONE {macro_prefix}_WRAP({one_val})
#define {macro_prefix}_MAX {macro_prefix}_WRAP({storage_max})
#define {macro_prefix}_MIN {macro_prefix}_WRAP({storage_min})

static inline {type_name} q{m}_{n}_add({type_name} a, {type_name} b) {{
    // Use unsigned arithmetic to avoid signed overflow UB
    return {macro_prefix}_WRAP(({storage_type})(({unsigned_type})a + ({unsigned_type})b));
}}

static inline {type_name} q{m}_{n}_sub({type_name} a, {type_name} b) {{
    return {macro_prefix}_WRAP(({storage_type})(({unsigned_type})a - ({unsigned_type})b));
}}

static inline {type_name} q{m}_{n}_mul({type_name} a, {type_name} b) {{
    {wide_type} prod = ({wide_type})a * ({wide_type})b;
    return {macro_prefix}_WRAP(({storage_type})((prod + (1 << ({n}-1))) >> {n}));
}}

static inline {type_name} q{m}_{n}_div({type_name} a, {type_name} b) {{
    if (b == 0) return (a >= 0) ? {macro_prefix}_MAX : {macro_prefix}_MIN;
    {wide_type} dividend = ({wide_type})a << {n};
    return {macro_prefix}_WRAP(({storage_type})(dividend / b));
}}

static inline {type_name} q{m}_{n}_from_double(double d) {{
    return {macro_prefix}_WRAP(({storage_type})(d * {float(1<<n)} + (d >= 0 ? 0.5 : -0.5)));
}}

static inline double q{m}_{n}_to_double({type_name} a) {{
    return (double)a / {float(1<<n)};
}}

#ifdef __cplusplus
}}
#endif

#endif // {guard}
"""

    with open(os.path.join(output_dir, filename), "w") as f:
        f.write(content)

def main():
    output_dir = "include/fixp/gen"
    os.makedirs(output_dir, exist_ok=True)

    # Q15.16 (32-bit standard "Q16.16")
    generate_header(15, 16, output_dir)

    # Literal Q16.16 (33-bit -> 64-bit)
    generate_header(16, 16, output_dir)

    # Q0.7 (Standard 8-bit)
    generate_header(0, 7, output_dir)

    # Q8.8 (Standard 16-bit) -> m=7, n=8
    generate_header(7, 8, output_dir)

    # Q24.8 (32-bit). 1 sign + 23 int + 8 frac.
    generate_header(23, 8, output_dir)

    # Q2.30 (32-bit). 1 sign + 1 int + 30 frac.
    generate_header(1, 30, output_dir)

    print(f"Generated key headers in {output_dir}")

if __name__ == "__main__":
    main()
