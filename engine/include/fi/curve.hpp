#pragma once
// Term structure of interest rates.
//
// A single flat YTM is a simplification: in reality each cash flow should be
// discounted at the zero (spot) rate for its own horizon. This module holds a
// zero curve, derives discount factors and forward rates from it, prices a bond
// off it, and bootstraps it from observed par yields.

#include <cmath>
#include <vector>
#include "bond.hpp"

namespace fi {

// Annually-compounded zero (spot) rates at increasing maturities (in years).
// Linear interpolation between nodes; flat extrapolation past the ends.
struct YieldCurve {
    std::vector<double> times;  // strictly increasing, years
    std::vector<double> zeros;  // spot rates, decimal

    double zeroRate(double t) const {
        if (times.empty()) return 0.0;
        if (t <= times.front()) return zeros.front();
        if (t >= times.back())  return zeros.back();
        for (std::size_t i = 1; i < times.size(); ++i) {
            if (t <= times[i]) {
                const double w = (t - times[i - 1]) / (times[i] - times[i - 1]);
                return zeros[i - 1] + w * (zeros[i] - zeros[i - 1]);
            }
        }
        return zeros.back();
    }

    // Present value today of 1 unit paid at time t (annual compounding).
    double discountFactor(double t) const {
        return std::pow(1.0 + zeroRate(t), -t);
    }

    // Annualised forward rate for the period [t1, t2], implied by the curve.
    double forwardRate(double t1, double t2) const {
        const double df1 = discountFactor(t1);
        const double df2 = discountFactor(t2);
        return std::pow(df1 / df2, 1.0 / (t2 - t1)) - 1.0;
    }
};

// Dirty price obtained by discounting each cash flow at its own spot rate.
inline double priceWithCurve(const Bond& b, const YieldCurve& curve) {
    double pv = 0.0;
    for (const auto& cf : cashflows(b))
        pv += cf.amount * curve.discountFactor(cf.years);
    return pv;
}

// Bootstrap a zero curve from annual par yields.
// parYields[i] is the coupon (= YTM) of a bond priced at par, maturing in
// (i + 1) years, paying annual coupons. Spot rates are solved one maturity at a
// time from the par condition  1 = c * sum(df_k) + df_N.
inline YieldCurve bootstrapFromParYields(const std::vector<double>& parYields) {
    YieldCurve curve;
    std::vector<double> df;  // discount factors at 1, 2, ... years
    for (std::size_t n = 0; n < parYields.size(); ++n) {
        const double c = parYields[n];
        double sum = 0.0;
        for (std::size_t k = 0; k < n; ++k) sum += c * df[k];
        const double dfn = (1.0 - sum) / (1.0 + c);
        df.push_back(dfn);

        const double t = static_cast<double>(n + 1);
        curve.times.push_back(t);
        curve.zeros.push_back(std::pow(dfn, -1.0 / t) - 1.0);
    }
    return curve;
}

} // namespace fi
