#include "gtest/gtest.h"
#include <huestream/common/util/Rand.h>

#include <vector>

class TestRand : public testing::Test {

};

TEST_F(TestRand, Rand) {
    std::vector<double> res;
    int t = 0;
    for (int i = 0; i < 100; i++) {
        double val = huestream::Rand(-1, 1);
        EXPECT_TRUE((val >= -1) && (val <= 1));
        if (std::find(res.begin(), res.end(), val) != res.end())
            t++;
        res.push_back(val);
    }
    EXPECT_LT((t/100), 0.01);
    res.clear();
}
