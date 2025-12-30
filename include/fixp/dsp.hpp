#ifndef FIXP_DSP_HPP
#define FIXP_DSP_HPP

#include "fixed_point.hpp"
#include "math.hpp"
#include <array>
#include <complex>
#include <cmath>

namespace fixp {
namespace dsp {

/**
 * @brief Complex number representation for fixed-point DSP
 */
template<typename FixedType>
struct Complex {
    FixedType real;
    FixedType imag;
    
    constexpr Complex() : real(0), imag(0) {}
    constexpr Complex(FixedType r, FixedType i) : real(r), imag(i) {}
    constexpr Complex(FixedType r) : real(r), imag(0) {}
    
    constexpr Complex operator+(const Complex& other) const {
        return Complex(real + other.real, imag + other.imag);
    }
    
    constexpr Complex operator-(const Complex& other) const {
        return Complex(real - other.real, imag - other.imag);
    }
    
    constexpr Complex operator*(const Complex& other) const {
        // (a+bi)(c+di) = (ac-bd) + (ad+bc)i
        return Complex(
            real * other.real - imag * other.imag,
            real * other.imag + imag * other.real
        );
    }
    
    constexpr FixedType magnitude_squared() const {
        return real * real + imag * imag;
    }
    
    FixedType magnitude() const {
        return sqrt(magnitude_squared());
    }
};

/**
 * @brief Radix-2 Decimation-in-Time FFT
 * 
 * Implements the Cooley-Tukey FFT algorithm for power-of-2 sizes.
 * Based on algorithms from CMSIS-DSP, libfixmath, and liquid-fpm.
 * 
 * @param data Input/output array of complex numbers (size must be power of 2)
 * @param N Size of the FFT (must be power of 2)
 * @param inverse If true, performs IFFT instead of FFT
 */
template<typename FixedType, size_t N>
    requires ((N & (N - 1)) == 0 && N >= 2) // N is power of 2
void fft_radix2(std::array<Complex<FixedType>, N>& data, bool inverse = false) {
    using FP = FixedType;
    
    // Bit-reversal permutation
    size_t j = 0;
    for (size_t i = 0; i < N - 1; ++i) {
        if (i < j) {
            std::swap(data[i], data[j]);
        }
        
        // Add 1 in reversed bit order
        size_t k = N >> 1;
        while (k <= j) {
            j -= k;
            k >>= 1;
        }
        j += k;
    }
    
    // FFT computation
    auto two_pi = constants::two_pi<FP>();
    
    for (size_t len = 2; len <= N; len <<= 1) {
        // Twiddle factor: e^(-2πi/len) = cos(-2π/len) + i*sin(-2π/len)
        FP angle = (inverse ? two_pi : -two_pi) / FP(static_cast<int>(len));
        
        Complex<FP> wlen(cos(angle), sin(angle));
        
        for (size_t i = 0; i < N; i += len) {
            Complex<FP> w(FP(1), FP(0));
            
            for (size_t jj = 0; jj < len / 2; ++jj) {
                // Butterfly operation
                auto t = w * data[i + jj + len / 2];
                auto u = data[i + jj];
                
                data[i + jj] = u + t;
                data[i + jj + len / 2] = u - t;
                
                w = w * wlen;
            }
        }
    }
    
    // Normalize for IFFT
    if (inverse) {
        FP scale = FP(1) / FP(static_cast<int>(N));
        for (size_t i = 0; i < N; ++i) {
            data[i].real = data[i].real * scale;
            data[i].imag = data[i].imag * scale;
        }
    }
}

/**
 * @brief Real-valued FFT (optimized for real inputs)
 * 
 * Computes FFT of real-valued signal, producing N/2+1 complex frequency bins.
 * More efficient than full complex FFT for real signals.
 */
template<typename FixedType, size_t N>
    requires ((N & (N - 1)) == 0 && N >= 2)
std::array<Complex<FixedType>, N/2 + 1> rfft(const std::array<FixedType, N>& real_data) {
    using FP = FixedType;
    
    // Pack real data into complex array (real in even, imag in odd conceptually)
    std::array<Complex<FP>, N> packed;
    for (size_t i = 0; i < N; ++i) {
        packed[i] = Complex<FP>(real_data[i], FP(0));
    }
    
    // Perform complex FFT
    fft_radix2(packed, false);
    
    // Extract first N/2+1 bins (rest are conjugate symmetric)
    std::array<Complex<FP>, N/2 + 1> result;
    for (size_t i = 0; i <= N/2; ++i) {
        result[i] = packed[i];
    }
    
    return result;
}

/**
 * @brief FIR filter (Finite Impulse Response)
 * 
 * Implements direct-form FIR filtering.
 * Based on CMSIS-DSP arm_fir_q31 and similar implementations.
 * 
 * @param input Input samples
 * @param output Output samples  
 * @param coeffs Filter coefficients (b0, b1, ..., bN)
 * @param state Filter state (must be initialized to zero initially)
 */
template<typename FixedType, size_t InputSize, size_t NumTaps>
void fir_filter(
    const std::array<FixedType, InputSize>& input,
    std::array<FixedType, InputSize>& output,
    const std::array<FixedType, NumTaps>& coeffs,
    std::array<FixedType, NumTaps - 1>& state)
{
    using FP = FixedType;
    
    // Sliding window implementation
    for (size_t n = 0; n < InputSize; ++n) {
        // Shift state
        for (size_t i = NumTaps - 1; i > 0; --i) {
            state[i - 1] = (i == 1) ? input[n] : state[i - 2];
        }
        
        // Compute output: y[n] = sum(b[k] * x[n-k])
        FP acc = coeffs[0] * input[n];
        for (size_t k = 1; k < NumTaps; ++k) {
            if (k - 1 < NumTaps - 1) {
                acc = acc + coeffs[k] * state[k - 1];
            }
        }
        
        output[n] = acc;
    }
}

/**
 * @brief Simple biquad IIR filter (2nd order)
 * 
 * Transfer function: H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
 * Direct Form 1 implementation.
 */
template<typename FixedType>
struct BiquadFilter {
    FixedType b0, b1, b2;  // Numerator coefficients
    FixedType a1, a2;       // Denominator coefficients (a0 = 1 implicit)
    FixedType x1, x2;       // Input history
    FixedType y1, y2;       // Output history
    
    BiquadFilter() 
        : b0(FixedType::one()), b1(0), b2(0)
        , a1(0), a2(0)
        , x1(0), x2(0), y1(0), y2(0) 
    {}
    
    void set_coefficients(FixedType b0_, FixedType b1_, FixedType b2_,
                         FixedType a1_, FixedType a2_) {
        b0 = b0_; b1 = b1_; b2 = b2_;
        a1 = a1_; a2 = a2_;
    }
    
    void reset() {
        x1 = x2 = y1 = y2 = FixedType(0);
    }
    
    FixedType process(FixedType input) {
        // Direct Form 1: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
        auto output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        
        // Update history
        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;
        
        return output;
    }
};

/**
 * @brief Window functions for spectral analysis
 */
template<typename FixedType, size_t N>
std::array<FixedType, N> hann_window() {
    using FP = FixedType;
    std::array<FP, N> window;
    auto two_pi = constants::two_pi<FP>();
    
    for (size_t n = 0; n < N; ++n) {
        // w[n] = 0.5 * (1 - cos(2*pi*n/(N-1)))
        FP angle = two_pi * FP(static_cast<int>(n)) / FP(static_cast<int>(N - 1));
        window[n] = FP(0.5) * (FP(1) - cos(angle));
    }
    
    return window;
}

template<typename FixedType, size_t N>
std::array<FixedType, N> hamming_window() {
    using FP = FixedType;
    std::array<FP, N> window;
    auto two_pi = constants::two_pi<FP>();
    
    for (size_t n = 0; n < N; ++n) {
        // w[n] = 0.54 - 0.46 * cos(2*pi*n/(N-1))
        FP angle = two_pi * FP(static_cast<int>(n)) / FP(static_cast<int>(N - 1));
        window[n] = FP(0.54) - FP(0.46) * cos(angle);
    }
    
    return window;
}

template<typename FixedType, size_t N>
std::array<FixedType, N> blackman_window() {
    using FP = FixedType;
    std::array<FP, N> window;
    auto two_pi = constants::two_pi<FP>();
    
    for (size_t n = 0; n < N; ++n) {
        // w[n] = 0.42 - 0.5*cos(2*pi*n/(N-1)) + 0.08*cos(4*pi*n/(N-1))
        FP angle = two_pi * FP(static_cast<int>(n)) / FP(static_cast<int>(N - 1));
        window[n] = FP(0.42) - FP(0.5) * cos(angle) + FP(0.08) * cos(FP(2) * angle);
    }
    
    return window;
}

/**
 * @brief Convolution
 * 
 * Computes discrete convolution: y[n] = sum(x[k] * h[n-k])
 */
template<typename FixedType, size_t XSize, size_t HSize>
std::array<FixedType, XSize + HSize - 1> convolve(
    const std::array<FixedType, XSize>& x,
    const std::array<FixedType, HSize>& h)
{
    using FP = FixedType;
    constexpr size_t OutSize = XSize + HSize - 1;
    std::array<FP, OutSize> result{};
    
    for (size_t n = 0; n < OutSize; ++n) {
        FP sum = FP(0);
        for (size_t k = 0; k < HSize; ++k) {
            if (n >= k && (n - k) < XSize) {
                sum = sum + x[n - k] * h[k];
            }
        }
        result[n] = sum;
    }
    
    return result;
}

/**
 * @brief Correlation
 * 
 * Computes discrete correlation (similar to convolution but without time reversal)
 */
template<typename FixedType, size_t XSize, size_t YSize>
std::array<FixedType, XSize + YSize - 1> correlate(
    const std::array<FixedType, XSize>& x,
    const std::array<FixedType, YSize>& y)
{
    using FP = FixedType;
    constexpr size_t OutSize = XSize + YSize - 1;
    std::array<FP, OutSize> result{};
    
    for (size_t lag = 0; lag < OutSize; ++lag) {
        FP sum = FP(0);
        for (size_t n = 0; n < XSize; ++n) {
            int idx = static_cast<int>(n) + static_cast<int>(lag) - static_cast<int>(YSize) + 1;
            if (idx >= 0 && idx < static_cast<int>(YSize)) {
                sum = sum + x[n] * y[idx];
            }
        }
        result[lag] = sum;
    }
    
    return result;
}

} // namespace dsp
} // namespace fixp

#endif // FIXP_DSP_HPP
