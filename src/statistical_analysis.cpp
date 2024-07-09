#include "statistical_analysis.h"

// Function to compute the variance of y values with Bessel's correction
double StatisticalAnalysis::computeVariance(const std::vector<double> &y)
{
    double mean = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
    double variance = 0;
    for (const auto &value : y)
    {
        variance += (value - mean) * (value - mean);
    }
    variance /= (y.size() - 1); // Bessel's correction
    return variance;
}


// Function to perform linear regression and return the slope and intercept
std::pair<double, double> StatisticalAnalysis::linearRegression(const std::vector<std::pair<double, double>> &points)
{
    size_t n = points.size();
    if (n < 2)
    {
        throw std::runtime_error("Not enough points for linear regression.");
    }

    double sum_x = 0, sum_y = 0, sum_xx = 0, sum_xy = 0;
    for (const auto &point : points)
    {
        sum_x += point.first;
        sum_y += point.second;
        sum_xx += point.first * point.first;
        sum_xy += point.first * point.second;
    }

    double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
    double intercept = (sum_y - slope * sum_x) / n;

    return {slope, intercept};
}

// Function to fit a 2D plane in the offset,slope,criterion space and return the coefficients of the plane
std::tuple<double, double, double> StatisticalAnalysis::fitPlaneToData(const std::vector<GridSearchResult> &results, size_t criterion_index)
    {
        if (results.empty())
        {
            throw std::runtime_error("No data points provided.");
        }

        size_t n = results.size();
        Eigen::MatrixXd A(n, 3);
        Eigen::VectorXd b(n);

        for (size_t i = 0; i < n; ++i)
        {
            A(i, 0) = results[i].offset;
            A(i, 1) = results[i].slope;
            A(i, 2) = 1.0; // for the intercept
            if (results[i].criteria_values.size() <= criterion_index)
            {
                throw std::runtime_error("Criterion index out of range.");
            }
            b(i) = results[i].criteria_values[criterion_index];
        }

        // Solve for the coefficients using least squares
        Eigen::VectorXd coeffs = A.colPivHouseholderQr().solve(b);

        double a = coeffs(0);
        double b_coef = coeffs(1);
        double c = coeffs(2);

        return std::make_tuple(a, b_coef, c);
    }