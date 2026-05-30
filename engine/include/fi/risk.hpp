#pragma once
// Interest-rate risk measures: how the price responds to a change in yield.
//
//   Macaulay duration  - PV-weighted average time to cash flows (years)
//   Modified duration   - %% price change per unit yield  (Macaulay / (1 + y/f))
//   Convexity           - curvature of the price/yield relationship
//   DV01                - absolute price change per 1 basis point

#include <cmath>
#include "bond.hpp"
#include "yield.hpp"

namespace fi {

// PV-weighted average time, in years.
inline double macaulayDuration(const Bond& b, double yield) {
    const int f = freqInt(b.frequency);
    double pv = 0.0, weighted = 0.0;
    for (const auto& cf : cashflows(b)) {
        const double dpv = cf.amount / std::pow(1.0 + yield / f, cf.t);
        pv       += dpv;
        weighted += cf.years * dpv;
    }
    return pv > 0.0 ? weighted / pv : 0.0;
}

inline double modifiedDuration(const Bond& b, double yield) {
    const int f = freqInt(b.frequency);
    return macaulayDuration(b, yield) / (1.0 + yield / f);
}

// Annualised convexity: sum of t(t+1) PV over P (1+y/f)^2 f^2, with t in periods.
inline double convexity(const Bond& b, double yield) {
    const int f = freqInt(b.frequency);
    double pv = 0.0, curv = 0.0;
    for (const auto& cf : cashflows(b)) {
        const double dpv = cf.amount / std::pow(1.0 + yield / f, cf.t);
        pv   += dpv;
        curv += cf.t * (cf.t + 1.0) * dpv;
    }
    if (pv <= 0.0) return 0.0;
    return curv / (pv * std::pow(1.0 + yield / f, 2.0) * f * f);
}

// Dollar value of a basis point: price drop for a +1bp yield move.
inline double dv01(const Bond& b, double yield) {
    return modifiedDuration(b, yield) * dirtyPrice(b, yield) * 1e-4;
}

// Second-order price estimate for a yield shift, from duration + convexity.
inline double priceChangeEstimate(const Bond& b, double yield, double dy) {
    const double p = dirtyPrice(b, yield);
    return p * (-modifiedDuration(b, yield) * dy
                + 0.5 * convexity(b, yield) * dy * dy);
}

} // namespace fi
