#include "harmonics_calculator.h"

int main()
{
    const boost::filesystem::path json_file_path = "../data/quad_double_HTS_3mm_22_5_ole_nokink_optimized_V02.json";
    HarmonicsCalculator calculator(json_file_path);

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
