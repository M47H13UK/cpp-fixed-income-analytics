// Interactive command-line driver over the fixed-income engine.
// Prices a fixed-rate coupon bond and reports price, accrued interest, and the
// core risk measures, then solves the inverse problem (price -> yield).

#include <iomanip>
#include <iostream>
#include <limits>

#include "fi/bond.hpp"
#include "fi/risk.hpp"
#include "fi/yield.hpp"

namespace {

double readPositive(const std::string& prompt, bool strict = true) {
    double v;
    std::cout << prompt;
    while (!(std::cin >> v) || v < 0 || (strict && v == 0)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Please enter a " << (strict ? "positive" : "non-negative")
                  << " number: ";
    }
    return v;
}

int readChoice(const std::string& prompt, std::initializer_list<int> allowed) {
    double v;
    std::cout << prompt;
    for (;;) {
        if (std::cin >> v && v == std::floor(v)) {
            const int iv = static_cast<int>(v);
            for (int a : allowed)
                if (a == iv) return iv;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Invalid. Try again: ";
    }
}

fi::Frequency toFrequency(int perYear) {
    switch (perYear) {
        case 1:  return fi::Frequency::Annual;
        case 2:  return fi::Frequency::SemiAnnual;
        case 4:  return fi::Frequency::Quarterly;
        default: return fi::Frequency::Monthly;
    }
}

} // namespace

int main() {
    std::cout << "Fixed-Income Bond Analytics\n"
              << "===========================\n\n";

    fi::Bond bond;
    bond.faceValue  = readPositive("Face value (e.g. 1000): ");
    bond.couponRate = readPositive("Coupon rate % (e.g. 5.5): ", false) / 100.0;
    const double ytm = readPositive("Market yield / YTM % (e.g. 6.0): ") / 100.0;
    const int years  = readChoice("Years to maturity (whole number): ", {1, 2, 3, 4, 5, 6, 7,
                                   8, 9, 10, 15, 20, 30});
    const int freq   = readChoice("Coupon frequency (1, 2, 4, 12 per year): ", {1, 2, 4, 12});

    bond.frequency  = toFrequency(freq);
    bond.maturity   = fi::addMonths(bond.settlement, years * 12);  // on-coupon pricing

    const double dirty = fi::dirtyPrice(bond, ytm);
    const double clean = fi::cleanPrice(bond, ytm);

    std::cout << std::fixed << std::setprecision(4) << '\n'
              << "Results\n-------\n"
              << "Coupon payment      : " << bond.faceValue * bond.couponRate / freq << '\n'
              << "Periods             : " << fi::cashflows(bond).size() << '\n'
              << "Clean price         : " << clean << '\n'
              << "Accrued interest    : " << fi::accruedInterest(bond) << '\n'
              << "Dirty price         : " << dirty << '\n'
              << "Macaulay duration   : " << fi::macaulayDuration(bond, ytm) << " yrs\n"
              << "Modified duration   : " << fi::modifiedDuration(bond, ytm) << " yrs\n"
              << "Convexity           : " << fi::convexity(bond, ytm) << '\n'
              << "DV01 (per 1bp)      : " << fi::dv01(bond, ytm) << '\n';

    // Demonstrate the inverse: recover the yield from the price we just printed.
    const double solved = fi::yieldToMaturity(bond, clean);
    std::cout << "YTM solved from price: " << solved * 100.0 << " %\n";

    return 0;
}
