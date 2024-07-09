#include "constants.h"

// DIRECTORIES

const std::string DATA_DIR_PATH = "../data";
const std::string MODEL_OUTPUT_DIR = "./optimized_cct/";
const std::string TEST_DATA_DIR = "../test_data/";

// OPTIMIZER CONSTANTS

// this will be dispslayed to the user as the default value and also used incase none is entered
const double DEFAULT_MAX_BN_VALUE = 0.1;
// max bn value used for the chiSquare optimizer
const double MAX_BN_CHISQUARE = 1;
// max datapoints before the optimizer moves on to the next harmonic
const int OPTIMIZER_MAX_DATAPOINTS = 7;
// weight of the chiSquare component in BO objective function
const double CHISQUARE_WEIGHT = 0.01;
// TODO let the user decide this
const bool USE_BN_OPTIMIZER_IN_CHISQUARED = true;
// when the step size in the optimizer is 0, this value is used instead
const double OPTIMIZER_DEFAULT_STEP = 0.000001;
// threshold for all bn values to start the final phase of the grid search optimizer
const double GRID_BN_THRESHOLD = 30;
// Time budget in minutes for one grid search in the grid search optimizer
const double TIME_BUDGET_GRID_SEARCH = 1;

// MODEL CONSTANTS

// max offset value allowed to be set in the model handler
const double MAX_OFFSET = 0.05; // 5mm
// max slope value allowed to be set in the model handler
const double MAX_SLOPE = 0.0001; // 1e-04 m/coil

// temp constants - TODO infer this from the model 
const double MAG_START_POS = 167; //167mm
const double MAG_END_POS = 909; //909m
const int MAIN_COMPONENT = 2; //B2