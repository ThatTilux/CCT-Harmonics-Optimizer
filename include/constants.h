#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

// DIRECTORIES

inline const std::string DATA_DIR_PATH = "../data";
inline const std::string MODEL_OUTPUT_DIR = "./optimized_cct/";
inline const std::string GRID_SEARCH_OUTPUT_DIR = "./grid_search/";
inline const std::string TEST_DATA_DIR = "../test_data/";

// LINEAR OPTIMIZER CONSTANTS

// this will be displayed to the user as the default value and also used in case none is entered
inline const double LINEAR_OPTIMIZER_DEFAULT_MAX_VALUE = 0.1;
// max value for an optimizer
inline const double AN_OPTIMIZER_MAX_VALUE = 1;
// max datapoints before the optimizer moves on to the next harmonic
inline const int LINEAR_OPTIMIZER_MAX_DATAPOINTS = 10;

// GRID SEARCH OPTIMIZER CONSTANTS

// when the step size in the optimizer is 0, this value is used instead
inline const double OPTIMIZER_DEFAULT_STEP = 0.000001;
// Default initial factor to increase the offset and slope ranges in the grid search optimizer
inline const double GRID_SEARCH_FACTOR = 0.05;
// Fallback value for an offset/slope range if the current one is 0 in the grid search optimizer
inline const double GRID_DRIVE_FALLBACK = 1e-7;
// Default number of steps per grid search
inline const int GRID_DEFAULT_STEPS = 49;

#endif // CONSTANTS_H
