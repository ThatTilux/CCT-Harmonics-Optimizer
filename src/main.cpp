#include "harmonics_calculator.h"
#include "model_handler.h"
#include "input_output.h"
#include "constants.h"
#include "bn_optimizer.h"
#include "grid_search_optimizer.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>


// TODO enable this again, create a grid search object
int run_grid_search(){
    // // TODO code duplication here, avoid by refactoring to func
    // // Get the JSON file path from user selection
    // boost::filesystem::path json_file_path;
    // try
    // {
    //     json_file_path = selectJsonFile();
    // }
    // catch (const std::exception &e)
    // {
    //     Logger::error(e.what());
    //     return 1;
    // }

    // // Handles manipulations of the JSON file
    // ModelHandler model_handler(json_file_path);

    // // Get all the scaling values for the custom CCT harmonics
    // HarmonicDriveParameterMap harmonic_drive_values = model_handler.getHarmonicDriveValues();

    // // Check that there are harmonic drives
    // if (harmonic_drive_values.empty()) {
    //     Logger::error("The program could not find any custom CCT harmonics (rat::mdl::cctharmonicdrive) whose name starts with the letter 'B'. Aborting...");
    //     return 1;
    // }

    // // Print them
    // print_harmonic_drive_values(harmonic_drive_values);

    // // Ask the user if they want to proceed
    // if (!askUserToProceed()) {
    //     Logger::info("Optimization aborted by user.");
    //     return 0;
    // }

    // // TODO log what the objective funciton is
    // // create Objective Function
    // std::shared_ptr<ObjectiveFunction> pObjective = std::make_shared<ObjectiveFunction>(model_handler, CHISQUARE_WEIGHT);

    // // grid search params
    // const double B1_OFFSET_MAX = -0.00011387 + 0.00001;
    // const double B1_OFFSET_MIN = -0.00011387 - 0.00001;

    // const double B1_SLOPE_MAX = -0.000038981 + 0.000003;
    // const double B1_SLOPE_MIN = -0.000038981 - 0.000003;

    // const double granularity_offset = 0.000001;
    // const double granularity_slope = 0.0000001;

    // // log all params
    // Logger::info("Grid search params:");
    // Logger::info("B1_OFFSET_MAX: " + std::to_string(B1_OFFSET_MAX));
    // Logger::info("B1_OFFSET_MIN: " + std::to_string(B1_OFFSET_MIN));
    // Logger::info("B1_SLOPE_MAX: " + std::to_string(B1_SLOPE_MAX));
    // Logger::info("B1_SLOPE_MIN: " + std::to_string(B1_SLOPE_MIN));
    // Logger::info("granularity_offset: " + std::to_string(granularity_offset));
    // Logger::info("granularity_slope: " + std::to_string(granularity_slope));

    // // assumed time used for 1 evaluation
    // const double time_per_evaluation_s = 0.7;

    // // compute how many evaluations we will do
    // double evaluations = ((B1_OFFSET_MAX - B1_OFFSET_MIN )/granularity_offset) * ((B1_SLOPE_MAX - B1_SLOPE_MIN)/granularity_slope);
    
    // // estimated time usage
    // double time_h = evaluations * time_per_evaluation_s / 60 / 60;

    // Logger::info("=== Running grid search for B1 with " + std::to_string(evaluations) + " evaluations ===");
    // Logger::info("Estimated time usage assuming " + std::to_string(time_per_evaluation_s) + "s for 1 eval: " + std::to_string(time_h) + " hours");

    // int counter = 1;

    // // run the grid search
    // for (double offset = B1_OFFSET_MIN; offset <= B1_OFFSET_MAX; offset += granularity_offset) {
    //     for (double slope = B1_SLOPE_MIN; slope <= B1_SLOPE_MAX; slope += granularity_slope) {
    //         Logger::info("== Running grid search iteration " + std::to_string(counter) + " ==");
    //         Logger::info("Offset: " + std::to_string(offset) + ", Slope: " + std::to_string(slope));
    //         HarmonicDriveParameterMap params;
    //         params["B1"] = HarmonicDriveParameters(offset, slope);
    //         pObjective->objective_function_slope(params);

    //         counter++;
    //     }
    // }

    return 0;
}

// TODO make this an object and move to a new file
int run_CSV_configs(const std::string& csv_file_path){
    // boost::filesystem::path json_file_path;
    // try
    // {
    //     json_file_path = selectJsonFile();
    // }
    // catch (const std::exception &e)
    // {
    //     Logger::error(e.what());
    //     return 1;
    // }

    // // Handles manipulations of the JSON file
    // ModelHandler model_handler(json_file_path);

    // // Get all the scaling values for the custom CCT harmonics
    // HarmonicDriveParameterMap harmonic_drive_values = model_handler.getHarmonicDriveValues();

    // // Check that there are harmonic drives
    // if (harmonic_drive_values.empty()) {
    //     Logger::error("The program could not find any custom CCT harmonics (rat::mdl::cctharmonicdrive) whose name starts with the letter 'B'. Aborting...");
    //     return 1;
    // }

    // // Print them
    // print_harmonic_drive_values(harmonic_drive_values);

    // // Ask the user if they want to proceed
    // if (!askUserToProceed()) {
    //     Logger::info("Optimization aborted by user.");
    //     return 0;
    // }

    // // TODO log what the objective funciton is
    // // create Objective Function
    // std::shared_ptr<ObjectiveFunction> pObjective = std::make_shared<ObjectiveFunction>(model_handler, CHISQUARE_WEIGHT);


    // // read CSV file
    // Logger::info("Reading configs from CSV file " + csv_file_path + "...");

    // std::ifstream file(csv_file_path);
    // if (!file.is_open()){
    //     Logger::error("Could not open file " + csv_file_path);
    //     return 1;
    // }

    // // read the offset and slope column to create the configs
    // std::vector<HarmonicDriveParameterMap> configs;

    // // read every line
    // std::string line;
    // // skip header line
    // std::getline(file, line);

    // while (std::getline(file, line)){
    //     std::stringstream ss(line);
    //     std::string index;
    //     std::string offset_str;
    //     std::string slope_str;
    //     std::getline(ss, index, ','); // we don't need this
    //     std::getline(ss, offset_str, ',');
    //     std::getline(ss, slope_str, ',');

    //     // convert to double
    //     double offset = std::stod(offset_str);
    //     double slope = std::stod(slope_str);

    //     // create the config
    //     HarmonicDriveParameterMap config;
    //     config["B1"] = HarmonicDriveParameters(offset, slope);

    //     configs.push_back(config);
    // }

    // Logger::info("Read " + std::to_string(configs.size()) + " configs from CSV file.");

    // // Run objective function for all configs
    // Logger::info("Running objective function for all configs...");

    // for (int i = 0; i < configs.size(); i++){
    //     Logger::info("== Running config " + std::to_string(i+1) + " of " + std::to_string(configs.size()) + " ==");
    //     HarmonicDriveParameterMap params = configs[i];
    //     Logger::info("Offset: " + std::to_string(params["B1"].getOffset()) + ", Slope: " + std::to_string(params["B1"].getSlope()));
    //     pObjective->objective_function_slope(configs[i]);
    // }

    return 0;
}
    


int main()
{   

    // check which optimization the user wants to do
    std::vector<std::string> optimization_options = {"bn optimization", "Grid search slope minimizer B1", "run CSV configs", "Run grid search optimizer"};
    int selected_optimization = 3; //TODO REMOVE selectFromList(optimization_options, "Please select the desired optimization:");



    if(selected_optimization == 0){
        // only bn optimization
        BnOptimizer optimizer = BnOptimizer();
        optimizer.optimize();
        optimizer.logResults();
        optimizer.exportModel();
        return 0;
    } else if (selected_optimization == 1){
        // grid search
        return run_grid_search();
    } else if (selected_optimization == 2){
        // run CSV configs
        return run_CSV_configs("./configs.csv");
    } else if (selected_optimization == 3){
        // run grid search optimizer
        GridSearchOptimizer optimizer = GridSearchOptimizer();
        optimizer.optimize();
        return 0;
    }


    
}