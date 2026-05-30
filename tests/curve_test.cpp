#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "fi/bond.hpp"
#include "fi/curve.hpp"
#include "fi/yield.hpp"

using Catch::Approx;

namespace {
fi::Bond annualBond(double couponPct, int years) {
    fi::Bond b;
    b.faceValue  = 1000.0;
    b.couponRate = couponPct / 100.0;
    b.frequency  = fi::Frequency::Annual;
    b.settlement = {2020, 1, 1};
    b.maturity   = fi::addMonths(b.settlement, years * 12);
    return b;
}
} // namespace

TEST_CASE("flat zero curve reproduces flat-yield pricing", "[curve]") {
    fi::YieldCurve flat{{1, 2, 3, 4, 5}, {0.06, 0.06, 0.06, 0.06, 0.06}};
    auto b = annualBond(5.0, 5);
    REQUIRE(fi::priceWithCurve(b, flat) == Approx(fi::dirtyPrice(b, 0.06)));
}

TEST_CASE("discount factors and forward rates are consistent", "[curve]") {
    fi::YieldCurve c{{1, 2, 3}, {0.03, 0.035, 0.04}};
    const double f = c.forwardRate(1.0, 3.0);
    // Rebuilding df(3) from df(1) and the forward must return df(3).
    REQUIRE(c.discountFactor(1.0) * std::pow(1.0 + f, -2.0)
            == Approx(c.discountFactor(3.0)));
}

TEST_CASE("flat par yields bootstrap to flat spot rates", "[curve]") {
    auto c = fi::bootstrapFromParYields({0.05, 0.05, 0.05, 0.05});
    for (double z : c.zeros) REQUIRE(z == Approx(0.05).margin(1e-12));
}

TEST_CASE("bootstrapped curve reprices its par bonds at par", "[curve]") {
    const std::vector<double> par{0.02, 0.025, 0.03, 0.035, 0.04};
    auto c = fi::bootstrapFromParYields(par);
    for (std::size_t n = 0; n < par.size(); ++n) {
        auto b = annualBond(par[n] * 100.0, static_cast<int>(n + 1));
        REQUIRE(fi::priceWithCurve(b, c) == Approx(b.faceValue).margin(1e-7));
    }
}

TEST_CASE("an upward curve prices a par-coupon bond below par", "[curve]") {
    // Rising spot rates -> later cash flows discounted harder -> price < face.
    auto c = fi::bootstrapFromParYields({0.02, 0.03, 0.04, 0.05, 0.06});
    auto b = annualBond(4.0, 5);  // 4% coupon against a steeper curve
    REQUIRE(fi::priceWithCurve(b, c) < b.faceValue);
}
