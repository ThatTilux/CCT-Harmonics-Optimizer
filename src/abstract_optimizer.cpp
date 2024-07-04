#include "abstract_optimizer.h"

AbstractOptimizer::AbstractOptimizer(bool disable_user_interaction) : disable_user_interaction_(disable_user_interaction)
{
}

// Function to initialize the model
ModelHandler& AbstractOptimizer::initModel()
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
    calculator_ = HarmonicsCalculator(model_handler_.getTempJsonPath());
}

// Function to let the user select the desired model
void AbstractOptimizer::getModelSelection()
{
    boost::filesystem::path json_file_path;
    try
    {
        json_file_path = selectJsonFile();
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
    if(!disable_user_interaction_){
        if (!askUserToProceed())
        {
            Logger::info("Optimization aborted by user.");
            throw std::runtime_error("User aborted optimization.");
        }
    }

    return harmonic_drive_values;
}

// Function to perform linear regression and return the slope and intercept
std::pair<double, double> AbstractOptimizer::linearRegression(const std::vector<std::pair<double, double>> &points)
{
    size_t n = points.size();
    if (n < 2)
    {
        throw std::runtime_error("Not enough points for linear regression.");
    }

    double sum_x = 0, sum_y = 0, sum_xx = 0, sum_xy = 0;
    for (const auto &point : points)
    {
        sum_x += point.first;
        sum_y += point.second;
        sum_xx += point.first * point.first;
        sum_xy += point.first * point.second;
    }

    double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
    double intercept = (sum_y - slope * sum_x) / n;

    return {slope, intercept};
}