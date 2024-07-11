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

    // Check the fit quality
    double fit_quality = checkFitQuality(A, b, coeffs);
    
    Logger::debug("Quality of plane fit: R^2 = " + std::to_string(fit_quality) + ".");

    return std::make_tuple(a, b_coef, c);
}

// Function to assess the quality of a 2D plane fit to 3D data using the R^2 metric. Returns the R^2 value, which should be close to 1 for a good fit.
double StatisticalAnalysis::checkFitQuality(const Eigen::MatrixXd &A, const Eigen::VectorXd &b, const Eigen::VectorXd &coeffs)
    {
        Eigen::VectorXd residuals = b - A * coeffs;
        double rss = residuals.squaredNorm();
        double tss = (b.array() - b.mean()).matrix().squaredNorm();
        double r2 = 1 - (rss / tss);

        // R^2 should be close to 1 for a good fit
        return r2;
    }

// Function that converts a plane equation (ax + by + c = z) to a linear function (y = mx + d) at the intersection with the z = 0 plane. Format: (offset, slope)
std::tuple<double, double> StatisticalAnalysis::planeToLinearFunction(double a, double b, double c)
{
    if (b == 0)
    {
        throw std::runtime_error("Plane is parallel to the y-axis. No linear function representation exists.");
    }

    double m = -a / b;
    double d = -c / b;
    return std::make_tuple(d, m);
}

// Function to check if two lines intersect and find the intersection point. Input format: (offset, slope). Output format: (x, y). Returns std::nullopt if lines are parallel.
std::optional<std::pair<double, double>> StatisticalAnalysis::findIntersection(const std::pair<double, double> &line1, const std::pair<double, double> &line2)
{
    if (line1.second == line2.second)
    {
        return std::nullopt; // Lines are parallel and do not intersect
    }

    double x = (line2.first - line1.first) / (line1.second - line2.second);
    double y = line1.second * x + line1.first;

    return std::make_pair(x, y);
}