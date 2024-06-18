#include "constants.h"

const std::string DATA_DIR_PATH = "../data";
const std::string MODEL_OUTPUT_DIR = "./optimized_cct/";
const std::string TEST_DATA_DIR = "../test_data/";
// this will be dispslayed to the user as the default value and also used incase none is entered
const double DEFAULT_MAX_BN_VALUE = 0.1;
// max datapoints before the optimizer moves on to the next harmonic
const int OPTIMIZER_MAX_DATAPOINTS = 10;
// weight of the chiSquare component in BO objective function
const double CHISQUARE_WEIGHT = 0.01;
// TODO let the user decide this
const bool USE_BN_OPTIMIZER_IN_CHISQUARED = true;
