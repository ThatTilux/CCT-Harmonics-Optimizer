#include "model_calculator.h"

ModelCalculator::ModelCalculator(const boost::filesystem::path &json_file_path)
{
    if (!load_model(json_file_path))
    {
        Logger::error("Failed to load model from JSON file.");
    }
}

// dummy constructor - do not use
ModelCalculator::ModelCalculator()
{
}

// Function to log the GPU information
void ModelCalculator::log_gpu_info()
{
#ifdef ENABLE_CUDA_KERNELS

    Logger::debug("Logging GPU information:");

    const rat::fmm::ShSettingsPr settings = harmonics_calc_->get_settings();
    std::set<int> gpus_available_for_calc = settings->get_gpu_devices();

    const int num_devices = rat::fmm::GpuKernels::get_num_devices();
    Logger::debug("Number of GPU devices: " + std::to_string(num_devices));

    for (const auto &gpu : gpus_available_for_calc)
    {
        Logger::debug("GPU available for calculation: " + std::to_string(gpu));
    }
    Logger::debug("Showing device info for device 0:");
    rat::fmm::GpuKernels::show_device_info(0, rat::cmn::Log::create());
    Logger::debug("");

#endif
}

bool ModelCalculator::load_model(const boost::filesystem::path &json_file_path)
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

    std::tie(harmonics_calc_, harmonics_calc_name_) = find_first_calc<rat::mdl::CalcHarmonics>(calc_tree_);
    std::tie(mesh_calc_, mesh_calc_name_) = find_first_calc<rat::mdl::CalcMesh>(calc_tree_);
    std::tie(harmonics_axis_, harmonics_axis_name_) = get_axis(harmonics_calc_);

    if (!harmonics_calc_ || !mesh_calc_ || !harmonics_axis_)
    {
        Logger::error("No Mesh Calculation or Harmonics Calculation with Axis Path could be found in the calculation tree. Exiting.");
        Logger::info("Press Enter to continue...");
        std::cin.get();
        return false;
    }

    // log results

    static bool logged_calc_names = false;
    if (!logged_calc_names)
    {
        if (harmonics_calc_)
        {
            Logger::info("Found Harmonics Calculation with the name: " + harmonics_calc_name_);
        }
        if (mesh_calc_)
        {
            Logger::info("Found Mesh Calculation with the name: " + mesh_calc_name_);
        }
        if (harmonics_axis_)
        {
            Logger::info("Found Axis Path with the name: " + harmonics_axis_name_);
        }
        logged_calc_names = true;
    }

    return true;
}

// Function to set the GPU settings for the calculation. Will do nothing when no GPU is available.
void ModelCalculator::enable_gpu(rat::mdl::ShCalcLeafPr calc_leaf)
{
    rat::fmm::ShSettingsPr settings = calc_leaf->get_settings();
    // get the number of CUDA compatible GPU devices
    int num_gpu_devices = 0;

#ifdef ENABLE_CUDA_KERNELS
    num_gpu_devices = rat::fmm::GpuKernels::get_num_devices();
#endif

    if (num_gpu_devices > 0)
    {
        // use the first GPU
        settings->set_enable_gpu(true);
        settings->add_gpu_device(0);

        // log GPU activation (only once per calc)
        static std::map<std::string, bool> logged_gpu;
        if (!logged_gpu[calc_leaf->get_name()])
        {
            Logger::info("GPU enabled for " + calc_leaf->get_name() + ".");
            logged_gpu[calc_leaf->get_name()] = true;
        }
    }
    else
    {
        // log no GPU available (only once per type)
        static std::map<std::string, bool> logged_no_gpu;
        if (!logged_no_gpu[calc_leaf->get_name()])
        {
            Logger::info("No GPU available for " + calc_leaf->get_name() + ". Running on CPU.");
            logged_no_gpu[calc_leaf->get_name()] = true;
        }
    }
}

// function for doing the harmonics calculation. Will update a HarmonicsHandler object that provides access to the results.
void ModelCalculator::calc_harmonics(HarmonicsDataHandler &harmonics_handler, bool disable_logging)
{
    if (harmonics_calc_)
    {

        Logger::info("Running harmonics calculation...");

        // do the harmonics calculation
        const rat::fltp output_time = RAT_CONST(0.0);
        const rat::cmn::ShLogPr lg = disable_logging ? rat::cmn::NullLog::create() : rat::cmn::Log::create(rat::cmn::Log::LogoType::RAT);
        const rat::mdl::ShSolverCachePr cache = rat::mdl::SolverCache::create();

        // Use GPU for calculation if available
        enable_gpu(harmonics_calc_);

        rat::mdl::ShHarmonicsDataPr harmonics_data = harmonics_calc_->calculate_harmonics(output_time, lg, cache);

        if (harmonics_data)
        {
            harmonics_handler = HarmonicsDataHandler(harmonics_data);
        }
        else
        {
            std::cerr << "Harmonics calculation failed." << std::endl;
            harmonics_handler = HarmonicsDataHandler();
        }
    }
}

// function for doing the mesh calculation. Will update a mesh handler object that provides access to the results.
void ModelCalculator::calc_mesh(MeshDataHandler &mesh_handler, bool disable_logging)
{
    if (mesh_calc_)
    {

        Logger::info("Running mesh calculation...");

        // do the mesh calculation
        const rat::fltp output_time = RAT_CONST(0.0);
        const rat::cmn::ShLogPr lg = disable_logging ? rat::cmn::NullLog::create() : rat::cmn::Log::create(rat::cmn::Log::LogoType::RAT);
        const rat::mdl::ShSolverCachePr cache = rat::mdl::SolverCache::create();

        // Use GPU for calculation if available
        enable_gpu(mesh_calc_);

        std::list<rat::mdl::ShMeshDataPr> mesh_data = mesh_calc_->calculate_mesh(output_time, lg, cache);

        if (!mesh_data.empty())
        {
            mesh_handler = MeshDataHandler(mesh_data);
        }
        else
        {
            std::cerr << "Mesh calculation failed." << std::endl;
            mesh_handler = MeshDataHandler();
        }
    }
}

// reloads the model from the json and computes the bn values
void ModelCalculator::reload_and_calc_harmonics(const boost::filesystem::path &json_file_path, HarmonicsDataHandler &harmonics_handler, bool disable_logging)
{
    load_model(json_file_path);
    calc_harmonics(harmonics_handler, disable_logging);
}

// reloads the model from the json and computes the mesh
void ModelCalculator::reload_and_calc_mesh(const boost::filesystem::path &json_file_path, MeshDataHandler &mesh_handler, bool disable_logging)
{
    load_model(json_file_path);
    calc_mesh(mesh_handler, disable_logging);
}

std::tuple<rat::mdl::ShModelPr, rat::mdl::ShModelRootPr, rat::mdl::ShModelGroupPr, rat::mdl::ShCalcGroupPr>
ModelCalculator::load_model_from_json(const boost::filesystem::path &json_file_path)
{
    if (!boost::filesystem::exists(json_file_path))
    {
        Logger::error("JSON file not found: " + json_file_path.string());
        return {nullptr, nullptr, nullptr, nullptr};
    }

    rat::mdl::ShSerializerPr serializer = rat::mdl::Serializer::create();
    serializer->import_json(json_file_path);

    if (!serializer->has_valid_json_root())
    {
        Logger::error("Invalid JSON root in file: " + json_file_path.string());
        return {nullptr, nullptr, nullptr, nullptr};
    }

    const rat::mdl::ShModelPr model = serializer->construct_tree<rat::mdl::Model>();

    if (!model)
    {
        Logger::error("Failed to construct model from JSON file.");
        return {nullptr, nullptr, nullptr, nullptr};
    }

    const rat::mdl::ShModelRootPr root = std::dynamic_pointer_cast<rat::mdl::ModelRoot>(model);

    if (!root)
    {
        Logger::error("Failed to cast model to ModelRoot.");
        return {nullptr, nullptr, nullptr, nullptr};
    }

    rat::mdl::ShModelGroupPr model_tree = root->get_model_tree();
    rat::mdl::ShCalcGroupPr calc_tree = root->get_calc_tree();

    if (!model_tree || !calc_tree)
    {
        Logger::error("Failed to extract model or calculation tree from the root.");
        return {nullptr, nullptr, nullptr, nullptr};
    }

    return {model, root, model_tree, calc_tree};
}

// function to search the calculation tree of a model for a calculation of the specified type. Will return the top-most calculation with that type of the tree
template <typename T>
std::tuple<std::shared_ptr<T>, std::string> ModelCalculator::find_first_calc(const rat::mdl::ShCalcGroupPr &calc_tree)
{
    // Ensure T is derived from rat::mdl::CalcLeaf
    static_assert(std::is_base_of<rat::mdl::CalcLeaf, T>::value, "T must be derived from rat::mdl::CalcLeaf");

    if (!calc_tree)
    {
        return {nullptr, ""};
    }

    for (const auto &calc : calc_tree->get_calculations())
    {
        auto specific_calc = std::dynamic_pointer_cast<T>(calc);
        if (specific_calc)
        {
            std::string myname = specific_calc->get_name();
            return {specific_calc, myname};
        }
    }

    return {nullptr, ""};
}

// Explicit template instantiation - define all possible types of T here
template std::tuple<std::shared_ptr<rat::mdl::CalcHarmonics>, std::string> ModelCalculator::find_first_calc(const rat::mdl::ShCalcGroupPr &calc_tree);
template std::tuple<std::shared_ptr<rat::mdl::CalcMesh>, std::string> ModelCalculator::find_first_calc(const rat::mdl::ShCalcGroupPr &calc_tree);

// Function to retieve the axis from a harmonics calculation
std::tuple<rat::mdl::ShPathAxisPr, std::string> ModelCalculator::get_axis(const rat::mdl::ShCalcHarmonicsPr &harmonics_calc)
{
    if (!harmonics_calc)
    {
        return {nullptr, ""};
    }

    rat::mdl::ShPathPr path = harmonics_calc->get_input_path();
    rat::mdl::ShPathAxisPr axis = std::dynamic_pointer_cast<rat::mdl::PathAxis>(path);
    std::string name = axis->get_name();

    return {axis, name};
}

// Function to get the z position in m of an axis with respect to all transformations
double ModelCalculator::get_axis_z_pos()
{
    if (!harmonics_axis_)
    {
        throw std::runtime_error("Axis has not been initialized yet. Cannot retrieve z position.");
    }

    // position without transformations
    arma::dvec3 pos = harmonics_axis_->get_position();

    // get all transformations
    std::list<rat::mdl::ShTransPr> transformations = harmonics_axis_->get_transformations();

    // apply transformations to position
    for (auto &trans : transformations)
    {
        trans->apply_coords(pos, 0);
    }

    // return z
    return pos(2);
}

double ModelCalculator::get_axis_ell()
{
    if (!harmonics_axis_)
    {
        throw std::runtime_error("Axis has not been initialized yet. Cannot retrieve ell.");
    }

    return harmonics_axis_->get_ell();

}

bool ModelCalculator::has_harmonics_calc()
{
    return harmonics_calc_ != nullptr;
}

bool ModelCalculator::has_mesh_calc()
{
    return mesh_calc_ != nullptr;
}