#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

// DIRECTORIES

/**
 * @brief The path to the data directory
 *
 * This is the directory where the input model JSON files are stored
 */
inline const std::string DATA_DIR_PATH = "../data";

/**
 * @brief The path to the model output directory
 *
 * This is the directory where the optimized models (inluding interim results) are stored
 */
inline const std::string MODEL_OUTPUT_DIR = "./optimized_cct/";

/**
 * @brief The path to the grid search output directory
 *
 * This is the directory where the output CSV files of all grid search runs are stored
 */
inline const std::string GRID_SEARCH_OUTPUT_DIR = "./grid_search/";

/**
 * @brief The path to the test data directory
 *
 * This is the directory where the model files used in tests are stored
 */
inline const std::string TEST_DATA_DIR = "../test_data/";

// LINEAR OPTIMIZER CONSTANTS

/**
 * @brief Default value for the linear optimizer's step size
 *
 * This is the default step size used in the linear optimizer. It is displayed to the user as the default value and also used in case none is entered
 */
inline const double LINEAR_OPTIMIZER_DEFAULT_MAX_VALUE = 0.1;

/**
 * @brief Maximum number of data points in the linear optimizer before the optimizer moves on to the next harmonic
 */
inline const int LINEAR_OPTIMIZER_MAX_DATAPOINTS = 10;


// GRID SEARCH OPTIMIZER CONSTANTS

/**
 * @brief Fallback value for the grid search optimizer's step size in case it is 0
 */
inline const double OPTIMIZER_FALLBACK_STEP = 0.000001;

/**
 * @brief Default search factor for the grid search optimizer for the main configuration.
 */
inline const double GRID_SEARCH_FACTOR = 0.05;

/**
 * @brief Fallback value for the grid search optimizer's offset/slope range center in case it is 0
 */
inline const double GRID_DRIVE_FALLBACK = 1e-7;

/**
 * @brief Default number of steps per grid search
 */
inline const int GRID_DEFAULT_STEPS = 49;

#endif // CONSTANTS_H
