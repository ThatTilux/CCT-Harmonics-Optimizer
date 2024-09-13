#ifndef MODEL_CALCULATOR_H
#define MODEL_CALCULATOR_H

#include <armadillo>
#include <boost/filesystem.hpp>
#include <rat/common/log.hh>
#include <rat/models/serializer.hh>
#include <rat/models/modelroot.hh>
#include <rat/models/calcgroup.hh>
#include <rat/models/calcharmonics.hh>
#include <rat/models/calcmesh.hh>
#include <rat/models/pathaxis.hh>
#include <iostream>
#include <string>
#include <vector>
#include "harmonics_data_handler.h"
#include "mesh_data_handler.h"
#include "Logger.hh"

class ModelCalculator
{
public:
    ModelCalculator(const boost::filesystem::path &json_file_path);
    // dummy - do not use
    ModelCalculator();


    void calc_harmonics(HarmonicsDataHandler &harmonics_handler, bool disable_logging = true);
    void reload_and_calc_harmonics(const boost::filesystem::path &json_file_path, HarmonicsDataHandler &harmonics_handler, bool disable_logging = true);

    void calc_mesh(MeshDataHandler &mesh_handler, bool disable_logging = true);
    void reload_and_calc_mesh(const boost::filesystem::path &json_file_path, MeshDataHandler &mesh_handler, bool disable_logging = true);

    double get_axis_z_pos();
    double get_axis_ell();

    bool has_harmonics_calc();
    bool has_mesh_calc();

private:
    bool load_model(const boost::filesystem::path &json_file_path);
    std::tuple<rat::mdl::ShModelPr, rat::mdl::ShModelRootPr, rat::mdl::ShModelGroupPr, rat::mdl::ShCalcGroupPr>
    load_model_from_json(const boost::filesystem::path &json_file_path);
    void enable_gpu(rat::mdl::ShCalcLeafPr calc_leaf);
    void log_gpu_info();


    static std::tuple<rat::mdl::ShPathAxisPr, std::string> get_axis(const rat::mdl::ShCalcHarmonicsPr &harmonics_calc);

    // template for the find_first_calc function
    template <typename T>
    static std::tuple<std::shared_ptr<T>, std::string> find_first_calc(const rat::mdl::ShCalcGroupPr &calc_tree);

    rat::mdl::ShModelPr model_;
    rat::mdl::ShModelRootPr root_;
    rat::mdl::ShModelGroupPr model_tree_;
    rat::mdl::ShCalcGroupPr calc_tree_;
    rat::mdl::ShCalcHarmonicsPr harmonics_calc_;
    rat::mdl::ShCalcMeshPr mesh_calc_;
    rat::mdl::ShPathAxisPr harmonics_axis_;
    std::string harmonics_calc_name_;
    std::string mesh_calc_name_;
    std::string harmonics_axis_name_;
};

#endif // MODEL_CALCULATOR_H
