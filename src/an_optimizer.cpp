#include "an_optimizer.h"

// Default constructor
AnOptimizer::AnOptimizer() : LinearOptimizer("an", "A")
{
}

// Constructor without any user interaction.
AnOptimizer::AnOptimizer(ModelHandler &model_handler, double max_value) : LinearOptimizer("an", "A", model_handler, max_value)
{}

// Get the an values from the harmonics handler
std::vector<double> AnOptimizer::getValues(HarmonicsDataHandler &harmonics_handler)
{
    return harmonics_handler.get_an();
}
