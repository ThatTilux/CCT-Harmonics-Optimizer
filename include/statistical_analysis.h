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

/**
 * @brief A utility class for statistical computations and data fitting.
 * 
 * The StatisticalAnalysis class provides static methods for various statistical calculations,
 * such as computing variance, performing linear regression, and evaluating fit quality. It is
 * designed for use in optimization and grid search processes.
 */
class StatisticalAnalysis
{
public:
    /**
     * @brief Function to compute the variance of y values with Bessel's correction
     * @param y The vector of double values to compute the variance for
     * @returns The variance of the y values
     */
    static double computeVariance(const std::vector<double> &y);

    /**
     * @brief Function to compute the Normalized Root Mean Square Error (NRMSE) of a fitted linear function
     * @param points The vector of data points to compute the NRMSE for
     * @param slope The slope of the linear function
     * @param intercept The intercept of the linear function
     * @returns The NRMSE of the fitted linear function with respect to the data points
     */
    static double computeNRMSE(const std::vector<std::pair<double, double>> &points, double slope, double intercept);

    /**
     * @brief Function to perform linear regression and return the slope and intercept
     * @param points The vector of data points to perform the linear regression on
     * @returns The slope and intercept of the linear regression
     */
    static std::pair<double, double> linearRegression(const std::vector<std::pair<double, double>> &points);

    /**
     * @brief Function to fit a 2D plane in the [offset,slope,criterion] space and return the coefficients of the plane
     * @param results The vector of GridSearchResult objects to fit the plane to
     * @param criterion_index The index of the criterion to fit the plane to
     * @returns The coefficients of the plane in the form (a, b, c) for ax + by + c = z
     *
     * This function fits a 2D plane to the [offset, slope, criterion] space of the grid search results.
     * The criterion index determines which criterion within GridSearchResult objects to use for the third dimension of the space.
     * The plane is fitted using a least squares approach and the coefficients are returned.
     * The quality of the fit is assessed and logged using the R^2 metric.
     */
    static std::tuple<double, double, double> fitPlaneToData(const std::vector<GridSearchResult> &results, size_t criterion_index);

    /**
     * @brief Function to convert a plane equation (ax + by + c = z) to a linear function (y = mx + d) at the intersection with the z = 0 plane
     * @param a The coefficient of x in the plane equation
     * @param b The coefficient of y in the plane equation
     * @param c The constant term in the plane equation
     * @returns The linear function representation of the plane at the intersection with the z = 0 plane in the form (m, d) for y = mx + d.
     * @throws std::runtime_error If the plane is parallel to the y-axis
     *
     * This function converts a plane equation (ax + by + c = z) to a linear function (y = mx + d) at the intersection with the z = 0 plane.
     * The linear function is returned in the form (m,d).
     */
    static std::tuple<double, double> planeToLinearFunction(double a, double b, double c);

    /**
     * @brief Function to find the intersection of two lines in the form y = mx + d
     * @param line1 The coefficients of the first line in the form (m, d)
     * @param line2 The coefficients of the second line in the form (m, d)
     * @returns The intersection point of the two lines in the form (x, y)
     * @returns An empty optional if the lines are parallel
     *
     * This function finds the intersection of two lines in the form y = mx + d.
     * The intersection point is returned in the form (x, y).
     * If the lines are parallel, an empty optional is returned.
     */
    static std::optional<std::pair<double, double>> findIntersection(const std::pair<double, double> &line1, const std::pair<double, double> &line2);

    /**
     * @brief Function to find the closest point on a line to a given point
     * @param linear_function The coefficients of the line in the form (m, d) for y = mx + d
     * @param point The coordinates of the point in the form (x, y)
     * @returns The coordinates of the closest point on the line to the given point in the form (x, y)
     *
     * This function finds the closest point on a line to a given point.
     * The coordinates of the closest point are returned.
     */
    static std::pair<double, double> closest_point_on_line(const std::pair<double, double> &linear_function, const std::pair<double, double> &point);

private:
    /**
     * @brief Function to assess the quality of a 2D plane fit to 3D data using the R^2 metric
     * @param A The matrix of data points in the form [offset, slope, 1]
     * @param b The vector of criterion values
     * @param coeffs The vector of coefficients of the plane in the form [a, b, c] for ax + by + c = z
     * @returns The R^2 value of the fit
     *
     * This function assesses the quality of a 2D plane fit to 3D data using the R^2 metric.
     * The R^2 value should be close to 1 for a good fit.
     */
    static double checkFitQuality(const Eigen::MatrixXd &A, const Eigen::VectorXd &b, const Eigen::VectorXd &coeffs);
};

#endif // STATISTICAL_ANALYSIS_H
