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

// Function to return the ell value in mm where the magnet begins relative to the axis.
double AbstractOptimizer::getMinMagnetEll()
{
    if (cct_ell_bounds_.first == 0.0 && cct_ell_bounds_.second == 0.0)
    {
        throw std::runtime_error("The ell bounds have not been set. Please set them before calling this function.");
    }
    return cct_ell_bounds_.first;
}

// Function to return the ell value in mm where the magnet ends relative to the axis.
double AbstractOptimizer::getMaxMagnetEll()
{
    if (cct_ell_bounds_.first == 0.0 && cct_ell_bounds_.second == 0.0)
    {
        throw std::runtime_error("The ell bounds have not been set. Please set them before calling this function.");
    }
    return cct_ell_bounds_.second;
}

// Function to compute and set the ell bounds for the magnet. The ell bounds are the part along the length of the axis where the magnet actually is.
void AbstractOptimizer::computeMagnetEllBounds()
{
    // retrieve the z bounds of the magnet
    MeshDataHandler mesh_handler;
    calculator_.reload_and_calc_mesh(model_handler_.getTempJsonPath(), mesh_handler);
    auto [z_min, z_max] = mesh_handler.getMinMaxZValues();

    // retrieve the axis position
    double axis_z = calculator_.get_axis_z_pos();

    // retrieve the axis length (=ell)
    double ell = calculator_.get_axis_ell();

    // the start and end of the magnet relative to the axis center [m]
    double magnet_start = axis_z - z_min;
    double magnet_end = axis_z - z_max;

    // the start and end of the magnet relative to the axis length [m]
    double magnet_start_ell = ell / 2 - magnet_start;
    double magnet_end_ell = ell / 2 - magnet_end;

    // set the bounds [mm]
    cct_ell_bounds_ = {magnet_start_ell * 1000, magnet_end_ell * 1000};

    // Log results
    Logger::info("Computed magnet ell bounds: " + std::to_string(cct_ell_bounds_.first) + " mm to " + std::to_string(cct_ell_bounds_.second) + " mm.");
    double length = cct_ell_bounds_.second - cct_ell_bounds_.first;
    Logger::info("Magnet length: " + std::to_string(length) + " mm.");
} 

// Function to export the optimized model
void AbstractOptimizer::exportModel()
{
    copyModelWithTimestamp(model_handler_.getTempJsonPath());
}