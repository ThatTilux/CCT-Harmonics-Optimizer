#ifndef GRID_SEARCH_H
#define GRID_SEARCH_H

#include "model_handler.h"
#include "model_calculator.h"
#include "abstract_objective.hh"

// Class to run a grid search for offset & slope parameters for one harmonic drive
class GridSearch
{
public:
    GridSearch(ModelHandler &model_handler, ModelCalculator calculator, int component, std::pair<double, double> &offset_range, std::pair<double, double> &slope_range, double &offset_granularity, double &slope_granularity, std::vector<GridSearchResult> &results, std::vector<std::shared_ptr<AbstractObjective>> &criteria, double estimated_time_per_calc);

private:
    ModelHandler model_handler_;
    ModelCalculator calculator_;
    int component_;
    std::pair<double, double> &offset_range_;
    std::pair<double, double> &slope_range_;
    double &offset_granularity_;
    double &slope_granularity_;
    std::vector<GridSearchResult> &results_;
    std::vector<std::shared_ptr<AbstractObjective>> &criteria_;

    // total number of steps in the grid search
    int total_steps_;
    // estimated time for one step in the grid search
    double estimated_time_per_step_;

    void run();
    void logParams();
    void logEstmatedTime();
};

#endif // GRID_SEARCH_H