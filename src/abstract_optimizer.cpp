#include "abstract_optimizer.h"

using CCTools::Logger;

AbstractOptimizer::AbstractOptimizer(bool disable_user_interaction) : disable_user_interaction_(disable_user_interaction)
{
}

CCTools::ModelHandler &AbstractOptimizer::initModel()
{
    // let user select the model
    getModelSelection();

    // load model
    model_handler_ = CCTools::ModelHandler(json_file_path_);

    return model_handler_;
}

void AbstractOptimizer::initCalculator()
{
    // make sure the model handler is initiliazed
    if (model_handler_.getTempJsonPath().empty())
    {
        Logger::error("Model handler not initialized");
        return;
    }

    // load calculator
    calculator_ = CCTools::ModelCalculator(model_handler_.getTempJsonPath());
}

void AbstractOptimizer::getModelSelection()
{
    try
    {
        json_file_path_ = selectModelFileForOptimization();
    }
    catch (const std::exception &e)
    {
        Logger::error(e.what());
    }
}

CCTools::HarmonicDriveParameterMap AbstractOptimizer::initHarmonicDrives()
{
    // Get all the scaling values for the custom CCT harmonics
    CCTools::HarmonicDriveParameterMap harmonic_drive_values = model_handler_.getHarmonicDriveValues(harmonic_drive_prefix_);

    // Check that there are harmonic drives
    if (harmonic_drive_values.empty())
    {
        Logger::error("The program could not find any custom CCT harmonics (rat::mdl::cctharmonicdrive) whose name starts with '" + harmonic_drive_prefix_ + "'. Aborting...");
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

void AbstractOptimizer::assertOnlyLinearDrives()
{
    CCTools::HarmonicDriveParameterMap params = model_handler_.getHarmonicDriveValues();
    for (auto &param : params)
    {
        if (!param.second.isOffsetAndSlope())
            throw std::runtime_error("The selected model has the custom harmonic " + param.first + " with an 'amplitude' value other than 'linear'. This is not supported for this optimizer.");
    }
}

void AbstractOptimizer::assertAllHarmonicsPresent()
{
    CCTools::HarmonicDriveParameterMap params = model_handler_.getHarmonicDriveValues();
    for (int i = 1; i <= 10; i++)
    {
        if (params.find("B" + std::to_string(i)) == params.end())
        {
            // i should be the main component
            try
            {
                // this will throw an error when the main component has not been set
                getMainComponent();
            }
            catch (const std::runtime_error &e)
            {
                // the main component has not been set, all good
                main_component_ = i;
                continue;
            }
            // if the main component has already been set, there is at least 1 component missing.
            throw std::runtime_error("The selected model does not have a custom harmonic for the harmonics " + std::to_string(getMainComponent()) + " and " + std::to_string(i) + " namend BX for X in 1..10. All component aside from the main component need such a harmonic for this optimizer. Aborting...");
        }
    }
    Logger::info("Detected B" + std::to_string(main_component_) + " as the main component.");
}

void AbstractOptimizer::checkMainComponent()
{
    // get the main component
    int main_component = getMainComponent();

    // get the bn values
    CCTools::HarmonicsDataHandler harmonics_handler;
    calculator_.reload_and_calc_harmonics(model_handler_.getTempJsonPath(), harmonics_handler);
    std::vector<double> bn_values = harmonics_handler.get_bn();

    // check if the main component has a bn of 10,000
    if (bn_values[main_component - 1] != 10000)
    {
        Logger::error("The main component B" + std::to_string(main_component) + " does not have a bn value of 10,000. The current value is " + std::to_string(bn_values[main_component - 1]) + ". Aborting...");
        throw std::runtime_error("The main component does not have a bn value of 10,000.");
    }

    // print the bn values
    log_vector(bn_values, "bn");
}

double AbstractOptimizer::getMinMagnetEll()
{
    if (cct_ell_bounds_.first == 0.0 && cct_ell_bounds_.second == 0.0)
    {
        throw std::runtime_error("The ell bounds have not been set. Please set them before calling this function.");
    }
    return cct_ell_bounds_.first;
}

double AbstractOptimizer::getMaxMagnetEll()
{
    if (cct_ell_bounds_.first == 0.0 && cct_ell_bounds_.second == 0.0)
    {
        throw std::runtime_error("The ell bounds have not been set. Please set them before calling this function.");
    }
    return cct_ell_bounds_.second;
}

double AbstractOptimizer::getMagnetLength()
{
    return getMaxMagnetEll() - getMinMagnetEll();
}

int AbstractOptimizer::getMainComponent()
{
    if (main_component_ == -1)
    {
        throw std::runtime_error("The main component has not been set. Please set it before calling this function.");
    }
    return main_component_;
}

void AbstractOptimizer::computeMagnetEllBounds()
{
    // retrieve the z bounds of the magnet
    CCTools::MeshDataHandler mesh_handler;
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

void AbstractOptimizer::exportModel()
{
    copyModelWithTimestamp(model_handler_.getTempJsonPath());
}