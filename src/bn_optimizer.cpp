#include "bn_optimizer.h"

// Default constructor
BnOptimizer::BnOptimizer() : LinearOptimizer("bn", "B") {}

// Constructor without any user interaction.
BnOptimizer::BnOptimizer(CCTools::ModelHandler &model_handler, double max_value) : LinearOptimizer("bn", "B", model_handler, max_value) {}

// Get the bn values from the harmonics handler
std::vector<double> BnOptimizer::getValues(CCTools::HarmonicsDataHandler &harmonics_handler)
{
    return harmonics_handler.get_bn();
}