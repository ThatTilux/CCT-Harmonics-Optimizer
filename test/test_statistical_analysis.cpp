#include "gtest/gtest.h"
#include "statistical_analysis.h"
#include "grid_search_result.h"
#include <vector>
#include <utility>

class StatisticalAnalysisTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No specific setup required
    }

    void TearDown() override {
        // No specific teardown required
    }
};

TEST_F(StatisticalAnalysisTest, ComputeVariance) {
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    double variance = StatisticalAnalysis::computeVariance(data);
    ASSERT_NEAR(variance, 2.5, 1e-6);
}


TEST_F(StatisticalAnalysisTest, ComputeNRMSE) {
    std::vector<std::pair<double, double>> points = {{1.0, 2.1}, {2.0, 4.1}, {3.0, 6.1}};
    double nrmse = StatisticalAnalysis::computeNRMSE(points, 2.0, 0.1);
    ASSERT_NEAR(nrmse, 0.0, 1e-6);
    double nrmse_2 = StatisticalAnalysis::computeNRMSE(points, 3.0, 0.1);
    ASSERT_NEAR(nrmse_2, 0.5400617248, 1e-6);
}

TEST_F(StatisticalAnalysisTest, LinearRegression) {
    std::vector<std::pair<double, double>> points = {{1.0, 2.0}, {2.0, 4.0}, {3.0, 6.0}, {4.0, 8.0}, {5.0, 10.0}};
    auto [slope, intercept] = StatisticalAnalysis::linearRegression(points);
    ASSERT_NEAR(slope, 2.0, 1e-6);
    ASSERT_NEAR(intercept, 0.0, 1e-6);
}

TEST_F(StatisticalAnalysisTest, FitPlaneToData) {
    std::vector<GridSearchResult> results = {
        {0, 0, {0}},
        {0, 1, {0.5}},
        {0, 2, {1}},
        {1, 0, {0}},
        {1, 1, {0.5}},
        {1, 2, {1}},
        {2, 0, {0}},
        {2, 1, {0.5}},
        {2, 2, {1}},
    };
    auto [a, b, c] = StatisticalAnalysis::fitPlaneToData(results, 0);
    ASSERT_NEAR(a, 0.0, 1e-6);
    ASSERT_NEAR(b, 0.5, 1e-6);
    ASSERT_NEAR(c, 0, 1e-6);

    std::vector<GridSearchResult> new_results = {
        {0, 0, {0}},
        {-1, 0, {-0.5}},
        {-2, 0, {-1}},
        {0, 5, {0}},
        {-1, 5, {-0.5}},
        {-2, 5, {-1}},
        {0, 10, {0}},
        {-1, 10, {-0.5}},
        {-2, 10, {-1}},
    };
    auto [new_a, new_b, new_c] = StatisticalAnalysis::fitPlaneToData(new_results, 0);
    ASSERT_NEAR(new_a, 0.5, 1e-6);
    ASSERT_NEAR(new_b, 0.0, 1e-6);
    ASSERT_NEAR(new_c, 0.0, 1e-6);
}

TEST_F(StatisticalAnalysisTest, PlaneToLinearFunction) {
    double a = 1.0, b = 2.0, c = 3.0;
    auto [d, m] = StatisticalAnalysis::planeToLinearFunction(a, b, c);
    ASSERT_NEAR(d, -1.5, 1e-6);
    ASSERT_NEAR(m, -0.5, 1e-6);
}

TEST_F(StatisticalAnalysisTest, FindIntersection) {
    std::pair<double, double> line1 = {1.0, 2.0};
    std::pair<double, double> line2 = {0.0, -1.0};
    auto intersection = StatisticalAnalysis::findIntersection(line1, line2);
    ASSERT_TRUE(intersection.has_value());
    ASSERT_NEAR(intersection->first, -0.3333333333, 1e-6);
    ASSERT_NEAR(intersection->second, 0.3333333333, 1e-6);
}

TEST_F(StatisticalAnalysisTest, ClosestPointOnLine) {
    std::pair<double, double> linear_function = {1.0, 2.0};
    std::pair<double, double> point = {2.0, 2.0};
    auto closest_point = StatisticalAnalysis::closest_point_on_line(linear_function, point);
    ASSERT_NEAR(closest_point.first, 0.8, 1e-6);
    ASSERT_NEAR(closest_point.second, 2.6, 1e-6);
}
