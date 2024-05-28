#include "harmonics_calculator.h"
#include "model_handler.h"
#include <iostream>
#include <limits>
#include <vector>
#include <numeric>

// Function to print harmonic drive values
void print_harmonic_drive_values(const std::vector<std::pair<int, double>> &harmonic_drive_values)
{
    std::cout << "Harmonic Drive Values: (units are m/coil and m)" << std::endl;
    for (const auto &value : harmonic_drive_values)
    {
        std::cout << "B" << value.first << ": " << value.second << std::endl;
    }
}

// Function to ask the user if they want to proceed
bool askUserToProceed()
{
    std::string input;
    std::cout << "The harmonic drive values above will be optimized to achieve bn values within the maximum value specified above. Do you want to proceed with the optimization? (Y/n): ";
    std::getline(std::cin, input);
    return input.empty() || input == "Y" || input == "y";
}

// Function to get user input with a default value
double getUserInput(const std::string &prompt, double default_value)
{
    std::cout << prompt << " (default: " << default_value << "): ";
    double value;
    std::string input;
    std::getline(std::cin, input);
    try
    {
        if (input.empty())
        {
            value = default_value;
        }
        else
        {
            value = std::stod(input);
        }
        std::cout << "Using " << value << " as maximum absolute bn value." << std::endl;
    }
    catch (...)
    {
        std::cerr << "Invalid input. Using default value: " << default_value << std::endl;
        value = default_value;
    }
    return value;
}

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

// Function to copy model from src to the build dir with appending timestamp
void copyModelWithTimestamp(const boost::filesystem::path &src_path)
{
    if (!boost::filesystem::exists(src_path))
    {
        std::cerr << "Source file does not exist: " << src_path << std::endl;
        return;
    }

    std::string filename = src_path.filename().string();
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    std::string base_filename = src_path.stem().string();  // filename without extension
    std::string extension = src_path.extension().string(); // file extension

    std::string new_filename = base_filename + "_" + std::to_string(now_sec) + extension;
    boost::filesystem::path dest_path = "./optimized_cct/" + new_filename;

    try
    {
        boost::filesystem::create_directory("./optimized_cct");
        boost::filesystem::copy_file(src_path, dest_path);

        // add the build path for clarity
        std::string modified_dest_path = dest_path.string();
        modified_dest_path.insert(1, "/build");

        // print to console
        std::cout << "The optimized model has been exported to: " << modified_dest_path << std::endl;
    }
    catch (const boost::filesystem::filesystem_error &e)
    {
        std::cerr << "Error while exporting optimized model: " << e.what() << std::endl;
        std::cerr << "The optimized model has instead been saved to: " << src_path << std::endl;
    }
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

                    if (std::abs(optimized_bn) <= max_harmonic_value)
                    {
                        // set the new values for the next iteration
                        current_bn_values = optimized_bn_values;
                        harmonic.second = optimized_value;
                        // print new harmonic drive value to console console
                        std::cout << "Optimized " << name << " with drive value " << optimized_value << " and bn value: " << optimized_bn << std::endl;
                        break;
                    }

                    current_value = optimized_value;
                    current_bn = optimized_bn;
                }
            }
        }
    } while (!all_within_margin);
}

int main()
{
    const boost::filesystem::path json_file_path = "../data/quad_double_HTS_3mm_22_5_ole_nokink_optimized_V04.json";

    // Handles manipulations of the JSON file
    ModelHandler model_handler(json_file_path);
    // get path of the temp model
    const boost::filesystem::path temp_json_file_path = model_handler.getTempJsonPath();
    // Handles calculations for the model
    HarmonicsCalculator calculator(json_file_path);

    // Get user input for maximum harmonic value
    double max_harmonic_value = getUserInput("Enter the maximum absolute value for harmonic values", 0.1);

    // Get all the scaling values for the custom CCT harmonics
    std::vector<std::pair<int, double>> harmonic_drive_values = model_handler.getHarmonicDriveValues();

    // Print them
    print_harmonic_drive_values(harmonic_drive_values);

    // Ask the user if they want to proceed
    if (!askUserToProceed())
    {
        std::cout << "Optimization aborted by user." << std::endl;
        return 0;
    }

    // optimizer will put resulting bn values in here
    std::vector<double> current_bn_values;

    // optimize the harmonic drive values
    optimize(calculator, model_handler, current_bn_values, harmonic_drive_values, max_harmonic_value, temp_json_file_path);

    // optimization was successfull, print results
    std::cout << "=== All harmonics have been optimized ===" << std::endl;
    std::cout << "User-specified margin was: " << max_harmonic_value << std::endl;
    print_harmonic_drive_values(harmonic_drive_values);
    print_bn(current_bn_values);

    // export the model
    copyModelWithTimestamp(temp_json_file_path);

    return 0;
}
