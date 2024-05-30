#ifndef HARMONICS_HANDLER_H
#define HARMONICS_HANDLER_H

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
#include <filesystem>
#include <input_output.h>

class HarmonicsHandler
{
public:
    HarmonicsHandler();
    HarmonicsHandler(rat::mdl::ShHarmonicsDataPr harmonics_data);
    std::vector<double> get_ell();
    std::vector<double> get_Bn(int component);
    std::vector<double> get_bn();
    void export_Bns_to_csv(const std::string& dir_path);

private:
    std::vector<double> convert_bn_to_vector(const arma::Row<rat::fltp> &bn);
    std::vector<double> extract_bn(rat::mdl::ShHarmonicsDataPr harmonics_data);
    std::vector<double> convert_row_to_vector(const arma::Row<rat::fltp> &row);
    std::vector<double> convert_col_to_vector(const arma::Col<rat::fltp>& col);
    std::tuple<std::vector<double>, std::vector<std::vector<double>>> extract_ell_Bn(rat::mdl::ShHarmonicsDataPr harmonics_data);


    rat::mdl::ShHarmonicsDataPr harmonics_data_;
    std::vector<double> bn_;
    std::vector<double> ell_;
    std::vector<std::vector<double>> Bn_per_component_;
};

#endif // HARMONICS_HANDLER_H
