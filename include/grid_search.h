#ifndef GRID_SEARCH_H
#define GRID_SEARCH_H

#include "model_handler.h"
#include "model_calculator.h"
#include "abstract_objective.hh"
#include "fitted_slope_objective.hh"

// Class to run a grid search for offset & slope parameters for one harmonic drive
class GridSearch
{
public:
    GridSearch(CCTools::ModelHandler &model_handler,
               CCTools::ModelCalculator calculator,
               int component,
               std::pair<double, double> &offset_range,
               std::pair<double, double> &slope_range,
               double &offset_granularity,
               double &slope_granularity,
               std::vector<GridSearchResult> &results,
               std::vector<std::shared_ptr<AbstractObjective>> &criteria,
               double mag_ell_start,
               double mag_ell_end,
               double estimated_time_per_calc);

private:
    CCTools::ModelHandler model_handler_;
    CCTools::ModelCalculator calculator_;
    int component_;
    std::pair<double, double> &offset_range_;
    std::pair<double, double> &slope_range_;
    double &offset_granularity_;
    double &slope_granularity_;
    double mag_ell_start_;
    double mag_ell_end_;
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