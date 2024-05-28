#include "harmonics_calculator.h"
#include "model_handler.h"
#include "input_output.h"
#include "optimizer.h"
#include "constants.h"

#include <iostream>
#include <vector>

int main()
{
    // Get the JSON file path from user selection
    boost::filesystem::path json_file_path;
    try
    {
        json_file_path = selectJsonFile();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    // Handles manipulations of the JSON file
    ModelHandler model_handler(json_file_path);
    // get path of the temp model
    const boost::filesystem::path temp_json_file_path = model_handler.getTempJsonPath();
    // Handles calculations for the model
    HarmonicsCalculator calculator(json_file_path);

    // Get user input for maximum harmonic value
    double max_harmonic_value = getUserInput("Enter the maximum absolute value for harmonic values", 0.1);

    // Get all the scaling values for the custom CCT harmonics
    std::vector<std::pair<int, double>> harmonic_drive_values = model_handler.getHarmonicDriveValues();

    // Check that there are harmonic drives
    if (harmonic_drive_values.empty())
    {
        std::cerr << "The program could not find any custom CCT harmonics (rat::mdl::cctharmonicdrive) whose name starts with the letter 'B'. Aborting..." << std::endl;
        return 1;
    }

    // Print them
    print_harmonic_drive_values(harmonic_drive_values);

    // Ask the user if they want to proceed
    if (!askUserToProceed())
    {
        std::cout << "Optimization aborted by user." << std::endl;
        return 0;
    }

    // optimizer will put resulting bn values in here
    std::vector<double> current_bn_values;

    // optimize the harmonic drive values
    optimize(calculator, model_handler, current_bn_values, harmonic_drive_values, max_harmonic_value, temp_json_file_path);

    // optimization was successful, print results
    std::cout << "=== All harmonics have been optimized ===" << std::endl;
    std::cout << "User-specified margin was: " << max_harmonic_value << std::endl;
    print_harmonic_drive_values(harmonic_drive_values);
    print_bn(current_bn_values);

    // export the model
    copyModelWithTimestamp(temp_json_file_path);

    return 0;
}