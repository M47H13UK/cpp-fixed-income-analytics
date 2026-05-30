#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "fi/bond.hpp"
#include "fi/yield.hpp"

using Catch::Approx;

namespace {
// Helper: a bond settling on a coupon date, `years` to maturity.
fi::Bond make(double face, double couponPct, int years, fi::Frequency freq) {
    fi::Bond b;
    b.faceValue  = face;
    b.couponRate = couponPct / 100.0;
    b.frequency  = freq;
    b.settlement = {2020, 1, 1};
    b.maturity   = fi::addMonths(b.settlement, years * 12);
    b.dayCount   = fi::DayCount::Thirty360;
    return b;
}
} // namespace

TEST_CASE("annual coupon bond matches the textbook value", "[pricing]") {
    auto b = make(1000, 5.0, 5, fi::Frequency::Annual);
    REQUIRE(fi::cleanPrice(b, 0.06) == Approx(957.876).margin(0.01));
    REQUIRE(fi::accruedInterest(b) == Approx(0.0));  // on a coupon date
}

TEST_CASE("semi-annual coupon bond matches the textbook value", "[pricing]") {
    auto b = make(1000, 5.0, 5, fi::Frequency::SemiAnnual);
    REQUIRE(fi::cleanPrice(b, 0.06) == Approx(957.35).margin(0.01));
}

TEST_CASE("zero-coupon bond is the discounted face value", "[pricing]") {
    auto b = make(1000, 0.0, 5, fi::Frequency::Annual);
    REQUIRE(fi::cleanPrice(b, 0.06) == Approx(747.258).margin(0.01));
}

TEST_CASE("a bond priced at its coupon trades at par", "[pricing]") {
    auto b = make(1000, 6.0, 7, fi::Frequency::SemiAnnual);
    REQUIRE(fi::cleanPrice(b, 0.06) == Approx(1000.0).margin(1e-6));
}

TEST_CASE("zero yield sums undiscounted cash flows (no singularity)", "[pricing]") {
    auto b = make(1000, 5.0, 5, fi::Frequency::Annual);
    // 5 coupons of 50 + 1000 face = 1250
    REQUIRE(fi::dirtyPrice(b, 0.0) == Approx(1250.0));
}

TEST_CASE("YTM inverts pricing (round trip)", "[ytm]") {
    auto b = make(1000, 4.5, 10, fi::Frequency::SemiAnnual);
    for (double y : {0.0, 0.01, 0.045, 0.08, 0.15}) {
        const double clean = fi::cleanPrice(b, y);
        REQUIRE(fi::yieldToMaturity(b, clean) == Approx(y).margin(1e-7));
    }
}

TEST_CASE("accrued interest is non-zero between coupon dates", "[pricing]") {
    auto b = make(1000, 6.0, 5, fi::Frequency::SemiAnnual);
    b.settlement = {2020, 4, 1};   // ~3 months into a 6-month period
    const double accrued = fi::accruedInterest(b);
    REQUIRE(accrued > 0.0);
    REQUIRE(accrued < 30.0);       // less than a full 30 coupon
    // Dirty exceeds clean by exactly the accrued interest.
    REQUIRE(fi::dirtyPrice(b, 0.06) - fi::cleanPrice(b, 0.06) == Approx(accrued));
}
