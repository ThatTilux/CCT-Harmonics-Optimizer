#include "an_optimizer.h"

// Default constructor
AnOptimizer::AnOptimizer() : LinearOptimizer("an", "A")
{
    setup();
}

// Constructor without any user interaction.
AnOptimizer::AnOptimizer(ModelHandler &model_handler, double max_value) : LinearOptimizer("an", "A", model_handler, max_value)
{
    setup();
}

// Setup called from all constructors
void AnOptimizer::setup(){
    // check that all custom harmonics have an amplitude of constant
    checkForHarmonicDriveConstraints(drive_values_);
}

// Get the an values from the harmonics handler
std::vector<double> AnOptimizer::getValues(HarmonicsDataHandler &harmonics_handler)
{
    return harmonics_handler.get_an();
}

// Function to check that all custom harmonics have an amplitude of constant - this is required for the an optimizer
void AnOptimizer::checkForHarmonicDriveConstraints(HarmonicDriveParameterMap harmonic_drive_values){
    for (auto &param : harmonic_drive_values)
    {
        if (!param.second.isConstant())
            throw std::runtime_error("The selected model has the custom harmonic "+ param.first +" with an 'amplitude' value other than 'constant'. This is not supported for this optimizer.");
    }
}