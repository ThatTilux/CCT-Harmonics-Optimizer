#ifndef STATISTICAL_ANALYSIS_H
#define STATISTICAL_ANALYSIS_H

#include <vector>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <Eigen/Dense>
#include <optional>
#include <Logger.hh>
#include "grid_search_result.h"

class StatisticalAnalysis {
public:
    static double computeVariance(const std::vector<double> &y);
    static std::pair<double, double> linearRegression(const std::vector<std::pair<double, double>> &points);
    static std::tuple<double, double, double> fitPlaneToData(const std::vector<GridSearchResult> &results, size_t criterion_index);
    static std::tuple<double, double> planeToLinearFunction(double a, double b, double c);
    static std::optional<std::pair<double, double>> findIntersection(const std::pair<double, double>& line1, const std::pair<double, double>& line2);

private:
    static double checkFitQuality(const Eigen::MatrixXd &A, const Eigen::VectorXd &b, const Eigen::VectorXd &coeffs);
};

#endif // STATISTICAL_ANALYSIS_H
