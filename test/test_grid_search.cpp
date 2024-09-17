#include <constants.h>
#include "gtest/gtest.h"
#include "grid_search.h"
#include "model_handler.h"
#include "model_calculator.h"
#include "bn_objective.hh"
#include "fitted_slope_objective.hh"
#include <vector>
#include <memory>

class GridSearchTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize the model handler
        model_handler_ = new CCTools::ModelHandler(TEST_DATA_DIR + "quad_test_all_linear.json");

        // Initialize the criteria
        criteria_.push_back(std::make_shared<BnObjective>());
        criteria_.push_back(std::make_shared<FittedSlopeObjective>());

        // Initialize the parameter ranges
        offset_range_ = std::make_pair(0, 1e-05);
        slope_range_ = std::make_pair(0, 1e-06);

        // Set granularities to ensure 9 steps
        offset_granularity_ = 0.34e-05;
        slope_granularity_ = 0.34e-06;

        // Initialize other parameters
        mag_ell_start_ = -std::numeric_limits<double>::infinity();
        mag_ell_end_ = std::numeric_limits<double>::infinity();
        estimated_time_per_calc_ = 0.5;

        // Initialize the results vector
        results_ = std::vector<GridSearchResult>();
    }

    void TearDown() override
    {
        delete model_handler_;
    }

    CCTools::ModelHandler *model_handler_;
    CCTools::ModelCalculator calculator_;
    std::vector<std::shared_ptr<AbstractObjective>> criteria_;
    std::pair<double, double> offset_range_;
    std::pair<double, double> slope_range_;
    double offset_granularity_;
    double slope_granularity_;
    double mag_ell_start_;
    double mag_ell_end_;
    double estimated_time_per_calc_;
    std::vector<GridSearchResult> results_;
};

TEST_F(GridSearchTest, RunGridSearch)
{
    // Create and run the grid search object
    GridSearch grid_search(*model_handler_, calculator_, 1, offset_range_, slope_range_, offset_granularity_, slope_granularity_, results_, criteria_, mag_ell_start_, mag_ell_end_, estimated_time_per_calc_);

    // Check that the results vector has 9 elements
    ASSERT_EQ(results_.size(), 9);

    // Check that each result has 2 criteria values
    for (const auto &result : results_)
    {
        ASSERT_EQ(result.criteria_values.size(), 2);
    }
}
