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

    // check which optimization the user wants to do
    std::vector<std::string> optimization_options = {"Grid Search Optimizer", "bn Optimizer"};
    int selected_optimization = selectFromList(optimization_options, "Please select the desired optimization:");

    if (selected_optimization == 0)
    {
        // run grid search optimizer
        GridSearchOptimizer optimizer = GridSearchOptimizer();
        optimizer.optimize();
        optimizer.logResults();
        optimizer.exportModel();
        return 0;
    }
    else if (selected_optimization == 1)
    {
        // only bn optimization
        BnOptimizer optimizer = BnOptimizer();
        optimizer.temp_test(); //TODO TEMP REMOVe
        optimizer.optimize();
        optimizer.logResults();
        optimizer.exportModel();
        return 0;
    }

    return 1;
}