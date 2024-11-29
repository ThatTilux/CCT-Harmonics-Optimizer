#ifndef GRID_SEARCH_RESULT_H
#define GRID_SEARCH_RESULT_H

#include <vector>

/**
 * @brief Struct for storing the results of one grid search iteration. 
 * 
 * This struct contains the offset and slope parameter used for the iteration and a vector of the output criteria values.
 */
struct GridSearchResult
{
    double offset;
    double slope;
    std::vector<double> criteria_values;
};

#endif // GRID_SEARCH_RESULT_H
