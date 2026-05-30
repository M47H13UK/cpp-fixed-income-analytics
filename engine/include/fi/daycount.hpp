#pragma once
// Day-count conventions: the rule for turning a pair of dates into a year
// fraction. Different markets quote different conventions, and they change the
// accrued interest and therefore the dirty price.

#include "date.hpp"

namespace fi {

enum class DayCount {
    Thirty360,  // 30/360 US (NASD) - common for US corporates
    Act360,     // Actual/360 - money markets
    Act365F,    // Actual/365 Fixed
    ActAct      // Actual/Actual (ISDA)
};

// 30/360 US: each month treated as 30 days, year as 360.
inline double thirty360(const Date& s, const Date& e) {
    int d1 = s.day, d2 = e.day;
    if (d1 == 31) d1 = 30;
    if (d2 == 31 && d1 == 30) d2 = 30;
    return ((e.year - s.year) * 360 + (e.month - s.month) * 30 + (d2 - d1)) / 360.0;
}

inline double yearFraction(const Date& s, const Date& e, DayCount dc) {
    switch (dc) {
        case DayCount::Thirty360:
            return thirty360(s, e);
        case DayCount::Act360:
            return (e - s) / 360.0;
        case DayCount::Act365F:
            return (e - s) / 365.0;
        case DayCount::ActAct: {
            // ISDA: split the interval at year boundaries, each piece divided
            // by the number of days in its own (leap-aware) year.
            if (s == e) return 0.0;
            if (e < s)  return -yearFraction(e, s, dc);
            double sum = 0.0;
            for (int y = s.year; y <= e.year; ++y) {
                Date ys = (y == s.year) ? s : Date{y, 1, 1};
                Date ye = (y == e.year) ? e : Date{y + 1, 1, 1};
                double denom = isLeap(y) ? 366.0 : 365.0;
                sum += (ye - ys) / denom;
            }
            return sum;
        }
    }
    return 0.0;
}

} // namespace fi
