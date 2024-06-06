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
    bool all_within_margin;
    // handler for handling the results of the harmonics calculation
    HarmonicsHandler harmonics_handler;
    // get the current bn values
    calculator.reload_and_calc(temp_json_file_path, harmonics_handler);
    current_bn_values = harmonics_handler.get_bn();

    // TODO TEMP START --------------------

    harmonics_handler.export_Bns_to_csv("./Bn");

    for (int i = 1; i <= 10; i++)
    {
        std::cout << "B" << i << ": " << std::endl;
        double chi_squared = chiSquared(harmonics_handler, i);
        std::cout << "ChiSquared" << ": " << chi_squared << std::endl;
    }

    // TODO TEMP END --------------------

    // optimize as long as not all bn values are within the margin
    do
    {
        all_within_margin = true;

        // optimize each harmonic drive value
        for (auto &harmonic : harmonic_drive_values)
        {
            // get current values
            std::string name = harmonic.first;
            // the component number, e.g. 5 for B5
            int component = std::stoi(name.substr(1));
            // either the current slope or constant value, depending on the harmonic type
            double current_drive_value;
            // type of drive parameter (slope or constant)
            HarmonicDriveParameterType drive_type;
            if (harmonic.second.isConstant()){
                current_drive_value = harmonic.second.getConstant();
                drive_type = HarmonicDriveParameterType::Constant;
            } else if (harmonic.second.isSlope()){
                current_drive_value = harmonic.second.getSlope();
                drive_type = HarmonicDriveParameterType::Slope;
            } else {
                throw std::logic_error("Optimizer only optimizes slope/constant parameters");
            }

            double current_bn = current_bn_values[component - 1];

            // if value is not optimized yet, do it
            if (std::abs(current_bn) > max_harmonic_value)
            {
                all_within_margin = false;

                // print info
                std::cout << "Now optimizing harmonic " << harmonic.first << ". Current drive value is " << current_drive_value << " with bn " << current_bn << std::endl;

                // collect all datapoints (x=drive value, y=bn) for a regression
                std::vector<std::pair<double, double>> data_points;
                data_points.emplace_back(current_drive_value, current_bn);

                // while the harmonic is not optimized yet
                while (true)
                {
                    // Take a small step in the scaling/slope value
                    double step = 0.01 * current_drive_value;
                    // to get a different datapoint when the drive value was 0
                    if (step == 0)
                        step = 0.000001;
                    model_handler.setHarmonicDriveValue(name, HarmonicDriveParameters(current_drive_value + step, drive_type));

                    // Compute the new bn values
                    // get the current bn values
                    calculator.reload_and_calc(temp_json_file_path, harmonics_handler);
                    std::vector<double> new_bn_values = harmonics_handler.get_bn();
                    double new_bn = new_bn_values[component - 1];

                    // Add the new data point
                    data_points.emplace_back(current_drive_value + step, new_bn);

                    // Perform linear regression to find the root
                    double optimized_value = fitLinearGetRoot(data_points);

                    // Set the optimized value and recompute bn
                    model_handler.setHarmonicDriveValue(name, HarmonicDriveParameters(optimized_value, drive_type));
                    calculator.reload_and_calc(temp_json_file_path, harmonics_handler);
                    std::vector<double> optimized_bn_values = harmonics_handler.get_bn();

                    // get the bn value for the component currently being optimized
                    double optimized_bn = optimized_bn_values[component - 1];

                    // check if the optimization was successfull or if it has to be aborted
                    if (std::abs(optimized_bn) <= max_harmonic_value || data_points.size() >= OPTIMIZER_MAX_DATAPOINTS)
                    {
                        // set the new values for the next iteration
                        current_bn_values = optimized_bn_values;
                        harmonic.second.setValue(optimized_value, drive_type);
                        // check if the optimizer stopped because of the max datapoints limit
                        if (data_points.size() >= OPTIMIZER_MAX_DATAPOINTS)
                        {
                            std::cout << "Optimizer moved on from " << name << " after " << OPTIMIZER_MAX_DATAPOINTS << " datapoints. This harmonic will be optimized in the next iteration." << std::endl;
                        }
                        else
                        {
                            // print new harmonic drive value to console console
                            std::cout << "Optimized " << name << " with drive value " << optimized_value << " and bn value: " << optimized_bn << std::endl;
                        }
                        break;
                    }

                    current_drive_value = optimized_value;
                    current_bn = optimized_bn;
                }
            }
        }
    } while (!all_within_margin);
}
