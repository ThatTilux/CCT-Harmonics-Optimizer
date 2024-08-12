#include "gtest/gtest.h"
#include "an_optimizer.h"
#include <boost/filesystem.hpp>
#include <constants.h>

class AnOptimizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_file = TEST_DATA_DIR + "quad_test_unoptimized.json";
    }

    boost::filesystem::path test_file;
};

// Simple AnOptimizer test. Optimizes `an` values.
TEST_F(AnOptimizerTest, AnOptimizer) {
    // Initialize variables
    ModelHandler model_handler(test_file);
    double max_harmonic_value = 1.0;

    // Create optimizer object and call optimization
    AnOptimizer optimizer(model_handler, max_harmonic_value);
    ASSERT_NO_THROW({
        optimizer.optimize();
    });

    // Get results
    std::vector<double> an_values = optimizer.getResults();

    // Ensure all variables are optimized within the max harmonic value
    for (auto &an : an_values) {
        if (an != 10000) { 
            ASSERT_LE(std::abs(an), max_harmonic_value);
        }
    }
}
