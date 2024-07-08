#ifndef STATISTICAL_ANALYSIS_H
#define STATISTICAL_ANALYSIS_H

#include <vector>
#include <numeric>
#include <cmath>
#include <stdexcept>

class StatisticalAnalysis {
public:
    static double computeVariance(const std::vector<double> &y);
    static std::pair<double, double> linearRegression(const std::vector<std::pair<double, double>> &points);
};

#endif // STATISTICAL_ANALYSIS_H
