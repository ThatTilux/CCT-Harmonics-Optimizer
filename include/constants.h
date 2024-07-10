#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

extern const std::string DATA_DIR_PATH;
extern const std::string MODEL_OUTPUT_DIR;
extern const int OPTIMIZER_MAX_DATAPOINTS;
extern const std::string TEST_DATA_DIR;
extern const double CHISQUARE_WEIGHT;
extern const bool USE_BN_OPTIMIZER_IN_CHISQUARED;
extern const double DEFAULT_MAX_BN_VALUE;
extern const double MAX_BN_CHISQUARE;
extern const double OPTIMIZER_DEFAULT_STEP;
extern const double GRID_BN_THRESHOLD;
extern const double GRID_SEARCH_FACTOR;
extern const double TIME_BUDGET_GRID_SEARCH;
extern const double MAG_START_POS;
extern const double MAG_END_POS;
extern const int MAIN_COMPONENT;

#endif // CONSTANTS_H
