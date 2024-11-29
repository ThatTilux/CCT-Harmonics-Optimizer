#include <boost/filesystem.hpp>
#include <vector>
#include <string>
#include <memory>
#include "constants.h"
#include <input_output.h>
#include <bn_objective.hh>
#include <bn_optimizer.h>
#include <an_optimizer.h>
#include <abstract_objective.hh>
#include <fitted_slope_objective.hh>
#include <grid_search_optimizer.h>

/**
 * @brief Main function to run the application.
 * 
 * This function lets the user select the desired optimization in the terminal and runs the corresponding optimizer with some well-tested configurations.
 */
int main()
{

    // create necessary directories
    boost::filesystem::create_directory(MODEL_OUTPUT_DIR);
    boost::filesystem::create_directory(GRID_SEARCH_OUTPUT_DIR);

    // check which optimization the user wants to do
    std::vector<std::string> optimization_options = {"Grid Search Optimizer", "bn Optimizer", "an Optimizer"};
    int selected_optimization = selectFromList(optimization_options, "Please select the desired optimization:");

    if (selected_optimization == 0)
    {
        // Thresholds and search factors
        std::vector<double> thresholds = {30, 1, 0.1, 0.01};
        std::vector<double> search_factors = {GRID_SEARCH_FACTOR, GRID_SEARCH_FACTOR, GRID_SEARCH_FACTOR / 10, GRID_SEARCH_FACTOR / 100};

        // Criteria
        std::vector<std::shared_ptr<AbstractObjective>> criteria;
        criteria.push_back(std::make_shared<BnObjective>());
        criteria.push_back(std::make_shared<FittedSlopeObjective>());

        // run grid search optimizer
        GridSearchOptimizer optimizer = GridSearchOptimizer(criteria, thresholds, search_factors);
        optimizer.optimize();
        optimizer.logResults();
        return 0;
    }
    else if (selected_optimization == 1)
    {
        // only bn optimization
        BnOptimizer optimizer = BnOptimizer();
        optimizer.optimize();
        optimizer.logResults();
        optimizer.exportModel();
        return 0;
    }
    else if (selected_optimization == 2)
    {
        // only an optimization
        AnOptimizer optimizer = AnOptimizer();
        optimizer.optimize();
        optimizer.logResults();
        optimizer.exportModel();
        return 0;
    }

    return 1;
}
