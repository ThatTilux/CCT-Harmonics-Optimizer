#ifndef BN_OPTIMIZER_H
#define BN_OPTIMIZER_H

#include "linear_optimizer.h"

/**
 * @brief Optimizer to minimize the bn values linearly.
 * 
 * This optimizer optimizes the scaling function of harmonic drives linearly.
 * The objective is achieved when the absolute bn values of all components are below the specified `max_value_`.
 * This optimizer performs several rounds of linear optimizations until the objective is met.
 * In each round, all harmonic drives present in the model (except for the main component) are optimized one by one. The `constant` or `slope` parameter of the scaling function (depending on the `amplitude` of the harmonic drive) is optimized linearly;
 * the optimizer changes the parameter in small steps and calculates the bn value for each step. The optimizer then fits a linear function to the data points and extracts the root of the function as the new parameter value.
 */
class BnOptimizer : public LinearOptimizer {
public:

    /**
     * @brief Construct a new BnOptimizer object with suppressed user interaction.
     * @param model_handler The ModelHandler object to be used.
     * @param max_value Threshold for absolute bn values in the optimizer.
     * 
     * This constructor initializes and sets up the optimizer. User interaction is suppressed.
     */
    BnOptimizer(CCTools::ModelHandler &model_handler, double max_value);
    
    /**
     * @brief Construct a new BnOptimizer object with user interaction.
     * 
     * This constructor initializes and sets up the optimizer.
     * The user is asked to select a model and enter a maximum absolute value for the optimized values from the command line.
     */
    BnOptimizer();

    virtual ~BnOptimizer() {};

protected:

    /**
     * @brief Get the bn values as the objective.
     * @param harmonics_handler Result of the harmonics calculation
     * 
     * This function provides the parent class with the simulated bn values as the objective function. 
     */
    std::vector<double> getValues(CCTools::HarmonicsDataHandler &harmonics_handler) override;
};

#endif // BN_OPTIMIZER_H
