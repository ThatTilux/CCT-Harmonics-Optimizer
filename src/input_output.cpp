#include "input_output.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <constants.h>
#include <chrono>

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
