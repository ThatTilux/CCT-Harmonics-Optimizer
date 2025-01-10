#ifndef INPUT_OUTPUT_H
#define INPUT_OUTPUT_H

#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <constants.h>
#include <armadillo>
#include <chrono>
#include <rat/common/log.hh>
#include "harmonic_drive_parameter.h"
#include "model_handler.h"
#include "Logger.hh"
#include "grid_search_result.h"

/**
 * @brief Function to log harmonic drive values.
 * @param harmonic_drive_values HarmonicDriveParameterMap containing the harmonic drive values.
 */
void print_harmonic_drive_values(CCTools::HarmonicDriveParameterMap &harmonic_drive_values);


/**
 * @brief Function to ask the user if they want to proceed.
 * @return True if the user wants to proceed, false otherwise.
 *
 * This function asks the user if they want to proceed with the optimization process.
 * The user can enter 'Y' to proceed or anything else to abort the program.
 */
bool askUserToProceed();

/**
 * @brief Function to get user input in the terminal with a prompt and default value. User must enter value > 0.
 * @param prompt The prompt to display to the user.
 * @param default_value The default value to display and use if the user does not enter anything.
 * @return The value entered by the user or the default value.
 *
 * This function prompts the user for input with a prompt and a default value.
 * If the user does not enter anything, the default value is returned.
 * The user is re-prompted as long as they enter a value <= 0 or non-numeric value.
 */
double getUserInput(const std::string &prompt, double default_value);

/**
 * @brief Function to display the JSON files in the data directory and allow the user to select one for optimization.
 * @returns The path to the selected JSON file.
 * @throws std::runtime_error If the data directory does not contain any JSON files.
 *
 * This function displays the JSON files in the data directory and prompts the user to select one for optimization.
 * The user can use the arrow keys to navigate the list and press enter to select a file.
 */
boost::filesystem::path selectModelFileForOptimization();

/**
 * @brief Helper function to display a list of options to the user in the terminal and let them select one.
 * @param options The list of options to display.
 * @param user_prompt The prompt to display to the user.
 * @returns The index of the selected option.
 *
 * This function displays a list of options to the user and lets them select one.
 * The user can use the arrow keys to navigate the list and press enter to select an option.
 */
int selectFromList(std::vector<std::string> options, std::string user_prompt);

/**
 * @brief Function to copy a model file from the source path to the build directory with a timestamp appended to the filename.
 * @param src_path The path to the source model file.
 * @returns The path to the copied model file.
 *
 * This function copies a model file from the source path to the build directory.
 * The copied file is stored in the `MODEL_OUTPUT_DIR` with the timestamp appended to the filename.
 * Logs a success or error message.
 */
const boost::filesystem::path copyModelWithTimestamp(const boost::filesystem::path &src_path);

/**
 * @brief Function to export a vector to a CSV file with ascending indexing.
 * @param vector The vector of double values to export.
 * @param csv_path The path to the CSV file to export the vector to.
 *
 * This function exports a vector of double values to a CSV file with ascending indexing.
 * The CSV file contains two columns: `Index` and `Value`, where `Value` is the `vector` value.
 */
void export_vector_to_csv(const std::vector<double> &vector, const std::string &csv_path);

/**
 * @brief Function to export a vector of data points to a CSV file.
 * @param vector The vector of data points to export.
 * @param csv_path The path to the CSV file to export the data points to.
 *
 * This function exports a vector of data points to a CSV file.
 * The CSV file contains two columns: `Index` and `Value`, where `Index` is the first value of the data point and `Value` is the second value.
 */
void export_data_to_csv(const std::vector<std::pair<double, double>> &vector, const std::string &csv_path);

/**
 * @brief Function to log the values from a vector.
 * @param data The vector of double values to log.
 * @param label The label to display before the values.
 *
 * This function logs the values from a vector with a label.
 * Format: `label{i}: data[i]`.
 */
void log_vector(const std::vector<double> &data, std::string label);

/**
 * @brief Function to export the results of a grid search to a CSV file.
 * @param results The vector of GridSearchResult objects to export.
 * @param csv_path The path to the CSV file to export the results to.
 * @param criteria_labels The labels for the output criteria values.
 *
 * This function exports the results of a grid search to a CSV file.
 * The CSV file contains the columns `Offset`, `Slope`, and each criteria label.
 * Each row corresponds to a GridSearchResult object.
 */
void export_grid_search_results_to_csv(const std::vector<GridSearchResult> &results, const std::string &csv_path, std::vector<std::string> criteria_labels);

#endif // INPUT_OUTPUT_H
