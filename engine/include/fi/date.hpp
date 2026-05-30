#pragma once
// Lightweight proleptic-Gregorian civil date. No external dependency so it
// compiles to a small WASM bundle. Serial day number uses Howard Hinnant's
// well-known days_from_civil algorithm, giving correct actual-day differences
// across months, years, and leap years.

namespace fi {

struct Date {
    int year  = 1970;
    int month = 1;   // 1-12
    int day   = 1;   // 1-31

    // Days since 1970-01-01 (can be negative for earlier dates).
    long serial() const {
        int y = year;
        const unsigned m = static_cast<unsigned>(month);
        const unsigned d = static_cast<unsigned>(day);
        y -= (m <= 2);
        const int era = (y >= 0 ? y : y - 399) / 400;
        const unsigned yoe = static_cast<unsigned>(y - era * 400);
        const unsigned doy = (153u * (m + (m > 2 ? -3u : 9u)) + 2u) / 5u + d - 1u;
        const unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
        return static_cast<long>(era) * 146097L + static_cast<long>(doe) - 719468L;
    }
};

inline bool isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

inline int daysInMonth(int y, int m) {
    switch (m) {
        case 2:  return isLeap(y) ? 29 : 28;
        case 4: case 6: case 9: case 11: return 30;
        default: return 31;
    }
}

// Actual calendar days between two dates (b - a).
inline long operator-(const Date& a, const Date& b) { return a.serial() - b.serial(); }

inline bool operator<(const Date& a, const Date& b)  { return a.serial() <  b.serial(); }
inline bool operator==(const Date& a, const Date& b) { return a.serial() == b.serial(); }
inline bool operator!=(const Date& a, const Date& b) { return !(a == b); }
inline bool operator<=(const Date& a, const Date& b) { return a.serial() <= b.serial(); }
inline bool operator>(const Date& a, const Date& b)  { return b <  a; }
inline bool operator>=(const Date& a, const Date& b) { return b <= a; }

// Shift by whole months; clamps the day to the target month's length
// (e.g. Jan 31 + 1 month -> Feb 28/29). Adequate for coupon schedules.
inline Date addMonths(const Date& dt, int n) {
    int m0 = dt.month - 1 + n;
    int y  = dt.year + (m0 >= 0 ? m0 / 12 : (m0 - 11) / 12);
    int m  = m0 % 12;
    if (m < 0) m += 12;
    m += 1;
    int d   = dt.day;
    int dim = daysInMonth(y, m);
    if (d > dim) d = dim;
    return Date{y, m, d};
}

} // namespace fi
