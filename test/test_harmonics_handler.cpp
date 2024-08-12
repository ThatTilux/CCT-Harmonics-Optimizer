#include "gtest/gtest.h"
#include "harmonics_data_handler.h"
#include "model_calculator.h"
#include <boost/filesystem.hpp>
#include <constants.h>

class HarmonicsHandlerTest : public ::testing::Test
{
protected:
    static boost::filesystem::path test_file;
    static HarmonicsDataHandler handler;

    // Setup before the first test
    // the calculation
    static void SetUpTestSuite()
    {
        test_file = TEST_DATA_DIR + "quad_test.json";
        ModelCalculator calculator(test_file);
        calculator.calc_harmonics(handler, true);
    }
};

// Initialize static members
boost::filesystem::path HarmonicsHandlerTest::test_file;
HarmonicsDataHandler HarmonicsHandlerTest::handler;

TEST_F(HarmonicsHandlerTest, GetBnCorrectSizes)
{
    // there should be Bn values for B1 - B10.
    for (int i = 1; i <= 10; ++i)
    {
        std::vector<std::pair<double, double>> Bn = handler.get_Bn(i);
        EXPECT_FALSE(Bn.empty());
    }

    EXPECT_THROW(handler.get_Bn(0), std::logic_error);
    EXPECT_THROW(handler.get_Bn(11), std::logic_error);
}

TEST_F(HarmonicsHandlerTest, GetbnCorrectSizes)
{
    std::vector<double> bn = handler.get_bn();

    // should have bn values for B1 to B10
    EXPECT_FALSE(bn.empty());
    EXPECT_EQ(bn.size(), 10);
}

TEST_F(HarmonicsHandlerTest, GetanCorrectSizes)
{
    std::vector<double> an = handler.get_an();

    // should have bn values for B1 to B10
    EXPECT_FALSE(an.empty());
    EXPECT_EQ(an.size(), 10);
}

TEST_F(HarmonicsHandlerTest, ConstructorHandlesNullData)
{
    ASSERT_NO_THROW({
        HarmonicsDataHandler handler(nullptr);
        EXPECT_TRUE(handler.get_Bn(1).empty());
    });
}
