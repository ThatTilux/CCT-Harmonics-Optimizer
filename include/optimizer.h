#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <vector>
#include "harmonics_calculator.h"
#include "model_handler.h"
#include "harmonics_handler.h"
#include <boost/filesystem.hpp>

void optimize(HarmonicsCalculator &calculator, ModelHandler &model_handler, std::vector<double> &current_bn_values, std::vector<std::pair<int, double>> &harmonic_drive_values, double max_harmonic_value, const boost::filesystem::path &temp_json_file_path);
double linearRegression(const std::vector<std::pair<double, double>> &points);

#endif // OPTIMIZER_H
