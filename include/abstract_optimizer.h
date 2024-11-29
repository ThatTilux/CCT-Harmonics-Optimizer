// AbstractOptimizer.h
#ifndef ABSTRACTOPTIMIZER_H
#define ABSTRACTOPTIMIZER_H

#include <iostream>
#include <boost/filesystem.hpp>
#include "input_output.h"
#include "model_calculator.h"
#include "statistical_analysis.h"

/**
 * @brief Abstract class for optimization algorithms.
 * 
 * This class provides common functions and variables for optimization algorithms.
 */
class AbstractOptimizer
{
public:
    /**
     * @brief Construct a new AbstractOptimizer object.
     * @param disable_user_interaction Flag to disable user interaction.
     * 
     * This constructor does not do anything. All setup is done in the derived classes.
     */
    AbstractOptimizer(bool disable_user_interaction = false);

    /**
     * @brief Export the current model.
     * 
     * This function copies the current model from the model handler to an output directory.
     */
    virtual void exportModel();

    /**
     * @brief Run the optimization algorithm.
     * 
     * This function is pure virtual and needs to be implemented in the derived classes.
     */
    virtual void optimize() = 0;

    /**
     * @brief Log the results of the optimization.
     * 
     * This function is pure virtual and needs to be implemented in the derived classes.
     */
    virtual void logResults() = 0;

    virtual ~AbstractOptimizer(){};

protected:

    /**
     * @brief Initialize the model.
     * 
     * This function lets the user select the model in the terminal and initializes the model handler.
     */
    CCTools::ModelHandler &initModel();

    /**
     * @brief Initialize the calculator.
     * 
     * This function initializes the model calculator using the model handler.
     */
    void initCalculator();

    /**
     * @brief Get the minimum magnet ell position.
     * @returns The minimum magnet ell position (mm).
     * @throws std::runtime_error if the ell bounds have not been set.
     * 
     * This function returns the ell value in mm where the magnet begins relative to the harmonic calculation's axis.
     * This value can be used to determine at which point of the harmonic calculation results' relative ell axis the magnet starts.
     */
    double getMinMagnetEll();

    /**
     * @brief Get the maximum magnet ell position.
     * @returns The maximum magnet ell position (mm).
     * @throws std::runtime_error if the ell bounds have not been set.
     * 
     * This function returns the ell value in mm where the magnet ends relative to the harmonic calculation's axis.
     * This value can be used to determine at which point of the harmonic calculation results' relative ell axis the magnet ends.
     */
    double getMaxMagnetEll();

    /**
     * @brief Get the inferred length of the magnet.
     * @returns The inferred length of the magnet (mm).
     * 
     * This function returns the inferred length of the magnet in mm.
     */
    double getMagnetLength();

    /**
     * @brief Get the main component of the magnet.
     * @returns The main component of the magnet (1-indexed).
     * @throws std::runtime_error if the main component has not been set.
     * 
     * This function returns the main component of the magnet (e.g., 2 for a quadrupole).
     */
    int getMainComponent();

    /**
     * @brief Compute and set the ell bounds for the magnet.
     * 
     * This function computes the ell bounds for the magnet by running a mesh calculation and sets them in the `cct_ell_bounds_` variable.
     * The ell bounds describe the part along the harmonic calculation's relative ell axis where the magnet actually is.
     */
    void computeMagnetEllBounds();

    /**
     * @brief Get, log and return all custom CCT harmonic drive values of the loaded model.
     * @returns A map of the custom CCT harmonic drive values.
     * 
     * This function gets all custom CCT harmonic drive values from the model handler and logs them in the terminal.
     * Asks the user for confirmation to proceed unless disabled by flag.
     */
    CCTools::HarmonicDriveParameterMap initHarmonicDrives();

    /**
     * @brief Assert that there are only linear drives in the model.
     * @throws std::runtime_error if a drive is not linear.
     * 
     * This function checks if all custom CCT harmonic drives have an `amplitude` of `linear`.
     */
    void assertOnlyLinearDrives();

    /**
     * @brief Assert that there are custom CCT harmonics for all components (except for the main one) present.
     * @throws std::runtime_error if a custom CCT harmonic is missing.
     * 
     * This function checks if custom CCT harmonics for B1 to B10 (except for the main component) are present in the model.
     * Sets the main component if not already set.
     */
    void assertAllHarmonicsPresent();

    /**
     * @brief Check if the `main_component_` variable has been set and if the main harmonic has a bn value of 10,000.
     * @throws std::runtime_error if the main component has not been set or the bn value is not 10,000.
     * 
     * This function checks if the `main_component_` variable has been set and runs a harmonic calculation to check if the main harmonic has a bn value of 10,000.
     * Logs the bn values.
     */
    void checkMainComponent();

    /**
     * @brief Path to the JSON file of the selected model.
     */
    boost::filesystem::path json_file_path_;

    /**
     * @brief Model handler to read and write the model.
     */
    CCTools::ModelHandler model_handler_;

    /**
     * @brief Model calculator to run calculations on the model.
     */
    CCTools::ModelCalculator calculator_;

    /**
     * @brief Flag to disable user interaction.
     */
    bool disable_user_interaction_;

    /**
     * @brief Prefix for the names of targeted custom CCT harmonics.
     * @details Default value: "B".
     */
    std::string harmonic_drive_prefix_ = "B";


private:

    /**
     * @brief Get the model selection from the user.
     * 
     * This function lets the user select the model in the terminal.
     */
    void getModelSelection();

    /**
     * @brief Bounds for the magnet ell position in mm.
     * @details Represents the part along the harmonic calculation's relative ell axis where the magnet actually is.
     * Default value: {0, 0}.
     */
    std::pair<double, double> cct_ell_bounds_ = {0, 0};

    /**
     * @brief Main component of the magnet (1-indexed).
     * @details Default value of -1 indicates the main component has not been set.
     */
    int main_component_ = -1;

};

#endif // ABSTRACTOPTIMIZER_H
