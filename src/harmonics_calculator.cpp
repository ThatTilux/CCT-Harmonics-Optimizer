#include "harmonics_calculator.h"

HarmonicsCalculator::HarmonicsCalculator(const boost::filesystem::path &json_file_path)
{
    if (!load_model(json_file_path))
    {
        std::cerr << "Failed to load model from JSON file." << std::endl;
    }
}

bool HarmonicsCalculator::load_model(const boost::filesystem::path &json_file_path)
{
    auto [model, root, model_tree, calc_tree] = load_model_from_json(json_file_path);
    if (!model || !root || !model_tree || !calc_tree)
    {
        return false;
    }

    model_ = model;
    root_ = root;
    model_tree_ = model_tree;
    calc_tree_ = calc_tree;

    std::tie(harmonics_calc_, harmonics_calc_name_) = find_first_calcharmonics(calc_tree_);
    if (!harmonics_calc_)
    {
        std::cerr << "No Harmonics Calculation could be found in the calculation tree. Exiting." << std::endl;
        std::cout << "Press Enter to continue..." << std::endl;
        std::cin.get();
        return false;
    }

    std::cout << "Found Harmonics Calculation with the name: " << harmonics_calc_name_ << std::endl;
    return true;
}

std::vector<double> HarmonicsCalculator::compute_bn()
{
    if (harmonics_calc_)
    {
        const rat::fltp output_time = RAT_CONST(0.0);
        const rat::cmn::ShLogPr lg = rat::cmn::Log::create(rat::cmn::Log::LogoType::RAT);
        const rat::mdl::ShSolverCachePr cache = rat::mdl::SolverCache::create();

        auto harmonics_data = harmonics_calc_->calculate_harmonics(output_time, lg, cache);

        if (harmonics_data)
        {
            arma::Row<rat::fltp> An, Bn;
            harmonics_data->get_harmonics(An, Bn);

            const arma::uword idx = arma::index_max(arma::max(arma::abs(An), arma::abs(Bn)));
            const rat::fltp ABmax = std::max(std::abs(An(idx)), std::abs(Bn(idx)));
            const arma::Row<rat::fltp> an = 1e4 * An / ABmax;
            const arma::Row<rat::fltp> bn = 1e4 * Bn / ABmax;

            return convert_bn_to_vector(bn);
        }
        else
        {
            std::cerr << "Harmonics calculation failed." << std::endl;
        }
    }
    return {};
}

// reloads the model from the json and computes the bn values
std::vector<double> HarmonicsCalculator::reload_and_compute_bn(const boost::filesystem::path &json_file_path){
    std::cout << "Reloading model..." << std::endl;
    load_model(json_file_path);
    return compute_bn();
}

std::tuple<rat::mdl::ShModelPr, rat::mdl::ShModelRootPr, rat::mdl::ShModelGroupPr, rat::mdl::ShCalcGroupPr>
HarmonicsCalculator::load_model_from_json(const boost::filesystem::path &json_file_path)
{
    if (!boost::filesystem::exists(json_file_path))
    {
        std::cerr << "JSON file not found: " << json_file_path << std::endl;
        return {nullptr, nullptr, nullptr, nullptr};
    }

    rat::mdl::ShSerializerPr serializer = rat::mdl::Serializer::create();
    serializer->import_json(json_file_path);

    if (!serializer->has_valid_json_root())
    {
        std::cerr << "Invalid JSON root in file: " << json_file_path << std::endl;
        return {nullptr, nullptr, nullptr, nullptr};
    }

    const rat::mdl::ShModelPr model = serializer->construct_tree<rat::mdl::Model>();

    if (!model)
    {
        std::cerr << "Failed to construct model from JSON file." << std::endl;
        return {nullptr, nullptr, nullptr, nullptr};
    }

    const rat::mdl::ShModelRootPr root = std::dynamic_pointer_cast<rat::mdl::ModelRoot>(model);

    if (!root)
    {
        std::cerr << "Failed to cast model to ModelRoot." << std::endl;
        return {nullptr, nullptr, nullptr, nullptr};
    }

    rat::mdl::ShModelGroupPr model_tree = root->get_model_tree();
    rat::mdl::ShCalcGroupPr calc_tree = root->get_calc_tree();

    if (!model_tree || !calc_tree)
    {
        std::cerr << "Failed to extract model or calculation tree from the root." << std::endl;
        return {nullptr, nullptr, nullptr, nullptr};
    }

    return {model, root, model_tree, calc_tree};
}

std::tuple<rat::mdl::ShCalcHarmonicsPr, std::string> HarmonicsCalculator::find_first_calcharmonics(const rat::mdl::ShCalcGroupPr &calc_tree)
{
    if (!calc_tree)
    {
        return {nullptr, ""};
    }

    for (const auto &calc : calc_tree->get_calculations())
    {
        auto harmonics_calc = std::dynamic_pointer_cast<rat::mdl::CalcHarmonics>(calc);
        if (harmonics_calc)
        {
            std::string myname = harmonics_calc->get_name();
            return {harmonics_calc, myname};
        }
    }

    return {nullptr, ""};
}

std::vector<double> HarmonicsCalculator::convert_bn_to_vector(const arma::Row<rat::fltp>& bn)
{
    std::vector<double> bn_values;
    for (arma::uword i = 1; i <= std::min(10u, (unsigned int)(bn.n_elem - 1)); ++i)
    {
        bn_values.push_back(bn(i));
    }
    return bn_values;
}

// Function to print bn values from a vector
void print_bn(const std::vector<double>& bn_values)
{
    std::cout << "bn values:" << std::endl;
    for (size_t i = 0; i < bn_values.size(); ++i)
    {
        std::cout << "bn[" << i + 1 << "] = " << bn_values[i] << std::endl;
    }
}
