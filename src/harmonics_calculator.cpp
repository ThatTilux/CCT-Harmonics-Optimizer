#include "harmonics_calculator.h"

HarmonicsCalculator::HarmonicsCalculator(const boost::filesystem::path &json_file_path)
{
    if (!load_model(json_file_path))
    {
        std::cerr << "Failed to load model from JSON file." << std::endl;
    }
}

// Function to log the GPU information
void HarmonicsCalculator::log_gpu_info()
{
    #ifdef ENABLE_CUDA_KERNELS

    Logger::debug("Logging GPU information:");

    const rat::fmm::ShSettingsPr settings = harmonics_calc_->get_settings();
    std::set<int> gpus_available_for_calc = settings->get_gpu_devices();

    const int num_devices = rat::fmm::GpuKernels::get_num_devices();
    Logger::debug("Number of GPU devices: " + std::to_string(num_devices));


    for (const auto& gpu : gpus_available_for_calc)
    {
        Logger::debug("GPU available for calculation: " + std::to_string(gpu));
    }
    Logger::debug("Showing device info for device 0:");
    rat::fmm::GpuKernels::show_device_info(0, rat::cmn::Log::create());
    Logger::debug("");

    #endif
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
        Logger::error("No Harmonics Calculation could be found in the calculation tree. Exiting.");
        Logger::info("Press Enter to continue...");
        std::cin.get();
        return false;
    }

    Logger::info("Found Harmonics Calculation with the name: " + harmonics_calc_name_);
    return true;
}

// Function to set the GPU settings for the harmonics calculation. Will do nothing when no GPU is available.
void HarmonicsCalculator::enable_gpu(){
    rat::fmm::ShSettingsPr settings = harmonics_calc_->get_settings();
    // get the number of CUDA compatible GPU devices
    int num_gpu_devices = 0;

    #ifdef ENABLE_CUDA_KERNELS
        num_gpu_devices = rat::fmm::GpuKernels::get_num_devices();
    #endif
    
    if (num_gpu_devices > 0){
        // use the first GPU 
        settings->set_enable_gpu(true);
        settings->add_gpu_device(0);

        Logger::info("GPU enabled for Harmonics Calculation.");
    } else {
        Logger::info("No GPU available for Harmonics Calculation.");
    }
}


// function for doing the harmonics calculation. Will update create a new HarmonicsHandler object that provides access to the results.
void HarmonicsCalculator::calc(HarmonicsHandler& harmonics_handler, bool disable_logging)
{
    if (harmonics_calc_)
    {

        // do the harmonics calculation
        const rat::fltp output_time = RAT_CONST(0.0);
        const rat::cmn::ShLogPr lg = disable_logging ? rat::cmn::SilentLog::create() : rat::cmn::Log::create(rat::cmn::Log::LogoType::RAT);
        const rat::mdl::ShSolverCachePr cache = rat::mdl::SolverCache::create();

        // Use GPU for calculation if available
        enable_gpu();


        rat::mdl::ShHarmonicsDataPr harmonics_data = harmonics_calc_->calculate_harmonics(output_time, lg, cache);

        if (harmonics_data)
        {
            harmonics_handler = HarmonicsHandler(harmonics_data);
        }
        else
        {
            std::cerr << "Harmonics calculation failed." << std::endl;
            harmonics_handler = HarmonicsHandler();
        }
    }
}

// reloads the model from the json and computes the bn values
void HarmonicsCalculator::reload_and_calc(const boost::filesystem::path &json_file_path, HarmonicsHandler& harmonics_handler, bool disable_logging){
    load_model(json_file_path);
    Logger::info("Model reloaded.");
    calc(harmonics_handler, disable_logging);
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

// function to saerch the calculation tree of a model for a harmonics calculation. Will return the top-most harmonics calculation of the tree
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

bool HarmonicsCalculator::has_harmonics_calc(){
    return harmonics_calc_ != nullptr;
}