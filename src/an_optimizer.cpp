#include "an_optimizer.h"

// Default constructor
AnOptimizer::AnOptimizer() : LinearOptimizer("an", AN_OPTIMIZER_MAX_VALUE)
{
}

// Constructor without any user interaction.
AnOptimizer::AnOptimizer(ModelHandler &model_handler, double max_value) : LinearOptimizer("an", model_handler, max_value)
{
    if (max_value > AN_OPTIMIZER_MAX_VALUE){
        throw std::runtime_error("Set max an value of " + std::to_string(max_value) + " is larger than the allowed upper bound of " + std::to_string(AN_OPTIMIZER_MAX_VALUE) + " for the an optimizer.");
    }
}

// Get the an values from the harmonics handler
std::vector<double> AnOptimizer::getValues(HarmonicsDataHandler &harmonics_handler)
{
    return harmonics_handler.get_an();
}
