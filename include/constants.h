#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

// DIRECTORIES

inline const std::string DATA_DIR_PATH = "../data";
inline const std::string MODEL_OUTPUT_DIR = "./optimized_cct/";
inline const std::string TEST_DATA_DIR = "../test_data/";


// BN OPTIMIZER CONSTANTS

// this will be displayed to the user as the default value and also used in case none is entered
inline const double DEFAULT_MAX_BN_VALUE = 0.1;
// max datapoints before the optimizer moves on to the next harmonic
inline const int OPTIMIZER_MAX_DATAPOINTS = 10;


// GRID SEARCH OPTIMIZER CONSTANTS

// when the step size in the optimizer is 0, this value is used instead
inline const double OPTIMIZER_DEFAULT_STEP = 0.000001;
// Initial threshold for all bn values to start the next phase of the grid search optimizer
inline const double GRID_BN_THRESHOLD = 30;
// Initial factor to increase the offset and slope ranges in the grid search optimizer
inline const double GRID_SEARCH_FACTOR = 0.05;
// Fallback value for an offset/slope range if the current one is 0 in the grid search optimizer
inline const double GRID_DRIVE_FALLBACK = 1e-7;
// Minimum number of steps per grid search
inline const int GRID_MIN_STEPS = 64;
// Time budget in minutes for one grid search in the grid search optimizer
inline const double TIME_BUDGET_GRID_SEARCH = 1;



#endif // CONSTANTS_H
