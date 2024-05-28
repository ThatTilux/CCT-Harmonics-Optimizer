#include "optimizer.h"
#include "constants.h"
#include <iostream>
#include <cmath>

// Function to perform linear regression and find the root
double linearRegression(const std::vector<std::pair<double, double>> &points)
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
    double optimized_value = -intercept / slope;

    return optimized_value;
}

// function to optimize all harmonic drive values so the corresponding (absolute) bn values are all within the max_harmonic_value 
void optimize(HarmonicsCalculator &calculator, ModelHandler &model_handler, std::vector<double> &current_bn_values, std::vector<std::pair<int, double>> &harmonic_drive_values, double max_harmonic_value, const boost::filesystem::path &temp_json_file_path)
{
    bool all_within_margin;
    // get the current bn values
    current_bn_values = calculator.reload_and_compute_bn(temp_json_file_path);
    // optimize as long as not all bn values are within the margin
    do
    {
        all_within_margin = true;

        // optimize each harmonic drive value
        for (auto &harmonic : harmonic_drive_values)
        {
            // get current values
            std::string name = "B" + std::to_string(harmonic.first);
            double current_value = harmonic.second;
            double current_bn = current_bn_values[harmonic.first - 1];

            // if value is not optimized yet, do it
            if (std::abs(current_bn) > max_harmonic_value)
            {
                all_within_margin = false;

                // print info
                std::cout << "Now optimizing harmonic B" << harmonic.first << ". Current drive value is " << current_value << " with bn " << current_bn << std::endl;

                // collect all datapoints (x=drive value, y=bn) for a regression
                std::vector<std::pair<double, double>> data_points;
                data_points.emplace_back(current_value, current_bn);

                // while the harmonic is not optimized yet
                while (true)
                {
                    // Take a small step in the scaling/slope value
                    double step = 0.01 * current_value;
                    // to get a different datapoint when the drive value was 0 
                    if (step == 0) step = 0.000001;
                    model_handler.setHarmonicDriveValue(name, current_value + step);

                    // Compute the new bn values
                    std::vector<double> new_bn_values = calculator.reload_and_compute_bn(temp_json_file_path);
                    double new_bn = new_bn_values[harmonic.first - 1];

                    // Add the new data point
                    data_points.emplace_back(current_value + step, new_bn);

                    // Perform linear regression to find the root
                    double optimized_value = linearRegression(data_points);

                    // Set the optimized value and recompute bn
                    model_handler.setHarmonicDriveValue(name, optimized_value);
                    std::vector<double> optimized_bn_values = calculator.reload_and_compute_bn(temp_json_file_path);
                    double optimized_bn = optimized_bn_values[harmonic.first - 1];

                    if (std::abs(optimized_bn) <= max_harmonic_value || data_points.size() >= OPTIMIZER_MAX_DATAPOINTS)
                    {
                        // set the new values for the next iteration
                        current_bn_values = optimized_bn_values;
                        harmonic.second = optimized_value;
                        // check if the optimizer stopped because of the max datapoints limit
                        if (data_points.size() >= OPTIMIZER_MAX_DATAPOINTS){
                            std::cout << "Optimizer moved on from " << name << " after " << OPTIMIZER_MAX_DATAPOINTS << " datapoints. This harmonic will be optimized in the next iteration." << std::endl;
                        } else {
                            // print new harmonic drive value to console console
                            std::cout << "Optimized " << name << " with drive value " << optimized_value << " and bn value: " << optimized_bn << std::endl;
                        }
                        break;
                    }

                    current_value = optimized_value;
                    current_bn = optimized_bn;
                }
            }
        }
    } while (!all_within_margin);
}
