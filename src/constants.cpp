#include "constants.h"

const std::string DATA_DIR_PATH = "../data";
const std::string MODEL_OUTPUT_DIR = "./optimized_cct/";
const std::string TEST_DATA_DIR = "../test_data/";
// max datapoints before the optimizer moves on to the next harmonic
const int OPTIMIZER_MAX_DATAPOINTS = 10;
// weight of the chiSquare component in BO objective function
const double CHISQUARE_WEIGHT = 0.01;
