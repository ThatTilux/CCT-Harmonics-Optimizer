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

class HarmonicsCalculator
{
public:
    HarmonicsCalculator(const boost::filesystem::path &json_file_path);

    std::vector<rat::fltp> compute_bn();

private:
    bool load_model(const boost::filesystem::path &json_file_path);
    std::tuple<rat::mdl::ShModelPr, rat::mdl::ShModelRootPr, rat::mdl::ShModelGroupPr, rat::mdl::ShCalcGroupPr>
    load_model_from_json(const boost::filesystem::path &json_file_path);

    std::tuple<rat::mdl::ShCalcHarmonicsPr, std::string> find_first_calcharmonics(const rat::mdl::ShCalcGroupPr &calc_tree);
    std::vector<rat::fltp> convert_bn_to_vector(const arma::Row<rat::fltp>& bn);

    rat::mdl::ShModelPr model_;
    rat::mdl::ShModelRootPr root_;
    rat::mdl::ShModelGroupPr model_tree_;
    rat::mdl::ShCalcGroupPr calc_tree_;
    rat::mdl::ShCalcHarmonicsPr harmonics_calc_;
    std::string harmonics_calc_name_;
};

void print_bn(const std::vector<rat::fltp>& bn_values);

#endif // HARMONICS_CALCULATOR_H
