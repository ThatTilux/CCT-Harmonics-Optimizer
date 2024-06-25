#include "gtest/gtest.h"
#include "optimizer.h"
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
    const boost::filesystem::path temp_json_file_path = model_handler.getTempJsonPath();
    double max_harmonic_value = 0.1;
    HarmonicsCalculator calculator(temp_json_file_path);
    HarmonicDriveParameterMap harmonic_drive_values = model_handler.getHarmonicDriveValues();

    // optimizer will store results here
    std::vector<double> current_bn_values;

    // call optimizer
    ASSERT_NO_THROW({
        optimize(calculator, model_handler, current_bn_values, harmonic_drive_values, max_harmonic_value, temp_json_file_path, true);
    });

    // make sure all variables are optimized
    for (auto &bn : current_bn_values) {
        if (bn != 10000){
            ASSERT_LE(std::abs(bn), max_harmonic_value);
        }
    }
}