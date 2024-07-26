#include "gtest/gtest.h"
#include "model_calculator.h"
#include "harmonics_data_handler.h"
#include <boost/filesystem.hpp>
#include <constants.h>

class HarmonicsCalculatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_file = TEST_DATA_DIR + "quad_test.json";
    }

    boost::filesystem::path test_file;
};

TEST_F(HarmonicsCalculatorTest, ConstructorLoadsModel)
{
    ASSERT_NO_THROW({
        ModelCalculator calculator(test_file);
        // Assuming that if the model is loaded, harmonics_calc_ would not be null
        EXPECT_TRUE(calculator.has_harmonics_calc());
    });
}

TEST_F(HarmonicsCalculatorTest, LoadModelFailsWithInvalidFile)
{
    boost::filesystem::path invalid_file = TEST_DATA_DIR + "invalid_test.json";
    ModelCalculator calculator(invalid_file);
    // The constructor should fail and print an error message, we check if the calc_ is still null
    EXPECT_FALSE(calculator.has_harmonics_calc());
}

TEST_F(HarmonicsCalculatorTest, CalcUpdatesHarmonicsHandler)
{
    ModelCalculator calculator(test_file);
    HarmonicsDataHandler handler;
    calculator.calc_harmonics(handler, true);
    // Check if the handler is updated
    EXPECT_FALSE(handler.get_bn().empty());
}

TEST_F(HarmonicsCalculatorTest, ReloadAndCalc)
{
    ModelCalculator calculator(test_file);
    HarmonicsDataHandler handler;
    calculator.reload_and_calc_harmonics(test_file, handler, true);
    // Check if the handler is updated after reloading and calculating
    EXPECT_FALSE(handler.get_bn().empty());
}

TEST_F(HarmonicsCalculatorTest, LoadModelFromJsonHandlesNonExistentFile)
{
    ModelCalculator calculator(TEST_DATA_DIR + "non_existent.json");
    // Ensure that loading a non-existent file does not work
    EXPECT_FALSE(calculator.has_harmonics_calc());
}
