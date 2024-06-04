#include "gtest/gtest.h"
#include "harmonics_handler.h"
#include "harmonics_calculator.h"
#include <boost/filesystem.hpp>
#include <constants.h>

class HarmonicsHandlerTest : public ::testing::Test {
protected:

    static boost::filesystem::path test_file;
    static HarmonicsHandler handler;

    // Setup before the first test
    // the calculation
    static void SetUpTestSuite() {
        test_file = TEST_DATA_DIR + "quad_test.json";
        HarmonicsCalculator calculator(test_file);
        calculator.calc(handler, true);
    }
};

// Initialize static members
boost::filesystem::path HarmonicsHandlerTest::test_file;
HarmonicsHandler HarmonicsHandlerTest::handler;

TEST_F(HarmonicsHandlerTest, CorrectSizesEllBn) {
    std::vector<double> ell = handler.get_ell();
    std::vector<std::vector<double>> Bn_components;
    
    for (int i = 1; i <= 10; ++i) {
        EXPECT_EQ(ell.size(), handler.get_Bn(i).size());
    }
}

TEST_F(HarmonicsHandlerTest, GetBnCorrectSizes) {
    // there should be Bn values for B1 - B10. 
    for (int i = 1; i <= 10; ++i) {
        std::vector<double> Bn = handler.get_Bn(i);
        EXPECT_FALSE(Bn.empty());
    }

    EXPECT_TRUE(handler.get_Bn(0).empty());
    EXPECT_TRUE(handler.get_Bn(11).empty());
}

TEST_F(HarmonicsHandlerTest, GetbnCorrectSizes) {
    std::vector<double> bn = handler.get_bn();

    // should have bn values for B1 to B10
    EXPECT_FALSE(bn.empty());
    EXPECT_EQ(bn.size(), 10);
}

TEST_F(HarmonicsHandlerTest, ConstructorHandlesNullData) {
    ASSERT_NO_THROW({
        HarmonicsHandler handler(nullptr);
        EXPECT_TRUE(handler.get_ell().empty());
        EXPECT_TRUE(handler.get_bn().empty());
    });
}
