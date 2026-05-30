#pragma once
// Facade: bundle every metric for a bond into one struct, plus the array-valued
// helpers the UI needs for charting. This is the single surface the WASM
// bindings expose, so the binding layer stays thin.

#include <vector>
#include "bond.hpp"
#include "curve.hpp"
#include "risk.hpp"
#include "yield.hpp"

namespace fi {

struct Analytics {
    double dirtyPrice;
    double cleanPrice;
    double accruedInterest;
    double macaulayDuration;
    double modifiedDuration;
    double convexity;
    double dv01;
    double ytmCheck;   // yield recovered from cleanPrice; should match the input
    int    periods;
};

inline Analytics analyze(const Bond& b, double yield) {
    Analytics a;
    a.dirtyPrice       = dirtyPrice(b, yield);
    a.accruedInterest  = accruedInterest(b);
    a.cleanPrice       = a.dirtyPrice - a.accruedInterest;
    a.macaulayDuration = macaulayDuration(b, yield);
    a.modifiedDuration = modifiedDuration(b, yield);
    a.convexity        = convexity(b, yield);
    a.dv01             = dv01(b, yield);
    a.ytmCheck         = yieldToMaturity(b, a.cleanPrice, yield);
    a.periods          = static_cast<int>(cashflows(b).size());
    return a;
}

struct Point { double x, y; };

// Clean price sampled across a yield range - the convex price/yield curve.
inline std::vector<Point> priceYieldCurve(const Bond& b, double yLo, double yHi, int n) {
    std::vector<Point> pts;
    pts.reserve(n + 1);
    for (int i = 0; i <= n; ++i) {
        const double y = yLo + (yHi - yLo) * i / n;
        pts.push_back({y, cleanPrice(b, y)});
    }
    return pts;
}

} // namespace fi
