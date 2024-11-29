#include "an_optimizer.h"

AnOptimizer::AnOptimizer() : LinearOptimizer("an", "A")
{
    setup();
}

AnOptimizer::AnOptimizer(CCTools::ModelHandler &model_handler, double max_value) : LinearOptimizer("an", "A", model_handler, max_value)
{
    setup();
}

void AnOptimizer::setup()
{
    // check that all custom harmonics have an amplitude of constant
    checkForHarmonicDriveConstraints(initial_drive_values_);
}

std::vector<double> AnOptimizer::getValues(CCTools::HarmonicsDataHandler &harmonics_handler)
{
    return harmonics_handler.get_an();
}

void AnOptimizer::checkForHarmonicDriveConstraints(CCTools::HarmonicDriveParameterMap harmonic_drive_values)
{
    for (auto &param : harmonic_drive_values)
    {
        if (!param.second.isConstant())
            throw std::runtime_error("The selected model has the custom harmonic " + param.first + " with an 'amplitude' value other than 'constant'. This is not supported for this optimizer.");
    }
}