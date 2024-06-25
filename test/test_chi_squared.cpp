#include "gtest/gtest.h"
#include "harmonics_calculator.h"
#include "harmonics_handler.h"
#include "optimizer.h"
#include <boost/filesystem.hpp>
#include <constants.h>
#include <cmath>

class ChiSquaredTest : public ::testing::Test {
protected:
    static boost::filesystem::path test_file;
    static HarmonicsCalculator* calculator;
    static HarmonicsHandler harmonics_handler;


    static void SetUpTestSuite() {
        test_file = TEST_DATA_DIR + "quad_test.json";
        HarmonicsCalculator calculator(test_file);
        calculator.calc(harmonics_handler, true);
    }
    
};

// Initialize static members
boost::filesystem::path ChiSquaredTest::test_file;
HarmonicsCalculator* ChiSquaredTest::calculator;
HarmonicsHandler ChiSquaredTest::harmonics_handler;

TEST_F(ChiSquaredTest, ChiSquaredComputations) {
    std::vector<double> expected_chi_squareds = {
        293.2880208792319,
        299.9999868400242,
        299.59644574285164,
        299.9999714649674,
        299.9045861957219,
        299.9983356467078,
        299.9994491960269,
        299.999690020906,
        299.9975730681565,
        299.99932534682966
    };

    for (int i = 1; i <= 10; ++i) {
        double chi_squared = chiSquared(harmonics_handler, i);
        // tolerance set very high here since the results differ a bit depending on whether GPU was used for the calculation
        EXPECT_NEAR(chi_squared, expected_chi_squareds[i - 1], 1e-2)
            << "Chi squared for component B" << i << " did not match the expected value.";
    }
}
