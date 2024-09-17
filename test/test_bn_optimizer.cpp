#include "gtest/gtest.h"
#include "bn_optimizer.h"
#include <boost/filesystem.hpp>
#include <constants.h>

// Define class that exposes the private method fitLinearGetRoot for testing
class TestBnOptimizer : public BnOptimizer
{
public:
    using BnOptimizer::BnOptimizer; // Inherit constructors

    using BnOptimizer::fitLinearGetRoot;
};

class BnOptimizerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_file = TEST_DATA_DIR + "quad_test_unoptimized.json";
    }

    boost::filesystem::path test_file;
};

// Simple BnOptimizer test. Only B1 needs to be optimized
TEST_F(BnOptimizerTest, BnOptimizer)
{
    // initialize variables
    CCTools::ModelHandler model_handler(test_file);
    double max_harmonic_value = 0.1;

    // create optimizer object and call optimization
    BnOptimizer optimizer(model_handler, max_harmonic_value);
    ASSERT_NO_THROW({
        optimizer.optimize();
    });

    // get results
    std::vector<double> bn_values = optimizer.getResults();

    // make sure all variables are optimized
    for (auto &bn : bn_values)
    {
        if (bn != 10000)
        {
            ASSERT_LE(std::abs(bn), max_harmonic_value);
        }
    }
}

// Test for the fitLinearGetRoot method
TEST_F(BnOptimizerTest, FitLinearGetRoot)
{
    // Initialize variables
    CCTools::ModelHandler model_handler(test_file);
    double max_harmonic_value = 0.1;
    TestBnOptimizer optimizer(model_handler, max_harmonic_value);

    // Define test data for linear regression (points on the line y = 2x + 1)
    std::vector<std::pair<double, double>> points = {{1.0, 3.0}, {2.0, 5.0}, {3.0, 7.0}};

    // Call the wrapper method to test fitLinearGetRoot
    double root = optimizer.fitLinearGetRoot(points);

    // The expected root for y = 2x + 1 is x = -0.5
    double expected_root = -0.5;

    // Check the result
    ASSERT_NEAR(root, expected_root, 1e-6);
}