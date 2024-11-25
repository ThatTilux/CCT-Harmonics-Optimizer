#include "bn_optimizer.h"

BnOptimizer::BnOptimizer() : LinearOptimizer("bn", "B") {}

BnOptimizer::BnOptimizer(CCTools::ModelHandler &model_handler, double max_value) : LinearOptimizer("bn", "B", model_handler, max_value) {}

std::vector<double> BnOptimizer::getValues(CCTools::HarmonicsDataHandler &harmonics_handler)
{
    return harmonics_handler.get_bn();
}