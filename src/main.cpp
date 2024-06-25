#include "harmonics_calculator.h"
#include "optimizer_bindings.h"
#include "model_handler.h"
#include "input_output.h"
#include "optimizer.h"
#include "constants.h"

#include <pybind11/embed.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

namespace py = pybind11;


int run_bn_optimization(){
    // Get the JSON file path from user selection
    boost::filesystem::path json_file_path;
    try
    {
        json_file_path = selectJsonFile();
    }
    catch (const std::exception &e)
    {
        Logger::error(e.what());
        return 1;
    }

    // Handles manipulations of the JSON file
    ModelHandler model_handler(json_file_path);
    // get path of the temp model
    const boost::filesystem::path temp_json_file_path = model_handler.getTempJsonPath();

    // Get user input for maximum harmonic value
    double max_harmonic_value = getUserInput("Enter the maximum absolute value for harmonic values", DEFAULT_MAX_BN_VALUE);

    // Handles calculations for the model
    HarmonicsCalculator calculator(temp_json_file_path);

    // Get all the scaling values for the custom CCT harmonics
    HarmonicDriveParameterMap harmonic_drive_values = model_handler.getHarmonicDriveValues();

    // Check that there are harmonic drives
    if (harmonic_drive_values.empty()) {
        Logger::error("The program could not find any custom CCT harmonics (rat::mdl::cctharmonicdrive) whose name starts with the letter 'B'. Aborting...");
        return 1;
    }

    // Print them
    print_harmonic_drive_values(harmonic_drive_values);

    // Ask the user if they want to proceed
    if (!askUserToProceed()) {
        Logger::info("Optimization aborted by user.");
        return 0;
    }

    // optimizer will put resulting bn values in here
    std::vector<double> current_bn_values;

    // optimize the harmonic drive values
    optimize(calculator, model_handler, current_bn_values, harmonic_drive_values, max_harmonic_value, temp_json_file_path);

    // optimization was successful, print results
    Logger::info("=== All harmonics have been optimized ===");
    Logger::info("User-specified margin was: " + std::to_string(max_harmonic_value));
    print_harmonic_drive_values(harmonic_drive_values);
    print_vector(current_bn_values, "bn");

    // export the model
    copyModelWithTimestamp(temp_json_file_path);

    return 0;
}

int run_bn_chisquare_optimization(){
    // TODO figure out a good value for chiSquared

    Logger::log_timestamp("Starting python script");
    Py_Initialize(); // Manually initialize the Python interpreter

    try {
        py::exec(R"(
            import sys
            sys.path.append('../scripts')
            sys.path.append('../build/lib')
        )");

        py::exec(R"(
            import bayesian_optimization # this will run the file
        )");
    } catch (const py::error_already_set &e) {
        Logger::error("Error: " + std::string(e.what()));
    }

    Py_Finalize(); // Manually finalize the Python interpreter
    Logger::log_timestamp("Python script finished");


    // results are logged in the Logger, no need to export them

    return 0;
}


int main()
{   

    // check which optimization the user wants to do
    std::vector<std::string> optimization_options = {"bn optimization", "bn and chiSquare optimization"};
    int selected_optimization = selectFromList(optimization_options, "Please select the desired optimization:");

    if(selected_optimization == 0){
        // only bn optimization
        return run_bn_optimization();
    } else if (selected_optimization == 1){
        // bn and chiSquare optimization
        return run_bn_chisquare_optimization();
    }

    
}