#ifndef GRID_SEARCH_OPTIMIZER_H
#define GRID_SEARCH_OPTIMIZER_H

#include "abstract_optimizer.h"
#include "abstract_objective.hh"
#include <grid_search.h>
#include "bn_objective.hh"
#include "fitted_slope_objective.hh"
#include "grid_search_result.h"

/**
 * @struct InterimResult
 * @brief Struct to store the interim results of the exported models.
 * 
 * The struct stores the file path of exported models and their b_n values.
 */
struct InterimResult {
    std::string file_path;
    std::vector<double> bn_values;
};

/**
 * @class GridSearchOptimizer
 * @brief Optimizes a model by performing several grid searches and extrapolating the optimal configuration for all custom CCT harmonics.
 * 
 * This optimizer optimizes the custom CCT harmonics of the magnet to achieve specified objectives.
 * The optimizer performs grid searches for one harmonic drive at a time and extrapolates the optimal configuration for the scaling function of that drive based on the results of the grid search. 
 * This is done iteratively in rounds until all objectives meet the specified thresholds.
 */
class GridSearchOptimizer : public AbstractOptimizer
{
public:
    /**
     * @brief Construct a new Grid Search Optimizer object with no user interaction.
     * @param model_handler ModelHandler object to be used for the optimization.
     * @param criteria Vector of shared pointers to AbstractObjective objects that define the criteria to be used for the grid search.
     * @param thresholds Vector of bn-thresholds for each optimization.
     * @param search_factors Vector of search factors to determine the grid size for each optimization.
     * @param grid_min_steps Minimum number of steps in each grid search.
     * @param harmonics_to_optimize Integer vector of harmonics to be optimized (1-indexed).
     * 
     * The constructor initializes the optimizer with the provided parameters and sets up the model for optimization.
     * The optimizer will run multiple optimizations (determined by the size of the `thresholds` and `search_factors` vectors) with the specified (usually decreasing) parameters.
     * In each optimization, the optimizer will perform several rounds until the b_n values of all harmonics are below the specified threshold.
     * In each round, the optimizer will perform one grid search for each harmonic drive, except the main harmonic, (unless a drive is already below the threshold).
     * This grid search is in the space of the offset and slope parameters of the scaling function of the harmonic drive. The results are used to extrapolate the optimal values for the offset and slope of the scaling function.
     */
    GridSearchOptimizer(CCTools::ModelHandler &model_handler, std::vector<std::shared_ptr<AbstractObjective>> criteria,
                        std::vector<double> thresholds, std::vector<double> search_factors,
                        const int grid_min_steps = GRID_DEFAULT_STEPS,
                        std::vector<int> harmonics_to_optimize = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

    /**
     * @brief Construct a new Grid Search Optimizer object with user interaction.
     * @param criteria Vector of shared pointers to AbstractObjective objects that define the criteria to be used for the grid search.
     * @param thresholds Vector of bn-thresholds for each optimization.
     * @param search_factors Vector of search factors to determine the grid size for each optimization.
     * @param grid_min_steps Minimum number of steps in each grid search.
     * @param harmonics_to_optimize Integer vector of harmonics to be optimized (1-indexed).
     * 
     * The constructor lets the user select a model from the command line, initializes the optimizer with the provided parameters, and sets up the model for optimization.
     * The optimizer will run multiple optimizations (determined by the size of the `thresholds` and `search_factors` vectors) with the specified (usually decreasing) parameters.
     * In each optimization, the optimizer will perform several rounds until the b_n values of all harmonics are below the specified threshold.
     * In each round, the optimizer will perform one grid search for each harmonic drive, except the main harmonic, (unless a drive is already below the threshold).
     * This grid search is in the space of the offset and slope parameters of the scaling function of the harmonic drive. The results are used to extrapolate the optimal values for the offset and slope of the scaling function.
     */
    GridSearchOptimizer(std::vector<std::shared_ptr<AbstractObjective>> criteria,
                        std::vector<double> thresholds, std::vector<double> search_factors,
                        const int grid_num_steps = GRID_DEFAULT_STEPS,
                        std::vector<int> harmonics_to_optimize = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

    /**
     * @brief Start the optimizer.
     * 
     * The function estimates the time per computation, logs the criteria to be used for the grid search, and runs the optimizer with the specified thresholds and search factors.
     */
    void optimize() override;

    /**
     * @brief Log the results of the optimizer.
     * 
     * The function logs the objective function values of the final result as well as all interim results with the corresponding models.
     */
    void logResults() override;

    /**
     * @brief Export the model of the optimizer.
     * 
     * The function exports the current model to a JSON file and logs the current b_n values.
     */
    void exportModel() override;

    /**
     * @brief Inject parameter ranges for the grid searches.
     * @param param_ranges Vector to overwrite the `param_ranges_` variable. Needs to have a length of 10 to cover all harmonics. Range for the main harmonic is ignored. 
     * 
     * This function injects a value for the `param_ranges_` variable. Must be called prior to the optimization. When injected, the optimizer will not compute the parameter ranges itself.
     * This function is designed for testing purposes. Caution is advised when using this function for non-testing purposes. 
     */
    void injectParamRanges(std::vector<std::pair<std::pair<double, double>, std::pair<double, double>>> param_ranges);

    /**
     * @brief Function to overwrite the `grid_num_steps_` variable.
     * @param num_steps Number of steps to be set.
     * 
     * This function overwrites the `grid_num_steps_` variable. Must be called prior to the optimization.
     * This function is designed for testing purposes. Caution is advised when using this function for non-testing purposes.
     */
    void setNumSteps(int num_steps);

    /**
     * @brief Function to compute and log all `crieria_` values.
     *
     * The function computes the criteria values for the current model and logs them.
     * This function is designed for testing purposes and will have no effect besides logging.
     */
    void logCriteriaValues();

    virtual ~GridSearchOptimizer() {};

protected:
    /**
     * @brief Function to start one optimization.
     * @param bn_threshold Threshold for the b_n values.
     * 
     * In this optimization, the optimizer will perform several rounds until the b_n values of all harmonics are below `bn_threshold`.
     * In each round, the optimizer will perform one grid search for each harmonic drive, except the main harmonic, (unless a drive is already below the threshold) to find the optimal configuration for the scaling function of that drive.
     */
    void optimize(double bn_threshold);

    /**
     * @brief Function to recompute the b_n values.
     * 
     * The function recomputes the b_n values of the current model and stores them in the `current_bn_values_` variable.
     */
    void recompute_bn();

    /**
     * @brief Function to check if any custom cct harmonic drive value has changed in the current loop iteration.
     * @param drive_values_before_loop HarmonicDriveParameterMap object with the harmonic drive values before the current loop iteration.
     * @returns True if at least one value has changed, false otherwise.
     * 
     * The function checks if any custom cct harmonic drive value has changed since the last optimization round by comparing the current drives to `drive_values_before_loop`.
     * If at least one value has changed, the function returns true. Otherwise, it returns false.
     */
    bool hasDriveValueChanged(CCTools::HarmonicDriveParameterMap &drive_values_before_loop);

    /**
     * @brief Function to check if the new configuration improved the b_n value. May revert to the previous configuration.
     * @param component Harmonic drive component to be checked (1-indexed).
     * @param prev_bn Previous b_n value of the component.
     * @param prev_drive_values HarmonicDriveParameterMap object with the harmonic drive values correspoding to `prev_bn`.
     * @returns True if the new b_n value is better than the previous one, false otherwise.
     * 
     * The function checks if the new configuration improved the b_n value of the specified component compared to the previous configuration.
     * If the new b_n value is better, the function returns true. 
     * If the new b_n value is the same, the function returns false. 
     * If the new b_n value is worse, the function reverts to the previous configuration `prev_drive_values` and returns false.
     */
    bool checkBnValue(int component, double prev_bn, CCTools::HarmonicDriveParameterMap &prev_drive_values);

    /**
     * @brief Function to check the sanity of the magnet model. Will revert to the fallback config if the length of the magnet changed considerably.
     * @param fallback_drives HarmonicDriveParameterMap object with the fallback harmonic drive values.
     * 
     * The function checks if the length of the magnet model has changed considerably since the last call of this function.
     * If the length has changed by more than 30% (e.g., because of edge regressions in the model), the function reverts to the previous configuration `fallback_drives`.
     */
    void checkLengthSanity(CCTools::HarmonicDriveParameterMap &fallback_drives);

    /**
     * @brief Function to run a grid search for one harmonic drive.
     * @param component Harmonic drive component to be optimized (1-indexed).
     * @param results Vector to store the results of the grid search. One element corresponds to one iteration of the grid search.
     * 
     * The function runs a grid search for the specified harmonic drive component and stores the results in the `results` vector.
     * Pulls the parameter ranges and granularities from the `param_ranges_` and `granularities_` variables.
     */
    void runGridSearch(int component, std::vector<GridSearchResult> &results);

    /**
     * @brief Function to extrapolate the optimal configuration from the grid search results.
     * @param results Vector of GridSearchResult objects with the results of the grid search.
     * @param current_drive HarmonicDriveParameters object with the current harmonic drive values.
     * @returns Pair of doubles with the optimal offset and slope configuration.
     * 
     * The function extrapolates the optimal configuration for the scaling function of one harmonic drive based on the results of the grid search.
     * The optimal configuration is the one that minimizes all objectives (criteria).
     * For every criterion, the function fits a 2D plane in the space of [offset, slope, criteria_value] to the data and extracts the linear function where the plane has the criteria_value of 0.
     * For the case of multiple criteria, the function calculates the intersection of all linear functions to extract the optimal offset and slope as the x and y coordinates of the intersection point.
     * For the case of a single criterion, the function calculates the point on the linear function that is closest to the current configuration.
     */
    std::pair<double, double> extrapolateOptimalConfiguration(std::vector<GridSearchResult> &results, CCTools::HarmonicDriveParameters &current_drive);

    /**
     * @brief Helper function to extrapolate the optimal configuration from the grid search results for two criteria.
     * @param linear_function1 Pair of doubles with the linear function for the first criterion.
     * @param linear_function2 Pair of doubles with the linear function for the second criterion.
     * @returns Pair of doubles with the optimal offset and slope configuration.
     * 
     * The function calculates the intersection of the linear functions of two criteria and returns the optimal offset and slope configuration as the x and y coordinates of the intersection point.
     */
    std::pair<double, double> extrapolateOptimalConfiguration(std::pair<double, double> linear_function1, std::pair<double, double> linear_function2);

    /**
     * @brief Helper function to extrapolate the optimal configuration from the grid search results for one criterion.
     * @param linear_function Pair of doubles with the linear function for the criterion.
     * @param current_drive HarmonicDriveParameters object with the current harmonic drive values.
     * @returns Pair of doubles with the optimal offset and slope configuration.
     * 
     * The function calculates the point on the linear function that is closest to the current configuration and returns the optimal offset and slope configuration.
     */
    std::pair<double, double> extrapolateOptimalConfiguration(std::pair<double, double> linear_function, CCTools::HarmonicDriveParameters &current_drive);
    
    /**
     * @brief Function to set the parameter ranges for the grid search.
     * @param factor Factor to determine the size of the grid.
     * 
     * The function sets the parameter ranges for the grid search based on the current harmonic drive values and the provided factor.
     * The parameter range will be set to the [current value +/- factor * current value] for the offset and slope parameters.
     * The parameter ranges are stored in the `param_ranges_` variable.
     */
    void setParamRanges(double factor);

    /**
     * @brief Function to compute the granularities for the grid search.
     * 
     * The function computes the granularities of all harmonics for the grid search (i.e., the step size of the grid) based on the parameter ranges and `grid_num_steps_`.
     * The grid is spanned equidistantly in both dimensions (offset, slope). The granularities are stored in the `granularities_` variable.
     */
    void computeGranularities();

    /**
     * @brief Helper Function to compute the granularities for the grid search.
     * @param offset_range Pair of doubles with the offset range.
     * @param slope_range Pair of doubles with the slope range.
     * @returns Pair of doubles with the offset and slope granularities.
     * 
     * The function computes the granularities for the grid search based on the provided offset and slope ranges and the `grid_num_steps_` variable.
     * The granularities are set so that the spanned grid in the parameter space is equidistant in both dimensions.
     */
    std::pair<double, double> computeGranularities(std::pair<double, double> offset_range, std::pair<double, double> slope_range);

    /**
     * @brief Function to estimate the time per harmonics calculation (i.e., one grid search iteration).
     * 
     * The function estimates the time it takes to run one harmonics calculation by performing a series of dummy computations on the current model.
     * The estimated time in seconds is stored in the `time_per_calc_` variable and logged.
     */
    void estimateTimePerComputation();

    /**
     * @brief Function to get the labels of all criteria.
     * @returns Vector of strings with the labels of all criteria.
     */
    std::vector<std::string> getCriteriaLabels();

    /**
     * @brief Function to get the model handler.
     * @returns Reference to the ModelHandler object.
     */
    const CCTools::ModelHandler &getModelHandler()
    {
        return model_handler_;
    }

    /**
     * @brief Function to get the current bn values.
     * @returns Vector of doubles with the current b_n values.
     * 
     * This function returns the bn values computed in the last harmonics calculation.
     */
    const std::vector<double> &getCurrentBnValues()
    {
        return current_bn_values_;
    }

    /**
     * @brief Function to get the granularities for the grid search.
     * @returns Vector of pairs of doubles with the granularities for the offset and slope parameters of all harmonics.
     * 
     * This function returns the granularities for the grid search of all 10 harmonics.
     */
    const std::vector<std::pair<double, double>> &getGranularities()
    {
        return granularities_;
    }

    /**
     * @brief Function to get the time per harmonics calculation.
     * @returns Double with the estimated time per harmonics calculation in seconds.
     * 
     * This function returns the estimated time it takes to run one harmonics calculation (i.e., one grid search iteration).
     */
    double getTimePerCalc()
    {
        return time_per_calc_;
    }

    /**
     * @brief Function to get the parameter ranges of a harmonic component for the grid search.
     * @param component Harmonic drive component to get the parameter ranges for (1-indexed).
     * @returns Pair of pairs of doubles with the offset and slope parameter ranges (start and end).
     * 
     * This function returns the parameter ranges for the offset and slope parameters of the specified harmonic drive component.
     * Format: {{offset_min, offset_max}, {slope_min, slope_max}}
     */
    std::pair<std::pair<double, double>, std::pair<double, double>> getParamRange(int component);

private:
    /**
     * @brief Function to set up the optimizer.
     * 
     * The function performs some model checks, initializes the calculator, and computes the magnet bounds.
     */
    void setup();

    /**
     * @brief Vector with interim results of exported models.
     */
    std::vector<InterimResult> interim_results_;

    /**
     * @brief Vector with ranges for offset and slope for each harmonic drive. 0-indexed.
     * @details Format: {{offset_min, offset_max}, {slope_min, slope_max}}
     */
    std::vector<std::pair<std::pair<double, double>, std::pair<double, double>>> param_ranges_;

    /**
     * @brief Vector with offset and slope granularities for each harmonic drive. 0-indexed.
     * @details Format: {offset_granularity, slope_granularity}
     */
    std::vector<std::pair<double, double>> granularities_;

    /**
     * @brief Criteria to be used for the grid search.
     */
    std::vector<std::shared_ptr<AbstractObjective>> criteria_;

    /**
     * @brief Harmonics to be optimized.
     */
    std::vector<int> harmonics_to_optimize_;

    /**
     * @brief bn values computed in the last harmonics calculation.
     */
    std::vector<double> current_bn_values_;

    /**
     * @brief Estimated time for one harmonics calculation (seconds).
     */
    double time_per_calc_;

    /**
     * @brief Thresholds for the bn values per optimization.
     */
    std::vector<double> thresholds_;

    /**
     * @brief Search factors for parameter ranges per optimization.
     */
    std::vector<double> search_factors_;

    /**
     * @brief Number of steps for every grid search.
     */
    int grid_num_steps_;

    /**
     * @brief Flag to not compute parameter ranges because they were injected manually.
     */
    bool injected_param_ranges_ = false;

};

#endif // GRID_SEARCH_OPTIMIZER_H
