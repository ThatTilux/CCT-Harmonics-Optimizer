#include "gtest/gtest.h"
#include "grid_search_optimizer.h"
#include "bn_objective.hh"
#include "fitted_slope_objective.hh"
#include <boost/filesystem.hpp>
#include <constants.h>

// Define class that is not abstract and exposes some protected/private methods for testing
class TestGridSearchOptimizer : public GridSearchOptimizer {
public:
    using GridSearchOptimizer::GridSearchOptimizer; // Inherit constructors

    using GridSearchOptimizer::hasDriveValueChanged;
    using GridSearchOptimizer::checkBnValue;
    using GridSearchOptimizer::checkLengthSanity;
    using GridSearchOptimizer::setParamRanges;
    using GridSearchOptimizer::computeGranularities;
    using GridSearchOptimizer::recompute_bn;
    using GridSearchOptimizer::getModelHandler;
    using GridSearchOptimizer::getCurrentBnValues;
    using GridSearchOptimizer::getParamRange;
    using GridSearchOptimizer::setNumSteps;
};

class GridSearchOptimizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Thresholds and search factors
        std::vector<double> thresholds = {30, 1, 0.1, 0.01};
        std::vector<double> search_factors = {GRID_SEARCH_FACTOR, GRID_SEARCH_FACTOR, GRID_SEARCH_FACTOR / 10, GRID_SEARCH_FACTOR / 100};

        // Criteria
        std::vector<std::shared_ptr<AbstractObjective>> criteria;
        criteria.push_back(std::make_shared<BnObjective>());
        criteria.push_back(std::make_shared<FittedSlopeObjective>());

        int grid_min_steps = 10;
        double time_budget_minutes = 30.0;
        std::vector<int> harmonics = {1};

        // Set up the model handlers for different test cases
        ModelHandler handler1(TEST_DATA_DIR + "quad_test_all_linear.json");
        opt_alllinear = new TestGridSearchOptimizer(handler1, criteria, thresholds, search_factors, grid_min_steps, harmonics);
    }

    void TearDown() override {
        delete opt_alllinear;
    }

    TestGridSearchOptimizer* opt_alllinear;
};

// Test cases for the methods specified
TEST_F(GridSearchOptimizerTest, hasDriveValueChanged) {
    ModelHandler model_handler = opt_alllinear->getModelHandler();

    // Initial parameters
    HarmonicDriveParameterMap drive_values_before = model_handler.getHarmonicDriveValues();
    
    // Call method and expect no change initially
    ASSERT_FALSE(opt_alllinear->hasDriveValueChanged(drive_values_before));

    // Change parameters
    HarmonicDriveParameterMap new_params;
    new_params["B1"] = HarmonicDriveParameters(drive_values_before["B1"].getOffset() + 0.001, drive_values_before["B1"].getSlope() + 0.0001);
    model_handler.apply_params(new_params);

    // Call method again and expect change
    ASSERT_TRUE(opt_alllinear->hasDriveValueChanged(drive_values_before));
}

TEST_F(GridSearchOptimizerTest, checkBnValue) {
    ModelHandler model_handler = opt_alllinear->getModelHandler();

    // Set initial params to get B1 bn to 5.25
    HarmonicDriveParameterMap initial_drive_values = model_handler.getHarmonicDriveValues();
    HarmonicDriveParameterMap prev_drive_values = initial_drive_values;
    prev_drive_values["B1"] = HarmonicDriveParameters(prev_drive_values["B1"].getOffset(), 0.0005);
    model_handler.apply_params(prev_drive_values);

    // Compute initial bns
    opt_alllinear->recompute_bn();
    double prev_bn = opt_alllinear->getCurrentBnValues()[0];

    // Change params for the worse
    prev_drive_values["B1"] = HarmonicDriveParameters(prev_drive_values["B1"].getOffset(), 0.0001);
    model_handler.apply_params(prev_drive_values);
    opt_alllinear->recompute_bn();

    // Call checkBnValue and expect the drives to be reset
    ASSERT_FALSE(opt_alllinear->checkBnValue(1, prev_bn, initial_drive_values));

    // Apply good params and recompute bn
    prev_drive_values["B1"] = HarmonicDriveParameters(prev_drive_values["B1"].getOffset(), 0);
    model_handler.apply_params(prev_drive_values);
    opt_alllinear->recompute_bn();

    // Call checkBnValue and expect the drives not to be reset
    ASSERT_TRUE(opt_alllinear->checkBnValue(1, prev_bn, initial_drive_values));
}

TEST_F(GridSearchOptimizerTest, checkLengthSanity) {
    ModelHandler model_handler = opt_alllinear->getModelHandler();

    // Initial check
    HarmonicDriveParameterMap fallback_drives = model_handler.getHarmonicDriveValues();
    ASSERT_NO_THROW(opt_alllinear->checkLengthSanity(fallback_drives));

    // Change B1 params to unreasonable values
    HarmonicDriveParameterMap new_params;
    new_params["B1"] = HarmonicDriveParameters(-0.0134697, 0.0155);
    model_handler.apply_params(new_params);

    // Check length sanity and expect reset
    ASSERT_NO_THROW(opt_alllinear->checkLengthSanity(fallback_drives));
}

TEST_F(GridSearchOptimizerTest, setParamRanges) {
    ModelHandler model_handler = opt_alllinear->getModelHandler();

    opt_alllinear->setParamRanges(0.05);
    auto param_range = opt_alllinear->getParamRange(1);

    HarmonicDriveParameterMap drive_values = model_handler.getHarmonicDriveValues();
    double offset = drive_values["B1"].getOffset();
    double slope = drive_values["B1"].getSlope();

    // make sure the param ranges span a square around the current drive
    ASSERT_NEAR(param_range.first.first, offset - 0.05 * std::abs(offset), 1e-6);
    ASSERT_NEAR(param_range.first.second, offset + 0.05 * std::abs(offset), 1e-6);
    ASSERT_NEAR(param_range.second.first, slope - 0.05 * std::abs(slope), 1e-6);
    ASSERT_NEAR(param_range.second.second, slope + 0.05 * std::abs(slope), 1e-6);
}

TEST_F(GridSearchOptimizerTest, computeGranularities) {
    std::pair<double, double> offset_range = {0,1};
    std::pair<double, double> slope_range = {0,1};

    double time_budget_minutes = 10;
    double time_per_step_second = 0.1;
    int minimum_steps = 1;

    // compute the number of steps given the time budget
    int steps_budget = time_budget_minutes * 60 / time_per_step_second;

    opt_alllinear->setNumSteps(steps_budget);

    std::pair<double, double> granularities = opt_alllinear->computeGranularities(offset_range, slope_range);



    // compute the number of steps given the granularities (using a loop to avoid floating point errors)
    int steps_actual = 0;
    for (double offset = offset_range.first; offset < offset_range.second; offset += granularities.first)
    {
        for (double slope = slope_range.first; slope < slope_range.second; slope += granularities.second)
        {
            steps_actual++;
        }
    }

    // make sure the number of steps is at least the minimum steps
    ASSERT_GE(steps_actual, minimum_steps);

    // make sure the number of steps is at around the budget (50%)
    ASSERT_NEAR(steps_actual, steps_budget, steps_budget * 0.5);   
}
