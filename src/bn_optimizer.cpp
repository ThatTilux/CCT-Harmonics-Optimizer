#include "bn_optimizer.h"

// Default constructor
BnOptimizer::BnOptimizer(bool disable_logging) : BnOptimizer(initModel(), getMaxHarmonicValue(), disable_logging)
{
}

// Constructor without any user interaction.
BnOptimizer::BnOptimizer(ModelHandler &model_handler, double max_harmonic_value, bool disable_logging) : AbstractOptimizer(disable_logging)
{
    // setup
    model_handler_ = model_handler;
    initCalcultor();
    harmonic_drive_values_ = initHarmonicDrives();
    max_harmonic_value_ = max_harmonic_value;
    disable_logging_ = disable_logging;
}


// Function to log the results from the optimizer
void BnOptimizer::logResults()
{
    Logger::info("=== All harmonics have been optimized ===");
    Logger::info("User-specified margin was: " + std::to_string(max_harmonic_value_));
    print_harmonic_drive_values(harmonic_drive_values_);
    print_vector(current_bn_values_, "bn");
}



// Function to get the bn values after optimization
std::vector<double>& BnOptimizer::getResults(){
    return current_bn_values_;
}

// Function to fit a linear function to data and extract the root
double BnOptimizer::fitLinearGetRoot(const std::vector<std::pair<double, double>> &points)
{
    auto [slope, intercept] = StatisticalAnalysis::linearRegression(points);
    double root = -intercept / slope;

    return root;
}

// Function to optimize all harmonic drive values (only constant/slope params) so the corresponding (absolute) bn values are all within the max_harmonic_value
void BnOptimizer::optimize()
{
    Logger::info("== Starting bn optimizer ==");

    const boost::filesystem::path temp_json_file_path = model_handler_.getTempJsonPath();

    // flag to check if all bn values are within the margin
    bool all_within_margin;

    // handler for handling the results of the harmonics calculation
    HarmonicsHandler harmonics_handler;

    // get the current bn values
    calculator_.reload_and_calc(temp_json_file_path, harmonics_handler, disable_logging_);
    current_bn_values_ = harmonics_handler.get_bn();

    // optimize as long as not all bn values are within the margin
    do
    {
        all_within_margin = true;

        // optimize each harmonic drive value
        for (auto &harmonic : harmonic_drive_values_)
        {

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

            double current_bn = current_bn_values_[component - 1];

            // if value is not optimized yet, do it
            if (std::abs(current_bn) > max_harmonic_value_)
            {
                all_within_margin = false;

                // log info
                Logger::info("Now optimizing harmonic " + harmonic.first + ". Current drive value is " + std::to_string(current_drive_value) + " with bn " + std::to_string(current_bn));

                // collect all datapoints (x=drive value, y=bn) for a regression
                std::vector<std::pair<double, double>> data_points;
                data_points.emplace_back(current_drive_value, current_bn);

                // change a small step to get the second data point for the linear regression

                // Take a small step in the scaling/slope value
                double step = 0.01 * current_drive_value;
                // to get a different datapoint when the drive value was 0
                if (step == 0)
                    step = OPTIMIZER_DEFAULT_STEP;

                double new_drive_value = current_drive_value + step;

                // this can happen sometimes
                if (std::isnan(new_drive_value))
                {
                    throw new std::runtime_error("New drive value is NaN. This indicates that the model received some invalid drive values. Aborting optimization.");
                }

                model_handler_.setHarmonicDriveValue(name, HarmonicDriveParameters(new_drive_value, drive_type));

                // Compute the new bn values
                // get the current bn values
                calculator_.reload_and_calc(temp_json_file_path, harmonics_handler, disable_logging_);
                std::vector<double> new_bn_values = harmonics_handler.get_bn();
                double new_bn = new_bn_values[component - 1];
                Logger::info("Initial step yielded new bn value: " + std::to_string(new_bn) + " for new drive value: " + std::to_string(new_drive_value));

                // Add the new data point
                data_points.emplace_back(new_drive_value, new_bn);

                // do linear regression until the harmonics is optimized
                while (true)
                {
                    // Perform linear regression to find the root
                    double optimized_value = fitLinearGetRoot(data_points);

                    // Set the optimized value and recompute bn
                    model_handler_.setHarmonicDriveValue(name, HarmonicDriveParameters(optimized_value, drive_type));

                    calculator_.reload_and_calc(temp_json_file_path, harmonics_handler, disable_logging_);

                    std::vector<double> optimized_bn_values = harmonics_handler.get_bn();

                    // get the bn value for the component currently being optimized
                    double optimized_bn = optimized_bn_values[component - 1];
                    Logger::info("New bn value: " + std::to_string(optimized_bn) + " for new drive value: " + std::to_string(optimized_value));

                    // Add the new data point
                    data_points.emplace_back(optimized_value, optimized_bn);

                    // check if the optimization was successful or if it has to be aborted
                    if (std::abs(optimized_bn) <= max_harmonic_value_ || data_points.size() >= OPTIMIZER_MAX_DATAPOINTS)
                    {
                        // set the new values for the next iteration
                        current_bn_values_ = optimized_bn_values;
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
        }
    } while (!all_within_margin);
}