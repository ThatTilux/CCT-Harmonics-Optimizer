// Include necessary headers
#include <armadillo>
#include <iostream>
#include <cmath>
#include <complex>
#include <cassert>
#include <boost/filesystem.hpp>
#include "rat/common/log.hh"
#include "rat/models/serializer.hh"
#include "rat/mat/database.hh"
#include "rat/models/crossrectangle.hh"
#include "rat/models/modelcoil.hh"
#include "rat/models/modelgroup.hh"
#include "rat/models/pathcable.hh"
#include "rat/models/modeltoroid.hh"
#include "rat/models/pathrectangle.hh"
#include "rat/models/pathdshape.hh"
#include "rat/models/serializer.hh"
#include "rat/models/calcinductance.hh"
#include "rat/models/calcmesh.hh"
#include "rat/models/calcgrid.hh"
#include "rat/models/calcpolargrid.hh"
#include "rat/models/modelroot.hh"
#include "rat/models/calcgroup.hh"
#include "rat/models/calcharmonics.hh"

// function for importing a model from json
std::tuple<rat::mdl::ShModelPr, rat::mdl::ShModelRootPr, rat::mdl::ShModelGroupPr, rat::mdl::ShCalcGroupPr>
load_model_from_json(const boost::filesystem::path &json_file_path)
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

// Function to find the first CalcHarmonics and return its myname_ attribute
std::tuple<rat::mdl::ShCalcHarmonicsPr, std::string> find_first_calcharmonics(const rat::mdl::ShCalcGroupPr &calc_tree)
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
            // Access the name of the CalcHarmonics object itself
            std::string myname = harmonics_calc->get_name();
            return {harmonics_calc, myname};
        }
    }

    return {nullptr, ""};
}

int main()
{
    const boost::filesystem::path json_file_path = "../data/quad_double_HTS_3mm_22_5_ole_nokink_optimized_V02.json";

    auto [model, root, model_tree, calc_tree] = load_model_from_json(json_file_path);

    if (!model || !root || !model_tree || !calc_tree)
    {
        return -1;
    }

    // Find the first CalcHarmonics object in the calculation tree and get its myname_ attribute
    auto [harmonics_calc, harmonics_calc_name] = find_first_calcharmonics(calc_tree);

    if (harmonics_calc)
    {
        std::cout << "Found Harmonics Calculation with the name: " << harmonics_calc_name << std::endl;
    }
    else
    {
        std::cerr << "No Harmonics Calculation could be found in the calculation tree. Exiting." << std::endl;
        std::cout << "Press Enter to continue..." << std::endl;
        std::cin.get(); // Wait for the user to press any key
        return -1;
    }

    const bool run_coil_field = true;
    const bool run_grid = false;
    const bool run_polar_grid = false;
    const bool run_json_export = true;
    const bool run_inductance = false;

    const boost::filesystem::path output_dir = "./cct/";
    const rat::fltp output_time = RAT_CONST(0.0);

    const rat::cmn::ShLogPr lg = rat::cmn::Log::create(rat::cmn::Log::LogoType::RAT);

    if (run_coil_field)
    {
        const rat::mdl::ShCalcMeshPr mesh = rat::mdl::CalcMesh::create(model);
        mesh->calculate_write({output_time}, output_dir, lg);
    }

    if (run_grid)
    {
        const rat::fltp grid_size = RAT_CONST(0.8);
        const arma::uword grid_num_steps = 100;
        const rat::mdl::ShCalcGridPr grid = rat::mdl::CalcGrid::create(
            model, grid_size, grid_size, grid_size, grid_num_steps, grid_num_steps, grid_num_steps);
        grid->calculate_write({output_time}, output_dir, lg);
    }

    if (run_polar_grid)
    {
        const rat::fltp rin = 0.0, rout = 0.4, theta1 = 0, theta2 = arma::Datum<rat::fltp>::tau, zlow = -0.25, zhigh = 0.25;
        const arma::uword num_rad = 40, num_theta = 90, num_axial = 50;
        const rat::mdl::ShCalcPolarGridPr grid = rat::mdl::CalcPolarGrid::create(model, 'z', rin, rout, num_rad, theta1, theta2, num_theta, zlow, zhigh, num_axial);
        grid->calculate_write({output_time}, output_dir, lg);
    }

    if (run_inductance)
    {
        const rat::mdl::ShCalcInductancePr inductance_calculator = rat::mdl::CalcInductance::create(model);
        inductance_calculator->calculate_write({output_time}, output_dir, lg);
    }

    if (run_json_export)
    {
        rat::mdl::ShSerializerPr serializer = rat::mdl::Serializer::create();
        serializer->flatten_tree(model);
        serializer->export_json(output_dir / "model.json");
    }

    return 0;
}
