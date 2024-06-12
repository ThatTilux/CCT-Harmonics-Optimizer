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
        std::cerr << e.what() << std::endl;
        return 1;
    }

    // Handles manipulations of the JSON file
    ModelHandler model_handler(json_file_path);
    // get path of the temp model
    const boost::filesystem::path temp_json_file_path = model_handler.getTempJsonPath();

    // Get user input for maximum harmonic value
    double max_harmonic_value = getUserInput("Enter the maximum absolute value for harmonic values", 0.1);

    // Handles calculations for the model
    HarmonicsCalculator calculator(temp_json_file_path);

    // Get all the scaling values for the custom CCT harmonics
    HarmonicDriveParameterMap harmonic_drive_values = model_handler.getHarmonicDriveValues();

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
    print_vector(current_bn_values, "bn");

    // export the model
    copyModelWithTimestamp(temp_json_file_path);

    return 0;
}

int run_bn_chisquare_optimization(){
    // TODO figure out a good value for chiSquared

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
        std::cerr << "Error: " << e.what() << std::endl;
    }

    Py_Finalize(); // Manually finalize the Python interpreter


    // read results
    std::ifstream file("optimization_results.txt");
    std::string line;
    std::vector<double> best_params;

    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.find("Best parameters:") != std::string::npos) {
                size_t pos = line.find(":");
                std::string params_str = line.substr(pos + 1);
                std::stringstream ss(params_str);
                double param;
                while (ss >> param) {
                    best_params.push_back(param);
                }
            }
        }
        file.close();
    }

    // print them
    std::cout << "Best parameters found:" << std::endl;
    for (double param : best_params) {
        std::cout << param << " ";
    }
    std::cout << std::endl;

    return 0;
}


int main()
{   

    // check which optimization the user wants to do
    std::vector<std::string> optimization_options = {"bn optimization", "bn and chiSquare optimization"};
    int selected_optimization = selectFromList(optimization_options);

    if(selected_optimization == 0){
        // only bn optimization
        return run_bn_optimization();
    } else if (selected_optimization == 1){
        // bn and chiSquare optimization
        return run_bn_chisquare_optimization();
    }

    
}