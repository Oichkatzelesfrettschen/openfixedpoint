#include <fixp/dsp.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace fixp;
using namespace fixp::dsp;

template<typename FP>
void test_fft() {
    std::cout << "Testing FFT\n";
    std::cout << "===========\n";
    
    // Test with simple signal: DC component + sine wave
    constexpr size_t N = 16;
    std::array<Complex<FP>, N> data;
    
    // Create test signal: DC + sine at bin 2
    for (size_t i = 0; i < N; ++i) {
        double angle = 2.0 * M_PI * 2.0 * static_cast<double>(i) / static_cast<double>(N); // 2 cycles in N samples
        data[i] = Complex<FP>(FP(1.0 + std::sin(angle)), FP(0));
    }
    
    std::cout << "Input (first 4 samples):\n";
    for (size_t i = 0; i < 4; ++i) {
        std::cout << "  [" << i << "] = " << static_cast<double>(data[i].real) << "\n";
    }
    
    // Perform FFT
    fft_radix2(data, false);
    
    std::cout << "\nFFT Output (magnitude of first 8 bins):\n";
    for (size_t i = 0; i < 8; ++i) {
        double mag = static_cast<double>(data[i].magnitude());
        std::cout << "  Bin " << i << ": " << std::setw(8) << mag << "\n";
    }
    
    // Perform IFFT
    fft_radix2(data, true);
    
    std::cout << "\nIFFT Output (first 4 samples, should match input):\n";
    for (size_t i = 0; i < 4; ++i) {
        std::cout << "  [" << i << "] = " << static_cast<double>(data[i].real) << "\n";
    }
    
    std::cout << "\n";
}

template<typename FP>
void test_fir_filter() {
    std::cout << "Testing FIR Filter\n";
    std::cout << "==================\n";
    
    // Simple 3-tap moving average filter
    constexpr size_t NumTaps = 3;
    constexpr size_t InputSize = 8;
    
    std::array<FP, NumTaps> coeffs = { FP(1.0/3.0), FP(1.0/3.0), FP(1.0/3.0) };
    std::array<FP, NumTaps - 1> state = { FP(0), FP(0) };
    std::array<FP, InputSize> input = { FP(1), FP(2), FP(3), FP(4), FP(3), FP(2), FP(1), FP(0) };
    std::array<FP, InputSize> output;
    
    fir_filter(input, output, coeffs, state);
    
    std::cout << "Input:  ";
    for (size_t i = 0; i < InputSize; ++i) {
        std::cout << std::setw(6) << static_cast<double>(input[i]) << " ";
    }
    std::cout << "\n";
    
    std::cout << "Output: ";
    for (size_t i = 0; i < InputSize; ++i) {
        std::cout << std::setw(6) << static_cast<double>(output[i]) << " ";
    }
    std::cout << "\n\n";
}

template<typename FP>
void test_biquad_filter() {
    std::cout << "Testing Biquad IIR Filter\n";
    std::cout << "=========================\n";
    
    // Simple lowpass filter
    BiquadFilter<FP> filter;
    filter.set_coefficients(
        FP(0.2), FP(0.4), FP(0.2),  // b0, b1, b2
        FP(-0.8), FP(0.2)            // a1, a2
    );
    
    // Test with impulse
    std::array<FP, 8> input = { FP(1), FP(0), FP(0), FP(0), FP(0), FP(0), FP(0), FP(0) };
    
    std::cout << "Impulse response:\n";
    for (size_t i = 0; i < 8; ++i) {
        double output = static_cast<double>(filter.process(input[i]));
        std::cout << "  [" << i << "] = " << std::setw(8) << output << "\n";
    }
    
    std::cout << "\n";
}

template<typename FP>
void test_windows() {
    std::cout << "Testing Window Functions\n";
    std::cout << "========================\n";
    
    constexpr size_t N = 16;
    
    auto hann = hann_window<FP, N>();
    auto hamming = hamming_window<FP, N>();
    auto blackman = blackman_window<FP, N>();
    
    std::cout << "Sample values (n=N/4):\n";
    std::cout << "  Hann:    " << static_cast<double>(hann[N/4]) << "\n";
    std::cout << "  Hamming: " << static_cast<double>(hamming[N/4]) << "\n";
    std::cout << "  Blackman:" << static_cast<double>(blackman[N/4]) << "\n";
    std::cout << "\n";
}

template<typename FP>
void test_convolution() {
    std::cout << "Testing Convolution\n";
    std::cout << "===================\n";
    
    std::array<FP, 3> x = { FP(1), FP(2), FP(3) };
    std::array<FP, 3> h = { FP(0.5), FP(1.0), FP(0.5) };
    
    auto result = convolve(x, h);
    
    std::cout << "x: ";
    for (auto v : x) std::cout << static_cast<double>(v) << " ";
    std::cout << "\n";
    
    std::cout << "h: ";
    for (auto v : h) std::cout << static_cast<double>(v) << " ";
    std::cout << "\n";
    
    std::cout << "Convolution result: ";
    for (auto v : result) std::cout << static_cast<double>(v) << " ";
    std::cout << "\n\n";
}

int main() {
    using FP = Q15_16;
    
    std::cout << "Fixed-Point DSP Function Tests\n";
    std::cout << "===============================\n\n";
    
    test_fft<FP>();
    test_fir_filter<FP>();
    test_biquad_filter<FP>();
    test_windows<FP>();
    test_convolution<FP>();
    
    std::cout << "All DSP tests completed!\n";
    
    return 0;
}
