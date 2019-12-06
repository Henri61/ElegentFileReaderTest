#include "pch.h"

#include "CalculationState.h"


//const double NO_AVG = score::DoubleState(score::CalculationState::NoAvg).as_double_state_in_high_dword();
const double NO_AVG = std::numeric_limits<double>::max()  / 1.996 / 9.01234e294;

// Used by Saturn code generator to convert NO_AVG to zero where required.
static __forceinline double __vectorcall HandleNoAvg(const double value) {
    return value == NO_AVG ? 0 : value;

    /*if (*reinterpret_cast<const int64_t*>(&value) == *reinterpret_cast<const int64_t*>(&NO_AVG))
        return 0.0;
    return value;*/
}

double d[] = { NO_AVG,2,3,4,5,6,7,8,9,10 };
double sums[] = { 0,0,0,0,0,0,0,0,0,0 };
size_t cc[] = { 0,0,0,0,0,0,0,0,0,0 };


TEST(ElegentFileReader, avxing) {
    size_t n;
    std::cout << "Testing AVX  - Run a small loop to debug \nEnter n <= 10 ";
    std::cin >> n;
    for (size_t i = 0; i < n; ++i) {
        sums[i] += (d[i] == NO_AVG) ? 0 : d[i];
    }
    for (size_t i = 0; i < n; ++i) {
        cc[i] += (d[i] == NO_AVG) ? 0 : 1;
    }
    EXPECT_EQ(0.0, sums[0]);
}

