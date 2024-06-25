#include "optimizer.h"

// Function to fit a linear function to data and extract the root
double fitLinearGetRoot(const std::vector<std::pair<double, double>> &points)
{
    auto [slope, intercept] = linearRegression(points);
    double root = -intercept / slope;

    return root;
}

// function to optimize all harmonic drive values (only constant/slope params) so the corresponding (absolute) bn values are all within the max_harmonic_value
void optimize(HarmonicsCalculator &calculator, ModelHandler &model_handler, std::vector<double> &current_bn_values, HarmonicDriveParameterMap &harmonic_drive_values, double max_harmonic_value, const boost::filesystem::path &temp_json_file_path)
{
    Logger::log_timestamp("Starting optimizer");

    bool all_within_margin;
    // handler for handling the results of the harmonics calculation
    HarmonicsHandler harmonics_handler;
    // get the current bn values
    calculator.reload_and_calc(temp_json_file_path, harmonics_handler);
    current_bn_values = harmonics_handler.get_bn();

    // optimize as long as not all bn values are within the margin
    do
    {
        all_within_margin = true;

        // optimize each harmonic drive value
        for (auto &harmonic : harmonic_drive_values)
        {
            Logger::log_timestamp("Optimizing next harmonic");

            // get current values
            std::string name = harmonic.first;
            // the component number, e.g. 5 for B5
            int component = std::stoi(name.substr(1));
            // either the current slope or constant value, depending on the harmonic type
            double current_drive_value;
            // type of drive parameter (slope or constant)
            HarmonicDriveParameterType drive_type;
            if (harmonic.second.isConstant())
            {
                current_drive_value = harmonic.second.getConstant();
                drive_type = HarmonicDriveParameterType::Constant;
            }
            else if (harmonic.second.isSlope())
            {
                current_drive_value = harmonic.second.getSlope();
                drive_type = HarmonicDriveParameterType::Slope;
            }
            else
            {
                throw std::logic_error("Optimizer only optimizes slope/constant parameters");
            }

            double current_bn = current_bn_values[component - 1];

            // if value is not optimized yet, do it
            if (std::abs(current_bn) > max_harmonic_value)
            {
                all_within_margin = false;

                // log info
                Logger::info("Now optimizing harmonic " + harmonic.first + ". Current drive value is " + std::to_string(current_drive_value) + " with bn " + std::to_string(current_bn));

                // collect all datapoints (x=drive value, y=bn) for a regression
                std::vector<std::pair<double, double>> data_points;
                data_points.emplace_back(current_drive_value, current_bn);

                // while the harmonic is not optimized yet
                while (true)
                {
                    Logger::log_timestamp("Starting next iteration of optimizing one harmonic.");
                    // Take a small step in the scaling/slope value
                    double step = 0.01 * current_drive_value;
                    // to get a different datapoint when the drive value was 0
                    if (step == 0)
                        step = OPTIMIZER_DEFAULT_STEP; 
                    model_handler.setHarmonicDriveValue(name, HarmonicDriveParameters(current_drive_value + step, drive_type));
                    Logger::log_timestamp("New drive value set.");

                    // Compute the new bn values
                    // get the current bn values
                    calculator.reload_and_calc(temp_json_file_path, harmonics_handler);
                    Logger::log_timestamp("Harmonics recalculated.");
                    std::vector<double> new_bn_values = harmonics_handler.get_bn();
                    double new_bn = new_bn_values[component - 1];
                    Logger::log_timestamp("New bn values retrieved.");

                    // Add the new data point
                    data_points.emplace_back(current_drive_value + step, new_bn);

                    // Perform linear regression to find the root
                    double optimized_value = fitLinearGetRoot(data_points);
                    Logger::log_timestamp("Linear regression done.");

                    // Set the optimized value and recompute bn
                    model_handler.setHarmonicDriveValue(name, HarmonicDriveParameters(optimized_value, drive_type));
                    Logger::log_timestamp("New drive value set once again.");
                    calculator.reload_and_calc(temp_json_file_path, harmonics_handler);
                    Logger::log_timestamp("Harmonics recalculated once again.");
                    std::vector<double> optimized_bn_values = harmonics_handler.get_bn();
                    Logger::log_timestamp("New bn values retrieved once again.");

                    // get the bn value for the component currently being optimized
                    double optimized_bn = optimized_bn_values[component - 1];

                    // check if the optimization was successful or if it has to be aborted
                    if (std::abs(optimized_bn) <= max_harmonic_value || data_points.size() >= OPTIMIZER_MAX_DATAPOINTS)
                    {
                        // set the new values for the next iteration
                        current_bn_values = optimized_bn_values;
                        harmonic.second.setValue(optimized_value, drive_type);
                        // check if the optimizer stopped because of the max datapoints limit
                        if (data_points.size() >= OPTIMIZER_MAX_DATAPOINTS)
                        {
                            Logger::info("Optimizer moved on from " + name + " after " + std::to_string(OPTIMIZER_MAX_DATAPOINTS) + " datapoints. This harmonic will be optimized in the next iteration.");
                        }
                        else
                        {
                            // log new harmonic drive value
                            Logger::info("Optimized " + name + " with drive value " + std::to_string(optimized_value) + " and bn value: " + std::to_string(optimized_bn));
                        }
                        break;
                    }

                    current_drive_value = optimized_value;
                    current_bn = optimized_bn;
                }
            }
            Logger::log_timestamp("Harmonic optimized");
        }
    } while (!all_within_margin);

    Logger::log_timestamp("Optimizer finished");
}
