#include "model_calculator.h"
#include "model_handler.h"
#include "input_output.h"
#include "constants.h"
#include "bn_optimizer.h"
#include "grid_search_optimizer.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

int main()
{

    // create necessary directories
    boost::filesystem::create_directory(MODEL_OUTPUT_DIR);
    boost::filesystem::create_directory(GRID_SEARCH_OUTPUT_DIR);


    // check which optimization the user wants to do
    std::vector<std::string> optimization_options = {"Grid Search Optimizer", "bn Optimizer", "Temp Grid Search", "Temp chisquare comp"};
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
        optimizer.exportModel();
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
    else if (selected_optimization == 2){
        // Thresholds and search factors
        std::vector<double> thresholds = {10000};
        std::vector<double> search_factors = {GRID_SEARCH_FACTOR};

        // Criteria
        std::vector<std::shared_ptr<AbstractObjective>> criteria;
        criteria.push_back(std::make_shared<BnObjective>());
        criteria.push_back(std::make_shared<FittedSlopeObjective>());

        // manual param ranges
        std::vector<std::pair<std::pair<double, double>, std::pair<double, double>>> param_ranges;
        param_ranges.resize(10);

        param_ranges[0] = {{-0.0025, 0.0025}, {-0.000025, 0.000025}}; // B1
        param_ranges[1] = {{1, 1.0001}, {1, 1.0001}}; // some dummy values here since B2 is main
        param_ranges[2] = {{-0.0025, 0.0025}, {-0.000025, 0.000025}}; //B3
        param_ranges[3] = {{-0.0025, 0.0025}, {-0.000025, 0.000025}}; //B4
        param_ranges[4] = {{-0.0005, 0.0005}, {-1e-06, 1e-06}}; //B5
        param_ranges[5] = {{-0.0005, 0.0005}, {-1e-06, 1e-06}}; //B6
        param_ranges[6] = {{-0.0005, 0.0005}, {-1e-06, 1e-06}}; //B7
        param_ranges[7] = {{-0.0005, 0.0005}, {-1e-06, 1e-06}}; //B8
        param_ranges[8] = {{-0.0005, 0.0005}, {-1e-06, 1e-06}}; //B9
        param_ranges[9] = {{-0.0005, 0.0005}, {-1e-06, 1e-06}}; //B10

        // run grid search optimizer
        GridSearchOptimizer optimizer = GridSearchOptimizer(criteria, thresholds, search_factors, 64, 60, {1,7});
        optimizer.injectParamRanges(param_ranges);
        optimizer.optimize();
        optimizer.logResults();
        optimizer.exportModel();
        return 0;
    }
    else if (selected_optimization == 3){
        // just get the chisquared values and slopes
        // Thresholds and search factors
        std::vector<double> thresholds = {10000};
        std::vector<double> search_factors = {GRID_SEARCH_FACTOR};

        // Criteria
        std::vector<std::shared_ptr<AbstractObjective>> criteria;
        criteria.push_back(std::make_shared<BnObjective>());
        criteria.push_back(std::make_shared<FittedSlopeObjective>());

        // manual param ranges
        std::vector<std::pair<std::pair<double, double>, std::pair<double, double>>> param_ranges;
        param_ranges.resize(10);

        param_ranges[0] = {{0, 0}, {0,0}}; // B1
        param_ranges[1] = {{1, 1.0001}, {1, 1.0001}}; // some dummy values here since B2 is main
        param_ranges[2] = {{0, 0}, {0,0}}; //B3
        param_ranges[3] = {{0, 0}, {0,0}}; //B4
        param_ranges[4] = {{0, 0}, {0,0}}; //B5
        param_ranges[5] = {{0, 0}, {0,0}}; //B6
        param_ranges[6] = {{0, 0}, {0,0}}; //B7
        param_ranges[7] = {{0, 0}, {0,0}}; //B8
        param_ranges[8] = {{0, 0}, {0,0}}; //B9
        param_ranges[9] = {{0, 0}, {0,0}}; //B10

        // run grid search optimizer
        GridSearchOptimizer optimizer = GridSearchOptimizer(criteria, thresholds, search_factors, 1, 60);
        optimizer.computeCriteria();
        return 0;
    }

    return 1;
}