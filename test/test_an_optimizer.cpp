#include "gtest/gtest.h"
#include "an_optimizer.h"
#include <boost/filesystem.hpp>
#include <constants.h>

// Define class that exposes the private method fitLinearGetRoot for testing
class TestAnOptimizer : public AnOptimizer
{
public:
    using AnOptimizer::AnOptimizer; // Inherit constructors

    using AnOptimizer::assertAllHarmonicsPresent;
    using AnOptimizer::getMainComponent;
};

class AnOptimizerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_file = TEST_DATA_DIR + "quad_test_unoptimized_allA.json";
        test_file_2 = TEST_DATA_DIR + "quad_test_unoptimized_allA_linear.json";
    }

    boost::filesystem::path test_file;
    boost::filesystem::path test_file_2;
};

// Simple AnOptimizer test. Optimizes `an` values.
TEST_F(AnOptimizerTest, AnOptimizer)
{
    // Initialize variables
    ModelHandler model_handler(test_file);
    double max_harmonic_value = 1.0;

    // Create optimizer object and call optimization
    TestAnOptimizer optimizer(model_handler, max_harmonic_value);
    ASSERT_NO_THROW({
        optimizer.optimize();
    });

    // Get results
    std::vector<double> an_values = optimizer.getResults();

    // to get main component set
    optimizer.assertAllHarmonicsPresent();

    int main_component = optimizer.getMainComponent();

    // Ensure all variables are optimized within the max harmonic value
    for (int i = 0; i < an_values.size(); i++)
    {
        if ((i + 1) != main_component)
        {
            ASSERT_LE(std::abs(an_values[i]), max_harmonic_value);
        }
    }
}

// Tests checkForHarmonicDriveConstraints function
TEST_F(AnOptimizerTest, checkForHarmonicDriveConstraints)
{
    // Initialize variables
    ModelHandler model_handler(test_file);
    ModelHandler model_handler_2(test_file_2);
    double max_harmonic_value = 1.0;

    // All linear harmonics
    ASSERT_NO_THROW({
        TestAnOptimizer optimizer(model_handler, max_harmonic_value);
        optimizer.optimize();
    });

    // One linear harmonic`
    ASSERT_THROW({
            TestAnOptimizer optimizer(model_handler_2, max_harmonic_value);
            optimizer.optimize(); }, std::runtime_error);
}
