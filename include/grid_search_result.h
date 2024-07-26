#ifndef GRID_SEARCH_RESULT_H
#define GRID_SEARCH_RESULT_H

#include <vector>

// Struct for storing the results of one grid search iteration. Contains the offset, slope and the criteria values.
struct GridSearchResult
{
    double offset;
    double slope;
    std::vector<double> criteria_values;
};

#endif // GRID_SEARCH_RESULT_H
