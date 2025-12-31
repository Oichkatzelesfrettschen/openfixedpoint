#!/usr/bin/env python3
"""
Generate C23 fixed-point math function headers.
Clean-room implementation generating wrappers around core algorithms.
"""

import sys
from pathlib import Path

def generate_math_header(m_bits, n_bits):
    """Generate math functions for Qm.n format"""
    total_bits = m_bits + n_bits + 1  # +1 for sign bit
    type_name = f"q{m_bits}_{n_bits}_t"
    
    if total_bits <= 8:
        base_type = "int8_t"
    elif total_bits <= 16:
        base_type = "int16_t"
    elif total_bits <= 32:
        base_type = "int32_t"
    elif total_bits <= 64:
        base_type = "int64_t"
    else:
        raise ValueError(f"Total bits {total_bits} exceeds 64")
    
    header = f"""#ifndef FIXP_Q{m_bits}_{n_bits}_MATH_H
#define FIXP_Q{m_bits}_{n_bits}_MATH_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {{
#endif

// Q{m_bits}.{n_bits} format: {total_bits}-bit signed fixed-point
// {m_bits} integer bits, {n_bits} fractional bits
typedef {base_type} {type_name};

// Constants
#define Q{m_bits}_{n_bits}_PI       (({type_name})(3.14159265358979323846 * (1LL << {n_bits})))
#define Q{m_bits}_{n_bits}_E        (({type_name})(2.71828182845904523536 * (1LL << {n_bits})))
#define Q{m_bits}_{n_bits}_ONE      (({type_name})(1LL << {n_bits}))
#define Q{m_bits}_{n_bits}_HALF     (({type_name})(1LL << ({n_bits} - 1)))
#define Q{m_bits}_{n_bits}_MAX      (({type_name})((1LL << ({total_bits} - 1)) - 1))
#define Q{m_bits}_{n_bits}_MIN      (({type_name})(-(1LL << ({total_bits} - 1))))

// Basic operations
static inline {type_name} q{m_bits}_{n_bits}_abs({type_name} x) {{
    return (x < 0) ? -x : x;
}}

static inline {type_name} q{m_bits}_{n_bits}_min({type_name} a, {type_name} b) {{
    return (a < b) ? a : b;
}}

static inline {type_name} q{m_bits}_{n_bits}_max({type_name} a, {type_name} b) {{
    return (a > b) ? a : b;
}}

static inline {type_name} q{m_bits}_{n_bits}_clamp({type_name} x, {type_name} lo, {type_name} hi) {{
    return q{m_bits}_{n_bits}_min(q{m_bits}_{n_bits}_max(x, lo), hi);
}}

// Rounding functions
static inline {type_name} q{m_bits}_{n_bits}_floor({type_name} x) {{
    const {type_name} frac_mask = Q{m_bits}_{n_bits}_ONE - 1;
    if (x >= 0) {{
        return x & ~frac_mask;
    }} else {{
        return (x & frac_mask) ? ((x & ~frac_mask) - Q{m_bits}_{n_bits}_ONE) : x;
    }}
}}

static inline {type_name} q{m_bits}_{n_bits}_ceil({type_name} x) {{
    const {type_name} frac_mask = Q{m_bits}_{n_bits}_ONE - 1;
    if (x >= 0) {{
        return (x & frac_mask) ? ((x & ~frac_mask) + Q{m_bits}_{n_bits}_ONE) : x;
    }} else {{
        return x & ~frac_mask;
    }}
}}

static inline {type_name} q{m_bits}_{n_bits}_round({type_name} x) {{
    if (x >= 0) {{
        return (x + Q{m_bits}_{n_bits}_HALF) & ~(Q{m_bits}_{n_bits}_ONE - 1);
    }} else {{
        return (x - Q{m_bits}_{n_bits}_HALF) & ~(Q{m_bits}_{n_bits}_ONE - 1);
    }}
}}

static inline {type_name} q{m_bits}_{n_bits}_trunc({type_name} x) {{
    return x & ~(Q{m_bits}_{n_bits}_ONE - 1);
}}

// Square root (Newton-Raphson)
{type_name} q{m_bits}_{n_bits}_sqrt({type_name} x);

// Trigonometric functions (CORDIC-based)
{type_name} q{m_bits}_{n_bits}_sin({type_name} angle);
{type_name} q{m_bits}_{n_bits}_cos({type_name} angle);
{type_name} q{m_bits}_{n_bits}_tan({type_name} angle);
{type_name} q{m_bits}_{n_bits}_atan({type_name} x);
{type_name} q{m_bits}_{n_bits}_atan2({type_name} y, {type_name} x);

// Exponential and logarithmic
{type_name} q{m_bits}_{n_bits}_exp({type_name} x);
{type_name} q{m_bits}_{n_bits}_log({type_name} x);
{type_name} q{m_bits}_{n_bits}_exp2({type_name} x);
{type_name} q{m_bits}_{n_bits}_log2({type_name} x);
{type_name} q{m_bits}_{n_bits}_pow({type_name} base, {type_name} exponent);

#ifdef __cplusplus
}}
#endif

#endif // FIXP_Q{m_bits}_{n_bits}_MATH_H
"""
    return header

def generate_cordic_tables(n_bits):
    """Generate CORDIC tables for given fractional bits"""
    import math
    
    k = 0.60725293500888  # CORDIC gain
    k_fixed = int(k * (1 << n_bits))
    
    atan_table = []
    for i in range(min(n_bits, 32)):
        atan_val = math.atan(2.0 ** -i)
        atan_fixed = int(atan_val * (1 << n_bits))
        atan_table.append(atan_fixed)
    
    return k_fixed, atan_table

def generate_cordic_c_file(m_bits, n_bits):
    """Generate C implementation file with CORDIC algorithms"""
    total_bits = m_bits + n_bits + 1
    type_name = f"q{m_bits}_{n_bits}_t"
    
    if total_bits <= 8:
        base_type = "int8_t"
    elif total_bits <= 16:
        base_type = "int16_t"
    elif total_bits <= 32:
        base_type = "int32_t"
    elif total_bits <= 64:
        base_type = "int64_t"
    else:
        raise ValueError(f"Total bits {total_bits} exceeds 64")
    
    k_fixed, atan_table = generate_cordic_tables(n_bits)
    
    atan_table_str = ",\n    ".join(str(x) for x in atan_table)
    
    impl = f"""#include "q{m_bits}_{n_bits}_math.h"
#include <stdint.h>

// CORDIC constants
#define CORDIC_K_{n_bits} (({type_name}){k_fixed})
#define CORDIC_ITERATIONS {min(n_bits, 16)}

static const {type_name} cordic_atan_table[CORDIC_ITERATIONS] = {{
    {atan_table_str}
}};

// CORDIC rotation mode
static void cordic_rotate({type_name}* x, {type_name}* y, {type_name} z) {{
    {type_name} x_val = CORDIC_K_{n_bits};
    {type_name} y_val = 0;
    
    for (int i = 0; i < CORDIC_ITERATIONS; i++) {{
        {type_name} x_new, y_new, z_new;
        
        if (z >= 0) {{
            x_new = x_val - (y_val >> i);
            y_new = y_val + (x_val >> i);
            z_new = z - cordic_atan_table[i];
        }} else {{
            x_new = x_val + (y_val >> i);
            y_new = y_val - (x_val >> i);
            z_new = z + cordic_atan_table[i];
        }}
        
        x_val = x_new;
        y_val = y_new;
        z = z_new;
    }}
    
    *x = x_val;
    *y = y_val;
}}

// CORDIC vectoring mode
static {type_name} cordic_vector({type_name} x, {type_name} y) {{
    {type_name} z = 0;
    
    for (int i = 0; i < CORDIC_ITERATIONS; i++) {{
        {type_name} x_new, y_new, z_new;
        
        if (y < 0) {{
            x_new = x - (y >> i);
            y_new = y + (x >> i);
            z_new = z - cordic_atan_table[i];
        }} else {{
            x_new = x + (y >> i);
            y_new = y - (x >> i);
            z_new = z + cordic_atan_table[i];
        }}
        
        x = x_new;
        y = y_new;
        z = z_new;
    }}
    
    return z;
}}

{type_name} q{m_bits}_{n_bits}_sin({type_name} angle) {{
    // Range reduction to [-π, π]
    while (angle > Q{m_bits}_{n_bits}_PI) angle -= 2 * Q{m_bits}_{n_bits}_PI;
    while (angle < -Q{m_bits}_{n_bits}_PI) angle += 2 * Q{m_bits}_{n_bits}_PI;
    
    {type_name} x, y;
    bool negate = false;
    
    if (angle < 0) {{
        angle = -angle;
        negate = true;
    }}
    
    cordic_rotate(&x, &y, angle);
    
    return negate ? -y : y;
}}

{type_name} q{m_bits}_{n_bits}_cos({type_name} angle) {{
    // Range reduction to [-π, π]
    while (angle > Q{m_bits}_{n_bits}_PI) angle -= 2 * Q{m_bits}_{n_bits}_PI;
    while (angle < -Q{m_bits}_{n_bits}_PI) angle += 2 * Q{m_bits}_{n_bits}_PI;
    
    {type_name} x, y;
    
    if (angle < 0) {{
        angle = -angle;
    }}
    
    cordic_rotate(&x, &y, angle);
    
    return x;
}}

{type_name} q{m_bits}_{n_bits}_tan({type_name} angle) {{
    {type_name} s = q{m_bits}_{n_bits}_sin(angle);
    {type_name} c = q{m_bits}_{n_bits}_cos(angle);
    
    if (c == 0) return Q{m_bits}_{n_bits}_MAX;
    
    // Division: (s << n_bits) / c
    return (({base_type})s << {n_bits}) / c;
}}

{type_name} q{m_bits}_{n_bits}_atan2({type_name} y, {type_name} x) {{
    if (x == 0 && y == 0) return 0;
    
    {type_name} result = cordic_vector(x, y);
    
    // Quadrant adjustment
    if (x < 0) {{
        if (y >= 0) {{
            result = Q{m_bits}_{n_bits}_PI - result;
        }} else {{
            result = -Q{m_bits}_{n_bits}_PI + result;
        }}
    }}
    
    return result;
}}

{type_name} q{m_bits}_{n_bits}_atan({type_name} x) {{
    return q{m_bits}_{n_bits}_atan2(x, Q{m_bits}_{n_bits}_ONE);
}}

{type_name} q{m_bits}_{n_bits}_sqrt({type_name} x) {{
    if (x <= 0) return 0;
    
    // Find highest bit
    {type_name} bit = 1;
    {type_name} val = x;
    while (val > 0) {{
        val >>= 1;
        bit <<= 1;
    }}
    
    // Initial guess
    {type_name} guess = bit >> 1;
    if (guess < Q{m_bits}_{n_bits}_ONE) guess = Q{m_bits}_{n_bits}_ONE;
    
    // Newton-Raphson iterations
    for (int i = 0; i < 4; i++) {{
        {type_name} div = (({base_type})x << {n_bits}) / guess;
        guess = (guess + div) >> 1;
    }}
    
    return guess;
}}

{type_name} q{m_bits}_{n_bits}_exp2({type_name} x) {{
    // Extract integer part
    int int_part = x >> {n_bits};
    {type_name} frac_part = x & (Q{m_bits}_{n_bits}_ONE - 1);
    
    // Check overflow
    if (int_part >= {m_bits}) return Q{m_bits}_{n_bits}_MAX;
    if (int_part < -{n_bits}) return 0;
    
    // Compute 2^int_part
    {type_name} int_result = Q{m_bits}_{n_bits}_ONE << int_part;
    
    // Approximate 2^frac using polynomial
    // 2^x ≈ 1 + x*ln(2) + (x*ln(2))^2/2
    {type_name} ln2 = ({type_name})(0.693147 * (1LL << {n_bits}));
    {type_name} term = (({base_type})frac_part * ln2) >> {n_bits};
    {type_name} result = Q{m_bits}_{n_bits}_ONE + term;
    term = (({base_type})term * term) >> ({n_bits} + 1);
    result += term;
    
    return (({base_type})int_result * result) >> {n_bits};
}}

{type_name} q{m_bits}_{n_bits}_exp({type_name} x) {{
    // e^x = 2^(x * log2(e))
    {type_name} log2e = ({type_name})(1.442695 * (1LL << {n_bits}));
    {type_name} scaled = (({base_type})x * log2e) >> {n_bits};
    return q{m_bits}_{n_bits}_exp2(scaled);
}}

{type_name} q{m_bits}_{n_bits}_log2({type_name} x) {{
    if (x <= 0) return Q{m_bits}_{n_bits}_MIN;
    
    // Find highest bit
    int bit_pos = 0;
    {type_name} val = x;
    while (val > 0) {{
        val >>= 1;
        bit_pos++;
    }}
    bit_pos--;
    
    int int_part = bit_pos - {n_bits};
    
    // Normalize to [1, 2)
    {type_name} normalized;
    if (bit_pos >= {n_bits}) {{
        normalized = x >> (bit_pos - {n_bits});
    }} else {{
        normalized = x << ({n_bits} - bit_pos);
    }}
    
    // Approximate log2(normalized) for normalized in [1,2)
    {type_name} frac = normalized - Q{m_bits}_{n_bits}_ONE;
    {type_name} inv_ln2 = ({type_name})(1.442695 * (1LL << {n_bits}));
    {type_name} log_frac = (({base_type})frac * inv_ln2) >> {n_bits};
    
    return (int_part << {n_bits}) + log_frac;
}}

{type_name} q{m_bits}_{n_bits}_log({type_name} x) {{
    {type_name} log2_val = q{m_bits}_{n_bits}_log2(x);
    {type_name} ln2 = ({type_name})(0.693147 * (1LL << {n_bits}));
    return (({base_type})log2_val * ln2) >> {n_bits};
}}

{type_name} q{m_bits}_{n_bits}_pow({type_name} base, {type_name} exponent) {{
    if (base <= 0) return 0;
    // x^y = 2^(y * log2(x))
    {type_name} log_base = q{m_bits}_{n_bits}_log2(base);
    {type_name} scaled = (({base_type})exponent * log_base) >> {n_bits};
    return q{m_bits}_{n_bits}_exp2(scaled);
}}
"""
    return impl

def main():
    output_dir = Path(__file__).parent.parent / "include" / "fixp" / "gen"
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Generate for common formats
    formats = [
        (15, 16),  # Q15.16 (32-bit)
        (8, 8),    # Q8.8 (16-bit)
        (7, 8),    # Q7.8 (16-bit)
        (0, 7),    # Q0.7 (8-bit)
        (0, 15),   # Q0.15 (16-bit)
        (23, 8),   # Q23.8 (32-bit)
        (31, 0),   # Q31.0 (32-bit integer)
    ]
    
    for m, n in formats:
        # Generate header
        header_content = generate_math_header(m, n)
        header_file = output_dir / f"q{m}_{n}_math.h"
        header_file.write_text(header_content)
        print(f"Generated {header_file}")
        
        # Generate implementation
        impl_content = generate_cordic_c_file(m, n)
        impl_file = output_dir / f"q{m}_{n}_math.c"
        impl_file.write_text(impl_content)
        print(f"Generated {impl_file}")

if __name__ == "__main__":
    main()
