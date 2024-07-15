#include "abstract_optimizer.h"

AbstractOptimizer::AbstractOptimizer(bool disable_user_interaction) : disable_user_interaction_(disable_user_interaction)
{
}

// Function to initialize the model
ModelHandler &AbstractOptimizer::initModel()
{
    // let user select model
    getModelSelection();

    // load model
    model_handler_ = ModelHandler(json_file_path_);

    return model_handler_;
}

// Function to init the harmonics calculator using the initialized model handler
void AbstractOptimizer::initCalcultor()
{
    // make sure the model handler is initiliazed
    if (model_handler_.getTempJsonPath().empty())
    {
        Logger::error("Model handler not initialized");
        return;
    }

    // load calculator
    calculator_ = ModelCalculator(model_handler_.getTempJsonPath());
}

// Function to let the user select the desired model
void AbstractOptimizer::getModelSelection()
{
    try
    {
        json_file_path_ = selectJsonFile();
    }
    catch (const std::exception &e)
    {
        Logger::error(e.what());
    }
}

// Function to let the user select the maximum bn value
double AbstractOptimizer::getMaxHarmonicValue()
{
    return getUserInput("Enter the maximum absolute value for harmonic values", DEFAULT_MAX_BN_VALUE);
}

// Function to get, print and return all custom CCT harmonics in the loaded model. Will ask for user's confirmation to proceed (if not disabled by flag)
HarmonicDriveParameterMap AbstractOptimizer::initHarmonicDrives()
{
    // Get all the scaling values for the custom CCT harmonics
    HarmonicDriveParameterMap harmonic_drive_values = model_handler_.getHarmonicDriveValues();

    // Check that there are harmonic drives
    if (harmonic_drive_values.empty())
    {
        Logger::error("The program could not find any custom CCT harmonics (rat::mdl::cctharmonicdrive) whose name starts with the letter 'B'. Aborting...");
        throw std::runtime_error("No custom CCT harmonics found in the model.");
    }

    // Print them
    print_harmonic_drive_values(harmonic_drive_values);

    // Ask the user if they want to proceed
    if (!disable_user_interaction_)
    {
        if (!askUserToProceed())
        {
            Logger::info("Optimization aborted by user.");
            throw std::runtime_error("User aborted optimization.");
        }
    }

    return harmonic_drive_values;
}

// Function to assert that there are only custom harmonics with an 'amplitude' of linear. Throws std::runtime_error if not
void AbstractOptimizer::assertOnlyLinearDrives()
{
    HarmonicDriveParameterMap params = model_handler_.getHarmonicDriveValues();
    for (auto &param : params)
    {
        if (!param.second.isOffsetAndSlope())
            throw std::runtime_error("The selected model has one or more custom harmonics with an 'amplitude' value other than 'linear'. This is not supported for this optimizer.");
    }
}

// Function to assert that there are custom harmonics for all harmonics from 1 to 10 (except for the main one). Throws std::runtime_error if not
void AbstractOptimizer::assertAllHarmonicsPresent()
{
    HarmonicDriveParameterMap params = model_handler_.getHarmonicDriveValues();
    for (int i = 1; i <= 10; i++)
    {
        if (i != MAIN_COMPONENT && params.find("B" + std::to_string(i)) == params.end())
            throw std::runtime_error("The selected model does not have a custom harmonic for harmonic " + std::to_string(i) + ". This is not supported for this optimizer.");
    }
}

// Function to check if all abs bn values are below a certain threshold
bool AbstractOptimizer::areAllHarmonicsBelowThreshold(double threshold)
{
    Logger::info("Checking if all bn values are below threshold " + std::to_string(threshold) + "...");
    HarmonicsDataHandler harmonics_handler;
    calculator_.reload_and_calc_harmonics(model_handler_.getTempJsonPath(), harmonics_handler);

    std::vector<double> bn_values = harmonics_handler.get_bn();
    for (double bn : bn_values)
    {
        if (std::abs(bn) > threshold)
        {
            Logger::info("Found bn value " + std::to_string(bn) + " above threshold " + std::to_string(threshold) + ".");
            return false;
        }
    }

    Logger::info("All bn values are below threshold " + std::to_string(threshold) + ".");
    return true;
}

// Function to export the optimized model
void AbstractOptimizer::exportModel()
{
    copyModelWithTimestamp(model_handler_.getTempJsonPath());
}