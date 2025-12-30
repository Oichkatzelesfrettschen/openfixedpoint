#include <fixp/math.hpp>
#include <cmath>
#include <iostream>
#include <iomanip>

using namespace fixp;

// Test helper
template<typename FP>
void test_math_function(const char* name, FP result, double expected, double tolerance = 0.01) {
    double actual = static_cast<double>(result);
    double error = std::abs(actual - expected);
    bool passed = error <= tolerance;
    
    std::cout << std::setw(20) << name << ": ";
    std::cout << "result=" << std::setw(10) << actual;
    std::cout << ", expected=" << std::setw(10) << expected;
    std::cout << ", error=" << std::setw(10) << error;
    std::cout << " [" << (passed ? "PASS" : "FAIL") << "]\n";
}

int main() {
    using FP = Q15_16;
    
    std::cout << "Testing Fixed-Point Math Functions\n";
    std::cout << "===================================\n\n";
    
    // Test constants
    std::cout << "Constants:\n";
    test_math_function("pi", constants::pi<FP>(), M_PI);
    test_math_function("e", constants::e<FP>(), M_E);
    test_math_function("pi/2", constants::pi_over_2<FP>(), M_PI/2);
    test_math_function("pi/4", constants::pi_over_4<FP>(), M_PI/4);
    std::cout << "\n";
    
    // Test basic operations
    std::cout << "Basic Operations:\n";
    test_math_function("abs(-5)", abs(FP(-5.0)), 5.0);
    test_math_function("abs(5)", abs(FP(5.0)), 5.0);
    test_math_function("min(3,5)", min(FP(3.0), FP(5.0)), 3.0);
    test_math_function("max(3,5)", max(FP(3.0), FP(5.0)), 5.0);
    test_math_function("clamp(7,0,5)", clamp(FP(7.0), FP(0.0), FP(5.0)), 5.0);
    std::cout << "\n";
    
    // Test rounding
    std::cout << "Rounding:\n";
    test_math_function("floor(2.7)", floor(FP(2.7)), 2.0);
    test_math_function("floor(-2.7)", floor(FP(-2.7)), -3.0);
    test_math_function("ceil(2.3)", ceil(FP(2.3)), 3.0);
    test_math_function("ceil(-2.3)", ceil(FP(-2.3)), -2.0);
    test_math_function("round(2.5)", round(FP(2.5)), 3.0);
    test_math_function("round(-2.5)", round(FP(-2.5)), -3.0);
    test_math_function("trunc(2.7)", trunc(FP(2.7)), 2.0);
    test_math_function("trunc(-2.7)", trunc(FP(-2.7)), -2.0);
    std::cout << "\n";
    
    // Test sqrt
    std::cout << "Square Root:\n";
    test_math_function("sqrt(4)", sqrt(FP(4.0)), 2.0, 0.01);
    test_math_function("sqrt(9)", sqrt(FP(9.0)), 3.0, 0.01);
    test_math_function("sqrt(2)", sqrt(FP(2.0)), std::sqrt(2.0), 0.01);
    test_math_function("sqrt(0.25)", sqrt(FP(0.25)), 0.5, 0.01);
    std::cout << "\n";
    
    // Test trigonometry
    std::cout << "Trigonometry:\n";
    test_math_function("sin(0)", sin(FP(0.0)), 0.0, 0.01);
    test_math_function("sin(pi/6)", sin(constants::pi<FP>() / FP(6.0)), 0.5, 0.02);
    test_math_function("sin(pi/4)", sin(constants::pi_over_4<FP>()), std::sin(M_PI/4), 0.02);
    test_math_function("sin(pi/2)", sin(constants::pi_over_2<FP>()), 1.0, 0.02);
    test_math_function("cos(0)", cos(FP(0.0)), 1.0, 0.01);
    test_math_function("cos(pi/4)", cos(constants::pi_over_4<FP>()), std::cos(M_PI/4), 0.02);
    test_math_function("cos(pi/2)", cos(constants::pi_over_2<FP>()), 0.0, 0.02);
    test_math_function("tan(0)", tan(FP(0.0)), 0.0, 0.02);
    test_math_function("tan(pi/4)", tan(constants::pi_over_4<FP>()), 1.0, 0.05);
    std::cout << "\n";
    
    // Test atan/atan2
    std::cout << "Inverse Trigonometry:\n";
    test_math_function("atan(0)", atan(FP(0.0)), 0.0, 0.02);
    test_math_function("atan(1)", atan(FP(1.0)), M_PI/4, 0.05);
    test_math_function("atan2(1,1)", atan2(FP(1.0), FP(1.0)), M_PI/4, 0.05);
    test_math_function("atan2(1,0)", atan2(FP(1.0), FP(0.0)), M_PI/2, 0.05);
    std::cout << "\n";
    
    // Test exponential
    std::cout << "Exponential:\n";
    test_math_function("exp(0)", exp(FP(0.0)), 1.0, 0.05);
    test_math_function("exp(1)", exp(FP(1.0)), M_E, 0.1);
    test_math_function("exp(0.5)", exp(FP(0.5)), std::exp(0.5), 0.1);
    test_math_function("log(1)", log(FP(1.0)), 0.0, 0.05);
    test_math_function("log(e)", log(constants::e<FP>()), 1.0, 0.1);
    test_math_function("log(2)", log(FP(2.0)), std::log(2.0), 0.1);
    test_math_function("pow(2,3)", pow(FP(2.0), FP(3.0)), 8.0, 0.2);
    test_math_function("pow(2,0.5)", pow(FP(2.0), FP(0.5)), std::sqrt(2.0), 0.15);
    std::cout << "\n";
    
    std::cout << "All tests completed!\n";
    
    return 0;
}
