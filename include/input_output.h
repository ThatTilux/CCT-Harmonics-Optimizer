#ifndef INPUT_OUTPUT_H
#define INPUT_OUTPUT_H

#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <constants.h>
#include <chrono>
#include "harmonic_drive_parameter.h"
#include "model_handler.h"
#include "Logger.hh"
#include "grid_search_result.h"


void print_harmonic_drive_values(HarmonicDriveParameterMap &harmonic_drive_values);
char getch();
bool askUserToProceed();
double getUserInput(const std::string &prompt, double default_value);
boost::filesystem::path selectJsonFile();
int selectFromList(std::vector<std::string> options, std::string user_prompt);
void copyModelWithTimestamp(const boost::filesystem::path &src_path);
void export_vector_to_csv(const std::vector<double>& vector, const std::string& csv_path);
void export_data_to_csv(const std::vector<std::pair<double, double>>& vector, const std::string& csv_path);
void print_vector(const std::vector<double>& data, std::string label);
void export_grid_search_results_to_csv(const std::vector<GridSearchResult>& results, const std::string& csv_path);



#endif // INPUT_OUTPUT_H
