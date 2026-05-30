#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "fi/bond.hpp"
#include "fi/risk.hpp"
#include "fi/yield.hpp"

using Catch::Approx;

namespace {
fi::Bond make(double couponPct, int years, fi::Frequency freq) {
    fi::Bond b;
    b.faceValue  = 1000.0;
    b.couponRate = couponPct / 100.0;
    b.frequency  = freq;
    b.settlement = {2020, 1, 1};
    b.maturity   = fi::addMonths(b.settlement, years * 12);
    return b;
}
} // namespace

TEST_CASE("modified duration agrees with a numerical bump", "[risk]") {
    auto b = make(5.0, 10, fi::Frequency::SemiAnnual);
    const double y = 0.06, h = 1e-5;
    // Numerical modified duration: -(dP/dy)/P via central difference on dirty price.
    const double p  = fi::dirtyPrice(b, y);
    const double num = -(fi::dirtyPrice(b, y + h) - fi::dirtyPrice(b, y - h)) / (2 * h) / p;
    REQUIRE(fi::modifiedDuration(b, y) == Approx(num).margin(1e-4));
}

TEST_CASE("convexity agrees with a numerical second derivative", "[risk]") {
    auto b = make(5.0, 10, fi::Frequency::SemiAnnual);
    const double y = 0.06, h = 1e-4;
    const double p  = fi::dirtyPrice(b, y);
    const double d2 = (fi::dirtyPrice(b, y + h) - 2 * p + fi::dirtyPrice(b, y - h)) / (h * h);
    REQUIRE(fi::convexity(b, y) == Approx(d2 / p).margin(1e-2));
}

TEST_CASE("Macaulay duration of a zero-coupon bond equals its maturity", "[risk]") {
    auto b = make(0.0, 5, fi::Frequency::Annual);
    REQUIRE(fi::macaulayDuration(b, 0.06) == Approx(5.0).margin(1e-9));
}

TEST_CASE("duration + convexity estimate tracks a full reprice for small shifts", "[risk]") {
    auto b = make(4.0, 20, fi::Frequency::SemiAnnual);
    const double y = 0.05, dy = 0.0010;  // +10bp
    const double estimate = fi::priceChangeEstimate(b, y, dy);
    const double actual   = fi::dirtyPrice(b, y + dy) - fi::dirtyPrice(b, y);
    REQUIRE(estimate == Approx(actual).margin(0.05));
}

TEST_CASE("DV01 is positive and small relative to price", "[risk]") {
    auto b = make(5.0, 10, fi::Frequency::SemiAnnual);
    const double d = fi::dv01(b, 0.06);
    REQUIRE(d > 0.0);
    REQUIRE(d < fi::dirtyPrice(b, 0.06));
}
