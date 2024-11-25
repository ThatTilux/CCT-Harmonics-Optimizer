#ifndef AN_OPTIMIZER_H
#define AN_OPTIMIZER_H

#include "linear_optimizer.h"
#include "an_objective.hh"

/**
 * @brief Optimizer to minimize the an values linearly.
 * 
 * This optimizer optimizes the scaling function of harmonic drives linearly.
 * The objective is achieved when the absolute an values of all components are below the specified `max_value_`.
 * This optimizer performs several rounds of linear optimizations until the objective is met.
 * In each round, all harmonic drives present in the model (except for the main component) are optimized one by one. The `constant` or parameter of the scaling function is optimized linearly (all harmonic drives must have an `amplitude` of `constant`);
 * the optimizer changes the parameter in small steps and calculates the an value for each step. The optimizer then fits a linear function to the data points and extracts the root of the function as the new parameter value.
 */
class AnOptimizer : public LinearOptimizer {
public:
    /**
     * @brief Construct a new AnOptimizer object with suppressed user interaction.
     * @param model_handler The ModelHandler object to be used.
     * @param max_value Threshold for absolute an values in the optimizer.
     * 
     * This constructor initializes and sets up the optimizer. User interaction is suppressed.
     */
    AnOptimizer(CCTools::ModelHandler &model_handler, double max_value);

    /**
     * @brief Construct a new AnOptimizer object with user interaction.
     * 
     * This constructor initializes and sets up the optimizer.
     * The user is asked to select a model and enter a maximum absolute value for the optimized values from the command line.
     */
    AnOptimizer();

    virtual ~AnOptimizer() {};

protected:

    /**
     * @brief Get the an values as the objective.
     * @param harmonics_handler Result of the harmonics calculation
     * 
     * This function provides the parent class with the simulated an values as the objective function. 
     */
    std::vector<double> getValues(CCTools::HarmonicsDataHandler &harmonics_handler) override;


private:
    /**
     * @brief Setup function called from constructors. Performs model checks.
     */
    void setup();

    /**
     * @brief Checks the required constraints on the model's harmonic drives for this optimizer.
     * @param harmonic_drive_values The harmonic drives of the model
     * 
     * This function asserts that all the custom cct harmonics in the model have an `amplitude` of `constant`.
     * This is required for this optimizer.
     */
    void checkForHarmonicDriveConstraints(CCTools::HarmonicDriveParameterMap harmonic_drive_values);
};

#endif // AN_OPTIMIZER_H
