#ifndef HARMONICS_CALCULATOR_H
#define HARMONICS_CALCULATOR_H

#include <armadillo>
#include <boost/filesystem.hpp>
#include <rat/common/log.hh>
#include <rat/models/serializer.hh>
#include <rat/models/modelroot.hh>
#include <rat/models/calcgroup.hh>
#include <rat/models/calcharmonics.hh>
#include <iostream>
#include <string>
#include <vector>
#include "harmonics_handler.h"


class HarmonicsCalculator
{
public:
    HarmonicsCalculator(const boost::filesystem::path &json_file_path);
    // dummy - do not use
    HarmonicsCalculator();

    void calc(HarmonicsHandler &harmonics_handler, bool disable_logging = true);
    void reload_and_calc(const boost::filesystem::path &json_file_path, HarmonicsHandler &harmonics_handler, bool disable_logging = true);

    bool has_harmonics_calc();

private:
    bool load_model(const boost::filesystem::path &json_file_path);
    std::tuple<rat::mdl::ShModelPr, rat::mdl::ShModelRootPr, rat::mdl::ShModelGroupPr, rat::mdl::ShCalcGroupPr>
    load_model_from_json(const boost::filesystem::path &json_file_path);
    void enable_gpu();
    std::tuple<rat::mdl::ShCalcHarmonicsPr, std::string> find_first_calcharmonics(const rat::mdl::ShCalcGroupPr &calc_tree);
    void log_gpu_info();

    rat::mdl::ShModelPr model_;
    rat::mdl::ShModelRootPr root_;
    rat::mdl::ShModelGroupPr model_tree_;
    rat::mdl::ShCalcGroupPr calc_tree_;
    rat::mdl::ShCalcHarmonicsPr harmonics_calc_;
    std::string harmonics_calc_name_;
};

#endif // HARMONICS_CALCULATOR_H
