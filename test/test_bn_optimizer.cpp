#include "gtest/gtest.h"
#include "bn_optimizer.h"
#include <boost/filesystem.hpp>
#include <constants.h>


class BnOptimizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_file = TEST_DATA_DIR + "quad_test_unoptimized.json";
    }

    boost::filesystem::path test_file;
};

// Simple BnOptimizer test. Only B1 needs to be optimized
TEST_F(BnOptimizerTest, BnOptimizer) {
    // initialize variables
    ModelHandler model_handler(test_file);
    double max_harmonic_value = 0.1;

    // create optimizer object and call optimization
    BnOptimizer optimizer(model_handler, max_harmonic_value);
    ASSERT_NO_THROW({
        optimizer.optimize();
    });


    // get results
    std::vector<double> bn_values = optimizer.getResults();

    

    // make sure all variables are optimized
    for (auto &bn : bn_values) {
        if (bn != 10000){
            ASSERT_LE(std::abs(bn), max_harmonic_value);
        }
    }
}