#ifndef GRID_SEARCH_OPTIMIZER_H
#define GRID_SEARCH_OPTIMIZER_H

#include "abstract_optimizer.h"
#include "abstract_objective.hh"
#include <grid_search.h>
#include "bn_objective.hh"
#include "fitted_slope_objective.hh"
#include "grid_search_result.h"

class GridSearchOptimizer : public AbstractOptimizer
{
public:
    GridSearchOptimizer(ModelHandler &model_handler);
    GridSearchOptimizer();

    void optimize() override;
    void logResults() override;

    virtual ~GridSearchOptimizer(){};

protected:
private:
    void optimize(double bn_threshold);
    void runGridSearch(int component, std::vector<GridSearchResult> &results);
    std::pair<double, double> extrapolateOptimalConfiguration(std::vector<GridSearchResult> &results);
    void initParamRanges();
    void updateParamRanges(double factor);
    void initCriteria();
    void computeGranularities();
    std::pair<double, double> computeGranularities(std::pair<double, double> offset_range,
                                                   std::pair<double, double> slope_range,
                                                   double time_budget_minutes,
                                                   double time_per_step_seconds);
    void estimateTimePerComputation();
    int getNumberOfSteps(std::pair<double, double> offset_range, std::pair<double, double> slope_range, double offset_granularity, double slope_granularity);

    std::pair<std::pair<double, double>, std::pair<double, double>> getParamRange(int component);

    // Vector with ranges for offset and slope for each harmonic drive. 0-indexed. Format: {{offset_min, offset_max}, {slope_min, slope_max}}
    std::vector<std::pair<std::pair<double, double>, std::pair<double, double>>> param_ranges_;

    // Vector with offset and slope granularities for each harmonic drive. 0-indexed. Format: {offset_granularity, slope_granularity}
    std::vector<std::pair<double, double>> granularities_;

    // Criteria to be used for the grid search
    std::vector<std::shared_ptr<AbstractObjective>> criteria_;

    // TODO implement this
    std::vector<double> current_bn_values_;

    // Estimated time for one harmonics calculation
    double time_per_calc_;
};

#endif // GRID_SEARCH_OPTIMIZER_H
