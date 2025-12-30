#ifndef FIXP_MATH_HPP
#define FIXP_MATH_HPP

#include "fixed_point.hpp"
#include <cstdint>
#include <bit>
#include <cassert>

namespace fixp {

//
// Constants
//
namespace constants {
    // Pre-computed for common Q formats
    // Users can compute: fixed_point(M_PI) for their format
    template<typename FixedType>
    constexpr FixedType pi() {
        return FixedType(3.14159265358979323846);
    }
    
    template<typename FixedType>
    constexpr FixedType e() {
        return FixedType(2.71828182845904523536);
    }
    
    template<typename FixedType>
    constexpr FixedType pi_over_2() {
        return FixedType(1.57079632679489661923);
    }
    
    template<typename FixedType>
    constexpr FixedType pi_over_4() {
        return FixedType(0.78539816339744830962);
    }
    
    template<typename FixedType>
    constexpr FixedType two_pi() {
        return FixedType(6.28318530717958647692);
    }
}

//
// Basic operations
//
template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
constexpr auto abs(FixedPoint<TotalBits, FracBits, Signed, Policy> x) {
    return (x < FixedPoint<TotalBits, FracBits, Signed, Policy>::zero()) ? -x : x;
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
constexpr auto min(FixedPoint<TotalBits, FracBits, Signed, Policy> a, 
                   FixedPoint<TotalBits, FracBits, Signed, Policy> b) {
    return (a < b) ? a : b;
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
constexpr auto max(FixedPoint<TotalBits, FracBits, Signed, Policy> a, 
                   FixedPoint<TotalBits, FracBits, Signed, Policy> b) {
    return (a > b) ? a : b;
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
constexpr auto clamp(FixedPoint<TotalBits, FracBits, Signed, Policy> value,
                     FixedPoint<TotalBits, FracBits, Signed, Policy> lo,
                     FixedPoint<TotalBits, FracBits, Signed, Policy> hi) {
    return min(max(value, lo), hi);
}

//
// Rounding functions
//
template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
inline auto floor(FixedPoint<TotalBits, FracBits, Signed, Policy> x) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    using raw_type = typename FP::raw_type;
    
    constexpr raw_type ONE = raw_type(1) << FracBits;
    constexpr raw_type FRAC_MASK = ONE - 1;
    auto raw = x.raw();
    
    if (raw >= 0) {
        // Positive: just clear fractional bits
        raw &= ~FRAC_MASK;
    } else {
        // Negative: round down (toward -infinity)
        if ((raw & FRAC_MASK) != 0) {
            raw = (raw & ~FRAC_MASK) - ONE;
        } else {
            raw &= ~FRAC_MASK;
        }
    }
    
    return FP::from_raw(raw);
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
inline auto ceil(FixedPoint<TotalBits, FracBits, Signed, Policy> x) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    using raw_type = typename FP::raw_type;
    
    constexpr raw_type ONE = raw_type(1) << FracBits;
    constexpr raw_type FRAC_MASK = ONE - 1;
    auto raw = x.raw();
    
    if (raw >= 0) {
        // Positive: round up
        if ((raw & FRAC_MASK) != 0) {
            raw = (raw & ~FRAC_MASK) + ONE;
        } else {
            raw &= ~FRAC_MASK;
        }
    } else {
        // Negative: just clear fractional bits (toward zero)
        raw &= ~FRAC_MASK;
    }
    
    return FP::from_raw(raw);
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
inline auto trunc(FixedPoint<TotalBits, FracBits, Signed, Policy> x) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    using raw_type = typename FP::raw_type;
    
    constexpr raw_type ONE = raw_type(1) << FracBits;
    constexpr raw_type FRAC_MASK = ONE - 1;
    auto raw = x.raw();
    raw &= ~FRAC_MASK;
    
    return FP::from_raw(raw);
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
inline auto round(FixedPoint<TotalBits, FracBits, Signed, Policy> x) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    using raw_type = typename FP::raw_type;
    
    constexpr raw_type HALF = raw_type(1) << (FracBits - 1);
    auto raw = x.raw();
    
    if (raw >= 0) {
        raw += HALF;
    } else {
        raw -= HALF;
    }
    
    constexpr raw_type FRAC_MASK = (raw_type(1) << FracBits) - 1;
    raw &= ~FRAC_MASK;
    
    return FP::from_raw(raw);
}

//
// Square root using Newton-Raphson iteration
//
template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
inline auto sqrt(FixedPoint<TotalBits, FracBits, Signed, Policy> x) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    using raw_type = typename FP::raw_type;
    
    if (x <= FP::zero()) {
        return FP::zero();
    }
    
    auto raw = x.raw();
    
    // Find highest bit position
    int bit_pos = std::bit_width(static_cast<std::make_unsigned_t<raw_type>>(raw)) - 1;
    
    // Initial guess: 2^((bit_pos + FracBits)/2 - FracBits) = 2^((bit_pos - FracBits)/2)
    // This gives us sqrt(2^bit_pos) in fixed-point
    raw_type guess = raw_type(1) << ((bit_pos + FracBits) / 2);
    
    // Newton-Raphson: x_new = (x_old + N/x_old) / 2
    // In fixed-point: x_new = (x_old + (N << FracBits) / x_old) >> 1
    for (int i = 0; i < 5; ++i) {
        if (guess == 0) break;
        using wide_type = storage_t<(TotalBits <= 32 ? TotalBits * 2 : 128), Signed>;
        wide_type num = static_cast<wide_type>(raw) << FracBits;
        raw_type div = static_cast<raw_type>(num / guess);
        guess = (guess + div) >> 1;
    }
    
    return FP::from_raw(guess);
}

//
// CORDIC-based trigonometric functions
//
namespace detail {
    // CORDIC constants for 32-bit with 16 fractional bits
    // K ≈ 0.60725293500888 (CORDIC gain)
    constexpr int32_t CORDIC_K_Q16 = 39797; // K * 2^16
    
    // Arctangent table: atan(2^-i) for i = 0 to 15
    constexpr int32_t CORDIC_ATAN_TABLE_Q16[] = {
        51472,  // atan(2^0)  ≈ 0.7854 rad (45°)
        30386,  // atan(2^-1) ≈ 0.4636 rad
        16055,  // atan(2^-2) ≈ 0.2450 rad
        8150,   // atan(2^-3) ≈ 0.1244 rad
        4091,   // atan(2^-4)
        2047,   // atan(2^-5)
        1024,   // atan(2^-6)
        512,    // atan(2^-7)
        256,    // atan(2^-8)
        128,    // atan(2^-9)
        64,     // atan(2^-10)
        32,     // atan(2^-11)
        16,     // atan(2^-12)
        8,      // atan(2^-13)
        4,      // atan(2^-14)
        2       // atan(2^-15)
    };
    
    // CORDIC rotation mode: compute sin and cos
    template<typename raw_type>
    void cordic_rotation(raw_type& x, raw_type& y, raw_type z, int iterations) {
        x = CORDIC_K_Q16;
        y = 0;
        
        for (int i = 0; i < iterations && i < 16; ++i) {
            raw_type x_new, y_new, z_new;
            
            if (z >= 0) {
                x_new = x - (y >> i);
                y_new = y + (x >> i);
                z_new = z - CORDIC_ATAN_TABLE_Q16[i];
            } else {
                x_new = x + (y >> i);
                y_new = y - (x >> i);
                z_new = z + CORDIC_ATAN_TABLE_Q16[i];
            }
            
            x = x_new;
            y = y_new;
            z = z_new;
        }
    }
    
    // CORDIC vectoring mode: compute atan2
    template<typename raw_type>
    raw_type cordic_vectoring(raw_type x, raw_type y, int iterations) {
        raw_type z = 0;
        
        for (int i = 0; i < iterations && i < 16; ++i) {
            raw_type x_new, y_new, z_new;
            
            if (y < 0) {
                x_new = x - (y >> i);
                y_new = y + (x >> i);
                z_new = z - CORDIC_ATAN_TABLE_Q16[i];
            } else {
                x_new = x + (y >> i);
                y_new = y - (x >> i);
                z_new = z + CORDIC_ATAN_TABLE_Q16[i];
            }
            
            x = x_new;
            y = y_new;
            z = z_new;
        }
        
        return z;
    }
}

// Sin and Cos for Q15_16 format (32-bit, 16 fractional)
// Input angle in radians
template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
    requires (TotalBits == 32 && FracBits == 16)
inline auto sin(FixedPoint<TotalBits, FracBits, Signed, Policy> angle) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    using raw_type = typename FP::raw_type;
    
    // Range reduction to [-π, π]
    auto pi = constants::pi<FP>();
    auto two_pi_val = constants::two_pi<FP>();
    
    // Normalize to [-π, π]
    while (angle > pi) angle = angle - two_pi_val;
    while (angle < -pi) angle = angle + two_pi_val;
    
    raw_type z = angle.raw();
    raw_type x, y;
    
    // Handle quadrants
    bool negate = false;
    if (z < 0) {
        z = -z;
        negate = true;
    }
    
    detail::cordic_rotation(x, y, z, 16);
    
    auto result = FP::from_raw(y);
    return negate ? -result : result;
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
    requires (TotalBits == 32 && FracBits == 16)
inline auto cos(FixedPoint<TotalBits, FracBits, Signed, Policy> angle) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    using raw_type = typename FP::raw_type;
    
    // Range reduction to [-π, π]
    auto pi = constants::pi<FP>();
    auto two_pi_val = constants::two_pi<FP>();
    
    while (angle > pi) angle = angle - two_pi_val;
    while (angle < -pi) angle = angle + two_pi_val;
    
    raw_type z = angle.raw();
    raw_type x, y;
    
    bool negate = false;
    if (z < 0) {
        z = -z;
    }
    
    // Check if we're in quadrants 2 or 3 (cos is negative)
    auto pi_over_2 = constants::pi_over_2<FP>();
    if (angle > pi_over_2 || angle < -pi_over_2) {
        negate = true;
    }
    
    detail::cordic_rotation(x, y, z, 16);
    
    auto result = FP::from_raw(x);
    return negate ? -result : result;
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
    requires (TotalBits == 32 && FracBits == 16)
inline auto tan(FixedPoint<TotalBits, FracBits, Signed, Policy> angle) {
    auto s = sin(angle);
    auto c = cos(angle);
    return s / c;
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
    requires (TotalBits == 32 && FracBits == 16)
inline auto atan2(FixedPoint<TotalBits, FracBits, Signed, Policy> y,
                  FixedPoint<TotalBits, FracBits, Signed, Policy> x) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    using raw_type = typename FP::raw_type;
    
    if (x == FP::zero() && y == FP::zero()) {
        return FP::zero();
    }
    
    raw_type result = detail::cordic_vectoring(x.raw(), y.raw(), 16);
    
    // Adjust for quadrants
    auto pi = constants::pi<FP>();
    if (x < FP::zero()) {
        if (y >= FP::zero()) {
            result = pi.raw() - result;
        } else {
            result = -pi.raw() + result;
        }
    }
    
    return FP::from_raw(result);
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
    requires (TotalBits == 32 && FracBits == 16)
inline auto atan(FixedPoint<TotalBits, FracBits, Signed, Policy> x) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    return atan2(x, FP::one());
}

//
// Exponential and logarithmic functions (simplified implementations)
// Full implementations would use more sophisticated algorithms
//

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
inline auto exp2(FixedPoint<TotalBits, FracBits, Signed, Policy> x) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    using raw_type = typename FP::raw_type;
    
    // Extract integer and fractional parts
    auto integer_part = static_cast<int>(x);
    auto frac_part = x - FP(integer_part);
    
    // Check overflow/underflow
    if (integer_part >= (TotalBits - FracBits - 1)) {
        return FP::max();
    }
    if (integer_part < -FracBits) {
        return FP::zero();
    }
    
    // Compute 2^integer_part by shifting
    raw_type int_result = FP::one().raw();
    if (integer_part > 0) {
        int_result <<= integer_part;
    } else if (integer_part < 0) {
        int_result >>= -integer_part;
    }
    
    // Approximate 2^frac using polynomial (for frac in [0,1])
    // 2^x ≈ 1 + x*ln(2) + (x*ln(2))^2/2 + (x*ln(2))^3/6
    auto ln2 = FP(0.693147180559945);
    auto term = frac_part * ln2;
    auto result = FP::one() + term;
    term = term * term / FP(2);
    result = result + term;
    term = term * frac_part * ln2 / FP(3);
    result = result + term;
    
    return FP::from_raw(static_cast<raw_type>((static_cast<int64_t>(int_result) * result.raw()) >> FracBits));
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
inline auto exp(FixedPoint<TotalBits, FracBits, Signed, Policy> x) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    // e^x = 2^(x * log2(e))
    // log2(e) ≈ 1.442695
    auto log2e = FP(1.442695);
    return exp2(x * log2e);
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
inline auto log2(FixedPoint<TotalBits, FracBits, Signed, Policy> x) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    using raw_type = typename FP::raw_type;
    
    if (x <= FP::zero()) {
        return -FP::max(); // log of non-positive is -infinity
    }
    
    // Find highest bit position
    auto raw = x.raw();
    int bit_pos = std::bit_width(static_cast<std::make_unsigned_t<raw_type>>(raw)) - 1;
    
    // Integer part: bit_pos - FracBits
    int integer_part = bit_pos - FracBits;
    
    // Normalize to [1, 2)
    raw_type normalized;
    if (bit_pos >= FracBits) {
        normalized = raw >> (bit_pos - FracBits);
    } else {
        normalized = raw << (FracBits - bit_pos);
    }
    
    // Approximate log2 of normalized value in [1, 2) using polynomial
    // log2(1+x) ≈ x/ln(2) for small x
    auto frac = FP::from_raw(normalized) - FP::one();
    auto log_frac = frac / FP(0.693147); // divide by ln(2)
    
    return FP(integer_part) + log_frac;
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
inline auto log(FixedPoint<TotalBits, FracBits, Signed, Policy> x) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    // ln(x) = log2(x) * ln(2)
    return log2(x) * FP(0.693147);
}

template<int TotalBits, int FracBits, bool Signed, OverflowPolicy Policy>
inline auto pow(FixedPoint<TotalBits, FracBits, Signed, Policy> base,
                FixedPoint<TotalBits, FracBits, Signed, Policy> exponent) {
    using FP = FixedPoint<TotalBits, FracBits, Signed, Policy>;
    
    if (base <= FP::zero()) {
        return FP::zero();
    }
    
    // x^y = 2^(y * log2(x))
    return exp2(exponent * log2(base));
}

} // namespace fixp

#endif // FIXP_MATH_HPP
