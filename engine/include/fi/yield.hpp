#pragma once
// Pricing and the inverse problem (yield to maturity).
//
// Price is the present value of the cash flows, each discounted at the periodic
// yield y/f. YTM is the y that reproduces a given clean price - found by
// Newton-Raphson with a guaranteed bisection fallback, since price is monotone
// decreasing in yield.

#include <cmath>
#include "bond.hpp"

namespace fi {

// Dirty price: PV of all future cash flows (includes accrued interest).
inline double dirtyPrice(const Bond& b, double yield) {
    const int f = freqInt(b.frequency);
    double pv = 0.0;
    for (const auto& cf : cashflows(b))
        pv += cf.amount / std::pow(1.0 + yield / f, cf.t);
    return pv;
}

// Clean (quoted) price: dirty minus accrued interest.
inline double cleanPrice(const Bond& b, double yield) {
    return dirtyPrice(b, yield) - accruedInterest(b);
}

// Yield to maturity from a clean price. `guess` seeds Newton-Raphson.
inline double yieldToMaturity(const Bond& b, double clean, double guess = 0.05) {
    const int f = freqInt(b.frequency);
    const auto err = [&](double y) { return cleanPrice(b, y) - clean; };

    // Newton-Raphson with a central-difference derivative.
    double y = guess;
    for (int it = 0; it < 100; ++it) {
        const double fy = err(y);
        const double h  = 1e-6;
        const double d  = (err(y + h) - err(y - h)) / (2.0 * h);
        if (std::abs(d) < 1e-12) break;
        const double next = y - fy / d;
        if (!std::isfinite(next)) break;
        if (std::abs(next - y) < 1e-12) { y = next; break; }
        y = next;
    }
    if (std::isfinite(y) && std::abs(err(y)) < 1e-7) return y;

    // Bisection fallback. Valid domain is y > -f (so 1 + y/f > 0).
    double lo = -0.99 * f, hi = 1.0;
    double flo = err(lo), fhi = err(hi);
    for (int guard = 0; flo * fhi > 0.0 && hi < 1e4 && guard < 80; ++guard) {
        hi *= 1.5;
        fhi = err(hi);
    }
    for (int it = 0; it < 200; ++it) {
        const double mid = 0.5 * (lo + hi);
        const double fm  = err(mid);
        if (std::abs(fm) < 1e-10) return mid;
        if (flo * fm <= 0.0) { hi = mid; fhi = fm; }
        else                 { lo = mid; flo = fm; }
    }
    return 0.5 * (lo + hi);
}

} // namespace fi
