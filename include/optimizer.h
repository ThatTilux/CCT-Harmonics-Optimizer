#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <vector>
#include "harmonic_drive_parameter.h"
#include "harmonics_calculator.h"
#include "model_handler.h"
#include "harmonics_handler.h"
#include <boost/filesystem.hpp>
#include "objective_function.h"
#include "constants.h"
#include <iostream>
#include <cmath>

void optimize(HarmonicsCalculator &calculator, ModelHandler &model_handler, std::vector<double> &current_bn_values, HarmonicDriveParameterMap &harmonic_drive_values, double max_harmonic_value, const boost::filesystem::path &temp_json_file_path, const bool disable_logging = false);

#endif // OPTIMIZER_H