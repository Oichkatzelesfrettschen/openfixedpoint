#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "fixp/fixed_point.hpp"

using namespace fixp;

TEST_CASE("FixedPoint Template Basic Operations", "[template]") {
    // Q16.16 equivalent
    using fp32 = FixedPoint<32, 16>;

    SECTION("Construction") {
        fp32 a(1.0);
        REQUIRE(static_cast<double>(a) == Catch::Approx(1.0));
        REQUIRE(a.raw() == 65536);

        fp32 b(-0.5);
        REQUIRE(static_cast<double>(b) == Catch::Approx(-0.5));
    }

    SECTION("Addition") {
        fp32 a(1.5);
        fp32 b(2.25);
        fp32 c = a + b;
        REQUIRE(static_cast<double>(c) == Catch::Approx(3.75));
    }

    SECTION("Multiplication") {
        fp32 a(2.0);
        fp32 b(3.0);
        fp32 c = a * b;
        REQUIRE(static_cast<double>(c) == Catch::Approx(6.0));
    }
}

TEST_CASE("FixedPoint Saturation", "[template][saturation]") {
    using sat8 = FixedPoint<8, 4, true, OverflowPolicy::Saturate>;
    // Range: -8.0 to 7.9375
    // Max: 0x7F (127) -> 7.9375
    // Min: 0x80 (-128) -> -8.0

    SECTION("Add Saturation") {
        sat8 a(7.0);
        sat8 b(2.0); // Sum 9.0 -> Overflow
        sat8 c = a + b;
        REQUIRE(c == sat8::max());
    }

    SECTION("Sub Saturation") {
        sat8 a(-7.0);
        sat8 b(2.0); // -9.0 -> Underflow
        sat8 c = a - b;
        REQUIRE(c == sat8::min());
    }
}

TEST_CASE("FixedPoint High Precision", "[template][wide]") {
    using hp = FixedPoint<64, 32>; // Q32.32

    SECTION("Precision") {
        hp a(1.0 / 3.0); // 0.3333333333...
        double d = static_cast<double>(a);
        REQUIRE(d == Catch::Approx(0.3333333333).epsilon(1e-9));
    }
}
