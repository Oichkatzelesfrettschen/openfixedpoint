#ifndef FIXP_FIXED_POINT_HPP
#define FIXP_FIXED_POINT_HPP

#include <cstdint>
#include <concepts>
#include <type_traits>
#include <limits>
#include <compare>
#include <bit>
#include <cmath>

namespace fixp {

/**
 * @brief Overflow policies for fixed-point arithmetic
 */
enum class OverflowPolicy {
    Wrap,
    Saturate
};

/**
 * @brief Helper to select storage type based on bits
 */
template<int Bits, bool Signed>
struct StorageType;

template<int Bits, bool Signed>
    requires (Bits <= 8)
struct StorageType<Bits, Signed> { using type = std::conditional_t<Signed, int8_t, uint8_t>; };

template<int Bits, bool Signed>
    requires (Bits > 8 && Bits <= 16)
struct StorageType<Bits, Signed> { using type = std::conditional_t<Signed, int16_t, uint16_t>; };

template<int Bits, bool Signed>
    requires (Bits > 16 && Bits <= 32)
struct StorageType<Bits, Signed> { using type = std::conditional_t<Signed, int32_t, uint32_t>; };

template<int Bits, bool Signed>
    requires (Bits > 32 && Bits <= 64)
struct StorageType<Bits, Signed> { using type = std::conditional_t<Signed, int64_t, uint64_t>; };

#ifdef __SIZEOF_INT128__
template<int Bits, bool Signed>
    requires (Bits > 64 && Bits <= 128)
struct StorageType<Bits, Signed> { using type = std::conditional_t<Signed, __int128_t, __uint128_t>; };
#endif

template<int Bits, bool Signed>
using storage_t = typename StorageType<Bits, Signed>::type;

/**
 * @brief Universal Fixed-Point Template
 *
 * @tparam TotalBits Total number of bits
 * @tparam FracBits Number of fractional bits
 * @tparam Signed Whether the type is signed
 * @tparam Policy Overflow handling policy
 */
template<int TotalBits, int FracBits, bool Signed = true, OverflowPolicy Policy = OverflowPolicy::Wrap>
class FixedPoint {
public:
    using raw_type = storage_t<TotalBits, Signed>;
    static constexpr int integer_bits = TotalBits - FracBits - (Signed ? 1 : 0);
    static constexpr int fractional_bits = FracBits;

    // Check invariants
    static_assert(TotalBits > 0, "Total bits must be positive");
    static_assert(FracBits >= 0, "Fractional bits must be non-negative");
    static_assert(FracBits <= TotalBits - (Signed ? 1 : 0), "Fractional bits exceed capacity");

    constexpr FixedPoint() = default;

    // Explicit construction from raw value
    struct RawTag {};
    constexpr explicit FixedPoint(raw_type raw, RawTag) : m_value(raw) {}

    static constexpr FixedPoint from_raw(raw_type raw) {
        return FixedPoint(raw, RawTag{});
    }

    // Construction from arithmetic types
    constexpr FixedPoint(float f) {
        if constexpr (Policy == OverflowPolicy::Saturate) {
            double scaled = static_cast<double>(f) * (1ULL << FracBits);
            if (scaled >= std::numeric_limits<raw_type>::max()) m_value = std::numeric_limits<raw_type>::max();
            else if (scaled <= std::numeric_limits<raw_type>::min()) m_value = std::numeric_limits<raw_type>::min();
            else m_value = static_cast<raw_type>(scaled + (f >= 0 ? 0.5 : -0.5));
        } else {
             m_value = static_cast<raw_type>(f * (1ULL << FracBits) + (f >= 0 ? 0.5f : -0.5f));
        }
    }

    constexpr FixedPoint(double d) {
        if constexpr (Policy == OverflowPolicy::Saturate) {
            double scaled = d * (1ULL << FracBits);
            if (scaled >= std::numeric_limits<raw_type>::max()) m_value = std::numeric_limits<raw_type>::max();
            else if (scaled <= std::numeric_limits<raw_type>::min()) m_value = std::numeric_limits<raw_type>::min();
            else m_value = static_cast<raw_type>(scaled + (d >= 0 ? 0.5 : -0.5));
        } else {
            m_value = static_cast<raw_type>(d * (1ULL << FracBits) + (d >= 0 ? 0.5 : -0.5));
        }
    }

    constexpr FixedPoint(int i) {
        // Saturation check for integer conversion?
        // Shift can overflow
        if constexpr (Policy == OverflowPolicy::Saturate) {
             // Logic to check if i << FracBits fits
             // simplified for now
             m_value = static_cast<raw_type>(i) << FracBits;
        } else {
            m_value = static_cast<raw_type>(i) << FracBits;
        }
    }

    // Conversion
    constexpr explicit operator float() const {
        return static_cast<float>(m_value) / (1ULL << FracBits);
    }

    constexpr explicit operator double() const {
        return static_cast<double>(m_value) / (1ULL << FracBits);
    }

    constexpr explicit operator int() const {
        return static_cast<int>(m_value >> FracBits);
    }

    constexpr raw_type raw() const { return m_value; }

    // Arithmetic Operators
    constexpr FixedPoint operator+(const FixedPoint& other) const {
        if constexpr (Policy == OverflowPolicy::Saturate) {
            raw_type res;
            if (__builtin_add_overflow(m_value, other.m_value, &res)) {
                return (other.m_value > 0) ? max() : min();
            }
            return FixedPoint(res, RawTag{});
        } else {
            return FixedPoint(m_value + other.m_value, RawTag{});
        }
    }

    constexpr FixedPoint operator-(const FixedPoint& other) const {
        if constexpr (Policy == OverflowPolicy::Saturate) {
             raw_type res;
             if (__builtin_sub_overflow(m_value, other.m_value, &res)) {
                 return (other.m_value < 0) ? max() : min();
             }
             return FixedPoint(res, RawTag{});
        } else {
            return FixedPoint(m_value - other.m_value, RawTag{});
        }
    }

    constexpr FixedPoint operator*(const FixedPoint& other) const {
        // Simple widening multiply
        // Determine wide type
        using wide_type = storage_t<(TotalBits <= 32 ? TotalBits * 2 : 128), Signed>;

        wide_type product = static_cast<wide_type>(m_value) * static_cast<wide_type>(other.m_value);

        if constexpr (Policy == OverflowPolicy::Saturate) {
             // Check if result >> FracBits fits in raw_type
             wide_type rounding = (wide_type)1 << (FracBits - 1);
             wide_type res = (product + rounding) >> FracBits;
             if (res > std::numeric_limits<raw_type>::max()) return max();
             if (res < std::numeric_limits<raw_type>::min()) return min();
             return FixedPoint(static_cast<raw_type>(res), RawTag{});
        } else {
            wide_type rounding = (wide_type)1 << (FracBits - 1);
            return FixedPoint(static_cast<raw_type>((product + rounding) >> FracBits), RawTag{});
        }
    }

    constexpr FixedPoint operator/(const FixedPoint& other) const {
        using wide_type = storage_t<(TotalBits <= 32 ? TotalBits * 2 : 128), Signed>;

        if (other.m_value == 0) {
            // Division by zero
            return (m_value >= 0) ? max() : min();
        }

        wide_type dividend = static_cast<wide_type>(m_value) << FracBits;

        // Add half divisor for rounding? simplified for now.
        return FixedPoint(static_cast<raw_type>(dividend / other.m_value), RawTag{});
    }

    constexpr FixedPoint operator-() const {
        if constexpr (Policy == OverflowPolicy::Saturate) {
            if (m_value == std::numeric_limits<raw_type>::min()) return max();
        }
        return FixedPoint(-m_value, RawTag{});
    }

    constexpr auto operator<=>(const FixedPoint&) const = default;

    // Constants
    static constexpr FixedPoint max() { return FixedPoint(std::numeric_limits<raw_type>::max(), RawTag{}); }
    static constexpr FixedPoint min() { return FixedPoint(std::numeric_limits<raw_type>::min(), RawTag{}); }
    static constexpr FixedPoint epsilon() { return FixedPoint(1, RawTag{}); }
    static constexpr FixedPoint zero() { return FixedPoint(0, RawTag{}); }
    static constexpr FixedPoint one() { return FixedPoint(1ULL << FracBits, RawTag{}); }

private:
    raw_type m_value;
};

// Common Aliases
using Q15_16 = FixedPoint<32, 16>;
using Q16_16 = FixedPoint<32, 16>; // Often synonymous
using Q0_7   = FixedPoint<8, 7>;

} // namespace fixp

#endif // FIXP_FIXED_POINT_HPP
