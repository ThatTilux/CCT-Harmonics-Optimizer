#include "constants.h"

const std::string DATA_DIR_PATH = "../data";
const std::string MODEL_OUTPUT_DIR = "./optimized_cct/";
const std::string TEST_DATA_DIR = "../test_data/";
// this will be dispslayed to the user as the default value and also used incase none is entered
const double DEFAULT_MAX_BN_VALUE = 0.1;
// max bn value used for the chiSquare optimizer
const double MAX_BN_CHISQUARE = 1;
// max datapoints before the optimizer moves on to the next harmonic
const int OPTIMIZER_MAX_DATAPOINTS = 5;
// weight of the chiSquare component in BO objective function
const double CHISQUARE_WEIGHT = 0.01;
// TODO let the user decide this
const bool USE_BN_OPTIMIZER_IN_CHISQUARED = true;
// when the step size in the optimizer is 0, this value is used instead
const double OPTIMIZER_DEFAULT_STEP = 0.000001;
