#pragma once
// A fixed-rate coupon bond and its cash-flow schedule.
//
// Cash flows are timed in *coupon periods* from the settlement date. For a
// settlement that falls between coupons, the first period is fractional: `w` is
// the fraction of the current period still remaining, so cash-flow times are
// w, 1+w, 2+w, ...  This single representation handles both on-coupon pricing
// and dirty/clean pricing mid-period, and keeps the discounting per-cash-flow
// (so a 0% yield is well defined - no closed-form annuity singularity).

#include <vector>
#include "date.hpp"
#include "daycount.hpp"

namespace fi {

enum class Frequency { Annual = 1, SemiAnnual = 2, Quarterly = 4, Monthly = 12 };

inline int freqInt(Frequency f) { return static_cast<int>(f); }

struct Bond {
    double     faceValue  = 1000.0;             // redemption / par value
    double     couponRate = 0.0;                // annual coupon, decimal (0.05 = 5%)
    Frequency  frequency  = Frequency::SemiAnnual;
    Date       settlement = {2020, 1, 1};       // valuation date
    Date       maturity   = {2025, 1, 1};
    DayCount   dayCount   = DayCount::Thirty360;
};

struct Cashflow {
    double t;       // time in coupon periods from settlement (w, 1+w, ...)
    double years;   // same time expressed in years (t / frequency)
    double amount;  // coupon, plus face on the final flow
    Date   date;
};

// The coupon period currently in progress: the last coupon on/before
// settlement, the next coupon after it, and `w`, the fraction of that period
// still remaining (1.0 when settlement sits exactly on a coupon date).
struct CouponPeriod {
    Date   previous;
    Date   next;
    double w;
};

inline CouponPeriod currentPeriod(const Bond& b) {
    const int step = 12 / freqInt(b.frequency);  // months per coupon period
    Date prev = b.maturity;
    while (prev > b.settlement) prev = addMonths(prev, -step);
    Date next = addMonths(prev, step);
    const double periodYf = yearFraction(prev, next, b.dayCount);
    const double w = periodYf > 0.0
                         ? yearFraction(b.settlement, next, b.dayCount) / periodYf
                         : 1.0;
    return {prev, next, w};
}

// Future cash flows (those strictly after settlement), earliest first.
inline std::vector<Cashflow> cashflows(const Bond& b) {
    const int    f      = freqInt(b.frequency);
    const int    step   = 12 / f;
    const double coupon = b.faceValue * b.couponRate / f;
    const CouponPeriod cp = currentPeriod(b);

    std::vector<Cashflow> cfs;
    int i = 0;
    for (Date cd = cp.next; cd <= b.maturity; cd = addMonths(cd, step), ++i) {
        const double amount = coupon + (cd == b.maturity ? b.faceValue : 0.0);
        const double t = cp.w + static_cast<double>(i);
        cfs.push_back({t, t / f, amount, cd});
    }
    return cfs;
}

// Interest accrued to the seller since the last coupon: coupon * (1 - w).
inline double accruedInterest(const Bond& b) {
    const int    f      = freqInt(b.frequency);
    const double coupon = b.faceValue * b.couponRate / f;
    return coupon * (1.0 - currentPeriod(b).w);
}

} // namespace fi
