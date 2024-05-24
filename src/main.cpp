#include "harmonics_calculator.h"
#include "model_handler.h"

int main()
{
    const boost::filesystem::path json_file_path = "../data/quad_double_HTS_3mm_22_5_ole_nokink_optimized_V03.json";
    
    // handles manipulations of the json file
    ModelHandler model_handler(json_file_path);
    // handles calculations for the model
    HarmonicsCalculator calculator(json_file_path);

    // get all the scaling values for the custom cct harmonics
    std::vector<std::pair<int, double>> harmonic_drive_values = model_handler.getHarmonicDriveValues();

    // print them
    std::cout << "Harmonic Drive Values:" << std::endl;
    for (const auto& value : harmonic_drive_values) {
        std::cout << "B" << value.first << ": " << value.second << std::endl;
    }




    auto bn_values = calculator.compute_bn();
    if (!bn_values.empty())
    {
        print_bn(bn_values);
    }
    else
    {
        std::cerr << "Failed to compute bn values." << std::endl;
    }

    return 0;
}
