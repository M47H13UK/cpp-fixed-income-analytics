// Emscripten/embind bindings: expose the C++ engine to JavaScript.
//
// The API is deliberately flat - functions take primitive arguments and return
// plain objects/arrays - so the React app never constructs C++ structs across
// the boundary. Rates are passed and returned in PERCENT (the UI's unit).

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "fi/analytics.hpp"

using namespace emscripten;

namespace {

// Build an on-coupon bond from UI inputs (settlement fixed; maturity = +years).
fi::Bond makeBond(double face, double couponPct, int freq, int years, int dayCount) {
    fi::Bond b;
    b.faceValue  = face;
    b.couponRate = couponPct / 100.0;
    b.frequency  = static_cast<fi::Frequency>(freq);
    b.settlement = {2020, 1, 1};
    b.maturity   = fi::addMonths(b.settlement, years * 12);
    b.dayCount   = static_cast<fi::DayCount>(dayCount);
    return b;
}

// Plain struct mirrored to a JS object via value_object below.
struct JsAnalytics {
    double dirty, clean, accrued, macaulay, modified, convexity, dv01, ytmCheck;
    int    periods;
};

JsAnalytics analyzeJs(double face, double couponPct, int freq, int years,
                      double ytmPct, int dayCount) {
    const fi::Bond b = makeBond(face, couponPct, freq, years, dayCount);
    const fi::Analytics a = fi::analyze(b, ytmPct / 100.0);
    return {a.dirtyPrice, a.cleanPrice, a.accruedInterest, a.macaulayDuration,
            a.modifiedDuration, a.convexity, a.dv01, a.ytmCheck * 100.0, a.periods};
}

val priceYieldCurveJs(double face, double couponPct, int freq, int years,
                      double yLoPct, double yHiPct, int n, int dayCount) {
    const fi::Bond b = makeBond(face, couponPct, freq, years, dayCount);
    const auto pts = fi::priceYieldCurve(b, yLoPct / 100.0, yHiPct / 100.0, n);
    val arr = val::array();
    for (std::size_t i = 0; i < pts.size(); ++i) {
        val o = val::object();
        o.set("yield", pts[i].x * 100.0);
        o.set("price", pts[i].y);
        arr.set(static_cast<int>(i), o);
    }
    return arr;
}

val cashflowsJs(double face, double couponPct, int freq, int years, int dayCount) {
    const fi::Bond b = makeBond(face, couponPct, freq, years, dayCount);
    val arr = val::array();
    int i = 0;
    for (const auto& cf : fi::cashflows(b)) {
        val o = val::object();
        o.set("year", cf.years);
        o.set("amount", cf.amount);
        arr.set(i++, o);
    }
    return arr;
}

double yieldToMaturityJs(double face, double couponPct, int freq, int years,
                         double cleanPrice, int dayCount) {
    const fi::Bond b = makeBond(face, couponPct, freq, years, dayCount);
    return fi::yieldToMaturity(b, cleanPrice) * 100.0;
}

// Bootstrap a zero curve from a JS array of annual par yields (percent, for
// maturities 1, 2, ... years) and return the par/zero/forward rate at each node.
val bootstrapCurveJs(val parYieldsPct) {
    const unsigned n = parYieldsPct["length"].as<unsigned>();
    std::vector<double> par;
    par.reserve(n);
    for (unsigned i = 0; i < n; ++i) par.push_back(parYieldsPct[i].as<double>() / 100.0);

    const fi::YieldCurve c = fi::bootstrapFromParYields(par);
    val arr = val::array();
    for (std::size_t i = 0; i < c.times.size(); ++i) {
        const double fwd = (i == 0)
            ? c.zeros[0]
            : c.forwardRate(c.times[i - 1], c.times[i]);
        val o = val::object();
        o.set("year", c.times[i]);
        o.set("par", par[i] * 100.0);
        o.set("zero", c.zeros[i] * 100.0);
        o.set("forward", fwd * 100.0);
        arr.set(static_cast<int>(i), o);
    }
    return arr;
}

} // namespace

EMSCRIPTEN_BINDINGS(fi_engine) {
    value_object<JsAnalytics>("Analytics")
        .field("dirty", &JsAnalytics::dirty)
        .field("clean", &JsAnalytics::clean)
        .field("accrued", &JsAnalytics::accrued)
        .field("macaulay", &JsAnalytics::macaulay)
        .field("modified", &JsAnalytics::modified)
        .field("convexity", &JsAnalytics::convexity)
        .field("dv01", &JsAnalytics::dv01)
        .field("ytmCheck", &JsAnalytics::ytmCheck)
        .field("periods", &JsAnalytics::periods);

    function("analyze", &analyzeJs);
    function("priceYieldCurve", &priceYieldCurveJs);
    function("cashflows", &cashflowsJs);
    function("yieldToMaturity", &yieldToMaturityJs);
    function("bootstrapCurve", &bootstrapCurveJs);
}
