#ifndef GRID_SEARCH_OPTIMIZER_H
#define GRID_SEARCH_OPTIMIZER_H

#include "abstract_optimizer.h"
#include "abstract_objective.hh"
#include <grid_search.h>
#include "bn_objective.hh"
#include "fitted_slope_objective.hh"
#include "grid_search_result.h"

struct InterimResult {
    std::string file_path;
    std::vector<double> bn_values;
};

class GridSearchOptimizer : public AbstractOptimizer
{
public:
    GridSearchOptimizer(ModelHandler &model_handler, std::vector<std::shared_ptr<AbstractObjective>> criteria,
                        std::vector<double> thresholds, std::vector<double> search_factors,
                        const int grid_min_steps = GRID_DEFAULT_STEPS,
                        std::vector<int> harmonics_to_optimize = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    GridSearchOptimizer(std::vector<std::shared_ptr<AbstractObjective>> criteria,
                        std::vector<double> thresholds, std::vector<double> search_factors,
                        const int grid_num_steps = GRID_DEFAULT_STEPS,
                        std::vector<int> harmonics_to_optimize = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

    void optimize() override;
    void logResults() override;
    void exportModel() override;

    void injectParamRanges(std::vector<std::pair<std::pair<double, double>, std::pair<double, double>>> param_ranges);
    void setNumSteps(int num_steps);
    void computeCriteria();

    virtual ~GridSearchOptimizer() {};

protected:
    void optimize(double bn_threshold);
    void recompute_bn();
    bool hasDriveValueChanged(HarmonicDriveParameterMap &drive_values_before_loop);
    bool checkBnValue(int component, double prev_bn, HarmonicDriveParameterMap &prev_drive_values);
    void checkLengthSanity(HarmonicDriveParameterMap &fallback_drives);
    void runGridSearch(int component, std::vector<GridSearchResult> &results);
    std::pair<double, double> extrapolateOptimalConfiguration(std::vector<GridSearchResult> &results, HarmonicDriveParameters &current_drive);
    std::pair<double, double> extrapolateOptimalConfiguration(std::pair<double, double> linear_function1, std::pair<double, double> linear_function2);
    std::pair<double, double> extrapolateOptimalConfiguration(std::pair<double, double> linear_function, HarmonicDriveParameters &current_drive);
    void setParamRanges(double factor);
    void computeGranularities();
    std::pair<double, double> computeGranularities(std::pair<double, double> offset_range, std::pair<double, double> slope_range);
    void estimateTimePerComputation();
    std::vector<std::string> getCriteriaLabels();

    // Getter methods for some private variables
    const ModelHandler &getModelHandler()
    {
        return model_handler_;
    }

    const std::vector<double> &getCurrentBnValues()
    {
        return current_bn_values_;
    }

    const std::vector<std::pair<double, double>> &getGranularities()
    {
        return granularities_;
    }

    double getTimePerCalc()
    {
        return time_per_calc_;
    }
    std::pair<std::pair<double, double>, std::pair<double, double>> getParamRange(int component);

private:
    void setup();

    // Vector with interim results of exported models
    std::vector<InterimResult> interim_results_;

    // Vector with ranges for offset and slope for each harmonic drive. 0-indexed. Format: {{offset_min, offset_max}, {slope_min, slope_max}}
    std::vector<std::pair<std::pair<double, double>, std::pair<double, double>>> param_ranges_;

    // Vector with offset and slope granularities for each harmonic drive. 0-indexed. Format: {offset_granularity, slope_granularity}
    std::vector<std::pair<double, double>> granularities_;

    // Criteria to be used for the grid search
    std::vector<std::shared_ptr<AbstractObjective>> criteria_;

    // Harmonics to be optimized
    std::vector<int> harmonics_to_optimize_;

    std::vector<double> current_bn_values_;

    // Estimated time for one harmonics calculation
    double time_per_calc_;

    // Thresholds for the bn values
    std::vector<double> thresholds_;

    // Search factors for parameter ranges
    std::vector<double> search_factors_;

    // Num of steps in grid search
    int grid_num_steps_;

    // Flag to not compute parameter ranges because they were injected manually
    bool injected_param_ranges_ = false;
};

#endif // GRID_SEARCH_OPTIMIZER_H
