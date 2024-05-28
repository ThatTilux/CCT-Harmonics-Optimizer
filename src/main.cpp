#include "harmonics_calculator.h"
#include "model_handler.h"
#include <iostream>
#include <limits>
#include <vector>
#include <numeric>
#include <termios.h>
#include <unistd.h>
#include <boost/filesystem.hpp>


// CONSTANTS
std::string DATA_DIR_PATH = "../data";
std::string MODEL_OUTPUT_DIR = "./optimized_cct/";



// Function to print harmonic drive values
void print_harmonic_drive_values(const std::vector<std::pair<int, double>> &harmonic_drive_values)
{
    std::cout << "Harmonic Drive Values: (units are m/coil and m)" << std::endl;
    for (const auto &value : harmonic_drive_values)
    {
        std::cout << "B" << value.first << ": " << value.second << std::endl;
    }
}

// Function to get a single character input without echoing to the console (POSIX)
char getch()
{
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(STDIN_FILENO, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(STDIN_FILENO, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}

// Function to ask the user if they want to proceed
bool askUserToProceed()
{
    std::string input;
    std::cout << "The harmonic drive values above will be optimized to achieve bn values within the maximum value specified above. Do you want to proceed with the optimization? (Y/n): ";
    std::getline(std::cin, input);
    return input == "Y" || input == "y";
}

// Function to get user input with a default value
double getUserInput(const std::string &prompt, double default_value)
{
    double value = 0;
    std::string input;

    while (value == 0)
    {
        std::cout << prompt << " (default: " << default_value << "): ";
        std::getline(std::cin, input);

        if (input.empty())
        {
            value = default_value;
        }
        else
        {
            try
            {
                value = std::stod(input);
                if (value <= 0)
                {
                    value = 0;
                    std::cerr << "Input must be greater than 0. Please try again." << std::endl;
                }
            }
            catch (...)
            {
                std::cerr << "Invalid input. Please enter a valid number." << std::endl;
            }
        }
    }

    std::cout << "Using " << value << " as maximum absolute bn value." << std::endl;
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
    boost::filesystem::path dest_path = MODEL_OUTPUT_DIR + new_filename;

    try
    {
        boost::filesystem::create_directory(MODEL_OUTPUT_DIR);
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

// Function to display files in a directory and allow user to select one
boost::filesystem::path selectJsonFile()
{
    boost::filesystem::path dir_path(DATA_DIR_PATH);
    std::vector<boost::filesystem::path> json_files;

    if (boost::filesystem::exists(dir_path) && boost::filesystem::is_directory(dir_path))
    {
        for (const auto &entry : boost::filesystem::directory_iterator(dir_path))
        {
            if (boost::filesystem::is_regular_file(entry) && entry.path().extension() == ".json")
            {
                json_files.push_back(entry.path());
            }
        }
    }

    if (json_files.empty())
    {
        throw std::runtime_error("No JSON files found in the " + DATA_DIR_PATH + " directory. Please add the JSON file of the model you wish to optimize there.");
    }

    int selected_index = 0;
    char key;
    while (true)
    {
        system("clear"); // Clear the terminal screen on POSIX systems
        std::cout << "Select the JSON file for the model you wish to optimize. If your model is not in the list, make sure it is placed in the " << DATA_DIR_PATH << " directory."<< std::endl;
        std::cout << "Use arrow keys and enter to select."<< std::endl;
        for (size_t i = 0; i < json_files.size(); ++i)
        {
            if (i == selected_index)
            {
                std::cout << "> " << json_files[i].filename().string() << std::endl;
            }
            else
            {
                std::cout << "  " << json_files[i].filename().string() << std::endl;
            }
        }

        key = getch();

        if (key == '\033')
        { // arrow keys for POSIX
            getch();    // skip the [
            switch (getch())
            {
            case 'A': // up
                if (selected_index > 0)
                    selected_index--;
                break;
            case 'B': // down
                if (selected_index < json_files.size() - 1)
                    selected_index++;
                break;
            }
        }
        else if (key == '\r' || key == '\n')
        {
            break;
        }
    }

    return json_files[selected_index];
}

int main()
{
    // Get the JSON file path from user selection
    boost::filesystem::path json_file_path;
    try
    {
        json_file_path = selectJsonFile();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

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

    // Check that there are harmonic drives
    if (harmonic_drive_values.empty())
    {
        std:cerr << "The program could not find any custom CCT harmonics (rat::mdl::cctharmonicdrive) whose name starts with the letter 'B'. Aborting..." << std::endl;
        return 1;
    }

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
