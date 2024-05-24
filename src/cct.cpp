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

int main() {
    // INPUT SETTINGS
    const boost::filesystem::path json_file_path = "../data/cct.json";
    
    if (!boost::filesystem::exists(json_file_path)) {
        std::cerr << "JSON file not found: " << json_file_path << std::endl;
        return -1;
    }

    rat::mdl::ShSerializerPr serializer = rat::mdl::Serializer::create();
    serializer->import_json(json_file_path);

    if (!serializer->has_valid_json_root()) {
        std::cerr << "Invalid JSON root in file: " << json_file_path << std::endl;
        return -1;
    }
    const rat::mdl::ShModelPr model = serializer->construct_tree<rat::mdl::Model>();

    if (!model) {
        std::cerr << "Failed to construct model from JSON file." << std::endl;
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

    if (run_coil_field) {
        const rat::mdl::ShCalcMeshPr mesh = rat::mdl::CalcMesh::create(model);
        mesh->calculate_write({output_time}, output_dir, lg);
    }

    if (run_grid) {
        const rat::fltp grid_size = RAT_CONST(0.8);
        const arma::uword grid_num_steps = 100;
        const rat::mdl::ShCalcGridPr grid = rat::mdl::CalcGrid::create(
            model, grid_size, grid_size, grid_size, grid_num_steps, grid_num_steps, grid_num_steps);
        grid->calculate_write({output_time}, output_dir, lg);
    }

    if (run_polar_grid) {
        const rat::fltp rin = 0.0, rout = 0.4, theta1 = 0, theta2 = arma::Datum<rat::fltp>::tau, zlow = -0.25, zhigh = 0.25;
        const arma::uword num_rad = 40, num_theta = 90, num_axial = 50;
        const rat::mdl::ShCalcPolarGridPr grid = rat::mdl::CalcPolarGrid::create(model, 'z', rin, rout, num_rad, theta1, theta2, num_theta, zlow, zhigh, num_axial);
        grid->calculate_write({output_time}, output_dir, lg);
    }

    if (run_inductance) {
        const rat::mdl::ShCalcInductancePr inductance_calculator = rat::mdl::CalcInductance::create(model);
        inductance_calculator->calculate_write({output_time}, output_dir, lg);
    }

    if (run_json_export) {
        serializer->flatten_tree(model);
        serializer->export_json(output_dir / "model.json");
    }

    return 0;
}
