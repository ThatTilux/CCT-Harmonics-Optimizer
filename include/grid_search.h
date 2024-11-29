#ifndef GRID_SEARCH_H
#define GRID_SEARCH_H

#include "model_handler.h"
#include "model_calculator.h"
#include "abstract_objective.hh"
#include "fitted_slope_objective.hh"

/**
 * @brief Class to perform a grid search optimization for the `offset` and `slope` parameters of one harmonic drive scaling function.
 *
 * The GridSearch class performs a grid search optimization for the `offset` and `slope` parameters of one harmonic drive scaling function.
 * The grid search iterates over the specified ranges with the specified granularities.
 * For every iteration, the search computes a harmonics calculation for these parameters and evaluates the specified criteria.
 */
class GridSearch
{
public:


    /**
     * @brief Constructor for the GridSearch class.
     * @param model_handler The ModelHandler object to handle the model files.
     * @param calculator The ModelCalculator object to calculate the harmonics.
     * @param component The harmonic component to optimize (1-indexed).
     * @param offset_range The range of the `offset` parameter for the grid search.
     * @param slope_range The range of the `slope` parameter for the grid search.
     * @param offset_granularity The granularity of the `offset` parameter for the grid search.
     * @param slope_granularity The granularity of the `slope` parameter for the grid search.
     * @param results The vector to store the results of the grid search.
     * @param criteria The vector of AbstractObjective objects to define the output criteria.
     * @param mag_ell_start The start value of the magnitude ellipsoid relative to the harmonic calculation's axis. Used for the FittedSlopeObjective.
     * @param mag_ell_end The end value of the magnitude ellipsoid relative to the harmonic calculation's axis. Used for the FittedSlopeObjective.
     * @param estimated_time_per_calc The estimated time for one harmonics calculation.
     *
     * This constructor initializes the GridSearch object with the specified parameters and runs the grid search.
     * 
     * @throws std::invalid_argument If any range is invalid (e.g., `min > max`) or granularity is non-positive.
     */
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

    /**
     * @brief Function to run the grid search optimization.
     *
     * This function runs the grid search optimization for the specified parameters.
     * It iterates over the `offset` and `slope` ranges with the specified granularities.
     * For every iteration, it computes a harmonics calculation for these parameters and evaluates the specified criteria.
     */
    void run();

    /**
     * @brief Function to log the grid search parameters.
     *
     * This function logs the grid search parameters, including the `offset` and `slope` ranges and granularities.
     */
    void logParams();

    /**
     * @brief Function to estimate and log the time it will take to run the grid search.
     *
     * This function estimates and logs the time it will take to run the grid search based on the total number of steps and the estimated time per step.
     */
    void logEstmatedTime();

    /**
     * @brief The ModelHandler object to handle the model files.
     */
    CCTools::ModelHandler model_handler_;

    /**
     * @brief The ModelCalculator object to calculate the harmonics.
     */
    CCTools::ModelCalculator calculator_;

    /**
     * @brief The harmonic component to optimize (1-indexed).
     */
    int component_;

    /**
     * @brief The range of the `offset` parameter for the grid search. Format: [min, max].
     */
    std::pair<double, double> &offset_range_;

    /**
     * @brief The range of the `slope` parameter for the grid search. Format: [min, max].
     */
    std::pair<double, double> &slope_range_;

    /**
     * @brief The granularity of the `offset` parameter for the grid search.
     */
    double &offset_granularity_;

    /**
     * @brief The granularity of the `slope` parameter for the grid search.
     */
    double &slope_granularity_;

    /**
     * @brief The start value of the magnitude ellipsoid relative to the harmonic calculation's axis. Used for the FittedSlopeObjective.
     */
    double mag_ell_start_;

    /**
     * @brief The end value of the magnitude ellipsoid relative to the harmonic calculation's axis. Used for the FittedSlopeObjective.
     */
    double mag_ell_end_;

    /**
     * @brief The vector to store the results of the grid search. Each GridSearchResult object represents one step.
     */
    std::vector<GridSearchResult> &results_;

    /**
     * @brief The vector of AbstractObjective objects to define the output criteria.
     */
    std::vector<std::shared_ptr<AbstractObjective>> &criteria_;

    /**
     * @brief The total number of steps for the grid search. Based on the `offset` and `slope` ranges and granularities.
     */
    int total_steps_;

    /**
     * @brief The estimated time for one harmonics calculation, i.e., one step of the grid search.
     */
    double estimated_time_per_step_;

};

#endif // GRID_SEARCH_H