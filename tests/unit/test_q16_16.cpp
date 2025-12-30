#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "fixp_q16_16.h"

using namespace fixp;

TEST_CASE("Q16.16 Basic Operations", "[q16.16]") {
    SECTION("Construction and Conversion") {
        Q16_16 a(1.0);
        REQUIRE(static_cast<double>(a) == Catch::Approx(1.0).epsilon(0.001));
        REQUIRE(static_cast<int>(a) == 1);
        REQUIRE(a.raw() == 0x10000);

        Q16_16 b(0.5);
        REQUIRE(static_cast<double>(b) == Catch::Approx(0.5).epsilon(0.001));
        REQUIRE(b.raw() == 0x8000);

        Q16_16 c(-1.0);
        REQUIRE(static_cast<double>(c) == Catch::Approx(-1.0).epsilon(0.001));
    }

    SECTION("Arithmetic") {
        Q16_16 a(1.5);
        Q16_16 b(2.0);

        REQUIRE(static_cast<double>(a + b) == Catch::Approx(3.5));
        REQUIRE(static_cast<double>(b - a) == Catch::Approx(0.5));
        REQUIRE(static_cast<double>(a * b) == Catch::Approx(3.0));
        REQUIRE(static_cast<double>(b / a) == Catch::Approx(1.33333).epsilon(0.001));
    }

    SECTION("Trigonometry") {
        // The header defines macros Q16_16_PI which are wrappers.
        // Let's use the namespace functions
    }
}

TEST_CASE("Q16.16 Trig Functions", "[trig]") {
    // 0
    REQUIRE(static_cast<double>(fixp::sin(Q16_16(0.0))) == Catch::Approx(0.0).margin(0.001));
    REQUIRE(static_cast<double>(fixp::cos(Q16_16(0.0))) == Catch::Approx(1.0).margin(0.001));

    // PI/2
    q16_16_t pi_2_raw = Q16_16_PI_2;
    Q16_16 pi_2 = Q16_16::from_raw((int32_t)Q16_16_RAW(pi_2_raw));

    REQUIRE(static_cast<double>(fixp::sin(pi_2)) == Catch::Approx(1.0).margin(0.001));
    REQUIRE(static_cast<double>(fixp::cos(pi_2)) == Catch::Approx(0.0).margin(0.001));

    // PI/4
    q16_16_t pi_4_raw = Q16_16_PI_4;
    Q16_16 pi_4 = Q16_16::from_raw((int32_t)Q16_16_RAW(pi_4_raw));

    REQUIRE(static_cast<double>(fixp::tan(pi_4)) == Catch::Approx(1.0).margin(0.001));
}
