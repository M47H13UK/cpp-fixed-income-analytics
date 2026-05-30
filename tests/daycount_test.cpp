#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "fi/daycount.hpp"
#include "fi/date.hpp"

using fi::Date;
using fi::DayCount;
using fi::yearFraction;
using Catch::Approx;

TEST_CASE("actual day differences span months and leap years", "[date]") {
    REQUIRE(Date{2021, 1, 1} - Date{2020, 1, 1} == 366);  // 2020 is a leap year
    REQUIRE(Date{2022, 1, 1} - Date{2021, 1, 1} == 365);
    REQUIRE(Date{2020, 3, 1} - Date{2020, 2, 1} == 29);   // leap February
}

TEST_CASE("addMonths clamps to month end", "[date]") {
    REQUIRE(fi::addMonths(Date{2021, 1, 31}, 1) == Date{2021, 2, 28});
    REQUIRE(fi::addMonths(Date{2020, 1, 31}, 1) == Date{2020, 2, 29});
    REQUIRE(fi::addMonths(Date{2020, 12, 15}, 1) == Date{2021, 1, 15});
    REQUIRE(fi::addMonths(Date{2020, 1, 15}, -1) == Date{2019, 12, 15});
}

TEST_CASE("30/360 treats a full year as 1.0", "[daycount]") {
    REQUIRE(yearFraction(Date{2020, 1, 1}, Date{2021, 1, 1}, DayCount::Thirty360)
            == Approx(1.0));
    REQUIRE(yearFraction(Date{2020, 1, 1}, Date{2020, 7, 1}, DayCount::Thirty360)
            == Approx(0.5));
}

TEST_CASE("actual conventions divide by their day basis", "[daycount]") {
    // 2020 is a leap year: 366 actual days.
    REQUIRE(yearFraction(Date{2020, 1, 1}, Date{2021, 1, 1}, DayCount::Act360)
            == Approx(366.0 / 360.0));
    REQUIRE(yearFraction(Date{2020, 1, 1}, Date{2021, 1, 1}, DayCount::Act365F)
            == Approx(366.0 / 365.0));
    // ACT/ACT ISDA: a full calendar year is exactly 1.0 regardless of leap.
    REQUIRE(yearFraction(Date{2020, 1, 1}, Date{2021, 1, 1}, DayCount::ActAct)
            == Approx(1.0));
    REQUIRE(yearFraction(Date{2021, 1, 1}, Date{2022, 1, 1}, DayCount::ActAct)
            == Approx(1.0));
}
